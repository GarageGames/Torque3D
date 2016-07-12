#include "platform/input/openVR/openVRProvider.h"
#include "platform/input/openVR/openVROverlay.h"

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11TextureObject.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"

#ifdef TORQUE_OPENGL
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#endif

#include "postFx/postEffectCommon.h"
#include "gui/controls/guiTextEditCtrl.h"

ImplementEnumType(OpenVROverlayType,
   "Desired overlay type for OpenVROverlay. .\n\n"
   "@ingroup OpenVR")
{ OpenVROverlay::OVERLAYTYPE_OVERLAY, "Overlay" },
{ OpenVROverlay::OVERLAYTYPE_DASHBOARD, "Dashboard" },
EndImplementEnumType;

IMPLEMENT_CONOBJECT(OpenVROverlay);

OpenVROverlay::OpenVROverlay()
{
   mTransform = MatrixF(1);
   mOverlayWidth = 1.5f;
   mOverlayFlags = 0;

   mOverlayColor = ColorF(1, 1, 1, 1);
   mTrackingOrigin = vr::TrackingUniverseSeated;

   mTargetFormat = GFXFormatR8G8B8A8_LINEAR_FORCE; // needed for openvr!
   mManualMouseHandling = true;

   mMouseScale = Point2F(1, 1);
}

OpenVROverlay::~OpenVROverlay()
{

}

static bool setProtectedOverlayTypeDirty(void *obj, const char *array, const char *data)
{
   OpenVROverlay *object = static_cast<OpenVROverlay*>(obj);
   object->mOverlayTypeDirty = true;
   return true;
}

static bool setProtectedOverlayDirty(void *obj, const char *array, const char *data)
{
   OpenVROverlay *object = static_cast<OpenVROverlay*>(obj);
   object->mOverlayDirty = true;
   return true;
}

