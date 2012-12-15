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
#include "T3D/physics/physX/pxWorld.h"

#include "T3D/physics/physX/px.h"
#include "T3D/physics/physX/pxPlugin.h"
#include "T3D/physics/physX/pxMaterial.h"
#include "T3D/physics/physX/pxContactReporter.h"
#include "T3D/physics/physX/pxStream.h"
#include "T3D/physics/physX/pxCasts.h"
#include "T3D/physics/physicsUserData.h"

#include "core/stream/bitStream.h"
#include "platform/profiler.h"
#include "sim/netConnection.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "T3D/tsstatic.h"
#include "T3D/gameBase/gameProcess.h"
#include "gfx/sim/debugDraw.h"
#include "gfx/primBuilder.h"

#include <NXU_helper.h>


static const F32 PhysicsStepTime = (F32)TickMs / 1000.0f;
static const U32 PhysicsMaxIterations = 8;
static const F32 PhysicsMaxTimeStep = PhysicsStepTime / 2.0f;

NxPhysicsSDK *gPhysicsSDK = NULL;
NxCookingInterface *PxWorld::smCooking = NULL;
PxConsoleStream *PxWorld::smConsoleStream = NULL;


PxWorld::PxWorld() :
   mScene( NULL ),
   mConactReporter( NULL ),
   mProcessList( NULL ),
   mIsSimulating( false ),
   mErrorReport( false ),
   mTickCount( 0 ),
   mIsEnabled( false ),
   mEditorTimeScale( 1.0f )
{
	if ( !CCTAllocator::mAllocator )
		CCTAllocator::mAllocator = new NxUserAllocatorDefault();
	mControllerManager = new CharacterControllerManager( CCTAllocator::mAllocator );
} 

PxWorld::~PxWorld()
{
	delete mControllerManager;
}

NxCookingInterface* PxWorld::getCooking()
{
   if ( !smCooking )
      smCooking = NxGetCookingLib( NX_PHYSICS_SDK_VERSION );

   return smCooking;
}

bool PxWorld::_init( bool isServer, ProcessList *processList )
{
   if ( !gPhysicsSDK )
   {
      Con::errorf( "PhysXWorld::init - PhysXSDK not initialized!" );
      return false;
   }

   // Create the scene description.
   NxSceneDesc sceneDesc;
   sceneDesc.userData = this;

   // Set up default gravity.
   sceneDesc.gravity.set( mGravity.x, mGravity.y, mGravity.z );

   // The master scene is always on the CPU and is used
   // mostly for static shapes.
   sceneDesc.simType = NX_SIMULATION_SW; // [9/28/2009 Pat] Why is this software? Should be software server, hardware client?

   // Threading... seems to improve performance.
   //
   // TODO: I was getting random crashes in debug when doing
   // edit and continue... lets see if i still get them with
   // the threading disabled.
   //
   sceneDesc.flags |= NX_SF_ENABLE_MULTITHREAD | NX_SF_DISABLE_SCENE_MUTEX;
   sceneDesc.threadMask = 0xfffffffe;
   sceneDesc.internalThreadCount = PHYSICSMGR->getThreadCount();

   // Create the scene.
   mScene = gPhysicsSDK->createScene(sceneDesc);
   if ( !mScene )
   {
      Con::errorf( "PhysXWorld - %s world createScene returned a null scene!", isServer ? "Server" : "Client" );
      return false;
   }

   /*
   // Make note of what we've created.
   String simType = sceneDesc.simType == NX_SIMULATION_SW ? "software" : "hardware";
   String clientOrServer = this == isServer ? "server" : "client";
   Con::printf( "PhysXWorld::init() - Created %s %s simulation!", 
      clientOrServer.c_str(), 
      simType.c_str() );
   */

   mScene->setTiming( PhysicsMaxTimeStep, PhysicsMaxIterations, NX_TIMESTEP_FIXED );

   // TODO: Add dummy actor with scene name!

   // Set the global contact reporter.

      mConactReporter = new PxContactReporter();
      mScene->setUserContactReport( mConactReporter );

   // Set the global PxUserNotify

      mUserNotify = new PxUserNotify();
      mScene->setUserNotify( mUserNotify );

   // Now create the dynamic rigid body compartment which
   // can reside on the hardware when hardware is present.
   /*
   NxCompartmentDesc compartmentDesc;
   compartmentDesc.type = NX_SCT_RIGIDBODY;
   compartmentDesc.deviceCode = NX_DC_PPU_AUTO_ASSIGN; 
   mRigidCompartment = mScene->createCompartment( compartmentDesc );
   if ( !mRigidCompartment )
   {
      Con::errorf( "PhysXWorld - Creation of rigid body compartment failed!" );
      return false;
   }
   */

   // Hook up the tick processing signals for advancing physics.
   //
   // First an overview of how physics and the game ticks
   // interact with each other.
   //
   // In Torque you normally tick the server and then the client
   // approximately every 32ms.  So before the game tick we call 
   // getPhysicsResults() to get the new physics state and call 
   // tickPhysics() when the game tick is done to start processing
   // the next physics state.  This means PhysX is happily doing
   // physics in a separate thread while we're doing rendering,
   // sound, input, networking, etc.
   // 
   // Because your frame rate is rarely perfectly even you can
   // get cases where you may tick the server or the client 
   // several times in a row.  This happens most often in debug
   // mode, but can also happen in release.
   //
   // The simple implementation is to do a getPhysicsResults() and
   // tickPhysics() for each tick.  But this very bad!  It forces
   // immediate results from PhysX which blocks the primary thread
   // and further slows down processing.  It leads to slower and
   // slower frame rates as the simulation is never able to catch
   // up to the current tick.
   //
   // The trick is processing physics once for backlogged ticks
   // with the total of the elapsed tick time.  This is a huge
   // performance gain and keeps you from blocking on PhysX.
   //
   // This does have a side effect that when it occurs you'll get
   // ticks where the physics state hasn't advanced, but this beats
   // single digit frame rates.
   //
   AssertFatal( processList, "PxWorld::init() - We need a process list to create the world!" );
   mProcessList = processList;
   mProcessList->preTickSignal().notify( this, &PxWorld::getPhysicsResults );
   mProcessList->postTickSignal().notify( this, &PxWorld::tickPhysics, 1000.0f );

   // Setup the default material.
   NxMaterial *dmat = mScene->getMaterialFromIndex( 0 );
   dmat->setRestitution( 0.2f );
   dmat->setStaticFriction( 0.6f );
   dmat->setDynamicFriction( 0.4f );      

   // Setup dominance groups.

   // Group 31 is for debris and other objects which can be pushed but cannot push back.
   // Group 0 is for everything else.
   
	NxConstraintDominance debrisDominance( 0.0f, 1.0f );
	mScene->setDominanceGroupPair( 0, 31, debrisDominance );		

   return true;
}

