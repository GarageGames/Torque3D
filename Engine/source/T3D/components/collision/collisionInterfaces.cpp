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

#include "T3D/components/collision/collisionInterfaces.h"
#include "scene/sceneObject.h"
#include "T3D/entity.h"
#include "console/engineAPI.h"
#include "T3D/trigger.h"
#include "materials/baseMatInstance.h"

void CollisionInterface::handleCollisionList( CollisionList &collisionList, VectorF velocity )
{
   Collision bestCol;

   mCollisionList = collisionList;

   for (U32 i=0; i < collisionList.getCount(); ++i)
   {
      Collision& colCheck = collisionList[i];

      if (colCheck.object)
      {
         if (colCheck.object->getTypeMask() & PlayerObjectType)
         {
            handleCollision( colCheck, velocity );
         }
         else if (colCheck.object->getTypeMask() & TriggerObjectType)
         {
            // We've hit it's bounding box, that's close enough for triggers
            Trigger* pTrigger = static_cast<Trigger*>(colCheck.object);

            Component *comp = dynamic_cast<Component*>(this);
            pTrigger->potentialEnterObject(comp->getOwner());
         }
         else if (colCheck.object->getTypeMask() & DynamicShapeObjectType)
         {
            Con::printf("HIT A GENERICALLY DYNAMIC OBJECT");
            handleCollision(colCheck, velocity);
         }
         else if(colCheck.object->getTypeMask() & EntityObjectType)
         {
            Entity* ent = dynamic_cast<Entity*>(colCheck.object);
            if (ent)
            {
               CollisionInterface *colObjectInterface = ent->getComponent<CollisionInterface>();
               if (colObjectInterface)
               {
                  //convert us to our component
                  Component *thisComp = dynamic_cast<Component*>(this);
                  if (thisComp)
                  {
                     colObjectInterface->onCollisionSignal.trigger(thisComp->getOwner());

                     //TODO: properly do this
                     Collision oppositeCol = colCheck;
                     oppositeCol.object = thisComp->getOwner();

                     colObjectInterface->handleCollision(oppositeCol, velocity);
                  }
               }
            }
         }
         else
         {
            handleCollision(colCheck, velocity);
         }
      }
   }
}

void CollisionInterface::handleCollision( Collision &col, VectorF velocity )
{
   if (col.object && (mContactInfo.contactObject == NULL ||
      col.object->getId() != mContactInfo.contactObject->getId()))
   {
      queueCollision(col.object, velocity - col.object->getVelocity());

      //do the callbacks to script for this collision
      Component *comp = dynamic_cast<Component*>(this);
      if (comp->isMethod("onCollision"))
      {
         S32 matId = col.material != NULL ? col.material->getMaterial()->getId() : 0;
         Con::executef(comp, "onCollision", col.object, col.normal, col.point, matId, velocity);
      }

      if (comp->getOwner()->isMethod("onCollisionEvent"))
      {
         S32 matId = col.material != NULL ? col.material->getMaterial()->getId() : 0;
         Con::executef(comp->getOwner(), "onCollisionEvent", col.object, col.normal, col.point, matId, velocity);
      }
   }
}

void CollisionInterface::handleCollisionNotifyList()
{
   //special handling for any collision components we should notify that a collision happened.
   for (U32 i = 0; i < mCollisionNotifyList.size(); ++i)
   {
      //convert us to our component
      Component *thisComp = dynamic_cast<Component*>(this);
      if (thisComp)
      {
         mCollisionNotifyList[i]->onCollisionSignal.trigger(thisComp->getOwner());
      }
   }

   mCollisionNotifyList.clear();
}

Chunker<CollisionInterface::CollisionTimeout> sCollisionTimeoutChunker;
CollisionInterface::CollisionTimeout* CollisionInterface::sFreeTimeoutList = 0;

