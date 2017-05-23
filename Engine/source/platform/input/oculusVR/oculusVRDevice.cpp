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
#include "platform/input/oculusVR/oculusVRSensorDevice.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/core/guiCanvas.h"

bool sOcculusEnabled = false;

MODULE_BEGIN( OculusVRDevice )

   MODULE_INIT_AFTER( InputEventManager )
   MODULE_SHUTDOWN_BEFORE( InputEventManager )

   MODULE_INIT
   {
      sOcculusEnabled = true; // TODO: init the dll
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
      sOcculusEnabled = false;
   }

MODULE_END;

//-----------------------------------------------------------------------------
// OculusVRDevice
//-----------------------------------------------------------------------------

bool OculusVRDevice::smEnableDevice = false;

bool OculusVRDevice::smSimulateHMD = true;

bool OculusVRDevice::smGenerateAngleAxisRotationEvents = true;
bool OculusVRDevice::smGenerateEulerRotationEvents = false;
bool OculusVRDevice::smGeneratePositionEvents = true;

bool OculusVRDevice::smGenerateRotationAsAxisEvents = false;
F32 OculusVRDevice::smMaximumAxisAngle = 25.0f;

bool OculusVRDevice::smGenerateSensorRawEvents = false;

bool OculusVRDevice::smGenerateWholeFrameEvents = false;

F32 OculusVRDevice::smDesiredPixelDensity = 1.0f;

bool OculusVRDevice::smWindowDebug = false;

F32 OculusVRDevice::smPositionTrackingScale = 1.0f;

OculusVRDevice::OculusVRDevice()
{
   // From IInputDevice
   dStrcpy(mName, "oculusvr");
   mDeviceType = INPUTMGR->getNextDeviceType();

   //
   mEnabled = false;
   mActive = false;
   mActiveDeviceId = 0;

   buildCodeTable();
   GFXDevice::getDeviceEventSignal().notify( this, &OculusVRDevice::_handleDeviceEvent );
}

