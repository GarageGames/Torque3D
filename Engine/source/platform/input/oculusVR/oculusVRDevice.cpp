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

#include "platform/input/oculusVR/oculusVRDevice.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"

MODULE_BEGIN( OculusVRDevice )

   MODULE_INIT_AFTER( InputEventManager )
   MODULE_SHUTDOWN_BEFORE( InputEventManager )

   MODULE_INIT
   {
      OculusVRDevice::staticInit();
      ManagedSingleton< OculusVRDevice >::createSingleton();
      if(OculusVRDevice::smEnableDevice)
      {
         OCULUSVRDEV->enable();
      }

      // Register the device with the Input Event Manager
      INPUTMGR->registerDevice(OCULUSVRDEV);
   }
   
   MODULE_SHUTDOWN
   {
      INPUTMGR->unregisterDevice(OCULUSVRDEV);
      ManagedSingleton< OculusVRDevice >::deleteSingleton();
   }

MODULE_END;

//-----------------------------------------------------------------------------
// OculusVRDevice
//-----------------------------------------------------------------------------

bool OculusVRDevice::smEnableDevice = true;

bool OculusVRDevice::smSimulateHMD = true;

bool OculusVRDevice::smGenerateAngleAxisRotationEvents = true;
bool OculusVRDevice::smGenerateEulerRotationEvents = false;

bool OculusVRDevice::smGenerateRotationAsAxisEvents = false;
F32 OculusVRDevice::smMaximumAxisAngle = 25.0f;

bool OculusVRDevice::smGenerateWholeFrameEvents = false;

OculusVRDevice::OculusVRDevice()
{
   // From IInputDevice
   dStrcpy(mName, "oculusvr");
   mDeviceType = INPUTMGR->getNextDeviceType();

   //
   mEnabled = false;
   mActive = false;

   // We don't current support scaling of the input texture.  The graphics pipeline will
   // need to be modified for this.
   mScaleInputTexture = false;

   mDeviceManager = NULL;
   mListener = NULL;

   buildCodeTable();
}

OculusVRDevice::~OculusVRDevice()
{
   cleanUp();
}

void OculusVRDevice::staticInit()
{
   Con::addVariable("pref::OculusVR::EnableDevice", TypeBool, &smEnableDevice, 
      "@brief If true, the Oculus VR device will be enabled, if present.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateAngleAxisRotationEvents", TypeBool, &smGenerateAngleAxisRotationEvents, 
      "@brief If true, broadcast sensor rotation events as angled axis.\n\n"
	   "@ingroup Game");
   Con::addVariable("OculusVR::GenerateEulerRotationEvents", TypeBool, &smGenerateEulerRotationEvents, 
      "@brief If true, broadcast sensor rotation events as Euler angles about the X, Y and Z axis.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateRotationAsAxisEvents", TypeBool, &smGenerateRotationAsAxisEvents, 
      "@brief If true, broadcast sensor rotation as axis events.\n\n"
	   "@ingroup Game");
   Con::addVariable("OculusVR::MaximumAxisAngle", TypeF32, &smMaximumAxisAngle, 
      "@brief The maximum sensor angle when used as an axis event as measured from a vector pointing straight up (in degrees).\n\n"
      "Should range from 0 to 90 degrees.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateWholeFrameEvents", TypeBool, &smGenerateWholeFrameEvents, 
      "@brief Indicates that a whole frame event should be generated and frames should be buffered.\n\n"
	   "@ingroup Game");
}

void OculusVRDevice::cleanUp()
{
   disable();
}

void OculusVRDevice::buildCodeTable()
{
   // Build the sensor device code table
   OculusVRSensorDevice::buildCodeTable();
}

