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

#ifndef _SHAPEBASE_H_
#define _SHAPEBASE_H_

#ifndef __RESOURCE_H__
   #include "core/resource.h"
#endif
#ifndef _GAMEBASE_H_
   #include "T3D/gameBase/gameBase.h"
#endif
#ifndef _MOVEMANAGER_H_
   #include "T3D/gameBase/moveManager.h"
#endif
#ifndef _COLOR_H_
   #include "core/color.h"
#endif
#ifndef _CONVEX_H_
   #include "collision/convex.h"
#endif
#ifndef _SCENERENDERSTATE_H_
   #include "scene/sceneRenderState.h"
#endif
#ifndef _NETSTRINGTABLE_H_
   #include "sim/netStringTable.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
   #include "renderInstance/renderPassManager.h"
#endif
#ifndef _TSSHAPE_H_
   #include "ts/tsShape.h"
#endif
#ifndef _BITVECTOR_H_
   #include "core/bitVector.h"
#endif
#ifndef _LIGHTINFO_H_
   #include "lighting/lightInfo.h"
#endif
#ifndef _REFLECTOR_H_
   #include "scene/reflector.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif

// Need full definition visible for SimObjectPtr<ParticleEmitter>
#include "T3D/fx/particleEmitter.h"

class GFXCubemap;
class TSShapeInstance;
class SceneRenderState;
class TSThread;
class GameConnection;
struct CameraScopeQuery;
class ProjectileData;
class ExplosionData;
struct DebrisData;
class ShapeBase;
class SFXSource;
class SFXTrack;
class SFXProfile;

typedef void* Light;


//--------------------------------------------------------------------------

extern void collisionFilter(SceneObject* object,S32 key);
extern void defaultFilter(SceneObject* object,S32 key);


//--------------------------------------------------------------------------
class ShapeBaseConvex : public Convex
{
   typedef Convex Parent;
   friend class ShapeBase;
   friend class Vehicle;
   friend class RigidShape;

  protected:
   ShapeBase*  pShapeBase;
   MatrixF*    nodeTransform;

  public:
   MatrixF*    transform;
   U32         hullId;
   Box3F       box;

  public:
   ShapeBaseConvex() { mType = ShapeBaseConvexType; nodeTransform = 0; }
   ShapeBaseConvex(const ShapeBaseConvex& cv) {
      mObject    = cv.mObject;
      pShapeBase = cv.pShapeBase;
      hullId     = cv.hullId;
      nodeTransform = cv.nodeTransform;
      box        = cv.box;
      transform  = 0;
   }

   void findNodeTransform();
   const MatrixF& getTransform() const;
   Box3F getBoundingBox() const;
   Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   Point3F      support(const VectorF& v) const;
   void         getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void         getPolyList(AbstractPolyList* list);
};

//--------------------------------------------------------------------------

struct ShapeBaseImageData: public GameBaseData {
  private:
   typedef GameBaseData Parent;

  public:
   enum Constants {
      MaxStates    = 31,            ///< We get one less than state bits because of
                                    /// the way data is packed.

      MaxShapes    = 2,             ///< The number of allowed shapes per image.  Only
                                    /// the first shape is required.

      MaxGenericTriggers = 4,       ///< The number of generic triggers for the image.

      StandardImageShape = 0,       ///< Shape index used for the standard image shape
      FirstPersonImageShape = 1,    ///< Shape index used for the optional first person image shape

      NumStateBits = 5,
   };
   enum LightType {
      NoLight = 0,
      ConstantLight,
      SpotLight,
      PulsingLight,
      WeaponFireLight,
      NumLightTypes
   };
   struct StateData {
      StateData();
      const char* name;             ///< State name

      /// @name Transition states
      ///
      /// @{

      ///
      struct Transition {
         S32 loaded[2];             ///< NotLoaded/Loaded
         S32 ammo[2];               ///< Noammo/ammo
         S32 target[2];             ///< target/noTarget
         S32 trigger[2];            ///< Trigger up/down
         S32 altTrigger[2];         ///< Second trigger up/down
         S32 wet[2];                ///< wet/notWet
         S32 motion[2];             ///< NoMotion/Motion
         S32 timeout;               ///< Transition after delay
         S32 genericTrigger[ShapeBaseImageData::MaxGenericTriggers][2];    ///< Generic trigger Out/In
      } transition;
      bool ignoreLoadedForReady;

      /// @}

      /// @name State attributes
      /// @{

      bool fire;                    ///< Can only have one fire state
      bool altFire;                 ///< Can only have one alternate fire state
      bool reload;                  ///< Can only have one reload state
      bool ejectShell;              ///< Should we eject a shell casing in this state?
      bool allowImageChange;        ///< Can we switch to another image while in this state?
                                    ///
                                    ///  For instance, if we have a rocket launcher, the player
                                    ///  shouldn't be able to switch out <i>while</i> firing. So,
                                    ///  you'd set allowImageChange to false in firing states, and
                                    ///  true the rest of the time.
      bool scaleAnimation;          ///< Scale animation to fit the state timeout
      bool scaleAnimationFP;        ///< Scale animation to fit the state timeout while in first person
      bool direction;               ///< Animation direction
      bool sequenceTransitionIn;    ///< Do we transition to the state's sequence when we enter the state?
      bool sequenceTransitionOut;   ///< Do we transition to the new state's sequence when we leave the state?
      bool sequenceNeverTransition; ///< Never allow a transition to this sequence.  Often used for a fire sequence.
      F32 sequenceTransitionTime;   ///< The time to transition in or out of a sequence.
      bool waitForTimeout;          ///< Require the timeout to pass before advancing to the next
                                    ///  state.
      F32 timeoutValue;             ///< A timeout value; the effect of this value is determined
                                    ///  by the flags scaleAnimation and waitForTimeout
      F32 energyDrain;              ///< Sets the energy drain rate per second of this state.
                                    ///
                                    ///  Energy is drained at energyDrain units/sec as long as
                                    ///  we are in this state.
      enum LoadedState {
         IgnoreLoaded,              ///< Don't change loaded state.
         Loaded,                    ///< Set us as loaded.
         NotLoaded,                 ///< Set us as not loaded.
         NumLoadedBits = 3          ///< How many bits to allocate to representing this state. (3 states needs 2 bits)
      } loaded;                     ///< Is the image considered loaded?
      enum SpinState {
         IgnoreSpin,                ///< Don't change spin state.
         NoSpin,                    ///< Mark us as having no spin (ie, stop spinning).
         SpinUp,                    ///< Mark us as spinning up.
         SpinDown,                  ///< Mark us as spinning down.
         FullSpin,                  ///< Mark us as being at full spin.
         NumSpinBits = 3            ///< How many bits to allocate to representing this state. (5 states needs 3 bits)
      } spin;                       ///< Spin thread state. (Used to control spinning weapons, e.g. chainguns)
      enum RecoilState {
         NoRecoil,
         LightRecoil,
         MediumRecoil,
         HeavyRecoil,
         NumRecoilBits = 3
      } recoil;                     ///< Recoil thread state.
                                    ///
                                    /// @note At this point, the only check being done is to see if we're in a
                                    ///       state which isn't NoRecoil; ie, no differentiation is made between
                                    ///       Light/Medium/Heavy recoils. Player::onImageRecoil() is the place
                                    ///       where this is handled.
      bool flashSequence[MaxShapes];///< Is this a muzzle flash sequence?
                                    ///
                                    ///  A muzzle flash sequence is used as a source to randomly display frames from,
                                    ///  so if this is a flashSequence, we'll display a random piece each frame.
      S32 sequence[MaxShapes];      ///< Main thread sequence ID.
                                    ///
                                    ///
      S32 sequenceVis[MaxShapes];   ///< Visibility thread sequence.

      StringTableEntry shapeSequence;  ///< Sequence that is played on mounting shape
      bool shapeSequenceScale;         ///< Should the mounting shape's sequence playback be scaled
                                       ///  to the length of the state.

      const char* script;           ///< Function on datablock to call when we enter this state; passed the id of
                                    ///  the imageSlot.
      ParticleEmitterData* emitter; ///< A particle emitter; this emitter will emit as long as the gun is in this
                                    ///  this state.
      SFXTrack* sound;
      F32 emitterTime;              ///<
      S32 emitterNode[MaxShapes];   ///< Node ID on the shape to emit from
   };

   /// @name State Data
   /// Individual state data used to initialize struct array
   /// @{
   const char*             fireStateName;

