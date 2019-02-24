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

#include "T3D/components/collision/collisionComponent.h"
#include "scene/sceneObject.h"
#include "T3D/entity.h"
#include "console/engineAPI.h"
#include "T3D/trigger.h"
#include "materials/baseMatInstance.h"
#include "collision/extrudedPolyList.h"
#include "opcode/Opcode.h"
#include "opcode/Ice/IceAABB.h"
#include "opcode/Ice/IcePoint.h"
#include "opcode/OPC_AABBTree.h"
#include "opcode/OPC_AABBCollider.h"
#include "collision/clippedPolyList.h"

static F32 sTractionDistance = 0.04f;

IMPLEMENT_CONOBJECT(CollisionComponent);

CollisionComponent::CollisionComponent() : Component()
{
   mFriendlyName = "Collision Component";

   mComponentType = "Collision";

   mDescription = getDescriptionText("A stub component class that collision components should inherit from.");

   mBlockColliding = true;

   CollisionMoveMask = (TerrainObjectType | PlayerObjectType |
      StaticShapeObjectType | VehicleObjectType |
      VehicleBlockerObjectType | DynamicShapeObjectType | StaticObjectType | EntityObjectType | TriggerObjectType);

   mPhysicsRep = nullptr;
   mPhysicsWorld = nullptr;

   mTimeoutList = nullptr;
}

CollisionComponent::~CollisionComponent()
{
   for (S32 i = 0; i < mFields.size(); ++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);

   SAFE_DELETE(mPhysicsRep);
}

bool CollisionComponent::checkCollisions(const F32 travelTime, Point3F *velocity, Point3F start)
{
   return false;
}

bool CollisionComponent::updateCollisions(F32 time, VectorF vector, VectorF velocity)
{
   return false;
}

void CollisionComponent::updateWorkingCollisionSet(const U32 mask)
{
}

void CollisionComponent::handleCollisionList( CollisionList &collisionList, VectorF velocity )
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
               CollisionComponent *colObjectInterface = ent->getComponent<CollisionComponent>();
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

void CollisionComponent::handleCollision( Collision &col, VectorF velocity )
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

void CollisionComponent::handleCollisionNotifyList()
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

Chunker<CollisionComponent::CollisionTimeout> sCollisionTimeoutChunker;
CollisionComponent::CollisionTimeout* CollisionComponent::sFreeTimeoutList = 0;

void CollisionComponent::queueCollision( SceneObject *obj, const VectorF &vec)
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

