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
#include "gui/core/guiDefaultControlRender.h"

#include "gui/core/guiTypes.h"
#include "core/color.h"
#include "math/mRect.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"


static ColorI colorLightGray(192, 192, 192);
static ColorI colorGray(128, 128, 128);
static ColorI colorDarkGray(64, 64, 64);
static ColorI colorWhite(255,255,255);
static ColorI colorBlack(0,0,0);

void renderRaisedBox( const RectI &bounds, GuiControlProfile *profile )
{
   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x - 1;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - 1;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   drawUtil->drawRectFill( bounds, profile->mFillColor);
   drawUtil->drawLine(l, t, l, b - 1, colorWhite);
   drawUtil->drawLine(l, t, r - 1, t, colorWhite);

   drawUtil->drawLine(l, b, r, b, colorBlack);
   drawUtil->drawLine(r, b - 1, r, t, colorBlack);

   drawUtil->drawLine(l + 1, b - 1, r - 1, b - 1, profile->mBorderColor);
   drawUtil->drawLine(r - 1, b - 2, r - 1, t + 1, profile->mBorderColor);
}

void renderSlightlyRaisedBox( const RectI &bounds, GuiControlProfile *profile )
{
   S32 l = bounds.point.x + 1, r = bounds.point.x + bounds.extent.x - 1;
   S32 t = bounds.point.y + 1, b = bounds.point.y + bounds.extent.y - 1;

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->drawRectFill( bounds, profile->mFillColor);
   drawer->drawLine(l, t, l, b, profile->mBorderColor);
   drawer->drawLine(l, t, r, t, profile->mBorderColor);
   drawer->drawLine(l + 1, b, r, b, profile->mBorderColor);
   drawer->drawLine(r, t + 1, r, b - 1, profile->mBorderColor);
}

void renderLoweredBox( const RectI &bounds, GuiControlProfile *profile )
{
   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x - 1;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - 1;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   drawUtil->drawRectFill( bounds, profile->mFillColor);

   drawUtil->drawLine(l, b, r, b, colorWhite);
   drawUtil->drawLine(r, b - 1, r, t, colorWhite);

   drawUtil->drawLine(l, t, r - 1, t, colorBlack);
   drawUtil->drawLine(l, t + 1, l, b - 1, colorBlack);

   drawUtil->drawLine(l + 1, t + 1, r - 2, t + 1, profile->mBorderColor);
   drawUtil->drawLine(l + 1, t + 2, l + 1, b - 2, profile->mBorderColor);
}

void renderSlightlyLoweredBox( const RectI &bounds, GuiControlProfile *profile )
{
   S32 l = bounds.point.x + 1, r = bounds.point.x + bounds.extent.x - 1;
   S32 t = bounds.point.y + 1, b = bounds.point.y + bounds.extent.y - 1;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   drawUtil->drawRectFill( bounds, profile->mFillColor);
   drawUtil->drawLine(l, b, r, b, profile->mBorderColor);
   drawUtil->drawLine(r, t, r, b - 1, profile->mBorderColor);
   drawUtil->drawLine(l, t, l, b - 1, profile->mBorderColor);
   drawUtil->drawLine(l + 1, t, r - 1, t, profile->mBorderColor);
}