   const char*             stateName                  [MaxStates];

   const char*             stateTransitionLoaded      [MaxStates];
   const char*             stateTransitionNotLoaded   [MaxStates];
   const char*             stateTransitionAmmo        [MaxStates];
   const char*             stateTransitionNoAmmo      [MaxStates];
   const char*             stateTransitionTarget      [MaxStates];
   const char*             stateTransitionNoTarget    [MaxStates];
   const char*             stateTransitionWet         [MaxStates];
   const char*             stateTransitionNotWet      [MaxStates];
   const char*             stateTransitionMotion      [MaxStates];
   const char*             stateTransitionNoMotion    [MaxStates];
   const char*             stateTransitionTriggerUp   [MaxStates];
   const char*             stateTransitionTriggerDown [MaxStates];
   const char*             stateTransitionAltTriggerUp[MaxStates];
   const char*             stateTransitionAltTriggerDown[MaxStates];
   const char*             stateTransitionGeneric0In  [MaxStates];
   const char*             stateTransitionGeneric0Out [MaxStates];
   const char*             stateTransitionGeneric1In  [MaxStates];
   const char*             stateTransitionGeneric1Out [MaxStates];
   const char*             stateTransitionGeneric2In  [MaxStates];
   const char*             stateTransitionGeneric2Out [MaxStates];
   const char*             stateTransitionGeneric3In  [MaxStates];
   const char*             stateTransitionGeneric3Out [MaxStates];
   const char*             stateTransitionTimeout     [MaxStates];
   F32                     stateTimeoutValue          [MaxStates];
   bool                    stateWaitForTimeout        [MaxStates];

   bool                    stateFire                  [MaxStates];
   bool                    stateAlternateFire         [MaxStates];
   bool                    stateReload                [MaxStates];
   bool                    stateEjectShell            [MaxStates];
   F32                     stateEnergyDrain           [MaxStates];
   bool                    stateAllowImageChange      [MaxStates];
   bool                    stateScaleAnimation        [MaxStates];
   bool                    stateScaleAnimationFP      [MaxStates];
   bool                    stateSequenceTransitionIn  [MaxStates];
   bool                    stateSequenceTransitionOut [MaxStates];
   bool                    stateSequenceNeverTransition [MaxStates];
   F32                     stateSequenceTransitionTime [MaxStates];
   bool                    stateDirection             [MaxStates];
   StateData::LoadedState  stateLoaded                [MaxStates];
   StateData::SpinState    stateSpin                  [MaxStates];
   StateData::RecoilState  stateRecoil                [MaxStates];
   const char*             stateSequence              [MaxStates];
   bool                    stateSequenceRandomFlash   [MaxStates];

   const char*             stateShapeSequence         [MaxStates];
   bool                    stateScaleShapeSequence    [MaxStates];

   bool                    stateIgnoreLoadedForReady  [MaxStates];

   SFXTrack*               stateSound                 [MaxStates];
   const char*             stateScript                [MaxStates];

   ParticleEmitterData*    stateEmitter               [MaxStates];
   F32                     stateEmitterTime           [MaxStates];
   const char*             stateEmitterNode           [MaxStates];
   /// @}
   
   /// @name Camera Shake ( while firing )
   /// @{
   bool              shakeCamera;
   VectorF           camShakeFreq;
   VectorF           camShakeAmp;
   F32               camShakeDuration;
   F32               camShakeRadius;
   F32               camShakeFalloff;
   /// @}

   /// Maximum number of sounds this image can play at a time.
   /// Any value <= 0 indicates that it can play an infinite number of sounds.
   S32 maxConcurrentSounds; 
   
   /// If true it we will allow multiple timeout transitions to occur within
   /// a single tick ( eg. they have a very small timeout ).
   bool useRemainderDT;

   //
   bool emap;                       ///< Environment mapping on?
   bool correctMuzzleVector;        ///< Adjust 1st person firing vector to eye's LOS point?
   bool correctMuzzleVectorTP;      ///< Adjust 3rd person firing vector to camera's LOS point?
   bool firstPerson;                ///< Render the image when in first person?
   bool useFirstPersonShape;        ///< Indicates the special first person shape should be used (true when shapeNameFP and useEyeOffset are defined)
   bool useEyeOffset;               ///< In first person, should we use the eyeTransform?
   bool useEyeNode;                 ///< In first person, should we attach the camera to the image's eye node?  Player still ultimately decides on what to do,
                                    ///  especially for multiple mounted images.

   bool animateAllShapes;           ///< Indicates that all shapes should be animated in sync.
   bool animateOnServer;            ///< Indicates that the image should be animated on the server.  In most cases
                                    ///  you'll want this set if you're using useEyeNode.  You may also want to
                                    ///  set this if the muzzlePoint is animated while it shoots.  You can set this
                                    ///  to false even if these previous cases are true if the image's shape is set
                                    ///  up in the correct position and orientation in the 'root' pose and none of
                                    ///  the nodes are animated at key times, such as the muzzlePoint essentially
                                    ///  remaining at the same position at the start of the fire state (it could
                                    ///  animate just fine after the projectile is away as the muzzle vector is only
                                    ///  calculated at the start of the state).  You'll also want to set this to true
                                    ///  if you're animating the camera using an image's 'eye' node -- unless the movement
                                    ///  is very subtle and doesn't need to be reflected on the server.

   F32 scriptAnimTransitionTime;    ///< The amount of time to transition between the previous sequence and new sequence
                                    ///< when the script prefix has changed.

   StringTableEntry  shapeName;     ///< Name of shape to render.
   StringTableEntry  shapeNameFP;   ///< Name of shape to render in first person (optional).

   StringTableEntry  imageAnimPrefix;     ///< Passed along to the mounting shape to modify
                                          ///  animation sequences played in 3rd person. [optional]
   StringTableEntry  imageAnimPrefixFP;   ///< Passed along to the mounting shape to modify
                                          ///  animation sequences played in first person. [optional]

   U32               mountPoint;    ///< Mount point for the image.
   MatrixF           mountOffset;   ///< Mount point offset, so we know where the image is.
   MatrixF           eyeOffset;     ///< Offset from eye for first person.

   ProjectileData* projectile;      ///< Information about what projectile this
                                    ///  image fires.

   F32   mass;                      ///< Mass!
   bool  usesEnergy;                ///< Does this use energy instead of ammo?
   F32   minEnergy;                 ///< Minimum energy for the weapon to be operable.
   bool  accuFire;                  ///< Should we automatically make image's aim converge with the crosshair?
   bool  cloakable;                 ///< Is this image cloakable when mounted?

   /// @name Lighting
   /// @{
   S32         lightType;           ///< Indicates the type of the light.
                                    ///
                                    ///  One of: ConstantLight, PulsingLight, WeaponFireLight.
   ColorF      lightColor;
   S32         lightDuration;       ///< The duration in SimTime of Pulsing or WeaponFire type lights.
   F32         lightRadius;         ///< Extent of light.
   F32         lightBrightness;     ///< Brightness of the light ( if it is WeaponFireLight ).
   /// @}

   /// @name Shape Data
   /// @{
   Resource<TSShape> shape[MaxShapes]; ///< Shape handle
   bool shapeIsValid[MaxShapes];       ///< Indicates that the shape has been loaded and is valid

   U32 mCRC[MaxShapes];                ///< Checksum of shape.
                                       ///
                                       ///  Calculated by the ResourceManager, see
                                       ///  ResourceManager::load().
   bool computeCRC;                    ///< Should the shape's CRC be checked?
   MatrixF mountTransform[MaxShapes];  ///< Transformation to get to the mountNode.
   /// @}

   /// @name Nodes
   /// @{
   S32 retractNode[MaxShapes];   ///< Retraction node ID.
                                 ///
                                 ///  When the player bumps against an object and the image is retracted to
                                 ///  avoid having it interpenetrating the object, it is retracted towards
                                 ///  this node.
   S32 muzzleNode[MaxShapes];    ///< Muzzle node ID.
                                 ///
                                 ///
   S32 ejectNode[MaxShapes];     ///< Ejection node ID.
                                 ///
                                 ///  The eject node is the node on the image from which shells are ejected.
   S32 emitterNode[MaxShapes];   ///< Emitter node ID.
                                 ///
                                 ///  The emitter node is the node from which particles are emitted.
   S32 eyeMountNode[MaxShapes];  ///< eyeMount node ID.  Optionally used to mount an image to the player's
                                 /// eye node for first person.
   S32 eyeNode[MaxShapes];       ///< Eye node ID.  Optionally used to attach the camera to for camera motion
                                 ///  control from the image.
   /// @}