void CollisionInterface::queueCollision( SceneObject *obj, const VectorF &vec)
{
   // Add object to list of collisions.
   SimTime time = Sim::getCurrentTime();
   S32 num = obj->getId();

   CollisionTimeout** adr = &mTimeoutList;
   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) 
   {
      if (ptr->objectNumber == num) 
      {
         if (ptr->expireTime < time) 
         {
            ptr->expireTime = time + CollisionTimeoutValue;
            ptr->object = obj;
            ptr->vector = vec;
         }
         return;
      }
      // Recover expired entries
      if (ptr->expireTime < time) 
      {
         CollisionTimeout* cur = ptr;
         *adr = ptr->next;
         ptr = ptr->next;
         cur->next = sFreeTimeoutList;
         sFreeTimeoutList = cur;
      }
      else 
      {
         adr = &ptr->next;
         ptr = ptr->next;
      }
   }

   // New entry for the object
   if (sFreeTimeoutList != NULL)
   {
      ptr = sFreeTimeoutList;
      sFreeTimeoutList = ptr->next;
      ptr->next = NULL;
   }
   else
   {
      ptr = sCollisionTimeoutChunker.alloc();
   }

   ptr->object = obj;
   ptr->objectNumber = obj->getId();
   ptr->vector = vec;
   ptr->expireTime = time + CollisionTimeoutValue;
   ptr->next = mTimeoutList;

   mTimeoutList = ptr;
}

bool CollisionInterface::checkEarlyOut(Point3F start, VectorF velocity, F32 time, Box3F objectBox, Point3F objectScale, 
														Box3F collisionBox, U32 collisionMask, CollisionWorkingList &colWorkingList)
{
   Point3F end = start + velocity * time;
   Point3F distance = end - start;

   Box3F scaledBox = objectBox;
   scaledBox.minExtents.convolve(objectScale);
   scaledBox.maxExtents.convolve(objectScale);

   if (mFabs(distance.x) < objectBox.len_x() &&
      mFabs(distance.y) < objectBox.len_y() &&
      mFabs(distance.z) < objectBox.len_z())
   {
      // We can potentially early out of this.  If there are no polys in the clipped polylist at our
      //  end position, then we can bail, and just set start = end;
      Box3F wBox = scaledBox;
      wBox.minExtents += end;
      wBox.maxExtents += end;

      static EarlyOutPolyList eaPolyList;
      eaPolyList.clear();
      eaPolyList.mNormal.set(0.0f, 0.0f, 0.0f);
      eaPolyList.mPlaneList.clear();
      eaPolyList.mPlaneList.setSize(6);
      eaPolyList.mPlaneList[0].set(wBox.minExtents,VectorF(-1.0f, 0.0f, 0.0f));
      eaPolyList.mPlaneList[1].set(wBox.maxExtents,VectorF(0.0f, 1.0f, 0.0f));
      eaPolyList.mPlaneList[2].set(wBox.maxExtents,VectorF(1.0f, 0.0f, 0.0f));
      eaPolyList.mPlaneList[3].set(wBox.minExtents,VectorF(0.0f, -1.0f, 0.0f));
      eaPolyList.mPlaneList[4].set(wBox.minExtents,VectorF(0.0f, 0.0f, -1.0f));
      eaPolyList.mPlaneList[5].set(wBox.maxExtents,VectorF(0.0f, 0.0f, 1.0f));

      // Build list from convex states here...
      CollisionWorkingList& rList = colWorkingList;
      CollisionWorkingList* pList = rList.wLink.mNext;
      while (pList != &rList) 
      {
         Convex* pConvex = pList->mConvex;

         if (pConvex->getObject()->getTypeMask() & collisionMask) 
         {
            Box3F convexBox = pConvex->getBoundingBox();

            if (wBox.isOverlapped(convexBox))
            {
               // No need to separate out the physical zones here, we want those
               //  to cause a fallthrough as well...
               pConvex->getPolyList(&eaPolyList);
            }
         }
         pList = pList->wLink.mNext;
      }

      if (eaPolyList.isEmpty())
      {
         return true;
      }
   }

   return false;
}


Collision* CollisionInterface::getCollision(S32 col) 
{ 
   if(col < mCollisionList.getCount() && col >= 0) 
      return &mCollisionList[col];
   else 
      return NULL; 
}