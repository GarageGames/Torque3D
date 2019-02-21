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
#include "T3D/gameBase/gameConnection.h"

#include <thread>
//
#include "gfx/sim/debugDraw.h"
//
#include "T3D/sfx/sfx3DWorld.h"

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

   mLifetimeMS = 0;

   mGameObjectAssetId = StringTable->insert("");

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

   addGroup("Misc");
   addField("LifetimeMS", TypeS32, Offset(mLifetimeMS, Entity), "Object world orientation.");
   endGroup("Misc");

   addGroup("GameObject");
   addProtectedField("gameObjectName", TypeGameObjectAssetPtr, Offset(mGameObjectAsset, Entity), &_setGameObject, &defaultProtectedGetFn,
      "The asset Id used for the game object this entity is based on.");
   endGroup("GameObject");
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

   mObjBox = Box3F(Point3F(-0.5, -0.5, -0.5), Point3F(0.5, 0.5, 0.5));
   
   resetWorldBox();
   setObjectBox(mObjBox);

   addToScene();

   //Make sure we get positioned
   if (isServerObject())
   {
      setMaskBits(TransformMask);
      setMaskBits(NamespaceMask);
   }
   else
   {
      //We can shortcut the initialization here because stuff generally ghosts down in order, and onPostAdd isn't called on ghosts.
      onPostAdd();
   }

   if (mLifetimeMS != 0)
      mStartTimeMS = Platform::getRealMilliseconds();

   return true;
}

void Entity::onRemove()
{
   clearComponents(true);

   removeFromScene();

   onDataSet.removeAll();

   mGameObjectAsset.clear();

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

   //Set up the networked components
   mNetworkedComponents.clear();
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      if (mComponents[i]->isNetworked())
      {
         NetworkedComponent netComp;
         netComp.componentIndex = i;
         netComp.updateState = NetworkedComponent::Adding;
         netComp.updateMaskBits = -1;

         mNetworkedComponents.push_back(netComp);
      }
   }

   if (!mNetworkedComponents.empty())
   {
      setMaskBits(AddComponentsMask);
      setMaskBits(ComponentsUpdateMask);
   }

   if (isMethod("onAdd"))
      Con::executef(this, "onAdd");
}

bool Entity::_setGameObject(void *object, const char *index, const char *data)
{
   // Sanity!
   AssertFatal(data != NULL, "Cannot use a NULL asset Id.");

   return true; //rbI->setMeshAsset(data);
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

      // Save current rigid state interpolation
      mDelta.posVec = getPosition();
      mDelta.rot[0] = mRot.asQuatF();

      //Handle any script updates, which can include physics stuff
      if (isServerObject() && isMethod("processTick"))
         Con::executef(this, "processTick");

      // Wrap up interpolation info
      mDelta.pos = getPosition();
      mDelta.posVec -= getPosition();
      mDelta.rot[1] = mRot.asQuatF();

      setTransform(getPosition(), mRot);

      //Lifetime test
      if (mLifetimeMS != 0)
      {
         S32 currentTime = Platform::getRealMilliseconds();
         if (currentTime - mStartTimeMS >= mLifetimeMS)
            deleteObject();
      }
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
      stream->writeCompressedPoint(mPos);
      mathWrite(*stream, getRotation());

      mDelta.move.pack(stream);

      stream->writeFlag(!(mask & NoWarpMask));
   }

   if (stream->writeFlag(mask & BoundsMask))
   {
      mathWrite(*stream, mObjBox);
   }

   if (stream->writeFlag(mask & AddComponentsMask))
   {
      U32 toAddComponentCount = 0;

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Adding)
         {
            toAddComponentCount++;
         }
      }

      //you reaaaaally shouldn't have >255 networked components on a single entity
      stream->writeInt(toAddComponentCount, 8);

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Adding)
         {
            const char* className = mComponents[mNetworkedComponents[i].componentIndex]->getClassName();
            stream->writeString(className, strlen(className));

            mNetworkedComponents[i].updateState = NetworkedComponent::Updating;
         }
      }
   }

   if (stream->writeFlag(mask & RemoveComponentsMask))
   {
      /*U32 toRemoveComponentCount = 0;

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Adding)
         {
            toRemoveComponentCount++;
         }
      }

      //you reaaaaally shouldn't have >255 networked components on a single entity
      stream->writeInt(toRemoveComponentCount, 8);

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Removing)
         {
            stream->writeInt(i, 16);
         }
      }*/

      /*for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::UpdateState::Removing)
         {
            removeComponent(mComponents[mNetworkedComponents[i].componentIndex], true);
            mNetworkedComponents.erase(i);
            i--;

         }
      }*/
   }

   //Update our components
   if (stream->writeFlag(mask & ComponentsUpdateMask))
   {
      U32 toUpdateComponentCount = 0;

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Updating)
         {
            toUpdateComponentCount++;
         }
      }

      //you reaaaaally shouldn't have >255 networked components on a single entity
      stream->writeInt(toUpdateComponentCount, 8);

      bool forceUpdate = false;

      for (U32 i = 0; i < mNetworkedComponents.size(); i++)
      {
         if (mNetworkedComponents[i].updateState == NetworkedComponent::Updating)
         {
            stream->writeInt(i, 8);

            mNetworkedComponents[i].updateMaskBits = mComponents[mNetworkedComponents[i].componentIndex]->packUpdate(con, mNetworkedComponents[i].updateMaskBits, stream);

            if (mNetworkedComponents[i].updateMaskBits != 0)
               forceUpdate = true;
            else
               mNetworkedComponents[i].updateState = NetworkedComponent::None;
         }
      }

      //If we have leftover, we need to re-iterate our packing
      if (forceUpdate)
         setMaskBits(ComponentsUpdateMask);
   }

   /*if (stream->writeFlag(mask & NamespaceMask))
   {
      const char* name = getName();
      if (stream->writeFlag(name && name[0]))
         stream->writeString(String(name));

      if (stream->writeFlag(mSuperClassName && mSuperClassName[0]))
         stream->writeString(String(mSuperClassName));

      if (stream->writeFlag(mClassName && mClassName[0]))
         stream->writeString(String(mClassName));
   }*/

   return retMask;
}

