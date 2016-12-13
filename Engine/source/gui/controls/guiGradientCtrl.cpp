//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#include "console/console.h"
#include "gfx/gfxDevice.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/controls/guiGradientCtrl.h"
#include "gui/controls/guiColorPicker.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

//-----------------------------------------------------------------------------
// GuiGradientSwatchCtrl

IMPLEMENT_CONOBJECT(GuiGradientSwatchCtrl);

ConsoleDocClass( GuiGradientSwatchCtrl,
   "@brief Swatch selector that appears inside the GuiGradientCtrl object. These objects are automatically created by GuiGradientCtrl. \n\n"
   "Currently only appears to be editor specific\n\n"
   "@see GuiSwatchButtonCtrl\n"
   "@see GuiGradientCtrl\n\n"
   "@ingroup GuiCore\n"
   "@internal"
);

IMPLEMENT_CALLBACK( GuiGradientSwatchCtrl, onMouseDown, void, (),(),
   "@brief Called whenever the left mouse button has entered the down state while in this control.\n\n"
   "@tsexample\n"
   "// The left mouse button is down on the control, causing the callback to occur.\n"
   "GuiGradientSwatchCtrl::onMouseDown(%this)\n"
   "	{\n"
   "		// Code to run when the callback occurs\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n"
   "@see GuiSwatchButtonCtrl\n\n"
   "@internal"
);

IMPLEMENT_CALLBACK( GuiGradientSwatchCtrl, onDoubleClick, void, (),(),
   "@brief Called whenever the left mouse button performs a double click while in this control.\n\n"
   "@tsexample\n"
   "// The left mouse button has performed a double click on the control, causing the callback to occur.\n"
   "GuiGradientSwatchCtrl::onDoubleClick(%this)\n"
   "	{\n"
   "		// Code to run when the callback occurs\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n"
   "@see GuiSwatchButtonCtrl\n\n"
   "@internal"
);


GuiGradientSwatchCtrl::GuiGradientSwatchCtrl()
{
	setPosition(0, 0);
	setExtent(14, 14);
	mMouseDownPosition = Point2I(0, 0);
	mSwatchColor = ColorI( 1, 1, 1, 1 );
	mColorFunction = StringTable->insert("getColorF");
	setDataField( StringTable->insert("Profile"), NULL, "GuiInspectorSwatchButtonProfile" );
}

bool GuiGradientSwatchCtrl::onWake()
{
	if ( !Parent::onWake() )
      return false;
	
	static const U32 bufSize = 512;
	char* altCommand = Con::getReturnBuffer(bufSize);
	dSprintf( altCommand, bufSize, "%s(%i.color, \"%i.setColor\");", mColorFunction, getId(), getId() );
	setField( "altCommand", altCommand );

	return true;
}

void GuiGradientSwatchCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   bool highlight = mMouseOver;

   ColorI borderColor = mActive ? ( highlight ? mProfile->mBorderColorHL : mProfile->mBorderColor ) : mProfile->mBorderColorNA;
   RectI renderRect( offset, getExtent() );

	if ( !highlight )
      renderRect.inset( 1, 1 );      

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();
	
   // Draw background transparency grid texture...
   if ( mGrid.isValid() )
      drawer->drawBitmapStretch( mGrid, renderRect );

   // Draw swatch color as fill...
   drawer->drawRectFill( renderRect, mSwatchColor );

   // Draw any borders...
   drawer->drawRect( renderRect, borderColor );
}

void GuiGradientSwatchCtrl::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
      return;

   if (mProfile->mCanKeyFocus)
      setFirstResponder();
	
	//capture current bounds and mouse down position
	mOrigBounds = getBounds();
	mMouseDownPosition = event.mousePoint;

   if(mUseMouseEvents)
      onMouseDown_callback();

   //lock the mouse
   mouseLock();
   mDepressed = true;

   // If we have a double click then execute the alt command.
   if ( event.mouseClickCount == 2 )
   {
      onDoubleClick_callback();

      execAltConsoleCallback();
   }

   setUpdate();
}

