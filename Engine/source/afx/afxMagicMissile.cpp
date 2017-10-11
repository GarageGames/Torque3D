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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//
// afxMagicMissile is a heavily modified variation of the stock Projectile class. In 
// addition to numerous AFX customizations, it also incorporates functionality based on
// the following TGE resources:
//
// Guided or Seeker Projectiles by Derk Adams
//   http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=6778
//
// Projectile Ballistic Coefficients (drag factors) by Mark Owen
//   http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=5128
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "core/resourceManager.h"
#include "ts/tsShapeInstance.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTypes.h"
#include "math/mathUtils.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "T3D/fx/particleEmitter.h"
#include "T3D/fx/splash.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsWorld.h"
#include "gfx/gfxTransformSaver.h"
#include "T3D/containerQuery.h"
#include "T3D/lightDescription.h"
#include "console/engineAPI.h"
#include "lighting/lightManager.h"

#include "afx/util/afxEase.h"
#include "afx/afxMagicMissile.h"
#include "afx/afxMagicSpell.h"
#include "afx/afxChoreographer.h"

class ObjectDeleteEvent : public SimEvent
{
public:
  void process(SimObject *object)
  {
    object->deleteObject();
  }
};

IMPLEMENT_CO_DATABLOCK_V1(afxMagicMissileData);

ConsoleDocClass( afxMagicMissileData,
   "@brief Defines a particular magic-missile type. (Use with afxMagicSpellData.)\n"
   "@tsexample\n"
   "datablock afxMagicMissileData(Fireball_MM)\n"
   "{\n"
   "  muzzleVelocity = 50;\n"
   "  velInheritFactor = 0;\n"
   "  lifetime = 20000;\n"
   "  isBallistic = true;\n"
   "  ballisticCoefficient = 0.85;\n"
   "  gravityMod = 0.05;\n"
   "  isGuided = true;\n"
   "  precision = 30;\n"
   "  trackDelay = 7;\n"
   "  launchOffset = \"0 0 43.7965\";\n"
   "  launchOnServerSignal = true;\n"
   "};\n"
   "@endtsexample\n"
   "@ingroup AFX\n"
);

IMPLEMENT_CO_NETOBJECT_V1(afxMagicMissile);

ConsoleDocClass( afxMagicMissile,
   "@brief Magic-missile class used internally by afxMagicSpell. Properties of individual missile types are defined using afxMagicMissileData.\n"
   "@ingroup AFX\n"
);

/* From stock Projectile code...
IMPLEMENT_CALLBACK( ProjectileData, onExplode, void, ( Projectile* proj, Point3F pos, F32 fade ), 
                   ( proj, pos, fade ),
					"Called when a projectile explodes.\n"
               "@param proj The projectile exploding.\n"
					"@param pos The position of the explosion.\n"
					"@param fade The currently fadeValue of the projectile, affects its visibility.\n"
					"@see Projectile, ProjectileData\n"
				  );

IMPLEMENT_CALLBACK( ProjectileData, onCollision, void, ( Projectile* proj, SceneObject* col, F32 fade, Point3F pos, Point3F normal ),
                   ( proj, col, fade, pos, normal ),
					"Called when a projectile collides with another object.\n"
					"@param proj The projectile colliding.\n"
					"@param col The object hit by the projectile.\n"
					"@param fade The current fadeValue of the projectile, affects its visibility.\n"
					"@param pos The collision position.\n"
               "@param normal The collision normal.\n"
					"@see Projectile, ProjectileData\n"
				  );

const U32 Projectile::csmStaticCollisionMask =  TerrainObjectType    |
                                                InteriorObjectType   |
                                                StaticObjectType;

const U32 Projectile::csmDynamicCollisionMask = PlayerObjectType        |
                                                VehicleObjectType       |
                                                DamagableItemObjectType;

const U32 Projectile::csmDamageableMask = Projectile::csmDynamicCollisionMask;

U32 Projectile::smProjectileWarpTicks = 5;
*/

//--------------------------------------------------------------------------
//
afxMagicMissileData::afxMagicMissileData()
{
   projectileShapeName = ST_NULLSTRING;

   sound = NULL;

   /* From stock Projectile code...
   explosion = NULL;
   explosionId = 0;

   waterExplosion = NULL;
   waterExplosionId = 0;
   */

   /* From stock Projectile code...
   faceViewer = false;
   */
   scale.set( 1.0f, 1.0f, 1.0f );

   isBallistic = false;

   /* From stock Projectile code...
	velInheritFactor = 1.0f;
   */
	muzzleVelocity = 50;
   /* From stock Projectile code...
   impactForce = 0.0f;

	armingDelay = 0;
   fadeDelay = 20000 / 32;
   lifetime = 20000 / 32;

   activateSeq = -1;
   maintainSeq = -1;
   */

   gravityMod = 1.0;
   /* From stock Projectile code...
   bounceElasticity = 0.999f;
   bounceFriction = 0.3f;
   */

   particleEmitter = NULL;
   particleEmitterId = 0;

   particleWaterEmitter = NULL;
   particleWaterEmitterId = 0;

   splash = NULL;
   splashId = 0;

   /* From stock Projectile code...
   decal = NULL;
   decalId = 0;
   */

   lightDesc = NULL;
   lightDescId = 0;
   
  starting_vel_vec.zero();

  isGuided = false;
  precision = 0; 
  trackDelay = 0;
  ballisticCoefficient = 1.0f; 

  followTerrain = false;
  followTerrainHeight = 0.1f;
  followTerrainAdjustRate = 20.0f;
  followTerrainAdjustDelay = 0;

  lifetime = MaxLifetimeTicks;
  collision_mask = arcaneFX::sMissileCollisionMask;

  acceleration = 0;
  accelDelay = 0;
  accelLifetime = 0;

  launch_node = ST_NULLSTRING;
  launch_offset.zero();
  launch_offset_server.zero();
  launch_offset_client.zero();
  launch_node_offset.zero();
  launch_pitch = 0;
  launch_pan = 0;
  launch_cons_s_spec = ST_NULLSTRING;
  launch_cons_c_spec = ST_NULLSTRING;

  echo_launch_offset = false;

  wiggle_axis_string = ST_NULLSTRING;
  wiggle_num_axis = 0;
  wiggle_axis = 0;

  hover_altitude = 0;
  hover_attack_distance = 0;
  hover_attack_gradient = 0;
  hover_time = 0;

  reverse_targeting = false;

  caster_safety_time = U32_MAX;
}

afxMagicMissileData::afxMagicMissileData(const afxMagicMissileData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  projectileShapeName = other.projectileShapeName;
  projectileShape = other.projectileShape; // -- TSShape loads using projectileShapeName
  sound = other.sound;
  splash = other.splash;
  splashId = other.splashId; // -- for pack/unpack of splash ptr
  lightDesc = other.lightDesc;
  lightDescId = other.lightDescId; // -- for pack/unpack of lightDesc ptr
  scale = other.scale;
  isBallistic = other.isBallistic;
  muzzleVelocity = other.muzzleVelocity;
  gravityMod = other.gravityMod;
  particleEmitter = other.particleEmitter;
  particleEmitterId = other.particleEmitterId; // -- for pack/unpack of particleEmitter ptr
  particleWaterEmitter = other.particleWaterEmitter;
  particleWaterEmitterId = other.particleWaterEmitterId; // -- for pack/unpack of particleWaterEmitter ptr

  collision_mask = other.collision_mask;
  starting_vel_vec = other.starting_vel_vec;
  isGuided = other.isGuided;
  precision = other.precision;
  trackDelay = other.trackDelay;
  ballisticCoefficient = other.ballisticCoefficient;
  followTerrain  = other.followTerrain;
  followTerrainHeight = other.followTerrainHeight;
  followTerrainAdjustRate = other.followTerrainAdjustRate;
  followTerrainAdjustDelay = other.followTerrainAdjustDelay;
  lifetime = other.lifetime;
  fadeDelay = other.fadeDelay;
  acceleration = other.acceleration;
  accelDelay = other.accelDelay;
  accelLifetime = other.accelLifetime;
  launch_node  = other.launch_node;
  launch_offset = other.launch_offset;
  launch_offset_server = other.launch_offset_server;
  launch_offset_client = other.launch_offset_client;
  launch_node_offset = other.launch_node_offset;
  launch_pitch = other.launch_pitch;
  launch_pan = other.launch_pan;
  launch_cons_s_spec = other.launch_cons_s_spec;
  launch_cons_c_spec = other.launch_cons_c_spec;
  launch_cons_s_def = other.launch_cons_s_def;
  launch_cons_c_def = other.launch_cons_c_def;
  echo_launch_offset = other.echo_launch_offset;
  wiggle_magnitudes = other.wiggle_magnitudes;
  wiggle_speeds = other.wiggle_speeds;
  wiggle_axis_string = other.wiggle_axis_string;
  wiggle_num_axis = other.wiggle_num_axis;
  wiggle_axis = other.wiggle_axis;
  hover_altitude = other.hover_altitude;
  hover_attack_distance = other.hover_attack_distance;
  hover_attack_gradient = other.hover_attack_gradient;
  hover_time = other.hover_time;
  reverse_targeting = other.reverse_targeting;
  caster_safety_time = other.caster_safety_time;
}

