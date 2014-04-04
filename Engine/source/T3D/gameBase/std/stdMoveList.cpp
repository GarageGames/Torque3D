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
#include "T3D/gameBase/std/stdMoveList.h"

#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"

#define MAX_MOVE_PACKET_SENDS 4


StdMoveList::StdMoveList()
{
   mMoveCredit = MaxMoveCount;
}

U32 StdMoveList::getMoves(Move** movePtr,U32* numMoves)
{
   if (!mConnection->isConnectionToServer())
   {
      if (mMoveVec.size() > mMoveCredit)
         mMoveVec.setSize(mMoveCredit);
   }
   return Parent::getMoves(movePtr,numMoves);
}

void StdMoveList::clearMoves(U32 count)
{
   if (!mConnection->isConnectionToServer())
   {
      count = mClamp(count,0,mMoveCredit);
      mMoveCredit -= count;
   }

   Parent::clearMoves(count);
}

void StdMoveList::advanceMove()
{
   AssertFatal(!mConnection->isConnectionToServer(), "Cannot inc move credit on the client.");

   // Game tick increment
   mMoveCredit++;
   if (mMoveCredit > MaxMoveCount)
      mMoveCredit = MaxMoveCount;

   // Clear pending moves for the elapsed time if there
   // is no control object.
   if ( mConnection->getControlObject() == NULL )
      mMoveVec.clear();
}

void StdMoveList::clientWriteMovePacket(BitStream *bstream)
{
   AssertFatal(mLastMoveAck == mFirstMoveIndex, "Invalid move index.");
   U32 count = mMoveVec.size();

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
   for (S32 i = 0; i < count; i++)
   {
      move[offset + i].sendCount++;
      move[offset + i].pack(bstream,prevMove);
      bstream->writeInt(move[offset + i].checksum & (~(0xFFFFFFFF << Move::ChecksumBits)),Move::ChecksumBits);
      prevMove = &move[offset+i];
   }
}

void StdMoveList::serverReadMovePacket(BitStream *bstream)
{
   // Server side packet read.
   U32 start = bstream->readInt(32);
   U32 count = bstream->readInt(MoveCountBits);

   Move * prevMove = NULL;
   Move prevMoveHolder;

   // Skip forward (must be starting up), or over the moves
   // we already have.
   S32 skip = mLastMoveAck - start;
   if (skip < 0) 
   {
      mLastMoveAck = start;
   }
   else 
   {
      if (skip > count)
         skip = count;
      for (S32 i = 0; i < skip; i++)
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
   S32 index = mMoveVec.size();
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
}

void StdMoveList::serverWriteMovePacket(BitStream * bstream)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("ack %i minus %i",mLastMoveAck,mMoveVec.size());
#endif

   // acknowledge only those moves that have been ticked
   bstream->writeInt(mLastMoveAck - mMoveVec.size(),32);
}

void StdMoveList::clientReadMovePacket(BitStream * bstream)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("pre move ack: %i", mLastMoveAck);
#endif

   mLastMoveAck = bstream->readInt(32);

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("post move ack %i, first move %i, last move %i", mLastMoveAck, mFirstMoveIndex, mLastClientMove);
#endif

   if (mLastMoveAck < mFirstMoveIndex)
      mLastMoveAck = mFirstMoveIndex;

   if(mLastMoveAck > mLastClientMove)
      mLastClientMove = mLastMoveAck;
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
}
