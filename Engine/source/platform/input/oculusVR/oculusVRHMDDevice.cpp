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
#include "platform/input/oculusVR/oculusVRDevice.h"
#include "platform/input/oculusVR/oculusVRSensorDevice.h"
#include "postFx/postEffectCommon.h"
#include "gui/core/guiCanvas.h"
#include "platform/input/oculusVR/oculusVRUtil.h"
#include "core/stream/fileStream.h"


#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/gfxStringEnumTranslate.h"
#undef D3D11

// Use D3D11 for win32
#ifdef TORQUE_OS_WIN
#define OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"
#define OCULUS_USE_D3D
#else
#include "OVR_CAPI_GL.h"
#define OCULUS_USE_GL
#endif

struct OculusTexture
{
   virtual void AdvanceToNextTexture() = 0;

   virtual ~OculusTexture() {
   }
};

//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.
struct D3D11OculusTexture : public OculusTexture
{
   ovrHmd                   hmd;
   ovrSwapTextureSet      * TextureSet;
   static const int         TextureCount = 2;
   GFXTexHandle  TexRtv[TextureCount];
   GFXDevice *Owner;

   D3D11OculusTexture(GFXDevice* owner) :
      hmd(nullptr),
      TextureSet(nullptr),
      Owner(owner)
   {
      TexRtv[0] = TexRtv[1] = nullptr;
   }

   bool Init(ovrHmd _hmd, int sizeW, int sizeH)
   {
      hmd = _hmd;

      D3D11_TEXTURE2D_DESC dsDesc;
      dsDesc.Width = sizeW;
      dsDesc.Height = sizeH;
      dsDesc.MipLevels = 1;
      dsDesc.ArraySize = 1;
      dsDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;// DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      dsDesc.SampleDesc.Count = 1;   // No multi-sampling allowed
      dsDesc.SampleDesc.Quality = 0;
      dsDesc.Usage = D3D11_USAGE_DEFAULT;
      dsDesc.CPUAccessFlags = 0;
      dsDesc.MiscFlags = 0;
      dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;


      GFXD3D11Device* device = static_cast<GFXD3D11Device*>(GFX);
      ovrResult result = ovr_CreateSwapTextureSetD3D11(hmd, device->mD3DDevice, &dsDesc, ovrSwapTextureSetD3D11_Typeless, &TextureSet);
      if (!OVR_SUCCESS(result))
         return false;

      AssertFatal(TextureSet->TextureCount == TextureCount, "TextureCount mismatch.");

      for (int i = 0; i < TextureCount; ++i)
      {
         ovrD3D11Texture* tex = (ovrD3D11Texture*)&TextureSet->Textures[i];
         D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
         rtvd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;
         rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

         GFXD3D11TextureObject* object = new GFXD3D11TextureObject(GFX, &VRTextureProfile);
         object->registerResourceWithDevice(GFX);
         *(object->getSRViewPtr()) = tex->D3D11.pSRView;
         *(object->get2DTexPtr()) = tex->D3D11.pTexture;
         device->mD3DDevice->CreateRenderTargetView(tex->D3D11.pTexture, &rtvd, object->getRTViewPtr());

         // Add refs for texture release later on
         if (object->getSRView()) object->getSRView()->AddRef();
         //object->getRTView()->AddRef();
         if (object->get2DTex()) object->get2DTex()->AddRef();
         object->isManaged = true;

         // Get the actual size of the texture...
         D3D11_TEXTURE2D_DESC probeDesc;
         ZeroMemory(&probeDesc, sizeof(D3D11_TEXTURE2D_DESC));
         object->get2DTex()->GetDesc(&probeDesc);

         object->mTextureSize.set(probeDesc.Width, probeDesc.Height, 0);
         object->mBitmapSize = object->mTextureSize;
         int fmt = probeDesc.Format;

         if (fmt == DXGI_FORMAT_R8G8B8A8_TYPELESS || fmt == DXGI_FORMAT_B8G8R8A8_TYPELESS)
         {
            object->mFormat = GFXFormatR8G8B8A8; // usual case
         }
         else
         {
            // TODO: improve this. this can be very bad.
            GFXREVERSE_LOOKUP(GFXD3D11TextureFormat, GFXFormat, fmt);
            object->mFormat = (GFXFormat)fmt;
         }
         TexRtv[i] = object;
      }

      return true;
   }

