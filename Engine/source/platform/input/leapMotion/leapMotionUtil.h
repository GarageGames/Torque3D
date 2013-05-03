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

#ifndef _LEAPMOTIONUTIL_H_
#define _LEAPMOTIONUTIL_H_

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "Leap.h"

namespace LeapMotionUtil
{
   /// Convert from a Leap Motion position to a Torque 3D position
   void convertPosition(const Leap::Vector& inPosition, F32& x, F32& y, F32& z);

   /// Convert from a Leap Motion position to a Torque 3D Point3F
   void convertPosition(const Leap::Vector& inPosition, Point3F& outPosition);

   /// Convert a Leap Motion hand's rotation to a Torque 3D matrix
   void convertHandRotation(const Leap::Hand& hand, MatrixF& outRotation);

   /// Calcualte a hand's rotation as if it were a thumb stick axis
   void calculateHandAxisRotation(const MatrixF& handRotation, const F32& maxHandAxisRadius, Point2F& outRotation);

   /// Convert a Leap Motion pointable's rotation to a Torque 3D matrix
   void convertPointableRotation(const Leap::Pointable& pointable, MatrixF& outRotation);
}

#endif
