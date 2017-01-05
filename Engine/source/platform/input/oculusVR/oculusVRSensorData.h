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
#include "OVR_CAPI_0_8_0.h"

struct OculusVRSensorData
{
   enum DataDifferences {
      DIFF_NONE            = 0,
      DIFF_ROT             = (1<<0),
      DIFF_ROTAXISX        = (1<<1),
      DIFF_ROTAXISY        = (1<<2),
      DIFF_ACCEL           = (1<<3),
      DIFF_ANGVEL          = (1<<4),
      DIFF_MAG             = (1<<5),
      DIFF_POS             = (1<<6),
      DIFF_STATUS          = (1<<7),

      DIFF_ROTAXIS = (DIFF_ROTAXISX | DIFF_ROTAXISY),
      DIFF_RAW = (DIFF_ACCEL | DIFF_ANGVEL | DIFF_MAG),
   };

   bool mDataSet;

   // Position
   Point3F mPosition;

   // Rotation
   MatrixF mRot;
   QuatF   mRotQuat;
   EulerF  mRotEuler;

   // Controller rotation as axis x, y
   Point2F mRotAxis;

   // Raw values
   VectorF mAcceleration;
   EulerF  mAngVelocity;
   VectorF mMagnetometer;

   U32 mStatusFlags;

   OculusVRSensorData();

   /// Reset the data
   void reset();

   /// Set data based on given sensor fusion
   void setData(ovrTrackingState& data, const F32& maxAxisRadius);

   /// Compare this data and given and return differences
   U32 compare(OculusVRSensorData* other, bool doRawCompare);
};

#endif   // _OCULUSVRSENSORDATA_H_
