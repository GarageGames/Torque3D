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

#ifndef _MINTERSECTOR_H_
#define _MINTERSECTOR_H_

#ifndef _MCONSTANTS_H_
#include "math/mConstants.h"
#endif

#ifndef _MBOX_H_
#include "math/mBox.h"
#endif

#ifndef _MORIENTEDBOX_H_
#include "math/mOrientedBox.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif

#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif

#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif

#ifndef _MPLANETRANSFORMER_H_
#include "math/mPlaneTransformer.h"
#endif

#ifndef _PROFILER_H_
#include "platform/profiler.h"
#endif


/// @file
/// Precise and fast geometric intersection testing.


/// Base class for intersector implementations.
template< typename Tester, typename Testee >
struct IntersectorBase
{
      typedef Tester TesterType;
      typedef Testee TesteeType;

   protected:

      TesterType mTester;

   public:

      IntersectorBase() {}
      IntersectorBase( const TesterType& tester )
         : mTester( tester ) {}
};


/// Convex polyhedron / AABB intersection.
///
/// This class implements the algorithm described in "Detecting Intersection of a Rectangular
/// Solid and a Convex Polyhedron", Graphics Gems IV, Chapter 1.7, by Ned Greene.
///
/// The polyhedron is preprocessed when an object of this class is constructed.
///
/// This class also assumes that the polyhedron is represented in object space and thus also
/// stores local copies of the polyhedron's planes transformed into world space.
///
/// The approach of the algorithm is simple.  It uses a maximum of three successive stages
/// to determine intersection.  Each stage can early out if it can draw a conclusive result
/// already.
///
/// 1. Simple tests of the polyhedron's bounding box against the input box.
/// 2. Standard test on plane set.
/// 3. Plane tests reduced to 2D and done for each of the orthographic side projections.
///
/// @note The intersector depends on planes facing inwards.
template< typename Polyhedron >
struct PolyhedronBoxIntersector : public IntersectorBase< Polyhedron, Box3F >
{
      typedef IntersectorBase< Polyhedron, Box3F > Parent;
      typedef Polyhedron PolyhedronType;

   protected:

      /// Bounds of the polyhedron.
      Box3F mBounds;

      /// World-space planes.
      Vector< PlaneF > mPlanes;

      /// Number of silhouette edges for each of the orthographic projections.
      Point3I mNumEdgeLines;

      /// Edge line equations.  X projection first, then Y, then Z.
      /// X of each equation is mapped to a, Y to b, and Z to c of the standard
      /// implicit form of line equations.
      Vector< Point3F > mEdgeLines;

      /// Run the preprocessing step on the current polyhedron.
      void _preprocess( const MatrixF& objToWorld, const Point3F& scale );

      /// Project the point orthogonally down the given axis such that
      /// the orientation of the resulting coordinate system remains
      /// right-handed.
      Point2F _project( U32 axis, const Point3F& p )
      {
         switch( axis )
         {
            case 0:  return Point2F( - p.y, p.z );
            case 1:  return Point2F( p.x, p.z );
            default: // silence compiler
            case 2:  return Point2F( p.x, - p.y );
         }
      }

   public:

      PolyhedronBoxIntersector() {}
      PolyhedronBoxIntersector( const PolyhedronType& polyhedron,
                                const MatrixF& objToWorld,
                                const Point3F& scale,
                                const Box3F& wsBounds )
         : Parent( polyhedron ), mBounds( wsBounds )
      {
         _preprocess( objToWorld, scale );
      }

      OverlapTestResult test( const Box3F& box ) const;
};

//-----------------------------------------------------------------------------

template< typename Polyhedron >
void PolyhedronBoxIntersector< Polyhedron >::_preprocess( const MatrixF& objToWorld, const Point3F& scale )
{
   PROFILE_SCOPE( PolyhedronBoxIntersector_preprocess );

   // Transform the planes.

   const U32 numPlanes = this->mTester.getNumPlanes();
   const typename Polyhedron::PlaneType* planes = this->mTester.getPlanes();
   
   PlaneTransformer transformer;
   transformer.set( objToWorld, scale );

   mPlanes.setSize( numPlanes );
   for( U32 i = 0; i < numPlanes; ++ i )
      transformer.transform( planes[ i ], mPlanes[ i ] );

   // Extract the silhouettes for each of the three
   // orthographic projections.

   const U32 numEdges = this->mTester.getNumEdges();
   const typename Polyhedron::EdgeType* edges = this->mTester.getEdges();
   const typename Polyhedron::PointType* points = this->mTester.getPoints();

   for( U32 i = 0; i < 3; ++ i )
   {
      U32 numEdgesThisProj = 0;

      // Gather edge-lines for this projection.

      for( U32 n = 0; n < numEdges; ++ n )
      {
         const typename Polyhedron::EdgeType& edge = edges[ n ];

         // Compute dot product with face normals.  With our projection
         // pointing straight down the current axis, this is reduced to
         // '1*normal[i]'.

         F32 dotFace[ 2 ];

         dotFace[ 0 ] = mPlanes[ edge.face[ 0 ] ][ i ];
         dotFace[ 1 ] = mPlanes[ edge.face[ 1 ] ][ i ];

         // Skip edge if not a silhouette edge in this view.

         if( mSign( dotFace[ 0 ] ) == mSign( dotFace[ 1 ] ) )
            continue;

         // Find out which face is the front facing one.  Since we expect normals
         // to be pointing inwards, this means a reversal of the normal back facing
         // test and we're looking for a normal facing the *same* way as our projection.

         const U32 frontFace = dotFace[ 0 ] > 0.f ? 0 : 1;
         if( dotFace[ frontFace ] <= 0.f )
            continue; // This face or other face is perpendicular to us.

         // Now we want to find the line equation for the edge.  For that, we first need
         // the normal.  The direction of the normal is important so that we identify
         // the half-spaces correctly.  We want it to be pointing to the inside of the
         // polyhedron.

         Point3F v1 = points[ edge.vertex[ 0 ] ];
         Point3F v2 = points[ edge.vertex[ 1 ] ];

         v1.convolve( scale );
         v2.convolve( scale );

         objToWorld.mulP( v1 );
         objToWorld.mulP( v2 );

         Point2F q = _project( i, v1 ); // First point on line.
         Point2F p = _project( i, v2 ); // Second point on line.

         if( frontFace != 0 )
            swap( p, q );

         Point2F normal( - ( p.y - q.y ), p.x - q.x );
         normal.normalize();

         // Now compute c.

         const F32 c = mDot( - q, normal );

         // Add the edge.
         
         mEdgeLines.push_back(
            Point3F( normal.x, normal.y, c )
         );
         
         numEdgesThisProj ++;
      }

      mNumEdgeLines[ i ] = numEdgesThisProj;
   }
}

