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

#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif
#ifndef _RIGID_H_
#include "T3D/rigid.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;
class ClippedPolyList;
struct RenderInst;
class Vehicle;

//----------------------------------------------------------------------------

struct VehicleData: public ShapeBaseData
{
   typedef ShapeBaseData Parent;

   struct Body {
      enum Sounds {
         SoftImpactSound,
         HardImpactSound,
         MaxSounds,
      };
      SFXProfile* sound[MaxSounds];
      F32 restitution;
      F32 friction;
   } body;

   enum VehicleConsts
   {
      VC_NUM_DUST_EMITTERS = 1,
      VC_NUM_DAMAGE_EMITTER_AREAS = 2,
      VC_NUM_DAMAGE_LEVELS = 2,
      VC_NUM_BUBBLE_EMITTERS = 1,
      VC_NUM_DAMAGE_EMITTERS = VC_NUM_DAMAGE_LEVELS + VC_NUM_BUBBLE_EMITTERS,
      VC_NUM_SPLASH_EMITTERS = 2,
      VC_BUBBLE_EMITTER = VC_NUM_DAMAGE_EMITTERS - VC_NUM_BUBBLE_EMITTERS,
   };

  enum Sounds {
      ExitWater,
      ImpactSoft,
      ImpactMedium,
      ImpactHard,
      Wake,
      MaxSounds
   };
   SFXProfile* waterSound[MaxSounds];
   F32 exitSplashSoundVel;
   F32 softSplashSoundVel;
   F32 medSplashSoundVel;
   F32 hardSplashSoundVel;

   F32 minImpactSpeed;
   F32 softImpactSpeed;
   F32 hardImpactSpeed;
   F32 minRollSpeed;
   F32 maxSteeringAngle;

   F32 collDamageThresholdVel;
   F32 collDamageMultiplier;

   bool cameraRoll;           ///< Roll the 3rd party camera
   F32 cameraLag;             ///< Amount of camera lag (lag += car velocity * lag)
   F32 cameraDecay;           ///< Rate at which camera returns to target pos.
   F32 cameraOffset;          ///< Vertical offset

   F32 minDrag;
   F32 maxDrag;
   S32 integration;           ///< # of physics steps per tick
   F32 collisionTol;          ///< Collision distance tolerance
   F32 contactTol;            ///< Contact velocity tolerance
   Point3F massCenter;        ///< Center of mass for rigid body
   Point3F massBox;           ///< Size of inertial box

   F32 jetForce;
   F32 jetEnergyDrain;        ///< Energy drain/tick
   F32 minJetEnergy;

   F32 steeringReturn;
   F32 steeringReturnSpeedScale;
   bool powerSteering;

   ParticleEmitterData * dustEmitter;
   S32 dustID;
   F32 triggerDustHeight;  ///< height vehicle has to be under to kick up dust
   F32 dustHeight;         ///< dust height above ground

   ParticleEmitterData *   damageEmitterList[ VC_NUM_DAMAGE_EMITTERS ];
   Point3F damageEmitterOffset[ VC_NUM_DAMAGE_EMITTER_AREAS ];
   S32 damageEmitterIDList[ VC_NUM_DAMAGE_EMITTERS ];
   F32 damageLevelTolerance[ VC_NUM_DAMAGE_LEVELS ];
   F32 numDmgEmitterAreas;

   ParticleEmitterData* splashEmitterList[VC_NUM_SPLASH_EMITTERS];
   S32 splashEmitterIDList[VC_NUM_SPLASH_EMITTERS];
   F32 splashFreqMod;
   F32 splashVelEpsilon;

   bool enablePhysicsRep;

   //
   VehicleData();
   bool preload(bool server, String &errorStr);
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   DECLARE_CONOBJECT(VehicleData);

   DECLARE_CALLBACK( void, onEnterLiquid, ( Vehicle* obj, F32 coverage, const char* type ) );
   DECLARE_CALLBACK( void, onLeaveLiquid, ( Vehicle* obj, const char* type ) );
};


//----------------------------------------------------------------------------
class PhysicsBody;

class Vehicle: public ShapeBase
{
   typedef ShapeBase Parent;

