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

#include "platform/input/oculusVR/oculusVRUtil.h"

namespace OculusVRUtil
{

void convertRotation(const F32 inRotMat[4][4], MatrixF& outRotation)
{
   // Set rotation.  We need to convert from sensor coordinates to
   // Torque coordinates.  The sensor matrix is stored row-major.
   // The conversion is:
   //
   // Sensor                       Torque
   // a b c         a  b  c        a -c  b
   // d e f   -->  -g -h -i  -->  -g  i -h
   // g h i         d  e  f        d -f  e
   outRotation.setColumn(0, Point4F( inRotMat[0][0], -inRotMat[2][0],  inRotMat[1][0], 0.0f));
   outRotation.setColumn(1, Point4F(-inRotMat[0][2],  inRotMat[2][2], -inRotMat[1][2], 0.0f));
   outRotation.setColumn(2, Point4F( inRotMat[0][1], -inRotMat[2][1],  inRotMat[1][1], 0.0f));
   outRotation.setPosition(Point3F::Zero);
}

void convertRotation(OVR::Quatf& inRotation, EulerF& outRotation)
{
   F32 yaw, pitch, roll;
   inRotation.GetEulerAngles<OVR::Axis_X, OVR::Axis_Z, OVR::Axis_Y, OVR::Rotate_CW, OVR::Handed_R>(&outRotation.x, &outRotation.y, &outRotation.z);
}

void calculateAxisRotation(const MatrixF& inRotation, const F32& maxAxisRadius, Point2F& outRotation)
{
   const VectorF& controllerUp = inRotation.getUpVector();
   Point2F axis(0,0);
   axis.x = controllerUp.x;
   axis.y = controllerUp.y;

   // Limit the axis angle to that given to us
   if(axis.len() > maxAxisRadius)
   {
      axis.normalize(maxAxisRadius);
   }

   // Renormalize to the range of 0..1
   if(maxAxisRadius != 0.0f)
   {
      axis /= maxAxisRadius;
   }

   outRotation.x = axis.x;
   outRotation.y = axis.y;
}

void convertAcceleration(OVR::Vector3f& inAcceleration, VectorF& outAcceleration)
{
   outAcceleration.set(inAcceleration.x, -inAcceleration.z, inAcceleration.y);
}

void convertAngularVelocity(OVR::Vector3f& inAngVel, EulerF& outAngVel)
{
   outAngVel.set(-inAngVel.x, inAngVel.z, -inAngVel.y);
}

void convertMagnetometer(OVR::Vector3f& inMagnetometer, VectorF& outMagnetometer)
{
   outMagnetometer.set(inMagnetometer.x, -inMagnetometer.z, inMagnetometer.y);
}

}
