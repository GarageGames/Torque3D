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

#include "T3D/turret/turretShape.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/stream/bitStream.h"
#include "math/mMath.h"
#include "math/mathIO.h"
#include "ts/tsShapeInstance.h"
#include "T3D/fx/cameraFXMgr.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/physics/physicsBody.h"

//----------------------------------------------------------------------------

// Client prediction
static F32 sMinWarpTicks = 0.5 ;        // Fraction of tick at which instant warp occures
static S32 sMaxWarpTicks = 3;           // Max warp duration in ticks

const U32 sClientCollisionMask = (TerrainObjectType     |
                                  StaticShapeObjectType |
                                  VehicleObjectType);

const U32 sServerCollisionMask = (sClientCollisionMask);

// Trigger objects that are not normally collided with.
static U32 sTriggerMask = ItemObjectType     |
                          TriggerObjectType  |
                          CorpseObjectType;

//----------------------------------------------------------------------------

ImplementEnumType( TurretShapeFireLinkType,
   "@brief How the weapons are linked to triggers for this TurretShape.\n\n"
   "@ingroup gameObjects\n\n")
   { TurretShapeData::FireTogether,    "FireTogether",   "All weapons fire under trigger 0.\n" },
   { TurretShapeData::GroupedFire,     "GroupedFire",    "Weapon mounts 0,2 fire under trigger 0, mounts 1,3 fire under trigger 1.\n" },
   { TurretShapeData::IndividualFire,  "IndividualFire", "Each weapon mount fires under its own trigger 0-3.\n" },
EndImplementEnumType;

IMPLEMENT_CO_DATABLOCK_V1(TurretShapeData);

ConsoleDocClass( TurretShapeData,
   "@brief Defines properties for a TurretShape object.\n\n"
   "@see TurretShape\n"
   "@see TurretShapeData\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( TurretShapeData, onMountObject, void, ( TurretShape* turret, SceneObject* obj, S32 node ),( turret, obj, node ),
   "@brief Informs the TurretShapeData object that a player is mounting it.\n\n"
   "@param turret The TurretShape object.\n"
   "@param obj The player that is mounting.\n"
   "@param node The node the player is mounting to.\n"
   "@note Server side only.\n"
);

IMPLEMENT_CALLBACK( TurretShapeData, onUnmountObject, void, ( TurretShape* turret, SceneObject* obj ),( turret, obj ),
   "@brief Informs the TurretShapeData object that a player is unmounting it.\n\n"
   "@param turret The TurretShape object.\n"
   "@param obj The player that is unmounting.\n"
   "@note Server side only.\n"
);

IMPLEMENT_CALLBACK( TurretShapeData, onStickyCollision, void, ( TurretShape* obj ),( obj ),
   "@brief Informs the TurretData object that it is now sticking to another object.\n\n"
   "This callback is only called if the TurretData::sticky property for this Turret is true.\n"
   "@param obj The Turret object that is colliding.\n"
   "@note Server side only.\n"
   "@see TurretShape, TurretData\n"
);

TurretShapeData::TurretShapeData()
{
   weaponLinkType = FireTogether;

   shadowEnable = true;

   zRotOnly = false;

   startLoaded = true;

   friction = 0;
   elasticity = 0;

   sticky = false;
   gravityMod = 1.0;
   maxVelocity = 25.0f;

   density = 2;
   drag = 0.5;

   cameraOffset = 0;

   maxHeading = 180.0f;
   minPitch = 90.0f;
   maxPitch = 90.0f;

   headingRate = -1;
   pitchRate = -1;

   headingNode = -1;
   pitchNode = -1;

   for (U32 i=0; i<NumMirrorDirectionNodes; ++i)
   {
      pitchNodes[i] = -1;
      headingNodes[i] = -1;
   }

   for (U32 i=0; i<ShapeBase::MaxMountedImages; ++i)
   {
      weaponMountNode[i] = -1;
   }
}

void TurretShapeData::initPersistFields()
{
   addField("zRotOnly",       TypeBool,         Offset(zRotOnly,       TurretShapeData),
      "@brief Should the turret allow only z rotations.\n\n"
      "True indicates that the turret may only be rotated on its z axis, just like the Item class.  "
      "This keeps the turret always upright regardless of the surface it lands on.\n");

   addField( "weaponLinkType", TYPEID< TurretShapeData::FireLinkType >(), Offset(weaponLinkType, TurretShapeData),
      "@brief Set how the mounted weapons are linked and triggered.\n\n"
      "<ul><li>FireTogether: All weapons fire under trigger 0.</li>"
      "<li>GroupedFire: Weapon mounts 0,2 fire under trigger 0, mounts 1,3 fire under trigger 1.</li>"
      "<li>IndividualFire: Each weapon mount fires under its own trigger 0-3.</li></ul>\n"
      "@see TurretShapeFireLinkType");

   addField("startLoaded",       TypeBool,       Offset(startLoaded,       TurretShapeData),
      "@brief Does the turret's mounted weapon(s) start in a loaded state.\n\n"
      "True indicates that all mounted weapons start in a loaded state.\n"
      "@see ShapeBase::setImageLoaded()");

   addField("cameraOffset",      TypeF32,       Offset(cameraOffset,       TurretShapeData),
      "Vertical (Z axis) height of the camera above the turret." );

   addField("maxHeading",        TypeF32,       Offset(maxHeading,         TurretShapeData),
      "@brief Maximum number of degrees to rotate from center.\n\n"
      "A value of 180 or more degrees indicates the turret may rotate completely around.\n");
   addField("minPitch",          TypeF32,       Offset(minPitch,           TurretShapeData),
      "@brief Minimum number of degrees to rotate down from straight ahead.\n\n");
   addField("maxPitch",          TypeF32,       Offset(maxPitch,           TurretShapeData),
      "@brief Maximum number of degrees to rotate up from straight ahead.\n\n");

   addField("headingRate",       TypeF32,       Offset(headingRate,        TurretShapeData),
      "@brief Degrees per second rotation.\n\n"
      "A value of 0 means no rotation is allowed.  A value less than 0 means the rotation is instantaneous.\n");
   addField("pitchRate",         TypeF32,       Offset(pitchRate,          TurretShapeData),
      "@brief Degrees per second rotation.\n\n"
      "A value of 0 means no rotation is allowed.  A value less than 0 means the rotation is instantaneous.\n");

   Parent::initPersistFields();
}

