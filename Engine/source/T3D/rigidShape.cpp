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
#include "T3D/rigidShape.h"

#include "app/game.h"
#include "math/mMath.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "collision/clippedPolyList.h"
#include "collision/planeExtractor.h"
#include "T3D/gameBase/moveManager.h"
#include "core/stream/bitStream.h"
#include "core/dnet.h"
#include "T3D/gameBase/gameConnection.h"
#include "ts/tsShapeInstance.h"
#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "T3D/fx/cameraFXMgr.h"
#include "T3D/trigger.h"
#include "T3D/item.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxSystem.h"
#include "T3D/fx/particleEmitter.h"
#include "console/engineAPI.h"


IMPLEMENT_CO_DATABLOCK_V1(RigidShapeData);

ConsoleDocClass( RigidShapeData,
   "@brief Defines the physics properties for an individual RigidShapeData physics object.\n\n"

   "@tsexample\n"
	"	datablock RigidShapeData( BouncingBoulder )\n"
	"	{\n"	
	"	   category = \"RigidShape\";\n"
	"\n"		
	"	   shapeFile = \"~/data/shapes/boulder/boulder.dts\";\n"
	"	   emap = true;\n"
	"\n"
	"	   // Rigid Body\n"
	"	   mass = 500;\n"
	"	   massCenter = \"0 0 0\";    // Center of mass for rigid body\n"
	"	   massBox = \"0 0 0\";         // Size of box used for moment of inertia,\n"
	"								  // if zero it defaults to object bounding box\n"
	"	   drag = 0.2;                // Drag coefficient\n"
	"	   bodyFriction = 0.2;\n"
	"	   bodyRestitution = 0.1;\n"
	"	   minImpactSpeed = 5;        // Impacts over this invoke the script callback\n"
	"	   softImpactSpeed = 5;       // Play SoftImpact Sound\n"
	"	   hardImpactSpeed = 15;      // Play HardImpact Sound\n"
	"	   integration = 4;           // Physics integration: TickSec/Rate\n"
	"	   collisionTol = 0.1;        // Collision distance tolerance\n"
	"	   contactTol = 0.1;          // Contact velocity tolerance\n"
	"\n"	   
	"	   minRollSpeed = 10;\n"
	"\n"	   
	"	   maxDrag = 0.5;\n"
	"	   minDrag = 0.01;\n"
	"\n"
	"	   dustHeight = 10;\n"
	"\n"
	"	   dragForce = 0.05;\n"
	"	   vertFactor = 0.05;\n"
	"	};\n"
   "@endtsexample\n\n"

   "@see RigidShape\n"
   "@see ShapeBase\n\n"

   "@ingroup Platform\n"
);


IMPLEMENT_CO_NETOBJECT_V1(RigidShape);

ConsoleDocClass( RigidShape,
   "@brief The RigidShape class implements rigid-body physics for DTS objects in the world.\n\n"

   "\"Rigid body physics\" refers to a system whereby objects are assumed to have a finite size,\n"
   "equally distributed masses, and where deformations of the objects themselves are not accounted for.\n"
   "Uses the RigidShape class to control its physics.\n\n"

   "@tsexample\n"
	"	datablock RigidShapeData( BouncingBoulder )\n"
	"	{\n"	
	"	   category = \"RigidShape\";\n"
	"\n"		
	"	   shapeFile = \"~/data/shapes/boulder/boulder.dts\";\n"
	"	   emap = true;\n"
	"\n"
	"	   // Rigid Body\n"
	"	   mass = 500;\n"
	"	   massCenter = \"0 0 0\";    // Center of mass for rigid body\n"
	"	   massBox = \"0 0 0\";         // Size of box used for moment of inertia,\n"
	"								  // if zero it defaults to object bounding box\n"
	"	   drag = 0.2;                // Drag coefficient\n"
	"	   bodyFriction = 0.2;\n"
	"	   bodyRestitution = 0.1;\n"
	"	   minImpactSpeed = 5;        // Impacts over this invoke the script callback\n"
	"	   softImpactSpeed = 5;       // Play SoftImpact Sound\n"
	"	   hardImpactSpeed = 15;      // Play HardImpact Sound\n"
	"	   integration = 4;           // Physics integration: TickSec/Rate\n"
	"	   collisionTol = 0.1;        // Collision distance tolerance\n"
	"	   contactTol = 0.1;          // Contact velocity tolerance\n"
	"\n"	   
	"	   minRollSpeed = 10;\n"
	"\n"	   
	"	   maxDrag = 0.5;\n"
	"	   minDrag = 0.01;\n"
	"\n"
	"	   dustHeight = 10;\n"
	"\n"
	"	   dragForce = 0.05;\n"
	"	   vertFactor = 0.05;\n"
	"	};\n"
	"\n"
	" new RigidShape()\n"
	"	{\n"
    "		dataBlock = \"BouncingBoulder\";\n"
    "		parentGroup = EWCreatorWindow.objectGroup;\n"
    "	};\n"
   "@endtsexample\n\n"

   "@see RigidShapeData\n"
   "@see ShapeBase\n\n"
   
   "@ingroup Platform\n"
);


IMPLEMENT_CALLBACK( RigidShape, onEnterLiquid, void, ( const char* objId, const char* waterCoverage, const char* liquidType ),
													 ( objId, waterCoverage, liquidType ),
   "@brief Called whenever this RigidShape object enters liquid.\n\n"
   "@param objId The ID of the rigidShape object.\n"
   "@param waterCoverage Amount of water coverage the RigidShape has.\n"
   "@param liquidType Type of liquid that was entered.\n\n"
   "@tsexample\n"
   "// The RigidShape object falls in a body of liquid, causing the callback to occur.\n"
   "RigidShape::onEnterLiquid(%this,%objId,%waterCoverage,%liquidType)\n"
   "	{\n"
   "		// Code to run whenever this callback occurs.\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see ShapeBase\n\n"
);

IMPLEMENT_CALLBACK( RigidShape, onLeaveLiquid, void, ( const char* objId, const char* liquidType ),( objId, liquidType ),
   "@brief Called whenever the RigidShape object exits liquid.\n\n"
   "@param objId The ID of the RigidShape object.\n"
   "@param liquidType Type if liquid that was exited.\n\n"
   "@tsexample\n"
   "// The RigidShape object exits in a body of liquid, causing the callback to occur.\n"
   "RigidShape::onLeaveLiquid(%this,%objId,%liquidType)\n"
   "	{\n"
   "		// Code to run whenever this callback occurs.\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see ShapeBase\n\n"
);

//----------------------------------------------------------------------------

namespace {

   const U32 sMoveRetryCount = 3;

   // Client prediction
   const S32 sMaxWarpTicks = 3;           // Max warp duration in ticks
   const S32 sMaxPredictionTicks = 30;    // Number of ticks to predict
   const F32 sRigidShapeGravity = -20;

   // Physics and collision constants
   static F32 sRestTol = 0.5;             // % of gravity energy to be at rest
   static int sRestCount = 10;            // Consecutive ticks before comming to rest

   const U32 sCollisionMoveMask = ( TerrainObjectType     | PlayerObjectType  | 
                                    StaticShapeObjectType | VehicleObjectType |
                                    VehicleBlockerObjectType );

