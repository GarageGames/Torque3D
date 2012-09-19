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
#include "forest/forestWindMgr.h"

#include "platform/profiler.h"
#include "core/tAlgorithm.h"
#include "core/module.h"
#include "console/consoleTypes.h"
#include "math/mathUtils.h"
#include "T3D/gameBase/gameConnection.h"
#include "forest/forest.h"
#include "forest/forestWindAccumulator.h"
#include "T3D/fx/particleEmitter.h"


MODULE_BEGIN( ForestWindMgr )

   MODULE_INIT
   {
      ManagedSingleton< ForestWindMgr >::createSingleton();
      ForestWindMgr::initConsole();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< ForestWindMgr >::deleteSingleton();
   }

MODULE_END;


ForestWindMgr::WindAdvanceSignal ForestWindMgr::smAdvanceSignal;  

F32 ForestWindMgr::smWindEffectRadius = 25.0f;


ForestWindMgr::ForestWindMgr()
{
   setProcessTicks( true );
   mSources = new IdToWindMap();
   mPrevSources = new IdToWindMap();
}

ForestWindMgr::~ForestWindMgr()
{
   IdToWindMap::Iterator sourceIter = mSources->begin();
   for( ; sourceIter != mSources->end(); sourceIter++ )      
      delete (*sourceIter).value;

   delete mSources;
   delete mPrevSources;
}

void ForestWindMgr::initConsole()
{
   Con::addVariable( "$pref::windEffectRadius", TypeF32, &smWindEffectRadius, "Radius to affect the wind.\n"
	   "@ingroup Rendering\n");
}

void ForestWindMgr::addEmitter( ForestWindEmitter *emitter )
{
   mEmitters.push_back( emitter );
}

void ForestWindMgr::removeEmitter( ForestWindEmitter *emitter )
{
   ForestWindEmitterList::iterator iter = find( mEmitters.begin(), 
                                                mEmitters.end(), 
                                                emitter );

   AssertFatal( iter != mEmitters.end(), 
      "SpeedTreeWindMgr::removeEmitter() - Bad emitter!" );

   mEmitters.erase( iter );
}

void ForestWindMgr::processTick()
{
   const F32 timeDelta = 0.032f;

   if ( mEmitters.empty() )
      return;

   PROFILE_SCOPE(ForestWindMgr_AdvanceTime);

   // Advance all ForestWinds.
   {
      PROFILE_SCOPE(ForestWindMgr_AdvanceTime_ForestWind_ProcessTick);

      ForestWindEmitterList::iterator iter = mEmitters.begin();
      for ( ; iter != mEmitters.end(); iter++ )
      {
         if (  (*iter)->getWind() &&
               (*iter)->isEnabled() )
         {
            (*iter)->updateMountPosition();

            ForestWind *wind = (*iter)->getWind();
            if ( wind )
               wind->processTick();
         }
      }
   }   

   // Assign the new global wind value used by the particle system.
   {
      ForestWindEmitter *pWindEmitter = getGlobalWind();
      if ( pWindEmitter == NULL )      
         ParticleEmitter::setWindVelocity( Point3F::Zero );
      else
      {
         ForestWind *pWind = pWindEmitter->getWind();
         ParticleEmitter::setWindVelocity( pWind->getDirection() * pWind->getStrength() );
      }
   }

   // Get the game connection and camera object
   // in order to retrieve the camera position.
   GameConnection *conn = GameConnection::getConnectionToServer();
   if ( !conn )
      return;

   GameBase *cam = conn->getCameraObject();
   if ( !cam )
      return;   

   const Point3F &camPos = cam->getPosition();

      

   // Gather TreePlacementInfo for trees near the camera.
   {
      PROFILE_SCOPE( ForestWindMgr_AdvanceTime_GatherTreePlacementInfo );
      
      smAdvanceSignal.trigger( camPos, smWindEffectRadius, &mPlacementInfo );
   }

   // Prepare to build a new local source map.
   {
      PROFILE_SCOPE( ForestWindMgr_AdvanceTime_SwapSources );
      AssertFatal( mPrevSources->isEmpty(), "prev sources not empty!" );
      
      swap( mSources, mPrevSources );      

      AssertFatal( mSources->isEmpty(), "swap failed!" );
   }

   // Update wind for each TreePlacementInfo
   {
      PROFILE_SCOPE( ForestWindMgr_AdvanceTime_UpdateWind );

      for( S32 i = 0; i < mPlacementInfo.size(); i++ )
      {
         const TreePlacementInfo &info = mPlacementInfo[i];      
         updateWind( camPos, info, timeDelta );    
      }

      mPlacementInfo.clear();
   }

   // Clean up any accumulators in the
   // previous local source map.
   {
      PROFILE_SCOPE( ForestWindMgr_AdvanceTime_Cleanup );

      IdToWindMap::Iterator sourceIter = mPrevSources->begin();
      for( ; sourceIter != mPrevSources->end(); sourceIter++ )
      {
         ForestWindAccumulator *accum = (*sourceIter).value;

         AssertFatal( accum, "Got null accumulator!" );
         delete accum;
      }

      mPrevSources->clear();
   }
}

