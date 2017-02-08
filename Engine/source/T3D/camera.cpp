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

#include "platform/platform.h"
#include "T3D/camera.h"

#include "math/mMath.h"
#include "core/stream/bitStream.h"
#include "T3D/fx/cameraFXMgr.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "gui/worldEditor/editor.h"
#include "console/engineAPI.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mathUtils.h"
#include "math/mTransform.h"

#ifdef TORQUE_EXTENDED_MOVE
   #include "T3D/gameBase/extended/extendedMove.h"
#endif

S32 Camera::smExtendedMovePosRotIndex = 0;  // The ExtendedMove position/rotation index used for camera movements

#define MaxPitch 1.5706f
#define CameraRadius 0.05f;


ImplementEnumType( CameraMotionMode,
   "Movement behavior type for Camera.\n\n"
   "@ingroup BaseCamera" )
   { Camera::StationaryMode,   "Stationary",  "Camera does not rotate or move." },
   { Camera::FreeRotateMode,   "FreeRotate",  "Camera may rotate but does not move." },
   { Camera::FlyMode,          "Fly",         "Camera may rotate and move freely." },
   { Camera::OrbitObjectMode,  "OrbitObject", "Camera orbits about a given object.  Damage flash and white out is determined by the object being orbited.  See Camera::setOrbitMode() to set the orbit object and other parameters." },
   { Camera::OrbitPointMode,   "OrbitPoint",  "Camera orbits about a given point.  See Camera::setOrbitMode() to set the orbit point and other parameters." },
   { Camera::TrackObjectMode,  "TrackObject", "Camera always faces a given object.  See Camera::setTrackObject() to set the object to track and a distance to remain from the object." },
   { Camera::OverheadMode,     "Overhead",    "Camera moves in the XY plane." },
   { Camera::EditOrbitMode,    "EditOrbit",   "Used by the World Editor to orbit about a point.  When first activated, the camera is rotated to face the orbit point rather than move to it." }
EndImplementEnumType;


//=============================================================================
//    CameraData.
//=============================================================================
// MARK: ---- CameraData ----

IMPLEMENT_CO_DATABLOCK_V1( CameraData );
ConsoleDocClass( CameraData, 
   "@brief A datablock that describes a camera.\n\n"

   "@tsexample\n"
   "datablock CameraData(Observer)\n"
   "{\n"
   "   mode = \"Observer\";\n"
   "};\n"
   "@endtsexample\n"

   "@see Camera\n\n"
   "@ref Datablock_Networking\n"
   "@ingroup BaseCamera\n"
   "@ingroup Datablocks\n"
);

//-----------------------------------------------------------------------------

void CameraData::initPersistFields()
{
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void CameraData::packData(BitStream* stream)
{
   Parent::packData(stream);
}

//-----------------------------------------------------------------------------

void CameraData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
}

//=============================================================================
//    Camera.
//=============================================================================
// MARK: ---- Camera ----

IMPLEMENT_CO_NETOBJECT_V1( Camera );
ConsoleDocClass( Camera, 
   "@brief Represents a position, direction and field of view to render a scene from.\n\n"

   "A camera is typically manipulated by a GameConnection.  When set as the connection's "
   "control object, the camera handles all movement actions ($mvForwardAction, $mvPitch, etc.) "
   "just like a Player.\n"

   "@tsexample\n"
   "// Set an already created camera as the GameConnection's control object\n"
   "%connection.setControlObject(%camera);\n"
   "@endtsexample\n\n"

   "<h3>Methods of Operation</h3>\n\n"

   "The camera has two general methods of operation.  The first is the standard mode where "
   "the camera starts and stops its motion and rotation instantly.  This is the default operation "
   "of the camera and is used by most games.  It may be specifically set with Camera::setFlyMode() "
   "for 6 DoF motion.  It is also typically the method used with Camera::setOrbitMode() or one of "
   "its helper methods to orbit about a specific object (such as the Player's dead body) or a "
   "specific point.\n\n"

   "The second method goes under the name of Newton as it follows Newton's 2nd law of "
   "motion: F=ma.  This provides the camera with an ease-in and ease-out feel for both movement "
   "and rotation.  To activate this method for movement, either use Camera::setNewtonFlyMode() or set "
   "the Camera::newtonMode field to true.  To activate this method for rotation, set the Camera::newtonRotation "
   "to true.  This method of operation is not typically used in games, and was developed to allow "
   "for a smooth fly through of a game level while recording a demo video.  But with the right force "
   "and drag settings, it may give a more organic feel to the camera to games that use an overhead view, "
   "such as a RTS.\n\n"

   "There is a third, minor method of operation but it is not generally used for games.  This is when the "
   "camera is used with Torque's World Editor in Edit Orbit Mode.  When set, this allows the camera "
   "to rotate about a specific point in the world, and move towards and away from this point.  See "
   "Camera::setEditOrbitMode() and Camera::setEditOrbitPoint().  While in this mode, Camera::autoFitRadius() "
   "may also be used.\n\n"

   "@tsexample\n"
   "// Create a camera in the level and set its position to a given spawn point.\n"
   "// Note: The camera starts in the standard fly mode.\n"
   "%cam = new Camera() {\n"
   "   datablock = \"Observer\";\n"
   "};\n"
   "MissionCleanup.add( %cam );\n"
   "%cam.setTransform( %spawnPoint.getTransform() );\n"
   "@endtsexample\n\n"

   "@tsexample\n"
   "// Create a camera at the given spawn point for the specified\n"
   "// GameConnection i.e. the client.  Uses the standard\n"
   "// Sim::spawnObject() function to create the camera using the\n"
   "// defined default settings.\n"
   "// Note: The camera starts in the standard fly mode.\n"
   "function GameConnection::spawnCamera(%this, %spawnPoint)\n"
   "{\n"
   "   // Set the control object to the default camera\n"
   "   if (!isObject(%this.camera))\n"
   "   {\n"
   "      if (isDefined(\"$Game::DefaultCameraClass\"))\n"
   "         %this.camera = spawnObject($Game::DefaultCameraClass, $Game::DefaultCameraDataBlock);\n"
   "   }\n"
   "\n"
   "   // If we have a camera then set up some properties\n"
   "   if (isObject(%this.camera))\n"
   "   {\n"
   "      // Make sure we're cleaned up when the mission ends\n"
   "      MissionCleanup.add( %this.camera );\n"
   "\n"
   "      // Make sure the camera is always in scope for the connection\n"
   "      %this.camera.scopeToClient(%this);\n"
   "\n"
   "      // Send all user input from the connection to the camera\n"
   "      %this.setControlObject(%this.camera);\n"
   "\n"
   "      if (isDefined(\"%spawnPoint\"))\n"
   "      {\n"
   "         // Attempt to treat %spawnPoint as an object, such as a\n"
   "         // SpawnSphere class.\n"
   "         if (getWordCount(%spawnPoint) == 1 && isObject(%spawnPoint))\n"
   "         {\n"
   "            %this.camera.setTransform(%spawnPoint.getTransform());\n"
   "         }\n"
   "         else\n"
   "         {\n"
   "            // Treat %spawnPoint as an AngleAxis transform\n"
   "            %this.camera.setTransform(%spawnPoint);\n"
   "         }\n"
   "      }\n"
   "   }\n"
   "}\n"
   "@endtsexample\n\n"

   "<h3>Motion Modes</h3>\n\n"

   "Beyond the different operation methods, the Camera may be set to one of a number "
   "of motion modes.  These motion modes determine how the camera will respond to input "
   "and may be used to constrain how the Camera moves.  The CameraMotionMode enumeration "
   "defines the possible set of modes and the Camera's current may be obtained by using "
   "getMode().\n\n"

   "Some of the motion modes may be set using specific script methods.  These often provide "
   "additional parameters to set up the mode in one go.  Otherwise, it is always possible to "
   "set a Camera's motion mode using the controlMode property.  Just pass in the name of the "
   "mode enum.  The following table lists the motion modes, how to set them up, and what they offer:\n\n"

   "<table border='1' cellpadding='1'>"
   "<tr><th>Mode</th><th>Set From Script</th><th>Input Move</th><th>Input Rotate</th><th>Can Use Newton Mode?</th></tr>"
   "<tr><td>Stationary</td><td>controlMode property</td><td>No</td><td>No</td><td>No</td></tr>"
   "<tr><td>FreeRotate</td><td>controlMode property</td><td>No</td><td>Yes</td><td>Rotate Only</td></tr>"
   "<tr><td>Fly</td><td>setFlyMode()</td><td>Yes</td><td>Yes</td><td>Yes</td></tr>"
   "<tr><td>OrbitObject</td><td>setOrbitMode()</td><td>Orbits object</td><td>Points to object</td><td>Move only</td></tr>"
   "<tr><td>OrbitPoint</td><td>setOrbitPoint()</td><td>Orbits point</td><td>Points to location</td><td>Move only</td></tr>"
   "<tr><td>TrackObject</td><td>setTrackObject()</td><td>No</td><td>Points to object</td><td>Yes</td></tr>"
   "<tr><td>Overhead</td><td>controlMode property</td><td>Yes</td><td>No</td><td>Yes</td></tr>"
   "<tr><td>EditOrbit (object selected)</td><td>setEditOrbitMode()</td><td>Orbits object</td><td>Points to object</td><td>Move only</td></tr>"
   "<tr><td>EditOrbit (no object)</td><td>setEditOrbitMode()</td><td>Yes</td><td>Yes</td><td>Yes</td></tr>"
   "</table>\n\n"

   "<h3>%Trigger Input</h3>\n\n"

   "Passing a move trigger ($mvTriggerCount0, $mvTriggerCount1, etc.) on to a Camera performs "
   "different actions depending on which mode the camera is in.  While in Fly, Overhead or "
   "EditOrbit mode, either trigger0 or trigger1 will cause a camera to move twice its normal "
   "movement speed.  You can see this in action within the World Editor, where holding down the "
   "left mouse button while in mouse look mode (right mouse button is also down) causes the Camera "
   "to move faster.\n\n"

   "Passing along trigger2 will put the camera into strafe mode.  While in this mode a Fly, "
   "FreeRotate or Overhead Camera will not rotate from the move input.  Instead the yaw motion "
   "will be applied to the Camera's x motion, and the pitch motion will be applied to the Camera's "
   "z motion.  You can see this in action within the World Editor where holding down the middle mouse "
   "button allows the user to move the camera up, down and side-to-side.\n\n"

   "While the camera is operating in Newton Mode, trigger0 and trigger1 behave slightly differently.  "
   "Here trigger0 activates a multiplier to the applied acceleration force as defined by speedMultiplier.  "
   "This has the affect of making the camera move up to speed faster.  trigger1 has the opposite affect "
   "by acting as a brake.  When trigger1 is active a multiplier is added to the Camera's drag as "
   "defined by brakeMultiplier.\n\n"

   "@see CameraData\n"
   "@see CameraMotionMode\n"
   "@see Camera::movementSpeed\n\n"
   "@ingroup BaseCamera\n"
);

