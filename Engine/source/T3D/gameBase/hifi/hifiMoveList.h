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

#ifndef _MOVELIST_HIFI_H_
#define _MOVELIST_HIFI_H_

#ifndef _MOVELIST_H_
#include "T3D/gameBase/moveList.h"
#endif

class HifiMoveList : public MoveList
{
   typedef MoveList Parent;

public:

   HifiMoveList();

   void init() { mTotalServerTicks = ServerTicksUninitialized; }

   void ghostReadExtra(NetObject *,BitStream *, bool newGhost);
   void ghostPreRead(NetObject *, bool newGhost);

   void clientWriteMovePacket(BitStream *bstream);
   void clientReadMovePacket(BitStream *);
   void serverWriteMovePacket(BitStream *);
   void serverReadMovePacket(BitStream *bstream);

   void markControlDirty();
   U32 getMoves(Move**,U32* numMoves);
   void onAdvanceObjects() { if (mMoveVec.size() > mLastSentMove-mFirstMoveIndex) mLastSentMove++; }

   void advanceMove();

protected:
   void resetMoveList();
   S32 getServerTicks(U32 serverTickNum);
   void updateClientServerTickDiff(S32 & tickDiff);
   bool serverTicksInitialized() { return mTotalServerTicks!=ServerTicksUninitialized; }

protected:
   U32 mLastSentMove;
   F32 mAvgMoveQueueSize;

   // server side move list management
   U32 mTargetMoveListSize;       // Target size of move buffer on server
   U32 mMaxMoveListSize;          // Max size move buffer allowed to grow to
   F32 mSmoothMoveAvg;            // Smoothing parameter for move list size running average
   F32 mMoveListSizeSlack;        // Amount above/below target size move list running average allowed to diverge
   bool mSuppressMove;            // If true, don't return move on server

   // client side tracking of server ticks
   enum { TotalTicksBits=10, TotalTicksMask = (1<<TotalTicksBits)-1, ServerTicksUninitialized=0xFFFFFFFF };
   U32 mTotalServerTicks;
};

#endif // _MOVELIST_HIFI_H_