   const U32 sServerCollisionMask = sCollisionMoveMask; // ItemObjectType
   const U32 sClientCollisionMask = sCollisionMoveMask;

   void nonFilter(SceneObject* object,void *key)
   {
      SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
      object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
   }

} // namespace {}


// Trigger objects that are not normally collided with.
static U32 sTriggerMask = ItemObjectType     |
TriggerObjectType  |
CorpseObjectType;


//----------------------------------------------------------------------------

RigidShapeData::RigidShapeData()
{
   shadowEnable = true;

   body.friction = 0;
   body.restitution = 1;

   minImpactSpeed = 25;
   softImpactSpeed = 25;
   hardImpactSpeed = 50;
   minRollSpeed = 0;

   cameraRoll = true;
   cameraLag = 0;
   cameraDecay = 0;
   cameraOffset = 0;

   minDrag = 0;
   maxDrag = 0;
   integration = 1;
   collisionTol = 0.1f;
   contactTol = 0.1f;
   massCenter.set(0,0,0);
   massBox.set(0,0,0);

   drag = 0.7f;
   density = 4;

   for (S32 i = 0; i < Body::MaxSounds; i++)
      body.sound[i] = 0;

   dustEmitter = NULL;
   dustID = 0;
   dustHeight = 1.0;

   dMemset( splashEmitterList, 0, sizeof( splashEmitterList ) );
   dMemset( splashEmitterIDList, 0, sizeof( splashEmitterIDList ) );

   splashFreqMod = 300.0;
   splashVelEpsilon = 0.50;
   exitSplashSoundVel = 2.0;
   softSplashSoundVel = 1.0;
   medSplashSoundVel = 2.0;
   hardSplashSoundVel = 3.0;

   dMemset(waterSound, 0, sizeof(waterSound));

   dragForce            = 0;
   vertFactor           = 0.25;

   dustTrailEmitter = NULL;
   dustTrailID = 0;
}

RigidShapeData::~RigidShapeData()
{

}

//----------------------------------------------------------------------------


bool RigidShapeData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


bool RigidShapeData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   // RigidShape objects must define a collision detail
   if (!collisionDetails.size() || collisionDetails[0] == -1)
   {
      Con::errorf("RigidShapeData::preload failed: Rigid shapes must define a collision-1 detail");
      return false;
   }

   // Resolve objects transmitted from server
   if (!server) {
      for (S32 i = 0; i < Body::MaxSounds; i++)
         sfxResolve( &body.sound[ i ], errorStr );
   }

   if( !dustEmitter && dustID != 0 )
   {
      if( !Sim::findObject( dustID, dustEmitter ) )
      {
         Con::errorf( ConsoleLogEntry::General, "RigidShapeData::preload Invalid packet, bad datablockId(dustEmitter): 0x%x", dustID );
      }
   }

   U32 i;
   for( i=0; i<VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( !splashEmitterList[i] && splashEmitterIDList[i] != 0 )
      {
         if( !Sim::findObject( splashEmitterIDList[i], splashEmitterList[i] ) )
         {
            Con::errorf( ConsoleLogEntry::General, "RigidShapeData::preload Invalid packet, bad datablockId(splashEmitter): 0x%x", splashEmitterIDList[i] );
         }
      }
   }

   if (dragForce <= 0.01f) 
   {
      Con::warnf("RigidShapeData::preload: dragForce must be at least 0.01");
      dragForce = 0.01f;
   }

   if (vertFactor < 0.0f || vertFactor > 1.0f) 
   {
      Con::warnf("RigidShapeData::preload: vert factor must be [0, 1]");
      vertFactor = vertFactor < 0.0f ? 0.0f : 1.0f;
   }

   if( !dustTrailEmitter && dustTrailID != 0 )
   {
      if( !Sim::findObject( dustTrailID, dustTrailEmitter ) )
      {
         Con::errorf( ConsoleLogEntry::General, "RigidShapeData::preload Invalid packet, bad datablockId(dustTrailEmitter): 0x%x", dustTrailID );
      }
   }

   return true;
}   


//----------------------------------------------------------------------------

