#include "platform/platform.h"
#include "T3D/gameBase/extended/extendedGameProcess.h"
#include "T3D/gameBase/extended/extendedMoveList.h"

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
#include "T3D/fx/cameraFXMgr.h"

MODULE_BEGIN( ProcessList )

   MODULE_INIT
   {
      ExtendedServerProcessList::init();
      ExtendedClientProcessList::init();
   }

   MODULE_SHUTDOWN
   {
      ExtendedServerProcessList::shutdown();
      ExtendedClientProcessList::shutdown();
   }

MODULE_END;

void ExtendedServerProcessList::init()
{
   smServerProcessList = new ExtendedServerProcessList();
}

void ExtendedServerProcessList::shutdown()
{
   delete smServerProcessList;
}

void ExtendedClientProcessList::init()
{
   smClientProcessList = new ExtendedClientProcessList();
}

void ExtendedClientProcessList::shutdown()
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
// ExtendedClientProcessList
//--------------------------------------------------------------------------

ExtendedClientProcessList::ExtendedClientProcessList()
{
}

bool ExtendedClientProcessList::advanceTime( SimTime timeDelta )
{
   PROFILE_SCOPE( ExtendedClientProcessList_AdvanceTime );

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
   
   return ret;
}

//----------------------------------------------------------------------------
void ExtendedClientProcessList::onAdvanceObjects()
{
   PROFILE_SCOPE( ExtendedClientProcessList_OnAdvanceObjects );

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

void ExtendedClientProcessList::onTickObject( ProcessObject *obj )
{
   PROFILE_SCOPE( ExtendedClientProcessList_OnTickObject );

   // In case the object deletes itself during its processTick.
   SimObjectPtr<SceneObject> safePtr = static_cast<SceneObject*>( obj );   

   // Each object is either advanced a single tick, or if it's
   // being controlled by a client, ticked once for each pending move.
   ExtendedMove* extMovePtr;
   U32 numMoves;
   GameConnection* con = obj->getControllingClient();
   if  ( con && con->getControlObject() == obj )
   {
      ExtendedMoveList* extMoveList = static_cast<ExtendedMoveList*>(con->mMoveList);
      extMoveList->getExtMoves( &extMovePtr, &numMoves );
      if ( numMoves )
      {
         // Note: should only have a single move at this point
         AssertFatal(numMoves==1,"ClientProccessList::onTickObject: more than one move in queue");

         #ifdef TORQUE_DEBUG_NET_MOVES
         U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum(obj->getControllingClient());
         #endif

         if ( obj->isTicking() )
            obj->processTick( extMovePtr );

         if ( bool(safePtr) && obj->getControllingClient() )
         {
            U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

            // set checksum if not set or check against stored value if set
            extMovePtr->checksum = newsum;

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

void ExtendedClientProcessList::advanceObjects()
{
   PROFILE_SCOPE( ExtendedClientProcessList_AdvanceObjects );

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("Advance client time...");
   #endif

   Parent::advanceObjects();

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
   #endif
}

void ExtendedClientProcessList::clientCatchup( GameConnection *  connection )
{
   SimObjectPtr<GameBase> control = connection->getControlObject();
   if ( control )
   {
      ExtendedMove * extMovePtr;
      U32 numMoves;
      U32 m = 0;
      ExtendedMoveList* extMoveList = static_cast<ExtendedMoveList*>(connection->mMoveList);
      extMoveList->getExtMoves( &extMovePtr, &numMoves );

      #ifdef TORQUE_DEBUG_NET_MOVES
      Con::printf("client catching up... (%i)", numMoves);
      #endif

      preTickSignal().trigger();

      if ( control->isTicking() )
         for ( m = 0; m < numMoves; m++ )
            control->processTick( extMovePtr++ );

      extMoveList->clearMoves( m );
   }

   #ifdef TORQUE_DEBUG_NET_MOVES
   Con::printf("---------");
   #endif
}

//--------------------------------------------------------------------------
// ServerProcessList
//--------------------------------------------------------------------------
   
ExtendedServerProcessList::ExtendedServerProcessList()
{
}

void ExtendedServerProcessList::onPreTickObject( ProcessObject *pobj )
{
   if ( pobj->mIsGameBase )
   {
      SimObjectPtr<GameBase> obj = getGameBase( pobj );

      // Each object is either advanced a single tick, or if it's
      // being controlled by a client, ticked once for each pending move.
      GameConnection *con = obj->getControllingClient();

      if ( con && con->getControlObject() == obj )
      {
         ExtendedMove* extMovePtr;
         U32 numMoves;
         ExtendedMoveList* extMoveList = static_cast<ExtendedMoveList*>(con->mMoveList);
         extMoveList->getExtMoves( &extMovePtr, &numMoves );

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

void ExtendedServerProcessList::onTickObject( ProcessObject *pobj )
{
   PROFILE_SCOPE( ExtendedServerProcessList_OnTickObject );
   
   // Each object is either advanced a single tick, or if it's
   // being controlled by a client, ticked once for each pending move.

   GameConnection *con = pobj->getControllingClient();

   if ( pobj->mIsGameBase && con && con->getControlObject() == pobj )
   {
      // In case the object is deleted during its own tick.
      SimObjectPtr<GameBase> obj = getGameBase( pobj );

      ExtendedMove* extMovePtr;
      U32 m, numMoves;
      ExtendedMoveList* extMoveList = static_cast<ExtendedMoveList*>(con->mMoveList);
      extMoveList->getExtMoves( &extMovePtr, &numMoves );

      // For debugging it can be useful to know when this happens.
      //if ( numMoves > 1 )
      //   Con::printf( "numMoves: %i", numMoves );

      // Do we really need to test the control object each iteration? Does it change?
      for ( m = 0; m < numMoves && con && con->getControlObject() == obj; m++, extMovePtr++ )
      {         
         #ifdef TORQUE_DEBUG_NET_MOVES
         U32 sum = Move::ChecksumMask & obj->getPacketDataChecksum(obj->getControllingClient());
         #endif
      
         if ( obj->isTicking() )
            obj->processTick( extMovePtr );

         if ( con && con->getControlObject() == obj )
         {
            U32 newsum = Move::ChecksumMask & obj->getPacketDataChecksum( obj->getControllingClient() );

            // check move checksum
            if ( extMovePtr->checksum != newsum )
            {
               #ifdef TORQUE_DEBUG_NET_MOVES
               if( !obj->isAIControlled() )
                  Con::printf("move %i checksum disagree: %i != %i, (start %i), (move %f %f %f)",
                     extMovePtr->id, extMovePtr->checksum,newsum,sum,extMovePtr->yaw,extMovePtr->y,extMovePtr->z);
               #endif

               extMovePtr->checksum = Move::ChecksumMismatch;
            }
            else
            {
               #ifdef TORQUE_DEBUG_NET_MOVES
               Con::printf("move %i checksum agree: %i == %i, (start %i), (move %f %f %f)",
                  extMovePtr->id, extMovePtr->checksum,newsum,sum,extMovePtr->yaw,extMovePtr->y,extMovePtr->z);
               #endif
            }
         }
      }

      extMoveList->clearMoves( m );
   }
   else if ( pobj->isTicking() )
      pobj->processTick( 0 );
}

void ExtendedServerProcessList::advanceObjects()
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