   /// @name Animation
   /// @{
   S32 spinSequence[MaxShapes];     ///< ID of the spin animation sequence.
   S32 ambientSequence[MaxShapes];  ///< ID of the ambient animation sequence.

   bool isAnimated[MaxShapes];      ///< This image contains at least one animated states
   bool hasFlash[MaxShapes];        ///< This image contains at least one muzzle flash animation state
   S32 fireState;                   ///< The ID of the fire state.
   S32 altFireState;                ///< The ID of the alternate fire state.
   S32 reloadState;                 ///< The ID of the reload state
   /// @}

   /// @name Shell casing data
   /// @{
   DebrisData *   casing;              ///< Information about shell casings.

   S32            casingID;            ///< ID of casing datablock.
                                       ///
                                       ///  When the network tells the client about the casing, it
                                       ///  it transmits the ID of the datablock. The datablocks
                                       ///  having previously been transmitted, all the client
                                       ///  needs to do is call Sim::findObject() and look up the
                                       ///  the datablock.

   Point3F        shellExitDir;        ///< Vector along which to eject shells from the image.
   F32            shellExitVariance;   ///< Variance from this vector in degrees.
   F32            shellVelocity;       ///< Velocity with which to eject shell casings.
   /// @}

   /// @name State Array
   ///
   /// State array is initialized onAdd from the individual state
   /// struct array elements.
   ///
   /// @{
   StateData state[MaxStates];   ///< Array of states.
   bool      statesLoaded;       ///< Are the states loaded yet?
   /// @}

   /// @name Infrastructure
   ///
   /// Miscellaneous inherited methods.
   /// @{

   DECLARE_CONOBJECT(ShapeBaseImageData);
   ShapeBaseImageData();
   ~ShapeBaseImageData();
   bool onAdd();
   bool preload(bool server, String &errorStr);
   S32 lookupState(const char* name);  ///< Get a state by name.
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   
   void inspectPostApply();

   /// @}

   /// @name Callbacks
   /// @{
   DECLARE_CALLBACK( void, onMount, ( SceneObject* obj, S32 slot, F32 dt ) );
   DECLARE_CALLBACK( void, onUnmount, ( SceneObject* obj, S32 slot, F32 dt ) );
   /// @}
};

typedef ShapeBaseImageData::LightType ShapeBaseImageLightType;
typedef ShapeBaseImageData::StateData::LoadedState ShapeBaseImageLoadedState;
typedef ShapeBaseImageData::StateData::SpinState ShapeBaseImageSpinState;
typedef ShapeBaseImageData::StateData::RecoilState ShapeBaseImageRecoilState;

DefineEnumType( ShapeBaseImageLightType );
DefineEnumType( ShapeBaseImageLoadedState );
DefineEnumType( ShapeBaseImageSpinState );
DefineEnumType( ShapeBaseImageRecoilState );

//--------------------------------------------------------------------------
/// @nosubgrouping
struct ShapeBaseData : public GameBaseData {
  private:
   typedef GameBaseData Parent;
   
   static bool _setMass( void* object, const char* index, const char* data );

public:
   /// Various constants relating to the ShapeBaseData
   enum Constants {
      MaxCollisionShapes = 8,
      AIRepairNode = 31
   };

   // TODO: These are only really used in Basic Lighting
   // mode... we should probably move them somewhere else.
   bool shadowEnable;
   U32 shadowSize;
   F32 shadowMaxVisibleDistance;
   F32 shadowProjectionDistance;
   F32 shadowSphereAdjust;


   StringTableEntry  shapeName;
   StringTableEntry  cloakTexName;

   String cubeDescName;
   U32 cubeDescId;
   ReflectorDesc *reflectorDesc;

   /// @name Destruction
   ///
   /// Everyone likes to blow things up!
   /// @{
   DebrisData *      debris;
   S32               debrisID;
   StringTableEntry  debrisShapeName;
   Resource<TSShape> debrisShape;

   ExplosionData*    explosion;
   S32               explosionID;

   ExplosionData*    underwaterExplosion;
   S32               underwaterExplosionID;
   /// @}

   /// @name Physical Properties
   /// @{
   F32 mass;
   F32 drag;
   F32 density;
   F32 maxEnergy;
   F32 maxDamage;
   F32 repairRate;                  ///< Rate per tick.

   F32 disabledLevel;
   F32 destroyedLevel;
   /// @}

   /// @name 3rd Person Camera
   /// @{
   F32 cameraMaxDist;               ///< Maximum distance from eye
   F32 cameraMinDist;               ///< Minumumistance from eye
   /// @}

   /// @name Camera FOV
   ///
   /// These are specified in degrees.
   /// @{
   F32 cameraDefaultFov;            ///< Default vertical FOV in degrees.
   F32 cameraMinFov;                ///< Min vertical FOV allowed in degrees.
   F32 cameraMaxFov;                ///< Max vertical FOV allowed in degrees.
   /// @}

   /// @name Camera Misc
   /// @{
   bool cameraCanBank;              ///< If the derrived class supports it, allow the camera to bank
   bool mountedImagesBank;          ///< Do mounted images bank along with the camera?
   /// @}

   /// @name Data initialized on preload
   /// @{

   Resource<TSShape> mShape;         ///< Shape handle
   U32 mCRC;
   bool computeCRC;

   S32 eyeNode;                         ///< Shape's eye node index
   S32 earNode;                         ///< Shape's ear node index
   S32 cameraNode;                      ///< Shape's camera node index
   S32 mountPointNode[SceneObject::NumMountPoints];  ///< Node index of mountPoint
   S32 debrisDetail;                    ///< Detail level used to generate debris
   S32 damageSequence;                  ///< Damage level decals
   S32 hulkSequence;                    ///< Destroyed hulk

   bool              observeThroughObject;   // observe this object through its camera transform and default fov

   /// @name Collision Data
   /// @{
   Vector<S32>   collisionDetails;  ///< Detail level used to collide with.
                                    ///
                                    /// These are detail IDs, see TSShape::findDetail()
   Vector<Box3F> collisionBounds;   ///< Detail level bounding boxes.

   Vector<S32>   LOSDetails;        ///< Detail level used to perform line-of-sight queries against.
                                    ///
                                    /// These are detail IDs, see TSShape::findDetail()
   /// @}

   /// @name Misc. Settings
   /// @{
   bool firstPersonOnly;            ///< Do we allow only first person view of this image?
   bool useEyePoint;                ///< Do we use this object's eye point to view from?
   bool isInvincible;               ///< If set, object cannot take damage (won't show up with damage bar either)
   bool renderWhenDestroyed;        ///< If set, will not render this object when destroyed.

   bool inheritEnergyFromMount;

   /// @}

   virtual bool preload(bool server, String &errorStr);
   void computeAccelerator(U32 i);
   S32  findMountPoint(U32 n);

   /// @name Infrastructure
   /// The derived class should provide the following:
   /// @{
   DECLARE_CONOBJECT(ShapeBaseData);
   ShapeBaseData();
   ~ShapeBaseData();
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   /// @}

   /// @name Callbacks
   /// @{
   DECLARE_CALLBACK( void, onEnabled, ( ShapeBase* obj, const char* lastState ) );
   DECLARE_CALLBACK( void, onDisabled, ( ShapeBase* obj, const char* lastState ) );
   DECLARE_CALLBACK( void, onDestroyed, ( ShapeBase* obj, const char* lastState ) );
   DECLARE_CALLBACK( void, onImpact, ( ShapeBase* obj, SceneObject* collObj, VectorF vec, F32 len ) );
   DECLARE_CALLBACK( void, onCollision, ( ShapeBase* obj, SceneObject* collObj, VectorF vec, F32 len ) );
   DECLARE_CALLBACK( void, onDamage, ( ShapeBase* obj, F32 delta ) );
   DECLARE_CALLBACK( void, onTrigger, ( ShapeBase* obj, S32 index, bool state ) );
   DECLARE_CALLBACK(void, onEndSequence, (ShapeBase* obj, S32 slot, const char* name));
   DECLARE_CALLBACK( void, onForceUncloak, ( ShapeBase* obj, const char* reason ) );
   /// @}
};


//----------------------------------------------------------------------------

class WaterObject;
class CameraShake;