void RigidShapeData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(body.restitution);
   stream->write(body.friction);
   for( U32 i = 0; i < Body::MaxSounds; ++ i )
      sfxWrite( stream, body.sound[ i ] );

   stream->write(minImpactSpeed);
   stream->write(softImpactSpeed);
   stream->write(hardImpactSpeed);
   stream->write(minRollSpeed);

   stream->write(maxDrag);
   stream->write(minDrag);
   stream->write(integration);
   stream->write(collisionTol);
   stream->write(contactTol);
   mathWrite(*stream,massCenter);
   mathWrite(*stream,massBox);

   stream->writeFlag(cameraRoll);
   stream->write(cameraLag);
   stream->write(cameraDecay);
   stream->write(cameraOffset);

   stream->write( dustHeight );

   stream->write(exitSplashSoundVel);
   stream->write(softSplashSoundVel);
   stream->write(medSplashSoundVel);
   stream->write(hardSplashSoundVel);

   // write the water sound profiles
   for( U32 i = 0; i < MaxSounds; ++ i )
      sfxWrite( stream, waterSound[ i ] );

   if (stream->writeFlag( dustEmitter ))
      stream->writeRangedU32( dustEmitter->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );

   for( U32 i = 0; i < VC_NUM_SPLASH_EMITTERS; ++ i )
   {
      if( stream->writeFlag( splashEmitterList[i] != NULL ) )
         stream->writeRangedU32( splashEmitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   stream->write(splashFreqMod);
   stream->write(splashVelEpsilon);

   stream->write(dragForce);
   stream->write(vertFactor);

   if (stream->writeFlag( dustTrailEmitter ))
      stream->writeRangedU32( dustTrailEmitter->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
}   

void RigidShapeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&body.restitution);
   stream->read(&body.friction);

   for( U32 i = 0; i < Body::MaxSounds; i++)
      sfxRead( stream, &body.sound[ i ] );

   stream->read(&minImpactSpeed);
   stream->read(&softImpactSpeed);
   stream->read(&hardImpactSpeed);
   stream->read(&minRollSpeed);

   stream->read(&maxDrag);
   stream->read(&minDrag);
   stream->read(&integration);
   stream->read(&collisionTol);
   stream->read(&contactTol);
   mathRead(*stream,&massCenter);
   mathRead(*stream,&massBox);

   cameraRoll = stream->readFlag();
   stream->read(&cameraLag);
   stream->read(&cameraDecay);
   stream->read(&cameraOffset);

   stream->read( &dustHeight );

   stream->read(&exitSplashSoundVel);
   stream->read(&softSplashSoundVel);
   stream->read(&medSplashSoundVel);
   stream->read(&hardSplashSoundVel);

   // write the water sound profiles
   for( U32 i = 0; i < MaxSounds; ++ i )
      sfxRead( stream, &waterSound[ i ] );

   if( stream->readFlag() )
      dustID = (S32) stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   for( U32 i = 0; i < VC_NUM_SPLASH_EMITTERS; ++ i )
   {
      if( stream->readFlag() )
         splashEmitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   stream->read(&splashFreqMod);
   stream->read(&splashVelEpsilon);

   stream->read(&dragForce);
   stream->read(&vertFactor);

   if( stream->readFlag() )
      dustTrailID = (S32) stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
}   


//----------------------------------------------------------------------------

void RigidShapeData::initPersistFields()
{
   addField("massCenter", TypePoint3F, Offset(massCenter, RigidShapeData), "Center of mass for rigid body.");
   addField("massBox", TypePoint3F, Offset(massBox, RigidShapeData), "Size of inertial box.");
   addField("bodyRestitution", TypeF32, Offset(body.restitution, RigidShapeData), "The percentage of kinetic energy kept by this object in a collision.");
   addField("bodyFriction", TypeF32, Offset(body.friction, RigidShapeData), "How much friction this object has. Lower values will cause the object to appear to be more slippery.");

   addField("minImpactSpeed", TypeF32, Offset(minImpactSpeed, RigidShapeData),
      "Minimum collision speed to classify collision as impact (triggers onImpact on server object)." );
   addField("softImpactSpeed", TypeF32, Offset(softImpactSpeed, RigidShapeData), "Minimum speed at which this object must be travelling for the soft impact sound to be played.");
   addField("hardImpactSpeed", TypeF32, Offset(hardImpactSpeed, RigidShapeData), "Minimum speed at which the object must be travelling for the hard impact sound to be played.");
   addField("minRollSpeed", TypeF32, Offset(minRollSpeed, RigidShapeData));

   addField("maxDrag", TypeF32, Offset(maxDrag, RigidShapeData), "Maximum drag available to this object.");
   addField("minDrag", TypeF32, Offset(minDrag, RigidShapeData), "Minimum drag available to this object.");
   addField("integration", TypeS32, Offset(integration, RigidShapeData), "Number of physics steps to process per tick.");
   addField("collisionTol", TypeF32, Offset(collisionTol, RigidShapeData), "Collision distance tolerance.");
   addField("contactTol", TypeF32, Offset(contactTol, RigidShapeData), "Contact velocity tolerance.");
   
   addGroup( "Forces" );

      addField("dragForce",            TypeF32, Offset(dragForce,            RigidShapeData), "Used to simulate the constant drag acting on the object");
      addField("vertFactor",           TypeF32, Offset(vertFactor,           RigidShapeData), "The scalar applied to the vertical portion of the velocity drag acting on a object.");
   
   endGroup( "Forces" );
   
   addGroup( "Particle Effects" );

      addField("dustEmitter",       TYPEID< ParticleEmitterData >(),   Offset(dustEmitter,        RigidShapeData), "Array of pointers to ParticleEmitterData datablocks which will be used to emit particles at object/terrain contact point.\n");
      addField("triggerDustHeight", TypeF32,                      Offset(triggerDustHeight,  RigidShapeData), "Maximum height from the ground at which the object will generate dust.\n");
      addField("dustHeight",        TypeF32,                      Offset(dustHeight,         RigidShapeData), "Height of dust effects.\n");

      addField("dustTrailEmitter",     TYPEID< ParticleEmitterData >(),   Offset(dustTrailEmitter,   RigidShapeData), "Particle emitter used to create a dust trail for the moving object.\n");

      addField("splashEmitter",        TYPEID< ParticleEmitterData >(),   Offset(splashEmitterList,     RigidShapeData), VC_NUM_SPLASH_EMITTERS, "Array of pointers to ParticleEmitterData datablocks which will generate splash effects.\n");

      addField("splashFreqMod",  TypeF32,                Offset(splashFreqMod,   RigidShapeData), "The simulated frequency modulation of a splash generated by this object. Multiplied along with speed and time elapsed when determining splash emition rate.\n");
      addField("splashVelEpsilon", TypeF32,              Offset(splashVelEpsilon, RigidShapeData), "The threshold speed at which we consider the object's movement to have stopped when updating splash effects.\n");
      
   endGroup( "Particle Effects" );
   
   addGroup( "Sounds" );

      addField("softImpactSound", TypeSFXTrackName, Offset(body.sound[Body::SoftImpactSound], RigidShapeData),
         "Sound to play when body impacts with at least softImageSpeed but less than hardImpactSpeed." );
      addField("hardImpactSound", TypeSFXTrackName, Offset(body.sound[Body::HardImpactSound], RigidShapeData),
         "Sound to play when body impacts with at least hardImpactSpeed." );

      addField("exitSplashSoundVelocity", TypeF32,       Offset(exitSplashSoundVel, RigidShapeData), "The minimum velocity at which the exit splash sound will be played when emerging from water.\n");
      addField("softSplashSoundVelocity", TypeF32,       Offset(softSplashSoundVel, RigidShapeData),"The minimum velocity at which the soft splash sound will be played when impacting water.\n");
      addField("mediumSplashSoundVelocity", TypeF32,     Offset(medSplashSoundVel, RigidShapeData), "The minimum velocity at which the medium splash sound will be played when impacting water.\n");
      addField("hardSplashSoundVelocity", TypeF32,       Offset(hardSplashSoundVel, RigidShapeData), "The minimum velocity at which the hard splash sound will be played when impacting water.\n");
      addField("exitingWater",      TypeSFXTrackName,   Offset(waterSound[ExitWater],   RigidShapeData), "The AudioProfile will be used to produce sounds when emerging from water.\n");
      addField("impactWaterEasy",   TypeSFXTrackName,   Offset(waterSound[ImpactSoft],   RigidShapeData), "The AudioProfile will be used to produce sounds when a soft impact with water occurs.\n");
      addField("impactWaterMedium", TypeSFXTrackName,   Offset(waterSound[ImpactMedium],   RigidShapeData), "The AudioProfile will be used to produce sounds when a medium impact with water occurs.\n");
      addField("impactWaterHard",   TypeSFXTrackName,   Offset(waterSound[ImpactHard],   RigidShapeData), "The AudioProfile will be used to produce sounds when a hard impact with water occurs.\n");
      addField("waterWakeSound",    TypeSFXTrackName,   Offset(waterSound[Wake],   RigidShapeData), "The AudioProfile will be used to produce sounds when a water wake is displayed.\n");
      
   endGroup( "Sounds" );
   
   addGroup( "Camera" );

      addField("cameraRoll",     TypeBool,       Offset(cameraRoll,     RigidShapeData), "Specifies whether the camera's rotation matrix, and the render eye transform are multiplied during camera updates.\n");
      addField("cameraLag",      TypeF32,        Offset(cameraLag,      RigidShapeData), "Scalar amount by which the third person camera lags the object, relative to the object's linear velocity.\n");
      addField("cameraDecay",  TypeF32,        Offset(cameraDecay,  RigidShapeData), "Scalar rate at which the third person camera offset decays, per tick.\n");
      addField("cameraOffset",   TypeF32,        Offset(cameraOffset,   RigidShapeData), "The vertical offset of the object's camera.\n");
      
   endGroup( "Camera" );

   Parent::initPersistFields();
}   


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

RigidShape::RigidShape()
{

   mNetFlags.set(Ghostable);

   mDustTrailEmitter = NULL;

   mDataBlock = 0;
   // [rene, 27-Apr-11] WTH is a RigidShape a vehicle???
   mTypeMask |= VehicleObjectType | DynamicShapeObjectType;

   mDelta.pos = Point3F(0,0,0);
   mDelta.posVec = Point3F(0,0,0);
   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.dt = 1;
   mDelta.move = NullMove;
   mPredictionCount = 0;
   mDelta.cameraOffset.set(0,0,0);
   mDelta.cameraVec.set(0,0,0);
   mDelta.cameraRot.set(0,0,0);
   mDelta.cameraRotVec.set(0,0,0);

   mRigid.linPosition.set(0, 0, 0);
   mRigid.linVelocity.set(0, 0, 0);
   mRigid.angPosition.identity();
   mRigid.angVelocity.set(0, 0, 0);
   mRigid.linMomentum.set(0, 0, 0);
   mRigid.angMomentum.set(0, 0, 0);
   mContacts.clear();

   mCameraOffset.set(0,0,0);

   dMemset( mDustEmitterList, 0, sizeof( mDustEmitterList ) );
   dMemset( mSplashEmitterList, 0, sizeof( mSplashEmitterList ) );

   mDisableMove = false; // start frozen by default
   restCount = 0;

   inLiquid = false;
}   

RigidShape::~RigidShape()
{
   //
}

U32 RigidShape::getCollisionMask()
{
   if (isServerObject())
      return sServerCollisionMask;
   else
      return sClientCollisionMask;
}

Point3F RigidShape::getVelocity() const
{
   return mRigid.linVelocity;
}

//----------------------------------------------------------------------------

bool RigidShape::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // When loading from a mission script, the base SceneObject's transform
   // will have been set and needs to be transfered to the rigid body.
   mRigid.setTransform(mObjToWorld);

   // Initialize interpolation vars.      
   mDelta.rot[1] = mDelta.rot[0] = mRigid.angPosition;
   mDelta.pos = mRigid.linPosition;
   mDelta.posVec = Point3F(0,0,0);

   // Create Emitters on the client
   if( isClientObject() )
   {
      if( mDataBlock->dustEmitter )
      {
         for( U32 i=0; i<RigidShapeData::VC_NUM_DUST_EMITTERS; i++ )
         {
            mDustEmitterList[i] = new ParticleEmitter;
            mDustEmitterList[i]->onNewDataBlock( mDataBlock->dustEmitter, false );
            if( !mDustEmitterList[i]->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register dust emitter for class: %s", mDataBlock->getName() );
               delete mDustEmitterList[i];
               mDustEmitterList[i] = NULL;
            }
         }
      }

      for( U32 j=0; j<RigidShapeData::VC_NUM_SPLASH_EMITTERS; j++ )
      {
         if( mDataBlock->splashEmitterList[j] )
         {
            mSplashEmitterList[j] = new ParticleEmitter;
            mSplashEmitterList[j]->onNewDataBlock( mDataBlock->splashEmitterList[j], false );
            if( !mSplashEmitterList[j]->registerObject() )
            {
               Con::warnf( ConsoleLogEntry::General, "Could not register splash emitter for class: %s", mDataBlock->getName() );
               delete mSplashEmitterList[j];
               mSplashEmitterList[j] = NULL;
            }

         }
      }
   }

   // Create a new convex.
   AssertFatal(mDataBlock->collisionDetails[0] != -1, "Error, a rigid shape must have a collision-1 detail!");
   mConvex.mObject    = this;
   mConvex.pShapeBase = this;
   mConvex.hullId     = 0;
   mConvex.box        = mObjBox;
   mConvex.box.minExtents.convolve(mObjScale);
   mConvex.box.maxExtents.convolve(mObjScale);
   mConvex.findNodeTransform();

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
   }


   if (isServerObject())
      scriptOnAdd();

   return true;
}

