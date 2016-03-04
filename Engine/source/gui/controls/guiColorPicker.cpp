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
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/controls/guiColorPicker.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"

/// @name Common colors we use
/// @{
ColorF colorWhite(1.,1.,1.);
ColorF colorWhiteBlend(1.,1.,1.,.75);
ColorF colorBlack(.0,.0,.0);
ColorF colorAlpha(0.0f, 0.0f, 0.0f, 0.0f);
ColorF colorAlphaW(1.0f, 1.0f, 1.0f, 0.0f);

ColorI GuiColorPickerCtrl::mColorRange[7] = {
   ColorI(255,0,0),     // Red
   ColorI(255,0,255),   // Pink
   ColorI(0,0,255),     // Blue
   ColorI(0,255,255),   // Light blue
   ColorI(0,255,0),     // Green
   ColorI(255,255,0),   // Yellow
   ColorI(255,0,0),     // Red
};
/// @}

IMPLEMENT_CONOBJECT(GuiColorPickerCtrl);

ConsoleDocClass( GuiColorPickerCtrl,
   "@brief Editor GUI used for picking a ColorF from a palette.\n\n"
   "@note Editor use only.\n\n"
   "@internal"
);

GuiColorPickerCtrl::GuiColorPickerCtrl()
{
   setExtent(140, 30);
   mDisplayMode = pPallet;
   mBaseColor = ColorF(1.,.0,1.);
   mPickColor = ColorF(.0,.0,.0);
   mSelectorPos = Point2I(0,0);
   mMouseDown = mMouseOver = false;
   mActive = true;
   mPositionChanged = false;
   mSelectorGap = 1;
   mActionOnMove = false;
   mShowReticle = true;
   mSelectColor = false;
   mSetColor = mSetColor.BLACK;
   mBitmap = NULL;
}

GuiColorPickerCtrl::~GuiColorPickerCtrl()
{
   if (mBitmap)
   {
      delete mBitmap;
      mBitmap = NULL;
   }
}

ImplementEnumType( GuiColorPickMode,
   "\n\n"
   "@ingroup GuiUtil"
   "@internal" )
   { GuiColorPickerCtrl::pPallet, "Pallete" },
   { GuiColorPickerCtrl::pHorizColorRange, "HorizColor"},
   { GuiColorPickerCtrl::pVertColorRange, "VertColor" },
   { GuiColorPickerCtrl::pHorizColorBrightnessRange, "HorizBrightnessColor" },
   { GuiColorPickerCtrl::pVertColorBrightnessRange, "VertBrightnessColor" },
   { GuiColorPickerCtrl::pBlendColorRange, "BlendColor" },
   { GuiColorPickerCtrl::pHorizAlphaRange, "HorizAlpha" },
   { GuiColorPickerCtrl::pVertAlphaRange, "VertAlpha" },
   { GuiColorPickerCtrl::pDropperBackground, "Dropper" },
EndImplementEnumType;

void GuiColorPickerCtrl::initPersistFields()
{
   addGroup("ColorPicker");
      addField("baseColor", TypeColorF, Offset(mBaseColor, GuiColorPickerCtrl));
      addField("pickColor", TypeColorF, Offset(mPickColor, GuiColorPickerCtrl));
      addField("selectorGap", TypeS32,  Offset(mSelectorGap, GuiColorPickerCtrl)); 
      addField("displayMode", TYPEID< PickMode >(), Offset(mDisplayMode, GuiColorPickerCtrl) );
      addField("actionOnMove", TypeBool,Offset(mActionOnMove, GuiColorPickerCtrl));
      addField("showReticle", TypeBool, Offset(mShowReticle, GuiColorPickerCtrl));
   endGroup("ColorPicker");

   Parent::initPersistFields();
}