/// ShapeBase is the renderable shape from which most of the scriptable objects
/// are derived, including the player, vehicle and items classes.  ShapeBase
/// provides basic shape loading, audio channels, and animation as well as damage
/// (and damage states), energy, and the ability to mount images and objects.
///
/// @nosubgrouping
class ShapeBase : public GameBase, public ISceneLight
{
   friend class ShapeBaseConvex;
   friend struct ShapeBaseImageData;
   friend void waterFind(SceneObject*, void*);
   friend void physicalZoneFind(SceneObject*, void*);

public:
   typedef GameBase Parent;

   enum PublicConstants {
      ThreadSequenceBits = 6,
      MaxSequenceIndex = (1 << ThreadSequenceBits) - 1,
      EnergyLevelBits = 5,
      DamageLevelBits = 6,
      DamageStateBits = 2,
      // The thread and image limits should not be changed without
      // also changing the ShapeBaseMasks enum values declared
      // further down.
      MaxSoundThreads =  4,            ///< Should be a power of 2
      MaxScriptThreads = 4,            ///< Should be a power of 2
      MaxMountedImages = 4,            ///< Should be a power of 2
      MaxImageEmitters = 3,
      NumImageBits = 3,
      CollisionTimeoutValue = 250      ///< Timeout in ms.
   };

   /// This enum indexes into the sDamageStateName array
   enum DamageState {
      Enabled,
      Disabled,
      Destroyed,
      NumDamageStates,
      NumDamageStateBits = 2,   ///< Should be log2 of the number of states.
   };

protected:
   ShapeBaseData*    mDataBlock;                ///< Datablock
   bool              mIsAiControlled;           ///< Is this object AI controlled?
   //GameConnection*   mControllingClient;        ///< Controlling client
   ShapeBase*        mControllingObject;        ///< Controlling object
   bool              mTrigger[MaxTriggerKeys];  ///< What triggers are set, if any.


   /// @name Scripted Sound
   /// @{
   struct Sound {
      bool play;                    ///< Are we playing this sound?
      SimTime timeout;              ///< Time until we stop playing this sound.
      SFXTrack* profile;            ///< Profile on server
      SFXSource* sound;             ///< Sound on client
   };
   Sound mSoundThread[MaxSoundThreads];
   /// @}

   /// @name Scripted Animation Threads
   /// @{

   struct Thread {
      /// State of the animation thread.
      enum State {
         Play, Stop, Pause, Destroy
      };
      TSThread* thread; ///< Pointer to 3space data.
      State state;      ///< State of the thread
      S32 sequence;     ///< The animation sequence which is running in this thread.
      F32 timescale;    ///< Timescale
      bool atEnd;       ///< Are we at the end of this thread?
      F32 position;
   };
   Thread mScriptThread[MaxScriptThreads];

   /// @}

   /// @name Motion
   /// @{
   bool mMoveMotion;    ///< Indicates that a Move has come in requesting x, y or z motion
   /// @}

protected:

   // ShapeBase pointer to our mount object if it is ShapeBase, else it is NULL.
   ShapeBase *mShapeBaseMount;

   /// @name Mounted Images
   /// @{

   /// An image mounted on a shapebase.
   struct MountedImage {
      ShapeBaseImageData* dataBlock;
      ShapeBaseImageData::StateData *state;
      ShapeBaseImageData* nextImage;
      NetStringHandle skinNameHandle;
      NetStringHandle nextSkinNameHandle;
      String appliedSkinName;
      NetStringHandle scriptAnimPrefix;      ///< The script based anim prefix

      /// @name State
      ///
      /// Variables tracking the state machine
      /// representing this specific mounted image.
      /// @{

      bool loaded;                  ///< Is the image loaded?
      bool nextLoaded;              ///< Is the next state going to result in the image being loaded?
      F32 delayTime;                ///< Time till next state.
      F32 rDT;                      ///< Remainder delta time. Used internally.
      U32 fireCount;                ///< Fire skip count.
                                    ///
                                    /// This is incremented every time the triggerDown bit is changed,
                                    /// so that the engine won't be too confused if the player toggles the
                                    /// trigger a bunch of times in a short period.
                                    ///
                                    /// @note The network deals with this variable at 3-bit precision, so it
                                    /// can only range 0-7.
                                    ///
                                    /// @see ShapeBase::setImageState()
      U32 altFireCount;             ///< Alternate fire skip count.
                                    ///< @see fireCount

      U32 reloadCount;              ///< Reload skip count.
                                    ///< @see fireCount

      bool triggerDown;             ///< Is the trigger down?
      bool altTriggerDown;          ///< Is the second trigger down?

      bool ammo;                    ///< Do we have ammo?
                                    ///
                                    /// May be true based on either energy OR ammo.

      bool target;                  ///< Have we acquired a targer?
      bool wet;                     ///< Is the weapon wet?

      bool motion;                  ///< Is the player in motion?

      bool genericTrigger[ShapeBaseImageData::MaxGenericTriggers];   ///< Generic triggers not assigned to anything in particular.  These
                                                                     ///  may be used to indicate some transition should occur.
      /// @}

      /// @name 3space
      ///
      /// Handles to threads and shapeinstances in the 3space system.
      /// @{
      TSShapeInstance* shapeInstance[ShapeBaseImageData::MaxShapes];
      TSThread *ambientThread[ShapeBaseImageData::MaxShapes];
      TSThread *visThread[ShapeBaseImageData::MaxShapes];
      TSThread *animThread[ShapeBaseImageData::MaxShapes];
      TSThread *flashThread[ShapeBaseImageData::MaxShapes];
      TSThread *spinThread[ShapeBaseImageData::MaxShapes];

      bool doAnimateAllShapes;      ///< Should all threads animate across all shapes to keep them in sync?
      bool forceAnimateAllShapes;   ///< If the mounted image's owner is being controlled by the client
                                    ///  and the image's datablock animateAllShapes field is true
                                    ///  then set this to true and pass along to the client.  This will help
                                    ///  in the cases where the client's control object is ghosted but does
                                    ///  not yet have its controlling client set correctly due to networking
                                    ///  order of operations.  All this for the MountedImage::updateDoAnimateAllShapes()
                                    ///  optimization.
      U32 lastShapeIndex;           ///< Tracks the last shape index.
      /// @}

      /// @name Effects
      ///
      /// Variables relating to lights, sounds, and particles.
      /// @{
      SimTime lightStart;     ///< Starting time for light flashes.
      LightInfo* lightInfo;   ///< The real light (if any) associated with this weapon image.

      Vector<SFXSource*> mSoundSources; ///< Vector of currently playing sounds
      void updateSoundSources(const MatrixF& renderTransform);  
      void addSoundSource(SFXSource* source);

      /// Represent the state of a specific particle emitter on the image.
      struct ImageEmitter {
         S32 node;
         F32 time;
         SimObjectPtr<ParticleEmitter> emitter;
      };
      ImageEmitter emitter[MaxImageEmitters];

      /// @}

      //
      MountedImage();
      ~MountedImage();

      void updateDoAnimateAllShapes(const ShapeBase* owner);
   };
   MountedImage mMountedImageList[MaxMountedImages];

   /// @}

   /// @name Render settings
   /// @{

   TSShapeInstance*  mShapeInstance;
   Convex *          mConvexList;
   NetStringHandle   mSkinNameHandle;
   String            mAppliedSkinName;

   NetStringHandle mShapeNameHandle;   ///< Name sent to client
   /// @}

   /// @name Physical Properties
   /// @{

   S32 mAiPose;                     ///< Current pose.
   F32 mEnergy;                     ///< Current enery level.
   F32 mRechargeRate;               ///< Energy recharge rate (in units/tick).

   F32 mMass;                       ///< Mass.
   F32 mOneOverMass;                ///< Inverse of mass.
                                    /// @note This is used to optimize certain physics calculations.

   /// @}

   /// @name Physical Properties
   ///
   /// Properties for the current object, which are calculated
   /// based on the container we are in.
   ///
   /// @see ShapeBase::updateContainer()
   /// @see ShapeBase::mContainer
   /// @{
   F32 mDrag;                       ///< Drag.
   F32 mBuoyancy;                   ///< Buoyancy factor.
   String mLiquidType;              ///< Type of liquid (if any) we are in.
   F32 mLiquidHeight;               ///< Height of liquid around us (from 0..1).
   F32 mWaterCoverage;              ///< Percent of this object covered by water

   Point3F mAppliedForce;
   F32 mGravityMod;
   /// @}

   F32 mDamageFlash;
   F32 mWhiteOut;

