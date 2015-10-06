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
#include "scene/sceneContainer.h"

#include "collision/extrudedPolyList.h"
#include "collision/earlyOutPolyList.h"
#include "scene/sceneObject.h"
#include "platform/profiler.h"
#include "console/engineAPI.h"
#include "math/util/frustum.h"


// [rene, 02-Mar-11]
//  - *Loads* of copy&paste sin in this file (among its many other sins); all the findObjectXXX methods
//    are trivial permutations of the same snippet of copy&pasted code
//  - FindCallback should return a bool so it's possible to use the findObjectXXX methods to look
//    for the first object matching a certain criteria


SceneContainer gServerContainer;
SceneContainer gClientContainer;

const U32 SceneContainer::csmNumBins = 16;
const F32 SceneContainer::csmBinSize = 64;
const F32 SceneContainer::csmTotalBinSize = SceneContainer::csmBinSize * SceneContainer::csmNumBins;
const U32 SceneContainer::csmRefPoolBlockSize = 4096;

// Statics used by buildPolyList methods
static AbstractPolyList* sPolyList;
static SphereF sBoundingSphere;
static Box3F sBoundingBox;


//=============================================================================
//    SceneContainer::Link.
//=============================================================================

//-----------------------------------------------------------------------------

SceneContainer::Link::Link()
{
   next = prev = this;
}

//-----------------------------------------------------------------------------

void SceneContainer::Link::unlink()
{
   next->prev = prev;
   prev->next = next;
   next = prev = this;
}

//-----------------------------------------------------------------------------

void SceneContainer::Link::linkAfter(SceneContainer::Link* ptr)
{
   next = ptr->next;
   next->prev = this;
   prev = ptr;
   prev->next = this;
}

//=============================================================================
//    SceneContainer.
//=============================================================================

//-----------------------------------------------------------------------------

SceneContainer::SceneContainer()
{
   mSearchInProgress = false;
   mCurrSeqKey = 0;

   mEnd.next = mEnd.prev = &mStart;
   mStart.next = mStart.prev = &mEnd;

   mBinArray = new SceneObjectRef[csmNumBins * csmNumBins];
   for (U32 i = 0; i < csmNumBins; i++) 
   {
      U32 base = i * csmNumBins;
      for (U32 j = 0; j < csmNumBins; j++) 
      {
         mBinArray[base + j].object    = NULL;
         mBinArray[base + j].nextInBin = NULL;
         mBinArray[base + j].prevInBin = NULL;
         mBinArray[base + j].nextInObj = NULL;
      }
   }
   mOverflowBin.object    = NULL;
   mOverflowBin.nextInBin = NULL;
   mOverflowBin.prevInBin = NULL;
   mOverflowBin.nextInObj = NULL;

   VECTOR_SET_ASSOCIATION( mRefPoolBlocks );
   VECTOR_SET_ASSOCIATION( mSearchList );
   VECTOR_SET_ASSOCIATION( mWaterAndZones );
   VECTOR_SET_ASSOCIATION( mTerrains );

   mFreeRefPool = NULL;
   addRefPoolBlock();

   cleanupSearchVectors();
}

//-----------------------------------------------------------------------------

SceneContainer::~SceneContainer()
{
   delete[] mBinArray;

   for (U32 i = 0; i < mRefPoolBlocks.size(); i++)
   {
      SceneObjectRef* pool = mRefPoolBlocks[i];
      for (U32 j = 0; j < csmRefPoolBlockSize; j++)
      {
         // Depressingly, this can give weird results if its pointing at bad memory...
         if(pool[j].object != NULL)
            Con::warnf("Error, a %s (%x) isn't properly out of the bins!", pool[j].object->getClassName(), pool[j].object);

         // If you're getting this it means that an object created didn't
         // remove itself from its container before we destroyed the
         // container. Typically you get this behavior from particle
         // emitters, as they try to hang around until all their particles
         // die. In general it's benign, though if you get it for things
         // that aren't particle emitters it can be a bad sign!
      }

      delete [] pool;
   }
   mFreeRefPool = NULL;

   cleanupSearchVectors();
}

//-----------------------------------------------------------------------------

bool SceneContainer::addObject(SceneObject* obj)
{
   AssertFatal(obj->mContainer == NULL, "Adding already added object.");
   obj->mContainer = this;
   obj->linkAfter(&mStart);

   insertIntoBins(obj);

   // Also insert water and physical zone types into the special vector.
   if ( obj->getTypeMask() & ( WaterObjectType | PhysicalZoneObjectType ) )
      mWaterAndZones.push_back(obj);
   if( obj->getTypeMask() & TerrainObjectType )
      mTerrains.push_back( obj );

   return true;
}

//-----------------------------------------------------------------------------

bool SceneContainer::removeObject(SceneObject* obj)
{
   AssertFatal(obj->mContainer == this, "Trying to remove from wrong container.");
   removeFromBins(obj);

   // Remove water and physical zone types from the special vector.
   if ( obj->getTypeMask() & ( WaterObjectType | PhysicalZoneObjectType ) )
   {
      Vector<SceneObject*>::iterator iter = find( mWaterAndZones.begin(), mWaterAndZones.end(), obj );
      if( iter != mTerrains.end() )
         mWaterAndZones.erase_fast(iter);
   }

   // Remove terrain objects from special vector.
   if( obj->getTypeMask() & TerrainObjectType )
   {
      Vector< SceneObject* >::iterator iter = find( mTerrains.begin(), mTerrains.end(), obj );
      if( iter != mTerrains.end() )
         mTerrains.erase_fast(iter);
   }

   obj->mContainer = 0;
   obj->unlink();
   return true;
}

//-----------------------------------------------------------------------------

void SceneContainer::addRefPoolBlock()
{
   mRefPoolBlocks.push_back(new SceneObjectRef[csmRefPoolBlockSize]);
   for (U32 i = 0; i < csmRefPoolBlockSize-1; i++)
   {
      mRefPoolBlocks.last()[i].object    = NULL;
      mRefPoolBlocks.last()[i].prevInBin = NULL;
      mRefPoolBlocks.last()[i].nextInBin = NULL;
      mRefPoolBlocks.last()[i].nextInObj = &(mRefPoolBlocks.last()[i+1]);
   }
   mRefPoolBlocks.last()[csmRefPoolBlockSize-1].object    = NULL;
   mRefPoolBlocks.last()[csmRefPoolBlockSize-1].prevInBin = NULL;
   mRefPoolBlocks.last()[csmRefPoolBlockSize-1].nextInBin = NULL;
   mRefPoolBlocks.last()[csmRefPoolBlockSize-1].nextInObj = mFreeRefPool;

   mFreeRefPool = &(mRefPoolBlocks.last()[0]);
}

//-----------------------------------------------------------------------------

