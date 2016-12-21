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
#include "T3D/vehicles/hoverVehicle.h"

#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "collision/clippedPolyList.h"
#include "collision/planeExtractor.h"
#include "T3D/gameBase/moveManager.h"
#include "ts/tsShapeInstance.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxSource.h"
#include "T3D/fx/particleEmitter.h"
#include "math/mathIO.h"


IMPLEMENT_CO_DATABLOCK_V1(HoverVehicleData);
IMPLEMENT_CO_NETOBJECT_V1(HoverVehicle);

ConsoleDocClass( HoverVehicleData,
   "@brief Defines the properties of a HoverVehicle.\n\n"
   "@ingroup Vehicles\n"
);

ConsoleDocClass( HoverVehicle,
   "@brief A hovering vehicle.\n\n"
   "A hover vehicle is a vehicle that maintains a specific distance between the "
   "vehicle and the ground at all times; unlike a flying vehicle which is free "
   "to ascend and descend at will."
   "The model used for the HoverVehicle has the following requirements:\n"
   "<dl>"
   "<dt>Collision mesh</dt><dd>A convex collision mesh at detail size -1.</dd>"
   "<dt>JetNozzle0-1 nodes</dt><dd>Particle emitter nodes used when thrusting "
   "forward.</dd>"
   "<dt>JetNozzle2-3 nodes</dt><dd>Particle emitter nodes used when thrusting "
   "downward.</dd>"
   "<dt>JetNozzleX node</dt><dd>Particle emitter node used when thrusting "
   "backward.</dd>"
   "<dt>activateBack animation</dt><dd>Non-cyclic animation sequence played "
   "when the vehicle begins thrusting forwards.</dd>"
   "<dt>maintainBack animation</dt><dd>Cyclic animation sequence played after "
   "activateBack when the vehicle continues thrusting forwards.</dd>"
   "</dl>"
   "@ingroup Vehicles\n"
);

namespace {
const F32 sHoverVehicleGravity  = -20;

const U32 sCollisionMoveMask = (TerrainObjectType     | PlayerObjectType  | 
                                StaticShapeObjectType | VehicleObjectType | 
                                VehicleBlockerObjectType);

const U32 sServerCollisionMask = sCollisionMoveMask; // ItemObjectType
const U32 sClientCollisionMask = sCollisionMoveMask;

void nonFilter(SceneObject* object,void *key)
{
   SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
   object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
}

} // namespace {}

const char* HoverVehicle::sJetSequence[HoverVehicle::JetAnimCount] =
{
   "activateBack",
   "maintainBack",
};

const char* HoverVehicleData::sJetNode[HoverVehicleData::MaxJetNodes] =
{
   "JetNozzle0",  // Thrust Forward
   "JetNozzle1",
   "JetNozzleX",  // Thrust Backward
   "JetNozzleX",
   "JetNozzle2",  // Thrust Downward
   "JetNozzle3",
};

// Convert thrust direction into nodes & emitters
HoverVehicle::JetActivation HoverVehicle::sJetActivation[NumThrustDirections] = {
   { HoverVehicleData::ForwardJetNode,  HoverVehicleData::ForwardJetEmitter },
   { HoverVehicleData::BackwardJetNode, HoverVehicleData::BackwardJetEmitter },
   { HoverVehicleData::DownwardJetNode, HoverVehicleData::DownwardJetEmitter },
};

//--------------------------------------------------------------------------
//--------------------------------------
//
HoverVehicleData::HoverVehicleData()
{
   dragForce            = 0;
   vertFactor           = 0.25f;
   floatingThrustFactor = 0.15f;

   mainThrustForce    = 0;
   reverseThrustForce = 0;
   strafeThrustForce  = 0;
   turboFactor        = 1.0f;

   stabLenMin = 0.5f;
   stabLenMax = 2.0f;
   stabSpringConstant  = 30;
   stabDampingConstant = 10;

   gyroDrag = 10;
   normalForce = 30;
   restorativeForce = 10;
   steeringForce = 25;
   rollForce = 2.5f;
   pitchForce = 2.5f;

   dustTrailEmitter = NULL;
   dustTrailID = 0;
   dustTrailOffset.set( 0.0f, 0.0f, 0.0f );
   dustTrailFreqMod = 15.0f;
   triggerTrailHeight = 2.5f;

   floatingGravMag = 1;
   brakingForce = 0;
   brakingActivationSpeed = 0;

   for (S32 k = 0; k < MaxJetNodes; k++)
      jetNode[k] = -1;

   for (S32 j = 0; j < MaxJetEmitters; j++)
      jetEmitter[j] = 0;

   for (S32 i = 0; i < MaxSounds; i++)
      sound[i] = 0;
}