// Function to draw a box which can have 4 different colors in each corner blended together
void GuiColorPickerCtrl::drawBlendBox(RectI &bounds, ColorF &c1, ColorF &c2, ColorF &c3, ColorF &c4)
{
   GFX->setStateBlock(mStateBlock);

   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y;
   
   //A couple of checks to determine if color blend
   //A couple of checks to determine if color blend
   if(c1 == colorWhite && c3 == colorAlpha && c4 == colorBlack)
   {
      //Color
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color( c2 );
      PrimBuild::vertex2i(l, t);

      PrimBuild::color( c2 );
      PrimBuild::vertex2i(r, t);

      PrimBuild::color( c2 );
      PrimBuild::vertex2i( l, b );

      PrimBuild::color( c2 );
      PrimBuild::vertex2i(r, b);

      PrimBuild::end();

      //White
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color(c1);
      PrimBuild::vertex2i(l, t);

      PrimBuild::color( colorAlphaW );
      PrimBuild::vertex2i(r, t);

      PrimBuild::color( c1 );
      PrimBuild::vertex2i( l, b );

      PrimBuild::color(colorAlphaW);
      PrimBuild::vertex2i(r, b);

      PrimBuild::end();

      //Black 
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color(c3);
      PrimBuild::vertex2i(l, t);
      PrimBuild::color( c3 );
      PrimBuild::vertex2i( r, t );

      PrimBuild::color( c4 );
      PrimBuild::vertex2i(l, b);

      PrimBuild::color( c4 );
      PrimBuild::vertex2i(r, b);

      PrimBuild::end();
   }
   else
   {
      PrimBuild::begin(GFXTriangleStrip, 4);

      PrimBuild::color( c1 );
      PrimBuild::vertex2i( l, t );

      PrimBuild::color( c2 );
      PrimBuild::vertex2i( r, t );

      PrimBuild::color(c4);
      PrimBuild::vertex2i(l, b);
	  
      PrimBuild::color( c3 );
      PrimBuild::vertex2i( r, b );

      PrimBuild::end();
   }
}

//--------------------------------------------------------------------------
/// Function to draw a set of boxes blending throughout an array of colors
void GuiColorPickerCtrl::drawBlendRangeBox(RectI &bounds, bool vertical, U8 numColors, ColorI *colors)
{
   GFX->setStateBlock(mStateBlock);

   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x + 4;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y + 4;

   // Calculate increment value
   S32 x_inc = int(mFloor((r - l) / F32(numColors - 1)));
   S32 y_inc = int(mFloor((b - t) / F32(numColors - 1)));

   for (U16 i = 0; i < numColors - 1; i++)
   {
      // This is not efficent, but then again it doesn't really need to be. -pw
      PrimBuild::begin(GFXTriangleStrip, 4);

      if (!vertical)  // Horizontal (+x)
      {
         // First color
         PrimBuild::color(colors[i]);
         PrimBuild::vertex2i(l, t);
         PrimBuild::color(colors[i + 1]);
         PrimBuild::vertex2i(l + x_inc, t);

         // Second color
         PrimBuild::color(colors[i]);
         PrimBuild::vertex2i(l, b);
         PrimBuild::color(colors[i + 1]);
         PrimBuild::vertex2i(l + x_inc, b);
         l += x_inc;
      }
      else  // Vertical (+y)
      {
         // First color
         PrimBuild::color(colors[i]);
         PrimBuild::vertex2i(l, t);
         PrimBuild::color(colors[i + 1]);
         PrimBuild::vertex2i(l, t + y_inc);

         // Second color
         PrimBuild::color(colors[i]);
         PrimBuild::vertex2i(r, t);
         PrimBuild::color(colors[i + 1]);
         PrimBuild::vertex2i(r, t + y_inc);
         t += y_inc;
      }
      PrimBuild::end();
   }
}

void GuiColorPickerCtrl::drawSelector(RectI &bounds, Point2I &selectorPos, SelectorMode mode)
{
   if( !mShowReticle )
      return; 

   U16 sMax = mSelectorGap*2;
   switch (mode)
   {
      case sVertical:
         // Now draw the vertical selector Up -> Pos
         if (selectorPos.y != bounds.point.y+1)
            GFX->getDrawUtil()->drawLine(selectorPos.x, bounds.point.y, selectorPos.x, selectorPos.y-sMax-1, colorWhiteBlend);
         // Down -> Pos
         if (selectorPos.y != bounds.point.y+bounds.extent.y) 
            GFX->getDrawUtil()->drawLine(selectorPos.x,	selectorPos.y + sMax, selectorPos.x, bounds.point.y + bounds.extent.y, colorWhiteBlend);
      break;
      case sHorizontal:
         // Now draw the horizontal selector Left -> Pos
         if (selectorPos.x != bounds.point.x) 
            GFX->getDrawUtil()->drawLine(bounds.point.x, selectorPos.y-1, selectorPos.x-sMax, selectorPos.y-1, colorWhiteBlend);
         // Right -> Pos
         if (selectorPos.x != bounds.point.x) 
            GFX->getDrawUtil()->drawLine(bounds.point.x+mSelectorPos.x+sMax, selectorPos.y-1, bounds.point.x + bounds.extent.x, selectorPos.y-1, colorWhiteBlend);
      break;
   }
}

