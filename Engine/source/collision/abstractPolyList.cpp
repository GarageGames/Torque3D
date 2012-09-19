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

#include "collision/abstractPolyList.h"


//----------------------------------------------------------------------------

AbstractPolyList::~AbstractPolyList()
{
   mInterestNormalRegistered = false;
}

static U32 PolyFace[6][4] = {
   { 3, 2, 1, 0 },
   { 7, 4, 5, 6 },
   { 0, 5, 4, 3 },
   { 6, 5, 0, 1 },
   { 7, 6, 1, 2 },
   { 4, 7, 2, 3 },
};

void AbstractPolyList::addBox(const Box3F &box, BaseMatInstance* material)
{
   Point3F pos = box.minExtents;
   F32 dx = box.maxExtents.x - box.minExtents.x;
   F32 dy = box.maxExtents.y - box.minExtents.y;
   F32 dz = box.maxExtents.z - box.minExtents.z;

   U32 base = addPoint(pos);
   pos.y += dy; addPoint(pos);
   pos.x += dx; addPoint(pos);
   pos.y -= dy; addPoint(pos);
   pos.z += dz; addPoint(pos);
   pos.x -= dx; addPoint(pos);
   pos.y += dy; addPoint(pos);
   pos.x += dx; addPoint(pos);

   for (S32 i = 0; i < 6; i++) {
      begin(material, i);
      S32 v1 = base + PolyFace[i][0];
      S32 v2 = base + PolyFace[i][1];
      S32 v3 = base + PolyFace[i][2];
      S32 v4 = base + PolyFace[i][3];
      vertex(v1);
      vertex(v2);
      vertex(v3);
      vertex(v4);
      plane(v1, v2, v3);
      end();
   }
}

bool AbstractPolyList::getMapping(MatrixF *, Box3F *)
{
   // return list transform and bounds in list space...optional
   return false;
}


bool AbstractPolyList::isInterestedInPlane(const PlaneF& plane)
{
   if (mInterestNormalRegistered == false) {
      return true;
   }
   else {
      PlaneF xformed;
      mPlaneTransformer.transform(plane, xformed);
      if (mDot(xformed, mInterestNormal) >= 0.0f)
         return false;
      else
         return true;
   }
}

bool AbstractPolyList::isInterestedInPlane(const U32 index)
{
   if (mInterestNormalRegistered == false) {
      return true;
   }
   else {
      const PlaneF& rPlane = getIndexedPlane(index);
      if (mDot(rPlane, mInterestNormal) >= 0.0f)
         return false;
      else
         return true;
   }
}

void AbstractPolyList::setInterestNormal(const Point3F& normal)
{
   mInterestNormalRegistered = true;
   mInterestNormal           = normal;
}

