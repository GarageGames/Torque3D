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

#ifndef _AFX_MAGIC_MISSILE_H_
#define _AFX_MAGIC_MISSILE_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "T3D/lightDescription.h"
#include "T3D/fx/particleEmitter.h"

#include "afx/afxConstraint.h"

class SplashData;
class ShapeBase;
class TSShapeInstance;
class PhysicsWorld;
class SFXTrack;
class SFXSource;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicMissileData

class afxMagicMissileData : public GameBaseData
{
  typedef GameBaseData Parent;
  
protected:
  bool onAdd();

public:
  enum { MaxLifetimeTicks = 4095 };
  
public:
   // variables set in datablock definition:
   // Shape related
  StringTableEntry      projectileShapeName;

  //bool                  hasLight;
  //F32                   lightRadius;
  //ColorF                lightColor;

  //bool                  hasWaterLight;
  //ColorF                waterLightColor;

  /*
  /// Set to true if it is a billboard and want it to always face the viewer, false otherwise
  bool faceViewer;
  */
  Point3F               scale;

  /*
  /// [0,1] scale of how much velocity should be inherited from the parent object
  F32 velInheritFactor;
  /// Speed of the projectile when fired
  */
  F32                   muzzleVelocity;

   /// Should it arc?
  bool                  isBallistic;

  /*
  /// How HIGH should it bounce (parallel to normal), [0,1]
  F32 bounceElasticity;
  /// How much momentum should be lost when it bounces (perpendicular to normal), [0,1]
  F32 bounceFriction;
  */

   /// Should this projectile fall/rise different than a default object?
  F32                   gravityMod;

   /// How long the projectile should exist before deleting itself
  U32                   lifetime;     // ticks
  /*
  /// How long it should not detonate on impact
  S32 armingDelay;  // the values are converted on initialization with
  */
  S32                   fadeDelay;    // ticks

  /*
  ExplosionData* explosion;           // Explosion Datablock
  S32 explosionId;                    // Explosion ID
  ExplosionData* waterExplosion;      // Water Explosion Datablock
  S32 waterExplosionId;               // Water Explosion ID
  */

  SplashData* splash;                 // Water Splash Datablock
  S32 splashId;                       // Water splash ID

  SFXTrack* sound;                    // Projectile Sound

  LightDescription *lightDesc;
  S32 lightDescId;   

  /*
  enum DecalConstants {               // Number of decals constant
    NumDecals = 6,
  };
  DecalData* decals[NumDecals];       // Decal Datablocks
  S32 decalId[NumDecals];             // Decal IDs
  U32 decalCount;                     // # of loaded Decal Datablocks
  */

   // variables set on preload:
  Resource<TSShape>     projectileShape;
  /*
  S32 activateSeq;
  S32 maintainSeq;
  */

  ParticleEmitterData*  particleEmitter;
  S32                   particleEmitterId;
  ParticleEmitterData*  particleWaterEmitter;
  S32                   particleWaterEmitterId;

  U32                   collision_mask;

  Point3F               starting_vel_vec;

                        // guidance behavior
  bool                  isGuided;
  F32                   precision;
  S32                   trackDelay;

                        // simple physics
  F32                   ballisticCoefficient;

                        // terrain following
  bool                  followTerrain;
  F32                   followTerrainHeight;
  F32                   followTerrainAdjustRate;
  S32                   followTerrainAdjustDelay;

  F32                   acceleration;
  S32                   accelDelay;
  U32                   accelLifetime;

  StringTableEntry      launch_node;
  Point3F               launch_offset;
  Point3F               launch_offset_server;
  Point3F               launch_offset_client;
  Point3F               launch_node_offset;
  F32                   launch_pitch;
  F32                   launch_pan;
  bool                  echo_launch_offset;

  StringTableEntry      launch_cons_s_spec;
  afxConstraintDef      launch_cons_s_def;
  StringTableEntry      launch_cons_c_spec;
  afxConstraintDef      launch_cons_c_def;

                        // wiggle behavior
  Vector<F32>           wiggle_magnitudes;
  Vector<F32>           wiggle_speeds;
  StringTableEntry      wiggle_axis_string;
  Point3F*              wiggle_axis;
  U32                   wiggle_num_axis;

                        // hover behavior
  F32                   hover_altitude;
  F32                   hover_attack_distance;
  F32                   hover_attack_gradient;
  U32                   hover_time;

  bool                  reverse_targeting;

  U32                   caster_safety_time;

public:
  /*C*/                 afxMagicMissileData();
  /*D*/                 ~afxMagicMissileData();
  
  void                  packData(BitStream*);
  void                  unpackData(BitStream*);

  bool                  preload(bool server, String &errorStr);
  
  static void           initPersistFields();
  
  DECLARE_CONOBJECT(afxMagicMissileData);
  DECLARE_CATEGORY("AFX");

public:
  /*C*/                 afxMagicMissileData(const afxMagicMissileData&, bool = false);

