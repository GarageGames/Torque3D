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
#include "T3D/physics/physx3/px3Body.h"

#include "T3D/physics/physx3/px3.h"
#include "T3D/physics/physx3/px3Casts.h"
#include "T3D/physics/physx3/px3World.h"
#include "T3D/physics/physx3/px3Collision.h"

#include "console/console.h"
#include "console/consoleTypes.h"


Px3Body::Px3Body() :
   mActor( NULL ),
   mMaterial( NULL ),
   mWorld( NULL ),
   mBodyFlags( 0 ),
   mIsEnabled( true ),
   mIsStatic(false)
{
}

Px3Body::~Px3Body()
{
   _releaseActor();
}

void Px3Body::_releaseActor()
{
   if ( !mActor )
      return;

   mWorld->releaseWriteLock();

   mActor->userData = NULL;

   mActor->release();
   mActor = NULL;
   mBodyFlags = 0;

   if ( mMaterial )
   {
      mMaterial->release();
   }

   mColShape = NULL;
}

bool Px3Body::init(   PhysicsCollision *shape, 
                     F32 mass,
                     U32 bodyFlags,
                     SceneObject *obj, 
                     PhysicsWorld *world )
{
   AssertFatal( obj, "Px3Body::init - Got a null scene object!" );
   AssertFatal( world, "Px3Body::init - Got a null world!" );
   AssertFatal( dynamic_cast<Px3World*>( world ), "Px3Body::init - The world is the wrong type!" );
   AssertFatal( shape, "Px3Body::init - Got a null collision shape!" );
   AssertFatal( dynamic_cast<Px3Collision*>( shape ), "Px3Body::init - The collision shape is the wrong type!" );
   AssertFatal( !((Px3Collision*)shape)->getShapes().empty(), "Px3Body::init - Got empty collision shape!" );
	 
   // Cleanup any previous actor.
   _releaseActor();

   mWorld = (Px3World*)world;
   mColShape = (Px3Collision*)shape;
   mBodyFlags = bodyFlags;

   const bool isKinematic = mBodyFlags & BF_KINEMATIC;
   const bool isTrigger = mBodyFlags & BF_TRIGGER;
   const bool isDebris = mBodyFlags & BF_DEBRIS;

   if ( isKinematic )
   {
		mActor = gPhysics3SDK->createRigidDynamic(physx::PxTransform(physx::PxIDENTITY()));
		physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
		actor->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, true);
		actor->setMass(getMax( mass, 1.0f ));
   }
   else if ( mass > 0.0f )
   {
      mActor = gPhysics3SDK->createRigidDynamic(physx::PxTransform(physx::PxIDENTITY()));
   }
   else
   {
      mActor = gPhysics3SDK->createRigidStatic(physx::PxTransform(physx::PxIDENTITY()));
      mIsStatic = true;
	}

   mMaterial = gPhysics3SDK->createMaterial(0.6f,0.4f,0.1f);
  
   // Add all the shapes.
   const Vector<Px3CollisionDesc*> &shapes = mColShape->getShapes();
   for ( U32 i=0; i < shapes.size(); i++ )
   {
	   Px3CollisionDesc* desc = shapes[i];
	   if( mass > 0.0f )
	   {
			if(desc->pGeometry->getType() == physx::PxGeometryType::eTRIANGLEMESH)
			{
				Con::errorf("PhysX3 Dynamic Triangle Mesh is not supported.");
			}
	   }
	   physx::PxShape * pShape = mActor->createShape(*desc->pGeometry,*mMaterial);
	   physx::PxFilterData colData;
	   if(isDebris)
			colData.word0 = PX3_DEBRIS;
	   else if(isTrigger)
        colData.word0 = PX3_TRIGGER;
	   else
		   colData.word0 = PX3_DEFAULT;

      //set local pose - actor->createShape with a local pose is deprecated in physx 3.3
      pShape->setLocalPose(desc->pose);
      //set the skin width
      pShape->setContactOffset(0.01f);
      pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
      pShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE,true);
      pShape->setSimulationFilterData(colData);
      pShape->setQueryFilterData(colData);
   }

   //mass & intertia has to be set after creating the shape
   if ( mass > 0.0f )
   {
		physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
		physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor,mass);
   }

    // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   mWorld->getScene()->addActor(*mActor);
   mIsEnabled = true;

   if ( isDebris )
     mActor->setDominanceGroup( 31 );

   mUserData.setObject( obj );
   mUserData.setBody( this );
   mActor->userData = &mUserData;

   return true;
}

