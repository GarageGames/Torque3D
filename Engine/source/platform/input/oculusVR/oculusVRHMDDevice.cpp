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

#include "platform/input/oculusVR/oculusVRHMDDevice.h"

OculusVRHMDDevice::OculusVRHMDDevice()
{
   mIsValid = false;
   mIsSimulation = false;
   mDevice = NULL;
}

OculusVRHMDDevice::~OculusVRHMDDevice()
{
   cleanUp();
}

void OculusVRHMDDevice::cleanUp()
{
   if(mDevice)
   {
      mDevice->Release();
      mDevice = NULL;
   }

   mIsValid = false;
}

void OculusVRHMDDevice::set(OVR::HMDDevice* hmd, OVR::HMDInfo& info, bool calculateDistortionScale)
{
   mIsValid = false;
   mIsSimulation = false;

   mDevice = hmd;

   // DeviceInfo
   mProductName = info.ProductName;
   mManufacturer = info.Manufacturer;
   mVersion = info.Version;

   mDisplayDeviceName = info.DisplayDeviceName;

   mResolution.x = info.HResolution;
   mResolution.y = info.VResolution;

   mScreenSize.x = info.HScreenSize;
   mScreenSize.y = info.VScreenSize;

   mVerticalEyeCenter = info.VScreenCenter;
   mEyeToScreen = info.EyeToScreenDistance;
   mLensSeparation = info.LensSeparationDistance;
   mInterpupillaryDistance = info.InterpupillaryDistance;

   mKDistortion.x = info.DistortionK[0];
   mKDistortion.y = info.DistortionK[1];
   mKDistortion.z = info.DistortionK[2];
   mKDistortion.w = info.DistortionK[3];

   // Calculated values
   calculateValues(calculateDistortionScale);

   mIsValid = true;
}

void OculusVRHMDDevice::createSimulation(SimulationTypes simulationType, bool calculateDistortionScale)
{
   if(simulationType == ST_RIFT_PREVIEW)
   {
      createSimulatedPreviewRift(calculateDistortionScale);
   }
}

void OculusVRHMDDevice::createSimulatedPreviewRift(bool calculateDistortionScale)
{
   mIsValid = true;
   mIsSimulation = true;

   mProductName = "Oculus Rift DK1-SLA1";
   mManufacturer = "Oculus VR";
   mVersion = 0;

   mDisplayDeviceName = "";

   mResolution.x = 1280;
   mResolution.y = 800;

   mScreenSize.x = 0.14975999f;
   mScreenSize.y = 0.093599997f;

   mVerticalEyeCenter = 0.046799999f;
   mEyeToScreen = 0.041000001f;
   mLensSeparation = 0.064000003f;
   mInterpupillaryDistance = 0.064000003f;

   mKDistortion.x = 1.0000000f;
   mKDistortion.y = 0.22000000f;
   mKDistortion.z = 0.23999999f;
   mKDistortion.w = 0.00000000f;

   calculateValues(calculateDistortionScale);
}

// Computes scale that should be applied to the input render texture
// before distortion to fit the result in the same screen size.
// The 'fitRadius' parameter specifies the distance away from distortion center at
// which the input and output coordinates will match, assuming [-1,1] range.
F32 OculusVRHMDDevice::calcScale(F32 fitRadius)
{
   F32 s = fitRadius;

   // This should match distortion equation used in shader.
   F32 ssq   = s * s;
   F32 scale = s * (mKDistortion.x + mKDistortion.y * ssq + mKDistortion.z * ssq * ssq + mKDistortion.w * ssq * ssq * ssq);
   return scale;
}

void OculusVRHMDDevice::calculateValues(bool calculateDistortionScale)
{
   F32 halfScreenX = mScreenSize.x * 0.5f;
   if(halfScreenX > 0)
   {
      F32 halfLensSeparation = mLensSeparation * 0.5;
      F32 offset = halfLensSeparation / halfScreenX;
      mEyeUVOffset.x = offset - 0.5;
      mEyeUVOffset.y = 1.0f - offset - 0.5;
   }
   else
   {
      mEyeUVOffset.x = 0.5f;
      mEyeUVOffset.y = 0.5f;
   }

   F32 lensOffset        = mLensSeparation * 0.5f;
   F32 lensShift         = mScreenSize.x * 0.25f - lensOffset;
   F32 lensViewportShift = 4.0f * lensShift / mScreenSize.x;
   mXCenterOffset= lensViewportShift;

   // Determine how the input texture should be scaled relative to the back buffer
   // so that we fit the distorted view to the backbuffer after calculating the
   // distortion.  In reference to section 5.6.3 Distortion Scale and FOV in the
   // SDK docs.
   if(!calculateDistortionScale)
   {
      // Do not calculate a distortion scale for the input texture.  This means that the input
      // texture and the backbuffer will be the same resolution.
      mDistortionFit.x = 0.0f;
      mDistortionFit.y = 0.0f;
   }
   else if (mScreenSize.x > 0.140f) // 7"
   {
      mDistortionFit.x = -1.0f;
      mDistortionFit.y = 0.0f;
   }
   else // 5"
   {
      mDistortionFit.x = 0.0f;
      mDistortionFit.y = 1.0f;
   }

   // Compute distortion scale from DistortionFitX & DistortionFitY.
   // Fit value of 0.0 means "no fit".
   if (mIsZero(mDistortionFit.x) && mIsZero(mDistortionFit.y))
   {
      mDistortionScale = 1.0f;
   }
   else
   {
      // Convert fit value to distortion-centered coordinates before fit radius
      // calculation.
      // NOTE: For now just assume a full view the same size as the HMD supports.  It is
      // possible that this full view is smaller or larger.
      F32 stereoAspect = 0.5f * mResolution.x / mResolution.y;
      F32 dx           = mDistortionFit.x - mXCenterOffset;
      F32 dy           = mDistortionFit.y / stereoAspect;
      F32 fitRadius    = sqrt(dx * dx + dy * dy);
      mDistortionScale   = calcScale(fitRadius)/fitRadius;
   }

   // Calculate the vertical FOV for a single eye
   mAspectRatio = F32(mResolution.x * 0.5f) / F32(mResolution.y);
   F32 halfScreenDistance = mScreenSize.y * 0.5f * mDistortionScale;
   mYFOV = 2.0f * mAtan(halfScreenDistance / mEyeToScreen);

   F32 viewCenter = mScreenSize.x * 0.25f;
   F32 eyeProjectionShift = viewCenter - (mInterpupillaryDistance * 0.5f);
   mProjectionCenterOffset.set(4.0f * eyeProjectionShift / mScreenSize.x, 0.0f);

   mEyeWorldOffset.set(mInterpupillaryDistance * 0.5f, 0.0f, 0.0f);
}