void OculusVRDevice::addHMDDevice(OVR::HMDDevice* hmd)
{
   if(!hmd)
      return;

   OVR::HMDInfo hmdInfo;
   if(!hmd->GetDeviceInfo(&hmdInfo))
      return;

   OculusVRHMDDevice* hmdd = new OculusVRHMDDevice();
   hmdd->set(hmd, hmdInfo, mScaleInputTexture);
   mHMDDevices.push_back(hmdd);

   Con::printf("   HMD found: %s by %s [v%d]", hmdInfo.ProductName, hmdInfo.Manufacturer, hmdInfo.Version);
}

void OculusVRDevice::createSimulatedHMD()
{
   OculusVRHMDDevice* hmdd = new OculusVRHMDDevice();
   hmdd->createSimulation(OculusVRHMDDevice::ST_RIFT_PREVIEW, mScaleInputTexture);
   mHMDDevices.push_back(hmdd);

   Con::printf("   HMD simulated: %s by %s [v%d]", hmdd->getProductName(), hmdd->getManufacturer(), hmdd->getVersion());
}

void OculusVRDevice::addSensorDevice(OVR::SensorDevice* sensor)
{
   if(!sensor)
      return;

   OVR::SensorInfo sensorInfo;
   if(!sensor->GetDeviceInfo(&sensorInfo))
      return;

   OculusVRSensorDevice* sensord = new OculusVRSensorDevice();
   sensord->set(sensor, sensorInfo, mSensorDevices.size());
   mSensorDevices.push_back(sensord);

   Con::printf("   Sensor found: %s by %s [v%d] %s", sensorInfo.ProductName, sensorInfo.Manufacturer, sensorInfo.Version, sensorInfo.SerialNumber);
}

void OculusVRDevice::createSimulatedSensor()
{
   OculusVRSensorDevice* sensord = new OculusVRSensorDevice();
   sensord->createSimulation(OculusVRSensorDevice::ST_RIFT_PREVIEW, mSensorDevices.size());
   mSensorDevices.push_back(sensord);

   Con::printf("   Sensor simulated: %s by %s [v%d] %s", sensord->getProductName(), sensord->getManufacturer(), sensord->getVersion(), sensord->getSerialNumber());
}

bool OculusVRDevice::enable()
{
   // Start off with disabling the device if it is already enabled
   disable();

   Con::printf("Oculus VR Device Init:");

   OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
   if(OVR::System::IsInitialized())
   {
      mEnabled = true;

      // Create the OVR device manager
      mDeviceManager = OVR::DeviceManager::Create();
      if(!mDeviceManager)
      {
         if(smSimulateHMD)
         {
            Con::printf("   Could not create a HMD device manager.  Simulating a HMD.");
            Con::printf("   ");

            createSimulatedHMD();
            createSimulatedSensor();
            setActive(true);
            return true;
         }
         else
         {
            Con::printf("   Could not create a HMD device manager.");
            Con::printf("   ");

            mEnabled = false;
            OVR::System::Destroy();
            return false;
         }
      }

      // Provide a message listener
      // NOTE: Commented out as non-functional in 0.1.2
      //mListener = new DeviceListener(this);
      //mDeviceManager->SetMessageHandler(mListener);

      // Enumerate HMDs and pick the first one
      OVR::HMDDevice* hmd = mDeviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
      if(hmd)
      {
         // Add the HMD to our list
         addHMDDevice(hmd);

         // Detect and add any sensor on the HMD
         OVR::SensorDevice* sensor = hmd->GetSensor();
         if(sensor)
         {
            addSensorDevice(sensor);
         }
         else
         {
            Con::printf("   No sensor device on HMD.");
         }

         setActive(true);
      }
      else
      {
         if(smSimulateHMD)
         {
            Con::printf("   Could not enumerate a HMD device.  Simulating a HMD.");
            createSimulatedHMD();
            createSimulatedSensor();
            setActive(true);
         }
         else
         {
            Con::printf("   Could not enumerate a HMD device.");
         }
      }

   }

   Con::printf("   ");

   return false;
}

