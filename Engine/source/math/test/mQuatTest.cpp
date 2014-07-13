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
#include "math/mQuat.h"
#include "math/mAngAxis.h"
#include "math/mMatrix.h"

/// For testing things that should be close to 0, but accounting for floating-
/// point inaccuracy.
static const F32 epsilon = 1e-3f;

/// Test quaternions for equality by expecting the angle between them to be
/// close to 0.
#define EXPECT_QUAT_EQ(q1, q2) EXPECT_LT(q1.angleBetween(q2), epsilon)

TEST(QuatF, AngleBetween)
{
   QuatF p(QuatF::Identity), q(QuatF::Identity);
   EXPECT_LT(p.angleBetween(q), epsilon)
      << "Angle between identity quaternions should be ~0.";

   p.set(EulerF(0.1, 0.15, -0.2));
   q = p;
   EXPECT_LT(p.angleBetween(q), epsilon)
      << "Angle between identical quaternions should be ~0.";
}

/// Test conversion from EulerF.
TEST(QuatF, Construction)
{
   EulerF eId(0, 0, 0);
   EXPECT_QUAT_EQ(QuatF(eId), QuatF::Identity)
      << "Quaternions constructed from identity EulerF and QuatF::Identity not equal.";

   EulerF eRot(0.0f, -0.0f, 1.5707963267948966f);
   MatrixF mat(eRot);
   AngAxisF aaRot(mat);
   EXPECT_QUAT_EQ(QuatF(eRot), QuatF(aaRot))
      << "Quaternions constructed from EulerF and AngAxisF not equal.";
}

#endif