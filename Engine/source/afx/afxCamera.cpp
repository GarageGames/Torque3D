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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//
// afxCamera implements a modified camera for demonstrating a third person camera style
// which is more common to RPG games than the standard FPS style camera. For the most part,
// it is a hybrid of the standard TGE camera and the third person mode of the Advanced Camera
// resource, authored by Thomas "Man of Ice" Lund. This camera implements the bare minimum
// required for demonstrating an RPG style camera and leaves tons of room for improvement. 
// It should be replaced with a better camera if possible.
//
// Advanced Camera Resource by Thomas "Man of Ice" Lund:
//   http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=5471
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "math/mathUtils.h"
#include "math/mathIO.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/camera.h"
#include "T3D/player.h"
#include "T3D/sfx/sfx3DWorld.h"

#include "afx/afxCamera.h"

#define MaxPitch      1.3962f
#define CameraRadius  0.05f;

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxCameraData

IMPLEMENT_CO_DATABLOCK_V1(afxCameraData);

ConsoleDocClass( afxCameraData,
   "@brief A datablock that describes an afxCamera.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

U32 afxCameraData::sCameraCollisionMask = TerrainObjectType | InteriorLikeObjectType | TerrainLikeObjectType;

void afxCameraData::initPersistFields()
{
  Con::addVariable("pref::afxCamera::collisionMask", TypeS32, &sCameraCollisionMask);

  Parent::initPersistFields();
}

void afxCameraData::packData(BitStream* stream)
{
  Parent::packData(stream);
}

void afxCameraData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxCamera

IMPLEMENT_CO_NETOBJECT_V1(afxCamera);

ConsoleDocClass( afxCamera,
   "@brief A 3rd person camera object.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
);

afxCamera::afxCamera()
{
  mNetFlags.clear(Ghostable);
  mTypeMask |= CameraObjectType;
  mDelta.pos = Point3F(0,0,100);
  mDelta.rot = Point3F(0,0,0);
  mDelta.posVec = mDelta.rotVec = VectorF(0,0,0);
  mObjToWorld.setColumn(3, mDelta.pos);
  mRot = mDelta.rot;
  
  mMinOrbitDist = 0;
  mMaxOrbitDist = 0;
  mCurOrbitDist = 0;
  mOrbitObject = NULL;
  mPosition.set(0.f, 0.f, 0.f);
  mObservingClientObject = false;
  mMode = FlyMode;
  
  mCam_subject = NULL;
  mCoi_offset.set(0, 0, 2);
  mCam_offset.set(0, 0, 0);
  mCam_distance = 0.0f;
  mCam_angle = 0.0f;
  mCam_dirty = false;
      
  mFlymode_saved = false;
  mThird_person_snap_s = 1;
  mThird_person_snap_c = 1;
  mFlymode_saved_pos.zero();

  mDamageState = Disabled;
}

afxCamera::~afxCamera()
{
}

//----------------------------------------------------------------------------

void afxCamera::cam_update(F32 dt, bool on_server) 
{
  if (mMode == ThirdPersonMode && mCam_subject)
    cam_update_3pov(dt, on_server);
}

void afxCamera::set_cam_pos(const Point3F& pos,const Point3F& rot)
{
   MatrixF xRot, zRot;
   xRot.set(EulerF(rot.x, 0, 0));
   zRot.set(EulerF(0, 0, rot.z));
   MatrixF temp;
   temp.mul(zRot, xRot);
   temp.setColumn(3, pos);
   Parent::setTransform(temp);
   mRot = rot;
}


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

Point3F &afxCamera::getPosition()
{
   static Point3F position;
   mObjToWorld.getColumn(3, &position);
   return position;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//    NEW Observer Code
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void afxCamera::setFlyMode()
{
  mMode = FlyMode;
  if (mFlymode_saved)
    snapToPosition(mFlymode_saved_pos);
  
  if (bool(mOrbitObject)) 
  {
    clearProcessAfter();
    clearNotify(mOrbitObject);
  }
  mOrbitObject = NULL;
}

void afxCamera::setOrbitMode(GameBase *obj, Point3F &pos, AngAxisF &rot, F32 minDist, F32 maxDist, F32 curDist, bool ownClientObject)
{
   mObservingClientObject = ownClientObject;

   if(bool(mOrbitObject)) {
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

   QuatF q(rot);
   MatrixF tempMat(true);
   q.setMatrix(&tempMat);
   Point3F dir;
   tempMat.getColumn(1, &dir);

   set_cam_pos(mPosition, dir);

   mMinOrbitDist = minDist;
   mMaxOrbitDist = maxDist;
   mCurOrbitDist = curDist;
}


void afxCamera::validateEyePoint(F32 pos, MatrixF *mat)
{
   if (pos != 0) {
      // Use the eye transform to orient the camera
      Point3F dir;
      mat->getColumn(1, &dir);
      pos *= mMaxOrbitDist - mMinOrbitDist;
      // Use the camera node's pos.
      Point3F startPos;
      Point3F endPos;
      mObjToWorld.getColumn(3,&startPos);

      // Make sure we don't extend the camera into anything solid
      if(mOrbitObject)
         mOrbitObject->disableCollision();
      disableCollision();
      RayInfo collision;

      SceneContainer* pContainer = isServerObject() ? &gServerContainer : &gClientContainer;
      if (!pContainer->castRay(startPos, startPos - dir * 2.5 * pos, afxCameraData::sCameraCollisionMask, &collision))
         endPos = startPos - dir * pos;
      else
      {
         float dot = mDot(dir, collision.normal);
         if(dot > 0.01)
         {
            float colDist = mDot(startPos - collision.point, dir) - (1 / dot) * CameraRadius;
            if(colDist > pos)
               colDist = pos;
            if(colDist < 0)
               colDist = 0;
            endPos = startPos - dir * colDist;
         }
         else
            endPos = startPos - dir * pos;
      }
      mat->setColumn(3,endPos);
      enableCollision();
      if(mOrbitObject)
         mOrbitObject->enableCollision();
   }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//





// Sets the position and calculates rotation
void afxCamera::snapToPosition(const Point3F& tPos) 
{
  MatrixF transMat;

  if (mCam_subject)
  {
    // get the subject's transform
    MatrixF objToWorld = mCam_subject->getRenderTransform();

    // transform the center-of-interest to world-space
    Point3F objPos;
    objToWorld.mulP(mCoi_offset, &objPos);

    // find normalized direction vector looking from camera to coi
    VectorF dirVec = objPos - tPos;
    dirVec.normalize();

    MathUtils::getAnglesFromVector(dirVec, mRot.z, mRot.x);
    mRot.x = 0 - mRot.x;

    transMat = MathUtils::createOrientFromDir(dirVec);
  } 
  else
  {
    transMat.identity();
  }

  transMat.setColumn(3, tPos);
  Parent::setTransform(transMat);
}

void afxCamera::setCameraSubject(SceneObject* new_subject)
{
  // cleanup any existing chase subject
  if (mCam_subject)
  {
    if (dynamic_cast<GameBase*>(mCam_subject))
      clearProcessAfter();
    clearNotify(mCam_subject);
  }
  
  mCam_subject = new_subject;
  
  // set associations with new chase subject 
  if (mCam_subject)
  {
    if (dynamic_cast<GameBase*>(mCam_subject))
      processAfter((GameBase*)mCam_subject);
    deleteNotify(mCam_subject);
  }

  mMode = (mCam_subject) ? ThirdPersonMode : FlyMode;
  setMaskBits(SubjectMask);
}

void afxCamera::setThirdPersonOffset(const Point3F& offset) 
{
  // new method
  if (mCam_distance > 0.0f)
  {
    if (isClientObject())
    {
      GameConnection* conn = GameConnection::getConnectionToServer();
      if (conn)
      {
        // this auto switches to/from first person 
        if (conn->isFirstPerson())
        {
          if (mCam_distance >= 1.0f)
            conn->setFirstPerson(false);
        }
        else
        {
          if (mCam_distance < 1.0f)
            conn->setFirstPerson(true);
        }
      }
    }

	mCam_offset = offset;
	mCam_dirty = true;

    return;
  }

  // old backwards-compatible method
  if (offset.y != mCam_offset.y && isClientObject())
  {
    GameConnection* conn = GameConnection::getConnectionToServer();
    if (conn)
    {
      // this auto switches to/from first person 
      if (conn->isFirstPerson())
      {
        if (offset.y <= -1.0f)
          conn->setFirstPerson(false);
      }
      else
      {
        if (offset.y > -1.0f)
          conn->setFirstPerson(true);
      }
    }
  }

  mCam_offset = offset;
  mCam_dirty = true;
}

void afxCamera::setThirdPersonOffset(const Point3F& offset, const Point3F& coi_offset) 
{
  mCoi_offset = coi_offset;
  setThirdPersonOffset(offset);
}

void afxCamera::setThirdPersonDistance(F32 distance) 
{
  mCam_distance = distance;
  mCam_dirty = true;
}

F32 afxCamera::getThirdPersonDistance() 
{
  return mCam_distance;
}

void afxCamera::setThirdPersonAngle(F32 angle) 
{
  mCam_angle = angle;
  mCam_dirty = true;
}

F32 afxCamera::getThirdPersonAngle() 
{
  return mCam_angle;
}

void afxCamera::setThirdPersonMode()
{
  mMode = ThirdPersonMode;
  mFlymode_saved_pos = getPosition();
  mFlymode_saved = true;
  mCam_dirty = true;
  mThird_person_snap_s++;
}

void afxCamera::setThirdPersonSnap()
{
  if (mMode == ThirdPersonMode)
    mThird_person_snap_s += 2;
}

void afxCamera::setThirdPersonSnapClient()
{
  if (mMode == ThirdPersonMode)
    mThird_person_snap_c++;
}

const char* afxCamera::getMode()
{
  switch (mMode)
  {
  case ThirdPersonMode:
    return "ThirdPerson";
  case FlyMode:
    return "Fly";
  case OrbitObjectMode:
    return "Orbit";
  }

  return "Unknown";
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Console Methods

DefineEngineStringlyVariadicMethod(afxCamera, setOrbitMode, void, 7, 8,
  "(GameBase orbitObject, TransformF mat, float minDistance, float maxDistance, float curDistance, bool ownClientObject)"
  "Set the camera to orbit around some given object.\n\n"
  "@param   orbitObject  Object we want to orbit.\n"
  "@param   mat          A set of fields: posX posY posZ aaX aaY aaZ aaTheta\n"
  "@param   minDistance  Minimum distance to keep from object.\n"
  "@param   maxDistance  Maximum distance to keep from object.\n"
  "@param   curDistance  Distance to set initially from object.\n"
  "@param   ownClientObj Are we observing an object owned by us?")
{
  Point3F pos;
  AngAxisF aa;
  F32 minDis, maxDis, curDis;
  
  GameBase *orbitObject = NULL;
  if(Sim::findObject(argv[2],orbitObject) == false)
  {
    Con::warnf("Cannot orbit non-existing object.");
    object->setFlyMode();
    return;
  }
  
  dSscanf(argv[3],"%f %f %f %f %f %f %f",
    &pos.x,&pos.y,&pos.z,&aa.axis.x,&aa.axis.y,&aa.axis.z,&aa.angle);
  minDis = dAtof(argv[4]);
  maxDis = dAtof(argv[5]);
  curDis = dAtof(argv[6]);
  
  object->setOrbitMode(orbitObject, pos, aa, minDis, maxDis, curDis, (argc == 8) ? dAtob(argv[7]) : false);
}

DefineEngineMethod(afxCamera, setFlyMode, void, (),, 
   "@brief Set the camera to be able to fly freely.")
{
   object->setFlyMode();
}

DefineEngineMethod(afxCamera, getPosition, Point3F, (),,
   "@brief Get the position of the camera.\n\n"
   "@returns The position of the camera.")
{
   return object->getPosition();
}

DefineEngineMethod(afxCamera, setCameraSubject, bool, (SceneObject* subject),, "")
{
   if (!subject)
   {
      Con::errorf("Camera subject not found.");
      return false;
   }

   object->setCameraSubject(subject);

   return true;
}

DefineEngineMethod(afxCamera, setThirdPersonDistance, bool, (F32 distance),, "")
{
   object->setThirdPersonDistance(distance);

   return true;
}

DefineEngineMethod(afxCamera, getThirdPersonDistance, F32, (),, "")
{
   return object->getThirdPersonDistance();
}

DefineEngineMethod(afxCamera, setThirdPersonAngle, bool, (F32 distance),, "")
{
   object->setThirdPersonAngle(distance);

   return true;
}

DefineEngineMethod(afxCamera, getThirdPersonAngle, F32, (),, "")
{
   return object->getThirdPersonAngle();
}

DefineEngineMethod(afxCamera, setThirdPersonOffset, void, (Point3F offset, Point3F coi_offset), (Point3F::Max), "")
{
   if (coi_offset == Point3F::Max)
   {
      object->setThirdPersonOffset(offset);
   }
   else
   {
      object->setThirdPersonOffset(offset, coi_offset);
   }
}

DefineEngineMethod(afxCamera, getThirdPersonOffset, Point3F, (),, "")
{
   return object->getThirdPersonOffset();
}

DefineEngineMethod(afxCamera, getThirdPersonCOIOffset, Point3F, (),, "")
{
   return object->getThirdPersonCOIOffset();
}

DefineEngineMethod(afxCamera, setThirdPersonMode, void, (),, "")
{
   object->setThirdPersonMode();
}

DefineEngineMethod(afxCamera, setThirdPersonSnap, void, (),, "")
{
   object->setThirdPersonSnap();
}

DefineEngineMethod(afxCamera, getMode, const char*, (),, "")
{
   return object->getMode();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// 3POV SECTION

void afxCamera::cam_update_3pov(F32 dt, bool on_server) 
{
  Point3F	goal_pos;
  Point3F curr_pos = getRenderPosition();
  MatrixF	xfm = mCam_subject->getRenderTransform();
  Point3F coi = mCam_subject->getRenderPosition() + mCoi_offset;

  // for player subjects, pitch is adjusted
  Player*	player_subj =	dynamic_cast<Player*>(mCam_subject);
  if (player_subj) 
  {
    if (mCam_distance > 0.0f)
    {
      // rotate xfm by amount of cam_angle
      F32	look_yaw = player_subj->getHeadRotation().z + mDegToRad(-mCam_angle);
      MatrixF	look_yaw_mtx(EulerF(0,0,look_yaw));
      xfm.mul(look_yaw_mtx);

      // rotate xfm by amount of head pitch in player
      F32	head_pitch = player_subj->getHeadRotation().x;
      MatrixF	head_pitch_mtx(EulerF(head_pitch,0,0));
      xfm.mul(head_pitch_mtx);

      VectorF	behind_vec(0, -mCam_distance, 0);
      xfm.mulP(behind_vec, &goal_pos);
      goal_pos += mCam_offset;
    }
    else // old backwards-compatible method
    {
      // rotate xfm by amount of head pitch in player
      F32	head_pitch = player_subj->getHeadRotation().x;
      MatrixF	head_pitch_mtx(EulerF(head_pitch,0,0));
      xfm.mul(head_pitch_mtx);

      VectorF	behind_vec(0, mCam_offset.y, 0);
      xfm.mulP(behind_vec, &goal_pos);
      goal_pos.z += mCam_offset.z;
    }
  }
  // for non-player subjects, camera will follow, but pitch won't adjust.
  else 
  {
    xfm.mulP(mCam_offset, &goal_pos);
  }

  // avoid view occlusion
  if (avoid_blocked_view(coi, goal_pos, goal_pos) && !on_server)
  {
    // snap to final position if path to goal is blocked
    if (test_blocked_line(curr_pos, goal_pos))
      mThird_person_snap_c++;
  }

  // place camera into its final position	

  // speed factor values
  //   15 -- tight
  //   10 -- normal
  //    5 -- loose
  //    1 -- very loose
  F32 speed_factor = 8.0f;
  F32 time_inc = 1.0f/speed_factor;

  // snap to final position
  if (on_server || (mThird_person_snap_c > 0 || dt > time_inc))
  {
    snapToPosition(goal_pos);
    if (!on_server && mThird_person_snap_c > 0)
      mThird_person_snap_c--;
    return;
  }
  // interpolate to final position
  else
  {
    // interpretation: always move a proportion of the distance
    // from current location to destination that would cover the
    // entire distance in time_inc duration at constant velocity.
    F32 t = (dt >= time_inc) ? 1.0f : dt*speed_factor;
    snapToPosition(goal_pos*t + curr_pos*(1.0-t));
  }
}

// See if the camera view is occluded by certain objects, 
// and move the camera closer to the subject in that case
bool afxCamera::avoid_blocked_view(const Point3F& startpos, const Point3F& endpos, Point3F& newpos) 
{ 
  // cast ray to check for intersection with potential blocker objects
  RayInfo hit_info;
  if (!getContainer()->castRay(startpos, endpos, afxCameraData::sCameraCollisionMask, &hit_info)) 
  {
    // no hit: just return original endpos
    newpos = endpos;
    return false;
  }

	// did hit: return the hit location nudged forward slightly
  // to avoid seeing clipped portions of blocking object.
	Point3F sight_line = startpos - hit_info.point;
  sight_line.normalize();
  newpos = hit_info.point + sight_line*0.4f;

  return true;
}

bool afxCamera::test_blocked_line(const Point3F& startpos, const Point3F& endpos) 
{ 
  RayInfo hit_info;
  return getContainer()->castRay(startpos, endpos, afxCameraData::sCameraCollisionMask, &hit_info);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

// STD OVERRIDES SECTION

bool afxCamera::onAdd()
{
  if(!Parent::onAdd())
    return false;

  mObjBox.maxExtents = mObjScale;
  mObjBox.minExtents = mObjScale;
  mObjBox.minExtents.neg();

  resetWorldBox();

  addToScene();

  return true;
}

void afxCamera::onRemove()
{
  removeFromScene();
  Parent::onRemove();
}

void afxCamera::onDeleteNotify(SimObject *obj)
{
  Parent::onDeleteNotify(obj);

  if (obj == (SimObject*)mOrbitObject)
  {
    mOrbitObject = NULL;
    if (mMode == OrbitObjectMode)
      mMode = OrbitPointMode;
  }

  if (obj == mCam_subject)
  {
    mCam_subject = NULL;
  }
}

void afxCamera::advanceTime(F32 dt) 
{
  Parent::advanceTime(dt);

  if (gSFX3DWorld)
  {
     if (mMode == ThirdPersonMode && mCam_subject)
     {
        if (gSFX3DWorld->getListener() != mCam_subject)
           gSFX3DWorld->setListener(mCam_subject);
     }
     else if (mMode == FlyMode)
     {
        if (gSFX3DWorld->getListener() != this)
           gSFX3DWorld->setListener(this);
     }
  }

  cam_update(dt, false);
}

void afxCamera::processTick(const Move* move)
{
  Parent::processTick(move);
  Point3F vec,pos;

  // move will be NULL unless camera becomes the control object as in FlyMode
  if (move) 
  {
    // UPDATE ORIENTATION //
	  mDelta.rotVec = mRot;
    mObjToWorld.getColumn(3, &mDelta.posVec);
    mRot.x = mClampF(mRot.x + move->pitch, -MaxPitch, MaxPitch);
    mRot.z += move->yaw;

    // ORBIT MODE // 
    if (mMode == OrbitObjectMode || mMode == OrbitPointMode)
    {
      if(mMode == OrbitObjectMode && bool(mOrbitObject))
      {
        // If this is a shapebase, use its render eye transform
        // to avoid jittering.
        GameBase *castObj = mOrbitObject;
        ShapeBase* shape = dynamic_cast<ShapeBase*>(castObj);
        if( shape != NULL ) {
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
      set_cam_pos(mPosition, mRot);
      validateEyePoint(1.0f, &mObjToWorld);
      pos = mPosition;
    }

    // NON-ORBIT MODE (FLY MODE) //
    else // if (mode == FlyMode)
    {
      // Update pos
      bool faster = move->trigger[0] || move->trigger[1];
      F32 scale = Camera::getMovementSpeed() * (faster + 1);

      mObjToWorld.getColumn(3,&pos);
      mObjToWorld.getColumn(0,&vec);
      pos += vec * move->x * TickSec * scale;
      mObjToWorld.getColumn(1,&vec);
      pos += vec * move->y * TickSec * scale;
      mObjToWorld.getColumn(2,&vec);
      pos += vec * move->z * TickSec * scale;
      set_cam_pos(pos,mRot);
    }

    // If on the client, calc delta for backstepping
    if (isClientObject()) 
    {
      mDelta.pos = pos;
	  mDelta.rot = mRot;
	  mDelta.posVec = mDelta.posVec - mDelta.pos;
	  mDelta.rotVec = mDelta.rotVec - mDelta.rot;
    }
    else
    {
      setMaskBits(MoveMask);
    }
  }
  else // if (!move)
  {
    if (isServerObject())
      cam_update(1.0/32.0, true);
  }

  if (getControllingClient() && mContainer)
    updateContainer();
}

void afxCamera::interpolateTick(F32 dt)
{
  Parent::interpolateTick(dt);

  if (mMode == ThirdPersonMode)
    return;

  Point3F rot = mDelta.rot + mDelta.rotVec * dt;

  if(mMode == OrbitObjectMode || mMode == OrbitPointMode)
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
    set_cam_pos(mPosition, rot);
    validateEyePoint(1.0f, &mObjToWorld);
  }
  else 
  {
    // NOTE - posVec is 0,0,0 unless cam is control-object and process tick is
    // updating the delta
    Point3F pos = mDelta.pos + mDelta.posVec * dt;
    set_cam_pos(pos,rot);
  }
}

void afxCamera::writePacketData(GameConnection *connection, BitStream *bstream)
{
  // Update client regardless of status flags.
  Parent::writePacketData(connection, bstream);

  Point3F pos; mObjToWorld.getColumn(3, &pos);
  bstream->setCompressionPoint(pos);                                      // SET COMPRESSION POINT
  mathWrite(*bstream, pos);                                               // SND POS
  bstream->write(mRot.x);                                                 // SND X ROT
  bstream->write(mRot.z);                                                 // SND Z ROT

  if (bstream->writeFlag(mCam_dirty))
  {
    mathWrite(*bstream, mCam_offset);                                        // SND CAM_OFFSET
    mathWrite(*bstream, mCoi_offset);                                        // SND COI_OFFSET
    bstream->write(mCam_distance);
    bstream->write(mCam_angle);
	mCam_dirty = false;
  }

  U32 writeMode = mMode;
  Point3F writePos = mPosition;
  S32 gIndex = -1;
  if (mMode == OrbitObjectMode)
  {
    gIndex = bool(mOrbitObject) ? connection->getGhostIndex(mOrbitObject): -1;
    if(gIndex == -1)
    {
      writeMode = OrbitPointMode;
      mOrbitObject->getWorldBox().getCenter(&writePos);
    }
  }

  bstream->writeRangedU32(writeMode, CameraFirstMode, CameraLastMode);    // SND MODE
  if (writeMode == ThirdPersonMode)
  {
    bstream->write(mThird_person_snap_s > 0);                              // SND SNAP
    if (mThird_person_snap_s > 0)
      mThird_person_snap_s--;
  }

  if (writeMode == OrbitObjectMode || writeMode == OrbitPointMode)
  {
    bstream->write(mMinOrbitDist);                                        // SND ORBIT MIN DIST
    bstream->write(mMaxOrbitDist);                                        // SND ORBIT MAX DIST
    bstream->write(mCurOrbitDist);                                        // SND ORBIT CURR DIST
    if(writeMode == OrbitObjectMode)
    {
      bstream->writeFlag(mObservingClientObject);                         // SND OBSERVING CLIENT OBJ
      bstream->writeInt(gIndex, NetConnection::GhostIdBitSize);           // SND ORBIT OBJ
    }
    if (writeMode == OrbitPointMode)
      bstream->writeCompressedPoint(writePos);                            // WRITE COMPRESSION POINT
  }
}

void afxCamera::readPacketData(GameConnection *connection, BitStream *bstream)
{
  Parent::readPacketData(connection, bstream);

  Point3F pos,rot;
  mathRead(*bstream, &pos);                                               // RCV POS
  bstream->setCompressionPoint(pos);
  bstream->read(&rot.x);                                                  // RCV X ROT
  bstream->read(&rot.z);                                                  // RCV Z ROT

  if (bstream->readFlag())
  {
    Point3F new_cam_offset, new_coi_offset;
    mathRead(*bstream, &new_cam_offset);                                    // RCV CAM_OFFSET
    mathRead(*bstream, &new_coi_offset);                                    // RCV COI_OFFSET
    bstream->read(&mCam_distance);
    bstream->read(&mCam_angle);
    setThirdPersonOffset(new_cam_offset, new_coi_offset);
  }

  GameBase* obj = 0;
  mMode = bstream->readRangedU32(CameraFirstMode,                          // RCV MODE
    CameraLastMode);
  if (mMode == ThirdPersonMode)
  {
    bool snap; bstream->read(&snap);
    if (snap)
      mThird_person_snap_c++;
  }

  mObservingClientObject = false;
  if (mMode == OrbitObjectMode || mMode == OrbitPointMode) {
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
  if (obj != (GameBase*)mOrbitObject) {
    if (mOrbitObject) {
      clearProcessAfter();
      clearNotify(mOrbitObject);
    }
    mOrbitObject = obj;
    if (mOrbitObject) {
      processAfter(mOrbitObject);
      deleteNotify(mOrbitObject);
    }
  }

  if (mMode == ThirdPersonMode)
    return;

  set_cam_pos(pos,rot);
  mDelta.pos = pos;
  mDelta.rot = rot;
  mDelta.rotVec.set(0,0,0);
  mDelta.posVec.set(0,0,0);
}

U32 afxCamera::packUpdate(NetConnection* conn, U32 mask, BitStream *bstream)
{
  U32 retMask = Parent::packUpdate(conn,mask,bstream);

  // The rest of the data is part of the control object packet update.
  // If we're controlled by this client, we don't need to send it.
  //if(bstream->writeFlag(getControllingClient() == conn && !(mask & InitialUpdateMask)))
  //   return 0;

  if (bstream->writeFlag(mask & MoveMask)) {
    Point3F pos;
    mObjToWorld.getColumn(3,&pos);
    bstream->write(pos.x);
    bstream->write(pos.y);
    bstream->write(pos.z);
    bstream->write(mRot.x);
    bstream->write(mRot.z);
  }

  if (bstream->writeFlag(mask & SubjectMask)) 
  {
    S32 ghost_id = (mCam_subject) ? conn->getGhostIndex(mCam_subject) : -1;
    if (bstream->writeFlag(ghost_id != -1))
      bstream->writeRangedU32(U32(ghost_id), 0, NetConnection::MaxGhostCount);
    else if (mCam_subject)
      retMask |= SubjectMask;
  }

  return retMask;
}

void afxCamera::unpackUpdate(NetConnection *conn, BitStream *bstream)
{
  Parent::unpackUpdate(conn,bstream);

  // controlled by the client?
  //if(bstream->readFlag())
  //   return;

  if (bstream->readFlag()) {
    Point3F pos,rot;
    bstream->read(&pos.x);
    bstream->read(&pos.y);
    bstream->read(&pos.z);
    bstream->read(&rot.x);
    bstream->read(&rot.z);
    set_cam_pos(pos,rot);

    // New delta for client side interpolation
	mDelta.pos = pos;
	mDelta.rot = rot;
	mDelta.posVec = mDelta.rotVec = VectorF(0,0,0);
  }

  if (bstream->readFlag()) 
  {
    if (bstream->readFlag())
    {
      S32 ghost_id = bstream->readRangedU32(0, NetConnection::MaxGhostCount);
	  mCam_subject = dynamic_cast<GameBase*>(conn->resolveGhost(ghost_id));
    }
    else
      mCam_subject = NULL;
  }
}

// Override to ensure both are kept in scope
void afxCamera::onCameraScopeQuery(NetConnection* conn, CameraScopeQuery* query) 
{
  if (mCam_subject)
    conn->objectInScope(mCam_subject);
  Parent::onCameraScopeQuery(conn, query);
}

//----------------------------------------------------------------------------
// check if the object needs to be observed through its own camera...
void afxCamera::getCameraTransform(F32* pos, MatrixF* mat)
{
  // The camera doesn't support a third person mode,
  // so we want to override the default ShapeBase behavior.
  ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
  if (obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
    obj->getCameraTransform(pos, mat);
  else
    getEyeTransform(mat);
}

void afxCamera::setTransform(const MatrixF& mat)
{
  // This method should never be called on the client.

  // This currently converts all rotation in the mat into
  // rotations around the z and x axis.
  Point3F pos,vec;
  mat.getColumn(1,&vec);
  mat.getColumn(3,&pos);
  Point3F rot(-mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)),0,-mAtan2(-vec.x,vec.y));
  set_cam_pos(pos,rot);
}

void afxCamera::onEditorEnable()
{
  mNetFlags.set(Ghostable);
}

void afxCamera::onEditorDisable()
{
  mNetFlags.clear(Ghostable);
}

F32 afxCamera::getCameraFov()
{
  ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
  if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
    return(obj->getCameraFov());
  else
    return(Parent::getCameraFov());
}

F32 afxCamera::getDefaultCameraFov()
{
  ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
  if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
    return(obj->getDefaultCameraFov());
  else
    return(Parent::getDefaultCameraFov());
}

bool afxCamera::isValidCameraFov(F32 fov)
{
  ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
  if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
    return(obj->isValidCameraFov(fov));
  else
    return(Parent::isValidCameraFov(fov));
}

void afxCamera::setCameraFov(F32 fov)
{
  ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mOrbitObject));
  if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
    obj->setCameraFov(fov);
  else
    Parent::setCameraFov(fov);
}

F32 afxCamera::getDamageFlash() const
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

F32 afxCamera::getWhiteOut() const
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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxCamera::setControllingClient( GameConnection* client )
{
   GameBase::setControllingClient( client );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
