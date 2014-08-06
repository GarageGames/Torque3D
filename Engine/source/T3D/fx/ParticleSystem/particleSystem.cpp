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

#include "particleSystem.h"
#include "particleEmitter.h"
#include <T3D/gameBase/gameProcess.h>
#include "scene/sceneManager.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "console/typeValidators.h"
#include "T3D/gameBase/gameConnection.h"

IMPLEMENT_NONINSTANTIABLE_CLASS(IParticleSystemData,
   "")
   END_IMPLEMENT_CLASS;

IMPLEMENT_NONINSTANTIABLE_CLASS(IParticleSystem,
   "")
   END_IMPLEMENT_CLASS;

IMPLEMENT_CO_DATABLOCK_V1(ParticleSystemData);
IMPLEMENT_CONOBJECT(ParticleSystem);

//*****************************************************************************
// ParticleSystemData
//*****************************************************************************

static const float sgDefaultWindCoefficient = 0.0f;
static const float sgDefaultConstantAcceleration = 0.f;
static const float sgDefaultSpinSpeed = 1.f;
static const float sgDefaultSpinRandomMin = 0.f;
static const float sgDefaultSpinRandomMax = 0.f;

//-----------------------------------------------------------------------------
// ParticleSystemData
//-----------------------------------------------------------------------------
ParticleSystemData::ParticleSystemData()
{
   mEmitterData = NULL;
   mRendererData = NULL;

   mLifetimeMS = 0;
   mParticlesPerSecond = 100;
   mParticlesPerSecondVariance = 0;

   mPartLifetimeMS = 1000;
   mPartLifetimeVarianceMS = 0;

   mSpinSpeed = sgDefaultSpinSpeed;
   mSpinRandomMin = sgDefaultSpinRandomMin;
   mSpinRandomMax = sgDefaultSpinRandomMax;

   mInheritedVelFactor = 0.0f;
   mConstantAcceleration = sgDefaultConstantAcceleration;
   mOverrideAdvance = false;

   mDragCoefficient = 0.0f;
   mWindCoefficient = sgDefaultWindCoefficient;
   mGravityCoefficient = 0.0f;

   mSimulationLODBegin = 10;
   mSimulationLODEnd = 100;

   for (int i = 0; i < ParticleBehaviourCount; i++)
      ParticleBHVs[i] = NULL;
}

FRangeValidator dragCoefFValidator(0.f, 5.f);
FRangeValidator gravCoefFValidator(-10.f, 10.f);

