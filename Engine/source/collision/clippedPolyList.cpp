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
#include "collision/clippedPolyList.h"

#include "math/mMath.h"
#include "console/console.h"
#include "platform/profiler.h"

#include "core/tAlgorithm.h"

bool ClippedPolyList::allowClipping = true;


//----------------------------------------------------------------------------

ClippedPolyList::ClippedPolyList()
 : mNormal( Point3F::Zero ),
   mNormalTolCosineRadians( 0.0f )
{
   VECTOR_SET_ASSOCIATION(mPolyList);
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mIndexList);
   VECTOR_SET_ASSOCIATION(mPolyPlaneList);
   VECTOR_SET_ASSOCIATION(mPlaneList);
   VECTOR_SET_ASSOCIATION(mNormalList);
   
   mIndexList.reserve(IndexListReserveSize);
}

ClippedPolyList::~ClippedPolyList()
{
}


//----------------------------------------------------------------------------

void ClippedPolyList::clear()
{
   // Only clears internal data
   mPolyList.clear();
   mVertexList.clear();
   mIndexList.clear();
   mPolyPlaneList.clear();
   mNormalList.clear();
}

bool ClippedPolyList::isEmpty() const
{
   return mPolyList.size() == 0;
}


//----------------------------------------------------------------------------

U32 ClippedPolyList::addPoint(const Point3F& p)
{
    return addPointAndNormal( p, Point3F::Zero );
}

U32 ClippedPolyList::addPointAndNormal(const Point3F& p, const Point3F& normal)
{
   mVertexList.increment();
   Vertex& v = mVertexList.last();
   v.point.x = p.x * mScale.x;
   v.point.y = p.y * mScale.y;
   v.point.z = p.z * mScale.z;
   mMatrix.mulP(v.point);

    mNormalList.increment();
    VectorF& n = mNormalList.last();
    n = normal;
    if ( !n.isZero() )
        mMatrix.mulV(n);

    AssertFatal(mNormalList.size() == mVertexList.size(), "Normals count does not match vertex count!");    

   // Build the plane mask
   U32      mask = 1;
   S32      count = mPlaneList.size();
   PlaneF * plane = mPlaneList.address();

   v.mask = 0;
   while(--count >= 0) {
      if (plane++->distToPlane(v.point) > 0)
         v.mask |= mask;
      mask <<= 1;
   }

   return mVertexList.size() - 1;
}


U32 ClippedPolyList::addPlane(const PlaneF& plane)
{
   mPolyPlaneList.increment();
   mPlaneTransformer.transform(plane, mPolyPlaneList.last());

   return mPolyPlaneList.size() - 1;
}


//----------------------------------------------------------------------------

void ClippedPolyList::begin(BaseMatInstance* material,U32 surfaceKey)
{
   mPolyList.increment();
   Poly& poly = mPolyList.last();
   poly.object = mCurrObject;
   poly.material = material;
   poly.vertexStart = mIndexList.size();
   poly.surfaceKey = surfaceKey;

   poly.polyFlags = 0;
   if(ClippedPolyList::allowClipping)
      poly.polyFlags = CLIPPEDPOLYLIST_FLAG_ALLOWCLIPPING;
}


//----------------------------------------------------------------------------

void ClippedPolyList::plane(U32 v1,U32 v2,U32 v3)
{
   mPolyList.last().plane.set(mVertexList[v1].point,
      mVertexList[v2].point,mVertexList[v3].point);
}

void ClippedPolyList::plane(const PlaneF& p)
{
   mPlaneTransformer.transform(p, mPolyList.last().plane);
}

void ClippedPolyList::plane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   mPolyList.last().plane = mPolyPlaneList[index];
}

const PlaneF& ClippedPolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   return mPolyPlaneList[index];
}


//----------------------------------------------------------------------------

void ClippedPolyList::vertex(U32 vi)
{
   mIndexList.push_back(vi);
}


//----------------------------------------------------------------------------