   bool mFlipFadeVal;

 public:

   /// @name Collision Notification
   /// This is used to keep us from spamming collision notifications. When
   /// a collision occurs, we add to this list; then we don't notify anyone
   /// of the collision until the CollisionTimeout expires (which by default
   /// occurs in 1/10 of a second).
   ///
   /// @see notifyCollision(), queueCollision()
   /// @{
   struct CollisionTimeout 
   {
      CollisionTimeout* next;
      SceneObject* object;
      U32 objectNumber;
      SimTime expireTime;
      VectorF vector;
   };
   CollisionTimeout* mTimeoutList;
   static CollisionTimeout* sFreeTimeoutList;

   /// Go through all the items in the collision queue and call onCollision on them all
   /// @see onCollision
   void notifyCollision();

   /// Add a collision to the queue of collisions waiting to be handled @see onCollision
   /// @param   object   Object collision occurs with
   /// @param   vec      Vector along which collision occurs
   void queueCollision( SceneObject *object, const VectorF &vec);

   /// @see SceneObject
   virtual void onCollision( SceneObject *object, const VectorF &vec );

   /// @}
 protected:

   /// @name Damage
   /// @{
   F32  mDamage;
   F32  mRepairRate;
   F32  mRepairReserve;
   DamageState mDamageState;
   TSThread *mDamageThread;
   TSThread *mHulkThread;
   VectorF damageDir;
   /// @}

   /// @name Cloaking
   /// @{
   bool mCloaked;
   F32  mCloakLevel;
//   TextureHandle mCloakTexture;
   /// @}

   /// @name Fading
   /// @{
   bool  mFadeOut;
   bool  mFading;
   F32   mFadeVal;
   F32   mFadeElapsedTime;
   F32   mFadeTime;
   F32   mFadeDelay;
public:
   F32   getFadeVal() { return mFadeVal; }
   /// @}
protected:

   /// @name Control info
   /// @{
   F32  mCameraFov;           ///< The camera vertical FOV in degrees.
   bool mIsControlled;        ///< Client side controlled flag

   /// @}
public:
   static U32 sLastRenderFrame;

protected:

   U32 mLastRenderFrame;
   F32 mLastRenderDistance;

   /// Do a reskin if necessary.
   virtual void reSkin();

   /// This recalculates the total mass of the object, and all mounted objects
   void updateMass();

   /// @name Image Manipulation
   /// @{

   /// Utility function to call script functions which have to do with ShapeBase
   /// objects.
   /// @param   imageSlot  Image Slot id
   /// @param   function   Function
   void scriptCallback(U32 imageSlot,const char* function);

   /// Assign a ShapeBaseImage to an image slot
   /// @param   imageSlot   Image Slot ID
   /// @param   imageData   ShapeBaseImageData to assign
   /// @param   skinNameHandle Skin texture name
   /// @param   loaded      Is the image loaded?
   /// @param   ammo        Does the image have ammo?
   /// @param   triggerDown Is the trigger on this image down?
   /// @param   altTriggerDown Is the second trigger on this image down?
   /// @param   target      Does the image have a target?
   virtual void setImage(  U32 imageSlot, 
                           ShapeBaseImageData* imageData, 
                           NetStringHandle &skinNameHandle,
                           bool loaded = true, bool ammo = false, 
                           bool triggerDown = false,
                           bool altTriggerDown = false,
                           bool motion = false,
                           bool genericTrigger0 = false,
                           bool genericTrigger1 = false,
                           bool genericTrigger2 = false,
                           bool genericTrigger3 = false,
                           bool target = false );

   /// Clear out an image slot
   /// @param   imageSlot   Image slot id
   void resetImageSlot(U32 imageSlot);

   /// Get the firing action state of the image
   /// @param   imageSlot   Image slot id
   U32  getImageFireState(U32 imageSlot);

   /// Get the alternate firing action state of the image
   /// @param   imageSlot   Image slot id
   U32  getImageAltFireState(U32 imageSlot);

   /// Get the reload action state of the image
   /// @param   imageSlot   Image slot id
   U32  getImageReloadState(U32 imageSlot);

   /// Sets the state of the image by state index
   /// @param   imageSlot   Image slot id
   /// @param   state       State id
   /// @param   force       Force image to state or let it finish then change
   void setImageState(U32 imageSlot, U32 state, bool force = false);

   void updateAnimThread(U32 imageSlot, S32 imageShapeIndex, ShapeBaseImageData::StateData* lastState=NULL);

   /// Get the animation prefix for the image
   /// @param   imageSlot        Image slot id
   /// @param   imageShapeIndex  Shape index (1st person, etc.) used to look up the prefix text
   virtual const char* getImageAnimPrefix(U32 imageSlot, S32 imageShapeIndex) { return ""; }

   /// Advance animation on a image
   /// @param   imageSlot   Image slot id
   /// @param   dt          Change in time since last animation update
   void updateImageAnimation(U32 imageSlot, F32 dt);

   /// Advance state of image
   /// @param   imageSlot   Image slot id
   /// @param   dt          Change in time since last state update
   void updateImageState(U32 imageSlot,F32 dt);

   /// Start up the particle emitter for the this shapebase
   /// @param   image   Mounted image
   /// @param   state   State of shape base image
   void startImageEmitter(MountedImage &image,ShapeBaseImageData::StateData &state);

   /// Get light information for a mounted image
   /// @param   imageSlot   Image slot id
   Light* getImageLight(U32 imageSlot);

   /// Get the shape index to use for a mounted image
   /// @param   image   Mounted image
   U32 getImageShapeIndex(const MountedImage& image) const;

   /// @}

   /// Prune out non looping sounds from the sound manager which have expired
   void updateServerAudio();

   /// Updates the audio state of the supplied sound
   /// @param   st   Sound
   void updateAudioState(Sound& st);

   /// Recalculates the spacial sound based on the current position of the object
   /// emitting the sound.
   void updateAudioPos();

   /// Update bouyency and drag properties
   void updateContainer();

   /// @name Events
   /// @{
   virtual void onDeleteNotify(SimObject*);
   virtual void onImage(U32 imageSlot, bool unmount);
   virtual void onImageRecoil(U32 imageSlot,ShapeBaseImageData::StateData::RecoilState);
   virtual void onImageStateAnimation(U32 imageSlot, const char* seqName, bool direction, bool scaleToState, F32 stateTimeOutValue);
   virtual void onImageAnimThreadChange(U32 imageSlot, S32 imageShapeIndex, ShapeBaseImageData::StateData* lastState, const char* anim, F32 pos, F32 timeScale, bool reset=false);
   virtual void onImageAnimThreadUpdate(U32 imageSlot, S32 imageShapeIndex, F32 dt);
   virtual void ejectShellCasing( U32 imageSlot );
   virtual void shakeCamera( U32 imageSlot );
   virtual void updateDamageLevel();
   virtual void updateDamageState();
   virtual void onImpact(SceneObject* obj, const VectorF& vec);
   virtual void onImpact(const VectorF& vec);
   /// @}

   /// The inner prep render function that does the 
   /// standard work to render the shapes.
   void _prepRenderImage(  SceneRenderState* state, 
                           bool renderSelf, 
                           bool renderMountedImages );

   /// Renders the shape bounds as well as the 
   /// bounds of all mounted shape images.
   void _renderBoundingBox( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* );

   void emitDust( ParticleEmitter* emitter, F32 triggerHeight, const Point3F& offset, U32 numMilliseconds, const Point3F& axis = Point3F::Zero );

public:
   ShapeBase();
   ~ShapeBase();

   TSShapeInstance* getShapeInstance() { return mShapeInstance; }

   static void initPersistFields();
   static bool _setFieldSkin( void *object, const char *index, const char *data );
   static const char *_getFieldSkin( void *object, const char *data );

   /// @name Network state masks
   /// @{

   ///
   enum ShapeBaseMasks {
      NameMask        = Parent::NextFreeMask,
      DamageMask      = Parent::NextFreeMask << 1,
      NoWarpMask      = Parent::NextFreeMask << 2,
      CloakMask       = Parent::NextFreeMask << 3,
      SkinMask        = Parent::NextFreeMask << 4,
      MeshHiddenMask  = Parent::NextFreeMask << 5,
      SoundMaskN      = Parent::NextFreeMask << 6,       ///< Extends + MaxSoundThreads bits
      ThreadMaskN     = SoundMaskN  << MaxSoundThreads,  ///< Extends + MaxScriptThreads bits
      ImageMaskN      = ThreadMaskN << MaxScriptThreads, ///< Extends + MaxMountedImage bits
      NextFreeMask    = ImageMaskN  << MaxMountedImages
   };

