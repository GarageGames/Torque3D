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

#ifndef _SPHERE_EMITTER_H
#define _SPHERE_EMITTER_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#include <T3D/fx/ParticleSystem/particle.h>
#include <T3D/fx/ParticleSystem/particleEmitter.h>
#include <T3D/fx/ParticleSystem/particleSystem.h>

//-----------------------------------------------
//! A datablock for @ref SphereEmitter's.
//! @ingroup particleemitters
//-----------------------------------------------
class SphereEmitterData : public ParticleEmitterData
{
   typedef ParticleEmitterData Parent;

public:
   SphereEmitterData();

   virtual ParticleEmitter* CreateEmitter(ParticleSystem *system);

   // Script interface
   static void  initPersistFields();

   // Networking
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   // Getters and setters
   F32 getThetaMax() { return mThetaMax; };
   F32 getThetaMin() { return mThetaMin; };
   F32 getPhiReferenceVel() { return mPhiReferenceVel; };
   F32 getPhiVariance() { return mPhiVariance; };
   F32 getEjectionVelocity() { return mEjectionVelocity; };
   F32 getVelocityVariance() { return mVelocityVariance; };
   F32 getEjectionOffset() { return mEjectionOffset; };
   F32 getEjectionOffsetVariance() { return mEjectionOffsetVariance; };

private:
   // Fields
   F32 mThetaMax; //!< Maximum angle, from the horizontal plane, to eject particles from.
   F32 mThetaMin; //!< Minimum angle, from the horizontal plane, to eject from.
   F32 mPhiReferenceVel; //!< Reference angle, from the vertical plane, to eject particles from.
   F32 mPhiVariance; //!< Variance from the reference angle, from 0 - 360.
   F32 mEjectionVelocity; //!< Particle ejection velocity.
   F32 mVelocityVariance; //!< Variance for ejection velocity, from 0 - ejectionVelocity.
   F32 mEjectionOffset; //!< Distance along ejection Z axis from which to eject particles.
   F32 mEjectionOffsetVariance; //!< Distance Padding along ejection Z axis from which to eject particles.

   DECLARE_CONOBJECT(SphereEmitterData);
};

//-----------------------------------------------
//! A @ref ParticleEmitter that emits particles on
//! a sphere.
//! @ingroup particleemitters
//-----------------------------------------------
class SphereEmitter : public ParticleEmitter
{
public:
   SphereEmitter(ParticleSystem* parentSystem) : ParticleEmitter(parentSystem) {}

   virtual bool addParticle(Point3F const& pos,
      Point3F const& axis,
      Point3F const& vel,
      Point3F const& axisx);

   virtual SphereEmitterData* getDataBlock() { return dynamic_cast<SphereEmitterData*>(mDataBlock); };
};

#endif //_SPHERE_EMITTER_H