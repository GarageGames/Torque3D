//-------------------------------------
//
// Icon Control
// Draws a movable icon with optional text field
//
// Use mTextLocation to choose where within the icon the text will be drawn, if at all.
// Use mTextMargin to set the text away from the icon sides or from the bitmap.
// Use mButtonMargin to set everything away from the button sides.
// Use mErrorBitmapName to set the name of a bitmap to draw if the main bitmap cannot be found.
// Use mFitBitmapToButton to force the bitmap to fill the entire button extent.  Usually used
// with no button text defined.
//

#include "platform/platform.h"
#include "gui/controls/guiIconControl.h"
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

IMPLEMENT_CONOBJECT(GuiIconControl);

ConsoleDocClass( GuiIconControl,
   "@brief Draws a moveable icon within a special button control.  Only a single bitmap is used and the\n"
   "icon will be drawn in a highlighted mode when the mouse hovers over it or when it\n"
   "has been clicked.\n\n"

   "@tsexample\n"
	"new GuiIconControl(TestIconButton)\n"
	"{\n"
    "	buttonMargin = \"4 4\";\n"
    "	iconBitmap = \"art/gui/lagIcon.png\";\n"
    "	iconLocation = \"Center\";\n"
    "	sizeIconToButton = \"0\";\n"
    "	makeIconSquare = \"1\";\n"
    "	textLocation = \"Bottom\";\n"
    "	textMargin = \"-2\";\n"
	"	autoSize = \"1\";\n"
	"	text = \"Lag Icon\";\n"
	"	textID = \"\"STR_LAG\"\";\n"
	"	buttonType = \"PushButton\";\n"
	"	profile = \"GuiIconButtonProfile\";\n"
	"};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n"
   "@see GuiButtonCtrl\n\n"
   "@see GuiButtonIconCtrl\n\n"

   "@ingroup GuiControls"
);

GuiIconControl::GuiIconControl()
{
   mTextLocation = GuiIconButtonCtrl::TextLocCenter;
   mIconLocation = GuiIconButtonCtrl::IconLocCenter;
   mCanDrag = true;
   mFitBitmapToButton = true;

   setExtent(32, 32);
}

void GuiIconControl::initPersistFields()
{
   Parent::initPersistFields();
}

void GuiIconControl::renderButton( Point2I &offset, const RectI& updateRect )
{
   ColorI fontColor = mProfile->mFontColor;
   RectI boundsRect(offset, getExtent());
   GFXDrawUtil *drawer = GFX->getDrawUtil();

   Point2I textPos = offset;
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

         if ( mIconLocation == GuiIconButtonCtrl::IconLocRight )    
         {
            iconRect.point.x = ( offset.x + getWidth() ) - ( mButtonMargin.x + textureSize.x );
            iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;
         }
         else if ( mIconLocation == GuiIconButtonCtrl::IconLocLeft )
         {            
            iconRect.point.x = offset.x + mButtonMargin.x;
            iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;
         }
         else if ( mIconLocation == GuiIconButtonCtrl::IconLocCenter )
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
   if ( mTextLocation != GuiIconButtonCtrl::TextLocNone )
   {
      // Clip text to fit (appends ...),
      // pad some space to keep it off our border
      String text( mButtonText );      
      S32 textWidth = clipText( text, getWidth() - 4 - mTextMargin );

      drawer->setBitmapModulation( fontColor );      

      if ( mTextLocation == GuiIconButtonCtrl::TextLocRight )
      {
         Point2I start( mTextMargin, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         if ( mTextureNormal && mIconLocation != GuiIconButtonCtrl::IconLocNone )
         {
            start.x = iconRect.extent.x + mButtonMargin.x + mTextMargin;
         }

         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == GuiIconButtonCtrl::TextLocLeft )
      {
         Point2I start( mTextMargin, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );

         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == GuiIconButtonCtrl::TextLocCenter )
      {
         Point2I start;
         if ( mTextureNormal && mIconLocation == GuiIconButtonCtrl::IconLocLeft )
         {
            start.set( ( getWidth() - textWidth - iconRect.extent.x ) / 2 + iconRect.extent.x, 
                       ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         }
         else
            start.set( ( getWidth() - textWidth ) / 2, ( getHeight() - mProfile->mFont->getHeight() ) / 2 );
         drawer->setBitmapModulation( fontColor );
         drawer->drawText( mProfile->mFont, start + offset, text, mProfile->mFontColors );
      }

      if ( mTextLocation == GuiIconButtonCtrl::TextLocBottom )
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
