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

#include "T3D/shapeBase.h"
#include "T3D/item.h"
#include "T3D/trigger.h"

//----------------------------------------------------------------------------

void collisionFilter(SceneObject* object,void *key)
{
   SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
   ShapeBase* ptr = reinterpret_cast<ShapeBase*>(info->key);

   if (object->getTypeMask() & ItemObjectType) {
      // We've hit it's bounding box, that's close enough for items.
      Item* item = static_cast<Item*>(object);
      if (ptr != item->getCollisionObject())
         ptr->queueCollision(item,ptr->getVelocity() - item->getVelocity());
   }
   else
      if (object->getTypeMask() & TriggerObjectType) {
         // We've hit it's bounding box, that's close enough for triggers
         Trigger* pTrigger = static_cast<Trigger*>(object);
         pTrigger->potentialEnterObject(ptr);
      }
      else
         if (object->getTypeMask() & CorpseObjectType)  {
            // Ok, guess it's close enough for corpses too...
            ShapeBase* col = static_cast<ShapeBase*>(object);
            ptr->queueCollision(col,ptr->getVelocity() - col->getVelocity());
         }
         else
            object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
}

void defaultFilter(SceneObject* object,void * key)
{
   SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
   object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
}