  protected:
   enum CollisionFaceFlags {
      BodyCollision =  0x1,
      WheelCollision = 0x2,
   };
   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0,
      EnergyMask   = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
   };

   struct StateDelta {
      Move move;                    ///< Last move from server
      F32 dt;                       ///< Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F posVec;
      QuatF rot[2];
      // Warp data
      S32 warpTicks;                ///< Number of ticks to warp
      S32 warpCount;                ///< Current pos in warp
      Point3F warpOffset;
      QuatF warpRot[2];
      //
      Point3F cameraOffset;
      Point3F cameraVec;
      Point3F cameraRot;
      Point3F cameraRotVec;
   };

   PhysicsBody *mPhysicsRep;

   StateDelta mDelta;
   S32 mPredictionCount;            ///< Number of ticks to predict
   VehicleData* mDataBlock;
   bool inLiquid;
   SFXSource* mWakeSound;

   Point3F mCameraOffset; ///< 3rd person camera

   // Control
   Point2F mSteering;
   F32 mThrottle;
   bool mJetting;

   // Rigid Body
   bool mDisableMove;

   GFXStateBlockRef  mSolidSB;

   Box3F         mWorkingQueryBox;
   S32           mWorkingQueryBoxCountDown;

   CollisionList mCollisionList;
   CollisionList mContacts;
   Rigid mRigid;
   ShapeBaseConvex mConvex;
   S32 restCount;

   SimObjectPtr<ParticleEmitter> mDustEmitterList[VehicleData::VC_NUM_DUST_EMITTERS];
   SimObjectPtr<ParticleEmitter> mDamageEmitterList[VehicleData::VC_NUM_DAMAGE_EMITTERS];
   SimObjectPtr<ParticleEmitter> mSplashEmitterList[VehicleData::VC_NUM_SPLASH_EMITTERS];

   //
   virtual bool onNewDataBlock( GameBaseData *dptr, bool reload );
   void updatePos(F32 dt);
   bool updateCollision(F32 dt);
   bool resolveCollision(Rigid& ns,CollisionList& cList);
   bool resolveContacts(Rigid& ns,CollisionList& cList,F32 dt);
   bool resolveDisplacement(Rigid& ns,CollisionState *state,F32 dt);
   bool findContacts(Rigid& ns,CollisionList& cList);
   void checkTriggers();
   static void findCallback(SceneObject* obj,void * key);

   void setPosition(const Point3F& pos,const QuatF& rot);
   void setRenderPosition(const Point3F& pos,const QuatF& rot);
   void setTransform(const MatrixF& mat);

//   virtual bool collideBody(const MatrixF& mat,Collision* info) = 0;
   virtual void updateMove(const Move* move);
   virtual void updateForces(F32 dt);

   void writePacketData(GameConnection * conn, BitStream *stream);
   void readPacketData (GameConnection * conn, BitStream *stream);
   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);

   void updateLiftoffDust( F32 dt );
   void updateDamageSmoke( F32 dt );

   void updateWorkingCollisionSet(const U32 mask);
   virtual U32 getCollisionMask();

   void updateFroth( F32 dt );
   bool collidingWithWater( Point3F &waterHeight );

   /// ObjectRenderInst delegate hooked up in prepBatchRender 
   /// if GameBase::gShowBoundingBox is true.
   void _renderMassAndContacts( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   /// ObjectRenderInst delegate hooked up in prepBatchRender 
   /// if GameBase::gShowBoundingBox is true.
   void _renderMuzzleVector( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

public:
   // Test code...
   static ClippedPolyList* sPolyList;
   static S32 sVehicleCount;

   //
   Vehicle();
   static void consoleInit();
   static void initPersistFields();
   void processTick(const Move *move);
   bool onAdd();
   void onRemove();

   void _createPhysics();

   /// Interpolates between move ticks @see processTick
   /// @param   dt   Change in time between the last call and this call to the function
   void interpolateTick(F32 dt);
   void advanceTime(F32 dt);

   /// Disables collisions for this vehicle and all mounted objects
   void disableCollision();

   /// Enables collisions for this vehicle and all mounted objects
   void enableCollision();

   /// Returns the velocity of the vehicle
   Point3F getVelocity() const;

   void setEnergyLevel(F32 energy);

   void prepBatchRender( SceneRenderState *state, S32 mountedImageIndex );

   ///@name Rigid body methods
   ///@{

   /// This method will get the velocity of the object, taking into account
   /// angular velocity.
   /// @param   r   Point on the object you want the velocity of, relative to Center of Mass
   /// @param   vel   Velocity (out)
   void getVelocity(const Point3F& r, Point3F* vel);

   /// Applies an impulse force
   /// @param   r   Point on the object to apply impulse to, r is relative to Center of Mass
   /// @param   impulse   Impulse vector to apply.
   void applyImpulse(const Point3F &r, const Point3F &impulse);

   void getCameraParameters(F32 *min, F32* max, Point3F* offset, MatrixF* rot);
   void getCameraTransform(F32* pos, MatrixF* mat);
   ///@}

   /// @name Mounted objects
   /// @{
   virtual void mountObject( SceneObject *obj, S32 node, const MatrixF &xfm = MatrixF::Identity );
   /// @}

   DECLARE_CONOBJECT(Vehicle);
};


#endif
