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


//-------------------------------------
//
// Icon Button Control
// Draws the bitmap within a special button control.  Only a single bitmap is used and the
// button will be drawn in a highlighted mode when the mouse hovers over it or when it
// has been clicked.
//
// Use mTextLocation to choose where within the button the text will be drawn, if at all.
// Use mTextMargin to set the text away from the button sides or from the bitmap.
// Use mButtonMargin to set everything away from the button sides.
// Use mErrorBitmapName to set the name of a bitmap to draw if the main bitmap cannot be found.
// Use mFitBitmapToButton to force the bitmap to fill the entire button extent.  Usually used
// with no button text defined.
//
//

#include "platform/platform.h"
#include "gui/buttons/guiIconButtonCtrl.h"

#include "console/console.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "console/engineAPI.h"

static const ColorI colorWhite(255,255,255);
static const ColorI colorBlack(0,0,0);

IMPLEMENT_CONOBJECT(GuiIconButtonCtrl);

ConsoleDocClass( GuiIconButtonCtrl,
   "@brief Draws the bitmap within a special button control.  Only a single bitmap is used and the\n"
   "button will be drawn in a highlighted mode when the mouse hovers over it or when it\n"
   "has been clicked.\n\n"

   "@tsexample\n"
	"new GuiIconButtonCtrl(TestIconButton)\n"
	"{\n"
    "	buttonMargin = \"4 4\";\n"
    "	iconBitmap = \"art/gui/lagIcon.png\";\n"
    "	iconLocation = \"Center\";\n"
    "	sizeIconToButton = \"0\";\n"
    "	makeIconSquare = \"1\";\n"
    "	textLocation = \"Bottom\";\n"
    "	textMargin = \"-2\";\n"
	"	autoSize = \"0\";\n"
	"	text = \"Lag Icon\";\n"
	"	textID = \"\"STR_LAG\"\";\n"
	"	buttonType = \"PushButton\";\n"
	"	profile = \"GuiIconButtonProfile\";\n"
	"};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n"
   "@see GuiButtonCtrl\n\n"

   "@ingroup GuiCore\n"
);


GuiIconButtonCtrl::GuiIconButtonCtrl()
{
   mBitmapName = StringTable->insert("");
   mTextLocation = TextLocLeft;
   mIconLocation = IconLocLeft;
   mTextMargin = 4;
   mButtonMargin.set(4,4);

   mFitBitmapToButton = false;
   mMakeIconSquare = false;

   mErrorBitmapName = StringTable->insert("");
   mErrorTextureHandle = NULL;

   mAutoSize = false;

   setExtent(140, 30);
}

ImplementEnumType( GuiIconButtonTextLocation,
   "\n\n"
   "@ingroup GuiImages" )
   { GuiIconButtonCtrl::TextLocNone, "None" },
   { GuiIconButtonCtrl::TextLocBottom, "Bottom" },
   { GuiIconButtonCtrl::TextLocRight, "Right" },
   { GuiIconButtonCtrl::TextLocTop, "Top" },
   { GuiIconButtonCtrl::TextLocLeft, "Left" },
   { GuiIconButtonCtrl::TextLocCenter, "Center" },
EndImplementEnumType;

ImplementEnumType( GuiIconButtonIconLocation,
   "\n\n"
   "@ingroup GuiImages" )
   { GuiIconButtonCtrl::IconLocNone, "None" },
   { GuiIconButtonCtrl::IconLocLeft, "Left" },
   { GuiIconButtonCtrl::IconLocRight, "Right" },
   { GuiIconButtonCtrl::IconLocCenter, "Center" }
EndImplementEnumType;

void GuiIconButtonCtrl::initPersistFields()
{
   addField( "buttonMargin",     TypePoint2I,   Offset( mButtonMargin, GuiIconButtonCtrl ),"Margin area around the button.\n");
   addField( "iconBitmap",       TypeFilename,  Offset( mBitmapName, GuiIconButtonCtrl ),"Bitmap file for the icon to display on the button.\n");
   addField( "iconLocation",     TYPEID< IconLocation >(), Offset( mIconLocation, GuiIconButtonCtrl ),"Where to place the icon on the control. Options are 0 (None), 1 (Left), 2 (Right), 3 (Center).\n");
   addField( "sizeIconToButton", TypeBool,      Offset( mFitBitmapToButton, GuiIconButtonCtrl ),"If true, the icon will be scaled to be the same size as the button.\n");
   addField( "makeIconSquare",   TypeBool,      Offset( mMakeIconSquare, GuiIconButtonCtrl ),"If true, will make sure the icon is square.\n");
   addField( "textLocation",     TYPEID< TextLocation >(),      Offset( mTextLocation, GuiIconButtonCtrl ),"Where to place the text on the control.\n"
																										   "Options are 0 (None), 1 (Bottom), 2 (Right), 3 (Top), 4 (Left), 5 (Center).\n");
   addField( "textMargin",       TypeS32,       Offset( mTextMargin, GuiIconButtonCtrl ),"Margin between the icon and the text.\n");
   addField( "autoSize",         TypeBool,      Offset( mAutoSize, GuiIconButtonCtrl ),"If true, the text and icon will be automatically sized to the size of the control.\n");
   Parent::initPersistFields();
}