void PxWorld::_destroy()
{
   // Make sure the simulation is stopped!
   getPhysicsResults();
   _releaseQueues();

   #ifdef TORQUE_DEBUG

      U32 actorCount = mScene->getNbActors();
      U32 jointCount = mScene->getNbJoints();
      
      if ( actorCount != 0 || jointCount != 0 )
      {
         // Dump the names of any actors or joints that
         // were not released before the destruction of
         // this scene.

         for ( U32 i=0; i < actorCount; i++ )
         {
            const NxActor *actor = mScene->getActors()[i];
            Con::errorf( "Orphan NxActor - '%s'!", actor->getName() );
         }

         mScene->resetJointIterator();
         for ( ;; )
         {
            const NxJoint *joint = mScene->getNextJoint();
            if ( !joint )
               break;

            Con::errorf( "Orphan NxJoint - '%s'!", joint->getName() );
         }

         AssertFatal( false, "PhysXWorld::_destroy() - Some actors and/or joints were not released!" );
      }

   #endif // TORQUE_DEBUG

   //NxCloseCooking();

   // Release the tick processing signals.
   if ( mProcessList )
   {
      mProcessList->preTickSignal().remove( this, &PxWorld::getPhysicsResults );
      mProcessList->postTickSignal().remove( this, &PxWorld::tickPhysics );
      mProcessList = NULL;
   }

   // Destroy the scene.
   if ( mScene )
   {
      // Delete the contact reporter.
      mScene->setUserContactReport( NULL );
      SAFE_DELETE( mConactReporter );

      // First shut down threads... this makes it 
      // safe to release the scene.
      mScene->shutdownWorkerThreads();

      // Release the scene.
      gPhysicsSDK->releaseScene( *mScene );
      mScene = NULL;
   }

   // Try to restart the sdk if we can.
   //restartSDK();
}

