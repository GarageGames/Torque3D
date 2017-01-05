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

#ifndef _SCENECULLINGSTATE_H_
#define _SCENECULLINGSTATE_H_

#ifndef _SCENEZONECULLINGSTATE_H_
#include "scene/culling/sceneZoneCullingState.h"
#endif

#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _SCENECAMERASTATE_H_
#include "scene/sceneCameraState.h"
#endif

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif

#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif


class SceneObject;
class SceneManager;


/// An object that gathers the culling state for a scene.
class SceneCullingState
{
   public:

      /// Used to disable the somewhat expensive terrain occlusion testing
      /// done in during scene culling.
      static bool smDisableTerrainOcclusion;

      /// Whether to force zone culling to off by default.
      static bool smDisableZoneCulling;

      /// @name Occluder Restrictions
      /// Size restrictions on occlusion culling volumes.  Any occlusion volume
      /// that does not meet these minimum requirements is not accepted into the
      /// rendering state.
      ///
      /// Having independent restrictions on both width and height allows filtering
      /// out occluders that might have a lot of area but only by covering very thin
      /// stretches of the screen.
      /// @{

      /// If more than this number of occlusion volumes are added to a ZoneState,
      /// then the occlusions volumes corresponding to the smallest amount of screen
      /// real estate get dropped such as to never exceed this total number of occlusion
      /// volumes.
      static U32 smMaxOccludersPerZone;

      /// Percentage of camera-space frustum near plane height that an occlusion culler must
      /// at least fill in order to not be rejected.
      /// @note The height computed for occluders is only an estimate.
      static F32 smOccluderMinHeightPercentage;

      /// Percentage of camera-space frustum near plane width that an occlusion culler must
      /// at least fill in order to not be rejected.
      /// @note The width computed for occluders is only an estimate.
      static F32 smOccluderMinWidthPercentage;

      /// @}

   protected:

      /// Scene which is being culled.
      SceneManager* mSceneManager;

      /// The viewing state that defines how the scene is being viewed.
      SceneCameraState mCameraState;

      /// The root culling volume corresponding to the culling frustum.
      SceneCullingVolume mRootVolume;

      /// The root culling frustum, which may be different from the camera frustum
      Frustum mCullingFrustum;

      /// Extra planes for culling.
      PlaneSetF mExtraPlanesCull;

      /// Occluders that have been added to this render state.  Adding an occluder does not
      /// necessarily result in an occluder volume being added.  To not repeatedly try to
      /// process the same occluder object, all objects that are added are recorded here.
      Vector< SceneObject* > mAddedOccluderObjects;

      ///
      BitVector mZoneVisibilityFlags;

      /// ZoneState entries for all zones in the scene.
      Vector< SceneZoneCullingState > mZoneStates;

      /// Allocator for culling data that can be freed in one go when
      /// the culling state is freed.
      DataChunker mDataChunker;

      /// If true, occlusion checks will not be done against the terrains
      /// in the scene.
      bool mDisableTerrainOcclusion;

      /// If true, all objects will only be tested against the root
      /// frustum.
      bool mDisableZoneCulling;

   public:

      ///
      SceneCullingState( SceneManager* sceneManager,
                         const SceneCameraState& cameraState );

      /// Return the scene which is being culled in this state.
      SceneManager* getSceneManager() const { return mSceneManager; }

      /// Return the root frustum which is used to set up scene visibility.
      const Frustum& getCullingFrustum() const { return mCullingFrustum; }

      /// Return the root frustum which is used to set up scene visibility.
      const Frustum& getCameraFrustum() const { return getCameraState().getFrustum(); }

      /// Return the viewing state that defines how the scene is being viewed.
      const SceneCameraState& getCameraState() const { return mCameraState; }
      