void RigidShape::onRemove()
{
   scriptOnRemove();
   removeFromScene();

   U32 i=0;
   for( i=0; i<RigidShapeData::VC_NUM_DUST_EMITTERS; i++ )
   {
      if( mDustEmitterList[i] )
      {
         mDustEmitterList[i]->deleteWhenEmpty();
         mDustEmitterList[i] = NULL;
      }
   }

   for( i=0; i<RigidShapeData::VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( mSplashEmitterList[i] )
      {
         mSplashEmitterList[i]->deleteWhenEmpty();
         mSplashEmitterList[i] = NULL;
      }
   }

   Parent::onRemove();
}


//----------------------------------------------------------------------------

void RigidShape::processTick(const Move* move)
{     
   Parent::processTick(move);

   // Warp to catch up to server
   if (mDelta.warpCount < mDelta.warpTicks) 
   {
      mDelta.warpCount++;

      // Set new pos.
      mObjToWorld.getColumn(3,&mDelta.pos);
      mDelta.pos += mDelta.warpOffset;
      mDelta.rot[0] = mDelta.rot[1];
      mDelta.rot[1].interpolate(mDelta.warpRot[0],mDelta.warpRot[1],F32(mDelta.warpCount)/mDelta.warpTicks);
      setPosition(mDelta.pos,mDelta.rot[1]);

      // Pos backstepping
      mDelta.posVec.x = -mDelta.warpOffset.x;
      mDelta.posVec.y = -mDelta.warpOffset.y;
      mDelta.posVec.z = -mDelta.warpOffset.z;
   }
   else 
   {
      if (!move) 
      {
         if (isGhost()) 
         {
            // If we haven't run out of prediction time,
            // predict using the last known move.
            if (mPredictionCount-- <= 0)
               return;
            move = &mDelta.move;
         }
         else
            move = &NullMove;
      }

      // Process input move
      updateMove(move);

      // Save current rigid state interpolation
      mDelta.posVec = mRigid.linPosition;
      mDelta.rot[0] = mRigid.angPosition;

      // Update the physics based on the integration rate
      S32 count = mDataBlock->integration;
      updateWorkingCollisionSet(getCollisionMask());
      for (U32 i = 0; i < count; i++)
         updatePos(TickSec / count);

      // Wrap up interpolation info
      mDelta.pos     = mRigid.linPosition;
      mDelta.posVec -= mRigid.linPosition;
      mDelta.rot[1]  = mRigid.angPosition;

      // Update container database
      setPosition(mRigid.linPosition, mRigid.angPosition);
      setMaskBits(PositionMask);
      updateContainer();
   }
}

void RigidShape::interpolateTick(F32 dt)
{     
   Parent::interpolateTick(dt);

   if(dt == 0.0f)
      setRenderPosition(mDelta.pos, mDelta.rot[1]);
   else
   {
      QuatF rot;
      rot.interpolate(mDelta.rot[1], mDelta.rot[0], dt);
      Point3F pos = mDelta.pos + mDelta.posVec * dt;
      setRenderPosition(pos,rot);
   }
   mDelta.dt = dt;
}

void RigidShape::advanceTime(F32 dt)
{     
   Parent::advanceTime(dt);

   updateFroth(dt);

   // Update 3rd person camera offset.  Camera update is done
   // here as it's a client side only animation.
   mCameraOffset -=
      (mCameraOffset * mDataBlock->cameraDecay +
      mRigid.linVelocity * mDataBlock->cameraLag) * dt;
}


