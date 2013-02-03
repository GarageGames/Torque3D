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
#include "T3D/physics/physX/pxPlayer.h"

#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physX/pxCasts.h"
#include "collision/collision.h"
//#include "gfx/gfxDrawUtil.h"
//#include "sim/netConnection.h"


PxPlayer::PxPlayer()
   :  PhysicsPlayer(),
      mController( NULL ),
      mWorld( NULL ),
      mObject( NULL ),
      mSkinWidth( 0.1f ),
      mOriginOffset( 0.0f ) 
{
   PHYSICSMGR->getPhysicsResetSignal().notify( this, &PxPlayer::_onPhysicsReset );
}

PxPlayer::~PxPlayer()
{
   _releaseController();
   PHYSICSMGR->getPhysicsResetSignal().remove( this, &PxPlayer::_onPhysicsReset );
}

void PxPlayer::_releaseController()
{
   if ( mController )
   {
      mController->getActor()->userData = NULL;
      mWorld->getStaticChangedSignal().remove( this, &PxPlayer::_onStaticChanged );
      mWorld->releaseController( *mController );
      mController = NULL;
   }
}

void PxPlayer::init( const char *type,
                     const Point3F &size,
                     F32 runSurfaceCos,
                     F32 stepHeight,
                     SceneObject *obj, 
                     PhysicsWorld *world )
{
   AssertFatal( obj, "PxPlayer::init - Got a null scene object!" );
   AssertFatal( world, "PxPlayer::init - Got a null world!" );
   AssertFatal( dynamic_cast<PxWorld*>( world ), "PxPlayer::init - The world is the wrong type!" );

   // Cleanup any previous controller.
   _releaseController();

   mObject = obj;
   mWorld = (PxWorld*)world;
   mOriginOffset = size.z * 0.5f;

   //if ( dStricmp( type, "Capsule" ) == 0 )
   {
      NxCapsuleControllerDesc desc;
      desc.skinWidth = 0.05f; // Expose?
      desc.radius = getMax( size.x, size.y ) * 0.5f;
      desc.radius -= desc.skinWidth;
      desc.height = size.z - ( desc.radius * 2.0f );
      desc.height -= desc.skinWidth * 2.0f;

      desc.climbingMode = CLIMB_CONSTRAINED;
      desc.position.set( 0, 0, 0 );
      desc.upDirection = NX_Z;
      desc.callback = this; // TODO: Fix this as well!
	   desc.slopeLimit = runSurfaceCos;
	   desc.stepOffset = stepHeight;
      mController = mWorld->createController( desc );
   }
   //else
   {
      //mColShape = new btBoxShape( btVector3( 0.5f, 0.5f, 1.0f ) );
      //mOriginOffset = 1.0f;
   }

    mWorld->getStaticChangedSignal().notify( this, &PxPlayer::_onStaticChanged );

   // Put the kinematic actor on group 29.
   NxActor *kineActor = mController->getActor();
   kineActor->setGroup( 29 );
   NxShape *const *shapes = kineActor->getShapes();
   for ( U32 i=0; i < kineActor->getNbShapes(); i++ )
      shapes[i]->setGroup( 29 );

   mUserData.setObject( obj );
   kineActor->userData = &mUserData;
}

void PxPlayer::_onStaticChanged()
{
    mController->reportSceneChanged();
}

void PxPlayer::_onPhysicsReset( PhysicsResetEvent reset )
{
   // The PhysX controller will crash out if it doesn't clear its
   // list of static elements when they are deleted.  By calling this
   // on physics events we clear the cache and we don't get crashes.
   //
   // This all depends on not doing moves and sweeps when the
   // simulation is paused... we need to stop operating.

   if ( mController )
      mController->reportSceneChanged();
}

Point3F PxPlayer::move( const VectorF &disp, CollisionList &outCol )
{
   AssertFatal( mController, "PxPlayer::move - The controller is null!" );

   // Return the last position if the simulation is stopped.
   //
   // See PxPlayer::_onPhysicsReset
   if ( !mWorld->isEnabled() )
   {
      Point3F newPos = pxCast<Point3F>( mController->getDebugPosition() );
      newPos.z -= mOriginOffset;
      //outCol->point = newPos;
      //outCol->normal.set( 0, 0, 1 );
      return newPos;
   }

   mWorld->releaseWriteLock();

   mCollisionList = &outCol;

   // PhysX 2.8.4 checks up an up displacement and if found will assume
   // the player is flying and remove the step offset.  If we have a small
   // z displacement here, zero it out.
   NxVec3 dispNx( disp.x, disp.y, disp.z );
   if (mIsZero(disp.z))
      dispNx.z = 0.0f;
   
   NxU32 activeGroups = 0xFFFFFFFF;
   activeGroups &= ~( 1<<31 ); // Skip activeGroup for triggers ( 31 )
   activeGroups &= ~( 1<<30 ); // Skip activeGroup for debris / non interactive dynamics ( 30 )

   NxU32 collisionFlags = NXCC_COLLISION_SIDES | NXCC_COLLISION_DOWN | NXCC_COLLISION_UP;

   mController->move( dispNx, activeGroups, 0.0001f, collisionFlags );

   Point3F newPos = pxCast<Point3F>( mController->getDebugPosition() );
   newPos.z -= mOriginOffset;

   mCollisionList = NULL;

   return newPos;
}

