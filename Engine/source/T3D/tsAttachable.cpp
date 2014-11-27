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
//
// Author: Michael A. Reino
// The concept for this class was taken directly from the PathShape
// resource: http://www.garagegames.com/community/resource/view/20385/1
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "tsAttachable.h"

#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/mathIO.h"
#include "math/mTransform.h"
#include "T3D/player.h"

IMPLEMENT_CO_NETOBJECT_V1(TSAttachable);

ConsoleDocClass( TSAttachable,
   "@brief A path shape that can be walked on/ridden.\n\n"

   "@tsexample\n"
         "new TSAttachable(RideShape) {\n"
         "   UseAutoAttach = \"1\";\n"
         "   RayLength = \"1.5\";\n"
         "   Path = \"mPathA\";\n"
         "   movementState = \"Stop\";\n"
         "   shapeName = \"art/shapes/rocks/boulder.dts\";\n"
         "   playAmbient = \"0\";\n"
         "   collisionType = \"Visible Mesh\";\n"
         "   decalType = \"Collision Mesh\";\n"
         "   position = \"315.18 -180.418 244.313\";\n"
         "   rotation = \"0 0 1 195.952\";\n"
         "   scale = \"1 1 1\";\n"
         "   isRenderEnabled = \"true\";\n"
         "   canSaveDynamicFields = \"1\";\n"
         "};\n"
   "@endtsexample\n"

   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( TSAttachable, onObjectAttached, void, ( SceneObject *obj ), ( obj ),
   "@brief Called when an object is attached to this one via script call or automatic attachment.\n\n"
   "@param obj the SceneObject object being attached\n");

IMPLEMENT_CALLBACK( TSAttachable, onObjectDetached, void, ( SceneObject *obj ), ( obj ),
   "@brief Called when an object is detached and will no longer move with the attachable shape.\n\n"
   "@param obj the SceneObject object that has been detached.\n");

TSAttachable::TSAttachable()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mUseAutoAttach = false;
   mAttachTypesMask = PlayerObjectType | DebrisObjectType | ItemObjectType;
   mRayLength = 1.0f;
   mInterpDelta = 0.0f;
   mWorldZRot = 0.0f;
}

TSAttachable::~TSAttachable()
{
}

void TSAttachable::initPersistFields()
{
   addGroup("Attachments");

      addField( "UseAutoAttach", TypeBool, Offset( mUseAutoAttach, TSAttachable ),
         "Scan for objects and automatically attach any that are within RayLength and directly above." );

      addField( "RayLength", TypeF32, Offset( mRayLength, TSAttachable ),
         "Length of ray to use when checking if objects can attach or should be detached from this shape. "
         "The ray is cast this distance directly below each object scanned." );

   endGroup("Attachments");

   Parent::initPersistFields();
}

bool TSAttachable::onAdd()
{
   PROFILE_SCOPE(TSAttachable_onAdd);

   if ( !Parent::onAdd() )
      return false;

   // Save our initial rotation value
   Point3F fwdVec;
   mObjToWorld.getColumn(1, &fwdVec);
   fwdVec.z = 0.0f;
   fwdVec.normalizeSafe();
   mWorldZRot = -mAtan2(-fwdVec.x, fwdVec.y);

   _updateShouldTick();

   return true;
}

void TSAttachable::onRemove()
{
   detachAll();
   Parent::onRemove();
}

void TSAttachable::onStaticModified( const char* slotName, const char*newValue )
{
   if ( dStricmp(slotName, "UseAutoAttach") == 0 )
   {  // Update our ticking state if the attachment method changes
      _updateShouldTick();
      if ( isServerObject() )
         setMaskBits( MethodMask );
   }
   else if ( dStricmp(slotName, "RayLength") == 0 )
   {
      if ( isServerObject() )
         setMaskBits( MethodMask );
   }
   else
      Parent::onStaticModified( slotName, newValue );
}

