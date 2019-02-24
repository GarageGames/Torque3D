//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "T3D/components/game/InteractableComponent.h"

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
InteractableComponent::InteractableComponent() : Component(),
   mInteractableWeight(1)
{
   mFriendlyName = "Interactable";
   mComponentType = "Game";

   mDescription = getDescriptionText("Allows owner entity to be interacted with.");
}

InteractableComponent::~InteractableComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(InteractableComponent);

bool InteractableComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void InteractableComponent::onRemove()
{
   Parent::onRemove();
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void InteractableComponent::onComponentAdd()
{
   Parent::onComponentAdd();
}

void InteractableComponent::onComponentRemove()
{
   Parent::onComponentRemove();
}

void InteractableComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("interactableWeight", TypeF32, Offset(mInteractableWeight, InteractableComponent), "Controls importance values when using radius mode for interaction");
}

void InteractableComponent::interact(InteractComponent* interactor)
{
   if (interactor != nullptr)
   {
      mOwner->notifyComponents("onInteract", interactor->getIdString());

      if(isMethod("onInteract"))
         Con::executef(this, "onInteract", interactor);
   }
}

void InteractableComponent::interact(InteractComponent* interactor, RayInfo rayInfo)
{
   if (interactor != nullptr)
   {
      mOwner->notifyComponents("onInteract", interactor->getIdString());

      if (isMethod("onInteract"))
         Con::executef(this, "onInteract", interactor);
   }
}