//--------------------------------------------------------------------------
/// Function to invoke calls to draw the picker box and selector
void GuiColorPickerCtrl::renderColorBox(RectI &bounds)
{
   RectI pickerBounds;
   pickerBounds.point.x = bounds.point.x+1;
   pickerBounds.point.y = bounds.point.y+1;
   pickerBounds.extent.x = bounds.extent.x-1;
   pickerBounds.extent.y = bounds.extent.y-1;

   if (mProfile->mBorder)
      GFX->getDrawUtil()->drawRect(bounds, mProfile->mBorderColor);

   Point2I selectorPos = Point2I(bounds.point.x+mSelectorPos.x+1, bounds.point.y+mSelectorPos.y+1);

   // Draw color box differently depending on mode
   RectI blendRect;
   switch (mDisplayMode)
   {
   case pHorizColorRange:
      drawBlendRangeBox( pickerBounds, false, 7, mColorRange);
      drawSelector( pickerBounds, selectorPos, sVertical );
   break;
   case pVertColorRange:
      drawBlendRangeBox( pickerBounds, true, 7, mColorRange);
      drawSelector( pickerBounds, selectorPos, sHorizontal );
   break;
   case pHorizColorBrightnessRange:
      blendRect = pickerBounds;
      blendRect.point.y++;
      blendRect.extent.y -= 2;
      drawBlendRangeBox( pickerBounds, false, 7, mColorRange);
      // This is being drawn slightly offset from the larger rect so as to insure 255 and 0
      // can both be selected for every color.
      drawBlendBox( blendRect, colorAlpha, colorAlpha, colorBlack, colorBlack );
      blendRect.point.y += blendRect.extent.y - 1;
      blendRect.extent.y = 2;
      GFX->getDrawUtil()->drawRect( blendRect, colorBlack);
      drawSelector( pickerBounds, selectorPos, sHorizontal );
      drawSelector( pickerBounds, selectorPos, sVertical );
   break;
   case pVertColorBrightnessRange:
      drawBlendRangeBox( pickerBounds, true, 7, mColorRange);
      drawBlendBox( pickerBounds, colorAlpha, colorBlack, colorBlack, colorAlpha );
      drawSelector( pickerBounds, selectorPos, sHorizontal );
      drawSelector( pickerBounds, selectorPos, sVertical );
   break;
   case pHorizAlphaRange:
      drawBlendBox( pickerBounds, colorBlack, colorWhite, colorWhite, colorBlack );
      drawSelector( pickerBounds, selectorPos, sVertical );
   break;
   case pVertAlphaRange:
      drawBlendBox( pickerBounds, colorBlack, colorBlack, colorWhite, colorWhite );
      drawSelector( pickerBounds, selectorPos, sHorizontal ); 
   break;
   case pBlendColorRange:
      drawBlendBox( pickerBounds, colorWhite, mBaseColor, colorAlpha, colorBlack );
      drawSelector( pickerBounds, selectorPos, sHorizontal );      
      drawSelector( pickerBounds, selectorPos, sVertical );
   break;
   case pDropperBackground:
   break;
   case pPallet:
   default:
      GFX->getDrawUtil()->drawRectFill( pickerBounds, mBaseColor );
   break;
   }
}

void GuiColorPickerCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   if (mStateBlock.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.setZReadWrite(false);
      desc.zWriteEnable = false;
      desc.setCullMode(GFXCullNone);
      mStateBlock = GFX->createStateBlock(desc);
   }

   RectI boundsRect(offset, getExtent());
   renderColorBox(boundsRect);

   if (mPositionChanged || mBitmap == NULL)
   {
      bool nullBitmap = false;

      if (mPositionChanged == false && mBitmap == NULL)
         nullBitmap = true;

      mPositionChanged = false;
      Point2I extent = getRoot()->getExtent();

      // If we are anything but a pallete, change the pick color
      if (mDisplayMode != pPallet)
      {
         Point2I resolution = getRoot()->getExtent();

         U32 buf_x = offset.x + mSelectorPos.x + 1;
         U32 buf_y = resolution.y - (extent.y - (offset.y + mSelectorPos.y + 1));

         GFXTexHandle bb( resolution.x, resolution.y, GFXFormatR8G8B8A8, &GFXDefaultRenderTargetProfile, avar("%s() - bb (line %d)", __FUNCTION__, __LINE__) );

         Point2I tmpPt(buf_x, buf_y);

         GFXTarget *targ = GFX->getActiveRenderTarget();
         targ->resolveTo(bb);

         if (mBitmap)
         {
            delete mBitmap;
            mBitmap = NULL;
         }

         mBitmap = new GBitmap(bb.getWidth(), bb.getHeight());

         bb.copyToBmp(mBitmap);

         if (!nullBitmap)
         {
            if (mSelectColor)
            {
               Point2I pos = findColor(mSetColor, offset, resolution, *mBitmap);
               mSetColor = mSetColor.BLACK;
               mSelectColor = false;
               setSelectorPos(pos);
            }
            else
            {
               ColorI tmp;
               mBitmap->getColor(buf_x, buf_y, tmp);

               mPickColor = (ColorF)tmp;

               // Now do onAction() if we are allowed
               if (mActionOnMove)
                  onAction();
            }
         }
      }
   }

   //render the children
   renderChildControls(offset, updateRect);
}

void GuiColorPickerCtrl::setSelectorPos(const ColorF & color)
{
   if (mBitmap && !mPositionChanged)
   {
      Point2I resolution = getRoot() ? getRoot()->getExtent() : Point2I(1024, 768);
      RectI rect(getGlobalBounds());
      Point2I pos = findColor(color, rect.point, resolution, *mBitmap);
      mSetColor = mSetColor.BLACK;
      mSelectColor = false;

      setSelectorPos(pos);
   }
   else
   {
      mSetColor = color;
      mSelectColor = true;
      mPositionChanged = true;
   }
}

Point2I GuiColorPickerCtrl::findColor(const ColorF & color, const Point2I& offset, const Point2I& resolution, GBitmap& bmp)
{
   RectI rect;
   Point2I ext = getExtent();
   if (mDisplayMode != pDropperBackground)
   {
      ext.x -= 3;
      ext.y -= 2;
      rect = RectI(Point2I(1, 1), ext);
   }
   else
   {
      rect = RectI(Point2I(0, 0), ext);
   }

   Point2I closestPos(-1, -1);

   /* Debugging
   char filename[256];
   dSprintf( filename, 256, "%s.%s", "colorPickerTest", "png" );

   // Open up the file on disk.
   FileStream fs;
   if ( !fs.open( filename, Torque::FS::File::Write ) )
   Con::errorf( "GuiObjectView::saveAsImage() - Failed to open output file '%s'!", filename );
   else
   {
   // Write it and close.
   bmp.writeBitmap( "png", fs );

   fs.close();
   }
   */

   ColorI tmp;
   U32 buf_x;
   U32 buf_y;
   ColorF curColor;
   F32 val(10000.0f);
   F32 closestVal(10000.0f);
   bool closestSet = false;

   for (S32 x = rect.point.x; x <= rect.extent.x; x++)
   {
      for (S32 y = rect.point.y; y <= rect.extent.y; y++)
      {
         buf_x = offset.x + x + 1;
         buf_y = (resolution.y - (offset.y + y + 1));
         buf_y = resolution.y - buf_y;

         //Get the color at that position
         bmp.getColor(buf_x, buf_y, tmp);
         curColor = (ColorF)tmp;

         //Evaluate how close the color is to our desired color
         val = mFabs(color.red - curColor.red) + mFabs(color.green - curColor.green) + mFabs(color.blue - curColor.blue);

         if (!closestSet)
         {
            closestVal = val;
            closestPos.set(x, y);
            closestSet = true;
         }
         else if (val < closestVal)
         {
            closestVal = val;
            closestPos.set(x, y);
         }
      }
   }

   return closestPos;
}

