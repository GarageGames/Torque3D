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
#include "T3D/physics/bullet/btBody.h"

#include "T3D/physics/bullet/bt.h"
#include "T3D/physics/bullet/btCasts.h"
#include "T3D/physics/bullet/btWorld.h"
#include "T3D/physics/bullet/btCollision.h"
#include "math/mBox.h"
#include "console/console.h"


BtBody::BtBody() :
   mActor( NULL ),
   mWorld( NULL ),
   mMass( 0.0f ),
   mCompound( NULL ),
   mCenterOfMass( NULL ),
   mInvCenterOfMass( NULL ),
   mIsDynamic( false ),
   mIsEnabled( false )
{
}

BtBody::~BtBody()
{
   _releaseActor();
}

void BtBody::_releaseActor()
{
   if ( mActor )
   {
      mWorld->getDynamicsWorld()->removeRigidBody( mActor );
      mActor->setUserPointer( NULL );
      SAFE_DELETE( mActor );
   }

   SAFE_DELETE( mCompound );
   SAFE_DELETE( mCenterOfMass );
   SAFE_DELETE( mInvCenterOfMass );
   
   mColShape = NULL;
}

bool BtBody::init(   PhysicsCollision *shape, 
                        F32 mass,
                        U32 bodyFlags,
                        SceneObject *obj, 
                        PhysicsWorld *world )
{
   AssertFatal( obj, "BtBody::init - Got a null scene object!" );
   AssertFatal( world, "BtBody::init - Got a null world!" );
   AssertFatal( dynamic_cast<BtWorld*>( world ), "BtBody::init - The world is the wrong type!" );
   AssertFatal( shape, "BtBody::init - Got a null collision shape!" );
   AssertFatal( dynamic_cast<BtCollision*>( shape ), "BtBody::init - The collision shape is the wrong type!" );
   AssertFatal( ((BtCollision*)shape)->getShape(), "BtBody::init - Got empty collision shape!" );
	 
   // Cleanup any previous actor.
   _releaseActor();

   mWorld = (BtWorld*)world;

   mColShape = (BtCollision*)shape;
   btCollisionShape *btColShape = mColShape->getShape();
   MatrixF localXfm = mColShape->getLocalTransform();
   btVector3 localInertia( 0, 0, 0 );   

   // If we have a mass then we're dynamic.
   mIsDynamic = mass > 0.0f;
   if ( mIsDynamic )
   {
      if ( btColShape->isCompound() )
      {
         btCompoundShape *btCompound = (btCompoundShape*)btColShape;

         btScalar *masses = new btScalar[ btCompound->getNumChildShapes() ];
         for ( U32 j=0; j < btCompound->getNumChildShapes(); j++ )
	         masses[j] = mass / btCompound->getNumChildShapes();

         btVector3 principalInertia;
         btTransform principal;
         btCompound->calculatePrincipalAxisTransform( masses, principal, principalInertia );
         delete [] masses;

	      // Create a new compound with the shifted children.
	      btColShape = mCompound = new btCompoundShape();
	      for ( U32 i=0; i < btCompound->getNumChildShapes(); i++ )
	      {
		      btTransform newChildTransform = principal.inverse() * btCompound->getChildTransform(i);
		      mCompound->addChildShape( newChildTransform, btCompound->getChildShape(i) );
	      }

         localXfm = btCast<MatrixF>( principal );
      }

      // Note... this looks like we're changing the shape, but 
      // we're not.  All this does is ask the shape to calculate the
      // local inertia vector from the mass... the shape doesn't change.
      btColShape->calculateLocalInertia( mass, localInertia );
   }

   // If we have a local transform then we need to
   // store it and the inverse to offset the center
   // of mass from the graphics origin.
   if ( !localXfm.isIdentity() )
   {
      mCenterOfMass = new MatrixF( localXfm );
      mInvCenterOfMass = new MatrixF( *mCenterOfMass );
      mInvCenterOfMass->inverse();
   }

   mMass = mass;
   mActor = new btRigidBody( mass, NULL, btColShape, localInertia );
   
   int btFlags = mActor->getCollisionFlags();

   if ( bodyFlags & BF_TRIGGER )
      btFlags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
   if ( bodyFlags & BF_KINEMATIC )
   {
      btFlags &= ~btCollisionObject::CF_STATIC_OBJECT;
      btFlags |= btCollisionObject::CF_KINEMATIC_OBJECT;
   }

   mActor->setCollisionFlags( btFlags );

   mWorld->getDynamicsWorld()->addRigidBody( mActor );
   mIsEnabled = true;

   mUserData.setObject( obj );
   mUserData.setBody( this );
   mActor->setUserPointer( &mUserData );

   return true;
}

