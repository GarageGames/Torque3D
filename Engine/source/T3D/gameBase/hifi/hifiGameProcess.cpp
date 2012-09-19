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
#include "T3D/gameBase/hifi/hifiGameProcess.h"

#include "platform/profiler.h"
#include "core/frameAllocator.h"
#include "core/stream/bitStream.h"
#include "math/mathUtils.h"
#include "T3D/gameBase/hifi/hifiMoveList.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameFunctions.h"


MODULE_BEGIN( ProcessList )

   MODULE_INIT
   {
      HifiServerProcessList::init();
      HifiClientProcessList::init();
   }

   MODULE_SHUTDOWN
   {
      HifiServerProcessList::shutdown();
      HifiClientProcessList::shutdown();
   }

MODULE_END;

void HifiServerProcessList::init()
{
   smServerProcessList = new HifiServerProcessList();
}

void HifiServerProcessList::shutdown()
{
   delete smServerProcessList;
}

void HifiClientProcessList::init()
{
   smClientProcessList = new HifiClientProcessList();
}

void HifiClientProcessList::shutdown()
{
   delete smClientProcessList;
}

//----------------------------------------------------------------------------

F32 gMaxHiFiVelSq = 100 * 100;

namespace
{
   inline GameBase * GetGameBase(ProcessObject * obj)
   {
      return static_cast<GameBase*>(obj);
   }

   // local work class
   struct GameBaseListNode
   {
      GameBaseListNode()
      {
         mPrev=this;
         mNext=this;
         mObject=NULL;
      }

      GameBaseListNode * mPrev;
      GameBaseListNode * mNext;
      GameBase * mObject;

      void linkBefore(GameBaseListNode * obj)
      {
         // Link this before obj
         mNext = obj;
         mPrev = obj->mPrev;
         obj->mPrev = this;
         mPrev->mNext = this;
      }
   };

   // Structure used for synchronizing move lists on client/server
   struct MoveSync
   {
      enum { ActionCount = 4 };

      S32 moveDiff;
      S32 moveDiffSteadyCount;
      S32 moveDiffSameSignCount;

      bool doAction() { return moveDiffSteadyCount>=ActionCount || moveDiffSameSignCount>=4*ActionCount; }
      void reset() { moveDiff=0; moveDiffSteadyCount=0; moveDiffSameSignCount=0; }
      void update(S32 diff);
   } moveSync;

   void MoveSync::update(S32 diff)
   {
      if (diff && diff==moveDiff)
      {
         moveDiffSteadyCount++;
         moveDiffSameSignCount++;
      }
      else if (diff*moveDiff>0)
      {
         moveDiffSteadyCount = 0;
         moveDiffSameSignCount++;
      }
      else
         reset();
      moveDiff = diff;
   }

} // namespace

//--------------------------------------------------------------------------
// HifiClientProcessList
//--------------------------------------------------------------------------

HifiClientProcessList::HifiClientProcessList()
{
   mSkipAdvanceObjectsMs = 0;
   mForceHifiReset = false;
   mCatchup = 0;
}

bool HifiClientProcessList::advanceTime( SimTime timeDelta )
{
   PROFILE_SCOPE( AdvanceClientTime );

   if ( mSkipAdvanceObjectsMs && timeDelta > mSkipAdvanceObjectsMs )
   {
      timeDelta -= mSkipAdvanceObjectsMs;
      advanceTime( mSkipAdvanceObjectsMs );
      AssertFatal( !mSkipAdvanceObjectsMs, "mSkipAdvanceObjectsMs must always be positive." );
   }

   if ( doBacklogged( timeDelta ) )
      return false;

   // remember interpolation value because we might need to set it back
   F32 oldLastDelta = mLastDelta;

   bool ret = Parent::advanceTime( timeDelta );

   if ( !mSkipAdvanceObjectsMs )
   {
      AssertFatal( mLastDelta >= 0.0f && mLastDelta <= 1.0f, "mLastDelta must always be zero to one." );
      for ( ProcessObject *pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next )
      {                
         if ( pobj->isTicking() )
            pobj->interpolateTick( mLastDelta );
      }

      // Inform objects of total elapsed delta so they can advance
      // client side animations.
      F32 dt = F32( timeDelta ) / 1000;
      for ( ProcessObject *pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next)
      {                  
         pobj->advanceTime( dt );
      }
   }
   else
   {
      mSkipAdvanceObjectsMs -= timeDelta;
      mLastDelta = oldLastDelta;
   }

   return ret;
}

