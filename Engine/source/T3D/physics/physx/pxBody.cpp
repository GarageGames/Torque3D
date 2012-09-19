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
#include "T3D/physics/physX/pxBody.h"

#include "T3D/physics/physX/px.h"
#include "T3D/physics/physX/pxCasts.h"
#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physX/pxCollision.h"


PxBody::PxBody() :
   mActor( NULL ),
   mMaterial( NULL ),
   mWorld( NULL ),
   mBodyFlags( 0 ),
   mIsEnabled( true )
{
}

PxBody::~PxBody()
{
   _releaseActor();
}

void PxBody::_releaseActor()
{
   if ( !mActor )
      return;

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   mActor->userData = NULL;

   mWorld->releaseActor( *mActor );
   mActor = NULL;
   mBodyFlags = 0;

   if ( mMaterial )
   {
      mWorld->releaseMaterial( *mMaterial );
      mMaterial = NULL;
   }

   mColShape = NULL;
}

bool PxBody::init(   PhysicsCollision *shape, 
                     F32 mass,
                     U32 bodyFlags,
                     SceneObject *obj, 
                     PhysicsWorld *world )
{
   AssertFatal( obj, "PxBody::init - Got a null scene object!" );
   AssertFatal( world, "PxBody::init - Got a null world!" );
   AssertFatal( dynamic_cast<PxWorld*>( world ), "PxBody::init - The world is the wrong type!" );
   AssertFatal( shape, "PxBody::init - Got a null collision shape!" );
   AssertFatal( dynamic_cast<PxCollision*>( shape ), "PxBody::init - The collision shape is the wrong type!" );
   AssertFatal( !((PxCollision*)shape)->getShapes().empty(), "PxBody::init - Got empty collision shape!" );
	 
   // Cleanup any previous actor.
   _releaseActor();

   mWorld = (PxWorld*)world;
   mColShape = (PxCollision*)shape;
   mBodyFlags = bodyFlags;

   NxActorDesc actorDesc;
   NxBodyDesc bodyDesc;

   const bool isKinematic = mBodyFlags & BF_KINEMATIC;
   const bool isTrigger = mBodyFlags & BF_TRIGGER;
   const bool isDebris = mBodyFlags & BF_DEBRIS;

   if ( isKinematic )
   {
      // Kinematics are dynamics... so they need
      // a body description.
      actorDesc.body = &bodyDesc;
      bodyDesc.mass = getMax( mass, 1.0f );
	   bodyDesc.flags	|= NX_BF_KINEMATIC;
   }
   else if ( mass > 0.0f )
   {
      // We have mass so its a dynamic.
      actorDesc.body = &bodyDesc;
      bodyDesc.mass = mass;
   }

   if ( isTrigger )
      actorDesc.flags |= NX_AF_DISABLE_RESPONSE;

   // Add all the shapes.
   const Vector<NxShapeDesc*> &shapes = mColShape->getShapes();
   for ( U32 i=0; i < shapes.size(); i++ )
   {
      NxShapeDesc *desc = shapes[i];

      // If this hits then something is broken with 
      // this descrption... check all the fields to be
      // sure their values are correctly filled out.
      AssertFatal( desc->isValid(), "PxBody::init - Got invalid shape description!" );

      if ( isTrigger )
         desc->group = 31;

      if ( isDebris )
         desc->group = 30;

      actorDesc.shapes.push_back( desc );
   }

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   mActor = mWorld->getScene()->createActor( actorDesc );
   mIsEnabled = true;

   if ( isDebris )
      mActor->setDominanceGroup( 31 );

   mUserData.setObject( obj );
   mUserData.setBody( this );
   mActor->userData = &mUserData;

   return true;
}

void PxBody::setMaterial(  F32 restitution,
                           F32 friction, 
                           F32 staticFriction )
{
   AssertFatal( mActor, "PxBody::setMaterial - The actor is null!" );

   // If the body is dynamic then wake it up as
   // it may need to change behavior.
   if ( isDynamic() )
      mActor->wakeUp();

   NxMaterialDesc desc;
   desc.restitution = restitution;
   desc.dynamicFriction = friction;
   desc.staticFriction = staticFriction;

   // If we have a material then just update it as the shapes
   // should already have them mapped.
   if ( mMaterial )
   {
      mMaterial->loadFromDesc( desc );
      return;
   }

   // If we got here then create a new material and
   // assign it to all our shapes.
   mMaterial = mWorld->createMaterial( desc );
   U32 matIndex = mMaterial->getMaterialIndex();
   U32 count = mActor->getNbShapes();
   NxShape*const* shapes = mActor->getShapes();
   for ( U32 i=0; i < count; i++ )
      shapes[i]->setMaterial( matIndex );
}

void PxBody::setSleepThreshold( F32 linear, F32 angular )
{
   AssertFatal( mActor, "PxBody::setSleepThreshold - The actor is null!" );

   mActor->setSleepLinearVelocity( linear );
   mActor->setSleepAngularVelocity( angular );
}

void PxBody::setDamping( F32 linear, F32 angular )
{
   AssertFatal( mActor, "PxBody::setDamping - The actor is null!" );
   mActor->setLinearDamping( linear );
   mActor->setAngularDamping( angular );
}

void PxBody::getState( PhysicsState *outState )
{
   AssertFatal( mActor, "PxBody::getState - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::getState - This call is only for dynamics!" );

   // TODO: Fix this to do what we intended... to return
   // false so that the caller can early out of the state
   // hasn't changed since the last tick.

   outState->position = pxCast<Point3F>( mActor->getGlobalPosition() );
   outState->orientation = pxCast<QuatF>( mActor->getGlobalOrientationQuat() );
   outState->linVelocity = pxCast<Point3F>( mActor->getLinearVelocity() ); 
   outState->angVelocity = pxCast<Point3F>( mActor->getAngularVelocity() ); 
   outState->sleeping = mActor->isSleeping();
   outState->momentum = pxCast<Point3F>( mActor->getLinearMomentum() );
}