void BtBody::setMaterial(  F32 restitution,
                           F32 friction, 
                           F32 staticFriction )
{
   AssertFatal( mActor, "BtBody::setMaterial - The actor is null!" );

   mActor->setRestitution( restitution );

   // TODO: Weird.. Bullet doesn't have seperate dynamic 
   // and static friction.
   //
   // Either add it and submit it as an official patch
   // or hack it via contact reporting or something
   // like that.

   mActor->setFriction( friction );

   // Wake it up... it may need to move.
   mActor->activate();
}

void BtBody::setSleepThreshold( F32 linear, F32 angular )
{
   AssertFatal( mActor, "BtBody::setSleepThreshold - The actor is null!" );
   mActor->setSleepingThresholds( linear, angular );
}

void BtBody::setDamping( F32 linear, F32 angular )
{
   AssertFatal( mActor, "BtBody::setDamping - The actor is null!" );
   mActor->setDamping( linear, angular );
}

void BtBody::getState( PhysicsState *outState )
{
   AssertFatal( isDynamic(), "BtBody::getState - This call is only for dynamics!" );

   // TODO: Fix this to do what we intended... to return
   // false so that the caller can early out of the state
   // hasn't changed since the last tick.

   MatrixF trans;
   if ( mInvCenterOfMass )
      trans.mul( btCast<MatrixF>( mActor->getCenterOfMassTransform() ), *mInvCenterOfMass );
   else
      trans = btCast<MatrixF>( mActor->getCenterOfMassTransform() );

   outState->position = trans.getPosition();
   outState->orientation.set( trans );
   outState->linVelocity = btCast<Point3F>( mActor->getLinearVelocity() ); 
   outState->angVelocity = btCast<Point3F>( mActor->getAngularVelocity() ); 
   outState->sleeping = !mActor->isActive();

   // Bullet doesn't keep the momentum... recalc it.
   outState->momentum = ( 1.0f / mActor->getInvMass() ) * outState->linVelocity;
}

Point3F BtBody::getCMassPosition() const
{
   AssertFatal( mActor, "BtBody::getCMassPosition - The actor is null!" );
   return btCast<Point3F>( mActor->getCenterOfMassTransform().getOrigin() );
}

void BtBody::setLinVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "BtBody::setLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::setLinVelocity - This call is only for dynamics!" );

   mActor->setLinearVelocity( btCast<btVector3>( vel ) );
}

void BtBody::setAngVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "BtBody::setAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::setAngVelocity - This call is only for dynamics!" );

   mActor->setAngularVelocity( btCast<btVector3>( vel ) );
}

Point3F BtBody::getLinVelocity() const
{
   AssertFatal( mActor, "BtBody::getLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::getLinVelocity - This call is only for dynamics!" );

   return btCast<Point3F>( mActor->getLinearVelocity() );
}

Point3F BtBody::getAngVelocity() const
{
   AssertFatal( mActor, "BtBody::getAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::getAngVelocity - This call is only for dynamics!" );

   return btCast<Point3F>( mActor->getAngularVelocity() );
}

void BtBody::setSleeping( bool sleeping )
{
   AssertFatal( mActor, "BtBody::setSleeping - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::setSleeping - This call is only for dynamics!" );

   if ( sleeping )
   {
      //mActor->setCollisionFlags( mActor->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
      mActor->setActivationState( WANTS_DEACTIVATION );
      mActor->setDeactivationTime( 0.0f );
   }
   else
   {
      //mActor->setCollisionFlags( mActor->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT );
      mActor->activate();
   }
}

PhysicsWorld* BtBody::getWorld() 
{
   return mWorld; 
}

PhysicsCollision* BtBody::getColShape() 
{
   return mColShape;
}

MatrixF& BtBody::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mActor, "BtBody::getTransform - The actor is null!" );

   if ( mInvCenterOfMass )
      outMatrix->mul( *mInvCenterOfMass, btCast<MatrixF>( mActor->getCenterOfMassTransform() ) );
   else
      *outMatrix = btCast<MatrixF>( mActor->getCenterOfMassTransform() );

   return *outMatrix;
}

