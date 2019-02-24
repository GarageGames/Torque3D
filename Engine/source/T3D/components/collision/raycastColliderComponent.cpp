#include "T3D/components/collision/raycastColliderComponent.h"
#include "T3D/physics/physicsPlugin.h"

IMPLEMENT_CO_DATABLOCK_V1(RaycastColliderComponent);

RaycastColliderComponent::RaycastColliderComponent() :
   mUseVelocity(false),
   mOwnerPhysicsComponent(nullptr),
   mRayDirection(VectorF::Zero),
   mRayLength(1),
   mPhysicsWorld(nullptr),
   mOldPosition(Point3F::Zero),
   mMask(-1)
{
}

RaycastColliderComponent::~RaycastColliderComponent() 
{

}

bool RaycastColliderComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (PHYSICSMGR)
      mPhysicsWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

   return true;
}
void RaycastColliderComponent::onRemove() 
{
   Parent::onRemove();
}

void RaycastColliderComponent::initPersistFields()
{
   Parent::initPersistFields();
}

void RaycastColliderComponent::onComponentAdd()
{
   PhysicsComponent* physComp = mOwner->getComponent<PhysicsComponent>();

   if (physComp)
   {
      mOwnerPhysicsComponent = physComp;
   }
}

void RaycastColliderComponent::onComponentRemove()
{
   mOwnerPhysicsComponent = nullptr;
}

void RaycastColliderComponent::componentAddedToOwner(Component *comp) 
{
   Parent::componentAddedToOwner(comp);

   PhysicsComponent* physComp = dynamic_cast<PhysicsComponent*>(comp);
   if (physComp)
   {
      mOwnerPhysicsComponent = physComp;
   }
}

void RaycastColliderComponent::componentRemovedFromOwner(Component *comp)
{
   Parent::componentRemovedFromOwner(comp);

   if (mOwnerPhysicsComponent != nullptr && mOwnerPhysicsComponent->getId() == comp->getId())
   {
      mOwnerPhysicsComponent = nullptr;
   }
}

void RaycastColliderComponent::processTick() 
{
   Parent::processTick();

   // Raycast the abstract PhysicsWorld if a PhysicsPlugin exists.
   bool hit = false;

   Point3F start = mOldPosition;
   Point3F end;

   if (mUseVelocity)
   {
      //our end is the new position
      end = mOwner->getPosition();
   }
   else
   {
      end = start + (mRayDirection * mRayLength);
   }

   RayInfo rInfo;

   if (mPhysicsWorld)
      hit = mPhysicsWorld->castRay(start, end, &rInfo, Point3F::Zero);
   else
      hit = mOwner->getContainer()->castRay(start, end, mMask, &rInfo);

   if (hit)
   {

   }

   if (mUseVelocity)
      mOldPosition = end;
}

void RaycastColliderComponent::interpolateTick(F32 dt) 
{
   Parent::interpolateTick(dt);
}

void RaycastColliderComponent::advanceTime(F32 dt) 
{
   Parent::advanceTime(dt);
}