   enum BaseMaskConstants {
      SoundMask      = (SoundMaskN << MaxSoundThreads) - SoundMaskN,
      ThreadMask     = (ThreadMaskN << MaxScriptThreads) - ThreadMaskN,
      ImageMask      = (ImageMaskN << MaxMountedImages) - ImageMaskN
   };

   /// @}

   static F32  sWhiteoutDec;
   static F32  sDamageFlashDec;
   static F32  sFullCorrectionDistance;
   static F32  sCloakSpeed;               // Time to cloak, in seconds
      
   CubeReflector mCubeReflector;

   /// @name Initialization
   /// @{

   bool onAdd();
   void onRemove();
   void onSceneRemove();
   static void consoleInit();
   bool onNewDataBlock( GameBaseData *dptr, bool reload );

   /// @}

   /// @name Name & Skin tags
   /// @{
   void setShapeName(const char*);
   const char* getShapeName();
   void setSkinName(const char*);
   const char* getSkinName();
   /// @}

   /// @name Mesh Visibility
   /// @{
   
protected:

   /// A bit vector of the meshes forced to be hidden.
   BitVector mMeshHidden;

   /// Sync the shape instance with the hidden mesh bit vector.
   void _updateHiddenMeshes();               

public:

   /// Change the hidden state on all the meshes.
   void setAllMeshesHidden( bool forceHidden );  

   /// Set the force hidden state on a mesh.
   void setMeshHidden( S32 meshIndex, bool forceHidden ); 
                        
   /// Set the force hidden state on a named mesh.
   void setMeshHidden( const char *meshName, bool forceHidden ); 
   
#ifndef TORQUE_SHIPPING

   /// Prints the list of meshes and their visibility state
   /// to the console for debugging purposes.
   void dumpMeshVisibility();
                      
#endif   

   /// @}

public:

   /// @name Basic attributes
   /// @{

   /// Sets the amount of damage on this object.
   void setDamageLevel(F32 damage);

   /// Changes the object's damage state.
   /// @param   state   New state of the object
   void setDamageState(DamageState state);

   /// Changes the object's damage state, based on a named state.
   /// @see setDamageState
   /// @param   state   New state of the object as a string.
   bool setDamageState(const char* state);

   /// Returns the name of the current damage state as a string.
   const char* getDamageStateName();

   /// Returns the current damage state.
   DamageState getDamageState() { return mDamageState; }

   /// Returns true if the object is destroyed.
   bool isDestroyed() { return mDamageState == Destroyed; }

   /// Sets the rate at which the object regenerates damage.
   ///
   /// @param  rate  Repair rate in units/second.
   void setRepairRate(F32 rate) { mRepairRate = rate; }

   /// Returns damage amount.
   F32  getDamageLevel()  { return mDamage; }

   /// Returns the damage percentage.
   ///
   /// @return Damage factor, between 0.0 - 1.0
   F32  getDamageValue();
 
   /// Returns the datablock.maxDamage value  
   F32 getMaxDamage(); 

   /// Returns the rate at which the object regenerates damage
   F32  getRepairRate() { return mRepairRate; }

   /// Adds damage to an object
   /// @param   amount   Amount of of damage to add
   void applyDamage(F32 amount);

   /// Removes damage to an object
   /// @param   amount   Amount to repair object by
   void applyRepair(F32 amount);

   /// Sets the direction from which the damage is coming
   /// @param   vec   Vector indicating the direction of the damage
   void setDamageDir(const VectorF& vec)  { damageDir = vec; }

   /// Sets the level of energy for this object
   /// @param   energy   Level of energy to assign to this object
   virtual void setEnergyLevel(F32 energy);

   /// Sets the rate at which the energy replentishes itself
   /// @param   rate   Rate at which energy restores
   void setRechargeRate(F32 rate) { mRechargeRate = rate; }

   /// Returns the amount of energy in the object
   F32  getEnergyLevel();

   /// Returns the percentage of energy, 0.0 - 1.0
   F32  getEnergyValue();

   /// Returns the recharge rate
   F32  getRechargeRate() { return mRechargeRate; }

   /// Makes the shape explode.
   virtual void blowUp();

   /// @}

   /// @name Script sounds
   /// @{

   /// Plays an audio sound from a mounted object
   /// @param   slot    Mount slot ID
   /// @param   track   Audio track to play
   void playAudio(U32 slot,SFXTrack* track);
   void playAudio( U32 slot, SFXProfile* profile ) { playAudio( slot, ( SFXTrack* ) profile ); }

   /// Stops audio from a mounted object
   /// @param   slot   Mount slot ID
   void stopAudio(U32 slot);
   /// @}

   /// @name Script animation
   /// @{

   const char *getThreadSequenceName( U32 slot );

   /// Sets the animation thread for a mounted object
   /// @param   slot   Mount slot ID
   /// @param    seq   Sequence id
   /// @param   reset   Reset the sequence
   bool setThreadSequence(U32 slot, S32 seq, bool reset = true);

   /// Update the animation thread
   /// @param   st   Thread to update
   void updateThread(Thread& st);

   /// Stop the current thread from playing on a mounted object
   /// @param   slot   Mount slot ID
   bool stopThread(U32 slot);

   /// Destroys the given animation thread
   /// @param   slot   Mount slot ID
   bool destroyThread(U32 slot);

   /// Pause the running animation thread
   /// @param   slot   Mount slot ID
   bool pauseThread(U32 slot);

   /// Start playing the running animation thread again
   /// @param   slot   Mount slot ID
   bool playThread(U32 slot);

   /// Set the thread position
   /// @param   slot   Mount slot ID
   /// @param   pos    Position
   bool setThreadPosition( U32 slot, F32 pos );

   /// Toggle the thread as reversed or normal (For example, sidestep-right reversed is sidestep-left)
   /// @param   slot   Mount slot ID
   /// @param   forward   True if the animation is to be played normally
   bool setThreadDir(U32 slot,bool forward);

   /// Set the thread time scale
   /// @param   slot   Mount slot ID
   /// @param   timescale   Timescale
   bool setThreadTimeScale( U32 slot, F32 timeScale );

   /// Advance all animation threads attached to this shapebase
   /// @param   dt   Change in time from last call to this function
   void advanceThreads(F32 dt);
   /// @}

   /// @name Cloaking
   /// @{

   /// Force uncloaking of object
   /// @param   reason   Reason this is being forced to uncloak, this is passed directly to script control
   void forceUncloak(const char *reason);

   /// Set cloaked state of object
   /// @param   cloaked   True if object is cloaked
   void setCloakedState(bool cloaked);

   /// Returns true if object is cloaked
   bool getCloakedState();

   /// Returns level of cloaking, as it's not an instant "now you see it, now you don't"
   F32 getCloakLevel();
   /// @}

   /// @name Mounted objects
   /// @{   
   virtual void onMount( SceneObject *obj, S32 node );   
   virtual void onUnmount( SceneObject *obj,S32 node );   
   virtual void getMountTransform( S32 index, const MatrixF &xfm, MatrixF *outMat );
   virtual void getRenderMountTransform( F32 delta, S32 index, const MatrixF &xfm, MatrixF *outMat );
   /// @}

   /// Returns where the AI should be to repair this object
   ///
   /// @note Legacy code from Tribes 2, but still works
   Point3F getAIRepairPoint();

   /// @name Mounted Images
   /// @{

   /// Mount an image (ShapeBaseImage) onto an image slot
   /// @param   image   ShapeBaseImage to mount
   /// @param   imageSlot Image mount point
   /// @param   loaded    True if weapon is loaded (it assumes it's a weapon)
   /// @param   skinNameHandle   Skin name for object
   virtual bool mountImage(ShapeBaseImageData* image,U32 imageSlot,bool loaded, NetStringHandle &skinNameHandle);

   /// Unmount an image from a slot
   /// @param   imageSlot   Mount point
   virtual bool unmountImage(U32 imageSlot);

   /// Gets the information on the image mounted in a slot
   /// @param   imageSlot   Mount point
   ShapeBaseImageData* getMountedImage(U32 imageSlot);

   /// Gets the mounted image on on a slot
   /// @param   imageSlot   Mount Point
   MountedImage* getImageStruct(U32 imageSlot);

