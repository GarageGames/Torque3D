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

#include "platform/platform.h"

#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"
#include "scene/sceneObject.h"
#include "collision/convex.h"
#include "collision/collision.h"
#include "T3D/tsStatic.h" // TODO: We shouldn't have this dependancy!
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsCollision.h"
#include "collision/concretePolyList.h"
#include "collision/vertexPolyList.h"
#include "platform/profiler.h"

#include "opcode/Opcode.h"
#include "opcode/Ice/IceAABB.h"
#include "opcode/Ice/IcePoint.h"
#include "opcode/OPC_AABBTree.h"
#include "opcode/OPC_AABBCollider.h"

static bool gOpcodeInitialized = false;

//-------------------------------------------------------------------------------------
// Collision methods
//-------------------------------------------------------------------------------------

bool TSShapeInstance::buildPolyList(AbstractPolyList * polyList, S32 dl)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::buildPolyList");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // This detail does not have any geometry.
   if ( ss < 0 )
      return false;

   // nothing emitted yet...
   bool emitted = false;
   U32 surfaceKey = 0;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF initialMat;
      Point3F initialScale;
      polyList->getTransform(&initialMat,&initialScale);

      // set up for first object's node
      MatrixF mat;
      MatrixF scaleMat(true);
      F32* p = scaleMat;
      p[0]  = initialScale.x;
      p[5]  = initialScale.y;
      p[10] = initialScale.z;
      const MatrixF *previousMat = &mMeshObjects[start].getTransform();
      mat.mul(initialMat,scaleMat);
      mat.mul(*previousMat);
      polyList->setTransform(&mat,Point3F(1, 1, 1));

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat.mul(initialMat,scaleMat);
               mat.mul(*previousMat);
               polyList->setTransform(&mat,Point3F(1, 1, 1));
            }
         }
         // collide...
         emitted |= mesh->buildPolyList(od,polyList,surfaceKey,mMaterialList);
      }

      // restore original transform...
      polyList->setTransform(&initialMat,initialScale);
   }

   return emitted;
}

bool TSShapeInstance::getFeatures(const MatrixF& mat, const Point3F& n, ConvexFeature* cf, S32 dl)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::buildPolyList");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;
   U32 surfaceKey = 0;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF final;
      const MatrixF* previousMat = &mMeshObjects[start].getTransform();
      final.mul(mat, *previousMat);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            previousMat = &mesh->getTransform();
            final.mul(mat, *previousMat);
         }
         emitted |= mesh->getFeatures(od, final, n, cf, surfaceKey);
      }
   }
   return emitted;
}

bool TSShapeInstance::castRay(const Point3F & a, const Point3F & b, RayInfo * rayInfo, S32 dl)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::castRay");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // This detail has no geometry to hit.
   if ( ss < 0 )
      return false;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   RayInfo saveRay;
   saveRay.t = 1.0f;
   const MatrixF * saveMat = NULL;
   bool found = false;
   if (start<end)
   {
      Point3F ta, tb;

      // set up for first object's node
      MatrixF mat;
      const MatrixF * previousMat = &mMeshObjects[start].getTransform();
      mat = *previousMat;
      mat.inverse();
      mat.mulP(a,&ta);
      mat.mulP(b,&tb);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat = *previousMat;
               mat.inverse();
               mat.mulP(a,&ta);
               mat.mulP(b,&tb);
            }
         }
         // collide...
         if (mesh->castRay(od,ta,tb,rayInfo, mMaterialList))
         {
            if (!rayInfo)
               return true;

            if (rayInfo->t <= saveRay.t)
            {
               saveRay = *rayInfo;
               saveMat = previousMat;
            }
            found = true;
         }
      }
   }

   // collide with any skins for this detail level...
   // TODO: if ever...

   // finalize the deal...
   if (found)
   {
      *rayInfo = saveRay;
      saveMat->mulV(rayInfo->normal);
      rayInfo->point  = b-a;
      rayInfo->point *= rayInfo->t;
      rayInfo->point += a;
   }
   return found;
}

bool TSShapeInstance::castRayRendered(const Point3F & a, const Point3F & b, RayInfo * rayInfo, S32 dl)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::castRayRendered");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   if ( ss == -1 )
      return false;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   RayInfo saveRay;
   saveRay.t = 1.0f;
   const MatrixF * saveMat = NULL;
   bool found = false;
   if (start<end)
   {
      Point3F ta, tb;

      // set up for first object's node
      MatrixF mat;
      const MatrixF * previousMat = &mMeshObjects[start].getTransform();
      mat = *previousMat;
      mat.inverse();
      mat.mulP(a,&ta);
      mat.mulP(b,&tb);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat = *previousMat;
               mat.inverse();
               mat.mulP(a,&ta);
               mat.mulP(b,&tb);
            }
         }
         // collide...
         if (mesh->castRayRendered(od,ta,tb,rayInfo, mMaterialList))
         {
            if (!rayInfo)
               return true;

            if (rayInfo->t <= saveRay.t)
            {
               saveRay = *rayInfo;
               saveMat = previousMat;
            }
            found = true;
         }
      }
   }

   // collide with any skins for this detail level...
   // TODO: if ever...

   // finalize the deal...
   if (found)
   {
      *rayInfo = saveRay;
      saveMat->mulV(rayInfo->normal);
      rayInfo->point  = b-a;
      rayInfo->point *= rayInfo->t;
      rayInfo->point += a;
   }
   return found;
}