HoverVehicleData::~HoverVehicleData()
{

}


//--------------------------------------------------------------------------
void HoverVehicleData::initPersistFields()
{
   addField( "dragForce", TypeF32, Offset(dragForce, HoverVehicleData),
      "Drag force factor that acts opposite to the vehicle velocity.\nAlso "
      "used to determnine the vehicle's maxThrustSpeed.\n@see mainThrustForce" );
   addField( "vertFactor", TypeF32, Offset(vertFactor, HoverVehicleData),
      "Scalar applied to the vertical portion of the velocity drag acting on "
      "the vehicle.\nFor the horizontal (X and Y) components of velocity drag, "
      "a factor of 0.25 is applied when the vehicle is floating, and a factor "
      "of 1.0 is applied when the vehicle is not floating. This velocity drag "
      "is multiplied by the vehicle's dragForce, as defined above, and the "
      "result is subtracted from it's movement force.\n"
      "@note The vertFactor must be between 0.0 and 1.0 (inclusive)." );
   addField( "floatingThrustFactor", TypeF32, Offset(floatingThrustFactor, HoverVehicleData),
      "Scalar applied to the vehicle's thrust force when the vehicle is floating.\n"
      "@note The floatingThrustFactor must be between 0.0 and 1.0 (inclusive)." );
   addField( "mainThrustForce", TypeF32, Offset(mainThrustForce, HoverVehicleData),
      "Force generated by thrusting the vehicle forward.\nAlso used to determine "
      "the maxThrustSpeed:\n\n"
      "@tsexample\n"
      "maxThrustSpeed = (mainThrustForce + strafeThrustForce) / dragForce;\n"
      "@endtsexample\n" );
   addField( "reverseThrustForce", TypeF32, Offset(reverseThrustForce, HoverVehicleData),
      "Force generated by thrusting the vehicle backward." );
   addField( "strafeThrustForce", TypeF32, Offset(strafeThrustForce, HoverVehicleData),
      "Force generated by thrusting the vehicle to one side.\nAlso used to "
      "determine the vehicle's maxThrustSpeed.\n@see mainThrustForce" );
   addField( "turboFactor", TypeF32, Offset(turboFactor, HoverVehicleData),
      "Scale factor applied to the vehicle's thrust force when jetting." );

   addField( "stabLenMin", TypeF32, Offset(stabLenMin, HoverVehicleData),
      "Length of the base stabalizer when travelling at minimum speed (0).\n"
      "Each tick, the vehicle performs 2 raycasts (from the center back and "
      "center front of the vehicle) to check for contact with the ground. The "
      "base stabalizer length determines the length of that raycast; if "
      "neither raycast hit the ground, the vehicle is floating, stabalizer "
      "spring and ground normal forces are not applied.\n\n"
      "<img src=\"images/hoverVehicle_forces.png\">\n"
      "@see stabSpringConstant" );
   addField( "stabLenMax", TypeF32, Offset(stabLenMax, HoverVehicleData),
      "Length of the base stabalizer when travelling at maximum speed "
      "(maxThrustSpeed).\n\n@see stabLenMin\n\n@see mainThrustForce" );

   addField( "stabSpringConstant", TypeF32, Offset(stabSpringConstant, HoverVehicleData),
      "Value used to generate stabalizer spring force. The force generated "
      "depends on stabilizer compression, that is how close the vehicle is "
      "to the ground proportional to current stabalizer length.\n\n"
      "@see stabLenMin" );
   addField( "stabDampingConstant", TypeF32, Offset(stabDampingConstant, HoverVehicleData),
      "Damping spring force acting against changes in the stabalizer length.\n\n"
      "@see stabLenMin" );

   addField( "gyroDrag", TypeF32, Offset(gyroDrag, HoverVehicleData),
      "Damping torque that acts against the vehicle's current angular momentum." );
   addField( "normalForce", TypeF32, Offset(normalForce, HoverVehicleData),
      "Force generated in the ground normal direction when the vehicle is not "
      "floating (within stabalizer length from the ground).\n\n"
      "@see stabLenMin" );
   addField( "restorativeForce", TypeF32, Offset(restorativeForce, HoverVehicleData),
      "Force generated to stabalize the vehicle (return it to neutral pitch/roll) "
      "when the vehicle is floating (more than stabalizer length from the "
      "ground.\n\n@see stabLenMin" );
   addField( "steeringForce", TypeF32, Offset(steeringForce, HoverVehicleData),
      "Yaw (rotation about the Z-axis) force applied when steering in the x-axis direction."
      "about the vehicle's Z-axis)" );
   addField( "rollForce", TypeF32, Offset(rollForce, HoverVehicleData),
      "Roll (rotation about the Y-axis) force applied when steering in the x-axis direction." );
   addField( "pitchForce", TypeF32, Offset(pitchForce, HoverVehicleData),
      "Pitch (rotation about the X-axis) force applied when steering in the y-axis direction." );

   addField( "jetSound", TYPEID< SFXProfile >(), Offset(sound[JetSound], HoverVehicleData),
      "Looping sound played when the vehicle is jetting." );
   addField( "engineSound", TYPEID< SFXProfile >(), Offset(sound[EngineSound], HoverVehicleData),
      "Looping engine sound.\nThe volume is dynamically adjusted based on the "
      "current thrust level." );
   addField( "floatSound", TYPEID< SFXProfile >(), Offset(sound[FloatSound], HoverVehicleData),
      "Looping sound played while the vehicle is floating.\n\n@see stabMinLen" );

   addField( "dustTrailEmitter", TYPEID< ParticleEmitterData >(), Offset(dustTrailEmitter, HoverVehicleData),
      "Emitter to generate particles for the vehicle's dust trail.\nThe trail "
      "of dust particles is generated only while the vehicle is moving." );
   addField( "dustTrailOffset", TypePoint3F, Offset(dustTrailOffset, HoverVehicleData),
      "\"X Y Z\" offset from the vehicle's origin from which to generate dust "
      "trail particles.\nBy default particles are emitted directly beneath the "
      "origin of the vehicle model." );
   addField( "triggerTrailHeight", TypeF32, Offset(triggerTrailHeight, HoverVehicleData),
      "Maximum height above surface to emit dust trail particles.\nIf the vehicle "
      "is less than triggerTrailHeight above a static surface with a material that "
      "has 'showDust' set to true, the vehicle will emit particles from the "
      "dustTrailEmitter." );
   addField( "dustTrailFreqMod", TypeF32, Offset(dustTrailFreqMod, HoverVehicleData),
      "Number of dust trail particles to generate based on vehicle speed.\nThe "
      "vehicle's speed is divided by this value to determine how many particles "
      "to generate each frame. Lower values give a more dense trail, higher "
      "values a more sparse trail." );

   addField( "floatingGravMag", TypeF32, Offset(floatingGravMag, HoverVehicleData),
      "Scale factor applied to the vehicle gravitational force when the vehicle "
      "is floating.\n\n@see stabLenMin" );
   addField( "brakingForce", TypeF32, Offset(brakingForce, HoverVehicleData),
      "Force generated by braking.\nThe vehicle is considered to be braking if "
      "it is moving, but the throttle is off, and no left or right thrust is "
      "being applied. This force is only applied when the vehicle's velocity is "
      "less than brakingActivationSpeed." );
   addField( "brakingActivationSpeed", TypeF32, Offset(brakingActivationSpeed, HoverVehicleData),
      "Maximum speed below which a braking force is applied.\n\n@see brakingForce" );

   addField( "forwardJetEmitter", TYPEID< ParticleEmitterData >(), Offset(jetEmitter[ForwardJetEmitter], HoverVehicleData),
      "Emitter to generate particles for forward jet thrust.\nForward jet "
      "thrust particles are emitted from model nodes JetNozzle0 and JetNozzle1." );

   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
bool HoverVehicleData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


bool HoverVehicleData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;

   if (dragForce <= 0.01f) {
      Con::warnf("HoverVehicleData::preload: dragForce must be at least 0.01");
      dragForce = 0.01f;
   }
   if (vertFactor < 0.0f || vertFactor > 1.0f) {
      Con::warnf("HoverVehicleData::preload: vert factor must be [0, 1]");
      vertFactor = vertFactor < 0.0f ? 0.0f : 1.0f;
   }
   if (floatingThrustFactor < 0.0f || floatingThrustFactor > 1.0f) {
      Con::warnf("HoverVehicleData::preload: floatingThrustFactor must be [0, 1]");
      floatingThrustFactor = floatingThrustFactor < 0.0f ? 0.0f : 1.0f;
   }

   maxThrustSpeed = (mainThrustForce + strafeThrustForce) / dragForce;

   massCenter = Point3F(0, 0, 0);

   // Resolve objects transmitted from server
   if (!server) {
      for (S32 i = 0; i < MaxSounds; i++)
         if (sound[i])
            Sim::findObject(SimObjectId((uintptr_t)sound[i]),sound[i]);
      for (S32 j = 0; j < MaxJetEmitters; j++)
         if (jetEmitter[j])
            Sim::findObject(SimObjectId((uintptr_t)jetEmitter[j]),jetEmitter[j]);
   }

   if( !dustTrailEmitter && dustTrailID != 0 )
   {
      if( !Sim::findObject( dustTrailID, dustTrailEmitter ) )
      {
         Con::errorf( ConsoleLogEntry::General, "HoverVehicleData::preload Invalid packet, bad datablockId(dustTrailEmitter): 0x%x", dustTrailID );
      }
   }
   // Resolve jet nodes
   for (S32 j = 0; j < MaxJetNodes; j++)
      jetNode[j] = mShape->findNode(sJetNode[j]);

   return true;
}


