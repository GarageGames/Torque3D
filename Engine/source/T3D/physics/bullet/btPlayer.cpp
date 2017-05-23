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
#include "T3D/physics/bullet/btPlayer.h"

#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/bullet/btWorld.h"
#include "T3D/physics/bullet/btCasts.h"
#include "collision/collision.h"

BtPlayer::BtPlayer()
   :  PhysicsPlayer(),
      mWorld( NULL ),
      mObject( NULL ),
      mGhostObject( NULL ),
      mColShape( NULL ),
      mOriginOffset( 0.0f )
{
}

BtPlayer::~BtPlayer()
{
   _releaseController();
}

void BtPlayer::_releaseController()
{
   if ( !mGhostObject )
      return;

   mWorld->getDynamicsWorld()->removeCollisionObject( mGhostObject );

   SAFE_DELETE( mGhostObject );
   SAFE_DELETE( mColShape );
}

void BtPlayer::init( const char *type, 
                     const Point3F &size,
                     F32 runSurfaceCos,
                     F32 stepHeight,
                     SceneObject *obj, 
                     PhysicsWorld *world )
{
   AssertFatal( obj, "BtPlayer::init - Got a null scene object!" );
   AssertFatal( world, "BtPlayer::init - Got a null world!" );
   AssertFatal( dynamic_cast<BtWorld*>( world ), "BtPlayer::init - The world is the wrong type!" );

   // Cleanup any previous controller.
   _releaseController();

   mObject = obj;
   mWorld = (BtWorld*)world;

   mSlopeAngle = runSurfaceCos;
   mStepHeight = stepHeight;

   //if ( dStricmp( type, "Capsule" ) == 0 )
   {
      F32 radius = getMax( size.x, size.y ) * 0.5f;
      F32 height = size.z - ( radius * 2.0f );
      mColShape = new btCapsuleShapeZ( radius, height );
      mColShape->setMargin( 0.05f );
      mOriginOffset = ( height * 0.5 ) + radius;
   }
   //else
   {
      //mColShape = new btBoxShape( btVector3( 0.5f, 0.5f, 1.0f ) );
      //mOriginOffset = 1.0f;
   }

   mGhostObject = new btPairCachingGhostObject();
   mGhostObject->setCollisionShape( mColShape );
   mGhostObject->setCollisionFlags( btCollisionObject::CF_CHARACTER_OBJECT );
   mWorld->getDynamicsWorld()->addCollisionObject( mGhostObject,
                                                   btBroadphaseProxy::CharacterFilter, 
                                                   btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter );
   
   mUserData.setObject( obj );
   mGhostObject->setUserPointer( &mUserData );
}

Point3F BtPlayer::move( const VectorF &disp, CollisionList &outCol )
{
   AssertFatal( mGhostObject, "BtPlayer::move - The controller is null!" );

   if (!mWorld->isEnabled())
   {
      btTransform currentTrans = mGhostObject->getWorldTransform();
      btVector3 currentPos = currentTrans.getOrigin();

      Point3F returnPos = btCast<Point3F>(currentPos);
     
      returnPos.z -= mOriginOffset;
      return returnPos;
   }

   // First recover from any penetrations from the previous tick.
   U32 numPenetrationLoops = 0;
   bool touchingContact = false;
   while ( _recoverFromPenetration() )
   {
      numPenetrationLoops++;
      touchingContact = true;
      if ( numPenetrationLoops > 4 )
         break;
   }

   btTransform newTrans = mGhostObject->getWorldTransform();
   btVector3 newPos = newTrans.getOrigin();

   // The move consists of 3 steps... the up step, the forward 
   // step, and the down step.

   btVector3 forwardSweep( disp.x, disp.y, 0.0f );
   const bool hasForwardSweep = forwardSweep.length2() > 0.0f;
   F32 upSweep = 0.0f;
   F32 downSweep = 0.0f;
	if ( disp[2] < 0.0f )
      downSweep = disp[2];
	else								
      upSweep = disp[2];

	// Only do auto stepping if the character is moving forward.
   F32 stepOffset = mStepHeight;
	if ( hasForwardSweep )
		upSweep += stepOffset;

   // First we do the up step which includes the passed in
   // upward displacement as well as the auto stepping.
   if (  upSweep > 0.0f &&
         _sweep( &newPos, btVector3( 0.0f, 0.0f, upSweep ), NULL ) )
   {
      // Keep track of how far we actually swept to make sure
      // we do not remove too much in the down sweep.
		F32 delta = newPos[2] - newTrans.getOrigin()[2];
		if ( delta < stepOffset )
			stepOffset = delta;
   }

   // Now do the forward step.
   _stepForward( &newPos, forwardSweep, &outCol );

   // Now remove what remains of our auto step 
   // from the down sweep.
	if ( hasForwardSweep )
		downSweep -= stepOffset;

   // Do the downward sweep.
   if ( downSweep < 0.0f )
      _sweep( &newPos, btVector3( 0.0f, 0.0f, downSweep ), &outCol );

   // Finally update the ghost with its new position.
	newTrans.setOrigin( newPos );
	mGhostObject->setWorldTransform( newTrans );

   // Return the current position of the ghost.
   newPos[2] -= mOriginOffset;
   return btCast<Point3F>( newPos );
}