void Px3Body::setMaterial(  F32 restitution,
                           F32 friction, 
                           F32 staticFriction )
{
   AssertFatal( mActor, "Px3Body::setMaterial - The actor is null!" );

   if ( isDynamic() )
   {
      physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
      actor->wakeUp();
   }

	 mMaterial->setRestitution(restitution);
	 mMaterial->setStaticFriction(staticFriction);
	 mMaterial->setDynamicFriction(friction);

}

void Px3Body::setSleepThreshold( F32 linear, F32 angular )
{
   AssertFatal( mActor, "Px3Body::setSleepThreshold - The actor is null!" );

   if(mIsStatic)
	   return;

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   physx::PxF32 massNormalized= (linear*linear+angular*angular)/2.0f;
   actor->setSleepThreshold(massNormalized);
}

void Px3Body::setDamping( F32 linear, F32 angular )
{
   AssertFatal( mActor, "Px3Body::setDamping - The actor is null!" );
   if(mIsStatic)
	   return;

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   actor->setLinearDamping( linear );
   actor->setAngularDamping( angular );
}

void Px3Body::getState( PhysicsState *outState )
{
   AssertFatal( mActor, "Px3Body::getState - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::getState - This call is only for dynamics!" );

   outState->position = px3Cast<Point3F>( mActor->getGlobalPose().p );
   outState->orientation = px3Cast<QuatF>( mActor->getGlobalPose().q );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   outState->linVelocity = px3Cast<Point3F>( actor->getLinearVelocity() ); 
   outState->angVelocity = px3Cast<Point3F>( actor->getAngularVelocity() );
   outState->sleeping = actor->isSleeping();
   outState->momentum = px3Cast<Point3F>( (1.0f/actor->getMass()) * actor->getLinearVelocity() );

}

F32 Px3Body::getMass() const
{
   AssertFatal( mActor, "PxBody::getCMassPosition - The actor is null!" );
   if(mIsStatic)
	   return 0;

   const physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   return actor->getMass();
}

Point3F Px3Body::getCMassPosition() const
{
   AssertFatal( mActor, "Px3Body::getCMassPosition - The actor is null!" );
   if(mIsStatic)
	   return px3Cast<Point3F>(mActor->getGlobalPose().p);

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   physx::PxTransform pose = actor->getGlobalPose() * actor->getCMassLocalPose();
   return px3Cast<Point3F>(pose.p);
}

void Px3Body::setLinVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "Px3Body::setLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::setLinVelocity - This call is only for dynamics!" );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   actor->setLinearVelocity( px3Cast<physx::PxVec3>( vel ) );
}

void Px3Body::setAngVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "Px3Body::setAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::setAngVelocity - This call is only for dynamics!" );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   actor->setAngularVelocity(px3Cast<physx::PxVec3>( vel ) );
}

Point3F Px3Body::getLinVelocity() const
{
   AssertFatal( mActor, "Px3Body::getLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::getLinVelocity - This call is only for dynamics!" );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   return px3Cast<Point3F>( actor->getLinearVelocity() );
}

Point3F Px3Body::getAngVelocity() const
{
   AssertFatal( mActor, "Px3Body::getAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::getAngVelocity - This call is only for dynamics!" );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   return px3Cast<Point3F>( actor->getAngularVelocity() );
}

void Px3Body::setSleeping( bool sleeping )
{
   AssertFatal( mActor, "Px3Body::setSleeping - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::setSleeping - This call is only for dynamics!" );

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   if ( sleeping )
      actor->putToSleep();
   else
      actor->wakeUp();
}

bool Px3Body::isDynamic() const
{
   AssertFatal( mActor, "PxBody::isDynamic - The actor is null!" );
   return !mIsStatic && ( mBodyFlags & BF_KINEMATIC ) == 0;
}

PhysicsWorld* Px3Body::getWorld() 
{
   return mWorld; 
}

PhysicsCollision* Px3Body::getColShape() 
{ 
   return mColShape; 
}

MatrixF& Px3Body::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mActor, "Px3Body::getTransform - The actor is null!" );

   *outMatrix = px3Cast<MatrixF>(mActor->getGlobalPose());
   return *outMatrix;
}

