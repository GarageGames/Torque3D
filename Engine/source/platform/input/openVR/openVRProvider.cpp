#include "platform/input/openVR/openVRProvider.h"
#include "platform/input/openVR/openVROverlay.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/core/guiCanvas.h"
#include "postFx/postEffectCommon.h"
#include "renderInstance/renderPassManager.h"
#include "scene/sceneRenderState.h"
#include "materials/baseMatInstance.h"
#include "materials/materialManager.h"
#include "console/consoleInternal.h"
#include "core/stream/fileStream.h"

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/gfxStringEnumTranslate.h"


#include "gfx/D3D9/gfxD3D9Device.h"
#include "gfx/D3D9/gfxD3D9TextureObject.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"

#include "materials/matTextureTarget.h"

#ifdef TORQUE_OPENGL
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#endif

struct OpenVRLoadedTexture
{
   vr::TextureID_t texId;
   NamedTexTarget texTarget;
};

AngAxisF gLastMoveRot; // jamesu - this is just here for temp debugging

namespace OpenVRUtil
{
   void convertTransformFromOVR(const MatrixF &inRotTMat, MatrixF& outRotation)
   {
      Point4F col0; inRotTMat.getColumn(0, &col0);
      Point4F col1; inRotTMat.getColumn(1, &col1);
      Point4F col2; inRotTMat.getColumn(2, &col2);
      Point4F col3; inRotTMat.getColumn(3, &col3);

      // Set rotation.  We need to convert from sensor coordinates to
      // Torque coordinates.  The sensor matrix is stored row-major.
      // The conversion is:
      //
      // Sensor                       Torque
      // a b c         a  b  c        a -c  b
      // d e f   -->  -g -h -i  -->  -g  i -h
      // g h i         d  e  f        d -f  e
      outRotation.setColumn(0, Point4F( col0.x, -col2.x, col1.x, 0.0f));
      outRotation.setColumn(1, Point4F(-col0.z, col2.z, -col1.z, 0.0f));
      outRotation.setColumn(2, Point4F( col0.y, -col2.y, col1.y, 0.0f));
      outRotation.setColumn(3, Point4F(-col3.x, col3.z, -col3.y, 1.0f));
   }

   void convertTransformToOVR(const MatrixF& inRotation, MatrixF& outRotation)
   {
      Point4F col0; inRotation.getColumn(0, &col0);
      Point4F col1; inRotation.getColumn(1, &col1);
      Point4F col2; inRotation.getColumn(2, &col2);
      Point4F col3; inRotation.getColumn(3, &col3);

      // This is basically a reverse of what is in convertTransformFromOVR
      outRotation.setColumn(0, Point4F(col0.x, col2.x, -col1.x, 0.0f));
      outRotation.setColumn(1, Point4F(col0.z, col2.z, -col1.z, 0.0f));
      outRotation.setColumn(2, Point4F(-col0.y, -col2.y, col1.y, 0.0f));
      outRotation.setColumn(3, Point4F(-col3.x, -col3.z, col3.y, 1.0f));
   }

   MatrixF convertSteamVRAffineMatrixToMatrixFPlain(const vr::HmdMatrix34_t &mat)
   {
      MatrixF outMat(1);

      outMat.setColumn(0, Point4F(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0));
      outMat.setColumn(1, Point4F(mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0));
      outMat.setColumn(2, Point4F(mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0));
      outMat.setColumn(3, Point4F(mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f)); // pos

      return outMat;
   }



   void convertMatrixFPlainToSteamVRAffineMatrix(const MatrixF &inMat, vr::HmdMatrix34_t &outMat)
   {
      Point4F row0; inMat.getRow(0, &row0);
      Point4F row1; inMat.getRow(1, &row1);
      Point4F row2; inMat.getRow(2, &row2);

      outMat.m[0][0] = row0.x;
      outMat.m[0][1] = row0.y;
      outMat.m[0][2] = row0.z;
      outMat.m[0][3] = row0.w;

      outMat.m[1][0] = row1.x;
      outMat.m[1][1] = row1.y;
      outMat.m[1][2] = row1.z;
      outMat.m[1][3] = row1.w;

      outMat.m[2][0] = row2.x;
      outMat.m[2][1] = row2.y;
      outMat.m[2][2] = row2.z;
      outMat.m[2][3] = row2.w;
   }

   U32 convertOpenVRButtonToTorqueButton(uint32_t vrButton)
   {
      switch (vrButton)
      {
      case vr::VRMouseButton_Left:
         return KEY_BUTTON0;
      case vr::VRMouseButton_Right:
         return KEY_BUTTON1;
      case vr::VRMouseButton_Middle:
         return KEY_BUTTON2;
      default:
         return KEY_NULL;
      }
   }


   vr::VRTextureBounds_t TorqueRectToBounds(const RectI &rect, const Point2I &widthHeight)
   {
      vr::VRTextureBounds_t bounds;
      F32 xRatio = 1.0 / (F32)widthHeight.x;
      F32 yRatio = 1.0 / (F32)widthHeight.y;
      bounds.uMin = rect.point.x * xRatio;
      bounds.vMin = rect.point.y * yRatio;
      bounds.uMax = (rect.point.x + rect.extent.x) * xRatio;
      bounds.vMax = (rect.point.y + rect.extent.y) * yRatio;
      return bounds;
   }

   String GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
   {
      uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
      if (unRequiredBufferLen == 0)
         return "";

      char *pchBuffer = new char[unRequiredBufferLen];
      unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
      String sResult = pchBuffer;
      delete[] pchBuffer;
      return sResult;
   }

}

//------------------------------------------------------------

bool OpenVRRenderModel::init(const vr::RenderModel_t & vrModel, StringTableEntry materialName)
{
   SAFE_DELETE(mMaterialInstance);
   mMaterialInstance = MATMGR->createMatInstance(materialName, getGFXVertexFormat< VertexType >());
   if (!mMaterialInstance)
      return false;

   mLocalBox = Box3F::Invalid;

   // Prepare primitives
   U16 *indPtr = NULL;
   GFXPrimitive *primPtr = NULL;
   mPrimitiveBuffer.set(GFX, vrModel.unTriangleCount * 3, 1, GFXBufferTypeStatic, "OpenVR Controller buffer");

   mPrimitiveBuffer.lock(&indPtr, &primPtr);
   if (!indPtr || !primPtr)
      return false;

   primPtr->minIndex = 0;
   primPtr->numPrimitives = vrModel.unTriangleCount;
   primPtr->numVertices = vrModel.unVertexCount;
   primPtr->startIndex = 0;
   primPtr->startVertex = 0;
   primPtr->type = GFXTriangleList;

   //dMemcpy(indPtr, vrModel.rIndexData, sizeof(U16) * vrModel.unTriangleCount * 3);

   for (U32 i = 0; i < vrModel.unTriangleCount; i++)
   {
      const U32 idx = i * 3;
      indPtr[idx + 0] = vrModel.rIndexData[idx + 2];
      indPtr[idx + 1] = vrModel.rIndexData[idx + 1];
      indPtr[idx + 2] = vrModel.rIndexData[idx + 0];
   }

   mPrimitiveBuffer.unlock();

   // Prepare verts
   mVertexBuffer.set(GFX, vrModel.unVertexCount, GFXBufferTypeStatic);
   VertexType *vertPtr = mVertexBuffer.lock();
   if (!vertPtr)
      return false;

   // Convert to torque coordinate system
   for (U32 i = 0; i < vrModel.unVertexCount; i++)
   {
      const vr::RenderModel_Vertex_t &vert = vrModel.rVertexData[i];
      vertPtr->point = OpenVRUtil::convertPointFromOVR(vert.vPosition);
      vertPtr->point.x = -vertPtr->point.x;
      vertPtr->point.y = -vertPtr->point.y;
      vertPtr->point.z = -vertPtr->point.z;
      vertPtr->normal = OpenVRUtil::convertPointFromOVR(vert.vNormal);
      vertPtr->normal.x = -vertPtr->normal.x;
      vertPtr->normal.y = -vertPtr->normal.y;
      vertPtr->normal.z = -vertPtr->normal.z;
      vertPtr->texCoord = Point2F(vert.rfTextureCoord[0], vert.rfTextureCoord[1]);
      vertPtr++;
   }

   mVertexBuffer.unlock();

   for (U32 i = 0, sz = vrModel.unVertexCount; i < sz; i++)
   {
      Point3F pos = Point3F(vrModel.rVertexData[i].vPosition.v[0], vrModel.rVertexData[i].vPosition.v[1], vrModel.rVertexData[i].vPosition.v[2]);
      mLocalBox.extend(pos);
   }

   return true;
}