//-----------------------------------------------------------------------------
// initPersistFields
//-----------------------------------------------------------------------------
void ParticleSystemData::initPersistFields()
{
   addGroup("Emission");

   addField("Lifetime", TypeS32, Offset(mLifetimeMS, ParticleSystemData),
      "Lifetime of the particle system in milliseconds");

   addField("ParticlesPerSecond", TypeS32, Offset(mParticlesPerSecond, ParticleSystemData),
      "The amount of particles emitted each second (max 1000)");

   addField("ParticlesPerSecondVariance", TypeS32, Offset(mParticlesPerSecondVariance, ParticleSystemData),
      "The maximal amount of particles to vary emission with");

   endGroup("Emission");

   addGroup("Particles");

   addField("PartLifeTime", TypeS32, Offset(mPartLifetimeMS, ParticleSystemData),
      "The lifetime of the particles in milliseconds");

   addField("PartLifeTimeVariance", TypeS32, Offset(mPartLifetimeVarianceMS, ParticleSystemData),
      "The time to vary the lifetime of emitted particles with");

   addField("SpinSpeed", TypeF32, Offset(mSpinSpeed, ParticleSystemData),
      "Speed at which to spin the particle");

   addField("SpinSpeedMin", TypeF32, Offset(mSpinRandomMin, ParticleSystemData),
      "Minimum allowed spin speed of this particle, between -1000 and spinRandomMax.");

   addField("SpinSpeedMax", TypeF32, Offset(mSpinRandomMax, ParticleSystemData),
      "Maximum allowed spin speed of this particle, between spinRandomMin and 1000.");

   endGroup("Particles");

   addGroup("Physics");

   addField("InheritedVelFactor", TypeF32, Offset(mInheritedVelFactor, ParticleSystemData),
      "Amount of emitter velocity to add to particle initial velocity.");

   addField("ConstantAcceleration", TypeF32, Offset(mConstantAcceleration, ParticleSystemData),
      "Constant acceleration to apply to this particle.");

   addField("OverrideAdvance", TypeBool, Offset(mOverrideAdvance, ParticleSystemData),
      "If false, particles emitted in the same frame have their positions "
      "adjusted. If true, adjustment is skipped and particles will clump "
      "together.");

   addFieldV("DragCoefficient", TYPEID< F32 >(), Offset(mDragCoefficient, ParticleSystemData), &dragCoefFValidator,
      "Particle physics drag amount.");

   addField("WindCoefficient", TYPEID< F32 >(), Offset(mWindCoefficient, ParticleSystemData),
      "Strength of wind on the particles.");

   addFieldV("GravityCoefficient", TYPEID< F32 >(), Offset(mGravityCoefficient, ParticleSystemData), &gravCoefFValidator,
      "Strength of gravity on the particles.");

   endGroup("Physics");

   addGroup("Level of detail");

   addField("SimulationLODBegin", TypeS32, Offset(mSimulationLODBegin, ParticleSystemData),
      "@brief How far the node must be from the camera before the emitter begins reducing the amount of update ticks per second it runs. ");

   addField("SimulationLODEnd", TypeS32, Offset(mSimulationLODEnd, ParticleSystemData),
      "@brief How far the node must be from the camera before the emitter stops updating the particles. ");

   endGroup("Level of detail");

   addGroup("Components");

   addField("Emitter", TYPEID<ParticleEmitterData>(), Offset(mEmitterData, ParticleSystemData),
      "The datablock for the particle emitter that spawns new particles.");

   addField("Renderer", TYPEID<ParticleRendererData>(), Offset(mRendererData, ParticleSystemData),
      "The datablock for the renderer to use when rendering the particle system.");

   endGroup("Components");

   addGroup("ParticleBehaviours");

   addField("ParticleBehaviour", TYPEID<IParticleBehaviour>(), Offset(ParticleBHVs, ParticleSystemData), ParticleBehaviourCount,
      "List of particle behaviours, to manipulate the physical behaviour of particles.");

   endGroup("ParticleBehaviours");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
// packData
//-----------------------------------------------------------------------------
void ParticleSystemData::packData(BitStream* stream)
{
   Parent::packData(stream);

   // We only write 19 bits here to save a little bit of traffic.
   // 19 bits gives you a maximal lifetime around 524 seconds.
   stream->writeInt(mLifetimeMS, 20);

   // Mathematical limit of 1000 means we only have to write
   // 11 bits, but we write 12 to be sure.
   stream->writeInt(mParticlesPerSecond, 12);
   stream->writeInt(mParticlesPerSecondVariance, 12);

   stream->writeInt(mPartLifetimeMS, 20);
   stream->writeInt(mPartLifetimeVarianceMS, 20);

   if (stream->writeFlag(mSpinSpeed != sgDefaultSpinSpeed))
      stream->write(mSpinSpeed);
   if (stream->writeFlag(mSpinRandomMin != sgDefaultSpinRandomMin || mSpinRandomMax != sgDefaultSpinRandomMax))
   {
      stream->writeInt((S32)(mSpinRandomMin * 1000), 11);
      stream->writeInt((S32)(mSpinRandomMax * 1000), 11);
   }

   stream->writeFloat(mInheritedVelFactor, 9);
   if (stream->writeFlag(mConstantAcceleration != sgDefaultConstantAcceleration))
      stream->write(mConstantAcceleration);
   stream->writeFlag(mOverrideAdvance);

   stream->writeFloat(mDragCoefficient / 5, 10);
   if (stream->writeFlag(mWindCoefficient != sgDefaultWindCoefficient))
      stream->write(mWindCoefficient);
   if (stream->writeFlag(mGravityCoefficient != 0.0f))
      stream->writeSignedFloat(mGravityCoefficient / 10, 12);

   stream->writeInt(mSimulationLODBegin * 1000, 18);
   stream->writeInt(mSimulationLODEnd * 1000, 18);

   if (stream->writeFlag(mEmitterData != NULL))
   {
      stream->writeRangedU32(mEmitterData->getId(),
         DataBlockObjectIdFirst,
         DataBlockObjectIdLast);
   }

   if (stream->writeFlag(mRendererData != NULL))
   {
      stream->writeRangedU32(mRendererData->getId(),
         DataBlockObjectIdFirst,
         DataBlockObjectIdLast);
   }

   for (int i = 0; i < ParticleBehaviourCount; i++)
   {
      if (stream->writeFlag(ParticleBHVs[i]))
      {
         stream->writeRangedU32(ParticleBHVs[i]->getId(),
            DataBlockObjectIdFirst,
            DataBlockObjectIdLast);
      }
   }
}

//-----------------------------------------------------------------------------
// unpackData
//-----------------------------------------------------------------------------
void ParticleSystemData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   // We only write 20 bits here to save a little bit of traffic.
   // 20 bits gives you a maximal lifetime around 524 seconds.
   mLifetimeMS = stream->readInt(20);

   // Mathematical limit of 1000 means we only have to write
   // 11 bits, but we write 12 to be sure.
   mParticlesPerSecond = stream->readInt(12);
   mParticlesPerSecondVariance = stream->readInt(12);

   mPartLifetimeMS = stream->readInt(20);
   mPartLifetimeVarianceMS = stream->readInt(20);

   if (stream->readFlag())
      stream->read(&mSpinSpeed);
   if (stream->readFlag())
   {
      mSpinRandomMin = stream->readInt(11) / 1000.0f;
      mSpinRandomMax = stream->readInt(11) / 1000.0f;
   }

   mInheritedVelFactor = stream->readFloat(9);
   if (stream->readFlag())
      stream->read(&mConstantAcceleration);
   mOverrideAdvance = stream->readFlag();

   mDragCoefficient = stream->readFloat(10) * 5;
   if (stream->readFlag())
      stream->read(&mWindCoefficient);
   else
      mWindCoefficient = sgDefaultWindCoefficient;
   if (stream->readFlag())
      mGravityCoefficient = stream->readSignedFloat(12) * 10;
   else
      mGravityCoefficient = 0.0f;

   mSimulationLODBegin = stream->readInt(18) / 1000.0f;
   mSimulationLODEnd = stream->readInt(18) / 1000.0f;

   // DataBlockMask
   if (stream->readFlag())
   {
      ParticleEmitterData *dptr;
      SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst,
         DataBlockObjectIdLast);

      if (!Sim::findObject(id, dptr))
         Con::errorf("Invalid Emitter datablock");
      else
         mEmitterData = dptr;
   }

   // DataBlockMask
   if (stream->readFlag())
   {
      ParticleRendererData *dptr;
      SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst,
         DataBlockObjectIdLast);

      if (!Sim::findObject(id, dptr))
         Con::errorf("Invalid Renderer datablock");
      else
         mRendererData = dptr;
   }

   for (int i = 0; i < ParticleBehaviourCount; i++)
   {
      if (stream->readFlag())
      {
         IParticleBehaviour *dptr = 0;
         SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst,
            DataBlockObjectIdLast);
         if (!Sim::findObject(id, dptr))
            ParticleBHVs[i] = dptr;
      }
   }
}