void GuiGradientSwatchCtrl::onMouseDragged(const GuiEvent &event)
{
	//gradientCtrl owns the y, x here however is regulated by the extent currently
	//dirty, please fix
	GuiGradientCtrl* parent = dynamic_cast<GuiGradientCtrl*>(getParent());
	if( !parent )
		return;
	
	//use bounds and delta to move the ctrl
	Point2I newPosition = mMouseDownPosition;
	Point2I deltaMousePosition = event.mousePoint - mMouseDownPosition;
	
	newPosition.x = mOrigBounds.point.x + deltaMousePosition.x;

	// default position but it needs to be standard; currently using this cops out a static y value
	newPosition.y = mOrigBounds.point.y; 

	if( newPosition.x + parent->mSwatchFactor >= parent->mBlendRangeBox.point.x &&
		newPosition.x + parent->mSwatchFactor <= parent->mBlendRangeBox.extent.x )
	{
		setPosition(newPosition);
		if( parent )
			parent->sortColorRange();
	}
}

void GuiGradientSwatchCtrl::onRightMouseDown(const GuiEvent &event)
{
	GuiGradientCtrl* parent = dynamic_cast<GuiGradientCtrl*>(getParent());
	if( parent )
		parent->removeColorRange( this );
}

//-----------------------------------------------------------------------------
// GuiGradientCtrl

static S32 QSORT_CALLBACK _numIncreasing( const void* a, const void* b )
{
	GuiGradientCtrl::ColorRange *crA = (GuiGradientCtrl::ColorRange *) (a);
   GuiGradientCtrl::ColorRange *crB = (GuiGradientCtrl::ColorRange *) (b);
   S32 posA = crA->swatch->getPosition().x;
   S32 posB = crB->swatch->getPosition().x;
	return ( (posA < posB) ? -1 : ((posA > posB) ? 1 : 0) );
}

ImplementEnumType( GuiGradientPickMode,
   "\n\n"
   "@ingroup GuiCore"
   "@internal")
   { GuiGradientCtrl::pHorizColorRange,	"HorizColor"},
   { GuiGradientCtrl::pHorizAlphaRange,	"HorizAlpha"},
EndImplementEnumType;

IMPLEMENT_CONOBJECT(GuiGradientCtrl);

ConsoleDocClass( GuiGradientCtrl,
   "@brief Visual representation of color box used with the GuiColorPickerCtrl\n\n"
   "Editor use only.\n\n"
   "@internal"
);


GuiGradientCtrl::GuiGradientCtrl()
{
   setExtent(140, 30);
   mDisplayMode = pHorizColorRange;
	mSaveDisplayMode = pHorizColorRange;
   mBaseColor = ColorF(1.,.0,1.);
   mPickColor = ColorF(.0,.0,.0);
   mMouseDown = mMouseOver = false;
   mActive = true;
   mPositionChanged = false;
   mActionOnMove = false;
	mShowReticle = true;
	colorWhiteBlend = ColorF(1.,1.,1.,.75);
	mSwatchFactor = 7;
}

//--------------------------------------------------------------------------
void GuiGradientCtrl::initPersistFields()
{
   addGroup("ColorPicker");
   addField("baseColor", TypeColorF, Offset(mBaseColor, GuiGradientCtrl));
   addField("pickColor", TypeColorF, Offset(mPickColor, GuiGradientCtrl));
   addField("displayMode", TYPEID< PickMode >(), Offset(mDisplayMode, GuiGradientCtrl) );
   addField("actionOnMove", TypeBool,Offset(mActionOnMove, GuiGradientCtrl));
	addField("showReticle", TypeBool, Offset(mShowReticle, GuiGradientCtrl));
	addField("swatchFactor", TypeS32, Offset(mSwatchFactor, GuiGradientCtrl));
   endGroup("ColorPicker");

   Parent::initPersistFields();
}

bool GuiGradientCtrl::onAdd()
{
	Parent::onAdd();

   RectI bounds = getBounds();
   S32 l = bounds.point.x + mSwatchFactor, r = bounds.point.x + bounds.extent.x - mSwatchFactor;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - mSwatchFactor;
	mBlendRangeBox = RectI( Point2I(l, t), Point2I(r, b) );
	
	setupDefaultRange();
	reInitSwatches( mDisplayMode );
	return true;
}

