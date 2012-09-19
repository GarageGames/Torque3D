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

#include "math/mMath.h"
#include "core/color.h"
#include "console/console.h"
#include "collision/optimizedPolyList.h"
#include "materials/baseMatInstance.h"
#include "materials/materialDefinition.h"

//----------------------------------------------------------------------------

OptimizedPolyList::OptimizedPolyList()
{
   VECTOR_SET_ASSOCIATION(mPoints);
   VECTOR_SET_ASSOCIATION(mNormals);
   VECTOR_SET_ASSOCIATION(mUV0s);
   VECTOR_SET_ASSOCIATION(mUV1s);
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mIndexList);
   VECTOR_SET_ASSOCIATION(mPlaneList);
   VECTOR_SET_ASSOCIATION(mPolyList);

   mIndexList.reserve(100);

   mCurrObject       = NULL;
   mBaseMatrix       = MatrixF::Identity;
   mMatrix           = MatrixF::Identity;
   mTransformMatrix  = MatrixF::Identity;
   mScale.set(1.0f, 1.0f, 1.0f);

   mPlaneTransformer.setIdentity();

   mInterestNormalRegistered = false;
}

OptimizedPolyList::~OptimizedPolyList()
{
   mPoints.clear();
   mNormals.clear();
   mUV0s.clear();
   mUV1s.clear();
   mVertexList.clear();
   mIndexList.clear();
   mPlaneList.clear();
   mPolyList.clear();
}


//----------------------------------------------------------------------------
void OptimizedPolyList::clear()
{
   mPoints.clear();
   mNormals.clear();
   mUV0s.clear();
   mUV1s.clear();
   mVertexList.clear();
   mIndexList.clear();
   mPlaneList.clear();
   mPolyList.clear();
}

