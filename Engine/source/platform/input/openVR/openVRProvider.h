
#ifndef _OPENVRDEVICE_H_
#define _OPENVRDEVICE_H_

#include "math/mQuat.h"
#include "math/mPoint4.h"
#include "math/util/frustum.h"
#include "core/util/tSingleton.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxVertexBuffer.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTarget.h"

#include "platform/input/IInputDevice.h"
#include "platform/input/event.h"
#include "platform/output/IDisplayDevice.h"

#include <openvr.h>

class OpenVRHMDDevice;

class VRTextureSet
{
public:
	static const int TextureCount = 2;
	GFXTexHandle mTextures[2];
	U32 mIndex;

	VRTextureSet() : mIndex(0)
	{
	}

	void init(U32 width, U32 height, GFXFormat fmt, GFXTextureProfile *profile, const String &desc)
	{
		for (U32 i = 0; i < TextureCount; i++)
		{
			mTextures[i].set(width, height, fmt, profile, desc);
		}
	}

	void clear()
	{
		for (U32 i = 0; i < TextureCount; i++)
		{
			mTextures[i] = NULL;
		}
	}

	void advance()
	{
		mIndex = (mIndex + 1) % TextureCount;
	}

	GFXTexHandle& getTextureHandle()
	{
		return mTextures[mIndex];
	}
};

struct OpenVRRenderState
{
   vr::IVRSystem *mHMD;

   FovPort mEyeFov[2];
   MatrixF mEyePose[2];
   MatrixF mHMDPose;

   RectI mEyeViewport[2];
   GFXTextureTargetRef mStereoRT;
   GFXTextureTargetRef mEyeRT[2];

   GFXTexHandle mStereoRenderTextures[2];
   GFXTexHandle mStereoDepthTextures[2];

   GFXVertexBufferHandle<GFXVertexPTTT> mDistortionVerts;
   GFXPrimitiveBufferHandle mDistortionInds;

   VRTextureSet mOutputEyeTextures[2];

   bool setupRenderTargets(U32 mode);
   void setupDistortion();

   void renderDistortion(U32 eye);

   void renderPreview();

   void reset(vr::IVRSystem* hmd);
};

class OpenVRProvider : public IDisplayDevice, public IInputDevice
{
public:

   enum DataDifferences {
      DIFF_NONE = 0,
      DIFF_ROT = (1 << 0),
      DIFF_ROTAXISX = (1 << 1),
      DIFF_ROTAXISY = (1 << 2),
      DIFF_ACCEL = (1 << 3),
      DIFF_ANGVEL = (1 << 4),
      DIFF_MAG = (1 << 5),
      DIFF_POS = (1 << 6),
      DIFF_STATUS = (1 << 7),

      DIFF_ROTAXIS = (DIFF_ROTAXISX | DIFF_ROTAXISY),
      DIFF_RAW = (DIFF_ACCEL | DIFF_ANGVEL | DIFF_MAG),
   };

   OpenVRProvider();
   ~OpenVRProvider();

   static void staticInit();

   bool enable();
   bool disable();

   bool getActive() { return mHMD != NULL; }

   /// @name Input handling
   /// {
   void buildInputCodeTable();
   virtual bool process();
   /// }

   /// @name Display handling
   /// {
   virtual bool providesFrameEyePose() const;
   virtual void getFrameEyePose(IDevicePose *pose, U32 eye) const;

   virtual bool providesEyeOffsets() const;
   /// Returns eye offset not taking into account any position tracking info
   virtual void getEyeOffsets(Point3F *dest) const;

   virtual bool providesFovPorts() const;
   virtual void getFovPorts(FovPort *out) const;

   virtual bool providesProjectionOffset() const;
   virtual const Point2F& getProjectionOffset() const;

   virtual void getStereoViewports(RectI *out) const;
   virtual void getStereoTargets(GFXTextureTarget **out) const;

   virtual void setDrawCanvas(GuiCanvas *canvas);

   virtual void setCurrentConnection(GameConnection *connection);
   virtual GameConnection* getCurrentConnection();

   virtual GFXTexHandle getPreviewTexture();

   virtual void onStartFrame();
   virtual void onEndFrame();

   virtual void onEyeRendered(U32 index);

   bool _handleDeviceEvent(GFXDevice::GFXDeviceEventType evt);

   S32 getDisplayDeviceId() const;
   /// }

   /// @name OpenVR handling
   /// {
   void processVREvent(const vr::VREvent_t & event);

   void updateTrackedPoses();
   void submitInputChanges();

   void resetSensors();
   /// }

   /// @name OpenVR state
   /// {
   vr::IVRSystem *mHMD;
   vr::IVRRenderModels *mRenderModels;
   String mDriver;
   String mDisplay;
   vr::TrackedDevicePose_t mTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
   IDevicePose mCurrentDevicePose[vr::k_unMaxTrackedDeviceCount];
   IDevicePose mPreviousInputTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
   U32 mValidPoseCount;

   char mDeviceClassChar[vr::k_unMaxTrackedDeviceCount];

   OpenVRRenderState mHMDRenderState;
   GFXAdapterLUID mLUID;
   /// }

   GuiCanvas* mDrawCanvas;
   GameConnection* mGameConnection;

   static U32 OVR_SENSORROT[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_SENSORROTANG[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_SENSORVELOCITY[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_SENSORANGVEL[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_SENSORMAGNETOMETER[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_SENSORPOSITION[vr::k_unMaxTrackedDeviceCount];

   static U32 OVR_BUTTONPRESSED[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_BUTTONTOUCHED[vr::k_unMaxTrackedDeviceCount];

   static U32 OVR_AXISNONE[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_AXISTRACKPAD[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_AXISJOYSTICK[vr::k_unMaxTrackedDeviceCount];
   static U32 OVR_AXISTRIGGER[vr::k_unMaxTrackedDeviceCount];


public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "OpenVRProvider"; }
};

/// Returns the OculusVRDevice singleton.
#define OCULUSVRDEV ManagedSingleton<OpenVRProvider>::instance()

#endif   // _OCULUSVRDEVICE_H_