void GuiGradientCtrl::inspectPreApply()
{
	mSaveDisplayMode = mDisplayMode;
}

void GuiGradientCtrl::inspectPostApply()
{
   if((mSaveDisplayMode != mDisplayMode) )
      reInitSwatches( mDisplayMode );

   // Apply any transformations set in the editor
   Parent::inspectPostApply();
}

void GuiGradientCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   if (mStateBlock.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.setZReadWrite(false);
      desc.zWriteEnable = false;
      desc.setCullMode(GFXCullNone);
      mStateBlock = GFX->createStateBlock( desc );
   }

   RectI boundsRect(offset, getExtent()); 
   renderColorBox(boundsRect);

   if (mPositionChanged) 
   {
      mPositionChanged = false;

      // Now do onAction() if we are allowed
      if (mActionOnMove) 
			onAction();  
   }
   
   //render the children
   renderChildControls( offset, updateRect);
}

/// Function to invoke calls to draw the picker box and swatch controls
void GuiGradientCtrl::renderColorBox(RectI &bounds)
{   
   // Draw color box differently depending on mode
	if( mDisplayMode == pHorizColorRange )
	{
      drawBlendRangeBox( bounds, false, mColorRange);
	}
	else if( mDisplayMode == pHorizAlphaRange )
	{
      drawBlendRangeBox( bounds, false, mAlphaRange);
   }
}

/// Function to draw a set of boxes blending throughout an array of colors
void GuiGradientCtrl::drawBlendRangeBox(RectI &bounds, bool vertical, Vector<ColorRange> colorRange)
{
   GFX->setStateBlock(mStateBlock);

   // Create new global dimensions
   S32 l = bounds.point.x + mSwatchFactor, r = bounds.point.x + bounds.extent.x - mSwatchFactor;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - mSwatchFactor;

   // Draw border using new global dimensions
   if (mProfile->mBorder)
      GFX->getDrawUtil()->drawRect(RectI(Point2I(l, t), Point2I(r, b)), mProfile->mBorderColor);

   // Update local dimensions
   mBlendRangeBox.point = globalToLocalCoord(Point2I(l, t));
   mBlendRangeBox.extent = globalToLocalCoord(Point2I(r, b));

   if (colorRange.size() == 1) // Only one color to draw
   {
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color(colorRange.first().swatch->getColor());
      PrimBuild::vertex2i(l, t);
      PrimBuild::vertex2i(r, t);

      PrimBuild::color(colorRange.first().swatch->getColor());
      PrimBuild::vertex2i(l, b);
      PrimBuild::vertex2i(r, b);

      PrimBuild::end();
   }
   else
   {
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color(colorRange.first().swatch->getColor());
      PrimBuild::vertex2i(l, t);
      PrimBuild::vertex2i(l + colorRange.first().swatch->getPosition().x, t);

      PrimBuild::color(colorRange.first().swatch->getColor());
      PrimBuild::vertex2i(l, b);
      PrimBuild::vertex2i(l + colorRange.first().swatch->getPosition().x, b);

      PrimBuild::end();

      for (U16 i = 0; i < colorRange.size() - 1; i++)
      {
         PrimBuild::begin(GFXTriangleStrip, 4);
         if (!vertical)  // Horizontal (+x)
         {
            // First color
            PrimBuild::color(colorRange[i].swatch->getColor());
            PrimBuild::vertex2i(l + colorRange[i].swatch->getPosition().x, t);
            PrimBuild::color(colorRange[i + 1].swatch->getColor());
            PrimBuild::vertex2i(l + colorRange[i + 1].swatch->getPosition().x, t);

            // First color
            PrimBuild::color(colorRange[i].swatch->getColor());
            PrimBuild::vertex2i(l + colorRange[i].swatch->getPosition().x, b);
            PrimBuild::color(colorRange[i + 1].swatch->getColor());
            PrimBuild::vertex2i(l + colorRange[i + 1].swatch->getPosition().x, b);
         }
         PrimBuild::end();
      }

      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color(colorRange.last().swatch->getColor());
      PrimBuild::vertex2i(l + colorRange.last().swatch->getPosition().x, t);
      PrimBuild::vertex2i(r, t);

      PrimBuild::color(colorRange.last().swatch->getColor());
      PrimBuild::vertex2i(l + colorRange.last().swatch->getPosition().x, b);
      PrimBuild::vertex2i(r, b);

      PrimBuild::end();
   }
}

