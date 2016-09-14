
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
class OpenVROverlay;
class BaseMatInstance;
class SceneRenderState;
struct MeshRenderInst;
class Namespace;
class NamedTexTarget;

typedef vr::VROverlayInputMethod OpenVROverlayInputMethod;
typedef vr::VROverlayTransformType OpenVROverlayTransformType;
typedef vr::EGamepadTextInputMode OpenVRGamepadTextInputMode;
typedef vr::EGamepadTextInputLineMode OpenVRGamepadTextInputLineMode;
typedef vr::ETrackingResult OpenVRTrackingResult;
typedef vr::ETrackingUniverseOrigin OpenVRTrackingUniverseOrigin;
typedef vr::EOverlayDirection OpenVROverlayDirection;
typedef vr::EVRState OpenVRState;
typedef vr::TrackedDeviceClass OpenVRTrackedDeviceClass;

DefineEnumType(OpenVROverlayInputMethod);
DefineEnumType(OpenVROverlayTransformType);
DefineEnumType(OpenVRGamepadTextInputMode);
DefineEnumType(OpenVRGamepadTextInputLineMode);
DefineEnumType(OpenVRTrackingResult);
DefineEnumType(OpenVRTrackingUniverseOrigin);
DefineEnumType(OpenVROverlayDirection);
DefineEnumType(OpenVRState);
DefineEnumType(OpenVRTrackedDeviceClass);

namespace OpenVRUtil
{
   /// Convert a matrix in OVR space to torque space
   void convertTransformFromOVR(const MatrixF &inRotTMat, MatrixF& outRotation);

   /// Convert a matrix in torque space to OVR space
   void convertTransformToOVR(const MatrixF& inRotation, MatrixF& outRotation);

   /// Converts vr::HmdMatrix34_t to a MatrixF
   MatrixF convertSteamVRAffineMatrixToMatrixFPlain(const vr::HmdMatrix34_t &mat);

   /// Converts a MatrixF to a vr::HmdMatrix34_t
   void convertMatrixFPlainToSteamVRAffineMatrix(const MatrixF &inMat, vr::HmdMatrix34_t &outMat);

   U32 convertOpenVRButtonToTorqueButton(uint32_t vrButton);

   /// Converts a point to OVR coords
   inline Point3F convertPointToOVR(const Point3F &point)
   {
      return Point3F(-point.x, -point.z, point.y);
   }

   /// Converts a point from OVR coords
   inline Point3F convertPointFromOVR(const Point3F &point)
   {
      return Point3F(-point.x, point.z, -point.y);
   }

   // Converts a point from OVR coords, from an input float array
   inline Point3F convertPointFromOVR(const vr::HmdVector3_t& v)
   {
      return Point3F(-v.v[0], v.v[2], -v.v[1]);
   }
};

