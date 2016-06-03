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
#include "T3D/entity.h"
#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "sim/netConnection.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameProcess.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "math/mTransform.h"

#include "T3D/components/coreInterfaces.h"
#include "T3D/components/render/renderComponentInterface.h"
#include "T3D/components/collision/collisionInterfaces.h"

#include "gui/controls/guiTreeViewCtrl.h"
#include "assets/assetManager.h"
#include "assets/assetQuery.h"
#include "T3D/assets/ComponentAsset.h"

#include "console/consoleInternal.h"
#include "T3D/gameBase/std/stdMoveList.h"

#include "T3D/prefab.h"

//
#include "gfx/sim/debugDraw.h"
//

extern bool gEditingMission;

// Client prediction
static F32 sMinWarpTicks = 0.5f;       // Fraction of tick at which instant warp occurs
static S32 sMaxWarpTicks = 3;          // Max warp duration in ticks
static S32 sMaxPredictionTicks = 30;   // Number of ticks to predict

IMPLEMENT_CO_NETOBJECT_V1(Entity);

ConsoleDocClass(Entity,
   "@brief Base Entity class.\n\n"

   "Entity is typically made up of a shape and up to two particle emitters.  In most cases Entity objects are "
   "not created directly.  They are usually produced automatically by other means, such as through the Explosion "
   "class.  When an explosion goes off, its ExplosionData datablock determines what Entity to emit.\n"

   "@tsexample\n"
   "datablock ExplosionData(GrenadeLauncherExplosion)\n"
   "{\n"
   "   // Assiging Entity data\n"
   "   Entity = GrenadeEntity;\n\n"
   "   // Adjust how Entity is ejected\n"
   "   EntityThetaMin = 10;\n"
   "   EntityThetaMax = 60;\n"
   "   EntityNum = 4;\n"
   "   EntityNumVariance = 2;\n"
   "   EntityVelocity = 25;\n"
   "   EntityVelocityVariance = 5;\n\n"
   "   // Note: other ExplosionData properties are not listed for this example\n"
   "};\n"
   "@endtsexample\n\n"

   "@note Entity are client side only objects.\n"

   "@see EntityData\n"
   "@see ExplosionData\n"
   "@see Explosion\n"

   "@ingroup FX\n"
   );

Entity::Entity()
{
   //mTypeMask |= DynamicShapeObjectType | StaticObjectType | ;
   mTypeMask |= EntityObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);

   mPos = Point3F(0, 0, 0);
   mRot = Point3F(0, 0, 0);

   mDelta.pos = mDelta.posVec = Point3F::Zero;
   mDelta.rot[0].identity();
   mDelta.rot[1].identity();
   mDelta.warpOffset.set(0.0f, 0.0f, 0.0f);

   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.dt = 1.0f;
   mDelta.move = NullMove;

   mComponents.clear();

   mStartComponentUpdate = false;

   mInitialized = false;

}

Entity::~Entity()
{

}

void Entity::initPersistFields()
{
   Parent::initPersistFields();

   removeField("DataBlock");

   addGroup("Transform");

   removeField("Position");
   addProtectedField("Position", TypePoint3F, Offset(mPos, Entity), &_setPosition, &_getPosition, "Object world orientation.");

   removeField("Rotation");
   addProtectedField("Rotation", TypeRotationF, Offset(mRot, Entity), &_setRotation, &_getRotation, "Object world orientation.");

   //These are basically renamed mountPos/Rot. pretty much there for conveinence
   addField("LocalPosition", TypeMatrixPosition, Offset(mMount.xfm, Entity), "Position we are mounted at ( object space of our mount object ).");
   addField("LocalRotation", TypeMatrixRotation, Offset(mMount.xfm, Entity), "Rotation we are mounted at ( object space of our mount object ).");

   endGroup("Transform");
}

//
bool Entity::_setPosition(void *object, const char *index, const char *data)
{
   Entity* so = static_cast<Entity*>(object);
   if (so)
   {
      Point3F pos;

      if (!dStrcmp(data, ""))
         pos = Point3F(0, 0, 0);
      else
         Con::setData(TypePoint3F, &pos, 0, 1, &data);

      so->setTransform(pos, so->mRot);
   }
   return false;
}

const char * Entity::_getPosition(void* obj, const char* data)
{
   Entity* so = static_cast<Entity*>(obj);
   if (so)
   {
      Point3F pos = so->getPosition();

      static const U32 bufSize = 256;
      char* returnBuffer = Con::getReturnBuffer(bufSize);
      dSprintf(returnBuffer, bufSize, "%g %g %g", pos.x, pos.y, pos.z);
      return returnBuffer;
   }
   return "0 0 0";
}

bool Entity::_setRotation(void *object, const char *index, const char *data)
{
   Entity* so = static_cast<Entity*>(object);
   if (so)
   {
      RotationF rot;
      Con::setData(TypeRotationF, &rot, 0, 1, &data);

      //so->mRot = rot;
      //MatrixF mat = rot.asMatrixF();
      //mat.setPosition(so->getPosition());
      //so->setTransform(mat);
      so->setTransform(so->getPosition(), rot);
   }
   return false;
}

const char * Entity::_getRotation(void* obj, const char* data)
{
   Entity* so = static_cast<Entity*>(obj);
   if (so)
   {
      EulerF eulRot = so->mRot.asEulerF();

      static const U32 bufSize = 256;
      char* returnBuffer = Con::getReturnBuffer(bufSize);
      dSprintf(returnBuffer, bufSize, "%g %g %g", mRadToDeg(eulRot.x), mRadToDeg(eulRot.y), mRadToDeg(eulRot.z));
      return returnBuffer;
   }
   return "0 0 0";
}

bool Entity::onAdd()
{
   if (!Parent::onAdd())
      return false;

   mObjBox = Box3F(Point3F(-1, -1, -1), Point3F(1, 1, 1));

   resetWorldBox();
   setObjectBox(mObjBox);

   addToScene();

   //Make sure we get positioned
   setMaskBits(TransformMask);

   return true;
}

void Entity::onRemove()
{
   clearComponents(true);

   removeFromScene();

   onDataSet.removeAll();

   Parent::onRemove();
}

void Entity::onPostAdd()
{
   mInitialized = true;

   //everything's done and added. go ahead and initialize the components
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      mComponents[i]->onComponentAdd();
   }

   if (isMethod("onAdd"))
      Con::executef(this, "onAdd");
}

void Entity::setDataField(StringTableEntry slotName, const char *array, const char *value)
{
   Parent::setDataField(slotName, array, value);

   onDataSet.trigger(this, slotName, value);
}