//-----------------------------------------------------------------------------

template< typename Polyhedron >
OverlapTestResult PolyhedronBoxIntersector< Polyhedron >::test( const Box3F& box ) const
{
   PROFILE_SCOPE( PolyhedronBoxIntersector_test );

   // -- Bounding box tests. --

   // If the box does not intersect with the AABB of the polyhedron,
   // it must be outside.

   if( !mBounds.isOverlapped( box ) )
      return GeometryOutside;

   // If the polyhedron's bounding box is fully contained in the given box,
   // the box is intersecting.
   
   if( box.isContained( mBounds ) )
      return GeometryIntersecting;

   // -- Face-plane tests. --
   
   bool insideAll = true;

   // Test each of the planes to see if the bounding box lies
   // fully in the negative space of any one of them.

   const U32 numPlanes = mPlanes.size();
   for( U32 i = 0; i < numPlanes; ++ i )
   {
      const PlaneF& plane = mPlanes[ i ];

      PlaneF::Side boxSide = plane.whichSide( box );
      if( boxSide == PlaneF::Back )
         return GeometryOutside;

      insideAll &= ( boxSide == PlaneF::Front );
   }

   // If the box is on the positive space of all of the polyhedron's
   // planes, it's inside.

   if( insideAll )
      return GeometryInside;

   // -- Edge-line tests. --

   U32 edgeLineIndex = 0;
   for( U32 i = 0; i < 3; ++ i )
   {
      // Determine the mapping of 3D to 2D for this projection.

      U32 xIndex = 0;
      U32 yIndex = 0;

      switch( i )
      {
         case 0:  xIndex = 1; yIndex = 2; break;
         case 1:  xIndex = 0; yIndex = 2; break;
         case 2:  xIndex = 0; yIndex = 1; break;
      }

      // Go through the edge-lines for this projection and
      // test the p-vertex for each edge line.

      const U32 numEdgesForThisProj = mNumEdgeLines[ i ];
      for( U32 n = 0; n < numEdgesForThisProj; ++ n, edgeLineIndex ++ )
      {
         const Point3F& edgeLine = mEdgeLines[ edgeLineIndex ];

         // Determine the p-vertex for the current AABB/edge combo.
         // Need to account for the axis flipping we have applied to maintain
         // a right-handed coordinate system.

         Point2F pVertex;

         switch( i )
         {
            case 0:
               pVertex.x = - ( edgeLine.x < 0.f ? box.maxExtents[ xIndex ] : box.minExtents[ xIndex ] );
               pVertex.y = edgeLine.y > 0.f ? box.maxExtents[ yIndex ] : box.minExtents[ yIndex ];
               break;

            case 1:
               pVertex.x = edgeLine.x > 0.f ? box.maxExtents[ xIndex ] : box.minExtents[ xIndex ];
               pVertex.y = edgeLine.y > 0.f ? box.maxExtents[ yIndex ] : box.minExtents[ yIndex ];
               break;

            case 2:
               pVertex.x = edgeLine.x > 0.f ? box.maxExtents[ xIndex ] : box.minExtents[ xIndex ];
               pVertex.y = - ( edgeLine.y < 0.f ? box.maxExtents[ yIndex ] : box.minExtents[ yIndex ] );
               break;
         }

         // See if the p-vertex lies inside in the negative half-space of the
         // edge line.  If so, the AABB is not intersecting the polyhedron in
         // this projection so we can conclude our search here.

         const F32 d = edgeLine.x * pVertex.x + edgeLine.y * pVertex.y + edgeLine.z;
         if( d < 0.f )
            return GeometryOutside;
      }
   }

   // Done.  Determined to be intersecting.

   return GeometryIntersecting;
}


/// Geometric intersecting testing.
///
/// This class is meant to be used for testing multiple geometries against
/// a specific geometric object.  Unlike the various intersection test routines
/// in other classes, this class might precompute and store data that is going
/// to be used repeatedly in the tests.  As such, Intersector can be faster
/// in certain cases.
///
/// Also, intersectors are required to implement *exact* intersection tests, i.e.
/// it is not acceptable for an Intersector to produce false positives on any
/// of the OverlapTestResult values.
///
/// This class itself has no functionality.  It depends on specializations.
template< typename Tester, typename Testee >
struct Intersector : public IntersectorBase< Tester, Testee > {};

// Specializations.

template<>
struct Intersector< AnyPolyhedron, Box3F > : public PolyhedronBoxIntersector< AnyPolyhedron > {};

#endif // !_MINTERSECTOR_H_