void TurretShapeData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeFlag(zRotOnly);

   stream->writeInt(weaponLinkType,NumFireLinkTypeBits);

   stream->write(cameraOffset);

   stream->write(maxHeading);
   stream->write(minPitch);
   stream->write(maxPitch);

   stream->write(headingRate);
   stream->write(pitchRate);
}

void TurretShapeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   zRotOnly = stream->readFlag();

   weaponLinkType = (FireLinkType)stream->readInt(NumFireLinkTypeBits);

   stream->read(&cameraOffset);

   stream->read(&maxHeading);
   stream->read(&minPitch);
   stream->read(&maxPitch);

   stream->read(&headingRate);
   stream->read(&pitchRate);
}

bool TurretShapeData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;

   // We have mShape at this point.  Resolve nodes.
   headingNode = mShape->findNode("heading");
   pitchNode = mShape->findNode("pitch");

   // Find any mirror pitch nodes
   for (U32 i = 0; i < NumMirrorDirectionNodes; ++i)
   {
      char name[32];
      dSprintf(name, 31, "pitch%d", i+1);
      pitchNodes[i] = mShape->findNode(name);

      dSprintf(name, 31, "heading%d", i+1);
      headingNodes[i] = mShape->findNode(name);
   }

   // Resolve weapon mount point node indexes
   for (U32 i = 0; i < ShapeBase::MaxMountedImages; i++) {
      char fullName[256];
      dSprintf(fullName,sizeof(fullName),"weaponMount%d",i);
      weaponMountNode[i] = mShape->findNode(fullName);
   }

   // Recoil animations
   recoilSequence[0] = mShape->findSequence("light_recoil");
   recoilSequence[1] = mShape->findSequence("medium_recoil");
   recoilSequence[2] = mShape->findSequence("heavy_recoil");

   // Optional sequences used when the turret rotates
   pitchSequence = mShape->findSequence("pitch");
   headingSequence = mShape->findSequence("heading");

   return true;
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(TurretShape);

ConsoleDocClass( TurretShape,
   "@ingroup gameObjects\n"
);

TurretShape::TurretShape()
{
   mTypeMask |= VehicleObjectType | DynamicShapeObjectType;
   mDataBlock = 0;

   allowManualRotation = true;
   allowManualFire = true;

   mTurretDelta.rot = Point3F(0.0f, 0.0f, 0.0f);
   mTurretDelta.rotVec = VectorF(0.0f, 0.0f, 0.0f);
   mTurretDelta.dt = 1;

   mRot = mTurretDelta.rot;

   mPitchAllowed = true;
   mHeadingAllowed = true;

   mPitchRate = -1;
   mHeadingRate = -1;

   mPitchUp = 0;
   mPitchDown = 0;
   mHeadingMax = mDegToRad(180.0f);

   mRespawn = false;

   mPitchThread = 0;
   mHeadingThread = 0;

   mSubclassTurretShapeHandlesScene = false;

   // For the Item class
   mSubclassItemHandlesScene = true;
}

TurretShape::~TurretShape()
{
}

//----------------------------------------------------------------------------

void TurretShape::initPersistFields()
{
   addField("respawn",        TypeBool,      Offset(mRespawn,      TurretShape),
      "@brief Respawn the turret after it has been destroyed.\n\n"
      "If true, the turret will respawn after it is destroyed.\n");

   Parent::initPersistFields();
}

bool TurretShape::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   // Add this object to the scene
   if (!mSubclassTurretShapeHandlesScene)
   {
      addToScene();
   }

   if (isServerObject() && !mSubclassTurretShapeHandlesScene)
   {
      scriptOnAdd();
   }

   return true;
}

void TurretShape::onRemove()
{
   Parent::onRemove();

   if (!mSubclassTurretShapeHandlesScene)
   {
      scriptOnRemove();

      // Remove this object from the scene
      removeFromScene();
   }
}