void BtBody::setTransform( const MatrixF &transform )
{
   AssertFatal( mActor, "BtBody::setTransform - The actor is null!" );

   if ( mCenterOfMass )
   {
      MatrixF xfm;
      xfm.mul( transform, *mCenterOfMass );
      mActor->setCenterOfMassTransform( btCast<btTransform>( xfm ) ); 
   }
   else
      mActor->setCenterOfMassTransform( btCast<btTransform>( transform ) ); 

   // If its dynamic we have more to do.
   if ( isDynamic() )
   {
      // Clear any velocity and forces... this is a warp.
      mActor->clearForces();
      mActor->setLinearVelocity( btVector3( 0, 0, 0 ) );
      mActor->setAngularVelocity( btVector3( 0, 0, 0 ) );
      mActor->activate();
   }
}

void BtBody::applyCorrection( const MatrixF &transform )
{
   AssertFatal( mActor, "BtBody::applyCorrection - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::applyCorrection - This call is only for dynamics!" );

   if ( mCenterOfMass )
   {
      MatrixF xfm;
      xfm.mul( transform, *mCenterOfMass );
      mActor->setCenterOfMassTransform( btCast<btTransform>( xfm ) ); 
   }
   else
      mActor->setCenterOfMassTransform( btCast<btTransform>( transform ) ); 
}

void BtBody::applyImpulse( const Point3F &origin, const Point3F &force )
{
   AssertFatal( mActor, "BtBody::applyImpulse - The actor is null!" );
   AssertFatal( isDynamic(), "BtBody::applyImpulse - This call is only for dynamics!" );

   // Convert the world position to local
   MatrixF trans = btCast<MatrixF>( mActor->getCenterOfMassTransform() );
   trans.inverse();
   Point3F localOrigin( origin );
   trans.mulP( localOrigin );

   if ( mCenterOfMass )
   {
      Point3F relOrigin( localOrigin );
      mCenterOfMass->mulP( relOrigin );
      Point3F relForce( force );
      mCenterOfMass->mulV( relForce );
      mActor->applyImpulse( btCast<btVector3>( relForce ), btCast<btVector3>( relOrigin ) );
   }
   else
      mActor->applyImpulse( btCast<btVector3>( force ), btCast<btVector3>( localOrigin ) );

   if ( !mActor->isActive() )
      mActor->activate();
}

void BtBody::applyTorque( const Point3F &torque )
{
   AssertFatal(mActor, "BtBody::applyTorque - The actor is null!");
   AssertFatal(isDynamic(), "BtBody::applyTorque - This call is only for dynamics!");

   mActor->applyTorque( btCast<btVector3>(torque) );

   if (!mActor->isActive())
      mActor->activate();
}

void BtBody::applyForce( const Point3F &force )
{
   AssertFatal(mActor, "BtBody::applyForce - The actor is null!");
   AssertFatal(isDynamic(), "BtBody::applyForce - This call is only for dynamics!");

   if (mCenterOfMass)
   {
      Point3F relForce(force);
      mCenterOfMass->mulV(relForce);
      mActor->applyCentralForce(btCast<btVector3>(relForce));
   }
   else
      mActor->applyCentralForce(btCast<btVector3>(force));

   if (!mActor->isActive())
      mActor->activate();
}

Box3F BtBody::getWorldBounds()
{   
   btVector3 min, max;
   mActor->getAabb( min, max );

   Box3F bounds( btCast<Point3F>( min ), btCast<Point3F>( max ) );

   return bounds;
}

void BtBody::setSimulationEnabled( bool enabled )
{
   if ( mIsEnabled == enabled )
      return;

   if ( !enabled )
      mWorld->getDynamicsWorld()->removeRigidBody( mActor );
   else
      mWorld->getDynamicsWorld()->addRigidBody( mActor );

   mIsEnabled = enabled;
}

void BtBody::findContact(SceneObject **contactObject,
   VectorF *contactNormal,
   Vector<SceneObject*> *outOverlapObjects) const
{
}

void BtBody::moveKinematicTo(const MatrixF &transform)
{
   AssertFatal(mActor, "BtBody::moveKinematicTo - The actor is null!");

   U32 bodyflags = mActor->getCollisionFlags();
   const bool isKinematic = bodyflags & BF_KINEMATIC;
   if (!isKinematic)
   {
      Con::errorf("BtBody::moveKinematicTo is only for kinematic bodies.");
      return;
   }

   if (mCenterOfMass)
   {
      MatrixF xfm;
      xfm.mul(transform, *mCenterOfMass);
      mActor->setCenterOfMassTransform(btCast<btTransform>(xfm));
   }
   else
      mActor->setCenterOfMassTransform(btCast<btTransform>(transform));
}