bool PxWorld::restartSDK( bool destroyOnly, PxWorld *clientWorld, PxWorld *serverWorld )
{
   // If either the client or the server still exist
   // then we cannot reset the SDK.
   if ( clientWorld || serverWorld )
      return false;

   // Destroy the existing SDK.
   if ( gPhysicsSDK )
   {
      NXU::releasePersistentMemory();
      gPhysicsSDK->release();
      gPhysicsSDK = NULL;
      smCooking = NULL;
      SAFE_DELETE( smConsoleStream );
   }

   // If we're not supposed to restart... return.
   if ( destroyOnly )
      return true;

   smConsoleStream = new PxConsoleStream();

   NxPhysicsSDKDesc sdkDesc;
   sdkDesc.flags |= NX_SDKF_NO_HARDWARE; // [9/28/2009 Pat] Why is this disabled?

   NxSDKCreateError error;
   gPhysicsSDK = NxCreatePhysicsSDK(   NX_PHYSICS_SDK_VERSION, 
                                       NULL,
                                       smConsoleStream,
                                       sdkDesc,
                                       &error );
   if ( !gPhysicsSDK )
   {
      Con::errorf( "PhysX failed to initialize!  Error code: %d", error );
      Platform::messageBox(   Con::getVariable( "$appName" ),
                              avar("PhysX could not be started!\r\n"
                              "Please be sure you have the latest version of PhysX installed.\r\n"
                              "Error Code: %d", error),
                              MBOk, MIStop );
      Platform::forceShutdown( -1 );
      
      // We shouldn't get here, but this shuts up
      // source diagnostic tools.
      return false;
   }

   // Set the default skin width for all actors.
   gPhysicsSDK->setParameter( NX_SKIN_WIDTH, 0.01f );

   return true;
}

void PxWorld::tickPhysics( U32 elapsedMs )
{
   if ( !mScene || !mIsEnabled )
      return;

   // Did we forget to call getPhysicsResults somewhere?
   AssertFatal( !mIsSimulating, "PhysXWorld::tickPhysics() - Already simulating!" );

   // The elapsed time should be non-zero and 
   // a multiple of TickMs!
   AssertFatal(   elapsedMs != 0 &&
                  ( elapsedMs % TickMs ) == 0 , "PhysXWorld::tickPhysics() - Got bad elapsed time!" );

   PROFILE_SCOPE(PxWorld_TickPhysics);

   // Convert it to seconds.
   const F32 elapsedSec = (F32)elapsedMs * 0.001f;

   // For some reason this gets reset all the time
   // and it must be called before the simulate.
   mScene->setFilterOps(   NX_FILTEROP_OR, 
                           NX_FILTEROP_OR, 
                           NX_FILTEROP_AND );
   mScene->setFilterBool( false );
   NxGroupsMask zeroMask;
   zeroMask.bits0=zeroMask.bits1=zeroMask.bits2=zeroMask.bits3=0;
   mScene->setFilterConstant0( zeroMask );
   mScene->setFilterConstant1( zeroMask );

   mScene->simulate( elapsedSec * mEditorTimeScale );
   mScene->flushStream();
   mIsSimulating = true;

   //Con::printf( "%s PhysXWorld::tickPhysics!", this == smClientWorld ? "Client" : "Server" );
}

void PxWorld::releaseWriteLocks()
{
   PxWorld *world = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( "server" ) );

   if ( world )
      world->releaseWriteLock();

   world = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( "client" ) );

   if ( world )
      world->releaseWriteLock();
}

void PxWorld::releaseWriteLock()
{
   if ( !mScene || !mIsSimulating ) 
      return;

   PROFILE_SCOPE(PxWorld_ReleaseWriteLock);

   // We use checkResults here to release the write lock
   // but we do not change the simulation flag or increment
   // the tick count... we may have gotten results, but the
   // simulation hasn't really ticked!
   mScene->checkResults( NX_RIGID_BODY_FINISHED, true );
   AssertFatal( mScene->isWritable(), "PhysXWorld::releaseWriteLock() - We should have been writable now!" );
}

void PxWorld::getPhysicsResults()
{
   if ( !mScene || !mIsSimulating ) 
      return;

   PROFILE_SCOPE(PxWorld_GetPhysicsResults);

   // Get results from scene.
   mScene->fetchResults( NX_RIGID_BODY_FINISHED, true );
   mIsSimulating = false;
   mTickCount++;

   // Release any joints/actors that were waiting
   // for the scene to become writable.
   _releaseQueues();

   //Con::printf( "%s PhysXWorld::getPhysicsResults!", this == smClientWorld ? "Client" : "Server" );
}

