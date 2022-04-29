#include "SoundEmitterObject.h"

IMPLEMENT_CO_NETOBJECT_V1(SoundEmitterObject);

SoundEmitterObject::SoundEmitterObject()
   :  mSoundComponent(nullptr)
{

}
SoundEmitterObject::~SoundEmitterObject()
{

}

bool SoundEmitterObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //Sound
   mSoundComponent = new SoundComponent();
   if (!mSoundComponent->registerObject())
   {
      Con::errorf("SoundEmitterObject::onAdd - unable to add soundComponent!");
      return false;
   }

   mSoundComponent->setInternalName("soundComponent");
   
   addComponent(mSoundComponent);

   return true;
}

void SoundEmitterObject::onRemove()
{
   Parent::onRemove();
}