void renderBorder( const RectI &bounds, GuiControlProfile *profile )
{
   S32 l = bounds.point.x, r = bounds.point.x + bounds.extent.x - 1;
   S32 t = bounds.point.y, b = bounds.point.y + bounds.extent.y - 1;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   switch(profile->mBorder)
   {
   case 1:
      drawer->drawRect(bounds, profile->mBorderColor);
      break;
   case 2:
      drawer->drawLine(l + 1, t + 1, l + 1, b - 2, profile->mBevelColorHL);
      drawer->drawLine(l + 2, t + 1, r - 2, t + 1, profile->mBevelColorHL);
      drawer->drawLine(r, t, r, b, profile->mBevelColorHL);
      drawer->drawLine(l, b, r - 1, b, profile->mBevelColorHL);
      drawer->drawLine(l, t, r - 1, t, profile->mBorderColorNA);
      drawer->drawLine(l, t + 1, l, b - 1, profile->mBorderColorNA);
      drawer->drawLine(l + 1, b - 1, r - 1, b - 1, profile->mBorderColorNA);
      drawer->drawLine(r - 1, t + 1, r - 1, b - 2, profile->mBorderColorNA);
      break;
   case 3:
      drawer->drawLine(l, b, r, b, profile->mBevelColorHL);
      drawer->drawLine(r, t, r, b - 1, profile->mBevelColorHL);
      drawer->drawLine(l + 1, b - 1, r - 1, b - 1, profile->mFillColor);
      drawer->drawLine(r - 1, t + 1, r - 1, b - 2, profile->mFillColor);
      drawer->drawLine(l, t, l, b - 1, profile->mBorderColorNA);
      drawer->drawLine(l + 1, t, r - 1, t, profile->mBorderColorNA);
      drawer->drawLine(l + 1, t + 1, l + 1, b - 2, profile->mBevelColorLL);
      drawer->drawLine(l + 2, t + 1, r - 2, t + 1, profile->mBevelColorLL);
      break;
   case 4:
      drawer->drawLine(l, t, l, b - 1, profile->mBevelColorHL);
      drawer->drawLine(l + 1, t, r, t, profile->mBevelColorHL);
      drawer->drawLine(l, b, r, b, profile->mBevelColorLL);
      drawer->drawLine(r, t + 1, r, b - 1, profile->mBevelColorLL);
      drawer->drawLine(l + 1, b - 1, r - 1, b - 1, profile->mBorderColor);
      drawer->drawLine(r - 1, t + 1, r - 1, b - 2, profile->mBorderColor);
      break;
   case 5:
      renderFilledBorder( bounds, profile );
      break;
      // 
   case -1:
      // Draw a simple sizable border with corners
      // Taken from the 'Skinnable GUI Controls in TGE' resource by Justin DuJardin       
      if(profile->mBitmapArrayRects.size() >= 8)
      {
         drawer->clearBitmapModulation();

         RectI destRect;
         RectI stretchRect;
         RectI* mBitmapBounds = profile->mBitmapArrayRects.address();

         //  Indices into the bitmap array
         enum
         {
            BorderTopLeft = 0,
            BorderTop,
            BorderTopRight,
            BorderLeft,
            //Fill,
            BorderRight,
            BorderBottomLeft,
            BorderBottom,
            BorderBottomRight,
            NumBitmaps
         };

         // Draw all corners first.

         //top left border
         drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y),mBitmapBounds[BorderTopLeft]);
         //top right border
         drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x + bounds.extent.x - mBitmapBounds[BorderTopRight].extent.x,bounds.point.y),mBitmapBounds[BorderTopRight]);

         //bottom left border
         drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y + bounds.extent.y - mBitmapBounds[BorderBottomLeft].extent.y),mBitmapBounds[BorderBottomLeft]);
         //bottom right border
         drawer->drawBitmapSR(profile->mTextureObject,Point2I(
            bounds.point.x + bounds.extent.x - mBitmapBounds[BorderBottomRight].extent.x,
            bounds.point.y + bounds.extent.y - mBitmapBounds[BorderBottomRight].extent.y),
            mBitmapBounds[BorderBottomRight]);

         // End drawing corners

         // Begin drawing sides and top stretched borders

         //start with top line stretch
         destRect.point.x = bounds.point.x + mBitmapBounds[BorderTopLeft].extent.x;
         destRect.extent.x = bounds.extent.x - mBitmapBounds[BorderTopRight].extent.x - mBitmapBounds[BorderTopLeft].extent.x;
         destRect.extent.y = mBitmapBounds[BorderTop].extent.y;
         destRect.point.y = bounds.point.y;
         //stretch it
         stretchRect = mBitmapBounds[BorderTop];
         stretchRect.inset(1,0);
         //draw it
         drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
         //bottom line stretch
         destRect.point.x = bounds.point.x + mBitmapBounds[BorderBottomLeft].extent.x;
         destRect.extent.x = bounds.extent.x - mBitmapBounds[BorderBottomRight].extent.x - mBitmapBounds[BorderBottomLeft].extent.x;
         destRect.extent.y = mBitmapBounds[BorderBottom].extent.y;
         destRect.point.y = bounds.point.y + bounds.extent.y - mBitmapBounds[BorderBottom].extent.y;
         //stretch it
         stretchRect = mBitmapBounds[BorderBottom];
         stretchRect.inset(1,0);
         //draw it
         drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
         //left line stretch
         destRect.point.x = bounds.point.x;
         destRect.extent.x = mBitmapBounds[BorderLeft].extent.x;
         destRect.extent.y = bounds.extent.y - mBitmapBounds[BorderTopLeft].extent.y - mBitmapBounds[BorderBottomLeft].extent.y;
         destRect.point.y = bounds.point.y + mBitmapBounds[BorderTopLeft].extent.y;
         //stretch it
         stretchRect = mBitmapBounds[BorderLeft];
         stretchRect.inset(0,1);
         //draw it
         drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
         //right line stretch
         destRect.point.x = bounds.point.x + bounds.extent.x - mBitmapBounds[BorderRight].extent.x;
         destRect.extent.x = mBitmapBounds[BorderRight].extent.x;
         destRect.extent.y = bounds.extent.y - mBitmapBounds[BorderTopRight].extent.y - mBitmapBounds[BorderBottomRight].extent.y;
         destRect.point.y = bounds.point.y + mBitmapBounds[BorderTopRight].extent.y;
         //stretch it
         stretchRect = mBitmapBounds[BorderRight];
         stretchRect.inset(0,1);
         //draw it
         drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);

         // End drawing sides and top stretched borders
         break;
      }
   case -2:
      // Draw a simple sizable border with corners that is filled in
      renderSizableBitmapBordersFilled(bounds, 1, profile);
      break;
   case -3:
      // Draw a simple fixed height border with center fill horizontally.
      renderFixedBitmapBordersFilled( bounds, 1, profile );
      break;

   }
}