   ~D3D11OculusTexture()
   {
      for (int i = 0; i < TextureCount; ++i)
      {
         SAFE_DELETE(TexRtv[i]);
      }
      if (TextureSet)
      {
         ovr_DestroySwapTextureSet(hmd, TextureSet);
      }
   }

   void AdvanceToNextTexture()
   {
      TextureSet->CurrentIndex = (TextureSet->CurrentIndex + 1) % TextureSet->TextureCount;
   }
};


OculusVRHMDDevice::OculusVRHMDDevice()
{
   mIsValid = false;
   mDevice = NULL;
   mCurrentCaps = 0;
   mSupportedCaps = 0;
   mVsync = true;
   mTimewarp = true;
   mRenderConfigurationDirty = true;
   mCurrentPixelDensity = OculusVRDevice::smDesiredPixelDensity;
   mDesiredRenderingMode = GFXDevice::RS_StereoSideBySide;
   mRTFormat = GFXFormatR8G8B8A8;
   mDrawCanvas = NULL;
   mFrameReady = false;
   mConnection = NULL;
   mSensor = NULL;
   mActionCodeIndex = 0;
   mTextureSwapSet = NULL;
}

OculusVRHMDDevice::~OculusVRHMDDevice()
{
   cleanUp();
}

void OculusVRHMDDevice::cleanUp()
{
   onDeviceDestroy();

   if (mSensor)
   {
      delete mSensor;
      mSensor = NULL;
   }

   if(mDevice)
   {
      ovr_Destroy(mDevice);
      mDevice = NULL;
   }

   mIsValid = false;
}

void OculusVRHMDDevice::set(ovrHmd hmd, ovrGraphicsLuid luid, U32 actionCodeIndex)
{
   cleanUp();

   mIsValid = false;
   mRenderConfigurationDirty = true;

   mDevice = hmd;

   ovrHmdDesc desc = ovr_GetHmdDesc(hmd);
   int caps = ovr_GetTrackingCaps(hmd);

   mSupportedCaps = desc.AvailableHmdCaps;
   mCurrentCaps = mSupportedCaps;
   
   mTimewarp = true;

   // DeviceInfo
   mProductName = desc.ProductName;
   mManufacturer = desc.Manufacturer;
   mVersion = desc.FirmwareMajor;

   //
   Vector<GFXAdapter*> adapterList;
   GFXD3D11Device::enumerateAdapters(adapterList);

   dMemcpy(&mLuid, &luid, sizeof(mLuid));
   mDisplayId = -1;

   for (U32 i = 0, sz = adapterList.size(); i < sz; i++)
   {
      GFXAdapter* adapter = adapterList[i];
      if (dMemcmp(&adapter->mLUID, &mLuid, sizeof(mLuid)) == 0)
      {
         mDisplayId = adapter->mIndex;
         mDisplayDeviceType = "D3D11"; // TOFIX this
      }
   }

   mResolution.x = desc.Resolution.w;
   mResolution.y = desc.Resolution.h;

   mProfileInterpupillaryDistance = ovr_GetFloat(hmd, OVR_KEY_IPD, OVR_DEFAULT_IPD);
   mLensSeparation = ovr_GetFloat(hmd, "LensSeparation", 0);
   ovr_GetFloatArray(hmd, "ScreenSize", &mScreenSize.x, 2);

   mActionCodeIndex = actionCodeIndex;

   mIsValid = true;

   mSensor = new OculusVRSensorDevice();
   mSensor->set(mDevice, mActionCodeIndex);

   mDebugMirrorTexture = NULL;

   updateCaps();
}

void OculusVRHMDDevice::setIPD(F32 ipd)
{
   mInterpupillaryDistance = ipd;
}

