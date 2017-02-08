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

#ifndef _PX3BODY_H_
#define _PX3BODY_H_

#ifndef _T3D_PHYSICS_PHYSICSBODY_H_
#include "T3D/physics/physicsBody.h"
#endif
#ifndef _PHYSICS_PHYSICSUSERDATA_H_
#include "T3D/physics/physicsUserData.h"
#endif
#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

class Px3World;
class Px3Collision;
struct Px3CollisionDesc;

namespace physx{
	class PxRigidActor;
	class PxMaterial;
	class PxShape;
}


class Px3Body : public PhysicsBody
{
protected:

   /// The physics world we are in.
   Px3World *mWorld;

   /// The physics actor.
   physx::PxRigidActor *mActor;

   /// The unshared local material used on all the 
   /// shapes on this actor.
   physx::PxMaterial *mMaterial;

   /// We hold the collision reference as it contains
   /// allocated objects that we own and must free.
   StrongRefPtr<Px3Collision> mColShape;

   /// 
   MatrixF mInternalTransform;

   /// The body flags set at creation time.
   U32 mBodyFlags;

   /// Is true if this body is enabled and active
   /// in the simulation of the scene.
   bool mIsEnabled;
   bool mIsStatic;

   ///
   void _releaseActor();

   
public:

   Px3Body();
   virtual ~Px3Body();

   // PhysicsObject
   virtual PhysicsWorld* getWorld();
   virtual void setTransform( const MatrixF &xfm );
   virtual MatrixF& getTransform( MatrixF *outMatrix );
   virtual Box3F getWorldBounds();
   virtual void setSimulationEnabled( bool enabled );
   virtual bool isSimulationEnabled() { return mIsEnabled; }

   // PhysicsBody
   virtual bool init(   PhysicsCollision *shape, 
                        F32 mass,
                        U32 bodyFlags,
                        SceneObject *obj, 
                        PhysicsWorld *world );

   virtual bool isDynamic() const;
   virtual PhysicsCollision* getColShape();
   virtual void setSleepThreshold( F32 linear, F32 angular );
   virtual void setDamping( F32 linear, F32 angular );
   virtual void getState( PhysicsState *outState );
   virtual F32 getMass() const;
   virtual Point3F getCMassPosition() const;
   virtual void setLinVelocity( const Point3F &vel );
   virtual void setAngVelocity( const Point3F &vel );
   virtual Point3F getLinVelocity() const;
   virtual Point3F getAngVelocity() const;
   virtual void setSleeping( bool sleeping );
   virtual void setMaterial(  F32 restitution,
                              F32 friction, 
                              F32 staticFriction );
   virtual void applyCorrection( const MatrixF &xfm );
   virtual void applyImpulse( const Point3F &origin, const Point3F &force );
   virtual void applyTorque( const Point3F &torque );
   virtual void applyForce( const Point3F &force );
   virtual void findContact(SceneObject **contactObject, VectorF *contactNormal,
      Vector<SceneObject*> *outOverlapObjects) const;
   virtual void moveKinematicTo(const MatrixF &xfm);

};

#endif // _PX3BODY_H_
