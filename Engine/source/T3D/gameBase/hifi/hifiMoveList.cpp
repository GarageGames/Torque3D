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

#include "T3D/gameBase/hifi/hifiMoveList.h"
#include "T3D/gameBase/hifi/hifiGameProcess.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"

#define MAX_MOVE_PACKET_SENDS 4

const U32 DefaultTargetMoveListSize = 3;
const U32 DefaultMaxMoveSizeList = 5;
const F32 DefaultSmoothMoveAvg = 0.15f;
const F32 DefaultMoveListSizeSlack = 1.0f;

HifiMoveList::HifiMoveList()
{
   mLastSentMove = 0;
   mAvgMoveQueueSize = DefaultTargetMoveListSize ;
   mTargetMoveListSize = DefaultTargetMoveListSize;
   mMaxMoveListSize = DefaultMaxMoveSizeList;
   mSmoothMoveAvg = DefaultSmoothMoveAvg;
   mMoveListSizeSlack = DefaultMoveListSizeSlack;
   mTotalServerTicks = ServerTicksUninitialized;
   mSuppressMove = false;
}

void HifiMoveList::updateClientServerTickDiff(S32 & tickDiff)
{
   if (mLastMoveAck==0)
      tickDiff=0;

   // Make adjustments to move list to account for tick mis-matches between client and server.
   if (tickDiff>0)
   {
      // Server ticked more than client.  Adjust for this by reseting all hifi objects
      // to a later position in the tick cache (see ageTickCache below) and at the same
      // time pulling back some moves we thought we had made (so that time on client
      // doesn't change).
      S32 dropTicks = tickDiff;
      while (dropTicks)
      {         
#ifdef TORQUE_DEBUG_NET_MOVES
         Con::printf("dropping move%s",mLastClientMove>mFirstMoveIndex ? "" : " but none there");
#endif
         if (mLastClientMove>mFirstMoveIndex)
            mLastClientMove--;
         else
            tickDiff--;
         dropTicks--;
      }
      AssertFatal(mLastClientMove >= mFirstMoveIndex, "Bad move request");
      AssertFatal(mLastClientMove - mFirstMoveIndex <= mMoveVec.size(), "Desynched first and last move.");
   }
   else
   {
      // Client ticked more than server.  Adjust for this by taking extra moves
      // (either adding back moves that were dropped above, or taking new ones).
      for (S32 i=0; i<-tickDiff; i++)
      {
         if (mMoveVec.size() > mLastClientMove - mFirstMoveIndex)
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            Con::printf("add back move");
#endif
            mLastClientMove++;
         }
         else
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            Con::printf("add back move -- create one");
#endif
            collectMove();
            mLastClientMove++;
         }
      }
   }

   // drop moves that are not made yet (because we rolled them back) and not yet sent   
   U32 len = getMax(mLastClientMove-mFirstMoveIndex,mLastSentMove-mFirstMoveIndex);
   mMoveVec.setSize(len);

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("move list size: %i, last move: %i, last sent: %i",mMoveVec.size(),mLastClientMove-mFirstMoveIndex,mLastSentMove-mFirstMoveIndex);
#endif      
}

S32 HifiMoveList::getServerTicks(U32 serverTickNum)
{
   S32 serverTicks=0;
   if (serverTicksInitialized())
   {
      // handle tick wrapping...
      const S32 MaxTickCount = (1<<TotalTicksBits);
      const S32 HalfMaxTickCount = MaxTickCount>>1;
      U32 prevTickNum = mTotalServerTicks & TotalTicksMask;
      serverTicks = serverTickNum-prevTickNum;
      if (serverTicks>HalfMaxTickCount)
         serverTicks -= MaxTickCount;
      else if (-serverTicks>HalfMaxTickCount)
         serverTicks += MaxTickCount;
      AssertFatal(serverTicks>=0,"Server can't tick backwards!!!");
      if (serverTicks<0)
         serverTicks=0;
   }
   mTotalServerTicks = serverTickNum;
   return serverTicks;
}

void HifiMoveList::markControlDirty()
{
   mLastClientMove = mLastMoveAck;

   // save state for future update
   GameBase *obj = mConnection->getControlObject();
   AssertFatal(obj,"ClientProcessList::markControlDirty: no control object");
   obj->setGhostUpdated(true);
   obj->getTickCache().beginCacheList();
   TickCacheEntry * tce = obj->getTickCache().incCacheList();
   BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
   obj->writePacketData( mConnection, &bs );
}

void HifiMoveList::resetMoveList()
{
   mMoveVec.clear();
   mLastMoveAck = 0;
   mLastClientMove = 0;
   mFirstMoveIndex = 0;
   mLastSentMove = 0;   
}

