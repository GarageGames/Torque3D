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

#ifndef _OCULUSVRHMDDEVICE_H_
#define _OCULUSVRHMDDEVICE_H_

#include "core/util/str.h"
#include "math/mQuat.h"
#include "math/mPoint2.h"
#include "math/mPoint3.h"
#include "math/mPoint4.h"
#include "platform/input/oculusVR/oculusVRConstants.h"
#include "platform/types.h"
#include "gfx/gfxTextureHandle.h"
#include "math/mRect.h"
#include "gfx/gfxDevice.h"

#include "OVR_CAPI_0_5_0.h"

class GuiCanvas;
class GameConnection;
struct DisplayPose;
class OculusVRSensorDevice;

class OculusVRHMDDevice
{
public:
   enum SimulationTypes {
      ST_RIFT_PREVIEW,
   };

protected:
   bool mIsValid;

   bool mVsync;
   bool mTimewarp;

   bool mRenderConfigurationDirty;
   bool mFrameReady;

   ovrHmd mDevice;

   U32 mSupportedDistortionCaps;
   U32 mCurrentDistortionCaps;

   U32 mSupportedCaps;
   U32 mCurrentCaps;

   // From OVR::DeviceInfo
   String   mProductName;
   String   mManufacturer;
   U32      mVersion;

   // Windows display device name used in EnumDisplaySettings/CreateDC
   String   mDisplayDeviceName;

   // MacOS display ID
   S32      mDisplayId;

   // Desktop coordinate position of the screen (can be negative; may not be present on all platforms)
   Point2I  mDesktopPosition;

   // Whole screen resolution
   Point2I  mResolution;

   // Physical screen size in meters
   Point2F  mScreenSize;

   // Physical distance between lens centers, in meters
   F32      mLensSeparation;

   // Physical distance between the user's eye centers as defined in the current profile
   F32      mProfileInterpupillaryDistance;

   // Physical distance between the user's eye centers
   F32      mInterpupillaryDistance;

   // The amount to offset the projection matrix to account for the eye not being in the
   // center of the screen.
   Point2F mProjectionCenterOffset;

   // Current pose of eyes
   ovrPosef         mCurrentEyePoses[2];
   ovrEyeRenderDesc mEyeRenderDesc[2];

   ovrFovPort mCurrentFovPorts[2];

   Point2I mWindowSize;

   GameConnection *mConnection;

   OculusVRSensorDevice *mSensor;
   U32 mActionCodeIndex;

protected:
   void updateRenderInfo();

public:
   OculusVRHMDDevice();
   ~OculusVRHMDDevice();

   void cleanUp();

   // Set the HMD properties based on information from the OVR device
   void set(ovrHmd hmd, U32 actionCodeIndex);

   // Sets optimal display size for canvas
   void setOptimalDisplaySize(GuiCanvas *canvas);

   bool isValid() const {return mIsValid;}

   const char* getProductName() const { return mProductName.c_str(); }
   const char* getManufacturer() const { return mManufacturer.c_str(); }
   U32 getVersion() const { return mVersion; }

   // Windows display device name used in EnumDisplaySettings/CreateDC
   const char* getDisplayDeviceName() const { return mDisplayDeviceName.c_str(); }

   // MacOS display ID
   S32 getDisplayDeviceId() const { return mDisplayId; }

   // Desktop coordinate position of the screen (can be negative; may not be present on all platforms)
   const Point2I& getDesktopPosition() const { return mDesktopPosition; }

   // Whole screen resolution
   const Point2I& getResolution() const { return mResolution; }

   // Physical screen size in meters
   const Point2F& getScreenSize() const { return mScreenSize; }

   // Physical distance between lens centers, in meters
   F32 getLensSeparation() const { return mLensSeparation; }

   // Physical distance between the user's eye centers as defined by the current profile
   F32 getProfileIPD() const { return mProfileInterpupillaryDistance; }

   // Physical distance between the user's eye centers
   F32 getIPD() const { return mInterpupillaryDistance; }

   // Set a new physical distance between the user's eye centers
   void setIPD(F32 ipd);

   // The amount to offset the projection matrix to account for the eye not being in the
   // center of the screen.
   const Point2F& getProjectionCenterOffset() const { return mProjectionCenterOffset; }
   
   void getStereoViewports(RectI *dest) const { dMemcpy(dest, mEyeViewport, sizeof(mEyeViewport)); }
   void getStereoTargets(GFXTextureTarget **dest) const { dest[0] = mEyeRT[0]; dest[1] = mEyeRT[1]; }

   void getFovPorts(FovPort *dest) const { dMemcpy(dest, mCurrentFovPorts, sizeof(mCurrentFovPorts)); }
   
   /// Returns eye offsets in torque coordinate space, i.e. z being up, x being left-right, and y being depth (forward).
   void getEyeOffsets(Point3F *offsets) const { 
      offsets[0] = Point3F(-mEyeRenderDesc[0].HmdToEyeViewOffset.x, mEyeRenderDesc[0].HmdToEyeViewOffset.z, -mEyeRenderDesc[0].HmdToEyeViewOffset.y); 
      offsets[1] = Point3F(-mEyeRenderDesc[1].HmdToEyeViewOffset.x, mEyeRenderDesc[1].HmdToEyeViewOffset.z, -mEyeRenderDesc[1].HmdToEyeViewOffset.y); }

   void getFrameEyePose(DisplayPose *outPose, U32 eyeId) const;

   void updateCaps();

   void onStartFrame();
   void onEndFrame();
   void onDeviceDestroy();

   Point2I generateRenderTarget(GFXTextureTargetRef &target, GFXTexHandle &texture, GFXTexHandle &depth, Point2I desiredSize);
   void clearRenderTargets();

   bool isDisplayingWarning();
   void dismissWarning();

   bool setupTargets();

   /// Designates canvas we are drawing to. Also updates render targets
   void setDrawCanvas(GuiCanvas *canvas) { if (mDrawCanvas != canvas) { mDrawCanvas = canvas; } updateRenderInfo(); }

   virtual void setCurrentConnection(GameConnection *connection) { mConnection = connection; }
   virtual GameConnection* getCurrentConnection() { return mConnection; }

   String dumpMetrics();

   // Stereo RT
   GFXTexHandle mStereoTexture;
   GFXTexHandle mStereoDepthTexture;
   GFXTextureTargetRef mStereoRT;

   // Eye RTs (if we are using separate targets)
   GFXTextureTargetRef mEyeRT[2];
   GFXTexHandle mEyeTexture[2];
   GFXTexHandle mEyeDepthTexture[2];

   // Current render target size for each eye
   Point2I mEyeRenderSize[2];

   // Recommended eye target size for each eye
   ovrSizei mRecomendedEyeTargetSize[2];

   // Desired viewport for each eye
   RectI mEyeViewport[2];

   F32 mCurrentPixelDensity;
   F32 smDesiredPixelDensity;

   ovrTrackingState mLastTrackingState;

   GFXDevice::GFXDeviceRenderStyles mDesiredRenderingMode;

   GFXFormat mRTFormat;

   // Canvas we should be drawing
   GuiCanvas *mDrawCanvas;

   OculusVRSensorDevice *getSensorDevice() { return mSensor; }
};

#endif   // _OCULUSVRHMDDEVICE_H_
