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

#ifndef _MPOLYHEDRON_IMPL_H_
#define _MPOLYHEDRON_IMPL_H_

#include "math/mPlaneTransformer.h"


//-----------------------------------------------------------------------------

template< typename Base >
Point3F PolyhedronImpl< Base >::getCenterPoint() const
{
   const U32 numPoints = this->getNumPoints();
   if( numPoints == 0 )
      return Point3F( 0.f, 0.f, 0.f );

   const typename Base::PointType* pointList = this->getPoints();
   Point3F center( pointList[ 0 ] );

   for( U32 i = 1; i < numPoints; ++ i )
      center += pointList[ i ];

   center /= ( F32 ) numPoints;
   return center;
}

//-----------------------------------------------------------------------------

template< typename Base >
void PolyhedronImpl< Base >::transform( const MatrixF& matrix, const Point3F& scale )
{
   // Transform points.

   const U32 numPoints = this->getNumPoints();
   typename Base::PointType* points = this->getPoints();

   for( U32 i = 0; i < numPoints; ++ i )
   {
      matrix.mulP( points[ i ] );
      points[ i ].convolve( scale );
   }

   // Transform planes.

   const U32 numPlanes = this->getNumPlanes();
   typename Base::PlaneType* planes = this->getPlanes();
   
   PlaneTransformer transformer;
   transformer.set( matrix, scale );

   for( U32 i = 0; i < numPlanes; ++ i )
   {
      const typename Base::PlaneType& plane = planes[ i ];

      PlaneF result;
      transformer.transform( plane, result );
      planes[ i ] = result;
   }
}

//-----------------------------------------------------------------------------