afxMagicMissileData::~afxMagicMissileData()
{
  if (wiggle_axis)
    delete [] wiggle_axis;
}

afxMagicMissileData* afxMagicMissileData::cloneAndPerformSubstitutions(const SimObject* owner, S32 index)
{
  if (!owner || getSubstitutionCount() == 0)
    return this;

  afxMagicMissileData* sub_missile_db = new afxMagicMissileData(*this, true);
  performSubstitutions(sub_missile_db, owner, index);

  return sub_missile_db;
}

//--------------------------------------------------------------------------

#define myOffset(field) Offset(field, afxMagicMissileData)

FRangeValidator muzzleVelocityValidator(0, 10000);
FRangeValidator missilePrecisionValidator(0.f, 100.f);
FRangeValidator missileTrackDelayValidator(0, 100000);
FRangeValidator missileBallisticCoefficientValidator(0, 1);

void afxMagicMissileData::initPersistFields()
{
   static IRangeValidatorScaled ticksFromMS(TickMs, 0, MaxLifetimeTicks);

   addField("particleEmitter", TYPEID<ParticleEmitterData>(), Offset(particleEmitter, afxMagicMissileData));
   addField("particleWaterEmitter", TYPEID<ParticleEmitterData>(), Offset(particleWaterEmitter, afxMagicMissileData));

   addField("projectileShapeName", TypeFilename, Offset(projectileShapeName, afxMagicMissileData));
   addField("scale", TypePoint3F, Offset(scale, afxMagicMissileData));

   addField("sound", TypeSFXTrackName, Offset(sound, afxMagicMissileData));

   /* From stock Projectile code...
   addField("explosion", TYPEID< ExplosionData >(), Offset(explosion, ProjectileData));
   addField("waterExplosion", TYPEID< ExplosionData >(), Offset(waterExplosion, ProjectileData));
   */

   addField("splash", TYPEID<SplashData>(), Offset(splash, afxMagicMissileData));
   /* From stock Projectile code...
   addField("decal", TYPEID< DecalData >(), Offset(decal, ProjectileData));
   */

   addField("lightDesc", TYPEID< LightDescription >(), Offset(lightDesc, afxMagicMissileData));

   addField("isBallistic", TypeBool,   Offset(isBallistic, afxMagicMissileData));
   /* From stock Projectile code...
   addField("velInheritFactor", TypeF32, Offset(velInheritFactor, ProjectileData));
   */
   addNamedFieldV(muzzleVelocity,    TypeF32,      afxMagicMissileData,  &muzzleVelocityValidator);
   /* From stock Projectile code...
   addField("impactForce", TypeF32, Offset(impactForce, ProjectileData));
   */
   addNamedFieldV(lifetime,    TypeS32,              afxMagicMissileData,  &ticksFromMS);
   /* From stock Projectile code...
   addProtectedField("armingDelay", TypeS32, Offset(armingDelay, ProjectileData), &setArmingDelay, &getScaledValue, 
      "The time in milliseconds before the projectile is armed and will cause damage or explode on impact." );

   addProtectedField("fadeDelay", TypeS32, Offset(fadeDelay, ProjectileData), &setFadeDelay, &getScaledValue,
      "The time in milliseconds when the projectile begins to fade out.  Must be less than the lifetime to have an effect." );

   addField("bounceElasticity", TypeF32, Offset(bounceElasticity, ProjectileData));
   addField("bounceFriction", TypeF32, Offset(bounceFriction, ProjectileData));
   */
   addField("gravityMod", TypeF32, Offset(gravityMod, afxMagicMissileData));

   // FIELDS ADDED BY MAGIC-MISSILE

   addField("missileShapeName",    TypeFilename, myOffset(projectileShapeName));
   addField("missileShapeScale",   TypePoint3F,  myOffset(scale));

   addField("startingVelocityVector",TypePoint3F,  myOffset(starting_vel_vec));

   addNamedField(isGuided,               TypeBool,   afxMagicMissileData);
   addNamedFieldV(precision,             TypeF32,    afxMagicMissileData,  &missilePrecisionValidator); 
   addNamedFieldV(trackDelay,            TypeS32,    afxMagicMissileData,  &missileTrackDelayValidator); 
   addNamedFieldV(ballisticCoefficient,  TypeF32,    afxMagicMissileData,  &missileBallisticCoefficientValidator);

   addField("collisionMask",         TypeS32,      myOffset(collision_mask));

   addField("followTerrain",             TypeBool, myOffset(followTerrain));
   addField("followTerrainHeight",       TypeF32,  myOffset(followTerrainHeight));
   addField("followTerrainAdjustRate",   TypeF32,  myOffset(followTerrainAdjustRate));
   addFieldV("followTerrainAdjustDelay", TypeS32,  myOffset(followTerrainAdjustDelay), &ticksFromMS); 

   addNamedField(acceleration,     TypeF32,  afxMagicMissileData);
   addNamedFieldV(accelDelay,      TypeS32,  afxMagicMissileData,  &ticksFromMS);
   addNamedFieldV(accelLifetime,   TypeS32,  afxMagicMissileData,  &ticksFromMS);

   addField("launchNode",        TypeString,   myOffset(launch_node));
   addField("launchOffset",      TypePoint3F,  myOffset(launch_offset));
   addField("launchOffsetServer",TypePoint3F,  myOffset(launch_offset_server));
   addField("launchOffsetClient",TypePoint3F,  myOffset(launch_offset_client));
   addField("launchNodeOffset",  TypePoint3F,  myOffset(launch_node_offset));
   addField("launchAimPitch",    TypeF32,      myOffset(launch_pitch));
   addField("launchAimPan",      TypeF32,      myOffset(launch_pan));
   addField("launchConstraintServer",  TypeString,   myOffset(launch_cons_s_spec));
   addField("launchConstraintClient",  TypeString,   myOffset(launch_cons_c_spec));
   //
   addField("echoLaunchOffset",  TypeBool,     myOffset(echo_launch_offset));

   addField("wiggleMagnitudes", TypeF32Vector, myOffset(wiggle_magnitudes));
   addField("wiggleSpeeds",     TypeF32Vector, myOffset(wiggle_speeds));
   addField("wiggleAxis",       TypeString,    myOffset(wiggle_axis_string));

   addField("hoverAltitude",       TypeF32,    myOffset(hover_altitude));
   addField("hoverAttackDistance", TypeF32,    myOffset(hover_attack_distance));
   addField("hoverAttackGradient", TypeF32,    myOffset(hover_attack_gradient));
   addFieldV("hoverTime",          TypeS32,    myOffset(hover_time), &ticksFromMS); 

   addField("reverseTargeting",  TypeBool,     myOffset(reverse_targeting));

   addFieldV("casterSafetyTime", TypeS32,      myOffset(caster_safety_time), &ticksFromMS); 

   Parent::initPersistFields();

   // disallow some field substitutions
   onlyKeepClearSubstitutions("particleEmitter"); // subs resolving to "~~", or "~0" are OK
   onlyKeepClearSubstitutions("particleWaterEmitter");
   onlyKeepClearSubstitutions("sound");
   onlyKeepClearSubstitutions("splash");
}