void OpenVRRenderModel::draw(SceneRenderState *state, MeshRenderInst* renderInstance)
{
   renderInstance->type = RenderPassManager::RIT_Mesh;
   renderInstance->matInst = state->getOverrideMaterial(mMaterialInstance);
   if (!renderInstance->matInst)
      return;

   renderInstance->vertBuff = &mVertexBuffer;
   renderInstance->primBuff = &mPrimitiveBuffer;
   renderInstance->prim = NULL;
   renderInstance->primBuffIndex = 0;

   if (renderInstance->matInst->getMaterial()->isTranslucent())
   {
      renderInstance->type = RenderPassManager::RIT_Translucent;
      renderInstance->translucentSort = true;
   }

   renderInstance->defaultKey = renderInstance->matInst->getStateHint();
   renderInstance->defaultKey2 = (uintptr_t)renderInstance->vertBuff;
}

//------------------------------------------------------------



DECLARE_SCOPE(OpenVR);
IMPLEMENT_SCOPE(OpenVR, OpenVRProvider, , "");
ConsoleDoc(
   "@class OpenVRProvider\n"
   "@brief This class is the interface between TorqueScript and OpenVR.\n\n"
   "@ingroup OpenVR\n"
   );

// Enum impls

ImplementEnumType(OpenVROverlayInputMethod,
   "Types of input supported by VR Overlays. .\n\n"
   "@ingroup OpenVR")
{ vr::VROverlayInputMethod_None, "None" },
{ vr::VROverlayInputMethod_Mouse, "Mouse" },
EndImplementEnumType;

ImplementEnumType(OpenVROverlayTransformType,
   "Allows the caller to figure out which overlay transform getter to call. .\n\n"
   "@ingroup OpenVR")
{ vr::VROverlayTransform_Absolute, "Absolute" },
{ vr::VROverlayTransform_TrackedDeviceRelative, "TrackedDeviceRelative" },
{ vr::VROverlayTransform_SystemOverlay, "SystemOverlay" },
{ vr::VROverlayTransform_TrackedComponent, "TrackedComponent" },
EndImplementEnumType;

ImplementEnumType(OpenVRGamepadTextInputMode,
   "Types of input supported by VR Overlays. .\n\n"
   "@ingroup OpenVR")
{ vr::k_EGamepadTextInputModeNormal, "Normal", },
{ vr::k_EGamepadTextInputModePassword, "Password", },
{ vr::k_EGamepadTextInputModeSubmit, "Submit" },
EndImplementEnumType;

ImplementEnumType(OpenVRGamepadTextInputLineMode,
   "Types of input supported by VR Overlays. .\n\n"
   "@ingroup OpenVR")
{ vr::k_EGamepadTextInputLineModeSingleLine, "SingleLine" },
{ vr::k_EGamepadTextInputLineModeMultipleLines, "MultipleLines" },
EndImplementEnumType;

ImplementEnumType(OpenVRTrackingResult,
   ". .\n\n"
   "@ingroup OpenVR")
{ vr::TrackingResult_Uninitialized, "None" },
{ vr::TrackingResult_Calibrating_InProgress, "Calibrating_InProgress" },
{ vr::TrackingResult_Calibrating_OutOfRange, "Calibrating_OutOfRange" },
{ vr::TrackingResult_Running_OK, "Running_Ok" },
{ vr::TrackingResult_Running_OutOfRange, "Running_OutOfRange" },
EndImplementEnumType;

ImplementEnumType(OpenVRTrackingUniverseOrigin,
   "Identifies which style of tracking origin the application wants to use for the poses it is requesting. .\n\n"
   "@ingroup OpenVR")
{ vr::TrackingUniverseSeated, "Seated" },
{ vr::TrackingUniverseStanding, "Standing" },
{ vr::TrackingUniverseRawAndUncalibrated, "RawAndUncalibrated" },
EndImplementEnumType;

ImplementEnumType(OpenVROverlayDirection,
   "Directions for changing focus between overlays with the gamepad. .\n\n"
   "@ingroup OpenVR")
{ vr::OverlayDirection_Up, "Up" },
{ vr::OverlayDirection_Down, "Down" },
{ vr::OverlayDirection_Left, "Left" },
{ vr::OverlayDirection_Right, "Right" },
EndImplementEnumType;

ImplementEnumType(OpenVRState,
   "Status of the overall system or tracked objects. .\n\n"
   "@ingroup OpenVR")
{ vr::VRState_Undefined, "Undefined" },
{ vr::VRState_Off, "Off" },
{ vr::VRState_Searching, "Searching" },
{ vr::VRState_Searching_Alert, "Searching_Alert" },
{ vr::VRState_Ready, "Ready" },
{ vr::VRState_Ready_Alert, "Ready_Alert" },
{ vr::VRState_NotReady, "NotReady" },
EndImplementEnumType;

ImplementEnumType(OpenVRTrackedDeviceClass,
   "Types of devices which are tracked .\n\n"
   "@ingroup OpenVR")
{ vr::TrackedDeviceClass_Invalid, "Invalid" },
{ vr::TrackedDeviceClass_HMD, "HMD" },
{ vr::TrackedDeviceClass_Controller, "Controller" },
{ vr::TrackedDeviceClass_TrackingReference, "TrackingReference" },
{ vr::TrackedDeviceClass_Other, "Other" },
EndImplementEnumType;

//------------------------------------------------------------