F32 PxBody::getMass() const
{
   AssertFatal( mActor, "PxBody::getCMassPosition - The actor is null!" );
   return mActor->getMass();
}

Point3F PxBody::getCMassPosition() const
{
   AssertFatal( mActor, "PxBody::getCMassPosition - The actor is null!" );
   return pxCast<Point3F>( mActor->getCMassGlobalPosition() );
}

void PxBody::setLinVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "PxBody::setLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::setLinVelocity - This call is only for dynamics!" );

   mActor->setLinearVelocity( pxCast<NxVec3>( vel ) );
}

void PxBody::setAngVelocity( const Point3F &vel )
{
   AssertFatal( mActor, "PxBody::setAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::setAngVelocity - This call is only for dynamics!" );

   mActor->setAngularVelocity( pxCast<NxVec3>( vel ) );
}

Point3F PxBody::getLinVelocity() const
{
   AssertFatal( mActor, "PxBody::getLinVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::getLinVelocity - This call is only for dynamics!" );

   return pxCast<Point3F>( mActor->getLinearVelocity() );
}

Point3F PxBody::getAngVelocity() const
{
   AssertFatal( mActor, "PxBody::getAngVelocity - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::getAngVelocity - This call is only for dynamics!" );

   return pxCast<Point3F>( mActor->getAngularVelocity() );
}

void PxBody::setSleeping( bool sleeping )
{
   AssertFatal( mActor, "PxBody::setSleeping - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::setSleeping - This call is only for dynamics!" );

   if ( sleeping )
      mActor->putToSleep();
   else
      mActor->wakeUp();
}

bool PxBody::isDynamic() const
{
   AssertFatal( mActor, "PxBody::isDynamic - The actor is null!" );
   return mActor->isDynamic() && ( mBodyFlags & BF_KINEMATIC ) == 0;
}

PhysicsWorld* PxBody::getWorld() 
{
   return mWorld; 
}

PhysicsCollision* PxBody::getColShape() 
{ 
   return mColShape; 
}

MatrixF& PxBody::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mActor, "PxBody::getTransform - The actor is null!" );

   mActor->getGlobalPose().getRowMajor44( *outMatrix );

   return *outMatrix;
}

Box3F PxBody::getWorldBounds()
{
   AssertFatal( mActor, "PxBody::getTransform - The actor is null!" );
   
   NxBounds3 bounds;
   bounds.setEmpty();   
   NxBounds3 shapeBounds;

   NxShape *const* pShapeArray = mActor->getShapes();
   U32 shapeCount = mActor->getNbShapes();

   for ( U32 i = 0; i < shapeCount; i++ )
   {
      // Get the shape's bounds.
      pShapeArray[i]->getWorldBounds( shapeBounds );

      // Combine them into the total bounds.
      bounds.combine( shapeBounds );   
   }

   return pxCast<Box3F>( bounds );
}

void PxBody::setSimulationEnabled( bool enabled )
{
   if ( mIsEnabled == enabled )
      return;
  
   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   if ( enabled )
   {      
      mIsEnabled = true;
      mActor->clearActorFlag( NX_AF_DISABLE_RESPONSE );
      mActor->clearActorFlag( NX_AF_DISABLE_COLLISION );

      // Don't clear the flag if its supposed to be kinematic.
      if ( !(mBodyFlags & BF_KINEMATIC) )
         mActor->clearBodyFlag( NX_BF_KINEMATIC );

      if ( isDynamic() )
         mActor->wakeUp();
   }
   else
   {
      mIsEnabled = false;
      mActor->raiseActorFlag( NX_AF_DISABLE_RESPONSE );
      mActor->raiseActorFlag( NX_AF_DISABLE_COLLISION );
      mActor->raiseBodyFlag( NX_BF_KINEMATIC );
   }

   NxShape *const* shapes = mActor->getShapes();
   for ( S32 i = 0; i < mActor->getNbShapes(); i++ )
      shapes[i]->setFlag( NX_SF_DISABLE_RAYCASTING, !mIsEnabled );
}

void PxBody::setTransform( const MatrixF &transform )
{
   AssertFatal( mActor, "PxBody::setTransform - The actor is null!" );

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   NxMat34 xfm;
   xfm.setRowMajor44( transform );
   mActor->setGlobalPose( xfm ); 

   // If its dynamic we have more to do.
   if ( mActor->isDynamic() && !mActor->readBodyFlag( NX_BF_KINEMATIC ) )
   {
      mActor->setLinearVelocity( NxVec3( 0, 0, 0 ) );
      mActor->setAngularVelocity( NxVec3( 0, 0, 0 ) );
      mActor->wakeUp();
   }
}

void PxBody::applyCorrection( const MatrixF &transform )
{
   AssertFatal( mActor, "PxBody::applyCorrection - The actor is null!" );
   AssertFatal( isDynamic(), "PxBody::applyCorrection - This call is only for dynamics!" );

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   NxMat34 xfm;
   xfm.setRowMajor44( transform );
   mActor->setGlobalPose( xfm ); 
}

void PxBody::applyImpulse( const Point3F &origin, const Point3F &force )
{
   AssertFatal( mActor, "PxBody::applyImpulse - The actor is null!" );

   // This sucks, but it has to happen if we want
   // to avoid write lock errors from PhysX right now.
   mWorld->releaseWriteLock();

   if ( mIsEnabled && isDynamic() )
      mActor->addForceAtPos(  pxCast<NxVec3>( force ), 
                              pxCast<NxVec3>( origin ),
                              NX_IMPULSE );
}

