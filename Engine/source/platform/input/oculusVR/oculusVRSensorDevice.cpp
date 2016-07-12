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

#include "platform/input/oculusVR/oculusVRSensorDevice.h"
#include "platform/input/oculusVR/oculusVRSensorData.h"
#include "platform/input/oculusVR/oculusVRUtil.h"
#include "platform/platformInput.h"
#include "console/simBase.h"
#include "console/engineAPI.h" 
#include "math/mAngAxis.h"
#include "OVR_CAPI_0_8_0.h"

U32 OculusVRSensorDevice::OVR_SENSORROT[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTANG[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTAXISX[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTAXISY[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORACCELERATION[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORANGVEL[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORMAGNETOMETER[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORPOSITION[OculusVRConstants::MaxSensors] = {0};

OculusVRSensorDevice::OculusVRSensorDevice()
{
   mIsValid = false;
   mDevice = NULL;
   mCurrentTrackingCaps = 0;
   mSupportedTrackingCaps = 0;
   mPositionTrackingDisabled = false;
   for(U32 i=0; i<2; ++i)
   {
      mDataBuffer[i] = new OculusVRSensorData();
   }
   mPrevData = mDataBuffer[0];
}

OculusVRSensorDevice::~OculusVRSensorDevice()
{
   cleanUp();

   for(U32 i=0; i<2; ++i)
   {
      delete mDataBuffer[i];
      mDataBuffer[i] = NULL;
   }
   mPrevData = NULL;
}

void OculusVRSensorDevice::cleanUp()
{
   mIsValid = false;

   ovr_ConfigureTracking(mDevice, 0, 0);
}

void OculusVRSensorDevice::set(ovrHmd sensor, S32 actionCodeIndex)
{
   mIsValid = false;
   mDevice = sensor;

   mSupportedTrackingCaps = ovr_GetTrackingCaps(sensor);
   mCurrentTrackingCaps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position;

   mCurrentTrackingCaps = mSupportedTrackingCaps & mCurrentTrackingCaps;
   mYawCorrectionDisabled = !(mCurrentTrackingCaps & ovrTrackingCap_MagYawCorrection);

   mPositionTrackingDisabled = !(mCurrentTrackingCaps & ovrTrackingCap_Position);

	ovrHmdDesc desc = ovr_GetHmdDesc(sensor);

   // DeviceInfo
   mProductName = desc.ProductName;
   mManufacturer = desc.Manufacturer;
   mVersion = desc.Type;

   // SensorInfo
   mVendorId = desc.VendorId;
   mProductId = desc.ProductId;
   mSerialNumber = desc.SerialNumber;

   mActionCodeIndex = actionCodeIndex;

   if(mActionCodeIndex >= OculusVRConstants::MaxSensors)
   {
      // Cannot declare more sensors than we are able to handle
      mIsValid = false;
   }
   else
   {
      mIsValid = true;
   }

   updateTrackingCaps();
}

void OculusVRSensorDevice::buildCodeTable()
{
   // Obtain all of the device codes
   for(U32 i=0; i<OculusVRConstants::MaxSensors; ++i)
   {
      OVR_SENSORROT[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORROTANG[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORROTAXISX[i] = INPUTMGR->getNextDeviceCode();
      OVR_SENSORROTAXISY[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORACCELERATION[i] = INPUTMGR->getNextDeviceCode();
      OVR_SENSORANGVEL[i] = INPUTMGR->getNextDeviceCode();
      OVR_SENSORMAGNETOMETER[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORPOSITION[i] = INPUTMGR->getNextDeviceCode();
   }

   // Build out the virtual map
   char buffer[64];
   for(U32 i=0; i<OculusVRConstants::MaxSensors; ++i)
   {
      dSprintf(buffer, 64, "ovr_sensorrot%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_ROT, OVR_SENSORROT[i] );

      dSprintf(buffer, 64, "ovr_sensorrotang%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, OVR_SENSORROTANG[i] );

      dSprintf(buffer, 64, "ovr_sensorrotaxisx%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, OVR_SENSORROTAXISX[i] );
      dSprintf(buffer, 64, "ovr_sensorrotaxisy%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_AXIS, OVR_SENSORROTAXISY[i] );

      dSprintf(buffer, 64, "ovr_sensoracceleration%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, OVR_SENSORACCELERATION[i] );

      dSprintf(buffer, 64, "ovr_sensorangvel%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, OVR_SENSORANGVEL[i] );

      dSprintf(buffer, 64, "ovr_sensormagnetometer%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, OVR_SENSORMAGNETOMETER[i] );

      dSprintf(buffer, 64, "ovr_sensorpos%d", i);
      INPUTMGR->addVirtualMap( buffer, SI_POS, OVR_SENSORPOSITION[i] );
   }
}

//-----------------------------------------------------------------------------

bool OculusVRSensorDevice::process(U32 deviceType, bool generateRotAsAngAxis, bool generateRotAsEuler, bool generateRotationAsAxisEvents, bool generatePositionEvents, F32 maxAxisRadius, bool generateRawSensor)
{
   if(!mIsValid)
      return false;

   // Grab current state
   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   mLastStatus = ts.StatusFlags;

   // Store the current data from the sensor and compare with previous data
   U32 diff;
   OculusVRSensorData* currentBuffer = (mPrevData == mDataBuffer[0]) ? mDataBuffer[1] : mDataBuffer[0];
   currentBuffer->setData(ts, maxAxisRadius);
   diff = mPrevData->compare(currentBuffer, generateRawSensor);

   // Update the previous data pointer.  We do this here in case someone calls our
   // console functions during one of the input events below.
   mPrevData = currentBuffer;

   // Rotation event
   if(diff & OculusVRSensorData::DIFF_ROT)
   {
      if(generateRotAsAngAxis)
      {
         AngAxisF axisAA(currentBuffer->mRotQuat);
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_ROT, OVR_SENSORROT[mActionCodeIndex], SI_MOVE, axisAA);
      }

      if(generateRotAsEuler)
      {
         // Convert angles to degrees
         VectorF angles;
         for(U32 i=0; i<3; ++i)  
         {
            angles[i] = mRadToDeg(currentBuffer->mRotEuler[i]);
         }
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_POS, OVR_SENSORROTANG[mActionCodeIndex], SI_MOVE, angles);
      }
   }

   // Rotation as axis event
   if(generateRotationAsAxisEvents && diff & OculusVRSensorData::DIFF_ROTAXIS)
   {
      if(diff & OculusVRSensorData::DIFF_ROTAXISX)
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_AXIS, OVR_SENSORROTAXISX[mActionCodeIndex], SI_MOVE, currentBuffer->mRotAxis.x);
      if(diff & OculusVRSensorData::DIFF_ROTAXISY)
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_AXIS, OVR_SENSORROTAXISY[mActionCodeIndex], SI_MOVE, currentBuffer->mRotAxis.y);
   }

   if (generatePositionEvents && diff & OculusVRSensorData::DIFF_POS)
   {
      INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_AXIS, OVR_SENSORROTAXISX[mActionCodeIndex], SI_MOVE, currentBuffer->mPosition);
   }

   // Raw sensor event
   if(generateRawSensor && diff & OculusVRSensorData::DIFF_RAW)
   {
      if(diff & OculusVRSensorData::DIFF_ACCEL)
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_POS, OVR_SENSORACCELERATION[mActionCodeIndex], SI_MOVE, currentBuffer->mAcceleration);

      if(diff & OculusVRSensorData::DIFF_ANGVEL)
      {
         // Convert angles to degrees
         VectorF angles;
         for(U32 i=0; i<3; ++i)
         {
            angles[i] = mRadToDeg(currentBuffer->mAngVelocity[i]);
         }
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_POS, OVR_SENSORANGVEL[mActionCodeIndex], SI_MOVE, angles);
      }

      if(diff & OculusVRSensorData::DIFF_MAG)
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_POS, OVR_SENSORMAGNETOMETER[mActionCodeIndex], SI_MOVE, currentBuffer->mMagnetometer);
   }

   if (diff & OculusVRSensorData::DIFF_STATUS)
   {
      if (Con::isFunction("onOculusStatusUpdate"))
      {
         Con::executef("onOculusStatusUpdate", ts.StatusFlags);
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

void OculusVRSensorDevice::reset()
{
   if(!mIsValid)
      return;

   ovr_RecenterPose(mDevice);
}

bool OculusVRSensorDevice::getYawCorrection() const
{
   if(!mIsValid)
      return false;

   return !(mCurrentTrackingCaps & ovrTrackingCap_MagYawCorrection);
}

void OculusVRSensorDevice::setYawCorrection(bool state)
{
   if(!mIsValid)
      return;

   if (state == !mYawCorrectionDisabled)
      return;

   // Don't allow if not capable
   if(state && !(mSupportedTrackingCaps & ovrTrackingCap_MagYawCorrection))
      return;

   mYawCorrectionDisabled = !state;
   updateTrackingCaps();
}

void OculusVRSensorDevice::setPositionTracking(bool state)
{
   if(!mIsValid)
      return;

   if (state == !mPositionTrackingDisabled)
      return;

   if(state && !(mSupportedTrackingCaps & ovrTrackingCap_Position))
      return;
   
   mPositionTrackingDisabled = !state;
   updateTrackingCaps();
}

bool OculusVRSensorDevice::getMagnetometerCalibrationAvailable() const
{
   if(!mIsValid)
      return false;

   return (mSupportedTrackingCaps & ovrTrackingCap_MagYawCorrection) != 0;
}

bool OculusVRSensorDevice::getOrientationTrackingAvailable() const
{
   if(!mIsValid)
      return false;

   return (mSupportedTrackingCaps & ovrTrackingCap_Orientation) != 0;
}

bool OculusVRSensorDevice::getPositionTrackingAvailable() const
{
   if(!mIsValid)
      return false;

   return (mSupportedTrackingCaps & ovrTrackingCap_Position) != 0;
}

//-----------------------------------------------------------------------------

EulerF OculusVRSensorDevice::getEulerRotation()
{
   if(!mIsValid)
      return Point3F::Zero;

   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   OVR::Quatf orientation = ts.HeadPose.ThePose.Orientation;

   // Sensor rotation in Euler format
   EulerF rot;
   OculusVRUtil::convertRotation(orientation, rot);

   return rot;
}

EulerF OculusVRSensorDevice::getRawEulerRotation()
{
   if(!mIsValid)
      return Point3F::Zero;

   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   OVR::Quatf orientation = ts.HeadPose.ThePose.Orientation;

   // Sensor rotation in Euler format
   EulerF rot;
   OculusVRUtil::convertRotation(orientation, rot);
   return rot;
}

VectorF OculusVRSensorDevice::getAcceleration()
{
   if(!mIsValid)
      return VectorF::Zero;
   
   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   OVR::Vector3f a = ts.HeadPose.LinearAcceleration;

   // Sensor acceleration in VectorF format
   VectorF acceleration;
   OculusVRUtil::convertAcceleration(a, acceleration);

   return acceleration;
}

EulerF OculusVRSensorDevice::getAngularVelocity()
{
   if(!mIsValid)
      return EulerF::Zero;
   
   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   OVR::Vector3f v = ts.HeadPose.AngularVelocity;
   
   // Sensor angular velocity in EulerF format
   EulerF vel;
   OculusVRUtil::convertAngularVelocity(v, vel);

   return vel;
}

Point3F OculusVRSensorDevice::getPosition()
{
   if(!mIsValid)
      return Point3F();
   
   ovrTrackingState ts = ovr_GetTrackingState(mDevice, ovr_GetTimeInSeconds(), ovrTrue);
   OVR::Vector3f v = ts.HeadPose.ThePose.Position;
   return Point3F(-v.x, v.z, -v.y);
}

void OculusVRSensorDevice::updateTrackingCaps()
{
   if (!mIsValid)
      return;

   // Set based on current vars
   mCurrentTrackingCaps = ovrTrackingCap_Orientation;

   if (!mYawCorrectionDisabled)
      mCurrentTrackingCaps |= ovrTrackingCap_MagYawCorrection;
   if (!mPositionTrackingDisabled)
      mCurrentTrackingCaps |= ovrTrackingCap_Position;

   ovr_ConfigureTracking(mDevice, mCurrentTrackingCaps, 0);
}
