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

#include "platform/platform.h"

#include "gui/core/guiControl.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"


// [rene 11/09/09] Why does this not use the bitmap array from its profile?


/// Renders a skinned border.
class GuiBitmapBorderCtrl : public GuiControl
{
   typedef GuiControl Parent;

   enum {
      BorderTopLeft,
      BorderTopRight,
      BorderTop,
      BorderLeft,
      BorderRight,
      BorderBottomLeft,
      BorderBottom,
      BorderBottomRight,
      NumBitmaps
   };
	RectI *mBitmapBounds;  ///< bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
   GFXTexHandle mTextureObject;
public:
   bool onWake();
   void onSleep();
   void onRender(Point2I offset, const RectI &updateRect);
   DECLARE_CONOBJECT(GuiBitmapBorderCtrl);
   DECLARE_CATEGORY( "Gui Images" );
   DECLARE_DESCRIPTION( "A control that renders a skinned border." );
};

IMPLEMENT_CONOBJECT(GuiBitmapBorderCtrl);

ConsoleDocClass( GuiBitmapBorderCtrl,
   "@brief A control that renders a skinned border specified in its profile.\n\n"

   "This control uses the bitmap specified in it's profile (GuiControlProfile::bitmapName).  It takes this image and breaks up aspects of it "
   "to skin the border of this control with.  It is also important to set GuiControlProfile::hasBitmapArray to true on the profile as well.\n\n"

   "The bitmap referenced should be broken up into a 3 x 3 grid (using the top left color pixel as a border color between each of the images) "
   "in which it will map to the following places:\n"
   "1 = Top Left Corner\n"
   "2 = Top Right Corner\n"
   "3 = Top Center\n"
   "4 = Left Center\n"
   "5 = Right Center\n"
   "6 = Bottom Left Corner\n"
   "7 = Bottom Center\n"
   "8 = Bottom Right Corner\n"
   "0 = Nothing\n\n"

   "1 2 3\n"
   "4 5 0\n"
   "6 7 8\n\n"

   "@tsexample\n"
   "singleton GuiControlProfile (BorderGUIProfile)\n"
   "{\n"
   "   bitmap = \"core/art/gui/images/borderArray\";\n"
   "   hasBitmapArray = true;\n"
   "   opaque = false;\n"
   "};\n\n"

   "new GuiBitmapBorderCtrl(BitmapBorderGUI)\n"
   "{\n"
   "   profile = \"BorderGUIProfile\";\n"
   "   position = \"0 0\";\n"
   "   extent = \"400 40\";\n"
   "   visible = \"1\";\n"
   "};"
   "@endtsexample\n\n"

   "@see GuiControlProfile::bitmapName\n"
   "@see GuiControlProfile::hasBitmapArray\n\n"

   "@ingroup GuiImages"
);

bool GuiBitmapBorderCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the texture for the close, minimize, and maximize buttons
   mBitmapBounds = NULL;
   mTextureObject = mProfile->mTextureObject;
   if( mProfile->constructBitmapArray() >= NumBitmaps )
      mBitmapBounds = mProfile->mBitmapArrayRects.address();
   else
      Con::errorf( "GuiBitmapBorderCtrl: Could not construct bitmap array for profile '%s'", mProfile->getName() );
      
   return true;
}

void GuiBitmapBorderCtrl::onSleep()
{
   mTextureObject = NULL;
   mBitmapBounds = NULL;
   
   Parent::onSleep();
}

void GuiBitmapBorderCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   renderChildControls( offset, updateRect );
   
   if( mBitmapBounds )
   {
      GFX->setClipRect(updateRect);

      GFXDrawUtil* drawUtil = GFX->getDrawUtil();

      //draw the outline
      RectI winRect;
      winRect.point = offset;
      winRect.extent = getExtent();

      winRect.point.x += mBitmapBounds[BorderLeft].extent.x;
      winRect.point.y += mBitmapBounds[BorderTop].extent.y;

      winRect.extent.x -= mBitmapBounds[BorderLeft].extent.x + mBitmapBounds[BorderRight].extent.x;
      winRect.extent.y -= mBitmapBounds[BorderTop].extent.y + mBitmapBounds[BorderBottom].extent.y;

      if(mProfile->mOpaque)
        drawUtil->drawRectFill(winRect, mProfile->mFillColor);

      drawUtil->clearBitmapModulation();
      drawUtil->drawBitmapSR(mTextureObject, offset, mBitmapBounds[BorderTopLeft]);
      drawUtil->drawBitmapSR(mTextureObject, Point2I(offset.x + getWidth() - mBitmapBounds[BorderTopRight].extent.x, offset.y),
                      mBitmapBounds[BorderTopRight]);

      RectI destRect;
      destRect.point.x = offset.x + mBitmapBounds[BorderTopLeft].extent.x;
      destRect.point.y = offset.y;
      destRect.extent.x = getWidth() - mBitmapBounds[BorderTopLeft].extent.x - mBitmapBounds[BorderTopRight].extent.x;
      destRect.extent.y = mBitmapBounds[BorderTop].extent.y;
      RectI stretchRect = mBitmapBounds[BorderTop];
      stretchRect.inset(1,0);
      drawUtil->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

      destRect.point.x = offset.x;
      destRect.point.y = offset.y + mBitmapBounds[BorderTopLeft].extent.y;
      destRect.extent.x = mBitmapBounds[BorderLeft].extent.x;
      destRect.extent.y = getHeight() - mBitmapBounds[BorderTopLeft].extent.y - mBitmapBounds[BorderBottomLeft].extent.y;
      stretchRect = mBitmapBounds[BorderLeft];
      stretchRect.inset(0,1);
      drawUtil->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

      destRect.point.x = offset.x + getWidth() - mBitmapBounds[BorderRight].extent.x;
      destRect.extent.x = mBitmapBounds[BorderRight].extent.x;
      destRect.point.y = offset.y + mBitmapBounds[BorderTopRight].extent.y;
      destRect.extent.y = getHeight() - mBitmapBounds[BorderTopRight].extent.y - mBitmapBounds[BorderBottomRight].extent.y;

      stretchRect = mBitmapBounds[BorderRight];
      stretchRect.inset(0,1);
      drawUtil->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

      drawUtil->drawBitmapSR(mTextureObject, offset + Point2I(0, getHeight() - mBitmapBounds[BorderBottomLeft].extent.y), mBitmapBounds[BorderBottomLeft]);
      drawUtil->drawBitmapSR(mTextureObject, offset + getExtent() - mBitmapBounds[BorderBottomRight].extent, mBitmapBounds[BorderBottomRight]);

      destRect.point.x = offset.x + mBitmapBounds[BorderBottomLeft].extent.x;
      destRect.extent.x = getWidth() - mBitmapBounds[BorderBottomLeft].extent.x - mBitmapBounds[BorderBottomRight].extent.x;

      destRect.point.y = offset.y + getHeight() - mBitmapBounds[BorderBottom].extent.y;
      destRect.extent.y = mBitmapBounds[BorderBottom].extent.y;
      stretchRect = mBitmapBounds[BorderBottom];
      stretchRect.inset(1,0);

      drawUtil->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);
   }
}
