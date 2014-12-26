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

#ifndef _WALKABLESHAPE_H_
#define _WALKABLESHAPE_H_

#ifndef _TSDYNAMIC_H_
#include "tsDynamic.h"
#endif

/// An object that other objects can be attached to.
class WalkableShape : public TSDynamic
{
private:
   typedef TSDynamic Parent;

public:

private:
   enum MaskBits 
   {
      AttachmentsMask   = Parent::NextFreeMask << 0,
      MovementMask      = Parent::NextFreeMask << 1,
      MethodMask        = Parent::NextFreeMask << 2,
      NextFreeMask      = Parent::NextFreeMask << 3
   };

   // This just saves the interpolation delta between the interpolateTick() call where we receive it
   // and advanceTime() where we use it
   F32 mInterpDelta;

   // Keep track of the shape's rotation around the z axis in world space so we
   // can send relative rotation values to clients
   F32 mWorldZRot, mlastWorldZRot;

protected:

   struct AttachmentData
   {
      SceneObject *obj;
      Point3F anchorPoint;
      F32 zRotDelta, relZRot;
      bool needsUpdate;
   };
   Vector<AttachmentData> mAttachments;

   bool onAdd();
   void onRemove();
   virtual void onStaticModified(const char* slotName, const char*newValue = NULL);

   // ProcessObject
   virtual void processTick( const Move *move );
   virtual void interpolateTick( F32 delta );
   virtual void advanceTime( F32 dt );

   virtual bool _getShouldTick();

   // Check to see if an object is above us. This is used to determine if an object can 
   // be attached or should be detached.
   bool isObjectAbove(SceneObject *obj);

   bool isObjectAttached(SceneObject *obj);

   // Check to see if any of our attachments have fallen off
   void checkAttachments();

   // Check to see if if there are any objects to attach and attach them
   void autoAttach();

   // Use automatic attachment scanning
   bool mUseAutoAttach;

   // Called each tick to move all attachments
   void moveAttachments(MatrixF &oldWTOMat, MatrixF &newOTWMat);
   void moveRenderedAttachments();

   // An attached object will automatically detatch itself if the object it's attached to
   // is not found in a raytrace this distance directly below it.
   F32 mRayLength;

   // A mask of all types that we can auto attach.
   U32 mAttachTypesMask;

public:

   WalkableShape();
   ~WalkableShape();

   DECLARE_CONOBJECT(WalkableShape);
   DECLARE_CALLBACK(void, onObjectAttached, (SceneObject *obj));
   DECLARE_CALLBACK(void, onObjectDetached, (SceneObject *obj));

   static void initPersistFields();

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );
   virtual void onDeleteNotify(SimObject *object);

   // Add an object to our attachments
   virtual bool attachObject(SceneObject *obj);

   // Remove an object from our attachments
   virtual bool detachObject(SceneObject *obj);

   // Removes all attached objects
   virtual void detachAll();

   // Returns the number of currently attached objects
   S32 getNumAttachments();

   // Return the attachment at index
   SceneObject *getAttachment(S32 index);

   virtual void getRelativeOrientation(SceneObject *attachedObj, Point3F &relPos, Point3F &relRot);
   virtual void flagAttachedUpdate(SceneObject *attachedObj, bool doUpdate);
};

#endif // _WALKABLESHAPE_H_