void TSAttachable::processTick( const Move *move )
{
   if ( isServerObject() )
   {  // Check if any attachments have fallen off
      checkAttachments();

      // Check for objects that need attached
      if ( mUseAutoAttach )
         autoAttach();
   }

   // Save the WorldToObj matrix from before our local shape move
   MatrixF oldWorldToObj = mWorldToObj;

   Parent::processTick(move);

   // Update our saved Z rotation
   mlastWorldZRot = mWorldZRot;
   Point3F fwdVec;
   mObjToWorld.getColumn(1, &fwdVec);
   fwdVec.z = 0.0f;
   fwdVec.normalizeSafe();
   mWorldZRot = -mAtan2(-fwdVec.x, fwdVec.y);
   if ( mWorldZRot < 0.0f )
      mWorldZRot += M_2PI_F;

   if ( mAttachments.size() )
   {  // Move all attachments along with the shape
      moveAttachments(oldWorldToObj, mObjToWorld);
      setMaskBits( MovementMask );
   }
}

void TSAttachable::interpolateTick( F32 delta )
{
   mInterpDelta = 1.0f - delta;
   Parent::interpolateTick(delta);
}

void TSAttachable::advanceTime( F32 dt )
{
   Parent::advanceTime(dt);
   
   if ( mAttachments.size() )
      moveRenderedAttachments();
}

bool TSAttachable::_getShouldTick()
{
   return mUseAutoAttach || mAttachments.size() || Parent::_getShouldTick();
}

//----------------------------------------------------------------------------
U32 TSAttachable::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & AttachmentsMask))
   {
      for ( U32 i = 0; i < mAttachments.size(); ++i )
      {
         S32 gIndex = con->getGhostIndex(mAttachments[i].obj);
         if ( gIndex != -1 )
         {
            stream->writeFlag(true);
            stream->writeInt( gIndex, NetConnection::GhostIdBitSize );
         }
         else
            retMask |= AttachmentsMask;
      }
      stream->writeFlag(false);
   }

   if (stream->writeFlag(mask & (MovementMask | AttachmentsMask)))
   {
      for ( U32 i = 0; i < mAttachments.size(); ++i )
      {
         S32 gIndex = con->getGhostIndex(mAttachments[i].obj);
         if ( gIndex == -1 )
            continue; // If we didn't send the object info yet, we can't send movement either.

         stream->writeFlag(true);

         // Send the objects local position
         SceneObject *obj = mAttachments[i].obj;
         MatrixF localTrans, objTrans = obj->getTransform();
         localTrans.mul(mWorldToObj, objTrans);
         Point3F anchorPoint;
         localTrans.getColumn(3, &anchorPoint);
         mathWrite(*stream, anchorPoint);

         // Send the world space z rotation relative to ours
         Point3F fwdVec;
         objTrans.getColumn(1, &fwdVec);
         F32 relativeRot = mWorldZRot - (-mAtan2(-fwdVec.x, fwdVec.y));
         if (  relativeRot < 0 )
             relativeRot += M_2PI_F;
         if (  relativeRot > M_2PI_F )
             relativeRot -= M_2PI_F;
         stream->write(relativeRot);
         mAttachments[i].relZRot = relativeRot;

         // Send a flag to let them know if this is their control object that's attached
         stream->writeFlag( con == ((NetConnection *) obj->getControllingClient()) );
      }
      stream->writeFlag(false);
   }

   if (stream->writeFlag(mask & MethodMask))
   {
      stream->writeFlag(mUseAutoAttach);
      stream->write(mRayLength);
   }

   return retMask;
}

