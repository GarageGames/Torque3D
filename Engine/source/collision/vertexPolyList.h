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

#ifndef _VERTEXPOLYLIST_H_
#define _VERTEXPOLYLIST_H_

#ifndef _ABSTRACTPOLYLIST_H_
#include "collision/abstractPolyList.h"
#endif


/// A simple polylist which only gathers the unique verticies passed to it.
class VertexPolyList : public AbstractPolyList
{
public:

   VertexPolyList();
   virtual ~VertexPolyList() {}

   // AbstractPolyList
   U32 addPoint(const Point3F& p);
   U32 addPlane(const PlaneF& plane) { return 0; } 
   void begin(BaseMatInstance* material,U32 surfaceKey) {}
   void plane(U32 v1,U32 v2,U32 v3) {}
   void plane(const PlaneF& p) {}
   void plane(const U32 index) {}
   void vertex(U32 vi) {}
   void end() {}
   const PlaneF& getIndexedPlane(const U32 index);

   /// Clears any captured verts.
   void clear();

   /// Returns true if the polylist contains no verts.
   bool isEmpty() const { return mVertexList.empty(); }

   /// Returns the vertex list.
   Vector<Point3F>& getVertexList() { return mVertexList; }

   /// Returns the constant vertex list.
   const Vector<Point3F>& getVertexList() const { return mVertexList; }

protected:

   /// The unique verts we captured.
   Vector<Point3F> mVertexList;

};


#endif  // _VERTEXPOLYLIST_H_