//--------------------------------------------------------------------------
bool afxMagicMissileData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   // ADDED BY MAGIC-MISSILE

   // Wiggle axes ////////////////////////////////////////////////////////////  
   if (wiggle_axis_string != ST_NULLSTRING && wiggle_num_axis == 0)   
   {
      // Tokenize input string and convert to Point3F array
      //
      Vector<char*> dataBlocks(__FILE__, __LINE__);

      // make a copy of points_string
      char* tokCopy = new char[dStrlen(wiggle_axis_string) + 1];
      dStrcpy(tokCopy, wiggle_axis_string);

      // extract tokens one by one, adding them to dataBlocks
      char* currTok = dStrtok(tokCopy, " \t");
      while (currTok != NULL) 
      {
         dataBlocks.push_back(currTok);
         currTok = dStrtok(NULL, " \t");
      }

      // bail if there were no tokens in the string
      if (dataBlocks.size() == 0) 
      {
         Con::warnf(ConsoleLogEntry::General, "afxMagicMissileData(%s) invalid wiggle axis string. No tokens found", getName());
         delete [] tokCopy;
         return false;
      }

      // Find wiggle_num_axis (round up to multiple of 3) // WARNING here if not multiple of 3?
      for (U32 i = 0; i < dataBlocks.size()%3; i++) 
      {
         dataBlocks.push_back("0.0");
      }

      wiggle_num_axis = dataBlocks.size()/3;
      wiggle_axis = new Point3F[wiggle_num_axis];

      U32 p_i = 0;
      for (U32 i = 0; i < dataBlocks.size(); i+=3, p_i++)
      {
         F32 x,y,z;
         x = dAtof(dataBlocks[i]);  // What about overflow?
         y = dAtof(dataBlocks[i+1]);
         z = dAtof(dataBlocks[i+2]);
         wiggle_axis[p_i].set(x,y,z);

         wiggle_axis[p_i].normalizeSafe(); // sufficient????
      }

      delete [] tokCopy; 
   }

   launch_cons_s_def.parseSpec(launch_cons_s_spec, true, false);
   launch_cons_c_def.parseSpec(launch_cons_c_spec, false, true);

   return true;
}


bool afxMagicMissileData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;
      
   if( !server )
   {
      if (!particleEmitter && particleEmitterId != 0)
         if (Sim::findObject(particleEmitterId, particleEmitter) == false)
            Con::errorf(ConsoleLogEntry::General, "afxMagicMissileData::preload: Invalid packet, bad datablockId(particleEmitter): %d", particleEmitterId);

      if (!particleWaterEmitter && particleWaterEmitterId != 0)
         if (Sim::findObject(particleWaterEmitterId, particleWaterEmitter) == false)
            Con::errorf(ConsoleLogEntry::General, "afxMagicMissileData::preload: Invalid packet, bad datablockId(particleWaterEmitter): %d", particleWaterEmitterId);

      /* From stock Projectile code...
      if (!explosion && explosionId != 0)
         if (Sim::findObject(explosionId, explosion) == false)
            Con::errorf(ConsoleLogEntry::General, "ProjectileData::preload: Invalid packet, bad datablockId(explosion): %d", explosionId);

      if (!waterExplosion && waterExplosionId != 0)
         if (Sim::findObject(waterExplosionId, waterExplosion) == false)
            Con::errorf(ConsoleLogEntry::General, "ProjectileData::preload: Invalid packet, bad datablockId(waterExplosion): %d", waterExplosionId);
      */

      if (!splash && splashId != 0)
         if (Sim::findObject(splashId, splash) == false)
            Con::errorf(ConsoleLogEntry::General, "afxMagicMissileData::preload: Invalid packet, bad datablockId(splash): %d", splashId);

      /* From stock Projectile code...
      if (!decal && decalId != 0)
         if (Sim::findObject(decalId, decal) == false)
            Con::errorf(ConsoleLogEntry::General, "ProjectileData::preload: Invalid packet, bad datablockId(decal): %d", decalId);
      */

      String errorStr;
      if( !sfxResolve( &sound, errorStr ) )
         Con::errorf(ConsoleLogEntry::General, "afxMagicMissileData::preload: Invalid packet: %s", errorStr.c_str());

      if (!lightDesc && lightDescId != 0)
         if (Sim::findObject(lightDescId, lightDesc) == false)
            Con::errorf(ConsoleLogEntry::General, "afxMagicMissileData::preload: Invalid packet, bad datablockid(lightDesc): %d", lightDescId);   
   }

   if (projectileShapeName != ST_NULLSTRING) 
   {
      projectileShape = ResourceManager::get().load(projectileShapeName);
      if (bool(projectileShape) == false)
      {
         errorStr = String::ToString("afxMagicMissileData::load: Couldn't load shape \"%s\"", projectileShapeName);
         return false;
      }
      /* From stock Projectile code...
      activateSeq = projectileShape->findSequence("activate");
      maintainSeq = projectileShape->findSequence("maintain");
      */
   }

   if (bool(projectileShape)) // create an instance to preload shape data
   {
      TSShapeInstance* pDummy = new TSShapeInstance(projectileShape, !server);
      delete pDummy;
   }

   return true;
}

//--------------------------------------------------------------------------
// Modified from floorPlanRes.cc
// Read a vector of items
template <class T>
bool readVector(Vector<T> & vec, Stream & stream, const char * msg)
{
   U32   num, i;
   bool  Ok = true;
   stream.read( & num );
   vec.setSize( num );
   for( i = 0; i < num && Ok; i++ ){
      Ok = stream.read(& vec[i]);
      AssertISV( Ok, avar("math vec read error (%s) on elem %d", msg, i) );
   }
   return Ok;
}
// Write a vector of items
template <class T>
bool writeVector(const Vector<T> & vec, Stream & stream, const char * msg)
{
   bool  Ok = true;
   stream.write( vec.size() );
   for( U32 i = 0; i < vec.size() && Ok; i++ ) {
      Ok = stream.write(vec[i]);
      AssertISV( Ok, avar("vec write error (%s) on elem %d", msg, i) );
   }
   return Ok;
}
//--------------------------------------------------------------------------

void afxMagicMissileData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString(projectileShapeName);
   /* From stock Projectile code...
   stream->writeFlag(faceViewer);
   */
   if(stream->writeFlag(scale.x != 1 || scale.y != 1 || scale.z != 1))
   {
      stream->write(scale.x);
      stream->write(scale.y);
      stream->write(scale.z);
   }

   stream->write(collision_mask);

   if (stream->writeFlag(particleEmitter != NULL))
      stream->writeRangedU32(particleEmitter->getId(), DataBlockObjectIdFirst,
                                                   DataBlockObjectIdLast);

   if (stream->writeFlag(particleWaterEmitter != NULL))
      stream->writeRangedU32(particleWaterEmitter->getId(), DataBlockObjectIdFirst,
                                                   DataBlockObjectIdLast);

   /* From stock Projectile code...
   if (stream->writeFlag(explosion != NULL))
      stream->writeRangedU32(explosion->getId(), DataBlockObjectIdFirst,
                                                 DataBlockObjectIdLast);

   if (stream->writeFlag(waterExplosion != NULL))
      stream->writeRangedU32(waterExplosion->getId(), DataBlockObjectIdFirst,
                                                      DataBlockObjectIdLast);
   */

   if (stream->writeFlag(splash != NULL))
      stream->writeRangedU32(splash->getId(), DataBlockObjectIdFirst,
                                              DataBlockObjectIdLast);

   /* From stock Projectile code...
   if (stream->writeFlag(decal != NULL))
      stream->writeRangedU32(decal->getId(), DataBlockObjectIdFirst,
                                              DataBlockObjectIdLast);
   */

   sfxWrite( stream, sound );

   if ( stream->writeFlag(lightDesc != NULL))
      stream->writeRangedU32(lightDesc->getId(), DataBlockObjectIdFirst,
                                                 DataBlockObjectIdLast);

   /* From stock Projectile code...
   stream->write(impactForce);
   */

   stream->write(lifetime);
   /* From stock Projectile code...
   stream->write(armingDelay);
   stream->write(fadeDelay);
   */

   if(stream->writeFlag(isBallistic))
   {
      stream->write(gravityMod);
      /* From stock Projectile code...
      stream->write(bounceElasticity);
      stream->write(bounceFriction);
      */
      stream->write(ballisticCoefficient);
   }

   if(stream->writeFlag(isGuided))
   {
      stream->write(precision);
      stream->write(trackDelay);
   }

   stream->write(muzzleVelocity);
   mathWrite(*stream, starting_vel_vec);
   stream->write(acceleration);
   stream->write(accelDelay);
   stream->write(accelLifetime);

   stream->writeString(launch_node);
   mathWrite(*stream, launch_offset);
   mathWrite(*stream, launch_offset_server);
   mathWrite(*stream, launch_offset_client);
   mathWrite(*stream, launch_node_offset);
   stream->write(launch_pitch);
   stream->write(launch_pan);
   stream->writeString(launch_cons_c_spec);
   stream->writeFlag(echo_launch_offset);

   writeVector(wiggle_magnitudes, *stream, "afxMagicMissile: wiggle_magnitudes");
   writeVector(wiggle_speeds, *stream, "afxMagicMissile: wiggle_speeds");

   stream->write(wiggle_num_axis);
   for (U32 i = 0; i < wiggle_num_axis; i++)
      mathWrite(*stream, wiggle_axis[i]);

   stream->write(hover_altitude);
   stream->write(hover_attack_distance);
   stream->write(hover_attack_gradient);
   stream->writeRangedU32(hover_time, 0, MaxLifetimeTicks);

   stream->writeFlag(reverse_targeting);

   stream->write(caster_safety_time);
}