void renderFilledBorder( const RectI &bounds, GuiControlProfile *profile )
{
   renderFilledBorder( bounds, profile->mBorderColor, profile->mFillColor, profile->mBorderThickness );
}

void renderFilledBorder( const RectI &bounds, const ColorI &borderColor, const ColorI &fillColor, U32 thickness )
{
   RectI fillBounds = bounds;
   fillBounds.inset( thickness, thickness );

   GFX->getDrawUtil()->drawRectFill( bounds, borderColor ); 
   GFX->getDrawUtil()->drawRectFill( fillBounds, fillColor );
}

//  Render out the sizable bitmap borders based on a multiplier into the bitmap array
// Based on the 'Skinnable GUI Controls in TGE' resource by Justin DuJardin
void renderSizableBitmapBordersFilled( const RectI &bounds, S32 baseMultiplier, GuiControlProfile *profile)
{
   //  Indices into the bitmap array
   S32 numBitmaps = 9;
   S32 borderTopLeft =     numBitmaps * baseMultiplier - numBitmaps;
   S32 borderTop =         1 + borderTopLeft;
   S32 borderTopRight =    2 + borderTopLeft;
   S32 borderLeft =        3 + borderTopLeft;
   S32 fill =              4 + borderTopLeft;
   S32 borderRight =       5 + borderTopLeft;
   S32 borderBottomLeft =  6 + borderTopLeft;
   S32 borderBottom =      7 + borderTopLeft;
   S32 borderBottomRight = 8 + borderTopLeft;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   drawer->clearBitmapModulation();

   if(profile->mBitmapArrayRects.size() >= (numBitmaps * baseMultiplier))
   {
      RectI destRect;
      RectI stretchRect;
      RectI* mBitmapBounds = profile->mBitmapArrayRects.address();

      // Draw all corners first.

      //top left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y),mBitmapBounds[borderTopLeft]);
      //top right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x + bounds.extent.x - mBitmapBounds[borderTopRight].extent.x,bounds.point.y),mBitmapBounds[borderTopRight]);

      //bottom left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottomLeft].extent.y),mBitmapBounds[borderBottomLeft]);
      //bottom right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(
         bounds.point.x + bounds.extent.x - mBitmapBounds[borderBottomRight].extent.x,
         bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottomRight].extent.y),
         mBitmapBounds[borderBottomRight]);

      // End drawing corners

      // Begin drawing sides and top stretched borders

      //start with top line stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderTopLeft].extent.x;
      destRect.extent.x = bounds.extent.x - mBitmapBounds[borderTopRight].extent.x - mBitmapBounds[borderTopLeft].extent.x;
      destRect.extent.y = mBitmapBounds[borderTop].extent.y;
      destRect.point.y = bounds.point.y;
      //stretch it
      stretchRect = mBitmapBounds[borderTop];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //bottom line stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderBottomLeft].extent.x;
      destRect.extent.x = bounds.extent.x - mBitmapBounds[borderBottomRight].extent.x - mBitmapBounds[borderBottomLeft].extent.x;
      destRect.extent.y = mBitmapBounds[borderBottom].extent.y;
      destRect.point.y = bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottom].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderBottom];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //left line stretch
      destRect.point.x = bounds.point.x;
      destRect.extent.x = mBitmapBounds[borderLeft].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTopLeft].extent.y - mBitmapBounds[borderBottomLeft].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTopLeft].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderLeft];
      stretchRect.inset(0,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //right line stretch
      destRect.point.x = bounds.point.x + bounds.extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.x = mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTopRight].extent.y - mBitmapBounds[borderBottomRight].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTopRight].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderRight];
      stretchRect.inset(0,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //fill stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderLeft].extent.x;
      destRect.extent.x = (bounds.extent.x) - mBitmapBounds[borderLeft].extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTop].extent.y - mBitmapBounds[borderBottom].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTop].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[fill];
      stretchRect.inset(1,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);

      // End drawing sides and top stretched borders
   }
}