//-----------------------------------------------------------------------------
// onAdd
//-----------------------------------------------------------------------------
bool ParticleSystemData::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   // Validate the parameters...
   //
   if (mParticlesPerSecond < 0)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) period < 1 ms", getName());
      mParticlesPerSecond = 1;
   }
   if (mParticlesPerSecondVariance >= mParticlesPerSecond)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) periodVariance >= period", getName());
      mParticlesPerSecondVariance = mParticlesPerSecond - 1;
   }
   if (mLifetimeMS < 0)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) lifetimeMS < 0.0f", getName());
      mLifetimeMS = 0;
   }
   if (mPartLifetimeMS < 0)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) PartLifetimeMS < 0.0f", getName());
      mPartLifetimeMS = 0;
   }
   if (mPartLifetimeVarianceMS < 0)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) PartLifetimeVarianceMS < 0.0f", getName());
      mPartLifetimeVarianceMS = 0;
   }
   if (mPartLifetimeVarianceMS >= mPartLifetimeMS)
   {
      Con::warnf(ConsoleLogEntry::General, "ParticleSystemData(%s) PartLifetimeVarianceMS > ParticleLifetimeMS", getName());
      mPartLifetimeVarianceMS = 0;
   }

   return true;
}

//-----------------------------------------------------------------------------
// preload
//-----------------------------------------------------------------------------
bool ParticleSystemData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;

   return true;
}

IParticleSystem* ParticleSystemData::createParticleSystem()
{
   return new ParticleSystem();
}

//*****************************************************************************
// ParticleSystem
//*****************************************************************************