U32 HifiMoveList::getMoves(Move** movePtr,U32* numMoves)
{
   if (mConnection->isConnectionToServer())
      // give back moves starting at the last client move...
      return Parent::getMoves(movePtr,numMoves);

   if (mSuppressMove || mMoveVec.size()==0)
   {
      *numMoves=0;
      *movePtr=NULL;
   }
   else
   {
      *numMoves=1;
      *movePtr=mMoveVec.begin();
   }

   return *numMoves;
}

void HifiMoveList::advanceMove()
{ 
   S32 numMoves = mMoveVec.size();
   mAvgMoveQueueSize *= (1.0f-mSmoothMoveAvg);
   mAvgMoveQueueSize += mSmoothMoveAvg * F32(numMoves);

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("moves remaining: %i, running avg: %f",numMoves,mAvgMoveQueueSize);
#endif

   if (mAvgMoveQueueSize<mTargetMoveListSize-mMoveListSizeSlack && numMoves<mTargetMoveListSize && numMoves)
   {
      numMoves=0;
      mAvgMoveQueueSize = (F32)getMax(S32(mAvgMoveQueueSize + mMoveListSizeSlack + 0.5f),numMoves);

#ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("too few moves on server, padding with null move");
#endif
   }
   if (numMoves)
      numMoves=1;

   if ( mMoveVec.size()>mMaxMoveListSize || (mAvgMoveQueueSize>mTargetMoveListSize+mMoveListSizeSlack && mMoveVec.size()>mTargetMoveListSize) )
   {
      U32 drop = mMoveVec.size()-mTargetMoveListSize;
      clearMoves(drop);
      mAvgMoveQueueSize = (F32)mTargetMoveListSize;

#ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("too many moves on server, dropping moves (%i)",drop);
#endif
   }

   mSuppressMove = numMoves == 0;
   
   // now clear move
   if (areMovesPending())
      clearMoves(1);
}

void HifiMoveList::clientWriteMovePacket(BitStream *bstream)
{
   if (!serverTicksInitialized())
      resetMoveList();

   AssertFatal(mLastMoveAck == mFirstMoveIndex, "Invalid move index.");

   // enforce limit on number of moves sent
   if (mLastSentMove<mFirstMoveIndex)
      mLastSentMove=mFirstMoveIndex;
   U32 count = mLastSentMove-mFirstMoveIndex;

   Move * move = mMoveVec.address();
   U32 start = mLastMoveAck;
   U32 offset;
   for(offset = 0; offset < count; offset++)
      if(move[offset].sendCount < MAX_MOVE_PACKET_SENDS)
         break;
   if(offset == count && count != 0)
      offset--;

   start += offset;
   count -= offset;

   if (count > MaxMoveCount)
      count = MaxMoveCount;
   bstream->writeInt(start,32);
   bstream->writeInt(count,MoveCountBits);
   Move * prevMove = NULL;
   for (int i = 0; i < count; i++)
   {
      move[offset + i].sendCount++;
      move[offset + i].pack(bstream,prevMove);
      bstream->writeInt(move[offset + i].checksum,Move::ChecksumBits);
      prevMove = &move[offset+i];
   }
}

void HifiMoveList::serverReadMovePacket(BitStream *bstream)
{
   // Server side packet read.
   U32 start = bstream->readInt(32);
   U32 count = bstream->readInt(MoveCountBits);

   Move * prevMove = NULL;
   Move prevMoveHolder;

   // Skip forward (must be starting up), or over the moves
   // we already have.
   int skip = mLastMoveAck - start;
   if (skip < 0) 
   {
      mLastMoveAck = start;
   }
   else 
   {
      if (skip > count)
         skip = count;
      for (int i = 0; i < skip; i++)
      {
         prevMoveHolder.unpack(bstream,prevMove);
         prevMoveHolder.checksum = bstream->readInt(Move::ChecksumBits);
         prevMove = &prevMoveHolder;
         S32 idx = mMoveVec.size()-skip+i;
         if (idx>=0)
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            if (mMoveVec[idx].checksum != prevMoveHolder.checksum)
               Con::printf("updated checksum on move %i from %i to %i",mMoveVec[idx].id,mMoveVec[idx].checksum,prevMoveHolder.checksum);
#endif
            mMoveVec[idx].checksum = prevMoveHolder.checksum;
         }
      }
      start += skip;
      count = count - skip;
   }

   // Put the rest on the move list.
   int index = mMoveVec.size();
   mMoveVec.increment(count);
   while (index < mMoveVec.size())
   {
      mMoveVec[index].unpack(bstream,prevMove);
      mMoveVec[index].checksum = bstream->readInt(Move::ChecksumBits);
      prevMove = &mMoveVec[index];
      mMoveVec[index].id = start++;
      index ++;
   }

   mLastMoveAck += count;

   if (mMoveVec.size()>mMaxMoveListSize)
   {
      U32 drop = mMoveVec.size()-mTargetMoveListSize;
      clearMoves(drop);
      mAvgMoveQueueSize = (F32)mTargetMoveListSize;

#ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("too many moves on server, dropping moves (%i)",drop);
#endif
   }
}

