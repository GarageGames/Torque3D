#pragma once

#include "T3D/components/component.h"
#include "T3D/components/game/interactComponent.h"

class InteractableComponent : public Component
{
   typedef Component Parent;

   //Controls importance values when using radius mode for interaction
   F32 mInteractableWeight;

public:
   InteractableComponent();
   ~InteractableComponent();

   DECLARE_CONOBJECT(InteractableComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   void interact(InteractComponent* interactor);
   void interact(InteractComponent* interactor, RayInfo rayInfo);

   F32 getWeight() { return mInteractableWeight; }
};