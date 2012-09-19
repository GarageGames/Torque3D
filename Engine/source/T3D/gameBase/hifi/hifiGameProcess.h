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

#ifndef _GAMEPROCESS_HIFI_H_
#define _GAMEPROCESS_HIFI_H_

#ifndef _GAMEPROCESS_H_
#include "T3D/gameBase/gameProcess.h"
#endif


/// List to keep track of GameBases to process.
class HifiClientProcessList : public ClientProcessList
{
   typedef ClientProcessList Parent;
   friend class HifiMoveList;

public:

   HifiClientProcessList();

   // ProcessList
   bool advanceTime(SimTime timeDelta);

   // ClientProcessList
   void clientCatchup(GameConnection*);

   static void init();
   static void shutdown();

protected:

   // tick cache functions -- client only
   void ageTickCache(S32 numToAge, S32 len);
   void forceHifiReset(bool reset) { mForceHifiReset=reset; }
   U32 getTotalTicks() { return mTotalTicks; }
   void updateMoveSync(S32 moveDiff);
   void skipAdvanceObjects(U32 ms) { mSkipAdvanceObjectsMs += ms; }

   // ProcessList
   void onTickObject(ProcessObject *);
   void advanceObjects();
   void onAdvanceObjects();

   void setCatchup(U32 catchup) { mCatchup = catchup; }

protected:

   U32 mSkipAdvanceObjectsMs;
   bool mForceHifiReset;
   U32 mCatchup;
};


class HifiServerProcessList : public ServerProcessList
{
   typedef ServerProcessList Parent;

public:

   HifiServerProcessList() {}

   static void init();
   static void shutdown();

protected:

   // ProcessList
   void onTickObject(ProcessObject *);
};

#endif // _GAMEPROCESS_HIFI_H_