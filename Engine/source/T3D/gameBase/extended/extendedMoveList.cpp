#include "platform/platform.h"
#include "T3D/gameBase/extended/extendedMoveList.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"

#define MAX_MOVE_PACKET_SENDS 4

ExtendedMoveList::ExtendedMoveList()
{
   mMoveCredit = MaxMoveCount;
   mControlMismatch = false;
   reset();
}

void ExtendedMoveList::reset()
{
   mLastMoveAck = 0;
   mLastClientMove = 0;
   mFirstMoveIndex = 0;

   mExtMoveVec.clear();
}

bool ExtendedMoveList::getNextExtMove( ExtendedMove &curMove )
{
   if ( mExtMoveVec.size() > MaxMoveQueueSize )
      return false;

   // From MoveList
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

   for(U32 i=0; i<ExtendedMove::MaxPositionsRotations; ++i)
   {
      // Process position
      curMove.posX[i] = ExtendedMoveManager::mPosX[i];
      curMove.posY[i] = ExtendedMoveManager::mPosY[i];
      curMove.posZ[i] = ExtendedMoveManager::mPosZ[i];

      // Process rotation.  There are two possible forms of rotation: Angle Axis and Euler angles.
      curMove.EulerBasedRotation[i] = ExtendedMoveManager::mRotIsEuler[i];
      if(curMove.EulerBasedRotation[i])
      {
         // Euler angle based rotation passed in as degrees.  We only need to work with three components.
         curMove.rotX[i] = mDegToRad(ExtendedMoveManager::mRotAX[i]);
         curMove.rotY[i] = mDegToRad(ExtendedMoveManager::mRotAY[i]);
         curMove.rotZ[i] = mDegToRad(ExtendedMoveManager::mRotAZ[i]);
      }
      else
      {
         //Rotation is passed in as an Angle Axis in degrees.  We need to convert this into a Quat.
         QuatF q(Point3F(ExtendedMoveManager::mRotAX[i], ExtendedMoveManager::mRotAY[i], ExtendedMoveManager::mRotAZ[i]), mDegToRad(ExtendedMoveManager::mRotAA[i]));
         curMove.rotX[i] = q.x;
         curMove.rotY[i] = q.y;
         curMove.rotZ[i] = q.z;
         curMove.rotW[i] = q.w;
      }
   }

   if (mConnection->getControlObject())
      mConnection->getControlObject()->preprocessMove(&curMove);

   curMove.clamp();  // clamp for net traffic
   return true;
}

U32 ExtendedMoveList::getMoves(Move** movePtr,U32* numMoves)
{
   // We don't want to be here
   AssertFatal(0, "getMoves() called");

   numMoves = 0;

   return 0;
}

U32 ExtendedMoveList::getExtMoves( ExtendedMove** movePtr, U32 *numMoves )
{
   if (!mConnection->isConnectionToServer())
   {
      if (mExtMoveVec.size() > mMoveCredit)
         mExtMoveVec.setSize(mMoveCredit);
   }

   // From MoveList but converted to use mExtMoveVec
   if (mConnection->isConnectionToServer())
   {
      // give back moves starting at the last client move...

      AssertFatal(mLastClientMove >= mFirstMoveIndex, "Bad move request");
      AssertFatal(mLastClientMove - mFirstMoveIndex <= mExtMoveVec.size(), "Desynched first and last move.");
      *numMoves = mExtMoveVec.size() - mLastClientMove + mFirstMoveIndex;
      *movePtr = mExtMoveVec.address() + mLastClientMove - mFirstMoveIndex;
   }
   else
   {
      // return the full list
      *numMoves = mExtMoveVec.size();
      *movePtr = mExtMoveVec.begin();
   }

   return *numMoves;
}

void ExtendedMoveList::collectMove()
{
   ExtendedMove mv;   
   if (mConnection)
   {
      if(!mConnection->isPlayingBack() && getNextExtMove(mv))
      {
         mv.checksum=Move::ChecksumMismatch;
         pushMove(mv);
         mConnection->recordBlock(GameConnection::BlockTypeMove, sizeof(ExtendedMove), &mv);
      }
   }
   else
   {
      if(getNextExtMove(mv))
      {
         mv.checksum=Move::ChecksumMismatch;
         pushMove(mv);
      }
   }
}

void ExtendedMoveList::pushMove(const Move &mv)
{
   const ExtendedMove* extMove = dynamic_cast<const ExtendedMove*>(&mv);
   AssertFatal(extMove, "Regular Move struct passed to pushMove()");

   pushExtMove(*extMove);
}

void ExtendedMoveList::pushExtMove( const ExtendedMove &mv )
{
   U32 id = mFirstMoveIndex + mExtMoveVec.size();
   U32 sz = mExtMoveVec.size();
   mExtMoveVec.push_back(mv);
   mExtMoveVec[sz].id = id;
   mExtMoveVec[sz].sendCount = 0;
}