//----------------------------------------------------------------------------

bool RigidShape::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<RigidShapeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   // Update Rigid Info
   mRigid.mass = mDataBlock->mass;
   mRigid.oneOverMass = 1 / mRigid.mass;
   mRigid.friction = mDataBlock->body.friction;
   mRigid.restitution = mDataBlock->body.restitution;
   mRigid.setCenterOfMass(mDataBlock->massCenter);

   // Ignores massBox, just set sphere for now. Derived objects
   // can set what they want.
   mRigid.setObjectInertia();

   scriptOnNewDataBlock();

   return true;
}


//----------------------------------------------------------------------------

void RigidShape::getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot)
{
   *min = mDataBlock->cameraMinDist;
   *max = mDataBlock->cameraMaxDist;

   off->set(0,0,mDataBlock->cameraOffset);
   rot->identity();
}


//----------------------------------------------------------------------------

void RigidShape::getCameraTransform(F32* pos,MatrixF* mat)
{
   // Returns camera to world space transform
   // Handles first person / third person camera position
   if (isServerObject() && mShapeInstance)
      mShapeInstance->animateNodeSubtrees(true);

   if (*pos == 0) 
   {
      getRenderEyeTransform(mat);
      return;
   }

   // Get the shape's camera parameters.
   F32 min,max;
   MatrixF rot;
   Point3F offset;
   getCameraParameters(&min,&max,&offset,&rot);

   // Start with the current eye position
   MatrixF eye;
   getRenderEyeTransform(&eye);

   // Build a transform that points along the eye axis
   // but where the Z axis is always up.
   if (mDataBlock->cameraRoll)
      mat->mul(eye,rot);
   else 
   {
      MatrixF cam(1);
      VectorF x,y,z(0,0,1);
      eye.getColumn(1, &y);
      
      mCross(y, z, &x);
      x.normalize();
      mCross(x, y, &z);
      z.normalize();

      cam.setColumn(0,x);
      cam.setColumn(1,y);
      cam.setColumn(2,z);
      mat->mul(cam,rot);
   }

   // Camera is positioned straight back along the eye's -Y axis.
   // A ray is cast to make sure the camera doesn't go through
   // anything solid.
   VectorF vp,vec;
   vp.x = vp.z = 0;
   vp.y = -(max - min) * *pos;
   eye.mulV(vp,&vec);

   // Use the camera node as the starting position if it exists.
   Point3F osp,sp;
   if (mDataBlock->cameraNode != -1) 
   {
      mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);
      getRenderTransform().mulP(osp,&sp);
   }
   else
      eye.getColumn(3,&sp);

   // Make sure we don't hit ourself...
   disableCollision();
   if (isMounted())
      getObjectMount()->disableCollision();

   // Cast the ray into the container database to see if we're going
   // to hit anything.
   RayInfo collision;
   Point3F ep = sp + vec + offset + mCameraOffset;
   if (mContainer->castRay(sp, ep,
      ~(WaterObjectType | GameBaseObjectType | DefaultObjectType),
      &collision) == true) 
   {

         // Shift the collision point back a little to try and
         // avoid clipping against the front camera plane.
         F32 t = collision.t - (-mDot(vec, collision.normal) / vec.len()) * 0.1;
         if (t > 0.0f)
            ep = sp + offset + mCameraOffset + (vec * t);
         else
            eye.getColumn(3,&ep);
      }
      mat->setColumn(3,ep);

      // Re-enable our collision.
      if (isMounted())
         getObjectMount()->enableCollision();
      enableCollision();

   // Apply Camera FX.
   mat->mul( gCamFXMgr.getTrans() );
}


//----------------------------------------------------------------------------

void RigidShape::getVelocity(const Point3F& r, Point3F* v)
{
   mRigid.getVelocity(r, v);
}

void RigidShape::applyImpulse(const Point3F &pos, const Point3F &impulse)
{
   Point3F r;
   mRigid.getOriginVector(pos,&r);
   mRigid.applyImpulse(r, impulse);
}


//----------------------------------------------------------------------------

void RigidShape::updateMove(const Move* move)
{
   mDelta.move = *move;
}

//----------------------------------------------------------------------------

void RigidShape::setPosition(const Point3F& pos,const QuatF& rot)
{
   MatrixF mat;
   rot.setMatrix(&mat);
   mat.setColumn(3,pos);
   Parent::setTransform(mat);
}

void RigidShape::setRenderPosition(const Point3F& pos, const QuatF& rot)
{
   MatrixF mat;
   rot.setMatrix(&mat);
   mat.setColumn(3,pos);
   Parent::setRenderTransform(mat);
}

void RigidShape::setTransform(const MatrixF& newMat)
{
   mRigid.setTransform(newMat);
   Parent::setTransform(newMat);
   mRigid.atRest = false;
   mContacts.clear();
}

void RigidShape::forceClientTransform()
{
   setMaskBits(ForceMoveMask);
}


//-----------------------------------------------------------------------------

void RigidShape::disableCollision()
{
   Parent::disableCollision();
}

void RigidShape::enableCollision()
{
   Parent::enableCollision();
}   


//----------------------------------------------------------------------------
/** Update the physics
*/

void RigidShape::updatePos(F32 dt)
{
   Point3F origVelocity = mRigid.linVelocity;

   // Update internal forces acting on the body.
   mRigid.clearForces();
   updateForces(dt);

   // Update collision information based on our current pos.
   bool collided = false;
   if (!mRigid.atRest && !mDisableMove) 
   {
      collided = updateCollision(dt);

      // Now that all the forces have been processed, lets       
      // see if we're at rest.  Basically, if the kinetic energy of
      // the shape is less than some percentage of the energy added
      // by gravity for a short period, we're considered at rest.
      // This should really be part of the rigid class...
      if (mCollisionList.getCount()) 
      {
         F32 k = mRigid.getKineticEnergy();
         F32 G = sRigidShapeGravity * dt;
         F32 Kg = 0.5 * mRigid.mass * G * G;
         if (k < sRestTol * Kg && ++restCount > sRestCount)
            mRigid.setAtRest();
      }
      else
         restCount = 0;
   }

   // Integrate forward
   if (!mRigid.atRest && !mDisableMove)
      mRigid.integrate(dt);

   // Deal with client and server scripting, sounds, etc.
   if (isServerObject()) 
   {

      // Check triggers and other objects that we normally don't
      // collide with.  This function must be called before notifyCollision
      // as it will queue collision.
      checkTriggers();

      // Invoke the onCollision notify callback for all the objects
      // we've just hit.
      notifyCollision();

      // Server side impact script callback
      if (collided) 
      {
         VectorF collVec = mRigid.linVelocity - origVelocity;
         F32 collSpeed = collVec.len();
         if (collSpeed > mDataBlock->minImpactSpeed)
            onImpact(collVec);
      }

      // Water script callbacks      
      if (!inLiquid && mWaterCoverage != 0.0f) 
      {
         onEnterLiquid_callback(getIdString(), Con::getFloatArg(mWaterCoverage), mLiquidType.c_str() );
         inLiquid = true;
      }
      else if (inLiquid && mWaterCoverage == 0.0f) 
      {
		 onLeaveLiquid_callback(getIdString(), mLiquidType.c_str() );
         inLiquid = false;
      }

   }
   else {

      // Play impact sounds on the client.
      if (collided) {
         F32 collSpeed = (mRigid.linVelocity - origVelocity).len();
         S32 impactSound = -1;
         if (collSpeed >= mDataBlock->hardImpactSpeed)
            impactSound = RigidShapeData::Body::HardImpactSound;
         else
            if (collSpeed >= mDataBlock->softImpactSpeed)
               impactSound = RigidShapeData::Body::SoftImpactSound;

         if (impactSound != -1 && mDataBlock->body.sound[impactSound] != NULL)
            SFX->playOnce(mDataBlock->body.sound[impactSound], &getTransform());
      }

      // Water volume sounds
      F32 vSpeed = getVelocity().len();
      if (!inLiquid && mWaterCoverage >= 0.8f) {
         if (vSpeed >= mDataBlock->hardSplashSoundVel) 
            SFX->playOnce(mDataBlock->waterSound[RigidShapeData::ImpactHard], &getTransform());
         else
            if (vSpeed >= mDataBlock->medSplashSoundVel)
               SFX->playOnce(mDataBlock->waterSound[RigidShapeData::ImpactMedium], &getTransform());
            else
               if (vSpeed >= mDataBlock->softSplashSoundVel)
                  SFX->playOnce(mDataBlock->waterSound[RigidShapeData::ImpactSoft], &getTransform());
         inLiquid = true;
      }   
      else
         if(inLiquid && mWaterCoverage < 0.8f) {
            if (vSpeed >= mDataBlock->exitSplashSoundVel)
               SFX->playOnce(mDataBlock->waterSound[RigidShapeData::ExitWater], &getTransform());
            inLiquid = false;
         }
   }
}