bool CollisionComponent::checkEarlyOut(Point3F start, VectorF velocity, F32 time, Box3F objectBox, Point3F objectScale, 
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


Collision* CollisionComponent::getCollision(S32 col) 
{ 
   if(col < mCollisionList.getCount() && col >= 0) 
      return &mCollisionList[col];
   else 
      return NULL; 
}

Point3F CollisionComponent::getContactNormal() 
{ 
   return mContactInfo.contactNormal; 
}

bool CollisionComponent::hasContact()
{
   if (mContactInfo.contactObject)
      return true;
   else
      return false;
}

S32 CollisionComponent::getCollisionCount()
{
   return mCollisionList.getCount();
}

Point3F CollisionComponent::getCollisionNormal(S32 collisionIndex)
{
   if (collisionIndex < 0 || mCollisionList.getCount() < collisionIndex)
      return Point3F::Zero;

   return mCollisionList[collisionIndex].normal;
}

F32 CollisionComponent::getCollisionAngle(S32 collisionIndex, Point3F upVector)
{
   if (collisionIndex < 0 || mCollisionList.getCount() < collisionIndex)
      return 0.0f;

   return mRadToDeg(mAcos(mDot(mCollisionList[collisionIndex].normal, upVector)));
}

S32 CollisionComponent::getBestCollision(Point3F upVector)
{
   S32 bestCollision = -1;

   F32 bestAngle = 360.f;
   S32 count = mCollisionList.getCount();
   for (U32 i = 0; i < count; ++i)
   {
      F32 angle = mRadToDeg(mAcos(mDot(mCollisionList[i].normal, upVector)));

      if (angle < bestAngle)
      {
         bestCollision = i;
         bestAngle = angle;
      }
   }

   return bestCollision;
}

F32 CollisionComponent::getBestCollisionAngle(VectorF upVector)
{
   S32 bestCol = getBestCollision(upVector);

   if (bestCol == -1)
      return 0;

   return getCollisionAngle(bestCol, upVector);
}

//
bool CollisionComponent::buildConvexOpcode(TSShapeInstance* sI, S32 dl, const Box3F &bounds, Convex *c, Convex *list)
{
   AssertFatal(dl >= 0 && dl < sI->getShape()->details.size(), "TSShapeInstance::buildConvexOpcode");

   TSShape* shape = sI->getShape();

   const MatrixF &objMat = mOwner->getObjToWorld();
   const Point3F &objScale = mOwner->getScale();

   // get subshape and object detail
   const TSDetail * detail = &shape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;

   S32 start = shape->subShapeFirstObject[ss];
   S32 end = shape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF initialMat = objMat;
      Point3F initialScale = objScale;

      // set up for first object's node
      MatrixF mat;
      MatrixF scaleMat(true);
      F32* p = scaleMat;
      p[0] = initialScale.x;
      p[5] = initialScale.y;
      p[10] = initialScale.z;
      const MatrixF * previousMat = &sI->mMeshObjects[start].getTransform();
      mat.mul(initialMat, scaleMat);
      mat.mul(*previousMat);

      // Update our bounding box...
      Box3F localBox = bounds;
      MatrixF otherMat = mat;
      otherMat.inverse();
      otherMat.mul(localBox);

      // run through objects and collide
      for (S32 i = start; i<end; i++)
      {
         TSShapeInstance::MeshObjectInstance * meshInstance = &sI->mMeshObjects[i];

         if (od >= meshInstance->object->numMeshes)
            continue;

         if (&meshInstance->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &meshInstance->getTransform();

            if (previousMat != NULL)
            {
               mat.mul(initialMat, scaleMat);
               mat.mul(*previousMat);

               // Update our bounding box...
               otherMat = mat;
               otherMat.inverse();
               localBox = bounds;
               otherMat.mul(localBox);
            }
         }

         // collide... note we pass the original mech transform
         // here so that the convex data returned is in mesh space.
         TSMesh * mesh = meshInstance->getMesh(od);
         if (mesh && !meshInstance->forceHidden && meshInstance->visible > 0.01f && localBox.isOverlapped(mesh->getBounds()))
            emitted |= buildMeshOpcode(mesh, *previousMat, localBox, c, list);
         else
            emitted |= false;
      }
   }

   return emitted;
}