template< typename Base >
U32 PolyhedronImpl< Base >::constructIntersection( const PlaneF& plane, Point3F* outPoints, U32 maxOutPoints ) const
{
   // The assumption here is that the polyhedron is entirely composed
   // of convex polygons implicitly described by the edges, points, and planes.
   // So, any polygon can only have one of three relations to the given plane
   //
   // 1) None of its edges intersect with the plane, i.e. the polygon can be ignored.
   // 2) One of its edges lies on the plane.
   // 2) Two of its edges intersect with the plane.
   //
   // Conceptually, we need to find the first face with an intersecting edge, then find
   // another edge on the same face that is also intersecting, and then continue with the
   // face on the opposite side of the edge.

   const U32 numEdges = this->getNumEdges();
   const typename Base::EdgeType* edges = this->getEdges();
   const typename Base::PointType* points = this->getPoints();
   U32 numOutPoints = 0;

   #define ADD_POINT( p ) \
      if( numOutPoints >= maxOutPoints ) \
         return 0; \
      outPoints[ numOutPoints ++ ] = p;

   Point3F intersection;

   S32 firstEdge = -1;
   S32 firstFace = -1;

   U32 v1 = 0;
   U32 v2 = 0;

   PlaneF::Side v1Side = PlaneF::Front;
   PlaneF::Side v2Side = PlaneF::Front;

   // Find an edge to start with.  This is the first edge
   // in the polyhedron that intersects the plane.

   for( U32 i = 0; i < numEdges; ++ i )
   {
      const typename Base::EdgeType& edge = edges[ i ];

      v1 = edge.vertex[ 0 ];
      v2 = edge.vertex[ 1 ];

      const Point3F& p1 = points[ v1 ];
      const Point3F& p2 = points[ v2 ];

      v1Side = plane.whichSide( p1 );
      v2Side = plane.whichSide( p2 );

      if( v1Side == PlaneF::On || v2Side == PlaneF::On ||
          plane.clipSegment( p1, p2, intersection ) )
      {
         firstEdge = i;
         firstFace = edge.face[ 0 ];
         break;
      }
   }

   if( firstEdge == -1 )
      return 0;

   // Slice around the faces of the polyhedron until
   // we get back to the starting face.

   U32 currentEdge = firstEdge;
   U32 currentFace = firstFace;

   do 
   {
      // Handle the current edge.

      if( v1Side == PlaneF::On && v2Side == PlaneF::On )
      {
         // Both points of the edge lie on the plane.  Add v2
         // and then look for an edge that is also connected to v1
         // *and* is connected to our current face.  The other face
         // of that edge is the one we need to continue with.

         ADD_POINT( points[ v2 ] );

         for( U32 n = 0; n < numEdges; ++ n )
         {
            const typename Base::EdgeType& e = edges[ n ];

            // Skip current edge.
            if( n == currentEdge )
               continue;

            // Skip edges not belonging to current face.
            if( e.face[ 0 ] != currentFace && e.face[ 1 ] != currentFace )
               continue;

            // Skip edges not connected to the current point.
            if( e.vertex[ 0 ] != edges[ currentEdge ].vertex[ 0 ] &&
                e.vertex[ 1 ] != edges[ currentEdge ].vertex[ 0 ] )
               continue;

            // It's our edge.  Continue on with the face that
            // isn't our current one.

            if( e.face[ 0 ] == currentFace )
               currentFace = e.face[ 1 ];
            else
               currentFace = e.face[ 0 ];
            currentEdge = n;
            break;
         }
      }
      else if( v1Side == PlaneF::On || v2Side == PlaneF::On )
      {
         // One of the points of the current edge is on the plane.
         // Add that point.

         if( v1Side == PlaneF::On )
         {
            ADD_POINT( points[ v1 ] );
         }
         else
         {
            ADD_POINT( points[ v2 ] );
         }

         // Find edge to continue with.

         for( U32 n = 0; n < numEdges; ++ n )
         {
            const typename Base::EdgeType& e = edges[ n ];

            // Skip current edge.
            if( n == currentEdge )
               continue;

            // Skip edges not belonging to current face.
            if( e.face[ 0 ] != currentFace && e.face[ 1 ] != currentFace )
               continue;

            // Skip edges connected to point that is on the plane.
            if( v1Side == PlaneF::On )
            {
               if( e.vertex[ 0 ] == v1 || e.vertex[ 1 ] == v1 )
                  continue;
            }
            else
            {
               if( e.vertex[ 0 ] == v2 || e.vertex[ 1 ] == v2 )
                  continue;
            }

            // Skip edges not intersecting the plane.

            U32 v1new = e.vertex[ 0 ];
            U32 v2new = e.vertex[ 1 ];

            const Point3F& p1 = points[ v1new ];
            const Point3F& p2 = points[ v2new ];

            PlaneF::Side v1SideNew = plane.whichSide( p1 );
            PlaneF::Side v2SideNew = plane.whichSide( p2 );

            if( v1SideNew != PlaneF::On && v2SideNew != PlaneF::On && !plane.clipSegment( p1, p2, intersection ) )
               continue;

            // It's our edge.  Continue with the face on the
            // opposite side.

            if( e.face[ 0 ] == currentFace )
               currentFace = e.face[ 1 ];
            else
               currentFace = e.face[ 0 ];
            currentEdge = n;

            v1 = v1new;
            v2 = v2new;

            v1Side = v1SideNew;
            v2Side = v2SideNew;

            break;
         }

         // Already have computed all the data.
         continue;
      }

      // It's a clean intersecting somewhere along the edge.  Add it.

      else
      {
         ADD_POINT( intersection );
      }

      // Find edge to continue with.

      for( U32 n = 0; n < numEdges; ++ n )
      {
         const typename Base::EdgeType& e = edges[ n ];

         // Skip current edge.
         if( n == currentEdge )
            continue;

         // Skip edges not belonging to current face.
         if( e.face[ 0 ] != currentFace && e.face[ 1 ] != currentFace )
            continue;

         // Skip edges not intersecting the plane.

         v1 = e.vertex[ 0 ];
         v2 = e.vertex[ 1 ];

         const Point3F& p1 = points[ v1 ];
         const Point3F& p2 = points[ v2 ];

         PlaneF::Side v1SideNew = plane.whichSide( p1 );
         PlaneF::Side v2SideNew = plane.whichSide( p2 );

         if( v1SideNew != PlaneF::On && v2SideNew != PlaneF::On && !plane.clipSegment( p1, p2, intersection ) )
            continue;

         // It's our edge.  Make it the current one.

         if( e.face[ 0 ] == currentFace )
            currentFace = e.face[ 1 ];
         else
            currentFace = e.face[ 0 ];
         currentEdge = n;

         break;
      }
   }
   //TODO: I guess this is insufficient; edges with vertices on the plane may lead us to take different
   // paths depending on edge order
   while( currentFace != firstFace &&
          currentEdge != firstEdge );

   return numOutPoints;

   #undef ADD_POINT
}

