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
#include "forest/forestCollision.h"

#include "forest/forest.h"
#include "forest/forestCell.h"
#include "forest/ts/tsForestItemData.h"
#include "ts/tsShapeInstance.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "collision/concretePolyList.h"
#include "platform/profiler.h"


/*
bool Forest::castRay( const Point3F &start, const Point3F &end, RayInfo* info )
{
   //if ( !mData )
      //return false;
   //return mData->castRay( start, end, outInfo );

   return false;
}

bool Forest::castRayI( const Point3F &start, const Point3F &end, ForestRayInfo *outInfo )
{
   if ( !mData )
      return false;

   return mData->castRay( start, end, outInfo );
}

ScriptMethod( Forest, forestRayCast, const char*, 4, 4, "( Point3F start, Point3F end )"
                "Cast a ray from start to end, checking for collision against trees in this forest.\n\n"                
                "@returns A string containing either null, if nothing was struck, or these fields:\n"
                "            - The ID of the tree type datablock.\n"
                "            - The tree ID (not a normal object id).\n" 
                "            - t, The time during the raycast at which the collision occured." )                
{

   Point3F start, end;
   dSscanf(argv[2], "%g %g %g", &start.x, &start.y, &start.z);
   dSscanf(argv[3], "%g %g %g", &end.x,   &end.y,   &end.z);   

   static const U32 bufSize = 256;
   char *returnBuffer = Con::getReturnBuffer(bufSize);
   returnBuffer[0] = '0';
   returnBuffer[1] = '\0';

   ForestRayInfo rinfo;
   if ( object->castRayI( start, end, &rinfo ) )
      dSprintf( returnBuffer, bufSize, "%d %d %g", rinfo.item->getData()->getId(), rinfo.key, rinfo.t );

   return returnBuffer;
}
*/

void ForestConvex::calculateTransform( const MatrixF &worldXfrm )
{
   mTransform = worldXfrm;
   return;
}

Box3F ForestConvex::getBoundingBox() const
{
   // This is probably a bad idea? -- BJG
   return getBoundingBox( mTransform, Point3F(mScale,mScale,mScale) );
}

Box3F ForestConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F newBox = box;
   newBox.minExtents.convolve(scale);
   newBox.maxExtents.convolve(scale);
   mat.mul(newBox);
   return newBox;
}

Point3F ForestConvex::support(const VectorF& v) const
{
   TSShapeInstance *si = mData->getShapeInstance();

   TSShape::ConvexHullAccelerator* pAccel =
      si->getShape()->getAccelerator(mData->getCollisionDetails()[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], v);
   U32 index = 0;
   for (U32 i = 1; i < pAccel->numVerts; i++) 
   {
      F32 dp = mDot(pAccel->vertexList[i], v);
      if (dp > currMaxDP) 
      {
         currMaxDP = dp;
         index = i;
      }
   }

   return pAccel->vertexList[index];
}

void ForestConvex::getFeatures( const MatrixF &mat, const VectorF &n, ConvexFeature *cf )
{
   cf->material = 0;
   cf->object = mObject;

   TSShapeInstance *si = mData->getShapeInstance();

   TSShape::ConvexHullAccelerator* pAccel =
      si->getShape()->getAccelerator(mData->getCollisionDetails()[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], n);
   U32 index = 0;
   U32 i;
   for (i = 1; i < pAccel->numVerts; i++) 
   {
      F32 dp = mDot(pAccel->vertexList[i], n);
      if (dp > currMaxDP) 
      {
         currMaxDP = dp;
         index = i;
      }
   }

   const U8* emitString = pAccel->emitStrings[index];
   U32 currPos = 0;
   U32 numVerts = emitString[currPos++];
   for (i = 0; i < numVerts; i++) 
   {
      cf->mVertexList.increment();
      U32 index = emitString[currPos++];
      mat.mulP(pAccel->vertexList[index], &cf->mVertexList.last());
   }

   U32 numEdges = emitString[currPos++];
   for (i = 0; i < numEdges; i++) 
   {
      U32 ev0 = emitString[currPos++];
      U32 ev1 = emitString[currPos++];
      cf->mEdgeList.increment();
      cf->mEdgeList.last().vertex[0] = ev0;
      cf->mEdgeList.last().vertex[1] = ev1;
   }

   U32 numFaces = emitString[currPos++];
   for (i = 0; i < numFaces; i++) 
   {
      cf->mFaceList.increment();
      U32 plane = emitString[currPos++];
      mat.mulV(pAccel->normalList[plane], &cf->mFaceList.last().normal);
      for (U32 j = 0; j < 3; j++)
         cf->mFaceList.last().vertex[j] = emitString[currPos++];
   }
}

void ForestConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform( &mTransform, Point3F(mScale,mScale,mScale));
   list->setObject(mObject);

   TSShapeInstance *si = mData->getShapeInstance();

   S32 detail = mData->getCollisionDetails()[hullId];
   si->animate(detail);
   si->buildPolyList(list, detail);
}


void Forest::buildConvex( const Box3F &box, Convex *convex )
{
   mConvexList->collectGarbage();

   // Get all ForestItem(s) within the box.
   Vector<ForestItem> trees;
   mData->getItems( box, &trees );
   if ( trees.empty() )
      return;

   for ( U32 i = 0; i < trees.size(); i++ )
   {
      const ForestItem &forestItem = trees[i];

      Box3F realBox = box;
      mWorldToObj.mul( realBox );
      realBox.minExtents.convolveInverse( mObjScale );
      realBox.maxExtents.convolveInverse( mObjScale );

      // JCF: is this really necessary if we already got this ForestItem
      // as a result from getItems?
      if ( realBox.isOverlapped( getObjBox() ) == false )
         continue;      

      TSForestItemData *data = (TSForestItemData*)forestItem.getData();

      // Find CollisionDetail(s) that are defined...
      const Vector<S32> &details = data->getCollisionDetails();
      for ( U32 j = 0; j < details.size(); j++ ) 
      {
         // JCFHACK: need to fix this if we want this to work with speedtree
         // or other cases in which we don't have a TSForestItemData.
         // Most likely via preventing this method and other torque collision
         // specific stuff from ever getting called.
         if ( details[j] == -1 ) 
            continue;         

         // See if this convex exists in the working set already...
         Convex* cc = 0;
         CollisionWorkingList& wl = convex->getWorkingList();
         for ( CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext ) 
         {
            if ( itr->mConvex->getType() == ForestConvexType )
            {
               ForestConvex *pConvex = static_cast<ForestConvex*>(itr->mConvex);

               if ( pConvex->mObject == this &&
                  pConvex->mForestItemKey == forestItem.getKey() &&
                  pConvex->hullId == j )
               {
                  cc = itr->mConvex;
                  break;
               }
            }
         }
         if (cc)
            continue;

         // Then we need to make one.
         ForestConvex *cp = new ForestConvex;
         mConvexList->registerObject(cp);
         convex->addToWorkingList(cp);
         cp->mObject          = this;
         cp->mForestItemKey   = forestItem.getKey();
         cp->mData            = data;
         cp->mScale           = forestItem.getScale();
         cp->hullId           = j;
         cp->box              = forestItem.getObjBox();
         cp->calculateTransform( forestItem.getTransform() );
      }
   }
}

bool Forest::buildPolyList( PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere )
{
   if ( context == PLC_Decal )
      return false;

   // Get all ForestItem(s) within the box.
   Vector<ForestItem> trees;
   if ( mData->getItems( box, &trees ) == 0 )
      return false;

   polyList->setObject(this);

   bool gotPoly = false;

   for ( U32 i = 0; i < trees.size(); i++ )
      gotPoly |= trees[i].buildPolyList( polyList, box, sphere );

   return gotPoly;
}

bool ForestItem::buildPolyList( AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere ) const
{
   TSForestItemData *data = (TSForestItemData*)mDataBlock;
   
   bool ret = false;

   MatrixF xfm = getTransform();
   polyList->setTransform( &xfm, Point3F(mScale,mScale,mScale) );

   TSShapeInstance *si = data->getShapeInstance();

   const Vector<S32> &details = data->getCollisionDetails();
   S32 detail;
   for  (U32 i = 0; i < details.size(); i++ ) 
   {
      detail = details[i];
      if (detail != -1) 
         ret |= si->buildPolyList( polyList, detail );         
   }

   return ret;   
}