void SceneContainer::insertIntoBins(SceneObject* obj)
{
   AssertFatal(obj != NULL, "No object?");
   AssertFatal(obj->mBinRefHead == NULL, "Error, already have a bin chain!");

   // The first thing we do is find which bins are covered in x and y...
   const Box3F* pWBox = &obj->getWorldBox();

   U32 minX, maxX, minY, maxY;
   getBinRange(pWBox->minExtents.x, pWBox->maxExtents.x, minX, maxX);
   getBinRange(pWBox->minExtents.y, pWBox->maxExtents.y, minY, maxY);

   // Store the current regions for later queries
   obj->mBinMinX = minX;
   obj->mBinMaxX = maxX;
   obj->mBinMinY = minY;
   obj->mBinMaxY = maxY;

   // For huge objects, dump them into the overflow bin.  Otherwise, everything
   //  goes into the grid...
   if (!obj->isGlobalBounds() && ((maxX - minX + 1) < csmNumBins || (maxY - minY + 1) < csmNumBins))
   {
      SceneObjectRef** pCurrInsert = &obj->mBinRefHead;

      for (U32 i = minY; i <= maxY; i++)
      {
         U32 insertY = i % csmNumBins;
         U32 base    = insertY * csmNumBins;
         for (U32 j = minX; j <= maxX; j++)
         {
            U32 insertX = j % csmNumBins;

            SceneObjectRef* ref = allocateObjectRef();

            ref->object    = obj;
            ref->nextInBin = mBinArray[base + insertX].nextInBin;
            ref->prevInBin = &mBinArray[base + insertX];
            ref->nextInObj = NULL;

            if (mBinArray[base + insertX].nextInBin)
               mBinArray[base + insertX].nextInBin->prevInBin = ref;
            mBinArray[base + insertX].nextInBin = ref;

            *pCurrInsert = ref;
            pCurrInsert  = &ref->nextInObj;
         }
      }
   }
   else
   {
      SceneObjectRef* ref = allocateObjectRef();

      ref->object    = obj;
      ref->nextInBin = mOverflowBin.nextInBin;
      ref->prevInBin = &mOverflowBin;
      ref->nextInObj = NULL;

      if (mOverflowBin.nextInBin)
         mOverflowBin.nextInBin->prevInBin = ref;
      mOverflowBin.nextInBin = ref;

      obj->mBinRefHead = ref;
   }
}

//-----------------------------------------------------------------------------

void SceneContainer::insertIntoBins(SceneObject* obj,
                               U32 minX, U32 maxX,
                               U32 minY, U32 maxY)
{
   PROFILE_START(InsertBins);
   AssertFatal(obj != NULL, "No object?");

   AssertFatal(obj->mBinRefHead == NULL, "Error, already have a bin chain!");
   // Store the current regions for later queries
   obj->mBinMinX = minX;
   obj->mBinMaxX = maxX;
   obj->mBinMinY = minY;
   obj->mBinMaxY = maxY;

   // For huge objects, dump them into the overflow bin.  Otherwise, everything
   //  goes into the grid...
   //
   if ((maxX - minX + 1) < csmNumBins || (maxY - minY + 1) < csmNumBins && !obj->isGlobalBounds())
   {
      SceneObjectRef** pCurrInsert = &obj->mBinRefHead;

      for (U32 i = minY; i <= maxY; i++)
      {
         U32 insertY = i % csmNumBins;
         U32 base    = insertY * csmNumBins;
         for (U32 j = minX; j <= maxX; j++)
         {
            U32 insertX = j % csmNumBins;

            SceneObjectRef* ref = allocateObjectRef();

            ref->object    = obj;
            ref->nextInBin = mBinArray[base + insertX].nextInBin;
            ref->prevInBin = &mBinArray[base + insertX];
            ref->nextInObj = NULL;

            if (mBinArray[base + insertX].nextInBin)
               mBinArray[base + insertX].nextInBin->prevInBin = ref;
            mBinArray[base + insertX].nextInBin = ref;

            *pCurrInsert = ref;
            pCurrInsert  = &ref->nextInObj;
         }
      }
   }
   else
   {
      SceneObjectRef* ref = allocateObjectRef();

      ref->object    = obj;
      ref->nextInBin = mOverflowBin.nextInBin;
      ref->prevInBin = &mOverflowBin;
      ref->nextInObj = NULL;

      if (mOverflowBin.nextInBin)
         mOverflowBin.nextInBin->prevInBin = ref;
      mOverflowBin.nextInBin = ref;
      obj->mBinRefHead = ref;
   }
   PROFILE_END();
}

//-----------------------------------------------------------------------------

void SceneContainer::removeFromBins(SceneObject* obj)
{
   PROFILE_START(RemoveFromBins);
   AssertFatal(obj != NULL, "No object?");

   SceneObjectRef* chain = obj->mBinRefHead;
   obj->mBinRefHead = NULL;

   while (chain)
   {
      SceneObjectRef* trash = chain;
      chain = chain->nextInObj;

      AssertFatal(trash->prevInBin != NULL, "Error, must have a previous entry in the bin!");
      if (trash->nextInBin)
         trash->nextInBin->prevInBin = trash->prevInBin;
      trash->prevInBin->nextInBin = trash->nextInBin;

      freeObjectRef(trash);
   }
   PROFILE_END();
}

//-----------------------------------------------------------------------------