void HifiMoveList::serverWriteMovePacket(BitStream * bstream)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("ack %i minus %i",mLastMoveAck,mMoveVec.size());
#endif

   // acknowledge only those moves that have been ticked
   bstream->writeInt(mLastMoveAck - mMoveVec.size(),32);

   // send over the current tick count on the server...
   bstream->writeInt(ServerProcessList::get()->getTotalTicks() & TotalTicksMask, TotalTicksBits);
}

void HifiMoveList::clientReadMovePacket(BitStream * bstream)
{
   if (!serverTicksInitialized())
      resetMoveList();

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("pre move ack: %i", mLastMoveAck);
#endif

   mLastMoveAck = bstream->readInt(32);

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("post move ack %i, first move %i, last move %i", mLastMoveAck, mFirstMoveIndex, mLastClientMove);
#endif

   // This is how many times we've ticked since last ack -- before adjustments below
   S32 ourTicks = mLastMoveAck - mFirstMoveIndex;

   if (mLastMoveAck < mFirstMoveIndex)
      mLastMoveAck = mFirstMoveIndex;

   if(mLastMoveAck > mLastClientMove)
   {
      ourTicks -= mLastMoveAck-mLastClientMove;
      mLastClientMove = mLastMoveAck;
   }
   while(mFirstMoveIndex < mLastMoveAck)
   {
      if (mMoveVec.size())
      {
         mMoveVec.pop_front();
         mFirstMoveIndex++;
      }
      else
      {
         AssertWarn(1, "Popping off too many moves!");
         mFirstMoveIndex = mLastMoveAck;
      }
   }

   // get server ticks using total number of ticks on server to date...
   U32 serverTickNum = bstream->readInt(TotalTicksBits);
   S32 serverTicks = getServerTicks(serverTickNum);
   S32 tickDiff = serverTicks - ourTicks;

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("server ticks: %i, client ticks: %i, diff: %i%s", serverTicks, ourTicks, tickDiff, !tickDiff ? "" : " (ticks mis-match)");
#endif


   // Apply the first (of two) client-side synchronization mechanisms.  Key is that
   // we need to both synchronize client/server move streams (so first move in list is made
   // at same "time" on both client and server) and maintain the "time" at which the most
   // recent move was made on the server.  In both cases, "time" is the number of ticks
   // it took to get to that move.
   updateClientServerTickDiff(tickDiff);

   // Apply the second (and final) client-side synchronization mechanism.  The tickDiff adjustments above 
   // make sure time is preserved on client.  But that assumes that a future (or previous) update will adjust
   // time in the other direction, so that we don't get too far behind or ahead of the server.  The updateMoveSync
   // mechanism tracks us over time to make sure we eventually return to be in sync, and makes adjustments
   // if we don't after a certain time period (number of updates).  Unlike the tickDiff mechanism, when
   // the updateMoveSync acts time is not preserved on the client.
   HifiClientProcessList * processList = dynamic_cast<HifiClientProcessList*>(ClientProcessList::get());
   if (processList)
   {
      processList->updateMoveSync(mLastSentMove-mLastClientMove);

      // set catchup parameters...
      U32 totalCatchup = mLastClientMove - mFirstMoveIndex;

      processList->ageTickCache(ourTicks + (tickDiff>0 ? tickDiff : 0), totalCatchup+1);
      processList->forceHifiReset(tickDiff!=0);
      processList->setCatchup(totalCatchup);
   }
}

void HifiMoveList::ghostPreRead(NetObject * nobj, bool newGhost)
{
   GameBase* obj = dynamic_cast<GameBase*>(nobj);
   if ( obj && ( obj->getTypeMask() & GameBaseHiFiObjectType ) && !newGhost )
   {
      // set next cache entry to start
      obj->getTickCache().beginCacheList();

      // reset to old state because we are about to unpack (and then tick forward)
      TickCacheEntry * tce = obj->getTickCache().incCacheList(false);
      if (tce)
      {
         BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
         obj->readPacketData(mConnection, &bs);
      }
   }
}

void HifiMoveList::ghostReadExtra(NetObject * nobj, BitStream * bstream, bool newGhost)
{
   // Receive additional per ghost information.
   // Get pending moves for ghosts that have them and add the moves to
   // the tick cache.
   GameBase* obj = dynamic_cast<GameBase*>(nobj);
   if ( obj && ( obj->getTypeMask() & GameBaseHiFiObjectType ) )
   {
      // mark ghost so that it updates correctly
      obj->setGhostUpdated(true);
      obj->setNewGhost(newGhost);

      // set next cache entry to start
      obj->getTickCache().beginCacheList();

      // save state for future update
      TickCacheEntry * tce = obj->getTickCache().incCacheList();
      BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
      obj->writePacketData(mConnection, &bs);
   }
}