U32 OpenVRProvider::OVR_SENSORROT[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_SENSORROTANG[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_SENSORVELOCITY[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_SENSORANGVEL[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_SENSORMAGNETOMETER[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_SENSORPOSITION[vr::k_unMaxTrackedDeviceCount] = { 0 };

U32 OpenVRProvider::OVR_BUTTONPRESSED[vr::k_unMaxTrackedDeviceCount];
U32 OpenVRProvider::OVR_BUTTONTOUCHED[vr::k_unMaxTrackedDeviceCount];

U32 OpenVRProvider::OVR_AXISNONE[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_AXISTRACKPAD[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_AXISJOYSTICK[vr::k_unMaxTrackedDeviceCount] = { 0 };
U32 OpenVRProvider::OVR_AXISTRIGGER[vr::k_unMaxTrackedDeviceCount] = { 0 };

EulerF OpenVRProvider::smHMDRotOffset(0);
F32 OpenVRProvider::smHMDmvYaw = 0;
F32 OpenVRProvider::smHMDmvPitch = 0;
bool OpenVRProvider::smRotateYawWithMoveActions = false;

static String GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
   uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
   if (unRequiredBufferLen == 0)
      return "";

   char *pchBuffer = new char[unRequiredBufferLen];
   unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
   String sResult = pchBuffer;
   delete[] pchBuffer;
   return sResult;
}

MODULE_BEGIN(OpenVRProvider)

MODULE_INIT_AFTER(InputEventManager)
MODULE_SHUTDOWN_BEFORE(InputEventManager)

MODULE_INIT
{
   OpenVRProvider::staticInit();
   ManagedSingleton< OpenVRProvider >::createSingleton();
}

MODULE_SHUTDOWN
{
   ManagedSingleton< OpenVRProvider >::deleteSingleton();
}

MODULE_END;


bool OpenVRRenderState::setupRenderTargets(GFXDevice::GFXDeviceRenderStyles mode)
{
   if (!mHMD)
      return false;

   if (mRenderMode == mode)
      return true;

   mRenderMode = mode;

   if (mode == GFXDevice::RS_Standard)
   {
      reset(mHMD);
      return true;
   }

   U32 sizeX, sizeY;
   Point2I newRTSize;
   mHMD->GetRecommendedRenderTargetSize(&sizeX, &sizeY);

   if (mode == GFXDevice::RS_StereoSeparate)
   {
      mEyeViewport[0] = RectI(Point2I(0, 0), Point2I(sizeX, sizeY));
      mEyeViewport[1] = RectI(Point2I(0, 0), Point2I(sizeX, sizeY));

      newRTSize.x = sizeX;
      newRTSize.y = sizeY;
   }
   else
   {
      mEyeViewport[0] = RectI(Point2I(0, 0), Point2I(sizeX, sizeY));
      mEyeViewport[1] = RectI(Point2I(sizeX, 0), Point2I(sizeX, sizeY));

      newRTSize.x = sizeX * 2;
      newRTSize.y = sizeY;
   }

   GFXTexHandle stereoTexture;
   stereoTexture.set(newRTSize.x, newRTSize.y, GFXFormatR8G8B8A8, &VRTextureProfile, "OpenVR Stereo RT Color");
   mStereoRenderTexture = stereoTexture;

   GFXTexHandle stereoDepthTexture;
   stereoDepthTexture.set(newRTSize.x, newRTSize.y, GFXFormatD24S8, &VRDepthProfile, "OpenVR Depth");
   mStereoDepthTexture = stereoDepthTexture;

   mStereoRT = GFX->allocRenderToTextureTarget();
   mStereoRT->attachTexture(GFXTextureTarget::Color0, stereoTexture);
   mStereoRT->attachTexture(GFXTextureTarget::DepthStencil, stereoDepthTexture);

   mOutputEyeTextures.init(newRTSize.x, newRTSize.y, GFXFormatR8G8B8A8, &VRTextureProfile, "OpenVR Stereo RT Color OUTPUT");

   return true;
}

void OpenVRRenderState::renderPreview()
{

}

void OpenVRRenderState::reset(vr::IVRSystem* hmd)
{
   mHMD = hmd;

   mStereoRT = NULL;

   mStereoRenderTexture = NULL;
   mStereoDepthTexture = NULL;

   mOutputEyeTextures.clear();

   if (!mHMD)
      return;

   updateHMDProjection();
}

void OpenVRRenderState::updateHMDProjection()
{
   vr::HmdMatrix34_t mat = mHMD->GetEyeToHeadTransform(vr::Eye_Left);
   mEyePose[0] = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(mat);
   mEyePose[0].inverse();

   mat = mHMD->GetEyeToHeadTransform(vr::Eye_Right);
   mEyePose[1] = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(mat);
   mEyePose[1].inverse();

   mHMD->GetProjectionRaw(vr::Eye_Left, &mEyeFov[0].leftTan, &mEyeFov[0].rightTan, &mEyeFov[0].upTan, &mEyeFov[0].downTan);
   mHMD->GetProjectionRaw(vr::Eye_Right, &mEyeFov[1].leftTan, &mEyeFov[1].rightTan, &mEyeFov[1].upTan, &mEyeFov[1].downTan);

   mEyeFov[0].upTan = -mEyeFov[0].upTan;
   mEyeFov[0].leftTan = -mEyeFov[0].leftTan;
   mEyeFov[1].upTan = -mEyeFov[1].upTan;
   mEyeFov[1].leftTan = -mEyeFov[1].leftTan;
}

OpenVRProvider::OpenVRProvider() :
   mHMD(NULL),
   mRenderModels(NULL),
   mDrawCanvas(NULL),
   mGameConnection(NULL)
{
   dStrcpy(mName, "openvr");
   mDeviceType = INPUTMGR->getNextDeviceType();
   buildInputCodeTable();
   GFXDevice::getDeviceEventSignal().notify(this, &OpenVRProvider::_handleDeviceEvent);
   INPUTMGR->registerDevice(this);
   dMemset(&mLUID, '\0', sizeof(mLUID));

   mTrackingSpace = vr::TrackingUniverseStanding;
}

OpenVRProvider::~OpenVRProvider()
{

}

void OpenVRProvider::staticInit()
{
   // Overlay flags
   Con::setIntVariable("$OpenVR::OverlayFlags_None", 1 << (U32)vr::VROverlayFlags_None);
   Con::setIntVariable("$OpenVR::OverlayFlags_Curved", 1 << (U32)vr::VROverlayFlags_Curved);
   Con::setIntVariable("$OpenVR::OverlayFlags_RGSS4X", 1 << (U32)vr::VROverlayFlags_RGSS4X);
   Con::setIntVariable("$OpenVR::OverlayFlags_NoDashboardTab", 1 << (U32)vr::VROverlayFlags_NoDashboardTab);
   Con::setIntVariable("$OpenVR::OverlayFlags_AcceptsGamepadEvents", 1 << (U32)vr::VROverlayFlags_AcceptsGamepadEvents);
   Con::setIntVariable("$OpenVR::OverlayFlags_ShowGamepadFocus", 1 << (U32)vr::VROverlayFlags_ShowGamepadFocus);
   Con::setIntVariable("$OpenVR::OverlayFlags_SendVRScrollEvents", 1 << (U32)vr::VROverlayFlags_SendVRScrollEvents);
   Con::setIntVariable("$OpenVR::OverlayFlags_SendVRTouchpadEvents", 1 << (U32)vr::VROverlayFlags_SendVRTouchpadEvents);
   Con::setIntVariable("$OpenVR::OverlayFlags_ShowTouchPadScrollWheel", 1 << (U32)vr::VROverlayFlags_ShowTouchPadScrollWheel);

   Con::addVariable("$OpenVR::HMDRotOffsetX", TypeF32, &smHMDRotOffset.x);
   Con::addVariable("$OpenVR::HMDRotOffsetY", TypeF32, &smHMDRotOffset.y);
   Con::addVariable("$OpenVR::HMDRotOffsetZ", TypeF32, &smHMDRotOffset.z);

   Con::addVariable("$OpenVR::HMDmvYaw", TypeF32, &smHMDmvYaw);
   Con::addVariable("$OpenVR::HMDmvPitch", TypeF32, &smHMDmvPitch);

   Con::addVariable("$OpenVR::HMDRotateYawWithMoveActions", TypeBool, &smRotateYawWithMoveActions);
}

bool OpenVRProvider::enable()
{
   mOpenVRNS = Namespace::find(StringTable->insert("OpenVR"));

   disable();

   // Load openvr runtime
   vr::EVRInitError eError = vr::VRInitError_None;
   mHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

   dMemset(mDeviceClassChar, '\0', sizeof(mDeviceClassChar));

   if (eError != vr::VRInitError_None)
   {
      mHMD = NULL;
      char buf[1024];
      sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
      Con::printf(buf);
      return false;
   }

   dMemset(&mLUID, '\0', sizeof(mLUID));

#ifdef TORQUE_OS_WIN32

   // For windows we need to lookup the DXGI record for this and grab the LUID for the display adapter. We need the LUID since 
   // T3D uses EnumAdapters1 not EnumAdapters whereas openvr uses EnumAdapters.
   int32_t AdapterIdx;
   IDXGIAdapter* EnumAdapter;
   IDXGIFactory1* DXGIFactory;
   mHMD->GetDXGIOutputInfo(&AdapterIdx);
   // Get the LUID of the device

   HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&DXGIFactory));

   if (FAILED(hr))
      AssertFatal(false, "OpenVRProvider::enable -> CreateDXGIFactory1 call failure");

   hr = DXGIFactory->EnumAdapters(AdapterIdx, &EnumAdapter);

   if (FAILED(hr))
   {
      Con::warnf("VR: HMD device has an invalid adapter.");
   }
   else
   {
      DXGI_ADAPTER_DESC desc;
      hr = EnumAdapter->GetDesc(&desc);
      if (FAILED(hr))
      {
         Con::warnf("VR: HMD device has an invalid adapter.");
      }
      else
      {
         dMemcpy(&mLUID, &desc.AdapterLuid, sizeof(mLUID));
      }
      SAFE_RELEASE(EnumAdapter);
   }

   SAFE_RELEASE(DXGIFactory);
#endif



   mRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
   if (!mRenderModels)
   {
      mHMD = NULL;
      vr::VR_Shutdown();

      char buf[1024];
      sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
      Con::printf(buf);
      return false;
   }

   mDriver = GetTrackedDeviceString(mHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
   mDisplay = GetTrackedDeviceString(mHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

   mHMDRenderState.mHMDPose = MatrixF(1);
   mHMDRenderState.mEyePose[0] = MatrixF(1);
   mHMDRenderState.mEyePose[1] = MatrixF(1);

   mHMDRenderState.reset(mHMD);
   mHMD->ResetSeatedZeroPose();
   dMemset(mPreviousInputTrackedDevicePose, '\0', sizeof(mPreviousInputTrackedDevicePose));

   mEnabled = true;

   dMemset(mCurrentControllerState, '\0', sizeof(mCurrentControllerState));
   dMemset(mPreviousCurrentControllerState, '\0', sizeof(mPreviousCurrentControllerState));

   return true;
}

bool OpenVRProvider::disable()
{
   if (mHMD)
   {
      mHMD = NULL;
      mRenderModels = NULL;
      mHMDRenderState.reset(NULL);
      vr::VR_Shutdown();
   }

   mEnabled = false;

   return true;
}

void OpenVRProvider::buildInputCodeTable()
{
   // Obtain all of the device codes
   for (U32 i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
   {
      OVR_SENSORROT[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORROTANG[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORVELOCITY[i] = INPUTMGR->getNextDeviceCode();
      OVR_SENSORANGVEL[i] = INPUTMGR->getNextDeviceCode();
      OVR_SENSORMAGNETOMETER[i] = INPUTMGR->getNextDeviceCode();

      OVR_SENSORPOSITION[i] = INPUTMGR->getNextDeviceCode();


      OVR_BUTTONPRESSED[i] = INPUTMGR->getNextDeviceCode();
      OVR_BUTTONTOUCHED[i] = INPUTMGR->getNextDeviceCode();

      OVR_AXISNONE[i] = INPUTMGR->getNextDeviceCode();
      OVR_AXISTRACKPAD[i] = INPUTMGR->getNextDeviceCode();
      OVR_AXISJOYSTICK[i] = INPUTMGR->getNextDeviceCode();
      OVR_AXISTRIGGER[i] = INPUTMGR->getNextDeviceCode();
   }

   // Build out the virtual map
   char buffer[64];
   for (U32 i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
   {
      dSprintf(buffer, 64, "opvr_sensorrot%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_ROT, OVR_SENSORROT[i]);

      dSprintf(buffer, 64, "opvr_sensorrotang%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_SENSORROTANG[i]);

      dSprintf(buffer, 64, "opvr_sensorvelocity%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_SENSORVELOCITY[i]);

      dSprintf(buffer, 64, "opvr_sensorangvel%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_SENSORANGVEL[i]);

      dSprintf(buffer, 64, "opvr_sensormagnetometer%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_SENSORMAGNETOMETER[i]);

      dSprintf(buffer, 64, "opvr_sensorpos%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_SENSORPOSITION[i]);

      dSprintf(buffer, 64, "opvr_buttonpressed%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_INT, OVR_BUTTONPRESSED[i]);
      dSprintf(buffer, 64, "opvr_buttontouched%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_INT, OVR_BUTTONTOUCHED[i]);

      dSprintf(buffer, 64, "opvr_axis_none%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_AXISNONE[i]);
      dSprintf(buffer, 64, "opvr_axis_trackpad%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_AXISTRACKPAD[i]);
      dSprintf(buffer, 64, "opvr_axis_joystick%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_POS, OVR_AXISJOYSTICK[i]);
      dSprintf(buffer, 64, "opvr_axis_trigger%d", i);
      INPUTMGR->addVirtualMap(buffer, SI_INT, OVR_AXISTRIGGER[i]);
   }
}

bool OpenVRProvider::process()
{
   if (!mHMD)
      return true;   

   if (!vr::VRCompositor())
      return true;

   if (smRotateYawWithMoveActions)
   {
      smHMDmvYaw += MoveManager::mRightAction - MoveManager::mLeftAction + MoveManager::mXAxis_L;
   }

   // Update HMD rotation offset
   smHMDRotOffset.z += smHMDmvYaw;
   smHMDRotOffset.x += smHMDmvPitch;

   while (smHMDRotOffset.x < -M_PI_F)
      smHMDRotOffset.x += M_2PI_F;
   while (smHMDRotOffset.x > M_PI_F)
      smHMDRotOffset.x -= M_2PI_F;
   while (smHMDRotOffset.z < -M_PI_F)
      smHMDRotOffset.z += M_2PI_F;
   while (smHMDRotOffset.z > M_PI_F)
      smHMDRotOffset.z -= M_2PI_F;

   smHMDmvYaw = 0;
   smHMDmvPitch = 0;

   // Process SteamVR events
   vr::VREvent_t event;
   while (mHMD->PollNextEvent(&event, sizeof(event)))
   {
      processVREvent(event);
   }

   // process overlay events
   for (U32 i = 0; i < mOverlays.size(); i++)
   {
      mOverlays[i]->handleOpenVREvents();
   }

   // Process SteamVR controller state
   for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
   {
      vr::VRControllerState_t state;
      if (mHMD->GetControllerState(unDevice, &state))
      {
        mCurrentControllerState[unDevice] = state;
      }
   }

   // Update input poses
   updateTrackedPoses();
   submitInputChanges();

   return true;
}

bool OpenVRProvider::providesFrameEyePose() const
{
   return mHMD != NULL;
}

inline Point3F OpenVRVecToTorqueVec(vr::HmdVector3_t vec)
{
   return Point3F(-vec.v[0], vec.v[2], -vec.v[1]);
}

void OpenVRTransformToRotPos(MatrixF mat, QuatF &outRot, Point3F &outPos)
{
   // Directly set the rotation and position from the eye transforms
   MatrixF torqueMat(1);
   OpenVRUtil::convertTransformFromOVR(mat, torqueMat);

   Point3F pos = torqueMat.getPosition();
   outRot = QuatF(torqueMat);
   outPos = pos;
   outRot.mulP(pos, &outPos); // jamesu - position needs to be multiplied by rotation in this case
}

void OpenVRTransformToRotPosMat(MatrixF mat, QuatF &outRot, Point3F &outPos, MatrixF &outMat)
{
   // Directly set the rotation and position from the eye transforms
   MatrixF torqueMat(1);
   OpenVRUtil::convertTransformFromOVR(mat, torqueMat);

   Point3F pos = torqueMat.getPosition();
   outRot = QuatF(torqueMat);
   outPos = pos;
   outRot.mulP(pos, &outPos); // jamesu - position needs to be multiplied by rotation in this case
   outMat = torqueMat;
}

void OpenVRProvider::getFrameEyePose(IDevicePose *pose, S32 eyeId) const
{
   AssertFatal(eyeId >= -1 && eyeId < 2, "Out of bounds eye");

   if (eyeId == -1)
   {
      // NOTE: this is codename for "head"
      MatrixF mat = mHMDRenderState.mHMDPose; // same order as in the openvr example

#ifdef DEBUG_DISPLAY_POSE
      pose->originalMatrix = mat;
      OpenVRTransformToRotPosMat(mat, pose->orientation, pose->position, pose->actualMatrix);
#else
      OpenVRTransformToRotPos(mat, pose->orientation, pose->position);
#endif

      pose->velocity = Point3F(0);
      pose->angularVelocity = Point3F(0);
   }
   else
   {
      MatrixF mat = mHMDRenderState.mEyePose[eyeId] * mHMDRenderState.mHMDPose; // same order as in the openvr example
      //mat =  mHMDRenderState.mHMDPose * mHMDRenderState.mEyePose[eyeId]; // same order as in the openvr example


#ifdef DEBUG_DISPLAY_POSE
      pose->originalMatrix = mat;
      OpenVRTransformToRotPosMat(mat, pose->orientation, pose->position, pose->actualMatrix);
#else
      OpenVRTransformToRotPos(mat, pose->orientation, pose->position);
#endif

      pose->velocity = Point3F(0);
      pose->angularVelocity = Point3F(0);
   }
}

bool OpenVRProvider::providesEyeOffsets() const
{
   return mHMD != NULL;
}

/// Returns eye offset not taking into account any position tracking info
void OpenVRProvider::getEyeOffsets(Point3F *dest) const
{
   dest[0] = mHMDRenderState.mEyePose[0].getPosition();
   dest[1] = mHMDRenderState.mEyePose[1].getPosition();

   dest[0] = Point3F(-dest[0].x, dest[0].y, dest[0].z); // convert from vr-space
   dest[1] = Point3F(-dest[1].x, dest[1].y, dest[1].z);
}

bool OpenVRProvider::providesFovPorts() const
{
   return mHMD != NULL;
}

void OpenVRProvider::getFovPorts(FovPort *out) const
{
   dMemcpy(out, mHMDRenderState.mEyeFov, sizeof(mHMDRenderState.mEyeFov));
}

void OpenVRProvider::getStereoViewports(RectI *out) const
{
   out[0] = mHMDRenderState.mEyeViewport[0];
   out[1] = mHMDRenderState.mEyeViewport[1];
}

void OpenVRProvider::getStereoTargets(GFXTextureTarget **out) const
{
   out[0] = mHMDRenderState.mStereoRT;
   out[1] = mHMDRenderState.mStereoRT;
}

void OpenVRProvider::setDrawCanvas(GuiCanvas *canvas)
{
   vr::EVRInitError peError = vr::VRInitError_None;

   if (!vr::VRCompositor())
   {
      Con::errorf("VR: Compositor initialization failed. See log file for details\n");
      return;
   }

   if (mDrawCanvas != canvas || mHMDRenderState.mHMD == NULL)
   {
      mHMDRenderState.setupRenderTargets(GFXDevice::RS_Standard);
   }
   mDrawCanvas = canvas;
}

void OpenVRProvider::setDrawMode(GFXDevice::GFXDeviceRenderStyles style)
{
   mHMDRenderState.setupRenderTargets(style);
}

void OpenVRProvider::setCurrentConnection(GameConnection *connection)
{
   mGameConnection = connection;
}

GameConnection* OpenVRProvider::getCurrentConnection()
{
   return mGameConnection;
}

GFXTexHandle OpenVRProvider::getPreviewTexture()
{
   return mHMDRenderState.mStereoRenderTexture; // TODO: render distortion preview
}

void OpenVRProvider::onStartFrame()
{
   if (!mHMD)
      return;

}

void OpenVRProvider::onEndFrame()
{
   if (!mHMD)
      return;
}

void OpenVRProvider::onEyeRendered(U32 index)
{
   if (!mHMD)
      return;

   vr::EVRCompositorError err = vr::VRCompositorError_None;
   vr::VRTextureBounds_t bounds;
   U32 textureIdxToSubmit = index;

   GFXTexHandle eyeTex = mHMDRenderState.mOutputEyeTextures.getTextureHandle();
   if (mHMDRenderState.mRenderMode == GFXDevice::RS_StereoSeparate)
   {
      mHMDRenderState.mStereoRT->resolveTo(eyeTex);
      mHMDRenderState.mOutputEyeTextures.advance();
   }
   else
   {
      // assuming side-by-side, so the right eye will be next
      if (index == 1)
      {
         mHMDRenderState.mStereoRT->resolveTo(eyeTex);
         mHMDRenderState.mOutputEyeTextures.advance();
      }
      else
      {
         return;
      }
   }

   if (GFX->getAdapterType() == Direct3D11)
   {
      vr::Texture_t eyeTexture;
      if (mHMDRenderState.mRenderMode == GFXDevice::RS_StereoSeparate)
      {
         // whatever eye we are on
         eyeTexture = { (void*)static_cast<GFXD3D11TextureObject*>(eyeTex.getPointer())->get2DTex(), vr::API_DirectX, vr::ColorSpace_Gamma };
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[index], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture, &bounds);
      }
      else
      {
         // left & right at the same time
         eyeTexture = { (void*)static_cast<GFXD3D11TextureObject*>(eyeTex.getPointer())->get2DTex(), vr::API_DirectX, vr::ColorSpace_Gamma };
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[0], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left), &eyeTexture, &bounds);
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[1], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Right), &eyeTexture, &bounds);
      }
   }
   else if (GFX->getAdapterType() == Direct3D9)
   {
      //vr::Texture_t eyeTexture = { (void*)static_cast<GFXD3D9TextureObject*>(mHMDRenderState.mStereoRenderTextures[index].getPointer())->get2DTex(), vr::API_DirectX, vr::ColorSpace_Gamma };
      //err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture);
   }
#ifdef TORQUE_OPENGL
   else if (GFX->getAdapterType() == OpenGL)
   {
      vr::Texture_t eyeTexture;
      if (mHMDRenderState.mRenderMode == GFXDevice::RS_StereoSeparate)
      {
         // whatever eye we are on
         eyeTexture = { (void*)static_cast<GFXGLTextureObject*>(eyeTex.getPointer())->getHandle(), vr::API_OpenGL, vr::ColorSpace_Gamma };
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[index], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture, &bounds);
      }
      else
      {
         // left & right at the same time
         eyeTexture = { (void*)static_cast<GFXGLTextureObject*>(eyeTex.getPointer())->getHandle(), vr::API_OpenGL, vr::ColorSpace_Gamma };
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[0], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left), &eyeTexture, &bounds);
         bounds = OpenVRUtil::TorqueRectToBounds(mHMDRenderState.mEyeViewport[1], mHMDRenderState.mStereoRenderTexture.getWidthHeight());
         err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Right), &eyeTexture, &bounds);
      }
   }
#endif

   AssertFatal(err == vr::VRCompositorError_None, "VR compositor error!");
}

void OpenVRProvider::setRoomTracking(bool room)
{
   vr::IVRCompositor* compositor = vr::VRCompositor();
   mTrackingSpace = room ? vr::TrackingUniverseStanding : vr::TrackingUniverseSeated;
   if (compositor) compositor->SetTrackingSpace(mTrackingSpace);
}

bool OpenVRProvider::_handleDeviceEvent(GFXDevice::GFXDeviceEventType evt)
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return true;
   }

   switch (evt)
   {
   case GFXDevice::deStartOfFrame:

      // Start of frame

      onStartFrame();

      break;

   case GFXDevice::dePostFrame:

      // End of frame

      onEndFrame();

      break;

   case GFXDevice::deDestroy:

      // Need to reinit rendering
      break;

   case GFXDevice::deLeftStereoFrameRendered:
      // 

      onEyeRendered(0);
      break;

   case GFXDevice::deRightStereoFrameRendered:
      // 

      onEyeRendered(1);
      break;

   default:
      break;
   }

   return true;
}

S32 OpenVRProvider::getDisplayDeviceId() const
{
#if defined(TORQUE_OS_WIN64) || defined(TORQUE_OS_WIN32)
   if (GFX && GFX->getAdapterType() == Direct3D11)
   {
      Vector<GFXAdapter*> adapterList;
      GFXD3D11Device::enumerateAdapters(adapterList);

      for (U32 i = 0, sz = adapterList.size(); i < sz; i++)
      {
         GFXAdapter* adapter = adapterList[i];
         if (dMemcmp(&adapter->mLUID, &mLUID, sizeof(mLUID)) == 0)
         {
            return adapter->mIndex;
         }
      }
   }
#endif

   return -1;
}

void OpenVRProvider::processVREvent(const vr::VREvent_t & evt)
{
   mVREventSignal.trigger(evt);
   switch (evt.eventType)
   {
   case vr::VREvent_InputFocusCaptured:
      //Con::executef()
      break;
   case vr::VREvent_TrackedDeviceActivated:
   {
      // Setup render model
   }
   break;
   case vr::VREvent_TrackedDeviceDeactivated:
   {
      // Deactivated
   }
   break;
   case vr::VREvent_TrackedDeviceUpdated:
   {
      // Updated
   }
   break;
   }
}

void OpenVRProvider::updateTrackedPoses()
{
   if (!mHMD)
      return;

   vr::IVRCompositor* compositor = vr::VRCompositor();

   if (!compositor)
      return;

   if (compositor->GetTrackingSpace() != mTrackingSpace)
   {
      compositor->SetTrackingSpace(mTrackingSpace);
   }

   compositor->WaitGetPoses(mTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

   // Make sure we're using the latest eye offset in case user has changed IPD
   mHMDRenderState.updateHMDProjection();

   mValidPoseCount = 0;

   for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
   {
      IDevicePose &inPose = mCurrentDevicePose[nDevice];
      if (mTrackedDevicePose[nDevice].bPoseIsValid)
      {
         mValidPoseCount++;
         MatrixF mat = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(mTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);

         if (nDevice == vr::k_unTrackedDeviceIndex_Hmd)
         {
            mHMDRenderState.mHMDPose = mat;

         /*
            MatrixF rotOffset(1);
            EulerF localRot(-smHMDRotOffset.x, -smHMDRotOffset.z, smHMDRotOffset.y);

            // NOTE: offsetting before is probably the best we're going to be able to do here, since if we apply the matrix AFTER 
            // we will get correct movements relative to the camera HOWEVER this also distorts any future movements from the HMD since 
            // we will then be on a really weird rotation axis.
            QuatF(localRot).setMatrix(&rotOffset);
            rotOffset.inverse();
            mHMDRenderState.mHMDPose = mat = rotOffset * mHMDRenderState.mHMDPose;
         */

            // jamesu - store the last rotation for temp debugging
            MatrixF torqueMat(1);
            OpenVRUtil::convertTransformFromOVR(mat, torqueMat);
            gLastMoveRot = AngAxisF(torqueMat);
            //Con::printf("gLastMoveRot = %f,%f,%f,%f", gLastMoveRot.axis.x, gLastMoveRot.axis.y, gLastMoveRot.axis.z, gLastMoveRot.angle);
            mHMDRenderState.mHMDPose.inverse();
         }

         vr::TrackedDevicePose_t &outPose = mTrackedDevicePose[nDevice];
         OpenVRTransformToRotPos(mat, inPose.orientation, inPose.position);

#ifdef DEBUG_DISPLAY_POSE
       OpenVRUtil::convertTransformFromOVR(mat, inPose.actualMatrix);
       inPose.originalMatrix = mat;
#endif

         inPose.state = outPose.eTrackingResult;
         inPose.valid = outPose.bPoseIsValid;
         inPose.connected = outPose.bDeviceIsConnected;

         inPose.velocity = OpenVRVecToTorqueVec(outPose.vVelocity);
         inPose.angularVelocity = OpenVRVecToTorqueVec(outPose.vAngularVelocity);
      }
      else
      {
         inPose.valid = false;
      }
   }
}

void OpenVRProvider::submitInputChanges()
{
   // Diff current frame with previous frame
   for (U32 i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
   {
      IDevicePose curPose = mCurrentDevicePose[i];
      IDevicePose prevPose = mPreviousInputTrackedDevicePose[i];

     S32 eventIdx = -1;
     
     if (!mDeviceEventMap.tryGetValue(i, eventIdx) || eventIdx < 0)
        continue;

      if (!curPose.valid || !curPose.connected)
         continue;

      if (curPose.orientation != prevPose.orientation)
      {
         AngAxisF axisAA(curPose.orientation);
         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_ROT, OVR_SENSORROT[eventIdx], SI_MOVE, axisAA);
      }

      if (curPose.position != prevPose.position)
      {
         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORPOSITION[eventIdx], SI_MOVE, curPose.position);
      }

      if (curPose.velocity != prevPose.velocity)
      {
         // Convert angles to degrees
         VectorF angles;
         angles.x = curPose.velocity.x;
         angles.y = curPose.velocity.y;
         angles.z = curPose.velocity.z;

         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORVELOCITY[eventIdx], SI_MOVE, angles);
      }

      if (curPose.angularVelocity != prevPose.angularVelocity)
      {
         // Convert angles to degrees
         VectorF angles;
         angles[0] = mRadToDeg(curPose.velocity.x);
         angles[1] = mRadToDeg(curPose.velocity.y);
         angles[2] = mRadToDeg(curPose.velocity.z);

         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORANGVEL[eventIdx], SI_MOVE, angles);
      }
      /*
      if (curPose.connected != prevPose.connected)
      {
         if (Con::isFunction("onOVRConnectionChanged"))
         {
            Con::executef("onOVRConnectionStatus", curPose.connected);
         }
      }*/

      if (curPose.state != prevPose.state)
      {
         if (Con::isFunction("onOVRStateChanged"))
         {
            Con::executef("onOVRStateChanged", curPose.state);
         }
      }
   }

   dMemcpy(mPreviousInputTrackedDevicePose, mCurrentDevicePose, sizeof(mPreviousInputTrackedDevicePose));
}

void OpenVRProvider::resetSensors()
{
   if (mHMD)
   {
      mHMD->ResetSeatedZeroPose();
   }
}

void OpenVRProvider::mapDeviceToEvent(U32 deviceIdx, S32 eventIdx)
{
   mDeviceEventMap[deviceIdx] = eventIdx;
}

void OpenVRProvider::resetEventMap()
{
   mDeviceEventMap.clear();
}

IDevicePose OpenVRProvider::getTrackedDevicePose(U32 idx)
{
   if (idx >= vr::k_unMaxTrackedDeviceCount)
   {
      IDevicePose ret;
      ret.connected = ret.valid = false;
      return ret;
   }

   return mCurrentDevicePose[idx];
}

void OpenVRProvider::registerOverlay(OpenVROverlay* overlay)
{
   mOverlays.push_back(overlay);
}

void OpenVRProvider::unregisterOverlay(OpenVROverlay* overlay)
{
   S32 index = mOverlays.find_next(overlay);
   if (index != -1)
   {
      mOverlays.erase(index);
   }
}

const S32 OpenVRProvider::preloadRenderModelTexture(U32 index)
{
   S32 idx = -1;
   if (mLoadedTextureLookup.tryGetValue(index, idx))
      return idx;

   char buffer[256];
   dSprintf(buffer, sizeof(buffer), "openvrtex_%u", index);

   OpenVRProvider::LoadedRenderTexture loadedTexture;
   loadedTexture.vrTextureId = index;
   loadedTexture.vrTexture = NULL;
   loadedTexture.texture = NULL;
   loadedTexture.textureError = vr::VRRenderModelError_Loading;
   loadedTexture.targetTexture = new NamedTexTarget();
   loadedTexture.targetTexture->registerWithName(buffer);
   mLoadedTextures.push_back(loadedTexture);
   mLoadedTextureLookup[index] = mLoadedTextures.size() - 1;

   return mLoadedTextures.size() - 1;
}

const S32 OpenVRProvider::preloadRenderModel(StringTableEntry name)
{
   S32 idx = -1;
   if (mLoadedModelLookup.tryGetValue(name, idx))
      return idx;

   OpenVRProvider::LoadedRenderModel loadedModel;
   loadedModel.name = name;
   loadedModel.model = NULL;
   loadedModel.vrModel = NULL;
   loadedModel.modelError = vr::VRRenderModelError_Loading;
   loadedModel.loadedTexture = false;
   loadedModel.textureId = -1;
   mLoadedModels.push_back(loadedModel);
   mLoadedModelLookup[name] = mLoadedModels.size() - 1;

   return mLoadedModels.size() - 1;
}


bool OpenVRProvider::getRenderModel(S32 idx, OpenVRRenderModel **ret, bool &failed)
{
   if (idx < 0 || idx > mLoadedModels.size())
   {
      failed = true;
      return true;
   }

   OpenVRProvider::LoadedRenderModel &loadedModel = mLoadedModels[idx];
   //Con::printf("RenderModel[%i] STAGE 1", idx);

   failed = false;

   if (loadedModel.modelError > vr::VRRenderModelError_Loading)
   {
      failed = true;
      return true;
   }

   // Stage 1 : model
   if (!loadedModel.model)
   {
      loadedModel.modelError = vr::VRRenderModels()->LoadRenderModel_Async(loadedModel.name, &loadedModel.vrModel);
      //Con::printf(" vr::VRRenderModels()->LoadRenderModel_Async(\"%s\", %x); -> %i", loadedModel.name, &loadedModel.vrModel, loadedModel.modelError);
      if (loadedModel.modelError == vr::VRRenderModelError_None)
      {
         if (loadedModel.vrModel == NULL)
         {
            failed = true;
            return true;
         }
         // Load the model
         loadedModel.model = new OpenVRRenderModel();
      }
      else if (loadedModel.modelError == vr::VRRenderModelError_Loading)
      {
         return false;
      }
   }

   //Con::printf("RenderModel[%i] STAGE 2 (texId == %i)", idx, loadedModel.vrModel->diffuseTextureId);

   // Stage 2 : texture
   if (!loadedModel.loadedTexture && loadedModel.model)
   {
      if (loadedModel.textureId == -1)
      {
         loadedModel.textureId = preloadRenderModelTexture(loadedModel.vrModel->diffuseTextureId);
      }

      if (loadedModel.textureId == -1)
      {
         failed = true;
         return true;
      }

      if (!getRenderModelTexture(loadedModel.textureId, NULL, failed))
      {
         return false;
      }

      if (failed)
      {
         return true;
      }

      loadedModel.loadedTexture = true;

      //Con::printf("RenderModel[%i] GOT TEXTURE");

      // Now we can load the model. Note we first need to get a Material for the mapped texture
      NamedTexTarget *namedTexture = mLoadedTextures[loadedModel.textureId].targetTexture;
      String materialName = MATMGR->getMapEntry(namedTexture->getName().c_str());
      if (materialName.isEmpty())
      {
         char buffer[256];
         dSprintf(buffer, sizeof(buffer), "#%s", namedTexture->getName().c_str());
         materialName = buffer;

         //Con::printf("RenderModel[%i] materialName == %s", idx, buffer);

         Material* mat = new Material();
         mat->mMapTo = namedTexture->getName();
         mat->mDiffuseMapFilename[0] = buffer;
         mat->mEmissive[0] = true;

         dSprintf(buffer, sizeof(buffer), "%s_Material", namedTexture->getName().c_str());
         if (!mat->registerObject(buffer))
         {
            Con::errorf("Couldn't create placeholder openvr material %s!", buffer);
            failed = true;
            return true;
         }

         materialName = buffer;
      }
      
      loadedModel.model->init(*loadedModel.vrModel, materialName);
   }

   if ((loadedModel.modelError > vr::VRRenderModelError_Loading) || 
       (loadedModel.textureId >= 0 && mLoadedTextures[loadedModel.textureId].textureError > vr::VRRenderModelError_Loading))
   {
      failed = true;
   }

   if (!failed && ret)
   {
      *ret = loadedModel.model;
   }
   return true;
}

bool OpenVRProvider::getRenderModelTexture(S32 idx, GFXTextureObject **outTex, bool &failed)
{
   if (idx < 0 || idx > mLoadedModels.size())
   {
      failed = true;
      return true;
   }

   failed = false;

   OpenVRProvider::LoadedRenderTexture &loadedTexture = mLoadedTextures[idx];

   if (loadedTexture.textureError > vr::VRRenderModelError_Loading)
   {
      failed = true;
      return true;
   }

   if (!loadedTexture.texture)
   {
      if (!loadedTexture.vrTexture)
      {
         loadedTexture.textureError = vr::VRRenderModels()->LoadTexture_Async(loadedTexture.vrTextureId, &loadedTexture.vrTexture);
         if (loadedTexture.textureError == vr::VRRenderModelError_None)
         {
            // Load the texture
            GFXTexHandle tex;

            const U32 sz = loadedTexture.vrTexture->unWidth * loadedTexture.vrTexture->unHeight * 4;
            GBitmap *bmp = new GBitmap(loadedTexture.vrTexture->unWidth, loadedTexture.vrTexture->unHeight, false, GFXFormatR8G8B8A8);

            Swizzles::bgra.ToBuffer(bmp->getAddress(0,0,0), loadedTexture.vrTexture->rubTextureMapData, sz);

            char buffer[256];
            dSprintf(buffer, 256, "OVRTEX-%i.png", loadedTexture.vrTextureId);

            FileStream fs;
            fs.open(buffer, Torque::FS::File::Write);
            bmp->writeBitmap("PNG", fs);
            fs.close();

            tex.set(bmp, &GFXDefaultStaticDiffuseProfile, true, "OpenVR Texture");
            //tex.set(loadedTexture.vrTexture->unWidth, loadedTexture.vrTexture->unHeight, 1, (void*)pixels, GFXFormatR8G8B8A8, &GFXDefaultStaticDiffuseProfile, "OpenVR Texture", 1);


            loadedTexture.targetTexture->setTexture(tex);
            loadedTexture.texture = tex;
         }
         else if (loadedTexture.textureError == vr::VRRenderModelError_Loading)
         {
            return false;
         }
      }
   }

   if (loadedTexture.textureError > vr::VRRenderModelError_Loading)
   {
      failed = true;
   }

   if (!failed && outTex)
   {
      *outTex = loadedTexture.texture;
   }

   return true;
}

bool OpenVRProvider::getRenderModelTextureName(S32 idx, String &outName)
{
   if (idx < 0 || idx >= mLoadedTextures.size())
      return false;

   if (mLoadedTextures[idx].targetTexture)
   {
      outName = mLoadedTextures[idx].targetTexture->getName();
      return true;
   }

   return false;
}

void OpenVRProvider::resetRenderModels()
{
   for (U32 i = 0, sz = mLoadedModels.size(); i < sz; i++)
   {
      SAFE_DELETE(mLoadedModels[i].model);
      if (mLoadedModels[i].vrModel) mRenderModels->FreeRenderModel(mLoadedModels[i].vrModel);
   }
   for (U32 i = 0, sz = mLoadedTextures.size(); i < sz; i++)
   {
      SAFE_DELETE(mLoadedTextures[i].targetTexture);
      if (mLoadedTextures[i].vrTexture) mRenderModels->FreeTexture(mLoadedTextures[i].vrTexture);
   }
   mLoadedModels.clear();
   mLoadedTextures.clear();
   mLoadedModelLookup.clear();
   mLoadedTextureLookup.clear();
}

OpenVROverlay *OpenVRProvider::getGamepadFocusOverlay()
{
   return NULL;
}

void OpenVRProvider::setOverlayNeighbour(vr::EOverlayDirection dir, OpenVROverlay *overlay)
{

}


bool OpenVRProvider::isDashboardVisible()
{
   return false;
}

void OpenVRProvider::showDashboard(const char *overlayToShow)
{

}

vr::TrackedDeviceIndex_t OpenVRProvider::getPrimaryDashboardDevice()
{
   return -1;
}

void OpenVRProvider::setKeyboardTransformAbsolute(const MatrixF &xfm)
{
   // mTrackingSpace
}

void OpenVRProvider::setKeyboardPositionForOverlay(OpenVROverlay *overlay, const RectI &rect)
{

}

void OpenVRProvider::getControllerDeviceIndexes(vr::TrackedDeviceClass &deviceClass, Vector<S32> &outList)
{
   for (U32 i = 0; i<vr::k_unMaxTrackedDeviceCount; i++)
   {
      if (!mCurrentDevicePose[i].connected)
         continue;

      vr::TrackedDeviceClass klass = mHMD->GetTrackedDeviceClass(i);
      if (klass == deviceClass)
      {
         outList.push_back(i);
      }
   }
}

StringTableEntry OpenVRProvider::getControllerModel(U32 idx)
{
   if (idx >= vr::k_unMaxTrackedDeviceCount || !mRenderModels)
      return NULL;

   String str = GetTrackedDeviceString(mHMD, idx, vr::Prop_RenderModelName_String, NULL);
   return StringTable->insert(str, true);
}

DefineEngineStaticMethod(OpenVR, getControllerDeviceIndexes, const char*, (OpenVRTrackedDeviceClass klass),,
   "@brief Gets the indexes of devices which match the required device class")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return "";
   }

   Vector<S32> outList;
   OPENVR->getControllerDeviceIndexes(klass, outList);
   return EngineMarshallData<Vector<S32>>(outList);
}

DefineEngineStaticMethod(OpenVR, getControllerModel, const char*, (S32 idx), ,
   "@brief Gets the indexes of devices which match the required device class")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return "";
   }

   return OPENVR->getControllerModel(idx);
}

DefineEngineStaticMethod(OpenVR, isDeviceActive, bool, (), ,
   "@brief Used to determine if the OpenVR input device is active\n\n"

   "The OpenVR device is considered active when the library has been "
   "initialized and either a real of simulated HMD is present.\n\n"

   "@return True if the OpenVR input device is active.\n"

   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return false;
   }

   return OPENVR->getActive();
}


DefineEngineStaticMethod(OpenVR, setEnabled, bool, (bool value), ,
   "@brief Used to determine if the OpenVR input device is active\n\n"

   "The OpenVR device is considered active when the library has been "
   "initialized and either a real of simulated HMD is present.\n\n"

   "@return True if the OpenVR input device is active.\n"

   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return false;
   }

   return value ? OPENVR->enable() : OPENVR->disable();
}


DefineEngineStaticMethod(OpenVR, setHMDAsGameConnectionDisplayDevice, bool, (GameConnection* conn), ,
   "@brief Sets the first HMD to be a GameConnection's display device\n\n"
   "@param conn The GameConnection to set.\n"
   "@return True if the GameConnection display device was set.\n"
   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      Con::errorf("setOVRHMDAsGameConnectionDisplayDevice(): No Oculus VR Device present.");
      return false;
   }

   if (!conn)
   {
      Con::errorf("setOVRHMDAsGameConnectionDisplayDevice(): Invalid GameConnection.");
      return false;
   }

   conn->setDisplayDevice(OPENVR);
   return true;
}