//----------------------------------------------------------------------------

void RigidShape::updateForces(F32 /*dt*/)
{
   Point3F gravForce(0, 0, sRigidShapeGravity * mRigid.mass * mGravityMod);

   MatrixF currTransform;
   mRigid.getTransform(&currTransform);
   mRigid.atRest = false;

   Point3F torque(0, 0, 0);
   Point3F force(0, 0, 0);

   Point3F vel = mRigid.linVelocity;

   // Gravity
   force += gravForce;

   // Apply drag
   Point3F vDrag = mRigid.linVelocity;
   vDrag.convolve(Point3F(1, 1, mDataBlock->vertFactor));
   force -= vDrag * mDataBlock->dragForce;

   // Add in physical zone force
   force += mAppliedForce;

   // Container buoyancy & drag
   force  += Point3F(0, 0,-mBuoyancy * sRigidShapeGravity * mRigid.mass * mGravityMod);
   force  -= mRigid.linVelocity * mDrag;
   torque -= mRigid.angMomentum * mDrag;

   mRigid.force  = force;
   mRigid.torque = torque;
}


//-----------------------------------------------------------------------------
/** Update collision information
Update the convex state and check for collisions. If the object is in
collision, impact and contact forces are generated.
*/

bool RigidShape::updateCollision(F32 dt)
{
   // Update collision information
   MatrixF mat,cmat;
   mConvex.transform = &mat;
   mRigid.getTransform(&mat);
   cmat = mConvex.getTransform();

   mCollisionList.clear();
   CollisionState *state = mConvex.findClosestState(cmat, getScale(), mDataBlock->collisionTol);
   if (state && state->dist <= mDataBlock->collisionTol) 
   {
      //resolveDisplacement(ns,state,dt);
      mConvex.getCollisionInfo(cmat, getScale(), &mCollisionList, mDataBlock->collisionTol);
   }

   // Resolve collisions
   bool collided = resolveCollision(mRigid,mCollisionList);
   resolveContacts(mRigid,mCollisionList,dt);
   return collided;
}


//----------------------------------------------------------------------------
/** Resolve collision impacts
Handle collision impacts, as opposed to contacts. Impulses are calculated based
on standard collision resolution formulas.
*/
bool RigidShape::resolveCollision(Rigid&  ns,CollisionList& cList)
{
   // Apply impulses to resolve collision
   bool colliding, collided = false;

   do 
   {
      colliding = false;
      for (S32 i = 0; i < cList.getCount(); i++) 
      {
         Collision& c = cList[i];
         if (c.distance < mDataBlock->collisionTol) 
         {
            // Velocity into surface
            Point3F v,r;
            ns.getOriginVector(c.point,&r);
            ns.getVelocity(r,&v);
            F32 vn = mDot(v,c.normal);

            // Only interested in velocities greater than sContactTol,
            // velocities less than that will be dealt with as contacts
            // "constraints".
            if (vn < -mDataBlock->contactTol) 
            {

               // Apply impulses to the rigid body to keep it from
               // penetrating the surface.
               ns.resolveCollision(cList[i].point,
                  cList[i].normal);
               colliding = collided  = true;

               // Keep track of objects we collide with
               if (!isGhost() && c.object->getTypeMask() & ShapeBaseObjectType) 
               {
                  ShapeBase* col = static_cast<ShapeBase*>(c.object);
                  queueCollision(col,v - col->getVelocity());
               }
            }
         }
      }
   } while (colliding);

   return collided;
}

//----------------------------------------------------------------------------
/** Resolve contact forces
Resolve contact forces using the "penalty" method. Forces are generated based
on the depth of penetration and the moment of inertia at the point of contact.
*/
bool RigidShape::resolveContacts(Rigid& ns,CollisionList& cList,F32 dt)
{
   // Use spring forces to manage contact constraints.
   bool collided = false;
   Point3F t,p(0,0,0),l(0,0,0);
   for (S32 i = 0; i < cList.getCount(); i++) 
   {
      Collision& c = cList[i];
      if (c.distance < mDataBlock->collisionTol) 
      {

         // Velocity into the surface
         Point3F v,r;
         ns.getOriginVector(c.point,&r);
         ns.getVelocity(r,&v);
         F32 vn = mDot(v,c.normal);

         // Only interested in velocities less than mDataBlock->contactTol,
         // velocities greater than that are dealt with as collisions.
         if (mFabs(vn) < mDataBlock->contactTol) 
         {
            collided = true;

            // Penetration force. This is actually a spring which
            // will seperate the body from the collision surface.
            F32 zi = 2 * mFabs(mRigid.getZeroImpulse(r,c.normal));
            F32 s = (mDataBlock->collisionTol - c.distance) * zi - ((vn / mDataBlock->contactTol) * zi);
            Point3F f = c.normal * s;

            // Friction impulse, calculated as a function of the
            // amount of force it would take to stop the motion
            // perpendicular to the normal.
            Point3F uv = v - (c.normal * vn);
            F32 ul = uv.len();
            if (s > 0 && ul) 
            {
               uv /= -ul;
               F32 u = ul * ns.getZeroImpulse(r,uv);
               s *= mRigid.friction;
               if (u > s)
                  u = s;
               f += uv * u;
            }

            // Accumulate forces
            p += f;
            mCross(r,f,&t);
            l += t;
         }
      }
   }

   // Contact constraint forces act over time...
   ns.linMomentum += p * dt;
   ns.angMomentum += l * dt;
   ns.updateVelocity();
   return true;
}


