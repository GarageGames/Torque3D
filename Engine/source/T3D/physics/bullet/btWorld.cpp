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
#include "T3D/physics/bullet/btWorld.h"

#include "T3D/physics/bullet/btPlugin.h"
#include "T3D/physics/bullet/btCasts.h"
#include "T3D/physics/physicsUserData.h"
#include "core/stream/bitStream.h"
#include "platform/profiler.h"
#include "sim/netConnection.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "scene/sceneRenderState.h"
#include "T3D/gameBase/gameProcess.h"
#ifdef _WIN32
#include "BulletMultiThreaded/Win32ThreadSupport.h"
#elif defined (USE_PTHREADS)
#include "BulletMultiThreaded/PosixThreadSupport.h"
#endif

BtWorld::BtWorld() :
   mProcessList( NULL ),
   mIsSimulating( false ),
   mErrorReport( false ),
   mTickCount( 0 ),
   mIsEnabled( false ),
   mEditorTimeScale( 1.0f ),
   mDynamicsWorld( NULL ),
   mThreadSupportCollision( NULL )
{
} 

BtWorld::~BtWorld()
{
}

bool BtWorld::initWorld( bool isServer, ProcessList *processList )
{
   // Collision configuration contains default setup for memory, collision setup.
   mCollisionConfiguration = new btDefaultCollisionConfiguration();

   // TODO: There is something wrong with multithreading
   // and compound convex shapes... so disable it for now.
   static const U32 smMaxThreads = 1;

   // Different initialization with threading enabled.
   if ( smMaxThreads > 1 )
   {
	   
	   // TODO: ifdef assumes smMaxThread is always one at this point. MACOSX support to be decided
#ifdef WIN32
      mThreadSupportCollision = new Win32ThreadSupport( 
         Win32ThreadSupport::Win32ThreadConstructionInfo(   isServer ? "bt_servercol" : "bt_clientcol",
								                                    processCollisionTask,
								                                    createCollisionLocalStoreMemory,
								                                    smMaxThreads ) );
      
      mDispatcher = new	SpuGatheringCollisionDispatcher( mThreadSupportCollision,
                                                         smMaxThreads,
                                                         mCollisionConfiguration );
#endif // WIN32
   }
   else
   {
      mThreadSupportCollision = NULL;
      mDispatcher = new	btCollisionDispatcher( mCollisionConfiguration );
   }
  
   btVector3 worldMin( -2000, -2000, -1000 );
   btVector3 worldMax( 2000, 2000, 1000 );
   btAxisSweep3 *sweepBP = new btAxisSweep3( worldMin, worldMax );
   mBroadphase = sweepBP;
   sweepBP->getOverlappingPairCache()->setInternalGhostPairCallback( new btGhostPairCallback() );

   // The default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded).
   mSolver = new btSequentialImpulseConstraintSolver;

   mDynamicsWorld = new btDiscreteDynamicsWorld( mDispatcher, mBroadphase, mSolver, mCollisionConfiguration );
   if ( !mDynamicsWorld )
   {
      Con::errorf( "BtWorld - %s failed to create dynamics world!", isServer ? "Server" : "Client" );
      return false;
   }

   // Removing the randomization in the solver is required
   // to make the simulation deterministic.
   mDynamicsWorld->getSolverInfo().m_solverMode &= ~SOLVER_RANDMIZE_ORDER;

   mDynamicsWorld->setGravity( btCast<btVector3>( mGravity ) );

   AssertFatal( processList, "BtWorld::init() - We need a process list to create the world!" );
   mProcessList = processList;
   mProcessList->preTickSignal().notify( this, &BtWorld::getPhysicsResults );
   mProcessList->postTickSignal().notify( this, &BtWorld::tickPhysics, 1000.0f );

   return true;
}

void BtWorld::_destroy()
{
   // Release the tick processing signals.
   if ( mProcessList )
   {
      mProcessList->preTickSignal().remove( this, &BtWorld::getPhysicsResults );
      mProcessList->postTickSignal().remove( this, &BtWorld::tickPhysics );
      mProcessList = NULL;
   }

   // TODO: Release any remaining
   // orphaned rigid bodies here.

   SAFE_DELETE( mDynamicsWorld );
   SAFE_DELETE( mSolver );
   SAFE_DELETE( mBroadphase );
   SAFE_DELETE( mDispatcher );
   SAFE_DELETE( mThreadSupportCollision );
   SAFE_DELETE( mCollisionConfiguration );
}

