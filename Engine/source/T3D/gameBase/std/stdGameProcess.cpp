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
#include "T3D/gameBase/std/stdGameProcess.h"

#include "platform/profiler.h"
#include "console/consoleTypes.h"
#include "core/dnet.h"
#include "core/stream/bitStream.h"
#include "core/frameAllocator.h"
#include "core/util/refBase.h"
#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mathUtils.h"
#include "T3D/gameBase/gameBase.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameBase/std/stdMoveList.h"
#include "T3D/fx/cameraFXMgr.h"

#ifdef TORQUE_EXPERIMENTAL_EC
#include "T3D/components/coreInterfaces.h"
#include "T3D/components/component.h"
#endif

MODULE_BEGIN( ProcessList )

   MODULE_INIT
   {
      StdServerProcessList::init();
      StdClientProcessList::init();
   }

   MODULE_SHUTDOWN
   {
      StdServerProcessList::shutdown();
      StdClientProcessList::shutdown();
   }

MODULE_END;

void StdServerProcessList::init()
{
   smServerProcessList = new StdServerProcessList();
}

void StdServerProcessList::shutdown()
{
   delete smServerProcessList;
}

void StdClientProcessList::init()
{
   smClientProcessList = new StdClientProcessList();
}

void StdClientProcessList::shutdown()
{
   delete smClientProcessList;
}

//----------------------------------------------------------------------------


namespace
{
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
} // namespace

//--------------------------------------------------------------------------
// ClientProcessList
//--------------------------------------------------------------------------

StdClientProcessList::StdClientProcessList()
{
}

bool StdClientProcessList::advanceTime( SimTime timeDelta )
{
   PROFILE_SCOPE( StdClientProcessList_AdvanceTime );

   if ( doBacklogged( timeDelta ) )
      return false;
 
   bool ret = Parent::advanceTime( timeDelta );
   ProcessObject *obj = NULL;

   AssertFatal( mLastDelta >= 0.0f && mLastDelta <= 1.0f, "mLastDelta is not zero to one.");
   
   obj = mHead.mProcessLink.next;
   while ( obj != &mHead )
   {      
      if ( obj->isTicking() )
         obj->interpolateTick( mLastDelta );

      obj = obj->mProcessLink.next;
   }

#ifdef TORQUE_EXPERIMENTAL_EC
   for (U32 i = 0; i < UpdateInterface::all.size(); i++)
   {
      Component *comp = dynamic_cast<Component*>(UpdateInterface::all[i]);

      if (!comp->isClientObject() || !comp->isActive())
            continue;

      UpdateInterface::all[i]->interpolateTick(mLastDelta);
   }
#endif

   // Inform objects of total elapsed delta so they can advance
   // client side animations.
   F32 dt = F32(timeDelta) / 1000;

   // Update camera FX.
   gCamFXMgr.update( dt );

   obj = mHead.mProcessLink.next;
   while ( obj != &mHead )
   {      
      obj->advanceTime( dt );
      obj = obj->mProcessLink.next;
   }
   
#ifdef TORQUE_EXPERIMENTAL_EC
   for (U32 i = 0; i < UpdateInterface::all.size(); i++)
   {
      Component *comp = dynamic_cast<Component*>(UpdateInterface::all[i]);

      if (comp)
      {
         if (!comp->isClientObject() || !comp->isActive())
            continue;
      }

      UpdateInterface::all[i]->advanceTime(dt);
   }
#endif

   return ret;
}

//----------------------------------------------------------------------------
void StdClientProcessList::onAdvanceObjects()
{
   PROFILE_SCOPE( StdClientProcessList_OnAdvanceObjects );

   GameConnection* connection = GameConnection::getConnectionToServer();
   if ( connection )
   {
      // process any demo blocks that are NOT moves, and exactly one move
      // we advance time in the demo stream by a move inserted on
      // each tick.  So before doing the tick processing we advance
      // the demo stream until a move is ready
      if ( connection->isPlayingBack() )
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
         while ( blockType != GameConnection::BlockTypeMove );
      }

      connection->mMoveList->collectMove();
      advanceObjects();
   }
   else
      advanceObjects();
}

void StdClientProcessList::onTickObject( ProcessObject *obj )
{
   PROFILE_SCOPE( StdClientProcessList_OnTickObject );

   // In case the object deletes itself during its processTick.
   SimObjectPtr<SceneObject> safePtr = static_cast<SceneObject*>( obj );   

   // Each object is either advanced a single tick, or if it's
   // being controlled by a client, ticked once for each pending move.
   Move* movePtr;
   U32 numMoves;
   GameConnection* con = obj->getControllingClient();
   if  ( con && con->getControlObject() == obj )
   {
      con->mMoveList->getMoves( &movePtr, &numMoves );
      if ( numMoves )
      {
         // Note: should only have a single move at this point
         AssertFatal(numMoves==1,"ClientProccessList::onTickObject: more than one move in queue");

         #ifdef TORQUE_DEBUG_NET_MOVES
         U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum(obj->getControllingClient());
         #endif

         if ( obj->isTicking() )
            obj->processTick( movePtr );

         if ( bool(safePtr) && obj->getControllingClient() )
         {
            U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

            // set checksum if not set or check against stored value if set
            movePtr->checksum = newsum;

            #ifdef TORQUE_DEBUG_NET_MOVES
            Con::printf("move checksum: %i, (start %i), (move %f %f %f)",
               movePtr->checksum,sum,movePtr->yaw,movePtr->y,movePtr->z);
            #endif
         }
         con->mMoveList->clearMoves( 1 );
      }
   }
   else if ( obj->isTicking() )
      obj->processTick( 0 );
}

