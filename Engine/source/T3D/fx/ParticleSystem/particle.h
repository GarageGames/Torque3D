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

#ifndef _PARTICLE_H_
#define _PARTICLE_H_
#include <platform/types.h>
#include <math/mPoint3.h>
#include <core/util/tVector.h>

//-----------------------------------------------
//! The particle structure used for the particles
//! in the pool
//! @ingroup particlesystem
//-----------------------------------------------
struct Particle
{
   Point3F  pos;     //!< Current instantaneous position
   Point3F  relPos; //!< Relative position
   Point3F  vel;     //!< Velocity
   Point3F  acc;     //!< Constant acceleration
   Point3F  orientDir;  //!< direction particle should go if using oriented particles

   U32            totalLifetime; //!< Total ms that this instance should be "live"

   U32            currentAge; //!< The current age of the particle
   F32            spinSpeed; //!< The current angular velocity
   Particle *     next; //!< The next particle in the list
};

//-----------------------------------------------
//! Stores a pre-allocated memory pool of particles
//! and manages a linked list of active particles.
//! @ingroup particlesystem
//-----------------------------------------------
class ParticlePool
{
public:

   /// Creates a particle pool without any room for
   /// particles.
   ParticlePool();

   /// Creates a particle pool with a set initial
   /// pool size.
   ParticlePool(U32 initSize);

   /// Adds a particle to the particle pool, 
   /// @returns true if the pool was resized.
   /// @param[out] pNew The new particle that was added.
   bool AddParticle(Particle*& part);

   /// Adds a particle to the particle pool
   /// @param[in] previousParticle The previous particle.
   void RemoveParticle(Particle* previousParticle);

   /// Clears the particle pool and re-initializes it
   /// with a new size.
   void clear(U32 initSize);

   /// Advances the particles in the pool, any particles
   /// exceeding their totalLifetime will be deleted.
   void AdvanceTime(U32 numMSToUpdate);
   Particle* GetParticleHead() { return &part_list_head; };
   S32 getCount() const { return n_parts; }
   S32 getCapacity() const { return n_part_capacity; }

private:

   /// @name Linked list management
   /// @{
   ///   These members are for implementing a link-list of the active emitter 
   ///   particles. Member part_store contains blocks of particles that can be
   ///   chained in a link-list. Usually the first part_store block is large
   ///   enough to contain all the particles but it can be expanded in emergency
   ///   circumstances.
   Vector <Particle*> part_store;
   Particle*  part_freelist;
   Particle   part_list_head;
   S32        n_part_capacity;
   S32        n_parts;
   //@}
};
#endif