void HifiClientProcessList::onAdvanceObjects()
{
   GameConnection* connection = GameConnection::getConnectionToServer();
   if(connection)
   {
      // process any demo blocks that are NOT moves, and exactly one move
      // we advance time in the demo stream by a move inserted on
      // each tick.  So before doing the tick processing we advance
      // the demo stream until a move is ready
      if(connection->isPlayingBack())
      {
         U32 blockType;
         do
         {
            blockType = connection->getNextBlockType();
            bool res = connection->processNextBlock();
            // if there are no more blocks, exit out of this function,
            // as no more client time needs to process right now - we'll
            // get it all on the next advanceClientTime()
            if(!res)
               return;
         }
         while(blockType != GameConnection::BlockTypeMove);
      }
      if (!mSkipAdvanceObjectsMs)
      {
         connection->mMoveList->collectMove();
         advanceObjects();
      }
      connection->mMoveList->onAdvanceObjects();
   }
}

void HifiClientProcessList::onTickObject(ProcessObject * pobj)
{
   // Each object is advanced a single tick
   // If it's controlled by a client, tick using a move.   

   Move *movePtr;
   U32 numMoves;
   GameConnection *con = pobj->getControllingClient();
   SimObjectPtr<GameBase> obj = getGameBase( pobj );

   if ( obj && con && con->getControlObject() == obj && con->mMoveList->getMoves( &movePtr, &numMoves) )
   {
#ifdef TORQUE_DEBUG_NET_MOVES
      U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );
#endif

      obj->processTick( movePtr );

      if ( bool(obj) && obj->getControllingClient() )
      {
         U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

         // set checksum if not set or check against stored value if set
         movePtr->checksum = newsum;

#ifdef TORQUE_DEBUG_NET_MOVES
         Con::printf( "move checksum: %i, (start %i), (move %f %f %f)",
            movePtr->checksum,sum,movePtr->yaw,movePtr->y,movePtr->z );
#endif
      }
      con->mMoveList->clearMoves( 1 );
   }
   else if ( pobj->isTicking() )
      pobj->processTick( 0 );
   
   if ( obj && ( obj->getTypeMask() & GameBaseHiFiObjectType ) )
   {
      GameConnection * serverConnection = GameConnection::getConnectionToServer();
      TickCacheEntry * tce = obj->getTickCache().addCacheEntry();
      BitStream bs( tce->packetData, TickCacheEntry::MaxPacketSize );
      obj->writePacketData( serverConnection, &bs );

      Point3F vel = obj->getVelocity();
      F32 velSq = mDot( vel, vel );
      gMaxHiFiVelSq = getMax( gMaxHiFiVelSq, velSq );
   }
}

void HifiClientProcessList::advanceObjects()
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("Advance client time...");
#endif

   // client re-computes this each time objects are advanced
   gMaxHiFiVelSq = 0;
   Parent::advanceObjects();

   // We need to consume a move on the connections whether 
   // there is a control object to consume the move or not,
   // otherwise client and server can get out of sync move-wise
   // during startup.  If there is a control object, we cleared
   // a move above.  Handle case where no control object here.
   // Note that we might consume an extra move here and there when
   // we had a control object in above loop but lost it during tick.
   // That is no big deal so we don't bother trying to carefully
   // track it.
   GameConnection * client = GameConnection::getConnectionToServer();
   if (client && client->getControlObject() == NULL)
      client->mMoveList->clearMoves(1);

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
#endif
}

void HifiClientProcessList::ageTickCache(S32 numToAge, S32 len)
{
   for (ProcessObject * pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next)
   {
      GameBase *obj = getGameBase(pobj);
      if ( obj && obj->getTypeMask() & GameBaseHiFiObjectType )
         obj->getTickCache().ageCache(numToAge,len);
   }
}

void HifiClientProcessList::updateMoveSync(S32 moveDiff)
{
   moveSync.update(moveDiff);
   if (moveSync.doAction() && moveDiff<0)
   {
      skipAdvanceObjects(TickMs * -moveDiff);
      moveSync.reset();
   }
}