void OculusVRDevice::disable()
{
   for(U32 i=0; i<mSensorDevices.size(); ++i)
   {
      delete mSensorDevices[i];
   }
   mSensorDevices.clear();

   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      delete mHMDDevices[i];
   }
   mHMDDevices.clear();

   if(mDeviceManager)
   {
      mDeviceManager->Release();
      mDeviceManager = NULL;
   }

   if(mEnabled)
   {
      OVR::System::Destroy();
   }

   if(mListener)
   {
      delete mListener;
      mListener = NULL;
   }

   setActive(false);
   mEnabled = false;
}

bool OculusVRDevice::process()
{
   if(!mEnabled)
      return false;

   if(!getActive())
      return false;

   //Build the maximum axis angle to be passed into the sensor process()
   F32 maxAxisRadius = mSin(mDegToRad(smMaximumAxisAngle));

   // Process each sensor
   for(U32 i=0; i<mSensorDevices.size(); ++i)
   {
      mSensorDevices[i]->process(mDeviceType, smGenerateAngleAxisRotationEvents, smGenerateEulerRotationEvents, smGenerateRotationAsAxisEvents, maxAxisRadius);
   }

   return true;
}

//-----------------------------------------------------------------------------

bool OculusVRDevice::providesYFOV() const
{
   if(!mHMDDevices.size())
      return false;

   return true;
}

F32 OculusVRDevice::getYFOV() const
{
   if(!mHMDDevices.size())
      return 0.0f;

   const OculusVRHMDDevice* hmd = getHMDDevice(0);
   if(!hmd)
      return 0.0f;

   return hmd->getYFOV();
}

bool OculusVRDevice::providesEyeOffset() const
{
   if(!mHMDDevices.size())
      return false;

   return true;
}

const Point3F& OculusVRDevice::getEyeOffset() const
{
   if(!mHMDDevices.size())
      return Point3F::Zero;

   const OculusVRHMDDevice* hmd = getHMDDevice(0);
   if(!hmd)
      return Point3F::Zero;

   return hmd->getEyeWorldOffset();
}

bool OculusVRDevice::providesProjectionOffset() const
{
   if(!mHMDDevices.size())
      return false;

   return true;
}

const Point2F& OculusVRDevice::getProjectionOffset() const
{
   if(!mHMDDevices.size())
      return Point2F::Zero;

   const OculusVRHMDDevice* hmd = getHMDDevice(0);
   if(!hmd)
      return Point2F::Zero;

   return hmd->getProjectionCenterOffset();
}

//-----------------------------------------------------------------------------

const OculusVRHMDDevice* OculusVRDevice::getHMDDevice(U32 index) const
{
   if(index >= mHMDDevices.size())
      return NULL;

   return mHMDDevices[index];
}

//-----------------------------------------------------------------------------

const OculusVRSensorDevice* OculusVRDevice::getSensorDevice(U32 index) const
{
   if(index >= mSensorDevices.size())
      return NULL;

   return mSensorDevices[index];
}

EulerF OculusVRDevice::getSensorEulerRotation(U32 index)
{
   if(index >= mSensorDevices.size())
      return Point3F::Zero;

   return mSensorDevices[index]->getEulerRotation();
}

F32 OculusVRDevice::getSensorPredictionTime(U32 index)
{
   const OculusVRSensorDevice* sensor = getSensorDevice(index);
   if(!sensor || !sensor->isValid())
      return 0.0f;

   return sensor->getPredictionTime();
}

void OculusVRDevice::setSensorPredictionTime(U32 index, F32 dt)
{
   if(index >= mSensorDevices.size())
      return;

   OculusVRSensorDevice* sensor = mSensorDevices[index];
   if(!sensor->isValid())
      return;

   sensor->setPredictionTime(dt);
}

void OculusVRDevice::setAllSensorPredictionTime(F32 dt)
{
   for(U32 i=0; i<mSensorDevices.size(); ++i)
   {
      mSensorDevices[i]->setPredictionTime(dt);
   }
}

void OculusVRDevice::resetAllSensors()
{
   // Reset each sensor
   for(U32 i=0; i<mSensorDevices.size(); ++i)
   {
      mSensorDevices[i]->reset();
   }
}

