#ifndef _OPENVROVERLAY_H_
#define _OPENVROVERLAY_H_

#ifndef _GUIOFFSCREENCANVAS_H_
#include "gui/core/guiOffscreenCanvas.h"
#endif
#ifndef _OPENVRDEVICE_H_
#include "platform/input/openVR/openVRProvider.h"
#endif
#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif


typedef vr::VROverlayInputMethod OpenVROverlayInputMethod;
typedef vr::VROverlayTransformType OpenVROverlayTransformType;
typedef vr::EGamepadTextInputMode OpenVRGamepadTextInputMode;
typedef vr::EGamepadTextInputLineMode OpenVRGamepadTextInputLineMode;
typedef vr::ETrackingResult OpenVRTrackingResult;
typedef vr::ETrackingUniverseOrigin OpenVRTrackingUniverseOrigin;
typedef vr::EOverlayDirection OpenVROverlayDirection;
typedef vr::EVRState OpenVRState;

class OpenVROverlay : public GuiOffscreenCanvas
{
public:
   typedef GuiOffscreenCanvas Parent;

   enum OverlayType
   {
      OVERLAYTYPE_OVERLAY,
      OVERLAYTYPE_DASHBOARD,
   };

   vr::VROverlayHandle_t mOverlayHandle;

   // Desired OpenVR state
   U32 mOverlayFlags;
   F32 mOverlayWidth;

   vr::VROverlayTransformType mOverlayTransformType;
   MatrixF mTransform;
   vr::TrackedDeviceIndex_t mTransformDeviceIndex;
   const char* mTransformDeviceComponent;


   vr::VROverlayInputMethod mInputMethod;
   Point2F mMouseScale;

   MatrixF mTrackingOrigin;
   vr::TrackedDeviceIndex_t mControllerDeviceIndex;

   bool mOverlayTypeDirty; ///< Overlay type is dirty
   bool mOverlayDirty; ///< Overlay properties are dirty
   OverlayType mOverlayType;

   //

   OpenVROverlay();
   virtual ~OpenVROverlay();

   static void initPersistFields();

   bool onAdd();
   void onRemove();

   void resetOverlay();
   void updateOverlay();

   void showOverlay();
   void hideOverlay();

   bool isOverlayVisible();
   bool isOverlayHoverTarget();

   bool isGamepadFocussed();
   bool isActiveDashboardOverlay();

   MatrixF getTransformForOverlayCoordinates(const vr::ETrackingUniverseOrigin trackingOrigin, const Point2F &pos);
   bool castRay(const vr::ETrackingUniverseOrigin trackingOrigin, const Point3F &origin, const Point3F &direction, RayInfo *info);

   void moveGamepadFocusToNeighbour();
};

typedef OpenVROverlay::OverlayType OpenVROverlayType;
DefineEnumType(OpenVROverlayType);


#endif
