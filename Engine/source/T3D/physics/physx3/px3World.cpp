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
#include "T3D/physics/physx3/px3World.h"

#include "T3D/physics/physx3/px3.h"
#include "T3D/physics/physx3/px3Plugin.h"
#include "T3D/physics/physx3/px3Casts.h"
#include "T3D/physics/physx3/px3Stream.h"
#include "T3D/physics/physicsUserData.h"

#include "console/engineAPI.h"
#include "core/stream/bitStream.h"
#include "platform/profiler.h"
#include "sim/netConnection.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "collision/collision.h"
#include "T3D/gameBase/gameProcess.h"
#include "gfx/sim/debugDraw.h"
#include "gfx/primBuilder.h"


physx::PxPhysics* gPhysics3SDK = NULL;
physx::PxCooking* Px3World::smCooking = NULL;
physx::PxFoundation* Px3World::smFoundation = NULL;
physx::PxProfileZoneManager* Px3World::smProfileZoneManager = NULL;
physx::PxDefaultCpuDispatcher* Px3World::smCpuDispatcher=NULL;
Px3ConsoleStream* Px3World::smErrorCallback = NULL;
physx::PxVisualDebuggerConnection* Px3World::smPvdConnection=NULL;
physx::PxDefaultAllocator Px3World::smMemoryAlloc;

Px3World::Px3World(): mScene( NULL ),
   mProcessList( NULL ),
   mIsSimulating( false ),
   mErrorReport( false ),
   mTickCount( 0 ),
   mIsEnabled( false ),
   mEditorTimeScale( 1.0f ),
   mAccumulator( 0 ),
   mControllerManager(NULL),
   mIsSceneLocked(false),
   mRenderBuffer(NULL)
{
}

Px3World::~Px3World()
{
}

physx::PxCooking *Px3World::getCooking()
{
	return smCooking;
}

bool Px3World::restartSDK( bool destroyOnly, Px3World *clientWorld, Px3World *serverWorld)
{
	// If either the client or the server still exist
	// then we cannot reset the SDK.
	if ( clientWorld || serverWorld )
		return false;

	if(smPvdConnection)
		smPvdConnection->release();

	if(smCooking)
		smCooking->release();

	if(smCpuDispatcher)
		smCpuDispatcher->release();

   // Destroy the existing SDK.
	if ( gPhysics3SDK )
	{
		PxCloseExtensions();
		gPhysics3SDK->release();
	}

   if(smErrorCallback)
   {
      SAFE_DELETE(smErrorCallback);
   }

	if(smFoundation)
	{
		smFoundation->release();
		SAFE_DELETE(smErrorCallback);
	}

	// If we're not supposed to restart... return.
	if ( destroyOnly )
      return true;

	bool memTrack = false;
 #ifdef TORQUE_DEBUG
	memTrack = true;
 #endif

	smErrorCallback  = new Px3ConsoleStream;
	smFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, smMemoryAlloc, *smErrorCallback);
	smProfileZoneManager = &physx::PxProfileZoneManager::createProfileZoneManager(smFoundation);
	gPhysics3SDK = PxCreatePhysics(PX_PHYSICS_VERSION, *smFoundation, physx::PxTolerancesScale(),memTrack,smProfileZoneManager);

	if ( !gPhysics3SDK )
	{
		Con::errorf( "PhysX3 failed to initialize!" );
		Platform::messageBox(   Con::getVariable( "$appName" ),
                              avar("PhysX3 could not be started!\r\n"),
                              MBOk, MIStop );
		Platform::forceShutdown( -1 );
      
		// We shouldn't get here, but this shuts up
		// source diagnostic tools.
		return false;
	}

	if(!PxInitExtensions(*gPhysics3SDK))
	{
		Con::errorf( "PhysX3 failed to initialize extensions!" );
		Platform::messageBox(   Con::getVariable( "$appName" ),
                              avar("PhysX3 could not be started!\r\n"),
                              MBOk, MIStop );
		Platform::forceShutdown( -1 );
		return false;
	}

	smCooking = PxCreateCooking(PX_PHYSICS_VERSION, *smFoundation, physx::PxCookingParams(physx::PxTolerancesScale()));
	if(!smCooking)
	{
		Con::errorf( "PhysX3 failed to initialize cooking!" );
		Platform::messageBox(   Con::getVariable( "$appName" ),
                              avar("PhysX3 could not be started!\r\n"),
                              MBOk, MIStop );
		Platform::forceShutdown( -1 );      
		return false;
	}