bool TurretShape::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<TurretShapeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   // Mark these nodes for control by code only (will not animate in a sequence)
   if (mDataBlock->headingNode != -1)
      mShapeInstance->setNodeAnimationState(mDataBlock->headingNode, TSShapeInstance::MaskNodeHandsOff);
   if (mDataBlock->pitchNode != -1)
      mShapeInstance->setNodeAnimationState(mDataBlock->pitchNode, TSShapeInstance::MaskNodeHandsOff);
   for (U32 i=0; i<TurretShapeData::NumMirrorDirectionNodes; ++i)
   {
      if (mDataBlock->pitchNodes[i] != -1)
      {
         mShapeInstance->setNodeAnimationState(mDataBlock->pitchNodes[i], TSShapeInstance::MaskNodeHandsOff);
      }

      if (mDataBlock->headingNodes[i] != -1)
      {
         mShapeInstance->setNodeAnimationState(mDataBlock->headingNodes[i], TSShapeInstance::MaskNodeHandsOff);
      }
   }

   if (mIsZero(mDataBlock->pitchRate))
   {
      mPitchAllowed = false;
   }
   else
   {
      mPitchAllowed = true;
      if (mDataBlock->pitchRate > 0)
      {
         mPitchRate = mDegToRad(mDataBlock->pitchRate);
      }
      else
      {
         mPitchRate = -1;
      }
   }

   if (mIsZero(mDataBlock->headingRate))
   {
      mHeadingAllowed = false;
   }
   else
   {
      mHeadingAllowed = true;
      if (mDataBlock->headingRate > 0)
      {
         mHeadingRate = mDegToRad(mDataBlock->headingRate);
      }
      else
      {
         mHeadingRate = -1;
      }
   }

   mPitchUp = -mDegToRad(mDataBlock->maxPitch);
   mPitchDown = mDegToRad(mDataBlock->minPitch);
   mHeadingMax = mDegToRad(mDataBlock->maxHeading);

   // Create Recoil thread if any recoil sequences are specified.
   // Note that the server player does not play this animation.
   mRecoilThread = 0;
   if (isGhost())
      for (U32 s = 0; s < TurretShapeData::NumRecoilSequences; s++)
         if (mDataBlock->recoilSequence[s] != -1) {
            mRecoilThread = mShapeInstance->addThread();
            mShapeInstance->setSequence(mRecoilThread, mDataBlock->recoilSequence[s], 0);
            mShapeInstance->setTimeScale(mRecoilThread, 0);
            break;
         }

   // Reset the image state driven animation thread.  This will be properly built
   // in onImageStateAnimation() when needed.
   mImageStateThread = 0;

   // Optional rotation threads.  These only play on the client.
   mPitchThread = 0;
   mHeadingThread = 0;
   if (isGhost())
   {
      if (mDataBlock->pitchSequence != -1)
      {
         mPitchThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mPitchThread, mDataBlock->pitchSequence, 0);
         mShapeInstance->setTimeScale(mPitchThread, 0);
      }
      if (mDataBlock->headingSequence != -1)
      {
         mHeadingThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mHeadingThread, mDataBlock->headingSequence, 0);
         mShapeInstance->setTimeScale(mHeadingThread, 0);
      }
   }

   if (!mSubclassTurretShapeHandlesScene)
   {
      scriptOnNewDataBlock();
   }

   return true;
}

//----------------------------------------------------------------------------

void TurretShape::updateAnimation(F32 dt)
{
   if (mRecoilThread)
      mShapeInstance->advanceTime(dt,mRecoilThread);
   if (mImageStateThread)
      mShapeInstance->advanceTime(dt,mImageStateThread);

   // Update any pitch and heading threads
   if (mPitchThread)
   {
      F32 d = mPitchDown - mPitchUp;
      if (!mIsZero(d))
      {
         F32 pos = (mRot.x - mPitchUp) / d;
         mShapeInstance->setPos(mPitchThread, mClampF(pos, 0.0f, 1.0f));
      }
   }
   if (mHeadingThread)
   {
      F32 pos = 0.0f;
      if (mHeadingMax < mDegToRad(180.0f))
      {
         F32 d = mHeadingMax * 2.0f;
         if (!mIsZero(d))
         {
            pos = (mRot.z + mHeadingMax) / d;
         }
      }
      else
      {
         pos = mRot.z / M_2PI;
         if (pos < 0.0f)
         {
            // We don't want negative rotations to simply mirror the
            // positive rotations but to animate into them as if -0.0
            // is equivalent to 1.0.
            pos = mFmod(pos, 1.0f) + 1.0f;
         }
         if (pos > 1.0f)
         {
            pos = mFmod(pos, 1.0f);
         }
      }
      mShapeInstance->setPos(mHeadingThread, mClampF(pos, 0.0f, 1.0f));
   }
}

//----------------------------------------------------------------------------

void TurretShape::onImage(U32 imageSlot, bool unmount)
{
   // Clear out any previous image state animation
   if (mImageStateThread)
   {
      mShapeInstance->destroyThread(mImageStateThread);
      mImageStateThread = 0;
   }
}

void TurretShape::onImageRecoil( U32, ShapeBaseImageData::StateData::RecoilState state )
{
   if ( mRecoilThread )
   {
      if ( state != ShapeBaseImageData::StateData::NoRecoil )
      {
         S32 stateIndex = state - ShapeBaseImageData::StateData::LightRecoil;
         if ( mDataBlock->recoilSequence[stateIndex] != -1 )
         {
            mShapeInstance->setSequence( mRecoilThread, mDataBlock->recoilSequence[stateIndex], 0 );
            mShapeInstance->setTimeScale( mRecoilThread, 1 );
         }
      }
   }
}

