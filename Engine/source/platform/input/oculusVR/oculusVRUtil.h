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

#ifndef _OCULUSVRUTIL_H_
#define _OCULUSVRUTIL_H_

#include "math/mPoint2.h"
#include "math/mMatrix.h"
#include "OVR_CAPI_0_8_0.h"

// NOTE: math code in oculus uses "Offset" which is a preprocessor macro
#define TorqueOffset Offset
#undef Offset
#include "Extras/OVR_Math.h"

namespace OculusVRUtil
{
   /// Convert an OVR sensor's rotation to a Torque 3D matrix
   void convertRotation(const F32 inRotMat[4][4], MatrixF& outRotation);

   /// Convert an OVR sensor's rotation to Torque 3D Euler angles (in radians)
   void convertRotation(OVR::Quatf& inRotation, EulerF& outRotation);

   /// Calcualte a sensor's rotation as if it were a thumb stick axis
   void calculateAxisRotation(const MatrixF& inRotation, const F32& maxAxisRadius, Point2F& outRotation);

   /// Convert an OVR sensor's acceleration to Torque 3D vector (in m/s^2)
   void convertAcceleration(OVR::Vector3f& inAcceleration, VectorF& outAcceleration);

   /// Convert OVR sensor's angular velocity to Torque 3D Euler angles (in radians/s)
   void convertAngularVelocity(OVR::Vector3f& inAngVel, EulerF& outAngVel);

   /// Convert an OVR sensor's magnetometer reading (direction and field strength) to Torque 3D vector (in Gauss)
   void convertMagnetometer(OVR::Vector3f& inMagnetometer, VectorF& outMagnetometer);
}

#define Offset TorqueOffset

#endif   // _OCULUSVRUTIL_H_