OculusVRDevice::~OculusVRDevice()
{
   GFXDevice::getDeviceEventSignal().remove( this, &OculusVRDevice::_handleDeviceEvent );
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
   Con::addVariable("OculusVR::GeneratePositionEvents", TypeBool, &smGeneratePositionEvents, 
      "@brief If true, broadcast sensor rotation events as Euler angles about the X, Y and Z axis.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateRotationAsAxisEvents", TypeBool, &smGenerateRotationAsAxisEvents, 
      "@brief If true, broadcast sensor rotation as axis events.\n\n"
	   "@ingroup Game");
   Con::addVariable("OculusVR::MaximumAxisAngle", TypeF32, &smMaximumAxisAngle, 
      "@brief The maximum sensor angle when used as an axis event as measured from a vector pointing straight up (in degrees).\n\n"
      "Should range from 0 to 90 degrees.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateSensorRawEvents", TypeBool, &smGenerateSensorRawEvents, 
      "@brief If ture, broadcast sensor raw data: acceleration, angular velocity, magnetometer reading.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::GenerateWholeFrameEvents", TypeBool, &smGenerateWholeFrameEvents, 
      "@brief Indicates that a whole frame event should be generated and frames should be buffered.\n\n"
	   "@ingroup Game");

   Con::addVariable("OculusVR::desiredPixelDensity", TypeF32, &smDesiredPixelDensity,
      "@brief Specifies the desired pixel density of the render target. \n\n"
      "@ingroup Game");

   Con::addVariable("OculusVR::windowDebug", TypeBool, &smWindowDebug, 
      "@brief Specifies if the window should stay on the main display for debugging. \n\n"
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

void OculusVRDevice::addHMDDevice(ovrHmd hmd, ovrGraphicsLuid luid)
{
   if(!hmd)
      return;

   OculusVRHMDDevice* hmdd = new OculusVRHMDDevice();
   hmdd->set(hmd, luid, mHMDDevices.size());
   mHMDDevices.push_back(hmdd);

	ovrHmdDesc desc = ovr_GetHmdDesc(hmd);
   Con::printf("   HMD found: %s by %s [v%d]", desc.ProductName, desc.Manufacturer, desc.Type);
}

void OculusVRDevice::createSimulatedHMD()
{/* TOFIX
   OculusVRHMDDevice* hmdd = new OculusVRHMDDevice();
   ovrHmd hmd = ovr_CreateDebug(ovrHmd_DK2);
   hmdd->set(hmd,mHMDDevices.size());
   mHMDDevices.push_back(hmdd);

   Con::printf("   HMD simulated: %s by %s [v%d]", hmdd->getProductName(), hmdd->getManufacturer(), hmdd->getVersion()); */
}

bool OculusVRDevice::enable()
{
   // Start off with disabling the device if it is already enabled
   disable();

   Con::printf("Oculus VR Device Init:");

   if(sOcculusEnabled && OVR_SUCCESS(ovr_Initialize(0)))
   {
      mEnabled = true;

      // Enumerate HMDs and pick the first one
		ovrHmd hmd;
		ovrGraphicsLuid luid;
      if(OVR_SUCCESS(ovr_Create(&hmd, &luid)))
      {
         // Add the HMD to our list
         addHMDDevice(hmd, luid);

         setActive(true);
      }
      else
      {
         if(smSimulateHMD)
         {
            Con::printf("   Could not enumerate a HMD device.  Simulating a HMD.");
            createSimulatedHMD();
            setActive(true);
         }
         else
         {
            Con::printf("   Could not enumerate a HMD device.");
         }
      }

   }
   else
   {
         if(smSimulateHMD)
         {
            Con::printf("   Could not enumerate a HMD device.  Simulating a HMD.");
            createSimulatedHMD();
            setActive(true);
         }
         else
         {
            Con::printf("   Could not enumerate a HMD device.");
         }
   }

   Con::printf("   ");

   return false;
}

void OculusVRDevice::disable()
{
   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      delete mHMDDevices[i];
   }
   mHMDDevices.clear();

   if(mEnabled)
   {
      ovr_Shutdown();
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
   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      mHMDDevices[i]->getSensorDevice()->process(mDeviceType, smGenerateAngleAxisRotationEvents, smGenerateEulerRotationEvents, smGenerateRotationAsAxisEvents, smGeneratePositionEvents, maxAxisRadius, smGenerateSensorRawEvents);
   }

   return true;
}

//-----------------------------------------------------------------------------

bool OculusVRDevice::providesFrameEyePose() const
{
   if(!mHMDDevices.size())
      return false;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return false;

   return true;
}

void OculusVRDevice::getFrameEyePose(DisplayPose *outPose, U32 eyeId) const
{
   if(!mHMDDevices.size())
      return;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->getFrameEyePose(outPose, eyeId);
}


bool OculusVRDevice::providesEyeOffsets() const
{
   if(!mHMDDevices.size())
      return false;

   return true;
}

void OculusVRDevice::getEyeOffsets(Point3F *dest) const
{
   if(!mHMDDevices.size())
      return;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->getEyeOffsets(dest);
}


void OculusVRDevice::getFovPorts(FovPort *out) const
{
   if(!mHMDDevices.size())
      return;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   return hmd->getFovPorts(out);
}

bool OculusVRDevice::providesProjectionOffset() const
{
   if(!mHMDDevices.size())
      return false;

   return false;
}

const Point2F& OculusVRDevice::getProjectionOffset() const
{
   if(!mHMDDevices.size())
      return Point2F::Zero;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return Point2F::Zero;

   return hmd->getProjectionCenterOffset();
}

void OculusVRDevice::getStereoViewports(RectI *out) const
{
   if(!mHMDDevices.size())
      return;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->getStereoViewports(out);
}

void OculusVRDevice::getStereoTargets(GFXTextureTarget **out) const
{
   if(!mHMDDevices.size())
      return;

   const OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->getStereoTargets(out);
}

void OculusVRDevice::onStartFrame()
{
   _handleDeviceEvent(GFXDevice::deStartOfFrame);
}

//-----------------------------------------------------------------------------

OculusVRHMDDevice* OculusVRDevice::getHMDDevice(U32 index) const
{
   if(index >= mHMDDevices.size())
      return NULL;

   return mHMDDevices[index];
}

F32 OculusVRDevice::getHMDCurrentIPD(U32 index)
{
   if(index >= mHMDDevices.size())
      return -1.0f;

   return mHMDDevices[index]->getIPD();
}

void OculusVRDevice::setHMDCurrentIPD(U32 index, F32 ipd)
{
   if(index >= mHMDDevices.size())
      return;

   return mHMDDevices[index]->setIPD(ipd);
}

void OculusVRDevice::setOptimalDisplaySize(U32 index, GuiCanvas *canvas)
{
   if(index >= mHMDDevices.size())
      return;

   mHMDDevices[index]->setOptimalDisplaySize(canvas);
}


//-----------------------------------------------------------------------------

const OculusVRSensorDevice* OculusVRDevice::getSensorDevice(U32 index) const
{
   if(index >= mHMDDevices.size())
      return NULL;

   return mHMDDevices[index]->getSensorDevice();
}

EulerF OculusVRDevice::getSensorEulerRotation(U32 index)
{
   if(index >= mHMDDevices.size())
      return Point3F::Zero;

   return mHMDDevices[index]->getSensorDevice()->getEulerRotation();
}

VectorF OculusVRDevice::getSensorAcceleration(U32 index)
{
   if(index >= mHMDDevices.size())
      return Point3F::Zero;

   return mHMDDevices[index]->getSensorDevice()->getAcceleration();
}

EulerF OculusVRDevice::getSensorAngularVelocity(U32 index)
{
   if(index >= mHMDDevices.size())
      return Point3F::Zero;

   return mHMDDevices[index]->getSensorDevice()->getAngularVelocity();
}

bool OculusVRDevice::getSensorYawCorrection(U32 index)
{
   const OculusVRSensorDevice* sensor = getSensorDevice(index);
   if(!sensor || !sensor->isValid())
      return false;

   return sensor->getYawCorrection();
}

void OculusVRDevice::setSensorYawCorrection(U32 index, bool state)
{
   if(index >= mHMDDevices.size())
      return;

   OculusVRSensorDevice* sensor = mHMDDevices[index]->getSensorDevice();
   if(!sensor->isValid())
      return;

   sensor->setYawCorrection(state);
}

bool OculusVRDevice::getSensorMagnetometerCalibrated(U32 index)
{
   const OculusVRSensorDevice* sensor = getSensorDevice(index);
   if(!sensor || !sensor->isValid())
      return false;

   return sensor->getMagnetometerCalibrationAvailable();
}

void OculusVRDevice::resetAllSensors()
{
   // Reset each sensor
   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      mHMDDevices[i]->getSensorDevice()->reset();
   }
}