bool BtPlayer::_recoverFromPenetration()
{
	bool penetration = false;

   btDynamicsWorld *collWorld = mWorld->getDynamicsWorld();

	collWorld->getDispatcher()->dispatchAllCollisionPairs(   mGhostObject->getOverlappingPairCache(), 
                                                            collWorld->getDispatchInfo(), 
                                                            collWorld->getDispatcher() );

   btVector3 currPos = mGhostObject->getWorldTransform().getOrigin();
	btScalar maxPen = 0.0f;
   btManifoldArray manifoldArray;

	for ( U32 i = 0; i < mGhostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++ )
	{
		btBroadphasePair *collisionPair = &mGhostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];

		if (  ((btCollisionObject*)collisionPair->m_pProxy0->m_clientObject)->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE ||
            ((btCollisionObject*)collisionPair->m_pProxy1->m_clientObject)->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE )
         continue;

		manifoldArray.resize(0);
		if (collisionPair->m_algorithm)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for ( U32 j=0; j < manifoldArray.size(); j++ )
		{
			btPersistentManifold* manifold = manifoldArray[j];
         btScalar directionSign = manifold->getBody0() == mGhostObject ? -1.0f : 1.0f;

         for ( U32 p=0; p < manifold->getNumContacts(); p++ )
			{
				const btManifoldPoint&pt = manifold->getContactPoint(p);

				if ( pt.getDistance() < -mColShape->getMargin() )
				{
					if ( pt.getDistance() < maxPen )
					{
						maxPen = pt.getDistance();
						//m_touchingNormal = pt.m_normalWorldOnB * directionSign;//??
					}

					currPos += pt.m_normalWorldOnB * directionSign * pt.getDistance(); // * 0.25f;
					penetration = true;
				} 
            else 
            {
					//printf("touching %f\n", pt.getDistance());
				}
			}

			//manifold->clearManifold();
		}
	}

   // Update the ghost transform.
   btTransform newTrans = mGhostObject->getWorldTransform();
	newTrans.setOrigin( currPos );
	mGhostObject->setWorldTransform( newTrans );

	return penetration;
}


class BtPlayerSweepCallback : public btCollisionWorld::ClosestConvexResultCallback
{
   typedef btCollisionWorld::ClosestConvexResultCallback Parent;

public:

   BtPlayerSweepCallback( btCollisionObject *me, const btVector3 &moveVec  ) 
      :  Parent( btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0) ),
         mMe( me ),
         mMoveVec( moveVec )
   {
   }

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
   {
		if ( proxy0->m_clientObject == mMe )
			return false;

      return Parent::needsCollision( proxy0 );
   }

   virtual btScalar addSingleResult(   btCollisionWorld::LocalConvexResult &convexResult, 
                                       bool normalInWorldSpace )
   {
      // NOTE: I shouldn't have to do any of this, but Bullet 
      // has some weird bugs.
      //
      // For one the plane type will return hits on a Z up surface
      // for sweeps that have no Z sweep component.
      //
      // Second the normal returned here is sometimes backwards
      // to the sweep direction... no clue why.
      //
      F32 dotN = mMoveVec.dot( convexResult.m_hitNormalLocal );
	   if ( mFabs( dotN ) < 0.1f )
         return 1.0f;

      if ( convexResult.m_hitCollisionObject->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE )
         return 1.0f;

	   return Parent::addSingleResult( convexResult, normalInWorldSpace );
   }

