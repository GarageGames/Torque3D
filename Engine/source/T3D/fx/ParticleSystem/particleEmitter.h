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

#ifndef _PARTICLE_EMITTER_H
#define _PARTICLE_EMITTER_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#include <T3D/fx/ParticleSystem/particle.h>

class ParticleSystem;
class ParticleEmitter;

/// @defgroup particleemitters Emitters
/// @ingroup particlesystem
/// @{

//-----------------------------------------------
//! The interface for ParticleEmitter datablocks.
//! Any class implementing this interface can be
//! assigned to a @ref ParticleSystem .
//! @ingroup particleemitters
//-----------------------------------------------
class ParticleEmitterData : public GameBaseData
{
   typedef GameBaseData Parent;

public:
   ParticleEmitterData()
   {
   }

   /// Creates an instance of the ParticleEmitter class associated with
   /// this datablock.
   /// @returns The new ParticleEmitter
   virtual ParticleEmitter* CreateEmitter(ParticleSystem* system) = 0;

   DECLARE_ABSTRACT_CLASS(ParticleEmitterData, GameBaseData);
};

//-----------------------------------------------
//! The interface for ParticleEmitters.
//! Any class implementing this interface can act
//! as a ParticleEmitter.
//! @ingroup particleemitters
//-----------------------------------------------
class ParticleEmitter
{
public:
   ParticleEmitter(ParticleSystem* parentSystem) : mParentSystem(parentSystem) {}
   virtual ~ParticleEmitter() {}

   /// Adds a new particle.
   /// @param[in] pos The position to emit the particle in 
   /// (not necessarily the position of the added particle).
   /// @param[in] axis The axis to emit the particle along.
   /// @param[in] vel The velocity at which the particle is emitted.
   /// @param[in] axisx 
   virtual bool addParticle(Point3F const& pos,
      Point3F const& axis,
      Point3F const& vel,
      Point3F const& axisx) = 0;

   /// Returns this object's datablock.
   /// @returns The datablock.
   virtual ParticleEmitterData* getDataBlock() { return mDataBlock; };

   /// Sets the datablock of this emitter.
   virtual void setDataBlock(ParticleEmitterData *data) { mDataBlock = data; };

protected:
   ParticleSystem* mParentSystem; ///< The ParticleSystem which owns this instance.
   ParticleEmitterData* mDataBlock; ///< The datablock for this instance.
};

/// @}

#endif //_PARTICLE_EMITTER_H