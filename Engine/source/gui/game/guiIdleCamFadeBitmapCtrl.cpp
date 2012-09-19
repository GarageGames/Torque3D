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
#include "gui/controls/guiBitmapCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDrawUtil.h"


class GuiIdleCamFadeBitmapCtrl : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;
public:
   DECLARE_CONOBJECT(GuiIdleCamFadeBitmapCtrl);
   DECLARE_CATEGORY( "Gui Images" );

   U32 wakeTime;
   bool done;
   U32 fadeinTime;
   U32 fadeoutTime;
   bool doFadeIn;
   bool doFadeOut;

   GuiIdleCamFadeBitmapCtrl()
   {
      wakeTime    = 0;
      fadeinTime  = 1000;
      fadeoutTime = 1000;
      done        = false;

      doFadeIn    = false;
      doFadeOut   = false;
   }
   void onPreRender()
   {
      Parent::onPreRender();
      setUpdate();
   }
   void onMouseDown(const GuiEvent &)
   {
      Con::executef(this, "click");
   }
   bool onKeyDown(const GuiEvent &)
   {
      Con::executef(this, "click");
      return true;
   }
   bool onWake()
   {
      if(!Parent::onWake())
         return false;
      wakeTime = Platform::getRealMilliseconds();
      return true;
   }

   void fadeIn()
   {
      wakeTime = Platform::getRealMilliseconds();
      doFadeIn = true;
      doFadeOut = false;
      done = false;
   }

   void fadeOut()
   {
      wakeTime = Platform::getRealMilliseconds();
      doFadeIn = false;
      doFadeOut = true;
      done = false;
   }

   void onRender(Point2I offset, const RectI &updateRect)
   {
      U32 elapsed = Platform::getRealMilliseconds() - wakeTime;

      U32 alpha;
      if (doFadeOut && elapsed < fadeoutTime)
      {
         // fade out
         alpha = 255 - (255 * (F32(elapsed) / F32(fadeoutTime)));
      }
      else if (doFadeIn && elapsed < fadeinTime)
      {
         // fade in
         alpha = 255 * F32(elapsed) / F32(fadeinTime);
      }
      else
      {
         // done state
         alpha = doFadeIn ? 255 : 0;
         done = true;
      }

      ColorI color(255,255,255,alpha);
      if (mTextureObject)
      {
         GFX->getDrawUtil()->setBitmapModulation(color);

         if(mWrap)
         {

            GFXTextureObject* texture = mTextureObject;
            RectI srcRegion;
            RectI dstRegion;
            float xdone = ((float)getExtent().x/(float)texture->mBitmapSize.x)+1;
            float ydone = ((float)getExtent().y/(float)texture->mBitmapSize.y)+1;

            int xshift = mStartPoint.x%texture->mBitmapSize.x;
            int yshift = mStartPoint.y%texture->mBitmapSize.y;
            for(int y = 0; y < ydone; ++y)
               for(int x = 0; x < xdone; ++x)
               {
                  srcRegion.set(0,0,texture->mBitmapSize.x,texture->mBitmapSize.y);
                  dstRegion.set( ((texture->mBitmapSize.x*x)+offset.x)-xshift,
                     ((texture->mBitmapSize.y*y)+offset.y)-yshift,
                     texture->mBitmapSize.x,
                     texture->mBitmapSize.y);
                  GFX->getDrawUtil()->drawBitmapStretchSR(texture,dstRegion, srcRegion);
               }

         }
         else
         {
            RectI rect(offset, getExtent());
            GFX->getDrawUtil()->drawBitmapStretch(mTextureObject, rect);
         }
      }

      if (mProfile->mBorder || !mTextureObject)
      {
         RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
         ColorI borderCol(mProfile->mBorderColor);
         borderCol.alpha = alpha;
         GFX->getDrawUtil()->drawRect(rect, borderCol);
      }

      renderChildControls(offset, updateRect);
   }

   static void initPersistFields()
   {
      addField("fadeinTime", TypeS32, Offset(fadeinTime, GuiIdleCamFadeBitmapCtrl));
      addField("fadeoutTime", TypeS32, Offset(fadeoutTime, GuiIdleCamFadeBitmapCtrl));
      addField("done", TypeBool, Offset(done, GuiIdleCamFadeBitmapCtrl));
      Parent::initPersistFields();
   }
};

IMPLEMENT_CONOBJECT(GuiIdleCamFadeBitmapCtrl);

ConsoleDocClass( GuiIdleCamFadeBitmapCtrl,
				"@brief GUI that will fade the current view in and out.\n\n"
				"Main difference between this and FadeinBitmap is this appears to "
				"fade based on the source texture.\n\n"
				"This is going to be deprecated, and any useful code ported to FadeinBitmap\n\n"
				"@internal");

ConsoleMethod(GuiIdleCamFadeBitmapCtrl, fadeIn, void, 2, 2, "()"
			  "@internal")
{
   object->fadeIn();
}

ConsoleMethod(GuiIdleCamFadeBitmapCtrl, fadeOut, void, 2, 2, "()"
			  "@internal")
{
   object->fadeOut();
}