void TurretShape::onImageStateAnimation(U32 imageSlot, const char* seqName, bool direction, bool scaleToState, F32 stateTimeOutValue)
{
   if (isGhost())
   {
      S32 seqIndex = mShapeInstance->getShape()->findSequence(seqName);

      if (seqIndex != -1)
      {
         if (!mImageStateThread)
         {
            mImageStateThread = mShapeInstance->addThread();
         }

         mShapeInstance->setSequence( mImageStateThread, seqIndex, 0 );

         F32 timeScale = (scaleToState && stateTimeOutValue) ?
            mShapeInstance->getDuration(mImageStateThread) / stateTimeOutValue : 1.0f;

         mShapeInstance->setTimeScale( mImageStateThread, direction ? timeScale : -timeScale );
      }
   }
}

//----------------------------------------------------------------------------

const char* TurretShape::getStateName()
{
   if (mDamageState != Enabled)
      return "Dead";
   if (isMounted())
      return "Mounted";
   return "Ready";
}

void TurretShape::updateDamageLevel()
{
   if (!isGhost())
      setDamageState((mDamage >= mDataBlock->maxDamage)? Destroyed: Enabled);
   if (mDamageThread)
      mShapeInstance->setPos(mDamageThread, mDamage / mDataBlock->destroyedLevel);
}

//----------------------------------------------------------------------------

void TurretShape::processTick(const Move* move)
{
   // Image Triggers
   if (getAllowManualFire() && move && mDamageState == Enabled)
   {
      switch(mDataBlock->weaponLinkType)
      {
         case TurretShapeData::FireTogether:
         {
            setImageTriggerState(0,move->trigger[0]);
            setImageTriggerState(1,move->trigger[0]);
            setImageTriggerState(2,move->trigger[0]);
            setImageTriggerState(3,move->trigger[0]);

            setImageAltTriggerState(0,move->trigger[1]);
            setImageAltTriggerState(1,move->trigger[1]);
            setImageAltTriggerState(2,move->trigger[1]);
            setImageAltTriggerState(3,move->trigger[1]);

            break;
         }

         case TurretShapeData::GroupedFire:
         {
            setImageTriggerState(0,move->trigger[0]);
            setImageTriggerState(1,move->trigger[1]);
            setImageTriggerState(2,move->trigger[0]);
            setImageTriggerState(3,move->trigger[1]);

            break;
         }

         case TurretShapeData::IndividualFire:
         {
            setImageTriggerState(0,move->trigger[0]);
            setImageTriggerState(1,move->trigger[1]);
            setImageTriggerState(2,move->trigger[2]);
            setImageTriggerState(3,move->trigger[3]);

            break;
         }
      }
   }

   Parent::processTick(move);

   // Change our type based on our rest state
   if (mAtRest)
   {
      // At rest so we're static
      mTypeMask &= ~DynamicShapeObjectType;
      mTypeMask |= StaticObjectType | StaticShapeObjectType;
   }
   else
   {
      // Not at rest so we're dynamic
      mTypeMask &= ~StaticObjectType;
      mTypeMask &= ~StaticShapeObjectType;
      mTypeMask |= DynamicShapeObjectType;
   }

   if (!isGhost())
      updateAnimation(TickSec);

   if (isMounted()) {
      MatrixF mat;
      mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
      ShapeBase::setTransform(mat);
      ShapeBase::setRenderTransform(mat);
   }

   updateMove(move);
}

void TurretShape::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);

   if (isMounted()) {
      MatrixF mat;
      mMount.object->getRenderMountTransform( dt, mMount.node, mMount.xfm, &mat );
      ShapeBase::setRenderTransform(mat);
   }

   // Orientation
   Point3F rot = mTurretDelta.rot + mTurretDelta.rotVec * dt;

   // Make sure we don't interpolate past the limits
   _applyLimits(rot);

   _setRotation(rot);
}

void TurretShape::advanceTime(F32 dt)
{
   // If there were any ShapeBase script threads that
   // have played, then we need to update all code
   // controlled nodes.  This is done before the Parent
   // call as script threads may play and be destroyed
   // before our code is called.
   bool updateNodes = false;
   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mScriptThread[i];
      if (st.thread)
      {
         updateNodes = true;
         break;
      }
   }

   // If there is a recoil or image-based thread then
   // we also need to update the nodes.
   if (mRecoilThread || mImageStateThread)
      updateNodes = true;

   Parent::advanceTime(dt);

   updateAnimation(dt);

   if (updateNodes)
   {
      _updateNodes(mRot);
   }
}

void TurretShape::setTransform( const MatrixF& mat )
{
   if (mDataBlock && mDataBlock->zRotOnly)
   {
      // Allow Item::setTransform() to do the work
      Parent::setTransform( mat );
   }
   else
   {
      // Do the transform work here to avoid Item's restriction on rotation
      ShapeBase::setTransform( mat );

      if ( !mStatic )
      {
         mAtRest = false;
         mAtRestCounter = 0;
      }

      if ( mPhysicsRep )
         mPhysicsRep->setTransform( getTransform() );

      setMaskBits( Item::RotationMask | Item::PositionMask | Item::NoWarpMask );
   }
}

