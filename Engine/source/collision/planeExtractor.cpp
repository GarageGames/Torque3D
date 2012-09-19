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
#include "math/mMath.h"
#include "console/console.h"
#include "collision/planeExtractor.h"

//----------------------------------------------------------------------------
// Plane matching parameters
static F32 NormalEpsilon = 0.93969f; // 20 deg.
static F32 DistanceEpsilon = 100.0f;


//----------------------------------------------------------------------------

PlaneExtractorPolyList::PlaneExtractorPolyList()
{
   VECTOR_SET_ASSOCIATION(mVertexList);
   VECTOR_SET_ASSOCIATION(mPolyPlaneList);
}

PlaneExtractorPolyList::~PlaneExtractorPolyList()
{
}


//----------------------------------------------------------------------------

void PlaneExtractorPolyList::clear()
{
   mVertexList.clear();
   mPolyPlaneList.clear();
}

U32 PlaneExtractorPolyList::addPoint(const Point3F& p)
{
   mVertexList.increment();
   Point3F& v = mVertexList.last();
   v.x = p.x * mScale.x;
   v.y = p.y * mScale.y;
   v.z = p.z * mScale.z;
   mMatrix.mulP(v);
   return mVertexList.size() - 1;
}

U32 PlaneExtractorPolyList::addPlane(const PlaneF& plane)
{
   mPolyPlaneList.increment();
   mPlaneTransformer.transform(plane, mPolyPlaneList.last());

   return mPolyPlaneList.size() - 1;
}


//----------------------------------------------------------------------------

void PlaneExtractorPolyList::plane(U32 v1,U32 v2,U32 v3)
{
   mPlaneList->last().set(mVertexList[v1],
      mVertexList[v2],mVertexList[v3]);
}

void PlaneExtractorPolyList::plane(const PlaneF& p)
{
   mPlaneTransformer.transform(p, mPlaneList->last());
}

void PlaneExtractorPolyList::plane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   mPlaneList->last() = mPolyPlaneList[index];
}

const PlaneF& PlaneExtractorPolyList::getIndexedPlane(const U32 index)
{
   AssertFatal(index < mPolyPlaneList.size(), "Out of bounds index!");
   return mPolyPlaneList[index];
}


//----------------------------------------------------------------------------

bool PlaneExtractorPolyList::isEmpty() const
{
   return true;
}

void PlaneExtractorPolyList::begin(BaseMatInstance*,U32)
{
   mPlaneList->increment();
}

void PlaneExtractorPolyList::end()
{
   // See if there are any duplicate planes
   PlaneF &plane = mPlaneList->last();
   PlaneF *ptr = mPlaneList->begin();
   for (; ptr != &plane; ptr++)
      if (mFabs(ptr->d - plane.d) < DistanceEpsilon &&
            mDot(*ptr,plane) > NormalEpsilon) {
         mPlaneList->decrement();
         return;
      }
}

void PlaneExtractorPolyList::vertex(U32)
{
}