//  Render out the sizable bitmap borders based on a multiplier into the bitmap array
// Based on the 'Skinnable GUI Controls in TGE' resource by Justin DuJardin
void renderSizableBitmapBordersFilledIndex( const RectI &bounds, S32 startIndex, GuiControlProfile *profile )
{
   //  Indices into the bitmap array
   S32 numBitmaps = 9;
   S32 borderTopLeft =     startIndex;
   S32 borderTop =         1 + borderTopLeft;
   S32 borderTopRight =    2 + borderTopLeft;
   S32 borderLeft =        3 + borderTopLeft;
   S32 fill =              4 + borderTopLeft;
   S32 borderRight =       5 + borderTopLeft;
   S32 borderBottomLeft =  6 + borderTopLeft;
   S32 borderBottom =      7 + borderTopLeft;
   S32 borderBottomRight = 8 + borderTopLeft;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   drawer->clearBitmapModulation();
   if(profile->mBitmapArrayRects.size() >= (startIndex + numBitmaps))
   {
      RectI destRect;
      RectI stretchRect;
      RectI* mBitmapBounds = profile->mBitmapArrayRects.address();

      // Draw all corners first.

      //top left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y),mBitmapBounds[borderTopLeft]);
      //top right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x + bounds.extent.x - mBitmapBounds[borderTopRight].extent.x,bounds.point.y),mBitmapBounds[borderTopRight]);

      //bottom left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottomLeft].extent.y),mBitmapBounds[borderBottomLeft]);
      //bottom right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(
         bounds.point.x + bounds.extent.x - mBitmapBounds[borderBottomRight].extent.x,
         bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottomRight].extent.y),
         mBitmapBounds[borderBottomRight]);

      // End drawing corners

      // Begin drawing sides and top stretched borders

      //start with top line stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderTopLeft].extent.x;
      destRect.extent.x = bounds.extent.x - mBitmapBounds[borderTopRight].extent.x - mBitmapBounds[borderTopLeft].extent.x;
      destRect.extent.y = mBitmapBounds[borderTop].extent.y;
      destRect.point.y = bounds.point.y;
      //stretch it
      stretchRect = mBitmapBounds[borderTop];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //bottom line stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderBottomLeft].extent.x;
      destRect.extent.x = bounds.extent.x - mBitmapBounds[borderBottomRight].extent.x - mBitmapBounds[borderBottomLeft].extent.x;
      destRect.extent.y = mBitmapBounds[borderBottom].extent.y;
      destRect.point.y = bounds.point.y + bounds.extent.y - mBitmapBounds[borderBottom].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderBottom];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //left line stretch
      destRect.point.x = bounds.point.x;
      destRect.extent.x = mBitmapBounds[borderLeft].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTopLeft].extent.y - mBitmapBounds[borderBottomLeft].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTopLeft].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderLeft];
      stretchRect.inset(0,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //left line stretch
      destRect.point.x = bounds.point.x + bounds.extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.x = mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTopRight].extent.y - mBitmapBounds[borderBottomRight].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTopRight].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[borderRight];
      stretchRect.inset(0,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);
      //fill stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderLeft].extent.x;
      destRect.extent.x = (bounds.extent.x) - mBitmapBounds[borderLeft].extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = bounds.extent.y - mBitmapBounds[borderTop].extent.y - mBitmapBounds[borderBottom].extent.y;
      destRect.point.y = bounds.point.y + mBitmapBounds[borderTop].extent.y;
      //stretch it
      stretchRect = mBitmapBounds[fill];
      stretchRect.inset(1,1);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);

      // End drawing sides and top stretched borders
   }
}



