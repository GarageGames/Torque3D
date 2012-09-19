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
#include "console/console.h"
#include "collision/earlyOutPolyList.h"


//----------------------------------------------------------------------------

EarlyOutPolyList::EarlyOutPolyList()
{
   VECTOR_SET_ASSOCIATION(mPolyList);
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mIndexList);
   VECTOR_SET_ASSOCIATION(mPolyPlaneList);
   VECTOR_SET_ASSOCIATION(mPlaneList);

   mNormal.set(0, 0, 0);
   mIndexList.reserve(100);

   mEarlyOut = false;
}

EarlyOutPolyList::~EarlyOutPolyList()
{

}


//----------------------------------------------------------------------------
void EarlyOutPolyList::clear()
{
   // Only clears internal data
   mPolyList.clear();
   mVertexList.clear();
   mIndexList.clear();
   mPolyPlaneList.clear();

   mEarlyOut = false;
}

bool EarlyOutPolyList::isEmpty() const
{
   return mEarlyOut == false;
}


//----------------------------------------------------------------------------

U32 EarlyOutPolyList::addPoint(const Point3F& p)
{
   if (mEarlyOut == true)
      return 0;

   mVertexList.increment();
   Vertex& v = mVertexList.last();
   v.point.x = p.x * mScale.x;
   v.point.y = p.y * mScale.y;
   v.point.z = p.z * mScale.z;
   mMatrix.mulP(v.point);

   // Build the plane mask
   v.mask = 0;
   for (U32 i = 0; i < mPlaneList.size(); i++)
      if (mPlaneList[i].distToPlane(v.point) > 0)
         v.mask |= 1 << i;

   // If the point is inside all the planes, then we're done!
   if (v.mask == 0)
      mEarlyOut = true;

   return mVertexList.size() - 1;
}


U32 EarlyOutPolyList::addPlane(const PlaneF& plane)
{
   mPolyPlaneList.increment();
   mPlaneTransformer.transform(plane, mPolyPlaneList.last());

   return mPolyPlaneList.size() - 1;
}


//----------------------------------------------------------------------------

void EarlyOutPolyList::begin(BaseMatInstance* material,U32 surfaceKey)
{
   if (mEarlyOut == true)
      return;

   mPolyList.increment();
   Poly& poly = mPolyList.last();
   poly.object = mCurrObject;
   poly.material = material;
   poly.vertexStart = mIndexList.size();
   poly.surfaceKey = surfaceKey;
}


//----------------------------------------------------------------------------

void EarlyOutPolyList::plane(U32 v1,U32 v2,U32 v3)
{
   if (mEarlyOut == true)
      return;

   mPolyList.last().plane.set(mVertexList[v1].point,
      mVertexList[v2].point,mVertexList[v3].point);
}

void EarlyOutPolyList::plane(const PlaneF& p)
{
   if (mEarlyOut == true)
      return;

   mPlaneTransformer.transform(p, mPolyList.last().plane);
}

void EarlyOutPolyList::plane(const U32 index)
{
   if (mEarlyOut == true)
      return;

   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   mPolyList.last().plane = mPolyPlaneList[index];
}

const PlaneF& EarlyOutPolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   return mPolyPlaneList[index];
}


//----------------------------------------------------------------------------

void EarlyOutPolyList::vertex(U32 vi)
{
   if (mEarlyOut == true)
      return;

   mIndexList.push_back(vi);
}


//----------------------------------------------------------------------------

void EarlyOutPolyList::end()
{
   if (mEarlyOut == true)
      return;

   Poly& poly = mPolyList.last();

   // Anything facing away from the mNormal is rejected
   if (mDot(poly.plane,mNormal) > 0) {
      mIndexList.setSize(poly.vertexStart);
      mPolyList.decrement();
      return;
   }

   // Build intial inside/outside plane masks
   U32 indexStart = poly.vertexStart;
   U32 vertexCount = mIndexList.size() - indexStart;

   U32 frontMask = 0,backMask = 0;
   U32 i;
   for (i = indexStart; i < mIndexList.size(); i++) {
      U32 mask = mVertexList[mIndexList[i]].mask;
      frontMask |= mask;
      backMask |= ~mask;
   }

   // Trivial accept if all the vertices are on the backsides of
   // all the planes.
   if (!frontMask) {
      poly.vertexCount = vertexCount;
      mEarlyOut = true;
      return;
   }

   // Trivial reject if any plane not crossed has all it's points
   // on the front.
   U32 crossMask = frontMask & backMask;
   if (~crossMask & frontMask) {
      mIndexList.setSize(poly.vertexStart);
      mPolyList.decrement();
      return;
   }

   // Need to do some clipping
   for (U32 p = 0; p < mPlaneList.size(); p++) {
      U32 pmask = 1 << p;
      // Only test against this plane if we have something
      // on both sides
      if (crossMask & pmask) {
         U32 indexEnd = mIndexList.size();
         U32 i1 = indexEnd - 1;
         U32 mask1 = mVertexList[mIndexList[i1]].mask;

         for (U32 i2 = indexStart; i2 < indexEnd; i2++) {
            U32 mask2 = mVertexList[mIndexList[i2]].mask;
            if ((mask1 ^ mask2) & pmask) {
               //
               mVertexList.increment();
               VectorF& v1 = mVertexList[mIndexList[i1]].point;
               VectorF& v2 = mVertexList[mIndexList[i2]].point;
               VectorF vv = v2 - v1;
               F32 t = -mPlaneList[p].distToPlane(v1) / mDot(mPlaneList[p],vv);

               mIndexList.push_back(mVertexList.size() - 1);
               Vertex& iv = mVertexList.last();
               iv.point.x = v1.x + vv.x * t;
               iv.point.y = v1.y + vv.y * t;
               iv.point.z = v1.z + vv.z * t;
               iv.mask = 0;

               // Test against the remaining planes
               for (U32 i = p + 1; i < mPlaneList.size(); i++)
                  if (mPlaneList[i].distToPlane(iv.point) > 0) {
                     iv.mask = 1 << i;
                     break;
                  }
            }
            if (!(mask2 & pmask)) {
               U32 index = mIndexList[i2];
               mIndexList.push_back(index);
            }
            mask1 = mask2;
            i1 = i2;
         }

         // Check for degenerate
         indexStart = indexEnd;
         if (mIndexList.size() - indexStart < 3) {
            mIndexList.setSize(poly.vertexStart);
            mPolyList.decrement();
            return;
         }
      }
   }

   // If we reach here, then there's a poly!
   mEarlyOut = true;

   // Emit what's left and compress the index list.
   poly.vertexCount = mIndexList.size() - indexStart;
   memcpy(&mIndexList[poly.vertexStart],
      &mIndexList[indexStart],poly.vertexCount);
   mIndexList.setSize(poly.vertexStart + poly.vertexCount);
}


//----------------------------------------------------------------------------

void EarlyOutPolyList::memcpy(U32* dst, U32* src,U32 size)
{
   U32* end = src + size;
   while (src != end)
      *dst++ = *src++;
}

