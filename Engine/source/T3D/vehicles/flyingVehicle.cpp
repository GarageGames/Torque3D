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
#include "T3D/vehicles/flyingVehicle.h"

#include "app/game.h"
#include "math/mMath.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "collision/clippedPolyList.h"
#include "collision/planeExtractor.h"
#include "core/stream/bitStream.h"
#include "core/dnet.h"
#include "T3D/gameBase/gameConnection.h"
#include "ts/tsShapeInstance.h"
#include "T3D/fx/particleEmitter.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxSource.h"
#include "T3D/missionArea.h"

//----------------------------------------------------------------------------

const static U32 sCollisionMoveMask = ( TerrainObjectType | WaterObjectType          | 
                                        PlayerObjectType  | StaticShapeObjectType    | 
                                        VehicleObjectType | VehicleBlockerObjectType );
                                        
static U32 sServerCollisionMask = sCollisionMoveMask; // ItemObjectType
static U32 sClientCollisionMask = sCollisionMoveMask;

static F32 sFlyingVehicleGravity = -20.0f;

//
const char* FlyingVehicle::sJetSequence[FlyingVehicle::JetAnimCount] =
{
   "activateBack",
   "maintainBack",
   "activateBot",
   "maintainBot",
};

const char* FlyingVehicleData::sJetNode[FlyingVehicleData::MaxJetNodes] =
{
   "JetNozzle0",  // Thrust Forward
   "JetNozzle1",
   "JetNozzleX",  // Thrust Backward
   "JetNozzleY",
   "JetNozzle2",  // Thrust Downward
   "JetNozzle3",
   "contrail0",   // Trail
   "contrail1",
   "contrail2",
   "contrail3",
};

// Convert thrust direction into nodes & emitters
FlyingVehicle::JetActivation FlyingVehicle::sJetActivation[NumThrustDirections] = {
   { FlyingVehicleData::ForwardJetNode, FlyingVehicleData::ForwardJetEmitter },
   { FlyingVehicleData::BackwardJetNode, FlyingVehicleData::BackwardJetEmitter },
   { FlyingVehicleData::DownwardJetNode, FlyingVehicleData::DownwardJetEmitter },
};


//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(FlyingVehicleData);

ConsoleDocClass( FlyingVehicleData,
   "@brief Defines the properties of a FlyingVehicle.\n\n"
   "@ingroup Vehicles\n"
);

FlyingVehicleData::FlyingVehicleData()
{
   maneuveringForce = 0;
   horizontalSurfaceForce = 0;
   verticalSurfaceForce = 0;
   autoInputDamping = 1;
   steeringForce = 1;
   steeringRollForce = 1;
   rollForce = 1;
   autoAngularForce = 0;
   rotationalDrag = 0;
   autoLinearForce = 0;
   maxAutoSpeed = 0;
   hoverHeight = 2;
   createHoverHeight = 2;
   maxSteeringAngle = M_PI_F;
   minTrailSpeed = 1;
   maxSpeed = 100;

   for (S32 k = 0; k < MaxJetNodes; k++)
      jetNode[k] = -1;

   for (S32 j = 0; j < MaxJetEmitters; j++)
      jetEmitter[j] = 0;

   for (S32 i = 0; i < MaxSounds; i++)
      sound[i] = 0;

   vertThrustMultiple = 1.0;
}

bool FlyingVehicleData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   TSShapeInstance* si = new TSShapeInstance(mShape, false);

   // Resolve objects transmitted from server
   if (!server) {
      for (S32 i = 0; i < MaxSounds; i++)
         if (sound[i])
            Sim::findObject(SimObjectId(sound[i]),sound[i]);

      for (S32 j = 0; j < MaxJetEmitters; j++)
         if (jetEmitter[j])
            Sim::findObject(SimObjectId(jetEmitter[j]),jetEmitter[j]);
   }

   // Extract collision planes from shape collision detail level
   if (collisionDetails[0] != -1)
   {
      MatrixF imat(1);
      PlaneExtractorPolyList polyList;
      polyList.mPlaneList = &rigidBody.mPlaneList;
      polyList.setTransform(&imat, Point3F(1,1,1));
      si->animate(collisionDetails[0]);
      si->buildPolyList(&polyList,collisionDetails[0]);
   }

   // Resolve jet nodes
   for (S32 j = 0; j < MaxJetNodes; j++)
      jetNode[j] = mShape->findNode(sJetNode[j]);

   //
   maxSpeed = maneuveringForce / minDrag;

   delete si;
   return true;
}

