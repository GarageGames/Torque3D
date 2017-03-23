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

#ifndef _PX3WORLD_H_
#define _PX3WORLD_H_

#ifndef _T3D_PHYSICS_PHYSICSWORLD_H_
#include "T3D/physics/physicsWorld.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _PHYSX3_H_
#include "T3D/physics/physx3/px3.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class Px3ConsoleStream;
class Px3ContactReporter;
class FixedStepper;

enum Px3CollisionGroup
{
	PX3_DEFAULT = BIT(0),
	PX3_PLAYER = BIT(1),
	PX3_DEBRIS = BIT(2),
	PX3_TRIGGER = BIT(3),
};

class Px3World : public PhysicsWorld
{
protected:

	physx::PxScene* mScene;
	bool mIsEnabled;
	bool mIsSimulating;
	bool mIsServer;
   bool mIsSceneLocked;
	U32 mTickCount;
	ProcessList *mProcessList;
	F32 mEditorTimeScale;
	bool mErrorReport;
   physx::PxRenderBuffer *mRenderBuffer;
	physx::PxControllerManager* mControllerManager;
	static Px3ConsoleStream *smErrorCallback;
	static physx::PxDefaultAllocator smMemoryAlloc;
	static physx::PxFoundation* smFoundation;
	static physx::PxCooking *smCooking;
	static physx::PxProfileZoneManager* smProfileZoneManager;
	static physx::PxDefaultCpuDispatcher* smCpuDispatcher;
	static physx::PxVisualDebuggerConnection* smPvdConnection;
	F32 mAccumulator;
	bool _simulate(const F32 dt);

public:

	Px3World();
	virtual ~Px3World();

	virtual bool initWorld( bool isServer, ProcessList *processList );
	virtual void destroyWorld();
	virtual void onDebugDraw( const SceneRenderState *state );
	virtual void reset() {}
	virtual bool castRay( const Point3F &startPnt, const Point3F &endPnt, RayInfo *ri, const Point3F &impulse );
	virtual PhysicsBody* castRay( const Point3F &start, const Point3F &end, U32 bodyTypes = BT_All );
	virtual void explosion( const Point3F &pos, F32 radius, F32 forceMagnitude ); 
	virtual bool isEnabled() const { return mIsEnabled; }
	physx::PxScene* getScene(){ return mScene;}
	void setEnabled( bool enabled );
	U32 getTick() { return mTickCount; }
   void tickPhysics( U32 elapsedMs );
	void getPhysicsResults();
	void setEditorTimeScale( F32 timeScale ) { mEditorTimeScale = timeScale; }
	const F32 getEditorTimeScale() const { return mEditorTimeScale; }
	void releaseWriteLock();
	bool isServer(){return mIsServer;}
	physx::PxController* createController( physx::PxControllerDesc &desc );
   void lockScene();
   void unlockScene();
	//static
	static bool restartSDK( bool destroyOnly = false, Px3World *clientWorld = NULL, Px3World *serverWorld = NULL );
	static void releaseWriteLocks();
	static physx::PxCooking *getCooking();
   static void lockScenes();
   static void unlockScenes();
};

#endif // _PX3WORLD_H_