void Entity::onStaticModified(const char* slotName, const char* newValue)
{
   Parent::onStaticModified(slotName, newValue);

   onDataSet.trigger(this, slotName, newValue);
}

//Updating
void Entity::processTick(const Move* move)
{
   if (!isHidden())
   {
      if (mDelta.warpCount < mDelta.warpTicks)
      {
         mDelta.warpCount++;

         // Set new pos.
         mObjToWorld.getColumn(3, &mDelta.pos);
         mDelta.pos += mDelta.warpOffset;
         mDelta.rot[0] = mDelta.rot[1];
         mDelta.rot[1].interpolate(mDelta.warpRot[0], mDelta.warpRot[1], F32(mDelta.warpCount) / mDelta.warpTicks);
         setTransform(mDelta.pos, mDelta.rot[1]);

         // Pos backstepping
         mDelta.posVec.x = -mDelta.warpOffset.x;
         mDelta.posVec.y = -mDelta.warpOffset.y;
         mDelta.posVec.z = -mDelta.warpOffset.z;
      }
      else
      {
         if (isMounted())
         {
            MatrixF mat;
            mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat);
            Parent::setTransform(mat);
            Parent::setRenderTransform(mat);
         }
         else
         {
            if (!move)
            {
               if (isGhost())
               {
                  // If we haven't run out of prediction time,
                  // predict using the last known move.
                  if (mPredictionCount-- <= 0)
                     return;

                  move = &mDelta.move;
               }
               else
               {
                  move = &NullMove;
               }
            }
         }
      }

      Move prevMove = lastMove;

      if (move != NULL)
         lastMove = *move;
      else
         lastMove = NullMove;

      if (move && isServerObject())
      {
         if ((move->y != 0 || prevMove.y != 0) 
            || (move->x != 0 || prevMove.x != 0) 
            || (move->z != 0 || prevMove.x != 0))
         {
            if (isMethod("moveVectorEvent"))
               Con::executef(this, "moveVectorEvent", move->x, move->y, move->z);
         }

         if (move->yaw != 0)
         {
            if (isMethod("moveYawEvent"))
               Con::executef(this, "moveYawEvent", move->yaw);
         }

         if (move->pitch != 0)
         {
            if (isMethod("movePitchEvent"))
               Con::executef(this, "movePitchEvent", move->pitch);
         }

         if (move->roll != 0)
         {
            if (isMethod("moveRollEvent"))
               Con::executef(this, "moveRollEvent", move->roll);
         }

         for (U32 i = 0; i < MaxTriggerKeys; i++)
         {
            if (move->trigger[i] != prevMove.trigger[i])
            {
               if (isMethod("moveTriggerEvent"))
                  Con::executef(this, "moveTriggerEvent", i, move->trigger[i]);
            }
         }
      }

      if (isMethod("processTick"))
         Con::executef(this, "processTick");
   }
}

void Entity::advanceTime(F32 dt)
{
}

void Entity::interpolateTick(F32 dt)
{
   if (dt == 0.0f)
   {
      setRenderTransform(mDelta.pos, mDelta.rot[1]);
   }
   else
   {
      QuatF rot;
      rot.interpolate(mDelta.rot[1], mDelta.rot[0], dt);
      Point3F pos = mDelta.pos + mDelta.posVec * dt;

      setRenderTransform(pos, rot);
   }

   mDelta.dt = dt;
}

//Render
void Entity::prepRenderImage(SceneRenderState *state)
{
}

//Networking
U32 Entity::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & TransformMask))
   {
      //mathWrite( *stream, getScale() );
      //stream->writeAffineTransform(mObjToWorld);
      //mathWrite(*stream, getPosition());
      //mathWrite(*stream, mPos);

      stream->writeCompressedPoint(mPos);
      mathWrite(*stream, getRotation());

      mDelta.move.pack(stream);

      stream->writeFlag(!(mask & NoWarpMask));
   }

   /*if (stream->writeFlag(mask & MountedMask))
   {
      mathWrite(*stream, mMount.xfm.getPosition());
      mathWrite(*stream, mMount.xfm.toEuler());
   }*/

   if (stream->writeFlag(mask & BoundsMask))
   {
      mathWrite(*stream, mObjBox);
   }

   //pass our behaviors around
   if (mask & ComponentsMask || mask & InitialUpdateMask)
   {
      stream->writeFlag(true);
      //now, we run through a list of our to-be-sent behaviors and begin sending them
      //if any fail, we keep our list and re-queue the mask
      S32 componentCount = mToLoadComponents.size();

      //build our 'ready' list
      //This requires both the instance and the instances' template to be prepped(if the template hasn't been ghosted,
      //then we know we shouldn't be passing the instance's ghosts around yet)
      U32 ghostedCompCnt = 0;
      for (U32 i = 0; i < componentCount; i++)
      {
         if (con->getGhostIndex(mToLoadComponents[i]) != -1)
            ghostedCompCnt++;
      }

      if (ghostedCompCnt != 0)
      {
         stream->writeFlag(true);

         stream->writeFlag(mStartComponentUpdate);

         //if not all the behaviors have been ghosted, we'll need another pass
         if (ghostedCompCnt != componentCount)
            retMask |= ComponentsMask;

         //write the currently ghosted behavior count
         stream->writeInt(ghostedCompCnt, 16);

         for (U32 i = 0; i < mToLoadComponents.size(); i++)
         {
            //now fetch them and pass the ghost
            S32 ghostIndex = con->getGhostIndex(mToLoadComponents[i]);
            if (ghostIndex != -1)
            {
               stream->writeInt(ghostIndex, NetConnection::GhostIdBitSize);
               mToLoadComponents.erase(i);
               i--;

               mStartComponentUpdate = false;
            }
         }
      }
      else if (componentCount)
      {
         //on the odd chance we have behaviors to ghost, but NONE of them have been yet, just set the flag now
         stream->writeFlag(false);
         retMask |= ComponentsMask;
      }
      else
         stream->writeFlag(false);
   }
   else
      stream->writeFlag(false);

   return retMask;
}