void GuiGradientCtrl::onMouseDown(const GuiEvent &event)
{
   if (!mActive)
      return;
   
   mouseLock(this);
   
   if (mProfile->mCanKeyFocus)
      setFirstResponder();
	
	if (mActive) 
      onAction();

	Point2I extent = getRoot()->getExtent();
   Point2I resolution = getRoot()->getExtent();
   GFXTexHandle bb( resolution.x, 
                    resolution.y, 
                    GFXFormatR8G8B8A8, &GFXDefaultRenderTargetProfile, avar("%s() - bb (line %d)", __FUNCTION__, __LINE__) );
   
   Point2I tmpPt( event.mousePoint.x, event.mousePoint.y );
   GFXTarget *targ = GFX->getActiveRenderTarget();
   targ->resolveTo( bb );
   GBitmap bmp( bb.getWidth(), bb.getHeight() );
   bb.copyToBmp( &bmp );
   ColorI tmp;
   bmp.getColor( event.mousePoint.x, event.mousePoint.y, tmp );
	
	addColorRange( globalToLocalCoord(event.mousePoint), ColorF(tmp) );
   
   mMouseDown = true;
}

void GuiGradientCtrl::onMouseUp(const GuiEvent &)
{
   //if we released the mouse within this control, perform the action
	if (mActive && mMouseDown ) 
      mMouseDown = false;
   
   mouseUnlock();
}

void GuiGradientCtrl::onMouseEnter(const GuiEvent &event)
{
   mMouseOver = true;
}

void GuiGradientCtrl::onMouseLeave(const GuiEvent &)
{
   // Reset state
   mMouseOver = false;
}

void GuiGradientCtrl::setupDefaultRange()
{
	S32 l = mBlendRangeBox.point.x - mSwatchFactor;
	S32 r = mBlendRangeBox.extent.x - mSwatchFactor;
	
	//setup alpha range (white/black only)
	ColorRange crW;
	crW.pos = l;
   crW.color = ColorI(255,255,255);
	crW.swatch = NULL;
	mAlphaRange.push_back( crW );

	ColorRange crB;
	crB.pos = r;
	crB.color = ColorI(0,0,0);
	crB.swatch = NULL;
	mAlphaRange.push_back( crB );

	//setup color range (only 1 color necessary)
	ColorRange crD;
   crD.pos = l;
   crD.color = ColorI(255,0,0);
	crD.swatch = NULL;
	mColorRange.push_back( crD );
}

void GuiGradientCtrl::reInitSwatches( GuiGradientCtrl::PickMode )
{
	//liable to crash in the guiEditor, needs fix
	for( S32 i = 0;i < mColorRange.size(); i++ ) 
	{
		if(mColorRange[i].swatch != NULL)
		{
			mColorRange[i].pos = mColorRange[i].swatch->getPosition().x;
			mColorRange[i].color = mColorRange[i].swatch->getColor();
			mColorRange[i].swatch->deleteObject();
			mColorRange[i].swatch = NULL;
		}
	}
	
	for( S32 i = 0;i < mAlphaRange.size(); i++ ) 
	{
		if(mAlphaRange[i].swatch != NULL)
		{
			mAlphaRange[i].pos = mAlphaRange[i].swatch->getPosition().x;
			mAlphaRange[i].color = mAlphaRange[i].swatch->getColor();
			mAlphaRange[i].swatch->deleteObject();
			mAlphaRange[i].swatch = NULL;
		}
	}
	
	S32 b = mBlendRangeBox.extent.y - mSwatchFactor;

   if( mDisplayMode == pHorizColorRange )
	{
		for( S32 i = 0;i < mColorRange.size(); i++ ) 
		{
			mColorRange[i].swatch = new GuiGradientSwatchCtrl();
			mColorRange[i].swatch->registerObject();
			addObject(mColorRange[i].swatch);
			mColorRange[i].swatch->setPosition( Point2I( mColorRange[i].pos, b ) );// needs to be adjusted
			mColorRange[i].swatch->setColor(ColorF(mColorRange[i].color));
		}
	}

   else if( mDisplayMode == pHorizAlphaRange )
	{
		for( S32 i = 0;i < mAlphaRange.size(); i++ ) 
		{
			mAlphaRange[i].swatch = new GuiGradientSwatchCtrl();
			mAlphaRange[i].swatch->registerObject();
			addObject(mAlphaRange[i].swatch);
			mAlphaRange[i].swatch->setPosition( Point2I( mAlphaRange[i].pos, b ) );// needs to be adjusted
			mAlphaRange[i].swatch->setColor(ColorF(mAlphaRange[i].color));
		}
	}
}