void BtWorld::tickPhysics( U32 elapsedMs )
{
   if ( !mDynamicsWorld || !mIsEnabled )
      return;

   // Did we forget to call getPhysicsResults somewhere?
   AssertFatal( !mIsSimulating, "PhysXWorld::tickPhysics() - Already simulating!" );

   // The elapsed time should be non-zero and 
   // a multiple of TickMs!
   AssertFatal(   elapsedMs != 0 &&
                  ( elapsedMs % TickMs ) == 0 , "PhysXWorld::tickPhysics() - Got bad elapsed time!" );

   PROFILE_SCOPE(BtWorld_TickPhysics);

   // Convert it to seconds.
   const F32 elapsedSec = (F32)elapsedMs * 0.001f;

   // Simulate... it is recommended to always use Bullet's default fixed timestep/
   mDynamicsWorld->stepSimulation( elapsedSec * mEditorTimeScale );

   mIsSimulating = true;

   //Con::printf( "%s BtWorld::tickPhysics!", this == smClientWorld ? "Client" : "Server" );
}

void BtWorld::getPhysicsResults()
{
   if ( !mDynamicsWorld || !mIsSimulating ) 
      return;

   PROFILE_SCOPE(BtWorld_GetPhysicsResults);

   // Get results from scene.
  // mScene->fetchResults( NX_RIGID_BODY_FINISHED, true );
   mIsSimulating = false;
   mTickCount++;
}

void BtWorld::setEnabled( bool enabled )
{
   mIsEnabled = enabled;

   if ( !mIsEnabled )
      getPhysicsResults();
}

void BtWorld::destroyWorld()
{
   _destroy();
}

bool BtWorld::castRay( const Point3F &startPnt, const Point3F &endPnt, RayInfo *ri, const Point3F &impulse )
{
   btCollisionWorld::ClosestRayResultCallback result( btCast<btVector3>( startPnt ), btCast<btVector3>( endPnt ) );
   mDynamicsWorld->rayTest( btCast<btVector3>( startPnt ), btCast<btVector3>( endPnt ), result );

   if ( !result.hasHit() || !result.m_collisionObject )
      return false;

   if ( ri )
   {
      ri->object = PhysicsUserData::getObject( result.m_collisionObject->getUserPointer() );
      
      // If we were passed a RayInfo, we can only return true signifying a collision
      // if we hit an object that actually has a torque object associated with it.
      //
      // In some ways this could be considered an error, either a physx object
      // has raycast-collision enabled that shouldn't or someone did not set
      // an object in this actor's userData.
      //
      if ( ri->object == NULL )
         return false;

      ri->distance = ( endPnt - startPnt ).len() * result.m_closestHitFraction;
      ri->normal = btCast<Point3F>( result.m_hitNormalWorld );
      ri->point = btCast<Point3F>( result.m_hitPointWorld );
      ri->t = result.m_closestHitFraction;
   }

   /*
   if ( impulse.isZero() ||
        !actor.isDynamic() ||
        actor.readBodyFlag( NX_BF_KINEMATIC ) )
      return true;
      
   NxVec3 force = pxCast<NxVec3>( impulse );//worldRay.dir * forceAmt; 
   actor.addForceAtPos( force, hitInfo.worldImpact, NX_IMPULSE );
   */

   return true;
}

PhysicsBody* BtWorld::castRay( const Point3F &start, const Point3F &end, U32 bodyTypes )
{
   btVector3 startPt = btCast<btVector3>( start );
   btVector3 endPt = btCast<btVector3>( end );

   btCollisionWorld::ClosestRayResultCallback result( startPt, endPt );
   mDynamicsWorld->rayTest( startPt, endPt, result );

   if ( !result.hasHit() || !result.m_collisionObject )
      return NULL;

   PhysicsUserData *userData = PhysicsUserData::cast( result.m_collisionObject->getUserPointer() );
   if ( !userData )
      return NULL;

   return userData->getBody();
}

