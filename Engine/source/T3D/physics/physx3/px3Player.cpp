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
#include "T3D/physics/physx3/px3Player.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physx3/px3World.h"
#include "T3D/physics/physx3/px3Casts.h"
#include "T3D/physics/physx3/px3Utils.h"
#include "collision/collision.h"


Px3Player::Px3Player()
   :  PhysicsPlayer(),
      mController( NULL ),
      mWorld( NULL ),
      mObject( NULL ),
      mSkinWidth( 0.05f ),
      mOriginOffset( 0.0f ),
      mElapsed(0)
{
   PHYSICSMGR->getPhysicsResetSignal().notify( this, &Px3Player::_onPhysicsReset );
}

Px3Player::~Px3Player()
{
   _releaseController();
   PHYSICSMGR->getPhysicsResetSignal().remove( this, &Px3Player::_onPhysicsReset );
}

void Px3Player::_releaseController()
{
   if ( mController )
   {
      mController->getActor()->userData = NULL;
      mWorld->getStaticChangedSignal().remove( this, &Px3Player::_onStaticChanged );
      mController->release();
   }
}

void Px3Player::init( const char *type,
                     const Point3F &size,
                     F32 runSurfaceCos,
                     F32 stepHeight,
                     SceneObject *obj, 
                     PhysicsWorld *world )
{
   AssertFatal( obj, "Px3Player::init - Got a null scene object!" );
   AssertFatal( world, "Px3Player::init - Got a null world!" );
   AssertFatal( dynamic_cast<Px3World*>( world ), "Px3Player::init - The world is the wrong type!" );

   // Cleanup any previous controller.
   _releaseController();

   mObject = obj;
   mWorld = (Px3World*)world;
   mOriginOffset = size.z * 0.5f;

	physx::PxCapsuleControllerDesc desc;
   desc.contactOffset = mSkinWidth;
   desc.radius = getMax( size.x, size.y ) * 0.5f;
   desc.radius -= mSkinWidth;
   desc.height = size.z - ( desc.radius * 2.0f );
   desc.height -= mSkinWidth * 2.0f;
	desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	desc.position.set( 0, 0, 0 );
	desc.upDirection = physx::PxVec3(0,0,1);
	desc.reportCallback = this;
	desc.slopeLimit = runSurfaceCos;
	desc.stepOffset = stepHeight;
	desc.behaviorCallback = NULL;
	desc.material = gPhysics3SDK->createMaterial(0.1f, 0.1f, 0.2f);

	mController = mWorld->createController( desc );

   mWorld->getStaticChangedSignal().notify( this, &Px3Player::_onStaticChanged );
   physx::PxRigidDynamic *kineActor = mController->getActor();

   //player only has one shape
	physx::PxShape *shape = px3GetFirstShape(kineActor);
	physx::PxFilterData colData;
	colData.word0 = PX3_PLAYER;
	shape->setSimulationFilterData(colData);
	shape->setQueryFilterData(colData);

   //store geometry for later use in findContact calls
   shape->getCapsuleGeometry(mGeometry);

   mUserData.setObject( obj );
   kineActor->userData = &mUserData;

}

void Px3Player::_onStaticChanged()
{
   if(mController)
      mController->invalidateCache();
}

void Px3Player::_onPhysicsReset( PhysicsResetEvent reset )
{
   if(mController)
      mController->invalidateCache();
}

