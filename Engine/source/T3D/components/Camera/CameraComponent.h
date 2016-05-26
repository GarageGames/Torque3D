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

#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif
#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef CORE_INTERFACES_H
#include "T3D/components/coreInterfaces.h"
#endif

class SceneRenderState;
struct CameraScopeQuery;

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class CameraComponent : public Component, public CameraInterface
{
   typedef Component Parent;

   F32  mCameraFov;           ///< The camera vertical FOV in degrees.

   Point2F mClientScreen;     ///< The dimensions of the client's screen. Used to calculate the aspect ratio.

   F32 mCameraDefaultFov;            ///< Default vertical FOV in degrees.
   F32 mCameraMinFov;                ///< Min vertical FOV allowed in degrees.
   F32 mCameraMaxFov;                ///< Max vertical FOV allowed in degrees.

protected:
   Point3F mPosOffset;
   RotationF mRotOffset;

   StringTableEntry mTargetNode;
   S32 mTargetNodeIdx;

   bool mUseParentTransform;

   enum
   {
      FOVMask = Parent::NextFreeMask,
      OffsetMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
   };

public:
   CameraComponent();
   virtual ~CameraComponent();
   DECLARE_CONOBJECT(CameraComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   static bool _setCameraFov(void *object, const char *index, const char *data);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   static bool _setNode(void *object, const char *index, const char *data);
   static bool _setPosOffset(void *object, const char *index, const char *data);
   static bool _setRotOffset(void *object, const char *index, const char *data);

   void setRotOffset(RotationF rot)
   {
      mRotOffset = rot;
      setMaskBits(OffsetMask);
   }

   RotationF getRotOffset()
   {
      return mRotOffset;
   }

   Point3F getPosOffset()
   {
      return mPosOffset;
   }

   /// Gets the minimum viewing distance, maximum viewing distance, camera offsetand rotation
   /// for this object, if the world were to be viewed through its eyes
   /// @param   min   Minimum viewing distance
   /// @param   max   Maximum viewing distance
   /// @param   offset Offset of the camera from the origin in local space
   /// @param   rot   Rotation matrix
   virtual void getCameraParameters(F32 *min, F32* max, Point3F* offset, MatrixF* rot);

   /// Gets the camera to world space transform matrix
   /// @todo Find out what pos does
   /// @param   pos   TODO: Find out what this does
   /// @param   mat   Camera transform (out)
   virtual bool getCameraTransform(F32* pos, MatrixF* mat);

   /// Returns the vertical field of view in degrees for 
   /// this object if used as a camera.
   virtual F32 getCameraFov() { return mCameraFov; }

   /// Returns the default vertical field of view in degrees
   /// if this object is used as a camera.
   virtual F32 getDefaultCameraFov() { return mCameraDefaultFov; }

   /// Sets the vertical field of view in degrees for this 
   /// object if used as a camera.
   /// @param   yfov  The vertical FOV in degrees to test.
   virtual void setCameraFov(F32 fov);

   /// Returns true if the vertical FOV in degrees is within 
   /// allowable parameters of the datablock.
   /// @param   yfov  The vertical FOV in degrees to test.
   /// @see ShapeBaseData::cameraMinFov
   /// @see ShapeBaseData::cameraMaxFov
   virtual bool isValidCameraFov(F32 fov);
   /// @}

   virtual Frustum getFrustum();

   /// Control object scoping
   void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *camInfo);

   void setForwardVector(VectorF newForward, VectorF upVector = VectorF::Zero);
   void setPosition(Point3F newPos);
   void setRotation(RotationF newRot);

protected:
   DECLARE_CALLBACK(F32, validateCameraFov, (F32 fov));
};

#endif // CAMERA_BEHAVIOR_H
