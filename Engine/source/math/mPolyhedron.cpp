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
#include "math/mPolyhedron.h"

#include "platform/typetraits.h"


//-----------------------------------------------------------------------------

void PolyhedronVectorData::buildFromPlanes( const PlaneSetF& planes )
{
   const U32 numSourcePlanes = planes.getNumPlanes();

   // Go through the planes and create edges by
   // intersecting the various planes.

   for( U32 i = 0; i < numSourcePlanes; ++ i )
   {
      const PlaneF& currentPlane = planes.getPlanes()[ i ];

      bool haveEdges = false;
      for( U32 n = 0; n < numSourcePlanes; ++ n )
      {
         if( n == i )
            continue;

         const PlaneF& intersectPlane = planes.getPlanes()[ n ];

         Point3F start;
         Point3F dir;

         // Intersect the two planes.

         if( !currentPlane.intersect( intersectPlane, start, dir ) )
            continue;

         // Absolutely make sure our direction vector is normalized.

         dir.normalize();

         // Find the two vertices on the line that are still
         // inside the polyhedron by clipping against the other
         // planes in the set.

         F32 minDist = TypeTraits< F32 >::MAX;
         F32 maxDist = TypeTraits< F32 >::MIN;

         Point3F v1;
         Point3F v2;

         Point3F end = start + dir;

         for( U32 j = 0; j < numSourcePlanes; j ++ )
         {
            // Skip if current or intersect plane.
            if( j == n || j == i )
               continue;

            const PlaneF& clipPlane = planes.getPlanes()[ j ];

            // Compute the distance at which the plane intersects
            // the line.  Skip if parallel.

            F32 dist = clipPlane.intersect( start, end );
            if( mIsEqual( dist, PARALLEL_PLANE ) )
               continue;

            // See if the resulting vertex is even inside the planes.
            // Skip if not.

            Point3F vertex = start + dir * dist;
            bool isContained = true;
            for( U32 nplane = 0; nplane < numSourcePlanes; ++ nplane )
            {
               // Skip all planes that we used to construct this vertex.
               if( nplane == j || nplane == n || nplane == i )
                  continue;

               if( planes.getPlanes()[ nplane ].whichSide( vertex ) == PlaneF::Back )
               {
                  isContained = false;
                  break;
               }
            }
            if( !isContained )
               continue;

            // Keep track of min and max distance.

            if( mIsEqual( dist, minDist ) || mIsEqual( dist, maxDist ) )
               continue;
            else if( dist < minDist )
            {
               if( minDist != TypeTraits< F32 >::MAX && maxDist == TypeTraits< F32 >::MIN )
               {
                  maxDist = minDist;
                  v2 = v1;
               }

               minDist = dist;
               v1 = vertex;
            }
            else if( dist > maxDist )
            {
               maxDist = dist;
               v2 = vertex;
            }
         }

         // Skip plane pair if there's no properly formed edge.

         if( minDist == TypeTraits< F32 >::MAX || maxDist == TypeTraits< F32 >::MIN || mIsEqual( minDist, maxDist ) )
            continue;

         // See if vertex 1 already exists.

         S32 v1index = -1;
         bool v1Existed = false;
         for( U32 nvert = 0; nvert < pointList.size(); ++ nvert )
            if( pointList[ nvert ].equal( v1, 0.001f ) )
            {
               v1index = nvert;
               v1Existed = true;
               break;
            }

         // See if vertex 2 already exists.

         S32 v2index = -1;
         bool v2Existed = false;
         for( U32 nvert = 0; nvert < pointList.size(); ++ nvert )
            if( pointList[ nvert ].equal( v2, 0.001f ) )
            {
               v2index = nvert;
               v2Existed = true;
               break;
            }

         // Add vertex 1, if necessary.

         if( !v1Existed )
         {
            v1index = pointList.size();
            pointList.push_back( v1 );
         }

         // Add vertex 2, if necessary.

         if( !v2Existed )
         {
            v2index = pointList.size();
            pointList.push_back( v2 );
         }

         // If both v1 and v2 already existed in the point
         // set, this must be an edge that we are sharing so try
         // to find it.

         const U32 thisPlaneIndex = planeList.size();
         bool foundExistingEdge = false;

         if( v1Existed && v2Existed )
         {
            for( U32 nedge = 0; nedge < edgeList.size(); ++ nedge )
            {
               Edge& edge = edgeList[ nedge ];

               if( ( edge.vertex[ 0 ] == v1index && edge.vertex[ 1 ] == v2index ) ||
                   ( edge.vertex[ 0 ] == v2index && edge.vertex[ 1 ] == v1index ) )
               {
                  edge.face[ 1 ] = thisPlaneIndex;
                  foundExistingEdge = true;
                  break;
               }
            }
         }

         // Otherwise, add a new edge.

         if( !foundExistingEdge )
         {
            bool invert = false;

            // We need to make sure to maintain CW ordering on face[0],
            // so test to see if we need to go v1->v2 or v2->v1.

            Point3F normal = mCross( currentPlane, v2 - v1 );
            Point3F testPoint = v1 + normal;

            for( U32 nplane = 0; nplane < numSourcePlanes; ++ nplane )
            {
               if( nplane == i )
                  continue;

               if( planes.getPlanes()[ nplane ].whichSide( testPoint ) == PlaneF::Back )
               {
                  invert = true;
                  break;
               }
            }

            if( !invert )
            {
               edgeList.push_back(
                  Edge( thisPlaneIndex, 0, v1index, v2index )
               );
            }
            else
            {
               edgeList.push_back(
                  Edge( thisPlaneIndex, 0, v2index, v1index )
               );
            }
         }

         // This plane has edges.

         haveEdges = true;
      }

      // If this plane produced edges, add it.

      if( haveEdges )
         planeList.push_back( currentPlane );
   }
}
