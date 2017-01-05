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

#ifndef _T3D_PHYSICS_PHYSICSBODY_H_
#define _T3D_PHYSICS_PHYSICSBODY_H_

#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSOBJECT_H_
#include "T3D/physics/physicsObject.h"
#endif

class PhysicsCollision;
class SceneObject;


/// Simple physics object that represents a single rigid body.
class PhysicsBody : public PhysicsObject
{
public:

   virtual ~PhysicsBody() {}

   enum
   {
      /// Marks the body as a trigger object which is only used
      /// to get collision events and not get collision response.
      BF_TRIGGER = BIT( 0 ),

      /// The body is kinematic and assumed to be moved by 
      /// the game code via transforms.
      BF_KINEMATIC = BIT( 1 ),

      /// The body responds to contacts but does not push forces into others.
      BF_DEBRIS = BIT( 2 )
   };

   /// Initialize the body with a collision shape 
   /// and basic physics properties.
   virtual bool init(   PhysicsCollision *shape, 
                        F32 mass,
                        U32 bodyFlags,
                        SceneObject *obj, 
                        PhysicsWorld *world ) = 0;

   /// Returns true if the object is a dynamic rigid body 
   /// animated by the physics simulation.
   ///
   /// Kinematics are not considered to be dynamic.
   ///
   virtual bool isDynamic() const = 0;

   /// Returns the collision shape used to create the body.
   virtual PhysicsCollision* getColShape() = 0;

   ///
   virtual void setSleepThreshold( F32 linear, F32 angular ) = 0;

   ///
   virtual void setDamping( F32 linear, F32 angular ) = 0;

   ///
   virtual void getState( PhysicsState *outState ) = 0;

   ///
   virtual F32 getMass() const = 0;

   ///
   virtual Point3F getCMassPosition() const = 0;

   ///
   virtual void setLinVelocity( const Point3F &vel ) = 0;

   ///
   virtual void setAngVelocity( const Point3F &vel ) = 0;

   ///
   virtual Point3F getLinVelocity() const = 0;

   ///
   virtual Point3F getAngVelocity() const = 0;

   ///
   virtual void setSleeping( bool sleeping ) = 0;

   ///
   virtual void setMaterial(  F32 restitution,
                              F32 friction, 
                              F32 staticFriction ) = 0;

   ///
   virtual void applyCorrection( const MatrixF &xfm ) = 0;

   ///
   virtual void applyImpulse( const Point3F &origin, const Point3F &force ) = 0;

   ///
   virtual void applyTorque( const Point3F &torque ) = 0;

   ///
   virtual void applyForce( const Point3F &force ) = 0;


   virtual void findContact(SceneObject **contactObject,
      VectorF *contactNormal,
      Vector<SceneObject*> *outOverlapObjects) const = 0;

   ///
   virtual void moveKinematicTo(const MatrixF &xfm) = 0;

};


#endif // _T3D_PHYSICS_PHYSICSBODY_H_