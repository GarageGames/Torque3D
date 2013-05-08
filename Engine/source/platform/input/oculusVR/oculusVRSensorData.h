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

#ifndef _OCULUSVRSENSORDATA_H_
#define _OCULUSVRSENSORDATA_H_

#include "platform/types.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "math/mPoint2.h"
#include "OVR.h"

struct OculusVRSensorData
{
   enum DataDifferences {
      DIFF_NONE            = 0,
      DIFF_ROT             = (1<<0),
      DIFF_ROTAXISX        = (1<<1),
      DIFF_ROTAXISY        = (1<<2),

      DIFF_ROTAXIS = (DIFF_ROTAXISX | DIFF_ROTAXISY),
   };

   bool mDataSet;

   // Rotation
   MatrixF mRot;
   QuatF   mRotQuat;
   EulerF  mRotEuler;

   // Controller rotation as axis x, y
   Point2F mRotAxis;

   OculusVRSensorData();

   /// Reset the data
   void reset();

   /// Set data based on given sensor fusion
   void setData(const OVR::SensorFusion& data, const F32& maxAxisRadius);

   /// Simulate valid data
   void simulateData(const F32& maxAxisRadius);

   /// Compare this data and given and return differences
   U32 compare(OculusVRSensorData* other);
};

#endif   // _OCULUSVRSENSORDATA_H_