//----------------------------------------------------------------------------

bool RigidShape::resolveDisplacement(Rigid& ns,CollisionState *state, F32 dt)
{
   SceneObject* obj = (state->a->getObject() == this)?
      state->b->getObject(): state->a->getObject();

   if (obj->isDisplacable() && ((obj->getTypeMask() & ShapeBaseObjectType) != 0))
   {
      // Try to displace the object by the amount we're trying to move
      Point3F objNewMom = ns.linVelocity * obj->getMass() * 1.1f;
      Point3F objOldMom = obj->getMomentum();
      Point3F objNewVel = objNewMom / obj->getMass();

      Point3F myCenter;
      Point3F theirCenter;
      getWorldBox().getCenter(&myCenter);
      obj->getWorldBox().getCenter(&theirCenter);
      if (mDot(myCenter - theirCenter, objNewMom) >= 0.0f || objNewVel.len() < 0.01)
      {
         objNewMom = (theirCenter - myCenter);
         objNewMom.normalize();
         objNewMom *= 1.0f * obj->getMass();
         objNewVel = objNewMom / obj->getMass();
      }

      obj->setMomentum(objNewMom);
      if (obj->displaceObject(objNewVel * 1.1f * dt) == true)
      {
         // Queue collision and change in velocity
         VectorF dv = (objOldMom - objNewMom) / obj->getMass();
         queueCollision(static_cast<ShapeBase*>(obj), dv);
         return true;
      }
   }

   return false;
}   


//----------------------------------------------------------------------------

void RigidShape::updateWorkingCollisionSet(const U32 mask)
{
   Box3F convexBox = mConvex.getBoundingBox(getTransform(), getScale());
   F32 len = (mRigid.linVelocity.len() + 50) * TickSec;
   F32 l = (len * 1.1) + 0.1;  // fudge factor
   convexBox.minExtents -= Point3F(l, l, l);
   convexBox.maxExtents += Point3F(l, l, l);

   disableCollision();
   mConvex.updateWorkingList(convexBox, mask);
   enableCollision();
}


//----------------------------------------------------------------------------
/** Check collisions with trigger and items
Perform a container search using the current bounding box
of the main body, wheels are not included.  This method should
only be called on the server.
*/
void RigidShape::checkTriggers()
{
   Box3F bbox = mConvex.getBoundingBox(getTransform(), getScale());
   gServerContainer.findObjects(bbox,sTriggerMask,findCallback,this);
}

/** The callback used in by the checkTriggers() method.
The checkTriggers method uses a container search which will
invoke this callback on each obj that matches.
*/
void RigidShape::findCallback(SceneObject* obj,void *key)
{
   RigidShape* shape = reinterpret_cast<RigidShape*>(key);
   U32 objectMask = obj->getTypeMask();

   // Check: triggers, corpses and items, basically the same things
   // that the player class checks for
   if (objectMask & TriggerObjectType) {
      Trigger* pTrigger = static_cast<Trigger*>(obj);
      pTrigger->potentialEnterObject(shape);
   }
   else if (objectMask & CorpseObjectType) {
      ShapeBase* col = static_cast<ShapeBase*>(obj);
      shape->queueCollision(col,shape->getVelocity() - col->getVelocity());
   }
   else if (objectMask & ItemObjectType) {
      Item* item = static_cast<Item*>(obj);
      if (shape != item->getCollisionObject())
         shape->queueCollision(item,shape->getVelocity() - item->getVelocity());
   }
}


//----------------------------------------------------------------------------

void RigidShape::writePacketData(GameConnection *connection, BitStream *stream)
{
   Parent::writePacketData(connection, stream);

   mathWrite(*stream, mRigid.linPosition);
   mathWrite(*stream, mRigid.angPosition);
   mathWrite(*stream, mRigid.linMomentum);
   mathWrite(*stream, mRigid.angMomentum);
   stream->writeFlag(mRigid.atRest);
   stream->writeFlag(mContacts.getCount() == 0);

   stream->writeFlag(mDisableMove);
   stream->setCompressionPoint(mRigid.linPosition);
}

void RigidShape::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);

   mathRead(*stream, &mRigid.linPosition);
   mathRead(*stream, &mRigid.angPosition);
   mathRead(*stream, &mRigid.linMomentum);
   mathRead(*stream, &mRigid.angMomentum);
   mRigid.atRest = stream->readFlag();
   if (stream->readFlag())
      mContacts.clear();
   mRigid.updateInertialTensor();
   mRigid.updateVelocity();

   mDisableMove = stream->readFlag();
   stream->setCompressionPoint(mRigid.linPosition);
}   


//----------------------------------------------------------------------------

U32 RigidShape::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if (stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return retMask;

   mDelta.move.pack(stream);

   if (stream->writeFlag(mask & PositionMask))
   {
      stream->writeFlag(mask & ForceMoveMask);

      stream->writeCompressedPoint(mRigid.linPosition);
      mathWrite(*stream, mRigid.angPosition);
      mathWrite(*stream, mRigid.linMomentum);
      mathWrite(*stream, mRigid.angMomentum);
      stream->writeFlag(mRigid.atRest);
   }
   
   if(stream->writeFlag(mask & FreezeMask))
      stream->writeFlag(mDisableMove);

   return retMask;
}   

void RigidShape::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con,stream);

   if (stream->readFlag())
      return;

   mDelta.move.unpack(stream);

   if (stream->readFlag()) 
   {
      // Check if we need to jump to the given transform
      // rather than interpolate to it.
      bool forceUpdate = stream->readFlag();

      mPredictionCount = sMaxPredictionTicks;
      F32 speed = mRigid.linVelocity.len();
      mDelta.warpRot[0] = mRigid.angPosition;

      // Read in new position and momentum values
      stream->readCompressedPoint(&mRigid.linPosition);
      mathRead(*stream, &mRigid.angPosition);
      mathRead(*stream, &mRigid.linMomentum);
      mathRead(*stream, &mRigid.angMomentum);
      mRigid.atRest = stream->readFlag();
      mRigid.updateVelocity();

      if (!forceUpdate && isProperlyAdded()) 
      {
         // Determine number of ticks to warp based on the average
         // of the client and server velocities.
         Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
         mDelta.warpOffset = mRigid.linPosition - cp;

         // Calc the distance covered in one tick as the average of
         // the old speed and the new speed from the server.
         F32 dt,as = (speed + mRigid.linVelocity.len()) * 0.5 * TickSec;

         // Cal how many ticks it will take to cover the warp offset.
         // If it's less than what's left in the current tick, we'll just
         // warp in the remaining time.
         if (!as || (dt = mDelta.warpOffset.len() / as) > sMaxWarpTicks)
            dt = mDelta.dt + sMaxWarpTicks;
         else
            dt = (dt <= mDelta.dt)? mDelta.dt : mCeil(dt - mDelta.dt) + mDelta.dt;

         // Adjust current frame interpolation
         if (mDelta.dt) 
         {
            mDelta.pos = cp + (mDelta.warpOffset * (mDelta.dt / dt));
            mDelta.posVec = (cp - mDelta.pos) / mDelta.dt;
            QuatF cr;
            cr.interpolate(mDelta.rot[1],mDelta.rot[0],mDelta.dt);
            mDelta.rot[1].interpolate(cr,mRigid.angPosition,mDelta.dt / dt);
            mDelta.rot[0].extrapolate(mDelta.rot[1],cr,mDelta.dt);
         }

         // Calculated multi-tick warp
         mDelta.warpCount = 0;
         mDelta.warpTicks = (S32)(mFloor(dt));
         if (mDelta.warpTicks) 
         {
            mDelta.warpOffset = mRigid.linPosition - mDelta.pos;
            mDelta.warpOffset /= mDelta.warpTicks;
            mDelta.warpRot[0] = mDelta.rot[1];
            mDelta.warpRot[1] = mRigid.angPosition;
         }
      }
      else 
      {
         // Set the shape to the server position
         mDelta.dt  = 0;
         mDelta.pos = mRigid.linPosition;
         mDelta.posVec.set(0,0,0);
         mDelta.rot[1] = mDelta.rot[0] = mRigid.angPosition;
         mDelta.warpCount = mDelta.warpTicks = 0;
         setPosition(mRigid.linPosition, mRigid.angPosition);
      }
   }
   
   if(stream->readFlag())
      mDisableMove = stream->readFlag();
}