void FlyingVehicleData::initPersistFields()
{
   addField( "jetSound", TYPEID< SFXProfile >(), Offset(sound[JetSound], FlyingVehicleData),
      "Looping sound to play while the vehicle is jetting." );
   addField( "engineSound", TYPEID< SFXProfile >(), Offset(sound[EngineSound], FlyingVehicleData),
      "Looping engine sound." );

   addField( "maneuveringForce", TypeF32, Offset(maneuveringForce, FlyingVehicleData),
      "@brief Maximum X and Y (horizontal plane) maneuvering force.\n\n"
      "The actual force applied depends on the current thrust." );
   addField( "horizontalSurfaceForce", TypeF32, Offset(horizontalSurfaceForce, FlyingVehicleData),
      "@brief Damping force in the opposite direction to sideways velocity.\n\n"
      "Provides \"bite\" into the wind for climbing/diving and turning)." );
   addField( "verticalSurfaceForce", TypeF32, Offset(verticalSurfaceForce, FlyingVehicleData),
      "@brief Damping force in the opposite direction to vertical velocity.\n\n"
      "Controls side slip; lower numbers give more slide." );
   addField( "vertThrustMultiple", TypeF32, Offset(vertThrustMultiple, FlyingVehicleData),
      "Multiplier applied to the jetForce (defined in VehicleData) when thrusting vertically." );
   addField( "steeringForce", TypeF32, Offset(steeringForce, FlyingVehicleData),
      "@brief Maximum X and Z (sideways and vertical) steering force.\n\n"
      "The actual force applied depends on the current steering input." );
   addField( "steeringRollForce", TypeF32, Offset(steeringRollForce, FlyingVehicleData),
      "Roll force induced by sideways steering input value (controls how much "
      "the vehicle rolls when turning)." );
   addField( "rollForce", TypeF32, Offset(rollForce, FlyingVehicleData),
      "@brief Damping torque against rolling maneuvers (rotation about the y-axis), "
      "proportional to linear velocity.\n\n"
      "Acts to adjust roll to a stable position over time as the vehicle moves." );
   addField( "rotationalDrag", TypeF32, Offset(rotationalDrag, FlyingVehicleData),
      "Rotational drag factor (slows vehicle rotation speed in all axes)." );

   addField( "maxAutoSpeed", TypeF32, Offset(maxAutoSpeed, FlyingVehicleData),
      "Maximum speed for automatic vehicle control assistance - vehicles "
      "travelling at speeds above this value do not get control assitance." );
   addField( "autoInputDamping", TypeF32, Offset(autoInputDamping, FlyingVehicleData),
      "@brief Scale factor applied to steering input if speed is less than "
      "maxAutoSpeed to.improve handling at very low speeds.\n\n"
      "Smaller values make steering less sensitive." );
   addField( "autoLinearForce", TypeF32, Offset(autoLinearForce, FlyingVehicleData),
      "@brief Corrective force applied to slow the vehicle when moving at less than "
      "maxAutoSpeed.\n\n"
      "The force is inversely proportional to vehicle speed." );
   addField( "autoAngularForce", TypeF32, Offset(autoAngularForce, FlyingVehicleData),
      "@brief Corrective torque applied to level out the vehicle when moving at less "
      "than maxAutoSpeed.\n\n"
      "The torque is inversely proportional to vehicle speed." );

   addField( "hoverHeight", TypeF32, Offset(hoverHeight, FlyingVehicleData),
      "The vehicle's height off the ground when at rest." );
   addField( "createHoverHeight", TypeF32, Offset(createHoverHeight, FlyingVehicleData),
      "@brief The vehicle's height off the ground when useCreateHeight is active.\n\n"
      "This can help avoid problems with spawning the vehicle." );

   addField( "forwardJetEmitter",TYPEID< ParticleEmitterData >(), Offset(jetEmitter[ForwardJetEmitter], FlyingVehicleData),
      "@brief Emitter to generate particles for forward jet thrust.\n\n"
      "Forward jet thrust particles are emitted from model nodes JetNozzle0 "
      "and JetNozzle1." );
   addField( "backwardJetEmitter",TYPEID< ParticleEmitterData >(), Offset(jetEmitter[BackwardJetEmitter], FlyingVehicleData),
      "@brief Emitter to generate particles for backward jet thrust.\n\n"
      "Backward jet thrust particles are emitted from model nodes JetNozzleX "
      "and JetNozzleY." );
   addField( "downJetEmitter",TYPEID< ParticleEmitterData >(), Offset(jetEmitter[DownwardJetEmitter], FlyingVehicleData),
      "@brief Emitter to generate particles for downward jet thrust.\n\n"
      "Downward jet thrust particles are emitted from model nodes JetNozzle2 "
      "and JetNozzle3." );
   addField( "trailEmitter",TYPEID< ParticleEmitterData >(), Offset(jetEmitter[TrailEmitter], FlyingVehicleData),
      "Emitter to generate contrail particles from model nodes contrail0 - contrail3." );
   addField( "minTrailSpeed", TypeF32, Offset(minTrailSpeed, FlyingVehicleData),
      "Minimum speed at which to start generating contrail particles." );

   Parent::initPersistFields();
}

