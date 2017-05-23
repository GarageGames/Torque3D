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
#include "T3D/fx/explosion.h"

#include "core/resourceManager.h"
#include "console/consoleTypes.h"
#include "console/typeValidators.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "math/mRandom.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "T3D/debris.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/fx/particleEmitter.h"
#include "T3D/fx/cameraFXMgr.h"
#include "T3D/debris.h"
#include "T3D/shapeBase.h"
#include "T3D/gameBase/gameProcess.h"
#include "renderInstance/renderPassManager.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(Explosion);

ConsoleDocClass( Explosion,
   "@brief The emitter for an explosion effect, with properties defined by a "
   "ExplosionData object.\n\n"
   "@ingroup FX\n"
   "The object will initiate the explosion effects automatically after being "
   "added to the simulation.\n"
   "@tsexample\n"
   "datablock ExplosionData( GrenadeSubExplosion )\n"
   "{\n"
   "   offset = 0.25;\n"
   "   emitter[0] = GrenadeExpSparkEmitter;\n\n"
   "   lightStartRadius = 4.0;\n"
   "   lightEndRadius = 0.0;\n"
   "   lightStartColor = \"0.9 0.7 0.7\";\n"
   "   lightEndColor = \"0.9 0.7 0.7\";\n"
   "   lightStartBrightness = 2.0;\n"
   "   lightEndBrightness = 0.0;\n"
   "};\n\n"
   "datablock ExplosionData( GrenadeLauncherExplosion )\n"
   "{\n"
   "   soundProfile = GrenadeLauncherExplosionSound;\n"
   "   lifeTimeMS = 400; // Quick flash, short burn, and moderate dispersal\n\n"
   "   // Volume particles\n"
   "   particleEmitter = GrenadeExpFireEmitter;\n"
   "   particleDensity = 75;\n"
   "   particleRadius = 2.25;\n\n"
   "   // Point emission\n"
   "   emitter[0] = GrenadeExpDustEmitter;\n"
   "   emitter[1] = GrenadeExpSparksEmitter;\n"
   "   emitter[2] = GrenadeExpSmokeEmitter;\n\n"
   "   // Sub explosion objects\n"
   "   subExplosion[0] = GrenadeSubExplosion;\n\n"
   "   // Camera Shaking\n"
   "   shakeCamera = true;\n"
   "   camShakeFreq = \"10.0 11.0 9.0\";\n"
   "   camShakeAmp = \"15.0 15.0 15.0\";\n"
   "   camShakeDuration = 1.5;\n"
   "   camShakeRadius = 20;\n\n"
   "   // Exploding debris\n"
   "   debris = GrenadeDebris;\n"
   "   debrisThetaMin = 10;\n"
   "   debrisThetaMax = 60;\n"
   "   debrisNum = 4;\n"
   "   debrisNumVariance = 2;\n"
   "   debrisVelocity = 25;\n"
   "   debrisVelocityVariance = 5;\n\n"
   "   lightStartRadius = 4.0;\n"
   "   lightEndRadius = 0.0;\n"
   "   lightStartColor = \"1.0 1.0 1.0\";\n"
   "   lightEndColor = \"1.0 1.0 1.0\";\n"
   "   lightStartBrightness = 4.0;\n"
   "   lightEndBrightness = 0.0;\n"
   "   lightNormalOffset = 2.0;\n"
   "};\n\n"
   "function ServerPlayExplosion(%position, %datablock)\n"
   "{\n"
   "   // Play the given explosion on every client.\n"
   "   // The explosion will be transmitted as an event, not attached to any object.\n"
   "   for(%idx = 0; %idx < ClientGroup.getCount(); %idx++)\n"
   "   {\n"
   "      %client = ClientGroup.getObject(%idx);\n"
   "      commandToClient(%client, 'PlayExplosion', %position, %datablock.getId());\n"
   "   }\n"
   "}\n\n"
   "function clientCmdPlayExplosion(%position, %effectDataBlock)\n"
   "{\n"
   "   // Play an explosion sent by the server. Make sure this function is defined\n"
   "   // on the client.\n"
   "   if (isObject(%effectDataBlock))\n"
   "   {\n"
   "      new Explosion()\n"
   "      {\n"
   "         position = %position;\n"
   "         dataBlock = %effectDataBlock;\n"
   "      };\n"
   "   }\n"
   "}\n\n"
   "// schedule an explosion\n"
   "schedule(1000, 0, ServerPlayExplosion, \"0 0 0\", GrenadeLauncherExplosion);\n"
   "@endtsexample"
);

#define MaxLightRadius 20

MRandomLCG sgRandom(0xdeadbeef);

//WLE - Vince - The defaults are bad, the whole point of calling this function\
//is to determine the explosion coverage on a object.  Why would you want them
//To call this with a null for the ID?  In fact, it just returns a 1f if
//it can't find the object.  Seems useless to me.  Cause how can I apply
//damage to a object that doesn't exist?

//I could possible see a use with passing in a null covMask, but even that
//sounds flaky because it will be 100 percent if your saying not to take
//any thing in consideration for coverage.  So I'm removing these defaults they are just bad.