NxControllerAction PxPlayer::onShapeHit( const NxControllerShapeHit& hit )
{
   if (!mCollisionList || mCollisionList->getCount() >= CollisionList::MaxCollisions)
      return NX_ACTION_NONE;

   NxActor *actor = &hit.shape->getActor();
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );

   if ( actor->readActorFlag( NX_AF_DISABLE_RESPONSE ) )
      return NX_ACTION_NONE;

   // Fill out the Collision 
   // structure for use later.
   Collision &col = mCollisionList->increment();
   dMemset( &col, 0, sizeof( col ) );

   col.normal = pxCast<Point3F>( hit.worldNormal );
   col.point.set( hit.worldPos.x, hit.worldPos.y, hit.worldPos.z );
   col.distance = hit.length;      
   if ( userData )
      col.object = userData->getObject();

   // If the collision direction is sideways then modify the collision normal
   // to remove any z component.  This takes care of any sideways collisions
   // with the round bottom of the capsule when it comes to the Player class
   // velocity calculations.  We want all sideways collisions to be treated
   // as if they hit the side of a cylinder.
   if (mIsZero(hit.dir.z))
   {
      if (col.normal.z > 0.0f)
      {
         // This will only remove the z component of the collision normal
         // for the bottom of the character controller, which would hit during
         // a step.  We'll leave the top hemisphere of the character's capsule
         // alone as bumping one's head is an entirely different story.  This
         // helps with low doorways.
         col.normal.z = 0.0f;
         col.normal.normalizeSafe();
      }
   }
   else
   {
      // PhysX doesn't perform callbacks in its upwards collision check so if
      // this isn't a sideways collision then it must be a downwards one.  In this
      // case we want to have the collision normal only point in the opposite direction.
      // i.e. up  If we include the sideways part of the normal then the Player class
      // velocity calculations using this normal will affect the player's forwards
      // momentum.  This is especially noticable on stairs as the rounded bottom of
      // the capsule slides up the corner of a stair.
      col.normal.set(0.0f, 0.0f, 1.0f);
   }

   /*
   if ( userData && 
        userData->mCanPush &&
        actor->isDynamic() && 
        !actor->readBodyFlag( NX_BF_KINEMATIC ) &&
        !mDummyMove )
   {
      NxActor *ctrlActor = mController->getActor();   

      // So the object is neither
      // a static or a kinematic,
      // meaning we need to figure out
      // if we have enough force to push it.
      
      // Get the hit object's force
      // and scale it by the amount
      // that it's acceleration is going
      // against our acceleration.
      const Point3F &hitObjLinVel = pxCast<Point3F>( actor->getLinearVelocity() );

      F32 hitObjMass = actor->getMass();

      VectorF hitObjDeltaVel = hitObjLinVel * TickSec;
      VectorF hitObjAccel = hitObjDeltaVel / TickSec;
      
      VectorF controllerLinVel = pxCast<Point3F>( controllerActor->getLinearVelocity() );
      VectorF controllerDeltaVel = controllerLinVel * TickSec;
      VectorF controllerAccel = controllerDeltaVel / TickSec;

      Point3F hitObjForce = (hitObjMass * hitObjAccel);
      Point3F playerForce = (controllerActor->getMass() * controllerAccel);

      VectorF normalizedObjVel( hitObjLinVel );
      normalizedObjVel.normalizeSafe();

      VectorF normalizedPlayerVel( pxCast<Point3F>( controllerActor->getLinearVelocity() ) );
      normalizedPlayerVel.normalizeSafe();

      F32 forceDot = mDot( normalizedObjVel, normalizedPlayerVel );

      hitObjForce *= forceDot;

      playerForce = playerForce - hitObjForce;

      if ( playerForce.x > 0.0f || playerForce.y > 0.0f || playerForce.z > 0.0f ) 
         actor->addForceAtPos( NxVec3( playerForce.x, playerForce.y, playerForce.z ), actor->getCMassGlobalPosition() );

      //Con::printf( "onShapeHit: %f %f %f", playerForce.x, playerForce.y, playerForce.z );
   }
   */

   return NX_ACTION_PUSH;
}