void Entity::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      /*Point3F scale;
      mathRead( *stream, &scale );
      setScale( scale);*/

      //MatrixF objToWorld;
      //stream->readAffineTransform(&objToWorld);

      Point3F pos;

      stream->readCompressedPoint(&pos);
      //mathRead(*stream, &pos);

      RotationF rot;

      mathRead(*stream, &rot);

      mDelta.move.unpack(stream);

      if (stream->readFlag() && isProperlyAdded())
      {
         // Determine number of ticks to warp based on the average
         // of the client and server velocities.
         /*mDelta.warpOffset = pos - mDelta.pos;

         F32 dt = mDelta.warpOffset.len() / (0.5f * TickSec);

         mDelta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5f), 1.0f) : 0.0f);

         //F32 as = (speed + mVelocity.len()) * 0.5f * TickSec;
         //F32 dt = (as > 0.00001f) ? mDelta.warpOffset.len() / as : sMaxWarpTicks;
         //mDelta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5f), 1.0f) : 0.0f);

         //mDelta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5f), 1.0f) : 0.0f);

         //mDelta.warpTicks = sMaxWarpTicks;

         mDelta.warpTicks = 0;

         if (mDelta.warpTicks)
         {
            // Setup the warp to start on the next tick.
            if (mDelta.warpTicks > sMaxWarpTicks)
               mDelta.warpTicks = sMaxWarpTicks;
            mDelta.warpOffset /= (F32)mDelta.warpTicks;

            mDelta.rot[0] = rot.asQuatF();
            mDelta.rot[1] = rot.asQuatF();

            mDelta.rotOffset = rot.asEulerF() - mDelta.rot.asEulerF();

            // Ignore small rotation differences
            if (mFabs(mDelta.rotOffset.x) < 0.001f)
               mDelta.rotOffset.x = 0;

            if (mFabs(mDelta.rotOffset.y) < 0.001f)
               mDelta.rotOffset.y = 0;

            if (mFabs(mDelta.rotOffset.z) < 0.001f)
               mDelta.rotOffset.z = 0;

            mDelta.rotOffset /= (F32)mDelta.warpTicks;
         }
         else
         {
            // Going to skip the warp, server and client are real close.
            // Adjust the frame interpolation to move smoothly to the
            // new position within the current tick.
            Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
            if (mDelta.dt == 0)
            {
               mDelta.posVec.set(0.0f, 0.0f, 0.0f);
               mDelta.rotVec.set(0.0f, 0.0f, 0.0f);
            }
            else
            {
               F32 dti = 1.0f / mDelta.dt;
               mDelta.posVec = (cp - pos) * dti;
               mDelta.rotVec.z = mRot.z - rot.z;

               mDelta.rotVec.z *= dti;
            }

            mDelta.pos = pos;
            mDelta.rot = rot;

            setTransform(pos, rot);
         }*/

         Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
         mDelta.warpOffset = pos - cp;

         // Calc the distance covered in one tick as the average of
         // the old speed and the new speed from the server.
         VectorF vel = pos - mDelta.pos;
         F32 dt, as = vel.len() * 0.5 * TickSec;

         // Cal how many ticks it will take to cover the warp offset.
         // If it's less than what's left in the current tick, we'll just
         // warp in the remaining time.
         if (!as || (dt = mDelta.warpOffset.len() / as) > sMaxWarpTicks)
            dt = mDelta.dt + sMaxWarpTicks;
         else
            dt = (dt <= mDelta.dt) ? mDelta.dt : mCeil(dt - mDelta.dt) + mDelta.dt;

         // Adjust current frame interpolation
         if (mDelta.dt)
         {
            mDelta.pos = cp + (mDelta.warpOffset * (mDelta.dt / dt));
            mDelta.posVec = (cp - mDelta.pos) / mDelta.dt;
            QuatF cr;
            cr.interpolate(mDelta.rot[1], mDelta.rot[0], mDelta.dt);

            mDelta.rot[1].interpolate(cr, rot.asQuatF(), mDelta.dt / dt);
            mDelta.rot[0].extrapolate(mDelta.rot[1], cr, mDelta.dt);
         }

         // Calculated multi-tick warp
         mDelta.warpCount = 0;
         mDelta.warpTicks = (S32)(mFloor(dt));
         if (mDelta.warpTicks)
         {
            mDelta.warpOffset = pos - mDelta.pos;
            mDelta.warpOffset /= mDelta.warpTicks;
            mDelta.warpRot[0] = mDelta.rot[1];
            mDelta.warpRot[1] = rot.asQuatF();
         }
      }
      else
      {
         // Set the entity to the server position
         mDelta.dt = 0;
         mDelta.pos = pos;
         mDelta.posVec.set(0, 0, 0);
         mDelta.rot[1] = mDelta.rot[0] = rot.asQuatF();
         mDelta.warpCount = mDelta.warpTicks = 0;
         setTransform(pos, rot);
      }
   }

   /*if (stream->readFlag())
   {
      Point3F mountOffset;
      EulerF mountRot;
      mathRead(*stream, &mountOffset);
      mathRead(*stream, &mountRot);

      RotationF rot = RotationF(mountRot);
      mountRot = rot.asEulerF(RotationF::Degrees);

      setMountOffset(mountOffset);
      setMountRotation(mountRot);
   }*/

   if (stream->readFlag())
   {
      mathRead(*stream, &mObjBox);
      resetWorldBox();
   }

   if (stream->readFlag())
   {
      //are we passing any behaviors currently?
      if (stream->readFlag())
      {
         //if we've just started the update, clear our behaviors
         if (stream->readFlag())
            clearComponents(false);

         S32 componentCount = stream->readInt(16);

         for (U32 i = 0; i < componentCount; i++)
         {
            S32 gIndex = stream->readInt(NetConnection::GhostIdBitSize);
            addComponent(dynamic_cast<Component*>(con->resolveGhost(gIndex)));
         }
      }
   }
}

//Manipulation
void Entity::setTransform(const MatrixF &mat)
{
   //setMaskBits(TransformMask);
   setMaskBits(TransformMask | NoWarpMask);

   if (isMounted())
   {
      // Use transform from mounted object
      Point3F newPos = mat.getPosition();
      Point3F parentPos = mMount.object->getTransform().getPosition();

      Point3F newOffset = newPos - parentPos;

      if (!newOffset.isZero())
      {
         //setMountOffset(newOffset);
         mPos = newOffset;
      }

      Point3F matEul = mat.toEuler();

      //mRot = Point3F(mRadToDeg(matEul.x), mRadToDeg(matEul.y), mRadToDeg(matEul.z));

      if (matEul != Point3F(0, 0, 0))
      {
         Point3F mountEul = mMount.object->getTransform().toEuler();
         Point3F diff = matEul - mountEul;

         //setMountRotation(Point3F(mRadToDeg(diff.x), mRadToDeg(diff.y), mRadToDeg(diff.z)));
         mRot = diff;
      }
      else
      {
         //setMountRotation(Point3F(0, 0, 0));
         mRot = Point3F(0, 0, 0);
      }

      RotationF addRot = mRot + RotationF(mMount.object->getTransform());
      MatrixF transf = addRot.asMatrixF();
      transf.setPosition(mPos + mMount.object->getPosition());

      Parent::setTransform(transf);
   }
   else
   {
      //Are we part of a prefab?
      /*Prefab* p = Prefab::getPrefabByChild(this);
      if (p)
      {
         //just let our prefab know we moved
         p->childTransformUpdated(this, mat);
      }*/
      //else
      {
         //mRot.set(mat);
         //Parent::setTransform(mat);

         RotationF rot = RotationF(mat);

         EulerF tempRot = rot.asEulerF(RotationF::Degrees);

         Point3F pos;

         mat.getColumn(3,&pos);

         setTransform(pos, rot);
      }
   }
}