void TSAttachable::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   // AttachmentsMask
   if (stream->readFlag())
   {
      detachAll();
      while ( stream->readFlag() )
      {
         S32 gIndex = stream->readInt( NetConnection::GhostIdBitSize );
         SceneObject* obj = dynamic_cast<SceneObject*>( con->resolveGhost( gIndex ) );
         if ( obj )
         {
            AttachmentData objData;
            objData.obj = obj;

            objData.zRotDelta = 0.0f;
            objData.needsUpdate = true;
            mAttachments.push_back(objData);
            obj->processAfter(this);
            obj->setAttachedToObj(this);
            deleteNotify(obj);
         }
      }
      _updateShouldTick();
   }

   // MovementMask
   if (stream->readFlag())
   {
      Point3F worldPos;
      F32 relativeRot, newZRot;
      S32 i = 0;
      while ( stream->readFlag() )
      {
         SceneObject *obj = mAttachments[i].obj;
         MatrixF objMat;

         mathRead(*stream, &worldPos);
         stream->read(&relativeRot);
         bool isLocalControlObj = stream->readFlag();

         mAttachments[i].anchorPoint = worldPos;
         mAttachments[i].relZRot = relativeRot;
         mObjToWorld.mulP(worldPos);

         newZRot = mWorldZRot - relativeRot;
         while (newZRot < 0)
            newZRot += M_2PI_F;
         while (newZRot >= M_2PI_F)
            newZRot -= M_2PI_F;
         if ( obj->getTypeMask() & PlayerObjectType )
         {  // Players just don't play nice when you modify their transform on a client
            Player *plrObj = (Player *) obj;
            if ( mAttachments[i].needsUpdate || !isLocalControlObj )
            {
               Point3F deltaRot(0.0f, 0.0f, newZRot);
               plrObj->setPosition(worldPos, deltaRot);
               plrObj->setDeltas(worldPos, deltaRot); // Reset interpolation to the current rotation so they don't warp it
               mAttachments[i].needsUpdate = false;
            }
         }
         else
         {
            objMat.set(EulerF(0.0f, 0.0f, newZRot));
            objMat.setPosition(worldPos);
            obj->setTransform(objMat);
         }
         i++;
      }
   }

   // MethodMask
   if (stream->readFlag())
   {
      mUseAutoAttach = stream->readFlag();
      stream->read(&mRayLength);
      _updateShouldTick();
   }
}

void TSAttachable::onDeleteNotify( SimObject *obj )
{
   for ( U32 i = 0; i < mAttachments.size(); ++i )
      if ( mAttachments[i].obj == obj )
      {
         mAttachments.erase(i);
         if ( isServerObject() )
         {
            setMaskBits( AttachmentsMask );
            onObjectDetached_callback(NULL);
         }
      }

   Parent::onDeleteNotify( obj );
   _updateShouldTick();
}

//----------------------------------------------------------------------------
void TSAttachable::moveAttachments(MatrixF &oldWTOMat, MatrixF &newOTWMat)
{  // Called each tick to move all attachments
   SceneObject *obj;
   MatrixF objMat;
   Point3F newPos, fwdVec;
   F32 newZRot, oldZRot = 0.0f;

   for ( S32 i = mAttachments.size() - 1; i >= 0; i-- )
   {
      obj = mAttachments[i].obj;
      objMat = obj->getTransform();

      objMat.getColumn(1, &fwdVec);
      oldZRot = -mAtan2(-fwdVec.x, fwdVec.y);
      if ( oldZRot < 0.0f )
         oldZRot += M_2PI_F;
      mAttachments[i].relZRot = mlastWorldZRot - oldZRot;
      if (  mAttachments[i].relZRot < 0.0f )
            mAttachments[i].relZRot += M_2PI_F;

      objMat.mulL(oldWTOMat);
      mAttachments[i].anchorPoint = objMat.getPosition(); 
      objMat.mulL(newOTWMat);

      // Save the amount that we're going to rotate the attachment this tick so we can interpolate it in.
      newZRot = mWorldZRot - mAttachments[i].relZRot;
      mAttachments[i].zRotDelta = newZRot - oldZRot;
      while ( mFabs(mAttachments[i].zRotDelta) > M_PI_F )
         mAttachments[i].zRotDelta += (mAttachments[i].zRotDelta < 0.0f) ? M_2PI_F : -M_2PI_F;

      objMat.getColumn(3, &newPos);
      objMat.set(EulerF(0.0f, 0.0f, newZRot));
      objMat.setPosition(newPos);

      obj->setTransform(objMat);
   }
}