Point3F TSShapeInstance::support(const Point3F & v, S32 dl)
{
   // if dl==-1, nothing to do
   AssertFatal(dl != -1, "Error, should never try to collide with a non-existant detail level!");
   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::support");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;

   F32     currMaxDP   = -1e9f;
   Point3F currSupport = Point3F(0, 0, 0);
   const MatrixF * previousMat = NULL;
   MatrixF mat;
   if (start<end)
   {
      Point3F va;

      // set up for first object's node
      previousMat = &mMeshObjects[start].getTransform();
      mat = *previousMat;
      mat.inverse();

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         TSMesh* physMesh = mesh->getMesh(od);
         if (physMesh && !mesh->forceHidden && mesh->visible > 0.01f)
         {
            // collide...
            if (&mesh->getTransform() != previousMat)
            {
               // different node from before, set up for this node
               previousMat = &mesh->getTransform();
               mat = *previousMat;
               mat.inverse();
            }
            mat.mulV(v, &va);
            physMesh->support(mesh->frame, va, &currMaxDP, &currSupport);
         }
      }
   }

   if (currMaxDP != -1e9f)
   {
      previousMat->mulP(currSupport);
      return currSupport;
   }
   else
   {
      return Point3F(0, 0, 0);
   }
}

void TSShapeInstance::computeBounds(S32 dl, Box3F & bounds)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::computeBounds");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // use shape bounds for imposter details
   if (ss < 0)
   {
      bounds = mShape->bounds;
      return;
   }

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;

   // run through objects and updating bounds as we go
   bounds.minExtents.set( 10E30f, 10E30f, 10E30f);
   bounds.maxExtents.set(-10E30f,-10E30f,-10E30f);
   Box3F box;
   for (S32 i=start; i<end; i++)
   {
      MeshObjectInstance * mesh = &mMeshObjects[i];

      if (od >= mesh->object->numMeshes)
         continue;

      if (mesh->getMesh(od))
      {
         mesh->getMesh(od)->computeBounds(mesh->getTransform(),box, 0); // use frame 0 so TSSkinMesh uses skinned verts to compute bounds
         bounds.minExtents.setMin(box.minExtents);
         bounds.maxExtents.setMax(box.maxExtents);
      }
   }
}

//-------------------------------------------------------------------------------------
// Object (MeshObjectInstance & PluginObjectInstance) collision methods
//-------------------------------------------------------------------------------------

bool TSShapeInstance::ObjectInstance::buildPolyList(S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials )
{
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( polyList );
   TORQUE_UNUSED( surfaceKey );
   TORQUE_UNUSED( materials );

   AssertFatal(0,"TSShapeInstance::ObjectInstance::buildPolyList:  no default method.");
   return false;
}

bool TSShapeInstance::ObjectInstance::getFeatures(S32 objectDetail, const MatrixF& mat, const Point3F& n, ConvexFeature* cf, U32& surfaceKey)
{
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( mat );
   TORQUE_UNUSED( n );
   TORQUE_UNUSED( cf );
   TORQUE_UNUSED( surfaceKey );

   AssertFatal(0,"TSShapeInstance::ObjectInstance::buildPolyList:  no default method.");
   return false;
}

void TSShapeInstance::ObjectInstance::support(S32, const Point3F&, F32*, Point3F*)
{
   AssertFatal(0,"TSShapeInstance::ObjectInstance::supprt:  no default method.");
}

bool TSShapeInstance::ObjectInstance::castRay( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *rayInfo, TSMaterialList *materials )
{
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( start );
   TORQUE_UNUSED( end );
   TORQUE_UNUSED( rayInfo );

   AssertFatal(0,"TSShapeInstance::ObjectInstance::castRay:  no default method.");
   return false;
}

bool TSShapeInstance::MeshObjectInstance::buildPolyList( S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials )
{
   TSMesh * mesh = getMesh(objectDetail);
   if (mesh && !forceHidden && visible>0.01f)
      return mesh->buildPolyList(frame,polyList,surfaceKey,materials);
   return false;
}

bool TSShapeInstance::MeshObjectInstance::getFeatures(S32 objectDetail, const MatrixF& mat, const Point3F& n, ConvexFeature* cf, U32& surfaceKey)
{
   TSMesh* mesh = getMesh(objectDetail);
   if (mesh && !forceHidden && visible > 0.01f)
      return mesh->getFeatures(frame, mat, n, cf, surfaceKey);
   return false;
}

void TSShapeInstance::MeshObjectInstance::support(S32 objectDetail, const Point3F& v, F32* currMaxDP, Point3F* currSupport)
{
   TSMesh* mesh = getMesh(objectDetail);
   if (mesh && !forceHidden && visible > 0.01f)
      mesh->support(frame, v, currMaxDP, currSupport);
}


bool TSShapeInstance::MeshObjectInstance::castRay( S32 objectDetail, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials )
{
   TSMesh* mesh = getMesh( objectDetail );
   if( mesh && !forceHidden && visible > 0.01f )
      return mesh->castRay( frame, start, end, rayInfo, materials );
   return false;
}

bool TSShapeInstance::MeshObjectInstance::castRayRendered( S32 objectDetail, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials )
{
   TSMesh* mesh = getMesh( objectDetail );
   if( mesh && !forceHidden && visible > 0.01f )
      return mesh->castRayRendered( frame, start, end, rayInfo, materials );
   return false;
}

