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
#include "gui/controls/guiAnimBitmapCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"



IMPLEMENT_CONOBJECT(guiAnimBitmapCtrl);

IMPLEMENT_CALLBACK(guiAnimBitmapCtrl, onLoop, void, (),
   (), "triggered when a loop completes");

IMPLEMENT_CALLBACK(guiAnimBitmapCtrl, onCompleted, void, (),
   (), "triggered when an animation completes");

IMPLEMENT_CALLBACK(guiAnimBitmapCtrl, onFrame, void, (S32 frameIndex, S32 frame),
   (frameIndex, frame), "triggered when a frame increments");

guiAnimBitmapCtrl::guiAnimBitmapCtrl(void)
{
   mAnimTexTiling = Point2I::One;
   mAnimTexFramesString = NULL;
   mAnimTexFrames.clear();
   mNumFrames = 0;
   mCurFrameIndex = 0;
   mFramesPerSec = 60;
   mAnimateTexture = false;
   mFrameTime = PlatformTimer::create();
   mLoop = true;
   mPlay = true;
   mReverse = false;
   mFinished = false;
}

guiAnimBitmapCtrl::~guiAnimBitmapCtrl(void)
{
   mAnimTexFrames.clear();
}
void guiAnimBitmapCtrl::initPersistFields()
{
   addField("AnimTexTiling", TYPEID< Point2I >(), Offset(mAnimTexTiling, guiAnimBitmapCtrl),
      "@brief The number of frames, in rows and columns stored in textureName "
      "(when animateTexture is true).\n\n"
      "A maximum of 256 frames can be stored in a single texture when using "
      "mAnimTexTiling. Value should be \"NumColumns NumRows\", for example \"4 4\".");
   addProtectedField("AnimTexFrames", TYPEID< StringTableEntry >(), Offset(mAnimTexFramesString, guiAnimBitmapCtrl), &ptSetFrameRanges, &defaultProtectedGetFn,
      "@brief A list of frames and/or frame ranges to use for particle "
      "animation if animateTexture is true.\n\n"
      "Each frame token must be separated by whitespace. A frame token must be "
      "a positive integer frame number or a range of frame numbers separated "
      "with a '-'. The range separator, '-', cannot have any whitspace around "
      "it.\n\n"
      "Ranges can be specified to move through the frames in reverse as well "
      "as forward (eg. 19-14). Frame numbers exceeding the number of tiles will "
      "wrap.\n"
      "@tsexample\n"
      "mAnimTexFrames = \"0-16 20 19 18 17 31-21\";\n"
      "@endtsexample\n");

   addField("loop", TypeBool, Offset(mLoop, guiAnimBitmapCtrl), "loop?");
   addField("play", TypeBool, Offset(mPlay, guiAnimBitmapCtrl), "play?");
   addField("reverse", TypeBool, Offset(mReverse, guiAnimBitmapCtrl), "play reversed?");
   addField("fps", TypeS32, Offset(mFramesPerSec, guiAnimBitmapCtrl), "Frame Rate");

   addProtectedField("curFrame", TypeS32, Offset(mCurFrameIndex, guiAnimBitmapCtrl), &ptSetFrame, &defaultProtectedGetFn, "Index of currently Displaying Frame ");

   Parent::initPersistFields();
   removeField("wrap");
}

bool guiAnimBitmapCtrl::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   if (!mAnimTexFramesString || !mAnimTexFramesString[0])
   {
      S32 n_tiles = mAnimTexTiling.x * mAnimTexTiling.y - 1;
      for (S32 i = 0; i <= n_tiles; i++)
         mAnimTexFrames.push_back(i);
      mNumFrames = mAnimTexFrames.size() - 1;
      if (mCurFrameIndex > mNumFrames)
         mCurFrameIndex = mNumFrames;
      return true;
   }

   return true;
}

bool guiAnimBitmapCtrl::ptSetFrame(void *object, const char *index, const char *data)
{
   guiAnimBitmapCtrl *pData = static_cast<guiAnimBitmapCtrl*>(object);

   if (!pData->mNumFrames)
   {
      pData->mCurFrameIndex = 0;
      return false;
   }

   S32 val = dAtoi(data);

   if (val < 0)
   {
      pData->mCurFrameIndex = pData->mNumFrames;
      return false;
   }
   else if (val > pData->mNumFrames)
   {
      pData->mCurFrameIndex = 0;
      return false;
   };

   pData->mCurFrameIndex = val;
   return true;
}

