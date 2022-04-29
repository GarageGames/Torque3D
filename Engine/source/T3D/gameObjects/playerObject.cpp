#include "playerObject.h"

IMPLEMENT_CO_NETOBJECT_V1(PlayerObject);

PlayerObject::PlayerObject()
   :  mMeshComponent(nullptr),
      mCollisionComponent(nullptr),
      mAnimationComponent(nullptr),
      mPhysicsComponent(nullptr)
{

}
PlayerObject::~PlayerObject()
{

}

bool PlayerObject::onAdd()
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
         Con::errorf("PlayerObject::onAdd - unable to add MeshComponent!");
         return false;
      }

      mMeshComponent->setInternalName("meshComponent");

      addComponent(mMeshComponent);

      //Collision
      mCollisionComponent = new ShapeCollisionComponent();
      if (!mCollisionComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add ShapeCollisionComponent!");
         return false;
      }

      mCollisionComponent->setInternalName("collisionComponent");

      addComponent(mCollisionComponent);

      //Animation
      mAnimationComponent = new ActionAnimationComponent();
      if (!mAnimationComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add ActionAnimationComponent!");
         return false;
      }

      mAnimationComponent->setInternalName("animationComponent");

      addComponent(mAnimationComponent);

      //Arm Animation
      mArmAnimationComponent = new ArmAnimationComponent();
      if (!mArmAnimationComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add ArmAnimationComponent!");
         return false;
      }

      mArmAnimationComponent->setInternalName("armAnimationComponent");

      addComponent(mArmAnimationComponent);

      //Physics control
      mPhysicsComponent = new PlayerControllerComponent();
      if (!mPhysicsComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add PhysicsComponent!");
         return false;
      }

      mPhysicsComponent->setInternalName("physicsComponent");

      addComponent(mPhysicsComponent);

      //State Machine
      mStateMachineComponent = new StateMachineComponent();
      if (!mStateMachineComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add StateMachineComponent!");
         return false;
      }

      mStateMachineComponent->setInternalName("stateMachineComponent");

      addComponent(mStateMachineComponent);

      //Camera
      mCameraComponent = new CameraComponent();
      if (!mCameraComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add CameraComponent!");
         return false;
      }

      mCameraComponent->setInternalName("cameraComponent");

      addComponent(mCameraComponent);

      //Camera Orbiter
      mCameraOrbiterComponent = new CameraOrbiterComponent();
      if (!mCameraOrbiterComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add CameraOrbiterComponent!");
         return false;
      }

      mCameraOrbiterComponent->setInternalName("cameraOrbiterComponent");

      addComponent(mCameraOrbiterComponent);

      //Control Object
      mControlObjectComponent = new ControlObjectComponent();
      if (!mControlObjectComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add ControlObjectComponent!");
         return false;
      }

      mControlObjectComponent->setInternalName("controlObjectComponent");

      addComponent(mControlObjectComponent);

      //Sound
      mSoundComponent = new SoundComponent();
      if (!mSoundComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add SoundComponent!");
         return false;
      }

      mSoundComponent->setInternalName("soundComponent");

      addComponent(mSoundComponent);

      //Interaction
      mInteractComponent = new InteractComponent();
      if (!mInteractComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add InteractComponent!");
         return false;
      }

      mInteractComponent->setInternalName("interactComponent");

      addComponent(mInteractComponent);
   }

   return true;
}

void PlayerObject::onRemove()
{
   Parent::onRemove();
}