void afxMagicMissileData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   projectileShapeName = stream->readSTString();
   /* From stock Projectile code...
   faceViewer = stream->readFlag();
   */
   if(stream->readFlag())
   {
      stream->read(&scale.x);
      stream->read(&scale.y);
      stream->read(&scale.z);
   }
   else
      scale.set(1,1,1);

   stream->read(&collision_mask);

   if (stream->readFlag())
      particleEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   if (stream->readFlag())
      particleWaterEmitterId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   /* From stock Projectile code...
   if (stream->readFlag())
      explosionId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   if (stream->readFlag())
      waterExplosionId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   */
   
   if (stream->readFlag())
      splashId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   /* From stock Projectile code...
   if (stream->readFlag())
      decalId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   */
   
   sfxRead( stream, &sound );

   if (stream->readFlag())
      lightDescId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   /* From stock Projectile code...
   stream->read(&impactForce);
   */

   stream->read(&lifetime);
   /* From stock Projectile code...
   stream->read(&armingDelay);
   stream->read(&fadeDelay);
   */

   isBallistic = stream->readFlag();
   if(isBallistic)
   {
      stream->read(&gravityMod);
      /* From stock Projectile code...
      stream->read(&bounceElasticity);
      stream->read(&bounceFriction);
      */
      stream->read(&ballisticCoefficient);
   }

   isGuided = stream->readFlag();
   if(isGuided)
   {
      stream->read(&precision);
      stream->read(&trackDelay);
   }

   stream->read(&muzzleVelocity);
   mathRead(*stream, &starting_vel_vec);
   stream->read(&acceleration);
   stream->read(&accelDelay);
   stream->read(&accelLifetime);

   launch_node = stream->readSTString();
   mathRead(*stream, &launch_offset);
   mathRead(*stream, &launch_offset_server);
   mathRead(*stream, &launch_offset_client);
   mathRead(*stream, &launch_node_offset);
   stream->read(&launch_pitch);
   stream->read(&launch_pan);
   launch_cons_c_spec = stream->readSTString();
   echo_launch_offset = stream->readFlag();

   readVector(wiggle_magnitudes, *stream, "afxMagicMissile: wiggle_magnitudes");
   readVector(wiggle_speeds, *stream, "afxMagicMissile: wiggle_speeds");

   if (wiggle_axis)
      delete [] wiggle_axis;
   wiggle_axis = 0;
   wiggle_num_axis = 0;

   stream->read(&wiggle_num_axis);
   if (wiggle_num_axis > 0)
   {
      wiggle_axis = new Point3F[wiggle_num_axis];
      for (U32 i = 0; i < wiggle_num_axis; i++)
         mathRead(*stream, &wiggle_axis[i]);
   }

   stream->read(&hover_altitude);
   stream->read(&hover_attack_distance);
   stream->read(&hover_attack_gradient);
   hover_time = stream->readRangedU32(0, MaxLifetimeTicks);

   reverse_targeting = stream->readFlag();

   stream->read(&caster_safety_time);
}

/* From stock Projectile code...
bool ProjectileData::setLifetime( void *obj, const char *index, const char *data )
{
	S32 value = dAtoi(data);
   value = scaleValue(value);
   
   ProjectileData *object = static_cast<ProjectileData*>(obj);
   object->lifetime = value;

   return false;
}

bool ProjectileData::setArmingDelay( void *obj, const char *index, const char *data )
{
	S32 value = dAtoi(data);
   value = scaleValue(value);

   ProjectileData *object = static_cast<ProjectileData*>(obj);
   object->armingDelay = value;

   return false;
}

bool ProjectileData::setFadeDelay( void *obj, const char *index, const char *data )
{
	S32 value = dAtoi(data);
   value = scaleValue(value);

   ProjectileData *object = static_cast<ProjectileData*>(obj);
   object->fadeDelay = value;

   return false;
}

const char *ProjectileData::getScaledValue( void *obj, const char *data)
{

	S32 value = dAtoi(data);
   value = scaleValue(value, false);

   String stringData = String::ToString(value);
   char *strBuffer = Con::getReturnBuffer(stringData.size());
   dMemcpy( strBuffer, stringData, stringData.size() );

   return strBuffer;
}

S32 ProjectileData::scaleValue( S32 value, bool down )
{
   S32 minV = 0;
   S32 maxV = Projectile::MaxLivingTicks;
   
   // scale down to ticks before we validate
   if( down )
      value /= TickMs;
   
   if(value < minV || value > maxV)
	{
      Con::errorf("ProjectileData::scaleValue(S32 value = %d, bool down = %b) - Scaled value must be between %d and %d", value, down, minV, maxV);
		if(value < minV)
			value = minV;
		else if(value > maxV)
			value = maxV;
	}

   // scale up from ticks after we validate
   if( !down )
      value *= TickMs;

   return value;
}
*/