//----------------------------------------------------------------------------
U32 OptimizedPolyList::insertPoint(const Point3F& point)
{
   S32 retIdx = -1;

   // Apply the transform
   Point3F transPoint = point;
   transPoint *= mScale;
   mMatrix.mulP(transPoint);

   for (U32 i = 0; i < mPoints.size(); i++)
   {
      if (mPoints[i].equal(transPoint))
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mPoints.size();
      mPoints.push_back(transPoint);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertNormal(const Point3F& normal)
{
   S32 retIdx = -1;

   // Apply the transform
   Point3F transNormal;
   mMatrix.mulV( normal, &transNormal );

   for (U32 i = 0; i < mNormals.size(); i++)
   {
      if (mNormals[i].equal(transNormal))
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mNormals.size();
      mNormals.push_back(transNormal);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertUV0(const Point2F& uv)
{
   S32 retIdx = -1;

   for (U32 i = 0; i < mUV0s.size(); i++)
   {
      if (mUV0s[i].equal(uv))
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mUV0s.size();
      mUV0s.push_back(uv);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertUV1(const Point2F& uv)
{
   S32 retIdx = -1;

   for (U32 i = 0; i < mUV1s.size(); i++)
   {
      if (mUV1s[i].equal(uv))
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mUV1s.size();
      mUV1s.push_back(uv);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertPlane(const PlaneF& plane)
{
   S32 retIdx = -1;

   // Apply the transform
   PlaneF transPlane;
   mPlaneTransformer.transform(plane, transPlane);

   for (U32 i = 0; i < mPlaneList.size(); i++)
   {
      if (mPlaneList[i].equal(transPlane) &&
          mFabs( mPlaneList[i].d - transPlane.d ) < POINT_EPSILON)
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mPlaneList.size();
      mPlaneList.push_back(transPlane);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertMaterial(BaseMatInstance* baseMat)
{
   S32 retIdx = -1;

   if ( !baseMat )
      return retIdx;

   Material* mat = dynamic_cast<Material*>(baseMat->getMaterial());

   for (U32 i = 0; i < mMaterialList.size(); i++)
   {
      Material* testMat = dynamic_cast<Material*>(mMaterialList[i]->getMaterial());

      if (mat && testMat)
      {
         if (testMat == mat)
         {
            retIdx = i;
            break;
         }
      }
      else if (mMaterialList[i] == baseMat)
      {
         retIdx = i;
         break;
      }
   }

   if (retIdx == -1)
   {
      retIdx = mMaterialList.size();
      mMaterialList.push_back(baseMat);
   }

   return (U32)retIdx;
}

U32 OptimizedPolyList::insertVertex(const Point3F& point, const Point3F& normal,
                                    const Point2F& uv0, const Point2F& uv1)
{
   VertIndex vert;

   vert.vertIdx   = insertPoint(point);
   vert.normalIdx = insertNormal(normal);
   vert.uv0Idx    = insertUV0(uv0);
   vert.uv1Idx    = insertUV1(uv1);

   return mVertexList.push_back_unique(vert);
}

U32 OptimizedPolyList::addPoint(const Point3F& p)
{
   return insertVertex(p);
}

U32 OptimizedPolyList::addPlane(const PlaneF& plane)
{
   return insertPlane(plane);
}


//----------------------------------------------------------------------------

void OptimizedPolyList::begin(BaseMatInstance* material, U32 surfaceKey)
{
   mPolyList.increment();
   Poly& poly = mPolyList.last();
   poly.material = insertMaterial(material);
   poly.vertexStart = mIndexList.size();
   poly.surfaceKey = surfaceKey;
   poly.type = TriangleFan;
   poly.object = mCurrObject;
}

void OptimizedPolyList::begin(BaseMatInstance* material, U32 surfaceKey, PolyType type)
{
   begin(material, surfaceKey);

   // Set the type
   mPolyList.last().type = type;
}


//----------------------------------------------------------------------------

void OptimizedPolyList::plane(U32 v1, U32 v2, U32 v3)
{
   /*
   AssertFatal(v1 < mPoints.size() && v2 < mPoints.size() && v3 < mPoints.size(),
      "OptimizedPolyList::plane(): Vertex indices are larger than vertex list size");

   mPolyList.last().plane = addPlane(PlaneF(mPoints[v1], mPoints[v2], mPoints[v3]));
   */

   mPolyList.last().plane = addPlane( PlaneF( mPoints[mVertexList[v1].vertIdx], mPoints[mVertexList[v2].vertIdx], mPoints[mVertexList[v3].vertIdx] ) );
}

void OptimizedPolyList::plane(const PlaneF& p)
{
   mPolyList.last().plane = addPlane(p);
}

void OptimizedPolyList::plane(const U32 index)
{
   AssertFatal(index < mPlaneList.size(), "Out of bounds index!");
   mPolyList.last().plane = index;
}

const PlaneF& OptimizedPolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPlaneList.size(), "Out of bounds index!");
   return mPlaneList[index];
}


//----------------------------------------------------------------------------

void OptimizedPolyList::vertex(U32 vi)
{
   mIndexList.push_back(vi);
}

void OptimizedPolyList::vertex(const Point3F& p)
{
   mIndexList.push_back(addPoint(p));
}

void OptimizedPolyList::vertex(const Point3F& p,
                               const Point3F& normal,
                               const Point2F& uv0,
                               const Point2F& uv1)
{
   mIndexList.push_back(insertVertex(p, normal, uv0, uv1));
}


//----------------------------------------------------------------------------

bool OptimizedPolyList::isEmpty() const
{
   return !mPolyList.size();
}

void OptimizedPolyList::end()
{
   Poly& poly = mPolyList.last();
   poly.vertexCount = mIndexList.size() - poly.vertexStart;
}

//----------------------------------------------------------------------------

Polyhedron OptimizedPolyList::toPolyhedron() const
{
   Polyhedron polyhedron;

   // Add the points, but filter out duplicates.

   Vector< S32 > pointRemap;
   pointRemap.setSize( mPoints.size() );
   pointRemap.fill( -1 );

   const U32 numPoints = mPoints.size();

   for( U32 i = 0; i < numPoints; ++ i )
   {
      bool isDuplicate = false;
      for( U32 npoint = 0; npoint < polyhedron.pointList.size(); ++ npoint )
      {
         if( npoint == i )
            continue;

         if( !polyhedron.pointList[ npoint ].equal( mPoints[ i ] ) )
            continue;

         pointRemap[ i ] = npoint;
         isDuplicate = true;
      }

      if( !isDuplicate )
      {
         pointRemap[ i ] = polyhedron.pointList.size();
         polyhedron.pointList.push_back( mPoints[ i ] );
      }
   }

   // Go through the polys and add all their edges and planes.
   // We will consolidate edges in a second pass.

   const U32 numPolys = mPolyList.size();
   for( U32 i = 0; i < numPolys; ++ i )
   {
      const Poly& poly = mPolyList[ i ];

      // Add the plane.

      const U32 polyIndex = polyhedron.planeList.size();
      polyhedron.planeList.push_back( mPlaneList[ poly.plane ] );

      // Account for polyhedrons expecting planes to
      // face inwards.

      polyhedron.planeList.last().invert();

      // Gather remapped indices according to the
      // current polygon type.

      Vector< U32 > indexList;
      switch( poly.type )
      {
         case TriangleFan:
            AssertFatal( false, "TriangleFan conversion not implemented" );
         case TriangleStrip:
            AssertFatal( false, "TriangleStrip conversion not implemented" );
         case TriangleList:
            {
               Vector< Polyhedron::Edge > tempEdges;

               // Loop over the triangles and gather all unshared edges
               // in tempEdges.  These are the exterior edges of the polygon.

               for( U32 n = poly.vertexStart; n < poly.vertexStart + poly.vertexCount; n += 3 )
               {
                  U32 indices[ 3 ];

                  // Get the remapped indices of the three vertices.

                  indices[ 0 ] = pointRemap[ mVertexList[ mIndexList[ n + 0 ] ].vertIdx ];
                  indices[ 1 ] = pointRemap[ mVertexList[ mIndexList[ n + 1 ] ].vertIdx ];
                  indices[ 2 ] = pointRemap[ mVertexList[ mIndexList[ n + 2 ] ].vertIdx ];

                  // Loop over the three edges.

                  for( U32 d = 0; d < 3; ++ d )
                  {
                     U32 index1 = indices[ d ];
                     U32 index2 = indices[ ( d + 1 ) % 3 ];

                     // See if this edge is already in the list.  If so,
                     // it's a shared edge and thus an interior one.  Remove
                     // it.

                     bool isShared = false;
                     for( U32 nedge = 0; nedge < tempEdges.size(); ++ nedge )
                     {
                        Polyhedron::Edge& edge = tempEdges[ nedge ];
                        if( ( edge.vertex[ 0 ] == index1 && edge.vertex[ 1 ] == index2 ) ||
                            ( edge.vertex[ 0 ] == index2 && edge.vertex[ 1 ] == index1 ) )
                        {
                           tempEdges.erase( nedge );

                           isShared = true;
                           break;
                        }
                     }

                     // If it wasn't in the list, add a new edge.

                     if( !isShared )
                        tempEdges.push_back(
                           Polyhedron::Edge( -1, -1, index1, index2 )
                        );
                  }
               }

               // Walk the edges and gather consecutive indices.

               U32 currentEdge = 0;
               for( U32 n = 0; n < tempEdges.size(); ++ n )
               {
                  // Add first vertex of edge.

                  indexList.push_back( tempEdges[ currentEdge ].vertex[ 0 ] );

                  // Find edge that begins at second vertex.

                  for( U32 nedge = 0; nedge < tempEdges.size(); ++ nedge )
                  {
                     if( nedge == currentEdge )
                        continue;

                     if( tempEdges[ nedge ].vertex[ 0 ] == tempEdges[ currentEdge ].vertex[ 1 ] )
                     {
                        currentEdge = nedge;
                        break;
                     }
                  }
               }
            }
      }

      // Create edges from the indices.  Indices are CCW ordered and
      // we want CW order, so step everything in reverse.

      U32 lastIndex = 0;
      for( S32 n = indexList.size() - 1; n >= 0; -- n )
      {
         polyhedron.edgeList.push_back(
            Polyhedron::Edge(
               polyIndex, 0, // face1 filled later
               indexList[ lastIndex ], indexList[ n ]
            )
         );

         lastIndex = n;
      }
   }

   // Finally, consolidate the edge list by merging all edges that
   // are shared by polygons.

   for( U32 i = 0; i < polyhedron.edgeList.size(); ++ i )
   {
      Polyhedron::Edge& edge = polyhedron.edgeList[ i ];

      // Find the corresponding duplicate edge, if any, and merge
      // it into our current edge.

      for( U32 n = i + 1; n < polyhedron.edgeList.size(); ++ n )
      {
         const Polyhedron::Edge& thisEdge = polyhedron.edgeList[ n ];

         if( ( thisEdge.vertex[ 0 ] == edge.vertex[ 1 ] &&
               thisEdge.vertex[ 1 ] == edge.vertex[ 0 ] ) ||
             ( thisEdge.vertex[ 0 ] == edge.vertex[ 0 ] &&
               thisEdge.vertex[ 1 ] == edge.vertex[ 1 ] ) )
         {
            edge.face[ 1 ] = thisEdge.face[ 0 ];
            polyhedron.edgeList.erase( n );
            break;
         }
      }
   }

   return polyhedron;
}