Point3F ParticleSystem::mWindVelocity(0.0, 0.0, 0.0);
const F32 ParticleSystem::AgedSpinToRadians = (1.0f / 1000.0f) * (1.0f / 360.0f) * M_PI_F * 2.0f;

ParticleSystem::ParticleSystem()
{
   mEmitter = NULL;
   mRenderer = NULL;

   mDataBlock = NULL;
   mParticlePool = NULL;

   mDead = false;
   mDeleteOnTick = false;
   mDeleteWhenEmpty = false;

   mElapsedTimeMS = 0.0f;
   mNextParticleTime = 0.0f;
   mInternalClock = 0.0f;

   mLastPosition = Point3F::Zero;
   mHasLastPosition = false;

   mSkipUpdateCount = 0;
   mSkippedUpdates = 0;
   mTSU = 0;

   // ParticleSystem should be allocated on the client only.
   mNetFlags.set(IsGhost);
}

ParticleSystem::~ParticleSystem()
{
   delete mEmitter;
   delete mRenderer;
   delete mParticlePool;
}

//-----------------------------------------------------------------------------
// emitParticles
//-----------------------------------------------------------------------------
void ParticleSystem::emitParticles(const Point3F& point,
   const bool     useLastPosition,
   const Point3F& axis,
   const Point3F& velocity,
   const U32      numMilliseconds)
{
   if (mDead) return;

   // lifetime over - no more particles
   if (mDataBlock->mLifetimeMS > 0 && mElapsedTimeMS > mDataBlock->mLifetimeMS)
   {
      return;
   }

   Point3F realStart;
   if (useLastPosition && mHasLastPosition)
      realStart = mLastPosition;
   else
      realStart = point;

   emitParticles(realStart, point,
      axis,
      velocity,
      numMilliseconds);
}

//-----------------------------------------------------------------------------
// emitParticles
//-----------------------------------------------------------------------------
void ParticleSystem::emitParticles(const Point3F& start,
   const Point3F& end,
   const Point3F& axis,
   const Point3F& velocity,
   const U32      numMilliseconds)
{
   if (mDead) return;

   //if( mDataBlock->particleDataBlocks.empty() )
   //   return;

   // lifetime over - no more particles
   if (mDataBlock->mLifetimeMS > 0 && mElapsedTimeMS > mDataBlock->mLifetimeMS)
   {
      return;
   }

   U32 currTime = 0;
   bool particlesAdded = false;

   Point3F axisx;
   if (mFabs(axis.z) < 0.9f)
      mCross(axis, Point3F(0, 0, 1), &axisx);
   else
      mCross(axis, Point3F(0, 1, 0), &axisx);
   axisx.normalize();

   if (mNextParticleTime != 0)
   {
      // Need to handle next particle
      //
      if (mNextParticleTime > numMilliseconds)
      {
         // Defer to next update
         //  (Note that this introduces a potential spatial irregularity if the owning
         //   object is accelerating, and updating at a low frequency)
         //
         mNextParticleTime -= numMilliseconds;
         mInternalClock += numMilliseconds;
         mLastPosition = end;
         mHasLastPosition = true;
         setPosition(mLastPosition);
         return;
      }
      else
      {
         currTime += mNextParticleTime;
         mInternalClock += mNextParticleTime;
         // Emit particle at curr time

         // Create particle at the correct position
         Point3F pos;
         pos.interpolate(start, end, F32(currTime) / F32(numMilliseconds));
         if (mEmitter->addParticle(pos, axis, velocity, axisx))
            particlesAdded = true;
         mNextParticleTime = 0;
      }
   }

   while (currTime < numMilliseconds)
   {
      S32 pps = mMax(1, mDataBlock->mParticlesPerSecond);
      S32 ppsVariance = 1.0f / (1 + mDataBlock->mParticlesPerSecondVariance);
      if (ppsVariance != 0)
      {
         pps += S32(gRandGen.randI() % (2 * ppsVariance + 1)) -
            S32(ppsVariance);
      }
      S32 nextTime = 1000.0f / mClamp(pps, 1, 1000);
      AssertFatal(nextTime > 0, "Error, next particle ejection time must always be greater than 0");

      if (currTime + nextTime > numMilliseconds)
      {
         mNextParticleTime = (currTime + nextTime) - numMilliseconds;
         mInternalClock += numMilliseconds - currTime;
         AssertFatal(mNextParticleTime > 0, "Error, should not have deferred this particle!");
         break;
      }

      currTime += nextTime;
      mInternalClock += nextTime;

      // Create particle at the correct position
      Point3F pos;
      pos.interpolate(start, end, F32(currTime) / F32(numMilliseconds));
      if (mEmitter->addParticle(pos, axis, velocity, axisx))
         particlesAdded = true;

      //   This override-advance code is restored in order to correctly adjust
      //   animated parameters of particles allocated within the same frame
      //   update. Note that ordering is important and this code correctly 
      //   adds particles in the same newest-to-oldest ordering of the link-list.
      //
      // NOTE: We are assuming that the just added particle is at the head of our
      //  list.  If that changes, so must this...
      U32 advanceMS = numMilliseconds - currTime;
      if (mDataBlock->mOverrideAdvance == false && advanceMS != 0 && particlesAdded)
      {
         Particle* last_part = mParticlePool->GetParticleHead()->next;
         if (advanceMS > last_part->totalLifetime)
         {
            mParticlePool->RemoveParticle(mParticlePool->GetParticleHead());
         }
         else
         {
            if (advanceMS != 0)
            {
               F32 t = F32(advanceMS) / 1000.0;

               Point3F a = last_part->acc;
               a -= last_part->vel * mDataBlock->mDragCoefficient;
               a -= mWindVelocity * mDataBlock->mWindCoefficient;
               a += Point3F(0.0f, 0.0f, -9.81f) * mDataBlock->mGravityCoefficient;

               last_part->vel += a * t;
               last_part->pos += last_part->vel * t;
            }
         }
      }
      /*
      // TODO: find out what this does and implement it.
      // DMMFIX: Lame and slow...
      if( particlesAdded == true )
      updateBBox();
      */
   }


   if (mParticlePool->getCount() > 0 && getSceneManager() == NULL)
   {
      gClientSceneGraph->addObjectToScene(this);
      ClientProcessList::get()->addObject(this);
   }

   mLastPosition = end;
   mHasLastPosition = true;
   setPosition(mLastPosition);
}