//--------------------------------------------------------------------------
void HoverVehicleData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(dragForce);
   stream->write(vertFactor);
   stream->write(floatingThrustFactor);
   stream->write(mainThrustForce);
   stream->write(reverseThrustForce);
   stream->write(strafeThrustForce);
   stream->write(turboFactor);
   stream->write(stabLenMin);
   stream->write(stabLenMax);
   stream->write(stabSpringConstant);
   stream->write(stabDampingConstant);
   stream->write(gyroDrag);
   stream->write(normalForce);
   stream->write(restorativeForce);
   stream->write(steeringForce);
   stream->write(rollForce);
   stream->write(pitchForce);
   mathWrite(*stream, dustTrailOffset);
   stream->write(triggerTrailHeight);
   stream->write(dustTrailFreqMod);

   for (S32 i = 0; i < MaxSounds; i++)
      if (stream->writeFlag(sound[i]))
         stream->writeRangedU32(packed? SimObjectId((uintptr_t)sound[i]):
                                sound[i]->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);

   for (S32 j = 0; j < MaxJetEmitters; j++)
   {
      if (stream->writeFlag(jetEmitter[j]))
      {
         SimObjectId writtenId = packed ? SimObjectId((uintptr_t)jetEmitter[j]) : jetEmitter[j]->getId();
         stream->writeRangedU32(writtenId, DataBlockObjectIdFirst,DataBlockObjectIdLast);
      }
   }

   if (stream->writeFlag( dustTrailEmitter ))
   {
      stream->writeRangedU32( dustTrailEmitter->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }
   stream->write(floatingGravMag);
   stream->write(brakingForce);
   stream->write(brakingActivationSpeed);
}


