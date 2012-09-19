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

#ifndef _SPHEREMESH_H_
#define _SPHEREMESH_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

//------------------------------------------------------------------------------
// Class: SphereMesh
//------------------------------------------------------------------------------
// * ctor takes type of base polyhedron that is subdivided to create sphere
// * getMesh(...) will subdivide the current mesh to the desired level where
//   (each level has 4 times the polys of the previous level)

class SphereMesh
{
   public:

      // regular polyhedra with triangle face polygons (num of faces)
      enum {
         Tetrahedron = 4,
         Octahedron = 8,
         Icosahedron = 20,

         MaxLevel       = 5
      };

      struct Triangle {
         Triangle() {}
         Triangle(const Point3F &a, const Point3F &b, const Point3F &c)
         {
            pnt[0] = a;
            pnt[1] = b;
            pnt[2] = c;
         }

         Point3F     pnt[3];
         Point3F     normal;
      };

      struct TriangleMesh {
         U32            numPoly;
         Triangle *     poly;
      };

      SphereMesh(U32 baseType = Octahedron);
      ~SphereMesh();

      const TriangleMesh * getMesh(U32 level = 0);

   private:

      TriangleMesh * createTetrahedron();
      TriangleMesh * createOctahedron();
      TriangleMesh * createIcosahedron();

      Vector<TriangleMesh*>      mDetails;

      void calcNormals(TriangleMesh *);
      TriangleMesh * subdivideMesh(TriangleMesh*);
};

#endif