F32 Camera::smMovementSpeed = 40.0f;

//----------------------------------------------------------------------------

Camera::Camera()
{
   mNetFlags.clear(Ghostable);
   mTypeMask |= CameraObjectType;
   mDataBlock = 0;
   mDelta.pos = Point3F(0.0f, 0.0f, 100.0f);
   mDelta.rot = Point3F(0.0f, 0.0f, 0.0f);
   mDelta.posVec = mDelta.rotVec = VectorF(0.0f, 0.0f, 0.0f);
   mObjToWorld.setColumn(3, mDelta.pos);
   mRot = mDelta.rot;

   mOffset.set(0.0f, 0.0f, 0.0f);

   mMinOrbitDist = 0.0f;
   mMaxOrbitDist = 0.0f;
   mCurOrbitDist = 0.0f;
   mOrbitObject = NULL;
   mPosition.set(0.0f, 0.0f, 0.0f);
   mObservingClientObject = false;
   mMode = FlyMode;

   mLastAbsoluteYaw = 0.0f;
   mLastAbsolutePitch = 0.0f;
   mLastAbsoluteRoll = 0.0f;

   // For NewtonFlyMode
   mNewtonRotation = false;
   mAngularVelocity.set(0.0f, 0.0f, 0.0f);
   mAngularForce = 100.0f;
   mAngularDrag = 2.0f;
   mVelocity.set(0.0f, 0.0f, 0.0f);
   mNewtonMode = false;
   mMass = 10.0f;
   mDrag = 2.0;
   mFlyForce = 500.0f;
   mSpeedMultiplier = 2.0f;
   mBrakeMultiplier = 2.0f;

   // For EditOrbitMode
   mValidEditOrbitPoint = false;
   mEditOrbitPoint.set(0.0f, 0.0f, 0.0f);
   mCurrentEditOrbitDist = 2.0;

   mLocked = false;
}

//----------------------------------------------------------------------------

Camera::~Camera()
{
}

//----------------------------------------------------------------------------

bool Camera::onAdd()
{
   if(!Parent::onAdd() || !mDataBlock)
      return false;

   mObjBox.maxExtents = mObjScale;
   mObjBox.minExtents = mObjScale;
   mObjBox.minExtents.neg();
   resetWorldBox();

   addToScene();

   scriptOnAdd();

   return true;
}

//----------------------------------------------------------------------------

void Camera::onRemove()
{
   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}

//----------------------------------------------------------------------------

bool Camera::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<CameraData*>(dptr);
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   scriptOnNewDataBlock();

   return true;
}

//----------------------------------------------------------------------------

void Camera::onEditorEnable()
{
   mNetFlags.set(Ghostable);
}

//----------------------------------------------------------------------------

void Camera::onEditorDisable()
{
   mNetFlags.clear(Ghostable);
}

//----------------------------------------------------------------------------

// check if the object needs to be observed through its own camera...
void Camera::getCameraTransform(F32* pos, MatrixF* mat)
{
   // The camera doesn't support a third person mode,
   // so we want to override the default ShapeBase behavior.
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      obj->getCameraTransform(pos, mat);
   else
      getRenderEyeTransform(mat);

   // Apply Camera FX.
   mat->mul( gCamFXMgr.getTrans() );
}

void Camera::getEyeCameraTransform(IDisplayDevice *displayDevice, U32 eyeId, MatrixF *outMat)
{
   // The camera doesn't support a third person mode,
   // so we want to override the default ShapeBase behavior.
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      obj->getEyeCameraTransform(displayDevice, eyeId, outMat);
   else
   {
      Parent::getEyeCameraTransform(displayDevice, eyeId, outMat);
   }
}

//----------------------------------------------------------------------------

F32 Camera::getCameraFov()
{
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      return(obj->getCameraFov());
   else
      return(Parent::getCameraFov());
}

//----------------------------------------------------------------------------

F32 Camera::getDefaultCameraFov()
{
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      return(obj->getDefaultCameraFov());
   else
      return(Parent::getDefaultCameraFov());
}

//----------------------------------------------------------------------------

bool Camera::isValidCameraFov(F32 fov)
{
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      return(obj->isValidCameraFov(fov));
   else
      return(Parent::isValidCameraFov(fov));
}

//----------------------------------------------------------------------------

void Camera::setCameraFov(F32 fov)
{
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      obj->setCameraFov(fov);
   else
      Parent::setCameraFov(fov);
}

//----------------------------------------------------------------------------

void clampPitchAngle(F32 &pitch)
{
   // Clamp pitch to +/-MaxPitch, but allow pitch=PI as it is used by some editor
   // views (bottom, front, right)
   if ((pitch > MaxPitch) && !mIsEqual(pitch, M_PI_F, 0.001f))
      pitch = MaxPitch;
   else if (pitch < -MaxPitch)
      pitch = -MaxPitch;
}

//----------------------------------------------------------------------------