void Entity::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      Point3F pos;
      stream->readCompressedPoint(&pos);

      RotationF rot;
      mathRead(*stream, &rot);

      mDelta.move.unpack(stream);

      if (stream->readFlag() && isProperlyAdded())
      {
         // Determine number of ticks to warp based on the average
         // of the client and server velocities.
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

   if (stream->readFlag())
   {
      mathRead(*stream, &mObjBox);
      resetWorldBox();
   }

   //AddComponentMask
   if (stream->readFlag())
   {
      U32 addedComponentCount = stream->readInt(8);

      for (U32 i = 0; i < addedComponentCount; i++)
      {
         char className[256] = "";
         stream->readString(className);

         //Change to components, so iterate our list and create any new components
         // Well, looks like we have to create a new object.
         const char* componentType = className;

         ConsoleObject *object = ConsoleObject::create(componentType);

         // Finally, set currentNewObject to point to the new one.
         Component* newComponent = dynamic_cast<Component *>(object);

         if (newComponent)
         {
            addComponent(newComponent);
         }
      }
   }

   //RemoveComponentMask
   if (stream->readFlag())
   {
      
   }

   //ComponentUpdateMask
   if (stream->readFlag())
   {
      U32 updatingComponents = stream->readInt(8);

      for (U32 i = 0; i < updatingComponents; i++)
      {
         U32 updateComponentIndex = stream->readInt(8);

         Component* comp = mComponents[updateComponentIndex];
         comp->unpackUpdate(con, stream);
      }
   }

   /*if (stream->readFlag())
   {
      if (stream->readFlag())
      {
         char name[256];
         stream->readString(name);
         assignName(name);
      }

      if (stream->readFlag())
      {
         char superClassname[256];
         stream->readString(superClassname);
         mSuperClassName = superClassname;
      }

      if (stream->readFlag())
      {
         char classname[256];
         stream->readString(classname);
         mClassName = classname;
      }

      linkNamespaces();
   }*/
}

void Entity::setComponentNetMask(Component* comp, U32 mask)
{
   setMaskBits(Entity::ComponentsUpdateMask);

   for (U32 i = 0; i < mNetworkedComponents.size(); i++)
   {
      U32 netCompId = mComponents[mNetworkedComponents[i].componentIndex]->getId();
      U32 compId = comp->getId();

      if (netCompId == compId && 
         (mNetworkedComponents[i].updateState == NetworkedComponent::None || mNetworkedComponents[i].updateState == NetworkedComponent::Updating))
      {
         mNetworkedComponents[i].updateState = NetworkedComponent::Updating;
         mNetworkedComponents[i].updateMaskBits |= mask;

         break;
      }
   }
}