//-----------------------------------------------------------------------------
// emitParticles
//-----------------------------------------------------------------------------
void ParticleSystem::emitParticles(const Point3F& rCenter,
   const Point3F& rNormal,
   const F32      radius,
   const Point3F& velocity,
   S32 count)
{
   if (mDead) return;

   // lifetime over - no more particles
   if (mDataBlock->mLifetimeMS > 0 && mElapsedTimeMS > mDataBlock->mLifetimeMS)
   {
      return;
   }


   Point3F axisx, axisy;
   Point3F axisz = rNormal;

   if (axisz.isZero())
   {
      axisz.set(0.0, 0.0, 1.0);
   }

   if (mFabs(axisz.z) < 0.98)
   {
      mCross(axisz, Point3F(0, 0, 1), &axisy);
      axisy.normalize();
   }
   else
   {
      mCross(axisz, Point3F(0, 1, 0), &axisy);
      axisy.normalize();
   }
   mCross(axisz, axisy, &axisx);
   axisx.normalize();

   // Should think of a better way to distribute the
   // particles within the hemisphere.
   for (S32 i = 0; i < count; i++)
   {
      Point3F pos = axisx * (radius * (1 - (2 * gRandGen.randF())));
      pos += axisy * (radius * (1 - (2 * gRandGen.randF())));
      pos += axisz * (radius * gRandGen.randF());

      Point3F axis = pos;
      axis.normalize();
      pos += rCenter;

      mEmitter->addParticle(pos, axis, velocity, axisz);
   }

   // Set world bounding box
   mObjBox.minExtents = rCenter - Point3F(radius, radius, radius);
   mObjBox.maxExtents = rCenter + Point3F(radius, radius, radius);
   resetWorldBox();

   // Make sure we're part of the world
   if (mParticlePool->getCount() > 0 && getSceneManager() == NULL)
   {
      gClientSceneGraph->addObjectToScene(this);
      ClientProcessList::get()->addObject(this);
   }

   mHasLastPosition = false;
}

//-----------------------------------------------------------------------------
// prepRenderImage
//-----------------------------------------------------------------------------
void ParticleSystem::prepRenderImage(SceneRenderState* state)
{
   if (!mRenderer->renderPool(mParticlePool, state))
      return;

   // TODO: Debug render code here
}