Point3F Px3Player::move( const VectorF &disp, CollisionList &outCol )
{
   AssertFatal( mController, "Px3Player::move - The controller is null!" );

   // Return the last position if the simulation is stopped.
   //
   // See PxPlayer::_onPhysicsReset
   if ( !mWorld->isEnabled() )
   {
      Point3F newPos = px3Cast<Point3F>( mController->getPosition() );
      newPos.z -= mOriginOffset;
      return newPos;
   }

   mWorld->releaseWriteLock();

   mCollisionList = &outCol;

   physx::PxVec3 dispNx( disp.x, disp.y, disp.z );
   if (mIsZero(disp.z))
      dispNx.z = 0.0f;
   
   U32 groups = 0xffffffff;
   groups &= ~( PX3_TRIGGER ); // No trigger shapes!
   groups &= ~( PX3_DEBRIS);
   physx::PxControllerFilters filter;
   physx::PxFilterData data;
   data.word0=groups;
   filter.mFilterData = &data;
   filter.mFilterFlags = physx::PxSceneQueryFilterFlags(physx::PxControllerFlag::eCOLLISION_DOWN|physx::PxControllerFlag::eCOLLISION_SIDES|physx::PxControllerFlag::eCOLLISION_UP);

   mController->move( dispNx,0.0001f,0, filter );

   Point3F newPos = px3Cast<Point3F>( mController->getPosition() );
   newPos.z -= mOriginOffset;

   mCollisionList = NULL;

   return newPos;
}

void Px3Player::onShapeHit( const physx::PxControllerShapeHit& hit )
{
   if (!mCollisionList || mCollisionList->getCount() >= CollisionList::MaxCollisions)
      return;

   physx::PxRigidActor *actor = hit.actor;
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );

   // Fill out the Collision 
   // structure for use later.
   Collision &col = mCollisionList->increment();
   dMemset( &col, 0, sizeof( col ) );

   col.normal = px3Cast<Point3F>( hit.worldNormal );
   col.point = px3Cast<Point3F>( hit.worldPos );
   col.distance = hit.length;      
   if ( userData )
      col.object = userData->getObject();

   if (mIsZero(hit.dir.z))
   {
      if (col.normal.z > 0.0f)
      {
         col.normal.z = 0.0f;
         col.normal.normalizeSafe();
      }
   }
   else
   {
      col.normal.set(0.0f, 0.0f, 1.0f);
   }


}

void Px3Player::onControllerHit( const physx::PxControllersHit& hit )
{
   if (!mCollisionList || mCollisionList->getCount() >= CollisionList::MaxCollisions)
      return;

   physx::PxRigidActor *actor = hit.other->getActor();
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );

   // For controller-to-controller hit we don't have an actual hit point, so all
   // we can do is set the hit object.
   Collision &col = mCollisionList->increment();
   dMemset( &col, 0, sizeof( col ) );
   if ( userData )
      col.object = userData->getObject();

}

void Px3Player::findContact(   SceneObject **contactObject, 
                              VectorF *contactNormal, 
                              Vector<SceneObject*> *outOverlapObjects ) const
{
  // Calculate the sweep motion...
   F32 halfCapSize = mOriginOffset;
   F32 halfSmallCapSize = halfCapSize * 0.8f;
   F32 diff = halfCapSize - halfSmallCapSize;

   F32 distance = diff + mSkinWidth + 0.01f; 
   physx::PxVec3 dir(0,0,-1);
 
   physx::PxScene *scene = mWorld->getScene();
   physx::PxHitFlags hitFlags(physx::PxHitFlag::eDEFAULT);
   physx::PxQueryFilterData filterData(physx::PxQueryFlag::eDYNAMIC|physx::PxQueryFlag::eSTATIC);
   filterData.data.word0 = PX3_DEFAULT;
   physx::PxSweepHit sweepHit;
   physx::PxRigidDynamic *actor= mController->getActor();
   physx::PxU32 shapeIndex;

   bool hit = physx::PxRigidBodyExt::linearSweepSingle(*actor,*scene,dir,distance,hitFlags,sweepHit,shapeIndex,filterData);
   if ( hit )
   {
      PhysicsUserData *data = PhysicsUserData::cast( sweepHit.actor->userData);
      if ( data )
      {
         *contactObject = data->getObject();
         *contactNormal = px3Cast<Point3F>( sweepHit.normal );
      }
   }

  // Check for overlapped objects ( triggers )

   if ( !outOverlapObjects )
      return;

   filterData.data.word0 = PX3_TRIGGER;

	const physx::PxU32 bufferSize = 10;
   physx::PxOverlapBufferN<bufferSize> hitBuffer;
	hit = scene->overlap(mGeometry,actor->getGlobalPose(),hitBuffer,filterData);
   if(hit)
   {
      for ( U32 i = 0; i < hitBuffer.nbTouches; i++ )
      {
         PhysicsUserData *data = PhysicsUserData::cast( hitBuffer.touches[i].actor->userData );
         if ( data )      
            outOverlapObjects->push_back( data->getObject() );
      }
   }

}