bool CollisionComponent::buildMeshOpcode(TSMesh *mesh, const MatrixF &meshToObjectMat,
   const Box3F &nodeBox, Convex *convex, Convex *list)
{
   /*PROFILE_SCOPE(MeshCollider_buildConvexOpcode);

   // This is small... there is no win for preallocating it.
   Opcode::AABBCollider opCollider;
   opCollider.SetPrimitiveTests(true);

   // This isn't really needed within the AABBCollider as 
   // we don't use temporal coherance... use a static to 
   // remove the allocation overhead.
   static Opcode::AABBCache opCache;

   IceMaths::AABB opBox;
   opBox.SetMinMax(Point(nodeBox.minExtents.x, nodeBox.minExtents.y, nodeBox.minExtents.z),
      Point(nodeBox.maxExtents.x, nodeBox.maxExtents.y, nodeBox.maxExtents.z));
   Opcode::CollisionAABB opCBox(opBox);

   if (!opCollider.Collide(opCache, opCBox, *mesh->mOptTree))
      return false;

   U32 cnt = opCollider.GetNbTouchedPrimitives();
   const udword *idx = opCollider.GetTouchedPrimitives();

   Opcode::VertexPointers vp;
   for (S32 i = 0; i < cnt; i++)
   {
      // First, check our active convexes for a potential match (and clean things
      // up, too.)
      const U32 curIdx = idx[i];

      // See if the square already exists as part of the working set.
      bool gotMatch = false;
      CollisionWorkingList& wl = convex->getWorkingList();
      for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext)
      {
         if (itr->mConvex->getType() != TSPolysoupConvexType)
            continue;

         const MeshColliderPolysoupConvex *chunkc = static_cast<MeshColliderPolysoupConvex*>(itr->mConvex);

         if (chunkc->getObject() != mOwner)
            continue;

         if (chunkc->mesh != mesh)
            continue;

         if (chunkc->idx != curIdx)
            continue;

         // A match! Don't need to add it.
         gotMatch = true;
         break;
      }

      if (gotMatch)
         continue;

      // Get the triangle...
      mesh->mOptTree->GetMeshInterface()->GetTriangle(vp, idx[i]);

      Point3F a(vp.Vertex[0]->x, vp.Vertex[0]->y, vp.Vertex[0]->z);
      Point3F b(vp.Vertex[1]->x, vp.Vertex[1]->y, vp.Vertex[1]->z);
      Point3F c(vp.Vertex[2]->x, vp.Vertex[2]->y, vp.Vertex[2]->z);

      // Transform the result into object space!
      meshToObjectMat.mulP(a);
      meshToObjectMat.mulP(b);
      meshToObjectMat.mulP(c);

      //If we're not doing debug rendering on the client, then set up our convex list as normal
      PlaneF p(c, b, a);
      Point3F peak = ((a + b + c) / 3.0f) - (p * 0.15f);

      // Set up the convex...
      MeshColliderPolysoupConvex *cp = new MeshColliderPolysoupConvex();

      list->registerObject(cp);
      convex->addToWorkingList(cp);

      cp->mesh = mesh;
      cp->idx = curIdx;
      cp->mObject = mOwner;

      cp->normal = p;
      cp->verts[0] = a;
      cp->verts[1] = b;
      cp->verts[2] = c;
      cp->verts[3] = peak;

      // Update the bounding box.
      Box3F &bounds = cp->box;
      bounds.minExtents.set(F32_MAX, F32_MAX, F32_MAX);
      bounds.maxExtents.set(-F32_MAX, -F32_MAX, -F32_MAX);

      bounds.minExtents.setMin(a);
      bounds.minExtents.setMin(b);
      bounds.minExtents.setMin(c);
      bounds.minExtents.setMin(peak);

      bounds.maxExtents.setMax(a);
      bounds.maxExtents.setMax(b);
      bounds.maxExtents.setMax(c);
      bounds.maxExtents.setMax(peak);
   }

   return true;*/
   return false;
}

