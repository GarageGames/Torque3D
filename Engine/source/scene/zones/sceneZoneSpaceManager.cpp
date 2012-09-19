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
#include "scene/zones/sceneZoneSpaceManager.h"

#include "platform/profiler.h"
#include "platform/platformMemory.h"
#include "scene/sceneContainer.h"
#include "scene/zones/sceneRootZone.h"
#include "scene/zones/sceneZoneSpace.h"


// Uncomment to enable verification code for debugging.  This slows the
// manager down significantly but will allow to find zoning state corruption
// much quicker.
//#define DEBUG_VERIFY

//#define DEBUG_SPEW


ClassChunker< SceneObject::ZoneRef > SceneZoneSpaceManager::smZoneRefChunker;


//-----------------------------------------------------------------------------

SceneZoneSpaceManager::SceneZoneSpaceManager( SceneContainer* container )
   : mContainer( container ),
     mRootZone( new SceneRootZone() ),
     mNumTotalAllocatedZones( 0 ),
     mNumActiveZones( 0 ),
     mDirtyArea( Box3F::Invalid )
{
   VECTOR_SET_ASSOCIATION( mZoneSpaces );
   VECTOR_SET_ASSOCIATION( mZoneLists );
   VECTOR_SET_ASSOCIATION( mZoneSpacesQueryList );
   VECTOR_SET_ASSOCIATION( mDirtyObjects );
   VECTOR_SET_ASSOCIATION( mDirtyZoneSpaces );
}

//-----------------------------------------------------------------------------