void FlyingVehicleData::packData(BitStream* stream)
{
   Parent::packData(stream);

   for (S32 i = 0; i < MaxSounds; i++)
   {
      if (stream->writeFlag(sound[i]))
      {
         SimObjectId writtenId = packed ? SimObjectId(sound[i]) : sound[i]->getId();
         stream->writeRangedU32(writtenId, DataBlockObjectIdFirst, DataBlockObjectIdLast);
      }
   }

   for (S32 j = 0; j < MaxJetEmitters; j++)
   {
      if (stream->writeFlag(jetEmitter[j]))
      {
         SimObjectId writtenId = packed ? SimObjectId(jetEmitter[j]) : jetEmitter[j]->getId();
         stream->writeRangedU32(writtenId, DataBlockObjectIdFirst,DataBlockObjectIdLast);
      }
   }

   stream->write(maneuveringForce);
   stream->write(horizontalSurfaceForce);
   stream->write(verticalSurfaceForce);
   stream->write(autoInputDamping);
   stream->write(steeringForce);
   stream->write(steeringRollForce);
   stream->write(rollForce);
   stream->write(autoAngularForce);
   stream->write(rotationalDrag);
   stream->write(autoLinearForce);
   stream->write(maxAutoSpeed);
   stream->write(hoverHeight);
   stream->write(createHoverHeight);
   stream->write(minTrailSpeed);
   stream->write(vertThrustMultiple);
}

void FlyingVehicleData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   for (S32 i = 0; i < MaxSounds; i++) {
      sound[i] = NULL;
      if (stream->readFlag())
         sound[i] = (SFXProfile*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                         DataBlockObjectIdLast);
   }

   for (S32 j = 0; j < MaxJetEmitters; j++) {
      jetEmitter[j] = NULL;
      if (stream->readFlag())
         jetEmitter[j] = (ParticleEmitterData*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                                     DataBlockObjectIdLast);
   }

   stream->read(&maneuveringForce);
   stream->read(&horizontalSurfaceForce);
   stream->read(&verticalSurfaceForce);
   stream->read(&autoInputDamping);
   stream->read(&steeringForce);
   stream->read(&steeringRollForce);
   stream->read(&rollForce);
   stream->read(&autoAngularForce);
   stream->read(&rotationalDrag);
   stream->read(&autoLinearForce);
   stream->read(&maxAutoSpeed);
   stream->read(&hoverHeight);
   stream->read(&createHoverHeight);
   stream->read(&minTrailSpeed);
   stream->read(&vertThrustMultiple);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(FlyingVehicle);

