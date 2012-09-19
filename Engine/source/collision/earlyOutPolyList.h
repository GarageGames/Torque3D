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

#ifndef _EARLYOUTPOLYLIST_H_
#define _EARLYOUTPOLYLIST_H_

#ifndef _ABSTRACTPOLYLIST_H_
#include "collision/abstractPolyList.h"
#endif


/// Early out check PolyList
///
/// This class is used primarily for triggers and similar checks. It checks to see
/// if any of the geometry you feed it is inside its area, and if it is, it stops
/// checking for any more data and returns a true value. This is good if you want
/// to know if anything is in your "trigger" area, for instance.
///
/// @see AbstractPolyList
class EarlyOutPolyList : public AbstractPolyList
{
   void memcpy(U32* d, U32* s,U32 size);

   // Internal data
   struct Vertex {
      Point3F point;
      U32 mask;
   };

   struct Poly {
      PlaneF plane;
      SceneObject* object;
      BaseMatInstance* material;
      U32 vertexStart;
      U32 vertexCount;
      U32 surfaceKey;
   };

  public:
   typedef Vector<PlaneF> PlaneList;
  private:
   typedef Vector<Vertex> VertexList;
   typedef Vector<Poly>   PolyList;
   typedef Vector<U32>    IndexList;

   PolyList   mPolyList;
   VertexList mVertexList;
   IndexList  mIndexList;
   bool       mEarlyOut;

   PlaneList  mPolyPlaneList;

  public:
   // Data set by caller
   PlaneList mPlaneList;
   VectorF   mNormal;

  public:
   EarlyOutPolyList();
   ~EarlyOutPolyList();
   void clear();

   // Virtual methods
   bool isEmpty() const;
   U32  addPoint(const Point3F& p);
   U32  addPlane(const PlaneF& plane);
   void begin(BaseMatInstance* material,U32 surfaceKey);
   void plane(U32 v1,U32 v2,U32 v3);
   void plane(const PlaneF& p);
   void plane(const U32 index);
   void vertex(U32 vi);
   void end();

  protected:
   const PlaneF& getIndexedPlane(const U32 index);
};

#endif  // _H_EARLYOUTPOLYLIST_
