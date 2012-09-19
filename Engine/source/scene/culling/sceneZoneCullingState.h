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

#ifndef _SCENEZONECULLINGSTATE_H_
#define _SCENEZONECULLINGSTATE_H_

#ifndef _SCENECULLINGVOLUME_H_
#include "scene/culling/sceneCullingVolume.h"
#endif


/// Culling state for a zone.
///
/// Zone states keep track of the culling volumes that are generated during traversal
/// for a particular zone in a scene.
///
/// @note This class has no meaningful constructor; the memory for all zone states is
///   cleared en bloc.
class SceneZoneCullingState 
{
   public:

      friend class SceneCullingState; // mCullingVolumes

      /// Result of a culling test in a zone.
      enum CullingTestResult
      {
         /// An includer tested positive on the bounding volume.
         CullingTestPositiveByInclusion,

         /// An occluder tested positive on the bounding volume.
         CullingTestPositiveByOcclusion,

         /// None of the culling volumes included or excluded the bounding volume.
         CullingTestNegative
      };

      /// A culling volume linked to a zone.
      ///
      /// @note Memory for CullingVolumeLink instances is maintained by SceneCullingState.
      struct CullingVolumeLink
      {
         /// Culling volume.
         SceneCullingVolume mVolume;

         /// Next culling volume linked to the zone.
         CullingVolumeLink* mNext;

         CullingVolumeLink( const SceneCullingVolume& volume )
            : mVolume( volume ) {}
      };

      /// Iterator over the culling volumes assigned to a zone.
      struct CullingVolumeIterator
      {
            CullingVolumeIterator( const SceneZoneCullingState& state )
               : mCurrent( state.getCullingVolumes() ) {}

            bool isValid() const { return mCurrent != NULL; }
            const SceneCullingVolume& operator *() const
            {
               AssertFatal( isValid(), "SceneCullingState::ZoneState::CullingVolumeIterator::operator* - Invalid iterator" );
               return mCurrent->mVolume;
            }
            const SceneCullingVolume* operator ->() const
            {
               AssertFatal( isValid(), "SceneCullingState::ZoneState::CullingVolumeIterator::operator-> - Invalid iterator" );
               return &mCurrent->mVolume;
            }
            CullingVolumeIterator& operator ++()
            {
               AssertFatal( isValid(), "SceneCullingState::ZoneState::CullingVolumeIterator::operator++ - Invalid iterator" );
               mCurrent = mCurrent->mNext;
               return *this;
            }

         private:
            CullingVolumeLink* mCurrent;
      };

   protected:

      /// Whether tests can be short-circuited, i.e. the first culler that rejects or accepts
      /// will cause the test to terminate.  This is the case if there are no includers inside
      /// occluders, i.e. if occluders can be trusted to fully exclude any space they cover.
      bool mCanShortcuit;//RDTODO: implement this

      /// Link of culling volumes defining the visibility state of the zone.  Since there may be
      /// multiple portals leading into a zone or multiple occluders inside a zone, we may have multiple
      /// culling volumes.
      mutable CullingVolumeLink* mCullingVolumes;

      /// Whether culling volumes for this zone state have already been sorted.
      mutable bool mHaveSortedVolumes;

      /// Whether there are inclusion volumes on this state.
      bool mHaveIncluders;

      /// Whether there are occlusion volumes on this state.
      bool mHaveOccluders;

      /// Culling volume test abstracted over bounding volume type.
      template< typename T > CullingTestResult _testVolumes( T bounds, bool occludersOnly ) const;

      /// Sort the culling volumes such that the volumes with the highest probability
      /// of rejecting objects come first in the list.  Also, make sure that all
      /// occluders come before all includers so that occlusion is handled correctly.
      void _sortVolumes() const;

      /// Insert the volume in @a link at the proper position in the list represented
      /// by @a head and @a tail.
      static void _insertSorted( CullingVolumeLink*& head, CullingVolumeLink*& tail, CullingVolumeLink* link );

   public:

      /// Zone states are constructed by SceneCullingState.  This constructor should not
      /// be used otherwise.  It is public due to the use through Vector in SceneCullingState.
      SceneZoneCullingState() {}

      /// Return true if the zone is visible.  This is the case if any
      /// includers have been added to the zone's rendering state.
      bool isZoneVisible() const { return mHaveIncluders; }

      /// Return the list of culling volumes attached to the zone.
      CullingVolumeLink* getCullingVolumes() const { _sortVolumes(); return mCullingVolumes; }

      /// Test whether the culling volumes added to the zone test positive on the
      /// given AABB, i.e. whether they include or exclude the given AABB.
      CullingTestResult testVolumes( const Box3F& aabb, bool occludersOnly = false ) const;

      /// Test whether the culling volumes added to the zone test positive on the
      /// given OBB, i.e. whether they include or exclude the given OBB.
      ///
      /// @param obb An OBB described by 8 points.
      /// @param invertedOnly If true, only inverted cullers are tested.
      CullingTestResult testVolumes( const OrientedBox3F& obb, bool occludersOnly = false ) const;

      /// Test whether the culling volumes added to the zone test positive on the
      /// given sphere, i.e. whether they include or exclude the given sphere.
      CullingTestResult testVolumes( const SphereF& sphere, bool occludersOnly = false ) const;

      /// Return true if the zone has more than one culling volume assigned to it.
      bool hasMultipleVolumes() const { return ( mCullingVolumes && mCullingVolumes->mNext ); }

      /// Return true if the zone has inclusion volumes assigned to it.
      bool hasIncluders() const { return mHaveIncluders; }

      /// Return true if the zone has occlusion volumes assigned to it.
      bool hasOccluders() const { return mHaveOccluders; }
};

#endif // !_SCENEZONECULLINGSTATE_H_