//-----------------------------------------------------------------------------

void OculusVRDevice::DeviceListener::OnMessage(const OVR::Message& msg)
{
   switch(msg.Type)
   {
      case OVR::Message_DeviceAdded:
         {
            const OVR::MessageDeviceStatus* status = static_cast<const OVR::MessageDeviceStatus*>(&msg);
            Con::printf("OVR: Device added of type: %d", status->Handle.GetType());
         }
         break;

      case OVR::Message_DeviceRemoved:
         Con::printf("OVR: Device removed of type: %d", msg.pDevice->GetType());
         break;

      default:
         break;
   }
}

//-----------------------------------------------------------------------------

DefineEngineFunction(isOculusVRDeviceActive, bool, (),,
   "@brief Used to determine if the Oculus VR input device is active\n\n"

   "The Oculus VR device is considered active when the library has been "
   "initialized and either a real of simulated HMD is present.\n\n"

   "@return True if the Oculus VR input device is active.\n"

   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return false;
   }

   return OCULUSVRDEV->getActive();
}

//-----------------------------------------------------------------------------

DefineEngineFunction(setOVRHMDAsGameConnectionDisplayDevice, bool, (GameConnection* conn),,
   "@brief Sets the first HMD to be a GameConnection's display device\n\n"
   "@param conn The GameConnection to set.\n"
   "@return True if the GameConnection display device was set.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      Con::errorf("setOVRHMDAsGameConnectionDisplayDevice(): No Oculus VR Device present.");
      return false;
   }

   if(!conn)
   {
      Con::errorf("setOVRHMDAsGameConnectionDisplayDevice(): Invalid GameConnection.");
      return false;
   }

   conn->setDisplayDevice(OCULUSVRDEV);
   return true;
}

//-----------------------------------------------------------------------------

DefineEngineFunction(getOVRHMDCount, S32, (),,
   "@brief Get the number of HMD devices that are currently connected.\n\n"
   "@return The number of Oculus VR HMD devices that are currently connected.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 0;
   }

   return OCULUSVRDEV->getHMDCount();
}

DefineEngineFunction(isOVRHMDSimulated, bool, (S32 index),,
   "@brief Determines if the requested OVR HMD is simulated or real.\n\n"
   "@param index The HMD index.\n"
   "@return True if the HMD is simulated.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return true;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return true;
   }

   return hmd->isSimulated();
}

DefineEngineFunction(getOVRHMDProductName, const char*, (S32 index),,
   "@brief Retrieves the HMD product name.\n\n"
   "@param index The HMD index.\n"
   "@return The name of the HMD product.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return "";
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return "";
   }

   return hmd->getProductName();
}

DefineEngineFunction(getOVRHMDManufacturer, const char*, (S32 index),,
   "@brief Retrieves the HMD manufacturer name.\n\n"
   "@param index The HMD index.\n"
   "@return The manufacturer of the HMD product.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return "";
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return "";
   }

   return hmd->getManufacturer();
}

DefineEngineFunction(getOVRHMDVersion, S32, (S32 index),,
   "@brief Retrieves the HMD version number.\n\n"
   "@param index The HMD index.\n"
   "@return The version number of the HMD product.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return -1;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return -1;
   }

   return hmd->getVersion();
}

DefineEngineFunction(getOVRHMDDisplayDeviceName, const char*, (S32 index),,
   "@brief Windows display device name used in EnumDisplaySettings/CreateDC.\n\n"
   "@param index The HMD index.\n"
   "@return The name of the HMD display device, if any.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return "";
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return "";
   }

   return hmd->getDisplayDeviceName();
}

DefineEngineFunction(getOVRHMDResolution, Point2I, (S32 index),,
   "@brief Provides the OVR HMD screen resolution.\n\n"
   "@param index The HMD index.\n"
   "@return A two component string with the screen's resolution.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return Point2I(1280, 800);
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return Point2I(1280, 800);
   }

   return hmd->getResolution();
}