bool CollisionComponent::castRayOpcode(S32 dl, const Point3F & startPos, const Point3F & endPos, RayInfo *info)
{
   // if dl==-1, nothing to do
   //if (dl == -1 || !getShapeInstance())
      return false;

   /*TSShape *shape = getShapeInstance()->getShape();

   AssertFatal(dl >= 0 && dl < shape->details.size(), "TSShapeInstance::castRayOpcode");

   info->t = 100.f;

   // get subshape and object detail
   const TSDetail * detail = &shape->details[dl];
   S32 ss = detail->subShapeNum;
   if (ss < 0)
      return false;

   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;

   const MatrixF* saveMat = NULL;
   S32 start = shape->subShapeFirstObject[ss];
   S32 end = shape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF mat;
      const MatrixF * previousMat = &getShapeInstance()->mMeshObjects[start].getTransform();
      mat = *previousMat;
      mat.inverse();
      Point3F localStart, localEnd;
      mat.mulP(startPos, &localStart);
      mat.mulP(endPos, &localEnd);

      // run through objects and collide
      for (S32 i = start; i<end; i++)
      {
         TSShapeInstance::MeshObjectInstance * meshInstance = &getShapeInstance()->mMeshObjects[i];

         if (od >= meshInstance->object->numMeshes)
            continue;

         if (&meshInstance->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &meshInstance->getTransform();

            if (previousMat != NULL)
            {
               mat = *previousMat;
               mat.inverse();
               mat.mulP(startPos, &localStart);
               mat.mulP(endPos, &localEnd);
            }
         }

         // collide...
         TSMesh * mesh = meshInstance->getMesh(od);
         if (mesh && !meshInstance->forceHidden && meshInstance->visible > 0.01f)
         {
            if (castRayMeshOpcode(mesh, localStart, localEnd, info, getShapeInstance()->mMaterialList))
            {
               saveMat = previousMat;
               emitted = true;
            }
         }
      }
   }

   if (emitted)
   {
      saveMat->mulV(info->normal);
      info->point = endPos - startPos;
      info->point *= info->t;
      info->point += startPos;
   }

   return emitted;*/
}

static Point3F	texGenAxis[18] =
{
   Point3F(0,0,1), Point3F(1,0,0), Point3F(0,-1,0),
   Point3F(0,0,-1), Point3F(1,0,0), Point3F(0,1,0),
   Point3F(1,0,0), Point3F(0,1,0), Point3F(0,0,1),
   Point3F(-1,0,0), Point3F(0,1,0), Point3F(0,0,-1),
   Point3F(0,1,0), Point3F(1,0,0), Point3F(0,0,1),
   Point3F(0,-1,0), Point3F(-1,0,0), Point3F(0,0,-1)
};

