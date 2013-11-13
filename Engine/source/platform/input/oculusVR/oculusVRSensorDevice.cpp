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

U32 OculusVRSensorDevice::OVR_SENSORROT[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTANG[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTAXISX[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORROTAXISY[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORACCELERATION[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORANGVEL[OculusVRConstants::MaxSensors] = {0};
U32 OculusVRSensorDevice::OVR_SENSORMAGNETOMETER[OculusVRConstants::MaxSensors] = {0};

OculusVRSensorDevice::OculusVRSensorDevice()
{
   mIsValid = false;
   mIsSimulation = false;
   mDevice = NULL;

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
   mSensorFusion.AttachToSensor(NULL);

   if(mDevice)
   {
      mDevice->Release();
      mDevice = NULL;
   }

   mIsValid = false;
}

void OculusVRSensorDevice::set(OVR::SensorDevice* sensor, OVR::SensorInfo& info, S32 actionCodeIndex)
{
   mIsValid = false;

   mDevice = sensor;
   mSensorFusion.AttachToSensor(sensor);
   mYawCorrectionDisabled = !mSensorFusion.IsYawCorrectionEnabled();

   // DeviceInfo
   mProductName = info.ProductName;
   mManufacturer = info.Manufacturer;
   mVersion = info.Version;

   // SensorInfo
   mVendorId = info.VendorId;
   mProductId = info.ProductId;
   mSerialNumber = info.SerialNumber;

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
}

void OculusVRSensorDevice::createSimulation(SimulationTypes simulationType, S32 actionCodeIndex)
{
   if(simulationType == ST_RIFT_PREVIEW)
   {
      createSimulatedPreviewRift(actionCodeIndex);
   }
}

void OculusVRSensorDevice::createSimulatedPreviewRift(S32 actionCodeIndex)
{
   mIsValid = false;
   mIsSimulation = true;
   mYawCorrectionDisabled = true;

   // DeviceInfo
   mProductName = "Tracker DK";
   mManufacturer = "Oculus VR, Inc.";
   mVersion = 0;

   // SensorInfo
   mVendorId = 10291;
   mProductId = 1;
   mSerialNumber = "000000000000";

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
   }
}

//-----------------------------------------------------------------------------

bool OculusVRSensorDevice::process(U32 deviceType, bool generateRotAsAngAxis, bool generateRotAsEuler, bool generateRotationAsAxisEvents, F32 maxAxisRadius, bool generateRawSensor)
{
   if(!mIsValid)
      return false;

   // Store the current data from the sensor and compare with previous data
   U32 diff;
   OculusVRSensorData* currentBuffer = (mPrevData == mDataBuffer[0]) ? mDataBuffer[1] : mDataBuffer[0];
   if(!mIsSimulation)
   {
      currentBuffer->setData(mSensorFusion, maxAxisRadius);
   }
   else
   {
      currentBuffer->simulateData(maxAxisRadius);
   }
   diff = mPrevData->compare(currentBuffer, generateRawSensor);

   // Update the previous data pointer.  We do this here in case someone calls our
   // console functions during one of the input events below.
   mPrevData = currentBuffer;

   // Rotation event
   if(diff & OculusVRSensorData::DIFF_ROT)
   {
      if(generateRotAsAngAxis)
      {
         INPUTMGR->buildInputEvent(deviceType, OculusVRConstants::DefaultOVRBase, SI_ROT, OVR_SENSORROT[mActionCodeIndex], SI_MOVE, currentBuffer->mRotQuat);
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

   return true;
}

//-----------------------------------------------------------------------------

void OculusVRSensorDevice::reset()
{
   if(!mIsValid)
      return;

   mSensorFusion.Reset();
}

F32 OculusVRSensorDevice::getPredictionTime() const
{
   if(!mIsValid)
      return 0.0f;

   return mSensorFusion.GetPredictionDelta();
}

void OculusVRSensorDevice::setPredictionTime(F32 dt)
{
   if(!mIsValid)
      return;

   mSensorFusion.SetPrediction(dt);
}

bool OculusVRSensorDevice::getGravityCorrection() const
{
   if(!mIsValid)
      return false;

   return mSensorFusion.IsGravityEnabled();
}

void OculusVRSensorDevice::setGravityCorrection(bool state)
{
   if(!mIsValid)
      return;

   mSensorFusion.SetGravityEnabled(state);
}

bool OculusVRSensorDevice::getYawCorrection() const
{
   if(!mIsValid)
      return false;

   return mSensorFusion.IsYawCorrectionEnabled();
}

void OculusVRSensorDevice::setYawCorrection(bool state)
{
   if(!mIsValid)
      return;

   if(mYawCorrectionDisabled || !mSensorFusion.HasMagCalibration())
      return;

   mSensorFusion.SetYawCorrectionEnabled(state);
}

bool OculusVRSensorDevice::getMagnetometerCalibrationAvailable() const
{
   if(!mIsValid)
      return false;

   return mSensorFusion.HasMagCalibration();
}

//-----------------------------------------------------------------------------

EulerF OculusVRSensorDevice::getEulerRotation()
{
   if(!mIsValid)
      return Point3F::Zero;

   OVR::Quatf orientation;
   if(mSensorFusion.GetPredictionDelta() > 0)
   {
      orientation = mSensorFusion.GetPredictedOrientation();
   }
   else
   {
      orientation = mSensorFusion.GetOrientation();
   }

   // Sensor rotation in Euler format
   EulerF rot;
   OculusVRUtil::convertRotation(orientation, rot);

   return rot;
}

EulerF OculusVRSensorDevice::getRawEulerRotation()
{
   if(!mIsValid)
      return Point3F::Zero;

   OVR::Quatf orientation;
   orientation = mSensorFusion.GetOrientation();

   // Sensor rotation in Euler format
   EulerF rot;
   OculusVRUtil::convertRotation(orientation, rot);

   return rot;
}

VectorF OculusVRSensorDevice::getAcceleration()
{
   if(!mIsValid)
      return VectorF::Zero;

   OVR::Vector3f a = mSensorFusion.GetAcceleration();
   
   // Sensor acceleration in VectorF format
   VectorF acceleration;
   OculusVRUtil::convertAcceleration(a, acceleration);

   return acceleration;
}

EulerF OculusVRSensorDevice::getAngularVelocity()
{
   if(!mIsValid)
      return EulerF::Zero;

   OVR::Vector3f v = mSensorFusion.GetAngularVelocity();
   
   // Sensor angular velocity in EulerF format
   EulerF vel;
   OculusVRUtil::convertAngularVelocity(v, vel);

   return vel;
}

VectorF OculusVRSensorDevice::getMagnetometer()
{
   if(!mIsValid)
      return VectorF::Zero;

   OVR::Vector3f m;
   if(mSensorFusion.HasMagCalibration() && mSensorFusion.IsYawCorrectionEnabled())
   {
      m = mSensorFusion.GetCalibratedMagnetometer();
   }
   else
   {
      m = mSensorFusion.GetMagnetometer();
   }
   
   // Sensor magnetometer reading in VectorF format
   VectorF mag;
   OculusVRUtil::convertMagnetometer(m, mag);

   return mag;
}

VectorF OculusVRSensorDevice::getRawMagnetometer()
{
   if(!mIsValid)
      return VectorF::Zero;

   OVR::Vector3f m = mSensorFusion.GetMagnetometer();
   
   // Sensor magnetometer reading in VectorF format
   VectorF mag;
   OculusVRUtil::convertMagnetometer(m, mag);

   return mag;
}