void OculusVRHMDDevice::setOptimalDisplaySize(GuiCanvas *canvas)
{
   if (!mDevice)
      return;

   PlatformWindow *window = canvas->getPlatformWindow();
   GFXTarget *target = window->getGFXTarget();

   Point2I requiredSize(0, 0);

   ovrHmdDesc desc = ovr_GetHmdDesc(mDevice);
   ovrSizei leftSize = ovr_GetFovTextureSize(mDevice, ovrEye_Left, desc.DefaultEyeFov[0], mCurrentPixelDensity);
   ovrSizei rightSize = ovr_GetFovTextureSize(mDevice, ovrEye_Right, desc.DefaultEyeFov[1], mCurrentPixelDensity);

   requiredSize.x = leftSize.w + rightSize.h;
   requiredSize.y = mMax(leftSize.h, rightSize.h);
   
   if (target && target->getSize() != requiredSize)
   {
      GFXVideoMode newMode;
      newMode.antialiasLevel = 0;
      newMode.bitDepth = 32;
      newMode.fullScreen = false;
      newMode.refreshRate = 75;
      newMode.resolution = requiredSize;
      newMode.wideScreen = false;
      window->setVideoMode(newMode);
      //AssertFatal(window->getClientExtent().x == requiredSize.x && window->getClientExtent().y == requiredSize.y, "Window didn't resize to correct dimensions");
   }
}

bool OculusVRHMDDevice::isDisplayingWarning()
{
   if (!mIsValid || !mDevice)
      return false;

   return false;/*
   ovrHSWDisplayState displayState;
   ovrHmd_GetHSWDisplayState(mDevice, &displayState);

   return displayState.Displayed;*/
}

void OculusVRHMDDevice::dismissWarning()
{
   if (!mIsValid || !mDevice)
      return;
   //ovr_DismissHSWDisplay(mDevice);
}

GFXTexHandle OculusVRHMDDevice::getPreviewTexture()
{
   if (!mIsValid || !mDevice)
      return NULL;

   return mDebugMirrorTextureHandle;
}