//-----------------------------------------------------------------------------
// processTick
//-----------------------------------------------------------------------------
void ParticleSystem::processTick(Move const*)
{
   if (mDeleteOnTick == true)
   {
      mDead = true;
      deleteObject();
   }
   else{
      // LOD
      if (isClientObject() && !mDeleteOnTick && !mDead)
      {
         if (Con::getBoolVariable("$ParticleSystem::ParticleLOD"))
         {
            GameConnection* gConnection = GameConnection::getLocalClientConnection();
            MatrixF camTrans;
            gConnection->getControlCameraTransform(0, &camTrans);
            F32 dist = (mLastPosition - camTrans.getPosition()).len();
            if (dist >= mDataBlock->mSimulationLODBegin)
            {
               F32 lodCoeff = (dist - mDataBlock->mSimulationLODBegin) / (mDataBlock->mSimulationLODEnd - mDataBlock->mSimulationLODBegin);
               mSkipUpdateCount = lodCoeff * 100;
               if (mSkipUpdateCount == 0)
                  mSkipUpdateCount = 1;
               if (mSkipUpdateCount > 100)
                  mSkipUpdateCount = 100;
            }

            /*if(dist >= mDataBlock->mEjectionLODStartDistance)
            {
            S32 ejPeriod;
            standAloneEmitter ? ejPeriod = sa_ejectionPeriodMS : ejPeriod = mDataBlock->ejectionPeriodMS;
            if(dist >= mDataBlock->ejectionLODEndDistance)
            mActive = false;
            else
            mActive = true;
            F32 lodCoeff = (dist - mDataBlock->ejectionLODStartDistance) / (mDataBlock->ejectionLODEndDistance - mDataBlock->ejectionLODStartDistance);
            RenderEjectionPeriodMS = ejPeriod + ((mDataBlock->lodEjectionPeriod - ejPeriod) * lodCoeff);
            }
            else
            RenderEjectionPeriodMS = standAloneEmitter ? sa_ejectionPeriodMS : mDataBlock->ejectionPeriodMS;*/
         }
         //else
         //   RenderEjectionPeriodMS = standAloneEmitter ? sa_ejectionPeriodMS : mDataBlock->ejectionPeriodMS;
      }
   }
}

//-----------------------------------------------------------------------------
// advanceTime
//-----------------------------------------------------------------------------
void ParticleSystem::advanceTime(F32 dt)
{
   if (dt < 0.00001) return;

   Parent::advanceTime(dt);

   if (dt > 0.5) dt = 0.5;

   if (mDead) return;

   mElapsedTimeMS += (S32)(dt * 1000.0f);

   U32 numMSToUpdate = (U32)(dt * 1000.0f);
   if (numMSToUpdate == 0) return;

   // TODO: Prefetch

   mParticlePool->AdvanceTime(numMSToUpdate);

   if (mParticlePool->getCount() < 1 && mDeleteWhenEmpty)
   {
      mDeleteOnTick = true;
      return;
   }

   if (numMSToUpdate != 0 && mParticlePool->getCount() > 0)
   {
      simulate(numMSToUpdate);
   }
}