void afxMagicMissileData::gather_cons_defs(Vector<afxConstraintDef>& defs)
{ 
  if (launch_cons_s_def.isDefined())
    defs.push_back(launch_cons_s_def);
  if (launch_cons_c_def.isDefined())
    defs.push_back(launch_cons_c_def);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicMissile

afxMagicMissile::afxMagicMissile()
{
  init(true, true);
}

afxMagicMissile::afxMagicMissile(bool on_server, bool on_client)
{
  init(on_server, on_client);
}

//--------------------------------------------------------------------------
//--------------------------------------
//
void afxMagicMissile::init(bool on_server, bool on_client)
{
  mPhysicsWorld = NULL;

  mTypeMask |= ProjectileObjectType | LightObjectType;

  mLight = LightManager::createLightInfo();
  mLight->setType( LightInfo::Point );   

  mLightState.clear();
  mLightState.setLightInfo( mLight );

  mCurrPosition.zero();
  mCurrVelocity.set(0, 0, 1);

  mCurrTick = 0;

  mParticleEmitter   = NULL;
  mParticleWaterEmitter = NULL;
  mSound = 0;

  mProjectileShape   = NULL;

  mDataBlock = NULL;

  choreographer = NULL;

  if (on_server != on_client)
  {
    client_only = on_client;
    server_only = on_server;
    mNetFlags.clear(Ghostable | ScopeAlways);
    if (client_only)
      mNetFlags.set(IsGhost);
  }
  else
  {
    // note -- setting neither server or client makes no sense so we
    // treat as if both are set.
    mNetFlags.set(Ghostable | ScopeAlways);
    client_only = server_only = false;
  }

  mCurrDeltaBase.zero();
  mCurrBackDelta.zero();
  collision_mask = 0;
  prec_inc = 0.0f;

  did_launch = false;
  did_impact = false;

  missile_target = NULL;
  collide_exempt = NULL;

  use_accel = false;

  hover_attack_go = false;
  hover_attack_tick = 0;

  starting_velocity = 0.0;
  starting_vel_vec.zero();

  ss_object = 0;
  ss_index = 0;
}

afxMagicMissile::~afxMagicMissile()
{
   SAFE_DELETE(mLight);

   delete mProjectileShape;
   mProjectileShape = NULL;
}

//--------------------------------------------------------------------------
void afxMagicMissile::initPersistFields()
{
   addGroup("Physics");
   addField("initialPosition", TypePoint3F, Offset(mCurrPosition, afxMagicMissile) ,
     "Initial starting position for this missile.");
   addField("initialVelocity", TypePoint3F, Offset(mCurrVelocity, afxMagicMissile) ,
     "Initial starting velocity for this missile.");
   endGroup("Physics");

   /* From stock Projectile code...
   addGroup("Source");
   addField("sourceObject",     TypeS32,     Offset(mSourceObjectId, Projectile) ,"The object that fires this projectile. If this projectile was fired by a WeaponImage, it will be the object that owns the WeaponImage, usually the player.");
   addField("sourceSlot",       TypeS32,     Offset(mSourceObjectSlot, Projectile) ,"Which weapon slot on the sourceObject that this projectile originates from.");
   endGroup("Source");
  */

   Parent::initPersistFields();
}

/* From stock Projectile code...
bool Projectile::calculateImpact(float,
                                 Point3F& pointOfImpact,
                                 float&   impactTime)
{
   Con::warnf(ConsoleLogEntry::General, "Projectile::calculateImpact: Should never be called");

   impactTime = 0;
   pointOfImpact.set(0, 0, 0);
   return false;
}


//--------------------------------------------------------------------------
F32 Projectile::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   F32 ret = Parent::getUpdatePriority(camInfo, updateMask, updateSkips);
   // if the camera "owns" this object, it should have a slightly higher priority
   if(mSourceObject == camInfo->camera)
      return ret + 0.2;
   return ret;
}
*/

bool afxMagicMissile::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (isServerObject())
   {
      /* From stock Projectile code...
      ShapeBase* ptr;
      if (Sim::findObject(mSourceObjectId, ptr))
      {
         mSourceObject = ptr;

         // Since we later do processAfter( mSourceObject ) we must clearProcessAfter
         // if it is deleted. SceneObject already handles this in onDeleteNotify so
         // all we need to do is register for the notification.
         deleteNotify( ptr );
      }
      else
      {
         if (mSourceObjectId != -1)
            Con::errorf(ConsoleLogEntry::General, "Projectile::onAdd: mSourceObjectId is invalid");
         mSourceObject = NULL;
      }

      // If we're on the server, we need to inherit some of our parent's velocity
      //
      mCurrTick = 0;
     */
   }
   else
   {
      if (bool(mDataBlock->projectileShape))
      {
         mProjectileShape = new TSShapeInstance(mDataBlock->projectileShape, true);
         /* From stock Projectile code...
         if (mDataBlock->activateSeq != -1)
         {
            mActivateThread = mProjectileShape->addThread();
            mProjectileShape->setTimeScale(mActivateThread, 1);
            mProjectileShape->setSequence(mActivateThread, mDataBlock->activateSeq, 0);
         }
         */
      }
      if (mDataBlock->particleEmitter != NULL)
      {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         //pEmitter->setDataBlock(mDataBlock->particleEmitter->cloneAndPerformSubstitutions(ss_object, ss_index));
         pEmitter->onNewDataBlock(mDataBlock->particleEmitter->cloneAndPerformSubstitutions(ss_object, ss_index), false);
         if (pEmitter->registerObject() == false)
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register particle emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mParticleEmitter = pEmitter;
      }

      if (mDataBlock->particleWaterEmitter != NULL)
      {
         ParticleEmitter* pEmitter = new ParticleEmitter;
         pEmitter->onNewDataBlock(mDataBlock->particleWaterEmitter->cloneAndPerformSubstitutions(ss_object, ss_index), false);
         if (pEmitter->registerObject() == false)
         {
            Con::warnf(ConsoleLogEntry::General, "Could not register particle emitter for particle of class: %s", mDataBlock->getName());
            delete pEmitter;
            pEmitter = NULL;
         }
         mParticleWaterEmitter = pEmitter;
      }
   }
   /* From stock Projectile code...
   if (mSourceObject.isValid())
      processAfter(mSourceObject);
  */

   // detect for acceleration
   use_accel = (mDataBlock->acceleration != 0 && mDataBlock->accelLifetime > 0);

   // Setup our bounding box
   if (bool(mDataBlock->projectileShape) == true)
      mObjBox = mDataBlock->projectileShape->bounds;
   else
      mObjBox = Box3F(Point3F(0, 0, 0), Point3F(0, 0, 0));
   resetWorldBox();
   addToScene();

   if ( PHYSICSMGR )
      mPhysicsWorld = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );

   return true;
}


void afxMagicMissile::onRemove()
{
   if(mParticleEmitter)
   {
      mParticleEmitter->deleteWhenEmpty();
      mParticleEmitter = NULL;
   }

   if(mParticleWaterEmitter)
   {
      mParticleWaterEmitter->deleteWhenEmpty();
      mParticleWaterEmitter = NULL;
   }

   SFX_DELETE( mSound );

   removeFromScene();
   Parent::onRemove();
}


bool afxMagicMissile::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<afxMagicMissileData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   starting_velocity = mDataBlock->muzzleVelocity;
   starting_vel_vec = mDataBlock->starting_vel_vec;
   collision_mask = mDataBlock->collision_mask;

   if ( isGhost() )
   {
      // Create the sound ahead of time.  This reduces runtime
      // costs and makes the system easier to understand.

      SFX_DELETE( mSound );

      if ( mDataBlock->sound )
         mSound = SFX->createSource( mDataBlock->sound );
   }

   return true;
}


//--------------------------------------------------------------------------

void afxMagicMissile::submitLights( LightManager *lm, bool staticLighting )
{
   if ( staticLighting || isHidden() || !mDataBlock->lightDesc )
      return;
   
   mDataBlock->lightDesc->submitLight( &mLightState, getRenderTransform(), lm, this );   
}

bool afxMagicMissile::pointInWater(const Point3F &point)
{
   // This is pretty much a hack so we can use the existing ContainerQueryInfo
   // and findObject router.
   
   // We only care if we intersect with water at all 
   // so build a box at the point that has only 1 z extent.
   // And test if water coverage is anything other than zero.

   Box3F boundsBox( point, point );
   boundsBox.maxExtents.z += 1.0f;

   ContainerQueryInfo info;
   info.box = boundsBox;
   info.mass = 0.0f;
   
   // Find and retreive physics info from intersecting WaterObject(s)
   if(mContainer != NULL)
   {
      mContainer->findObjects( boundsBox, WaterObjectType, findRouter, &info );
   }
   else
   {
      // Handle special case where the projectile has exploded prior to having
      // called onAdd() on the client.  This occurs when the projectile on the
      // server is created and then explodes in the same network update tick.
      // On the client end in NetConnection::ghostReadPacket() the ghost is
      // created and then Projectile::unpackUpdate() is called prior to the
      // projectile being registered.  Within unpackUpdate() the explosion
      // is triggered, but without being registered onAdd() isn't called and
      // the container is not set.  As all we're doing is checking if the
      // given explosion point is within water, we should be able to use the
      // global container here.  We could likely always get away with this,
      // but using the actual defined container when possible is the right
      // thing to do.  DAW
      AssertFatal(isClientObject(), "Server projectile has not been properly added");
      gClientContainer.findObjects( boundsBox, WaterObjectType, findRouter, &info );
   }

   return ( info.waterCoverage > 0.0f );
}

//----------------------------------------------------------------------------

void afxMagicMissile::emitParticles(const Point3F& from, const Point3F& to, const Point3F& vel, const U32 ms)
{
  /* From stock Projectile code...
   if ( isHidden() )
      return;
  */

   Point3F axis = -vel;

   if( axis.isZero() )
      axis.set( 0.0, 0.0, 1.0 );
   else
      axis.normalize();

   bool fromWater = pointInWater(from);
   bool toWater   = pointInWater(to);

   if (!fromWater && !toWater && bool(mParticleEmitter))                                        // not in water
      mParticleEmitter->emitParticles(from, to, axis, vel, ms);
   else if (fromWater && toWater && bool(mParticleWaterEmitter))                                // in water
      mParticleWaterEmitter->emitParticles(from, to, axis, vel, ms);
   else if (!fromWater && toWater)     // entering water
   {
      // cast the ray to get the surface point of the water
      RayInfo rInfo;
      if (gClientContainer.castRay(from, to, WaterObjectType, &rInfo))
      {
         create_splash(rInfo.point);

         // create an emitter for the particles out of water and the particles in water
         if (mParticleEmitter)
            mParticleEmitter->emitParticles(from, rInfo.point, axis, vel, ms);

         if (mParticleWaterEmitter)
            mParticleWaterEmitter->emitParticles(rInfo.point, to, axis, vel, ms);
      }
   }
   else if (fromWater && !toWater)     // leaving water
   {
      // cast the ray in the opposite direction since that point is out of the water, otherwise
      //  we hit water immediately and wont get the appropriate surface point
      RayInfo rInfo;
      if (gClientContainer.castRay(to, from, WaterObjectType, &rInfo))
      {
      create_splash(rInfo.point);

         // create an emitter for the particles out of water and the particles in water
         if (mParticleEmitter)
            mParticleEmitter->emitParticles(rInfo.point, to, axis, vel, ms);

         if (mParticleWaterEmitter)
            mParticleWaterEmitter->emitParticles(from, rInfo.point, axis, vel, ms);
      }
   }
}