void Entity::setTransform(Point3F position, RotationF rotation)
{
   if (isMounted())
   {
      mPos = position;
      mRot = rotation;

      RotationF addRot = mRot + RotationF(mMount.object->getTransform());
      MatrixF transf = addRot.asMatrixF();
      transf.setPosition(mPos + mMount.object->getPosition());

      Parent::setTransform(transf);

      setMaskBits(TransformMask);
   }
   else
   {
      /*MatrixF newMat, imat, xmat, ymat, zmat;
      Point3F radRot = Point3F(mDegToRad(rotation.x), mDegToRad(rotation.y), mDegToRad(rotation.z));
      xmat.set(EulerF(radRot.x, 0, 0));
      ymat.set(EulerF(0.0f, radRot.y, 0.0f));
      zmat.set(EulerF(0, 0, radRot.z));
      imat.mul(zmat, xmat);
      newMat.mul(imat, ymat);*/

      MatrixF newMat = rotation.asMatrixF();

      newMat.setColumn(3, position);

      mPos = position;
      mRot = rotation;

      setMaskBits(TransformMask);
      //if (isServerObject())
      //   setMaskBits(TransformMask);

      //setTransform(temp);

      // This test is a bit expensive so turn it off in release.   
#ifdef TORQUE_DEBUG
      //AssertFatal( mat.isAffine(), "SceneObject::setTransform() - Bad transform (non affine)!" );
#endif

      //PROFILE_SCOPE(Entity_setTransform);

      // Update the transforms.

      Parent::setTransform(newMat);

      onTransformSet.trigger(&newMat);

      /*mObjToWorld = mWorldToObj = newMat;
      mWorldToObj.affineInverse();
      // Update the world-space AABB.
      resetWorldBox();
      // If we're in a SceneManager, sync our scene state.
      if (mSceneManager != NULL)
      mSceneManager->notifyObjectDirty(this);
      setRenderTransform(newMat);*/
   }
}

void Entity::setRenderTransform(const MatrixF &mat)
{
   Parent::setRenderTransform(mat);
}

void Entity::setRenderTransform(Point3F position, RotationF rotation)
{
   if (isMounted())
   {
      mPos = position;
      mRot = rotation;

      RotationF addRot = mRot + RotationF(mMount.object->getTransform());
      MatrixF transf = addRot.asMatrixF();
      transf.setPosition(mPos + mMount.object->getPosition());

      Parent::setRenderTransform(transf);
   }
   else
   {
      MatrixF newMat = rotation.asMatrixF();

      newMat.setColumn(3, position);

      mPos = position;
      mRot = rotation;

      Parent::setRenderTransform(newMat);

      onTransformSet.trigger(&newMat);
   }
}

MatrixF Entity::getTransform()
{
   if (isMounted())
   {
      MatrixF mat;

      //Use transform from mount
      mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat);

      Point3F transPos = mat.getPosition() + mPos;

      mat.mul(mRot.asMatrixF());

      mat.setPosition(transPos);

      return mat;
   }
   else
   {
      return Parent::getTransform();
   }
}

void Entity::setMountOffset(Point3F posOffset)
{
   if (isMounted())
   {
      mMount.xfm.setColumn(3, posOffset);
      //mPos = posOffset;
      setMaskBits(MountedMask);
   }
}

void Entity::setMountRotation(EulerF rotOffset)
{
   if (isMounted())
   {
      MatrixF temp, imat, xmat, ymat, zmat;

      Point3F radRot = Point3F(mDegToRad(rotOffset.x), mDegToRad(rotOffset.y), mDegToRad(rotOffset.z));
      xmat.set(EulerF(radRot.x, 0, 0));
      ymat.set(EulerF(0.0f, radRot.y, 0.0f));
      zmat.set(EulerF(0, 0, radRot.z));

      imat.mul(zmat, xmat);
      temp.mul(imat, ymat);

      temp.setColumn(3, mMount.xfm.getPosition());

      mMount.xfm = temp;
      //mRot = RotationF(temp);
      setMaskBits(MountedMask);
   }
}
//
void Entity::getCameraTransform(F32* pos, MatrixF* mat)
{
   Vector<CameraInterface*> updaters = getComponents<CameraInterface>();
   for (Vector<CameraInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++) 
   {
      if ((*it)->getCameraTransform(pos, mat)) 
      {
         return;
      }
   }
}

