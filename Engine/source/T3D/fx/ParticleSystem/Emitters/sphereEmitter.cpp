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

#include "sphereEmitter.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

IMPLEMENT_CO_DATABLOCK_V1(SphereEmitterData);

static const float sgDefaultEjectionOffset = 0.f;
static const float sgDefaultPhiReferenceVel = 0.f;
static const float sgDefaultPhiVariance = 360.f;

SphereEmitterData::SphereEmitterData()
{
   mThetaMax = 0.0f; // All heights
   mThetaMin = 90.0f;
   mPhiReferenceVel = sgDefaultPhiReferenceVel; // All directions
   mPhiVariance = sgDefaultPhiVariance;
   mEjectionVelocity = 2.0f;
   mVelocityVariance = 1.0f;
   mEjectionOffset = sgDefaultEjectionOffset;
   mEjectionOffsetVariance = 0.0f;
}

ParticleEmitter* SphereEmitterData::CreateEmitter(ParticleSystem* system)
{
   SphereEmitter *emitter = new SphereEmitter(system);
   emitter->setDataBlock(this);
   return emitter;
}

void SphereEmitterData::initPersistFields()
{
   addGroup("SphereEmitterData");

   addField("ThetaMax", TypeF32, Offset(mThetaMax, SphereEmitterData),
      "Maximum angle, from the horizontal plane, to eject particles from.");

   addField("ThetaMin", TypeF32, Offset(mThetaMin, SphereEmitterData),
      "Minimum angle, from the horizontal plane, to eject from.");

   addField("PhiReferenceVel", TypeF32, Offset(mPhiReferenceVel, SphereEmitterData),
      "Reference angle, from the vertical plane, to eject particles from.");

   addField("PhiVariance", TypeF32, Offset(mPhiVariance, SphereEmitterData),
      "Variance from the reference angle, from 0 - 360.");

   addField("EjectionVelocity", TypeF32, Offset(mEjectionVelocity, SphereEmitterData),
      "Particle ejection velocity.");

   addField("VelocityVariance", TypeF32, Offset(mVelocityVariance, SphereEmitterData),
      "Variance for ejection velocity, from 0 - ejectionVelocity.");

   addField("EjectionOffset", TypeF32, Offset(mEjectionOffset, SphereEmitterData),
      "Distance along ejection Z axis from which to eject particles.");

   addField("EjectionOffsetVariance", TypeF32, Offset(mEjectionOffsetVariance, SphereEmitterData),
      "Distance Padding along ejection Z axis from which to eject particles.");

   endGroup("SphereEmitterData");

   Parent::initPersistFields();
}

void SphereEmitterData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeInt((S32)(mEjectionVelocity * 100), 16);
   stream->writeInt((S32)(mVelocityVariance * 100), 14);
   if (stream->writeFlag(mEjectionOffset != sgDefaultEjectionOffset))
      stream->writeInt((S32)(mEjectionOffset * 100), 16);
   if (stream->writeFlag(mEjectionOffsetVariance != 0.0f))
      stream->writeInt((S32)(mEjectionOffsetVariance * 100), 16);
   stream->writeRangedU32((U32)mThetaMin, 0, 180);
   stream->writeRangedU32((U32)mThetaMax, 0, 180);
   if (stream->writeFlag(mPhiReferenceVel != sgDefaultPhiReferenceVel))
      stream->writeRangedU32((U32)mPhiReferenceVel, 0, 360);
   if (stream->writeFlag(mPhiVariance != sgDefaultPhiVariance))
      stream->writeRangedU32((U32)mPhiVariance, 0, 360);
}

void SphereEmitterData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   mEjectionVelocity = stream->readInt(16) / 100.0f;
   mVelocityVariance = stream->readInt(14) / 100.0f;
   if (stream->readFlag())
      mEjectionOffset = stream->readInt(16) / 100.0f;
   else
      mEjectionOffset = sgDefaultEjectionOffset;
   if (stream->readFlag())
      mEjectionOffsetVariance = stream->readInt(16) / 100.0f;
   else
      mEjectionOffsetVariance = 0.0f;
   mThetaMin = (F32)stream->readRangedU32(0, 180);
   mThetaMax = (F32)stream->readRangedU32(0, 180);
   if (stream->readFlag())
      mPhiReferenceVel = (F32)stream->readRangedU32(0, 360);
   else
      mPhiReferenceVel = sgDefaultPhiReferenceVel;

   if (stream->readFlag())
      mPhiVariance = (F32)stream->readRangedU32(0, 360);
   else
      mPhiVariance = sgDefaultPhiVariance;
}

bool SphereEmitter::addParticle(Point3F const& pos,
   Point3F const& axis,
   Point3F const& vel,
   Point3F const& axisx)
{
   SphereEmitterData* DataBlock = getDataBlock();
   Particle* pNew;
   mParentSystem->getParticlePool()->AddParticle(pNew);
   Point3F ejectionAxis = axis;

   F32 theta = (DataBlock->getThetaMax() - DataBlock->getThetaMin()) * gRandGen.randF() +
      DataBlock->getThetaMin();

   F32 ref = F32(mParentSystem->getInternalClock()) / 1000.0 * DataBlock->getPhiReferenceVel();
   F32 phi = ref + gRandGen.randF() * DataBlock->getPhiVariance();

   // Both phi and theta are in degs.  Create axis angles out of them, and create the
   //  appropriate rotation matrix...
   AngAxisF thetaRot(axisx, theta * (M_PI / 180.0));
   AngAxisF phiRot(axis, phi   * (M_PI / 180.0));

   MatrixF temp(true);
   thetaRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);
   phiRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);

   F32 initialVel = DataBlock->getEjectionVelocity();
   initialVel += (DataBlock->getVelocityVariance() * 2.0f * gRandGen.randF()) - DataBlock->getVelocityVariance();

   pNew->relPos = (ejectionAxis * (DataBlock->getEjectionOffset() + DataBlock->getEjectionOffsetVariance()* gRandGen.randF()));
   pNew->pos = pos + pNew->relPos;
   pNew->vel = ejectionAxis * initialVel;
   pNew->orientDir = ejectionAxis;
   pNew->acc.set(0, 0, 0);
   pNew->currentAge = 0;

   // Calculate the constant accleration...
   pNew->vel += vel * mParentSystem->getDataBlock()->getInheritedVelFactor();
   pNew->acc = pNew->vel * mParentSystem->getDataBlock()->getConstantAcceleration();

   // Calculate this instance's lifetime...
   pNew->totalLifetime = mParentSystem->getDataBlock()->getPartLifetimeMS();
   if (mParentSystem->getDataBlock()->getPartLifetimeVarianceMS() != 0)
      pNew->totalLifetime += S32(gRandGen.randI() % (2 * mParentSystem->getDataBlock()->getPartLifetimeVarianceMS() + 1)) - S32(mParentSystem->getDataBlock()->getPartLifetimeVarianceMS());
   // assign spin amount
   pNew->spinSpeed = mParentSystem->getDataBlock()->getSpinSpeed() * gRandGen.randF(mParentSystem->getDataBlock()->getSpinRandomMin(), mParentSystem->getDataBlock()->getSpinRandomMax());

   return true;
}