#ifdef TORQUE_DEBUG
	physx::PxVisualDebuggerConnectionFlags connectionFlags(physx::PxVisualDebuggerExt::getAllConnectionFlags());
	smPvdConnection = physx::PxVisualDebuggerExt::createConnection(gPhysics3SDK->getPvdConnectionManager(), 
				"localhost", 5425, 100, connectionFlags);	
#endif

	return true;
}

void Px3World::destroyWorld()
{
	getPhysicsResults();

   mRenderBuffer = NULL;

	// Release the tick processing signals.
	if ( mProcessList )
	{
		mProcessList->preTickSignal().remove( this, &Px3World::getPhysicsResults );
		mProcessList->postTickSignal().remove( this, &Px3World::tickPhysics );
		mProcessList = NULL;
	}

   if(mControllerManager)
   {
      mControllerManager->release();
      mControllerManager = NULL;
   }
   
	// Destroy the scene.
	if ( mScene )
	{
		// Release the scene.
		mScene->release();
		mScene = NULL;
	}
}

bool Px3World::initWorld( bool isServer, ProcessList *processList )
{
   if ( !gPhysics3SDK )
   {
      Con::errorf( "Physx3World::init - PhysXSDK not initialized!" );
      return false;
   }

   mIsServer = isServer;
	
   physx::PxSceneDesc sceneDesc(gPhysics3SDK->getTolerancesScale());

   sceneDesc.gravity = px3Cast<physx::PxVec3>(mGravity);
   sceneDesc.userData = this;
   if(!sceneDesc.cpuDispatcher)
   {
      //Create shared cpu dispatcher
      if(!smCpuDispatcher)
         smCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(PHYSICSMGR->getThreadCount());
 
      sceneDesc.cpuDispatcher = smCpuDispatcher;
      Con::printf("PhysX3 using Cpu: %d workers", smCpuDispatcher->getWorkerCount());
   }
 	
   sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
   sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
   sceneDesc.filterShader  = physx::PxDefaultSimulationFilterShader;

	mScene = gPhysics3SDK->createScene(sceneDesc);
   //cache renderbuffer for use with debug drawing
   mRenderBuffer = const_cast<physx::PxRenderBuffer*>(&mScene->getRenderBuffer());

	physx::PxDominanceGroupPair debrisDominance( 0.0f, 1.0f );
	mScene->setDominanceGroupPair(0,31,debrisDominance);

   mControllerManager = PxCreateControllerManager(*mScene);

	AssertFatal( processList, "Px3World::init() - We need a process list to create the world!" );
	mProcessList = processList;
	mProcessList->preTickSignal().notify( this, &Px3World::getPhysicsResults );
	mProcessList->postTickSignal().notify( this, &Px3World::tickPhysics, 1000.0f );

	return true;
}
// Most of this borrowed from bullet physics library, see btDiscreteDynamicsWorld.cpp
bool Px3World::_simulate(const F32 dt)
{
   S32 numSimulationSubSteps = 0;
   //fixed timestep with interpolation
   mAccumulator += dt;
   if (mAccumulator >= smPhysicsStepTime)
   {
      numSimulationSubSteps = S32(mAccumulator / smPhysicsStepTime);
      mAccumulator -= numSimulationSubSteps * smPhysicsStepTime;
   }
	if (numSimulationSubSteps)
	{
		//clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
		S32 clampedSimulationSteps = (numSimulationSubSteps > smPhysicsMaxSubSteps)? smPhysicsMaxSubSteps : numSimulationSubSteps;
		
		for (S32 i=0;i<clampedSimulationSteps;i++)
		{
			mScene->fetchResults(true);
			mScene->simulate(smPhysicsStepTime);
		}
	}
	
	mIsSimulating = true;

	return true;
}