DefineEngineStaticMethod(OpenVR, getDisplayDeviceId, S32, (), ,
   "@brief MacOS display ID.\n\n"
   "@param index The HMD index.\n"
   "@return The ID of the HMD display device, if any.\n"
   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return -1;
   }

   return OPENVR->getDisplayDeviceId();
}

DefineEngineStaticMethod(OpenVR, resetSensors, void, (), ,
   "@brief Resets all Oculus VR sensors.\n\n"
   "This resets all sensor orientations such that their 'normal' rotation "
   "is defined when this function is called.  This defines an HMD's forwards "
   "and up direction, for example."
   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return;
   }

   OPENVR->resetSensors();
}

DefineEngineStaticMethod(OpenVR, mapDeviceToEvent, void, (S32 deviceId, S32 eventId), ,
   "@brief Maps a device to an event code.\n\n"
   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return;
   }

   OPENVR->mapDeviceToEvent(deviceId, eventId);
}

DefineEngineStaticMethod(OpenVR, resetEventMap, void, (), ,
   "@brief Resets event map.\n\n"
   "@ingroup Game")
{
   if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      return;
   }

   OPENVR->resetEventMap();
}

// Overlay stuff

DefineEngineFunction(OpenVRIsCompiledIn, bool, (), , "")
{
   return true;
}
