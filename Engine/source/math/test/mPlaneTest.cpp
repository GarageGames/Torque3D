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
#include "math/mPlane.h"

// Static test data. All combinations of position and normal are tested in each
// test case. This allows a large number of tests without introducing non-
// deterministic test behavior.

static const Point3F positions[] = {Point3F(0, 0, 0), Point3F(1, -2, 3), Point3F(1e-2, -2e-2, 1)};
static const U32 numPositions = sizeof(positions) / sizeof(Point3F);

static const Point3F normals[] = {Point3F(1, 0, 0), Point3F(-4, -2, 6)};
static const U32 numNormals = sizeof(normals) / sizeof(Point3F);

/// Tests that points in the direction of the normal are in 'Front' of the
/// plane, while points in the reverse direction of the normal are in
/// 'Back' of the plane.
TEST(Plane, WhichSide)
{
   for(U32 i = 0; i < numPositions; i++) {
      for(U32 j = 0; j < numNormals; j++) {
         Point3F position = positions[i];
         Point3F normal = normals[j];

         PlaneF p(position, normal);

         EXPECT_EQ(p.whichSide(position + normal), PlaneF::Front );
         EXPECT_EQ(p.whichSide(position - normal), PlaneF::Back );
         EXPECT_EQ(p.whichSide(position), PlaneF::On );
      }
   }
}

/// Tests that the distToPlane method returns the exact length that the test
/// point is offset by in the direction of the normal.
TEST(Plane, DistToPlane)
{
   for(U32 i = 0; i < numPositions; i++) {
      for(U32 j = 0; j < numNormals; j++) {
         Point3F position = positions[i];
         Point3F normal = normals[j];

         PlaneF p(position, normal);

         EXPECT_FLOAT_EQ(p.distToPlane(position + normal), normal.len());
         EXPECT_FLOAT_EQ(p.distToPlane(position - normal), -normal.len());
         EXPECT_FLOAT_EQ(p.distToPlane(position), 0);
      }
   }
}

#endif