void Px3World::tickPhysics( U32 elapsedMs )
{
   if ( !mScene || !mIsEnabled )
      return;

   // Did we forget to call getPhysicsResults somewhere?
   AssertFatal( !mIsSimulating, "PhysX3World::tickPhysics() - Already simulating!" );

   // The elapsed time should be non-zero and 
   // a multiple of TickMs!
   AssertFatal(   elapsedMs != 0 &&
                  ( elapsedMs % TickMs ) == 0 , "PhysX3World::tickPhysics() - Got bad elapsed time!" );

   PROFILE_SCOPE(Px3World_TickPhysics);

   // Convert it to seconds.
   const F32 elapsedSec = (F32)elapsedMs * 0.001f;
   mIsSimulating = _simulate(elapsedSec * mEditorTimeScale);

   //Con::printf( "%s PhysX3World::tickPhysics!", mIsServer ? "Client" : "Server" );
}

void Px3World::getPhysicsResults()
{
	if ( !mScene || !mIsSimulating ) 
		return;

	PROFILE_SCOPE(Px3World_GetPhysicsResults);

	// Get results from scene.
	mScene->fetchResults(true);
	mIsSimulating = false;
	mTickCount++;

  // Con::printf( "%s PhysXWorld::getPhysicsResults!", this == smClientWorld ? "Client" : "Server" );
}

void Px3World::releaseWriteLocks()
{
	Px3World *world = dynamic_cast<Px3World*>( PHYSICSMGR->getWorld( "server" ) );

	if ( world )
		world->releaseWriteLock();

	world = dynamic_cast<Px3World*>( PHYSICSMGR->getWorld( "client" ) );

	if ( world )
		world->releaseWriteLock();
}

void Px3World::releaseWriteLock()
{
	if ( !mScene || !mIsSimulating ) 
		return;

	PROFILE_SCOPE(PxWorld_ReleaseWriteLock);

	// We use checkResults here to release the write lock
	// but we do not change the simulation flag or increment
	// the tick count... we may have gotten results, but the
	// simulation hasn't really ticked!
	mScene->checkResults( true );
	//AssertFatal( mScene->isWritable(), "PhysX3World::releaseWriteLock() - We should have been writable now!" );
}

void Px3World::lockScenes()
{
   Px3World *world = dynamic_cast<Px3World*>(PHYSICSMGR->getWorld("server"));

   if (world)
      world->lockScene();

   world = dynamic_cast<Px3World*>(PHYSICSMGR->getWorld("client"));

   if (world)
      world->lockScene();
}

void Px3World::unlockScenes()
{
   Px3World *world = dynamic_cast<Px3World*>(PHYSICSMGR->getWorld("server"));

   if (world)
      world->unlockScene();

   world = dynamic_cast<Px3World*>(PHYSICSMGR->getWorld("client"));

   if (world)
      world->unlockScene();
}

void Px3World::lockScene()
{
   if (!mScene)
      return;

   if (mIsSceneLocked)
   {
      Con::printf("Px3World: Attempting to lock a scene that is already locked.");
      return;
   }

   mScene->lockWrite();
   mIsSceneLocked = true;
}

void Px3World::unlockScene()
{
   if (!mScene)
      return;

   if (!mIsSceneLocked)
   {
      Con::printf("Px3World: Attempting to unlock a scene that is not locked.");
      return;
   }

   mScene->unlockWrite();
   mIsSceneLocked = false;
}