void TSAttachable::moveRenderedAttachments()
{  // Called each frame to set render transform for all attachments
   SceneObject *obj;
   Point3F fwdVec, worldPos;
   MatrixF objMat;
   F32 newRot;

   for ( S32 i = mAttachments.size() - 1; i >= 0; i-- )
   {
      // Find the attachment's new render position
      obj = mAttachments[i].obj;
      objMat = obj->getRenderTransform();

      worldPos = mAttachments[i].anchorPoint;
      mRenderObjToWorld.mulP(worldPos);

      if ( mFabs(mAttachments[i].zRotDelta) > 0.0002f )
      {  // Add the amount that we've rotated the attachment
         objMat.getColumn(1, &fwdVec);
         newRot = -mAtan2(-fwdVec.x,fwdVec.y);
         newRot += mInterpDelta * mAttachments[i].zRotDelta;

         if ( newRot < 0.0f )
            newRot += M_2PI_F;
         if ( newRot > M_2PI_F )
            newRot -= M_2PI_F;
         objMat.set(EulerF(0.0f, 0.0f, newRot));
      }
      objMat.setColumn(3, worldPos);
      obj->setRenderTransform(objMat);
   }
}

//----------------------------------------------------------------------------
void TSAttachable::getRelativeOrientation(SceneObject *attachedObj, Point3F &relPos, Point3F &relRot)
{
   // Fills the position and rotation for an attached object relative to the object that
   // it's attached to.
   for ( U32 i = 0; i < mAttachments.size(); ++i )
      if ( mAttachments[i].obj == attachedObj )
      {
         relPos = mAttachments[i].anchorPoint;
         relRot.set(0.0f, 0.0f, mAttachments[i].relZRot);
         return;
      }

   relPos = relRot = Point3F::Zero;
}

void TSAttachable::flagAttachedUpdate(SceneObject *attachedObj, bool doUpdate)
{
   for ( U32 i = 0; i < mAttachments.size(); ++i )
      if ( mAttachments[i].obj == attachedObj )
      {
         mAttachments[i].needsUpdate = doUpdate;
         return;
      }
}

//----------------------------------------------------------------------------
bool TSAttachable::isObjectAbove(SceneObject *obj)
{
   bool isAbove = false;
   Point3F pos = obj->getPosition();
   Point3F startPos(pos.x, pos.y, pos.z + 0.1f);
   Point3F endPos(pos.x, pos.y, pos.z - mRayLength);
   RayInfo rInfo;
   obj->disableCollision();
   if ( isServerObject() )
   {
      if ( gServerContainer.castRay(startPos, endPos, mTypeMask, &rInfo) )
         if ( rInfo.object == this )
            isAbove = true;
   }
   else
   {
      if ( gClientContainer.castRayRendered(startPos, endPos, mTypeMask, &rInfo) )
         if ( rInfo.object == this )
            isAbove = true;
   }
   obj->enableCollision();

   return isAbove;
}

bool TSAttachable::isObjectAttached(SceneObject *obj)
{
   for ( U32 i = 0; i < mAttachments.size(); ++i )
      if ( mAttachments[i].obj == obj )
         return true;

   return false;
}

void TSAttachable::checkAttachments()
{  // Check to see if any of our attachments have fallen off
   SceneObject *obj;

   for ( S32 i = mAttachments.size() - 1; i >= 0; i-- )
   {
      obj = mAttachments[i].obj;
      if ( !isObjectAbove(obj) )
         detachObject(obj);
   }
}

void TSAttachable::autoAttach()
{  // Check to see if there are any objects to attach and attach them
   Vector< SceneObject* > foundList;
   Box3F searchBox = mWorldBox;
   searchBox.maxExtents.z = mWorldBox.maxExtents.z + mRayLength;
   gServerContainer.findObjectList(searchBox, mAttachTypesMask, &foundList);
   for ( U32 i = 0; i < foundList.size(); ++i )
      attachObject(foundList[i]);
}