void Entity::getMountTransform(S32 index, const MatrixF &xfm, MatrixF *outMat)
{
   RenderComponentInterface* renderInterface = getComponent<RenderComponentInterface>();

   if (renderInterface)
   {
      renderInterface->getShapeInstance()->animate();
      S32 nodeCount = renderInterface->getShapeInstance()->getShape()->nodes.size();

      if (index >= 0 && index < nodeCount)
      {
         MatrixF mountTransform = renderInterface->getShapeInstance()->mNodeTransforms[index];
         mountTransform.mul(xfm);
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve(scale);
         mountTransform.setPosition(position);

         // Also we would like the object to be scaled to the model.
         outMat->mul(mObjToWorld, mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   Parent::getMountTransform(index, xfm, outMat);
}

void Entity::getRenderMountTransform(F32 delta, S32 index, const MatrixF &xfm, MatrixF *outMat)
{
   RenderComponentInterface* renderInterface = getComponent<RenderComponentInterface>();

   if (renderInterface && renderInterface->getShapeInstance())
   {
      renderInterface->getShapeInstance()->animate();
      S32 nodeCount = renderInterface->getShapeInstance()->getShape()->nodes.size();

      if (index >= 0 && index < nodeCount)
      {
         MatrixF mountTransform = renderInterface->getShapeInstance()->mNodeTransforms[index];
         mountTransform.mul(xfm);
         const Point3F& scale = getScale();

         // The position of the mount point needs to be scaled.
         Point3F position = mountTransform.getPosition();
         position.convolve(scale);
         mountTransform.setPosition(position);

         // Also we would like the object to be scaled to the model.
         outMat->mul(getRenderTransform(), mountTransform);
         return;
      }
   }

   // Then let SceneObject handle it.
   Parent::getMountTransform(index, xfm, outMat);
}

void Entity::setForwardVector(VectorF newForward, VectorF upVector)
{
   MatrixF mat = getTransform();

   VectorF up(0.0f, 0.0f, 1.0f);
   VectorF axisX;
   VectorF axisY = newForward;
   VectorF axisZ;

   if (upVector != VectorF::Zero)
      up = upVector;

   // Validate and normalize input:  
   F32 lenSq;
   lenSq = axisY.lenSquared();
   if (lenSq < 0.000001f)
   {
      axisY.set(0.0f, 1.0f, 0.0f);
      Con::errorf("Entity::setForwardVector() - degenerate forward vector");
   }
   else
   {
      axisY /= mSqrt(lenSq);
   }


   lenSq = up.lenSquared();
   if (lenSq < 0.000001f)
   {
      up.set(0.0f, 0.0f, 1.0f);
      Con::errorf("SceneObject::setForwardVector() - degenerate up vector - too small");
   }
   else
   {
      up /= mSqrt(lenSq);
   }

   if (fabsf(mDot(up, axisY)) > 0.9999f)
   {
      Con::errorf("SceneObject::setForwardVector() - degenerate up vector - same as forward");
      // i haven't really tested this, but i think it generates something which should be not parallel to the previous vector:  
      F32 tmp = up.x;
      up.x = -up.y;
      up.y = up.z;
      up.z = tmp;
   }

   // construct the remaining axes:  
   mCross(axisY, up, &axisX);
   mCross(axisX, axisY, &axisZ);

   mat.setColumn(0, axisX);
   mat.setColumn(1, axisY);
   mat.setColumn(2, axisZ);

   setTransform(mat);
}
//
//These basically just redirect to any collision behaviors we have
bool Entity::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   Vector<CastRayInterface*> updaters = getComponents<CastRayInterface>();
   for (Vector<CastRayInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++)
   {
      if ((*it)->castRay(start, end, info))
      {
         return true;
      }
   }
   return false;
}

bool Entity::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
{
   Vector<CastRayRenderedInterface*> updaters = getComponents<CastRayRenderedInterface>();
   for (Vector<CastRayRenderedInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++)
   {
      if ((*it)->castRayRendered(start, end, info))
      {
         return true;
      }
   }
   return false;
}

bool Entity::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere)
{
   Vector<BuildPolyListInterface*> updaters = getComponents<BuildPolyListInterface>();
   for (Vector<BuildPolyListInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++)
   {
      return (*it)->buildPolyList(context, polyList, box, sphere);
   }

   return false;
}

void Entity::buildConvex(const Box3F& box, Convex* convex)
{
   Vector<BuildConvexInterface*> updaters = getComponents<BuildConvexInterface>();
   for (Vector<BuildConvexInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++)
   {
      (*it)->buildConvex(box, convex);
   }
}

//
// Mounting and heirarchy manipulation
void Entity::mountObject(SceneObject* objB, MatrixF txfm)
{
   Parent::mountObject(objB, -1, txfm);
   Parent::addObject(objB);
}

void Entity::mountObject(SceneObject *obj, S32 node, const MatrixF &xfm)
{
   Parent::mountObject(obj, node, xfm);
}

void Entity::onMount(SceneObject *obj, S32 node)
{
   deleteNotify(obj);

   // Are we mounting to a GameBase object?
   Entity *entityObj = dynamic_cast<Entity*>(obj);

   if (entityObj && entityObj->getControlObject() != this)
      processAfter(entityObj);

   if (!isGhost()) {
      setMaskBits(MountedMask);

      //TODO implement this callback
      //onMount_callback( this, obj, node );
   }
}

void Entity::onUnmount(SceneObject *obj, S32 node)
{
   clearNotify(obj);

   Entity *entityObj = dynamic_cast<Entity*>(obj);

   if (entityObj && entityObj->getControlObject() != this)
      clearProcessAfter();

   if (!isGhost()) {
      setMaskBits(MountedMask);

      //TODO implement this callback
      //onUnmount_callback( this, obj, node );
   }
}

//Heirarchy stuff
void Entity::addObject(SimObject* object)
{
   Component* component = dynamic_cast<Component*>(object);
   if (component)
   {
      addComponent(component);
      return;
   }

   Entity* e = dynamic_cast<Entity*>(object);
   if (e)
   {
      MatrixF offset;

      //offset.mul(getWorldTransform(), e->getWorldTransform());

      //check if we're mounting to a node on a shape we have
      String node = e->getDataField("mountNode", NULL);
      if (!node.isEmpty())
      {
         RenderComponentInterface *renderInterface = getComponent<RenderComponentInterface>();
         if (renderInterface)
         {
            TSShape* shape = renderInterface->getShape();
            S32 nodeIdx = shape->findNode(node);

            mountObject(e, nodeIdx, MatrixF::Identity);
         }
         else
         {
            mountObject(e, MatrixF::Identity);
         }
      }
      else
      {
         /*Point3F posOffset = mPos - e->getPosition();
         mPos = posOffset;

         RotationF rotOffset = mRot - e->getRotation();
         mRot = rotOffset;
         setMaskBits(TransformMask);
         mountObject(e, MatrixF::Identity);*/

         mountObject(e, MatrixF::Identity);
      }

      //e->setMountOffset(e->getPosition() - getPosition());

      //Point3F diff = getWorldTransform().toEuler() - e->getWorldTransform().toEuler();

      //e->setMountRotation(Point3F(mRadToDeg(diff.x),mRadToDeg(diff.y),mRadToDeg(diff.z)));

      //mountObject(e, offset);
   }
   else
   {
      SceneObject* so = dynamic_cast<SceneObject*>(object);
      if (so)
      {
         //get the difference and build it as our offset!
         Point3F posOffset = so->getPosition() - mPos;
         RotationF rotOffset = RotationF(so->getTransform()) - mRot;

         MatrixF offset = rotOffset.asMatrixF();
         offset.setPosition(posOffset);

         mountObject(so, offset);
         return;
      }
   }

   Parent::addObject(object);
}

void Entity::removeObject(SimObject* object)
{
   Entity* e = dynamic_cast<Entity*>(object);
   if (e)
   {
      mPos = mPos + e->getPosition();
      mRot = mRot + e->getRotation();
      unmountObject(e);
      setMaskBits(TransformMask);
   }
   else
   {
      SceneObject* so = dynamic_cast<SceneObject*>(object);
      if (so)
         unmountObject(so);
   }

   Parent::removeObject(object);
}

bool Entity::addComponent(Component *comp)
{
   if (comp == NULL)
      return false;

   //double-check were not re-adding anything
   mComponents.push_back(comp);

   // Register the component with this owner.
   comp->setOwner(this);

   //if we've already been added and this is being added after the fact(at runtime), 
   //then just go ahead and call it's onComponentAdd so it can get to work
   if (mInitialized)
      comp->onComponentAdd();

   onComponentAdded.trigger(comp);

   return true;
}

SimObject* Entity::findObjectByInternalName(StringTableEntry internalName, bool searchChildren)
{
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      if (mComponents[i]->getInternalName() == internalName)
      {
         return mComponents[i];
      }
   }

   return Parent::findObjectByInternalName(internalName, searchChildren);
}

