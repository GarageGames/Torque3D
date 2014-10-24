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

#ifndef _PARTICLE_SYSTEM_H
#define _PARTICLE_SYSTEM_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

#include "particle.h"
#include <T3D/fx/ParticleSystem/particleRenderer.h>
#include <T3D/fx/ParticleSystem/particleEmitter.h>
#include <T3D/fx/ParticleSystem/particleBehaviour.h>

/// @defgroup particlesystem Particle System
/// @{

#ifndef ParticleBehaviourCount
#define ParticleBehaviourCount (U8)8 ///< The maximum amount of particle behaviours.
#endif

//-----------------------------------------------
//! Provides an interface for the rest of the 
//! engine to use when working with particlesystems.
//! Any class implementing this interface can act
//! as a particle system.
//-----------------------------------------------
class IParticleSystem : public GameBase
{
public:
   virtual ~IParticleSystem() {};

   /// @name Particle Emission
   /// Main interface for creating particles.  The emitter does _not_ track changes
   ///  in axis or velocity over the course of a single update, so this should be called
   ///  at a fairly fine grain.  The emitter will potentially track the last particle
   ///  to be created into the next call to this function in order to create a uniformly
   ///  random time distribution of the particles.  If the object to which the emitter is
   ///  attached is in motion, it should try to ensure that for call (n+1) to this
   ///  function, start is equal to the end from call (n).  This will ensure a uniform
   ///  spatial distribution.
   /// @{

public:
   virtual void emitParticles(const Point3F& start,
      const Point3F& end,
      const Point3F& axis,
      const Point3F& velocity,
      const U32      numMilliseconds) = 0;

   virtual void emitParticles(const Point3F& point,
      const bool     useLastPosition,
      const Point3F& axis,
      const Point3F& velocity,
      const U32      numMilliseconds) = 0;

   virtual void emitParticles(const Point3F& rCenter,
      const Point3F& rNormal,
      const F32      radius,
      const Point3F& velocity,
      S32 count) = 0;

   /// @}

   /// Deletes the ParticleSystem when all particles are dead.
   /// Stops emitting particles, and deletes the particle system
   /// when all the particles are dead.
   virtual void deleteWhenEmpty() = 0;

   virtual ParticleEmitter* getEmitter() = 0;
   virtual ParticleRenderer* getRenderer() = 0;

   DECLARE_ABSTRACT_CLASS(IParticleSystem, GameBase);
};

//-----------------------------------------------
//! Provides an interface for datablocks of a
//! particle system, which the rest of the engine
//! references.
//-----------------------------------------------
class IParticleSystemData : public GameBaseData
{
   typedef GameBaseData Parent;

public:
   IParticleSystemData() {};

   /// Creates an instance of the particle system which
   /// is associated with this datablock.
   virtual IParticleSystem* createParticleSystem() = 0;

   DECLARE_ABSTRACT_CLASS(IParticleSystemData, GameBaseData);
};

class ParticleSystem;

//-----------------------------------------------
//! A datablock for the data fields of the particle
//! system.
//-----------------------------------------------
class ParticleSystemData : public IParticleSystemData
{
   typedef GameBaseData Parent;

public:
   // Creation
   ParticleSystemData();

   bool preload(bool server, String &errorStr);
   bool onAdd();

   // Script interface
   static void  initPersistFields();

   // Networking
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   // IParticleSystemData
   virtual IParticleSystem* createParticleSystem();

   /// @name Getters and setters
   /// @{

   /// @returns The lifetime of the emitted particles.
   F32 getPartLifetimeMS() { return mPartLifetimeMS; };

   /// @returns The lifetime variance of the emitted particles.
   S32 getPartLifetimeVarianceMS() { return mPartLifetimeVarianceMS; };

   /// @returns The angular velocity of the emitted particles.
   F32 getSpinSpeed() { return mSpinSpeed; };

   /// @returns The lower boundary for the random value to add 
   /// to the angular velocity.
   F32 getSpinRandomMin() { return mSpinRandomMin; };

   /// @returns The upper boundary for the random value to add 
   /// to the angular velocity.
   F32 getSpinRandomMax() { return mSpinRandomMax; };

   /// @returns The inherited velocity factor of the emitted particles.
   F32 getInheritedVelFactor() { return mInheritedVelFactor; };

   /// @returns The constant acceleration factor of the emitted particles.
   F32 getConstantAcceleration() { return mConstantAcceleration; };

   ///@}

   /// @name Components
   ///  The components that make up this ParticleSystem.
   /// @{

   ParticleEmitterData *mEmitterData; ///< The datablock for the ParticleEmitter component.
   ParticleRendererData *mRendererData; ///< The datablock for the ParticleRenderer component.
   IParticleBehaviour *ParticleBHVs[ParticleBehaviourCount]; ///< The array of ParticleBehaviour components.

   /// @}

   /// @name Emission
   /// These fields affect how the particles are emitted.
   /// @{

   /// Lifetime parameter for limiting the lifetime of the ParticleSystem.
   /// A lifetime of [TODO] means infinite.
   F32 mLifetimeMS;

   /// The amount of particles to emit per second, can maximally be 1000.
   S32 mParticlesPerSecond;

   /// The amount of particles the mParticlesPerSecond can vary with.
   S32 mParticlesPerSecondVariance;

   /// @}

   /// @name Particles
   /// These fields affect how the particles behave.
   /// @{

   S32 mPartLifetimeMS; ///< The lifetime of particles in milliseconds.
   S32 mPartLifetimeVarianceMS; ///< The lifetime variance of particles in milliseconds.
   F32 mSpinSpeed; ///< The angular velocity of particles.

