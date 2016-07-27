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

#ifndef _GUIANIMBITMAPCTRL_H_
#define _GUIANIMBITMAPCTRL_H_

#ifndef _GUIBITMAPCTRL_H_
#include "gui/controls/guiBitmapCtrl.h"
#endif

class guiAnimBitmapCtrl : public GuiBitmapCtrl
{
public:
   typedef GuiBitmapCtrl Parent;

protected:
   /// max frames (x,y)
   Point2I           mAnimTexTiling;
   /// frames to use
   StringTableEntry  mAnimTexFramesString;
   /// frames to use (internal)
   Vector<S32>       mAnimTexFrames;
   U32               mNumFrames;
   S32               mCurFrameIndex;

   bool              mLoop;
   bool              mPlay;
   bool              mReverse;
   S32               mFramesPerSec;
   bool              mAnimateTexture;
   PlatformTimer *   mFrameTime;
   bool              mFinished;
   static bool ptSetFrame(void *object, const char *index, const char *data);
   static bool ptSetFrameRanges(void *object, const char *index, const char *data);
public:
   guiAnimBitmapCtrl();
   ~guiAnimBitmapCtrl();
   bool onAdd();

   static void initPersistFields();
   void onRender(Point2I offset, const RectI &updateRect);
   DECLARE_CONOBJECT(guiAnimBitmapCtrl);
   DECLARE_CATEGORY("Gui Images");
   DECLARE_DESCRIPTION("A control that clips a bitmap based on %.");

   DECLARE_CALLBACK(void, onLoop, ());
   DECLARE_CALLBACK(void, onCompleted, ());
   DECLARE_CALLBACK(void, onFrame, (S32 frameIndex, S32 frame));
};

#endif