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

#ifndef _OCULUSVRSENSORDEVICE_H_
#define _OCULUSVRSENSORDEVICE_H_

#include "core/util/str.h"
#include "math/mQuat.h"
#include "math/mPoint2.h"
#include "math/mPoint3.h"
#include "math/mPoint4.h"
#include "platform/input/oculusVR/oculusVRConstants.h"
#include "platform/types.h"
#include "OVR_CAPI.h"

struct OculusVRSensorData;

class OculusVRSensorDevice
{
public:
   // Action codes
   static U32 OVR_SENSORROT[OculusVRConstants::MaxSensors];       // SI_ROT

   static U32 OVR_SENSORROTANG[OculusVRConstants::MaxSensors];    // SI_POS but is EulerF

   static U32 OVR_SENSORROTAXISX[OculusVRConstants::MaxSensors];  // SI_AXIS
   static U32 OVR_SENSORROTAXISY[OculusVRConstants::MaxSensors];

   static U32 OVR_SENSORACCELERATION[OculusVRConstants::MaxSensors];    // SI_POS
   static U32 OVR_SENSORANGVEL[OculusVRConstants::MaxSensors];          // SI_POS but is EulerF
   static U32 OVR_SENSORMAGNETOMETER[OculusVRConstants::MaxSensors];    // SI_POS

   static U32 OVR_SENSORPOSITION[OculusVRConstants::MaxSensors];

protected:
   bool mIsValid;

   ovrHmd mDevice;
   U32 mCurrentTrackingCaps;
   U32 mSupportedTrackingCaps;
   
   // From OVR::DeviceInfo
   String   mProductName;
   String   mManufacturer;
   U32      mVersion;

   // From OVR::SensorInfo
   U16      mVendorId;
   U16      mProductId;
   String   mSerialNumber;

   // Has yaw correction been disabled by the control panel
   bool     mYawCorrectionDisabled;

   // Has position tracking been disabled
   bool     mPositionTrackingDisabled;

   // Last tracking status
   U32 mLastStatus;

   // Assigned by the OculusVRDevice
   S32 mActionCodeIndex;

   // Buffers to store data for sensor
   OculusVRSensorData* mDataBuffer[2];

   // Points to the buffer that holds the previously collected data
   // for the sensor
   OculusVRSensorData* mPrevData;

public:
   OculusVRSensorDevice();
   virtual ~OculusVRSensorDevice();

   static void buildCodeTable();

   void cleanUp();

   // Set the sensor properties based on information from the OVR device
   void set(ovrHmd sensor, S32 actionCodeIndex);

   bool isValid() const {return mIsValid;}

   bool process(U32 deviceType, bool generateRotAsAngAxis, bool generateRotAsEuler, bool generateRotationAsAxisEvents, bool generatePositionEvents, F32 maxAxisRadius, bool generateRawSensor);

   void reset();

   // Has yaw correction been disabled using the control panel
   bool getYawCorrectionUserDisabled() const { return mYawCorrectionDisabled; }

   // Is yaw correction enabled
   bool getYawCorrection() const;

   // Position is valid
   bool getHasValidPosition() const { return mLastStatus & ovrStatus_PositionTracked; }

   // Set the yaw correction. Note: if magnetometer calibration data is not present,
   // or user has disabled yaw correction in the control panel, this method will
   // not enable it.
   void setYawCorrection(bool state);

   // Sets position tracking state
   void setPositionTracking(bool state);

   // Is magnetometer calibration data available for this sensor
   bool getMagnetometerCalibrationAvailable() const;

   // Is position tracking data available for this sensor
   bool getOrientationTrackingAvailable() const;

   // Is position tracking data available for this sensor
   bool getPositionTrackingAvailable() const;

   U32 getLastTrackingStatus() const { return mLastStatus; }

   const char* getProductName() { return mProductName.c_str(); }
   const char* getManufacturer() { return mManufacturer.c_str(); }
   U32 getVersion() { return mVersion; }
   U16 getVendorId() { return mVendorId; }
   U16 getProductId() { return mProductId; }
   const char* getSerialNumber() { return mSerialNumber; }

   // Get the current rotation of the sensor.  Uses prediction if set.
   EulerF getEulerRotation();

   // Get the current rotation of the sensor.
   EulerF getRawEulerRotation();

   // Get the current absolute acceleration reading, in m/s^2
   VectorF getAcceleration();

   // Get the current angular velocity reading, in rad/s
   EulerF getAngularVelocity();

   // Get the current position
   Point3F getPosition();

   void updateTrackingCaps();
};

#endif   // _OCULUSVRSENSORDEVICE_H_