bool TSShapeInstance::ObjectInstance::castRayOpcode( S32 objectDetail, const Point3F & start, const Point3F & end, RayInfo *rayInfo, TSMaterialList* materials )
{
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( start );
   TORQUE_UNUSED( end );
   TORQUE_UNUSED( rayInfo );
   TORQUE_UNUSED( materials );

   return false;
}

bool TSShapeInstance::ObjectInstance::buildPolyListOpcode( S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials )
{
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( polyList );
   TORQUE_UNUSED( surfaceKey );
   TORQUE_UNUSED( materials );

   return false;
}

bool TSShapeInstance::ObjectInstance::buildConvexOpcode( const MatrixF &mat, S32 objectDetail, const Box3F &bounds, Convex *c, Convex *list )
{
   TORQUE_UNUSED( mat );
   TORQUE_UNUSED( objectDetail );
   TORQUE_UNUSED( bounds );
   TORQUE_UNUSED( c );
   TORQUE_UNUSED( list );

   return false;
}

bool TSShapeInstance::MeshObjectInstance::castRayOpcode( S32 objectDetail, const Point3F & start, const Point3F & end, RayInfo *info, TSMaterialList* materials )
{
   TSMesh * mesh = getMesh(objectDetail);
   if (mesh && !forceHidden && visible>0.01f)
      return mesh->castRayOpcode(start, end, info, materials);
   return false;
}

bool TSShapeInstance::MeshObjectInstance::buildPolyListOpcode( S32 objectDetail, AbstractPolyList *polyList, const Box3F &box, TSMaterialList *materials )
{
   TSMesh * mesh = getMesh(objectDetail);
   if ( mesh && !forceHidden && visible > 0.01f && box.isOverlapped( mesh->getBounds() ) )
      return mesh->buildPolyListOpcode(frame,polyList,box,materials);
   return false;
}

bool TSShapeInstance::MeshObjectInstance::buildConvexOpcode( const MatrixF &mat, S32 objectDetail, const Box3F &bounds, Convex *c, Convex *list)
{
   TSMesh * mesh = getMesh(objectDetail);
   if ( mesh && !forceHidden && visible > 0.01f && bounds.isOverlapped( mesh->getBounds() ) )
      return mesh->buildConvexOpcode(mat, bounds, c, list);
   return false;
}

bool TSShapeInstance::buildPolyListOpcode( S32 dl, AbstractPolyList *polyList, const Box3F &box )
{
   PROFILE_SCOPE( TSShapeInstance_buildPolyListOpcode_MeshObjInst );

   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::buildPolyListOpcode");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   if ( ss < 0 )
      return false;

   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF initialMat;
      Point3F initialScale;
      polyList->getTransform(&initialMat,&initialScale);

      // set up for first object's node
      MatrixF mat;
      MatrixF scaleMat(true);
      F32* p = scaleMat;
      p[0]  = initialScale.x;
      p[5]  = initialScale.y;
      p[10] = initialScale.z;
      const MatrixF * previousMat = &mMeshObjects[start].getTransform();
      mat.mul(initialMat,scaleMat);
      mat.mul(*previousMat);
      polyList->setTransform(&mat,Point3F(1, 1, 1));

      // Update our bounding box...
      Box3F localBox = box;
      MatrixF otherMat = mat;
      otherMat.inverse();
      otherMat.mul(localBox);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat.mul(initialMat,scaleMat);
               mat.mul(*previousMat);
               polyList->setTransform(&mat,Point3F(1, 1, 1));

               // Update our bounding box...
               otherMat = mat;
               otherMat.inverse();
               localBox = box;
               otherMat.mul(localBox);
            }
         }

         // collide...
         emitted |= mesh->buildPolyListOpcode(od,polyList,localBox,mMaterialList);
      }

      // restore original transform...
      polyList->setTransform(&initialMat,initialScale);
   }

   return emitted;
}

bool TSShapeInstance::castRayOpcode( S32 dl, const Point3F & startPos, const Point3F & endPos, RayInfo *info)
{
   // if dl==-1, nothing to do
   if (dl==-1)
      return false;

   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::castRayOpcode");

   info->t = 100.f;

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   if ( ss < 0 )
      return false;

   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;

   const MatrixF* saveMat = NULL;
   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF mat;
      const MatrixF * previousMat = &mMeshObjects[start].getTransform();
      mat = *previousMat;
      mat.inverse();
      Point3F localStart, localEnd;
      mat.mulP(startPos, &localStart);
      mat.mulP(endPos, &localEnd);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat = *previousMat;
               mat.inverse();
               mat.mulP(startPos, &localStart);
               mat.mulP(endPos, &localEnd);
            }
         }

         // collide...
         if ( mesh->castRayOpcode(od,localStart, localEnd, info, mMaterialList) )
         {
            saveMat = previousMat;
            emitted = true;
         }
      }
   }

   if ( emitted )
   {
      saveMat->mulV(info->normal);
      info->point  = endPos - startPos;
      info->point *= info->t;
      info->point += startPos;
   }

   return emitted;
}

