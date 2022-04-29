#include "aiPlayerObject.h"

IMPLEMENT_CO_NETOBJECT_V1(AIPlayerObject);

AIPlayerObject::AIPlayerObject()
   : mAIControllerComponent(nullptr)
{

}
AIPlayerObject::~AIPlayerObject()
{

}

bool AIPlayerObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //If we don't delinate from the template, just spawn as apropos here
   if (!mDirtyGameObject)
   {
      //AI Controller
      mAIControllerComponent = new AIControllerComponent();
      if (!mAIControllerComponent->registerObject())
      {
         Con::errorf("PlayerObject::onAdd - unable to add mAIControllerComponent!");
         return false;
      }

      mAIControllerComponent->setInternalName("aiControllerComponent");

      addComponent(mAIControllerComponent);
   }

   return true;
}

void AIPlayerObject::onRemove()
{
   Parent::onRemove();
}