void HoverVehicleData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&dragForce);
   stream->read(&vertFactor);
   stream->read(&floatingThrustFactor);
   stream->read(&mainThrustForce);
   stream->read(&reverseThrustForce);
   stream->read(&strafeThrustForce);
   stream->read(&turboFactor);
   stream->read(&stabLenMin);
   stream->read(&stabLenMax);
   stream->read(&stabSpringConstant);
   stream->read(&stabDampingConstant);
   stream->read(&gyroDrag);
   stream->read(&normalForce);
   stream->read(&restorativeForce);
   stream->read(&steeringForce);
   stream->read(&rollForce);
   stream->read(&pitchForce);
   mathRead(*stream, &dustTrailOffset);
   stream->read(&triggerTrailHeight);
   stream->read(&dustTrailFreqMod);

   for (S32 i = 0; i < MaxSounds; i++)
      sound[i] = stream->readFlag()?
         (SFXProfile*) stream->readRangedU32(DataBlockObjectIdFirst,
                                               DataBlockObjectIdLast): 0;

   for (S32 j = 0; j < MaxJetEmitters; j++) {
      jetEmitter[j] = NULL;
      if (stream->readFlag())
         jetEmitter[j] = (ParticleEmitterData*)stream->readRangedU32(DataBlockObjectIdFirst,
                                                                     DataBlockObjectIdLast);
   }

   if( stream->readFlag() )
   {
      dustTrailID = (S32) stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   }
   stream->read(&floatingGravMag);
   stream->read(&brakingForce);
   stream->read(&brakingActivationSpeed);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