bool OculusVRHMDDevice::setupTargets()
{
   // Create eye render buffers
   ID3D11RenderTargetView * eyeRenderTexRtv[2];
   ovrLayerEyeFov           ld = { { ovrLayerType_EyeFov } };
   mRenderLayer = ld;

   GFXD3D11Device* device = static_cast<GFXD3D11Device*>(GFX);

   ovrHmdDesc desc = ovr_GetHmdDesc(mDevice);
   for (int i = 0; i < 2; i++)
   {
      mRenderLayer.Fov[i] = desc.DefaultEyeFov[i];
      mRenderLayer.Viewport[i].Size = ovr_GetFovTextureSize(mDevice, (ovrEyeType)i, mRenderLayer.Fov[i], mCurrentPixelDensity);
      mEyeRenderDesc[i] = ovr_GetRenderDesc(mDevice, (ovrEyeType_)(ovrEye_Left+i), mRenderLayer.Fov[i]);
   }

   ovrSizei recommendedEyeTargetSize[2];
   recommendedEyeTargetSize[0] = mRenderLayer.Viewport[0].Size;
   recommendedEyeTargetSize[1] = mRenderLayer.Viewport[1].Size;

   if (mTextureSwapSet)
   {
      delete mTextureSwapSet;
      mTextureSwapSet = NULL;
   }

   // Calculate render target size
   if (mDesiredRenderingMode == GFXDevice::RS_StereoSideBySide)
   {
      // Setup a single texture, side-by-side viewports
      Point2I rtSize(
         recommendedEyeTargetSize[0].w + recommendedEyeTargetSize[1].w,
         recommendedEyeTargetSize[0].h > recommendedEyeTargetSize[1].h ? recommendedEyeTargetSize[0].h : recommendedEyeTargetSize[1].h
         );

      GFXFormat targetFormat = GFX->getActiveRenderTarget()->getFormat();
      mRTFormat = targetFormat;

      rtSize = generateRenderTarget(mStereoRT, mStereoDepthTexture, rtSize);

      // Generate the swap texture we need to store the final image
      D3D11OculusTexture* tex = new D3D11OculusTexture(GFX);
      if (tex->Init(mDevice, rtSize.x, rtSize.y))
      {
         mTextureSwapSet = tex;
      }

      mRenderLayer.ColorTexture[0] = tex->TextureSet;
      mRenderLayer.ColorTexture[1] = tex->TextureSet;

      mRenderLayer.Viewport[0].Pos.x = 0;
      mRenderLayer.Viewport[0].Pos.y = 0;
      mRenderLayer.Viewport[1].Pos.x = (rtSize.x + 1) / 2;
      mRenderLayer.Viewport[1].Pos.y = 0;

      // Left
      mEyeRT[0] = mStereoRT;
      mEyeViewport[0] = RectI(Point2I(mRenderLayer.Viewport[0].Pos.x, mRenderLayer.Viewport[0].Pos.y), Point2I(mRenderLayer.Viewport[0].Size.w, mRenderLayer.Viewport[0].Size.h));

      // Right
      mEyeRT[1] = mStereoRT;
      mEyeViewport[1] = RectI(Point2I(mRenderLayer.Viewport[1].Pos.x, mRenderLayer.Viewport[1].Pos.y), Point2I(mRenderLayer.Viewport[1].Size.w, mRenderLayer.Viewport[1].Size.h));

      GFXD3D11Device* device = static_cast<GFXD3D11Device*>(GFX);

      D3D11_TEXTURE2D_DESC dsDesc;
      dsDesc.Width = rtSize.x;
      dsDesc.Height = rtSize.y;
      dsDesc.MipLevels = 1;
      dsDesc.ArraySize = 1;
      dsDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;// DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      dsDesc.SampleDesc.Count = 1;
      dsDesc.SampleDesc.Quality = 0;
      dsDesc.Usage = D3D11_USAGE_DEFAULT;
      dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      dsDesc.CPUAccessFlags = 0;
      dsDesc.MiscFlags = 0;

      // Create typeless when we are rendering as non-sRGB since we will override the texture format in the RTV
      bool reinterpretSrgbAsLinear = true;
      unsigned compositorTextureFlags = 0;
      if (reinterpretSrgbAsLinear)
         compositorTextureFlags |= ovrSwapTextureSetD3D11_Typeless;

      ovrResult result = ovr_CreateMirrorTextureD3D11(mDevice, device->mD3DDevice, &dsDesc, compositorTextureFlags, &mDebugMirrorTexture);
      
      if (result == ovrError_DisplayLost || !mDebugMirrorTexture)
      {
         AssertFatal(false, "Something went wrong");
         return NULL;
      }

      // Create texture handle so we can render it in-game
      ovrD3D11Texture* mirror_tex = (ovrD3D11Texture*)mDebugMirrorTexture;
      D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
      rtvd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;
      rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      GFXD3D11TextureObject* object = new GFXD3D11TextureObject(GFX, &VRTextureProfile);
      object->registerResourceWithDevice(GFX);
      *(object->getSRViewPtr()) = mirror_tex->D3D11.pSRView;
      *(object->get2DTexPtr()) = mirror_tex->D3D11.pTexture;
      device->mD3DDevice->CreateRenderTargetView(mirror_tex->D3D11.pTexture, &rtvd, object->getRTViewPtr());


      // Add refs for texture release later on
      if (object->getSRView()) object->getSRView()->AddRef();
      //object->getRTView()->AddRef();
      if (object->get2DTex()) object->get2DTex()->AddRef();
      object->isManaged = true;

      // Get the actual size of the texture...
      D3D11_TEXTURE2D_DESC probeDesc;
      ZeroMemory(&probeDesc, sizeof(D3D11_TEXTURE2D_DESC));
      object->get2DTex()->GetDesc(&probeDesc);

      object->mTextureSize.set(probeDesc.Width, probeDesc.Height, 0);
      object->mBitmapSize = object->mTextureSize;
      int fmt = probeDesc.Format;

      if (fmt == DXGI_FORMAT_R8G8B8A8_TYPELESS || fmt == DXGI_FORMAT_B8G8R8A8_TYPELESS)
      {
         object->mFormat = GFXFormatR8G8B8A8; // usual case
      }
      else
      {
         // TODO: improve this. this can be very bad.
         GFXREVERSE_LOOKUP(GFXD3D11TextureFormat, GFXFormat, fmt);
         object->mFormat = (GFXFormat)fmt;
      }
      
      mDebugMirrorTextureHandle = object;
   }
   else
   {
      // No rendering, abort!
      return false;
   }

   return true;
}

String OculusVRHMDDevice::dumpMetrics()
{
   StringBuilder sb;

   EulerF rot = mSensor->getEulerRotation();
   Point3F pos = mSensor->getPosition();
   FovPort eyeFov[2];
   this->getFovPorts(eyeFov);

   mSensor->getPositionTrackingAvailable();

   F32 ipd = this->getIPD();
   U32 lastStatus = mSensor->getLastTrackingStatus();

   sb.format("   | OVR Sensor %i | rot: %f %f %f, pos: %f %f %f, FOV (%f %f %f %f, %f %f %f %f), IPD %f, Track:%s%s",
             mActionCodeIndex,
             rot.x, rot.y, rot.z,
             pos.x, pos.y, pos.z,
             eyeFov[0].upTan, eyeFov[0].downTan, eyeFov[0].leftTan, eyeFov[0].rightTan, eyeFov[1].upTan, eyeFov[1].downTan, eyeFov[1].leftTan, eyeFov[1].rightTan,
             getIPD(),
             lastStatus & ovrStatus_OrientationTracked ? " ORIENT" : "",
             lastStatus & ovrStatus_PositionTracked ? " POS" : "");

   return sb.data();
}