   TSShapeInstance* getImageShapeInstance(U32 imageSlot)
   {
      const MountedImage &image = mMountedImageList[imageSlot];
      U32 imageShapeIndex = getImageShapeIndex(image);
      if(image.dataBlock && image.shapeInstance[imageShapeIndex])
         return image.shapeInstance[imageShapeIndex];
      return NULL;
   }

   /// Gets the next image which will be put in an image slot
   /// @see setImageState
   /// @param   imageSlot   mount Point
   ShapeBaseImageData* getPendingImage(U32 imageSlot);


   /// Returns true if the mounted image is firing
   /// @param   imageSlot   Mountpoint
   bool isImageFiring(U32 imageSlot);

   /// Returns true if the mounted image is alternate firing
   /// @param   imageSlot   Mountpoint
   bool isImageAltFiring(U32 imageSlot);

   /// Returns true if the mounted image is reloading
   /// @param   imageSlot   Mountpoint
   bool isImageReloading(U32 imageSlot);

   /// This will return true if, when triggered, the object will fire.
   /// @param   imageSlot   mount point
   /// @param   ns          Used internally for recursion, do not mess with
   /// @param   depth       Used internally for recursion, do not mess with
   bool isImageReady(U32 imageSlot,U32 ns = (U32)-1,U32 depth = 0);

   /// Returns true if the specified image is mounted
   /// @param   image   ShapeBase image
   bool isImageMounted(ShapeBaseImageData* image);

   /// Returns the slot which the image specified is mounted on
   /// @param   image   Image to test for
   S32 getMountSlot(ShapeBaseImageData* image);

   /// Returns the skin for the image in a slot
   /// @param   imageSlot   Image slot to get the skin from
   NetStringHandle getImageSkinTag(U32 imageSlot);

   /// Check if the given state exists on the mounted Image
   /// @param   imageSlot   Image slot id
   /// @param   state       Image state to check for
   bool hasImageState(U32 imageSlot, const char* state);

   /// Returns the image state as a string
   /// @param   imageSlot   Image slot to check state
   const char* getImageState(U32 imageSlot);

   /// Sets the generic trigger state of the image
   /// @param   imageSlot   Image slot
   /// @param   trigger     Generic trigger number 0-3
   /// @param   state       True if generic trigger is down
   void setImageGenericTriggerState(U32 imageSlot, U32 trigger, bool state);

   /// Returns the generic trigger state of the image
   /// @param   imageSlot   Image slot
   /// @param   trigger     Generic trigger number 0-3
   bool getImageGenericTriggerState(U32 imageSlot, U32 trigger);

   /// Sets the trigger state of the image (Ie trigger pulled down on gun)
   /// @param   imageSlot   Image slot
   /// @param   trigger     True if trigger is down
   void setImageTriggerState(U32 imageSlot,bool trigger);

   /// Returns the trigger state of the image
   /// @param   imageSlot   Image slot
   bool getImageTriggerState(U32 imageSlot);

   /// Sets the alt trigger state of the image (Ie trigger pulled down on gun)
   /// @param   imageSlot   Image slot
   /// @param   trigger     True if trigger is down
   void setImageAltTriggerState( U32 imageSlot, bool trigger );

   /// Returns the alt trigger state of the image
   /// @param   imageSlot   Image slot
   bool getImageAltTriggerState( U32 imageSlot );

   /// Sets the flag if the image uses ammo or energy
   /// @param   imageSlot   Image slot
   /// @param   ammo        True if the weapon uses ammo, not energy
   void setImageAmmoState(U32 imageSlot,bool ammo);

   /// Returns true if the image uses ammo, not energy
   /// @param   imageSlot   Image slot
   bool getImageAmmoState(U32 imageSlot);

   /// Sets the image as wet or not, IE if you wanted a gun not to function underwater
   /// @param   imageSlot   Image slot
   /// @param   wet         True if image is wet
   void setImageWetState(U32 imageSlot,bool wet);

   /// Returns true if image is wet
   /// @param   imageSlot   image slot
   bool getImageWetState(U32 imageSlot);

   /// Sets the image as in motion or not, IE if you wanted a gun not to sway while the player is in motion
   /// @param   imageSlot   Image slot
   /// @param   motion     True if image is in motion
   void setImageMotionState(U32 imageSlot,bool motion);

   /// Returns true if image is in motion
   /// @param   imageSlot   image slot
   bool getImageMotionState(U32 imageSlot);

   /// Sets the flag if the image has a target
   /// @param   imageSlot   Image slot
   /// @param   ammo        True if the weapon has a target
   void setImageTargetState(U32 imageSlot,bool ammo);

   /// Returns true if the image has a target
   /// @param   imageSlot   Image slot
   bool getImageTargetState(U32 imageSlot);

   /// Sets the flag of if the image is loaded with ammo
   /// @param   imageSlot   Image slot
   /// @param   loaded      True if object is loaded with ammo
   void setImageLoadedState(U32 imageSlot,bool loaded);

   /// Returns true if object is loaded with ammo
   /// @param   imageSlot   Image slot
   bool getImageLoadedState(U32 imageSlot);

   /// Set the script animation prefix for the image
   /// @param   imageSlot        Image slot id
   /// @param   prefix           The prefix applied to the image
   void setImageScriptAnimPrefix(U32 imageSlot, NetStringHandle prefix);

   /// Get the script animation prefix for the image
   /// @param   imageSlot        Image slot id
   /// @param   imageShapeIndex  Shape index (1st person, etc.) used to look up the prefix text
   NetStringHandle getImageScriptAnimPrefix(U32 imageSlot);

   /// Modify muzzle, if needed, to aim at whatever is straight in front of eye.
   /// Returns true if result is actually modified.
   /// @param   muzMat   Muzzle transform (in/out)
   /// @param   result   Corrected muzzle vector (out)
   bool getCorrectedAim(const MatrixF& muzMat, VectorF* result);

   /// Gets the muzzle vector of a specified slot
   /// @param   imageSlot   Image slot to check transform for
   /// @param   vec   Muzzle vector (out)
   virtual void getMuzzleVector(U32 imageSlot,VectorF* vec);

   /// Gets the point of the muzzle of the image
   /// @param   imageSlot   Image slot
   /// @param   pos   Muzzle point (out)
   void getMuzzlePoint(U32 imageSlot,Point3F* pos);

   /// @}

   /// @name Transforms
   /// @{

   /// Gets the minimum viewing distance, maximum viewing distance, camera offsetand rotation
   /// for this object, if the world were to be viewed through its eyes
   /// @param   min   Minimum viewing distance
   /// @param   max   Maximum viewing distance
   /// @param   offset Offset of the camera from the origin in local space
   /// @param   rot   Rotation matrix
   virtual void getCameraParameters(F32 *min,F32* max,Point3F* offset,MatrixF* rot);

   /// Gets the camera to world space transform matrix
   /// @todo Find out what pos does
   /// @param   pos   TODO: Find out what this does
   /// @param   mat   Camera transform (out)
   virtual void getCameraTransform(F32* pos,MatrixF* mat);

   /// Gets the view transform for a particular eye, taking into account the current absolute 
   /// orient and position values of the display device.
   virtual void getEyeCameraTransform( IDisplayDevice *display, U32 eyeId, MatrixF *outMat );

   /// Gets the index of a node inside a mounted image given the name
   /// @param   imageSlot   Image slot
   /// @param   nodeName    Node name
   S32 getNodeIndex(U32 imageSlot,StringTableEntry nodeName);

   /// @}

   /// @name Object Transforms
   /// @{

   /// Returns the eye transform of this shape, IE the eyes of a player
   /// @param   mat   Eye transform (out)
   virtual void getEyeTransform(MatrixF* mat);

   /// Returns the eye transform of this shape without including mounted images, IE the eyes of a player
   /// @param   mat   Eye transform (out)
   virtual void getEyeBaseTransform(MatrixF* mat, bool includeBank);

   /// The retraction transform is the muzzle transform in world space.
   ///
   /// If the gun is pushed back, for instance, if the player ran against something,
   /// the muzzle of the gun gets pushed back towards the player, towards this location.
   /// @param   imageSlot   Image slot
   /// @param   mat   Transform (out)
   virtual void getRetractionTransform(U32 imageSlot,MatrixF* mat);

   /// Muzzle transform of mounted object in world space
   /// @param   imageSlot   Image slot
   /// @param   mat         Muzzle transform (out)
   virtual void getMuzzleTransform(U32 imageSlot,MatrixF* mat);

