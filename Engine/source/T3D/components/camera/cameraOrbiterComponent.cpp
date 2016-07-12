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

#include "T3D/components/camera/cameraOrbiterComponent.h"
#include "core/util/safeDelete.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/mathUtils.h"

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
CameraOrbiterComponent::CameraOrbiterComponent() : Component()
{
   mMinOrbitDist = 0.0f;
   mMaxOrbitDist = 0.0f;
   mCurOrbitDist = 8.0f;
   mPosition.set(0.0f, 0.0f, 0.0f);

   mMaxPitchAngle = 70;
   mMinPitchAngle = -10;

   mRotation.set(0, 0, 0);

   mCamera = NULL;
}

CameraOrbiterComponent::~CameraOrbiterComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(CameraOrbiterComponent);

bool CameraOrbiterComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void CameraOrbiterComponent::onRemove()
{
   Parent::onRemove();
}
void CameraOrbiterComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("orbitDistance", TypeF32, Offset(mCurOrbitDist, CameraOrbiterComponent), "Object world orientation.");
   addField("Rotation", TypeRotationF, Offset(mRotation, CameraOrbiterComponent), "Object world orientation.");
   addField("maxPitchAngle", TypeF32, Offset(mMaxPitchAngle, CameraOrbiterComponent), "Object world orientation.");
   addField("minPitchAngle", TypeF32, Offset(mMinPitchAngle, CameraOrbiterComponent), "Object world orientation.");
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void CameraOrbiterComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   CameraComponent *cam = mOwner->getComponent<CameraComponent>();
   if (cam)
   {
      mCamera = cam;
   }
}

void CameraOrbiterComponent::onComponentRemove()
{
   Parent::onComponentRemove();
}

void CameraOrbiterComponent::componentAddedToOwner(Component *comp)
{
   if (comp->getId() == getId())
      return;

   //test if this is a shape component!
   CameraComponent *camComponent = dynamic_cast<CameraComponent*>(comp);
   if (camComponent)
   {
      mCamera = camComponent;
   }
}

void CameraOrbiterComponent::componentRemovedFromOwner(Component *comp)
{
   if (comp->getId() == getId()) //?????????
      return;

   //test if this is a shape component!
   CameraComponent *camComponent = dynamic_cast<CameraComponent*>(comp);
   if (camComponent)
   {
      mCamera = NULL;
   }
}

U32 CameraOrbiterComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   return retMask;
}

void CameraOrbiterComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
}

void CameraOrbiterComponent::processTick()
{
   Parent::processTick();

   if (!mOwner)
      return;

   if (mCamera)
   {
      //Clamp our pitch to whatever range we allow, first.
      mRotation.x = mClampF(mRotation.x, mDegToRad(mMinPitchAngle), mDegToRad(mMaxPitchAngle));

      MatrixF ownerTrans = mOwner->getRenderTransform();
      Point3F ownerPos = ownerTrans.getPosition();

      Point3F pos;
      pos.x = mCurOrbitDist * mSin(mRotation.x + M_HALFPI_F) * mCos(-1.0f * (mRotation.z + M_HALFPI_F));
      pos.y = mCurOrbitDist * mSin(mRotation.x + M_HALFPI_F) * mSin(-1.0f * (mRotation.z + M_HALFPI_F));
      pos.z = mCurOrbitDist * mSin(mRotation.x);

      //orient the camera towards the owner
      VectorF ownerVec = ownerPos - pos;
      ownerVec.normalize();

      MatrixF xRot, zRot, cameraMatrix;
      xRot.set(EulerF(mRotation.x, 0.0f, 0.0f));
      zRot.set(EulerF(0.0f, 0.0f, mRotation.z));

      cameraMatrix.mul(zRot, xRot);
      cameraMatrix.getColumn(1, &ownerVec);
      cameraMatrix.setColumn(3, pos - ownerVec * pos);

      RotationF camRot = RotationF(cameraMatrix);

      if (camRot != mCamera->getRotOffset())
         mCamera->setRotation(camRot);

      if (pos != mCamera->getPosOffset())
         mCamera->setPosition(pos);
   }
}