bool CollisionComponent::castRayMeshOpcode(TSMesh *mesh, const Point3F &s, const Point3F &e, RayInfo *info, TSMaterialList *materials)
{
   Opcode::RayCollider ray;
   Opcode::CollisionFaces cfs;

   IceMaths::Point dir(e.x - s.x, e.y - s.y, e.z - s.z);
   const F32 rayLen = dir.Magnitude();
   IceMaths::Ray vec(Point(s.x, s.y, s.z), dir.Normalize());

   ray.SetDestination(&cfs);
   ray.SetFirstContact(false);
   ray.SetClosestHit(true);
   ray.SetPrimitiveTests(true);
   ray.SetCulling(true);
   ray.SetMaxDist(rayLen);

   AssertFatal(ray.ValidateSettings() == NULL, "invalid ray settings");

   // Do collision.
   bool safety = ray.Collide(vec, *mesh->mOptTree);
   AssertFatal(safety, "CollisionComponent::castRayOpcode - no good ray collide!");

   // If no hit, just skip out.
   if (cfs.GetNbFaces() == 0)
      return false;

   // Got a hit!
   AssertFatal(cfs.GetNbFaces() == 1, "bad");
   const Opcode::CollisionFace &face = cfs.GetFaces()[0];

   // If the cast was successful let's check if the t value is less than what we had
   // and toggle the collision boolean
   // Stupid t... i prefer coffee
   const F32 t = face.mDistance / rayLen;

   if (t < 0.0f || t > 1.0f)
      return false;

   if (t <= info->t)
   {
      info->t = t;

      // Calculate the normal.
      Opcode::VertexPointers vp;
      mesh->mOptTree->GetMeshInterface()->GetTriangle(vp, face.mFaceID);

      if (materials && vp.MatIdx >= 0 && vp.MatIdx < materials->size())
         info->material = materials->getMaterialInst(vp.MatIdx);

      // Get the two edges.
      IceMaths::Point baseVert = *vp.Vertex[0];
      IceMaths::Point a = *vp.Vertex[1] - baseVert;
      IceMaths::Point b = *vp.Vertex[2] - baseVert;

      IceMaths::Point n;
      n.Cross(a, b);
      n.Normalize();

      info->normal.set(n.x, n.y, n.z);

      // generate UV coordinate across mesh based on 
      // matching normals, this isn't done by default and is 
      // primarily of interest in matching a collision point to 
      // either a GUI control coordinate or finding a hit pixel in texture space
      if (info->generateTexCoord)
      {
         baseVert = *vp.Vertex[0];
         a = *vp.Vertex[1];
         b = *vp.Vertex[2];

         Point3F facePoint = (1.0f - face.mU - face.mV) * Point3F(baseVert.x, baseVert.y, baseVert.z)
            + face.mU * Point3F(a.x, a.y, a.z) + face.mV * Point3F(b.x, b.y, b.z);

         U32 faces[1024];
         U32 numFaces = 0;
         for (U32 i = 0; i < mesh->mOptTree->GetMeshInterface()->GetNbTriangles(); i++)
         {
            if (i == face.mFaceID)
            {
               faces[numFaces++] = i;
            }
            else
            {
               IceMaths::Point n2;

               mesh->mOptTree->GetMeshInterface()->GetTriangle(vp, i);

               baseVert = *vp.Vertex[0];
               a = *vp.Vertex[1] - baseVert;
               b = *vp.Vertex[2] - baseVert;
               n2.Cross(a, b);
               n2.Normalize();

               F32 eps = .01f;
               if (mFabs(n.x - n2.x) < eps && mFabs(n.y - n2.y) < eps && mFabs(n.z - n2.z) < eps)
               {
                  faces[numFaces++] = i;
               }
            }

            if (numFaces == 1024)
            {
               // too many faces in this collision mesh for UV generation
               return true;
            }

         }

         Point3F min(F32_MAX, F32_MAX, F32_MAX);
         Point3F max(-F32_MAX, -F32_MAX, -F32_MAX);

         for (U32 i = 0; i < numFaces; i++)
         {
            mesh->mOptTree->GetMeshInterface()->GetTriangle(vp, faces[i]);

            for (U32 j = 0; j < 3; j++)
            {
               a = *vp.Vertex[j];

               if (a.x < min.x)
                  min.x = a.x;
               if (a.y < min.y)
                  min.y = a.y;
               if (a.z < min.z)
                  min.z = a.z;

               if (a.x > max.x)
                  max.x = a.x;
               if (a.y > max.y)
                  max.y = a.y;
               if (a.z > max.z)
                  max.z = a.z;

            }

         }

         // slerp
         Point3F s = ((max - min) - (facePoint - min)) / (max - min);

         // compute axis
         S32		bestAxis = 0;
         F32      best = 0.f;

         for (U32 i = 0; i < 6; i++)
         {
            F32 dot = mDot(info->normal, texGenAxis[i * 3]);
            if (dot > best)
            {
               best = dot;
               bestAxis = i;
            }
         }

         Point3F xv = texGenAxis[bestAxis * 3 + 1];
         Point3F yv = texGenAxis[bestAxis * 3 + 2];

         S32 sv, tv;

         if (xv.x)
            sv = 0;
         else if (xv.y)
            sv = 1;
         else
            sv = 2;

         if (yv.x)
            tv = 0;
         else if (yv.y)
            tv = 1;
         else
            tv = 2;

         // handle coord translation
         if (bestAxis == 2 || bestAxis == 3)
         {
            S32 x = sv;
            sv = tv;
            tv = x;

            if (yv.z < 0)
               s[sv] = 1.f - s[sv];
         }

         if (bestAxis < 2)
         {
            if (yv.y < 0)
               s[sv] = 1.f - s[sv];
         }

         if (bestAxis > 3)
         {
            s[sv] = 1.f - s[sv];
            if (yv.z > 0)
               s[tv] = 1.f - s[tv];

         }

         // done!
         info->texCoord.set(s[sv], s[tv]);

      }

      return true;
   }

   return false;
}