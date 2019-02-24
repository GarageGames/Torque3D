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

#include "T3D/components/game/interactComponent.h"
#include "scene/sceneContainer.h"
#include "T3D/components/game/interactableComponent.h"

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
InteractComponent::InteractComponent() : Component(),
   mUseRaycastInteract(true),
   mUseRenderedRaycast(false),
   mUseRadiusInteract(true),
   mInteractRadius(1.5),
   mInteractRayDist(1.5),
   mUseNaturalReach(false)
{
   mFriendlyName = "Interact";
   mComponentType = "Game";

   mDescription = getDescriptionText("Allows owner entity interact.");
}

InteractComponent::~InteractComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(InteractComponent);

bool InteractComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void InteractComponent::onRemove()
{
   Parent::onRemove();
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void InteractComponent::onComponentAdd()
{
   Parent::onComponentAdd();
}

void InteractComponent::onComponentRemove()
{
   Parent::onComponentRemove();
}

void InteractComponent::initPersistFields()
{
   Parent::initPersistFields();
}

void InteractComponent::processTick()
{
   if (isClientObject())
      return; //this shouldn't happen

   //First, if we're doing rays, try that as if we get a hit, we can always assume that's the "correct" option
   if (mUseRaycastInteract)
   {
      mOwner->disableCollision();

      Point3F start = mOwner->getPosition();
      Point3F end = mOwner->getTransform().getForwardVector() * mInteractRayDist + start;

      RayInfo rinfo;
      S32 ret = 0;

      if (mUseRenderedRaycast)
      {
         rinfo.generateTexCoord = true;
         if (gServerContainer.castRayRendered(mOwner->getPosition(), end, EntityObjectType, &rinfo) == true)
            ret = rinfo.object->getId();
      }
      else
      {
         if (gServerContainer.castRay(mOwner->getPosition(), end, EntityObjectType, &rinfo) == true)
            ret = rinfo.object->getId();
      }

      mOwner->enableCollision();

      Entity* hitEntity = static_cast<Entity*>(rinfo.object);

      if (hitEntity)
      {
         //call on that badboy!
         InteractableComponent* iComp = hitEntity->getComponent<InteractableComponent>();

         if (iComp)
         {
            iComp->interact(this, rinfo);
            return;
         }
      }
   }

   //If not using rays or no hit, then do the radius search if we have it
   if (mUseRadiusInteract)
   {
      gServerContainer.initRadiusSearch(mOwner->getPosition(), mInteractRadius, EntityObjectType);

      F32 lastBestDist = 9999;
      F32 lastBestWeight = 0;
      Entity* bestFitEntity = nullptr;

      Entity* e = static_cast<Entity*>(gServerContainer.containerSearchNextObject());

      while (e != nullptr)
      {
         InteractableComponent* iComp = e->getComponent<InteractableComponent>();

         if (iComp != nullptr)
         {
            F32 weight = iComp->getWeight();
            VectorF distVec = e->getPosition() - mOwner->getPosition();
            F32 dist = distVec.len();

            //If the weight is better, always pick it
            if (weight > lastBestWeight)
            {
               lastBestDist = dist;
               lastBestWeight = weight;
               bestFitEntity = e;
            }
            //Otherwise, if the weight is matched and the distance is closer, pick that
            else if (weight >= lastBestWeight && lastBestDist < dist)
            {
               lastBestWeight = weight;
               bestFitEntity = e;
            }
         }

         e = static_cast<Entity*>(gServerContainer.containerSearchNextObject()); //loop 'round
      }

      if (bestFitEntity)
      {
         //call on that badboy!
         InteractableComponent* iComp = bestFitEntity->getComponent<InteractableComponent>();

         iComp->interact(this);
      }
   }
}