ConsoleDocClass( FlyingVehicle,
   "@brief A flying vehicle.\n\n"
   "@ingroup Vehicles\n"
);

FlyingVehicle::FlyingVehicle()
{
   mSteering.set(0,0);
   mThrottle = 0;
   mJetting = false;

   mJetSound = 0;
   mEngineSound = 0;

   mBackMaintainOn = false;
   mBottomMaintainOn = false;
   createHeightOn = false;

   for (S32 i = 0; i < JetAnimCount; i++)
      mJetThread[i] = 0;
}

FlyingVehicle::~FlyingVehicle()
{
}


//----------------------------------------------------------------------------

bool FlyingVehicle::onAdd()
{
   if(!Parent::onAdd())
      return false;

   addToScene();

   if (isServerObject())
      scriptOnAdd();
   return true;
}

bool FlyingVehicle::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<FlyingVehicleData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
      return false;

   // Sounds
   if ( isGhost() ) 
   {
      // Create the sounds ahead of time.  This reduces runtime
      // costs and makes the system easier to understand.

      SFX_DELETE( mJetSound );
      SFX_DELETE( mEngineSound );

      if ( mDataBlock->sound[FlyingVehicleData::EngineSound] )
         mEngineSound = SFX->createSource( mDataBlock->sound[FlyingVehicleData::EngineSound], &getTransform() );

      if ( mDataBlock->sound[FlyingVehicleData::JetSound] )
         mJetSound = SFX->createSource( mDataBlock->sound[FlyingVehicleData::JetSound], &getTransform() );
   }

   // Jet Sequences
   for (S32 i = 0; i < JetAnimCount; i++) {
      TSShape const* shape = mShapeInstance->getShape();
      mJetSeq[i] = shape->findSequence(sJetSequence[i]);
      if (mJetSeq[i] != -1) {
         if (i == BackActivate || i == BottomActivate) {
            mJetThread[i] = mShapeInstance->addThread();
            mShapeInstance->setSequence(mJetThread[i],mJetSeq[i],0);
            mShapeInstance->setTimeScale(mJetThread[i],0);
         }
      }
      else
         mJetThread[i] = 0;
   }

   scriptOnNewDataBlock();
   return true;
}

void FlyingVehicle::onRemove()
{
   SFX_DELETE( mJetSound );
   SFX_DELETE( mEngineSound );

   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}


//----------------------------------------------------------------------------

void FlyingVehicle::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   updateEngineSound(1);
   updateJet(dt);
}


//----------------------------------------------------------------------------

void FlyingVehicle::updateMove(const Move* move)
{
   PROFILE_SCOPE( FlyingVehicle_UpdateMove );

   Parent::updateMove(move);

   if (move == &NullMove)
      mSteering.set(0,0);

   F32 speed = mRigid.linVelocity.len();
   if (speed < mDataBlock->maxAutoSpeed)
      mSteering *= mDataBlock->autoInputDamping;

   // Check the mission area to get the factor for the flight ceiling
   MissionArea * obj = MissionArea::getServerObject();
   mCeilingFactor = 1.0f;
   if (obj != NULL)
   {
      F32 flightCeiling = obj->getFlightCeiling();
      F32 ceilingRange  = obj->getFlightCeilingRange();

      if (mRigid.linPosition.z > flightCeiling)
      {
         // Thrust starts to fade at the ceiling, and is 0 at ceil + range
         if (ceilingRange == 0)
         {
            mCeilingFactor = 0;
         }
         else
         {
            mCeilingFactor = 1.0f - ((mRigid.linPosition.z - flightCeiling) / (flightCeiling + ceilingRange));
            if (mCeilingFactor < 0.0f)
               mCeilingFactor = 0.0f;
         }
      }
   }

   mThrust.x = move->x;
   mThrust.y = move->y;

   if (mThrust.y != 0.0f)
      if (mThrust.y > 0)
         mThrustDirection = ThrustForward;
      else
         mThrustDirection = ThrustBackward;
   else
      mThrustDirection = ThrustDown;

   if (mCeilingFactor != 1.0f)
      mJetting = false;
}