void HifiClientProcessList::clientCatchup(GameConnection * connection)
{
#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("client catching up... (%i)%s", mCatchup, mForceHifiReset ? " reset" : "");
#endif

   if (connection->getControlObject() && connection->getControlObject()->isGhostUpdated())
      // if control object is reset, make sure moves are reset too
      connection->mMoveList->resetCatchup();

   const F32 maxVel = mSqrt(gMaxHiFiVelSq) * 1.25f;
   F32 dt = F32(mCatchup+1) * TickSec;
   Point3F bigDelta(maxVel*dt,maxVel*dt,maxVel*dt);

   // walk through all process objects looking for ones which were updated
   // -- during first pass merely collect neighbors which need to be reset and updated in unison
   ProcessObject * pobj;
   if (mCatchup && !mForceHifiReset)
   {
      for (pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next)
      {
         GameBase *obj = getGameBase( pobj );
         static SimpleQueryList nearby;
         nearby.mList.clear();
         // check for nearby objects which need to be reset and then caught up
         // note the funky loop logic -- first time through obj is us, then
         // we start iterating through nearby list (to look for objects nearby
         // the nearby objects), which is why index starts at -1
         // [objects nearby the nearby objects also get added to the nearby list]
         for (S32 i=-1; obj; obj = ++i<nearby.mList.size() ? (GameBase*)nearby.mList[i] : NULL)
         {
            if (obj->isGhostUpdated() && (obj->getTypeMask() & GameBaseHiFiObjectType) && !obj->isNetNearbyAdded())
            {
               Point3F start = obj->getWorldSphere().center;
               Point3F end = start + 1.1f * dt * obj->getVelocity();
               F32 rad = 1.5f * obj->getWorldSphere().radius;

               // find nearby items not updated but are hi fi, mark them as updated (and restore old loc)
               // check to see if added items have neighbors that need updating
               Box3F box;
               Point3F rads(rad,rad,rad);
               box.minExtents = box.maxExtents = start;
               box.minExtents -= bigDelta + rads;
               box.maxExtents += bigDelta + rads;

               // CodeReview - this is left in for MBU, but also so we can deal with the issue later.
               // add marble blast hack so hifi networking can see hidden objects
               // (since hidden is under control of hifi networking)
               //  gForceNotHidden = true;

               S32 j = nearby.mList.size();
               gClientContainer.findObjects(box, GameBaseHiFiObjectType, SimpleQueryList::insertionCallback, &nearby);

               // CodeReview - this is left in for MBU, but also so we can deal with the issue later.
               // disable above hack
               //  gForceNotHidden = false;

               // drop anyone not heading toward us or already checked
               for (; j<nearby.mList.size(); j++)
               {
                  GameBase * obj2 = (GameBase*)nearby.mList[j];
                  // if both passive, these guys don't interact with each other
                  bool passive = obj->isHifiPassive() && obj2->isHifiPassive();
                  if (!obj2->isGhostUpdated() && !passive)
                  {
                     // compare swept spheres of obj and obj2
                     // if collide, reset obj2, setGhostUpdated(true), and continue
                     Point3F end2 = obj2->getWorldSphere().center;
                     Point3F start2 = end2 - 1.1f * dt * obj2->getVelocity();
                     F32 rad2 = 1.5f * obj->getWorldSphere().radius;
                     if (MathUtils::capsuleCapsuleOverlap(start,end,rad,start2,end2,rad2))
                     {
                        // better add obj2
                        obj2->getTickCache().beginCacheList();
                        TickCacheEntry * tce = obj2->getTickCache().incCacheList();
                        BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
                        obj2->readPacketData(connection,&bs);
                        obj2->setGhostUpdated(true);

                        // continue so we later add the neighbors too
                        continue;
                     }

                  }

                  // didn't pass above test...so don't add it or nearby objects
                  nearby.mList[j] = nearby.mList.last();
                  nearby.mList.decrement();
                  j--;
               }
               obj->setNetNearbyAdded(true);
            }
         }
      }
   }

   // save water mark -- for game base list
   FrameAllocatorMarker mark;

   // build ordered list of client objects which need to be caught up
   GameBaseListNode list;
   for (pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next)
   {
      GameBase *obj = getGameBase( pobj );
      //GameBase *obj = dynamic_cast<GameBase*>( pobj );
      //GameBase *obj = (GameBase*)pobj;

      // Not a GameBase object so nothing to do.
      if ( !obj )
         continue;

      if (obj->isGhostUpdated() && (obj->getTypeMask() & GameBaseHiFiObjectType))
      {
         // construct process object and add it to the list
         // hold pointer to our object in mAfterObject
         GameBaseListNode * po = (GameBaseListNode*)FrameAllocator::alloc(sizeof(GameBaseListNode));
         po->mObject = obj;
         po->linkBefore(&list);

         // begin iterating through tick list (skip first tick since that is the state we've been reset to)
         obj->getTickCache().beginCacheList();
         obj->getTickCache().incCacheList();
      }
      else if (mForceHifiReset && (obj->getTypeMask() & GameBaseHiFiObjectType))
      {
         // add all hifi objects
         obj->getTickCache().beginCacheList();
         TickCacheEntry * tce = obj->getTickCache().incCacheList();
         BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
         obj->readPacketData(connection,&bs);
         obj->setGhostUpdated(true);

         // construct process object and add it to the list
         // hold pointer to our object in mAfterObject
         GameBaseListNode * po = (GameBaseListNode*)FrameAllocator::alloc(sizeof(GameBaseListNode));
         po->mObject = obj;
         po->linkBefore(&list);
      }
      else if (obj == connection->getControlObject() && obj->isGhostUpdated())
      {
         // construct process object and add it to the list
         // hold pointer to our object in mAfterObject
         // .. but this is not a hi fi object, so don't mess with tick cache
         GameBaseListNode * po = (GameBaseListNode*)FrameAllocator::alloc(sizeof(GameBaseListNode));
         po->mObject = obj;
         po->linkBefore(&list);
      }
      else if (obj->isGhostUpdated())
      {
         // not hifi but we were updated, so perform net smooth now
         obj->computeNetSmooth(mLastDelta);
      }

      // clear out work flags
      obj->setNetNearbyAdded(false);
      obj->setGhostUpdated(false);
   }

   // run through all the moves in the move list so we can play them with our control object
   Move* movePtr;
   U32 numMoves;
   connection->mMoveList->resetClientMoves();
   connection->mMoveList->getMoves(&movePtr, &numMoves);
   AssertFatal(mCatchup<=numMoves,"doh");

   // tick catchup time
   for (U32 m=0; m<mCatchup; m++)
   {
      for (GameBaseListNode * walk = list.mNext; walk != &list; walk = walk->mNext)
      {
         // note that we get object from after object not getGameBase function
         // this is because we are an on the fly linked list which uses mAfterObject
         // rather than the linked list embedded in GameBase (clean this up?)
         GameBase * obj = walk->mObject;

         // it's possible for a non-hifi object to get in here, but
         // only if it is a control object...make sure we don't do any
         // of the tick cache stuff if we are not hifi.
         bool hifi = obj->getTypeMask() & GameBaseHiFiObjectType;
         TickCacheEntry * tce = hifi ? obj->getTickCache().incCacheList() : NULL;

         // tick object
         if (obj==connection->getControlObject())
         {
            obj->processTick(movePtr);
            movePtr->checksum = obj->getPacketDataChecksum(connection);
            movePtr++;
         }
         else
         {
            AssertFatal(tce && hifi,"Should not get in here unless a hi fi object!!!");
            obj->processTick(tce->move);
         }

         if (hifi)
         {
            BitStream bs(tce->packetData,TickCacheEntry::MaxPacketSize);
            obj->writePacketData(connection,&bs);
         }
      }
      if (connection->getControlObject() == NULL)
         movePtr++;
   }
   connection->mMoveList->clearMoves(mCatchup);

   // Handle network error smoothing here...but only for control object
   GameBase * control = connection->getControlObject();
   if (control && !control->isNewGhost())
   {
      control->computeNetSmooth(mLastDelta);
      control->setNewGhost(false);
   }

   if (moveSync.doAction() && moveSync.moveDiff>0)
   {
      S32 moveDiff = moveSync.moveDiff;
#ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("client timewarping to catchup %i moves",moveDiff);
#endif
      while (moveDiff--)
         advanceObjects();
      moveSync.reset();
   }

#ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
#endif

   // all caught up
   mCatchup = 0;
}