void Camera::processTick(const Move* move)
{
   Parent::processTick(move);

   if ( isMounted() )
   {
      // Update SceneContainer.
      updateContainer();
      return;
   }

   Point3F vec,pos;
   if (move) 
   {
      bool strafeMode = move->trigger[2];

      // If using editor then force camera into fly mode, unless using EditOrbitMode
      if(gEditingMission && mMode != FlyMode && mMode != EditOrbitMode)
         setFlyMode();

      // Massage the mode if we're in EditOrbitMode
      CameraMotionMode virtualMode = mMode;
      if(mMode == EditOrbitMode)
      {
         if(!mValidEditOrbitPoint)
         {
            virtualMode = FlyMode;
         }
         else
         {
            // Reset any Newton camera velocities for when we switch
            // out of EditOrbitMode.
            mNewtonRotation = false;
            mVelocity.set(0.0f, 0.0f, 0.0f);
            mAngularVelocity.set(0.0f, 0.0f, 0.0f);
         }
      }

      // Update orientation
      mDelta.rotVec = mRot;

      VectorF rotVec(0, 0, 0);

      bool doStandardMove = true;

#ifdef TORQUE_EXTENDED_MOVE
      GameConnection* con = getControllingClient();

      // Work with an absolute rotation from the ExtendedMove class?
      if(con && con->getControlSchemeAbsoluteRotation())
      {
         doStandardMove = false;
         const ExtendedMove* emove = dynamic_cast<const ExtendedMove*>(move);
         U32 emoveIndex = smExtendedMovePosRotIndex;
         if(emoveIndex >= ExtendedMove::MaxPositionsRotations)
            emoveIndex = 0;

         if(emove->EulerBasedRotation[emoveIndex])
         {
            if(virtualMode != StationaryMode &&
               virtualMode != TrackObjectMode &&
               (!mLocked || virtualMode != OrbitObjectMode && virtualMode != OrbitPointMode))
            {
               // Pitch
               mRot.x += (emove->rotX[emoveIndex] - mLastAbsolutePitch);

               // Do we also include the relative pitch value?
               if(con->getControlSchemeAddPitchToAbsRot() && !strafeMode)
               {
                  F32 x = move->pitch;
                  if (x > M_PI_F)
                     x -= M_2PI_F;

                  mRot.x += x;
               }

               // Constrain the range of mRot.x
               while (mRot.x < -M_PI_F) 
                  mRot.x += M_2PI_F;
               while (mRot.x > M_PI_F) 
                  mRot.x -= M_2PI_F;

               // Yaw
               mRot.z += (emove->rotZ[emoveIndex] - mLastAbsoluteYaw);

               // Do we also include the relative yaw value?
               if(con->getControlSchemeAddYawToAbsRot() && !strafeMode)
               {
                  F32 z = move->yaw;
                  if (z > M_PI_F)
                     z -= M_2PI_F;

                  mRot.z += z;
               }

               // Constrain the range of mRot.z
               while (mRot.z < -M_PI_F) 
                  mRot.z += M_2PI_F;
               while (mRot.z > M_PI_F) 
                  mRot.z -= M_2PI_F;

               mLastAbsoluteYaw = emove->rotZ[emoveIndex];
               mLastAbsolutePitch = emove->rotX[emoveIndex];
               mLastAbsoluteRoll = emove->rotY[emoveIndex];

               // Bank
               mRot.y = emove->rotY[emoveIndex];

               // Constrain the range of mRot.y
               while (mRot.y > M_PI_F) 
                  mRot.y -= M_2PI_F;
            }
         }
      }
#endif

      if(doStandardMove)
      {
         // process input/determine rotation vector
         if(virtualMode != StationaryMode &&
            virtualMode != TrackObjectMode &&
            (!mLocked || ((virtualMode != OrbitObjectMode) && (virtualMode != OrbitPointMode))))
         {
            if(!strafeMode)
            {
               rotVec.x = move->pitch;
               rotVec.z = move->yaw;
            }
         }
         else if(virtualMode == TrackObjectMode && bool(mOrbitObject))
         {
            // orient the camera to face the object
            Point3F objPos;
            // If this is a shapebase, use its render eye transform
            // to avoid jittering.
            ShapeBase *shape = dynamic_cast<ShapeBase*>((GameBase*)mOrbitObject);
            if( shape != NULL )
            {
               MatrixF ret;
               shape->getRenderEyeTransform( &ret );
               objPos = ret.getPosition();
            }
            else
            {
               mOrbitObject->getWorldBox().getCenter(&objPos);
            }
            mObjToWorld.getColumn(3,&pos);
            vec = objPos - pos;
            vec.normalizeSafe();
            F32 pitch, yaw;
            MathUtils::getAnglesFromVector(vec, yaw, pitch);
            rotVec.x = -pitch - mRot.x;
            rotVec.z = yaw - mRot.z;
            if(rotVec.z > M_PI_F)
               rotVec.z -= M_2PI_F;
            else if(rotVec.z < -M_PI_F)
               rotVec.z += M_2PI_F;
         }

         // apply rotation vector according to physics rules
         if(mNewtonRotation)
         {
            const F32 force = mAngularForce;
            const F32 drag = mAngularDrag;

            VectorF acc(0.0f, 0.0f, 0.0f);

            rotVec.x *= 2.0f;   // Assume that our -2PI to 2PI range was clamped to -PI to PI in script
            rotVec.z *= 2.0f;   // Assume that our -2PI to 2PI range was clamped to -PI to PI in script

            F32 rotVecL = rotVec.len();
            if(rotVecL > 0)
            {
               acc = (rotVec * force / mMass) * TickSec;
            }

            // Accelerate
            mAngularVelocity += acc;

            // Drag
            mAngularVelocity -= mAngularVelocity * drag * TickSec;

            // Rotate
            mRot += mAngularVelocity * TickSec;
            clampPitchAngle(mRot.x);
         }
         else
         {
            mRot.x += rotVec.x;
            mRot.z += rotVec.z;
            clampPitchAngle(mRot.x);
         }
      }

      // Update position
      VectorF posVec(0, 0, 0);
      bool mustValidateEyePoint = false;
      bool serverInterpolate = false;

      // process input/determine translation vector
      if(virtualMode == OrbitObjectMode || virtualMode == OrbitPointMode)
      {
         pos = mDelta.pos;
         if(virtualMode == OrbitObjectMode && bool(mOrbitObject)) 
         {
            // If this is a shapebase, use its render eye transform
            // to avoid jittering.
            GameBase *castObj = mOrbitObject;
            ShapeBase* shape = dynamic_cast<ShapeBase*>(castObj);
            if( shape != NULL ) 
            {
               MatrixF ret;
               shape->getRenderEyeTransform( &ret );
               mPosition = ret.getPosition();
            }
            else 
            {
               // Hopefully this is a static object that doesn't move,
               // because the worldbox doesn't get updated between ticks.
               mOrbitObject->getWorldBox().getCenter(&mPosition);
            }
         }

         posVec = (mPosition + mOffset) - pos;
         mustValidateEyePoint = true;
         serverInterpolate = mNewtonMode;
      }
      else if(virtualMode == EditOrbitMode && mValidEditOrbitPoint)
      {
         bool faster = move->trigger[0] || move->trigger[1];
         F32 scale = smMovementSpeed * (faster + 1);
         mCurrentEditOrbitDist -= move->y * TickSec * scale;
         mCurrentEditOrbitDist -= move->roll * TickSec * scale;   // roll will be -Pi to Pi and we'll attempt to scale it here to be in line with the move->y calculation above
         if(mCurrentEditOrbitDist < 0.0f)
            mCurrentEditOrbitDist = 0.0f;

         mPosition = mEditOrbitPoint;
         _setPosition(mPosition, mRot);
         _calcEditOrbitPoint(&mObjToWorld, mRot);
         pos = mPosition;
      }
      else if(virtualMode == FlyMode)
      {
         bool faster = move->trigger[0] || move->trigger[1];
         F32 scale = smMovementSpeed * (faster + 1);

         mObjToWorld.getColumn(3,&pos);
         mObjToWorld.getColumn(0,&vec);
         posVec = vec * move->x * TickSec * scale + vec * (strafeMode ? move->yaw * 2.0f * TickSec * scale : 0.0f);
         mObjToWorld.getColumn(1,&vec);
         posVec += vec * move->y * TickSec * scale + vec * move->roll * TickSec * scale;
         mObjToWorld.getColumn(2,&vec);
         posVec += vec * move->z * TickSec * scale - vec * (strafeMode ? move->pitch * 2.0f * TickSec * scale : 0.0f);
      }
      else if(virtualMode == OverheadMode)
      {
         bool faster = move->trigger[0] || move->trigger[1];
         F32 scale = smMovementSpeed * (faster + 1);

         mObjToWorld.getColumn(3,&pos);

         mObjToWorld.getColumn(0,&vec);
         vec.z = 0;
         vec.normalizeSafe();
         vec = vec * move->x * TickSec * scale + (strafeMode ? vec * move->yaw * 2.0f * TickSec * scale : Point3F(0, 0, 0));
         posVec = vec;

         mObjToWorld.getColumn(1,&vec);
         vec.z = 0;
         if (vec.isZero())
         {
            mObjToWorld.getColumn(2,&vec);
            vec.z = 0;
         }
         vec.normalizeSafe();
         vec = vec * move->y * TickSec * scale - (strafeMode ? vec * move->pitch * 2.0f * TickSec * scale : Point3F(0, 0, 0));
         posVec += vec;
         posVec.z += move->z * TickSec * scale + move->roll * TickSec * scale;
      }
      else // ignore input
      {
         mObjToWorld.getColumn(3,&pos);
      }

      // apply translation vector according to physics rules
      mDelta.posVec = pos;
      if(mNewtonMode)
      {
         bool faster = move->trigger[0];
         bool brake = move->trigger[1];

         const F32 movementSpeedMultiplier = smMovementSpeed / 40.0f; // Using the starting value as the base
         const F32 force = faster ? mFlyForce * movementSpeedMultiplier * mSpeedMultiplier : mFlyForce * movementSpeedMultiplier;
         const F32 drag = brake ? mDrag * mBrakeMultiplier : mDrag;

         VectorF acc(0.0f, 0.0f, 0.0f);

         F32 posVecL = posVec.len();
         if(posVecL > 0)
         {
            acc = (posVec * force / mMass) * TickSec;
         }

         // Accelerate
         mVelocity += acc;

         // Drag
         mVelocity -= mVelocity * drag * TickSec;

         // Move
         pos += mVelocity * TickSec;
      }
      else
      {
         pos += posVec;
      }

      _setPosition(pos,mRot);

      // If on the client, calc delta for backstepping
      if (serverInterpolate || isClientObject())
      {
         mDelta.pos = pos;
         mDelta.rot = mRot;
         mDelta.posVec = mDelta.posVec - mDelta.pos;
         mDelta.rotVec = mDelta.rotVec - mDelta.rot;
         for(U32 i=0; i<3; ++i)
         {
            if (mDelta.rotVec[i] > M_PI_F)
               mDelta.rotVec[i] -= M_2PI_F;
            else if (mDelta.rotVec[i] < -M_PI_F)
               mDelta.rotVec[i] += M_2PI_F;
         }
      }

      if(mustValidateEyePoint)
         _validateEyePoint(1.0f, &mObjToWorld);

      setMaskBits(MoveMask);
   }

   // Need to calculate the orbit position for the editor so the camera position
   // doesn't change when switching between orbit and other camera modes
   if (isServerObject() && (mMode == EditOrbitMode && mValidEditOrbitPoint))
      _calcEditOrbitPoint(&mObjToWorld, mRot);

   if(getControllingClient() && mContainer)
      updateContainer();
}