//////////////////////////////////////////////////////////////////////////

bool Entity::removeComponent(Component *comp, bool deleteComponent)
{
   if (comp == NULL)
      return false;

   if(mComponents.remove(comp))
   {
      AssertFatal(comp->isProperlyAdded(), "Don't know how but a component is not registered w/ the sim");

      //setComponentsDirty();

      onComponentRemoved.trigger(comp);

      comp->setOwner(NULL);

      comp->onComponentRemove(); //in case the behavior needs to do cleanup on the owner

      if (deleteComponent)
         comp->safeDeleteObject();

      return true;
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////
//NOTE:
//The actor class calls this and flags the deletion of the behaviors to false so that behaviors that should no longer be attached during
//a network update will indeed be removed from the object. The reason it doesn't delete them is because when clearing the local behavior
//list, it would delete them, purging the ghost, and causing a crash when the unpack update tried to fetch any existing behaviors' ghosts
//to re-add them. Need to implement a clean clear function that will clear the local list, and only delete unused behaviors during an update.
void Entity::clearComponents(bool deleteComponents)
{
   bool srv = isServerObject();
   if (!deleteComponents)
   {
      while (mComponents.size() > 0)
      {
         removeComponent(mComponents.first(), deleteComponents);
      }
   }
   else
   {
      while (mComponents.size() > 0)
      {
         Component* comp = mComponents.first();

         if (comp)
         {
            comp->onComponentRemove(); //in case the behavior needs to do cleanup on the owner

            bool removed = mComponents.remove(comp);

            //we only need to delete them on the server side. they'll be cleaned up on the client side
            //via the ghosting system for us
            if (isServerObject())
               comp->deleteObject();
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////
Component *Entity::getComponent(const U32 index) const
{
   if (index < mComponents.size())
      return mComponents[index];

   return NULL;
}

Component *Entity::getComponent(String componentType)
{
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      Component* comp = mComponents[i];

      /*String namespaceName = comp->getNamespace()->mName;
       //check our namespace first
      if (namespaceName == componentType)
      {
         return comp;
      }
      else
      {*/
         //lets scan up, just to be sure
         Namespace *NS = comp->getNamespace();

         //we shouldn't ever go past Component into net object, as we're no longer dealing with component classes
         while (dStrcmp(NS->getName(), "NetObject"))
         {
            String namespaceName = NS->getName();

            if (namespaceName == componentType)
            {
               return comp;
            }
            else
            {
               NS = NS->getParent();
            }
         }
      //}
   }

   return NULL;
}

void Entity::onInspect()
{
   Vector<EditorInspectInterface*> updaters = getComponents<EditorInspectInterface>();
   for (Vector<EditorInspectInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++) 
   {
      (*it)->onInspect();
   }

   GuiTreeViewCtrl *editorTree = dynamic_cast<GuiTreeViewCtrl*>(Sim::findObject("EditorTree"));
   if (!editorTree)
      return;

   GuiTreeViewCtrl::Item *newItem, *parentItem;

   parentItem = editorTree->getItem(editorTree->findItemByObjectId(getId()));

   S32 componentID = editorTree->insertItem(parentItem->getID(), "Components");

   newItem = editorTree->getItem(componentID);
   newItem->mState.set(GuiTreeViewCtrl::Item::VirtualParent);
   newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);
   //newItem->mState.set(GuiTreeViewCtrl::Item::InspectorData);
   newItem->mState.set(GuiTreeViewCtrl::Item::ForceItemName);
   //newItem->mInspectorInfo.mObject = this;

   AssetManager *assetDB = dynamic_cast<AssetManager*>(Sim::findObject("AssetDatabase"));
   if (!assetDB)
      return;

   //This is used in the event of script-created assets, which likely only have
   //the name and other 'friendly' properties stored in a ComponentAsset.
   //So we'll do a query for those assets and find the asset based on the component's
   //class name
   AssetQuery* qry = new AssetQuery();
   qry->registerObject();

   assetDB->findAssetType(qry, "ComponentAsset");

   for (U32 i = 0; i < mComponents.size(); ++i)
   {
      String compName = mComponents[i]->getFriendlyName();

      if (compName == String(""))
      {
         String componentClass = mComponents[i]->getClassNamespace();

         //Means that it's a script-derived component and we should consult the asset to try
         //to get the info for it
         S32 compAssetCount = qry->mAssetList.size();
         for (U32 c = 0; c < compAssetCount; ++c)
         {
            StringTableEntry assetID = qry->mAssetList[c];

            ComponentAsset* compAsset = assetDB->acquireAsset<ComponentAsset>(assetID);

            String compAssetClass = compAsset->getComponentName();
            if (componentClass == compAssetClass)
            {
               compName = compAsset->getFriendlyName();
               break;
            }
         }
      }

      S32 compID = editorTree->insertItem(componentID, compName);
      newItem = editorTree->getItem(compID);
      newItem->mInspectorInfo.mObject = mComponents[i];
      newItem->mState.set(GuiTreeViewCtrl::Item::ForceItemName);
      newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);
      newItem->mState.set(GuiTreeViewCtrl::Item::InspectorData);
   }

   editorTree->buildVisibleTree(true);
}

void Entity::onEndInspect()
{
   Vector<EditorInspectInterface*> updaters = getComponents<EditorInspectInterface>();
   for (Vector<EditorInspectInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++) {
      (*it)->onEndInspect();
   }

   GuiTreeViewCtrl *editorTree = dynamic_cast<GuiTreeViewCtrl*>(Sim::findObject("EditorTree"));
   if (!editorTree)
      return;

   S32 componentItemIdx = editorTree->findItemByName("Components");

   editorTree->removeItem(componentItemIdx, false);
}

static void writeTabs(Stream &stream, U32 count)
{
   char tab[] = "   ";
   while (count--)
      stream.write(3, (void*)tab);
}

void Entity::write(Stream &stream, U32 tabStop, U32 flags)
{
   // Do *not* call parent on this

   /*VectorPtr<ComponentObject *> &componentList = lockComponentList();
   // export selected only?
   if( ( flags & SelectedOnly ) && !isSelected() )
   {
   for( BehaviorObjectIterator i = componentList.begin(); i != componentList.end(); i++ )
   (*i)->write(stream, tabStop, flags);

   goto write_end;
   }*/

   //catch if we have any written behavior fields already in the file, and clear them. We don't need to double-up
   //the entries for no reason.
   /*if(getFieldDictionary())
   {
   //get our dynamic field count, then parse through them to see if they're a behavior or not

   //reset it
   SimFieldDictionary* fieldDictionary = getFieldDictionary();
   SimFieldDictionaryIterator itr(fieldDictionary);
   for (S32 i = 0; i < fieldDictionary->getNumFields(); i++)
   {
   if (!(*itr))
   break;

   SimFieldDictionary::Entry* entry = *itr;
   if(strstr(entry->slotName, "_behavior"))
   {
   entry->slotName = "";
   entry->value = "";
   }

   ++itr;
   }
   }*/
   //all existing written behavior fields should be cleared. now write the object block

   writeTabs(stream, tabStop);

   char buffer[1024];
   dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() ? getName() : "");
   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   stream.write(1, "\n");
   ////first, write out our behavior objects

   // NOW we write the behavior fields proper
   if (mComponents.size() > 0)
   {
      // Pack out the behaviors into fields
      U32 i = 0;
      for (U32 i = 0; i < mComponents.size(); i++)
      {
         writeTabs(stream, tabStop + 1);
         char buffer[1024];
         dSprintf(buffer, sizeof(buffer), "new %s() {\r\n", mComponents[i]->getClassName());
         stream.write(dStrlen(buffer), buffer);
         //bi->writeFields( stream, tabStop + 2 );

         mComponents[i]->packToStream(stream, tabStop + 2, i - 1, flags);

         writeTabs(stream, tabStop + 1);
         stream.write(4, "};\r\n");
      }
   }

   //
   //if (size() > 0)
   //   stream.write(2, "\r\n");

   for (U32 i = 0; i < size(); i++)
   {
      SimObject* child = (*this)[i];
      if (child->getCanSave())
         child->write(stream, tabStop + 1, flags);
   }

   //stream.write(2, "\r\n");

   writeTabs(stream, tabStop);
   stream.write(4, "};\r\n");

   //write_end:
   //unlockComponentList();
}

SimObject* Entity::getTamlChild(const U32 childIndex) const
{
   // Sanity!
   AssertFatal(childIndex < getTamlChildCount(), "SimSet::getTamlChild() - Child index is out of range.");

   // For when the assert is not used.
   if (childIndex >= getTamlChildCount())
      return NULL;

   //we always order components first, child objects second
   if (childIndex >= getComponentCount())
      return at(childIndex - getComponentCount());
   else
      return getComponent(childIndex);
}
//
void Entity::onCameraScopeQuery(NetConnection* connection, CameraScopeQuery* query)
{
   // Object itself is in scope.
   Parent::onCameraScopeQuery(connection, query);

   if (CameraInterface* cI = getComponent<CameraInterface>())
   {
      cI->onCameraScopeQuery(connection, query);
   }
}
//
void Entity::setObjectBox(Box3F objBox)
{
   mObjBox = objBox;
   resetWorldBox();

   if (isServerObject())
      setMaskBits(BoundsMask);
}

void Entity::updateContainer()
{
   PROFILE_SCOPE(Entity_updateContainer);

   // Update container drag and buoyancy properties
   containerInfo.box = getWorldBox();
   //containerInfo.mass = mMass;

   getContainer()->findObjects(containerInfo.box, WaterObjectType | PhysicalZoneObjectType, findRouter, &containerInfo);

   //mWaterCoverage = info.waterCoverage;
   //mLiquidType    = info.liquidType;
   //mLiquidHeight  = info.waterHeight;   
   //setCurrentWaterObject( info.waterObject );

   // This value might be useful as a datablock value,
   // This is what allows the player to stand in shallow water (below this coverage)
   // without jiggling from buoyancy
   /*if (info.waterCoverage >= 0.25f)
   {
      // water viscosity is used as drag for in water.
      // ShapeBaseData drag is used for drag outside of water.
      // Combine these two components to calculate this ShapeBase object's 
      // current drag.
      mDrag = (info.waterCoverage * info.waterViscosity) +
         (1.0f - info.waterCoverage) * mDrag;
      //mBuoyancy = (info.waterDensity / mDataBlock->density) * info.waterCoverage;
   }

   //mAppliedForce = info.appliedForce;
   mGravityMod = info.gravityScale;*/
}
//

void Entity::setComponentsDirty()
{
   if (mToLoadComponents.empty())
      mStartComponentUpdate = true;

   //we need to build a list of behaviors that need to be pushed across the network
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      // We can do this because both are in the string table
      Component *comp = mComponents[i];

      if (comp->isNetworked())
      {
         bool unique = true;
         for (U32 i = 0; i < mToLoadComponents.size(); i++)
         {
            if (mToLoadComponents[i]->getId() == comp->getId())
            {
               unique = false;
               break;
            }
         }
         if (unique)
            mToLoadComponents.push_back(comp);
      }
   }

   setMaskBits(ComponentsMask);
}

void Entity::setComponentDirty(Component *comp, bool forceUpdate)
{
   bool found = false;
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      if (mComponents[i]->getId() == comp->getId())
      {
         mComponents[i]->setOwner(this);
         return;
      }
   }

   if (!found)
      return;

   //if(mToLoadComponents.empty())
   //	mStartComponentUpdate = true;

   /*if (comp->isNetworked() || forceUpdate)
   {
      bool unique = true;
      for (U32 i = 0; i < mToLoadComponents.size(); i++)
      {
         if (mToLoadComponents[i]->getId() == comp->getId())
         {
            unique = false;
            break;
         }
      }
      if (unique)
         mToLoadComponents.push_back(comp);
   }

   setMaskBits(ComponentsMask);*/
   
}