//----------------------------------------------------------------------------

void FlyingVehicle::updateForces(F32 /*dt*/)
{
   PROFILE_SCOPE( FlyingVehicle_UpdateForces );

   MatrixF currPosMat;
   mRigid.getTransform(&currPosMat);
   mRigid.atRest = false;

   Point3F massCenter;
   currPosMat.mulP(mDataBlock->massCenter,&massCenter);

   Point3F xv,yv,zv;
   currPosMat.getColumn(0,&xv);
   currPosMat.getColumn(1,&yv);
   currPosMat.getColumn(2,&zv);
   F32 speed = mRigid.linVelocity.len();

   Point3F force  = Point3F(0, 0, sFlyingVehicleGravity * mRigid.mass * mGravityMod);
   Point3F torque = Point3F(0, 0, 0);

   // Drag at any speed
   force  -= mRigid.linVelocity * mDataBlock->minDrag;
   torque -= mRigid.angMomentum * mDataBlock->rotationalDrag;

   // Auto-stop at low speeds
   if (speed < mDataBlock->maxAutoSpeed) {
      F32 autoScale = 1 - speed / mDataBlock->maxAutoSpeed;

      // Gyroscope
      F32 gf = mDataBlock->autoAngularForce * autoScale;
      torque -= xv * gf * mDot(yv,Point3F(0,0,1));

      // Manuevering jets
      F32 sf = mDataBlock->autoLinearForce * autoScale;
      force -= yv * sf * mDot(yv, mRigid.linVelocity);
      force -= xv * sf * mDot(xv, mRigid.linVelocity);
   }

   // Hovering Jet
   F32 vf = -sFlyingVehicleGravity * mRigid.mass * mGravityMod;
   F32 h  = getHeight();
   if (h <= 1) {
      if (h > 0) {
         vf -= vf * h * 0.1;
      } else {
         vf += mDataBlock->jetForce * -h;
      }
   }
   force += zv * vf;

   // Damping "surfaces"
   force -= xv * mDot(xv,mRigid.linVelocity) * mDataBlock->horizontalSurfaceForce;
   force -= zv * mDot(zv,mRigid.linVelocity) * mDataBlock->verticalSurfaceForce;

   // Turbo Jet
   if (mJetting) {
      if (mThrustDirection == ThrustForward)
         force += yv * mDataBlock->jetForce * mCeilingFactor;
      else if (mThrustDirection == ThrustBackward)
         force -= yv * mDataBlock->jetForce * mCeilingFactor;
      else
         force += zv * mDataBlock->jetForce * mDataBlock->vertThrustMultiple * mCeilingFactor;
   }

   // Maneuvering jets
   force += yv * (mThrust.y * mDataBlock->maneuveringForce * mCeilingFactor);
   force += xv * (mThrust.x * mDataBlock->maneuveringForce * mCeilingFactor);

   // Steering
   Point2F steering;
   steering.x = mSteering.x / mDataBlock->maxSteeringAngle;
   steering.x *= mFabs(steering.x);
   steering.y = mSteering.y / mDataBlock->maxSteeringAngle;
   steering.y *= mFabs(steering.y);
   torque -= xv * steering.y * mDataBlock->steeringForce;
   torque -= zv * steering.x * mDataBlock->steeringForce;

   // Roll
   torque += yv * steering.x * mDataBlock->steeringRollForce;
   F32 ar = mDataBlock->autoAngularForce * mDot(xv,Point3F(0,0,1));
   ar -= mDataBlock->rollForce * mDot(xv, mRigid.linVelocity);
   torque += yv * ar;

   // Add in force from physical zones...
   force += mAppliedForce;

   // Container buoyancy & drag
   force -= Point3F(0, 0, 1) * (mBuoyancy * sFlyingVehicleGravity * mRigid.mass * mGravityMod);
   force -= mRigid.linVelocity * mDrag;

   //
   mRigid.force  = force;
   mRigid.torque = torque;
}