//----------------------------------------------------------------------------

void Camera::onDeleteNotify( SimObject* obj )
{   
   if( obj == mOrbitObject )
   {
      mOrbitObject = NULL;

      if( mMode == OrbitObjectMode )
         mMode = OrbitPointMode;
   }

   Parent::onDeleteNotify( obj );
}

//----------------------------------------------------------------------------

void Camera::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);

   if ( isMounted() )
      return;

   Point3F rot = mDelta.rot + mDelta.rotVec * dt;

   if((mMode == OrbitObjectMode || mMode == OrbitPointMode) && !mNewtonMode)
   {
      if(mMode == OrbitObjectMode && bool(mOrbitObject))
      {
         // If this is a shapebase, use its render eye transform
         // to avoid jittering.
         GameBase *castObj = mOrbitObject;
         ShapeBase* shape = dynamic_cast<ShapeBase*>(castObj);
         if( shape != NULL ) 
         {
            MatrixF ret;
            shape->getRenderEyeTransform( &ret );
            mPosition = ret.getPosition();
         } 
         else 
         {
            // Hopefully this is a static object that doesn't move,
            // because the worldbox doesn't get updated between ticks.
            mOrbitObject->getWorldBox().getCenter(&mPosition);
         }
      }
      _setRenderPosition( mPosition + mOffset, rot );
      _validateEyePoint( 1.0f, &mRenderObjToWorld );
   }
   else if(mMode == EditOrbitMode && mValidEditOrbitPoint)
   {
      mPosition = mEditOrbitPoint;
      _setRenderPosition(mPosition, rot);
      _calcEditOrbitPoint(&mRenderObjToWorld, rot);
   }
   else if(mMode == TrackObjectMode && bool(mOrbitObject) && !mNewtonRotation)
   {
      // orient the camera to face the object
      Point3F objPos;
      // If this is a shapebase, use its render eye transform
      // to avoid jittering.
      ShapeBase *shape = dynamic_cast<ShapeBase*>((GameBase*)mOrbitObject);
      if( shape != NULL )
      {
         MatrixF ret;
         shape->getRenderEyeTransform( &ret );
         objPos = ret.getPosition();
      }
      else
      {
         mOrbitObject->getWorldBox().getCenter(&objPos);
      }
      Point3F pos = mDelta.pos + mDelta.posVec * dt;
      Point3F vec = objPos - pos;
      vec.normalizeSafe();
      F32 pitch, yaw;
      MathUtils::getAnglesFromVector(vec, yaw, pitch);
      rot.x = -pitch;
      rot.z = yaw;
      _setRenderPosition(pos, rot);
   }
   else 
   {
      Point3F pos = mDelta.pos + mDelta.posVec * dt;
      _setRenderPosition(pos,rot);
      if(mMode == OrbitObjectMode || mMode == OrbitPointMode)
         _validateEyePoint(1.0f, &mRenderObjToWorld);
   }
}

//----------------------------------------------------------------------------

void Camera::_setPosition(const Point3F& pos, const Point3F& rot)
{
   MatrixF xRot, zRot;
   xRot.set(EulerF(rot.x, 0.0f, 0.0f));
   zRot.set(EulerF(0.0f, 0.0f, rot.z));
   
   MatrixF temp;

   if(mDataBlock && mDataBlock->cameraCanBank)
   {
      // Take rot.y into account to bank the camera
      MatrixF imat;
      imat.mul(zRot, xRot);
      MatrixF ymat;
      ymat.set(EulerF(0.0f, rot.y, 0.0f));
      temp.mul(imat, ymat);
   }
   else
   {
      temp.mul(zRot, xRot);
   }

   temp.setColumn(3, pos);
   Parent::setTransform(temp);
   mRot = rot;
}

//----------------------------------------------------------------------------

void Camera::setRotation(const Point3F& rot)
{
   MatrixF xRot, zRot;
   xRot.set(EulerF(rot.x, 0.0f, 0.0f));
   zRot.set(EulerF(0.0f, 0.0f, rot.z));

   MatrixF temp;

   if(mDataBlock && mDataBlock->cameraCanBank)
   {
      // Take rot.y into account to bank the camera
      MatrixF imat;
      imat.mul(zRot, xRot);
      MatrixF ymat;
      ymat.set(EulerF(0.0f, rot.y, 0.0f));
      temp.mul(imat, ymat);
   }
   else
   {
      temp.mul(zRot, xRot);
   }

   temp.setColumn(3, getPosition());
   Parent::setTransform(temp);
   mRot = rot;
}

//----------------------------------------------------------------------------

void Camera::_setRenderPosition(const Point3F& pos,const Point3F& rot)
{
   MatrixF xRot, zRot;
   xRot.set(EulerF(rot.x, 0, 0));
   zRot.set(EulerF(0, 0, rot.z));

   MatrixF temp;

   // mDataBlock may not be defined yet as this method is called during
   // SceneObject::onAdd().
   if(mDataBlock && mDataBlock->cameraCanBank)
   {
      // Take rot.y into account to bank the camera
      MatrixF imat;
      imat.mul(zRot, xRot);
      MatrixF ymat;
      ymat.set(EulerF(0.0f, rot.y, 0.0f));
      temp.mul(imat, ymat);
   }
   else
   {
      temp.mul(zRot, xRot);
   }

   temp.setColumn(3, pos);
   Parent::setRenderTransform(temp);
}

//----------------------------------------------------------------------------

void Camera::writePacketData(GameConnection *connection, BitStream *bstream)
{
   // Update client regardless of status flags.
   Parent::writePacketData(connection, bstream);

   Point3F pos;
   mObjToWorld.getColumn(3,&pos);
   bstream->setCompressionPoint(pos);
   mathWrite(*bstream, pos);
   bstream->write(mRot.x);
   if(mDataBlock && bstream->writeFlag(mDataBlock->cameraCanBank))
   {
      // Include mRot.y to allow for camera banking
      bstream->write(mRot.y);
   }
   bstream->write(mRot.z);

   U32 writeMode = mMode;
   Point3F writePos = mPosition;
   S32 gIndex = -1;
   if(mMode == OrbitObjectMode)
   {
      gIndex = bool(mOrbitObject) ? connection->getGhostIndex(mOrbitObject): -1;
      if(gIndex == -1)
      {
         writeMode = OrbitPointMode;
         if(bool(mOrbitObject))
            mOrbitObject->getWorldBox().getCenter(&writePos);
      }
   }
   else if(mMode == TrackObjectMode)
   {
      gIndex = bool(mOrbitObject) ? connection->getGhostIndex(mOrbitObject): -1;
      if(gIndex == -1)
         writeMode = StationaryMode;
   }
   bstream->writeRangedU32(writeMode, CameraFirstMode, CameraLastMode);

   if (writeMode == OrbitObjectMode || writeMode == OrbitPointMode)
   {
      bstream->write(mMinOrbitDist);
      bstream->write(mMaxOrbitDist);
      bstream->write(mCurOrbitDist);
      if(writeMode == OrbitObjectMode)
      {
         bstream->writeFlag(mObservingClientObject);
         bstream->writeInt(gIndex, NetConnection::GhostIdBitSize);
      }
      if (writeMode == OrbitPointMode)
         bstream->writeCompressedPoint(writePos);
   }
   else if(writeMode == TrackObjectMode)
   {
      bstream->writeInt(gIndex, NetConnection::GhostIdBitSize);
   }

   if(bstream->writeFlag(mNewtonMode))
   {
      bstream->write(mVelocity.x);
      bstream->write(mVelocity.y);
      bstream->write(mVelocity.z);
   }
   if(bstream->writeFlag(mNewtonRotation))
   {
      bstream->write(mAngularVelocity.x);
      bstream->write(mAngularVelocity.y);
      bstream->write(mAngularVelocity.z);
   }

   bstream->writeFlag(mValidEditOrbitPoint);
   if(writeMode == EditOrbitMode)
   {
      bstream->write(mEditOrbitPoint.x);
      bstream->write(mEditOrbitPoint.y);
      bstream->write(mEditOrbitPoint.z);
      bstream->write(mCurrentEditOrbitDist);
   }
}

