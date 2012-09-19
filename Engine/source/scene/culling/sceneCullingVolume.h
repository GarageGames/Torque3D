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

#ifndef _SCENECULLINGVOLUME_H_
#define _SCENECULLINGVOLUME_H_

#ifndef _MPLANESET_H_
#include "math/mPlaneSet.h"
#endif


/// A volume used to include or exclude space in a scene.
///
/// Culling volumes are represented as sets of clipping planes.
///
/// @note Culling is performed in world space so the plane data for culling volumes
///   must be in world space too.
class SceneCullingVolume
{
   public:

      /// Type of culling.
      enum Type
      {
         Includer,
         Occluder,
      };

   protected:

      /// What type of culling volume this is.
      Type mType;

      ///
      F32 mSortPoint;

      /// The set of clipping planes that defines the clipping volume for this culler.
      PlaneSetF mClippingPlanes;

      /// Test the given bounds against this culling volume.
      ///
      /// Note that we allow false positives here for includers.  This will only cause an
      /// occasional object to be classified as intersecting when in fact it is outside.
      /// This is still better though than requiring the expensive intersection tests for
      /// all intersecting objects.
      ///
      /// @return True if the culling volume accepts the given bounds.
      template< typename B > bool _testBounds( const B& bounds ) const
      {
         if( isOccluder() )
            return getPlanes().isContained( bounds );
         else
            return ( getPlanes().testPotentialIntersection( bounds ) != GeometryOutside );
      }

   public:

      /// Create an *uninitialized* culling volume.
      SceneCullingVolume() {}

      ///
      SceneCullingVolume( Type type, const PlaneSetF& planes )
         : mType( type ), mClippingPlanes( planes ), mSortPoint( 1.f ) {}

      /// Return the type of volume defined by this culling volume, i.e. whether it includes
      /// or excludes space.
      Type getType() const { return mType; }

      /// Return true if this is an inclusion volume.
      bool isIncluder() const { return ( getType() == Includer ); }

      /// Return true if this is an occlusion volume.
      bool isOccluder() const { return ( getType() == Occluder ); }

      /// Return the set of clipping planes that defines the culling volume.
      const PlaneSetF& getPlanes() const { return mClippingPlanes; }

      /// @name Sorting
      ///
      /// Before testing, culling volumes will be sorted by decreasing probability of causing
      /// test positives.  Thus, the sort point of a volume should be a rough metric of the amount
      /// of scene/screen space it covers.
      ///
      /// Note that sort points for occluders are independent of sort points for includers.
      /// @{

      /// Return the sort point value of the volume.  The larger the value, the more likely the
      /// volume is to cause positive test results with bounding volumes.
      F32 getSortPoint() const { return mSortPoint; }

      ///
      void setSortPoint( F32 value ) { mSortPoint = value; }

      /// @}

      /// @name Testing
      /// @{

      /// Return true if the volume accepts the given AABB.
      bool test( const Box3F& aabb ) const { return _testBounds( aabb ); }

      /// Return true if the volume accepts the given OBB.
      bool test( const OrientedBox3F& obb ) const { return _testBounds( obb ); }

      /// Return true if the volume accepts the given sphere.
      bool test( const SphereF& sphere ) const { return _testBounds( sphere ); }

      /// @}
};

#endif // !_SCENECULLINGVOLUME_H_