bool TSShapeInstance::buildConvexOpcode( const MatrixF &objMat, const Point3F &objScale, S32 dl, const Box3F &bounds, Convex *c, Convex *list )
{
   AssertFatal(dl>=0 && dl<mShape->details.size(),"TSShapeInstance::buildConvexOpcode");

   // get subshape and object detail
   const TSDetail * detail = &mShape->details[dl];
   S32 ss = detail->subShapeNum;
   S32 od = detail->objectDetailNum;

   // nothing emitted yet...
   bool emitted = false;

   S32 start = mShape->subShapeFirstObject[ss];
   S32 end   = mShape->subShapeNumObjects[ss] + start;
   if (start<end)
   {
      MatrixF initialMat = objMat;
      Point3F initialScale = objScale;

      // set up for first object's node
      MatrixF mat;
      MatrixF scaleMat(true);
      F32* p = scaleMat;
      p[0]  = initialScale.x;
      p[5]  = initialScale.y;
      p[10] = initialScale.z;
      const MatrixF * previousMat = &mMeshObjects[start].getTransform();
      mat.mul(initialMat,scaleMat);
      mat.mul(*previousMat);

      // Update our bounding box...
      Box3F localBox = bounds;
      MatrixF otherMat = mat;
      otherMat.inverse();
      otherMat.mul(localBox);

      // run through objects and collide
      for (S32 i=start; i<end; i++)
      {
         MeshObjectInstance * mesh = &mMeshObjects[i];

         if (od >= mesh->object->numMeshes)
            continue;

         if (&mesh->getTransform() != previousMat)
         {
            // different node from before, set up for this node
            previousMat = &mesh->getTransform();

            if (previousMat != NULL)
            {
               mat.mul(initialMat,scaleMat);
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
         emitted |= mesh->buildConvexOpcode(*previousMat,od,localBox,c, list);
      }
   }

   return emitted;
}

void TSShape::findColDetails( bool useVisibleMesh, Vector<S32> *outDetails, Vector<S32> *outLOSDetails ) const
{
   PROFILE_SCOPE( TSShape_findColDetails );

   if ( useVisibleMesh )
   {
      // If we're using the visible mesh for collision then
      // find the highest detail and use that.

      U32 highestDetail = -1;
      F32 highestSize = -F32_MAX;

      for ( U32 i = 0; i < details.size(); i++ )
      {
         // Make sure we skip any details that shouldn't be rendered
         if ( details[i].size < 0 )
            continue;

         /*
         // Also make sure we skip any collision details with a size.
         const String &name = names[details[i].nameIndex];
         if (  dStrStartsWith( name, "Collision" ) ||
               dStrStartsWith( name, "LOS" ) )
            continue;
         */

         // Otherwise test against the current highest size
         if ( details[i].size > highestSize )
         {
            highestDetail = i;
            highestSize = details[i].size;
         }
      }

      // We use the same detail for both raycast and collisions.
      if ( highestDetail != -1 )
      {
         outDetails->push_back( highestDetail );
         if ( outLOSDetails )
            outLOSDetails->push_back( highestDetail );
      }

      return;
   }

   // Detail meshes starting with COL or LOS is considered
   // to be a collision mesh.
   //
   // The LOS (light of sight) details are used for raycasts.

   for ( U32 i = 0; i < details.size(); i++ )
   {
      const String &name = names[ details[i].nameIndex ];
      if ( !dStrStartsWith( name, "Collision" ) )
         continue;

      outDetails->push_back(i);

      // If we're not returning LOS details then skip out.
      if ( !outLOSDetails )
         continue;

      // The way LOS works is that it will check to see if there is a LOS detail that matches
      // the the collision detail + 1 + MaxCollisionShapes (this variable name should change in
      // the future). If it can't find a matching LOS it will simply use the collision instead.
      // We check for any "unmatched" LOS's further down.

      // Extract the detail number from the name.
      S32 number = 0;
      String::GetTrailingNumber( name, number );
      
      // Look for a matching LOS collision detail.
      //
      // TODO: Fix the old 9 detail offset which is there
      // because you cannot have two detail markers with
      // the same detail number.
      //
      const S32 LOSOverrideOffset = 9;
      String buff = String::ToString( "LOS-%d", mAbs( number ) + LOSOverrideOffset );
      S32 los = findDetail( buff );
      
      // If we didn't find the lod detail then use the
      // normal collision detail for LOS tests.
      if ( los == -1 )
         los = i;

      outLOSDetails->push_back( los );
   }

   // If we're not returning LOS details then skip out.
   if ( !outLOSDetails )
      return;

   // Snag any "unmatched" LOS details and put 
   // them at the end of the list.
   for ( U32 i = 0; i < details.size(); i++ )
   {
      const String &name = names[ details[i].nameIndex ];
      if ( !dStrStartsWith( name, "LOS" ) )
         continue;

      // See if we already have this LOS
      bool found = false;
      for (U32 j = 0; j < outLOSDetails->size(); j++)
      {
         if ( (*outLOSDetails)[j] == i )
         {
            found = true;
            break;
         }
      }

      if ( !found )
         outLOSDetails->push_back(i);
   }
}

PhysicsCollision* TSShape::buildColShape( bool useVisibleMesh, const Point3F &scale )
{
   return _buildColShapes( useVisibleMesh, scale, NULL, false );
}

void TSShape::buildColShapes( bool useVisibleMesh, const Point3F &scale, Vector< CollisionShapeInfo > *list )
{
   _buildColShapes( useVisibleMesh, scale, list, true );
}

PhysicsCollision* TSShape::_buildColShapes( bool useVisibleMesh, const Point3F &scale, Vector< CollisionShapeInfo > *list, bool perMesh )
{
   PROFILE_SCOPE( TSShape_buildColShapes );

   if ( !PHYSICSMGR )
      return NULL;

   PhysicsCollision *colShape = NULL;
   U32 surfaceKey = 0;

   if ( useVisibleMesh )
   {
      // Here we build triangle collision meshes from the
      // visible detail levels.

      // A negative subshape on the detail means we don't have geometry.
      const TSShape::Detail &detail = details[0];     
      if ( detail.subShapeNum < 0 )
         return NULL;

      // We don't try to optimize the triangles we're given
      // and assume the art was created properly for collision.
      ConcretePolyList polyList;
      polyList.setTransform( &MatrixF::Identity, scale );

      // Create the collision meshes.
      S32 start = subShapeFirstObject[ detail.subShapeNum ];
      S32 end = start + subShapeNumObjects[ detail.subShapeNum ];
      for ( S32 o=start; o < end; o++ )
      {
         const TSShape::Object &object = objects[o];
         if ( detail.objectDetailNum >= object.numMeshes )
            continue;

         // No mesh or no verts.... nothing to do.
         TSMesh *mesh = meshes[ object.startMeshIndex + detail.objectDetailNum ];
         if ( !mesh || mesh->mNumVerts == 0 )
            continue;

         // Gather the mesh triangles.
         polyList.clear();
         mesh->buildPolyList( 0, &polyList, surfaceKey, NULL );

         // Create the collision shape if we haven't already.
         if ( !colShape )
            colShape = PHYSICSMGR->createCollision();

         // Get the object space mesh transform.
         MatrixF localXfm;
         getNodeWorldTransform( object.nodeIndex, &localXfm );

         colShape->addTriangleMesh( polyList.mVertexList.address(),
            polyList.mVertexList.size(),
            polyList.mIndexList.address(),
            polyList.mIndexList.size() / 3,
            localXfm );

         if ( perMesh )
         {
            list->increment();
            list->last().colNode = -1;
            list->last().colShape = colShape;
            colShape = NULL;
         }
      }

      // Return what we built... if anything.
      return colShape;
   }


   // Scan out the collision hulls...
   //
   // TODO: We need to support LOS collision for physics.
   //
   for ( U32 i = 0; i < details.size(); i++ )
   {
      const TSShape::Detail &detail = details[i];
      const String &name = names[detail.nameIndex];

      // Is this a valid collision detail.
      if ( !dStrStartsWith( name, "Collision" ) || detail.subShapeNum < 0 )
         continue;

      // Now go thru the meshes for this detail.
      S32 start = subShapeFirstObject[ detail.subShapeNum ];
      S32 end = start + subShapeNumObjects[ detail.subShapeNum ];
      if ( start >= end )
         continue;         

      for ( S32 o=start; o < end; o++ )
      {
         const TSShape::Object &object = objects[o];
         const String &meshName = names[ object.nameIndex ];

         if ( object.numMeshes <= detail.objectDetailNum )
            continue;

         // No mesh, a flat bounds, or no verts.... nothing to do.
         TSMesh *mesh = meshes[ object.startMeshIndex + detail.objectDetailNum ];
         if ( !mesh || mesh->getBounds().isEmpty() || mesh->mNumVerts == 0 )
            continue;

         // We need the default mesh transform.
         MatrixF localXfm;
         getNodeWorldTransform( object.nodeIndex, &localXfm );

         // We have some sort of collision shape... so allocate it.
         if ( !colShape )
            colShape = PHYSICSMGR->createCollision();

         // We have geometry... what is it?
         if ( dStrStartsWith( meshName, "Colbox" ) )
         {
            // The bounds define the box extents directly.
            Point3F halfWidth = mesh->getBounds().getExtents() * 0.5f;

            // Add the offset to the center of the bounds 
            // into the local space transform.
            MatrixF centerXfm( true );
            centerXfm.setPosition( mesh->getBounds().getCenter() );
            localXfm.mul( centerXfm );

            colShape->addBox( halfWidth, localXfm );
         }
         else if ( dStrStartsWith( meshName, "Colsphere" ) )
         {
            // Get a sphere inscribed to the bounds.
            F32 radius = mesh->getBounds().len_min() * 0.5f;

            // Add the offset to the center of the bounds 
            // into the local space transform.
            MatrixF primXfm( true );
            primXfm.setPosition( mesh->getBounds().getCenter() );
            localXfm.mul( primXfm );

            colShape->addSphere( radius, localXfm );
         }
         else if ( dStrStartsWith( meshName, "Colcapsule" ) )
         {
            // Use the smallest extent as the radius for the capsule.
            Point3F extents = mesh->getBounds().getExtents();
            F32 radius = extents.least() * 0.5f;

            // We need to center the capsule and align it to the Y axis.
            MatrixF primXfm( true );
            primXfm.setPosition( mesh->getBounds().getCenter() );

            // Use the longest axis as the capsule height.
            F32 height = -radius * 2.0f;
            if ( extents.x > extents.y && extents.x > extents.z )
            {
               primXfm.setColumn( 0, Point3F( 0, 0, 1 ) );
               primXfm.setColumn( 1, Point3F( 1, 0, 0 ) );
               primXfm.setColumn( 2, Point3F( 0, 1, 0 ) );
               height += extents.x;
            }
            else if ( extents.z > extents.x && extents.z > extents.y )
            {
               primXfm.setColumn( 0, Point3F( 0, 1, 0 ) );
               primXfm.setColumn( 1, Point3F( 0, 0, 1 ) );
               primXfm.setColumn( 2, Point3F( 1, 0, 0 ) );
               height += extents.z;
            }
            else
               height += extents.y;

            // Add the primitive transform into the local transform.
            localXfm.mul( primXfm );

            // If we didn't find a positive height then fallback to
            // creating a sphere which is better than nothing.
            if ( height > 0.0f )
               colShape->addCapsule( radius, height, localXfm );
            else
               colShape->addSphere( radius, localXfm );
         }
         else if ( dStrStartsWith( meshName, "Colmesh" ) )
         {
            // For a triangle mesh we gather the triangles raw from the
            // mesh and don't do any optimizations or cleanup.
            ConcretePolyList polyList;
            polyList.setTransform( &MatrixF::Identity, scale );
            mesh->buildPolyList( 0, &polyList, surfaceKey, NULL );
            colShape->addTriangleMesh( polyList.mVertexList.address(), 
                                       polyList.mVertexList.size(),
                                       polyList.mIndexList.address(),
                                       polyList.mIndexList.size() / 3,
                                       localXfm );
         }
         else
         {
            // Any other mesh name we assume as a generic convex hull.
            //
            // Collect the verts using the vertex polylist which will 
            // filter out duplicates.  This is importaint as the convex
            // generators can sometimes fail with duplicate verts.
            //
            VertexPolyList polyList;
            MatrixF meshMat( localXfm );

            Point3F t = meshMat.getPosition();
            t.convolve( scale );
            meshMat.setPosition( t );            

            polyList.setTransform( &MatrixF::Identity, scale );
            mesh->buildPolyList( 0, &polyList, surfaceKey, NULL );
            colShape->addConvex( polyList.getVertexList().address(), 
                                 polyList.getVertexList().size(),
                                 meshMat );
         }

         if ( perMesh )
         {
            list->increment();
            
            S32 detailNum;
            String::GetTrailingNumber( name, detailNum );            
            
            String str = String::ToString( "%s%i", meshName.c_str(), detailNum );
            S32 found = findNode( str );

            if ( found == -1 )
            {
               str = str.replace('-','_');
               found = findNode( str );
            }

            list->last().colNode = found;            
            list->last().colShape = colShape;

            colShape = NULL;
         }

      } // objects

   } // details

   return colShape;
}

bool TSMesh::buildPolyListOpcode( const S32 od, AbstractPolyList *polyList, const Box3F &nodeBox, TSMaterialList *materials )
{
   PROFILE_SCOPE( TSMesh_buildPolyListOpcode );

   // This is small... there is no win for preallocating it.
   Opcode::AABBCollider opCollider;
   opCollider.SetPrimitiveTests( true );

   // This isn't really needed within the AABBCollider as 
   // we don't use temporal coherance... use a static to 
   // remove the allocation overhead.
   static Opcode::AABBCache opCache;

   IceMaths::AABB opBox;
   opBox.SetMinMax(  Point( nodeBox.minExtents.x, nodeBox.minExtents.y, nodeBox.minExtents.z ),
                     Point( nodeBox.maxExtents.x, nodeBox.maxExtents.y, nodeBox.maxExtents.z ) );
   Opcode::CollisionAABB opCBox(opBox);

   if ( !opCollider.Collide( opCache, opCBox, *mOptTree ) )
      return false;

   U32 count = opCollider.GetNbTouchedPrimitives();
   const udword *idx = opCollider.GetTouchedPrimitives();

   Opcode::VertexPointers vp;
   U32 plIdx[3];
   S32 j;
   Point3F tmp;
   const IceMaths::Point **verts;
   const	Opcode::MeshInterface *mi = mOptTree->GetMeshInterface();

   for ( S32 i=0; i < count; i++ )
   {
      // Get the triangle...
      mi->GetTriangle( vp, idx[i] );
      verts = vp.Vertex;

      // And register it in the polylist...
      polyList->begin( NULL, i );

      for( j = 2; j > -1; j-- )
      {
         tmp.set( verts[j]->x, verts[j]->y, verts[j]->z );
         plIdx[j] = polyList->addPoint( tmp );
         polyList->vertex( plIdx[j] );
      }

      polyList->plane( plIdx[0], plIdx[2], plIdx[1] );

      polyList->end();
   }

   // TODO: Add a polyList->getCount() so we can see if we
   // got clipped polys and didn't really emit anything.
   return count > 0;
}

bool TSMesh::buildConvexOpcode( const MatrixF &meshToObjectMat, const Box3F &nodeBox, Convex *convex, Convex *list )
{
   PROFILE_SCOPE( TSMesh_buildConvexOpcode );

   // This is small... there is no win for preallocating it.
   Opcode::AABBCollider opCollider;
   opCollider.SetPrimitiveTests( true );

   // This isn't really needed within the AABBCollider as 
   // we don't use temporal coherance... use a static to 
   // remove the allocation overhead.
   static Opcode::AABBCache opCache;

   IceMaths::AABB opBox;
   opBox.SetMinMax(  Point( nodeBox.minExtents.x, nodeBox.minExtents.y, nodeBox.minExtents.z ),
                     Point( nodeBox.maxExtents.x, nodeBox.maxExtents.y, nodeBox.maxExtents.z ) );
   Opcode::CollisionAABB opCBox(opBox);

   if( !opCollider.Collide( opCache, opCBox, *mOptTree ) )
      return false;

   U32 cnt = opCollider.GetNbTouchedPrimitives();
   const udword *idx = opCollider.GetTouchedPrimitives();

   Opcode::VertexPointers vp;
   for ( S32 i = 0; i < cnt; i++ )
   {
      // First, check our active convexes for a potential match (and clean things
      // up, too.)
      const U32 curIdx = idx[i];

      // See if the square already exists as part of the working set.
      bool gotMatch = false;
      CollisionWorkingList& wl = convex->getWorkingList();
      for ( CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext )
      {
         if( itr->mConvex->getType() != TSPolysoupConvexType )
            continue;

         const TSStaticPolysoupConvex *chunkc = static_cast<TSStaticPolysoupConvex*>( itr->mConvex );

         if( chunkc->getObject() != TSStaticPolysoupConvex::smCurObject )
            continue;
               
         if( chunkc->mesh != this )
            continue;

         if( chunkc->idx != curIdx )
            continue;

         // A match! Don't need to add it.
         gotMatch = true;
         break;
      }

      if( gotMatch )
         continue;

      // Get the triangle...
      mOptTree->GetMeshInterface()->GetTriangle( vp, idx[i] );

      Point3F a( vp.Vertex[0]->x, vp.Vertex[0]->y, vp.Vertex[0]->z );
      Point3F b( vp.Vertex[1]->x, vp.Vertex[1]->y, vp.Vertex[1]->z );
      Point3F c( vp.Vertex[2]->x, vp.Vertex[2]->y, vp.Vertex[2]->z );

      // Transform the result into object space!
      meshToObjectMat.mulP( a );
      meshToObjectMat.mulP( b );
      meshToObjectMat.mulP( c );

      PlaneF p( c, b, a );
      Point3F peak = ((a + b + c) / 3.0f) - (p * 0.15f);

      // Set up the convex...
      TSStaticPolysoupConvex *cp = new TSStaticPolysoupConvex();

      list->registerObject( cp );
      convex->addToWorkingList( cp );

      cp->mesh    = this;
      cp->idx     = curIdx;
      cp->mObject = TSStaticPolysoupConvex::smCurObject;

      cp->normal = p;
      cp->verts[0] = a;
      cp->verts[1] = b;
      cp->verts[2] = c;
      cp->verts[3] = peak;

      // Update the bounding box.
      Box3F &bounds = cp->box;
      bounds.minExtents.set( F32_MAX,  F32_MAX,  F32_MAX );
      bounds.maxExtents.set( -F32_MAX, -F32_MAX, -F32_MAX );

      bounds.minExtents.setMin( a );
      bounds.minExtents.setMin( b );
      bounds.minExtents.setMin( c );
      bounds.minExtents.setMin( peak );

      bounds.maxExtents.setMax( a );
      bounds.maxExtents.setMax( b );
      bounds.maxExtents.setMax( c );
      bounds.maxExtents.setMax( peak );
   }

   return true;
}

void TSMesh::prepOpcodeCollision()
{
   // Make sure opcode is loaded!
   if( !gOpcodeInitialized )
   {
      Opcode::InitOpcode();
      gOpcodeInitialized = true;
   }

   // Don't re init if we already have something...
   if ( mOptTree )
      return;

   // Ok, first set up a MeshInterface
   Opcode::MeshInterface *mi = new Opcode::MeshInterface();
   mOpMeshInterface = mi;

   // Figure out how many triangles we have...
   U32 triCount = 0;
   const U32 base = 0;
   for ( U32 i = 0; i < primitives.size(); i++ )
   {
      TSDrawPrimitive & draw = primitives[i];
      const U32 start = draw.start;

      AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::buildPolyList (1)" );

      // gonna depend on what kind of primitive it is...
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
         triCount += draw.numElements / 3;
      else
      {
         // Have to walk the tristrip to get a count... may have degenerates
         U32 idx0 = base + indices[start + 0];
         U32 idx1;
         U32 idx2 = base + indices[start + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            //            nextIdx = (j%2)==0 ? &idx0 : &idx1;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = base + indices[start + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

            triCount++;
         }
      }
   }

   // Just do the first trilist for now.
   mi->SetNbVertices( mVertexData.isReady() ? mNumVerts : verts.size() );
   mi->SetNbTriangles( triCount );

   // Stuff everything into appropriate arrays.
   IceMaths::IndexedTriangle *its = new IceMaths::IndexedTriangle[ mi->GetNbTriangles() ], *curIts = its;
   IceMaths::Point           *pts = new IceMaths::Point[ mi->GetNbVertices() ];

   mOpTris = its;
   mOpPoints = pts;

   // add the polys...
   for ( U32 i = 0; i < primitives.size(); i++ )
   {
      TSDrawPrimitive & draw = primitives[i];
      const U32 start = draw.start;

      AssertFatal( draw.matIndex & TSDrawPrimitive::Indexed,"TSMesh::buildPolyList (1)" );

      const U32 matIndex = draw.matIndex & TSDrawPrimitive::MaterialMask;

      // gonna depend on what kind of primitive it is...
      if ( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Triangles )
      {
         for ( S32 j = 0; j < draw.numElements; )
         {
            curIts->mVRef[2] = base + indices[start + j + 0];
            curIts->mVRef[1] = base + indices[start + j + 1];
            curIts->mVRef[0] = base + indices[start + j + 2];
            curIts->mMatIdx = matIndex;
            curIts++;

            j += 3;
         }
      }
      else
      {
         AssertFatal( (draw.matIndex & TSDrawPrimitive::TypeMask) == TSDrawPrimitive::Strip,"TSMesh::buildPolyList (2)" );

         U32 idx0 = base + indices[start + 0];
         U32 idx1;
         U32 idx2 = base + indices[start + 1];
         U32 * nextIdx = &idx1;
         for ( S32 j = 2; j < draw.numElements; j++ )
         {
            *nextIdx = idx2;
            //            nextIdx = (j%2)==0 ? &idx0 : &idx1;
            nextIdx = (U32*) ( (dsize_t)nextIdx ^ (dsize_t)&idx0 ^ (dsize_t)&idx1);
            idx2 = base + indices[start + j];
            if ( idx0 == idx1 || idx0 == idx2 || idx1 == idx2 )
               continue;

            curIts->mVRef[2] = idx0;
            curIts->mVRef[1] = idx1;
            curIts->mVRef[0] = idx2;
            curIts->mMatIdx = matIndex;
            curIts++;
         }
      }
   }

   AssertFatal( (curIts - its) == mi->GetNbTriangles(), "Triangle count mismatch!" );

   for( S32 i = 0; i < mi->GetNbVertices(); i++ )
   {
      if( mVertexData.isReady() )
      {
         const __TSMeshVertexBase &vertData = mVertexData.getBase(i);
         pts[i].Set( vertData.vert().x, vertData.vert().y, vertData.vert().z );
      }
      else
      {
         pts[i].Set( verts[i].x, verts[i].y, verts[i].z );
      }
   }

   mi->SetPointers( its, pts );

   // Ok, we've got a mesh interface populated, now let's build a thingy to collide against.
   mOptTree = new Opcode::Model();

   Opcode::OPCODECREATE opcc;

   opcc.mCanRemap = true;
   opcc.mIMesh = mi;
   opcc.mKeepOriginal = false;
   opcc.mNoLeaf = false;
   opcc.mQuantized = false;
   opcc.mSettings.mLimit = 1;

   mOptTree->Build( opcc );
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


bool TSMesh::castRayOpcode( const Point3F &s, const Point3F &e, RayInfo *info, TSMaterialList *materials )
{
   Opcode::RayCollider ray;
   Opcode::CollisionFaces cfs;

   IceMaths::Point dir( e.x - s.x, e.y - s.y, e.z - s.z );
   const F32 rayLen = dir.Magnitude();
   IceMaths::Ray vec( Point(s.x, s.y, s.z), dir.Normalize() );

   ray.SetDestination( &cfs);
   ray.SetFirstContact( false );
   ray.SetClosestHit( true );
   ray.SetPrimitiveTests( true );
   ray.SetCulling( true );
   ray.SetMaxDist( rayLen );

   AssertFatal( ray.ValidateSettings() == NULL, "invalid ray settings" );

   // Do collision.
   bool safety = ray.Collide( vec, *mOptTree );
   AssertFatal( safety, "TSMesh::castRayOpcode - no good ray collide!" );

   // If no hit, just skip out.
   if( cfs.GetNbFaces() == 0 )
      return false;

   // Got a hit!
   AssertFatal( cfs.GetNbFaces() == 1, "bad" );
   const Opcode::CollisionFace &face = cfs.GetFaces()[0];

   // If the cast was successful let's check if the t value is less than what we had
   // and toggle the collision boolean
   // Stupid t... i prefer coffee
   const F32 t = face.mDistance / rayLen;

   if( t < 0.0f || t > 1.0f )
      return false;

   if( t <= info->t )
   {
      info->t = t;

      // Calculate the normal.
      Opcode::VertexPointers vp;
      mOptTree->GetMeshInterface()->GetTriangle( vp, face.mFaceID );

      if ( materials && vp.MatIdx >= 0 && vp.MatIdx < materials->size() )
         info->material = materials->getMaterialInst( vp.MatIdx );

      // Get the two edges.
      IceMaths::Point baseVert = *vp.Vertex[0];
      IceMaths::Point a = *vp.Vertex[1] - baseVert;
      IceMaths::Point b = *vp.Vertex[2] - baseVert;

      IceMaths::Point n;
      n.Cross( a, b );
      n.Normalize();

      info->normal.set( n.x, n.y, n.z );

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
         for (U32 i = 0; i < mOptTree->GetMeshInterface()->GetNbTriangles(); i++)
         {
            if ( i == face.mFaceID )
            {
               faces[numFaces++] = i;
            }
            else
            {
               IceMaths::Point n2;

               mOptTree->GetMeshInterface()->GetTriangle( vp, i );

               baseVert = *vp.Vertex[0];
               a = *vp.Vertex[1] - baseVert;
               b = *vp.Vertex[2] - baseVert;
               n2.Cross( a, b );
               n2.Normalize();

               F32 eps = .01f;
               if ( mFabs(n.x - n2.x) < eps && mFabs(n.y - n2.y) < eps && mFabs(n.z - n2.z) < eps)
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
            mOptTree->GetMeshInterface()->GetTriangle( vp, faces[i] );

            for ( U32 j =0; j < 3; j++)
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
         Point3F divSafe = (max - min);
         if (divSafe.x == 0.0f) divSafe.x = POINT_EPSILON;
         if (divSafe.y == 0.0f) divSafe.y = POINT_EPSILON;
         if (divSafe.z == 0.0f) divSafe.z = POINT_EPSILON;

         Point3F s = ( (max - min) - (facePoint - min) ) / divSafe;

         // compute axis
         S32		bestAxis = 0;
         F32      best = 0.f;

         for (U32 i = 0 ; i < 6 ; i++)
         {
            F32 dot = mDot (info->normal, texGenAxis[i*3]);
            if (dot > best)
            {
               best = dot;
               bestAxis = i;
            }
         }

         Point3F xv = texGenAxis[bestAxis*3+1];
         Point3F yv = texGenAxis[bestAxis*3+2];

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