      /// Return the root culling volume that corresponds to the camera frustum.
      /// @note This volume omits the near and far plane of the frustum's polyhedron
      ///   as these will be tested separately during culling.  Testing them repeatedly
      ///   just wastes time.
      const SceneCullingVolume& getRootVolume() const { return mRootVolume; }

      /// @name Visibility and Occlusion
      /// @{

      enum CullOptions
      {
         /// Cull objects that have their SceneObject::DisableCullingInEditorFlag set.
         /// By default, these objects will not get culled if the editor is active.
         CullEditorOverrides = BIT( 0 ),

         /// Do not cull objects that are render-disabled.
         /// @see SceneObject::isRenderEnabled()
         DontCullRenderDisabled = BIT( 1 )
      };

      /// Cull the given list of objects according to the current culling state.
      ///
      /// @param object Array of objects.  This array will be modified in place.
      /// @param numObjects Number of objects in @a objects.
      /// @param cullOptions Combination of CullOptions.
      ///
      /// @return Number of objects remaining in the list.
      U32 cullObjects( SceneObject** objects, U32 numObjects, U32 cullOptions = 0 ) const;

      /// Return true if the given object is culled according to the current culling state.
      bool isCulled( SceneObject* object ) const { return ( cullObjects( &object, 1 ) == 0 ); }

      /// Return true if the given AABB is culled in any of the given zones.
      bool isCulled( const Box3F& aabb, const U32* zones, U32 numZones ) const;

      /// Return true if the given OBB is culled in any of the given zones.
      bool isCulled( const OrientedBox3F& obb, const U32* zones, U32 numZones ) const;

      /// Return true if the given sphere is culled in any of the given zones.
      bool isCulled( const SphereF& sphere, const U32* zones, U32 numZones ) const;

      /// Return true if the given object is occluded according to the current culling state.
      bool isOccluded( SceneObject* object ) const;

      /// Return true if the given AABB is occluded according to the current culling state.
      bool isOccluded( const Box3F& aabb, const U32* zones, U32 numZones ) const;

      /// Return true if the given OBB is occluded according to the current culling state.
      bool isOccluded( const OrientedBox3F& obb, const U32* zones, U32 numZones ) const;

      /// Return true if the given sphere is occluded according to the current culling state.
      bool isOccluded( const SphereF& sphere, const U32* zones, U32 numZones ) const;

      /// Add the occlusion information contained in the given object.
      ///
      /// @note This should only be called after all positive frustums have been added
      ///   to the zone state.
      void addOccluder( SceneObject* object );

      /// Test whether the given object is occluded by any of the terrains
      /// in the scene.
      bool isOccludedByTerrain( SceneObject* object ) const;

      /// Set whether isCulled() should do terrain occlusion checks or not.
      void setDisableTerrainOcclusion( bool value ) { mDisableTerrainOcclusion = value; }

      /// @}

      /// @name Zones
      /// @{

      /// If true, culling will only be performed against the root frustum
      /// and not against frustums of individual zones.
      ///
      /// @note This also disables occluders as these are added to the zone frustums.
      bool disableZoneCulling() const { return mDisableZoneCulling; }
      void disableZoneCulling( bool value ) { mDisableZoneCulling = value; }

      /// Return true if any of the zones that the object is currently are
      /// visible.
      bool isWithinVisibleZone( SceneObject* object ) const;

      /// Return a bit vector with one bit for each zone in the scene.  If the bit is set,
      /// the zone has includer culling volumes attached to it and thus is visible.
      const BitVector& getZoneVisibilityFlags() const { return mZoneVisibilityFlags; }

      /// Return the culling state for a particular zone.
      /// @param zoneId Numeric ID of zone.
      const SceneZoneCullingState& getZoneState( U32 zoneId ) const
      {
         AssertFatal( zoneId < ( U32 ) mZoneStates.size(), "SceneCullingState::getZoneState - Index out of bounds" );
         return mZoneStates[ zoneId ];
      }

