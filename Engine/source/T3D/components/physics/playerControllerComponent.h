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

#ifndef PLAYER_CONTORLLER_COMPONENT_H
#define PLAYER_CONTORLLER_COMPONENT_H

#ifndef PHYSICSBEHAVIOR_H
#include "T3D/components/physics/physicsBehavior.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSWORLD_H_
#include "T3D/physics/physicsWorld.h"
#endif
#ifndef PHYSICS_COMPONENT_INTERFACE_H
#include "T3D/components/physics/physicsComponentInterface.h"
#endif
#ifndef COLLISION_INTERFACES_H
#include "T3D/components/collision/collisionInterfaces.h"
#endif

class SceneRenderState;
class PhysicsWorld;
class PhysicsPlayer;
class SimplePhysicsBehaviorInstance;
class CollisionInterface;

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class PlayerControllerComponent : public Component,
   public PhysicsComponentInterface
{
   typedef Component Parent;

   enum MaskBits {
      VelocityMask = Parent::NextFreeMask << 0,
      PositionMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
   };

   struct StateDelta
   {
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
   };

   StateDelta mDelta;

   PhysicsPlayer *mPhysicsRep;
   PhysicsWorld  *mPhysicsWorld;

   CollisionInterface* mOwnerCollisionInterface;

   struct ContactInfo
   {
      bool contacted, jump, run;
      SceneObject *contactObject;
      VectorF  contactNormal;
      F32 contactTime;

      void clear()
      {
         contacted = jump = run = false;
         contactObject = NULL;
         contactNormal.set(1, 1, 1);
      }

      ContactInfo() { clear(); }

   } mContactInfo;

protected:
   F32 mDrag;
   F32 mBuoyancy;
   F32 mFriction;
   F32 mElasticity;
   F32 mMaxVelocity;
   bool mSticky;

   bool mFalling;
   bool mSwimming;
   bool mInWater;

   S32 mContactTimer;               ///< Ticks since last contact

   U32 mIntegrationCount;

   Point3F mJumpSurfaceNormal;      ///< Normal of the surface the player last jumped on

   F32 maxStepHeight;         ///< Maximum height the player can step up
   F32 moveSurfaceAngle;      ///< Maximum angle from vertical in degrees the player can run up
   F32 contactSurfaceAngle;   ///< Maximum angle from vertical in degrees we consider having real 'contact'

   F32 horizMaxSpeed;         ///< Max speed attainable in the horizontal
   F32 horizMaxAccel;
   F32 horizResistSpeed;      ///< Speed at which resistance will take place
   F32 horizResistFactor;     ///< Factor of resistance once horizResistSpeed has been reached

   F32 upMaxSpeed;            ///< Max vertical speed attainable
   F32 upMaxAccel;
   F32 upResistSpeed;         ///< Speed at which resistance will take place
   F32 upResistFactor;        ///< Factor of resistance once upResistSpeed has been reached

   F32 fallingSpeedThreshold; ///< Downward speed at which we consider the player falling

   // Air control
   F32 airControl;

   Point3F mInputVelocity;

   bool mUseDirectMoveInput;

public:
   PlayerControllerComponent();
   virtual ~PlayerControllerComponent();
   DECLARE_CONOBJECT(PlayerControllerComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void ownerTransformSet(MatrixF *mat);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   void updatePhysics(PhysicsCollision *collision = NULL);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void updatePos(const F32 dt);
   void updateMove();

   virtual VectorF getVelocity() { return mVelocity; }
   virtual void setVelocity(const VectorF& vel);
   virtual void setTransform(const MatrixF& mat);

   void findContact(bool *run, bool *jump, VectorF *contactNormal);
   Point3F getContactNormal() { return mContactInfo.contactNormal; }
   SceneObject* getContactObject() { return mContactInfo.contactObject; }
   bool isContacted() { return mContactInfo.contacted; }

   //
   void applyImpulse(const Point3F &pos, const VectorF &vec);

   //This is a weird artifact of the PhysicsReps. We want the collision component to be privvy to any events that happen
   //so when the physics components do a findContact test during their update, they'll have a signal collision components
   //can be listening to to update themselves with that info
   Signal< void(SceneObject*) > onContactSignal;

   //
   DECLARE_CALLBACK(void, updateMove, (PlayerControllerComponent* obj));
};

#endif // _COMPONENT_H_