SceneZoneSpaceManager::~SceneZoneSpaceManager()
{
   // Delete root zone.
   SAFE_DELETE( mRootZone );

   mNumTotalAllocatedZones = 0;
   mNumActiveZones = 0; 
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::registerZones( SceneZoneSpace* object, U32 numZones )
{
   AssertFatal( _getZoneSpaceIndex( object ) == -1, "SceneZoneSpaceManager::registerZones - Object already registered" );
   _compactZonesCheck();

   const U32 zoneRangeStart = mNumTotalAllocatedZones;

   mNumTotalAllocatedZones += numZones;
   mNumActiveZones += numZones;

   object->mNumZones = numZones;
   object->mZoneRangeStart = zoneRangeStart;

   // Allocate zone lists for all of the zones managed by the object.
   // Add an entry to each list that points back to the zone space.

   mZoneLists.increment( numZones );
   for( U32 i = zoneRangeStart; i < mNumTotalAllocatedZones; ++ i )
   {
      SceneObject::ZoneRef* zoneRef = smZoneRefChunker.alloc();

      zoneRef->object    = object;
      zoneRef->nextInBin = NULL;
      zoneRef->prevInBin = NULL;
      zoneRef->nextInObj = NULL;
      zoneRef->zone      = i;

      mZoneLists[ i ] = zoneRef;
   }

   // Add space to list.

   mZoneSpaces.push_back( object );
   object->mManager = this;

   // Set ZoneObjectType.

   object->mTypeMask |= ZoneObjectType;

   // Put the object on the dirty list.

   if( !object->isRootZone() )
   {
      // Make sure the object gets on the zone space list even
      // if it is already on the object dirty list.
      object->mZoneRefDirty = false;

      notifyObjectChanged( object );
   }

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SceneZoneSpaceManager] Range %i-%i allocated to: %s",
      zoneRangeStart, numZones, object->describeSelf().c_str() );
   #endif
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::unregisterZones( SceneZoneSpace* object )
{
   S32 zoneSpaceIndex = _getZoneSpaceIndex( object );
   
   AssertFatal( zoneSpaceIndex != -1, "SceneZoneSpaceManager::unregisterZones - Object not registered as zone space" );
   AssertFatal( mNumActiveZones >= object->mNumZones, "SceneZoneSpaceManager::unregisterZones - Too many zones removed");

   const U32 zoneRangeStart = object->getZoneRangeStart();
   const U32 numZones = object->getZoneRange();

   // Destroy the zone lists for the zones registered
   // by the object.

   for( U32 j = zoneRangeStart; j < zoneRangeStart + numZones; j ++ )
   {
      // Delete all object links.

      _clearZoneList( j );

      // Delete the first link which refers to the zone itself.

      smZoneRefChunker.free( mZoneLists[ j ] );
      mZoneLists[ j ] = NULL;
   }

   // Destroy the connections the zone space has.

   object->_disconnectAllZoneSpaces();

   // Remove the zone manager entry.

   mNumActiveZones -= numZones;
   mZoneSpaces.erase( zoneSpaceIndex );

   // Clear ZoneObjectType.

   object->mTypeMask &= ~ZoneObjectType;

   // Clear zone assignments.

   object->mZoneRangeStart = InvalidZoneId;
   object->mNumZones = 0;
   object->mManager = NULL;

   // Mark the zone space's area as dirty.

   mDirtyArea.intersect( object->getWorldBox() );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SceneZoneSpaceManager] Range %i-%i released from: %s",
      zoneRangeStart, numZones, object->describeSelf().c_str() );
   #endif
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_rezoneObjects( const Box3F& area )
{
   static Vector< SceneObject* > sObjects( __FILE__, __LINE__ );

   // Find all objects in the area.  We cannot use the callback
   // version here and directly trigger rezoning since the rezoning
   // itself does a container query.

   sObjects.clear();
   mContainer->findObjectList( area, 0xFFFFFFFF, &sObjects );

   // Rezone the objects.

   const U32 numObjects = sObjects.size();
   for( U32 i = 0; i < numObjects; ++ i )
   {
      SceneObject* object = sObjects[ i ];
      if( object != getRootZone() )
         _rezoneObject( object );
   }
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_compactZonesCheck()
{
   if( mNumActiveZones > ( mNumTotalAllocatedZones / 2 ) )
      return;

   // Redistribute the zone IDs among the current zone spaces
   // so that the range of IDs is consecutive.

   const U32 numZoneSpaces = mZoneSpaces.size();
   U32 nextZoneId = 0;
   
   Vector< SceneObject::ZoneRef* > newZoneLists;
   newZoneLists.setSize( mNumActiveZones );

   for( U32 i = 0; i < numZoneSpaces; ++ i )
   {
      SceneZoneSpace* space = mZoneSpaces[ i ];

      const U32 oldZoneRangeStart = space->getZoneRangeStart();
      const U32 newZoneRangeStart = nextZoneId;
      const U32 numZones = space->getZoneRange();

      // Assign the new zone range start.

      space->mZoneRangeStart = newZoneRangeStart;
      nextZoneId += numZones;

      // Relocate the zone lists to match the new zone IDs and update
      // the contents of the zone lists to match the new IDs.

      for( U32 n = 0; n < numZones; ++ n )
      {
         const U32 newZoneId = newZoneRangeStart + n;
         const U32 oldZoneId = oldZoneRangeStart + n;

         // Relocate list.

         newZoneLists[ newZoneId ] = mZoneLists[ oldZoneId ];

         // Update entries.

         for( SceneObject::ZoneRef* ref = newZoneLists[ newZoneId ]; ref != NULL; ref = ref->nextInBin )
            ref->zone = newZoneId;
      }
   }

   mNumTotalAllocatedZones = nextZoneId;
   mZoneLists = newZoneLists;

   AssertFatal( mNumTotalAllocatedZones == mNumActiveZones, "SceneZoneSpaceManager::_compactZonesCheck - Error during compact; mismatch between active and allocated zones" );
}

//-----------------------------------------------------------------------------

S32 SceneZoneSpaceManager::_getZoneSpaceIndex( SceneZoneSpace* object ) const
{
   const U32 numZoneSpaces = getNumZoneSpaces();
   for( U32 i = 0; i < numZoneSpaces; ++ i )
      if( mZoneSpaces[ i ] == object )
         return i;

   return -1;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::findZone( const Point3F& p, SceneZoneSpace*& owner, U32& zone ) const
{
   AssertFatal( mNumActiveZones >= 1, "SceneZoneSpaceManager::findZone - Must have at least one active zone in scene (outdoor zone)" );

   // If there are no zones in the level other than the outdoor
   // zone, just return that.

   if( mNumActiveZones == 1 )
   {
      owner = getRootZone();
      zone = RootZoneId;

      return;
   }
   
   PROFILE_SCOPE( SceneZoneSpaceManager_findZone );

   // Query the scene container for zones with a query
   // box that tightly fits around the point.

   Box3F queryBox(   p.x - 0.1f, p.y - 0.1f, p.z - 0.1f,
                     p.x + 0.1f, p.y + 0.1f, p.z + 0.1f );

   _queryZoneSpaces( queryBox );

   // Go through the zones and look for the first one that
   // contains the given point.

   const U32 numZones = mZoneSpacesQueryList.size();
   for( U32 i = 0; i < numZones; ++ i )
   {
      SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( mZoneSpacesQueryList[ i ] );
      if( !zoneSpace )
         continue;

      AssertFatal( zoneSpace != getRootZone(), "SceneZoneSpaceManager::findZone - SceneRootZone returned by zone manager query" );

      // If the point is in one of the zones of this manager,
      // then make this the result.

      U32 inZone = zoneSpace->getPointZone( p );
      if( inZone != InvalidZoneId )
      {
         owner = zoneSpace;
         zone = inZone;

         return;
      }
   }

   // No other zone matched so return the outdoor zone.

   owner = getRootZone();
   zone = RootZoneId;
}

//-----------------------------------------------------------------------------

U32 SceneZoneSpaceManager::findZones( const Box3F& area, Vector< U32 >& outZones ) const
{
   // Query all zone spaces in the area.

   _queryZoneSpaces( area );

   // Query each zone space for overlaps with the given
   // area and add the zones to outZones.

   bool outsideIncluded = false;
   U32 numTotalZones = 0;

   const U32 numZoneSpaces = mZoneSpacesQueryList.size();
   for( U32 i = 0; i < numZoneSpaces; ++ i )
   {
      SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( mZoneSpacesQueryList[ i ] );
      if( !zoneSpace )
         continue;

      AssertFatal( zoneSpace != getRootZone(), "SceneZoneSpaceManager::findZones - SceneRootZone returned by zone manager query" );

      // Query manager.

      U32 zones[ SceneObject::MaxObjectZones ];
      U32 numZones = 0;

      outsideIncluded |= zoneSpace->getOverlappingZones( area, zones, numZones );

      // Add overlapped zones.
      
      for( U32 n = 0; n < numZones; n ++ )
      {
         outZones.push_back( zones[ n ] );
         numTotalZones ++;
      }
   }

   // If the area box wasn't fully enclosed by the zones of the
   // manager(s) or the query only returned the outside zone,
   // add the outside zone to the list.

   if( outsideIncluded || numTotalZones == 0 )
   {
      outZones.push_back( RootZoneId );
      numTotalZones ++;
   }

   return numTotalZones;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_rezoneObject( SceneObject* object )
{
   PROFILE_SCOPE( SceneZoneSpaceManager_rezoneObject );

   AssertFatal( !dynamic_cast< SceneRootZone* >( object ), "SceneZoneSpaceManager::_rezoneObject - Cannot rezone the SceneRootZone!" );

   // If the object is not yet assigned to zones,
   // do so now and return.

   if( !object->mNumCurrZones )
   {
      _zoneInsert( object );
      return;
   }

   // If we have no zones in the scene other than the outdoor zone or if the
   // object has global bounds on (and thus is always in the outdoor zone) or
   // is an object that is restricted to the outdoor zone, leave the object's
   // zoning state untouched.

   if( mNumActiveZones == 1 || object->isGlobalBounds() || object->getTypeMask() & OUTDOOR_OBJECT_TYPEMASK )
   {
      object->mZoneRefDirty = false;
      return;
   }

   // First, find out whether there's even a chance of the zoning to have changed
   // for the object.

   _queryZoneSpaces( object->getWorldBox() );

   const U32 numZoneSpaces = mZoneSpacesQueryList.size();
   if( !numZoneSpaces )
   {
      // There is no zone in the object's area.  If it is already assigned to the
      // root zone, then we don't need an update.  Otherwise, we do.

      if( object->mNumCurrZones == 1 &&
          object->mZoneRefHead &&
          object->mZoneRefHead->zone == RootZoneId )
      {
         object->mZoneRefDirty = false;
         return;
      }
   }

   // Update the object's zoning information by removing and recomputing
   // its zoning information.

   _zoneRemove( object );
   _zoneInsert( object, true ); // Query already in place.
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::registerObject( SceneObject* object )
{
   // Just put it on the dirty list.

   notifyObjectChanged( object );
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::unregisterObject( SceneObject* object )
{
   // Remove from dirty list.

   mDirtyObjects.remove( object );

   // Remove from zone lists.

   _zoneRemove( object );

   // If it's a zone space, unregister it.

   if( object->getTypeMask() & ZoneObjectType && dynamic_cast< SceneZoneSpace* >( object ) )
   {
      SceneZoneSpace* zoneSpace = static_cast< SceneZoneSpace* >( object );
      unregisterZones( zoneSpace );
      mDirtyZoneSpaces.remove( zoneSpace );
   }
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::updateObject( SceneObject* object )
{
   // If no zone spaces have changed and the object's zoning
   // state is clean, then there's nothing to do for this object.

   if( mDirtyZoneSpaces.empty() && !object->mZoneRefDirty )
      return;

   // Otherwise update all the dirty zoning state.

   updateZoningState();
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::notifyObjectChanged( SceneObject* object )
{
   AssertFatal( object != getRootZone(), "SceneZoneSpaceManager::notifyObjectChanged - Cannot dirty root zone!" );

   // Ignore if object is already on the dirty list.

   if( object->mZoneRefDirty )
      return;

   // Put the object on the respective dirty list.

   if( object->getTypeMask() & ZoneObjectType )
   {
      SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( object );
      AssertFatal( zoneSpace != NULL, "SceneZoneSpaceManager::notifyObjectChanged - ZoneObjectType is not a SceneZoneSpace!" );

      if( zoneSpace )
         mDirtyZoneSpaces.push_back( zoneSpace );
   }
   else
   {
      mDirtyObjects.push_back( object );
   }

   // Mark object as dirty.

   object->mZoneRefDirty = true;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::updateZoningState()
{
   // If there are no dirty objects, there's nothing to do.

   if( mDirtyObjects.empty() &&
       mDirtyZoneSpaces.empty() &&
       mDirtyArea == Box3F::Invalid )
      return;

   // Otherwise, first update the zone spaces.  Do this in two passes:
   // first take all the dirty zone spaces out of the zoning state and
   // then rezone the combined area of all dirty zone spaces.
   //
   // Note that this path here is pretty much only relevant during loading
   // or editing and thus can be less performant than the path for individual
   // objects below.

   while( !mDirtyZoneSpaces.empty() )
   {
      SceneZoneSpace* zoneSpace = mDirtyZoneSpaces.last();
      mDirtyZoneSpaces.decrement();
      
      // Remove the zoning state of the object.

      _zoneRemove( zoneSpace );

      // Destroy all connections that this zone space has to
      // other zone spaces.

      zoneSpace->_disconnectAllZoneSpaces();

      // Nuke its zone lists.

      const U32 numZones = zoneSpace->getZoneRange();
      for( U32 n = 0; n < numZones; ++ n )
         _clearZoneList( zoneSpace->getZoneRangeStart() + n );

      // Merge into dirty region.

      mDirtyArea.intersect( zoneSpace->getWorldBox() );
   }

   if( mDirtyArea != Box3F::Invalid )
   {
      // Rezone everything in the dirty region.

      _rezoneObjects( mDirtyArea );
      mDirtyArea = Box3F::Invalid;

      // Verify zoning state.

      #ifdef DEBUG_VERIFY
      verifyState();
      #endif

      // Fire the zoning changed signal to let interested parties
      // know that the zoning setup of the scene has changed.

      getZoningChangedSignal().trigger( this );
   }

   // And finally, update objects that have changed state.

   while( !mDirtyObjects.empty() )
   {
      SceneObject* object = mDirtyObjects.last();
      mDirtyObjects.decrement();

      if( object->mZoneRefDirty )
         _rezoneObject( object );

      AssertFatal( !object->mZoneRefDirty, "SceneZoneSpaceManager::updateZoningState - Object still dirty!" );
   }

   AssertFatal( mDirtyObjects.empty(), "SceneZoneSpaceManager::updateZoningState - Still have dirty objects!" );
   AssertFatal( mDirtyZoneSpaces.empty(), "SceneZoneSpaceManager::updateZoningState - Still have dirty zones!" );
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_zoneInsert( SceneObject* object, bool queryListInitialized ) 
{
   PROFILE_SCOPE( SceneZoneSpaceManager_zoneInsert );

   AssertFatal( object->mNumCurrZones == 0, "SceneZoneSpaceManager::_zoneInsert - Object already in zone list" );
   AssertFatal( object->getContainer() != NULL, "SceneZoneSpaceManager::_zoneInsert - Object must be in scene" );
   AssertFatal( !dynamic_cast< SceneRootZone* >( object ), "SceneZoneSpaceManager::_zoneInsert - Must not be called on SceneRootZone" );

   // If all we have is a single zone in the scene, then it must
   // be the outdoor zone.  Simply assign the object to it.  Also do this
   // if the object has global bounds on since we always assign these to
   // just the outdoor zone.  Finally, also do it for all object types that
   // we want to restrict to the outdoor zone.

   if( mNumActiveZones == 1 || object->isGlobalBounds() || object->getTypeMask() & OUTDOOR_OBJECT_TYPEMASK )
      _addToOutdoorZone( object );
   else
   {
      // Otherwise find all zones spaces that intersect with the object's
      // world box.

      if( !queryListInitialized )
         _queryZoneSpaces( object->getWorldBox() );

      // Go through the zone spaces and link all zones that the object
      // overlaps.

      bool outsideIncluded = true;
      const U32 numZoneSpaces = mZoneSpacesQueryList.size();
      for( U32 i = 0; i < numZoneSpaces; ++ i )
      {
         SceneZoneSpace* zoneSpace = dynamic_cast< SceneZoneSpace* >( mZoneSpacesQueryList[ i ] );
         if( !zoneSpace )
            continue;

         AssertFatal( zoneSpace != getRootZone(), "SceneZoneSpaceManager::_zoneInsert - SceneRootZone returned by zone space query" );

         // If we are inserting a zone space, then the query will turn up
         // the object itself at some point.  Skip it.

         if( zoneSpace == object )
            continue;

         // Find the zones that the object overlaps within
         // the zone space.

         U32 numZones = 0;
         U32 zones[ SceneObject::MaxObjectZones ];

         bool overlapsOutside = zoneSpace->getOverlappingZones( object, zones, numZones );
         AssertFatal( numZones != 0 || overlapsOutside,
            "SceneZoneSpaceManager::_zoneInsert - Object must be fully contained in one or more zones or intersect the outside zone" );

         outsideIncluded &= overlapsOutside; // Only include outside if *none* of the zones fully contains the object.

         // Link the object to the zones.

         for( U32 n = 0; n < numZones; ++ n )
            _addToZoneList( zones[ n ], object );

         // Let the zone manager know we have added objects to its
         // zones.

         if( numZones > 0 )
            zoneSpace->_onZoneAddObject( object, zones, numZones );
      }

      // If the object crosses into the outside zone or hasn't been
      // added to any zone above, add it to the outside zone.

      if( outsideIncluded )
         _addToOutdoorZone( object );
   }

   // Mark the zoning state of the object as current.

   object->mZoneRefDirty = false;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_zoneRemove( SceneObject* obj )
{
   PROFILE_SCOPE( SceneZoneSpaceManager_zoneRemove );

   // Remove the object from the zone lists.

   for( SceneObject::ZoneRef* walk = obj->mZoneRefHead; walk != NULL; )
   {
      // Let the zone owner know we are removing an object
      // from its zones.

      getZoneOwner( walk->zone )->_onZoneRemoveObject( walk->object );

      // Now remove the ZoneRef link this object has in the
      // zone list of the current zone.

      SceneObject::ZoneRef* remove = walk;
      walk = walk->nextInObj;

      remove->prevInBin->nextInBin = remove->nextInBin;
      if( remove->nextInBin )
         remove->nextInBin->prevInBin = remove->prevInBin;

      smZoneRefChunker.free( remove );
   }

   // Clear the object's zoning state.

   obj->mZoneRefHead = NULL;
   obj->mZoneRefDirty = false;
   obj->mNumCurrZones = 0;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_addToZoneList( U32 zoneId, SceneObject* object )
{
   SceneObject::ZoneRef* zoneList = mZoneLists[ zoneId ];

   AssertFatal( zoneList != NULL, "SceneZoneSpaceManager::_addToZoneList - Zone list not initialized" );
   AssertFatal( object != zoneList->object, "SCene::_addToZoneList - Cannot add zone to itself" );

   SceneObject::ZoneRef* newRef = smZoneRefChunker.alloc();

   // Add the object to the zone list.

   newRef->zone      = zoneId;
   newRef->object    = object;
   newRef->nextInBin = zoneList->nextInBin;
   newRef->prevInBin = zoneList;

   if( zoneList->nextInBin )
      zoneList->nextInBin->prevInBin = newRef;

   zoneList->nextInBin = newRef;

   // Add the zone to the object list.

   newRef->nextInObj = object->mZoneRefHead;
   object->mZoneRefHead = newRef;
   object->mNumCurrZones ++;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_clearZoneList( U32 zoneId )
{
   AssertFatal( zoneId < getNumZones(), "SceneZoneSpaceManager::_clearZoneList - Zone ID out of range" );

   SceneObject::ZoneRef* list = mZoneLists[ zoneId ];
   SceneZoneSpace* zoneSpace = getZoneOwner( zoneId );

   // Go through the objects in the zone list and unlink and
   // delete their zone entries.

   for( SceneObject::ZoneRef* walk = list->nextInBin; walk != NULL; walk = walk->nextInBin )
   {
      SceneObject* object = walk->object;
      AssertFatal( object != NULL, "SceneZoneSpaceManager::_clearZoneList - Object field not set on link" );

      // The zone entry links on the objects are singly-linked lists
      // linked through nextInObject so we need to find where in the
      // objects zone entry list the node for the current zone is.

      SceneObject::ZoneRef** ptrNext = &object->mZoneRefHead;
      while( *ptrNext && *ptrNext != walk )
         ptrNext = &( *ptrNext )->nextInObj;

      AssertFatal( *ptrNext == walk, "SceneZoneSpaceManager::_clearZoneList - Zone entry not found on object in zone list!");

      // Unlink and delete the entry.

      *ptrNext = ( *ptrNext )->nextInObj;
      smZoneRefChunker.free( walk );

      object->mNumCurrZones --;

      // If this is the only zone the object was in, mark
      // its zoning state as dirty so it will get assigned
      // to the outdoor zone on the next update.

      if( !object->mZoneRefHead )
         object->mZoneRefDirty = true;

      // Let the zone know we have removed the object.

      zoneSpace->_onZoneRemoveObject( object );
   }

   list->nextInBin = NULL;
}

//-----------------------------------------------------------------------------

SceneObject::ZoneRef* SceneZoneSpaceManager::_findInZoneList( U32 zoneId, SceneObject* object ) const
{
   for( SceneObject::ZoneRef* ref = object->mZoneRefHead; ref != NULL; ref = ref->nextInObj )
      if( ref->zone == zoneId )
         return ref;

   return NULL;
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_addToOutdoorZone( SceneObject* object )
{
   AssertFatal( !object->mZoneRefHead || !_findInZoneList( RootZoneId, object ),
      "SceneZoneSpaceManager::_addToOutdoorZone - Object already added to outdoor zone" );

   // Add the object to the outside's zone list.  This method is always called
   // *last* after the object has already been assigned to any other zone it
   // intersects.  Since we always prepend to the zoning lists, this means that
   // the outdoor zone will always be *first* in the list of zones that an object
   // is assigned to which generally is a good order.

   _addToZoneList( RootZoneId, object );

   // Let the zone know we added an object to it.

   const U32 zoneId = RootZoneId;
   static_cast< SceneZoneSpace* >( getRootZone() )->_onZoneAddObject( object, &zoneId, 1 );
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::_queryZoneSpaces( const Box3F& area ) const
{
   mZoneSpacesQueryList.clear();
   mContainer->findObjectList( area, ZoneObjectType, &mZoneSpacesQueryList );
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::dumpZoneStates( bool update )
{
   if( update )
      _rezoneObjects( getRootZone()->getWorldBox() );

   const U32 numZoneSpaces = mZoneSpaces.size();
   for( U32 i = 0; i < numZoneSpaces; ++ i )
      mZoneSpaces[ i ]->dumpZoneState( false );
}

//-----------------------------------------------------------------------------

void SceneZoneSpaceManager::verifyState()
{
   AssertFatal( mZoneSpaces.size() <= mNumActiveZones,
      "SceneZoneSpaceManager::verifyState - More zone spaces than active zones!" );
   AssertFatal( mNumTotalAllocatedZones >= mNumActiveZones,
      "SceneZoneSpaceManager::verifyState - Fewer allocated than active zones!" );
   AssertFatal( mRootZone->getZoneRangeStart() == 0,
      "SceneZoneSpaceManager::verifyState - Invalid ID on root zone!" );
   AssertFatal( mRootZone->getZoneRange() == 1,
      "SceneZoneSpaceManager::verifyState - Invalid zone range on root zone!" );

   // First validate the zone spaces themselves.

   const U32 numZoneSpaces = mZoneSpaces.size();
   for( U32 i = 0; i < numZoneSpaces; ++ i )
   {
      SceneZoneSpace* space = mZoneSpaces[ i ];

      #ifndef TORQUE_DISABLE_MEMORY_MANAGER
      Memory::checkPtr( space );
      #endif

      AssertFatal( space->getTypeMask() & ZoneObjectType, "SceneZoneSpaceManager::verifyState - Zone space is not a ZoneObjectType!" );

      const U32 zoneRangeStart = space->getZoneRangeStart();
      const U32 numZones = space->getZoneRange();

      // Verify each of the allocated zones in this space.

      for( U32 n = 0; n < numZones; ++ n )
      {
         const U32 zoneId = zoneRangeStart + n;

         // Simple validation of zone ID.
         AssertFatal( isValidZoneId( zoneId ), "SceneZoneSpaceManager::verifyState - Zone space is assigned in invalid zone ID!" );

         AssertFatal( mZoneLists[ zoneId ] != NULL, "SceneZoneSpaceManager::verifyState - Zone list missing for zone!" );
         AssertFatal( mZoneLists[ zoneId ]->object == space, "SceneZoneSpaceManager::verifyState - Zone list entry #0 is not referring back to zone!" );

         for( SceneObject::ZoneRef* ref = mZoneLists[ zoneId ]; ref != NULL; ref = ref->nextInBin )
         {
            AssertFatal( ref->zone == zoneId, "SceneZoneSpaceManager::verifyState - Incorrect ID in zone list!" );
            AssertFatal( ref->object != NULL, "SceneZoneSpaceManager::verifyState - Null object pointer in zone list!" );

            #ifndef TORQUE_DISABLE_MEMORY_MANAGER
            Memory::checkPtr( ref->object );
            #endif
         }
      }

      // Make sure no other zone space owns any of the same IDs.

      for( U32 n = 0; n < numZoneSpaces; ++ n )
      {
         if( n == i )
            continue;

         SceneZoneSpace* otherSpace = mZoneSpaces[ n ];
         AssertFatal( otherSpace->getZoneRangeStart() >= zoneRangeStart + numZones ||
                      otherSpace->getZoneRangeStart() + otherSpace->getZoneRange() <= zoneRangeStart,
            "SceneZoneSpaceManager::verifyState - Overlap between zone ID ranges of zone spaces!" );
      }

      // Make sure that all zone connections appear to be valid.

      for( SceneZoneSpace::ZoneSpaceRef* ref = space->mConnectedZoneSpaces; ref != NULL; ref = ref->mNext )
      {
         #ifndef TORQUE_DISABLE_MEMORY_MANAGER
         Memory::checkPtr( ref->mZoneSpace );
         #endif

         AssertFatal( _getZoneSpaceIndex( ref->mZoneSpace ) != -1, "SceneZoneSpaceManager::verifyState - Zone connected to invalid zone!" );
         AssertFatal( ref->mZoneSpace->getTypeMask() & ZoneObjectType, "SceneZoneSpaceManager::verifyState - Zone space is not a ZoneObjectType!" );
      }
   }

   //TODO: can do a lot more validation here
}