void StdClientProcessList::advanceObjects()
{
   PROFILE_SCOPE( StdClientProcessList_AdvanceObjects );

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("Advance client time...");
   #endif

   Parent::advanceObjects();

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
   #endif
}

void StdClientProcessList::clientCatchup( GameConnection *  connection )
{
   SimObjectPtr<GameBase> control = connection->getControlObject();
   if ( control )
   {
      Move * movePtr;
      U32 numMoves;
      U32 m = 0;
      connection->mMoveList->getMoves( &movePtr, &numMoves );

      #ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("client catching up... (%i)", numMoves);
      #endif

      preTickSignal().trigger();

      if ( control->isTicking() )
         for ( m = 0; m < numMoves; m++ )
            control->processTick( movePtr++ );

      connection->mMoveList->clearMoves( m );
   }

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
   #endif
}

//--------------------------------------------------------------------------
// ServerProcessList
//--------------------------------------------------------------------------
   
StdServerProcessList::StdServerProcessList()
{
}

void StdServerProcessList::onPreTickObject( ProcessObject *pobj )
{
   if ( pobj->mIsGameBase )
   {
      SimObjectPtr<GameBase> obj = getGameBase( pobj );

      // Each object is either advanced a single tick, or if it's
      // being controlled by a client, ticked once for each pending move.
      GameConnection *con = obj->getControllingClient();

      if ( con && con->getControlObject() == obj )
      {
         Move* movePtr;
         U32 numMoves;
         con->mMoveList->getMoves( &movePtr, &numMoves );

         if ( numMoves == 0 )
         {
   #ifdef TORQUE_DEBUG_NET_MOVES
            Con::printf("no moves on object %i, skip tick",obj->getId());
   #endif         
            return;
         }
      }
   }

   Parent::onPreTickObject (pobj );
}

void StdServerProcessList::onTickObject( ProcessObject *pobj )
{
   PROFILE_SCOPE( StdServerProcessList_OnTickObject );
   
   // Each object is either advanced a single tick, or if it's
   // being controlled by a client, ticked once for each pending move.

   GameConnection *con = pobj->getControllingClient();

   if ( pobj->mIsGameBase && con && con->getControlObject() == pobj )
   {
      // In case the object is deleted during its own tick.
      SimObjectPtr<GameBase> obj = getGameBase( pobj );

      Move* movePtr;
      U32 m, numMoves;
      con->mMoveList->getMoves( &movePtr, &numMoves );

      // For debugging it can be useful to know when this happens.
      //if ( numMoves > 1 )
      //   Con::printf( "numMoves: %i", numMoves );

      // Do we really need to test the control object each iteration? Does it change?
      for ( m = 0; m < numMoves && con && con->getControlObject() == obj; m++, movePtr++ )
      {         
         #ifdef TORQUE_DEBUG_NET_MOVES
         U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum(obj->getControllingClient());
         #endif
      
         if ( obj->isTicking() )
            obj->processTick( movePtr );

         if ( con && con->getControlObject() == obj )
         {
            U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

            // check move checksum
            if ( movePtr->checksum != newsum )
            {
               #ifdef TORQUE_DEBUG_NET_MOVES
               if( !obj->isAIControlled() )
                  Con::printf("move %i checksum disagree: %i != %i, (start %i), (move %f %f %f)",
                     movePtr->id, movePtr->checksum,newsum,sum,movePtr->yaw,movePtr->y,movePtr->z);
               #endif

               movePtr->checksum = Move::ChecksumMismatch;
            }
            else
            {
               #ifdef TORQUE_DEBUG_NET_MOVES
               Con::printf("move %i checksum agree: %i == %i, (start %i), (move %f %f %f)",
                  movePtr->id, movePtr->checksum,newsum,sum,movePtr->yaw,movePtr->y,movePtr->z);
               #endif
            }
         }
      }

      con->mMoveList->clearMoves( m );
   }
   else if ( pobj->isTicking() )
      pobj->processTick( 0 );
}

void StdServerProcessList::advanceObjects()
{
   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("Advance server time...");
   #endif

   Parent::advanceObjects();

   // Credit all connections with the elapsed tick
   SimGroup *clientGroup = Sim::getClientGroup();
   for(SimGroup::iterator i = clientGroup->begin(); i != clientGroup->end(); i++)
   {
      if (GameConnection *con = dynamic_cast<GameConnection *>(*i))
      {
         con->mMoveList->advanceMove();
      }
   }

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
   #endif
}



