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

#ifndef _T3D_PHYSICS_PHYSICSOBJECT_H_
#define _T3D_PHYSICS_PHYSICSOBJECT_H_

#ifndef _PHYSICS_PHYSICSUSERDATA_H_
#include "T3D/physics/physicsUserData.h"
#endif
#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif
#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif

class PhysicsWorld;
class MatrixF;
class Point3F;
class Box3F;


///
class PhysicsObject : public WeakRefBase 
{
public:
   
   virtual ~PhysicsObject();

   /// Returns the physics world this object is a member of.
   virtual PhysicsWorld* getWorld() = 0;

   /// Sets the transform on the physics object.
   ///
   /// For static objects this is only intended to be used for
   /// for infrequent changes when editing the mission.
   ///
   virtual void setTransform( const MatrixF &transform ) = 0;

   /// Returns the transform of the physics body at 
   /// the last processed simulation tick.
   virtual MatrixF& getTransform( MatrixF *outMatrix ) = 0;

   /// Returns the world aligned bounding box containing the PhysicsObject.
   virtual Box3F getWorldBounds() = 0;

   /// 
   void queueCallback( U32 ms, Delegate<void()> callback );

   const PhysicsUserData& getUserData() const { return mUserData; }

   PhysicsUserData& getUserData() { return mUserData; }

   /// Set false to skip simulation of this object or temporarily remove
   /// it from the physics simulation. Implementation is PhysicsPlugin specific.
   virtual void setSimulationEnabled( bool enabled ) = 0;
   virtual bool isSimulationEnabled() = 0;

protected:

   /// You shouldn't be creating this object directly.
   PhysicsObject();

   /// The user data object assigned to this object.
   PhysicsUserData mUserData;

   /// The last queued callback event.
   /// @see queueCallback
   U32 mQueuedEvent;
};

#endif // _T3D_PHYSICS_PHYSICSOBJECT_H_