DefineEngineFunction(calcExplosionCoverage, F32, (Point3F pos, S32 id, U32 covMask),,
   "@brief Calculates how much an explosion effects a specific object.\n\n"
   "Use this to determine how much damage to apply to objects based on their "
   "distance from the explosion's center point, and whether the explosion is "
   "blocked by other objects.\n\n"
   "@param pos Center position of the explosion.\n"
   "@param id Id of the object of which to check coverage.\n"
   "@param covMask Mask of object types that may block the explosion.\n"
   "@return Coverage value from 0 (not affected by the explosion) to 1 (fully affected)\n\n"
   "@tsexample\n"
   "// Get the position of the explosion.\n"
   "%position = %explosion.getPosition();\n\n"
   "// Set a list of TypeMasks (defined in gameFunctioncs.cpp), seperated by the | character.\n"
   "%TypeMasks = $TypeMasks::StaticObjectType | $TypeMasks::ItemObjectType\n\n"
   "// Acquire the damage value from 0.0f - 1.0f.\n"
   "%coverage = calcExplosionCoverage( %position, %sceneObject, %TypeMasks );\n\n"
   "// Apply damage to object\n" 
   "%sceneObject.applyDamage( %coverage * 20 );\n"
   "@endtsexample\n"
   "@ingroup FX")
{
   Point3F center;

   SceneObject* sceneObject = NULL;
   if (Sim::findObject(id, sceneObject) == false) {
      Con::warnf(ConsoleLogEntry::General, "calcExplosionCoverage: couldn't find object: %d", id);
      return 1.0f;
   }
   if (sceneObject->isClientObject() || sceneObject->getContainer() == NULL) {
      Con::warnf(ConsoleLogEntry::General, "calcExplosionCoverage: object is on the client, or not in the container system");
      return 1.0f;
   }

   sceneObject->getObjBox().getCenter(&center);
   center.convolve(sceneObject->getScale());
   sceneObject->getTransform().mulP(center);

   RayInfo rayInfo;
   sceneObject->disableCollision();
   if (sceneObject->getContainer()->castRay(pos, center, covMask, &rayInfo) == true) {
      // Try casting up and then out
      if (sceneObject->getContainer()->castRay(pos, pos + Point3F(0.0f, 0.0f, 1.0f), covMask, &rayInfo) == false)
      {
         if (sceneObject->getContainer()->castRay(pos + Point3F(0.0f, 0.0f, 1.0f), center, covMask, &rayInfo) == false)
         {
            sceneObject->enableCollision();
            return 1.0f;
         }
      }

      sceneObject->enableCollision();
      return 0.0f;
   } else {
      sceneObject->enableCollision();
      return 1.0f;
   }
}

//----------------------------------------------------------------------------
//
IMPLEMENT_CO_DATABLOCK_V1(ExplosionData);

ConsoleDocClass( ExplosionData,
   "@brief Defines the attributes of an Explosion: particleEmitters, debris, "
   "lighting and camera shake effects.\n"
   "@ingroup FX\n"
);

ExplosionData::ExplosionData()
{
   dtsFileName  = NULL;
   particleDensity = 10;
   particleRadius = 1.0f;

   faceViewer   = false;

   soundProfile      = NULL;
   particleEmitter   = NULL;
   particleEmitterId = 0;

   explosionScale.set(1.0f, 1.0f, 1.0f);
   playSpeed = 1.0f;

   dMemset( emitterList, 0, sizeof( emitterList ) );
   dMemset( emitterIDList, 0, sizeof( emitterIDList ) );
   dMemset( debrisList, 0, sizeof( debrisList ) );
   dMemset( debrisIDList, 0, sizeof( debrisIDList ) );

   debrisThetaMin = 0.0f;
   debrisThetaMax = 90.0f;
   debrisPhiMin = 0.0f;
   debrisPhiMax = 360.0f;
   debrisNum = 1;
   debrisNumVariance = 0;
   debrisVelocity = 2.0f;
   debrisVelocityVariance = 0.0f;

   dMemset( explosionList, 0, sizeof( explosionList ) );
   dMemset( explosionIDList, 0, sizeof( explosionIDList ) );

   delayMS = 0;
   delayVariance = 0;
   lifetimeMS = 1000;
   lifetimeVariance = 0;
   offset = 0.0f;

   shakeCamera = false;
   camShakeFreq.set( 10.0f, 10.0f, 10.0f );
   camShakeAmp.set( 1.0f, 1.0f, 1.0f );
   camShakeDuration = 1.5f;
   camShakeRadius = 10.0f;
   camShakeFalloff = 10.0f;

   for( U32 i=0; i<EC_NUM_TIME_KEYS; i++ )
   {
      times[i] = 1.0f;
   }
   times[0] = 0.0f;

   for( U32 j=0; j<EC_NUM_TIME_KEYS; j++ )
   {
      sizes[j].set( 1.0f, 1.0f, 1.0f );
   }

   //
   lightStartRadius = lightEndRadius = 0.0f;
   lightStartColor.set(1.0f,1.0f,1.0f);
   lightEndColor.set(1.0f,1.0f,1.0f);
   lightStartBrightness = 1.0f;
   lightEndBrightness = 1.0f;
   lightNormalOffset = 0.1f;
}