bool OculusVRDevice::isDiplayingWarning()
{
   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      if (mHMDDevices[i]->isDisplayingWarning())
         return true;
   }

   return false;
}

void OculusVRDevice::dismissWarning()
{
   for(U32 i=0; i<mHMDDevices.size(); ++i)
   {
      mHMDDevices[i]->dismissWarning();
   }
}

String OculusVRDevice::dumpMetrics(U32 idx)
{
   return mHMDDevices[idx]->dumpMetrics();
}

void OculusVRDevice::setDrawCanvas(GuiCanvas *canvas)
{
   if(!mHMDDevices.size())
      return;

   OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->setDrawCanvas(canvas);
}


void OculusVRDevice::setCurrentConnection(GameConnection *connection)
{
   if(!mHMDDevices.size())
      return;

   OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return;

   hmd->setCurrentConnection(connection);
}

GameConnection* OculusVRDevice::getCurrentConnection()
{
   if(!mHMDDevices.size())
      return NULL;

   OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if(!hmd)
      return NULL;

   return hmd->getCurrentConnection();
}

//-----------------------------------------------------------------------------

GFXTexHandle OculusVRDevice::getPreviewTexture()
{
   if (!mHMDDevices.size())
      return NULL;

   OculusVRHMDDevice* hmd = getHMDDevice(mActiveDeviceId);
   if (!hmd)
      return NULL;

   return hmd->getPreviewTexture();
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

DefineEngineFunction(setOptimalOVRCanvasSize, bool, (GuiCanvas* canvas),,
   "@brief Sets the first HMD to be a GameConnection's display device\n\n"
   "@param conn The GameConnection to set.\n"
   "@return True if the GameConnection display device was set.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      Con::errorf("setOptimalOVRCanvasSize(): No Oculus VR Device present.");
      return false;
   }

   if(!canvas)
   {
      Con::errorf("setOptimalOVRCanvasSize(): Invalid Canvas.");
      return false;
   }

   OCULUSVRDEV->setOptimalDisplaySize(0, canvas);
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

DefineEngineFunction(getOVRHMDDisplayDeviceType, const char*, (S32 index),,
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

   return hmd->getDisplayDeviceType();
}

DefineEngineFunction(getOVRHMDDisplayDeviceId, S32, (S32 index),,
   "@brief MacOS display ID.\n\n"
   "@param index The HMD index.\n"
   "@return The ID of the HMD display device, if any.\n"
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

   return hmd->getDisplayDeviceId();
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

DefineEngineFunction(getOVRHMDProfileIPD, F32, (S32 index),,
   "@brief Physical distance between the user's eye centers as defined by the current profile.\n\n"
   "@param index The HMD index.\n"
   "@return The profile IPD.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return -1.0f;
   }

   const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(index);
   if(!hmd)
   {
      return -1.0f;
   }

   return hmd->getProfileIPD();
}

DefineEngineFunction(getOVRHMDCurrentIPD, F32, (S32 index),,
   "@brief Physical distance between the user's eye centers.\n\n"
   "@param index The HMD index.\n"
   "@return The current IPD.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return -1.0f;
   }

   return OCULUSVRDEV->getHMDCurrentIPD(index);
}