void ExtendedMoveList::clearMoves(U32 count)
{
   if (!mConnection->isConnectionToServer())
   {
      count = mClamp(count,0,mMoveCredit);
      mMoveCredit -= count;
   }

   // From MoveList but converted to use mExtMoveVec
   if (mConnection->isConnectionToServer())
   {
      mLastClientMove += count;
      AssertFatal(mLastClientMove >= mFirstMoveIndex, "Bad move request");
      AssertFatal(mLastClientMove - mFirstMoveIndex <= mExtMoveVec.size(), "Desynched first and last move.");
      if (!mConnection)
         // drop right away if no connection
         ackMoves(count);
   }
   else 
   {
      AssertFatal(count <= mExtMoveVec.size(),"GameConnection: Clearing too many moves");
      for (S32 i=0; i<count; i++)
         if (mExtMoveVec[i].checksum == Move::ChecksumMismatch)
            mControlMismatch = true;
         else 
            mControlMismatch = false;
      if (count == mExtMoveVec.size())
         mExtMoveVec.clear();
      else
         while (count--)
            mExtMoveVec.pop_front();
   }
}

void ExtendedMoveList::advanceMove()
{
   AssertFatal(!mConnection->isConnectionToServer(), "Cannot inc move credit on the client.");

   // Game tick increment
   mMoveCredit++;
   if (mMoveCredit > MaxMoveCount)
      mMoveCredit = MaxMoveCount;

   // Clear pending moves for the elapsed time if there
   // is no control object.
   if ( mConnection->getControlObject() == NULL )
      mExtMoveVec.clear();
}

void ExtendedMoveList::clientWriteMovePacket(BitStream *bstream)
{
   AssertFatal(mLastMoveAck == mFirstMoveIndex, "Invalid move index.");
   U32 count = mExtMoveVec.size();

   ExtendedMove* extMove = mExtMoveVec.address();
   U32 start = mLastMoveAck;
   U32 offset;
   for(offset = 0; offset < count; offset++)
      if(extMove[offset].sendCount < MAX_MOVE_PACKET_SENDS)
         break;
   if(offset == count && count != 0)
      offset--;

   start += offset;
   count -= offset;

   if (count > MaxMoveCount)
      count = MaxMoveCount;
   bstream->writeInt(start,32);
   bstream->writeInt(count,MoveCountBits);
   ExtendedMove* prevExtMove = NULL;
   for (int i = 0; i < count; i++)
   {
      extMove[offset + i].sendCount++;
      extMove[offset + i].pack(bstream,prevExtMove);
      bstream->writeInt(extMove[offset + i].checksum & (~(0xFFFFFFFF << Move::ChecksumBits)),Move::ChecksumBits);
      prevExtMove = &extMove[offset+i];
   }
}

void ExtendedMoveList::serverReadMovePacket(BitStream *bstream)
{
   // Server side packet read.
   U32 start = bstream->readInt(32);
   U32 count = bstream->readInt(MoveCountBits);

   ExtendedMove* prevExtMove = NULL;
   ExtendedMove prevExtMoveHolder;

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
         prevExtMoveHolder.unpack(bstream,prevExtMove);
         prevExtMoveHolder.checksum = bstream->readInt(Move::ChecksumBits);
         prevExtMove = &prevExtMoveHolder;
         S32 idx = mExtMoveVec.size()-skip+i;
         if (idx>=0)
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            if (mExtMoveVec[idx].checksum != prevExtMoveHolder.checksum)
               Con::printf("updated checksum on move %i from %i to %i",mExtMoveVec[idx].id,mExtMoveVec[idx].checksum,prevExtMoveHolder.checksum);
#endif
            mExtMoveVec[idx].checksum = prevExtMoveHolder.checksum;
         }
      }
      start += skip;
      count = count - skip;
   }

   // Put the rest on the move list.
   int index = mExtMoveVec.size();
   mExtMoveVec.increment(count);
   while (index < mExtMoveVec.size())
   {
      mExtMoveVec[index].unpack(bstream,prevExtMove);
      mExtMoveVec[index].checksum = bstream->readInt(Move::ChecksumBits);
      prevExtMove = &mExtMoveVec[index];
      mExtMoveVec[index].id = start++;
      index ++;
   }

   mLastMoveAck += count;
}

void ExtendedMoveList::serverWriteMovePacket(BitStream * bstream)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("ack %i minus %i",mLastMoveAck,mExtMoveVec.size());
#endif

   // acknowledge only those moves that have been ticked
   bstream->writeInt(mLastMoveAck - mExtMoveVec.size(),32);
}

void ExtendedMoveList::clientReadMovePacket(BitStream * bstream)
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
      if (mExtMoveVec.size())
      {
         mExtMoveVec.pop_front();
         mFirstMoveIndex++;
      }
      else
      {
         AssertWarn(1, "Popping off too many moves!");
         mFirstMoveIndex = mLastMoveAck;
      }
   }
}

bool ExtendedMoveList::isBacklogged()
{
   if ( !mConnection->isConnectionToServer() )
      return false;

   return mLastClientMove - mFirstMoveIndex == mExtMoveVec.size() && 
          mExtMoveVec.size() >= MaxMoveCount;
}

bool ExtendedMoveList::areMovesPending()
{
   return mConnection->isConnectionToServer() ?
          mExtMoveVec.size() - mLastClientMove + mFirstMoveIndex :
          mExtMoveVec.size();
}

void ExtendedMoveList::ackMoves(U32 count)
{
   mLastMoveAck += count;
   if(mLastMoveAck > mLastClientMove)
      mLastClientMove = mLastMoveAck;
   while(mFirstMoveIndex < mLastMoveAck)
   {
      if (mExtMoveVec.size())
      {
         mExtMoveVec.pop_front();
         mFirstMoveIndex++;
      }
      else
      {
         AssertWarn(1, "Popping off too many moves!");
         mFirstMoveIndex = mLastMoveAck;
      }
   }
}