HoverVehicle::HoverVehicle()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);

   mFloating      = false;
   mForwardThrust = 0;
   mReverseThrust = 0;
   mLeftThrust    = 0;
   mRightThrust   = 0;

   mJetSound    = NULL;
   mEngineSound = NULL;
   mFloatSound  = NULL;

   mDustTrailEmitter = NULL;

   mBackMaintainOn = false;
   for (S32 i = 0; i < JetAnimCount; i++)
      mJetThread[i] = 0;
}

HoverVehicle::~HoverVehicle()
{
   //
}

//--------------------------------------------------------------------------
bool HoverVehicle::onAdd()
{
   if(!Parent::onAdd())
      return false;

   addToScene();


   if( !isServerObject() )
   {
      if( mDataBlock->dustTrailEmitter )
      {
         mDustTrailEmitter = new ParticleEmitter;
         mDustTrailEmitter->onNewDataBlock( mDataBlock->dustTrailEmitter, false );
         if( !mDustTrailEmitter->registerObject() )
         {
            Con::warnf( ConsoleLogEntry::General, "Could not register dust emitter for class: %s", mDataBlock->getName() );
            delete mDustTrailEmitter;
            mDustTrailEmitter = NULL;
         }
      }
      // Jet Sequences
      for (S32 i = 0; i < JetAnimCount; i++) {
         TSShape const* shape = mShapeInstance->getShape();
         mJetSeq[i] = shape->findSequence(sJetSequence[i]);
         if (mJetSeq[i] != -1) {
            if (i == BackActivate) {
               mJetThread[i] = mShapeInstance->addThread();
               mShapeInstance->setSequence(mJetThread[i],mJetSeq[i],0);
               mShapeInstance->setTimeScale(mJetThread[i],0);
            }
         }
         else
            mJetThread[i] = 0;
      }
   }


   if (isServerObject())
      scriptOnAdd();

   return true;
}


void HoverVehicle::onRemove()
{
   SFX_DELETE( mJetSound );
   SFX_DELETE( mEngineSound );
   SFX_DELETE( mFloatSound );

   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}


bool HoverVehicle::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<HoverVehicleData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
      return false;

   if (isGhost()) 
   {
      // Create the sounds ahead of time.  This reduces runtime
      // costs and makes the system easier to understand.

      SFX_DELETE( mEngineSound );
      SFX_DELETE( mFloatSound );
      SFX_DELETE( mJetSound );

      if ( mDataBlock->sound[HoverVehicleData::EngineSound] )
         mEngineSound = SFX->createSource( mDataBlock->sound[HoverVehicleData::EngineSound], &getTransform() );

      if ( !mDataBlock->sound[HoverVehicleData::FloatSound] )
         mFloatSound = SFX->createSource( mDataBlock->sound[HoverVehicleData::FloatSound], &getTransform() );

      if ( mDataBlock->sound[HoverVehicleData::JetSound] )
         mJetSound = SFX->createSource( mDataBlock->sound[HoverVehicleData::JetSound], &getTransform() );
   }

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();

   return true;
}



//--------------------------------------------------------------------------
void HoverVehicle::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   // Update jetsound...
   if ( mJetSound ) 
   {
      if ( mJetting )
      {
         if ( !mJetSound->isPlaying() )
            mJetSound->play();

         mJetSound->setTransform( getTransform() );
      }
      else 
         mJetSound->stop();
   }

   // Update engine sound...
   if ( mEngineSound )
   {
      if ( !mEngineSound->isPlaying() )
         mEngineSound->play();

      mEngineSound->setTransform( getTransform() );

      F32 denom  = mDataBlock->mainThrustForce + mDataBlock->strafeThrustForce;
      F32 factor = getMin(mThrustLevel, denom) / denom;
      F32 vol = 0.25 + factor * 0.75;
      mEngineSound->setVolume( vol );
   }

   // Are we floating?  If so, start the floating sound...
   if ( mFloatSound )
   {
      if ( mFloating )
      {
         if ( !mFloatSound->isPlaying() )
            mFloatSound->play();

         mFloatSound->setTransform( getTransform() );
      }
      else
         mFloatSound->stop();
   }

   updateJet(dt);
   updateDustTrail( dt );
}


//--------------------------------------------------------------------------

U32 HoverVehicle::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   //
   stream->writeInt(mThrustDirection,NumThrustBits);

   return retMask;
}

void HoverVehicle::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   mThrustDirection = ThrustDirection(stream->readInt(NumThrustBits));
   //
}