//----------------------------------------------------------------------------

void RigidShape::initPersistFields()
{
   Parent::initPersistFields();
}

//----------------------------------------------------------------------------

void RigidShape::updateLiftoffDust( F32 dt )
{
   Point3F offset( 0.0, 0.0, mDataBlock->dustHeight );
   emitDust( mDustEmitterList[ 0 ], mDataBlock->triggerDustHeight, offset,
             ( U32 )( dt * 1000 ) );
}

//--------------------------------------------------------------------------
void RigidShape::updateFroth( F32 dt )
{
   // update bubbles
   Point3F moveDir = getVelocity();

   Point3F contactPoint;

   F32 speed = moveDir.len();
   if( speed < mDataBlock->splashVelEpsilon ) speed = 0.0;

   U32 emitRate = (U32)(speed * mDataBlock->splashFreqMod * dt);

   U32 i;
   for( i=0; i<RigidShapeData::VC_NUM_SPLASH_EMITTERS; i++ )
   {
      if( mSplashEmitterList[i] )
      {
         mSplashEmitterList[i]->emitParticles( contactPoint, contactPoint, Point3F( 0.0, 0.0, 1.0 ), 
            moveDir, emitRate );
      }
   }

}

//--------------------------------------------------------------------------
// Returns true if shape is intersecting a water surface (roughly)
//--------------------------------------------------------------------------
bool RigidShape::collidingWithWater( Point3F &waterHeight )
{
   Point3F curPos = getPosition();

   F32 height = mFabs( mObjBox.maxExtents.z - mObjBox.minExtents.z );

   RayInfo rInfo;
   if( gClientContainer.castRay( curPos + Point3F(0.0, 0.0, height), curPos, WaterObjectType, &rInfo) )
   {
      waterHeight = rInfo.point;
      return true;
   }

   return false;
}

void RigidShape::setEnergyLevel(F32 energy)
{
   Parent::setEnergyLevel(energy);
   setMaskBits(EnergyMask);
}

void RigidShape::prepBatchRender( SceneRenderState *state, S32 mountedImageIndex )
{
   Parent::prepBatchRender( state, mountedImageIndex );

   if ( !gShowBoundingBox )
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &RigidShape::_renderMassAndContacts );
   ri->type = RenderPassManager::RIT_Editor;
   state->getRenderPass()->addInst( ri );
}

void RigidShape::_renderMassAndContacts( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   // Box for the center of Mass
   GFXStateBlockDesc desc;
   desc.setBlend(false, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   desc.setZReadWrite(false);
   desc.fillMode = GFXFillWireframe;

   GFX->getDrawUtil()->drawCube( desc, Point3F(0.1f,0.1f,0.1f), mDataBlock->massCenter, ColorI(255, 255, 255), &mRenderObjToWorld );

   // Collision points...
   for (int i = 0; i < mCollisionList.getCount(); i++)
   {
      const Collision& collision = mCollisionList[i];
      GFX->getDrawUtil()->drawCube( desc, Point3F(0.05f,0.05f,0.05f), collision.point, ColorI(0, 0, 255) );
   }

   // Render the normals as one big batch... 
   PrimBuild::begin(GFXLineList, mCollisionList.getCount() * 2);
   for (int i = 0; i < mCollisionList.getCount(); i++)
   {

      const Collision& collision = mCollisionList[i];
      PrimBuild::color3f(1, 1, 1);
      PrimBuild::vertex3fv(collision.point);
      PrimBuild::vertex3fv(collision.point + collision.normal * 0.05f);
   }
   PrimBuild::end();

   // Build and render the collision polylist which is returned
   // in the server's world space.
   ClippedPolyList polyList;
   polyList.mPlaneList.setSize(6);
   polyList.mPlaneList[0].set(getWorldBox().minExtents,VectorF(-1,0,0));
   polyList.mPlaneList[1].set(getWorldBox().minExtents,VectorF(0,-1,0));
   polyList.mPlaneList[2].set(getWorldBox().minExtents,VectorF(0,0,-1));
   polyList.mPlaneList[3].set(getWorldBox().maxExtents,VectorF(1,0,0));
   polyList.mPlaneList[4].set(getWorldBox().maxExtents,VectorF(0,1,0));
   polyList.mPlaneList[5].set(getWorldBox().maxExtents,VectorF(0,0,1));
   Box3F dummyBox;
   SphereF dummySphere;
   buildPolyList(PLC_Collision, &polyList, dummyBox, dummySphere);
   //polyList.render();
}

void RigidShape::reset()
{
   mRigid.clearForces();
   mRigid.setAtRest();
}

void RigidShape::freezeSim(bool frozen)
{
   mDisableMove = frozen;
   setMaskBits(FreezeMask);
}

DefineEngineMethod( RigidShape, reset, void, (),,
   "@brief Clears physic forces from the shape and sets it at rest.\n\n"
   "@tsexample\n"
   "// Inform the RigidShape object to reset.\n"
   "%thisRigidShape.reset();\n"
   "@endtsexample\n\n"
   "@see ShapeBaseData")
{
   object->reset();
}

DefineEngineMethod( RigidShape, freezeSim, void, (bool isFrozen),,
   "@brief Enables or disables the physics simulation on the RigidShape object.\n\n"
   "@param isFrozen Boolean frozen state to set the object.\n"
   "@tsexample\n"
   "// Define the frozen state.\n"
   "%isFrozen = \"true\";\n\n"
   "// Inform the object of the defined frozen state\n"
   "%thisRigidShape.freezeSim(%isFrozen);\n"
   "@endtsexample\n\n"
   "@see ShapeBaseData")
{
   object->freezeSim(isFrozen);
}

DefineEngineMethod( RigidShape, forceClientTransform, void, (),,
   "@brief Forces the client to jump to the RigidShape's transform rather then warp to it.\n\n")
{
   if(object->isServerObject())
   {
      object->forceClientTransform();
   }
}
