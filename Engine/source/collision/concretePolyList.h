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

#ifndef _CONCRETEPOLYLIST_H_
#define _CONCRETEPOLYLIST_H_

#ifndef _ABSTRACTPOLYLIST_H_
#include "collision/abstractPolyList.h"
#endif

/// A concrete, renderable PolyList
///
/// This class is used to store geometry from a PolyList query.
///
/// It allows you to render this data, as well.
///
/// @see AbstractPolyList
class ConcretePolyList : public AbstractPolyList
{
  public:

   struct Poly {
      PlaneF plane;
      SceneObject* object;
      BaseMatInstance* material;
      U32 vertexStart;
      U32 vertexCount;
      U32 surfaceKey;

      Poly()
      {
         object = NULL;
         material = NULL;
      }
   };

   typedef Vector<PlaneF> PlaneList;
   typedef Vector<Point3F> VertexList;
   typedef Vector<Poly>   PolyList;
   typedef Vector<U32>    IndexList;

   PolyList   mPolyList;
   VertexList mVertexList;
   IndexList  mIndexList;

   PlaneList  mPolyPlaneList;

  public:
   ConcretePolyList();
   ~ConcretePolyList();
   void clear();

   // Virtual methods
   U32  addPoint(const Point3F& p);
   U32  addPlane(const PlaneF& plane);
   void begin(BaseMatInstance* material,U32 surfaceKey);
   void plane(U32 v1,U32 v2,U32 v3);
   void plane(const PlaneF& p);
   void plane(const U32 index);
   void vertex(U32 vi);
   void end();
   void render();

   bool isEmpty() const;

   /// This breaks all polys in the polylist into triangles.
   void triangulate();

  protected:
   const PlaneF& getIndexedPlane(const U32 index);
};

#endif  // _CONCRETEPOLYLIST_H_