   /// The lower boundary for the random value to add 
   /// to the angular velocity.
   F32 mSpinRandomMin;

   /// The upper boundary for the random value to add 
   /// to the angular velocity.
   F32 mSpinRandomMax;

   F32 mInheritedVelFactor; ///< The inherited velocity factor of the emitted particles.
   F32 mConstantAcceleration; ///< The constant acceleration factor of the emitted particles.
   bool mOverrideAdvance; ///< 

   // Physics constants
   F32 mDragCoefficient; ///< The constant wind resistance coefficient.
   F32 mWindCoefficient; ///< The constant wind speed force coefficient.
   F32 mGravityCoefficient; ///< The constant gravity force coefficent.

   /// @}

   /// @name LOD
   /// These fields control the Level-Of-Detail of the particles.
   /// @{

   F32   mSimulationLODBegin; ///< The distance at which simulation will begin to be LOD'ed down.
   F32   mSimulationLODEnd; ///< The distance at which particles will reach their lowest simulation LOD.

   /// @}

   DECLARE_CONOBJECT(ParticleSystemData);
};

//*****************************************************************************
// Particle System
//*****************************************************************************
class ParticleSystem : public IParticleSystem
{
   typedef IParticleSystem Parent;

public:
   /// @name Constants
   /// @{

   static F32 const AgedSpinToRadians;

   enum PDConst
   {
      PDC_NUM_KEYS = 4,
   };

   /// @}

   /// @name Environmental variables
   /// @{

   static Point3F mWindVelocity;
   static void setWindVelocity(const Point3F &vel){ mWindVelocity = vel; }

   /// @}

   ParticleSystem();
   ~ParticleSystem();

   bool onAdd();
   void onRemove();

   // 
public:
   /// @name Particle Emission
   /// Main interface for creating particles.  The emitter does _not_ track changes
   ///  in axis or velocity over the course of a single update, so this should be called
   ///  at a fairly fine grain.  The emitter will potentially track the last particle
   ///  to be created into the next call to this function in order to create a uniformly
   ///  random time distribution of the particles.  If the object to which the emitter is
   ///  attached is in motion, it should try to ensure that for call (n+1) to this
   ///  function, start is equal to the end from call (n).  This will ensure a uniform
   ///  spatial distribution.
   /// @{

   void emitParticles(const Point3F& start,
      const Point3F& end,
      const Point3F& axis,
      const Point3F& velocity,
      const U32      numMilliseconds);

   void emitParticles(const Point3F& point,
      const bool     useLastPosition,
      const Point3F& axis,
      const Point3F& velocity,
      const U32      numMilliseconds);

   void emitParticles(const Point3F& rCenter,
      const Point3F& rNormal,
      const F32      radius,
      const Point3F& velocity,
      S32 count);

   /// @}

   /// Renders the particle system
   void prepRenderImage(SceneRenderState* state);

   // Updating
   virtual void processTick(const Move*);
   virtual void advanceTime(F32 dt);
   void simulate(U32 ms);

   // Networking
   //U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   //void unpackUpdate(NetConnection *conn,           BitStream *stream);

   /// Deletes the ParticleSystem when all particles are dead.
   /// Stops emitting particles, and deletes the particle system
   /// when all the particles are dead.
   void deleteWhenEmpty();

   /// @name Getters and setters
   /// @{

   /// Returns the ParticleSystems internal clock
   /// @returns the ParticleSystems internal clock
   F32 getInternalClock() { return mInternalClock; }

   /// True if the ParticleSystem has a last position defined.
   /// This will be the case if emitParticles have been called
   /// earlier.
   /// @returns True if the ParticleSystem has a lastPosition.
   bool hasLastPosition() { return mHasLastPosition; };
   /// @returns The last position where particles were emitted.
   Point3F getLastPosition() { return mLastPosition; };

   /// @returns The ParticleSystems @ref ParticlePool
   ParticlePool* getParticlePool() { return mParticlePool; };

   /// @returns The ParticleSystems @ref ParticleEmitter
   ParticleEmitter* getEmitter() { return mEmitter; };
   /// @returns The ParticleSystems @ref ParticleRenderer
   ParticleRenderer* getRenderer() { return mRenderer; };

   /// @returns The ParticleSystems @ref ParticleSystemData
   ParticleSystemData* getDataBlock() { return mDataBlock; };

   /// Sets a new datablock.
   bool onNewDataBlock(GameBaseData *dptr, bool reload);

   /// @}

private:
   /// @name Components
   ///  The components that make up this ParticleSystem.
   /// @{

   ParticlePool* mParticlePool;

   ParticleEmitter* mEmitter;
   ParticleRenderer* mRenderer;

   ParticleSystemData* mDataBlock;

   /// @}

   bool mDead; ///< True if the particle system no longer emits particles.
   bool mDeleteOnTick; ///< True if the particle system should self-destruct on next engine-tick.
   bool mDeleteWhenEmpty; ///< True if the ParticleSystem should self-destruct when all particles are dead.

   F32 mElapsedTimeMS; ///< 
   F32 mNextParticleTime; ///<
   F32 mInternalClock; ///<

   Point3F mLastPosition; ///< The last position where particles were emitted.
   bool mHasLastPosition; ///< True if mLastPosition is defined.

   /// @name LOD
   /// @{

   U32 mSkipUpdateCount; ///< The amount of simulation updates to skip.
   U32 mSkippedUpdates; ///< The amount of skipped updates.
   U32 mTSU; ///< Time since last update.

   /// @}

   DECLARE_CONOBJECT(ParticleSystem);
};
///@}
#endif // _PARTICLE_SYSTEM_H

