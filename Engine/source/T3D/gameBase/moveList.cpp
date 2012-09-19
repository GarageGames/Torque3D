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

#include "T3D/gameBase/moveList.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"

MoveList::MoveList()
{
   mControlMismatch = false;
   reset();
}

void MoveList::reset()
{
   mLastMoveAck = 0;
   mLastClientMove = 0;
   mFirstMoveIndex = 0;
   mMoveVec.clear();
}

bool MoveList::getNextMove(Move &curMove)
{
   if ( mMoveVec.size() > MaxMoveQueueSize )
      return false;

   F32 pitchAdd = MoveManager::mPitchUpSpeed - MoveManager::mPitchDownSpeed;
   F32 yawAdd = MoveManager::mYawLeftSpeed - MoveManager::mYawRightSpeed;
   F32 rollAdd = MoveManager::mRollRightSpeed - MoveManager::mRollLeftSpeed;

   curMove.pitch = MoveManager::mPitch + pitchAdd;
   curMove.yaw = MoveManager::mYaw + yawAdd;
   curMove.roll = MoveManager::mRoll + rollAdd;

   MoveManager::mPitch = 0;
   MoveManager::mYaw = 0;
   MoveManager::mRoll = 0;

   curMove.x = MoveManager::mRightAction - MoveManager::mLeftAction + MoveManager::mXAxis_L;
   curMove.y = MoveManager::mForwardAction - MoveManager::mBackwardAction + MoveManager::mYAxis_L;
   curMove.z = MoveManager::mUpAction - MoveManager::mDownAction;

   curMove.freeLook = MoveManager::mFreeLook;
   curMove.deviceIsKeyboardMouse = MoveManager::mDeviceIsKeyboardMouse;

   for(U32 i = 0; i < MaxTriggerKeys; i++)
   {
      curMove.trigger[i] = false;
      if(MoveManager::mTriggerCount[i] & 1)
         curMove.trigger[i] = true;
      else if(!(MoveManager::mPrevTriggerCount[i] & 1) && MoveManager::mPrevTriggerCount[i] != MoveManager::mTriggerCount[i])
         curMove.trigger[i] = true;
      MoveManager::mPrevTriggerCount[i] = MoveManager::mTriggerCount[i];
   }

   if (mConnection->getControlObject())
      mConnection->getControlObject()->preprocessMove(&curMove);

   curMove.clamp();  // clamp for net traffic
   return true;
}

void MoveList::pushMove(const Move &mv)
{
   U32 id = mFirstMoveIndex + mMoveVec.size();
   U32 sz = mMoveVec.size();
   mMoveVec.push_back(mv);
   mMoveVec[sz].id = id;
   mMoveVec[sz].sendCount = 0;
}

U32 MoveList::getMoves(Move** movePtr,U32* numMoves)
{
   if (mConnection->isConnectionToServer())
   {
      // give back moves starting at the last client move...

      AssertFatal(mLastClientMove >= mFirstMoveIndex, "Bad move request");
      AssertFatal(mLastClientMove - mFirstMoveIndex <= mMoveVec.size(), "Desynched first and last move.");
      *numMoves = mMoveVec.size() - mLastClientMove + mFirstMoveIndex;
      *movePtr = mMoveVec.address() + mLastClientMove - mFirstMoveIndex;
   }
   else
   {
      // return the full list
      *numMoves = mMoveVec.size();
      *movePtr = mMoveVec.begin();
   }

   return *numMoves;
}

void MoveList::collectMove()
{
   Move mv;   
   if (mConnection)
   {
      if(!mConnection->isPlayingBack() && getNextMove(mv))
      {
         mv.checksum=Move::ChecksumMismatch;
         pushMove(mv);
         mConnection->recordBlock(GameConnection::BlockTypeMove, sizeof(Move), &mv);
      }
   }
   else
   {
      if(getNextMove(mv))
      {
         mv.checksum=Move::ChecksumMismatch;
         pushMove(mv);
      }
   }
}

void MoveList::clearMoves(U32 count)
{
   if (mConnection->isConnectionToServer())
   {
      mLastClientMove += count;
      AssertFatal(mLastClientMove >= mFirstMoveIndex, "Bad move request");
      AssertFatal(mLastClientMove - mFirstMoveIndex <= mMoveVec.size(), "Desynched first and last move.");
      if (!mConnection)
         // drop right away if no connection
         ackMoves(count);
   }
   else 
   {
      AssertFatal(count <= mMoveVec.size(),"GameConnection: Clearing too many moves");
      for (S32 i=0; i<count; i++)
         if (mMoveVec[i].checksum == Move::ChecksumMismatch)
            mControlMismatch = true;
         else 
            mControlMismatch = false;
      if (count == mMoveVec.size())
         mMoveVec.clear();
      else
         while (count--)
            mMoveVec.pop_front();
   }
}

bool MoveList::areMovesPending()
{
   return mConnection->isConnectionToServer() ?
          mMoveVec.size() - mLastClientMove + mFirstMoveIndex :
          mMoveVec.size();
}

bool MoveList::isBacklogged()
{
   if ( !mConnection->isConnectionToServer() )
      return false;

   return mLastClientMove - mFirstMoveIndex == mMoveVec.size() && 
          mMoveVec.size() >= MaxMoveCount;
}

void MoveList::ackMoves(U32 count)
{
   mLastMoveAck += count;
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

void MoveList::writeDemoStartBlock(ResizeBitStream *stream)
{
   stream->write(mLastMoveAck);
   stream->write(mLastClientMove);
   stream->write(mFirstMoveIndex);

   stream->write(U32(mMoveVec.size()));
   for(U32 j = 0; j < mMoveVec.size(); j++)
      mMoveVec[j].pack(stream);
}

void MoveList::readDemoStartBlock(BitStream *stream)
{
   stream->read(&mLastMoveAck);
   stream->read(&mLastClientMove);
   stream->read(&mFirstMoveIndex);

   U32 size;
   Move mv;
   stream->read(&size);
   mMoveVec.clear();
   while(size--)
   {
      mv.unpack(stream);
      pushMove(mv);
   }
}
