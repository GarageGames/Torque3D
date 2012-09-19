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
#include "scene/culling/sceneZoneCullingState.h"

#include "scene/culling/sceneCullingState.h"
#include "platform/profiler.h"


//-----------------------------------------------------------------------------

template< typename T >
inline SceneZoneCullingState::CullingTestResult SceneZoneCullingState::_testVolumes( T bounds, bool occludersOnly ) const
{
   // If we are testing for occlusion only and we don't have any
   // occlusion volumes, we can early out here.

   if( occludersOnly && !mHaveOccluders )
      return CullingTestNegative;

   // If we haven't sorted the volumes on this zone state yet,
   // do so now.

   if( !mHaveSortedVolumes )
      _sortVolumes();

   // Now go through the volumes in this zone and test them
   // against the bounds.

   for( CullingVolumeLink* link = mCullingVolumes; link != NULL; link = link->mNext )
   {
      const SceneCullingVolume& volume = link->mVolume;

      if( volume.isOccluder() )
      {
         if( volume.test( bounds ) )
            return CullingTestPositiveByOcclusion;
      }
      else
      {
         // If we are testing for occlusion only, we can early out as soon
         // as we have reached the first non-inverted volume.
         if( occludersOnly )
            return CullingTestNegative;

         if( volume.test( bounds ) )
            return CullingTestPositiveByInclusion;
      }
   }

   return CullingTestNegative;
}

//-----------------------------------------------------------------------------

SceneZoneCullingState::CullingTestResult SceneZoneCullingState::testVolumes( const Box3F& aabb, bool invertedOnly ) const
{
   PROFILE_SCOPE( SceneZoneCullingState_testVolumes );
   return _testVolumes( aabb, invertedOnly );
}

//-----------------------------------------------------------------------------

SceneZoneCullingState::CullingTestResult SceneZoneCullingState::testVolumes( const OrientedBox3F& obb, bool invertedOnly ) const
{
   PROFILE_SCOPE( SceneZoneCullingState_testVolumes_OBB );
   return _testVolumes( obb, invertedOnly );
}

//-----------------------------------------------------------------------------

SceneZoneCullingState::CullingTestResult SceneZoneCullingState::testVolumes( const SphereF& sphere, bool invertedOnly ) const
{
   PROFILE_SCOPE( SceneZoneCullingState_testVolumes_Sphere );
   return _testVolumes( sphere, invertedOnly );
}

//-----------------------------------------------------------------------------

void SceneZoneCullingState::_sortVolumes() const
{
   // First do a pass to gather all occlusion volumes.  These must be put on the
   // list in front of all inclusion volumes.  Otherwise, an inclusion volume
   // may test positive when in fact an occlusion volume would reject the object.

   CullingVolumeLink* occluderHead = NULL;
   CullingVolumeLink* occluderTail = NULL;

   if( mHaveOccluders )
   {
      U32 numOccluders = 0;

      for( CullingVolumeLink* current = mCullingVolumes, *prev = NULL; current != NULL; )
      {
         CullingVolumeLink* next = current->mNext;

         if( !current->mVolume.isOccluder() )
            prev = current;
         else
         {
            // Unlink from list.

            if( prev )
               prev->mNext = next;
            else
               mCullingVolumes = next;

            // Sort into list.

            _insertSorted( occluderHead, occluderTail, current );
            ++ numOccluders;
         }

         current = next;
      }

      // If we ended up with more inverted (occlusion) volumes than we want,
      // chop off any but the first N volumes.  Since we have sorted the volumes
      // by screen coverage, this will get rid of smallest occlusion volumes.

      if( numOccluders > SceneCullingState::smMaxOccludersPerZone )
      {
         CullingVolumeLink* last = occluderHead;
         for( U32 i = 0; i < ( SceneCullingState::smMaxOccludersPerZone - 1 ); ++ i )
            last = last->mNext;

         // Chop off rest.  The links are allocated on the chunker
         // and thus will simply disappear when the state gets deleted.

         last->mNext = NULL;
         occluderTail = last;
      }
   }

   // Now, do a second pass to sort all includer volumes by decreasing screen
   // real estate so that when testing against them, we test the larger volumes first
   // and the smaller ones later.

   CullingVolumeLink* includerHead = NULL;
   CullingVolumeLink* includerTail = NULL;

   while( mCullingVolumes )
   {
      CullingVolumeLink* current = mCullingVolumes;

      AssertFatal( !current->mVolume.isOccluder(),
         "SceneCullingState::ZoneState::_sortFrustums - Occluders must have been filtered out at this point" );

      // Unlink from list.

      mCullingVolumes = current->mNext;

      // Sort into list.

      _insertSorted( includerHead, includerTail, current );
   }

   // Merge the two lists.  Put inverted volumes first and
   // non-inverted volumes second.

   if( occluderHead != NULL )
   {
      mCullingVolumes = occluderHead;
      occluderTail->mNext = includerHead;
   }
   else
      mCullingVolumes = includerHead;

   // Done.

   mHaveSortedVolumes = true;
}

//-----------------------------------------------------------------------------

void SceneZoneCullingState::_insertSorted( CullingVolumeLink*& head, CullingVolumeLink*& tail, CullingVolumeLink* link )
{
   // If first element, just put it in the list
   // and return.

   if( !head )
   {
      head = link;
      tail = link;
      link->mNext = NULL;

      return;
   }

   // Otherwise, search for where to put it.

   F32 sortPoint = link->mVolume.getSortPoint();

   for( CullingVolumeLink* current = head, *prev = NULL; current != NULL; prev = current, current = current->mNext )
   {
      F32 currentSortPoint = current->mVolume.getSortPoint();
      if( currentSortPoint > sortPoint )
         continue;

      if( !prev )
         head = link;
      else
         prev->mNext = link;

      link->mNext = current;
      return;
   }

   // Smallest frustum in list.  Append to end.

   tail->mNext = link;
   link->mNext = NULL;

   tail = link;
}