void OpenVROverlay::initPersistFields()
{
   addProtectedField("overlayType", TypeOpenVROverlayType, Offset(mOverlayType, OpenVROverlay), &setProtectedOverlayTypeDirty, &defaultProtectedGetFn,
      "Type of overlay.");
   addProtectedField("overlayFlags", TypeS32, Offset(mOverlayFlags, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Flags for overlay.");
   addProtectedField("overlayWidth", TypeF32, Offset(mOverlayWidth, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Width of overlay.");
   addProtectedField("overlayColor", TypeColorF, Offset(mOverlayColor, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Backing color of overlay.");

   addProtectedField("transformType", TypeOpenVROverlayTransformType, Offset(mOverlayTransformType, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Transform type of overlay.");
   addProtectedField("transformPosition", TypeMatrixPosition, Offset(mTransform, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Position of overlay.");
   addProtectedField("transformRotation", TypeMatrixRotation, Offset(mTransform, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Rotation of overlay.");
   addProtectedField("transformDeviceIndex", TypeS32, Offset(mTransformDeviceIndex, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Rotation of overlay.");
   addProtectedField("transformDeviceComponent", TypeString, Offset(mTransformDeviceComponent, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Rotation of overlay.");

   addProtectedField("inputMethod", TypeOpenVROverlayInputMethod, Offset(mInputMethod, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Type of input method.");
   addProtectedField("mouseScale", TypePoint2F, Offset(mMouseScale, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Scale of mouse input.");

   addProtectedField("trackingOrigin", TypeOpenVRTrackingUniverseOrigin, Offset(mTrackingOrigin, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Tracking origin.");

   addProtectedField("controllerDevice", TypeS32, Offset(mControllerDeviceIndex, OpenVROverlay), &setProtectedOverlayDirty, &defaultProtectedGetFn,
      "Index of controller to attach overlay to.");

   addField("manualMouseHandling", TypeBool, Offset(mManualMouseHandling, OpenVROverlay), "Forces openvr to create mouse events for overlay");

   Parent::initPersistFields();
}

bool OpenVROverlay::onAdd()
{
   if (Parent::onAdd())
   {
      mOverlayTypeDirty = true;
      mOverlayDirty = true;

      if (OPENVR)
      {
         OPENVR->registerOverlay(this);
      }

      return true;
   }

   return false;
}

void OpenVROverlay::onRemove()
{
   if (mOverlayHandle)
   {
      vr::VROverlay()->DestroyOverlay(mOverlayHandle);
      mOverlayHandle = NULL;
   }

   if (mThumbOverlayHandle)
   {
      vr::VROverlay()->DestroyOverlay(mThumbOverlayHandle);
      mThumbOverlayHandle = NULL;
   }

   if (ManagedSingleton<OpenVRProvider>::instanceOrNull())
   {
      OPENVR->unregisterOverlay(this);
   }
}

void OpenVROverlay::resetOverlay()
{
   vr::IVROverlay *overlay = vr::VROverlay();
   if (!overlay)
      return;

   if (mOverlayHandle)
   {
      overlay->DestroyOverlay(mOverlayHandle);
      mOverlayHandle = NULL;
   }

   if (mThumbOverlayHandle)
   {
      overlay->DestroyOverlay(mThumbOverlayHandle);
      mThumbOverlayHandle = NULL;
   }

   if (mOverlayType == OpenVROverlay::OVERLAYTYPE_DASHBOARD)
   {
      overlay->CreateDashboardOverlay(mInternalName, mInternalName, &mOverlayHandle, &mThumbOverlayHandle);
   }
   else
   {
      overlay->CreateOverlay(mInternalName, mInternalName, &mOverlayHandle);
   }

   mOverlayDirty = true;
   mOverlayTypeDirty = false;

   // Pre-render start frame so we have a texture available
   if (!mTarget)
   {
      renderFrame(false, false);
   }
}

void OpenVROverlay::updateOverlay()
{
   if (mOverlayTypeDirty)
      resetOverlay();

   // Update params
   vr::IVROverlay *overlay = vr::VROverlay();
   if (!overlay || !mOverlayHandle)
      return;

   if (!mOverlayDirty)
      return;

   MatrixF vrMat(1);
   vr::HmdMatrix34_t ovrMat;
   vr::HmdVector2_t ovrMouseScale;
   ovrMouseScale.v[0] = mMouseScale.x;
   ovrMouseScale.v[1] = mMouseScale.y;

   OpenVRUtil::convertTransformToOVR(mTransform, vrMat);
   OpenVRUtil::convertMatrixFPlainToSteamVRAffineMatrix(vrMat, ovrMat);

   MatrixF reverseMat = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(ovrMat);
   MatrixF finalReverseMat(1);
   OpenVRUtil::convertTransformFromOVR(reverseMat, finalReverseMat);

   switch (mOverlayTransformType)
   {
      case vr::VROverlayTransform_Absolute:
         overlay->SetOverlayTransformAbsolute(mOverlayHandle, mTrackingOrigin, &ovrMat);
         break;
      case vr::VROverlayTransform_TrackedDeviceRelative:
         overlay->SetOverlayTransformTrackedDeviceRelative(mOverlayHandle, mTransformDeviceIndex, &ovrMat);
         break;
      case vr::VROverlayTransform_TrackedComponent:
         overlay->SetOverlayTransformTrackedDeviceComponent(mOverlayHandle, mTransformDeviceIndex, mTransformDeviceComponent.c_str());
         break;
      // NOTE: system not handled here - doesn't seem possible to create these
      default:
         break;
   }

  // overlay->SetOverlayColor(mOverlayHandle, mOverlayColor.red, mOverlayColor.green, mOverlayColor.blue);
   overlay->SetOverlayAlpha(mOverlayHandle, mOverlayColor.alpha);
   overlay->SetOverlayMouseScale(mOverlayHandle, &ovrMouseScale);
   overlay->SetOverlayInputMethod(mOverlayHandle, mInputMethod);
   overlay->SetOverlayWidthInMeters(mOverlayHandle, mOverlayWidth);

   // NOTE: if flags in openvr change, double check this
   for (U32 i = vr::VROverlayFlags_None; i <= vr::VROverlayFlags_ShowTouchPadScrollWheel; i++)
   {
      overlay->SetOverlayFlag(mOverlayHandle, (vr::VROverlayFlags)i, mOverlayFlags & (1 << i));
   }

   mOverlayDirty = false;
}

void OpenVROverlay::showOverlay()
{
   updateOverlay();
   if (mOverlayHandle == NULL)
      return;

   if (mOverlayType != OVERLAYTYPE_DASHBOARD)
   {
      vr::EVROverlayError err = vr::VROverlay()->ShowOverlay(mOverlayHandle);
      if (err != vr::VROverlayError_None)
      {
         Con::errorf("VR Overlay error!");
      }
   }

   if (!mStagingTexture)
   {
      renderFrame(false, false);
   }
}

void OpenVROverlay::hideOverlay()
{
   if (mOverlayHandle == NULL)
      return;

   if (mOverlayType != OVERLAYTYPE_DASHBOARD)
   {
      vr::VROverlay()->HideOverlay(mOverlayHandle);
   }
}


bool OpenVROverlay::isOverlayVisible()
{
   if (mOverlayHandle == NULL)
      return false;

   return vr::VROverlay()->IsOverlayVisible(mOverlayHandle);
}

bool OpenVROverlay::isOverlayHoverTarget()
{
   if (mOverlayHandle == NULL)
      return false;

   return vr::VROverlay()->IsHoverTargetOverlay(mOverlayHandle);
}


bool OpenVROverlay::isGamepadFocussed()
{
   if (mOverlayHandle == NULL)
      return false;

   return vr::VROverlay()->GetGamepadFocusOverlay() == mOverlayHandle;
}

bool OpenVROverlay::isActiveDashboardOverlay()
{
   return false; // TODO WHERE DID I GET THIS FROM
}

MatrixF OpenVROverlay::getTransformForOverlayCoordinates(const Point2F &pos)
{
   if (mOverlayHandle == NULL)
      return MatrixF::Identity;

   vr::HmdVector2_t vec;
   vec.v[0] = pos.x;
   vec.v[1] = pos.y;
   vr::HmdMatrix34_t outMat;
   MatrixF outTorqueMat;
   if (vr::VROverlay()->GetTransformForOverlayCoordinates(mOverlayHandle, mTrackingOrigin, vec, &outMat) != vr::VROverlayError_None)
      return MatrixF::Identity;

   MatrixF vrMat(1);
   vrMat = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(outMat);
   OpenVRUtil::convertTransformFromOVR(vrMat, outTorqueMat);
   return outTorqueMat;
}

bool OpenVROverlay::castRay(const Point3F &origin, const Point3F &direction, RayInfo *info)
{
   if (mOverlayHandle == NULL)
      return false;

   vr::VROverlayIntersectionParams_t params;
   vr::VROverlayIntersectionResults_t result;

   Point3F ovrOrigin = OpenVRUtil::convertPointToOVR(origin);
   Point3F ovrDirection = OpenVRUtil::convertPointToOVR(direction);

   params.eOrigin = mTrackingOrigin;
   params.vSource.v[0] = ovrOrigin.x;
   params.vSource.v[1] = ovrOrigin.y;
   params.vSource.v[2] = ovrOrigin.z;
   params.vDirection.v[0] = ovrDirection.x;
   params.vDirection.v[1] = ovrDirection.y;
   params.vDirection.v[2] = ovrDirection.z;

   bool rayHit = vr::VROverlay()->ComputeOverlayIntersection(mOverlayHandle, &params, &result);

   if (rayHit && info)
   {
      info->t = result.fDistance;
      info->point = OpenVRUtil::convertPointFromOVR(result.vPoint); // TODO: need to transform this FROM vr-space
      info->normal = OpenVRUtil::convertPointFromOVR(result.vNormal);
      info->texCoord = Point2F(result.vUVs.v[0], result.vUVs.v[1]);
      info->object = NULL;
      info->userData = this;
   }

   return rayHit;
}

void OpenVROverlay::moveGamepadFocusToNeighbour()
{

}

void OpenVROverlay::handleOpenVREvents()
{
   if (mManualMouseHandling)
   {
      // tell OpenVR to make some events for us
      for (vr::TrackedDeviceIndex_t unDeviceId = 1; unDeviceId < vr::k_unControllerStateAxisCount; unDeviceId++)
      {
         if (vr::VROverlay()->HandleControllerOverlayInteractionAsMouse(mOverlayHandle, unDeviceId))
         {
            break;
         }
      }
   }


   vr::VREvent_t vrEvent;
   while (vr::VROverlay()->PollNextOverlayEvent(mOverlayHandle, &vrEvent, sizeof(vrEvent)))
   {
      InputEventInfo eventInfo;
      eventInfo.deviceType = MouseDeviceType;
      eventInfo.deviceInst = 0;
      eventInfo.objType = SI_AXIS;
      eventInfo.modifier = (InputModifiers)0;
      eventInfo.ascii = 0;

      //Con::printf("Overlay event %i", vrEvent.eventType);

      switch (vrEvent.eventType)
      {
      case vr::VREvent_MouseMove:
      {
         //Con::printf("mousemove %f,%f", vrEvent.data.mouse.x, vrEvent.data.mouse.y);
         eventInfo.objType = SI_AXIS;
         eventInfo.objInst = SI_XAXIS;
         eventInfo.action = SI_MAKE;
         eventInfo.fValue = getExtent().x * vrEvent.data.mouse.x;
         processMouseEvent(eventInfo);

         eventInfo.objType = SI_AXIS;
         eventInfo.objInst = SI_YAXIS;
         eventInfo.action = SI_MAKE;
         eventInfo.fValue = getExtent().y * (1.0 - vrEvent.data.mouse.y);
         processMouseEvent(eventInfo);
      }
      break;

      case vr::VREvent_MouseButtonDown:
      {
         eventInfo.objType = SI_BUTTON;
         eventInfo.objInst = (InputObjectInstances)OpenVRUtil::convertOpenVRButtonToTorqueButton(vrEvent.data.mouse.button);
         eventInfo.action = SI_MAKE;
         eventInfo.fValue = 1.0f;
         processMouseEvent(eventInfo);
      }
      break;

      case vr::VREvent_MouseButtonUp:
      {
         eventInfo.objType = SI_BUTTON;
         eventInfo.objInst = (InputObjectInstances)OpenVRUtil::convertOpenVRButtonToTorqueButton(vrEvent.data.mouse.button);
         eventInfo.action = SI_BREAK;
         eventInfo.fValue = 0.0f;
         processMouseEvent(eventInfo);
      }
      break;

      case vr::VREvent_OverlayShown:
      {
         markDirty();
      }
      break;

      case vr::VREvent_Quit:
         AssertFatal(false, "WTF is going on here");
         break;

      case vr::VREvent_KeyboardCharInput:
      case vr::VREvent_KeyboardDone:
         updateTextControl((GuiControl*)vrEvent.data.keyboard.uUserValue);
         break;
      }

   }

   if (mThumbOverlayHandle != vr::k_ulOverlayHandleInvalid)
   {
      while (vr::VROverlay()->PollNextOverlayEvent(mThumbOverlayHandle, &vrEvent, sizeof(vrEvent)))
      {
         switch (vrEvent.eventType)
         {
         case vr::VREvent_OverlayShown:
         {
            markDirty();
         }
         break;
         }
      }
   }
}

void OpenVROverlay::updateTextControl(GuiControl* ctrl)
{
   if (!ctrl)
      return;

   GuiTextCtrl* textCtrl = dynamic_cast<GuiTextCtrl*>(ctrl);
   if (textCtrl)
   {
      char text[GuiTextCtrl::MAX_STRING_LENGTH];
      vr::VROverlay()->GetKeyboardText(text, GuiTextCtrl::MAX_STRING_LENGTH);
      textCtrl->setText(text);
   }
}

void OpenVROverlay::onFrameRendered()
{
   vr::IVROverlay *overlay = vr::VROverlay();
   if (!overlay || !mOverlayHandle)
      return;

   updateOverlay();

   Point2I desiredSize = mTarget->getSize();
   if (mStagingTexture.isNull() || mStagingTexture.getWidthHeight() != desiredSize)
   {
      Point2I sz = mStagingTexture.getWidthHeight();
      mStagingTexture.set(desiredSize.x, desiredSize.y, mTargetFormat, &VRTextureProfile, "OpenVROverlay staging texture");
   }
   mTarget->resolveTo(mStagingTexture);

   vr::Texture_t tex;
   if (GFX->getAdapterType() == Direct3D11)
   {
      tex = { (void*)static_cast<GFXD3D11TextureObject*>(mStagingTexture.getPointer())->getResource(), vr::API_DirectX, vr::ColorSpace_Auto };
   }
#ifdef TORQUE_OPENGL
   else if (GFX->getAdapterType() == OpenGL)
   {
      tex = { (void*)static_cast<GFXGLTextureObject*>(mStagingTexture.getPointer())->getHandle(), vr::API_OpenGL, vr::ColorSpace_Auto };

   }
#endif
   else
   {
      return;
   }

   //mStagingTexture->dumpToDisk("PNG", "D:\\test.png");

   vr::EVROverlayError err = overlay->SetOverlayTexture(mOverlayHandle, &tex);
   if (err != vr::VROverlayError_None)
   {
      Con::errorf("VR: Error setting overlay texture.");
   }

   //Con::printf("Overlay visible ? %s", vr::VROverlay()->IsOverlayVisible(mOverlayHandle) ? "YES" : "NO");
}

void OpenVROverlay::enableKeyboardTranslation()
{
   vr::IVROverlay *overlay = vr::VROverlay();
   if (!overlay || !mOverlayHandle)
      return;

   GuiTextEditCtrl* ctrl = dynamic_cast<GuiTextEditCtrl*>(getFirstResponder());
   if (ctrl)
   {
      vr::EGamepadTextInputMode inputMode = ctrl->isPasswordText() ? vr::k_EGamepadTextInputModePassword : vr::k_EGamepadTextInputModeNormal;
      char text[GuiTextCtrl::MAX_STRING_LENGTH + 1];
      ctrl->getText(text);
      overlay->ShowKeyboardForOverlay(mOverlayHandle, inputMode, vr::k_EGamepadTextInputLineModeSingleLine, ctrl->getTooltip().c_str(), GuiTextCtrl::MAX_STRING_LENGTH, text, false, (uint64_t)ctrl);
   }
}

void OpenVROverlay::disableKeyboardTranslation()
{
   vr::IVROverlay *overlay = vr::VROverlay();
   if (!overlay || !mOverlayHandle)
      return;

   overlay->HideKeyboard();
}

void OpenVROverlay::setNativeAcceleratorsEnabled(bool enabled)
{
}

DefineEngineMethod(OpenVROverlay, showOverlay, void, (), , "")
{
   object->showOverlay();
}

DefineEngineMethod(OpenVROverlay, hideOverlay, void, (), , "")
{
   object->hideOverlay();
}