      /// Returns the culling state for a particular zone.
      /// @param zoneId Numeric ID of zone.
      SceneZoneCullingState& getZoneState( U32 zoneId )
      {
         return const_cast< SceneZoneCullingState& >( static_cast< const SceneCullingState* >( this )->getZoneState( zoneId ) );
      }

      /// Add a culling volume to the visibility state of the given zone.
      ///
      /// @param zoneId ID of zone to which to add the given frustum's visibility information.
      /// @param volume A culling volume.  Note that the data in the volume must have
      ///   a lifetime at least as long as the culling state.
      ///
      /// @return True if the visibility state of the zone has changed, i.e. if the volume
      ///   was either added in whole or merged with an existing set of planes.  If the visibility
      ///   state of the zone has not changed, returns false.
      bool addCullingVolumeToZone( U32 zoneId, const SceneCullingVolume& volume );

      /// Copy the data from the given polyhedron to the culling state, create
      /// a new culling volume it and add it to the current culling state of the given zone.
      ///
      /// @param zoneId ID of zone to which to add the given frustum's visibility information.
      /// @param type Which type of culling volume to add.
      /// @param polyhedron Polyhedron describing the space of the culling volume.
      bool addCullingVolumeToZone( U32 zoneId, SceneCullingVolume::Type type, const AnyPolyhedron& polyhedron );

      /// Create a new culling volume by extruding the given polygon away from the viewpoint.
      ///
      /// @param vertices Array of polygon vertices.
      /// @param numVertices Number of vertices in @a vertices.
      /// @param type Type of culling volume to create.
      /// @param outVolume (out) Receives the generated volume, if successful.
      ///
      /// @return True if a volume could be generated from the given polygon or false if not.
      bool createCullingVolume( const Point3F* vertices, U32 numVertices, SceneCullingVolume::Type type, SceneCullingVolume& outVolume );

      /// @}

      /// @name Memory Management
      ///
      /// Rather than allocating a lot of individual point and plane data for the culling volumes,
      /// it is more efficient to batch allocate chunks of memory and then release all the memory
      /// for all culling volumes in one go.  This is facilitated by this interface.
      ///
      /// @{

      /// Allocate memory from this culling state.  The memory is freed when the
      /// culling state is destroyed.
      void* allocateData( U32 size ) { return mDataChunker.alloc( size ); }

      /// Allocate memory for @a num instances of T from this culling state.
      template< typename T >
      T* allocateData( U32 num ) { return reinterpret_cast< T* >( allocateData( sizeof( T ) * num ) ); }

      /// @}

      /// Queue debug visualizations of the culling volumes of all currently selected zones
      /// (or, if no zone is selected, all volumes in the outdoor zone) to the debug drawer.
      void debugRenderCullingVolumes() const;

      /// Set planes for extra culling
      void setExtraPlanesCull( const PlaneSetF &cull) { mExtraPlanesCull = cull; }

      /// Clear planes for extra culling.
      void clearExtraPlanesCull() { mExtraPlanesCull = PlaneSetF(NULL, 0); }

      /// Check extra planes culling
      bool isOccludedWithExtraPlanesCull(const Box3F &box) const
      {
         if(mExtraPlanesCull.getNumPlanes())
            return mExtraPlanesCull.testPotentialIntersection( box ) == GeometryOutside;

         return false;
      }

   private:

      typedef SceneZoneCullingState::CullingTestResult CullingTestResult;

      // Helper methods to avoid code duplication.

      template< bool OCCLUDERS_ONLY, typename T > CullingTestResult _test( const T& bounds, const U32* zones, U32 numZones ) const;
      template< typename T, typename Iter > CullingTestResult _test
         ( const T& bounds, Iter iter, const PlaneF& nearPlane, const PlaneF& farPlane ) const;
      template< typename T, typename Iter > CullingTestResult _testOccludersOnly( const T& bounds, Iter iter ) const;
};

#endif // !_SCENECULLINGSTATE_H_
