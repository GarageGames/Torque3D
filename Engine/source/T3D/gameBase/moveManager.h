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

#ifndef _MOVEMANAGER_H_
#define _MOVEMANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

enum MoveConstants {
   MaxTriggerKeys = 6,
   MaxMoveQueueSize = 45,
};

class BitStream;

struct Move
{
   enum : U32 
   { 
      ChecksumBits = 16, 
      ChecksumMask = ((1<<ChecksumBits)-1), 
      ChecksumMismatch = U32(-1) 
   };

   // packed storage rep, set in clamp
   S32 px, py, pz;
   U32 pyaw, ppitch, proll;
   F32 x, y, z;          // float -1 to 1
   F32 yaw, pitch, roll; // 0-2PI
   U32 id;               // sync'd between server & client - debugging tool.
   U32 sendCount;
   U32 checksum;

   bool deviceIsKeyboardMouse;
   bool freeLook;
   bool trigger[MaxTriggerKeys];

   Move();

   virtual void pack(BitStream *stream, const Move * move = NULL);
   virtual void unpack(BitStream *stream, const Move * move = NULL);
   virtual void clamp();
   virtual void unclamp();

protected:
   bool packMove(BitStream *stream, const Move* basemove, bool alwaysWriteAll);
   bool unpackMove(BitStream *stream, const Move* basemove, bool alwaysReadAll);
};

extern const Move NullMove;

class MoveManager
{
public:
   static bool mDeviceIsKeyboardMouse;
   static F32 mForwardAction;
   static F32 mBackwardAction;
   static F32 mUpAction;
   static F32 mDownAction;
   static F32 mLeftAction;
   static F32 mRightAction;

   static bool mFreeLook;
   static F32 mPitch;
   static F32 mYaw;
   static F32 mRoll;

   static F32 mPitchUpSpeed;
   static F32 mPitchDownSpeed;
   static F32 mYawLeftSpeed;
   static F32 mYawRightSpeed;
   static F32 mRollLeftSpeed;
   static F32 mRollRightSpeed;
   static F32 mXAxis_L;
   static F32 mYAxis_L;
   static F32 mXAxis_R;
   static F32 mYAxis_R;

   static U32 mTriggerCount[MaxTriggerKeys];
   static U32 mPrevTriggerCount[MaxTriggerKeys];

   static void init();
};

#endif