void Px3Player::enableCollision()
{
   AssertFatal( mController, "Px3Player::enableCollision - The controller is null!" );

   mWorld->releaseWriteLock();
   px3GetFirstShape(mController->getActor())->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,true);
}

void Px3Player::disableCollision()
{
   AssertFatal( mController, "Px3Player::disableCollision - The controller is null!" );

   mWorld->releaseWriteLock();
	px3GetFirstShape(mController->getActor())->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,false); 
}

PhysicsWorld* Px3Player::getWorld()
{
   return mWorld;
}

void Px3Player::setTransform( const MatrixF &transform )
{
   AssertFatal( mController, "Px3Player::setTransform - The controller is null!" );

   mWorld->releaseWriteLock();

   Point3F newPos = transform.getPosition();
   newPos.z += mOriginOffset;

   const Point3F &curPos = px3Cast<Point3F>(mController->getPosition());
   
   if ( !(newPos - curPos ).isZero() )
      mController->setPosition( px3Cast<physx::PxExtendedVec3>(newPos) );
}

MatrixF& Px3Player::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mController, "Px3Player::getTransform - The controller is null!" );

   Point3F newPos = px3Cast<Point3F>( mController->getPosition() );
   newPos.z -= mOriginOffset;
   outMatrix->setPosition( newPos );

   return *outMatrix;
}

void Px3Player::setScale( const Point3F &scale )
{
   //Ignored
}

Box3F Px3Player::getWorldBounds()
{
   physx::PxBounds3 bounds;
   physx::PxRigidDynamic *actor = mController->getActor();
   physx::PxShape *shape = px3GetFirstShape(actor);
   bounds = physx::PxShapeExt::getWorldBounds(*shape,*actor);
   return px3Cast<Box3F>( bounds );
}

bool Px3Player::testSpacials(const Point3F &nPos, const Point3F &nSize) const
{
   F32 offset = nSize.z * 0.5f;
   F32 radius = getMax(nSize.x, nSize.y) * 0.5f - mSkinWidth;
   F32 height = (nSize.z - (radius * 2.0f)) * 0.5f;
   height -= mSkinWidth * 2.0f;
   physx::PxCapsuleGeometry geom(radius, height);

   physx::PxVec3 pos(nPos.x, nPos.y, nPos.z + offset);
   physx::PxQuat orientation(Float_HalfPi, physx::PxVec3(0.0f, 1.0f, 0.0f));

   physx::PxOverlapBuffer hit;
   physx::PxQueryFilterData queryFilter(physx::PxQueryFlag::eANY_HIT | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC);
   queryFilter.data.word0 = PX3_DEFAULT;
   bool hasHit = mWorld->getScene()->overlap(geom, physx::PxTransform(pos, orientation), hit, queryFilter);

   return !hasHit;   // Return true if there are no overlapping objects
}

void Px3Player::setSpacials(const Point3F &nPos, const Point3F &nSize)
{
   mOriginOffset = nSize.z * 0.5f;
   F32 radius = getMax(nSize.x, nSize.y) * 0.5f - mSkinWidth;
   F32 height = nSize.z - (radius * 2.0f);
   height -= mSkinWidth * 2.0f;

   mWorld->releaseWriteLock();
   mController->resize(height);
   px3GetFirstShape(mController->getActor())->getCapsuleGeometry(mGeometry);
}