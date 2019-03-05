#pragma once
#include "playerObject.h"

#include "T3D/components/ai/aiControllerComponent.h"

class AIPlayerObject : public PlayerObject
{
   typedef PlayerObject Parent;

   AIControllerComponent* mAIControllerComponent;

public:
   AIPlayerObject();
   ~AIPlayerObject();

   virtual bool onAdd();
   virtual void onRemove();

   DECLARE_CONOBJECT(AIPlayerObject);
};