void ClippedPolyList::end()
{
   PROFILE_SCOPE( ClippedPolyList_Clip );

   Poly& poly = mPolyList.last();
   
   // Reject polygons facing away from our normal.   
   if ( mDot( poly.plane, mNormal ) < mNormalTolCosineRadians )
   {
      mIndexList.setSize(poly.vertexStart);
      mPolyList.decrement();
      return;
   }

   // Build initial inside/outside plane masks
   U32 indexStart = poly.vertexStart;
   U32 vertexCount = mIndexList.size() - indexStart;

   U32 frontMask = 0,backMask = 0;
   U32 i;
   for (i = indexStart; i < mIndexList.size(); i++) 
   {
      U32 mask = mVertexList[mIndexList[i]].mask;
      frontMask |= mask;
      backMask |= ~mask;
   }

   // Trivial accept if all the vertices are on the backsides of
   // all the planes.
   if (!frontMask) 
   {
      poly.vertexCount = vertexCount;
      return;
   }

   // Trivial reject if any plane not crossed has all it's points
   // on the front.
   U32 crossMask = frontMask & backMask;
   if (~crossMask & frontMask) 
   {
      mIndexList.setSize(poly.vertexStart);
      mPolyList.decrement();
      return;
   }

   // Potentially, this will add up to mPlaneList.size() * (indexStart - indexEnd) 
   // elements to mIndexList, so ensure that it has enough space to store that
   // so we can use push_back_noresize. If you find this code block getting hit
   // frequently, changing the value of 'IndexListReserveSize' or doing some selective
   // allocation is suggested
   //
   // TODO: Re-visit this, since it obviously does not work correctly, and than
   // re-enable the push_back_noresize
   //while(mIndexList.size() + mPlaneList.size() * (mIndexList.size() - indexStart) > mIndexList.capacity() )
   //   mIndexList.reserve(mIndexList.capacity() * 2);

   // Need to do some clipping
   for (U32 p = 0; p < mPlaneList.size(); p++) 
   {
      U32 pmask = 1 << p;

      // Only test against this plane if we have something
      // on both sides
      if (!(crossMask & pmask))
         continue;

      U32 indexEnd = mIndexList.size();
      U32 i1 = indexEnd - 1;
      U32 mask1 = mVertexList[mIndexList[i1]].mask;

      for (U32 i2 = indexStart; i2 < indexEnd; i2++) 
      {
         U32 mask2 = mVertexList[mIndexList[i2]].mask;
         if ((mask1 ^ mask2) & pmask) 
         {
            //
            mVertexList.increment();
            VectorF& v1 = mVertexList[mIndexList[i1]].point;
            VectorF& v2 = mVertexList[mIndexList[i2]].point;
            VectorF vv = v2 - v1;
            F32 t = -mPlaneList[p].distToPlane(v1) / mDot(mPlaneList[p],vv);

            mNormalList.increment();
            VectorF& n1 = mNormalList[mIndexList[i1]];
            VectorF& n2 = mNormalList[mIndexList[i1]];
            VectorF nn = mLerp( n1, n2, t );
            nn.normalizeSafe();
            mNormalList.last() = nn;

            mIndexList.push_back/*_noresize*/(mVertexList.size() - 1);
            Vertex& iv = mVertexList.last();
            iv.point.x = v1.x + vv.x * t;
            iv.point.y = v1.y + vv.y * t;
            iv.point.z = v1.z + vv.z * t;
            iv.mask = 0;

            // Test against the remaining planes
            for (U32 i = p + 1; i < mPlaneList.size(); i++)
               if (mPlaneList[i].distToPlane(iv.point) > 0) 
               {
                  iv.mask = 1 << i;
                  break;
               }
         }

         if (!(mask2 & pmask)) 
         {
            U32 index = mIndexList[i2];
            mIndexList.push_back/*_noresize*/(index);
         }

         mask1 = mask2;
         i1 = i2;
      }

      // Check for degenerate
      indexStart = indexEnd;
      if (mIndexList.size() - indexStart < 3) 
      {
         mIndexList.setSize(poly.vertexStart);
         mPolyList.decrement();
         return;
      }
   }

   // Emit what's left and compress the index list.
   poly.vertexCount = mIndexList.size() - indexStart;
   memcpy(&mIndexList[poly.vertexStart],
      &mIndexList[indexStart],poly.vertexCount);
   mIndexList.setSize(poly.vertexStart + poly.vertexCount);
}


//----------------------------------------------------------------------------

void ClippedPolyList::memcpy(U32* dst, U32* src,U32 size)
{
   U32* end = src + size;
   while (src != end)
      *dst++ = *src++;
}

