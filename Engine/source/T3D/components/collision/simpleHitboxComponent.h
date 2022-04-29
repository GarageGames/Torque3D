#pragma once

#include "T3D/components/component.h"

class SimpleHitboxComponent : public Component
{
   typedef Component Parent;

   // location of head, torso, legs
   F32 mBoxHeadPercentage;
   F32 mBoxTorsoPercentage;

   // damage locations
   F32 mBoxLeftPercentage;
   F32 mBoxRightPercentage;
   F32 mBoxBackPercentage;
   F32 mBoxFrontPercentage;

   // Is our hitbox horizontal, usually due to being prone, swimming, etc
   bool mIsProne;

public:
   SimpleHitboxComponent();
   ~SimpleHitboxComponent();

   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void advanceTime(F32 dt);

   void getDamageLocation(const Point3F& in_rPos, const char *&out_rpVert, const char *&out_rpQuad);

   DECLARE_CONOBJECT(SimpleHitboxComponent);
};