  afxMagicMissileData*  cloneAndPerformSubstitutions(const SimObject*, S32 index=0);
  virtual bool          allowSubstitutions() const { return true; }
  void                  gather_cons_defs(Vector<afxConstraintDef>& defs);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicMissile

//class afxMagicSpell;
class afxChoreographer;

class afxMagicMissile : public GameBase, public ISceneLight
{
  typedef GameBase Parent;

public:
  /*
  // Initial conditions
  enum ProjectileConstants {
    SourceIdTimeoutTicks = 7,   // = 231 ms
    DeleteWaitTime       = 500, ///< 500 ms delete timeout (for network transmission delays)
    ExcessVelDirBits     = 7,
    MaxLivingTicks       = 4095,
  };
  */
  enum UpdateMasks {
    /*
    BounceMask    = Parent::NextFreeMask,
    ExplosionMask = Parent::NextFreeMask << 1,
    */
    GuideMask     = Parent::NextFreeMask << 0,
    LaunchMask    = Parent::NextFreeMask << 1,
    ImpactMask    = Parent::NextFreeMask << 2,
    NextFreeMask  = Parent::NextFreeMask << 3
  }; 
protected:
  PhysicsWorld *mPhysicsWorld;

  afxMagicMissileData* mDataBlock;

  ParticleEmitter*  mParticleEmitter;
  ParticleEmitter*  mParticleWaterEmitter;
  SFXSource* mSound;

  Point3F  mCurrPosition;
  Point3F  mCurrVelocity;
  /*
  S32      mSourceObjectId;
  S32      mSourceObjectSlot;
  */

   // Time related variables common to all projectiles, managed by processTick

  U32 mCurrTick;                         ///< Current time in ticks
  /*
  SimObjectPtr<ShapeBase> mSourceObject; ///< Actual pointer to the source object, times out after SourceIdTimeoutTicks
  */

   // Rendering related variables
  TSShapeInstance* mProjectileShape;
  /*
  TSThread*        mActivateThread;
  TSThread*        mMaintainThread;

  Point3F          mLastRenderPos;
  */

  // ISceneLight
  virtual void submitLights( LightManager *lm, bool staticLighting );
  virtual LightInfo* getLight() { return mLight; }

  LightInfo *mLight;
  LightState mLightState;

  /*
  bool             mHidden;        ///< set by the derived class, if true, projectile doesn't render
  F32              mFadeValue;     ///< set in processTick, interpolation between fadeDelay and lifetime
                                 ///< in data block
  */

  /*
  // Warping and back delta variables.  Only valid on the client
  //
  Point3F mWarpStart;
  Point3F mWarpEnd;
  U32     mWarpTicksRemaining;
  */

  Point3F mCurrDeltaBase;
  Point3F mCurrBackDelta;

  /*
  Point3F mExplosionPosition;
  Point3F mExplosionNormal;
  U32     mCollideHitType;
  */
  
  bool onAdd();
  void onRemove();
  bool onNewDataBlock(GameBaseData *dptr, bool reload);

  // Rendering
  virtual void prepRenderImage(SceneRenderState*);
  void prepBatchRender( SceneRenderState *state); 

  void processTick(const Move *move);
  /*
  void advanceTime(F32 dt);
  */
  void interpolateTick(F32 delta);

  /*
  /// What to do once this projectile collides with something
  virtual void onCollision(const Point3F& p, const Point3F& n, SceneObject*);

  /// What to do when this projectile explodes
  virtual void explode(const Point3F& p, const Point3F& n, const U32 collideType );

  /// Returns the velocity of the projectile
  Point3F getVelocity() const;
  */

  void              emitParticles(const Point3F&, const Point3F&, const Point3F&, const U32);
  void              updateSound();

  // Rendering
  /*
  void prepModelView    ( SceneRenderState *state);
  */

   // These are stolen from the player class ..
  bool              pointInWater(const Point3F &point);

  U32  packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
  void unpackUpdate(NetConnection *conn, BitStream *stream);

  afxChoreographer* choreographer;

  bool              client_only;
  bool              server_only;
  bool              use_accel;
  U32               collision_mask;
  F32               prec_inc;

  bool              did_launch;
  bool              did_impact; 
  
  SceneObject*      missile_target;
  SceneObject*      collide_exempt;

  bool              hover_attack_go;
  U32               hover_attack_tick;
  
  F32               starting_velocity;
  Point3F           starting_vel_vec;

  SimObject*        ss_object;
  S32               ss_index;

private:
  void              init(bool on_server, bool on_client);
  void              create_splash(const Point3F& pos);
  SceneObject*      get_default_launcher() const;
  void              get_launch_constraint_data(Point3F& pos, Point3F& vel);
  void              get_launch_data(Point3F& pos, Point3F& vel);
  bool              is_active() const { return (did_launch && !did_impact); }

public:
  /*
  F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);
  */
  /*C*/             afxMagicMissile();
  /*C*/             afxMagicMissile(bool on_server, bool on_client);
  /*D*/             ~afxMagicMissile();
  virtual void      onDeleteNotify(SimObject*);

  DECLARE_CONOBJECT(afxMagicMissile);
  DECLARE_CATEGORY("AFX");

  static void       initPersistFields();  

  /*
  virtual bool calculateImpact(float    simTime,
                               Point3F& pointOfImpact,
                               float&   impactTime);

  static U32 smProjectileWarpTicks;

protected:
  static const U32 csmStaticCollisionMask;
  static const U32 csmDynamicCollisionMask;
  static const U32 csmDamageableMask;
  */
  
  void              launch();
  void              setChoreographer(afxChoreographer*); 
  void              setStartingVelocityVector(const Point3F& vel_vec);
  void              setStartingVelocity(const F32 vel);
  void              getStartingVelocityValues(F32& vel, Point3F& vel_vec); 
  void              setSubstitutionData(SimObject* obj, S32 idx=0) { ss_object = obj; ss_index = idx; }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxMagicMissileCallback

class afxMagicMissileCallback
{
public:
  virtual void impactNotify(const Point3F& p, const Point3F& n, SceneObject*)=0;
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_MAGIC_MISSILE_H_