//-----------------------------------------------------------------------------
// Update particles
//-----------------------------------------------------------------------------
void ParticleSystem::simulate(U32 ms)
{
   // TODO: Prefetch
   if (Con::getBoolVariable("$ParticleSystem::ParticleLOD"))
   {
      if (mSkipUpdateCount < 1000)
      {
         if (mSkippedUpdates == 0)
            mSkippedUpdates = 1;
         if (mSkippedUpdates < mSkipUpdateCount)
         {
            mSkippedUpdates++;
            mTSU += ms;
            return;
         }
         if (mSkippedUpdates >= mSkipUpdateCount)
         {
            mSkippedUpdates = 1;
            ms += mTSU;
            mTSU = 0;
         }
      }
      else return;
   }

   IParticleBehaviour **BHVs = getDataBlock()->ParticleBHVs;
   // TODO: WHAT SO MANY SORTS PER SECOND? WTF DO A DIRTY FLAG!
   dQsort(BHVs, ParticleBehaviourCount, sizeof(IParticleBehaviour*), &IParticleBehaviour::compare);

   for (Particle* part = mParticlePool->GetParticleHead()->next; part != NULL; part = part->next)
   {
      F32 t = F32(ms) / 1000.0;
      // TODO: Don't overwrite acceleration
      part->acc.zero();
      // First calculate the acceleration of the particle.
      for (int i = 0; i < ParticleBehaviourCount; i++)
      {
         IParticleBehaviour* bhv = BHVs[i];
         if (!bhv)
            continue;
         if (bhv->getType() == behaviourType::Acceleration)
            bhv->updateParticle(this, part, t);
      }

      // Apply physics constants to the acceleration
      Point3F a = part->acc;
      a -= part->vel * mDataBlock->mDragCoefficient;
      a -= mWindVelocity * mDataBlock->mWindCoefficient;
      a += Point3F(0.0f, 0.0f, -9.81f) * mDataBlock->mGravityCoefficient;

      // Acceleration multiplied by time gives the amount of increased velocity
      part->vel += a * t;
      for (int i = 0; i < ParticleBehaviourCount; i++)
      {
         IParticleBehaviour* bhv = BHVs[i];
         if (!bhv)
            continue;
         if (bhv->getType() == behaviourType::Velocity)
            bhv->updateParticle(this, part, t);
      }

      // Velocity multiplied by time gives the distance the particle has travelled
      part->relPos += part->vel * t;
      part->pos += part->vel * t;
      for (int i = 0; i < ParticleBehaviourCount; i++)
      {
         IParticleBehaviour* bhv = BHVs[i];
         if (!bhv)
            continue;
         if (bhv->getType() == behaviourType::Position)
            bhv->updateParticle(this, part, t);
      }
   }
}

//-----------------------------------------------------------------------------
// deleteWhenEmpty
// - Makes sure that the ParticleSystem is deleted but only after all the 
//   particles has died.
//-----------------------------------------------------------------------------
void ParticleSystem::deleteWhenEmpty()
{
   // if the following asserts fire, there is a reasonable chance that you are trying to delete a particle emitter
   // that has already been deleted (possibly by ClientMissionCleanup). If so, use a SimObjectPtr to the emitter and check it
   // for null before calling this function.
   AssertFatal(isProperlyAdded(), "ParticleSystem must be registed before calling deleteWhenEmpty");
   AssertFatal(!mDead, "ParticleSystem already deleted");
   AssertFatal(!isDeleted(), "ParticleSystem already deleted");
   AssertFatal(!isRemoved(), "ParticleSystem already removed");

   // this check is for non debug case, so that we don't write in to freed memory
   bool okToDelete = !mDead && isProperlyAdded() && !isDeleted() && !isRemoved();
   if (okToDelete)
   {
      mDeleteWhenEmpty = true;
      if (!mParticlePool->getCount())
      {
         // We're already empty, so delete us now.

         mDead = true;
         deleteObject();
      }
      else
         AssertFatal(getSceneManager() != NULL, "ParticleSystem not on process list and won't get ticked to death");
   }
}

//-----------------------------------------------------------------------------
// onNewDataBlock
//-----------------------------------------------------------------------------
bool ParticleSystem::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<ParticleSystemData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;
   if (mDataBlock->mEmitterData
      && mDataBlock->mRendererData)
   {
      mEmitter = mDataBlock->mEmitterData->CreateEmitter(this);
      // Max lifetime divided by minimum time between each emitted particle.
      U32 partListInitSize = (mDataBlock->mPartLifetimeMS + mDataBlock->mPartLifetimeVarianceMS)
         * (1000 / (mDataBlock->mParticlesPerSecond + mDataBlock->mParticlesPerSecondVariance));
      partListInitSize += 8; // add 8 as "fudge factor" to make sure it doesn't realloc if it goes over by 1
      mParticlePool = new ParticlePool(partListInitSize);
      mRenderer = mDataBlock->mRendererData->CreateRenderer(this);
   }

}

//-----------------------------------------------------------------------------
// onNewDataBlock
//-----------------------------------------------------------------------------
bool ParticleSystem::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // add to client side mission cleanup
   SimGroup *cleanup = dynamic_cast<SimGroup *>(Sim::findObject("ClientMissionCleanup"));
   if (cleanup != NULL)
   {
      cleanup->addObject(this);
   }

   removeFromProcessList();

   F32 radius = 5.0;
   mObjBox.minExtents = Point3F(-radius, -radius, -radius);
   mObjBox.maxExtents = Point3F(radius, radius, radius);
   resetWorldBox();

   return true;
}

//-----------------------------------------------------------------------------
// onRemove
//-----------------------------------------------------------------------------
void ParticleSystem::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}