void TurretShape::updateMove(const Move* move)
{
   PROFILE_SCOPE( TurretShape_UpdateMove );

   if (!move)
      return;

   Point3F vec, pos;

   // Update orientation
   mTurretDelta.rotVec = mRot;

   VectorF rotVec(0, 0, 0);
   if (getAllowManualRotation())
   {
      if (mPitchAllowed)
      {
         rotVec.x = move->pitch * 2.0f;   // Assume that our -2PI to 2PI range was clamped to -PI to PI in script;
         if (mPitchRate > 0)
         {
            rotVec.x *= mPitchRate * TickSec;
         }
      }
      if (mHeadingAllowed)
      {
         rotVec.z = move->yaw * 2.0f;     // Assume that our -2PI to 2PI range was clamped to -PI to PI in script
         if (mHeadingRate > 0)
         {
            rotVec.z *= mHeadingRate * TickSec;
         }
      }
   }

   mRot.x += rotVec.x;
   mRot.z += rotVec.z;
   _applyLimits(mRot);

   if (isServerObject())
   {
      // As this ends up animating shape nodes, we have no sense of a transform and
      // render transform.  Therefore we treat this as the true transform and leave the
      // client shape node changes to interpolateTick() as the render transform.  Otherwise
      // on the client we'll have this node change from processTick() and then backstepping
      // and catching up to the true node change in interpolateTick(), which causes the
      // turret to stutter.
      _setRotation( mRot );
   }
   else
   {
      // If on the client, calc delta for backstepping
      mTurretDelta.rot = mRot;
      mTurretDelta.rotVec = mTurretDelta.rotVec - mTurretDelta.rot;
   }

   setMaskBits(TurretUpdateMask);
}

bool TurretShape::getNodeTransform(S32 node, MatrixF& mat)
{
   if (node == -1)
      return false;

   MatrixF nodeTransform = mShapeInstance->mNodeTransforms[node];
   const Point3F& scale = getScale();

   // The position of the node needs to be scaled.
   Point3F position = nodeTransform.getPosition();
   position.convolve( scale );
   nodeTransform.setPosition( position );

   mat.mul(mObjToWorld, nodeTransform);
   return true;
}

bool TurretShape::getWorldNodeTransform(S32 node, MatrixF& mat)
{
   MatrixF nodeMat;
   if (!getNodeTransform(node, nodeMat))
      return false;

   nodeMat.affineInverse();
   mat = nodeMat;
   return true;
}

void TurretShape::_setRotation(const Point3F& rot)
{
   _updateNodes(rot);

   mShapeInstance->animate();

   mRot = rot;
}

void TurretShape::_updateNodes(const Point3F& rot)
{
   EulerF xRot(rot.x, 0.0f, 0.0f);
   EulerF zRot(0.0f, 0.0f, rot.z);

   // Set heading
   S32 node = mDataBlock->headingNode;
   if (node != -1)
   {
      MatrixF* mat = &mShapeInstance->mNodeTransforms[node];
      Point3F defaultPos = mShapeInstance->getShape()->defaultTranslations[node];
      Quat16 defaultRot = mShapeInstance->getShape()->defaultRotations[node];

      QuatF qrot(zRot);
      qrot *= defaultRot.getQuatF();
      qrot.setMatrix( mat );      
      mat->setColumn(3, defaultPos);
   }

   // Set pitch
   node = mDataBlock->pitchNode;
   if (node != -1)
   {
      MatrixF* mat = &mShapeInstance->mNodeTransforms[node];
      Point3F defaultPos = mShapeInstance->getShape()->defaultTranslations[node];
      Quat16 defaultRot = mShapeInstance->getShape()->defaultRotations[node];

      QuatF qrot(xRot);
      qrot *= defaultRot.getQuatF();
      qrot.setMatrix( mat );    
      mat->setColumn(3, defaultPos);
   }

   // Now the mirror direction nodes, if any
   for (U32 i=0; i<TurretShapeData::NumMirrorDirectionNodes; ++i)
   {
      node = mDataBlock->pitchNodes[i];
      if (node != -1)
      {
         MatrixF* mat = &mShapeInstance->mNodeTransforms[node];
         Point3F defaultPos = mShapeInstance->getShape()->defaultTranslations[node];
         Quat16 defaultRot = mShapeInstance->getShape()->defaultRotations[node];         

         QuatF qrot(xRot);
         qrot *= defaultRot.getQuatF();
         qrot.setMatrix( mat );    
         mat->setColumn(3, defaultPos);
      }

      node = mDataBlock->headingNodes[i];
      if (node != -1)
      {
         MatrixF* mat = &mShapeInstance->mNodeTransforms[node];
         Point3F defaultPos = mShapeInstance->getShape()->defaultTranslations[node];
         Quat16 defaultRot = mShapeInstance->getShape()->defaultRotations[node];

         QuatF qrot(zRot);
         qrot *= defaultRot.getQuatF();
         qrot.setMatrix( mat );      
         mat->setColumn(3, defaultPos);
      }
   }

   mShapeInstance->setDirty(TSShapeInstance::TransformDirty);
}

void TurretShape::_applyLimits(Point3F& rot)
{
   rot.x = mClampF(rot.x, mPitchUp, mPitchDown);
   if (mHeadingMax < mDegToRad(180.0f))
   {
      rot.z = mClampF(rot.z, -mHeadingMax, mHeadingMax);
   }
}

bool TurretShape::_outsideLimits(Point3F& rot)
{
   if (rot.x < mPitchUp || rot.x > mPitchDown)
      return true;

   if (mHeadingMax < mDegToRad(180.0f))
   {
      if (rot.z < -mHeadingMax || rot.z > mHeadingMax)
         return true;
   }

   return false;
}

//----------------------------------------------------------------------------

void TurretShape::mountObject( SceneObject *obj, S32 node, const MatrixF &xfm )
{
   Parent::mountObject(obj, node, xfm);

   if (isClientObject())
   {
      if (obj)
      {
         GameConnection* conn = obj->getControllingClient();
         if (conn)
         {
            // Allow the client to set up any action maps, HUD, etc.
            Con::executef("turretMountCallback", Con::getIntArg(getId()), Con::getIntArg(obj->getId()), Con::getIntArg(true));
         }
      }
   }
   else
   {
      mDataBlock->onMountObject_callback( this, obj, node );
   }
}