//--------------------------------------------------------------------------
void HoverVehicle::updateMove(const Move* move)
{
   Parent::updateMove(move);

   mForwardThrust = mThrottle > 0.0f ?  mThrottle : 0.0f;
   mReverseThrust = mThrottle < 0.0f ? -mThrottle : 0.0f;
   mLeftThrust    = move->x   < 0.0f ? -move->x   : 0.0f;
   mRightThrust   = move->x   > 0.0f ?  move->x   : 0.0f;

   mThrustDirection = (!move->y)? ThrustDown: (move->y > 0)? ThrustForward: ThrustBackward;
}

F32 HoverVehicle::getBaseStabilizerLength() const
{
   F32 base = mDataBlock->stabLenMin;
   F32 lengthDiff = mDataBlock->stabLenMax - mDataBlock->stabLenMin;
   F32 velLength  = mRigid.linVelocity.len();
   F32 minVel     = getMin(velLength, mDataBlock->maxThrustSpeed);
   F32 velDiff    = mDataBlock->maxThrustSpeed - minVel;
   // Protect against divide by zero.
   F32 velRatio   = mDataBlock->maxThrustSpeed != 0.0f ? ( velDiff / mDataBlock->maxThrustSpeed ) : 0.0f;
   F32 inc        = lengthDiff * ( 1.0 - velRatio );
   base += inc;

   return base;
}


struct StabPoint
{
   Point3F osPoint;           //
   Point3F wsPoint;           //
   F32     extension;
   Point3F wsExtension;       //
   Point3F wsVelocity;        //
};