ForestWindAccumulator* ForestWindMgr::getLocalWind( ForestItemKey key )
{
   PROFILE_SCOPE( ForestWindMgr_getLocalWind );

   ForestWindAccumulator *accumulator = NULL;
   IdToWindMap::Iterator iter = mSources->find( key );
   if ( iter != mSources->end() )
      accumulator = iter->value;

   if ( accumulator )
      return accumulator;
   else
   {
      /*
      ForestWindEmitterList::iterator iter = mEmitters.begin();
      for ( ; iter != mEmitters.end(); iter++ )
      {
         if (  (*iter)->getWind() &&
               (*iter)->isEnabled() &&
               !(*iter)->isLocalWind() )
         {
            return (*iter)->getWind();
         }
      }
      */
      return NULL;
   }
}

ForestWindEmitter* ForestWindMgr::getGlobalWind()
{
   ForestWindEmitterList::iterator itr = mEmitters.begin();
   for ( ; itr != mEmitters.end(); itr++ )
   {
      if ( !(*itr)->isRadialEmitter() )
         return *itr;
   }

   return NULL;
}

void ForestWindMgr::updateWind(  const Point3F &camPos, 
                                 const TreePlacementInfo &info, 
                                 F32 timeDelta )
{
   PROFILE_SCOPE(ForestWindMgr_updateWind);

   // See if we have the blended source available.
   ForestWindAccumulator *blendDest = NULL;
   {
      IdToWindMap::Iterator iter = mPrevSources->find( info.itemKey );
      if ( iter != mPrevSources->end() )
      {
         blendDest = iter->value;
         mPrevSources->erase( iter );
      }
   }

   // Get some stuff we'll need for finding the emitters.
   F32 treeHeight = info.scale * info.dataBlock->getObjBox().len_z();
   Point3F top = info.pos;
   top.z += treeHeight;
   if ( blendDest )
      top += ( 1.0f / info.scale ) * blendDest->getDirection();

   // Go thru the emitters to accumulate the total wind force.
   VectorF windForce( 0, 0, 0 );

   F32 time = Sim::getCurrentTime() / 1000.0f;

   ForestWindEmitterList::iterator iter = mEmitters.begin();
   for ( ; iter != mEmitters.end(); iter++ )
   {
      ForestWindEmitter *emitter = (*iter);

      // If disabled or no wind object... skip it.
      if ( !emitter->isEnabled() || !emitter->getWind() )
         continue;

      ForestWind *wind = emitter->getWind();

      F32 strength = wind->getStrength();
      
      if ( emitter->isRadialEmitter() )
      {
         Point3F closest = MathUtils::mClosestPointOnSegment( info.pos, top, emitter->getPosition() );
         Point3F dir = closest - emitter->getPosition();
         F32 lenSquared = dir.lenSquared();
         if ( lenSquared > emitter->getWindRadiusSquared() )
            continue;

         dir *= 1.0f / mSqrt( lenSquared );

         F32 att = lenSquared / emitter->getWindRadiusSquared();
         strength *= 1.0f - att;
         windForce += dir * strength;
      }
      else
      {
         F32 d = mDot( info.pos, Point3F::One ); //PlaneF( Point3F::Zero, wind->getDirection() ).distToPlane( Point3F( info.pos.x, info.pos.y, 0 ) );
         //F32 d = PlaneF( Point3F::Zero, wind->getDirection() ).distToPlane( Point3F( info.pos.x, info.pos.y, 0 ) );
         F32 scale = 1.0f + ( mSin( d + ( time / 10.0 ) ) * 0.5f );
         windForce += wind->getDirection() * strength * scale;
      }
   }

   // If we need a accumulator then we also need to presimulate.
   if ( !blendDest )
   {
      blendDest = new ForestWindAccumulator( info );
      blendDest->presimulate( windForce, 4.0f / TickSec );
   }
   else
      blendDest->updateWind( windForce, timeDelta );

   mSources->insertUnique( info.itemKey, blendDest );
}