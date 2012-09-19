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

#ifndef _MORIENTEDBOX_H_
#define _MORIENTEDBOX_H_

#ifndef _MBOXBASE_H_
#include "math/mBoxBase.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


class MatrixF;
class Box3F;


/// An oriented bounding box (OBB) described by a center point, three normalizes axis
/// vectors, and half-extents along each of the axes.
class OrientedBox3F : public BoxBase
{
   public:

      enum Axis
      {
         RightVector,
         ForwardVector,
         UpVector
      };

   protected:

      /// Center point.
      Point3F mCenter;

      /// Normalized axis vectors.
      Point3F mAxes[ 3 ];

      /// Box half-extents along each axis.
      Point3F mHalfExtents;

      /// Corner points.
      Point3F mPoints[ NUM_POINTS ];

      void _initPoints();

   public:

      OrientedBox3F() {}
      OrientedBox3F( const MatrixF& transform, const Point3F& extents ) { set( transform, extents ); }
      OrientedBox3F( const MatrixF& transform, const Box3F& aabb ) { set( transform, aabb ); }

      /// Return the center point of the bounding box.
      const Point3F& getCenter() const { return mCenter; }

      /// Return the normalized axis vector for the given world-space axis. 
      const Point3F& getAxis( U32 i ) const
      {
         AssertFatal( i < 3, "OrientedBox3F::getAxis - Index out of range" );
         return mAxes[ i ];
      }

      /// Return the half-extents along each axis.
      ///
      /// Since the OBBs are symmetrical across each axis, we store half-extents
      /// instead of full extents as usually half-extents are needed in the computations.
      const Point3F& getHalfExtents() const { return mHalfExtents; }

      /// Return true if the given point is contained in the OBB.
      bool isContained( const Point3F& point ) const;

      /// Return the corner points of the box.
      const Point3F* getPoints() const { return mPoints; }

      /// Return the array of corner points for the box.
      operator const Point3F*() const { return getPoints(); }

      /// Compute the OBB values from the given transform and extents.
      ///
      /// @param transform World->object space transform.
      /// @param extents Box extent on each axis.
      void set( const MatrixF& transform, const Point3F& extents );

      /// Compute the OBB from an AABB in the given transform space.
      ///
      /// @param transform Transform space for the AABB.
      /// @param aabb An axis-aligned bounding box in the given transform space.
      void set( const MatrixF& transform, const Box3F& aabb );
};

#endif // !_MORIENTEDBOX_H_