//  Render out the fixed bitmap borders based on a multiplier into the bitmap array
// It renders left and right caps, with a sizable fill area in the middle to reach
// the x extent.  It does not stretch in the y direction.
void renderFixedBitmapBordersFilled( const RectI &bounds, S32 baseMultiplier, GuiControlProfile *profile )
{
   //  Indices into the bitmap array
   S32 numBitmaps = 3;
   S32 borderLeft =     numBitmaps * baseMultiplier - numBitmaps;
   S32 fill =              1 + borderLeft;
   S32 borderRight =       2 + borderLeft;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   drawer->clearBitmapModulation();
   if(profile->mBitmapArrayRects.size() >= (numBitmaps * baseMultiplier))
   {
      RectI destRect;
      RectI stretchRect;
      RectI* mBitmapBounds = profile->mBitmapArrayRects.address();

      // Draw all corners first.

      //left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y),mBitmapBounds[borderLeft]);
      //right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x + bounds.extent.x - mBitmapBounds[borderRight].extent.x,bounds.point.y),mBitmapBounds[borderRight]);

      // End drawing corners

      // Begin drawing fill

      //fill stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderLeft].extent.x;
      destRect.extent.x = (bounds.extent.x) - mBitmapBounds[borderLeft].extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = mBitmapBounds[fill].extent.y;
      destRect.point.y = bounds.point.y;
      //stretch it
      stretchRect = mBitmapBounds[fill];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);

      // End drawing fill
   }
}

//  Render out the fixed bitmap borders based on a multiplier into the bitmap array
// It renders left and right caps, with a sizable fill area in the middle to reach
// the x extent.  It does not stretch in the y direction.
void renderFixedBitmapBordersFilledIndex( const RectI &bounds, S32 startIndex, GuiControlProfile *profile )
{
   //  Indices into the bitmap array
   S32 numBitmaps = 3;
   S32 borderLeft =     startIndex;
   S32 fill =              1 + startIndex;
   S32 borderRight =       2 + startIndex;

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();
   if(profile->mBitmapArrayRects.size() >= (startIndex + numBitmaps))
   {
      RectI destRect;
      RectI stretchRect;
      RectI* mBitmapBounds = profile->mBitmapArrayRects.address();

      // Draw all corners first.

      //left border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x,bounds.point.y),mBitmapBounds[borderLeft]);
      //right border
      drawer->drawBitmapSR(profile->mTextureObject,Point2I(bounds.point.x + bounds.extent.x - mBitmapBounds[borderRight].extent.x,bounds.point.y),mBitmapBounds[borderRight]);

      // End drawing corners

      // Begin drawing fill

      //fill stretch
      destRect.point.x = bounds.point.x + mBitmapBounds[borderLeft].extent.x;
      destRect.extent.x = (bounds.extent.x) - mBitmapBounds[borderLeft].extent.x - mBitmapBounds[borderRight].extent.x;
      destRect.extent.y = mBitmapBounds[fill].extent.y;
      destRect.point.y = bounds.point.y;
      //stretch it
      stretchRect = mBitmapBounds[fill];
      stretchRect.inset(1,0);
      //draw it
      drawer->drawBitmapStretchSR(profile->mTextureObject,destRect,stretchRect);

      // End drawing fill
   }
}
