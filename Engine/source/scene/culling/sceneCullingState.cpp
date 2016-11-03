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
#include "scene/culling/sceneCullingState.h"

#include "scene/sceneManager.h"
#include "scene/sceneObject.h"
#include "scene/zones/sceneZoneSpace.h"
#include "math/mathUtils.h"
#include "platform/profiler.h"
#include "terrain/terrData.h"
#include "util/tempAlloc.h"
#include "gfx/sim/debugDraw.h"


extern bool gEditingMission;


bool SceneCullingState::smDisableTerrainOcclusion = false;
bool SceneCullingState::smDisableZoneCulling = false;
U32 SceneCullingState::smMaxOccludersPerZone = 4;
F32 SceneCullingState::smOccluderMinWidthPercentage = 0.1f;
F32 SceneCullingState::smOccluderMinHeightPercentage = 0.1f;



//-----------------------------------------------------------------------------

SceneCullingState::SceneCullingState( SceneManager* sceneManager, const SceneCameraState& viewState )
   : mSceneManager( sceneManager ),
     mCameraState( viewState ),
     mDisableZoneCulling( smDisableZoneCulling ),
     mDisableTerrainOcclusion( smDisableTerrainOcclusion )
{
   AssertFatal( sceneManager->getZoneManager(), "SceneCullingState::SceneCullingState - SceneManager must have a zone manager!" );

   VECTOR_SET_ASSOCIATION( mZoneStates );
   VECTOR_SET_ASSOCIATION( mAddedOccluderObjects );

   // Allocate zone states.

   const U32 numZones = sceneManager->getZoneManager()->getNumZones();
   mZoneStates.setSize( numZones );
   dMemset( mZoneStates.address(), 0, sizeof( SceneZoneCullingState ) * numZones );

   // Allocate the zone visibility flags.

   mZoneVisibilityFlags.setSize( numZones );
   mZoneVisibilityFlags.clear();

   // Culling frustum

   mCullingFrustum = mCameraState.getFrustum();
   mCullingFrustum.bakeProjectionOffset();

   // Construct the root culling volume from
   // the culling frustum.  Omit the frustum's
   // near and far plane so we don't test it repeatedly.

   PlaneF* planes = allocateData< PlaneF >( 4 );

   planes[ 0 ] = mCullingFrustum.getPlanes()[ Frustum::PlaneLeft ];
   planes[ 1 ] = mCullingFrustum.getPlanes()[ Frustum::PlaneRight ];
   planes[ 2 ] = mCullingFrustum.getPlanes()[ Frustum::PlaneTop];
   planes[ 3 ] = mCullingFrustum.getPlanes()[ Frustum::PlaneBottom ];

   mRootVolume = SceneCullingVolume(
      SceneCullingVolume::Includer,
      PlaneSetF( planes, 4 )
   );

   clearExtraPlanesCull();
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isWithinVisibleZone( SceneObject* object ) const
{
   for(  SceneObject::ZoneRef* ref = object->_getZoneRefHead();
         ref != NULL; ref = ref->nextInObj )
      if( mZoneVisibilityFlags.test( ref->zone ) )
         return true;

   return false;
}

//-----------------------------------------------------------------------------

void SceneCullingState::addOccluder( SceneObject* object )
{
   PROFILE_SCOPE( SceneCullingState_addOccluder );

   // If the occluder is itself occluded, don't add it.
   //
   // NOTE: We do allow near plane intersections here.  Silhouette extraction
   //    should take that into account.

   if( cullObjects( &object, 1, DontCullRenderDisabled ) != 1 )
      return;

   // If the occluder has already been added, do nothing.  Check this
   // after the culling check since the same occluder can be added by
   // two separate zones and not be visible in one yet be visible in the
   // other.

   if( mAddedOccluderObjects.contains( object ) )
      return;
   mAddedOccluderObjects.push_back( object );

   // Let the object build a silhouette.  If it doesn't
   // return one, abort.

   Vector< Point3F > silhouette;
   object->buildSilhouette( getCameraState(), silhouette );

   if( silhouette.empty() || silhouette.size() < 3 )
      return;

   // Generate the culling volume.

   SceneCullingVolume volume;
   if( !createCullingVolume(
         silhouette.address(),
         silhouette.size(),
         SceneCullingVolume::Occluder,
         volume ) )
      return;

   // Add the frustum to all zones that the object is assigned to.

   for( SceneObject::ZoneRef* ref = object->_getZoneRefHead(); ref != NULL; ref = ref->nextInObj )
      addCullingVolumeToZone( ref->zone, volume );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::addCullingVolumeToZone( U32 zoneId, const SceneCullingVolume& volume )
{
   PROFILE_SCOPE( SceneCullingState_addCullingVolumeToZone );

   AssertFatal( zoneId < mZoneStates.size(), "SceneCullingState::addCullingVolumeToZone - Zone ID out of range" );
   SceneZoneCullingState& zoneState = mZoneStates[ zoneId ];

   // [rene, 07-Apr-10] I previously used to attempt to merge things here and detect whether
   //    the visibility state of the zone has changed at all.  Since we allow polyhedra to be
   //    degenerate here and since polyhedra cannot be merged easily like frustums, I have opted
   //    to remove this for now.  I'm also convinced that with the current traversal system it
   //    adds little benefit.

   // Link the volume to the zone state.

   typedef SceneZoneCullingState::CullingVolumeLink LinkType;

   LinkType* link = reinterpret_cast< LinkType* >( allocateData( sizeof( LinkType ) ) );

   link->mVolume = volume;
   link->mNext = zoneState.mCullingVolumes;
   zoneState.mCullingVolumes = link;

   if( volume.isOccluder() )
      zoneState.mHaveOccluders = true;
   else
      zoneState.mHaveIncluders = true;

   // Mark sorting state as dirty.

   zoneState.mHaveSortedVolumes = false;

   // Set the visibility flag for the zone.
   
   if( volume.isIncluder() )
      mZoneVisibilityFlags.set( zoneId );

   return true;
}

//-----------------------------------------------------------------------------

bool SceneCullingState::addCullingVolumeToZone( U32 zoneId, SceneCullingVolume::Type type, const AnyPolyhedron& polyhedron )
{
   // Allocate space on our chunker.

   const U32 numPlanes = polyhedron.getNumPlanes();
   PlaneF* planes = allocateData< PlaneF >( numPlanes );

   // Copy the planes over.

   dMemcpy( planes, polyhedron.getPlanes(), numPlanes * sizeof( planes[ 0 ] ) );
   
   // Create a culling volume.

   SceneCullingVolume volume(
      type,
      PlaneSetF( planes, numPlanes )
   );

   // And add it.

   return addCullingVolumeToZone( zoneId, volume );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::createCullingVolume( const Point3F* vertices, U32 numVertices, SceneCullingVolume::Type type, SceneCullingVolume& outVolume )
{
   const Point3F& viewPos = getCameraState().getViewPosition();
   const Point3F& viewDir = getCameraState().getViewDirection();
   const bool isOrtho = getCullingFrustum().isOrtho();

   //TODO: check if we need to handle penetration of the near plane for occluders specially

   // Allocate space for the clipping planes we generate.  Assume the worst case
   // of every edge generating a plane and, for includers, all edges meeting at
   // steep angles so we need to insert extra planes (the latter is not possible,
   // of course, but it makes things less complicated here).  For occluders, add
   // an extra plane for the near cap.

   const U32 maxPlanes = ( type == SceneCullingVolume::Occluder ? numVertices + 1 : numVertices * 2 );
   PlaneF* planes = allocateData< PlaneF >( maxPlanes );

   // Keep track of the world-space bounds of the polygon.  We use this later
   // to derive some metrics.

   Box3F wsPolyBounds;

   wsPolyBounds.minExtents = Point3F( TypeTraits< F32 >::MAX, TypeTraits< F32 >::MAX, TypeTraits< F32 >::MAX );
   wsPolyBounds.maxExtents = Point3F( TypeTraits< F32 >::MIN, TypeTraits< F32 >::MIN, TypeTraits< F32 >::MIN );

   // For occluders, also keep track of the nearest, and two farthest silhouette points.  We use
   // this later to construct a near capping plane.
   F32 minVertexDistanceSquared = TypeTraits< F32 >::MAX;
   U32 leastDistantVert = 0;

   F32 maxVertexDistancesSquared[ 2 ] = { TypeTraits< F32 >::MIN, TypeTraits< F32 >::MIN };
   U32 mostDistantVertices[ 2 ] = { 0, 0 };

   // Generate the extrusion volume.  For orthographic projections, extrude
   // parallel to the view direction whereas for parallel projections, extrude
   // from the viewpoint.

   U32 numPlanes = 0;
   U32 lastVertex = numVertices - 1;
   bool invert = false;

   for( U32 i = 0; i < numVertices; lastVertex = i, ++ i )
   {
      AssertFatal( numPlanes < maxPlanes, "SceneCullingState::createCullingVolume - Did not allocate enough planes!" );

      const Point3F& v1 = vertices[ i ];
      const Point3F& v2 = vertices[ lastVertex ];

      // Keep track of bounds.

      wsPolyBounds.minExtents.setMin( v1 );
      wsPolyBounds.maxExtents.setMax( v1 );

      // Skip the edge if it's length is really short.

      const Point3F edgeVector = v2 - v1;
      const F32 edgeVectorLenSquared = edgeVector.lenSquared();
      if( edgeVectorLenSquared < 0.025f )
         continue;

      //TODO: might need to do additional checks here for non-planar polygons used by occluders
      //TODO: test for colinearity of edge vector with view vector (occluders only)

      // Create a plane for the edge.

      if( isOrtho )
      {
         // Compute a plane through the two edge vertices and one
         // of the vertices extended along the view direction.

         if( !invert )
            planes[ numPlanes ] = PlaneF( v1, v1 + viewDir, v2 );
         else
            planes[ numPlanes ] = PlaneF( v2, v1 + viewDir, v1 );
      }
      else
      {
         // Compute a plane going through the viewpoint and the two
         // edge vertices.

         if( !invert )
            planes[ numPlanes ] = PlaneF( v1, viewPos, v2 );
         else
            planes[ numPlanes ] = PlaneF( v2, viewPos, v1 );
      }

      numPlanes ++;

      // If this is the first plane that we have created, find out whether
      // the vertex ordering is giving us the plane orientations that we want
      // (facing inside).  If not, invert vertex order from now on.

      if( numPlanes == 1 )
      {
         Point3F center( 0, 0, 0 );
         for( U32 n = 0; n < numVertices; ++ n )
            center += vertices[n];
         center /= numVertices;

         if( planes[numPlanes - 1].whichSide( center ) == PlaneF::Back )
         {
            invert = true;
            planes[ numPlanes - 1 ].invert();
         }
      }

      // For occluders, keep tabs of the nearest, and two farthest vertices.

      if( type == SceneCullingVolume::Occluder )
      {
         const F32 distSquared = ( v1 - viewPos ).lenSquared();
         if( distSquared < minVertexDistanceSquared )
         {
            minVertexDistanceSquared = distSquared;
            leastDistantVert = i;
         }
         if( distSquared > maxVertexDistancesSquared[ 0 ] )
         {
            // Move 0 to 1.
            maxVertexDistancesSquared[ 1 ] = maxVertexDistancesSquared[ 0 ];
            mostDistantVertices[ 1 ] = mostDistantVertices[ 0 ];

            // Replace 0.
            maxVertexDistancesSquared[ 0 ] = distSquared;
            mostDistantVertices[ 0 ] = i;
         }
         else if( distSquared > maxVertexDistancesSquared[ 1 ] )
         {
            // Replace 1.
            maxVertexDistancesSquared[ 1 ] = distSquared;
            mostDistantVertices[ 1 ] = i;
         }
      }
   }

   // If the extrusion produced no useful result, abort.

   if( numPlanes < 3 )
      return false;

   // For includers, test the angle of the edges at the current vertex.
   // If too steep, add an extra plane to improve culling efficiency.

   if( false )//type == SceneCullingVolume::Includer )
   {
      const U32 numOriginalPlanes = numPlanes;
      U32 lastPlaneIndex = numPlanes - 1;

      for( U32 i = 0; i < numOriginalPlanes; lastPlaneIndex = i, ++ i )
      {
         const PlaneF& currentPlane = planes[ i ];
         const PlaneF& lastPlane = planes[ lastPlaneIndex ];

         // Compute the cosine of the angle between the two plane normals.

         const F32 cosAngle = mFabs( mDot( currentPlane, lastPlane ) );

         // The planes meet at increasingly steep angles the more they point
         // in opposite directions, i.e the closer the angle of their normals
         // is to 180 degrees.  Skip any two planes that don't get near that.

         if( cosAngle > 0.1f )
            continue;

         Point3F newNormal = currentPlane + lastPlane;//addNormals - mDot( addNormals, crossNormals ) * crossNormals;

         //

         planes[ numPlanes ] = PlaneF( currentPlane.getPosition(), newNormal );
         numPlanes ++;
      }
   }

   // Compute the metrics of the culling volume in relation to the view frustum.
   //
   // For this, we are short-circuiting things slightly.  The correct way (other than doing
   // full screen projections) would be to transform all the polygon points into camera
   // space, lay an AABB around those points, and then find the X and Z extents on the near plane.
   //
   // However, while not as accurate, a faster way is to just project the axial vectors
   // of the bounding box onto both the camera right and up vector.  This gives us a rough
   // estimate of the camera-space size of the polygon we're looking at.
   
   const MatrixF& cameraTransform = getCameraState().getViewWorldMatrix();
   const Point3F cameraRight = cameraTransform.getRightVector();
   const Point3F cameraUp = cameraTransform.getUpVector();

   const Point3F wsPolyBoundsExtents = wsPolyBounds.getExtents();
   
   F32 widthEstimate =
      getMax( mFabs( wsPolyBoundsExtents.x * cameraRight.x ),
         getMax( mFabs( wsPolyBoundsExtents.y * cameraRight.y ),
                 mFabs( wsPolyBoundsExtents.z * cameraRight.z ) ) );

   F32 heightEstimate =
      getMax( mFabs( wsPolyBoundsExtents.x * cameraUp.x ),
         getMax( mFabs( wsPolyBoundsExtents.y * cameraUp.y ),
                 mFabs( wsPolyBoundsExtents.z * cameraUp.z ) ) );

   // If the current camera is a perspective one, divide the two estimates
   // by the distance of the nearest bounding box vertex to the camera
   // to account for perspective distortion.

   if( !isOrtho )
   {
      const Point3F nearestVertex = wsPolyBounds.computeVertex(
         Box3F::getPointIndexFromOctant( - viewDir )
      );

      const F32 distance = ( nearestVertex - viewPos ).len();

      widthEstimate /= distance;
      heightEstimate /= distance;
   }

   // If we are creating an occluder, check to see if the estimates fit
   // our minimum requirements.

   if( type == SceneCullingVolume::Occluder )
   {
      const F32 widthEstimatePercentage = widthEstimate / getCullingFrustum().getWidth();
      const F32 heightEstimatePercentage = heightEstimate / getCullingFrustum().getHeight();

      if( widthEstimatePercentage < smOccluderMinWidthPercentage ||
          heightEstimatePercentage < smOccluderMinHeightPercentage )
         return false; // Reject.
   }

   // Use the area estimate as the volume's sort point.

   const F32 sortPoint = widthEstimate * heightEstimate;

   // Finally, if it's an occluder, compute a near cap.  The near cap prevents objects
   // in front of the occluder from testing positive.  The same could be achieved by
   // manually comparing distances before testing objects but since that would amount
   // to the same checks the plane/AABB tests do, it's easier to just add another plane.
   // Additionally, it gives the benefit of being able to create more precise culling
   // results by angling the plane.

   //NOTE: Could consider adding a near cap for includers too when generating a volume
   //  for the outdoor zone as that may prevent quite a bit of space from being included.
   //  However, given that this space will most likely just be filled with interior
   //  stuff anyway, it's probably not worth it.

   if( type == SceneCullingVolume::Occluder )
   {
      const U32 nearCapIndex = numPlanes;
      planes[ nearCapIndex ] = PlaneF(
         vertices[ mostDistantVertices[ 0 ] ],
         vertices[ mostDistantVertices[ 1 ] ],
         vertices[ leastDistantVert ] );

      // Invert the plane, if necessary.
      if( planes[ nearCapIndex ].whichSide( viewPos ) == PlaneF::Front )
         planes[ nearCapIndex ].invert();

      numPlanes ++;
   }

   // Create the volume from the planes.

   outVolume = SceneCullingVolume(
      type,
      PlaneSetF( planes, numPlanes )
   );
   outVolume.setSortPoint( sortPoint );

   // Done.

   return true;
}

//-----------------------------------------------------------------------------

namespace {
   struct ZoneArrayIterator
   {
      U32 mCurrent;
      U32 mNumZones;
      const U32* mZones;

      ZoneArrayIterator( const U32* zones, U32 numZones )
         : mCurrent( 0 ),
           mNumZones( numZones ),
           mZones( zones ) {}

      bool isValid() const
      {
         return ( mCurrent < mNumZones );
      }
      ZoneArrayIterator& operator ++()
      {
         mCurrent ++;
         return *this;
      }
      U32 operator *() const
      {
         return mZones[ mCurrent ];
      }
   };
}

template< typename T, typename Iter >
inline SceneZoneCullingState::CullingTestResult SceneCullingState::_testOccludersOnly( const T& bounds, Iter zoneIter ) const
{
   // Test the culling states of all zones that the object
   // is assigned to.

   for( ; zoneIter.isValid(); ++ zoneIter )
   {
      const SceneZoneCullingState& zoneState = getZoneState( *zoneIter );

      // Skip zone if there are no occluders.

      if( !zoneState.hasOccluders() )
         continue;

      // If the object's world bounds overlaps any of the volumes
      // for this zone, it's rendered.

      if( zoneState.testVolumes( bounds, true ) == SceneZoneCullingState::CullingTestPositiveByOcclusion )
         return SceneZoneCullingState::CullingTestPositiveByOcclusion;
   }

   return SceneZoneCullingState::CullingTestNegative;
}

template< typename T, typename Iter >
inline SceneZoneCullingState::CullingTestResult SceneCullingState::_test( const T& bounds, Iter zoneIter,
                                                                          const PlaneF& nearPlane, const PlaneF& farPlane ) const
{
   // Defer test of near and far plane until we've hit a zone
   // which actually has visible space.  This prevents us from
   // doing near/far tests on objects that were included in the
   // potential render list but aren't actually in any visible
   // zone.
   bool haveTestedNearAndFar = false;

   // Test the culling states of all zones that the object
   // is assigned to.

   for( ; zoneIter.isValid(); ++ zoneIter )
   {
      const SceneZoneCullingState& zoneState = getZoneState( *zoneIter );

      // Skip zone if there are no positive culling volumes.

      if( !zoneState.hasIncluders() )
         continue;

      // If we haven't tested the near and far plane yet, do so
      // now.

      if( !haveTestedNearAndFar )
      {
         // Test near plane.

         PlaneF::Side nearSide = nearPlane.whichSide( bounds );
         if( nearSide == PlaneF::Back )
            return SceneZoneCullingState::CullingTestNegative;

         // Test far plane.

         PlaneF::Side farSide = farPlane.whichSide( bounds );
         if( farSide == PlaneF::Back )
            return SceneZoneCullingState::CullingTestNegative;

         haveTestedNearAndFar = true;
      }

      // If the object's world bounds overlaps any of the volumes
      // for this zone, it's rendered.

      SceneZoneCullingState::CullingTestResult result = zoneState.testVolumes( bounds );

      if( result == SceneZoneCullingState::CullingTestPositiveByInclusion )
         return result;
      else if( result == SceneZoneCullingState::CullingTestPositiveByOcclusion )
         return result;
   }

   return SceneZoneCullingState::CullingTestNegative;
}

//-----------------------------------------------------------------------------

template< bool OCCLUDERS_ONLY, typename T >
inline SceneZoneCullingState::CullingTestResult SceneCullingState::_test( const T& bounds, const U32* zones, U32 numZones ) const
{
   // If zone culling is disabled, only test against
   // the root frustum.

   if( disableZoneCulling() )
   {
      if( !OCCLUDERS_ONLY && !getCullingFrustum().isCulled( bounds ) )
         return SceneZoneCullingState::CullingTestPositiveByInclusion;

      return SceneZoneCullingState::CullingTestNegative;
   }

   // Otherwise test each of the zones.

   if( OCCLUDERS_ONLY )
   {
      return _testOccludersOnly(
         bounds,
         ZoneArrayIterator( zones, numZones )
      );
   }
   else
   {
      const PlaneF* frustumPlanes = getCullingFrustum().getPlanes();

      return _test(
         bounds,
         ZoneArrayIterator( zones, numZones ),
         frustumPlanes[ Frustum::PlaneNear ],
         frustumPlanes[ Frustum::PlaneFar ]
      );
   }
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isCulled( const Box3F& aabb, const U32* zones, U32 numZones ) const
{
   SceneZoneCullingState::CullingTestResult result = _test< false >( aabb, zones, numZones );
   return ( result == SceneZoneCullingState::CullingTestNegative ||
            result == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isCulled( const OrientedBox3F& obb, const U32* zones, U32 numZones ) const
{
   SceneZoneCullingState::CullingTestResult result = _test< false >( obb, zones, numZones );
   return ( result == SceneZoneCullingState::CullingTestNegative ||
            result == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isCulled( const SphereF& sphere, const U32* zones, U32 numZones ) const
{
   SceneZoneCullingState::CullingTestResult result = _test< false >( sphere, zones, numZones );
   return ( result == SceneZoneCullingState::CullingTestNegative ||
            result == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isOccluded( SceneObject* object ) const
{
   if( disableZoneCulling() )
      return false;

   CullingTestResult result = _testOccludersOnly(
      object->getWorldBox(),
      SceneObject::ObjectZonesIterator( object )
   );

   return ( result == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isOccluded( const Box3F& aabb, const U32* zones, U32 numZones ) const
{
   return ( _test< true >( aabb, zones, numZones ) == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isOccluded( const OrientedBox3F& obb, const U32* zones, U32 numZones ) const
{
   return ( _test< true >( obb, zones, numZones ) == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isOccluded( const SphereF& sphere, const U32* zones, U32 numZones ) const
{
   return ( _test< true >( sphere, zones, numZones ) == SceneZoneCullingState::CullingTestPositiveByOcclusion );
}

//-----------------------------------------------------------------------------

U32 SceneCullingState::cullObjects( SceneObject** objects, U32 numObjects, U32 cullOptions ) const
{
   PROFILE_SCOPE( SceneCullingState_cullObjects );

   U32 numRemainingObjects = 0;

   // We test near and far planes separately in order to not do the tests
   // repeatedly, so fetch the planes now.
   const PlaneF& nearPlane = getCullingFrustum().getPlanes()[ Frustum::PlaneNear ];
   const PlaneF& farPlane = getCullingFrustum().getPlanes()[ Frustum::PlaneFar ];

   for( U32 i = 0; i < numObjects; ++ i )
   {
      SceneObject* object = objects[ i ];
      bool isCulled = true;

      // If we should respect editor overrides, test that now.

      if( !( cullOptions & CullEditorOverrides ) &&
          gEditingMission &&
          ( ( object->isCullingDisabledInEditor() && object->isRenderEnabled() ) || object->isSelected() ) )
      {
         isCulled = false;
      }

      // If the object is render-disabled, it gets culled.  The only
      // way around this is the editor override above.

      else if( !( cullOptions & DontCullRenderDisabled ) &&
               !object->isRenderEnabled() )
      {
         isCulled = true;
      }

      // Global bounds objects are never culled.  Note that this means
      // that if these objects are to respect zoning, they need to manually
      // trigger the respective culling checks for whatever they want to
      // batch.

      else if( object->isGlobalBounds() )
         isCulled = false;

      // If terrain occlusion checks are enabled, run them now.

      else if( !mDisableTerrainOcclusion &&
               object->getWorldBox().minExtents.x > -1e5 &&
               isOccludedByTerrain( object ) )
      {
         // Occluded by terrain.
         isCulled = true;
      }

      // If the object shouldn't be subjected to more fine-grained culling
      // or if zone culling is disabled, just test against the root frustum.

      else if( !( object->getTypeMask() & CULLING_INCLUDE_TYPEMASK ) ||
               ( object->getTypeMask() & CULLING_EXCLUDE_TYPEMASK ) ||
               disableZoneCulling() )
      {
         isCulled = getCullingFrustum().isCulled( object->getWorldBox() );
      }

      // Go through the zones that the object is assigned to and
      // test the object against the frustums of each of the zones.

      else
      {
         CullingTestResult result = _test(
            object->getWorldBox(),
            SceneObject::ObjectZonesIterator( object ),
            nearPlane,
            farPlane
         );

         isCulled = ( result == SceneZoneCullingState::CullingTestNegative ||
                      result == SceneZoneCullingState::CullingTestPositiveByOcclusion );
      }

      if( !isCulled )
         isCulled = isOccludedWithExtraPlanesCull( object->getWorldBox() );

      if( !isCulled )
         objects[ numRemainingObjects ++ ] = object;
   }

   return numRemainingObjects;
}

//-----------------------------------------------------------------------------

bool SceneCullingState::isOccludedByTerrain( SceneObject* object ) const
{
   PROFILE_SCOPE( SceneCullingState_isOccludedByTerrain );

   // Don't try to occlude globally bounded objects.
   if( object->isGlobalBounds() )
      return false;

   const Vector< SceneObject* >& terrains = getSceneManager()->getContainer()->getTerrains();
   const U32 numTerrains = terrains.size();

   for( U32 terrainIdx = 0; terrainIdx < numTerrains; ++ terrainIdx )
   {
      TerrainBlock* terrain = dynamic_cast< TerrainBlock* >( terrains[ terrainIdx ] );
      if( !terrain )
         continue;

      MatrixF terrWorldTransform = terrain->getWorldTransform();

      Point3F localCamPos = getCameraState().getViewPosition();
      terrWorldTransform.mulP(localCamPos);
      F32 height;
      terrain->getHeight( Point2F( localCamPos.x, localCamPos.y ), &height );
      bool aboveTerrain = ( height <= localCamPos.z );

      // Don't occlude if we're below the terrain.  This prevents problems when
      //  looking out from underground bases...
      if( !aboveTerrain )
         continue;

      const Box3F& oBox = object->getObjBox();
      F32 minSide = getMin(oBox.len_x(), oBox.len_y());
      if (minSide > 85.0f)
         continue;

      const Box3F& rBox = object->getWorldBox();
      Point3F ul(rBox.minExtents.x, rBox.minExtents.y, rBox.maxExtents.z);
      Point3F ur(rBox.minExtents.x, rBox.maxExtents.y, rBox.maxExtents.z);
      Point3F ll(rBox.maxExtents.x, rBox.minExtents.y, rBox.maxExtents.z);
      Point3F lr(rBox.maxExtents.x, rBox.maxExtents.y, rBox.maxExtents.z);

      terrWorldTransform.mulP(ul);
      terrWorldTransform.mulP(ur);
      terrWorldTransform.mulP(ll);
      terrWorldTransform.mulP(lr);

      Point3F xBaseL0_s = ul - localCamPos;
      Point3F xBaseL0_e = lr - localCamPos;
      Point3F xBaseL1_s = ur - localCamPos;
      Point3F xBaseL1_e = ll - localCamPos;

      static F32 checkPoints[3] = {0.75, 0.5, 0.25};
      RayInfo rinfo;
      for( U32 i = 0; i < 3; i ++ )
      {
         Point3F start = (xBaseL0_s * checkPoints[i]) + localCamPos;
         Point3F end   = (xBaseL0_e * checkPoints[i]) + localCamPos;

         if (terrain->castRay(start, end, &rinfo))
            continue;

         terrain->getHeight(Point2F(start.x, start.y), &height);
         if ((height <= start.z) == aboveTerrain)
            continue;

         start = (xBaseL1_s * checkPoints[i]) + localCamPos;
         end   = (xBaseL1_e * checkPoints[i]) + localCamPos;

         if (terrain->castRay(start, end, &rinfo))
            continue;

         Point3F test = (start + end) * 0.5;
         if (terrain->castRay(localCamPos, test, &rinfo) == false)
            continue;

         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

void SceneCullingState::debugRenderCullingVolumes() const
{
   const ColorI occluderColor( 255, 0, 0, 255 );
   const ColorI includerColor( 0, 255, 0, 255 );

   const PlaneF& nearPlane = getCullingFrustum().getPlanes()[ Frustum::PlaneNear ];
   const PlaneF& farPlane = getCullingFrustum().getPlanes()[ Frustum::PlaneFar ];

   DebugDrawer* drawer = DebugDrawer::get();
   const SceneZoneSpaceManager* zoneManager = mSceneManager->getZoneManager();

   bool haveDebugZone = false;
   const U32 numZones = mZoneStates.size();
   for( S32 zoneId = numZones - 1; zoneId >= 0; -- zoneId )
   {
      if( !zoneManager->isValidZoneId( zoneId ) )
         continue;

      const SceneZoneCullingState& zoneState = mZoneStates[ zoneId ];
      if( !zoneManager->getZoneOwner( zoneId )->isSelected() && ( zoneId != SceneZoneSpaceManager::RootZoneId || haveDebugZone ) )
         continue;

      haveDebugZone = true;

      for( SceneZoneCullingState::CullingVolumeIterator iter( zoneState );
           iter.isValid(); ++ iter )
      {
         // Temporarily add near and far plane to culling volume so that
         // no matter how it is defined, it has a chance of being properly
         // capped.

         const U32 numPlanes = iter->getPlanes().getNumPlanes();
         const PlaneF* planes = iter->getPlanes().getPlanes();

         TempAlloc< PlaneF > tempPlanes( numPlanes + 2 );

         tempPlanes[ 0 ] = nearPlane;
         tempPlanes[ 1 ] = farPlane;

         dMemcpy( &tempPlanes[ 2 ], planes, numPlanes * sizeof( PlaneF ) );

         // Build a polyhedron from the plane set.

         Polyhedron polyhedron;
         polyhedron.buildFromPlanes(
            PlaneSetF( tempPlanes, numPlanes + 2 )
         );

         // If the polyhedron has any renderable data,
         // hand it over to the debug drawer.

         if( polyhedron.getNumEdges() )
            drawer->drawPolyhedron( polyhedron, iter->isOccluder() ? occluderColor : includerColor );
      }
   }
}
