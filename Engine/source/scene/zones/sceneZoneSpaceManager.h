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

#ifndef _SCENEZONESPACEMANAGER_H_
#define _SCENEZONESPACEMANAGER_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif



class SceneContainer;
class SceneRootZone;
class SceneZoneSpace;


/// Object that manages zone spaces in a scene.
class SceneZoneSpaceManager
{
   public:

      class ZoneContentIterator;

      friend class SceneZoneSpace; // mZoneLists
      friend class ZoneContentIterator; // mZoneLists

      /// A signal used to notify that the zone setup of the scene has changed.
      ///
      /// @note If you use this signal to maintain state that depends on the zoning
      ///   setup, it's best to not immediately update the sate in response to this
      ///   signal.  The reason is that during loading and editing, the signal may
      ///   be fired a lot and continuously updating dependent data may waste a lot
      ///   of time.
      typedef Signal< void( SceneZoneSpaceManager* ) > ZoningChangedSignal;

      enum
      {
         /// Zone ID of the exterior zone.
         RootZoneId = 0,

         /// Constant to indicate an invalid zone ID.
         InvalidZoneId = 0xFFFFFFFF,
      };

      /// Iterator for the contents of a given zone.
      class ZoneContentIterator
      {
         public:

            ZoneContentIterator( SceneZoneSpaceManager* manager, int zoneId, bool upToDate = true )
            {
               AssertFatal( zoneId < manager->getNumZones(), "SceneZoneSpaceManager::ZoneContentIterator - Zone ID out of range" );

               if( upToDate )
               {
                  // Since zoning is updated lazily, the zone contents may actually
                  // be out of date.  Force an update by triggering rezoning on the
                  // zone object.  This is brute-force but this iterator is not meant
                  // to be used for high-frequency code anyway.
                  //
                  // Use the area-based rezoning so that we can also properly iterate
                  // over the contents of SceneRootZone.
                  manager->_rezoneObjects( ( ( SceneObject* ) manager->getZoneOwner( zoneId ) )->getWorldBox() );
               }

               mCurrent = manager->mZoneLists[ zoneId ]->nextInBin; // Skip zone object entry.
            }

            bool isValid() const
            {
               return ( mCurrent != NULL );
            }
            bool operator !() const
            {
               return ( mCurrent == NULL );
            }
            ZoneContentIterator& operator ++()
            {
               if( mCurrent )
                  mCurrent = mCurrent->nextInBin;
               return *this;
            }
            ZoneContentIterator& operator --()
            {
               if( mCurrent )
                  mCurrent = mCurrent->prevInBin;
               return *this;
            }
            SceneObject* operator *() const
            {
               AssertFatal( mCurrent != NULL, "SceneManager::ZoneContentIterator::operator* - Invalid iterator" );
               return mCurrent->object;
            }
            SceneObject* operator ->() const
            {
               AssertFatal( mCurrent != NULL, "SceneManager::ZoneContentIterator::operator-> - Invalid iterator" );
               return mCurrent->object;
            }

         private:

            SceneObject::ZoneRef* mCurrent;
      };

   protected:

      /// The root and outdoor zone of the scene.
      SceneRootZone* mRootZone;

      /// Scene container that holds the zone spaces we are managing.
      SceneContainer* mContainer;

      /// Collection of objects that manage zones.
      Vector< SceneZoneSpace* > mZoneSpaces;

      /// Total number of zones that have been allocated in the scene.
      U32 mNumTotalAllocatedZones;

      /// Number of zone IDs that are in active use.
      U32 mNumActiveZones;

      /// Object list for each zone in the scene.
      /// First entry in the list points back to the zone manager.
      Vector< SceneObject::ZoneRef* > mZoneLists;

      /// Vector used repeatedly for zone space queries on the container.
      mutable Vector< SceneObject* > mZoneSpacesQueryList;

      /// Allocator for ZoneRefs.
      static ClassChunker< SceneObject::ZoneRef > smZoneRefChunker;

      /// @name Dirty Lists
      /// Updating the zoning state of a scene is done en block rather than
      /// individually for each object as it changes transforms or size.
      /// @{

      /// Area of the scene that needs to be rezoned.
      Box3F mDirtyArea;

      /// List of zone spaces that have changed state and need updating.
      Vector< SceneZoneSpace* > mDirtyZoneSpaces;

      /// List of objects (non-zone spaces) that have changed state and need
      /// updating.
      Vector< SceneObject* > mDirtyObjects;

      /// @}

      /// Check to see if we have accumulated a lot of unallocate zone IDs and if so,
      /// compact the zoning lists by reassigning IDs.
      ///
      /// @warn This method may alter all zone IDs in the scene!
      void _compactZonesCheck();

      /// Return the index into #mZoneSpaces for the given object or -1 if
      /// @object is not a zone manager.
      S32 _getZoneSpaceIndex( SceneZoneSpace* object ) const;

