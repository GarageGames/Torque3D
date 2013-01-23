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

#include "platform/input/leapMotion/leapMotionUtil.h"

namespace LeapMotionUtil
{

void convertPosition(const Leap::Vector& inPosition, F32& x, F32& y, F32& z)
{
   // Convert to Torque coordinates.  The conversion is:
   //
   // Motion       Torque
   // x y z   -->  x -z y
   x = inPosition.x;    // x = x
   y = -inPosition.z;   // y = -z
   z = inPosition.y;    // z = y;
}

void convertPosition(const Leap::Vector& inPosition, Point3F& outPosition)
{
   // Convert to Torque coordinates.  The conversion is:
   //
   // Motion       Torque
   // x y z   -->  x -z y
   outPosition.x = inPosition.x;    // x = x
   outPosition.y = -inPosition.z;   // y = -z
   outPosition.z = inPosition.y;    // z = y;
}

void convertHandRotation(const Leap::Hand& hand, MatrixF& outRotation)
{
   // We need to convert from Motion coordinates to
   // Torque coordinates.  The conversion is:
   //
   // Motion                       Torque
   // a b c         a  b  c        a -c  b
   // d e f   -->  -g -h -i  -->  -g  i -h
   // g h i         d  e  f        d -f  e
   const Leap::Vector& handToFingers = hand.direction();
   Leap::Vector handFront = -handToFingers;
   const Leap::Vector& handDown = hand.palmNormal();
   Leap::Vector handUp = -handDown;
   Leap::Vector handRight = handUp.cross(handFront);

   outRotation.setColumn(0, Point4F(  handRight.x, -handRight.z,  handRight.y,  0.0f));
   outRotation.setColumn(1, Point4F( -handFront.x,  handFront.z, -handFront.y,  0.0f));
   outRotation.setColumn(2, Point4F(  handUp.x,    -handUp.z,     handUp.y,     0.0f));
   outRotation.setPosition(Point3F::Zero);
}

void calculateHandAxisRotation(const MatrixF& handRotation, const F32& maxHandAxisRadius, Point2F& outRotation)
{
   const VectorF& controllerUp = handRotation.getUpVector();
   outRotation.x = controllerUp.x;
   outRotation.y = controllerUp.y;

   // Limit the axis angle to that given to us
   if(outRotation.len() > maxHandAxisRadius)
   {
      outRotation.normalize(maxHandAxisRadius);
   }

   // Renormalize to the range of 0..1
   if(maxHandAxisRadius != 0.0f)
   {
      outRotation /= maxHandAxisRadius;
   }
}

void convertPointableRotation(const Leap::Pointable& pointable, MatrixF& outRotation)
{
   // We need to convert from Motion coordinates to
   // Torque coordinates.  The conversion is:
   //
   // Motion                       Torque
   // a b c         a  b  c        a -c  b
   // d e f   -->  -g -h -i  -->  -g  i -h
   // g h i         d  e  f        d -f  e
   Leap::Vector pointableFront = -pointable.direction();
   Leap::Vector pointableRight = Leap::Vector::up().cross(pointableFront);
   Leap::Vector pointableUp = pointableFront.cross(pointableRight);

   outRotation.setColumn(0, Point4F(  pointableRight.x, -pointableRight.z,  pointableRight.y,  0.0f));
   outRotation.setColumn(1, Point4F( -pointableFront.x,  pointableFront.z, -pointableFront.y,  0.0f));
   outRotation.setColumn(2, Point4F(  pointableUp.x,    -pointableUp.z,     pointableUp.y,     0.0f));
   outRotation.setPosition(Point3F::Zero);
}

}