//--------------------------------------------------------------------------
// HifiServerProcessList
//--------------------------------------------------------------------------

void HifiServerProcessList::onTickObject(ProcessObject * pobj)
{
   // Each object is advanced a single tick
   // If it's controlled by a client, tick using a move.
               
   Move *movePtr;
   U32 numMoves;
   GameConnection *con = pobj->getControllingClient();
   SimObjectPtr<GameBase> obj = getGameBase( pobj );

   if ( obj && con && con->getControlObject() == obj && con->mMoveList->getMoves( &movePtr, &numMoves ) )
   {
#ifdef TORQUE_DEBUG_NET_MOVES
      U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );
#endif

      obj->processTick(movePtr);

      if ( bool(obj) && obj->getControllingClient() )
      {
         U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

         // check move checksum
         if ( movePtr->checksum != newsum )
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            if ( !obj->mIsAiControlled )
               Con::printf( "move %i checksum disagree: %i != %i, (start %i), (move %f %f %f)",
               movePtr->id, movePtr->checksum, newsum, sum, movePtr->yaw, movePtr->y, movePtr->z );
#endif
            movePtr->checksum = Move::ChecksumMismatch;
         }
         else
         {
#ifdef TORQUE_DEBUG_NET_MOVES
            Con::printf( "move %i checksum agree: %i == %i, (start %i), (move %f %f %f)",
               movePtr->id, movePtr->checksum, newsum, sum, movePtr->yaw, movePtr->y, movePtr->z );
#endif
         }

         // Adding this seems to fix constant corrections, but is it
         // really a sound fix?
         con->mMoveList->clearMoves( 1 );
      }
   }
   else if ( pobj->isTicking() )
      pobj->processTick( 0 );
}