      /// Attach zoning state to the given object.
      void _zoneInsert( SceneObject* object, bool queryListInitialized = false );

      /// Detach zoning state from the given object.
      void _zoneRemove( SceneObject* object );

      /// Add to given object to the zone list of the given zone.
      void _addToZoneList( U32 zoneId, SceneObject* object );

      /// Clear all objects assigned to the given zone.
      /// @note This does not remove the first link in the zone list which is the link
      ///   back to the zone manager.
      void _clearZoneList( U32 zoneId );

      /// Find the given object in the zone list of the given zone.
      SceneObject::ZoneRef* _findInZoneList( U32 zoneId, SceneObject* object ) const;

      /// Assign the given object to the outdoor zone.
      void _addToOutdoorZone( SceneObject* object );

      /// Rezone all objects in the given area.
      void _rezoneObjects( const Box3F& area );

      /// Update the zoning state of the given object.
      void _rezoneObject( SceneObject* object );

      /// Fill #mZoneSpacesQueryList with all ZoneObjectType objects in the given area.
      void _queryZoneSpaces( const Box3F& area ) const;

   public:

      SceneZoneSpaceManager( SceneContainer* container );
      ~SceneZoneSpaceManager();

      /// Bring the zoning state of the scene up to date.  This will cause objects
      /// that have moved or have been resized to be rezoned and will updated regions
      /// of the scene that had their zoning setup changed.
      ///
      /// @note This method depends on proper use of notifyObjectChanged().
      void updateZoningState();

      /// @name Objects
      /// @{

      /// Add zoning state to the given object.
      void registerObject( SceneObject* object );

      /// Remove the given object from the zoning state.
      void unregisterObject( SceneObject* object );

      /// Let the manager know that state relevant to zoning of the given
      /// object has changed.
      void notifyObjectChanged( SceneObject* object );

      /// Update the zoning state of the given object.
      void updateObject( SceneObject* object );

      /// @}

      /// @name Zones
      /// @{

      /// Return the root zone of the scene.
      SceneRootZone* getRootZone() const { return mRootZone; }

      /// Register a zone manager.
      ///
      /// @param object SceneZoneSpace object that contains zones.
      /// @param numZones Number of zones that @a object contains.
      void registerZones( SceneZoneSpace* object, U32 numZones );

      /// Unregister a zone manager.
      ///
      /// @param object Object that contains zones.
      void unregisterZones( SceneZoneSpace* object );

      /// Return true if the given ID belongs to a currently registered zone.
      bool isValidZoneId( const U32 zoneId ) const
      {
         return ( zoneId < mNumTotalAllocatedZones && mZoneLists[ zoneId ] );
      }

      /// Get the scene object that contains the zone with the given ID.
      ///
      /// @param zoneId ID of the zone.  Must be valid.
      /// @return The zone space that has registered the given zone.
      SceneZoneSpace* getZoneOwner( const U32 zoneId ) const
      {
         AssertFatal( isValidZoneId( zoneId ), "SceneManager::getZoneOwner - Invalid zone ID!");
         return ( SceneZoneSpace* ) mZoneLists[ zoneId ]->object;
      }

      /// Return the total number of zones in the scene.
      U32 getNumZones() const { return mNumTotalAllocatedZones; }

      /// Return the effective amount of used zone IDs in the scene.
      U32 getNumActiveZones() const { return mNumActiveZones; }

      /// Return the total number of objects in the scene that manage zones.
      U32 getNumZoneSpaces() const { return mZoneSpaces.size(); }

      /// Find the zone that contains the given point.
      ///
      /// Note that the result can be <em>any</em> zone containing the given
      /// point.
      void findZone( const Point3F& point, SceneZoneSpace*& outZoneSpace, U32& outZoneID ) const;

      /// Collect the IDs of all zones that overlap the given area.
      ///
      /// @param area AABB of scene space to query.
      /// @param outZones IDs of all overlapped zones are added to this vector.
      ///
      /// @return Number of zones that have been added to @a outZones.  Always at least
      ///   1 as at least the outdoor zone always overlaps the given area (though if another zone
      ///   manager fully contains @a area, the outdoor zone will not be added to the list).
      U32 findZones( const Box3F& area, Vector< U32 >& outZones ) const;

      static ZoningChangedSignal& getZoningChangedSignal()
      {
         static ZoningChangedSignal sSignal;
         return sSignal;
      }

      /// @name Debugging
      /// @{

      /// Verify the current zoning state.  This makes sure all the connectivity
      /// information and all the zone assignments appear to be correct.
      void verifyState();

      /// Dump the current state of all zone spaces in the scene to the console.
      /// @param update If true, zoning state states are updated first; if false, zoning is
      ///   dumped as is.
      void dumpZoneStates( bool update = true );

      /// @}

      /// @}
};

#endif // !_SCENEZONESPACEMANAGER_H_
