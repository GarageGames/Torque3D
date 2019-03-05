#pragma once

#include "T3D/entity.h"
#include "T3D/components/audio/soundComponent.h"

class SoundEmitterObject : public Entity
{
   typedef Entity Parent;

   SoundComponent* mSoundComponent;

public:
   SoundEmitterObject();
   ~SoundEmitterObject();

   virtual bool onAdd();
   virtual void onRemove();

   DECLARE_CONOBJECT(SoundEmitterObject);
};