template<int TEXSIZE> class VRTextureSet
{
public:
   static const int TextureCount = TEXSIZE;
   GFXTexHandle mTextures[TEXSIZE];
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

/// Simple class to handle rendering native OpenVR model data
class OpenVRRenderModel
{
public:
   typedef GFXVertexPNT VertexType;
   GFXVertexBufferHandle<VertexType> mVertexBuffer;
   GFXPrimitiveBufferHandle mPrimitiveBuffer;
   BaseMatInstance* mMaterialInstance; ///< Material to use for rendering. NOTE:  
   Box3F mLocalBox;

   OpenVRRenderModel() : mMaterialInstance(NULL)
   {
   }

   ~OpenVRRenderModel()
   {
      SAFE_DELETE(mMaterialInstance);
   }

   Box3F getWorldBox(MatrixF &mat)
   {
      Box3F ret = mLocalBox;
      mat.mul(ret);
      return ret;
   }

   bool init(const vr::RenderModel_t & vrModel, StringTableEntry materialName);
   void draw(SceneRenderState *state, MeshRenderInst* renderInstance);
};

struct OpenVRRenderState
{
   vr::IVRSystem *mHMD;

   FovPort mEyeFov[2];
   MatrixF mEyePose[2];
   MatrixF mHMDPose;

   RectI mEyeViewport[2];
   GFXTextureTargetRef mStereoRT;

   GFXTexHandle mStereoRenderTexture;
   GFXTexHandle mStereoDepthTexture;

   VRTextureSet<4> mOutputEyeTextures;

   GFXDevice::GFXDeviceRenderStyles mRenderMode;

   bool setupRenderTargets(GFXDevice::GFXDeviceRenderStyles mode);

   void renderPreview();

   void reset(vr::IVRSystem* hmd);
   void updateHMDProjection();
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

   struct LoadedRenderModel
   {
      StringTableEntry name;
      vr::RenderModel_t *vrModel;
      OpenVRRenderModel *model;
      vr::EVRRenderModelError modelError;
      S32 textureId;
      bool loadedTexture;
   };

   struct LoadedRenderTexture
   {
      U32 vrTextureId;
      vr::RenderModel_TextureMap_t *vrTexture;
      GFXTextureObject *texture;
      NamedTexTarget *targetTexture;
      vr::EVRRenderModelError textureError;
   };

   OpenVRProvider();
   ~OpenVRProvider();

   typedef Signal <void(const vr::VREvent_t &evt)> VREventSignal;
   VREventSignal& getVREventSignal() { return mVREventSignal;  }

   static void staticInit();

   bool enable();
   bool disable();

   bool getActive() { return mHMD != NULL; }
   inline vr::IVRRenderModels* getRenderModels() { return mRenderModels; }

   /// @name Input handling
   /// {
   void buildInputCodeTable();
   virtual bool process();
   /// }

   /// @name Display handling
   /// {
   virtual bool providesFrameEyePose() const;
   virtual void getFrameEyePose(IDevicePose *pose, S32 eyeId) const;

   virtual bool providesEyeOffsets() const;
   /// Returns eye offset not taking into account any position tracking info
   virtual void getEyeOffsets(Point3F *dest) const;

   virtual bool providesFovPorts() const;
   virtual void getFovPorts(FovPort *out) const;

   virtual void getStereoViewports(RectI *out) const;
   virtual void getStereoTargets(GFXTextureTarget **out) const;

   virtual void setDrawCanvas(GuiCanvas *canvas);
   virtual void setDrawMode(GFXDevice::GFXDeviceRenderStyles style);

   virtual void setCurrentConnection(GameConnection *connection);
   virtual GameConnection* getCurrentConnection();

   virtual GFXTexHandle getPreviewTexture();

   virtual void onStartFrame();
   virtual void onEndFrame();

   virtual void onEyeRendered(U32 index);

   virtual void setRoomTracking(bool room);

   bool _handleDeviceEvent(GFXDevice::GFXDeviceEventType evt);

   S32 getDisplayDeviceId() const;
   /// }

   /// @name OpenVR handling
   /// {
   void processVREvent(const vr::VREvent_t & event);

   void updateTrackedPoses();
   void submitInputChanges();

   void resetSensors();

   void mapDeviceToEvent(U32 deviceIdx, S32 eventIdx);
   void resetEventMap();

   IDevicePose getTrackedDevicePose(U32 idx);
   /// }

   /// @name Overlay registration
   /// {
   void registerOverlay(OpenVROverlay* overlay);
   void unregisterOverlay(OpenVROverlay* overlay);
   /// }

   /// @name Model loading
   /// {
   const S32 preloadRenderModel(StringTableEntry name);
   const S32 preloadRenderModelTexture(U32 index);
   bool getRenderModel(S32 idx, OpenVRRenderModel **ret, bool &failed);
   bool getRenderModelTexture(S32 idx, GFXTextureObject **outTex, bool &failed);
   bool getRenderModelTextureName(S32 idx, String &outName);
   void resetRenderModels();
   /// }


   /// @name Console API
   /// {
   OpenVROverlay *getGamepadFocusOverlay();
   void setOverlayNeighbour(vr::EOverlayDirection dir, OpenVROverlay *overlay);

   bool isDashboardVisible();
   void showDashboard(const char *overlayToShow);

   vr::TrackedDeviceIndex_t getPrimaryDashboardDevice();

   void setKeyboardTransformAbsolute(const MatrixF &xfm);
   void setKeyboardPositionForOverlay(OpenVROverlay *overlay, const RectI &rect);

   void getControllerDeviceIndexes(vr::TrackedDeviceClass &deviceClass, Vector<S32> &outList);
   StringTableEntry getControllerModel(U32 idx);
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

   vr::VRControllerState_t mCurrentControllerState[vr::k_unMaxTrackedDeviceCount];
   vr::VRControllerState_t mPreviousCurrentControllerState[vr::k_unMaxTrackedDeviceCount];

   char mDeviceClassChar[vr::k_unMaxTrackedDeviceCount];

   OpenVRRenderState mHMDRenderState;
   GFXAdapterLUID mLUID;

   vr::ETrackingUniverseOrigin mTrackingSpace;

   Vector<OpenVROverlay*> mOverlays;

   VREventSignal mVREventSignal;
   Namespace *mOpenVRNS;

   Vector<LoadedRenderModel> mLoadedModels;
   Vector<LoadedRenderTexture> mLoadedTextures;
   Map<StringTableEntry, S32> mLoadedModelLookup;
   Map<U32, S32> mLoadedTextureLookup;

   Map<U32, S32> mDeviceEventMap;
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

   /// @name HMD Rotation offset
   /// {
   static EulerF smHMDRotOffset;
   static F32 smHMDmvYaw;
   static F32 smHMDmvPitch;
   static bool smRotateYawWithMoveActions;
   /// }

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "OpenVRProvider"; }
};

/// Returns the OculusVRDevice singleton.
#define OPENVR ManagedSingleton<OpenVRProvider>::instance()

#endif   // _OCULUSVRDEVICE_H_
