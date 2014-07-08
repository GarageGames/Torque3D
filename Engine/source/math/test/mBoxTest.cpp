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
#include "math/mBox.h"

TEST(Box3F, GetOverlap)
{
   Box3F b1(Point3F(-1, -1, -1), Point3F(1, 1, 1));
   EXPECT_EQ(b1.getOverlap(b1), b1)
      << "A box's overlap with itself should be itself.";

   Box3F b2(Point3F(0, 0, 0), Point3F(1, 1, 1));
   EXPECT_EQ(b1.getOverlap(b2), b2)
      << "Box's overlap should be the intersection of two boxes.";

   Box3F b3(Point3F(10, 10, 10), Point3F(11, 11, 11));
   EXPECT_TRUE(b1.getOverlap(b3).isEmpty())
      << "Overlap of boxes that do not overlap should be empty.";
}

#endif