//Manipulation
void Entity::setTransform(const MatrixF &mat)
{
   MatrixF oldTransform = getTransform();

   if (isMounted())
   {
      // Use transform from mounted object
      Point3F newPos = mat.getPosition();
      Point3F parentPos = mMount.object->getTransform().getPosition();

      Point3F newOffset = newPos - parentPos;

      if (!newOffset.isZero())
      {
         mPos = newOffset;
      }

      Point3F matEul = mat.toEuler();

      if (matEul != Point3F(0, 0, 0))
      {
         Point3F mountEul = mMount.object->getTransform().toEuler();
         Point3F diff = matEul - mountEul;

         mRot = diff;
      }
      else
      {
         mRot = Point3F(0, 0, 0);
      }

      RotationF addRot = mRot + RotationF(mMount.object->getTransform());
      MatrixF transf = addRot.asMatrixF();
      transf.setPosition(mPos + mMount.object->getPosition());

      Parent::setTransform(transf);

      if (transf != oldTransform)
         setMaskBits(TransformMask);
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

void Entity::setTransform(const Point3F& position, const RotationF& rotation)
{
   MatrixF oldTransform = getTransform();

   if (isMounted())
   {
      mPos = position;
      mRot = rotation;

      RotationF addRot = mRot + RotationF(mMount.object->getTransform());
      MatrixF transf = addRot.asMatrixF();
      transf.setPosition(mPos + mMount.object->getPosition());

      Parent::setTransform(transf);

      if (transf != oldTransform)
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

      U32 compCount = mComponents.size();
      for (U32 i = 0; i < compCount; ++i)
      {
         mComponents[i]->ownerTransformSet(&newMat);
      }

      Point3F newPos = newMat.getPosition();
      RotationF newRot = newMat;

      Point3F oldPos = oldTransform.getPosition();
      RotationF oldRot = oldTransform;

      if (newPos != oldPos || newRot != oldRot)
         setMaskBits(TransformMask);
   }
}

void Entity::setRenderTransform(const MatrixF &mat)
{
   Parent::setRenderTransform(mat);
}

void Entity::setRenderTransform(const Point3F& position, const RotationF& rotation)
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

      U32 compCount = mComponents.size();
      for (U32 i = 0; i < compCount; ++i)
      {
         mComponents[i]->ownerTransformSet(&newMat);
      }
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

void Entity::setMountOffset(const Point3F& posOffset)
{
   if (isMounted())
   {
      mMount.xfm.setColumn(3, posOffset);
      //mPos = posOffset;
      setMaskBits(MountedMask);
   }
}

void Entity::setMountRotation(const EulerF& rotOffset)
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
   Con::errorf("Build Poly List not yet implemented as a passthrough for Entity");
   /*Vector<BuildPolyListInterface*> updaters = getComponents<BuildPolyListInterface>();
   for (Vector<BuildPolyListInterface*>::iterator it = updaters.begin(); it != updaters.end(); it++)
   {
      return (*it)->buildPolyList(context, polyList, box, sphere);
   }*/

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
void Entity::mountObject(SceneObject* objB, const MatrixF& txfm)
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

void Entity::setControllingClient(GameConnection* client)
{
   if (isGhost() && gSFX3DWorld)
   {
      if (gSFX3DWorld->getListener() == this && !client && getControllingClient() && getControllingClient()->isConnectionToServer())
      {
         // We are the current listener and are no longer a controller object on the
         // connection, so clear our listener status.

         gSFX3DWorld->setListener(NULL);
      }
      else if (client && client->isConnectionToServer() && !getControllingObject())
      {
         // We're on the local client and not controlled by another object, so make
         // us the current SFX listener.

         gSFX3DWorld->setListener(this);
      }
   }
   Parent::setControllingClient(client);
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

   comp->setIsServerObject(isServerObject());

   //if we've already been added and this is being added after the fact(at runtime), 
   //then just go ahead and call it's onComponentAdd so it can get to work
   //if (mInitialized)
   {
      comp->onComponentAdd();

      if (comp->isNetworked())
      {
         NetworkedComponent netComp;
         netComp.componentIndex = mComponents.size() - 1;
         netComp.updateState = NetworkedComponent::Adding;
         netComp.updateMaskBits = -1;

         mNetworkedComponents.push_back(netComp);

         setMaskBits(AddComponentsMask);
         setMaskBits(ComponentsUpdateMask);
      }
   }

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

      comp->onComponentRemove(); //in case the behavior needs to do cleanup on the owner
      comp->setOwner(NULL);

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
         while (dStrcmp(NS->getName(), "SimObject"))
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
      for (U32 i = 0; i < mComponents.size(); i++)
      {
         writeTabs(stream, tabStop + 1);
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
void Entity::setObjectBox(const Box3F& objBox)
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

void Entity::notifyComponents(String signalFunction, String argA, String argB, String argC, String argD, String argE)
{
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      // We can do this because both are in the string table
      Component *comp = mComponents[i];

      if (comp->isActive())
      {
         if (comp->isMethod(signalFunction))
            Con::executef(comp, signalFunction, argA, argB, argC, argD, argE);
      }
   }
}

void Entity::setComponentsDirty()
{
   /*if (mToLoadComponents.empty())
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

   setMaskBits(ComponentsMask);*/
}

void Entity::setComponentDirty(Component *comp, bool forceUpdate)
{
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      if (mComponents[i]->getId() == comp->getId())
      {
         mComponents[i]->setOwner(this);
         return;
      }
   }

   //if (!found)
   //   return;

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


/*DefineEngineMethod(Entity, callOnComponents, void, (const char* functionName), ,
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

DefineEngineMethod(Entity, addComponent, bool, (Component* comp),,
   "@brief Add a behavior to the object\n"
   "@param bi The behavior instance to add"
   "@return (bool success) Whether or not the behavior was successfully added")
{
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

DefineEngineMethod(Entity, removeComponent, bool, (Component* comp, bool deleteComponent), (true),
   "@param bi The behavior instance to remove\n"
   "@param deleteBehavior Whether or not to delete the behavior\n"
   "@return (bool success) Whether the behavior was successfully removed")
{
   return object->removeComponent(comp, deleteComponent);
}

DefineEngineMethod(Entity, clearComponents, void, (),, "Clear all behavior instances\n"
   "@return No return value")
{
   object->clearComponents();
}

DefineEngineMethod(Entity, getComponentByIndex, Component*, (S32 index),, 
   "@brief Gets a particular behavior\n"
   "@param index The index of the behavior to get\n"
   "@return (ComponentInstance bi) The behavior instance you requested")
{
   return object->getComponent(index);
}

DefineEngineMethod(Entity, getComponent, Component*, (String componentName), (""),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   return object->getComponent(componentName);
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

DefineEngineMethod(Entity, getComponentCount, S32, (),, 
   "@brief Get the count of behaviors on an object\n"
   "@return (int count) The number of behaviors on an object")
{
   return object->getComponentCount();
}

DefineEngineMethod(Entity, setComponentDirty, void, (S32 componentID, bool forceUpdate), (0, false),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   /*Component* comp;
   if (Sim::findObject(componentID, comp))
      object->setComponentDirty(comp, forceUpdate);*/
}

DefineEngineMethod(Entity, getMoveVector, VectorF, (),,
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

DefineEngineMethod(Entity, getMoveRotation, VectorF, (), ,
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

DefineEngineMethod(Entity, getMoveTrigger, bool, (S32 triggerNum), (0),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   if (object->getControllingClient() != NULL && triggerNum < MaxTriggerKeys)
   {
      return object->lastMove.trigger[triggerNum];
   }

   return false;
}

DefineEngineMethod(Entity, getForwardVector, VectorF, (), ,
   "Get the direction this object is facing.\n"
   "@return a vector indicating the direction this object is facing.\n"
   "@note This is the object's y axis.")
{
   VectorF forVec = object->getTransform().getForwardVector();
   return forVec;
}

DefineEngineMethod(Entity, setForwardVector, void, (VectorF newForward), (VectorF(0,0,0)),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   object->setForwardVector(newForward);
}

DefineEngineMethod(Entity, lookAt, void, (Point3F lookPosition),,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   //object->setForwardVector(newForward);
}

DefineEngineMethod(Entity, rotateTo, void, (Point3F lookPosition, F32 degreePerSecond), (1.0),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   //object->setForwardVector(newForward);
}

DefineEngineMethod(Entity, notify, void, (String signalFunction, String argA, String argB, String argC, String argD, String argE),
("", "", "", "", "", ""),
"Triggers a signal call to all components for a certain function.")
{
   if (signalFunction == String(""))
      return;

   object->notifyComponents(signalFunction, argA, argB, argC, argD, argE);
}

DefineEngineFunction(findEntitiesByTag, const char*, (SimGroup* searchingGroup, String tags), (nullAsType<SimGroup*>(), ""),
"Finds all entities that have the provided tags.\n"
"@param searchingGroup The SimGroup to search inside. If null, we'll search the entire dictionary(this can be slow!).\n"
"@param tags Word delimited list of tags to search for. If multiple tags are included, the list is eclusively parsed, requiring all tags provided to be found on an entity for a match.\n"
"@return A word list of IDs of entities that match the tag search terms.")
{
   //if (tags.isEmpty())
      return "";

   /*if (searchingGroup == nullptr)
   {
      searchingGroup = Sim::getRootGroup();
   }

   StringTableEntry entityStr = StringTable->insert("Entity");

   std::thread threadBob;

   std::thread::id a = threadBob.get_id();
   std::thread::id b = std::this_thread::get_id().;

   if (a == b)
   {
      //do
   }

   for (SimGroup::iterator itr = searchingGroup->begin(); itr != searchingGroup->end(); itr++)
   {
      Entity* ent = dynamic_cast<Entity*>((*itr));

      if (ent != nullptr)
      {
         ent->mTags.
      }
   }

   object->notifyComponents(signalFunction, argA, argB, argC, argD, argE);*/
}