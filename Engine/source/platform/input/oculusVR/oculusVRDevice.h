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

#ifndef _OCULUSVRDEVICE_H_
#define _OCULUSVRDEVICE_H_

#include "platform/input/oculusVR/oculusVRConstants.h"
#include "platform/input/oculusVR/oculusVRHMDDevice.h"
#include "platform/input/oculusVR/oculusVRSensorDevice.h"
#include "platform/input/IInputDevice.h"
#include "platform/input/event.h"
#include "platform/output/IDisplayDevice.h"
#include "core/util/tSingleton.h"
#include "math/mQuat.h"
#include "math/mPoint4.h"
#include "gfx/gfxDevice.h"
#include "OVR_CAPI_0_8_0.h"

#define DEFAULT_RIFT_UNIT 0

class OculusVRDevice : public IInputDevice, public IDisplayDevice
{
public:
   static bool smEnableDevice;

   // If no HMD is present simulate it being available
   static bool smSimulateHMD;

   // Type of rotation events to broadcast
   static bool smGenerateAngleAxisRotationEvents;
   static bool smGenerateEulerRotationEvents;
   static bool smGeneratePositionEvents;

   // Broadcast sensor rotation as axis
   static bool smGenerateRotationAsAxisEvents;

   // The maximum sensor angle when used as an axis event
   // as measured from a vector pointing straight up (in degrees)
   static F32 smMaximumAxisAngle;

   // Broadcast sensor raw data: acceleration, angular velocity, magnetometer reading
   static bool smGenerateSensorRawEvents;

   // Indicates that a whole frame event should be generated and frames
   // should be buffered.
   static bool smGenerateWholeFrameEvents;

   /// Determines desired pixel density for render target
   static F32 smDesiredPixelDensity;

   /// Determined if the window is moved to the oculus display
   static bool smWindowDebug;

   static F32 smPositionTrackingScale;

protected:

   // Discovered HMD devices
   Vector<OculusVRHMDDevice*> mHMDDevices;

   /// Is the device active
   bool mActive;

   /// Which HMD is the active one
   U32 mActiveDeviceId;

   /// Device id we need to use to hook up with oculus
   ovrGraphicsLuid mLuid;

protected:
   void cleanUp();

   /// Build out the codes used for controller actions with the
   /// Input Event Manager
   void buildCodeTable();

   void addHMDDevice(ovrHmd hmd, ovrGraphicsLuid luid);

   void createSimulatedHMD();

public:
   OculusVRDevice();
   ~OculusVRDevice();

   static void staticInit();

   bool enable();
   void disable();

   bool getActive() { return mActive; }
   void setActive(bool state) { mActive = state; }

   bool process();

   // IDisplayDevice
   virtual bool providesFrameEyePose() const;
   virtual void getFrameEyePose(DisplayPose *outPose, U32 eyeId) const;
   virtual bool providesEyeOffsets() const;
   virtual bool providesFovPorts() const { return true;  }
   virtual void getEyeOffsets(Point3F *dest) const;
   virtual void getFovPorts(FovPort *out) const;
   virtual bool providesProjectionOffset() const;
   virtual const Point2F& getProjectionOffset() const;
   virtual void getStereoViewports(RectI *out) const;
   virtual void getStereoTargets(GFXTextureTarget **out) const;
   virtual void onStartFrame();

   // HMDs
   U32 getHMDCount() const { return mHMDDevices.size(); }
   OculusVRHMDDevice* getHMDDevice(U32 index) const;
   F32 getHMDCurrentIPD(U32 index);
   void setHMDCurrentIPD(U32 index, F32 ipd);

   // Sensors
   U32 getSensorCount() const { return mHMDDevices.size(); }
   const OculusVRSensorDevice* getSensorDevice(U32 index) const;
   EulerF getSensorEulerRotation(U32 index);
   VectorF getSensorAcceleration(U32 index);
   EulerF getSensorAngularVelocity(U32 index);
   bool getSensorYawCorrection(U32 index);
   void setSensorYawCorrection(U32 index, bool state);
   bool getSensorMagnetometerCalibrated(U32 index);

   void setOptimalDisplaySize(U32 idx, GuiCanvas *canvas);
   void resetAllSensors();

   bool isDiplayingWarning();
   void dismissWarning();

   String dumpMetrics(U32 idx);


   void setDrawCanvas(GuiCanvas *canvas);
   
   virtual void setCurrentConnection(GameConnection *connection);
   virtual GameConnection* getCurrentConnection();

   GFXTexHandle getPreviewTexture();

   bool _handleDeviceEvent( GFXDevice::GFXDeviceEventType evt );

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "OculusVRDevice"; }   
};

/// Returns the OculusVRDevice singleton.
#define OCULUSVRDEV ManagedSingleton<OculusVRDevice>::instance()

#endif   // _OCULUSVRDEVICE_H_