DefineEngineFunction(setOVRHMDCurrentIPD, void, (S32 index, F32 ipd),,
   "@brief Set the physical distance between the user's eye centers.\n\n"
   "@param index The HMD index.\n"
   "@param ipd The IPD to use.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->setHMDCurrentIPD(index, ipd);
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

DefineEngineFunction(getOVRSensorAcceleration, Point3F, (S32 index),,
   "@brief Get the acceleration values for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return The acceleration values of the Oculus VR sensor, in m/s^2.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return Point3F::Zero;
   }

   return OCULUSVRDEV->getSensorAcceleration(index);
}

DefineEngineFunction(getOVRSensorAngVelocity, Point3F, (S32 index),,
   "@brief Get the angular velocity values for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return The angular velocity values of the Oculus VR sensor, in degrees/s.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return Point3F::Zero;
   }

   EulerF rot = OCULUSVRDEV->getSensorAngularVelocity(index);
   return Point3F(mRadToDeg(rot.x), mRadToDeg(rot.y), mRadToDeg(rot.z));
}

DefineEngineFunction(getOVRSensorYawCorrection, bool, (S32 index),,
   "@brief Get the yaw correction state for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return True if yaw correction (using magnetometer calibration data) is active.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return false;
   }

   return OCULUSVRDEV->getSensorYawCorrection(index);
}

DefineEngineFunction(setOVRSensorYawCorrection, void, (S32 index, bool state),,
   "@brief Set the yaw correction state for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@param state The yaw correction state to change to.\n"
   "@note Yaw correction cannot be enabled if the user has disabled it through "
   "the Oculus VR control panel.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->setSensorYawCorrection(index, state);
}

DefineEngineFunction(getOVRSensorMagnetometerCalibrated, bool, (S32 index),,
   "@brief Get the magnetometer calibrated data state for the given sensor index.\n\n"
   "@param index The sensor index.\n"
   "@return True if magnetometer calibration data is available.\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return false;
   }

   return OCULUSVRDEV->getSensorMagnetometerCalibrated(index);
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

DefineEngineFunction(ovrIsDisplayingWarning, bool, (),,
   "@brief returns is warning is being displayed.\n\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return false;
   }

   return OCULUSVRDEV->isDiplayingWarning();
}

DefineEngineFunction(ovrDismissWarnings, void, (),,
   "@brief dismisses warnings.\n\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return;
   }

   OCULUSVRDEV->dismissWarning();
}

DefineEngineFunction(ovrDumpMetrics, String, (S32 idx),(0),
   "@brief dumps sensor metrics.\n\n"
   "@ingroup Game")
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return "";
   }

   return OCULUSVRDEV->dumpMetrics(idx);
}

bool OculusVRDevice::_handleDeviceEvent( GFXDevice::GFXDeviceEventType evt )
{
   if(!ManagedSingleton<OculusVRDevice>::instanceOrNull())
   {
      return true;
   }

   switch( evt )
   {
      case GFXDevice::deStartOfFrame:
         
         for (U32 i=0; i<OCULUSVRDEV->mHMDDevices.size(); i++)
         {
            OCULUSVRDEV->mHMDDevices[i]->onStartFrame();
         }

         // Fall through
         break;

      case GFXDevice::dePostFrame:
         
         for (U32 i=0; i<OCULUSVRDEV->mHMDDevices.size(); i++)
         {
            OCULUSVRDEV->mHMDDevices[i]->onEndFrame();
         }

         break;

      case GFXDevice::deDestroy:
         
         for (U32 i=0; i<OCULUSVRDEV->mHMDDevices.size(); i++)
         {
            OCULUSVRDEV->mHMDDevices[i]->onDeviceDestroy();
         }
   
      default:
         break;
   }

   return true;
}