bool Px3World::castRay( const Point3F &startPnt, const Point3F &endPnt, RayInfo *ri, const Point3F &impulse )
{
    
	physx::PxVec3 orig = px3Cast<physx::PxVec3>( startPnt );
   physx::PxVec3 dir = px3Cast<physx::PxVec3>( endPnt - startPnt );
   physx::PxF32 maxDist = dir.magnitude();
   dir.normalize();

   U32 groups = 0xffffffff;
   groups &= ~( PX3_TRIGGER ); // No trigger shapes!

   physx::PxHitFlags outFlags(physx::PxHitFlag::eDISTANCE | physx::PxHitFlag::eIMPACT | physx::PxHitFlag::eNORMAL);
   physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC|physx::PxQueryFlag::eDYNAMIC);
   filterData.data.word0 = groups;
   physx::PxRaycastBuffer buf;

   if(!mScene->raycast(orig,dir,maxDist,buf,outFlags,filterData))
	  return false;
   if(!buf.hasBlock)
	 return false;

	const physx::PxRaycastHit hit = buf.block;
   physx::PxRigidActor *actor = hit.actor;
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );

   if ( ri )
   {
      ri->object = ( userData != NULL ) ? userData->getObject() : NULL;
      
      if ( ri->object == NULL )

      ri->distance = hit.distance;
      ri->normal = px3Cast<Point3F>( hit.normal );
      ri->point = px3Cast<Point3F>( hit.position );
      ri->t = maxDist / hit.distance;
   }

   if ( impulse.isZero() ||
        !actor->isRigidDynamic() ||
        actor->is<physx::PxRigidDynamic>()->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC )
      return true;
  
   physx::PxRigidBody *body = actor->is<physx::PxRigidBody>();
   physx::PxVec3 force = px3Cast<physx::PxVec3>( impulse );
   physx::PxRigidBodyExt::addForceAtPos(*body,force,hit.position,physx::PxForceMode::eIMPULSE);

   return true;
}

PhysicsBody* Px3World::castRay( const Point3F &start, const Point3F &end, U32 bodyTypes )
{
   physx::PxVec3 orig = px3Cast<physx::PxVec3>( start );
   physx::PxVec3 dir = px3Cast<physx::PxVec3>( end - start );
   physx::PxF32 maxDist = dir.magnitude();
   dir.normalize();

   U32 groups = 0xFFFFFFFF;
   if ( !( bodyTypes & BT_Player ) )
      groups &= ~( PX3_PLAYER );

   // TODO: For now always skip triggers and debris,
   // but we should consider how game specifc this API
   // should be in the future.
   groups &= ~( PX3_TRIGGER ); // triggers
   groups &= ~( PX3_DEBRIS ); // debris

   physx::PxHitFlags outFlags(physx::PxHitFlag::eDISTANCE | physx::PxHitFlag::eIMPACT | physx::PxHitFlag::eNORMAL);
   physx::PxQueryFilterData filterData;
   if(bodyTypes & BT_Static)
	   filterData.flags |= physx::PxQueryFlag::eSTATIC;
   if(bodyTypes & BT_Dynamic)
	   filterData.flags |= physx::PxQueryFlag::eDYNAMIC;

   filterData.data.word0 = groups;
   physx::PxRaycastBuffer buf;  

   if( !mScene->raycast(orig,dir,maxDist,buf,outFlags,filterData) )
	   return NULL;
   if(!buf.hasBlock)
      return NULL;

   physx::PxRigidActor *actor = buf.block.actor;
   PhysicsUserData *userData = PhysicsUserData::cast( actor->userData );
   if( !userData )
      return NULL;

   return userData->getBody();
}