void ExplosionData::initPersistFields()
{
   addField( "explosionShape", TypeShapeFilename, Offset(dtsFileName, ExplosionData),
      "@brief Optional DTS or DAE shape to place at the center of the explosion.\n\n"
      "The <i>ambient</i> animation of this model will be played automatically at "
      "the start of the explosion." );
   addField( "explosionScale", TypePoint3F, Offset(explosionScale, ExplosionData),
      "\"X Y Z\" scale factor applied to the explosionShape model at the start "
      "of the explosion." );
   addField( "playSpeed", TypeF32, Offset(playSpeed, ExplosionData),
      "Time scale at which to play the explosionShape <i>ambient</i> sequence." );
   addField( "soundProfile", TYPEID< SFXTrack >(), Offset(soundProfile, ExplosionData),
      "Non-looping sound effect that will be played at the start of the explosion." );
   addField( "faceViewer", TypeBool, Offset(faceViewer, ExplosionData),
      "Controls whether the visual effects of the explosion always face the camera." );

   addField( "particleEmitter", TYPEID< ParticleEmitterData >(), Offset(particleEmitter, ExplosionData),
      "@brief Emitter used to generate a cloud of particles at the start of the explosion.\n\n"
      "Explosions can generate two different particle effects. The first is a "
      "single burst of particles at the start of the explosion emitted in a "
      "spherical cloud using particleEmitter.\n\n"
      "The second effect spawns the list of ParticleEmitters given by the emitter[] "
      "field. These emitters generate particles in the normal way throughout the "
      "lifetime of the explosion." );
   addField( "particleDensity", TypeS32, Offset(particleDensity, ExplosionData),
      "@brief Density of the particle cloud created at the start of the explosion.\n\n"
      "@see particleEmitter" );
   addField( "particleRadius", TypeF32, Offset(particleRadius, ExplosionData),
      "@brief Radial distance from the explosion center at which cloud particles "
      "are emitted.\n\n"
      "@see particleEmitter" );
   addField( "emitter", TYPEID< ParticleEmitterData >(), Offset(emitterList, ExplosionData), EC_NUM_EMITTERS,
      "@brief List of additional ParticleEmitterData objects to spawn with this "
      "explosion.\n\n"
      "@see particleEmitter" );

   addField( "debris", TYPEID< DebrisData >(), Offset(debrisList, ExplosionData), EC_NUM_DEBRIS_TYPES,
      "List of DebrisData objects to spawn with this explosion." );
   addField( "debrisThetaMin", TypeF32, Offset(debrisThetaMin, ExplosionData),
      "Minimum angle, from the horizontal plane, to eject debris from." );
   addField( "debrisThetaMax", TypeF32, Offset(debrisThetaMax, ExplosionData),
      "Maximum angle, from the horizontal plane, to eject debris from." );
   addField( "debrisPhiMin", TypeF32, Offset(debrisPhiMin, ExplosionData),
      "Minimum reference angle, from the vertical plane, to eject debris from." );
   addField( "debrisPhiMax", TypeF32, Offset(debrisPhiMax, ExplosionData),
      "Maximum reference angle, from the vertical plane, to eject debris from." );
   addField( "debrisNum", TypeS32, Offset(debrisNum, ExplosionData),
      "Number of debris objects to create." );
   addField( "debrisNumVariance", TypeS32, Offset(debrisNumVariance, ExplosionData),
      "Variance in the number of debris objects to create (must be from 0 - debrisNum)." );
   addField( "debrisVelocity", TypeF32, Offset(debrisVelocity, ExplosionData),
      "Velocity to toss debris at." );
   addField( "debrisVelocityVariance", TypeF32, Offset(debrisVelocityVariance, ExplosionData),
      "Variance in the debris initial velocity (must be >= 0)." );

   addField( "subExplosion", TYPEID< ExplosionData >(), Offset(explosionList, ExplosionData), EC_MAX_SUB_EXPLOSIONS,
      "List of additional ExplosionData objects to create at the start of the "
      "explosion." );

   addField( "delayMS", TypeS32, Offset(delayMS, ExplosionData),
      "Amount of time, in milliseconds, to delay the start of the explosion effect "
      "from the creation of the Explosion object." );
   addField( "delayVariance", TypeS32, Offset(delayVariance, ExplosionData),
      "Variance, in milliseconds, of delayMS." );
   addField( "lifetimeMS", TypeS32, Offset(lifetimeMS, ExplosionData),
      "@brief Lifetime, in milliseconds, of the Explosion object.\n\n"
      "@note If explosionShape is defined and contains an <i>ambient</i> animation, "
      "this field is ignored, and the playSpeed scaled duration of the animation "
      "is used instead." );
   addField( "lifetimeVariance", TypeS32, Offset(lifetimeVariance, ExplosionData),
      "Variance, in milliseconds, of the lifetimeMS of the Explosion object.\n" );
   addField( "offset", TypeF32, Offset(offset, ExplosionData),
      "@brief Offset distance (in a random direction) of the center of the explosion "
      "from the Explosion object position.\n\n"
      "Most often used to create some variance in position for subExplosion effects." );

   addField( "times", TypeF32, Offset(times, ExplosionData), EC_NUM_TIME_KEYS,
      "@brief Time keyframes used to scale the explosionShape model.\n\n"
      "Values should be in increasing order from 0.0 - 1.0, and correspond to "
      "the life of the Explosion where 0 is the beginning and 1 is the end of "
      "the explosion lifetime.\n"
      "@see lifetimeMS" );
   addField( "sizes", TypePoint3F, Offset(sizes, ExplosionData), EC_NUM_TIME_KEYS,
      "@brief \"X Y Z\" size keyframes used to scale the explosionShape model.\n\n"
      "The explosionShape (if defined) will be scaled using the times/sizes "
      "keyframes over the lifetime of the explosion.\n"
      "@see lifetimeMS" );

   addField( "shakeCamera", TypeBool, Offset(shakeCamera, ExplosionData),
      "Controls whether the camera shakes during this explosion." );
   addField( "camShakeFreq", TypePoint3F, Offset(camShakeFreq, ExplosionData),
      "Frequency of camera shaking, defined in the \"X Y Z\" axes." );
   addField( "camShakeAmp", TypePoint3F, Offset(camShakeAmp, ExplosionData),
      "@brief Amplitude of camera shaking, defined in the \"X Y Z\" axes.\n\n"
      "Set any value to 0 to disable shaking in that axis." );
   addField( "camShakeDuration", TypeF32, Offset(camShakeDuration, ExplosionData),
      "Duration (in seconds) to shake the camera." );
   addField( "camShakeRadius", TypeF32, Offset(camShakeRadius, ExplosionData),
      "Radial distance that a camera's position must be within relative to the "
      "center of the explosion to be shaken." );
   addField( "camShakeFalloff", TypeF32, Offset(camShakeFalloff, ExplosionData),
      "Falloff value for the camera shake." );

   addField( "lightStartRadius", TypeF32, Offset(lightStartRadius, ExplosionData),
      "@brief Initial radius of the PointLight created by this explosion.\n\n"
      "Radius is linearly interpolated from lightStartRadius to lightEndRadius "
      "over the lifetime of the explosion.\n"
      "@see lifetimeMS" );
   addField( "lightEndRadius", TypeF32, Offset(lightEndRadius, ExplosionData),
      "@brief Final radius of the PointLight created by this explosion.\n\n"
      "@see lightStartRadius" );
   addField( "lightStartColor", TypeColorF, Offset(lightStartColor, ExplosionData),
      "@brief Initial color of the PointLight created by this explosion.\n\n"
      "Color is linearly interpolated from lightStartColor to lightEndColor "
      "over the lifetime of the explosion.\n"
      "@see lifetimeMS" );
   addField( "lightEndColor", TypeColorF, Offset(lightEndColor, ExplosionData),
      "@brief Final color of the PointLight created by this explosion.\n\n"
      "@see lightStartColor" );
   addField( "lightStartBrightness", TypeF32, Offset(lightStartBrightness, ExplosionData),
      "@brief Initial brightness of the PointLight created by this explosion.\n\n"
      "Brightness is linearly interpolated from lightStartBrightness to "
      "lightEndBrightness over the lifetime of the explosion.\n"
      "@see lifetimeMS" );
   addField("lightEndBrightness", TypeF32, Offset(lightEndBrightness, ExplosionData),
      "@brief Final brightness of the PointLight created by this explosion.\n\n"
      "@see lightStartBrightness" );
   addField( "lightNormalOffset", TypeF32, Offset(lightNormalOffset, ExplosionData),
      "Distance (in the explosion normal direction) of the PointLight position "
      "from the explosion center." );

   Parent::initPersistFields();
}