bool GuiIconButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);

   setBitmap(mBitmapName);

   if( mProfile )
      mProfile->constructBitmapArray();

   return true;
}

void GuiIconButtonCtrl::onSleep()
{
   mTextureNormal = NULL;
   Parent::onSleep();
}

void GuiIconButtonCtrl::inspectPostApply()
{
   Parent::inspectPostApply();
}

void GuiIconButtonCtrl::onStaticModified(const char* slotName, const char* newValue)
{
   if ( isProperlyAdded() && !dStricmp(slotName, "autoSize") )
      resize( getPosition(), getExtent() );
}

bool GuiIconButtonCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{   
   if ( !mAutoSize || !mProfile->mFont )   
      return Parent::resize( newPosition, newExtent );
   
   Point2I autoExtent( mMinExtent );

   if ( mIconLocation != IconLocNone )
   {      
      autoExtent.y = mTextureNormal.getHeight() + mButtonMargin.y * 2;
      autoExtent.x = mTextureNormal.getWidth() + mButtonMargin.x * 2;
   }

   if ( mTextLocation != TextLocNone && mButtonText && mButtonText[0] )
   {
      U32 strWidth = mProfile->mFont->getStrWidthPrecise( mButtonText );
      
      if ( mTextLocation == TextLocLeft || mTextLocation == TextLocRight )
      {
         autoExtent.x += strWidth + mTextMargin * 2;
      }
      else // Top, Bottom, Center
      {
         strWidth += mTextMargin * 2;
         if ( strWidth > autoExtent.x )
            autoExtent.x = strWidth;
      }
   }

   return Parent::resize( newPosition, autoExtent );
}

void GuiIconButtonCtrl::setBitmap(const char *name)
{
   mBitmapName = StringTable->insert(name);
   if(!isAwake())
      return;

   if (*mBitmapName)
   {
      mTextureNormal = GFXTexHandle( name, &GFXDefaultPersistentProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__) );
   }
   else
   {
      mTextureNormal = NULL;
   }

   // So that extent is recalculated if autoSize is set.
   resize( getPosition(), getExtent() );

   setUpdate();
}   

void GuiIconButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   renderButton( offset, updateRect);
}