void afxMagicMissile::processTick(const Move* move)
{
   Parent::processTick( move );
   
   // only active from launch to impact
   if (!is_active())
      return;

   mCurrTick++;

   // missile fizzles out by exceeding lifetime
   if ((isServerObject() || client_only) && mCurrTick >= mDataBlock->lifetime)
   {
      did_impact = true;
      setMaskBits(ImpactMask);
      if (choreographer)
      {
         Point3F n = mCurrVelocity; n.normalizeSafe();
         choreographer->impactNotify(mCurrPosition, n, 0);
      }
      Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + 500);
      return;
   }

   static F32 dT = F32(TickMs)*0.001f;

   Point3F old_pos = mCurrPosition;

   // adjust missile velocity from gravity and drag influences
   if (mDataBlock->isBallistic)
   {
      F32 dV = (1 - mDataBlock->ballisticCoefficient)*dT;
      Point3F d(mCurrVelocity.x*dV, mCurrVelocity.y*dV, 9.81f*mDataBlock->gravityMod*dT);
      mCurrVelocity -= d;
   }

   // adjust missile velocity from acceleration
   if (use_accel)
   {
      if (mCurrTick > mDataBlock->accelDelay && 
         mCurrTick <= mDataBlock->accelDelay + mDataBlock->accelLifetime)
      {
         Point3F d = mCurrVelocity; d.normalizeSafe();
         mCurrVelocity += d*mDataBlock->acceleration*dT;
      }
   }

   // adjust mCurrVelocity from guidance system influences
   if (mDataBlock->isGuided && missile_target && mCurrTick > mDataBlock->trackDelay) 
   {
      // get the position tracked by the guidance system
      Point3F target_pos = missile_target->getBoxCenter(); 

      Point3F target_vec = target_pos - mCurrPosition;

      F32 target_dist_sq = target_vec.lenSquared();
      if (target_dist_sq < 4.0f)
         prec_inc += 1.0f;

      // hover
      if (mDataBlock->hover_altitude > 0.0f)
      {
         Point3F target_vec_xy(target_vec.x, target_vec.y, 0);
         F32 xy_dist = target_vec_xy.len();

         if (xy_dist > mDataBlock->hover_attack_distance)
         {          
            hover_attack_go = false;

            if (xy_dist > mDataBlock->hover_attack_distance + mDataBlock->hover_attack_gradient)
            {
               target_pos.z += mDataBlock->hover_altitude;          
            }
            else
            {
               target_pos.z += afxEase::eq( (xy_dist-mDataBlock->hover_attack_distance)/mDataBlock->hover_attack_gradient, 
                  0.0f, mDataBlock->hover_altitude, 
                  0.25f, 0.75f);
            }          			
            target_vec = target_pos - mCurrPosition;
         }

         else
         {
            if (!hover_attack_go) 
            {
               hover_attack_go = true;
               hover_attack_tick = 0;
            }
            hover_attack_tick++;

            if (hover_attack_tick < mDataBlock->hover_time)
            {
               target_pos.z += mDataBlock->hover_altitude;
               target_vec = target_pos - mCurrPosition;
            }
         }
      }

      // apply precision 

      // extract speed
      F32 speed = mCurrVelocity.len(); 

      // normalize vectors
      target_vec.normalizeSafe();
      mCurrVelocity.normalize();

      F32 prec = mDataBlock->precision;

      // fade in precision gradually to avoid sudden turn
      if (mCurrTick < mDataBlock->trackDelay + 16)
         prec *= (mCurrTick - mDataBlock->trackDelay)/16.0f;

      prec += prec_inc;
      if (prec > 100)
         prec = 100;

      // apply precision weighting
      target_vec *= prec;
      mCurrVelocity *= (100 - prec);

      mCurrVelocity += target_vec;
      mCurrVelocity.normalize();
      mCurrVelocity *= speed;
   } 

   // wiggle
   for (U32 i = 0; i < mDataBlock->wiggle_num_axis; i++)
   {
      if (i >= mDataBlock->wiggle_magnitudes.size() || i >= mDataBlock->wiggle_speeds.size()) 
         break;

      F32 wiggle_mag   = mDataBlock->wiggle_magnitudes[i];
      F32 wiggle_speed = mDataBlock->wiggle_speeds[i];
      Point3F wiggle_axis = mDataBlock->wiggle_axis[i];
      //wiggle_axis.normalizeSafe(); // sufficient????

      F32 theta = wiggle_mag * mSin(wiggle_speed*(mCurrTick*TickSec));
      //Con::printf( "theta: %f", theta );    
      AngAxisF thetaRot(wiggle_axis, theta);
      MatrixF temp(true);
      thetaRot.setMatrix(&temp);
      temp.mulP(mCurrVelocity);
   }

   Point3F new_pos = old_pos + mCurrVelocity*dT;

   // conform to terrain
   if (mDataBlock->followTerrain && mCurrTick >= mDataBlock->followTerrainAdjustDelay) 
   {
      U32 mask = TerrainObjectType | TerrainLikeObjectType; //  | InteriorObjectType;

      F32 ht = mDataBlock->followTerrainHeight;
      F32 ht_rate = mDataBlock->followTerrainAdjustRate;
      F32 ht_min = 0.05f;
      if (ht < ht_min)
         ht = ht_min;

      SceneContainer* container = (isServerObject()) ? &gServerContainer : &gClientContainer;
      Point3F above_pos = new_pos; above_pos.z += 10000;
      Point3F below_pos = new_pos; below_pos.z -= 10000;
      RayInfo rInfo;
      if (container && container->castRay(above_pos, below_pos, mask, &rInfo)) 
      {
         F32 terrain_z = rInfo.point.z;
         F32 seek_z = terrain_z + ht;
         if (new_pos.z < seek_z)
         {
            new_pos.z += ht_rate*dT;
            if (new_pos.z > seek_z)
               new_pos.z = seek_z;
         }
         else if (new_pos.z > seek_z)
         {
            new_pos.z -= ht_rate*dT;
            if (new_pos.z < seek_z)
               new_pos.z = seek_z;
         }

         if (new_pos.z < terrain_z + ht_min)
            new_pos.z = terrain_z + ht_min;
      }
   }

   // only check for impacts on server
   if (isServerObject())
   {
      // avoid collision with the spellcaster
      if (collide_exempt && mCurrTick <= mDataBlock->caster_safety_time)
         collide_exempt->disableCollision();

      // check for collision along ray from old to new position
      RayInfo rInfo;
      bool did_hit = false;

      if (mPhysicsWorld)
      {
         // did_hit = mPhysicsWorld->castRay(old_pos, new_pos, &rInfo, Point3F(new_pos - old_pos) * mDataBlock->impactForce );
         // Impulse currently hardcoded for testing purposes
         did_hit = mPhysicsWorld->castRay(old_pos, new_pos, &rInfo, Point3F(new_pos - old_pos) * 1000.0f );
      }
      else
      {
         did_hit = getContainer()->castRay(old_pos, new_pos, collision_mask, &rInfo);
      }

      // restore collisions on spellcaster 
      if (collide_exempt && mCurrTick <= mDataBlock->caster_safety_time)
         collide_exempt->enableCollision();

      // process impact
      if (did_hit)
      {
         MatrixF xform(true);
         xform.setColumn(3, rInfo.point);
         setTransform(xform);
         mCurrPosition = rInfo.point;
         mCurrVelocity = Point3F(0, 0, 0);
         did_impact = true;
         setMaskBits(ImpactMask);
         if (choreographer)
         {
            choreographer->impactNotify(rInfo.point, rInfo.normal, rInfo.object);
            Sim::postEvent(this, new ObjectDeleteEvent, Sim::getCurrentTime() + 500);
         }
      }
   }
   else // if (isClientObject())
   {
      emitParticles(mCurrPosition, new_pos, mCurrVelocity, TickMs);
      updateSound();
   }

   // interp values used in interpolateTick()
   mCurrDeltaBase = new_pos;
   mCurrBackDelta = mCurrPosition - new_pos;

   mCurrPosition = new_pos;

   MatrixF xform(true);
   xform.setColumn(3, mCurrPosition);
   setTransform(xform);
}