void GuiColorPickerCtrl::setSelectorPos(const Point2I &pos)
{
   Point2I extent = getExtent();
   RectI rect;
   if (mDisplayMode != pDropperBackground) 
   {
      extent.x -= 3;
      extent.y -= 2;
      rect = RectI(Point2I(1,1), extent);
   }
   else
   {
      rect = RectI(Point2I(0,0), extent);
   }
   
   if (rect.pointInRect(pos)) 
   {
      mSelectorPos = pos;
      mPositionChanged = true;
      // We now need to update
      setUpdate();
   }

   else
   {
      if ((pos.x > rect.point.x) && (pos.x < (rect.point.x + rect.extent.x)))
         mSelectorPos.x = pos.x;
      else if (pos.x <= rect.point.x)
         mSelectorPos.x = rect.point.x;
      else if (pos.x >= (rect.point.x + rect.extent.x))
         mSelectorPos.x = rect.point.x + rect.extent.x - 1;

      if ((pos.y > rect.point.y) && (pos.y < (rect.point.y + rect.extent.y)))
         mSelectorPos.y = pos.y;
      else if (pos.y <= rect.point.y)
         mSelectorPos.y = rect.point.y;
      else if (pos.y >= (rect.point.y + rect.extent.y))
         mSelectorPos.y = rect.point.y + rect.extent.y - 1;

      mPositionChanged = true;
      setUpdate();
   }
}

void GuiColorPickerCtrl::onMouseDown(const GuiEvent &event)
{
   if (!mActive)
      return;
   
   if (mDisplayMode == pDropperBackground)
      return;

   mouseLock(this);
   
   if (mProfile->mCanKeyFocus)
      setFirstResponder();

   if (mActive && (mDisplayMode != pDropperBackground))
      onAction();

   // Update the picker cross position
   if (mDisplayMode != pPallet)
      setSelectorPos(globalToLocalCoord(event.mousePoint));

   mMouseDown = true;
}

//--------------------------------------------------------------------------
void GuiColorPickerCtrl::onMouseDragged(const GuiEvent &event)
{
   if ((mActive && mMouseDown) || (mActive && (mDisplayMode == pDropperBackground)))
   {
      // Update the picker cross position
      if (mDisplayMode != pPallet)
         setSelectorPos(globalToLocalCoord(event.mousePoint));
   }

   if( !mActionOnMove )
      execAltConsoleCallback();
}

void GuiColorPickerCtrl::onMouseMove(const GuiEvent &event)
{
   // Only for dropper mode
   if (mActive && (mDisplayMode == pDropperBackground))
      setSelectorPos(globalToLocalCoord(event.mousePoint));
}

void GuiColorPickerCtrl::onMouseEnter(const GuiEvent &event)
{
   mMouseOver = true;
}

void GuiColorPickerCtrl::onMouseLeave(const GuiEvent &)
{
   // Reset state
   mMouseOver = false;
}

void GuiColorPickerCtrl::onMouseUp(const GuiEvent &)
{
   //if we released the mouse within this control, perform the action
   if (mActive && mMouseDown && (mDisplayMode != pDropperBackground))
      mMouseDown = false;

   if (mActive && (mDisplayMode == pDropperBackground))
   {
      // In a dropper, the alt command executes the mouse up action (to signal stopping)
      execAltConsoleCallback();
   }

   mouseUnlock();
}

const char *GuiColorPickerCtrl::getScriptValue()
{
   static char temp[256];
   ColorF color = getValue();
   dSprintf( temp, 256, "%f %f %f %f", color.red, color.green, color.blue, color.alpha );
   return temp;
}

void GuiColorPickerCtrl::setScriptValue(const char *value)
{
   ColorF newValue;
   dSscanf(value, "%f %f %f %f", &newValue.red, &newValue.green, &newValue.blue, &newValue.alpha);
   setValue(newValue);
}

DefineConsoleMethod(GuiColorPickerCtrl, getSelectorPos, Point2I, (), , "Gets the current position of the selector")
{
   return object->getSelectorPos();
}

DefineConsoleMethod(GuiColorPickerCtrl, setSelectorPos, void, (Point2I newPos), , "Sets the current position of the selector")
{
   object->setSelectorPos(newPos);
}

DefineConsoleMethod(GuiColorPickerCtrl, updateColor, void, (), , "Forces update of pick color")
{
   object->updateColor();
}

DefineEngineMethod(GuiColorPickerCtrl, setSelectorColor, void, (ColorF color), ,
   "Sets the current position of the selector based on a color.n"
   "@param color Color to look for.n")
{
   object->setSelectorPos(color);
}
