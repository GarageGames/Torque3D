//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "math/mPolyhedron.h"

FIXTURE(Polyhedron)
{
protected:
   Vector<PlaneF> planes;

   virtual void SetUp()
   {
      // Build planes for a unit cube centered at the origin.
      // Note that the normals must be facing inwards.
      planes.push_back(PlaneF(Point3F(-0.5f,  0.f,   0.f ), Point3F( 1.f,  0.f,  0.f)));
      planes.push_back(PlaneF(Point3F( 0.5f,  0.f,   0.f ), Point3F(-1.f,  0.f,  0.f)));
      planes.push_back(PlaneF(Point3F( 0.f,  -0.5f,  0.f ), Point3F( 0.f,  1.f,  0.f)));
      planes.push_back(PlaneF(Point3F( 0.f,   0.5f,  0.f ), Point3F( 0.f, -1.f,  0.f)));
      planes.push_back(PlaneF(Point3F( 0.f,   0.f,  -0.5f), Point3F( 0.f,  0.f,  1.f)));
      planes.push_back(PlaneF(Point3F( 0.f,   0.f,   0.5f), Point3F( 0.f,  0.f, -1.f)));
   }
};

TEST_FIX(Polyhedron, BuildFromPlanes)
{
   // Turn planes into a polyhedron.
   Polyhedron p1;
   p1.buildFromPlanes(PlaneSetF(planes.address(), planes.size()));

   // Check if we got a cube back.
   EXPECT_EQ(p1.getNumPoints(), 8);
   EXPECT_EQ(p1.getNumPlanes(), 6);
   EXPECT_EQ(p1.getNumEdges(), 12);

   // Add extra plane that doesn't contribute a new edge.
   Vector<PlaneF> planes2 = planes;
   planes2.push_back( PlaneF( Point3F( 0.5f, 0.5f, 0.5f ), Point3F( -1.f, -1.f, -1.f ) ) );

   // Turn them into another polyhedron.
   Polyhedron p2;
   p2.buildFromPlanes(PlaneSetF(planes2.address(), planes2.size()));

   // Check if we got a cube back.
   EXPECT_EQ(p2.getNumPoints(), 8);
   EXPECT_EQ(p2.getNumPlanes(), 6);
   EXPECT_EQ(p2.getNumEdges(), 12);
}

#endif