NxMaterial* PxWorld::createMaterial( NxMaterialDesc &material )
{
   if ( !mScene )
      return NULL;

   // We need the writelock to create a material!
   releaseWriteLock();

   NxMaterial *mat = mScene->createMaterial( material );
   if ( !mat )
      return NULL;

   return mat;
}

NxController* PxWorld::createController( NxControllerDesc &desc )
{
   if ( !mScene )
      return NULL;

   // We need the writelock!
   releaseWriteLock();

   return mControllerManager->createController( mScene, desc );
}

void PxWorld::releaseActor( NxActor &actor )
{
   AssertFatal( &actor.getScene() == mScene, "PhysXWorld::releaseActor() - Bad scene!" );

   // Clear the userdata.
   actor.userData = NULL;   

   // If the scene is not simulating then we have the
   // write lock and can safely delete it now.
   if ( !mIsSimulating )
   {
      mScene->releaseActor( actor );
   }
   else
   {
      mReleaseActorQueue.push_back( &actor );
   }
}

void PxWorld::releaseMaterial( NxMaterial &mat )
{
   AssertFatal( &mat.getScene() == mScene, "PhysXWorld::releaseMaterial() - Bad scene!" );

   // If the scene is not simulating then we have the
   // write lock and can safely delete it now.
   if ( !mIsSimulating )
      mScene->releaseMaterial( mat );
   else
      mReleaseMaterialQueue.push_back( &mat );
}

void PxWorld::releaseHeightField( NxHeightField &heightfield )
{
   // Always delay releasing a heightfield, for whatever reason,
   // it causes lots of deadlock asserts if we do it here, even if
   // the scene "says" its writable. 
   //
   // Actually this is probably because a heightfield is owned by the "sdk" and 
   // not an individual scene so if either the client "or" server scene are 
   // simulating it asserts, thats just my theory.

   mReleaseHeightFieldQueue.push_back( &heightfield );
}

void PxWorld::releaseJoint( NxJoint &joint )
{
   AssertFatal( &joint.getScene() == mScene, "PhysXWorld::releaseJoint() - Bad scene!" );

   AssertFatal( !mReleaseJointQueue.contains( &joint ), 
      "PhysXWorld::releaseJoint() - Joint already exists in the release queue!" );

   // Clear the userdata.
   joint.userData = NULL;

   // If the scene is not simulating then we have the
   // write lock and can safely delete it now.
   if ( !mIsSimulating )
      mScene->releaseJoint( joint );
   else
      mReleaseJointQueue.push_back( &joint );
}

void PxWorld::releaseCloth( NxCloth &cloth )
{
   AssertFatal( &cloth.getScene() == mScene, "PhysXWorld::releaseCloth() - Bad scene!" );

   // Clear the userdata.
   cloth.userData = NULL;

   // If the scene is not simulating then we have the
   // write lock and can safely delete it now.
   if ( !mIsSimulating )
      mScene->releaseCloth( cloth );
   else
      mReleaseClothQueue.push_back( &cloth );
}

void PxWorld::releaseFluid( NxFluid &fluid )
{
   AssertFatal( &fluid.getScene() == mScene, "PhysXWorld::releaseFluid() - Bad scene!" );

   // Clear the userdata.
   fluid.userData = NULL;

   // If the scene is not simulating then we have the
   // write lock and can safely delete it now.
   if ( !mIsSimulating )
      mScene->releaseFluid( fluid );
   else
      mReleaseFluidQueue.push_back( &fluid );
}

void PxWorld::releaseClothMesh( NxClothMesh &clothMesh )
{
   // We need the writelock to release.
   releaseWriteLock();

   gPhysicsSDK->releaseClothMesh( clothMesh );
}

void PxWorld::releaseController( NxController &controller )
{
   // TODO: This isn't safe to do with actors and
   // joints, so we probably need a queue like we
   // do for them.

   // We need the writelock to release.
   releaseWriteLock();

   mControllerManager->releaseController( controller );
}