protected:
   btVector3 mMoveVec;
   btCollisionObject *mMe;
};

bool BtPlayer::_sweep( btVector3 *inOutCurrPos, const btVector3 &disp, CollisionList *outCol )
{
   btTransform start( btTransform::getIdentity() );
	start.setOrigin ( *inOutCurrPos );

   btTransform end( btTransform::getIdentity() );
	end.setOrigin ( *inOutCurrPos + disp );

   BtPlayerSweepCallback callback( mGhostObject, disp.normalized() );
	callback.m_collisionFilterGroup = mGhostObject->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = mGhostObject->getBroadphaseHandle()->m_collisionFilterMask;

   if (disp.length()>0.0001)
      mGhostObject->convexSweepTest( mColShape, start, end, callback, 0.0f );

	inOutCurrPos->setInterpolate3( start.getOrigin(), end.getOrigin(), callback.m_closestHitFraction );
   if ( callback.hasHit() )
   {
      if ( outCol )
      {
         Collision& col = outCol->increment();
         dMemset( &col, 0, sizeof( col ) );

         col.normal = btCast<Point3F>( callback.m_hitNormalWorld );
         col.object = PhysicsUserData::getObject( callback.m_hitCollisionObject->getUserPointer() );

         F32 vd = col.normal.z;
         if (vd < mSlopeAngle)
            return false;
      }

      return true;
   }

   return false;
}

void BtPlayer::_stepForward( btVector3 *inOutCurrPos, const btVector3 &displacement, CollisionList *outCol )
{
   btTransform start( btTransform::getIdentity() );
   btTransform end( btTransform::getIdentity() );
   F32 fraction = 1.0f;
   S32 maxIter = 10;
   btVector3 disp = displacement;

	while ( fraction > 0.01f && maxIter-- > 0 )
	{
      // Setup the sweep start and end transforms.
		start.setOrigin( *inOutCurrPos );
		end.setOrigin( *inOutCurrPos + disp );

      BtPlayerSweepCallback callback( mGhostObject, disp.length2() > SIMD_EPSILON ? disp.normalized() : disp );
		callback.m_collisionFilterGroup = mGhostObject->getBroadphaseHandle()->m_collisionFilterGroup;
		callback.m_collisionFilterMask = mGhostObject->getBroadphaseHandle()->m_collisionFilterMask;

      if (disp.length()>0.0001)
         mGhostObject->convexSweepTest( mColShape, start, end, callback, 0.0f );

      // Subtract from the travel fraction.
      fraction -= callback.m_closestHitFraction;

      // Did we get a hit?
		if ( callback.hasHit() )
		{
         /*
         // Get the real hit normal... Bullet returns the 'seperating normal' and not
         // the normal of the hit object.
         btTransform rayStart( btTransform::getIdentity() );
         rayStart.setOrigin( callback.m_hitPointWorld + callback.m_hitNormalWorld );
         btTransform rayEnd( btTransform::getIdentity() );
         rayEnd.setOrigin( callback.m_hitPointWorld - callback.m_hitNormalWorld );

         btCollisionWorld::ClosestRayResultCallback rayHit( rayStart.getOrigin(), rayEnd.getOrigin() ); 
         mWorld->getDynamicsWorld()->rayTestSingle(   rayStart, 
                                                      rayEnd, 
                                                      callback.m_hitCollisionObject, 
                                                      callback.m_hitCollisionObject->getCollisionShape(),
                                                      callback.m_hitCollisionObject->getWorldTransform(),
                                                      rayHit );

         if ( !rayHit.hasHit() )
            break;
         */

         Collision& col = outCol->increment();
         dMemset( &col, 0, sizeof( col ) );

         col.normal = btCast<Point3F>( callback.m_hitNormalWorld );
         col.object = PhysicsUserData::getObject( callback.m_hitCollisionObject->getUserPointer() );

         // If the collision direction is sideways then modify the collision normal
         // to remove any z component.  This takes care of any sideways collisions
         // with the round bottom of the capsule when it comes to the Player class
         // velocity calculations.  We want all sideways collisions to be treated
         // as if they hit the side of a cylinder.
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

         // Interpolate to the new position.
			inOutCurrPos->setInterpolate3( start.getOrigin(), end.getOrigin(), callback.m_closestHitFraction );

         // Subtract out the displacement along the collision normal.
         F32 bd = -disp.dot( callback.m_hitNormalWorld );
         btVector3 dv = callback.m_hitNormalWorld * bd;
         disp += dv;
		}
      else
      {
			// we moved whole way
			*inOutCurrPos = end.getOrigin();
         break;
		}
	}
}

