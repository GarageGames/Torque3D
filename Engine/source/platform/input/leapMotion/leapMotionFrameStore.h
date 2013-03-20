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

#ifndef _LEAPMOTIONFRAMESTORE_H_
#define _LEAPMOTIONFRAMESTORE_H_

#include "platformWin32/platformWin32.h"
#include "Leap.h"

class SimGroup;

class LeapMotionFrameStore
{
public:
   // The maximum number of frames to keep
   static S32 smMaximumFramesStored;

   static SimGroup* smFrameGroup;

public:
   LeapMotionFrameStore();
   virtual ~LeapMotionFrameStore();

   static void staticInit();

   static bool isFrameGroupDefined() { return smFrameGroup != NULL; }
   static SimGroup* getFrameGroup() { return smFrameGroup; }

   S32 generateNewFrame(const Leap::Frame& frame, const F32& maxHandAxisRadius);

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "LeapMotionFrameStore"; }   
};

/// Returns the LeapMotionFrameStore singleton.
#define LEAPMOTIONFS ManagedSingleton<LeapMotionFrameStore>::instance()

#endif   // _LEAPMOTIONFRAMESTORE_H_