//-----------------------------------------------------------------------------

template< typename Base >
template< typename IndexType >
U32 PolyhedronImpl< Base >::extractFace( U32 plane, IndexType* outIndices, U32 maxOutIndices ) const
{
   AssertFatal( plane < this->getNumPlanes(), "PolyhedronImpl::extractFace - Plane index out of range!" );

   // This method relies on the fact that vertices on the edges must be CW ordered
   // for face[0].  If that is not the case, it is still possible to infer the correct
   // ordering by finding one edge and a point not on the edge but still on
   // the polygon.  By constructing a plane through that edge (simple cross product) and
   // then seeing which side the point is on, we know which direction is the right one
   // for the polygon.  The implicit CW ordering spares us from having to do that, though.

   const U32 numEdges = this->getNumEdges();
   const Edge* edges = this->getEdges();

   // Find first edge that belongs to the plane.

   const Edge* firstEdge = 0;

   for( U32 i = 0; i < numEdges; ++ i )
   {
      const Edge& edge = edges[ i ];
      if( edge.face[ 0 ] == plane || edge.face[ 1 ] == plane )
      {
         firstEdge = &edge;
         break;
      }
   }

   // If we have found no edge, the polyhedron is degenerate,
   // so abort.

   if( !firstEdge )
      return 0;

   // Choose vertex that begins a CCW traversal for this plane.
   //
   // Note that we expect the planes to be facing inwards by default so we
   // go the opposite direction to yield a polygon facing the other way.

   U32 idx = 0;
   U32 currentVertex;
   const Edge* currentEdge = firstEdge;

   if( firstEdge->face[ 0 ] == plane )
      currentVertex = firstEdge->vertex[ 0 ];
   else
      currentVertex = firstEdge->vertex[ 1 ];

   // Now spider along the edges until we have gathered all indices
   // for the plane in the right order.
   //
   // For larger edge sets, it would be more efficient to first extract
   // all edges for the plane and then loop only over this subset to
   // spider along the indices.  However, we tend to have small sets
   // so it should be sufficiently fast to just loop over the original
   // set.

   U32 indexItr = 0;

   do 
   {
      // Add the vertex for the current edge.

      if( idx >= maxOutIndices )
         return 0;

      ++indexItr;

      if (indexItr >= 3)
      {
         outIndices[idx++] = firstEdge->vertex[0];
         indexItr = 0;
      }

      outIndices[idx++] = currentVertex;

      // Look for next edge.

      for( U32 i = 0; i < numEdges; ++ i )
      {
         const Edge& edge = edges[ i ];

         // Skip if we hit the edge that we are looking to continue from.

         if( &edge == currentEdge )
            continue;

         // Skip edge if it doesn't belong to the current plane.

         if( edge.face[ 0 ] != plane && edge.face[ 1 ] != plane )
            continue;

         // If edge connects to vertex we are looking for, make it
         // the current edge and push its other vertex.

         if( edge.vertex[ 0 ] == currentVertex )
            currentVertex = edge.vertex[ 1 ];
         else if( edge.vertex[ 1 ] == currentVertex )
            currentVertex = edge.vertex[ 0 ];
         else
            continue; // Skip edge.

         currentEdge = &edge;
         break;
      }
   }
   while( currentEdge != firstEdge );

   // Done.

   return idx;
}

