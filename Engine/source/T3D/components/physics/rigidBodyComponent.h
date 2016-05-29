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

#ifndef RIGID_BODY_COMPONENT_H
#define RIGID_BODY_COMPONENT_H

#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef COLLISION_COMPONENT_H
#include "T3D/components/collision/collisionComponent.h"
#endif
#ifndef PHYSICS_COMPONENT_INTERFACE_H
#include "T3D/components/physics/physicsComponentInterface.h"
#endif

class PhysicsBody;

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class RigidBodyComponent : public Component, public PhysicsComponentInterface
{
   typedef Component Parent;

   enum SimType
   {
      /// This physics representation only exists on the client
      /// world and the server only does ghosting.
      SimType_ClientOnly,

      /// The physics representation only exists on the server world
      /// and the client gets delta updates for rendering.
      SimType_ServerOnly,

      /// The physics representation exists on the client and the server
      /// worlds with corrections occuring when the client gets out of sync.
      SimType_ClientServer,

      /// The bits used to pack the SimType field.
      SimType_Bits = 3,

   } mSimType;

   //
   //
   /// The current physics state.
   PhysicsState mState;

   /// The previous and current render states.
   PhysicsState mRenderState[2];

   /// The abstracted physics actor.
   PhysicsBody *mPhysicsRep;

   PhysicsWorld *mWorld;

   /// The starting position to place the shape when
   /// the level begins or is reset.
   MatrixF mResetPos;
   //
   //

   /// If true then no corrections are sent from the server 
   /// and/or applied from the client.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoCorrections;

   /// If true then no smoothing is done on the client when
   /// applying server corrections.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoSmoothing;

   ///
   F32 mMass;

   /// 
   F32 mDynamicFriction;

   /// 
   F32 mStaticFriction;

   ///
   F32 mRestitution;

   ///
   F32 mLinearDamping;

   ///
   F32 mAngularDamping;

   /// 
   F32 mLinearSleepThreshold;

   ///
   F32 mAngularSleepThreshold;

   // A scale applied to the normal linear and angular damping
   // when the object enters a water volume.
   F32 mWaterDampingScale;

   // The density of this object used for water buoyancy effects.
   F32 mBuoyancyDensity;

   CollisionComponent* mOwnerColComponent;

   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0,
      FreezeMask = Parent::NextFreeMask << 1,
      StateMask = Parent::NextFreeMask << 2,
      VelocityMask = Parent::NextFreeMask << 3,
      NextFreeMask = Parent::NextFreeMask << 4
   };

public:
   RigidBodyComponent();
   virtual ~RigidBodyComponent();
   DECLARE_CONOBJECT(RigidBodyComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void ownerTransformSet(MatrixF *mat);

   inline F32 getMass() { return mMass; }
   Point3F getVelocity() const { return mState.linVelocity; }
   void applyImpulse(const Point3F &pos, const VectorF &vec);
   void applyRadialImpulse(const Point3F &origin, F32 radius, F32 magnitude);

   void updateContainerForces();

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();

   void findContact();

   /// Save the current transform as where we return to when a physics reset
   /// event occurs. This is automatically set in onAdd but some manipulators
   /// such as Prefab need to make use of this.
   void storeRestorePos();

   void updatePhysics(PhysicsCollision *collision = NULL);

   void _onPhysicsReset(PhysicsResetEvent reset);
};

#endif // _RIGID_BODY_COMPONENT_H_