NxControllerAction PxPlayer::onControllerHit( const NxControllersHit& hit )
{
   if (!mCollisionList || mCollisionList->getCount() >= CollisionList::MaxCollisions)
      return NX_ACTION_NONE;

   NxActor *actor = hit.other->getActor();
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );

   if ( actor->readActorFlag( NX_AF_DISABLE_RESPONSE ) )
      return NX_ACTION_NONE;

   // For controller-to-controller hit we don't have an actual hit point, so all
   // we can do is set the hit object.
   Collision &col = mCollisionList->increment();
   dMemset( &col, 0, sizeof( col ) );
   if ( userData )
      col.object = userData->getObject();

   return NX_ACTION_NONE;
}

void PxPlayer::findContact(   SceneObject **contactObject, 
                              VectorF *contactNormal, 
                              Vector<SceneObject*> *outOverlapObjects ) const
{
   AssertFatal( mController, "PxPlayer::findContact - The controller is null!" );

   // See PxPlayer::_onPhysicsReset
   if ( !mWorld->isEnabled() )
      return;

   // Calculate the sweep motion...
   F32 halfCapSize = mOriginOffset;
   F32 halfSmallCapSize = halfCapSize * 0.8f;
   F32 diff = halfCapSize - halfSmallCapSize;

   const F32 mSkinWidth = 0.1f;

   F32 offsetDist = diff + mSkinWidth + 0.01f; 
   NxVec3 motion(0,0,-offsetDist);

   /*
   // Construct the capsule...
   F32 radius = mCapsuleController->getRadius();
   F32 halfHeight = mCapsuleController->getHeight() * 0.5f;

   NxCapsule capsule;
   capsule.p0 = capsule.p1 = pxCast<NxVec3>( mCapsuleController->getDebugPosition() );
   capsule.p0.z -= halfHeight;
   capsule.p1.z += halfHeight;
   capsule.radius = radius;
   */

   NxSweepQueryHit sweepHit;
   NxU32 hitCount = mController->getActor()->linearSweep( motion, NX_SF_STATICS | NX_SF_DYNAMICS, NULL, 1, &sweepHit, NULL );

   if ( hitCount > 0 )
   {
      PhysicsUserData *data = PhysicsUserData::cast( sweepHit.hitShape->getActor().userData );
      if ( data )
      {
         *contactObject = data->getObject();
         *contactNormal = pxCast<Point3F>( sweepHit.normal );
      }
   }

   // Check for overlapped objects ( triggers )

   if ( !outOverlapObjects )
      return;

   NxCapsuleShape *shape = reinterpret_cast<NxCapsuleShape*>( mController->getActor()->getShapes()[0] );
   NxCapsule worldCapsule;
   shape->getWorldCapsule( worldCapsule );

   // Test only against activeGroup with triggers ( 31 ).
   NxU32 activeGroups = 1 << 31;

   NxShape *shapes[10];

   hitCount = mWorld->getScene()->overlapCapsuleShapes( worldCapsule, NX_ALL_SHAPES, 10, shapes, NULL, activeGroups );

   for ( S32 i = 0; i < hitCount; i++ )
   {
      PhysicsUserData *data = PhysicsUserData::cast( shapes[i]->getActor().userData );
      if ( data )      
         outOverlapObjects->push_back( data->getObject() );      
   }
}

void PxPlayer::enableCollision()
{
   AssertFatal( mController, "PxPlayer::enableCollision - The controller is null!" );

   mWorld->releaseWriteLock();
   mController->setCollision( true );   
}

void PxPlayer::disableCollision()
{
   AssertFatal( mController, "PxPlayer::disableCollision - The controller is null!" );

   mWorld->releaseWriteLock();
   mController->setCollision( false );   
}

PhysicsWorld* PxPlayer::getWorld()
{
   return mWorld;
}

void PxPlayer::setTransform( const MatrixF &transform )
{
   AssertFatal( mController, "PxPlayer::setTransform - The controller is null!" );

   mWorld->releaseWriteLock();

   Point3F newPos = transform.getPosition();
   newPos.z += mOriginOffset;

   const Point3F &curPos = pxCast<Point3F>(mController->getDebugPosition());
   
   if ( !(newPos - curPos ).isZero() )
      mController->setPosition( pxCast<NxExtendedVec3>(newPos) );
}

MatrixF& PxPlayer::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mController, "PxPlayer::getTransform - The controller is null!" );

   Point3F newPos = pxCast<Point3F>( mController->getDebugPosition() );
   newPos.z -= mOriginOffset;
   outMatrix->setPosition( newPos );

   return *outMatrix;
}

void PxPlayer::setScale( const Point3F &scale )
{
}

Box3F PxPlayer::getWorldBounds()
{
   Con::warnf( "PxPlayer::getWorldBounds - not implemented" );
   return Box3F::Invalid;
}