void GuiIconButtonCtrl::renderButton( Point2I &offset, const RectI& updateRect )
{
   bool highlight = mMouseOver;
   bool depressed = mDepressed;
   
   ColorI fontColor   = mActive ? (highlight ? mProfile->mFontColor : mProfile->mFontColor) : mProfile->mFontColorNA;
   
   RectI boundsRect(offset, getExtent());

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   if (mDepressed || mStateOn)
   {
      // If there is a bitmap array then render using it.  
      // Otherwise use a standard fill.
      if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
         renderBitmapArray(boundsRect, statePressed);
      else
         renderSlightlyLoweredBox(boundsRect, mProfile);
   }
   else if(mMouseOver && mActive)
   {
      // If there is a bitmap array then render using it.  
      // Otherwise use a standard fill.
      if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
         renderBitmapArray(boundsRect, stateMouseOver);
      else
         renderSlightlyRaisedBox(boundsRect, mProfile);
   }
   else
   {
      // If there is a bitmap array then render using it.  
      // Otherwise use a standard fill.
      if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
      {
         if(mActive)
            renderBitmapArray(boundsRect, stateNormal);
         else
            renderBitmapArray(boundsRect, stateDisabled);
      }
      else
      {
         drawer->drawRectFill(boundsRect, mProfile->mFillColorNA);
         drawer->drawRect(boundsRect, mProfile->mBorderColorNA);
      }
   }

   Point2I textPos = offset;
   if(depressed)
      textPos += Point2I(1,1);

   RectI iconRect( 0, 0, 0, 0 );

   // Render the icon
   if ( mTextureNormal && mIconLocation != GuiIconButtonCtrl::IconLocNone )
   {
      // Render the normal bitmap
      drawer->clearBitmapModulation();

      // Maintain the bitmap size or fill the button?
      if ( !mFitBitmapToButton )
      {
         Point2I textureSize( mTextureNormal->getWidth(), mTextureNormal->getHeight() );
         iconRect.set( offset + mButtonMargin, textureSize );

         if ( mIconLocation == IconLocRight )    
         {
            iconRect.point.x = ( offset.x + getWidth() ) - ( mButtonMargin.x + textureSize.x );
            iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;
         }
         else if ( mIconLocation == IconLocLeft )
         {            
            iconRect.point.x = offset.x + mButtonMargin.x;
            iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;
         }
         else if ( mIconLocation == IconLocCenter )
         {
            iconRect.point.x = offset.x + ( getWidth() - textureSize.x ) / 2;
            iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;
         }

         drawer->drawBitmapStretch( mTextureNormal, iconRect );

      } 
      else
      {
         iconRect.set( offset + mButtonMargin, getExtent() - (mButtonMargin * 2) );
         
         if ( mMakeIconSquare )
         {
            // Square the icon to the smaller axis extent.
            if ( iconRect.extent.x < iconRect.extent.y )
               iconRect.extent.y = iconRect.extent.x;
            else
               iconRect.extent.x = iconRect.extent.y;            
         }

         drawer->drawBitmapStretch( mTextureNormal, iconRect );
      }
   }

   // Render text
   if ( mTextLocation != TextLocNone )
   {
      // Clip text to fit (appends ...),
      // pad some space to keep it off our border
      String text( mButtonText );      
      S32 textWidth = clipText( text, getWidth() - 4 - mTextMargin );

      drawer->setBitmapModulation( fontColor );      

      if ( mTextLocation == TextLocRight )
      {
         Point2I start( mTextMargin, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         if ( mTextureNormal && mIconLocation != IconLocNone )
         {
            start.x = iconRect.extent.x + mButtonMargin.x + mTextMargin;
         }

         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == TextLocLeft )
      {
         Point2I start( mTextMargin, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );

         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == TextLocCenter )
      {
         Point2I start;
         if ( mTextureNormal && mIconLocation == IconLocLeft )
         {
            start.set( ( getWidth() - textWidth - iconRect.extent.x ) / 2 + iconRect.extent.x, 
                       ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         }
         else
            start.set( ( getWidth() - textWidth ) / 2, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         drawer->setBitmapModulation( fontColor );
         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == TextLocBottom )
      {
         Point2I start;
         start.set( ( getWidth() - textWidth ) / 2, getHeight() - mProfile->mFont->getHeight() - mTextMargin );

         // If the text is longer then the box size
         // it will get clipped, force Left Justify
         if( textWidth > getWidth() )
            start.x = 0;

         drawer->setBitmapModulation( fontColor );
         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }
   }

   renderChildControls( offset, updateRect);
}

// Draw the bitmap array's borders according to the button's state.
void GuiIconButtonCtrl::renderBitmapArray(RectI &bounds, S32 state)
{
   switch(state)
   {
   case stateNormal:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 1, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 1, mProfile);
      break;

   case stateMouseOver:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 2, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 2, mProfile);
      break;

   case statePressed:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 3, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 3, mProfile);
      break;

   case stateDisabled:
      if(mProfile->mBorder == -2)
         renderSizableBitmapBordersFilled(bounds, 4, mProfile);
      else
         renderFixedBitmapBordersFilled(bounds, 4, mProfile);
      break;
   }
}

DefineEngineMethod( GuiIconButtonCtrl, setBitmap, void, (const char* buttonFilename),,
   "@brief Set the bitmap to use for the button portion of this control.\n\n"
   "@param buttonFilename Filename for the image\n"
   "@tsexample\n"
   "// Define the button filename\n"
   "%buttonFilename = \"pearlButton\";\n\n"
   "// Inform the GuiIconButtonCtrl control to update its main button graphic to the defined bitmap\n"
   "%thisGuiIconButtonCtrl.setBitmap(%buttonFilename);\n"
   "@endtsexample\n\n"
   "@see GuiControl\n"
   "@see GuiButtonCtrl\n\n")
{
   char* argBuffer = Con::getArgBuffer( 512 );
   Platform::makeFullPathName( buttonFilename, argBuffer, 512 );
   object->setBitmap( argBuffer );
}