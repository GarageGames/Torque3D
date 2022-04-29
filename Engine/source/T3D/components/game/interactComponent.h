#pragma once

#include "T3D/components/component.h"


class InteractComponent : public Component
{
   typedef Component Parent;

   bool mUseRaycastInteract;
   bool mUseRenderedRaycast;
   bool mUseRadiusInteract;

   F32 mInteractRadius;
   F32 mInteractRayDist;

   //Adjusts the length of the ray based on the idea of further reach if you look down because of crouching
   bool mUseNaturalReach;

public:
   InteractComponent();
   ~InteractComponent();

   DECLARE_CONOBJECT(InteractComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   virtual void processTick();
};