void GuiGradientCtrl::addColorRange(Point2I pos, const ColorF& color)
{
	if( pos.x + mSwatchFactor < mBlendRangeBox.point.x &&
		pos.x + mSwatchFactor > mBlendRangeBox.extent.x )
	{
		return;
	}

	ColorRange range;
   range.pos = pos.x - mSwatchFactor;
   range.color = color;
	
	S32 b = mBlendRangeBox.extent.y - mSwatchFactor;

	range.swatch = new GuiGradientSwatchCtrl();
	range.swatch->registerObject();
	addObject( range.swatch );
	range.swatch->setPosition( pos.x - mSwatchFactor, b );//swatch factor and default location is going to have to be placed
	range.swatch->setColor( color );
	
	if( mDisplayMode == pHorizColorRange )
	{
		mColorRange.push_back( range );
		S32 size = mColorRange.size();
		if( size > 0 )
			dQsort( mColorRange.address(), size, sizeof(ColorRange), _numIncreasing);
	}
	else if( mDisplayMode == pHorizAlphaRange )
	{
		mAlphaRange.push_back( range );
		S32 size = mAlphaRange.size();
		if( size > 0 )
			dQsort( mAlphaRange.address(), size, sizeof(ColorRange), _numIncreasing);
	}
}

void GuiGradientCtrl::removeColorRange( GuiGradientSwatchCtrl* swatch )
{
	if( mDisplayMode == pHorizColorRange )
	{
		if( mColorRange.size() <= 1 )
			return;

		for( S32 i = 0;i < mColorRange.size(); i++ ) 
		{
			if( mColorRange[i].swatch == swatch )
			{
				mColorRange.erase( U32(i) );
				swatch->safeDeleteObject();
				break;
			}
		}
	}
   else if( mDisplayMode == pHorizAlphaRange )
	{
		if( mAlphaRange.size() <= 1 )
			return;

		for( S32 i = 0;i < mAlphaRange.size(); i++ ) 
		{
			if( mAlphaRange[i].swatch == swatch )
			{
				mAlphaRange.erase( U32(i) );
				swatch->safeDeleteObject();
				break;	
			}
		}
	}
}

void GuiGradientCtrl::sortColorRange()
{
	if( mDisplayMode == pHorizColorRange )
		dQsort( mColorRange.address(), mColorRange.size(), sizeof(ColorRange), _numIncreasing);
	else if( mDisplayMode == pHorizAlphaRange )
		dQsort( mAlphaRange.address(), mAlphaRange.size(), sizeof(ColorRange), _numIncreasing);
}

DefineConsoleMethod(GuiGradientCtrl, getColorCount, S32, (), , "Get color count")
{
	if( object->getDisplayMode() == GuiGradientCtrl::pHorizColorRange )
		return object->mColorRange.size();
	else if( object->getDisplayMode() == GuiGradientCtrl::pHorizColorRange )
		return object->mColorRange.size();
	
	return 0;
}

DefineConsoleMethod(GuiGradientCtrl, getColor, ColorF, (S32 idx), , "Get color value")
{

	if( object->getDisplayMode() == GuiGradientCtrl::pHorizColorRange )
	{
		if ( idx >= 0 && idx < object->mColorRange.size() )
		{

			return object->mColorRange[idx].swatch->getColor();
		}
	}
	else if( object->getDisplayMode() == GuiGradientCtrl::pHorizColorRange )
	{
		if ( idx >= 0 && idx < object->mAlphaRange.size() )
		{

			return object->mAlphaRange[idx].swatch->getColor();
		}
	}

	return ColorF::ONE;
}