void Px3World::explosion( const Point3F &pos, F32 radius, F32 forceMagnitude )
{
	physx::PxVec3 nxPos = px3Cast<physx::PxVec3>( pos );
   const physx::PxU32 bufferSize = 10;
   physx::PxSphereGeometry worldSphere(radius);
   physx::PxTransform pose(nxPos);
   physx::PxOverlapBufferN<bufferSize> buffer;
  
   if(!mScene->overlap(worldSphere,pose,buffer))
      return;

   for ( physx::PxU32 i = 0; i < buffer.nbTouches; i++ )
   {
      physx::PxRigidActor *actor = buffer.touches[i].actor;
      
      bool dynamic = actor->isRigidDynamic();
      
      if ( !dynamic )
         continue;

      bool kinematic = actor->is<physx::PxRigidDynamic>()->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC;
      
      if ( kinematic )
         continue;

      physx::PxVec3 force = actor->getGlobalPose().p - nxPos;
      force.normalize();
      force *= forceMagnitude;

      physx::PxRigidBody *body = actor->is<physx::PxRigidBody>();
      physx::PxRigidBodyExt::addForceAtPos(*body,force,nxPos,physx::PxForceMode::eIMPULSE);
   }
}

void Px3World::setEnabled( bool enabled )
{
   mIsEnabled = enabled;

   if ( !mIsEnabled )
      getPhysicsResults();
}

physx::PxController* Px3World::createController( physx::PxControllerDesc &desc )
{
	if ( !mScene )
		return NULL;

	// We need the writelock!
	releaseWriteLock();
	physx::PxController* pController = mControllerManager->createController(desc);
	AssertFatal( pController, "Px3World::createController - Got a null!" );
	return pController;
}

static ColorI getDebugColor( physx::PxU32 packed )
{
   ColorI col;
   col.blue = (packed)&0xff;
   col.green = (packed>>8)&0xff;
   col.red = (packed>>16)&0xff;
   col.alpha = 255;

   return col;
}

void Px3World::onDebugDraw( const SceneRenderState *state )
{
   if ( !mScene || !mRenderBuffer )
      return;

   mScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,1.0f);
   mScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_AXES,1.0f);
   mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES,1.0f);

   // Render points
   {
      physx::PxU32 numPoints = mRenderBuffer->getNbPoints();
      const physx::PxDebugPoint *points = mRenderBuffer->getPoints();

      PrimBuild::begin( GFXPointList, numPoints );
      
      while ( numPoints-- )
      {
         PrimBuild::color( getDebugColor(points->color) );
         PrimBuild::vertex3fv(px3Cast<Point3F>(points->pos));
         points++;
      }

      PrimBuild::end();
   }

   // Render lines
   {
      physx::PxU32 numLines = mRenderBuffer->getNbLines();
      const physx::PxDebugLine *lines = mRenderBuffer->getLines();

      PrimBuild::begin( GFXLineList, numLines * 2 );

      while ( numLines-- )
      {
         PrimBuild::color( getDebugColor( lines->color0 ) );
         PrimBuild::vertex3fv( px3Cast<Point3F>(lines->pos0));
         PrimBuild::color( getDebugColor( lines->color1 ) );
         PrimBuild::vertex3fv( px3Cast<Point3F>(lines->pos1));
         lines++;
      }

      PrimBuild::end();
   }

   // Render triangles
   {
      physx::PxU32 numTris = mRenderBuffer->getNbTriangles();
      const physx::PxDebugTriangle *triangles = mRenderBuffer->getTriangles();

      PrimBuild::begin( GFXTriangleList, numTris * 3 );
      
      while ( numTris-- )
      {
         PrimBuild::color( getDebugColor( triangles->color0 ) );
         PrimBuild::vertex3fv( px3Cast<Point3F>(triangles->pos0) );
         PrimBuild::color( getDebugColor( triangles->color1 ) );
         PrimBuild::vertex3fv( px3Cast<Point3F>(triangles->pos1));
         PrimBuild::color( getDebugColor( triangles->color2 ) );
         PrimBuild::vertex3fv( px3Cast<Point3F>(triangles->pos2) );
         triangles++;
      }

      PrimBuild::end();
   }

}