bool ExplosionData::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   if (explosionScale.x < 0.01f || explosionScale.y < 0.01f || explosionScale.z < 0.01f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s)::onAdd: ExplosionScale components must be >= 0.01", getName());
      explosionScale.x = explosionScale.x < 0.01f ? 0.01f : explosionScale.x;
      explosionScale.y = explosionScale.y < 0.01f ? 0.01f : explosionScale.y;
      explosionScale.z = explosionScale.z < 0.01f ? 0.01f : explosionScale.z;
   }

   if (debrisThetaMin < 0.0f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisThetaMin < 0.0", getName());
      debrisThetaMin = 0.0f;
   }
   if (debrisThetaMax > 180.0f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisThetaMax > 180.0", getName());
      debrisThetaMax = 180.0f;
   }
   if (debrisThetaMin > debrisThetaMax) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisThetaMin > debrisThetaMax", getName());
      debrisThetaMin = debrisThetaMax;
   }
   if (debrisPhiMin < 0.0f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisPhiMin < 0.0", getName());
      debrisPhiMin = 0.0f;
   }
   if (debrisPhiMax > 360.0f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisPhiMax > 360.0", getName());
      debrisPhiMax = 360.0f;
   }
   if (debrisPhiMin > debrisPhiMax) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisPhiMin > debrisPhiMax", getName());
      debrisPhiMin = debrisPhiMax;
   }
   if (debrisNum > 1000) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisNum > 1000", getName());
      debrisNum = 1000;
   }
   if (debrisNumVariance > 1000) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisNumVariance > 1000", getName());
      debrisNumVariance = 1000;
   }
   if (debrisVelocity < 0.1f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisVelocity < 0.1", getName());
      debrisVelocity = 0.1f;
   }
   if (debrisVelocityVariance > 1000) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) debrisVelocityVariance > 1000", getName());
      debrisVelocityVariance = 1000;
   }
   if (playSpeed < 0.05f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) playSpeed < 0.05", getName());
      playSpeed = 0.05f;
   }
   if (lifetimeMS < 1) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) lifetimeMS < 1", getName());
      lifetimeMS = 1;
   }
   if (lifetimeVariance > lifetimeMS) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) lifetimeVariance > lifetimeMS", getName());
      lifetimeVariance = lifetimeMS;
   }
   if (delayMS < 0) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) delayMS < 0", getName());
      delayMS = 0;
   }
   if (delayVariance > delayMS) {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) delayVariance > delayMS", getName());
      delayVariance = delayMS;
   }
   if (offset < 0.0f)
   {
      Con::warnf(ConsoleLogEntry::General, "ExplosionData(%s) offset < 0.0", getName());
      offset = 0.0f;
   }

   S32 i;
   for( i=0; i<EC_NUM_DEBRIS_TYPES; i++ )
   {
      if( !debrisList[i] && debrisIDList[i] != 0 )
      {
         if( !Sim::findObject( debrisIDList[i], debrisList[i] ) )
         {
            Con::errorf( ConsoleLogEntry::General, "ExplosionData::onAdd: Invalid packet, bad datablockId(debris): 0x%x", debrisIDList[i] );
         }
      }
   }

   for( i=0; i<EC_NUM_EMITTERS; i++ )
   {
      if( !emitterList[i] && emitterIDList[i] != 0 )
      {
         if( Sim::findObject( emitterIDList[i], emitterList[i] ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ExplosionData::onAdd: Invalid packet, bad datablockId(particle emitter): 0x%x", emitterIDList[i] );
         }
      }
   }

   for( S32 k=0; k<EC_MAX_SUB_EXPLOSIONS; k++ )
   {
      if( !explosionList[k] && explosionIDList[k] != 0 )
      {
         if( Sim::findObject( explosionIDList[k], explosionList[k] ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ExplosionData::onAdd: Invalid packet, bad datablockId(explosion): 0x%x", explosionIDList[k] );
         }
      }
   }

   return true;
}

void ExplosionData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString(dtsFileName);

   sfxWrite( stream, soundProfile );
   if (stream->writeFlag(particleEmitter))
      stream->writeRangedU32(particleEmitter->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);

   stream->writeInt(particleDensity, 14);
   stream->write(particleRadius);
   stream->writeFlag(faceViewer);
   if(stream->writeFlag(explosionScale.x != 1 || explosionScale.y != 1 || explosionScale.z != 1))
   {
      stream->writeInt((S32)(explosionScale.x * 100), 16);
      stream->writeInt((S32)(explosionScale.y * 100), 16);
      stream->writeInt((S32)(explosionScale.z * 100), 16);
   }
   stream->writeInt((S32)(playSpeed * 20), 14);
   stream->writeRangedU32((U32)debrisThetaMin, 0, 180);
   stream->writeRangedU32((U32)debrisThetaMax, 0, 180);
   stream->writeRangedU32((U32)debrisPhiMin, 0, 360);
   stream->writeRangedU32((U32)debrisPhiMax, 0, 360);
   stream->writeRangedU32((U32)debrisNum, 0, 1000);
   stream->writeRangedU32(debrisNumVariance, 0, 1000);
   stream->writeInt((S32)(debrisVelocity * 10), 14);
   stream->writeRangedU32((U32)(debrisVelocityVariance * 10), 0, 10000);
   stream->writeInt(delayMS >> 5, 16);
   stream->writeInt(delayVariance >> 5, 16);
   stream->writeInt(lifetimeMS >> 5, 16);
   stream->writeInt(lifetimeVariance >> 5, 16);
   stream->write(offset);

   stream->writeFlag( shakeCamera );
   stream->write(camShakeFreq.x);
   stream->write(camShakeFreq.y);
   stream->write(camShakeFreq.z);
   stream->write(camShakeAmp.x);
   stream->write(camShakeAmp.y);
   stream->write(camShakeAmp.z);
   stream->write(camShakeDuration);
   stream->write(camShakeRadius);
   stream->write(camShakeFalloff);

   for( S32 j=0; j<EC_NUM_DEBRIS_TYPES; j++ )
   {
      if( stream->writeFlag( debrisList[j] ) )
      {
         stream->writeRangedU32( debrisList[j]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

   S32 i;
   for( i=0; i<EC_NUM_EMITTERS; i++ )
   {
      if( stream->writeFlag( emitterList[i] != NULL ) )
      {
         stream->writeRangedU32( emitterList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }

   for( i=0; i<EC_MAX_SUB_EXPLOSIONS; i++ )
   {
      if( stream->writeFlag( explosionList[i] != NULL ) )
      {
         stream->writeRangedU32( explosionList[i]->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
      }
   }
   U32 count;
   for(count = 0; count < EC_NUM_TIME_KEYS; count++)
      if(times[count] >= 1)
         break;
   count++;
   if(count > EC_NUM_TIME_KEYS)
      count = EC_NUM_TIME_KEYS;

   stream->writeRangedU32(count, 0, EC_NUM_TIME_KEYS);

   for( i=0; i<count; i++ )
      stream->writeFloat( times[i], 8 );

   for( i=0; i<count; i++ )
   {
      stream->writeRangedU32((U32)(sizes[i].x * 100), 0, 16000);
      stream->writeRangedU32((U32)(sizes[i].y * 100), 0, 16000);
      stream->writeRangedU32((U32)(sizes[i].z * 100), 0, 16000);
   }

   // Dynamic light info
   stream->writeFloat(lightStartRadius/MaxLightRadius, 8);
   stream->writeFloat(lightEndRadius/MaxLightRadius, 8);
   stream->writeFloat(lightStartColor.red,7);
   stream->writeFloat(lightStartColor.green,7);
   stream->writeFloat(lightStartColor.blue,7);
   stream->writeFloat(lightEndColor.red,7);
   stream->writeFloat(lightEndColor.green,7);
   stream->writeFloat(lightEndColor.blue,7);
   stream->writeFloat(lightStartBrightness/MaxLightRadius, 8);
   stream->writeFloat(lightEndBrightness/MaxLightRadius, 8);
   stream->write(lightNormalOffset);
}

void ExplosionData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);

   dtsFileName = stream->readSTString();

   sfxRead( stream, &soundProfile );

   if (stream->readFlag())
      particleEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   else
      particleEmitterId = 0;

   particleDensity = stream->readInt(14);
   stream->read(&particleRadius);
   faceViewer = stream->readFlag();
   if(stream->readFlag())
   {
      explosionScale.x = stream->readInt(16) / 100.0f;
      explosionScale.y = stream->readInt(16) / 100.0f;
      explosionScale.z = stream->readInt(16) / 100.0f;
   }
   else
      explosionScale.set(1,1,1);
   playSpeed = stream->readInt(14) / 20.0f;
   debrisThetaMin = stream->readRangedU32(0, 180);
   debrisThetaMax = stream->readRangedU32(0, 180);
   debrisPhiMin = stream->readRangedU32(0, 360);
   debrisPhiMax = stream->readRangedU32(0, 360);
   debrisNum = stream->readRangedU32(0, 1000);
   debrisNumVariance = stream->readRangedU32(0, 1000);

   debrisVelocity = stream->readInt(14) / 10.0f;
   debrisVelocityVariance = stream->readRangedU32(0, 10000) / 10.0f;
   delayMS = stream->readInt(16) << 5;
   delayVariance = stream->readInt(16) << 5;
   lifetimeMS = stream->readInt(16) << 5;
   lifetimeVariance = stream->readInt(16) << 5;

   stream->read(&offset);

   shakeCamera = stream->readFlag();
   stream->read(&camShakeFreq.x);
   stream->read(&camShakeFreq.y);
   stream->read(&camShakeFreq.z);
   stream->read(&camShakeAmp.x);
   stream->read(&camShakeAmp.y);
   stream->read(&camShakeAmp.z);
   stream->read(&camShakeDuration);
   stream->read(&camShakeRadius);
   stream->read(&camShakeFalloff);


   for( S32 j=0; j<EC_NUM_DEBRIS_TYPES; j++ )
   {
      if( stream->readFlag() )
      {
         debrisIDList[j] = (S32) stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   U32 i;
   for( i=0; i<EC_NUM_EMITTERS; i++ )
   {
      if( stream->readFlag() )
      {
         emitterIDList[i] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   for( S32 k=0; k<EC_MAX_SUB_EXPLOSIONS; k++ )
   {
      if( stream->readFlag() )
      {
         explosionIDList[k] = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
      }
   }

   U32 count = stream->readRangedU32(0, EC_NUM_TIME_KEYS);

   for( i=0; i<count; i++ )
      times[i] = stream->readFloat(8);

   for( i=0; i<count; i++ )
   {
      sizes[i].x = stream->readRangedU32(0, 16000) / 100.0f;
      sizes[i].y = stream->readRangedU32(0, 16000) / 100.0f;
      sizes[i].z = stream->readRangedU32(0, 16000) / 100.0f;
   }

   //
   lightStartRadius = stream->readFloat(8) * MaxLightRadius;
   lightEndRadius = stream->readFloat(8) * MaxLightRadius;
   lightStartColor.red = stream->readFloat(7);
   lightStartColor.green = stream->readFloat(7);
   lightStartColor.blue = stream->readFloat(7);
   lightEndColor.red = stream->readFloat(7);
   lightEndColor.green = stream->readFloat(7);
   lightEndColor.blue = stream->readFloat(7);
   lightStartBrightness = stream->readFloat(8) * MaxLightRadius;
   lightEndBrightness = stream->readFloat(8) * MaxLightRadius;
   stream->read( &lightNormalOffset );
}

bool ExplosionData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;
      
   if( !server )
   {
      String sfxErrorStr;
      if( !sfxResolve( &soundProfile, sfxErrorStr ) )
         Con::errorf(ConsoleLogEntry::General, "Error, unable to load sound profile for explosion datablock: %s", sfxErrorStr.c_str());
      if (!particleEmitter && particleEmitterId != 0)
         if (Sim::findObject(particleEmitterId, particleEmitter) == false)
            Con::errorf(ConsoleLogEntry::General, "Error, unable to load particle emitter for explosion datablock");
   }

   if (dtsFileName && dtsFileName[0]) {
      explosionShape = ResourceManager::get().load(dtsFileName);
      if (!bool(explosionShape)) {
         errorStr = String::ToString("ExplosionData: Couldn't load shape \"%s\"", dtsFileName);
         return false;
      }

      // Resolve animations
      explosionAnimation = explosionShape->findSequence("ambient");

      // Preload textures with a dummy instance...
      TSShapeInstance* pDummy = new TSShapeInstance(explosionShape, !server);
      delete pDummy;

   } else {
      explosionShape     = NULL;
      explosionAnimation = -1;
   }

   return true;
}


//--------------------------------------------------------------------------
//--------------------------------------
//
Explosion::Explosion()
   : mDataBlock( NULL )
{
   mTypeMask |= ExplosionObjectType | LightObjectType;

   mExplosionInstance = NULL;
   mExplosionThread   = NULL;

   dMemset( mEmitterList, 0, sizeof( mEmitterList ) );
   mMainEmitter = NULL;

   mFade = 1;
   mDelayMS = 0;
   mCurrMS = 0;
   mEndingMS = 1000;
   mActive = false;
   mCollideType = 0;

   mInitialNormal.set( 0.0f, 0.0f, 1.0f );
   mRandAngle = sgRandom.randF( 0.0f, 1.0f ) * M_PI_F * 2.0f;
   mLight = LIGHTMGR->createLightInfo();

   mNetFlags.set( IsGhost );
}

Explosion::~Explosion()
{
   if( mExplosionInstance )
   {
      delete mExplosionInstance;
      mExplosionInstance = NULL;
      mExplosionThread   = NULL;
   }
   
   SAFE_DELETE(mLight);
}


void Explosion::setInitialState(const Point3F& point, const Point3F& normal, const F32 fade)
{
   setPosition(point);
   mInitialNormal   = normal;
   mFade            = fade;
}

//--------------------------------------------------------------------------
void Explosion::initPersistFields()
{
   Parent::initPersistFields();

   //
}

//--------------------------------------------------------------------------
bool Explosion::onAdd()
{
   // first check if we have a server connection, if we dont then this is on the server
   //  and we should exit, then check if the parent fails to add the object
   GameConnection *conn = GameConnection::getConnectionToServer();
   if ( !conn || !Parent::onAdd() )
      return false;

   if( !mDataBlock )
   {
      Con::errorf("Explosion::onAdd - Fail - No datablok");
      return false;
   }

   mDelayMS = mDataBlock->delayMS + sgRandom.randI( -mDataBlock->delayVariance, mDataBlock->delayVariance );
   mEndingMS = mDataBlock->lifetimeMS + sgRandom.randI( -mDataBlock->lifetimeVariance, mDataBlock->lifetimeVariance );

   if( mFabs( mDataBlock->offset ) > 0.001f )
   {
      MatrixF axisOrient = MathUtils::createOrientFromDir( mInitialNormal );

      MatrixF trans = getTransform();
      Point3F randVec;
      randVec.x = sgRandom.randF( -1.0f, 1.0f );
      randVec.y = sgRandom.randF( 0.0f, 1.0f );
      randVec.z = sgRandom.randF( -1.0f, 1.0f );
      randVec.normalize();
      randVec *= mDataBlock->offset;
      axisOrient.mulV( randVec );
      trans.setPosition( trans.getPosition() + randVec );
      setTransform( trans );
   }

   // shake camera
   if( mDataBlock->shakeCamera )
   {
      // first check if explosion is near player
      GameConnection* connection = GameConnection::getConnectionToServer();
      ShapeBase *obj = dynamic_cast<ShapeBase*>(connection->getControlObject());

      bool applyShake = true;

      if( obj )
      {
         ShapeBase* cObj = obj;
         while((cObj = cObj->getControlObject()) != 0)
         {
            if(cObj->useObjsEyePoint())
            {
               applyShake = false;
               break;
            }
         }
      }


      if( applyShake && obj )
      {
         VectorF diff = obj->getPosition() - getPosition();
         F32 dist = diff.len();
         if( dist < mDataBlock->camShakeRadius )
         {
            CameraShake *camShake = new CameraShake;
            camShake->setDuration( mDataBlock->camShakeDuration );
            camShake->setFrequency( mDataBlock->camShakeFreq );

            F32 falloff =  dist / mDataBlock->camShakeRadius;
            falloff = 1.0f + falloff * 10.0f;
            falloff = 1.0f / (falloff * falloff);

            VectorF shakeAmp = mDataBlock->camShakeAmp * falloff;
            camShake->setAmplitude( shakeAmp );
            camShake->setFalloff( mDataBlock->camShakeFalloff );
            camShake->init();
            gCamFXMgr.addFX( camShake );
         }
      }
   }


   if( mDelayMS == 0 )
   {
      if( !explode() )
      {
         return false;
      }
   }

   gClientSceneGraph->addObjectToScene(this);

   removeFromProcessList();
   ClientProcessList::get()->addObject(this);

   mRandomVal = sgRandom.randF();

   NetConnection* pNC = NetConnection::getConnectionToServer();
   AssertFatal(pNC != NULL, "Error, must have a connection to the server!");
   pNC->addObject(this);

   // Initialize the light structure and register as a dynamic light
   if (mDataBlock->lightStartRadius != 0.0f || mDataBlock->lightEndRadius)
   {
      mLight->setType( LightInfo::Point );
      mLight->setRange( mDataBlock->lightStartRadius );
      mLight->setColor( mDataBlock->lightStartColor );
   }

   return true;
}

void Explosion::onRemove()
{
   for( S32 i=0; i<ExplosionData::EC_NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->deleteWhenEmpty();
         mEmitterList[i] = NULL;
      }
   }

   if( mMainEmitter )
   {
      mMainEmitter->deleteWhenEmpty();
      mMainEmitter = NULL;
   }

   removeFromScene();

   Parent::onRemove();
}


bool Explosion::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<ExplosionData*>( dptr );
   if (!mDataBlock || !Parent::onNewDataBlock( dptr, reload ))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void Explosion::prepRenderImage( SceneRenderState* state )
{
   prepBatchRender( state );
}

void Explosion::setCurrentScale()
{
   F32 t = F32(mCurrMS) / F32(mEndingMS);

   for( U32 i = 1; i < ExplosionData::EC_NUM_TIME_KEYS; i++ )
   {
      if( mDataBlock->times[i] >= t )
      {
         F32 firstPart =   t - mDataBlock->times[i-1];
         F32 total     =   mDataBlock->times[i] -
                           mDataBlock->times[i-1];

         firstPart /= total;

         mObjScale =      (mDataBlock->sizes[i-1] * (1.0f - firstPart)) +
                          (mDataBlock->sizes[i]   * firstPart);

         return;
      }
   }

}

//--------------------------------------------------------------------------
// Make the explosion face the viewer (if desired)
//--------------------------------------------------------------------------
void Explosion::prepModelView(SceneRenderState* state)
{
   MatrixF rotMatrix( true );
   Point3F targetVector;

   if( mDataBlock->faceViewer )
   {
      targetVector = getPosition() - state->getCameraPosition();
      targetVector.normalize();

      // rotate explosion each time so it's a little different
      rotMatrix.set( EulerF( 0.0f, mRandAngle, 0.0f ) );
   }
   else
   {
      targetVector = mInitialNormal;
   }

   MatrixF explOrient = MathUtils::createOrientFromDir( targetVector );
   explOrient.mul( rotMatrix );
   explOrient.setPosition( getPosition() );

   setCurrentScale();
   explOrient.scale( mObjScale );
   GFX->setWorldMatrix( explOrient );
}

//--------------------------------------------------------------------------
// Render object
//--------------------------------------------------------------------------
void Explosion::prepBatchRender(SceneRenderState* state)
{
   if ( !mExplosionInstance )
      return;

   MatrixF proj = GFX->getProjectionMatrix();
   RectI viewport = GFX->getViewport();

   // Set up our TS render state here.
   TSRenderState rdata;
   rdata.setSceneState( state );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   // render mesh
   GFX->pushWorldMatrix();

   prepModelView( state );

   mExplosionInstance->animate();
   mExplosionInstance->render( rdata );

   GFX->popWorldMatrix();
   GFX->setProjectionMatrix( proj );
   GFX->setViewport( viewport );
}

void Explosion::submitLights( LightManager *lm, bool staticLighting )
{
   if ( staticLighting )
      return;

   // Update the light's info and add it to the scene, the light will
   // only be visible for this current frame.
   mLight->setPosition( getRenderTransform().getPosition() + mInitialNormal * mDataBlock->lightNormalOffset );
   F32 t = F32(mCurrMS) / F32(mEndingMS);
   mLight->setRange( mDataBlock->lightStartRadius +
      (mDataBlock->lightEndRadius - mDataBlock->lightStartRadius) * t );
   mLight->setColor( mDataBlock->lightStartColor +
      (mDataBlock->lightEndColor - mDataBlock->lightStartColor) * t );
   mLight->setBrightness( mDataBlock->lightStartBrightness +
      (mDataBlock->lightEndBrightness - mDataBlock->lightStartBrightness) * t );

   lm->registerGlobalLight( mLight, this );
}


//--------------------------------------------------------------------------
void Explosion::processTick(const Move*)
{
   mCurrMS += TickMs;

   if( mCurrMS >= mEndingMS )
   {
         deleteObject();
         return;
   }
         
   if( (mCurrMS > mDelayMS) && !mActive )
      explode();
}

void Explosion::advanceTime(F32 dt)
{
   if (dt == 0.0f)
      return;

   GameConnection* conn = GameConnection::getConnectionToServer();
   if(!conn)
      return;

   updateEmitters( dt );

   if( mExplosionInstance )
      mExplosionInstance->advanceTime(dt, mExplosionThread);
}

//----------------------------------------------------------------------------
// Update emitters
//----------------------------------------------------------------------------
void Explosion::updateEmitters( F32 dt )
{
   Point3F pos = getPosition();

   for( S32 i=0; i<ExplosionData::EC_NUM_EMITTERS; i++ )
   {
      if( mEmitterList[i] )
      {
         mEmitterList[i]->emitParticles( pos, pos, mInitialNormal, Point3F( 0.0f, 0.0f, 0.0f ), (U32)(dt * 1000));
      }
   }

}

//----------------------------------------------------------------------------
// Launch Debris
//----------------------------------------------------------------------------
void Explosion::launchDebris( Point3F &axis )
{
   GameConnection* conn = GameConnection::getConnectionToServer();
   if(!conn)
      return;

   bool hasDebris = false;
   for( S32 j=0; j<ExplosionData::EC_NUM_DEBRIS_TYPES; j++ )
   {
      if( mDataBlock->debrisList[j] )
      {
         hasDebris = true;
         break;
      }
   }
   if( !hasDebris )
   {
      return;
   }

   Point3F axisx;
   if (mFabs(axis.z) < 0.999f)
      mCross(axis, Point3F(0.0f, 0.0f, 1.0f), &axisx);
   else
      mCross(axis, Point3F(0.0f, 1.0f, 0.0f), &axisx);
   axisx.normalize();

   Point3F pos( 0.0f, 0.0f, 0.5f );
   pos += getPosition();


   U32 numDebris = mDataBlock->debrisNum + sgRandom.randI( -mDataBlock->debrisNumVariance, mDataBlock->debrisNumVariance );

   for( S32 i=0; i<numDebris; i++ )
   {

      Point3F launchDir = MathUtils::randomDir( axis, mDataBlock->debrisThetaMin, mDataBlock->debrisThetaMax,
                                                mDataBlock->debrisPhiMin, mDataBlock->debrisPhiMax );

      F32 debrisVel = mDataBlock->debrisVelocity + mDataBlock->debrisVelocityVariance * sgRandom.randF( -1.0f, 1.0f );

      launchDir *= debrisVel;

      Debris *debris = new Debris;
      debris->setDataBlock( mDataBlock->debrisList[0] );
      debris->setTransform( getTransform() );
      debris->init( pos, launchDir );

      if( !debris->registerObject() )
      {
         Con::warnf( ConsoleLogEntry::General, "Could not register debris for class: %s", mDataBlock->getName() );
         delete debris;
         debris = NULL;
      }
   }
}

//----------------------------------------------------------------------------
// Spawn sub explosions
//----------------------------------------------------------------------------
void Explosion::spawnSubExplosions()
{
   GameConnection* conn = GameConnection::getConnectionToServer();
   if(!conn)
      return;

   for( S32 i=0; i<ExplosionData::EC_MAX_SUB_EXPLOSIONS; i++ )
   {
      if( mDataBlock->explosionList[i] )
      {
         MatrixF trans = getTransform();
         Explosion* pExplosion = new Explosion;
         pExplosion->setDataBlock( mDataBlock->explosionList[i] );
         pExplosion->setTransform( trans );
         pExplosion->setInitialState( trans.getPosition(), mInitialNormal, 1);
         if (!pExplosion->registerObject())
            delete pExplosion;
      }
   }
}

//----------------------------------------------------------------------------
// Explode
//----------------------------------------------------------------------------
bool Explosion::explode()
{
   mActive = true;

   GameConnection* conn = GameConnection::getConnectionToServer();
   if(!conn)
      return false;

   launchDebris( mInitialNormal );
   spawnSubExplosions();

   if (bool(mDataBlock->explosionShape) && mDataBlock->explosionAnimation != -1) {
      mExplosionInstance = new TSShapeInstance(mDataBlock->explosionShape, true);

      mExplosionThread   = mExplosionInstance->addThread();
      mExplosionInstance->setSequence(mExplosionThread, mDataBlock->explosionAnimation, 0);
      mExplosionInstance->setTimeScale(mExplosionThread, mDataBlock->playSpeed);

      mCurrMS   = 0;
      mEndingMS = U32(mExplosionInstance->getScaledDuration(mExplosionThread) * 1000.0f);

      mObjScale.convolve(mDataBlock->explosionScale);
      mObjBox = mDataBlock->explosionShape->bounds;
      resetWorldBox();
   }

   if (mDataBlock->soundProfile)
      SFX->playOnce( mDataBlock->soundProfile, &getTransform() );

   if (mDataBlock->particleEmitter) {
      mMainEmitter = new ParticleEmitter;
      mMainEmitter->setDataBlock(mDataBlock->particleEmitter);
      mMainEmitter->registerObject();

      mMainEmitter->emitParticles(getPosition(), mInitialNormal, mDataBlock->particleRadius,
         Point3F::Zero, U32(mDataBlock->particleDensity * mFade));
   }

   for( S32 i=0; i<ExplosionData::EC_NUM_EMITTERS; i++ )
   {
      if( mDataBlock->emitterList[i] != NULL )
      {
         ParticleEmitter * pEmitter = new ParticleEmitter;
         pEmitter->setDataBlock( mDataBlock->emitterList[i] );
         if( !pEmitter->registerObject() )
         {
            Con::warnf( ConsoleLogEntry::General, "Could not register emitter for particle of class: %s", mDataBlock->getName() );
            SAFE_DELETE(pEmitter);
         }
         mEmitterList[i] = pEmitter;
      }
   }

   return true;
}