DefineEngineMethod(Entity, mountObject, bool,
   (SceneObject* objB, TransformF txfm), (MatrixF::Identity),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (objB)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      object->mountObject(objB, /*MatrixF::Identity*/txfm.getMatrix());
      return true;
   }
   return false;
}

DefineEngineMethod(Entity, setMountOffset, void,
   (Point3F posOffset), (Point3F(0, 0, 0)),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   object->setMountOffset(posOffset);
}

DefineEngineMethod(Entity, setMountRotation, void,
   (EulerF rotOffset), (EulerF(0, 0, 0)),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   object->setMountRotation(rotOffset);
}

DefineEngineMethod(Entity, getMountTransform, TransformF, (), ,
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   MatrixF mat;
   object->getMountTransform(0, MatrixF::Identity, &mat);
   return mat;
}

DefineEngineMethod(Entity, setBox, void,
   (Point3F box), (Point3F(1, 1, 1)),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   object->setObjectBox(Box3F(-box, box));
}


/*DefineConsoleMethod(Entity, callOnComponents, void, (const char* functionName), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   object->callOnComponents(functionName);
}

ConsoleMethod(Entity, callMethod, void, 3, 64, "(methodName, argi) Calls script defined method\n"
   "@param methodName The method's name as a string\n"
   "@param argi Any arguments to pass to the method\n"
   "@return No return value"
   "@note %obj.callMethod( %methodName, %arg1, %arg2, ... );\n")

{
   object->callMethodArgList(argc - 1, argv + 2);
}

ConsoleMethod(Entity, addComponents, void, 2, 2, "() - Add all fielded behaviors\n"
   "@return No return value")
{
   object->addComponents();
}*/