//----------------------------------------------------------------------------
bool TSAttachable::attachObject(SceneObject *obj)
{
   if ( !obj || !isObjectAbove(obj) || (obj == this) || obj->isMounted() )
      return false;

   if ( isObjectAttached(obj) )
      return true;

   AttachmentData objData;
   objData.obj = obj;
   mAttachments.push_back(objData);
   obj->processAfter(this);
   obj->setAttachedToObj(this);
   deleteNotify(obj);

   MatrixF objMat;
   objMat = obj->getTransform();
   obj->setTransform(objMat); // This forces a position update on the client for this object

   _updateShouldTick();

   if ( isServerObject() )
   {
      setMaskBits( AttachmentsMask );
      onObjectAttached_callback(obj);
   }

   return true;
}

bool TSAttachable::detachObject(SceneObject *obj)
{
   if ( !obj )
      return false;

   for ( U32 i = 0; i < mAttachments.size(); ++i )
      if ( mAttachments[i].obj == obj )
      {
         clearNotify(obj);
         if ( obj->getAfterObject() == this )
            obj->clearProcessAfter();
         if ( obj->getAttachedToObj() == this )
            obj->setAttachedToObj(NULL);
         mAttachments.erase(i);
         if ( isServerObject() )
         {
            setMaskBits( AttachmentsMask );
            onObjectDetached_callback(obj);
         }
         _updateShouldTick();

         return true;
      }

   return false;
}

void TSAttachable::detachAll()
{
   for ( U32 i = 0; i < mAttachments.size(); ++i )
   {
      SceneObject *obj = mAttachments[i].obj;
      clearNotify(obj);
      if ( obj->getAfterObject() == this )
         obj->clearProcessAfter();
      if ( obj->getAttachedToObj() == this )
         obj->setAttachedToObj(NULL);
   }

   mAttachments.clear();
   setMaskBits( AttachmentsMask );
   _updateShouldTick();
}

S32 TSAttachable::getNumAttachments()
{
   return mAttachments.size();
}

SceneObject *TSAttachable::getAttachment(S32 index)
{
   if ( (index < 0) || (index >= mAttachments.size()) )
      return NULL;
   return mAttachments[index].obj;
}

//----------------------------------------------------------------------------
DefineEngineMethod(TSAttachable, attachObject, bool, (SceneObject *obj), ,
      "Attaches an object to this one.\n"
      "@param obj The scene object to attach to us\n"
      "@return true if successful, false if failed. This function will fail if the object passed "
      "is invalid or is not located directly above and within RayLength of this shape.\n")
{
   return object->attachObject(obj);
}

DefineEngineMethod(TSAttachable, detachObject, bool, (SceneObject *obj), ,
      "Detaches an object from this one.\n"
      "@param obj The scene object to be detached\n"
      "@return true if successful, false if failed. This function will fail if the object passed "
      "is invalid or is not currently attached to this shape.")
{
   return object->detachObject(obj);
}

DefineEngineMethod(TSAttachable, detachAll, void, (), ,
      "Detaches all attached objects. Note: if UseAutoAttach is true when this is called, all of"
      " the objects may be re-attached on the next tick.\n\n"
      "@tsexample\n"
         "// Dump all riders\n"
         "%attachableObj.UseAutoAttach = false\n"
         "%attachableObj.detachAll(); = false\n"
      "@endtsexample\n\n")
{
   object->detachAll();
   return;
}

DefineEngineMethod(TSAttachable, getNumAttachments, S32, (), ,
      "Returns the number of objects that are currently attached.\n")
{
   return object->getNumAttachments();
}

DefineEngineMethod(TSAttachable, getAttachment, SceneObject *, (S32 index), (0),
      "Returns the attachment at the passed index value.\n")
{
   return object->getAttachment(index);
}