//----------------------------------------------------------------------------

F32 FlyingVehicle::getHeight()
{
   Point3F sp,ep;
   RayInfo collision;
   F32 height = (createHeightOn) ? mDataBlock->createHoverHeight : mDataBlock->hoverHeight;
   F32 r = 10 + height;
   getTransform().getColumn(3, &sp);
   ep.x = sp.x;
   ep.y = sp.y;
   ep.z = sp.z - r;
   disableCollision();
   if( !mContainer->castRay(sp, ep, sClientCollisionMask, &collision) == true )
      collision.t = 1;
   enableCollision();
   return (r * collision.t - height) / 10;
}


//----------------------------------------------------------------------------
U32 FlyingVehicle::getCollisionMask()
{
   if (isServerObject())
      return sServerCollisionMask;
   else
      return sClientCollisionMask;
}

//----------------------------------------------------------------------------

void FlyingVehicle::updateEngineSound(F32 level)
{
   if ( !mEngineSound )
      return;

   if ( !mEngineSound->isPlaying() )
      mEngineSound->play();

   mEngineSound->setTransform( getTransform() );
   mEngineSound->setVelocity( getVelocity() );

   mEngineSound->setPitch( level );
}

void FlyingVehicle::updateJet(F32 dt)
{
   // Thrust Animation threads
   //  Back
   if (mJetSeq[BackActivate] >=0 ) {
      if(!mBackMaintainOn || mThrustDirection != ThrustForward) {
         if(mBackMaintainOn) {
            mShapeInstance->setPos(mJetThread[BackActivate], 1);
            mShapeInstance->destroyThread(mJetThread[BackMaintain]);
            mBackMaintainOn = false;
         }
         mShapeInstance->setTimeScale(mJetThread[BackActivate],
            (mThrustDirection == ThrustForward)? 1.0f : -1.0f);
         mShapeInstance->advanceTime(dt,mJetThread[BackActivate]);
      }
      if(mJetSeq[BackMaintain] >= 0 && !mBackMaintainOn &&
            mShapeInstance->getPos(mJetThread[BackActivate]) >= 1.0) {
         mShapeInstance->setPos(mJetThread[BackActivate], 0);
         mShapeInstance->setTimeScale(mJetThread[BackActivate], 0);
         mJetThread[BackMaintain] = mShapeInstance->addThread();
         mShapeInstance->setSequence(mJetThread[BackMaintain],mJetSeq[BackMaintain],0);
         mShapeInstance->setTimeScale(mJetThread[BackMaintain],1);
         mBackMaintainOn = true;
      }
      if(mBackMaintainOn)
         mShapeInstance->advanceTime(dt,mJetThread[BackMaintain]);
   }

   // Thrust Animation threads
   //   Bottom
   if (mJetSeq[BottomActivate] >=0 ) {
      if(!mBottomMaintainOn || mThrustDirection != ThrustDown || !mJetting) {
         if(mBottomMaintainOn) {
            mShapeInstance->setPos(mJetThread[BottomActivate], 1);
            mShapeInstance->destroyThread(mJetThread[BottomMaintain]);
            mBottomMaintainOn = false;
         }
         mShapeInstance->setTimeScale(mJetThread[BottomActivate],
            (mThrustDirection == ThrustDown && mJetting)? 1.0f : -1.0f);
         mShapeInstance->advanceTime(dt,mJetThread[BottomActivate]);
      }
      if(mJetSeq[BottomMaintain] >= 0 && !mBottomMaintainOn &&
            mShapeInstance->getPos(mJetThread[BottomActivate]) >= 1.0) {
         mShapeInstance->setPos(mJetThread[BottomActivate], 0);
         mShapeInstance->setTimeScale(mJetThread[BottomActivate], 0);
         mJetThread[BottomMaintain] = mShapeInstance->addThread();
         mShapeInstance->setSequence(mJetThread[BottomMaintain],mJetSeq[BottomMaintain],0);
         mShapeInstance->setTimeScale(mJetThread[BottomMaintain],1);
         mBottomMaintainOn = true;
      }
      if(mBottomMaintainOn)
         mShapeInstance->advanceTime(dt,mJetThread[BottomMaintain]);
   }

   // Jet particles
   for (S32 j = 0; j < NumThrustDirections; j++) {
      JetActivation& jet = sJetActivation[j];
      updateEmitter(mJetting && j == mThrustDirection,dt,mDataBlock->jetEmitter[jet.emitter],
                    jet.node,FlyingVehicleData::MaxDirectionJets);
   }

   // Trail jets
   Point3F yv;
   mObjToWorld.getColumn(1,&yv);
   F32 speed = mFabs(mDot(yv,mRigid.linVelocity));
   F32 trail = 0;
   if (speed > mDataBlock->minTrailSpeed) {
      trail = dt;
      if (speed < mDataBlock->maxSpeed)
         trail *= (speed - mDataBlock->minTrailSpeed) / mDataBlock->maxSpeed;
   }
   updateEmitter(trail,trail,mDataBlock->jetEmitter[FlyingVehicleData::TrailEmitter],
                 FlyingVehicleData::TrailNode,FlyingVehicleData::MaxTrails);

   // Allocate/Deallocate voice on demand.
   if ( !mJetSound )
      return;

   if ( !mJetting ) 
      mJetSound->stop();
   else 
   {
      if ( !mJetSound->isPlaying() )
         mJetSound->play();

      mJetSound->setTransform( getTransform() );
      mJetSound->setVelocity( getVelocity() );
   }
}

