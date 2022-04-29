#pragma once

#include "T3D/components/collision/collisionComponent.h"
#include "T3D/components/physics/physicsComponent.h"
#include "T3D/physics/physicsWorld.h"

class RaycastColliderComponent : public CollisionComponent
{
   typedef CollisionComponent Parent;

   //If we're velocity based, we need a physics component on our owner to calculate the vel
   bool mUseVelocity;
   PhysicsComponent* mOwnerPhysicsComponent;

   //If we're not using velocity, we'll just have a set direction and length we check against
   VectorF mRayDirection;
   F32 mRayLength;

   PhysicsWorld *mPhysicsWorld;

   Point3F mOldPosition;

   U32 mMask;

public:
   DECLARE_CONOBJECT(RaycastColliderComponent);

   RaycastColliderComponent();
   ~RaycastColliderComponent();

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   //This is called when a different component is added to our owner entity
   virtual void componentAddedToOwner(Component *comp);
   //This is called when a different component is removed from our owner entity
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void advanceTime(F32 dt);
};