//----------------------------------------------------------------------------

void Camera::readPacketData(GameConnection *connection, BitStream *bstream)
{
   Parent::readPacketData(connection, bstream);
   Point3F pos,rot;
   mathRead(*bstream, &pos);
   bstream->setCompressionPoint(pos);
   bstream->read(&rot.x);
   if(bstream->readFlag())
   {
      // Include rot.y to allow for camera banking
      bstream->read(&rot.y);
   }
   bstream->read(&rot.z);

   GameBase* obj = 0;
   mMode = (CameraMotionMode)bstream->readRangedU32(CameraFirstMode, CameraLastMode);
   mObservingClientObject = false;
   if (mMode == OrbitObjectMode || mMode == OrbitPointMode)
   {
      bstream->read(&mMinOrbitDist);
      bstream->read(&mMaxOrbitDist);
      bstream->read(&mCurOrbitDist);

      if(mMode == OrbitObjectMode)
      {
         mObservingClientObject = bstream->readFlag();
         S32 gIndex = bstream->readInt(NetConnection::GhostIdBitSize);
         obj = static_cast<GameBase*>(connection->resolveGhost(gIndex));
      }
      if (mMode == OrbitPointMode)
         bstream->readCompressedPoint(&mPosition);
   }
   else if (mMode == TrackObjectMode)
   {
      S32 gIndex = bstream->readInt(NetConnection::GhostIdBitSize);
      obj = static_cast<GameBase*>(connection->resolveGhost(gIndex));
   }

   if (obj != (GameBase*)mOrbitObject)
   {
      if (mOrbitObject)
      {
         clearProcessAfter();
         clearNotify(mOrbitObject);
      }

      mOrbitObject = obj;
      if (mOrbitObject)
      {
         processAfter(mOrbitObject);
         deleteNotify(mOrbitObject);
      }
   }

   mNewtonMode = bstream->readFlag();
   if(mNewtonMode)
   {
      bstream->read(&mVelocity.x);
      bstream->read(&mVelocity.y);
      bstream->read(&mVelocity.z);
   }

   mNewtonRotation = bstream->readFlag();
   if(mNewtonRotation)
   {
      bstream->read(&mAngularVelocity.x);
      bstream->read(&mAngularVelocity.y);
      bstream->read(&mAngularVelocity.z);
   }

   mValidEditOrbitPoint = bstream->readFlag();
   if(mMode == EditOrbitMode)
   {
      bstream->read(&mEditOrbitPoint.x);
      bstream->read(&mEditOrbitPoint.y);
      bstream->read(&mEditOrbitPoint.z);
      bstream->read(&mCurrentEditOrbitDist);
   }

   _setPosition(pos,rot);
   // Movement in OrbitObjectMode is not input-based - don't reset interpolation
   if(mMode != OrbitObjectMode)
   {
      mDelta.pos = pos;
      mDelta.posVec.set(0.0f, 0.0f, 0.0f);
      mDelta.rot = rot;
      mDelta.rotVec.set(0.0f, 0.0f, 0.0f);
   }
}

//----------------------------------------------------------------------------