void BtPlayer::findContact(   SceneObject **contactObject, 
                              VectorF *contactNormal, 
                              Vector<SceneObject*> *outOverlapObjects ) const
{
   AssertFatal( mGhostObject, "BtPlayer::findContact - The controller is null!" );

   VectorF normal;
   F32 maxDot = -1.0f;

   // Go thru the contact points... get the first contact.
   btHashedOverlappingPairCache *pairCache = mGhostObject->getOverlappingPairCache();
   btBroadphasePairArray& pairArray = pairCache->getOverlappingPairArray();
   U32 numPairs = pairArray.size();
   btManifoldArray manifoldArray;

   for ( U32 i=0; i < numPairs; i++ )
   {
      const btBroadphasePair &pair = pairArray[i];
      
      btBroadphasePair *collisionPair = pairCache->findPair( pair.m_pProxy0, pair.m_pProxy1 );
      if ( !collisionPair || !collisionPair->m_algorithm )
         continue;

      btCollisionObject *other = (btCollisionObject*)pair.m_pProxy0->m_clientObject;
      if ( other == mGhostObject )
         other = (btCollisionObject*)pair.m_pProxy1->m_clientObject;

      if (!outOverlapObjects->contains(PhysicsUserData::getObject(other->getUserPointer())))
         outOverlapObjects->push_back( PhysicsUserData::getObject( other->getUserPointer() ) );

      if ( other->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE )
         continue;

      manifoldArray.clear();
      collisionPair->m_algorithm->getAllContactManifolds( manifoldArray );

      for ( U32 j=0; j < manifoldArray.size(); j++ )
      {                                 
         btPersistentManifold *manifold = manifoldArray[j];
      	btScalar directionSign = manifold->getBody0() == mGhostObject ? 1.0f : -1.0f;

         for ( U32 p=0; p < manifold->getNumContacts(); p++ )
         {
            const btManifoldPoint &pt = manifold->getContactPoint(p);

            // Test the normal... is it the most vertical one we got?
            normal = btCast<Point3F>( pt.m_normalWorldOnB * directionSign );
            F32 dot = mDot( normal, VectorF( 0, 0, 1 ) );
            if ( dot > maxDot )
            {
               maxDot = dot;

               btCollisionObject *colObject = (btCollisionObject*)collisionPair->m_pProxy0->m_clientObject;
               *contactObject = PhysicsUserData::getObject( colObject->getUserPointer() );
               *contactNormal = normal; 
            }
         }
      }
   }
}

void BtPlayer::enableCollision()
{
   AssertFatal( mGhostObject, "BtPlayer::enableCollision - The controller is null!" );

   //mController->setCollision( true );   
}

void BtPlayer::disableCollision()
{
   AssertFatal( mGhostObject, "BtPlayer::disableCollision - The controller is null!" );

   //mController->setCollision( false );   
}

PhysicsWorld* BtPlayer::getWorld()
{
   return mWorld;
}

void BtPlayer::setTransform( const MatrixF &transform )
{
   AssertFatal( mGhostObject, "BtPlayer::setTransform - The ghost object is null!" );

   btTransform xfm = btCast<btTransform>( transform );
   xfm.getOrigin()[2] += mOriginOffset;

   mGhostObject->setWorldTransform( xfm );
}

MatrixF& BtPlayer::getTransform( MatrixF *outMatrix )
{
   AssertFatal( mGhostObject, "BtPlayer::getTransform - The ghost object is null!" );

   *outMatrix = btCast<MatrixF>( mGhostObject->getWorldTransform() );
   *outMatrix[11] -= mOriginOffset;

   return *outMatrix;
}

void BtPlayer::setScale( const Point3F &scale )
{
}