void ClippedPolyList::cullUnusedVerts()
{
   PROFILE_SCOPE( ClippedPolyList_CullUnusedVerts );

   U32 i = 0;
   U32 k, n, numDeleted;
   bool result;

   IndexListIterator iNextIter;
   VertexListIterator nextVIter;
   VertexListIterator vIter;

   for ( vIter = mVertexList.begin(); vIter != mVertexList.end(); vIter++, i++ )
   {
      // Is this vertex used?
      iNextIter = find( mIndexList.begin(), mIndexList.end(), i );
      if ( iNextIter != mIndexList.end() )
         continue;

      // If not, find the next used vertex.

      // i is an unused vertex
      // k is a used vertex
      // delete the vertices from i to j - 1
      k = 0;
      n = i + 1;
      result = false;
      numDeleted = 0;

      for ( nextVIter = vIter + 1; nextVIter != mVertexList.end(); nextVIter++, n++ )
      {
         iNextIter = find( mIndexList.begin(), mIndexList.end(), n );
         
         // If we found a used vertex
         // grab its index for later use
         // and set our result bool.
         if ( (*iNextIter) == n )
         {
            k = n;
            result = true;
            break;
         }
      }

      // All the remaining verts are unused.
      if ( !result )
      {
         mVertexList.setSize( i );
         mNormalList.setSize( i );
         break;
      }

      // Erase unused verts.
      numDeleted = (k-1) - i + 1;       
      mVertexList.erase( i, numDeleted );
      mNormalList.erase( i, numDeleted );

      // Find any references to vertices after those deleted
      // in the mIndexList and correct with an offset
      for ( iNextIter = mIndexList.begin(); iNextIter != mIndexList.end(); iNextIter++ )
      {
         if ( (*iNextIter) > i )
             (*iNextIter) -= numDeleted;
      }

      // After the erase the current iter should
      // point at the used vertex we found... the
      // loop will continue with the next vert.
   }
}

void ClippedPolyList::triangulate()
{
   PROFILE_SCOPE( ClippedPolyList_Triangulate );

   // Copy the source lists to our temp list and clear
   // the originals which will recieve the results.
   mTempPolyList.set( mPolyList.address(), mPolyList.size() );
   mTempIndexList.set( mIndexList.address(), mIndexList.size() );
   mPolyList.clear();
   mIndexList.clear();

   U32 j, numTriangles;

   //
   PolyListIterator polyIter = mTempPolyList.begin();
   for ( ; polyIter != mTempPolyList.end(); polyIter++ )
   {
      const Poly &poly = *polyIter;

      // How many triangles in this poly?
      numTriangles = poly.vertexCount - 2;        

      // Build out the triangles.
      for ( j = 0; j < numTriangles; j++ )
      {
         mPolyList.increment();
         
         Poly &triangle = mPolyList.last();
         triangle = poly;
         triangle.vertexCount = 3;
         triangle.vertexStart = mIndexList.size();

         mIndexList.push_back( mTempIndexList[ poly.vertexStart ] );
         mIndexList.push_back( mTempIndexList[ poly.vertexStart + 1 + j ] );
         mIndexList.push_back( mTempIndexList[ poly.vertexStart + 2 + j ] );
      }
   }
}

void ClippedPolyList::generateNormals()
{
   PROFILE_SCOPE( ClippedPolyList_GenerateNormals );

   AssertFatal(mNormalList.size() == mVertexList.size(), "Normals count does not match vertex count!");    

   U32 i, polyCount;
   VectorF normal;
   PolyListIterator polyIter;
   IndexListIterator indexIter;

   Vector<VectorF>::iterator normalIter = mNormalList.begin();
   U32 n = 0;
   for ( ; normalIter != mNormalList.end(); normalIter++, n++ )
   {
       // Skip normals that already have values.
       if ( !normalIter->isZero() )
           continue;

      // Average all the face normals which 
      // share this vertex index.
      indexIter = mIndexList.begin();
      normal.zero();
      polyCount = 0;
      i = 0;

      for ( ; indexIter != mIndexList.end(); indexIter++, i++ )
      {
         if ( n != *indexIter )
            continue;

         polyIter = mPolyList.begin();
         for ( ; polyIter != mPolyList.end(); polyIter++ )
         {
            const Poly& poly = *polyIter;
            if ( i < poly.vertexStart || i > poly.vertexStart + poly.vertexCount )
               continue;

            ++polyCount;
            normal += poly.plane;
         }        
      }

      // Average it.
      if ( polyCount > 0 )
         normal /= (F32)polyCount;

      // Note: we use a temporary for the normal averaging 
      // then copy the result to limit the number of arrays
      // we're touching during the innermost loop.
      *normalIter = normal;
   }
}