bool guiAnimBitmapCtrl::ptSetFrameRanges(void *object, const char *index, const char *data)
{
   guiAnimBitmapCtrl *pData = static_cast<guiAnimBitmapCtrl*>(object);

   // Here we parse mAnimTexFramesString into byte-size frame numbers in mAnimTexFrames.
   // Each frame token must be separated by whitespace.
   // A frame token must be a positive integer frame number or a range of frame numbers
   // separated with a '-'. 
   // The range separator, '-', cannot have any whitspace around it.
   // Ranges can be specified to move through the frames in reverse as well as forward.
   // Frame numbers exceeding the number of tiles will wrap.
   //   example:
   //     "0-16 20 19 18 17 31-21"

   S32 n_tiles = pData->mAnimTexTiling.x * pData->mAnimTexTiling.y - 1;

   pData->mAnimTexFrames.clear();

   if (!data || !data[0])
   {
      for (S32 i = 0; i <= n_tiles; i++)
         pData->mAnimTexFrames.push_back(i);
      pData->mNumFrames = pData->mAnimTexFrames.size() - 1;
      if (pData->mCurFrameIndex > pData->mNumFrames)
         pData->mCurFrameIndex = pData->mNumFrames;
      return true;
   }
   char* tokCopy = new char[dStrlen(data) + 1];
   dStrcpy(tokCopy, data);

   char* currTok = dStrtok(tokCopy, " \t");
   while (currTok != NULL)
   {
      char* minus = dStrchr(currTok, '-');
      if (minus)
      {
         // add a range of frames
         *minus = '\0';
         S32 range_a = dAtoi(currTok);
         S32 range_b = dAtoi(minus + 1);
         if (range_b < range_a)
         {
            // reverse frame range
            for (S32 i = range_a; i >= range_b; i--)
               pData->mAnimTexFrames.push_back(i);
         }
         else
         {
            // forward frame range
            for (S32 i = range_a; i <= range_b; i++)
               pData->mAnimTexFrames.push_back(i);
         }
      }
      else
      {
         // add one frame
         pData->mAnimTexFrames.push_back(dAtoi(currTok));
      }
      currTok = dStrtok(NULL, " \t");
   }

   // cleanup
   delete[] tokCopy;
   pData->mNumFrames = pData->mAnimTexFrames.size() - 1;
   if (pData->mCurFrameIndex > pData->mNumFrames)
      pData->mCurFrameIndex = pData->mNumFrames;
   return true;
}

void guiAnimBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if (mTextureObject)
   {
      if (mFrameTime->getElapsedMs() > 1000 / mFramesPerSec) //fps to msfp conversion
      {
         mFrameTime->reset();

         if (mPlay)
         {
            if (mReverse) //play backward
            {
               mCurFrameIndex--;
               if (mCurFrameIndex < 0)
               {
                  if (mLoop)
                  {
                     mCurFrameIndex = mNumFrames;
                     onLoop_callback();
                     mFinished = false;
                  }
                  else
                  {
                     mCurFrameIndex = 0;
                     if (!mFinished)
                        onCompleted_callback();
                     mFinished = true;
                  }
               }
               else
                  onFrame_callback(mCurFrameIndex, mAnimTexFrames[mCurFrameIndex]);
            }
            else // play forward
            {
               mCurFrameIndex++;

               if (mCurFrameIndex > mNumFrames)
               {
                  if (mLoop)
                  {
                     mCurFrameIndex = 0;
                     onLoop_callback();
                     mFinished = false;
                  }
                  else
                  {
                     mCurFrameIndex = mNumFrames;
                     if (!mFinished)
                        onCompleted_callback();
                     mFinished = true;
                  }
               }
               else
                  onFrame_callback(mCurFrameIndex, mAnimTexFrames[mCurFrameIndex]);
            }
         }
      }

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->setBitmapModulation(mColor);

      GFXTextureObject* texture = mTextureObject;

      Point2I modifiedSRC = Point2I(texture->mBitmapSize.x / mAnimTexTiling.x, texture->mBitmapSize.y / mAnimTexTiling.y);
      RectI srcRegion;
      Point2I offsetSRC = Point2I::Zero;

      offsetSRC.x = (texture->mBitmapSize.x / mAnimTexTiling.x) * (mAnimTexFrames[mCurFrameIndex] % mAnimTexTiling.x);
      offsetSRC.y = (texture->mBitmapSize.y / mAnimTexTiling.y) * (mAnimTexFrames[mCurFrameIndex] / mAnimTexTiling.x);

      srcRegion.set(offsetSRC, modifiedSRC);

      GFX->getDrawUtil()->drawBitmapStretchSR(texture, updateRect, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear, false);
   }

   if (mProfile->mBorder || !mTextureObject)
   {
      RectI rect(offset, getExtent());
      GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
   }

   renderChildControls(offset, updateRect);
}