void OculusVRHMDDevice::updateRenderInfo()
{
   // Check console values first
   if (mCurrentPixelDensity != OculusVRDevice::smDesiredPixelDensity)
   {
      mRenderConfigurationDirty = true;
      mCurrentPixelDensity = OculusVRDevice::smDesiredPixelDensity;
   }

   if (!mIsValid || !mDevice || !mRenderConfigurationDirty)
      return;

   if (!mDrawCanvas)
      return;
   
   PlatformWindow *window = mDrawCanvas->getPlatformWindow();

   ovrHmdDesc desc = ovr_GetHmdDesc(mDevice);

   // Update window size if it's incorrect
   Point2I backbufferSize = mDrawCanvas->getBounds().extent;

   // Finally setup!
   if (!setupTargets())
   {
      onDeviceDestroy();
      return;
   }

   mRenderConfigurationDirty = false;
}

Point2I OculusVRHMDDevice::generateRenderTarget(GFXTextureTargetRef &target, GFXTexHandle &depth, Point2I desiredSize)
{
    // Texture size that we already have might be big enough.
    Point2I newRTSize;
    bool newRT = false;
    
    if (!target.getPointer())
    {
       target = GFX->allocRenderToTextureTarget();
       newRTSize = desiredSize;
       newRT = true;
    }
    else
    {
       Point2I currentSize = target->getSize();
       newRTSize = currentSize;
    }

    // %50 linear growth each time is a nice balance between being too greedy
    // for a 2D surface and too slow to prevent fragmentation.
    while ( newRTSize.x < desiredSize.x )
    {
        newRTSize.x += newRTSize.x/2;
    }
    while ( newRTSize.y < desiredSize.y )
    {
        newRTSize.y += newRTSize.y/2;
    }

    // Put some sane limits on it. 4k x 4k is fine for most modern video cards.
    // Nobody should be messing around with surfaces smaller than 4k pixels these days.
    newRTSize.setMin(Point2I(4096, 4096));
    newRTSize.setMax(Point2I(64, 64));

    // Stereo RT needs to be the same size as the recommended RT
    /*if ( newRT || mDebugStereoTexture.getWidthHeight() != newRTSize )
    {
       mDebugStereoTexture.set( newRTSize.x, newRTSize.y, mRTFormat, &VRTextureProfile,  avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
       target->attachTexture( GFXTextureTarget::Color0, mDebugStereoTexture);
       Con::printf("generateRenderTarget generated %x", mDebugStereoTexture.getPointer());
    }*/

    if ( depth.getWidthHeight() != newRTSize )
    {
       depth.set( newRTSize.x, newRTSize.y, GFXFormatD24S8, &VRDepthProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
       target->attachTexture( GFXTextureTarget::DepthStencil, depth );
       Con::printf("generateRenderTarget generated depth %x", depth.getPointer());
    }

    return newRTSize;
}

void OculusVRHMDDevice::clearRenderTargets()
{
   mStereoRT = NULL;
   mEyeRT[0] = NULL;
   mEyeRT[1] = NULL;

   if (mDebugMirrorTexture)
   {
      ovr_DestroyMirrorTexture(mDevice, mDebugMirrorTexture);
      mDebugMirrorTexture = NULL;
      mDebugMirrorTextureHandle = NULL;
   }
}

void OculusVRHMDDevice::updateCaps()
{
   if (!mIsValid || !mDevice)
      return;

   ovr_SetEnabledCaps(mDevice, mCurrentCaps);
}

static bool sInFrame = false; // protects against recursive onStartFrame calls

void OculusVRHMDDevice::onStartFrame()
{
   if (!mIsValid || !mDevice || !mDrawCanvas || sInFrame || mFrameReady)
      return;

   sInFrame = true;

   ovrVector3f hmdToEyeViewOffset[2] = { mEyeRenderDesc[0].HmdToEyeViewOffset, mEyeRenderDesc[1].HmdToEyeViewOffset };
   ovrTrackingState hmdState = ovr_GetTrackingState(mDevice, 0, ovrTrue);
   ovr_CalcEyePoses(hmdState.HeadPose.ThePose, hmdToEyeViewOffset, mRenderLayer.RenderPose);

   for (U32 i=0; i<2; i++)
   {
      mRenderLayer.RenderPose[i].Position.x *= OculusVRDevice::smPositionTrackingScale;
      mRenderLayer.RenderPose[i].Position.y *= OculusVRDevice::smPositionTrackingScale;
      mRenderLayer.RenderPose[i].Position.z *= OculusVRDevice::smPositionTrackingScale;
   }

   mRenderLayer.SensorSampleTime = ovr_GetTimeInSeconds();

   // Set current dest texture on stereo render target
   D3D11OculusTexture* texSwap = (D3D11OculusTexture*)mTextureSwapSet;
   mStereoRT->attachTexture(GFXTextureTarget::Color0, texSwap->TexRtv[texSwap->TextureSet->CurrentIndex]);

   sInFrame = false;
   mFrameReady = true;
}

void OculusVRHMDDevice::onEndFrame()
{
   if (!mIsValid || !mDevice || !mDrawCanvas || sInFrame || !mFrameReady || !mTextureSwapSet)
      return;

   Point2I eyeSize;
   GFXTarget *windowTarget = mDrawCanvas->getPlatformWindow()->getGFXTarget();

   GFXD3D11Device *d3d11GFX = dynamic_cast<GFXD3D11Device*>(GFX);

   ovrViewScaleDesc viewScaleDesc;
   ovrVector3f hmdToEyeViewOffset[2] = { mEyeRenderDesc[0].HmdToEyeViewOffset, mEyeRenderDesc[1].HmdToEyeViewOffset };
   viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
   viewScaleDesc.HmdToEyeViewOffset[0] = hmdToEyeViewOffset[0];
   viewScaleDesc.HmdToEyeViewOffset[1] = hmdToEyeViewOffset[1];


   ovrLayerDirect           ld = { { ovrLayerType_Direct } };
   mDebugRenderLayer = ld;

   mDebugRenderLayer.ColorTexture[0] = mRenderLayer.ColorTexture[0];
   mDebugRenderLayer.ColorTexture[1] = mRenderLayer.ColorTexture[1];
   mDebugRenderLayer.Viewport[0] = mRenderLayer.Viewport[0];
   mDebugRenderLayer.Viewport[1] = mRenderLayer.Viewport[1];

   // TODO: use ovrViewScaleDesc
   ovrLayerHeader* layers = &mRenderLayer.Header;
   ovrResult result = ovr_SubmitFrame(mDevice, 0, &viewScaleDesc, &layers, 1);
   mTextureSwapSet->AdvanceToNextTexture();

   if (OVR_SUCCESS(result))
   {
      int woo = 1;
   }

   // TODO: render preview in display?

   mFrameReady = false;
}

void OculusVRHMDDevice::getFrameEyePose(DisplayPose *outPose, U32 eyeId) const
{
   // Directly set the rotation and position from the eye transforms
   ovrPosef pose = mRenderLayer.RenderPose[eyeId];
   OVR::Quatf orientation = pose.Orientation;
   const OVR::Vector3f position = pose.Position;

   MatrixF torqueMat(1);
   OVR::Matrix4f mat(orientation);
   OculusVRUtil::convertRotation(mat.M, torqueMat);

   outPose->orientation = QuatF(torqueMat);
   outPose->position = Point3F(-position.x, position.z, -position.y);
}

void OculusVRHMDDevice::onDeviceDestroy()
{
   if (!mIsValid || !mDevice)
      return;

   if (mStereoRT.getPointer())
   {
      mStereoRT->zombify();
   }

   if (mEyeRT[1].getPointer() && mEyeRT[1] != mStereoRT)
   {
      mEyeRT[0]->zombify();
      mEyeRT[1]->zombify();
   }

   if (mTextureSwapSet)
   {
      delete mTextureSwapSet;
      mTextureSwapSet = NULL;
   }

   mStereoRT = NULL;
   mStereoDepthTexture = NULL;

   mEyeRT[0] = NULL;
   mEyeRT[1] = NULL;

   mRenderConfigurationDirty = true;
}