ConsoleMethod(Entity, addComponent, bool, 3, 3, "(ComponentInstance bi) - Add a behavior to the object\n"
   "@param bi The behavior instance to add"
   "@return (bool success) Whether or not the behavior was successfully added")
{
   Component *comp = dynamic_cast<Component *>(Sim::findObject(argv[2]));

   if (comp != NULL)
   {
      bool success = object->addComponent(comp);

      if (success)
      {
         //Placed here so we can differentiate against adding a new behavior during runtime, or when we load all
         //fielded behaviors on mission load. This way, we can ensure that we only call the callback
         //once everything is loaded. This avoids any problems with looking for behaviors that haven't been added yet, etc.
         if (comp->isMethod("onBehaviorAdd"))
            Con::executef(comp, "onBehaviorAdd");

         return true;
      }
   }

   return false;
}

ConsoleMethod(Entity, removeComponent, bool, 3, 4, "(ComponentInstance bi, [bool deleteBehavior = true])\n"
   "@param bi The behavior instance to remove\n"
   "@param deleteBehavior Whether or not to delete the behavior\n"
   "@return (bool success) Whether the behavior was successfully removed")
{
   bool deleteComponent = true;
   if (argc > 3)
      deleteComponent = dAtob(argv[3]);

   return object->removeComponent(dynamic_cast<Component *>(Sim::findObject(argv[2])), deleteComponent);
}

ConsoleMethod(Entity, clearComponents, void, 2, 2, "() - Clear all behavior instances\n"
   "@return No return value")
{
   object->clearComponents();
}

ConsoleMethod(Entity, getComponentByIndex, S32, 3, 3, "(int index) - Gets a particular behavior\n"
   "@param index The index of the behavior to get\n"
   "@return (ComponentInstance bi) The behavior instance you requested")
{
   Component *comp = object->getComponent(dAtoi(argv[2]));

   return (comp != NULL) ? comp->getId() : 0;
}

DefineConsoleMethod(Entity, getComponent, S32, (String componentName), (""),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   Component *comp = object->getComponent(componentName);

   return (comp != NULL) ? comp->getId() : 0;
   return 0;
}

/*ConsoleMethod(Entity, getBehaviorByType, S32, 3, 3, "(string BehaviorTemplateName) - gets a behavior\n"
   "@param BehaviorTemplateName The name of the template of the behavior instance you want\n"
   "@return (ComponentInstance bi) The behavior instance you requested")
{
   ComponentInstance *bInstance = object->getComponentByType(StringTable->insert(argv[2]));

   return (bInstance != NULL) ? bInstance->getId() : 0;
}*/

/*ConsoleMethod(Entity, reOrder, bool, 3, 3, "(ComponentInstance inst, [int desiredIndex = 0])\n"
   "@param inst The behavior instance you want to reorder\n"
   "@param desiredIndex The index you want the behavior instance to be reordered to\n"
   "@return (bool success) Whether or not the behavior instance was successfully reordered")
{
   Component *inst = dynamic_cast<Component *>(Sim::findObject(argv[1]));

   if (inst == NULL)
      return false;

   U32 idx = 0;
   if (argc > 2)
      idx = dAtoi(argv[2]);

   return object->reOrder(inst, idx);
}*/

ConsoleMethod(Entity, getComponentCount, S32, 2, 2, "() - Get the count of behaviors on an object\n"
   "@return (int count) The number of behaviors on an object")
{
   return object->getComponentCount();
}

DefineConsoleMethod(Entity, setComponentDirty, void, (S32 componentID, bool forceUpdate), (0, false),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   /*Component* comp;
   if (Sim::findObject(componentID, comp))
      object->setComponentDirty(comp, forceUpdate);*/
}

DefineConsoleMethod(Entity, getMoveVector, VectorF, (),,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   if (object->getControllingClient() != NULL)
   {
      //fetch our last move
      if (object->lastMove.x != 0 || object->lastMove.y != 0 || object->lastMove.z != 0)
         return VectorF(object->lastMove.x, object->lastMove.y, object->lastMove.z);
   }

   return VectorF::Zero;
}

DefineConsoleMethod(Entity, getMoveRotation, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   if(object->getControllingClient() != NULL)
   {
      //fetch our last move
      if (object->lastMove.pitch != 0 || object->lastMove.roll != 0 || object->lastMove.yaw != 0)
         return VectorF(object->lastMove.pitch, object->lastMove.roll, object->lastMove.yaw);
   }

   return VectorF::Zero;
}

DefineConsoleMethod(Entity, getMoveTrigger, bool, (S32 triggerNum), (0),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   if (object->getControllingClient() != NULL && triggerNum < MaxTriggerKeys)
   {
      return object->lastMove.trigger[triggerNum];
   }

   return false;
}

DefineConsoleMethod(Entity, setForwardVector, void, (VectorF newForward), (VectorF(0,0,0)),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   object->setForwardVector(newForward);
}

DefineConsoleMethod(Entity, lookAt, void, (Point3F lookPosition),,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   //object->setForwardVector(newForward);
}

DefineConsoleMethod(Entity, rotateTo, void, (Point3F lookPosition, F32 degreePerSecond), (1.0),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   //object->setForwardVector(newForward);
}