void afxMagicMissile::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   // only active from launch to impact
   if (!is_active())
      return;

   Point3F interpPos = mCurrDeltaBase + mCurrBackDelta * delta;
   Point3F dir = mCurrVelocity;
   if(dir.isZero())
      dir.set(0,0,1);
   else
      dir.normalize();

   MatrixF xform(true);
	xform = MathUtils::createOrientFromDir(dir);
   xform.setPosition(interpPos);
   setRenderTransform(xform);

   updateSound();
}

//--------------------------------------------------------------------------
U32 afxMagicMissile::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate( con, mask, stream );

   const bool isInitalUpdate = mask & GameBase::InitialUpdateMask;

   // InitialUpdateMask
   if ( stream->writeFlag( isInitalUpdate ) )
   {
      Point3F pos;
      getTransform().getColumn( 3, &pos );
      stream->writeCompressedPoint( pos );
      F32 len = mCurrVelocity.len();

      if ( stream->writeFlag( len > 0.02 ) )
      {
         Point3F outVel = mCurrVelocity;
         outVel *= 1 / len;
         stream->writeNormalVector( outVel, 10 );
         len *= 32.0; // 5 bits for fraction

         if ( len > 8191 )
             len = 8191;

         stream->writeInt( (S32)len, 13 );
      }

      stream->writeRangedU32( mCurrTick, 0, afxMagicMissileData::MaxLifetimeTicks );

      if (choreographer)
      {
         S32 ghostIndex = con->getGhostIndex( choreographer );
         if ( stream->writeFlag( ghostIndex != -1 ) )
         {
            stream->writeRangedU32( U32(ghostIndex), 
                                    0, 
                                    NetConnection::MaxGhostCount );
         }
         else 
            // have not recieved the ghost for the source object yet, try again later
            retMask |= GameBase::InitialUpdateMask;
      }
      else
         stream->writeFlag( false );
   }

   // impact update
   if (stream->writeFlag(mask & ImpactMask))
   {
      mathWrite(*stream, mCurrPosition);
      mathWrite(*stream, mCurrVelocity);
      stream->writeFlag(did_impact);
   }

   // guided update
   if (stream->writeFlag(mask & GuideMask))
   {
      mathWrite(*stream, mCurrPosition);
      mathWrite(*stream, mCurrVelocity);
   }

   return retMask;
}

void afxMagicMissile::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);
   
   if ( stream->readFlag() ) // InitialUpdateMask
   {
      Point3F pos;
      stream->readCompressedPoint( &pos );
      if ( stream->readFlag() )
      {
         stream->readNormalVector( &mCurrVelocity, 10 );
         mCurrVelocity *= stream->readInt( 13 ) / 32.0f;
      }
      else
         mCurrVelocity.zero();

      mCurrDeltaBase = pos;
      mCurrBackDelta = mCurrPosition - pos;
      mCurrPosition  = pos;
      setPosition( mCurrPosition );

      mCurrTick = stream->readRangedU32(0, afxMagicMissileData::MaxLifetimeTicks);
      if ( stream->readFlag() )
      {
         U32 id   = stream->readRangedU32(0, NetConnection::MaxGhostCount);
         choreographer = dynamic_cast<afxChoreographer*>(con->resolveGhost(id));
         if (choreographer)
         {
            deleteNotify(choreographer);
         }
      }
      else
      {
         if (choreographer)
            clearNotify(choreographer);
         choreographer = 0;
      }
   }
   
   // impact update
   if (stream->readFlag())
   {
      mathRead(*stream, &mCurrPosition);
      mathRead(*stream, &mCurrVelocity);
      did_impact = stream->readFlag();

   }

   if (stream->readFlag()) // guided update
   {
      mathRead( *stream, &mCurrPosition );
      mathRead( *stream, &mCurrVelocity );
   }
}

//--------------------------------------------------------------------------
void afxMagicMissile::prepRenderImage(SceneRenderState* state)
{
   if (!is_active())
      return;

   /*
   if (isHidden() || mFadeValue <= (1.0/255.0))
      return;
   */

   if ( mDataBlock->lightDesc )
   {
     mDataBlock->lightDesc->prepRender( state, &mLightState, getRenderTransform() );
   }

   /*
   if ( mFlareData )
   {
     mFlareState.fullBrightness = mDataBlock->lightDesc->mBrightness;
     mFlareState.scale = mFlareScale;
     mFlareState.lightInfo = mLight;
     mFlareState.lightMat = getTransform();

     mFlareData->prepRender( state, &mFlareState );
   }
   */

   prepBatchRender( state );
}

void afxMagicMissile::prepBatchRender(SceneRenderState* state)
{
   if ( !mProjectileShape )
      return;

   GFXTransformSaver saver;

   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState( state );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   MatrixF mat = getRenderTransform();
   mat.scale( mObjScale );
   mat.scale( mDataBlock->scale );
   GFX->setWorldMatrix( mat );

   mProjectileShape->setDetailFromPosAndScale( state, mat.getPosition(), mObjScale );
   mProjectileShape->animate();

   mProjectileShape->render( rdata );
}