//-----------------------------------------------------------------------------

template< typename Polyhedron >
void PolyhedronData::buildBoxData( Polyhedron& poly, const MatrixF& mat, const Box3F& box, bool invertNormals )
{
   AssertFatal( poly.getNumPoints() == 8, "PolyhedronData::buildBox - Incorrect point count!" );
   AssertFatal( poly.getNumEdges() == 12, "PolyhedronData::buildBox - Incorrect edge count!" );
   AssertFatal( poly.getNumPlanes() == 6, "PolyhedronData::buildBox - Incorrect plane count!" );

   // Box is assumed to be axis aligned in the source space.
   // Transform into geometry space.

   Point3F xvec = mat.getRightVector() * box.len_x();
   Point3F yvec = mat.getForwardVector() * box.len_y();
   Point3F zvec = mat.getUpVector() * box.len_z();

   Point3F min;
   mat.mulP( box.minExtents, &min );

   // Corner points.

   typename Polyhedron::PointListType& pointList = poly.pointList;

   pointList[ 0 ] = min; // near left bottom
   pointList[ 1 ] = min + yvec; // far left bottom
   pointList[ 2 ] = min + xvec + yvec; // far right bottom
   pointList[ 3 ] = min + xvec; // near right bottom
   pointList[ 4 ] = pointList[ 0 ] + zvec; // near left top
   pointList[ 5 ] = pointList[ 1 ] + zvec; // far left top
   pointList[ 6 ] = pointList[ 2 ] + zvec; // far right top
   pointList[ 7 ] = pointList[ 3 ] + zvec; // near right top

   // Side planes.

   typename Polyhedron::PlaneListType& planeList = poly.planeList;

   const F32 pos = invertNormals ? -1.f : 1.f;
   const F32 neg = - pos;

   planeList[ 0 ].set( pointList[ 0 ], xvec * pos ); // left
   planeList[ 1 ].set( pointList[ 2 ], yvec * neg ); // far
   planeList[ 2 ].set( pointList[ 2 ], xvec * neg ); // right
   planeList[ 3 ].set( pointList[ 0 ], yvec * pos ); // front
   planeList[ 4 ].set( pointList[ 0 ], zvec * pos ); // bottom
   planeList[ 5 ].set( pointList[ 4 ], zvec * neg ); // top

   // The edges are constructed so that the vertices
   // are oriented clockwise for face[0].

   typename Polyhedron::EdgeType* edge = &poly.edgeList[ 0 ];

   for( U32 i = 0; i < 4; ++ i )
   {
      S32 n = ( i == 3 ) ? 0: i + 1;
      S32 p = ( i == 0 ) ? 3: i - 1;

      edge->vertex[ 0 ] = !invertNormals ? n : i;
      edge->vertex[ 1 ] = !invertNormals ? i : n;
      edge->face[ 0 ] = i;
      edge->face[ 1 ] = 4;
      edge ++;

      edge->vertex[ 0 ] = !invertNormals ? 4 + n : 4 + i;
      edge->vertex[ 1 ] = !invertNormals ? 4 + i : 4 + n;
      edge->face[ 0 ] = 5;
      edge->face[ 1 ] = i;
      edge ++;

      edge->vertex[ 0 ] = !invertNormals ? 4 + i : i;
      edge->vertex[ 1 ] = !invertNormals ? i : 4 + i;
      edge->face[ 0 ] = p;
      edge->face[ 1 ] = i;
      edge ++;
   }
}

#endif // _MPOLYHEDRON_IMPL_H_