//----------------------------------------------------------------------------

void FlyingVehicle::updateEmitter(bool active,F32 dt,ParticleEmitterData *emitter,S32 idx,S32 count)
{
   if (!emitter)
      return;
   for (S32 j = idx; j < idx + count; j++)
      if (active) {
         if (mDataBlock->jetNode[j] != -1) {
            if (!bool(mJetEmitter[j])) {
               mJetEmitter[j] = new ParticleEmitter;
               mJetEmitter[j]->onNewDataBlock(emitter,false);
               mJetEmitter[j]->registerObject();
            }
            MatrixF mat;
            Point3F pos,axis;
            mat.mul(getRenderTransform(),
                    mShapeInstance->mNodeTransforms[mDataBlock->jetNode[j]]);
            mat.getColumn(1,&axis);
            mat.getColumn(3,&pos);
            mJetEmitter[j]->emitParticles(pos,true,axis,getVelocity(),(U32)(dt * 1000));
         }
      }
      else {
         for (S32 j = idx; j < idx + count; j++)
            if (bool(mJetEmitter[j])) {
               mJetEmitter[j]->deleteWhenEmpty();
               mJetEmitter[j] = 0;
            }
      }
}


//----------------------------------------------------------------------------

void FlyingVehicle::writePacketData(GameConnection *connection, BitStream *stream)
{
   Parent::writePacketData(connection, stream);
}

void FlyingVehicle::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);

   setPosition(mRigid.linPosition,mRigid.angPosition);
   mDelta.pos = mRigid.linPosition;
   mDelta.rot[1] = mRigid.angPosition;
}

U32 FlyingVehicle::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if(stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return retMask;

   stream->writeFlag(createHeightOn);

   stream->writeInt(mThrustDirection,NumThrustBits);

   return retMask;
}

void FlyingVehicle::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con,stream);

   if(stream->readFlag())
      return;

   createHeightOn = stream->readFlag();

   mThrustDirection = ThrustDirection(stream->readInt(NumThrustBits));
}

void FlyingVehicle::initPersistFields()
{
   Parent::initPersistFields();
}

DefineEngineMethod( FlyingVehicle, useCreateHeight, void, ( bool enabled ),,
   "@brief Set whether the vehicle should temporarily use the createHoverHeight "
   "specified in the datablock.\n\nThis can help avoid problems with spawning.\n"
   "@param enabled true to use the datablock createHoverHeight, false otherwise\n" )
{
   object->useCreateHeight( enabled );
}

void FlyingVehicle::useCreateHeight(bool val)
{
   createHeightOn = val;
   setMaskBits(HoverHeight);
}