void BtWorld::explosion( const Point3F &pos, F32 radius, F32 forceMagnitude )
{
   /*
   // Find Actors at the position within the radius
   // and apply force to them.

   NxVec3 nxPos = pxCast<NxVec3>( pos );
   NxShape **shapes = (NxShape**)NxAlloca(10*sizeof(NxShape*));
   NxSphere worldSphere( nxPos, radius );

   NxU32 numHits = mScene->overlapSphereShapes( worldSphere, NX_ALL_SHAPES, 10, shapes, NULL );

   for ( NxU32 i = 0; i < numHits; i++ )
   {
      NxActor &actor = shapes[i]->getActor();
      
      bool dynamic = actor.isDynamic();
      
      if ( !dynamic )
         continue;

      bool kinematic = actor.readBodyFlag( NX_BF_KINEMATIC );
      
      if ( kinematic )
         continue;

      NxVec3 force = actor.getGlobalPosition() - nxPos;
      force.normalize();
      force *= forceMagnitude;

      actor.addForceAtPos( force, nxPos, NX_IMPULSE, true );
   }
   */
}

void BtWorld::onDebugDraw( const SceneRenderState *state )
{
   mDebugDraw.setCuller( &state->getFrustum() );

   mDynamicsWorld->setDebugDrawer( &mDebugDraw );
   mDynamicsWorld->debugDrawWorld();
   mDynamicsWorld->setDebugDrawer( NULL );

   mDebugDraw.flush();
}

void BtWorld::reset()
{
   if ( !mDynamicsWorld )
      return;

    ///create a copy of the array, not a reference!
    btCollisionObjectArray copyArray = mDynamicsWorld->getCollisionObjectArray();

    S32 numObjects = mDynamicsWorld->getNumCollisionObjects();
    for ( S32 i=0; i < numObjects; i++ )
    {
            btCollisionObject* colObj = copyArray[i];
            btRigidBody* body = btRigidBody::upcast(colObj);

            if (body)
            {
                    if (body->getMotionState())
                    {
                            //btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
                            //myMotionState->m_graphicsWorldTrans = myMotionState->m_startWorldTrans;
                            //body->setCenterOfMassTransform( myMotionState->m_graphicsWorldTrans );
                            //colObj->setInterpolationWorldTransform( myMotionState->m_startWorldTrans );
                            colObj->forceActivationState(ACTIVE_TAG);
                            colObj->activate();
                            colObj->setDeactivationTime(0);
                            //colObj->setActivationState(WANTS_DEACTIVATION);
                    }

                    //removed cached contact points (this is not necessary if all objects have been removed from the dynamics world)
                    //m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(colObj->getBroadphaseHandle(),getDynamicsWorld()->getDispatcher());

                    btRigidBody* body = btRigidBody::upcast(colObj);
                    if (body && !body->isStaticObject())
                    {
                         btRigidBody::upcast(colObj)->setLinearVelocity(btVector3(0,0,0));
                         btRigidBody::upcast(colObj)->setAngularVelocity(btVector3(0,0,0));
                    }
            }

    }

    // reset some internal cached data in the broadphase
    mDynamicsWorld->getBroadphase()->resetPool( mDynamicsWorld->getDispatcher() );
    mDynamicsWorld->getConstraintSolver()->reset();
}

/*
ConsoleFunction( castForceRay, const char*, 4, 4, "( Point3F startPnt, Point3F endPnt, VectorF impulseVec )" )
{
   PhysicsWorld *world = PHYSICSPLUGIN->getWorld( "server" );
   if ( !world )
      return NULL;
   
   char *returnBuffer = Con::getReturnBuffer(256);

   Point3F impulse;
   Point3F startPnt, endPnt;
   dSscanf( argv[1], "%f %f %f", &startPnt.x, &startPnt.y, &startPnt.z );
   dSscanf( argv[2], "%f %f %f", &endPnt.x, &endPnt.y, &endPnt.z );
   dSscanf( argv[3], "%f %f %f", &impulse.x, &impulse.y, &impulse.z );

   Point3F hitPoint;

   RayInfo rinfo;

   bool hit = world->castRay( startPnt, endPnt, &rinfo, impulse );

   DebugDrawer *ddraw = DebugDrawer::get();
   if ( ddraw )
   {
      ddraw->drawLine( startPnt, endPnt, hit ? ColorF::RED : ColorF::GREEN );
      ddraw->setLastTTL( 3000 );
   }

   if ( hit )
   {
      dSprintf(returnBuffer, 256, "%g %g %g",
            rinfo.point.x, rinfo.point.y, rinfo.point.z );
      return returnBuffer;
   }
   else 
      return NULL;
}
*/