bool Forest::collideBox( const Point3F &start, const Point3F &end, RayInfo *outInfo )
{   
   //Con::warnf( "Forest::collideBox() - not yet implemented!" );
   return Parent::collideBox( start, end, outInfo );
}

bool Forest::castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo )
{
   return castRayBase( start, end, outInfo, false );   
}

bool Forest::castRayRendered( const Point3F &start, const Point3F &end, RayInfo *outInfo )
{
   return castRayBase( start, end, outInfo, true );
}

bool Forest::castRayBase( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered )
{
   if ( !getObjBox().collideLine( start, end ) )
      return false;

   if ( mData->castRay( start, end, outInfo, rendered ) )
   {
      outInfo->object = this;
      return true;
   }

   return false;
}

void Forest::updateCollision()
{
   if ( !mData )
      return;

   mData->buildPhysicsRep( this );

   // Make the assumption that if collision needs
   // to be updated that the zoning probably changed too.
   if ( isClientObject() )
      mZoningDirty = true;
}

bool ForestData::castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered ) const
{
   RayInfo shortest;
   shortest.userData = outInfo->userData;
   shortest.t = F32_MAX;

   BucketTable::ConstIterator iter = mBuckets.begin();
   for (; iter != mBuckets.end(); ++iter)
   {
      if ( iter->value->castRay( start, end, outInfo, rendered ) )
      {
         if ( outInfo->t < shortest.t )         
            shortest = *outInfo;         
      }
   }

   *outInfo = shortest;

   return ( outInfo->t < F32_MAX );
}

bool ForestCell::castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered ) const
{
   // JCF: start/end are in object space of the Forest but mBounds
   // is in world space... Luckily Forest is global bounds and always at the origin
   // with no scale.
   if ( !mBounds.collideLine( start, end ) )
      return false;

   RayInfo shortest;
   shortest.userData = outInfo->userData;
   shortest.t = F32_MAX;

   if ( !isLeaf() )
   {
      for ( U32 i=0; i < 4; i++ )
      {
         if ( mSubCells[i]->castRay( start, end, outInfo, rendered ) )
         {
            if ( outInfo->t < shortest.t )
               shortest = *outInfo;
         }
      }
   }
   else
   {
      for ( U32 i = 0; i < mItems.size(); i++ )
      {     
         if ( mItems[i].castRay( start, end, outInfo, rendered ) )
         {
            if ( outInfo->t < shortest.t )
               shortest = *outInfo;
         }
      }
   }

   *outInfo = shortest;

   return ( outInfo->t < F32_MAX );
}

bool ForestItem::castRay( const Point3F &start, const Point3F &end, RayInfo *outInfo, bool rendered ) const
{
   if ( !mWorldBox.collideLine( start, end ) )
      return false;

   Point3F s, e;
   MatrixF mat = getTransform();
   mat.scale( Point3F(mScale) );
   mat.inverse();

   mat.mulP( start, &s );
   mat.mulP( end, &e );

   TSForestItemData *data = (TSForestItemData*)mDataBlock;
   TSShapeInstance *si = data->getShapeInstance();

   if ( !si ) 
      return false;
   
   if ( rendered )
   {
      // Always raycast against detail level zero
      // because it makes lots of things easier.
      if ( !si->castRayRendered( s, e, outInfo, 0 ) )
         return false;

      if ( outInfo->userData != NULL )
         *(ForestItem*)(outInfo->userData) = *this;

      return true;
   }

   RayInfo shortest;  
   shortest.t = 1e8;
   bool gotMatch = false;
   S32 detail;

   const Vector<S32> &details = data->getLOSDetails();
   for (U32 i = 0; i < details.size(); i++) 
   {
      detail = details[i];
      if (detail != -1) 
      {
         //si->animate(data->mLOSDetails[i]);

         if ( si->castRayOpcode( detail, s, e, outInfo ) )         
         {
            if (outInfo->t < shortest.t)
            {
               gotMatch = true;
               shortest = *outInfo;      
            }
         }
      }
   }

   if ( !gotMatch )
      return false;

   // Copy out the shortest time...
   *outInfo = shortest;

   if ( outInfo->userData != NULL )
      *(ForestItem*)(outInfo->userData) = *this;

   return true;   
}