void PxWorld::_releaseQueues()
{
   AssertFatal( mScene, "PhysXWorld::_releaseQueues() - The scene is null!" );

   // We release joints still pending in the queue 
   // first as they depend on the actors.
   for ( S32 i = 0; i < mReleaseJointQueue.size(); i++ )
   {
      NxJoint *currJoint = mReleaseJointQueue[i];
      mScene->releaseJoint( *currJoint );
   }
   
   // All the joints should be released, clear the queue.
   mReleaseJointQueue.clear();

   // Now release any actors still pending in the queue.
   bool staticChanged = false;
   for ( S32 i = 0; i < mReleaseActorQueue.size(); i++ )
   {
      NxActor *currActor = mReleaseActorQueue[i];
      staticChanged |= !currActor->isDynamic();
      mScene->releaseActor( *currActor );
   }

   // All the actors should be released, clear the queue.
   mReleaseActorQueue.clear();

   // Now release any materials still pending in the queue.
   for ( S32 i = 0; i < mReleaseMaterialQueue.size(); i++ )
   {
      NxMaterial *currMat = mReleaseMaterialQueue[i];     
      mScene->releaseMaterial( *currMat );
   }

   // All the materials should be released, clear the queue.
   mReleaseMaterialQueue.clear();

   // Now release any cloth still pending in the queue.
   for ( S32 i = 0; i < mReleaseClothQueue.size(); i++ )
   {
      NxCloth *currCloth = mReleaseClothQueue[i];
      mScene->releaseCloth( *currCloth );
   }

   // All the actors should be released, clear the queue.
   mReleaseClothQueue.clear();

   // Release heightfields that don't still have references.
   for ( S32 i = 0; i < mReleaseHeightFieldQueue.size(); i++ )
   {
      NxHeightField *currHeightfield = mReleaseHeightFieldQueue[i];
      
      if ( currHeightfield->getReferenceCount() == 0 )
      {
         gPhysicsSDK->releaseHeightField( *currHeightfield );      
         mReleaseHeightFieldQueue.erase_fast( i );
         i--;
      }
   }

   // Clear fluid queue
   for ( S32 i = 0; i < mReleaseFluidQueue.size(); i++ )
   {
      NxFluid *currFluid = mReleaseFluidQueue[i];
      mScene->releaseFluid( *currFluid );
   }
   mReleaseFluidQueue.clear();

   if ( staticChanged )
    mStaticChangedSignal.trigger();
}

void PxWorld::setEnabled( bool enabled )
{
   mIsEnabled = enabled;

   if ( !mIsEnabled )
      getPhysicsResults();
}

bool PxWorld::initWorld( bool isServer, ProcessList *processList )
{
   /* This stuff is handled outside.
   PxWorld* world = PxWorld::getWorld( isServer );
   if ( world )
   {
      Con::errorf( "PhysXWorld::initWorld - %s world already exists!", isServer ? "Server" : "Client" );
      return false;
   }
   */
   
   if ( !_init( isServer, processList ) )
      return false;

   return true;
}

void PxWorld::destroyWorld()
{
   //PxWorld* world = PxWorld::getWorld( serverWorld );
   /*
   if ( !world )
   {
      Con::errorf( "PhysXWorld::destroyWorld - %s world already destroyed!", serverWorld ? "Server" : "Client" );
      return;
   }
   */
   //world->_destroy();
   //delete world;

   _destroy();
}

bool PxWorld::castRay( const Point3F &startPnt, const Point3F &endPnt, RayInfo *ri, const Point3F &impulse )
{
   NxRay worldRay;    
   worldRay.orig = pxCast<NxVec3>( startPnt );
   worldRay.dir = pxCast<NxVec3>( endPnt - startPnt );
   NxF32 maxDist = worldRay.dir.magnitude();
   worldRay.dir.normalize();

   U32 groups = 0xffffffff;
   groups &= ~( 1<<31 ); // No trigger shapes!

   NxRaycastHit hitInfo;
   NxShape *hitShape = mScene->raycastClosestShape( worldRay, NX_ALL_SHAPES, hitInfo, groups, maxDist );

   if ( !hitShape )
      return false;

   //if ( hitShape->userData != NULL )
   //   return false;

   NxActor &actor = hitShape->getActor();
   PhysicsUserData *userData = PhysicsUserData::cast( actor.userData );

   if ( ri )
   {
      ri->object = ( userData != NULL ) ? userData->getObject() : NULL;
      
      // If we were passed a RayInfo, we can only return true signifying a collision
      // if we hit an object that actually has a torque object associated with it.
      //
      // In some ways this could be considered an error, either a physx object
      // has raycast-collision enabled that shouldn't or someone did not set
      // an object in this actor's userData.
      //
      if ( ri->object == NULL )
         return false;

      ri->distance = hitInfo.distance;
      ri->normal = pxCast<Point3F>( hitInfo.worldNormal );
      ri->point = pxCast<Point3F>( hitInfo.worldImpact );
      ri->t = maxDist / hitInfo.distance;
   }

   if ( impulse.isZero() ||
        !actor.isDynamic() ||
        actor.readBodyFlag( NX_BF_KINEMATIC ) )
      return true;
      
   NxVec3 force = pxCast<NxVec3>( impulse );//worldRay.dir * forceAmt; 
   actor.addForceAtPos( force, hitInfo.worldImpact, NX_IMPULSE );

   return true;
}

