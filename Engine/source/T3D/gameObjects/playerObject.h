#pragma once

#include "T3D/entity.h"
#include "T3D/components/render/meshComponent.h"
#include "T3D/components/collision/shapeCollisionComponent.h"
#include "T3D/components/animation/actionAnimationComponent.h"
#include "T3D/components/animation/armAnimationComponent.h"
#include "T3D/components/physics/playerControllerComponent.h"
#include "T3D/components/game/stateMachineComponent.h"
#include "T3D/components/camera/cameraComponent.h"
#include "T3D/components/camera/cameraOrbiterComponent.h"
#include "T3D/components/game/controlObjectComponent.h"
#include "T3D/components/audio/soundComponent.h"
#include "T3D/components/game/interactComponent.h"

class PlayerObject : public Entity
{
   typedef Entity Parent;

   MeshComponent* mMeshComponent;
   ShapeCollisionComponent* mCollisionComponent;
   ActionAnimationComponent* mAnimationComponent;
   ArmAnimationComponent* mArmAnimationComponent;
   PlayerControllerComponent* mPhysicsComponent;
   StateMachineComponent* mStateMachineComponent;
   CameraComponent* mCameraComponent;
   CameraOrbiterComponent* mCameraOrbiterComponent;
   ControlObjectComponent* mControlObjectComponent;
   SoundComponent* mSoundComponent;
   InteractComponent* mInteractComponent;

public:
   PlayerObject();
   ~PlayerObject();

   virtual bool onAdd();
   virtual void onRemove();

   DECLARE_CONOBJECT(PlayerObject);
};