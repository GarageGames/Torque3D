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

#ifndef _T3D_PHYSICS_PHYSICSFORCE_H_
#define _T3D_PHYSICS_PHYSICSFORCE_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSOBJECT_H_
#include "T3D/physics/physicsObject.h"
#endif

class PhysicsBody;
class PhysicsWorld;


/// A physics force controller used for gameplay effects.
class PhysicsForce : public SceneObject
{
   typedef SceneObject Parent;

public:

   PhysicsForce();
   virtual ~PhysicsForce();

   DECLARE_CONOBJECT( PhysicsForce );

   // SimObject
   static void initPersistFields();
   bool onAdd();
   void onRemove();   
   
   // SceneObject
   void onMount( SceneObject *obj, S32 node );
   void onUnmount( SceneObject *obj, S32 node );

   // ProcessObject
   void processTick( const Move *move );

   ///
   void attach( const Point3F &start, const Point3F &direction, F32 maxDist );

   ///
   void detach( const Point3F &force = Point3F::Zero );

   ///
   bool isAttached() const { return mBody != NULL; }

protected:

   void _preTick();

   ///
   PhysicsWorld *mWorld;

   F32 mForce;

   ///
   bool mPhysicsTick;

   ///
   WeakRefPtr<PhysicsBody> mBody;

};


#endif // _T3D_PHYSICS_PHYSICSFORCE_H_