PhysicsBody* PxWorld::castRay( const Point3F &start, const Point3F &end, U32 bodyTypes )
{
   NxRay worldRay;
   worldRay.orig = pxCast<NxVec3>( start );
   worldRay.dir = pxCast<NxVec3>( end - start );
   F32 maxDist = worldRay.dir.normalize();

   U32 groups = 0xFFFFFFFF;
   if ( !( bodyTypes & BT_Player ) )
      groups &= ~( 1<<29 );

   // TODO: For now always skip triggers and debris,
   // but we should consider how game specifc this API
   // should be in the future.
   groups &= ~( 1<<31 ); // triggers
   groups &= ~( 1<<30 ); // debris

   U32 shapesType = 0;
   if ( bodyTypes & BT_Static )
      shapesType |= NX_STATIC_SHAPES;
   if ( bodyTypes & BT_Dynamic )
      shapesType |= NX_DYNAMIC_SHAPES;

   NxRaycastHit hitInfo;
   NxShape *hitShape = mScene->raycastClosestShape( worldRay, (NxShapesType)shapesType, hitInfo, groups, maxDist );
   if ( !hitShape )
      return NULL;

   NxActor &actor = hitShape->getActor();
   PhysicsUserData *userData = PhysicsUserData::cast( actor.userData );
   if ( !userData )
      return NULL;

   return userData->getBody();
}

void PxWorld::explosion( const Point3F &pos, F32 radius, F32 forceMagnitude )
{
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
}

static ColorI getDebugColor( NxU32 packed )
{
   ColorI col;
   col.blue = (packed)&0xff;
   col.green = (packed>>8)&0xff;
   col.red = (packed>>16)&0xff;
   col.alpha = 255;

   return col;
}

void PxWorld::onDebugDraw( const SceneRenderState *state )
{
   if ( !mScene )
      return;

   // We need the write lock to be able to request 
   // the NxDebugRenderable object.
   releaseWriteLock();

   // TODO: We need to expose the different types of visualization
   // options to script and add a GUI for toggling them!

   gPhysicsSDK->setParameter( NX_VISUALIZATION_SCALE, 1.0f );
   //gPhysicsSDK->setParameter( NX_VISUALIZE_BODY_MASS_AXES, 0.0f );
   gPhysicsSDK->setParameter( NX_VISUALIZE_BODY_AXES, 1.0f );   
   gPhysicsSDK->setParameter( NX_VISUALIZE_COLLISION_SHAPES, 1.0f );

   const NxDebugRenderable *data = mScene->getDebugRenderable();  
   if ( !data )
      return;

   // Render points
   {
      NxU32 numPoints = data->getNbPoints();
      const NxDebugPoint *points = data->getPoints();

      PrimBuild::begin( GFXPointList, numPoints );
      
      while ( numPoints-- )
      {
         PrimBuild::color( getDebugColor(points->color) );
         PrimBuild::vertex3fv( &points->p.x );
         points++;
      }

      PrimBuild::end();
   }

   // Render lines
   {
      NxU32 numLines = data->getNbLines();
      const NxDebugLine *lines = data->getLines();

      PrimBuild::begin( GFXLineList, numLines * 2 );

      while ( numLines-- )
      {
         PrimBuild::color( getDebugColor( lines->color ) );
         PrimBuild::vertex3fv( &lines->p0.x );
         PrimBuild::vertex3fv( &lines->p1.x );
         lines++;
      }

      PrimBuild::end();
   }

   // Render triangles
   {
      NxU32 numTris = data->getNbTriangles();
      const NxDebugTriangle *triangles = data->getTriangles();

      PrimBuild::begin( GFXTriangleList, numTris * 3 );
      
      while ( numTris-- )
      {
         PrimBuild::color( getDebugColor( triangles->color ) );
         PrimBuild::vertex3fv( &triangles->p0.x );
         PrimBuild::vertex3fv( &triangles->p1.x );
         PrimBuild::vertex3fv( &triangles->p2.x );
         triangles++;
      }

      PrimBuild::end();
   }
}