   /// Gets the transform of a mounted image in world space
   /// @param   imageSlot   Image slot
   /// @param   mat    Transform (out)
   virtual void getImageTransform(U32 imageSlot,MatrixF* mat);

   /// Gets the transform of a node on a mounted image in world space
   /// @param   imageSlot   Image Slot
   /// @param   node    node on image
   /// @param   mat   Transform (out)
   virtual void getImageTransform(U32 imageSlot,S32 node, MatrixF* mat);

   /// Gets the transform of a node on a mounted image in world space
   /// @param   imageSlot   Image Slot
   /// @param   nodeName    Name of node on image
   /// @param   mat         Transform (out)
   virtual void getImageTransform(U32 imageSlot, StringTableEntry nodeName, MatrixF* mat);

   ///@}

   /// @name Render transforms
   /// Render transforms are different from object transforms in that the render transform of an object
   /// is where, in world space, the object is actually rendered. The object transform is the
   /// absolute position of the object, as in where it should be.
   ///
   /// The render transforms typically vary from object transforms due to client side prediction.
   ///
   /// Other than that, these functions are identical to their object-transform counterparts
   ///
   /// @note These are meaningless on the server.
   /// @{
   virtual void getRenderRetractionTransform(U32 index,MatrixF* mat);   
   virtual void getRenderMuzzleTransform(U32 index,MatrixF* mat);   
   virtual void getRenderImageTransform(U32 imageSlot,MatrixF* mat,bool noEyeOffset=false);
   virtual void getRenderImageTransform(U32 index,S32 node, MatrixF* mat);
   virtual void getRenderImageTransform(U32 index, StringTableEntry nodeName, MatrixF* mat);
   virtual void getRenderMuzzleVector(U32 imageSlot,VectorF* vec);
   virtual void getRenderMuzzlePoint(U32 imageSlot,Point3F* pos);
   virtual void getRenderEyeTransform(MatrixF* mat);
   virtual void getRenderEyeBaseTransform(MatrixF* mat, bool includeBank);
   /// @}



   /// @name Screen Flashing
   /// @{

   /// Returns the level of screenflash that should be used
   virtual F32  getDamageFlash() const;

   /// Sets the flash level
   /// @param   amt   Level of flash
   virtual void setDamageFlash(const F32 amt);

   /// White out is the flash-grenade blindness effect
   /// This returns the level of flash to create
   virtual F32  getWhiteOut() const;

   /// Set the level of flash blindness
   virtual void setWhiteOut(const F32);
   /// @}

   /// @name Movement & velocity
   /// @{

   /// Sets the velocity of this object
   /// @param   vel   Velocity vector
   virtual void setVelocity(const VectorF& vel);

   /// Applies an impulse force to this object
   /// @param   pos   Position where impulse came from in world space
   /// @param   vec   Velocity vector (Impulse force F = m * v)
   virtual void applyImpulse(const Point3F& pos,const VectorF& vec);

   /// @}

   /// @name Cameras and Control
   /// @{

   /// Returns the object controlling this object
   ShapeBase* getControllingObject()   { return mControllingObject; }

   /// Sets the controlling object
   /// @param   obj   New controlling object
   virtual void setControllingObject(ShapeBase* obj);
   
   ///
   virtual void setControllingClient( GameConnection* connection );

   /// Returns the object this is controlling
   virtual ShapeBase* getControlObject();

   /// sets the object this is controlling
   /// @param   obj   New controlled object
   virtual void setControlObject(ShapeBase *obj);

   /// Returns true if this object is controlling by something
   bool isControlled() { return(mIsControlled); }

   /// Returns true if this object is being used as a camera in first person
   bool isFirstPerson() const;

   /// Returns true if the camera uses this objects eye point (defined by modeler)
   bool useObjsEyePoint() const;

   /// Returns true if this object can only be used as a first person camera
   bool onlyFirstPerson() const;

   /// Returns the vertical field of view in degrees for 
   /// this object if used as a camera.
   virtual F32 getCameraFov() { return mCameraFov; }

   /// Returns the default vertical field of view in degrees
   /// if this object is used as a camera.
   virtual F32 getDefaultCameraFov() { return mDataBlock->cameraDefaultFov; }

   /// Sets the vertical field of view in degrees for this 
   /// object if used as a camera.
   /// @param   yfov  The vertical FOV in degrees to test.
   virtual void setCameraFov(F32 fov);

   /// Returns true if the vertical FOV in degrees is within 
   /// allowable parameters of the datablock.
   /// @param   yfov  The vertical FOV in degrees to test.
   /// @see ShapeBaseData::cameraMinFov
   /// @see ShapeBaseData::cameraMaxFov
   virtual bool isValidCameraFov(F32 fov);
   /// @}


   void processTick(const Move *move);
   void advanceTime(F32 dt);

   /// @name Rendering
   /// @{

   /// Returns the renderable shape of this object
   TSShape const* getShape();

   /// @see SceneObject
   virtual void prepRenderImage( SceneRenderState* state );

   /// Used from ShapeBase::_prepRenderImage() to submit render 
   /// instances for the main shape or its mounted elements.
   virtual void prepBatchRender( SceneRenderState *state, S32 mountedImageIndex );

   /// Preprender logic
   virtual void calcClassRenderData() { }

   /// Virtualize this so other classes may override it for custom reasons.
   virtual void renderMountedImage( U32 imageSlot, TSRenderState &rstate, SceneRenderState *state );
   /// @}

   /// Control object scoping
   void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *camInfo);

   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo* info);
   bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF& sphere);
   void buildConvex(const Box3F& box, Convex* convex);

   /// @name Rendering
   /// @{

   /// Increments the last rendered frame number
   static void incRenderFrame()    { sLastRenderFrame++; }

   /// Returns true if the last frame calculated rendered
   bool didRenderLastRender() { return mLastRenderFrame == sLastRenderFrame; }

   /// Sets the state of this object as hidden or not. If an object is hidden
   /// it is removed entirely from collisions, it is not ghosted and is
   /// essentially "non existant" as far as simulation is concerned.
   /// @param   hidden   True if object is to be hidden
   virtual void setHidden(bool hidden);

   /// Returns true if this object can be damaged
   bool isInvincible();

   /// Start fade of object in/out
   /// @param   fadeTime Time fade should take
   /// @param   fadeDelay Delay before starting fade
   /// @param   fadeOut   True if object is fading out, false if fading in.
   void startFade( F32 fadeTime, F32 fadeDelay = 0.0, bool fadeOut = true );

   /// Traverses mounted objects and registers light sources with the light manager
   /// @param   lightManager   Light manager to register with
   /// @param   lightingScene  Set to true if the scene is being lit, in which case these lights will not be used
   //void registerLights(LightManager * lightManager, bool lightingScene);

   // ISceneLight
   virtual void submitLights( LightManager *lm, bool staticLighting );
   virtual LightInfo* getLight() { return NULL; }

   /// @}

   /// Returns true if the point specified is in the water
   /// @param   point    Point to test in world space
   bool pointInWater( Point3F &point );

   /// Returns the percentage of this object covered by water
   F32 getWaterCoverage()  { return mWaterCoverage; }

   /// Returns the height of the liquid on this object
   F32 getLiquidHeight()  { return mLiquidHeight; }

   virtual WaterObject* getCurrentWaterObject();

   void setCurrentWaterObject( WaterObject *obj );

   virtual F32 getMass() const { return mMass; }

   /// @name Network
   /// @{

   F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);
   U32  packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);
   void writePacketData(GameConnection *conn, BitStream *stream);
   void readPacketData(GameConnection *conn, BitStream *stream);

   /// @}

   DECLARE_CONOBJECT(ShapeBase);

protected:
   DECLARE_CALLBACK( F32, validateCameraFov, (F32 fov) );

};


//------------------------------------------------------------------------------
// inlines
//------------------------------------------------------------------------------

inline bool ShapeBase::getCloakedState()
{
   return(mCloaked);
}

inline F32 ShapeBase::getCloakLevel()
{
   return(mCloakLevel);
}

inline const char* ShapeBase::getShapeName()
{
   return mShapeNameHandle.getString();
}

inline const char* ShapeBase::getSkinName()
{
   return mSkinNameHandle.getString();
}

inline WaterObject* ShapeBase::getCurrentWaterObject()
{
   if ( isMounted() && mShapeBaseMount )   
      return mShapeBaseMount->getCurrentWaterObject();
   
   return mCurrentWaterObject;
}

#endif  // _H_SHAPEBASE_
