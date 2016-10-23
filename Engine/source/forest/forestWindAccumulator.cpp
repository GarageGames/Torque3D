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
#include "forest/forestWindAccumulator.h"

#include "forest/forestWindMgr.h"
#include "forest/forestItem.h"
#include "platform/profiler.h"


ForestWindAccumulator::ForestWindAccumulator( const TreePlacementInfo &info )
:  mCurrentStrength( 0.0f )
{
   mCurrentDir.set( 0, 0 );   
   mPosition.set( info.pos );
   mScale = info.scale;

   mDataBlock = info.dataBlock;

   dMemset( &mParticles[0], 0, sizeof( VerletParticle ) );
   dMemset( &mParticles[1], 0, sizeof( VerletParticle ) );
}

ForestWindAccumulator::~ForestWindAccumulator()
{
}

void ForestWindAccumulator::presimulate( const VectorF &windVector, U32 ticks )
{
   PROFILE_SCOPE( ForestWindAccumulator_Presimulate );

   for ( U32 i = 0; i < ticks; i++ )
      updateWind( windVector, TickSec );
}

void ForestWindAccumulator::updateWind( const VectorF &windForce, F32 timeDelta )
{
   PROFILE_SCOPE( ForestWindAccumulator_UpdateWind );

   // Update values from datablock... this way we can
   // change settings live and see instant results.
   const F32 tightnessCoefficient = mDataBlock->mTightnessCoefficient;
   const F32 dampingCoefficient = mDataBlock->mDampingCoefficient;
   const F32 mass = mDataBlock->mMass * mScale;
   const F32 rigidity = mDataBlock->mRigidity * mScale;

   // This will be the accumulated
   // target strength for flutter.
   //F32 targetStrength = windForce.len();

   // This will be the accumulated
   // target displacement vector.
   Point2F target( windForce.x, windForce.y );

   // This particle is the spring target.
   // It has a mass of 0, which we count as
   // an infinite mass.
   mParticles[0].position = target;

   Point2F diff( 0, 0 );
   Point2F springForce( 0, 0 );

   // Spring length is the target
   // particle's position minus the
   // current displacement/direction vector.
   diff = mParticles[0].position - mCurrentDir;

   // F = diff * tightness - v * -damping
   diff *= tightnessCoefficient;
   springForce = diff - ( (mParticles[1].position - mParticles[1].lastPosition) * -dampingCoefficient );

   Point2F accel( 0, 0 );
   accel = springForce * (rigidity * 0.001f) / (mass * 0.001f);

   _updateParticle( &mParticles[1], accel, timeDelta );

   mCurrentDir *= 0.989f;
   mCurrentDir += mParticles[1].position;

   mCurrentStrength += windForce.len() * timeDelta;
   mCurrentStrength *= 0.98f;
}

void ForestWindAccumulator::_updateParticle( VerletParticle *particle, const Point2F &accel, F32 timeDelta )
{
   // Verlet integration:
   // x' = 2x - x* + a * dt^2
   // x' is the new position.
   // x is the current position.
   // x* is the last position.
   // a is the acceleration for this frame.
   // dt is the delta time.
  
   particle->position = ((particle->position * 2.0f) - particle->lastPosition) + accel * (timeDelta * timeDelta);
   particle->lastPosition = particle->position;
}

void ForestWindAccumulator::applyImpulse( const VectorF &impulse )
{
   // First build the current force.
   VectorF force( mCurrentDir.x, mCurrentDir.y, 0 );

   // Add in our mass corrected force.
   const F32 mass = mDataBlock->mMass * mScale;
   force += impulse / mass;

   // Set the new direction and force.
   mCurrentDir.set( force.x, force.y );  
   mCurrentStrength += impulse.len() * TickSec;
}