void HoverVehicle::updateForces(F32 /*dt*/)
{
   PROFILE_SCOPE( HoverVehicle_UpdateForces );

   Point3F gravForce(0, 0, sHoverVehicleGravity * mRigid.mass * mGravityMod);

   MatrixF currTransform;
   mRigid.getTransform(&currTransform);
   mRigid.atRest = false;

   mThrustLevel = (mForwardThrust * mDataBlock->mainThrustForce    +
                   mReverseThrust * mDataBlock->reverseThrustForce +
                   mLeftThrust    * mDataBlock->strafeThrustForce  +
                   mRightThrust   * mDataBlock->strafeThrustForce);

   Point3F thrustForce = ((Point3F( 0,  1, 0) * (mForwardThrust * mDataBlock->mainThrustForce))    +
                          (Point3F( 0, -1, 0) * (mReverseThrust * mDataBlock->reverseThrustForce)) +
                          (Point3F(-1,  0, 0) * (mLeftThrust    * mDataBlock->strafeThrustForce))  +
                          (Point3F( 1,  0, 0) * (mRightThrust   * mDataBlock->strafeThrustForce)));
   currTransform.mulV(thrustForce);
   if (mJetting)
      thrustForce *= mDataBlock->turboFactor;

   Point3F torque(0, 0, 0);
   Point3F force(0, 0, 0);

   Point3F vel = mRigid.linVelocity;
   F32 baseStabLen = getBaseStabilizerLength();
   Point3F stabExtend(0, 0, -baseStabLen);
   currTransform.mulV(stabExtend);

   StabPoint stabPoints[2];
   stabPoints[0].osPoint = Point3F((mObjBox.minExtents.x + mObjBox.maxExtents.x) * 0.5,
                                   mObjBox.maxExtents.y,
                                   (mObjBox.minExtents.z + mObjBox.maxExtents.z) * 0.5);
   stabPoints[1].osPoint = Point3F((mObjBox.minExtents.x + mObjBox.maxExtents.x) * 0.5,
                                   mObjBox.minExtents.y,
                                   (mObjBox.minExtents.z + mObjBox.maxExtents.z) * 0.5);
   U32 j, i;
   for (i = 0; i < 2; i++) {
      currTransform.mulP(stabPoints[i].osPoint, &stabPoints[i].wsPoint);
      stabPoints[i].wsExtension = stabExtend;
      stabPoints[i].extension   = baseStabLen;
      stabPoints[i].wsVelocity  = mRigid.linVelocity;
   }

   RayInfo rinfo;

   mFloating = true;
   bool reallyFloating = true;
   F32 compression[2] = { 0.0f, 0.0f };
   F32  normalMod[2]  = { 0.0f, 0.0f };
   bool normalSet[2]  = { false, false };
   Point3F normal[2];

   for (j = 0; j < 2; j++) {
      if (getContainer()->castRay(stabPoints[j].wsPoint, stabPoints[j].wsPoint + stabPoints[j].wsExtension * 2.0,
                                  TerrainObjectType | 
                                  WaterObjectType, &rinfo)) 
      {
         reallyFloating = false;

         if (rinfo.t <= 0.5) {
            // Ok, stab is in contact with the ground, let's calc the forces...
            compression[j] = (1.0 - (rinfo.t * 2.0)) * baseStabLen;
         }
         normalSet[j] = true;
         normalMod[j] = rinfo.t < 0.5 ? 1.0 : (1.0 - ((rinfo.t - 0.5) * 2.0));

         normal[j] = rinfo.normal;
      }
      
      if ( pointInWater( stabPoints[j].wsPoint ) )
         compression[j] = baseStabLen;
   }

   for (j = 0; j < 2; j++) {
      if (compression[j] != 0.0) {
         mFloating = false;

         // Spring force and damping
         Point3F springForce = -stabPoints[j].wsExtension;
         springForce.normalize();
         springForce *= compression[j] * mDataBlock->stabSpringConstant;

         Point3F springDamping = -stabPoints[j].wsExtension;
         springDamping.normalize();
         springDamping *= -getMin(mDot(springDamping, stabPoints[j].wsVelocity), 0.7f) * mDataBlock->stabDampingConstant;

         force += springForce + springDamping;
      }
   }

   // Gravity
   if (reallyFloating == false)
      force += gravForce;
   else
      force += gravForce * mDataBlock->floatingGravMag;

   // Braking
   F32 vellen = mRigid.linVelocity.len();
   if (mThrottle == 0.0f &&
       mLeftThrust == 0.0f &&
       mRightThrust == 0.0f &&
       vellen != 0.0f &&
       vellen < mDataBlock->brakingActivationSpeed)
   {
      Point3F dir = mRigid.linVelocity;
      dir.normalize();
      dir.neg();
      force += dir *  mDataBlock->brakingForce;
   }

   // Gyro Drag
   torque = -mRigid.angMomentum * mDataBlock->gyroDrag;

   // Move to proper normal
   Point3F sn, r;
   currTransform.getColumn(2, &sn);
   if (normalSet[0] || normalSet[1]) {
      if (normalSet[0] && normalSet[1]) {
         F32 dot = mDot(normal[0], normal[1]);
         if (dot > 0.999) {
            // Just pick the first normal.  They're too close to call
            if ((sn - normal[0]).lenSquared() > 0.00001) {
               mCross(sn, normal[0], &r);
               torque += r * mDataBlock->normalForce * normalMod[0];
            }
         } else {
            Point3F rotAxis;
            mCross(normal[0], normal[1], &rotAxis);
            rotAxis.normalize();

            F32 angle = mAcos(dot) * (normalMod[0] / (normalMod[0] + normalMod[1]));
            AngAxisF aa(rotAxis, angle);
            QuatF q(aa);
            MatrixF tempMat(true);
            q.setMatrix(&tempMat);
            Point3F newNormal;
            tempMat.mulV(normal[1], &newNormal);

            if ((sn - newNormal).lenSquared() > 0.00001) {
               mCross(sn, newNormal, &r);
               torque += r * (mDataBlock->normalForce * ((normalMod[0] + normalMod[1]) * 0.5));
            }
         }
      } else {
         Point3F useNormal;
         F32     useMod;
         if (normalSet[0]) {
            useNormal = normal[0];
            useMod    = normalMod[0];
         } else {
            useNormal = normal[1];
            useMod    = normalMod[1];
         }

         if ((sn - useNormal).lenSquared() > 0.00001) {
            mCross(sn, useNormal, &r);
            torque += r * mDataBlock->normalForce * useMod;
         }
      }
   } else {
      if ((sn - Point3F(0, 0, 1)).lenSquared() > 0.00001) {
         mCross(sn, Point3F(0, 0, 1), &r);
         torque += r * mDataBlock->restorativeForce;
      }
   }

   Point3F sn2;
   currTransform.getColumn(0, &sn);
   currTransform.getColumn(1, &sn2);
   mCross(sn, sn2, &r);
   r.normalize();
   torque -= r * (mSteering.x * mDataBlock->steeringForce);

   currTransform.getColumn(0, &sn);
   currTransform.getColumn(2, &sn2);
   mCross(sn, sn2, &r);
   r.normalize();
   torque -= r * (mSteering.x * mDataBlock->rollForce);

   currTransform.getColumn(1, &sn);
   currTransform.getColumn(2, &sn2);
   mCross(sn, sn2, &r);
   r.normalize();
   torque -= r * (mSteering.y * mDataBlock->pitchForce);

   // Apply drag
   Point3F vDrag = mRigid.linVelocity;
   if (!mFloating) {
      vDrag.convolve(Point3F(1, 1, mDataBlock->vertFactor));
   } else {
      vDrag.convolve(Point3F(0.25, 0.25, mDataBlock->vertFactor));
   }
   force -= vDrag * mDataBlock->dragForce;

   force += mFloating ? thrustForce * mDataBlock->floatingThrustFactor : thrustForce;

   // Add in physical zone force
   force += mAppliedForce;

   // Container buoyancy & drag
   force  += Point3F(0, 0,-mBuoyancy * sHoverVehicleGravity * mRigid.mass * mGravityMod);
   force  -= mRigid.linVelocity * mDrag;
   torque -= mRigid.angMomentum * mDrag;

   mRigid.force  = force;
   mRigid.torque = torque;
}