void afxMagicMissile::onDeleteNotify(SimObject* obj)
{
  ShapeBase* shape_test = dynamic_cast<ShapeBase*>(obj);
  if (shape_test == collide_exempt)
  {
    collide_exempt = NULL;
    Parent::onDeleteNotify(obj);
    return;
  }

  SceneObject* target_test = dynamic_cast<SceneObject*>(obj);
  if (target_test == missile_target)
  {
    missile_target = NULL;
    Parent::onDeleteNotify(obj);
    return;
  }

  afxChoreographer* ch = dynamic_cast<afxChoreographer*>(obj);
  if (ch == choreographer)
  {
    choreographer = NULL;
    Parent::onDeleteNotify(obj);
    return;
  }

  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// private:

void afxMagicMissile::create_splash(const Point3F& pos)
{
  if (!mDataBlock || !mDataBlock->splash)
    return;

  MatrixF xfm = getTransform();
  xfm.setPosition(pos);

  Splash* splash = new Splash();
  splash->onNewDataBlock(mDataBlock->splash, false);
  splash->setTransform(xfm);
  splash->setInitialState(xfm.getPosition(), Point3F(0.0, 0.0, 1.0));
  if (!splash->registerObject())
  {
    delete splash;
    splash = NULL;
  }
}

void afxMagicMissile::get_launch_constraint_data(Point3F& pos, Point3F& vel)
{
  // need a choreographer
  if (!choreographer)
  {
    Con::errorf("afxMagicMissile::get_launch_constraint_data(): missing choreographer.");
    return;
  }

  // need a constraint manager
  afxConstraintMgr* cons_mgr = choreographer->getConstraintMgr();
  if (!cons_mgr)
  {
    Con::errorf("afxMagicMissile::get_launch_constraint_data(): missing constriant manager.");
    return;
  }

  // need a valid constraint
  afxConstraintID launch_cons_id;
  if (isServerObject())
    launch_cons_id = cons_mgr->getConstraintId(mDataBlock->launch_cons_s_def);
  else
    launch_cons_id = cons_mgr->getConstraintId(mDataBlock->launch_cons_c_def);

  afxConstraint* launch_cons = cons_mgr->getConstraint(launch_cons_id);
  if (!launch_cons)
  {
    Con::errorf("afxMagicMissile::get_launch_constraint_data(): constraint undefined.");
    return;
  }

  MatrixF launch_xfm;
  launch_cons->getTransform(launch_xfm);

  Point3F launch_pos;
  launch_cons->getPosition(launch_pos);

  pos = launch_pos;

  // echo the actual launch position to the console
  if (mDataBlock->echo_launch_offset)
  {
    SceneObject* default_launcher = get_default_launcher();
    if (default_launcher)
    {
      MatrixF launcher_xfm_inv = default_launcher->getWorldTransform();
      VectorF offset = pos - default_launcher->getRenderPosition();
      launcher_xfm_inv.mulV(offset);
      if (isServerObject())
        Con::printf("launchOffsetServer = \"%g %g %g\";", offset.x, offset.y, offset.z);
      else
        Con::printf("launchOffsetClient = \"%g %g %g\";", offset.x, offset.y, offset.z);
    }
  }

  // setup aiming matrix to straight forward and level
  MatrixF aim_mtx;
  AngAxisF aim_aa(Point3F(0,1,0),0);
  aim_aa.setMatrix(&aim_mtx);

  // calculate final aiming vector
  MatrixF aim2_mtx;
  aim2_mtx.mul(launch_xfm, aim_mtx);

  VectorF aim_vec;
  aim2_mtx.getColumn(1,&aim_vec);
  aim_vec.normalizeSafe();

  // give velocity vector a magnitude
  vel = aim_vec*mDataBlock->muzzleVelocity;
}

// resolve the launch constraint object. normally it's the caster, but for
// reverse_targeting the target object us used.
SceneObject* afxMagicMissile::get_default_launcher() const
{
  SceneObject* launch_cons_obj = 0;
  if (mDataBlock->reverse_targeting)
  {
    if (dynamic_cast<afxMagicSpell*>(choreographer))
      launch_cons_obj = ((afxMagicSpell*)choreographer)->target;
    if (!launch_cons_obj)
    {
      Con::errorf("afxMagicMissile::get_launch_data(): missing target constraint object for reverse targeted missile.");
      return 0;
    }
  }
  else
  {
    if (dynamic_cast<afxMagicSpell*>(choreographer))
      launch_cons_obj = ((afxMagicSpell*)choreographer)->caster;
    if (!launch_cons_obj)
    {
      Con::errorf("afxMagicMissile::get_launch_data(): missing launch constraint object missile.");
      return 0;
    }
  }

  return launch_cons_obj;
}

void afxMagicMissile::get_launch_data(Point3F& pos, Point3F& vel)
{
  bool use_constraint = (isServerObject()) ? mDataBlock->launch_cons_s_def.isDefined() : mDataBlock->launch_cons_c_def.isDefined();
  if (use_constraint)
  {
    get_launch_constraint_data(pos, vel);
    return;
  }  
  
  // a choreographer pointer is required
  if (!choreographer)
  {
    Con::errorf("afxMagicMissile::get_launch_data(): missing choreographer.");
    return;
  }

  SceneObject* launch_cons_obj = get_default_launcher();
  if (!launch_cons_obj)
    return;

  MatrixF launch_xfm = launch_cons_obj->getRenderTransform();

  // calculate launch position
  Point3F offset_override = (isClientObject()) ?  mDataBlock->launch_offset_client : 
                                                  mDataBlock->launch_offset_server;
  // override
  if (!offset_override.isZero())
  {
    launch_xfm.mulV(offset_override);
    pos = launch_cons_obj->getRenderPosition() + offset_override;
  }
  // no override 
  else
  {
    // get transformed launch offset
    VectorF launch_offset = mDataBlock->launch_offset;
    launch_xfm.mulV(launch_offset);
    
    StringTableEntry launch_node = mDataBlock->launch_node;
    
    // calculate position of missile at launch
    if (launch_node != ST_NULLSTRING)
    {
      ShapeBase* launch_cons_shape = dynamic_cast<ShapeBase*>(launch_cons_obj);
      TSShapeInstance* shape_inst = (launch_cons_shape) ? launch_cons_shape->getShapeInstance() : 0;
      if (!shape_inst || !shape_inst->getShape())
        launch_node = ST_NULLSTRING;
      else
      {
        S32 node_ID = shape_inst->getShape()->findNode(launch_node);
        MatrixF node_xfm = launch_cons_obj->getRenderTransform();
        node_xfm.scale(launch_cons_obj->getScale());
        if (node_ID >= 0)
          node_xfm.mul(shape_inst->mNodeTransforms[node_ID]);
        
        VectorF node_offset = mDataBlock->launch_node_offset;
        node_xfm.mulV(node_offset);
        
        pos = node_xfm.getPosition() + launch_offset + node_offset;
      }
    }   
    // calculate launch position without launch node
    else
      pos = launch_cons_obj->getRenderPosition() + launch_offset;
  }

  // echo the actual launch position to the console
  if (mDataBlock->echo_launch_offset)
  {
    VectorF offset = pos - launch_cons_obj->getRenderPosition();
    MatrixF caster_xfm_inv = launch_xfm;
    caster_xfm_inv.affineInverse();
    caster_xfm_inv.mulV(offset);
    if (isServerObject())
      Con::printf("launchOffsetServer = \"%g %g %g\";", offset.x, offset.y, offset.z);
    else
      Con::printf("launchOffsetClient = \"%g %g %g\";", offset.x, offset.y, offset.z);
  }

  // calculate launch velocity vector
  if (starting_vel_vec.isZero())
  {
    // setup aiming matrix to straight forward and level
    MatrixF aim_mtx;
    AngAxisF aim_aa(Point3F(0,1,0),0);
    aim_aa.setMatrix(&aim_mtx);
    
    // setup pitch matrix
    MatrixF pitch_mtx;
    AngAxisF pitch_aa(Point3F(1,0,0),mDegToRad(mDataBlock->launch_pitch));
    pitch_aa.setMatrix(&pitch_mtx);
    
    // setup pan matrix
    MatrixF pan_mtx;
    AngAxisF pan_aa(Point3F(0,0,1),mDegToRad(mDataBlock->launch_pan));
    pan_aa.setMatrix(&pan_mtx);
    
    // calculate adjusted aiming matrix
    aim_mtx.mul(pitch_mtx);
    aim_mtx.mul(pan_mtx);
    
    // calculate final aiming vector
    MatrixF aim2_mtx;
    aim2_mtx.mul(launch_xfm, aim_mtx);
    VectorF aim_vec;
    aim2_mtx.getColumn(1,&aim_vec);
    aim_vec.normalizeSafe();
    
    // give velocity vector a magnitude
    vel = aim_vec*mDataBlock->muzzleVelocity;
  }
  else
  {
    vel = starting_vel_vec*starting_velocity;
  }
}

void afxMagicMissile::updateSound()
{
  if (!mDataBlock->sound)
    return;

  if ( mSound )
  {
    if ( !mSound->isPlaying() )
      mSound->play();

    mSound->setVelocity( getVelocity() );
    mSound->setTransform( getRenderTransform() );
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// public:

void afxMagicMissile::launch()
{
  get_launch_data(mCurrPosition, mCurrVelocity);

  mCurrDeltaBase = mCurrPosition;
  mCurrBackDelta.zero();

  did_launch = true;

  setPosition(mCurrPosition);

  afxMagicSpell* spell = dynamic_cast<afxMagicSpell*>(choreographer);
  if (spell)
  {
    if (mDataBlock->reverse_targeting)
    {
      missile_target = spell->caster;
      collide_exempt = spell->target;
    }
    else
    {
      missile_target = spell->target;
      collide_exempt = spell->caster;
    }

    if (spell->caster)
      processAfter(spell->caster);
    if (missile_target)
      deleteNotify(missile_target);
    if (collide_exempt)
      deleteNotify(collide_exempt);
  }
  else
  {
    missile_target = 0;
    collide_exempt = 0;
  }
}

void afxMagicMissile::setChoreographer(afxChoreographer* chor)
{
  if (choreographer)
    clearNotify(choreographer);
  choreographer = chor;
  if (choreographer)
    deleteNotify(choreographer);
}

void afxMagicMissile::setStartingVelocityVector(const Point3F& vel_vec)
{
  starting_vel_vec = vel_vec;
}

void afxMagicMissile::setStartingVelocity(const F32 vel)
{
  starting_velocity = vel;
}

void afxMagicMissile::getStartingVelocityValues(F32& vel, Point3F& vel_vec)
{
  vel = starting_velocity;
  vel_vec = starting_vel_vec;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

/* From stock Projectile code...
DefineEngineMethod(Projectile, presimulate, void, (F32 seconds), (1.0f), "Updates velocity and position, and performs collision testing.\n"
													"@param seconds Amount of time, in seconds, since the simulation began, to start the simulation at.\n"
													"@tsexample\n"
														"// Tell the projectile object to process a simulation event, and provide the amount of time\n"
														"   in seconds that has passed since the simulation began.\n"
														"%seconds = 2000;\n"
														"%projectile.presimulate(%seconds);\n"
													"@endtsexample\n")
{
	object->simulate( seconds );
}
*/

DefineEngineMethod(afxMagicMissile, setStartingVelocityVector, void, (Point3F velocityVec),,
                   "Set the starting velocity-vector for a magic-missile.\n\n"
                   "@ingroup AFX")
{
  object->setStartingVelocityVector(velocityVec);
}

DefineEngineMethod(afxMagicMissile, setStartingVelocity, void, (float velocity),,
                   "Set the starting velocity for a magic-missile.\n\n"
                   "@ingroup AFX")
{
  object->setStartingVelocity(velocity);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
