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
#include "gui/controls/guiBitmapBarCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"



IMPLEMENT_CONOBJECT(GuiBitmapBarCtrl);

GuiBitmapBarCtrl::GuiBitmapBarCtrl(void)
{
   mPercent = 100.0;
   mVertical = false;
   mFlipClip = false;
}


void GuiBitmapBarCtrl::initPersistFields()
{
   addField("percent", TypeF32, Offset(mPercent, GuiBitmapBarCtrl),
      "% shown");
   addField("vertical", TypeBool, Offset(mVertical, GuiBitmapBarCtrl),
      "If true, the bitmap is clipped vertically.");
   addField("flipClip", TypeBool, Offset(mFlipClip, GuiBitmapBarCtrl),
      "If true, the bitmap is clipped in the oposite direction.");
   Parent::initPersistFields();
   removeField("wrap");
}

void GuiBitmapBarCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if (mTextureObject)
   {
      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->setBitmapModulation(mColor);
      F32 pct = (mPercent / 100.0);
      GFXTextureObject* texture = mTextureObject;
      Point2I modifiedSRC;
      modifiedSRC.x = mVertical ? (F32)texture->mBitmapSize.x : (F32)(texture->mBitmapSize.x*pct);
      modifiedSRC.y = mVertical ? (F32)(texture->mBitmapSize.y*pct) : (F32)texture->mBitmapSize.y;
      RectI srcRegion;
      Point2I offsetSRC = Point2I::Zero;
      if (mFlipClip)
      {
         offsetSRC.x = texture->mBitmapSize.x - modifiedSRC.x;
         offsetSRC.y = texture->mBitmapSize.y - modifiedSRC.y;
      }

      srcRegion.set(offsetSRC, modifiedSRC);

      RectI destRegion;
      Point2I modifiedDest;
      modifiedDest.x = mVertical ? (F32)updateRect.len_x() : (F32)(updateRect.len_x()*pct);
      modifiedDest.y = mVertical ? (F32)(updateRect.len_y()*pct) : (F32)updateRect.len_y();

      Point2I offsetDest = Point2I::Zero;
      if (mFlipClip)
      {
         offsetDest.x = updateRect.len_x() - modifiedDest.x;
         offsetDest.y = updateRect.len_y() - modifiedDest.y;
      }
      offsetDest += offset;
      destRegion.set(offsetDest, modifiedDest);

      GFX->getDrawUtil()->drawBitmapStretchSR(texture, destRegion, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear, false);
   }

   if (mProfile->mBorder || !mTextureObject)
   {
      RectI rect(offset, getExtent());
      GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
   }

   renderChildControls(offset, updateRect);
}