//--------------------------------------------------------------------------
U32 HoverVehicle::getCollisionMask()
{
   if (isServerObject())
      return sServerCollisionMask;
   else
      return sClientCollisionMask;
}

void HoverVehicle::updateDustTrail( F32 dt )
{
   // Check to see if we're moving.

   VectorF velocityVector = getVelocity();
   F32 velocity = velocityVector.len();

   if( velocity > 2.0 )
   {
      velocityVector.normalize();
      emitDust( mDustTrailEmitter, mDataBlock->triggerTrailHeight, mDataBlock->dustTrailOffset,
                ( U32 )( dt * 1000 * ( velocity / mDataBlock->dustTrailFreqMod ) ),
                velocityVector );
   }
}

void HoverVehicle::updateJet(F32 dt)
{
   if (mJetThread[BackActivate] == NULL)
      return;

   // Thrust Animation threads
   //  Back
   if (mJetSeq[BackActivate] >=0 ) {
      if (!mBackMaintainOn || mThrustDirection != ThrustForward) {
         if (mBackMaintainOn) {
            mShapeInstance->setPos(mJetThread[BackActivate], 1);
            mShapeInstance->destroyThread(mJetThread[BackMaintain]);
            mBackMaintainOn = false;
         }
         mShapeInstance->setTimeScale(mJetThread[BackActivate],
                                      (mThrustDirection == ThrustForward)? 1.0f : -1.0f);
         mShapeInstance->advanceTime(dt,mJetThread[BackActivate]);
      }
   }

   if (mJetSeq[BackMaintain] >= 0 && !mBackMaintainOn &&
       mShapeInstance->getPos(mJetThread[BackActivate]) >= 1.0f)
   {
      mShapeInstance->setPos(mJetThread[BackActivate], 0);
      mShapeInstance->setTimeScale(mJetThread[BackActivate], 0);
      mJetThread[BackMaintain] = mShapeInstance->addThread();
      mShapeInstance->setSequence(mJetThread[BackMaintain],mJetSeq[BackMaintain],0);
      mShapeInstance->setTimeScale(mJetThread[BackMaintain],1);
      mBackMaintainOn = true;
   }

   if(mBackMaintainOn)
      mShapeInstance->advanceTime(dt,mJetThread[BackMaintain]);

   // Jet particles
   for (S32 j = 0; j < NumThrustDirections; j++) {
      JetActivation& jet = sJetActivation[j];
      updateEmitter(mJetting && j == mThrustDirection,dt,mDataBlock->jetEmitter[jet.emitter],
                    jet.node,HoverVehicleData::MaxDirectionJets);
   }
}

void HoverVehicle::updateEmitter(bool active,F32 dt,ParticleEmitterData *emitter,S32 idx,S32 count)
{
   if (!emitter)
      return;
   for (S32 j = idx; j < idx + count; j++)
      if (active) {
         if (mDataBlock->jetNode[j] != -1) {
            if (!bool(mJetEmitter[j])) {
               mJetEmitter[j] = new ParticleEmitter;
               mJetEmitter[j]->onNewDataBlock( emitter, false );
               mJetEmitter[j]->registerObject();
            }
            MatrixF mat;
            Point3F pos,axis;
            mat.mul(getRenderTransform(),
                    mShapeInstance->mNodeTransforms[mDataBlock->jetNode[j]]);
            mat.getColumn(1,&axis);
            mat.getColumn(3,&pos);
            mJetEmitter[j]->emitParticles(pos,true,axis,getVelocity(),(U32)(dt * 1000.0f));
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
