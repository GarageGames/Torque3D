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

#ifndef PARTICLE_BEHAVIOUR_H
#define PARTICLE_BEHAVIOUR_H

#include "sim/netConnection.h"
#include "console/consoleTypes.h"
#include "console/simDatablock.h"
#include <T3D/fx/ParticleSystem/particle.h>

class ParticleSystem;

/// @defgroup particlebehaviours Behaviours
/// @ingroup particlesystem
/// @{

namespace behaviourType{
   /// The behaviourType defines when during the simulation
   /// the behaviour will be executed.
   /// @ingroup particlebehaviours
   enum behaviourType{
      Acceleration, ///< Execute during acceleration calculation.
      Velocity, ///< Execute during velocity calculation.
      Position, ///< Execute during position calculation.
      Error
   };
}

//-----------------------------------------------
//! The interface for ParticleBehaviours.
//! ParticleBehaviours are implemented as datablocks
//! but doesn't have an object associated with them.
//! Instead they are global objects, like @ref Materials.
//! @ingroup particlebehaviours
//-----------------------------------------------
class IParticleBehaviour : public SimDataBlock
{
   typedef SimDataBlock Parent;
public:

   /// Gets the priority of the behaviour.
   /// The behaviours are sorted based on priority such that they can execute in a specific order.
   /// @returns The priority.
   virtual U8 getPriority() = 0;

   /// Updates the given particle.
   /// The particle will have it's position, velocity or acceleration altered based on
   /// the nature of the behaviour.
   /// @param[in] system The @ref ParticleSystem that owns the particle.
   /// @param[in] part The particle to be update.
   /// @param[in] time How long time the update should cover.
   virtual void updateParticle(ParticleSystem* system, Particle* part, F32 time) = 0;

   /// Returns the @ref behaviourType of this behaviour.
   virtual behaviourType::behaviourType getType() { return behaviourType::Error; };

   /// ParticleBehaviours are compared based on their priority.
   bool operator<(IParticleBehaviour* IPB) { return getPriority() < IPB->getPriority(); };

   /// A compare function for sorting.
   static S32 compare(const void* e1, const void* e2)
   {
      if (*(IParticleBehaviour**)e1 == NULL
         && *(IParticleBehaviour**)e2 == NULL)
         return 0;
      if (*(IParticleBehaviour**)e1 == NULL)
         return -1;
      if (*(IParticleBehaviour**)e2 == NULL)
         return 1;
      return (*(IParticleBehaviour**)e1)->getPriority() - (*(IParticleBehaviour**)e2)->getPriority();
   };

   DECLARE_ABSTRACT_CLASS(IParticleBehaviour, SimDataBlock);
};

/// @}

#endif // PARTICLE_BEHAVIOUR_H