#include "platform/input/openVR/openVROverlay.h"

ImplementEnumType(OpenVROverlayType,
   "Desired overlay type for OpenVROverlay. .\n\n"
   "@ingroup OpenVR")
{ OpenVROverlay::OVERLAYTYPE_OVERLAY, "Overlay" },
{ OpenVROverlay::OVERLAYTYPE_DASHBOARD, "Dashboard" },
EndImplementEnumType;

OpenVROverlay::OpenVROverlay()
{

}

OpenVROverlay::~OpenVROverlay()
{

}

void OpenVROverlay::initPersistFields()
{
   Parent::initPersistFields();
}

bool OpenVROverlay::onAdd()
{
   if (Parent::onAdd())
   {
      mOverlayTypeDirty = true;
      mOverlayDirty = true;
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
}

void OpenVROverlay::resetOverlay()
{
   mOverlayTypeDirty = false;
}

void OpenVROverlay::updateOverlay()
{
   if (mOverlayTypeDirty)
      resetOverlay();

   // Update params TODO
   mOverlayDirty = false;
}

void OpenVROverlay::showOverlay()
{
   if (mOverlayHandle == NULL)
      return;

   vr::VROverlay()->ShowOverlay(mOverlayHandle);
}

void OpenVROverlay::hideOverlay()
{
   if (mOverlayHandle == NULL)
      return;

   vr::VROverlay()->HideOverlay(mOverlayHandle);
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

MatrixF OpenVROverlay::getTransformForOverlayCoordinates(const vr::ETrackingUniverseOrigin trackingOrigin, const Point2F &pos)
{
   if (mOverlayHandle == NULL)
      return MatrixF::Identity;

   vr::HmdVector2_t vec;
   vec.v[0] = pos.x;
   vec.v[1] = pos.y;
   vr::HmdMatrix34_t outMat;
   MatrixF outTorqueMat;
   if (vr::VROverlay()->GetTransformForOverlayCoordinates(mOverlayHandle, trackingOrigin, vec, &outMat) != vr::VROverlayError_None)
      return MatrixF::Identity;

   MatrixF vrMat(1);
   vrMat = OpenVRUtil::convertSteamVRAffineMatrixToMatrixFPlain(outMat);
   OpenVRUtil::convertTransformFromOVR(vrMat, outTorqueMat);
   return outTorqueMat;
}

bool OpenVROverlay::castRay(const  vr::ETrackingUniverseOrigin trackingOrigin, const Point3F &origin, const Point3F &direction, RayInfo *info)
{
   if (mOverlayHandle == NULL)
      return false;

   vr::VROverlayIntersectionParams_t params;
   vr::VROverlayIntersectionResults_t result;

   params.eOrigin = trackingOrigin;
   params.vSource.v[0] = origin.x;
   params.vSource.v[1] = origin.y;
   params.vSource.v[2] = origin.z;
   params.vDirection.v[0] = direction.x; // TODO: need to transform this to vr-space
   params.vDirection.v[1] = direction.y;
   params.vDirection.v[2] = direction.z;

   bool rayHit = vr::VROverlay()->ComputeOverlayIntersection(mOverlayHandle, &params, &result);

   if (rayHit && info)
   {
      info->t = result.fDistance;
      info->point = Point3F(result.vPoint.v[0], result.vPoint.v[1], result.vPoint.v[2]); // TODO: need to transform this FROM vr-space
      info->normal = Point3F(result.vNormal.v[0], result.vNormal.v[1], result.vNormal.v[2]);
      info->texCoord = Point2F(result.vUVs.v[0], result.vUVs.v[1]);
      info->object = NULL;
      info->userData = this;
   }

   return rayHit;
}

void OpenVROverlay::moveGamepadFocusToNeighbour()
{

}