Box3F Px3Body::getWorldBounds()
{
   AssertFatal( mActor, "Px3Body::getTransform - The actor is null!" );
   
   physx::PxBounds3 bounds;
   bounds.setEmpty();   
   physx::PxBounds3 shapeBounds;
 
  
   U32 shapeCount = mActor->getNbShapes();
	physx::PxShape **shapes = new physx::PxShape*[shapeCount];
	mActor->getShapes(shapes, shapeCount);
   for ( U32 i = 0; i < shapeCount; i++ )
   {
      // Get the shape's bounds.	   
      shapeBounds = physx::PxShapeExt::getWorldBounds(*shapes[i],*mActor);
      // Combine them into the total bounds.
      bounds.include( shapeBounds ); 
   }

   delete [] shapes;

   return px3Cast<Box3F>( bounds );
}

void Px3Body::setSimulationEnabled( bool enabled )
{
   if ( mIsEnabled == enabled )
      return;

   //Don't need to enable/disable eSIMULATION_SHAPE for trigger,it's disabled permanently
   if(mBodyFlags & BF_TRIGGER)
      return;
  
   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   U32 shapeCount = mActor->getNbShapes();
	physx::PxShape **shapes = new physx::PxShape*[shapeCount];
	mActor->getShapes(shapes, shapeCount);
   for ( S32 i = 0; i < mActor->getNbShapes(); i++ )
   {
	   shapes[i]->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,!mIsEnabled);//?????
   }

   delete [] shapes;
}
void Px3Body::setTransform( const MatrixF &transform )
{
   AssertFatal( mActor, "Px3Body::setTransform - The actor is null!" );


   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   
   mActor->setGlobalPose(px3Cast<physx::PxTransform>(transform),false);

   if(mIsStatic)
	   return;

	physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
	bool kinematic = actor->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC;
   // If its dynamic we have more to do.
   if ( isDynamic() && !kinematic )
   {
      actor->setLinearVelocity( physx::PxVec3(0) );
      actor->setAngularVelocity( physx::PxVec3(0) );
      actor->wakeUp();
   }
}

void Px3Body::applyCorrection( const MatrixF &transform )
{
   AssertFatal( mActor, "Px3Body::applyCorrection - The actor is null!" );
   AssertFatal( isDynamic(), "Px3Body::applyCorrection - This call is only for dynamics!" );

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   mActor->setGlobalPose( px3Cast<physx::PxTransform>(transform) ); 
}

void Px3Body::applyImpulse( const Point3F &origin, const Point3F &force )
{
   AssertFatal( mActor, "Px3Body::applyImpulse - The actor is null!" );

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();
   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   if ( mIsEnabled && isDynamic() )
   physx::PxRigidBodyExt::addForceAtPos(*actor,px3Cast<physx::PxVec3>(force),
												px3Cast<physx::PxVec3>(origin),
												physx::PxForceMode::eIMPULSE);

}

void Px3Body::applyTorque( const Point3F &torque )
{
   AssertFatal(mActor, "Px3Body::applyImpulse - The actor is null!");

   mWorld->releaseWriteLock();
   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   if (mIsEnabled && isDynamic())
      actor->addTorque( px3Cast<physx::PxVec3>(torque), physx::PxForceMode::eFORCE, true);
}

void Px3Body::applyForce( const Point3F &force )
{
   AssertFatal(mActor, "Px3Body::applyTorque - The actor is null!");

   mWorld->releaseWriteLock();
   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   if (mIsEnabled && isDynamic())
      actor->addForce( px3Cast<physx::PxVec3>(force), physx::PxForceMode::eFORCE, true);
}

void Px3Body::findContact(SceneObject **contactObject,
   VectorF *contactNormal,
   Vector<SceneObject*> *outOverlapObjects) const
{
}

void Px3Body::moveKinematicTo(const MatrixF &transform)
{
   AssertFatal(mActor, "Px3Body::moveKinematicTo - The actor is null!");

   const bool isKinematic = mBodyFlags & BF_KINEMATIC;
   if (!isKinematic)
   {
      Con::errorf("Px3Body::moveKinematicTo is only for kinematic bodies.");
      return;
   }

   mWorld->lockScene();

   physx::PxRigidDynamic *actor = mActor->is<physx::PxRigidDynamic>();
   actor->setKinematicTarget(px3Cast<physx::PxTransform>(transform));

   mWorld->unlockScene();
}