DefineEngineFunction(getOVRHMDDistortionCoefficients, String, (S32 index),,
   "@brief Provides the OVR HMD distortion coefficients.\n\n"
   "@param index The HMD index.\n"
   "@return A four component string with the distortion coefficients.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return "0 0 0 0";
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return "0 0 0 0";
   }

   const Point4F& k = hmd->getKDistortion();
   char buf[256];
   dSprintf(buf, 256, "%g %g %g %g", k.x, k.y, k.z, k.w);

   return buf;
}

DefineEngineFunction(getOVRHMDEyeXOffsets, Point2F, (S32 index),,
   "@brief Provides the OVR HMD eye x offsets in uv coordinates.\n\n"
   "@param index The HMD index.\n"
   "@return A two component string with the left and right eye x offsets.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return Point2F(0.5, 0.5);
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return Point2F(0.5, 0.5);
   }

   // X component is left, Y component is right
   const Point2F& offset = hmd->getEyeUVOffset();
   return offset;
}

DefineEngineFunction(getOVRHMDXCenterOffset, F32, (S32 index),,
   "@brief Provides the OVR HMD calculated XCenterOffset.\n\n"
   "@param index The HMD index.\n"
   "@return The calculated XCenterOffset.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 0.0f;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return 0.0f;
   }

   F32 offset = hmd->getCenterOffset();
   return offset;
}

DefineEngineFunction(getOVRHMDDistortionScale, F32, (S32 index),,
   "@brief Provides the OVR HMD calculated distortion scale.\n\n"
   "@param index The HMD index.\n"
   "@return The calculated distortion scale.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 1.0f;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return 1.0f;
   }

   F32 scale = hmd->getDistortionScale();
   return scale;
}

DefineEngineFunction(getOVRHMDYFOV, F32, (S32 index),,
   "@brief Provides the OVR HMD calculated Y FOV.\n\n"
   "@param index The HMD index.\n"
   "@return The calculated Y FOV.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 1.0f;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return 1.0f;
   }

   F32 fov = hmd->getYFOV();
   return mRadToDeg(fov);
}

//-----------------------------------------------------------------------------

DefineEngineFunction(getOVRSensorCount, S32, (),,
   "@brief Get the number of sensor devices that are currently connected.\n\n"
   "@return The number of Oculus VR sensor devices that are currently connected.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 0;
   }

   return OCULUSVRDEV->getSensorCount();
}

DefineEngineFunction(getOVRSensorEulerRotation, Point3F, (S32 index),,
   "@brief Get the Euler rotation values for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return The Euler rotation values of the Oculus VR sensor, in degrees.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return Point3F::Zero;
   }

   EulerF rot = OCULUSVRDEV->getSensorEulerRotation(index);
   return Point3F(mRadToDeg(rot.x), mRadToDeg(rot.y), mRadToDeg(rot.z));
}

DefineEngineFunction(getOVRSensorPredictionTime, F32, (S32 index),,
   "@brief Get the prediction time set for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return The prediction time of the Oculus VR sensor, given in seconds.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return 0;
   }

   return OCULUSVRDEV->getSensorPredictionTime(index);
}

DefineEngineFunction(setSensorPredictionTime, void, (S32 index, F32 dt),,
   "@brief Set the prediction time set for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@param dt The prediction time to set given in seconds.  Setting to 0 disables prediction.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->setSensorPredictionTime(index, dt);
}

DefineEngineFunction(setAllSensorPredictionTime, void, (F32 dt),,
   "@brief Set the prediction time set for all sensors.\n\n"
   "@param dt The prediction time to set given in seconds.  Setting to 0 disables prediction.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->setAllSensorPredictionTime(dt);
}

DefineEngineFunction(ovrResetAllSensors, void, (),,
   "@brief Resets all Oculus VR sensors.\n\n"
   "This resets all sensor orientations such that their 'normal' rotation "
   "is defined when this function is called.  This defines an HMD's forwards "
   "and up direction, for example."
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->resetAllSensors();
}