void TurretShape::unmountObject( SceneObject *obj )
{
   Parent::unmountObject(obj);

   if (isClientObject())
   {
      if (obj)
      {
         GameConnection* conn = obj->getControllingClient();
         if (conn)
         {
            // Allow the client to set up any action maps, HUD, etc.
            Con::executef("turretMountCallback", Con::getIntArg(getId()), Con::getIntArg(obj->getId()), Con::getIntArg(false));
         }
      }
   }
   else
   {
      mDataBlock->onUnmountObject_callback( this, obj );
   }
}

void TurretShape::onUnmount(ShapeBase*,S32)
{
   // Make sure the client get's the final server pos of this turret.
   setMaskBits(PositionMask);
}

//----------------------------------------------------------------------------

void TurretShape::getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot)
{
   *min = mDataBlock->cameraMinDist;
   *max = mDataBlock->cameraMaxDist;

   off->set(0,0,mDataBlock->cameraOffset);
   rot->identity();
}

void TurretShape::getCameraTransform(F32* pos,MatrixF* mat)
{
   // Returns camera to world space transform
   // Handles first person / third person camera position
   if (isServerObject() && mShapeInstance)
      mShapeInstance->animateNodeSubtrees(true);

   if (*pos == 0) {
      getRenderEyeTransform(mat);
      return;
   }

   // Get the shape's camera parameters.
   F32 min,max;
   MatrixF rot;
   Point3F offset;
   getCameraParameters(&min,&max,&offset,&rot);

   // Start with the current eye position
   MatrixF eye;
   getRenderEyeTransform(&eye);

   // Build a transform that points along the eye axis
   // but where the Z axis is always up.
   {
      MatrixF cam(1);
      VectorF x,y,z(0,0,1);
      eye.getColumn(1, &y);
      mCross(y, z, &x);
      x.normalize();
      mCross(x, y, &z);
      z.normalize();
      cam.setColumn(0,x);
      cam.setColumn(1,y);
      cam.setColumn(2,z);
      mat->mul(cam,rot);
   }

   // Camera is positioned straight back along the eye's -Y axis.
   // A ray is cast to make sure the camera doesn't go through
   // anything solid.
   VectorF vp,vec;
   vp.x = vp.z = 0;
   vp.y = -(max - min) * *pos;
   eye.mulV(vp,&vec);

   // Use the camera node as the starting position if it exists.
   Point3F osp,sp;
   if (mDataBlock->cameraNode != -1) 
   {
      mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);
      getRenderTransform().mulP(osp,&sp);
   }
   else
      eye.getColumn(3,&sp);

   // Make sure we don't hit ourself...
   disableCollision();
   if (isMounted())
      getObjectMount()->disableCollision();

   // Cast the ray into the container database to see if we're going
   // to hit anything.
   RayInfo collision;
   Point3F ep = sp + vec + offset;
   if (mContainer->castRay(sp, ep,
         ~(WaterObjectType | GameBaseObjectType | DefaultObjectType | sTriggerMask),
         &collision) == true) {

      // Shift the collision point back a little to try and
      // avoid clipping against the front camera plane.
      F32 t = collision.t - (-mDot(vec, collision.normal) / vec.len()) * 0.1;
      if (t > 0.0f)
         ep = sp + offset + (vec * t);
      else
         eye.getColumn(3,&ep);
   }
   mat->setColumn(3,ep);

   // Re-enable our collision.
   if (isMounted())
      getObjectMount()->enableCollision();
   enableCollision();

   // Apply Camera FX.
   mat->mul( gCamFXMgr.getTrans() );
}

//----------------------------------------------------------------------------

void TurretShape::writePacketData(GameConnection *connection, BitStream *stream)
{
   // Update client regardless of status flags.
   Parent::writePacketData(connection, stream);
   
   stream->writeSignedFloat(mRot.x / M_2PI_F, 7);
   stream->writeSignedFloat(mRot.z / M_2PI_F, 7);
}

void TurretShape::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);

   Point3F rot(0.0f, 0.0f, 0.0f);
   rot.x = stream->readSignedFloat(7) * M_2PI_F;
   rot.z = stream->readSignedFloat(7) * M_2PI_F;
   _setRotation(rot);

   mTurretDelta.rot = rot;
   mTurretDelta.rotVec.set(0.0f, 0.0f, 0.0f);
}

U32 TurretShape::packUpdate(NetConnection *connection, U32 mask, BitStream *stream)
{
   // Handle rotation ourselves (so it is not locked to the Z axis like for Items)
   U32 retMask = Parent::packUpdate( connection, mask & (~Item::RotationMask), stream );

   if (stream->writeFlag(mask & InitialUpdateMask)) {
      stream->writeFlag(mRespawn);
   }

   if ( stream->writeFlag( mask & Item::RotationMask ) )
   {
      QuatF rot( mObjToWorld );
      mathWrite( *stream, rot );
   }

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if(stream->writeFlag((NetConnection*)getControllingClient() == connection && !(mask & InitialUpdateMask)))
      return 0;

   if (stream->writeFlag(mask & TurretUpdateMask))
   {
      stream->writeSignedFloat(mRot.x / M_2PI_F, 7);
      stream->writeSignedFloat(mRot.z / M_2PI_F, 7);
      stream->write(allowManualRotation);
      stream->write(allowManualFire);
   }

   return retMask;
}