U32 Camera::packUpdate(NetConnection *con, U32 mask, BitStream *bstream)
{
   Parent::packUpdate(con, mask, bstream);

   if (bstream->writeFlag(mask & UpdateMask))
   {
      bstream->writeFlag(mLocked);
      mathWrite(*bstream, mOffset);
   }

   if(bstream->writeFlag(mask & NewtonCameraMask))
   {
      bstream->write(mAngularForce);
      bstream->write(mAngularDrag);
      bstream->write(mMass);
      bstream->write(mDrag);
      bstream->write(mFlyForce);
      bstream->write(mSpeedMultiplier);
      bstream->write(mBrakeMultiplier);
   }

   if(bstream->writeFlag(mask & EditOrbitMask))
   {
      bstream->write(mEditOrbitPoint.x);
      bstream->write(mEditOrbitPoint.y);
      bstream->write(mEditOrbitPoint.z);
      bstream->write(mCurrentEditOrbitDist);
   }

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if(bstream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return 0;

   if (bstream->writeFlag(mask & MoveMask))
   {
      Point3F pos;
      mObjToWorld.getColumn(3,&pos);
      bstream->write(pos.x);
      bstream->write(pos.y);
      bstream->write(pos.z);
      bstream->write(mRot.x);
      bstream->write(mRot.z);

      // Only required if in NewtonFlyMode
      F32 len = mVelocity.len();
      if(bstream->writeFlag(mNewtonMode && len > 0.02f))
      {
         Point3F outVel = mVelocity;
         outVel *= 1.0f/len;
         bstream->writeNormalVector(outVel, 10);
         len *= 32.0f;  // 5 bits of fraction
         if(len > 8191)
            len = 8191;
         bstream->writeInt((S32)len, 13);
      }

      // Rotation
      len = mAngularVelocity.len();
      if(bstream->writeFlag(mNewtonRotation && len > 0.02f))
      {
         Point3F outVel = mAngularVelocity;
         outVel *= 1.0f/len;
         bstream->writeNormalVector(outVel, 10);
         len *= 32.0f;  // 5 bits of fraction
         if(len > 8191)
            len = 8191;
         bstream->writeInt((S32)len, 13);
      }
   }

   return 0;
}

//----------------------------------------------------------------------------

void Camera::unpackUpdate(NetConnection *con, BitStream *bstream)
{
   Parent::unpackUpdate(con,bstream);

   if (bstream->readFlag())
   {
      mLocked = bstream->readFlag();
      mathRead(*bstream, &mOffset);
   }

   // NewtonCameraMask
   if(bstream->readFlag())
   {
      bstream->read(&mAngularForce);
      bstream->read(&mAngularDrag);
      bstream->read(&mMass);
      bstream->read(&mDrag);
      bstream->read(&mFlyForce);
      bstream->read(&mSpeedMultiplier);
      bstream->read(&mBrakeMultiplier);
   }

   // EditOrbitMask
   if(bstream->readFlag())
   {
      bstream->read(&mEditOrbitPoint.x);
      bstream->read(&mEditOrbitPoint.y);
      bstream->read(&mEditOrbitPoint.z);
      bstream->read(&mCurrentEditOrbitDist);
   }

   // controlled by the client?
   if(bstream->readFlag())
      return;

   // MoveMask
   if (bstream->readFlag())
   {
      Point3F pos,rot;
      bstream->read(&pos.x);
      bstream->read(&pos.y);
      bstream->read(&pos.z);
      bstream->read(&rot.x);
      bstream->read(&rot.z);
      _setPosition(pos,rot);

      // NewtonMode
      if(bstream->readFlag())
      {
         bstream->readNormalVector(&mVelocity, 10);
         mVelocity *= bstream->readInt(13) / 32.0f;
      }

      // NewtonRotation
      mNewtonRotation = bstream->readFlag();
      if(mNewtonRotation)
      {
         bstream->readNormalVector(&mAngularVelocity, 10);
         mAngularVelocity *= bstream->readInt(13) / 32.0f;
      }

      if(mMode != OrbitObjectMode)
      {
         // New delta for client side interpolation
         mDelta.pos = pos;
         mDelta.rot = rot;
         mDelta.posVec = mDelta.rotVec = VectorF(0.0f, 0.0f, 0.0f);
      }
   }
}

//----------------------------------------------------------------------------

void Camera::initPersistFields()
{
   addGroup( "Camera" );
      addProtectedField( "controlMode", TYPEID< CameraMotionMode >(), Offset( mMode, Camera ),
         &_setModeField, &defaultProtectedGetFn,
         "The current camera control mode." );
   endGroup( "Camera" );

   addGroup( "Camera: Newton Mode" );
      addField( "newtonMode",               TypeBool,   Offset(mNewtonMode, Camera),
         "Apply smoothing (acceleration and damping) to camera movements." );
      addField( "newtonRotation",           TypeBool,   Offset(mNewtonRotation, Camera),
         "Apply smoothing (acceleration and damping) to camera rotations." );
      addProtectedField( "mass",            TypeF32,    Offset(mMass, Camera),            &_setNewtonField, &defaultProtectedGetFn,
         "The camera's mass (Newton mode only).  Default value is 10." );
      addProtectedField( "drag",            TypeF32,    Offset(mDrag, Camera),            &_setNewtonField, &defaultProtectedGetFn,
         "Drag on camera when moving (Newton mode only).  Default value is 2." );
      addProtectedField( "force",           TypeF32,    Offset(mFlyForce, Camera),        &_setNewtonField, &defaultProtectedGetFn,
         "Force applied on camera when asked to move (Newton mode only).  Default value is 500." );
      addProtectedField( "angularDrag",     TypeF32,    Offset(mAngularDrag, Camera),     &_setNewtonField, &defaultProtectedGetFn,
         "Drag on camera when rotating (Newton mode only).  Default value is 2." );
      addProtectedField( "angularForce",    TypeF32,    Offset(mAngularForce, Camera),    &_setNewtonField, &defaultProtectedGetFn,
         "Force applied on camera when asked to rotate (Newton mode only).  Default value is 100." );
      addProtectedField( "speedMultiplier", TypeF32,    Offset(mSpeedMultiplier, Camera), &_setNewtonField, &defaultProtectedGetFn,
         "Speed multiplier when triggering the accelerator (Newton mode only).  Default value is 2." );
      addProtectedField( "brakeMultiplier", TypeF32,    Offset(mBrakeMultiplier, Camera), &_setNewtonField, &defaultProtectedGetFn,
         "Speed multiplier when triggering the brake (Newton mode only).  Default value is 2." );
   endGroup( "Camera: Newton Mode" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void Camera::consoleInit()
{
   Con::addVariable("$Camera::movementSpeed", TypeF32, &smMovementSpeed,
      "@brief Global camera movement speed in units/s (typically m/s), with a base value of 40.\n\n"
      "Used in the following camera modes:\n"
      "- Edit Orbit Mode\n"
      "- Fly Mode\n"
      "- Overhead Mode\n"
      "@ingroup BaseCamera\n");

   // ExtendedMove support
   Con::addVariable("$camera::extendedMovePosRotIndex", TypeS32, &smExtendedMovePosRotIndex, 
      "@brief The ExtendedMove position/rotation index used for camera movements.\n\n"
	   "@ingroup BaseCamera\n");
}

//-----------------------------------------------------------------------------

bool Camera::_setNewtonField( void *object, const char *index, const char *data )
{
   static_cast<Camera*>(object)->setMaskBits(NewtonCameraMask);
   return true;  // ok to set value
}

//-----------------------------------------------------------------------------

bool Camera::_setModeField( void *object, const char *index, const char *data )
{
   Camera *cam = static_cast<Camera*>( object );

   if( dStricmp(data, "Fly") == 0 )
   {
      cam->setFlyMode();
      return false; // already changed mode
   }

   else if( dStricmp(data, "EditOrbit" ) == 0 )
   {
      cam->setEditOrbitMode();
      return false; // already changed mode
   }

   else if( (dStricmp(data, "OrbitObject") == 0 && cam->mMode != OrbitObjectMode) ||
            (dStricmp(data, "TrackObject") == 0 && cam->mMode != TrackObjectMode) ||
            (dStricmp(data, "OrbitPoint")  == 0 && cam->mMode != OrbitPointMode)  )
   {
      Con::warnf("Couldn't change Camera mode to %s: required information missing.  Use camera.set%s().", data, data);
      return false; // don't change the mode - not valid
   }

   else if( dStricmp(data, "OrbitObject") != 0 &&
            dStricmp(data, "TrackObject") != 0 &&
            bool(cam->mOrbitObject) )
   {
      cam->clearProcessAfter();
      cam->clearNotify(cam->mOrbitObject);
      cam->mOrbitObject = NULL;
   }

   // make sure the requested mode is supported, and set it
   // NOTE: The _CameraMode namespace is generated by ImplementEnumType above
   
   const EngineEnumTable& enums = *( TYPE< CameraMotionMode >()->getEnumTable() );
   const U32 numValues = enums.getNumValues();
   
   for( S32 i = 0; i < numValues; ++i)
   {
      if( dStricmp(data, enums[i].mName) == 0 )
      {
         cam->mMode = (CameraMotionMode) enums[i].mInt;
         return false;
      }
   }

   Con::warnf("Unsupported camera mode: %s", data);
   return false;
}

//-----------------------------------------------------------------------------

Point3F Camera::getPosition()
{
   static Point3F position;
   mObjToWorld.getColumn(3, &position);
   return position;
}

//-----------------------------------------------------------------------------

void Camera::setFlyMode()
{
   mMode = FlyMode;

   if (bool(mOrbitObject))
   {
      clearProcessAfter();
      clearNotify(mOrbitObject);
   }
   mOrbitObject = NULL;
}

//-----------------------------------------------------------------------------

void Camera::setNewtonFlyMode()
{
   mNewtonMode = true;
   setFlyMode();
}

//-----------------------------------------------------------------------------

void Camera::setOrbitMode(GameBase *obj, const Point3F &pos, const Point3F &rot, const Point3F& offset, F32 minDist, F32 maxDist, F32 curDist, bool ownClientObject, bool locked)
{
   mObservingClientObject = ownClientObject;

   if (bool(mOrbitObject))
   {
      clearProcessAfter();
      clearNotify(mOrbitObject);
   }

   mOrbitObject = obj;
   if(bool(mOrbitObject))
   {
      processAfter(mOrbitObject);
      deleteNotify(mOrbitObject);
      mOrbitObject->getWorldBox().getCenter(&mPosition);
      mMode = OrbitObjectMode;
   }
   else
   {
      mMode = OrbitPointMode;
      mPosition = pos;
   }

   _setPosition(mPosition, rot);

   mMinOrbitDist = minDist;
   mMaxOrbitDist = maxDist;
   mCurOrbitDist = curDist;

   if (locked != mLocked || mOffset != offset)
   {
      mLocked = locked;
      mOffset = offset;
      setMaskBits(UpdateMask);
   }
}

//-----------------------------------------------------------------------------

void Camera::setTrackObject(GameBase *obj, const Point3F &offset)
{
   if(bool(mOrbitObject))
   {
      clearProcessAfter();
      clearNotify(mOrbitObject);
   }
   mOrbitObject = obj;
   if(bool(mOrbitObject))
   {
      processAfter(mOrbitObject);
      deleteNotify(mOrbitObject);
   }

   mOffset = offset;
   mMode = TrackObjectMode;

   setMaskBits( UpdateMask );
}

//-----------------------------------------------------------------------------

void Camera::_validateEyePoint(F32 pos, MatrixF *mat)
{
   if (pos != 0) 
   {
      // Use the eye transform to orient the camera
      Point3F dir;
      mat->getColumn(1, &dir);
      if (mMaxOrbitDist - mMinOrbitDist > 0.0f)
         pos *= mMaxOrbitDist - mMinOrbitDist;
      
      // Use the camera node's pos.
      Point3F startPos = getRenderPosition();
      Point3F endPos;

      // Make sure we don't extend the camera into anything solid
      if(mOrbitObject)
         mOrbitObject->disableCollision();
      disableCollision();
      RayInfo collision;
      U32 mask = TerrainObjectType |
                 WaterObjectType |
                 StaticShapeObjectType |
                 PlayerObjectType |
                 ItemObjectType |
                 VehicleObjectType;

      SceneContainer* pContainer = isServerObject() ? &gServerContainer : &gClientContainer;
      if (!pContainer->castRay(startPos, startPos - dir * 2.5 * pos, mask, &collision))
         endPos = startPos - dir * pos;
      else
      {
         float dot = mDot(dir, collision.normal);
         if (dot > 0.01f)
         {
            F32 colDist = mDot(startPos - collision.point, dir) - (1 / dot) * CameraRadius;
            if (colDist > pos)
               colDist = pos;
            if (colDist < 0.0f)
               colDist = 0.0f;
            endPos = startPos - dir * colDist;
         }
         else
            endPos = startPos - dir * pos;
      }
      mat->setColumn(3, endPos);
      enableCollision();
      if(mOrbitObject)
         mOrbitObject->enableCollision();
   }
}

//-----------------------------------------------------------------------------

void Camera::setTransform(const MatrixF& mat)
{
   // This method should never be called on the client.

   // This currently converts all rotation in the mat into
   // rotations around the z and x axis.
   Point3F pos,vec;
   mat.getColumn(1, &vec);
   mat.getColumn(3, &pos);
   Point3F rot(-mAtan2(vec.z, mSqrt(vec.x * vec.x + vec.y * vec.y)), 0.0f, -mAtan2(-vec.x, vec.y));
   _setPosition(pos,rot);
}

//-----------------------------------------------------------------------------

void Camera::setRenderTransform(const MatrixF& mat)
{
   // This method should never be called on the client.

   // This currently converts all rotation in the mat into
   // rotations around the z and x axis.
   Point3F pos,vec;
   mat.getColumn(1,&vec);
   mat.getColumn(3,&pos);
   Point3F rot(-mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)),0,-mAtan2(-vec.x,vec.y));
   _setRenderPosition(pos,rot);
}

//-----------------------------------------------------------------------------

F32 Camera::getDamageFlash() const
{
   if (mMode == OrbitObjectMode && isServerObject() && bool(mOrbitObject))
   {
      const GameBase *castObj = mOrbitObject;
      const ShapeBase* psb = dynamic_cast<const ShapeBase*>(castObj);
      if (psb)
         return psb->getDamageFlash();
   }

   return mDamageFlash;
}

//-----------------------------------------------------------------------------

F32 Camera::getWhiteOut() const
{
   if (mMode == OrbitObjectMode && isServerObject() && bool(mOrbitObject))
   {
      const GameBase *castObj = mOrbitObject;
      const ShapeBase* psb = dynamic_cast<const ShapeBase*>(castObj);
      if (psb)
         return psb->getWhiteOut();
   }

   return mWhiteOut;
}

//-----------------------------------------------------------------------------

void Camera::setVelocity(const VectorF& vel)
{
   mVelocity = vel;
   setMaskBits(MoveMask);
}

//-----------------------------------------------------------------------------

void Camera::setAngularVelocity(const VectorF& vel)
{
   mAngularVelocity = vel;
   setMaskBits(MoveMask);
}

//-----------------------------------------------------------------------------

void Camera::setEditOrbitMode()
{
   mMode = EditOrbitMode;

   if (bool(mOrbitObject))
   {
      clearProcessAfter();
      clearNotify(mOrbitObject);
   }
   mOrbitObject = NULL;

   // If there is a valid orbit point, then point to it
   // rather than move the camera.
   if(mValidEditOrbitPoint)
   {
      Point3F currentPos;
      mObjToWorld.getColumn(3,&currentPos);

      Point3F dir = mEditOrbitPoint - currentPos;
      mCurrentEditOrbitDist = dir.len();
      dir.normalize();

      F32 yaw, pitch;
      MathUtils::getAnglesFromVector(dir, yaw, pitch);
      mRot.x = -pitch;
      mRot.z = yaw;
   }
}

//-----------------------------------------------------------------------------

void Camera::_calcOrbitPoint( MatrixF* mat, const Point3F& rot )
{
   Point3F pos;

   pos.x = mCurOrbitDist * mSin( rot.x + mDegToRad( 90.0f ) ) * mCos( -1.0f * ( rot.z + mDegToRad( 90.0f ) ) ) + mPosition.x + mOffset.x;
   pos.y = mCurOrbitDist * mSin( rot.x + mDegToRad( 90.0f ) ) * mSin( -1.0f * ( rot.z + mDegToRad( 90.0f ) ) ) + mPosition.y + mOffset.y;
   pos.z = mCurOrbitDist * mSin( rot.x ) + mPosition.z + mOffset.z;

   mat->setColumn( 3, pos );
}

//-----------------------------------------------------------------------------

void Camera::_calcEditOrbitPoint( MatrixF *mat, const Point3F& rot )
{
   //Point3F dir;
   //mat->getColumn(1, &dir);

   //Point3F startPos = getRenderPosition();
   //Point3F endPos = startPos - dir * mCurrentEditOrbitDist;

   Point3F pos;
   pos.x = mCurrentEditOrbitDist * mSin(rot.x + mDegToRad(90.0f)) * mCos(-1.0f * (rot.z + mDegToRad(90.0f))) + mEditOrbitPoint.x;
   pos.y = mCurrentEditOrbitDist * mSin(rot.x + mDegToRad(90.0f)) * mSin(-1.0f * (rot.z + mDegToRad(90.0f))) + mEditOrbitPoint.y;
   pos.z = mCurrentEditOrbitDist * mSin(rot.x) + mEditOrbitPoint.z;

   mat->setColumn(3, pos);
}

//-----------------------------------------------------------------------------

void Camera::setValidEditOrbitPoint( bool state )
{
   mValidEditOrbitPoint = state;
   setMaskBits(EditOrbitMask);
}

//-----------------------------------------------------------------------------

Point3F Camera::getEditOrbitPoint() const
{
   return mEditOrbitPoint;
}

//-----------------------------------------------------------------------------

void Camera::setEditOrbitPoint( const Point3F& pnt )
{
   // Change the point that we orbit in EditOrbitMode.
   // We'll also change the facing and distance of the
   // camera so that it doesn't jump around.
   Point3F currentPos;
   mObjToWorld.getColumn(3,&currentPos);

   Point3F dir = pnt - currentPos;
   mCurrentEditOrbitDist = dir.len();

   if(mMode == EditOrbitMode)
   {
      dir.normalize();

      F32 yaw, pitch;
      MathUtils::getAnglesFromVector(dir, yaw, pitch);
      mRot.x = -pitch;
      mRot.z = yaw;
   }

   mEditOrbitPoint = pnt;

   setMaskBits(EditOrbitMask);
}

//----------------------------------------------------------------------------

void Camera::lookAt( const Point3F& pos )
{
   Point3F vec;
   mObjToWorld.getColumn(3, &mPosition);
   vec = pos - mPosition;
   vec.normalizeSafe();
   F32 pitch, yaw;
   MathUtils::getAnglesFromVector(vec, yaw, pitch);
   mRot.x = -pitch;
   mRot.z = yaw;
   _setPosition(mPosition, mRot);
}

//----------------------------------------------------------------------------

void Camera::autoFitRadius( F32 radius )
{
   F32 fov = mDegToRad( getCameraFov() );
   F32 viewradius = (radius * 2.0f) / mTan(fov * 0.5f);

   // Be careful of infinite sized objects.  Clip to 16km
   if(viewradius > 16000.0f)
      viewradius = 16000.0f;

   if(mMode == EditOrbitMode && mValidEditOrbitPoint)
   {
      mCurrentEditOrbitDist = viewradius;
   }
   else if(mValidEditOrbitPoint)
   {
      mCurrentEditOrbitDist = viewradius;

      Point3F currentPos;
      mObjToWorld.getColumn(3,&currentPos);

      Point3F dir = mEditOrbitPoint - currentPos;
      dir.normalize();

      F32 yaw, pitch;
      MathUtils::getAnglesFromVector(dir, yaw, pitch);
      mRot.x = -pitch;
      mRot.z = yaw;

      mPosition = mEditOrbitPoint;
      _setPosition(mPosition, mRot);
      _calcEditOrbitPoint(&mObjToWorld, mRot);
   }
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getMode, Camera::CameraMotionMode, (), ,
                   "Returns the current camera control mode.\n\n"
                   "@see CameraMotionMode")
{
   return object->getMode();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getPosition, Point3F, (), ,
                   "Get the camera's position in the world.\n\n"
                   "@returns The position in the form of \"x y z\".")
{
   return object->getPosition();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getRotation, Point3F, (), ,
                   "Get the camera's Euler rotation in radians.\n\n"
                   "@returns The rotation in radians in the form of \"x y z\".")
{
   return object->getRotation();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setRotation, void, ( Point3F rot ), ,
                   "Set the camera's Euler rotation in radians.\n\n"
                   "@param rot The rotation in radians in the form of \"x y z\"."
                   "@note Rotation around the Y axis is ignored" )
{
   return object->setRotation( rot );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getOffset, Point3F, (), ,
                   "Get the camera's offset from its orbit or tracking point.\n\n"
                   "The offset is added to the camera's position when set to CameraMode::OrbitObject.\n"
                   "@returns The offset in the form of \"x y z\".")
{
   return object->getOffset();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setOffset, void, (Point3F offset), ,
                   "Set the camera's offset.\n\n"
                   "The offset is added to the camera's position when set to CameraMode::OrbitObject.\n"
                   "@param offset The distance to offset the camera by in the form of \"x y z\".")
{
   object->setOffset(offset);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setOrbitMode, void, (GameBase* orbitObject, TransformF orbitPoint, F32 minDistance, F32 maxDistance, 
                    F32 initDistance, bool ownClientObj, Point3F offset, bool locked), (false, Point3F::Zero, false),
                    "Set the camera to orbit around the given object, or if none is given, around the given point.\n\n"
                    "@param orbitObject The object to orbit around.  If no object is given (0 or blank string is passed in) use the orbitPoint instead\n"
                    "@param orbitPoint The point to orbit around when no object is given.  In the form of \"x y z ax ay az aa\" such as returned by SceneObject::getTransform().\n"
                    "@param minDistance The minimum distance allowed to the orbit object or point.\n"
                    "@param maxDistance The maximum distance allowed from the orbit object or point.\n"
                    "@param initDistance The initial distance from the orbit object or point.\n"
                    "@param ownClientObj [optional] Are we orbiting an object that is owned by us?  Default is false.\n"
                    "@param offset [optional] An offset added to the camera's position.  Default is no offset.\n"
                    "@param locked [optional] Indicates the camera does not receive input from the player.  Default is false.\n"
                    "@see Camera::setOrbitObject()\n"
                    "@see Camera::setOrbitPoint()\n")
{
   MatrixF mat;
   orbitPoint.mOrientation.setMatrix(&mat);
   object->setOrbitMode(orbitObject, orbitPoint.mPosition, mat.toEuler(), offset, 
      minDistance, maxDistance, initDistance, ownClientObj, locked);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setOrbitObject, bool, (GameBase* orbitObject, VectorF rotation, F32 minDistance, 
                    F32 maxDistance, F32 initDistance, bool ownClientObject, Point3F offset, bool locked),
                    (false, Point3F::Zero, false),
                    "Set the camera to orbit around a given object.\n\n"
                    "@param orbitObject The object to orbit around.\n"
                    "@param rotation The initial camera rotation about the object in radians in the form of \"x y z\".\n"
                    "@param minDistance The minimum distance allowed to the orbit object or point.\n"
                    "@param maxDistance The maximum distance allowed from the orbit object or point.\n"
                    "@param initDistance The initial distance from the orbit object or point.\n"
                    "@param ownClientObject [optional] Are we orbiting an object that is owned by us?  Default is false.\n"
                    "@param offset [optional] An offset added to the camera's position.  Default is no offset.\n"
                    "@param locked [optional] Indicates the camera does not receive input from the player.  Default is false.\n"
                    "@returns false if the given object could not be found.\n"
                    "@see Camera::setOrbitMode()\n")
{
   if( !orbitObject )
   {
      Con::errorf( "Camera::setOrbitObject - Invalid object");
      return false;
   }

   object->setOrbitMode(orbitObject, Point3F(0.0f, 0.0f, 0.0f), rotation, offset, 
      minDistance, maxDistance, initDistance, ownClientObject, locked);
   return true;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setOrbitPoint, void, (TransformF orbitPoint, F32 minDistance, F32 maxDistance, F32 initDistance, 
                    Point3F offset, bool locked),
                    (Point3F::Zero, false),
                    "Set the camera to orbit around a given point.\n\n"
                    "@param orbitPoint The point to orbit around.  In the form of \"x y z ax ay az aa\" such as returned by SceneObject::getTransform().\n"
                    "@param minDistance The minimum distance allowed to the orbit object or point.\n"
                    "@param maxDistance The maximum distance allowed from the orbit object or point.\n"
                    "@param initDistance The initial distance from the orbit object or point.\n"
                    "@param offset [optional] An offset added to the camera's position.  Default is no offset.\n"
                    "@param locked [optional] Indicates the camera does not receive input from the player.  Default is false.\n"
                    "@see Camera::setOrbitMode()\n")
{
   MatrixF mat;
   orbitPoint.mOrientation.setMatrix(&mat);
   object->setOrbitMode(NULL, orbitPoint.mPosition, mat.toEuler(), offset, 
      minDistance, maxDistance, initDistance, false, locked);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setTrackObject, bool, (GameBase* trackObject, Point3F offset), (Point3F::Zero),
                    "Set the camera to track a given object.\n\n"
                    "@param trackObject The object to track.\n"
                    "@param offset [optional] An offset added to the camera's position.  Default is no offset.\n"
                    "@returns false if the given object could not be found.\n")
{
   if(!trackObject)
   {
      Con::warnf("Cannot track non-existing object.");
      return false;
   }

   object->setTrackObject(trackObject, offset);
   return true;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setEditOrbitMode, void, (), ,
                    "Set the editor camera to orbit around a point set with Camera::setEditOrbitPoint().\n\n"
                    "@note This method is generally used only within the World Editor and other tools.  To "
                    "orbit about an object or point within a game, see Camera::setOrbitMode() and its helper methods.\n")
{
   object->setEditOrbitMode();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setFlyMode, void, (), ,
                   "Set the camera to fly freely.\n\n"
                   "Allows the camera to have 6 degrees of freedom.  Provides for instantaneous motion "
                   "and rotation unless one of the Newton fields has been set to true.  See Camera::newtonMode "
                   "and Camera::newtonRotation.\n")
{
   object->setFlyMode();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setNewtonFlyMode, void, (), ,
                    "Set the camera to fly freely, but with ease-in and ease-out.\n\n"
                    "This method allows for the same 6 degrees of freedom as Camera::setFlyMode() but "
                    "activates the ease-in and ease-out on the camera's movement.  To also activate "
                    "Newton mode for the camera's rotation, set Camera::newtonRotation to true.")
{
   object->setNewtonFlyMode();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, isRotationDamped, bool, (), ,
                    "Is this a Newton Fly mode camera with damped rotation?\n\n"
                    "@returns true if the camera uses a damped rotation.  i.e. Camera::newtonRotation is set to true.\n")
{
   return object->isRotationDamped();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getAngularVelocity, VectorF, (), ,
                   "Get the angular velocity for a Newton mode camera.\n\n"
                   "@returns The angular velocity in the form of \"x y z\".\n"
                   "@note Only returns useful results when Camera::newtonRotation is set to true.")
{
   return object->getAngularVelocity();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setAngularVelocity, void, (VectorF velocity), ,
                   "Set the angular velocity for a Newton mode camera.\n\n"
                   "@param velocity The angular velocity infor form of \"x y z\".\n"
                   "@note Only takes affect when Camera::newtonRotation is set to true.")
{
   object->setAngularVelocity(velocity);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setAngularForce, void, (F32 force), ,
                   "Set the angular force for a Newton mode camera.\n\n"
                   "@param force The angular force applied when attempting to rotate the camera."
                   "@note Only takes affect when Camera::newtonRotation is set to true.")
{
   object->setAngularForce(force);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setAngularDrag, void, (F32 drag), ,
                   "Set the angular drag for a Newton mode camera.\n\n"
                   "@param drag The angular drag applied while the camera is rotating."
                   "@note Only takes affect when Camera::newtonRotation is set to true.")
{
   object->setAngularDrag(drag);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setMass, void, (F32 mass), ,
                   "Set the mass for a Newton mode camera.\n\n"
                   "@param mass The mass used during ease-in and ease-out calculations."
                   "@note Only used when Camera is in Newton mode.")
{
   object->setMass(mass);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, getVelocity, VectorF, (), ,
                   "Get the velocity for the camera.\n\n"
                   "@returns The camera's velocity in the form of \"x y z\"."
                   "@note Only useful when the Camera is in Newton mode.")
{
   return object->getVelocity();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setVelocity, void, (VectorF velocity), ,
                   "Set the velocity for the camera.\n\n"
                   "@param velocity The camera's velocity in the form of \"x y z\"."
                   "@note Only affects the Camera when in Newton mode.")
{
   object->setVelocity(velocity);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setDrag, void, (F32 drag), ,
                   "Set the drag for a Newton mode camera.\n\n"
                   "@param drag The drag applied to the camera while moving."
                   "@note Only used when Camera is in Newton mode.")
{
   object->setDrag(drag);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setFlyForce, void, (F32 force), ,
                   "Set the force applied to a Newton mode camera while moving.\n\n"
                   "@param force The force applied to the camera while attempting to move."
                   "@note Only used when Camera is in Newton mode.")
{
   object->setFlyForce(force);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setSpeedMultiplier, void, (F32 multiplier), ,
                   "Set the Newton mode camera speed multiplier when trigger[0] is active.\n\n"
                   "@param multiplier The speed multiplier to apply."
                   "@note Only used when Camera is in Newton mode.")
{
   object->setSpeedMultiplier(multiplier);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( Camera, setBrakeMultiplier, void, (F32 multiplier), ,
                   "Set the Newton mode camera brake multiplier when trigger[1] is active.\n\n"
                   "@param multiplier The brake multiplier to apply."
                   "@note Only used when Camera is in Newton mode.")
{
   object->setBrakeMultiplier(multiplier);
}

//----------------------------------------------------------------------------

DefineEngineMethod( Camera, isEditOrbitMode, bool, (), ,
                   "Is the camera in edit orbit mode?\n\n"
                   "@returns true if the camera is in edit orbit mode.")
{
   return object->isEditOrbitMode();
}

//----------------------------------------------------------------------------

DefineEngineMethod( Camera, setValidEditOrbitPoint, void, (bool validPoint), ,
                   "Set if there is a valid editor camera orbit point.\n"
                   "When validPoint is set to false the Camera operates as if it is "
                   "in Fly mode rather than an Orbit mode.\n\n"
                   "@param validPoint Indicates the validity of the orbit point."
                   "@note Only used when Camera is in Edit Orbit Mode.")
{
   object->setValidEditOrbitPoint(validPoint);
}

//----------------------------------------------------------------------------

DefineEngineMethod( Camera, setEditOrbitPoint, void, (Point3F point), ,
                   "Set the editor camera's orbit point.\n\n"
                   "@param point The point the camera will orbit in the form of \"x y z\".")
{
   object->setEditOrbitPoint(point);
}

//----------------------------------------------------------------------------

DefineEngineMethod( Camera, autoFitRadius, void, (F32 radius), ,
                   "Move the camera to fully view the given radius.\n\n"
                   "@note For this operation to take affect a valid edit orbit point must first be specified.  See Camera::setEditOrbitPoint().\n"
                   "@param radius The radius to view.")
{
   object->autoFitRadius(radius);
}

//----------------------------------------------------------------------------

DefineEngineMethod( Camera, lookAt, void, (Point3F point), ,
                   "Point the camera at the specified position.  Does not work in Orbit or Track modes.\n\n"
                   "@param point The position to point the camera at.")
{
   object->lookAt(point);
}
