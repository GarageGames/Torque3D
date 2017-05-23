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

#ifndef _T3D_PHYSICS_BTBODY_H_
#define _T3D_PHYSICS_BTBODY_H_

#ifndef _T3D_PHYSICS_PHYSICSBODY_H_
#include "T3D/physics/physicsBody.h"
#endif
#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

class BtWorld;
class btRigidBody;
class btCompoundShape;
class BtCollision;


class BtBody : public PhysicsBody
{
protected:

   /// The physics world we are in.
   BtWorld *mWorld;

   /// The physics actor.
   btRigidBody *mActor;

   /// The collision representation.
   StrongRefPtr<BtCollision> mColShape;

   /// Our local compound if we had to adjust
   /// the mass center on a dynamic.
   btCompoundShape *mCompound;

   ///
   F32 mMass;

   ///
   bool mIsDynamic;

   /// Is the body participating in the physics simulation.
   bool mIsEnabled;

   /// The center of mass offset used if the graphical
   /// transform is not at the mass center.
   MatrixF *mCenterOfMass;

   /// The inverse center of mass offset.
   MatrixF *mInvCenterOfMass;

   ///
   void _releaseActor();

public:

   BtBody();
   virtual ~BtBody();

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
   virtual bool isDynamic() const { return mIsDynamic; }
   virtual PhysicsCollision* getColShape();
   virtual void setSleepThreshold( F32 linear, F32 angular );
   virtual void setDamping( F32 linear, F32 angular );
   virtual void getState( PhysicsState *outState );
   virtual F32 getMass() const { return mMass; }
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
   virtual void findContact(SceneObject **contactObject, VectorF *contactNormal, Vector<SceneObject*> *outOverlapObjects) const;
   virtual void moveKinematicTo(const MatrixF &xfm);

};

#endif // _T3D_PHYSICS_BTBODY_H_
