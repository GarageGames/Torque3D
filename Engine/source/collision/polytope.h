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

#ifndef _POLYTOPE_H_
#define _POLYTOPE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


//----------------------------------------------------------------------------

class SimObject;


//----------------------------------------------------------------------------

class Polytope
{
   // Convex Polyhedron
public:
   struct Vertex {
      Point3F point;
      /// Temp BSP clip info
      S32 side;
   };
   struct Edge {
      S32 vertex[2];
      S32 face[2];
      S32 next;
   };
   struct Face {
      PlaneF plane;
      S32 original;
      /// Temp BSP clip info
      S32 vertex;
   };
   struct Volume
   {
      S32 edgeList;
      S32 material;
      SimObject* object;
   };
   struct StackElement
   {
      S32 edgeList;
      const BSPNode *node;
   };
   struct Collision {
      SimObject* object;
      S32 material;
      PlaneF plane;
      Point3F point;
      F32 distance;

      Collision()
      {
         object = NULL;
         distance = 0.0;
      }
   };

   typedef Vector<Edge> EdgeList;
   typedef Vector<Face> FaceList;
   typedef Vector<Vertex> VertexList;
   typedef Vector<Volume> VolumeList;
   typedef Vector<StackElement> VolumeStack;

   //
   S32 sideCount;
   EdgeList mEdgeList;
   FaceList mFaceList;
   VertexList mVertexList;
   VolumeList mVolumeList;

private:
   bool intersect(const PlaneF& plane,const Point3F& sp,const Point3F& ep);

public:
   //
   Polytope();
   void buildBox(const MatrixF& transform,const Box3F& box);
   void intersect(SimObject*, const BSPNode* node);
   inline bool didIntersect()  { return mVolumeList.size() > 1; }
   void extrudeFace(int fi,const VectorF& vec,Polytope* out);
   bool findCollision(const VectorF& vec,Polytope::Collision *best);
};




#endif
