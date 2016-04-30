#include "platform/input/openVR/openVRProvider.h"
#include "platform/platformInput.h"
#include "core/module.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/core/guiCanvas.h"
#include "postFx/postEffectCommon.h"

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/gfxStringEnumTranslate.h"


#include "gfx/D3D9/gfxD3D9Device.h"
#include "gfx/D3D9/gfxD3D9TextureObject.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"

/*
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
*/

#include "platform/input/oculusVR/oculusVRUtil.h"


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

static MatrixF ConvertSteamVRAffineMatrixToMatrixFPlain(const vr::HmdMatrix34_t &mat)
{
   MatrixF outMat(1);

   outMat.setColumn(0, Point4F(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0));
   outMat.setColumn(1, Point4F(mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0));
   outMat.setColumn(2, Point4F(mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0));
   outMat.setColumn(3, Point4F(mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f)); // pos

   return outMat;
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


bool OpenVRRenderState::setupRenderTargets(U32 mode)
{
   if (!mHMD)
      return false;

   U32 sizeX, sizeY;
   Point2I newRTSize;
   mHMD->GetRecommendedRenderTargetSize(&sizeX, &sizeY);

   mEyeViewport[0] = RectI(Point2I(0, 0), Point2I(sizeX, sizeY));
   mEyeViewport[1] = RectI(Point2I(0, 0), Point2I(sizeX, sizeY));

   newRTSize.x = sizeX;
   newRTSize.y = sizeY;

   GFXTexHandle stereoTexture;
   stereoTexture.set(newRTSize.x, newRTSize.y, GFXFormatR8G8B8A8, &VRTextureProfile, "OpenVR Stereo RT Color");
   mStereoRenderTextures[0] = mStereoRenderTextures[1] = stereoTexture;

   GFXTexHandle stereoDepthTexture;
   stereoDepthTexture.set(newRTSize.x, newRTSize.y, GFXFormatD24S8, &VRDepthProfile, "OpenVR Depth");
   mStereoDepthTextures[0] = mStereoDepthTextures[1] = stereoDepthTexture;

   mStereoRT = GFX->allocRenderToTextureTarget();
   mStereoRT->attachTexture(GFXTextureTarget::Color0, stereoTexture);
   mStereoRT->attachTexture(GFXTextureTarget::DepthStencil, stereoDepthTexture);

   mEyeRT[0] = mEyeRT[1] = mStereoRT;

   mOutputEyeTextures[0].init(newRTSize.x, newRTSize.y, GFXFormatR8G8B8A8, &VRTextureProfile, "OpenVR Stereo RT Color OUTPUT");
   mOutputEyeTextures[1].init(newRTSize.x, newRTSize.y, GFXFormatR8G8B8A8, &VRTextureProfile, "OpenVR Stereo RT Color OUTPUT");

   return true;
}

void OpenVRRenderState::setupDistortion()
{
   if (!mHMD)
      return;

   U16 m_iLensGridSegmentCountH = 43;
   U16 m_iLensGridSegmentCountV = 43;

   float w = (float)(1.0 / float(m_iLensGridSegmentCountH - 1));
   float h = (float)(1.0 / float(m_iLensGridSegmentCountV - 1));

   float u, v = 0;

   Vector<GFXVertexPTTT> vVerts(0);
   GFXVertexPTTT *vert;

   vVerts.reserve((m_iLensGridSegmentCountV * m_iLensGridSegmentCountH) * 2);

   mDistortionVerts.set(GFX, (m_iLensGridSegmentCountV * m_iLensGridSegmentCountH) * 2, GFXBufferTypeStatic);

   vert = mDistortionVerts.lock();

   //left eye distortion verts
   float Xoffset = -1;
   for (int y = 0; y < m_iLensGridSegmentCountV; y++)
   {
      for (int x = 0; x < m_iLensGridSegmentCountH; x++)
      {
         u = x*w; v = 1 - y*h;
         vert->point = Point3F(Xoffset + u, -1 + 2 * y*h, 0.0f);

         vr::DistortionCoordinates_t dc0 = mHMD->ComputeDistortion(vr::Eye_Left, u, v);

         vert->texCoord1 = Point2F(dc0.rfRed[0], 1 - dc0.rfRed[1]); // r
         vert->texCoord2 = Point2F(dc0.rfGreen[0], 1 - dc0.rfGreen[1]); // g
         vert->texCoord3 = Point2F(dc0.rfBlue[0], 1 - dc0.rfBlue[1]); // b

         vert++;
      }
   }

   //right eye distortion verts
   Xoffset = 0;
   for (int y = 0; y < m_iLensGridSegmentCountV; y++)
   {
      for (int x = 0; x < m_iLensGridSegmentCountH; x++)
      {
         u = x*w; v = 1 - y*h;
         vert->point = Point3F(Xoffset + u, -1 + 2 * y*h, 0.0f);

         vr::DistortionCoordinates_t dc0 = mHMD->ComputeDistortion(vr::Eye_Right, u, v);

         vert->texCoord1 = Point2F(dc0.rfRed[0], 1 - dc0.rfRed[1]);
         vert->texCoord2 = Point2F(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
         vert->texCoord3 = Point2F(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

         vert++;
      }
   }

   mDistortionVerts.unlock();

   mDistortionInds.set(GFX, m_iLensGridSegmentCountV * m_iLensGridSegmentCountH * 6 * 2, 0, GFXBufferTypeStatic);

   GFXPrimitive *prim;
   U16 *index;

   mDistortionInds.lock(&index, &prim);
   U16 a, b, c, d;

   U16 offset = 0;
   for (U16 y = 0; y < m_iLensGridSegmentCountV - 1; y++)
   {
      for (U16 x = 0; x < m_iLensGridSegmentCountH - 1; x++)
      {
         a = m_iLensGridSegmentCountH*y + x + offset;
         b = m_iLensGridSegmentCountH*y + x + 1 + offset;
         c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
         d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
         *index++ = a;
         *index++ = b;
         *index++ = c;

         *index++ = a;
         *index++ = c;
         *index++ = d;
      }
   }

   offset = (m_iLensGridSegmentCountH)*(m_iLensGridSegmentCountV);
   for (U16 y = 0; y < m_iLensGridSegmentCountV - 1; y++)
   {
      for (U16 x = 0; x < m_iLensGridSegmentCountH - 1; x++)
      {
         a = m_iLensGridSegmentCountH*y + x + offset;
         b = m_iLensGridSegmentCountH*y + x + 1 + offset;
         c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
         d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
         *index++ = a;
         *index++ = b;
         *index++ = c;

         *index++ = a;
         *index++ = c;
         *index++ = d;
      }
   }

   mDistortionInds.unlock();
}

void OpenVRRenderState::renderDistortion(U32 eye)
{
   // Updates distortion for an eye (this should only be the case for backend APIS where image should be predistorted)
   /*

   glDisable(GL_DEPTH_TEST);
   glViewport( 0, 0, m_nWindowWidth, m_nWindowHeight );

   glBindVertexArray( m_unLensVAO );
   glUseProgram( m_unLensProgramID );

   //render left lens (first half of index array )
   glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   glDrawElements( GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, 0 );

   //render right lens (second half of index array )
   glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   glDrawElements( GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(m_uiIndexSize) );

   glBindVertexArray( 0 );
   glUseProgram( 0 );
   */
}

void OpenVRRenderState::renderPreview()
{

}

void OpenVRRenderState::reset(vr::IVRSystem* hmd)
{
   mHMD = hmd;

   mStereoRT = NULL;
   mEyeRT[0] = mEyeRT[1] = NULL;

   mStereoRenderTextures[0] = mStereoRenderTextures[1] = NULL;
   mStereoDepthTextures[0] = mStereoDepthTextures[1] = NULL;

   mDistortionVerts = NULL;
   mDistortionInds = NULL;

   mOutputEyeTextures[0].clear();
   mOutputEyeTextures[1].clear();

   if (!mHMD)
      return;

   vr::HmdMatrix34_t mat = mHMD->GetEyeToHeadTransform(vr::Eye_Left);
   mEyePose[0] = ConvertSteamVRAffineMatrixToMatrixFPlain(mat);
   mEyePose[0].inverse();

   mat = mHMD->GetEyeToHeadTransform(vr::Eye_Right);
   mEyePose[1] = ConvertSteamVRAffineMatrixToMatrixFPlain(mat);
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
}

OpenVRProvider::~OpenVRProvider()
{

}

void OpenVRProvider::staticInit()
{
   // TODO: Add console vars
}

bool OpenVRProvider::enable()
{
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

   mHMDRenderState.reset(mHMD);
   mHMD->ResetSeatedZeroPose();
   dMemset(mPreviousInputTrackedDevicePose, '\0', sizeof(mPreviousInputTrackedDevicePose));

   mEnabled = true;

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

   // Process SteamVR events
   vr::VREvent_t event;
   while (mHMD->PollNextEvent(&event, sizeof(event)))
   {
      processVREvent(event);
   }

   // Process SteamVR controller state
   for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
   {
      vr::VRControllerState_t state;
      if (mHMD->GetControllerState(unDevice, &state))
      {
         // TODO
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

   F32 inRotMat[4][4];
   Point4F col0; mat.getColumn(0, &col0);
   Point4F col1; mat.getColumn(1, &col1);
   Point4F col2; mat.getColumn(2, &col2);
   Point4F col3; mat.getColumn(3, &col3);
   inRotMat[0][0] = col0.x;
   inRotMat[0][1] = col0.y;
   inRotMat[0][2] = col0.z;
   inRotMat[0][3] = col0.w;
   inRotMat[1][0] = col1.x;
   inRotMat[1][1] = col1.y;
   inRotMat[1][2] = col1.z;
   inRotMat[1][3] = col1.w;
   inRotMat[2][0] = col2.x;
   inRotMat[2][1] = col2.y;
   inRotMat[2][2] = col2.z;
   inRotMat[2][3] = col2.w;
   inRotMat[3][0] = col3.x;
   inRotMat[3][1] = col3.y;
   inRotMat[3][2] = col3.z;
   inRotMat[3][3] = col3.w;

   OculusVRUtil::convertRotation(inRotMat, torqueMat);

   Point3F pos = torqueMat.getPosition();
   outRot = QuatF(torqueMat);
   outPos = Point3F(-pos.x, pos.z, -pos.y);
}

void OpenVRProvider::getFrameEyePose(IDevicePose *pose, U32 eye) const
{
   AssertFatal(eye >= 0 && eye < 2, "Out of bounds eye");

   MatrixF mat = mHMDRenderState.mHMDPose * mHMDRenderState.mEyePose[eye];

   OpenVRTransformToRotPos(mat, pose->orientation, pose->position);
   pose->velocity = Point3F(0);
   pose->angularVelocity = Point3F(0);
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
}

bool OpenVRProvider::providesFovPorts() const
{
   return mHMD != NULL;
}

void OpenVRProvider::getFovPorts(FovPort *out) const
{
   dMemcpy(out, mHMDRenderState.mEyeFov, sizeof(mHMDRenderState.mEyeFov));
}

bool OpenVRProvider::providesProjectionOffset() const
{
   return mHMD != NULL;
}

const Point2F& OpenVRProvider::getProjectionOffset() const
{
   return Point2F(0, 0);
}

void OpenVRProvider::getStereoViewports(RectI *out) const
{
   out[0] = mHMDRenderState.mEyeViewport[0];
   out[1] = mHMDRenderState.mEyeViewport[1];
}

void OpenVRProvider::getStereoTargets(GFXTextureTarget **out) const
{
   out[0] = mHMDRenderState.mEyeRT[0];
   out[1] = mHMDRenderState.mEyeRT[1];
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
      mHMDRenderState.setupRenderTargets(0);
   }
   mDrawCanvas = canvas;
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
   return mHMDRenderState.mStereoRenderTextures[0]; // TODO: render distortion preview
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

   GFXTexHandle eyeTex = mHMDRenderState.mOutputEyeTextures[index].getTextureHandle();
   mHMDRenderState.mEyeRT[0]->resolveTo(eyeTex);
   mHMDRenderState.mOutputEyeTextures[index].advance();

   if (GFX->getAdapterType() == Direct3D11)
   {
	  GFXFormat fmt1 = eyeTex->getFormat();
      vr::Texture_t eyeTexture = { (void*)static_cast<GFXD3D11TextureObject*>(eyeTex.getPointer())->get2DTex(), vr::API_DirectX, vr::ColorSpace_Gamma };
      err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture);
   }
   else if (GFX->getAdapterType() == Direct3D9)
   {
	   //vr::Texture_t eyeTexture = { (void*)static_cast<GFXD3D9TextureObject*>(mHMDRenderState.mStereoRenderTextures[index].getPointer())->get2DTex(), vr::API_DirectX, vr::ColorSpace_Gamma };
	   //err = vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture);
   }
   else if (GFX->getAdapterType() == OpenGL)
   {/*
      vr::Texture_t eyeTexture = { (void*)static_cast<GFXGLTextureObject*>(mHMDRenderState.mStereoRenderTextures[index].getPointer())->getHandle(), vr::API_OpenGL, vr::ColorSpace_Gamma };
      vr::VRCompositor()->Submit((vr::EVREye)(vr::Eye_Left + index), &eyeTexture);*/
   }

   AssertFatal(err == vr::VRCompositorError_None, "VR compositor error!");
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
	return -1;
#ifdef TORQUE_OS_WIN32
	if (GFX->getAdapterType() == Direct3D11)
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

void OpenVRProvider::processVREvent(const vr::VREvent_t & event)
{
   switch (event.eventType)
   {
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

   vr::VRCompositor()->WaitGetPoses(mTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

   mValidPoseCount = 0;

   for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
   {
      IDevicePose &inPose = mCurrentDevicePose[nDevice];
      if (mTrackedDevicePose[nDevice].bPoseIsValid)
      {
         mValidPoseCount++;
         MatrixF mat = ConvertSteamVRAffineMatrixToMatrixFPlain(mTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
         mat.inverse();

         if (nDevice == vr::k_unTrackedDeviceIndex_Hmd)
         {
            mHMDRenderState.mHMDPose = mat;
         }

         vr::TrackedDevicePose_t &outPose = mTrackedDevicePose[nDevice];
         OpenVRTransformToRotPos(mat, inPose.orientation, inPose.position);

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

      if (!curPose.valid || !curPose.connected)
         continue;

      if (curPose.orientation != prevPose.orientation)
      {
         AngAxisF axisAA(curPose.orientation);
         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_ROT, OVR_SENSORROT[i], SI_MOVE, axisAA);
      }

      if (curPose.position != prevPose.position)
      {
         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORPOSITION[i], SI_MOVE, curPose.position);
      }

      if (curPose.velocity != prevPose.velocity)
      {
         // Convert angles to degrees
         VectorF angles;
         angles.x = curPose.velocity.x;
         angles.y = curPose.velocity.y;
         angles.z = curPose.velocity.z;

         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORVELOCITY[i], SI_MOVE, angles);
      }

      if (curPose.angularVelocity != prevPose.angularVelocity)
      {
         // Convert angles to degrees
         VectorF angles;
         angles[0] = mRadToDeg(curPose.velocity.x);
         angles[1] = mRadToDeg(curPose.velocity.y);
         angles[2] = mRadToDeg(curPose.velocity.z);

         INPUTMGR->buildInputEvent(mDeviceType, 0, SI_POS, OVR_SENSORANGVEL[i], SI_MOVE, angles);
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

DefineEngineFunction(isOpenVRDeviceActive, bool, (), ,
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

   return OCULUSVRDEV->getActive();
}


DefineEngineFunction(OpenVRSetEnabled, bool, (bool value), ,
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

   return value ? ManagedSingleton<OpenVRProvider>::instance()->enable() : ManagedSingleton<OpenVRProvider>::instance()->disable();
}



DefineEngineFunction(setOpenVRHMDAsGameConnectionDisplayDevice, bool, (GameConnection* conn), ,
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

   conn->setDisplayDevice(ManagedSingleton<OpenVRProvider>::instance());
   return true;
}


DefineEngineFunction(OpenVRGetDisplayDeviceId, S32, (), ,
	"@brief MacOS display ID.\n\n"
	"@param index The HMD index.\n"
	"@return The ID of the HMD display device, if any.\n"
	"@ingroup Game")
{
	if (!ManagedSingleton<OpenVRProvider>::instanceOrNull())
	{
		return -1;
	}

	return ManagedSingleton<OpenVRProvider>::instance()->getDisplayDeviceId();
}

DefineEngineFunction(OpenVRResetSensors, void, (), ,
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

   ManagedSingleton<OpenVRProvider>::instance()->resetSensors();
}