void TurretShape::unpackUpdate(NetConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection,stream);

   // InitialUpdateMask
   if (stream->readFlag()) {
      mRespawn = stream->readFlag();
   }

   // Item::RotationMask
   if ( stream->readFlag() )
   {
      QuatF rot;
      mathRead( *stream, &rot );

      Point3F pos = mObjToWorld.getPosition();
      rot.setMatrix( &mObjToWorld );
      mObjToWorld.setPosition( pos );
   }

   // controlled by the client?
   if(stream->readFlag())
      return;

   // TurretUpdateMask
   if (stream->readFlag())
   {
      Point3F rot(0.0f, 0.0f, 0.0f);
      rot.x = stream->readSignedFloat(7) * M_2PI_F;
      rot.z = stream->readSignedFloat(7) * M_2PI_F;
      _setRotation(rot);

      // New delta for client side interpolation
      mTurretDelta.rot = rot;
      mTurretDelta.rotVec = VectorF(0.0f, 0.0f, 0.0f);

      stream->read(&allowManualRotation);
      stream->read(&allowManualFire);
   }
}

//----------------------------------------------------------------------------

void TurretShape::getWeaponMountTransform( S32 index, const MatrixF &xfm, MatrixF *outMat )
{
   // Returns mount point to world space transform
   if ( index >= 0 && index < ShapeBase::MaxMountedImages) {
      S32 ni = mDataBlock->weaponMountNode[index];
      if (ni != -1) {
         MatrixF mountTransform = mShapeInstance->mNodeTransforms[ni];
         mountTransform.mul( xfm );
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve( scale );
         mountTransform.setPosition( position );

         // Also we would like the object to be scaled to the model.
         outMat->mul(mObjToWorld, mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   GrandParent::getMountTransform( index, xfm, outMat );      
}

void TurretShape::getRenderWeaponMountTransform( F32 delta, S32 mountPoint, const MatrixF &xfm, MatrixF *outMat )
{
   // Returns mount point to world space transform
   if ( mountPoint >= 0 && mountPoint < ShapeBase::MaxMountedImages) {
      S32 ni = mDataBlock->weaponMountNode[mountPoint];
      if (ni != -1) {
         MatrixF mountTransform = mShapeInstance->mNodeTransforms[ni];
         mountTransform.mul( xfm );
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve( scale );
         mountTransform.setPosition( position );

         // Also we would like the object to be scaled to the model.
         mountTransform.scale( scale );
         outMat->mul(getRenderTransform(), mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   GrandParent::getRenderMountTransform( delta, mountPoint, xfm, outMat );   
}

void TurretShape::getImageTransform(U32 imageSlot,MatrixF* mat)
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) {
      ShapeBaseImageData& data = *image.dataBlock;

      MatrixF nmat;
      if (data.useEyeOffset && isFirstPerson()) {
         getEyeTransform(&nmat);
         mat->mul(nmat,data.eyeOffset);
      }
      else {
         getWeaponMountTransform( imageSlot, MatrixF::Identity, &nmat );
         mat->mul(nmat,data.mountTransform[getImageShapeIndex(image)]);
      }
   }
   else
      *mat = mObjToWorld;
}

void TurretShape::getRenderImageTransform( U32 imageSlot, MatrixF* mat, bool noEyeOffset )
{
   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock) 
   {
      ShapeBaseImageData& data = *image.dataBlock;

      MatrixF nmat;
      if ( !noEyeOffset && data.useEyeOffset && isFirstPerson() ) 
      {
         getRenderEyeTransform(&nmat);
         mat->mul(nmat,data.eyeOffset);
      }
      else 
      {
         getRenderWeaponMountTransform( 0.0f, imageSlot, MatrixF::Identity, &nmat );
         mat->mul(nmat,data.mountTransform[getImageShapeIndex(image)]);
      }
   }
   else
      *mat = getRenderTransform();
}

void TurretShape::getImageTransform(U32 imageSlot,S32 node,MatrixF* mat)
{
   // Same as ShapeBase::getImageTransform() other than getRenderWeaponMountTransform() below

   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      if (node != -1)
      {
         ShapeBaseImageData& data = *image.dataBlock;
         U32 shapeIndex = getImageShapeIndex(image);

         MatrixF nmat = image.shapeInstance[shapeIndex]->mNodeTransforms[node];
         MatrixF mmat;

         if (data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1)
         {
            // We need to animate, even on the server, to make sure the nodes are in the correct location.
            image.shapeInstance[shapeIndex]->animate();

            MatrixF emat;
            getEyeBaseTransform(&emat, mDataBlock->mountedImagesBank);

            MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];
            mountTransform.affineInverse();

            mmat.mul(emat, mountTransform);
         }
         else if (data.useEyeOffset && isFirstPerson())
         {
            MatrixF emat;
            getEyeTransform(&emat);
            mmat.mul(emat,data.eyeOffset);
         }
         else
         {
            MatrixF emat;
            getWeaponMountTransform( imageSlot, MatrixF::Identity, &emat );
            mmat.mul(emat,data.mountTransform[shapeIndex]);
         }

         mat->mul(mmat, nmat);
      }
      else
         getImageTransform(imageSlot,mat);
   }
   else
      *mat = mObjToWorld;
}

void TurretShape::getRenderImageTransform(U32 imageSlot,S32 node,MatrixF* mat)
{
   // Same as ShapeBase::getRenderImageTransform() other than getRenderWeaponMountTransform() below

   // Image transform in world space
   MountedImage& image = mMountedImageList[imageSlot];
   if (image.dataBlock)
   {
      if (node != -1)
      {
         ShapeBaseImageData& data = *image.dataBlock;
         U32 shapeIndex = getImageShapeIndex(image);

         MatrixF nmat = image.shapeInstance[shapeIndex]->mNodeTransforms[node];
         MatrixF mmat;

         if ( data.useEyeNode && isFirstPerson() && data.eyeMountNode[shapeIndex] != -1 )
         {
            MatrixF emat;
            getRenderEyeBaseTransform(&emat, mDataBlock->mountedImagesBank);

            MatrixF mountTransform = image.shapeInstance[shapeIndex]->mNodeTransforms[data.eyeMountNode[shapeIndex]];
            mountTransform.affineInverse();

            mmat.mul(emat, mountTransform);
         }
         else if ( data.useEyeOffset && isFirstPerson() ) 
         {
            MatrixF emat;
            getRenderEyeTransform(&emat);
            mmat.mul(emat,data.eyeOffset);
         }
         else 
         {
            MatrixF emat;
            getRenderWeaponMountTransform( 0.0f, imageSlot, MatrixF::Identity, &emat );
            mmat.mul(emat,data.mountTransform[shapeIndex]);
         }

         mat->mul(mmat, nmat);
      }
      else
         getRenderImageTransform(imageSlot,mat);
   }
   else
      *mat = getRenderTransform();
}

//----------------------------------------------------------------------------

void TurretShape::prepRenderImage( SceneRenderState *state )
{
   // Skip the Item class rendering
   _prepRenderImage( state, true, true );
}

void TurretShape::prepBatchRender( SceneRenderState *state, S32 mountedImageIndex )
{
   Parent::prepBatchRender( state, mountedImageIndex );

   if ( !gShowBoundingBox )
      return;

   //if ( mountedImageIndex != -1 )
   //{
   //   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   //   ri->renderDelegate.bind( this, &Vehicle::_renderMuzzleVector );
   //   ri->objectIndex = mountedImageIndex;
   //   ri->type = RenderPassManager::RIT_Editor;
   //   state->getRenderPass()->addInst( ri );
   //   return;
   //}

   //ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   //ri->renderDelegate.bind( this, &Vehicle::_renderMassAndContacts );
   //ri->type = RenderPassManager::RIT_Editor;
   //state->getRenderPass()->addInst( ri );
}

//----------------------------------------------------------------------------

DefineEngineMethod( TurretShape, getAllowManualRotation, bool, (),,
   "@brief Get if the turret is allowed to rotate through moves.\n\n"
   "@return True if the turret is allowed to rotate through moves.\n" )
{
   return object->getAllowManualRotation();
}

DefineEngineMethod( TurretShape, setAllowManualRotation, void, (bool allow),,
   "@brief Set if the turret is allowed to rotate through moves.\n\n"
   "@param allow If true then the turret may be rotated through moves.\n")
{
   return object->setAllowManualRotation(allow);
}

DefineEngineMethod( TurretShape, getAllowManualFire, bool, (),,
   "@brief Get if the turret is allowed to fire through moves.\n\n"
   "@return True if the turret is allowed to fire through moves.\n" )
{
   return object->getAllowManualFire();
}

DefineEngineMethod( TurretShape, setAllowManualFire, void, (bool allow),,
   "@brief Set if the turret is allowed to fire through moves.\n\n"
   "@param allow If true then the turret may be fired through moves.\n")
{
   return object->setAllowManualFire(allow);
}

DefineEngineMethod( TurretShape, getState, const char*, (),,
   "@brief Get the name of the turret's current state.\n\n"

   "The state is one of the following:\n\n<ul>"
   "<li>Dead - The TurretShape is destroyed.</li>"
   "<li>Mounted - The TurretShape is mounted to an object such as a vehicle.</li>"
   "<li>Ready - The TurretShape is free to move.  The usual state.</li></ul>\n"

   "@return The current state; one of: \"Dead\", \"Mounted\", \"Ready\"\n" )
{
   return object->getStateName();
}

DefineEngineMethod( TurretShape, getTurretEulerRotation, Point3F, (),,
   "@brief Get Euler rotation of this turret's heading and pitch nodes.\n\n"
   "@return the orientation of the turret's heading and pitch nodes in the "
   "form of rotations around the X, Y and Z axes in degrees.\n" )
{
   Point3F euler = object->getTurretRotation();
   
   // Convert to degrees.
   euler.x = mRadToDeg( euler.x );
   euler.y = mRadToDeg( euler.y );
   euler.z = mRadToDeg( euler.z );
   
   return euler;
}

DefineEngineMethod( TurretShape, setTurretEulerRotation, void, ( Point3F rot ),,
   "@brief Set Euler rotation of this turret's heading and pitch nodes in degrees.\n\n"
   "@param rot The rotation in degrees.  The pitch is the X component and the "
   "heading is the Z component.  The Y component is ignored.\n")
{
   object->setTurretRotation( rot );
}

DefineEngineMethod( TurretShape, doRespawn, bool, (),,
   "@brief Does the turret respawn after it has been destroyed.\n\n"
   "@returns True if the turret respawns.\n")
{
   return object->doRespawn();
}
