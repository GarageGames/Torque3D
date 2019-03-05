#include "staticShapeObject.h"

IMPLEMENT_CO_NETOBJECT_V1(StaticShapeObject);

StaticShapeObject::StaticShapeObject()
   :  mMeshComponent(nullptr),
      mCollisionComponent(nullptr),
      mAnimationComponent(nullptr)
{

}
StaticShapeObject::~StaticShapeObject()
{

}

bool StaticShapeObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //If we don't delinate from the template, just spawn as apropos here
   if (!mDirtyGameObject)
   {
      //Mesh
      mMeshComponent = new MeshComponent();
      if (!mMeshComponent->registerObject())
      {
         Con::errorf("StaticShapeObject::onAdd - unable to add MeshComponent!");
         return false;
      }

      mMeshComponent->setInternalName("meshComponent");

      addComponent(mMeshComponent);

      //Collision
      mCollisionComponent = new ShapeCollisionComponent();
      if (!mCollisionComponent->registerObject())
      {
         Con::errorf("StaticShapeObject::onAdd - unable to add ShapeCollisionComponent!");
         return false;
      }

      mCollisionComponent->setInternalName("collisionComponent");

      addComponent(mCollisionComponent);

      //Animation
      mAnimationComponent = new AnimationComponent();
      if (!mAnimationComponent->registerObject())
      {
         Con::errorf("StaticShapeObject::onAdd - unable to add AnimationComponent!");
         return false;
      }

      mAnimationComponent->setInternalName("animationComponent");

      addComponent(mAnimationComponent);
   }

   return true;
}

void StaticShapeObject::onRemove()
{
   Parent::onRemove();
}