void SceneContainer::checkBins(SceneObject* obj)
{
   AssertFatal(obj != NULL, "No object?");

   PROFILE_START(CheckBins);
   if (obj->mBinRefHead == NULL)
   {
      insertIntoBins(obj);
      PROFILE_END();
      return;
   }

   // Otherwise, the object is already in the bins.  Let's see if it has strayed out of
   //  the bins that it's currently in...
   const Box3F* pWBox = &obj->getWorldBox();

   U32 minX, maxX, minY, maxY;
   getBinRange(pWBox->minExtents.x, pWBox->maxExtents.x, minX, maxX);
   getBinRange(pWBox->minExtents.y, pWBox->maxExtents.y, minY, maxY);

   if (obj->mBinMinX != minX || obj->mBinMaxX != maxX ||
       obj->mBinMinY != minY || obj->mBinMaxY != maxY)
   {
      // We have to rebin the object
      removeFromBins(obj);
      insertIntoBins(obj, minX, maxX, minY, maxY);
   }
   PROFILE_END();
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjects(const Box3F& box, U32 mask, FindCallback callback, void *key)
{
   PROFILE_SCOPE(ContainerFindObjects_Box);

   // If we're searching for just water, just physical zones, or
   // just water and physical zones then use the optimized path.
   if ( mask == WaterObjectType || 
        mask == PhysicalZoneObjectType ||
        mask == (WaterObjectType|PhysicalZoneObjectType) )
   {
      _findSpecialObjects( mWaterAndZones, box, mask, callback, key );
      return;
   }
   else if( mask == TerrainObjectType )
   {
      _findSpecialObjects( mTerrains, box, mask, callback, key );
      return;
   }

   AssertFatal( !mSearchInProgress, "SceneContainer::findObjects - Container queries are not re-entrant" );
   mSearchInProgress = true;

   U32 minX, maxX, minY, maxY;
   getBinRange(box.minExtents.x, box.maxExtents.x, minX, maxX);
   getBinRange(box.minExtents.y, box.maxExtents.y, minY, maxY);
   mCurrSeqKey++;
   for (U32 i = minY; i <= maxY; i++)
   {
      U32 insertY = i % csmNumBins;
      U32 base    = insertY * csmNumBins;
      for (U32 j = minX; j <= maxX; j++)
      {
         U32 insertX = j % csmNumBins;

         SceneObjectRef* chain = mBinArray[base + insertX].nextInBin;
         while (chain)
         {
            if (chain->object->getContainerSeqKey() != mCurrSeqKey)
            {
               chain->object->setContainerSeqKey(mCurrSeqKey);

               if ((chain->object->getTypeMask() & mask) != 0 &&
                   chain->object->isCollisionEnabled())
               {
                  if (chain->object->getWorldBox().isOverlapped(box) || chain->object->isGlobalBounds())
                  {
                     (*callback)(chain->object,key);
                  }
               }
            }
            chain = chain->nextInBin;
         }
      }
   }
   SceneObjectRef* chain = mOverflowBin.nextInBin;
   while (chain)
   {
      if (chain->object->getContainerSeqKey() != mCurrSeqKey)
      {
         chain->object->setContainerSeqKey(mCurrSeqKey);

         if ((chain->object->getTypeMask() & mask) != 0 &&
             chain->object->isCollisionEnabled())
         {
            if (chain->object->getWorldBox().isOverlapped(box) || chain->object->isGlobalBounds())
            {
               (*callback)(chain->object,key);
            }
         }
      }
      chain = chain->nextInBin;
   }

   mSearchInProgress = false;
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjects( const Frustum &frustum, U32 mask, FindCallback callback, void *key )
{
   PROFILE_SCOPE(ContainerFindObjects_Frustum);

   Box3F searchBox = frustum.getBounds();

   if (  mask == WaterObjectType || 
         mask == PhysicalZoneObjectType ||
         mask == (WaterObjectType|PhysicalZoneObjectType) )
   {
      _findSpecialObjects( mWaterAndZones, searchBox, mask, callback, key );
      return;
   }
   else if( mask == TerrainObjectType )
   {
      _findSpecialObjects( mTerrains, searchBox, mask, callback, key );
      return;
   }

   AssertFatal( !mSearchInProgress, "SceneContainer::findObjects - Container queries are not re-entrant" );
   mSearchInProgress = true;

   U32 minX, maxX, minY, maxY;
   getBinRange(searchBox.minExtents.x, searchBox.maxExtents.x, minX, maxX);
   getBinRange(searchBox.minExtents.y, searchBox.maxExtents.y, minY, maxY);
   mCurrSeqKey++;

   for (U32 i = minY; i <= maxY; i++)
   {
      U32 insertY = i % csmNumBins;
      U32 base    = insertY * csmNumBins;
      for (U32 j = minX; j <= maxX; j++)
      {
         U32 insertX = j % csmNumBins;

         SceneObjectRef* chain = mBinArray[base + insertX].nextInBin;
         while (chain)
         {
            SceneObject *object = chain->object;

            if (object->getContainerSeqKey() != mCurrSeqKey)
            {
               object->setContainerSeqKey(mCurrSeqKey);

               if ((object->getTypeMask() & mask) != 0 &&
                  object->isCollisionEnabled())
               {
                  const Box3F &worldBox = object->getWorldBox();
                  if ( object->isGlobalBounds() || worldBox.isOverlapped(searchBox) )
                  {
                     if ( !frustum.isCulled( worldBox ) )
                        (*callback)(chain->object,key);
                  }
               }
            }
            chain = chain->nextInBin;
         }
      }
   }

   SceneObjectRef* chain = mOverflowBin.nextInBin;
   while (chain)
   {
      SceneObject *object = chain->object;

      if (object->getContainerSeqKey() != mCurrSeqKey)
      {
         object->setContainerSeqKey(mCurrSeqKey);

         if ((object->getTypeMask() & mask) != 0 &&
            object->isCollisionEnabled())
         {
            const Box3F &worldBox = object->getWorldBox();

            if ( object->isGlobalBounds() || worldBox.isOverlapped(searchBox) )
            {
               if ( !frustum.isCulled( worldBox ) )
                  (*callback)(object,key);
            }
         }
      }
      chain = chain->nextInBin;
   }

   mSearchInProgress = false;
}

//-----------------------------------------------------------------------------

void SceneContainer::polyhedronFindObjects(const Polyhedron& polyhedron, U32 mask, FindCallback callback, void *key)
{
   PROFILE_SCOPE(ContainerFindObjects_polyhedron);

   U32 i;
   Box3F box;
   box.minExtents.set(1e9, 1e9, 1e9);
   box.maxExtents.set(-1e9, -1e9, -1e9);
   for (i = 0; i < polyhedron.pointList.size(); i++)
   {
      box.minExtents.setMin(polyhedron.pointList[i]);
      box.maxExtents.setMax(polyhedron.pointList[i]);
   }

   if (  mask == WaterObjectType || 
         mask == PhysicalZoneObjectType ||
         mask == (WaterObjectType|PhysicalZoneObjectType) )
   {
      _findSpecialObjects( mWaterAndZones, box, mask, callback, key );
      return;
   }
   else if( mask == TerrainObjectType )
   {
      _findSpecialObjects( mTerrains, mask, callback, key );
      return;
   }

   AssertFatal( !mSearchInProgress, "SceneContainer::polyhedronFindObjects - Container queries are not re-entrant" );
   mSearchInProgress = true;

   U32 minX, maxX, minY, maxY;
   getBinRange(box.minExtents.x, box.maxExtents.x, minX, maxX);
   getBinRange(box.minExtents.y, box.maxExtents.y, minY, maxY);
   mCurrSeqKey++;
   for (i = minY; i <= maxY; i++)
   {
      U32 insertY = i % csmNumBins;
      U32 base    = insertY * csmNumBins;
      for (U32 j = minX; j <= maxX; j++)
      {
         U32 insertX = j % csmNumBins;

         SceneObjectRef* chain = mBinArray[base + insertX].nextInBin;
         while (chain)
         {
            if (chain->object->getContainerSeqKey() != mCurrSeqKey)
            {
               chain->object->setContainerSeqKey(mCurrSeqKey);

               if ((chain->object->getTypeMask() & mask) != 0 &&
                   chain->object->isCollisionEnabled())
               {
                  if (chain->object->getWorldBox().isOverlapped(box) || chain->object->isGlobalBounds())
                  {
                     (*callback)(chain->object,key);
                  }
               }
            }
            chain = chain->nextInBin;
         }
      }
   }
   SceneObjectRef* chain = mOverflowBin.nextInBin;
   while (chain)
   {
      if (chain->object->getContainerSeqKey() != mCurrSeqKey)
      {
         chain->object->setContainerSeqKey(mCurrSeqKey);

         if ((chain->object->getTypeMask() & mask) != 0 &&
             chain->object->isCollisionEnabled())
         {
            if (chain->object->getWorldBox().isOverlapped(box) || chain->object->isGlobalBounds())
            {
               (*callback)(chain->object,key);
            }
         }
      }
      chain = chain->nextInBin;
   }

   mSearchInProgress = false;
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjectList( const Box3F& searchBox, U32 mask, Vector<SceneObject*> *outFound )
{
   PROFILE_SCOPE( Container_FindObjectList_Box );

   AssertFatal( !mSearchInProgress, "SceneContainer::findObjectList - Container queries are not re-entrant" );
   mSearchInProgress = true;

   // TODO: Optimize for water and zones?

   U32 minX, maxX, minY, maxY;
   getBinRange(searchBox.minExtents.x, searchBox.maxExtents.x, minX, maxX);
   getBinRange(searchBox.minExtents.y, searchBox.maxExtents.y, minY, maxY);
   mCurrSeqKey++;

   for (U32 i = minY; i <= maxY; i++)
   {
      U32 insertY = i % csmNumBins;
      U32 base    = insertY * csmNumBins;
      for (U32 j = minX; j <= maxX; j++)
      {
         U32 insertX = j % csmNumBins;

         SceneObjectRef* chain = mBinArray[base + insertX].nextInBin;
         while (chain)
         {
            SceneObject *object = chain->object;

            if (object->getContainerSeqKey() != mCurrSeqKey)
            {
               object->setContainerSeqKey(mCurrSeqKey);

               if ((object->getTypeMask() & mask) != 0 &&
                  object->isCollisionEnabled())
               {
                  const Box3F &worldBox = object->getWorldBox();
                  if ( object->isGlobalBounds() || worldBox.isOverlapped( searchBox ) )
                  {
                     outFound->push_back( object );
                  }
               }
            }
            chain = chain->nextInBin;
         }
      }
   }

   SceneObjectRef* chain = mOverflowBin.nextInBin;
   while (chain)
   {
      SceneObject *object = chain->object;

      if (object->getContainerSeqKey() != mCurrSeqKey)
      {
         object->setContainerSeqKey(mCurrSeqKey);

         if ((object->getTypeMask() & mask) != 0 &&
            object->isCollisionEnabled())
         {
            const Box3F &worldBox = object->getWorldBox();

            if ( object->isGlobalBounds() || worldBox.isOverlapped( searchBox ) )
            {
               outFound->push_back( object );
            }
         }
      }
      chain = chain->nextInBin;
   }

   mSearchInProgress = false;
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjectList( const Frustum &frustum, U32 mask, Vector<SceneObject*> *outFound )
{
   PROFILE_SCOPE( Container_FindObjectList_Frustum );

   // Do a box find first.
   findObjectList( frustum.getBounds(), mask, outFound );

   // Now do the frustum testing.
   for ( U32 i=0; i < outFound->size(); )
   {
      const Box3F &worldBox = (*outFound)[i]->getWorldBox();
      if ( frustum.isCulled( worldBox ) )
         outFound->erase_fast( i );
      else
         i++;
   }
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjectList( U32 mask, Vector<SceneObject*> *outFound )
{
   for ( Link* itr = mStart.next; itr != &mEnd; itr = itr->next )
   {
      SceneObject* ptr = static_cast<SceneObject*>( itr );
      if ( ( ptr->getTypeMask() & mask ) != 0 )
         outFound->push_back( ptr );
   }
}

//-----------------------------------------------------------------------------

void SceneContainer::findObjects( U32 mask, FindCallback callback, void *key )
{
   for (Link* itr = mStart.next; itr != &mEnd; itr = itr->next) {
      SceneObject* ptr = static_cast<SceneObject*>(itr);
      if ((ptr->getTypeMask() & mask) != 0 && !ptr->mCollisionCount)
         (*callback)(ptr,key);
   }
}

//-----------------------------------------------------------------------------

void SceneContainer::_findSpecialObjects( const Vector< SceneObject* >& vector, U32 mask, FindCallback callback, void *key )
{
   PROFILE_SCOPE( Container_findSpecialObjects );

   Vector<SceneObject*>::const_iterator iter = vector.begin();
   for ( ; iter != vector.end(); iter++ )
   {
      if ( (*iter)->getTypeMask() & mask )
         callback( *iter, key );
   }   
}

//-----------------------------------------------------------------------------

void SceneContainer::_findSpecialObjects( const Vector< SceneObject* >& vector, const Box3F &box, U32 mask, FindCallback callback, void *key )
{
   PROFILE_SCOPE( Container_findSpecialObjects_Box );

   Vector<SceneObject*>::const_iterator iter = vector.begin();

   for ( ; iter != vector.end(); iter++ )
   {
      SceneObject *pObj = *iter;
      
      if ( pObj->getTypeMask() & mask &&
           ( pObj->isGlobalBounds() || pObj->getWorldBox().isOverlapped(box) ) )
      {
         callback( pObj, key );
      }
   }  
}

//-----------------------------------------------------------------------------

bool SceneContainer::castRay( const Point3F& start, const Point3F& end, U32 mask, RayInfo* info, CastRayCallback callback )
{
   AssertFatal( info->userData == NULL, "SceneContainer::castRay - RayInfo->userData cannot be used here!" );

   PROFILE_START( SceneContainer_CastRay );
   bool result = _castRay( CollisionGeometry, start, end, mask, info, callback );
   PROFILE_END();
   return result;
}

//-----------------------------------------------------------------------------

bool SceneContainer::castRayRendered( const Point3F& start, const Point3F& end, U32 mask, RayInfo* info, CastRayCallback callback )
{
   AssertFatal( info->userData == NULL, "SceneContainer::castRayRendered - RayInfo->userData cannot be used here!" );

   PROFILE_START( SceneContainer_CastRayRendered );
   bool result = _castRay( RenderedGeometry, start, end, mask, info, callback );
   PROFILE_END();
   return result;
}

//-----------------------------------------------------------------------------

// DMMNOTE: There are still some optimizations to be done here.  In particular:
//           - After checking the overflow bin, we can potentially shorten the line
//             that we rasterize against the grid if there is a collision with say,
//             the terrain.
//           - The optimal grid size isn't necessarily what we have set here. possibly
//             a resolution of 16 meters would give better results
//           - The line rasterizer is pretty lame.  Unfortunately we can't use a
//             simple bres. here, since we need to check every grid element that the line
//             passes through, which bres does _not_ do for us.  Possibly there's a
//             rasterizer for anti-aliased lines that will serve better than what
//             we have below.

bool SceneContainer::_castRay( U32 type, const Point3F& start, const Point3F& end, U32 mask, RayInfo* info, CastRayCallback callback )
{
   AssertFatal( !mSearchInProgress, "SceneContainer::_castRay - Container queries are not re-entrant" );
   mSearchInProgress = true;

   F32 currentT = 2.0;
   mCurrSeqKey++;

   SceneObjectRef* chain = mOverflowBin.nextInBin;
   while (chain)
   {
      SceneObject* ptr = chain->object;
      if (ptr->getContainerSeqKey() != mCurrSeqKey)
      {
         ptr->setContainerSeqKey(mCurrSeqKey);

         // In the overflow bin, the world box is always going to intersect the line,
         //  so we can omit that test...
         if ((ptr->getTypeMask() & mask) != 0 &&
             ptr->isCollisionEnabled() == true)
         {
            Point3F xformedStart, xformedEnd;
            ptr->mWorldToObj.mulP(start, &xformedStart);
            ptr->mWorldToObj.mulP(end,   &xformedEnd);
            xformedStart.convolveInverse(ptr->mObjScale);
            xformedEnd.convolveInverse(ptr->mObjScale);

            RayInfo ri;
            ri.generateTexCoord  = info->generateTexCoord;
            bool result = false;
            if (type == CollisionGeometry)
               result = ptr->castRay(xformedStart, xformedEnd, &ri);
            else if (type == RenderedGeometry)
               result = ptr->castRayRendered(xformedStart, xformedEnd, &ri);
            if (result)
            {
               if( ri.t < currentT && ( !callback || callback( &ri ) ) )
               {
                  *info = ri;
                  info->point.interpolate(start, end, info->t);
                  currentT = ri.t;
						info->distance = (start - info->point).len();
               }
            }
         }
      }
      chain = chain->nextInBin;
   }

   // These are just for rasterizing the line against the grid.  We want the x coord
   //  of the start to be <= the x coord of the end
   Point3F normalStart, normalEnd;
   if (start.x <= end.x)
   {
      normalStart = start;
      normalEnd   = end;
   }
   else
   {
      normalStart = end;
      normalEnd   = start;
   }

   // Ok, let's scan the grids.  The simplest way to do this will be to scan across in
   //  x, finding the y range for each affected bin...
   U32 minX, maxX;
   U32 minY, maxY;
//if (normalStart.x == normalEnd.x)
//   Con::printf("X start = %g, end = %g", normalStart.x, normalEnd.x);

   getBinRange(normalStart.x, normalEnd.x, minX, maxX);
   getBinRange(getMin(normalStart.y, normalEnd.y),
               getMax(normalStart.y, normalEnd.y), minY, maxY);

//if (normalStart.x == normalEnd.x && minX != maxX)
//   Con::printf("X min = %d, max = %d", minX, maxX);
//if (normalStart.y == normalEnd.y && minY != maxY)
//   Con::printf("Y min = %d, max = %d", minY, maxY);

   // We'll optimize the case that the line is contained in one bin row or column, which
   //  will be quite a few lines.  No sense doing more work than we have to...
   //
   if ((mFabs(normalStart.x - normalEnd.x) < csmTotalBinSize && minX == maxX) ||
       (mFabs(normalStart.y - normalEnd.y) < csmTotalBinSize && minY == maxY))
   {
      U32 count;
      U32 incX, incY;
      if (minX == maxX)
      {
         count = maxY - minY + 1;
         incX  = 0;
         incY  = 1;
      }
      else
      {
         count = maxX - minX + 1;
         incX  = 1;
         incY  = 0;
      }

      U32 x = minX;
      U32 y = minY;
      for (U32 i = 0; i < count; i++)
      {
         U32 checkX = x % csmNumBins;
         U32 checkY = y % csmNumBins;

         SceneObjectRef* chain = mBinArray[(checkY * csmNumBins) + checkX].nextInBin;
         while (chain)
         {
            SceneObject* ptr = chain->object;
            if (ptr->getContainerSeqKey() != mCurrSeqKey)
            {
               ptr->setContainerSeqKey(mCurrSeqKey);

               if ((ptr->getTypeMask() & mask) != 0      &&
                   ptr->isCollisionEnabled() == true)
               {
                  if (ptr->getWorldBox().collideLine(start, end) || chain->object->isGlobalBounds())
                  {
                     Point3F xformedStart, xformedEnd;
                     ptr->mWorldToObj.mulP(start, &xformedStart);
                     ptr->mWorldToObj.mulP(end,   &xformedEnd);
                     xformedStart.convolveInverse(ptr->mObjScale);
                     xformedEnd.convolveInverse(ptr->mObjScale);

                     RayInfo ri;
                     ri.generateTexCoord  = info->generateTexCoord;
                     bool result = false;
                     if (type == CollisionGeometry)
                        result = ptr->castRay(xformedStart, xformedEnd, &ri);
                     else if (type == RenderedGeometry)
                        result = ptr->castRayRendered(xformedStart, xformedEnd, &ri);
                     if (result)
                     {
                        if( ri.t < currentT && ( !callback || callback( &ri ) ) )
                        {
                           *info = ri;
                           info->point.interpolate(start, end, info->t);
                           currentT = ri.t;
						         info->distance = (start - info->point).len();
                        }
                     }
                  }
               }
            }
            chain = chain->nextInBin;
         }

         x += incX;
         y += incY;
      }
   }
   else
   {
      // Oh well, let's earn our keep.  We know that after the above conditional, we're
      //  going to cross at least one boundary, so that simplifies our job...

      F32 currStartX = normalStart.x;

      AssertFatal(currStartX != normalEnd.x, "This is going to cause problems in SceneContainer::castRay");
      if(mIsNaN_F(currStartX))
      {
         PROFILE_END();
         return false;
      }
      while (currStartX != normalEnd.x)
      {
         F32 currEndX   = getMin(currStartX + csmTotalBinSize, normalEnd.x);

         F32 currStartT = (currStartX - normalStart.x) / (normalEnd.x - normalStart.x);
         F32 currEndT   = (currEndX   - normalStart.x) / (normalEnd.x - normalStart.x);

         F32 y1 = normalStart.y + (normalEnd.y - normalStart.y) * currStartT;
         F32 y2 = normalStart.y + (normalEnd.y - normalStart.y) * currEndT;

         U32 subMinX, subMaxX;
         getBinRange(currStartX, currEndX, subMinX, subMaxX);

         F32 subStartX = currStartX;
         F32 subEndX   = currStartX;

         if (currStartX < 0.0f)
            subEndX -= mFmod(subEndX, csmBinSize);
         else
            subEndX += (csmBinSize - mFmod(subEndX, csmBinSize));

         for (U32 currXBin = subMinX; currXBin <= subMaxX; currXBin++)
         {
            U32 checkX = currXBin % csmNumBins;

            F32 subStartT = (subStartX - currStartX) / (currEndX - currStartX);
            F32 subEndT   = getMin(F32((subEndX   - currStartX) / (currEndX - currStartX)), 1.f);

            F32 subY1 = y1 + (y2 - y1) * subStartT;
            F32 subY2 = y1 + (y2 - y1) * subEndT;

            U32 newMinY, newMaxY;
            getBinRange(getMin(subY1, subY2), getMax(subY1, subY2), newMinY, newMaxY);

            for (U32 i = newMinY; i <= newMaxY; i++)
            {
               U32 checkY = i % csmNumBins;

               SceneObjectRef* chain = mBinArray[(checkY * csmNumBins) + checkX].nextInBin;
               while (chain)
               {
                  SceneObject* ptr = chain->object;
                  if (ptr->getContainerSeqKey() != mCurrSeqKey)
                  {
                     ptr->setContainerSeqKey(mCurrSeqKey);

                     if ((ptr->getTypeMask() & mask) != 0      &&
                         ptr->isCollisionEnabled() == true)
                     {
                        if (ptr->getWorldBox().collideLine(start, end))
                        {
                           Point3F xformedStart, xformedEnd;
                           ptr->mWorldToObj.mulP(start, &xformedStart);
                           ptr->mWorldToObj.mulP(end,   &xformedEnd);
                           xformedStart.convolveInverse(ptr->mObjScale);
                           xformedEnd.convolveInverse(ptr->mObjScale);

                           RayInfo ri;
                           ri.generateTexCoord  = info->generateTexCoord;
                           bool result = false;
                           if (type == CollisionGeometry)
                              result = ptr->castRay(xformedStart, xformedEnd, &ri);
                           else if (type == RenderedGeometry)
                              result = ptr->castRayRendered(xformedStart, xformedEnd, &ri);
                           if (result)
                           {
                              if( ri.t < currentT && ( !callback || callback( &ri ) ) )
                              {
                                 *info = ri;
                                 info->point.interpolate(start, end, info->t);
                                 currentT = ri.t;
								         info->distance = (start - info->point).len();
                              }
                           }
                        }
                     }
                  }
                  chain = chain->nextInBin;
               }
            }

            subStartX = subEndX;
            subEndX   = getMin(subEndX + csmBinSize, currEndX);
         }

         currStartX = currEndX;
      }
   }

   mSearchInProgress = false;

   // Bump the normal into worldspace if appropriate.
   if(currentT != 2)
   {
      PlaneF fakePlane;
      fakePlane.x = info->normal.x;
      fakePlane.y = info->normal.y;
      fakePlane.z = info->normal.z;
      fakePlane.d = 0;

      PlaneF result;
      mTransformPlane(info->object->getTransform(), info->object->getScale(), fakePlane, &result);
      info->normal = result;

      return true;
   }
   else
   {
      // Do nothing and exit...
      return false;
   }
}

//-----------------------------------------------------------------------------

// collide with the objects projected object box
bool SceneContainer::collideBox(const Point3F &start, const Point3F &end, U32 mask, RayInfo * info)
{
   AssertFatal( info->userData == NULL, "SceneContainer::collideBox - RayInfo->userData cannot be used here!" );

   F32 currentT = 2;
   for (Link* itr = mStart.next; itr != &mEnd; itr = itr->next)
   {
      SceneObject* ptr = static_cast<SceneObject*>(itr);
      if (ptr->getTypeMask() & mask && !ptr->mCollisionCount)
      {
         Point3F xformedStart, xformedEnd;
         ptr->mWorldToObj.mulP(start, &xformedStart);
         ptr->mWorldToObj.mulP(end,   &xformedEnd);
         xformedStart.convolveInverse(ptr->mObjScale);
         xformedEnd.convolveInverse(ptr->mObjScale);

         RayInfo ri;
         if(ptr->collideBox(xformedStart, xformedEnd, &ri))
         {
            if(ri.t < currentT)
            {
               *info = ri;
               info->point.interpolate(start, end, info->t);
               currentT = ri.t;
            }
         }
      }
   }
   return currentT != 2;
}

//-----------------------------------------------------------------------------

static void buildCallback(SceneObject* object,void *key)
{
   SceneContainer::CallbackInfo* info = reinterpret_cast<SceneContainer::CallbackInfo*>(key);
   object->buildPolyList(info->context,info->polyList,info->boundingBox,info->boundingSphere);
}

bool SceneContainer::buildPolyList(PolyListContext context, const Box3F &box, U32 mask, AbstractPolyList *polyList)
{
   CallbackInfo info;
   info.context = context;
   info.boundingBox = box;
   info.polyList = polyList;

   // Build bounding sphere
   info.boundingSphere.center = (info.boundingBox.minExtents + info.boundingBox.maxExtents) * 0.5;
   VectorF bv = box.maxExtents - info.boundingSphere.center;
   info.boundingSphere.radius = bv.len();

   sPolyList = polyList;
   findObjects(box,mask,buildCallback,&info);
   return !polyList->isEmpty();
}

//-----------------------------------------------------------------------------

void SceneContainer::cleanupSearchVectors()
{
   for (U32 i = 0; i < mSearchList.size(); i++)
      delete mSearchList[i];
   mSearchList.clear();
   mCurrSearchPos = -1;
}

//-----------------------------------------------------------------------------

static Point3F sgSortReferencePoint;
static S32 QSORT_CALLBACK cmpSearchPointers(const void* inP1, const void* inP2)
{
   SimObjectPtr<SceneObject>** p1 = (SimObjectPtr<SceneObject>**)inP1;
   SimObjectPtr<SceneObject>** p2 = (SimObjectPtr<SceneObject>**)inP2;

   Point3F temp;
   F32 d1, d2;

   if (bool(**p1))
   {
      (**p1)->getWorldBox().getCenter(&temp);
      d1 = (temp - sgSortReferencePoint).len();
   }
   else
   {
      d1 = 0;
   }
   if (bool(**p2))
   {
      (**p2)->getWorldBox().getCenter(&temp);
      d2 = (temp - sgSortReferencePoint).len();
   }
   else
   {
      d2 = 0;
   }

   if (d1 > d2)
      return 1;
   else if (d1 < d2)
      return -1;
   else
      return 0;
}

void SceneContainer::initRadiusSearch(const Point3F& searchPoint,
                                 const F32      searchRadius,
                                 const U32      searchMask)
{
   cleanupSearchVectors();

   mSearchReferencePoint = searchPoint;

   Box3F queryBox(searchPoint, searchPoint);
   queryBox.minExtents -= Point3F(searchRadius, searchRadius, searchRadius);
   queryBox.maxExtents += Point3F(searchRadius, searchRadius, searchRadius);

   SimpleQueryList queryList;
   findObjects(queryBox, searchMask, SimpleQueryList::insertionCallback, &queryList);

   F32 radiusSquared = searchRadius * searchRadius;

   const F32* pPoint = &searchPoint.x;
   for (U32 i = 0; i < queryList.mList.size(); i++)
   {
      const F32* bMins;
      const F32* bMaxs;
      bMins = &queryList.mList[i]->getWorldBox().minExtents.x;
      bMaxs = &queryList.mList[i]->getWorldBox().maxExtents.x;
      F32 sum = 0;
      for (U32 j = 0; j < 3; j++)
      {
         if (pPoint[j] < bMins[j])
            sum += (pPoint[j] - bMins[j])*(pPoint[j] - bMins[j]);
         else if (pPoint[j] > bMaxs[j])
            sum += (pPoint[j] - bMaxs[j])*(pPoint[j] - bMaxs[j]);
      }
      if (sum < radiusSquared || queryList.mList[i]->isGlobalBounds())
      {
         mSearchList.push_back(new SimObjectPtr<SceneObject>);
         *(mSearchList.last()) = queryList.mList[i];
      }
   }
   if (mSearchList.size() != 0)
   {
      sgSortReferencePoint = mSearchReferencePoint;
      dQsort(mSearchList.address(), mSearchList.size(),
             sizeof(SimObjectPtr<SceneObject>*), cmpSearchPointers);
   }
}

//-----------------------------------------------------------------------------

void SceneContainer::initTypeSearch(const U32      searchMask)
{
   cleanupSearchVectors();

   SimpleQueryList queryList;
   findObjects(searchMask, SimpleQueryList::insertionCallback, &queryList);

   for (U32 i = 0; i < queryList.mList.size(); i++)
   {
         mSearchList.push_back(new SimObjectPtr<SceneObject>);
         *(mSearchList.last()) = queryList.mList[i];
   }
   if (mSearchList.size() != 0)
   {
      sgSortReferencePoint = mSearchReferencePoint;
      dQsort(mSearchList.address(), mSearchList.size(),
             sizeof(SimObjectPtr<SceneObject>*), cmpSearchPointers);
   }
}

//-----------------------------------------------------------------------------

SceneObject* SceneContainer::containerSearchNextObject()
{
   if (mCurrSearchPos >= mSearchList.size())
      return NULL;

   mCurrSearchPos++;
   while (mCurrSearchPos < mSearchList.size() && bool(*mSearchList[mCurrSearchPos]) == false)
      mCurrSearchPos++;

   if (mCurrSearchPos == mSearchList.size())
      return NULL;

   return (*mSearchList[mCurrSearchPos]);
}

//-----------------------------------------------------------------------------

U32 SceneContainer::containerSearchNext()
{
   SceneObject* object = containerSearchNextObject();
   if( !object )
      return 0;
   return object->getId();
}

//-----------------------------------------------------------------------------

F32 SceneContainer::containerSearchCurrDist()
{
   AssertFatal(mCurrSearchPos != -1, "Error, must call containerSearchNext before containerSearchCurrDist");

   if (mCurrSearchPos == -1 || mCurrSearchPos >= mSearchList.size() ||
       bool(*mSearchList[mCurrSearchPos]) == false)
      return 0.0;

   Point3F pos;
   (*mSearchList[mCurrSearchPos])->getWorldBox().getCenter(&pos);
   return (pos - mSearchReferencePoint).len();
}

//-----------------------------------------------------------------------------

F32 SceneContainer::containerSearchCurrRadiusDist()
{
   AssertFatal(mCurrSearchPos != -1, "Error, must call containerSearchNext before containerSearchCurrDist");

   if (mCurrSearchPos == -1 || mCurrSearchPos >= mSearchList.size() ||
       bool(*mSearchList[mCurrSearchPos]) == false)
      return 0.0;

   Point3F pos;
   Box3F worldBox = (*mSearchList[mCurrSearchPos])->getWorldBox();
   worldBox.getCenter(&pos);

   F32 dist = (pos - mSearchReferencePoint).len();

   F32 min = worldBox.len_x();
   if (worldBox.len_y() < min)
      min = worldBox.len_y();
   if (worldBox.len_z() < min)
      min = worldBox.len_z();

   dist -= min;
   if (dist < 0)
      dist = 0;

   return dist;
}

//-----------------------------------------------------------------------------

void SceneContainer::getBinRange( const F32 min, const F32 max, U32& minBin, U32& maxBin )
{
   AssertFatal(max >= min, avar("Error, bad range in getBinRange. min: %f, max: %f", min, max));

   if ((max - min) >= (SceneContainer::csmTotalBinSize - SceneContainer::csmBinSize))
   {
      F32 minCoord = mFmod(min, SceneContainer::csmTotalBinSize);
      if (minCoord < 0.0f) 
      {
         minCoord += SceneContainer::csmTotalBinSize;

         // This is truly lame, but it can happen.  There must be a better way to
         //  deal with this.
         if (minCoord == SceneContainer::csmTotalBinSize)
            minCoord = SceneContainer::csmTotalBinSize - 0.01;
      }

      AssertFatal(minCoord >= 0.0 && minCoord < SceneContainer::csmTotalBinSize, "Bad minCoord");

      minBin = U32(minCoord / SceneContainer::csmBinSize);
      AssertFatal(minBin < SceneContainer::csmNumBins, avar("Error, bad clipping! (%g, %d)", minCoord, minBin));

      maxBin = minBin + (SceneContainer::csmNumBins - 1);
      return;
   }
   else 
   {

      F32 minCoord = mFmod(min, SceneContainer::csmTotalBinSize);
      
      if (minCoord < 0.0f) 
      {
         minCoord += SceneContainer::csmTotalBinSize;

         // This is truly lame, but it can happen.  There must be a better way to
         //  deal with this.
         if (minCoord == SceneContainer::csmTotalBinSize)
            minCoord = SceneContainer::csmTotalBinSize - 0.01;
      }
      AssertFatal(minCoord >= 0.0 && minCoord < SceneContainer::csmTotalBinSize, "Bad minCoord");

      F32 maxCoord = mFmod(max, SceneContainer::csmTotalBinSize);
      if (maxCoord < 0.0f) {
         maxCoord += SceneContainer::csmTotalBinSize;

         // This is truly lame, but it can happen.  There must be a better way to
         //  deal with this.
         if (maxCoord == SceneContainer::csmTotalBinSize)
            maxCoord = SceneContainer::csmTotalBinSize - 0.01;
      }
      AssertFatal(maxCoord >= 0.0 && maxCoord < SceneContainer::csmTotalBinSize, "Bad maxCoord");

      minBin = U32(minCoord / SceneContainer::csmBinSize);
      maxBin = U32(maxCoord / SceneContainer::csmBinSize);
      AssertFatal(minBin < SceneContainer::csmNumBins, avar("Error, bad clipping(min)! (%g, %d)", maxCoord, minBin));
      AssertFatal(minBin < SceneContainer::csmNumBins, avar("Error, bad clipping(max)! (%g, %d)", maxCoord, maxBin));

      // MSVC6 seems to be generating some bad floating point code around
      // here when full optimizations are on.  The min != max test should
      // not be needed, but it clears up the VC issue.
      if (min != max && minCoord > maxCoord)
         maxBin += SceneContainer::csmNumBins;

      AssertFatal(maxBin >= minBin, "Error, min should always be less than max!");
   }
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

ConsoleFunctionGroupBegin( Containers,  "Functions for ray casting and spatial queries.\n\n");

//-----------------------------------------------------------------------------

DefineEngineFunction( containerBoxEmpty, bool,
   ( U32 mask, Point3F center, F32 xRadius, F32 yRadius, F32 zRadius, bool useClientContainer ), ( -1, -1, false ),
   "@brief See if any objects of the given types are present in box of given extent.\n\n"
   "@note Extent parameter is last since only one radius is often needed.  If "
   "one radius is provided, the yRadius and zRadius are assumed to be the same.  Unfortunately, "
   "if you need to use the client container, you'll need to set all of the radius parameters.  "
   "Fortunately, this function is mostly used on the server.\n"
   "@param  mask   Indicates the type of objects we are checking against.\n"
   "@param  center Center of box.\n"
   "@param  xRadius Search radius in the x-axis. See note above.\n"
   "@param  yRadius Search radius in the y-axis. See note above.\n"
   "@param  zRadius Search radius in the z-axis. See note above.\n"
   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"
   "@return true if the box is empty, false if any object is found.\n"
   "@ingroup Game")
{
   Point3F extent( xRadius, yRadius, zRadius );
   extent.y = extent.y >= 0 ? extent.y : extent.x;
   extent.z = extent.z >= 0 ? extent.z : extent.x;

   Box3F    B(center - extent, center + extent, true);

   EarlyOutPolyList polyList;
   polyList.mPlaneList.clear();
   polyList.mNormal.set(0,0,0);
   polyList.mPlaneList.setSize(6);
   polyList.mPlaneList[0].set(B.minExtents, VectorF(-1,0,0));
   polyList.mPlaneList[1].set(B.maxExtents, VectorF(0,1,0));
   polyList.mPlaneList[2].set(B.maxExtents, VectorF(1,0,0));
   polyList.mPlaneList[3].set(B.minExtents, VectorF(0,-1,0));
   polyList.mPlaneList[4].set(B.minExtents, VectorF(0,0,-1));
   polyList.mPlaneList[5].set(B.maxExtents, VectorF(0,0,1));

   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   return ! pContainer->buildPolyList(PLC_Collision, B, mask, &polyList);
}

//-----------------------------------------------------------------------------

DefineEngineFunction( initContainerRadiusSearch, void, ( Point3F pos, F32 radius, U32 mask, bool useClientContainer ), ( false ),
   "@brief Start a search for items at the given position and within the given radius, filtering by mask.\n\n"

   "@param pos Center position for the search\n"
   "@param radius Search radius\n"
   "@param mask Bitmask of object types to include in the search\n"
   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"

   "@see containerSearchNext\n" 
   "@ingroup Game")
{
   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   pContainer->initRadiusSearch( pos, radius, mask );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( initContainerTypeSearch, void, ( U32 mask, bool useClientContainer ), ( false ),
   "@brief Start a search for all items of the types specified by the bitset mask.\n\n"

   "@param mask Bitmask of object types to include in the search\n"
   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"

   "@see containerSearchNext\n" 
   "@ingroup Game")
{
   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   pContainer->initTypeSearch( mask );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( containerSearchNext, SceneObject*, ( bool useClientContainer ), ( false ),
   "@brief Get next item from a search started with initContainerRadiusSearch() or "
   "initContainerTypeSearch().\n\n"

   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"
   "@return the next object found in the search, or null if no more\n"

   "@tsexample\n"
   "// print the names of all nearby ShapeBase derived objects\n"
   "%position = %obj.getPosition;\n"
   "%radius = 20;\n"
   "%mask = $TypeMasks::ShapeBaseObjectType;\n"
   "initContainerRadiusSearch( %position, %radius, %mask );\n"
   "while ( (%targetObject = containerSearchNext()) != 0 )\n"
   "{\n"
   "   echo( \"Found: \" @ %targetObject.getName() );\n"
   "}\n"
   "@endtsexample\n"

   "@see initContainerRadiusSearch()\n"
   "@see initContainerTypeSearch()\n"
   "@ingroup Game")
{
   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   return pContainer->containerSearchNextObject();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( containerSearchCurrDist, F32, ( bool useClientContainer ), ( false ),
   "@brief Get distance of the center of the current item from the center of the "
   "current initContainerRadiusSearch.\n\n"

   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"
   "@return distance from the center of the current object to the center of "
   "the search\n"

   "@see containerSearchNext\n"
   "@ingroup Game")
{
   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   return pContainer->containerSearchCurrDist();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( containerSearchCurrRadiusDist, F32, ( bool useClientContainer ), ( false ),
   "@brief Get the distance of the closest point of the current item from the center "
   "of the current initContainerRadiusSearch.\n\n"

   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"
   "@return distance from the closest point of the current object to the "
   "center of the search\n"

   "@see containerSearchNext\n" 
   "@ingroup Game")
{
   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   return pContainer->containerSearchCurrRadiusDist();
}

//-----------------------------------------------------------------------------

//TODO: make RayInfo an API type
DefineEngineFunction( containerRayCast, const char*,
   ( Point3F start, Point3F end, U32 mask, SceneObject *pExempt, bool useClientContainer ), ( NULL, false ),
   "@brief Cast a ray from start to end, checking for collision against items matching mask.\n\n"

   "If pExempt is specified, then it is temporarily excluded from collision checks (For "
   "instance, you might want to exclude the player if said player was firing a weapon.)\n"

   "@param start An XYZ vector containing the tail position of the ray.\n"
   "@param end An XYZ vector containing the head position of the ray\n"
   "@param mask A bitmask corresponding to the type of objects to check for\n"
   "@param pExempt An optional ID for a single object that ignored for this raycast\n"
   "@param useClientContainer Optionally indicates the search should be within the "
   "client container.\n"

   "@returns A string containing either null, if nothing was struck, or these fields:\n"
   "<ul><li>The ID of the object that was struck.</li>"
   "<li>The x, y, z position that it was struck.</li>"
   "<li>The x, y, z of the normal of the face that was struck.</li>"
   "<li>The distance between the start point and the position we hit.</li></ul>" 

   "@ingroup Game")
{
   if (pExempt)
      pExempt->disableCollision();

   SceneContainer* pContainer = useClientContainer ? &gClientContainer : &gServerContainer;

   RayInfo rinfo;
   S32 ret = 0;
   if (pContainer->castRay(start, end, mask, &rinfo) == true)
      ret = rinfo.object->getId();

   if (pExempt)
      pExempt->enableCollision();

   // add the hit position and normal?
   static const U32 bufSize = 256;
   char *returnBuffer = Con::getReturnBuffer(bufSize);
   if(ret)
   {
      dSprintf(returnBuffer, bufSize, "%d %g %g %g %g %g %g %g",
               ret, rinfo.point.x, rinfo.point.y, rinfo.point.z,
               rinfo.normal.x, rinfo.normal.y, rinfo.normal.z, rinfo.distance);
   }
   else
   {
      returnBuffer[0] = '0';
      returnBuffer[1] = '\0';
   }

   return(returnBuffer);
}

ConsoleFunctionGroupEnd( Containers );
