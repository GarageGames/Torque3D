//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physics.h"

#ifdef PHYSICS_ODE
#include "T3D/BlissGMK/physics/ode/physicsODE.h"
#endif

#ifdef PHYSICS_BULLET
#include "T3D/BlissGMK/physics/bullet/physicsBullet.h"
#endif

#ifdef PHYSICS_PHYSX
#include "T3D/BlissGMK/physics/physx/physicsPhysX.h"
#endif

#include "T3D/BlissGMK/physics/physicsDummy.h"
#include "T3D/BlissGMK/physics/physShapeDummy.h"

#include "T3D/physics/physicsWorld.h"

#include "console/console.h"
#include "scene/sceneObject.h"

Physics* Physics::mServerPhysics = NULL;
Physics* Physics::mClientPhysics = NULL;
PhysLib Physics::mPhysLib = DUMMY_PHYSICS_LIB;
bool Physics::m_inited = false;

const F32 Physics::minVelCallbackThreshold = 1.f;

void Physics::init(const char * library)
{
	m_inited = true;

	if (library && dStricmp(library,""))
	{
		if (!dStricmp(library,"default") || !dStricmp(library,"PhysX"))
			mPhysLib = PHYSX_LIB;
		else if (!dStricmp(library,"Bullet"))
			mPhysLib = BULLET_LIB;
		else
		{
			/* this is a fatal condition, we NEED to 
			know if we do not have the proper library
			AssertWarn(false, "Unknown physics lib");
			*/
			AssertFatal(library, "Unknown physics lib : init error ?");
		}
	}
	else
		mPhysLib = DUMMY_PHYSICS_LIB;

	//define script constants
	//shape types
	Con::setVariable("$ShapeType::Box",avar("%d",PhysInfo::ST_BOX));
	Con::setVariable("$ShapeType::Sphere",avar("%d",PhysInfo::ST_SPHERE));
	Con::setVariable("$ShapeType::Capsule",avar("%d",PhysInfo::ST_CAPSULE));
	Con::setVariable("$ShapeType::Cylinder",avar("%d",PhysInfo::ST_CYLINDER));
	Con::setVariable("$ShapeType::SoftMesh",avar("%d",PhysInfo::ST_SOFTMESH));
	Con::setVariable("$ShapeType::Mesh",avar("%d",PhysInfo::ST_MESH));
	//joint types
	Con::setVariable("$JointType::ConeTwist",avar("%d",PhysJointInfo::JT_CONETWIST));
	Con::setVariable("$JointType::Hinge",avar("%d",PhysJointInfo::JT_HINGE));
	Con::setVariable("$JointType::Dof6",avar("%d",PhysJointInfo::JT_DOF6));
	Con::setVariable("$JointType::BallSocket",avar("%d",PhysJointInfo::JT_BALLSOCKET));
}

void Physics::destroy()
{
	destroyPhysics(true);
	destroyPhysics(false);
}

void Physics::destroyPhysics(bool isServer)
{
	if (isServer)
	{
		SAFE_DELETE(mServerPhysics);
	}
	else
	{
		SAFE_DELETE(mClientPhysics);
	}
}

void Physics::createPhysics(bool isServer, PhysicsWorld* world)
{
	AssertFatal(m_inited, "Physics is not initialized");

	Physics* physics = NULL;
	
#ifdef PHYSICS_BULLET
	if (mPhysLib == BULLET_LIB)
		physics = PhysicsBullet::createPhysicsBullet(world);
#endif

#ifdef PHYSICS_PHYSX	
	if (mPhysLib == PHYSX_LIB)
		physics = PhysicsPhysX::createPhysicsX(world);
#endif
	
	if (!physics)
	{
		if (!mPhysLib!=DUMMY_PHYSICS_LIB)
		{
			Con::warnf("Physics::createPhysics::can't find proper physics. Dummy physics is loaded");
		}
		physics = PhysicsDummy::createPhysicsDummy();
	}
	
	AssertFatal(physics, "No physics created");
	
	if (isServer) 
		mServerPhysics = physics;
	else
		mClientPhysics = physics;
}
Physics* Physics::getPhysics(bool isServer)
{
	if (!m_inited)
	{
		//create dummy physics
		init();
		createPhysics(true, NULL);
		createPhysics(false, NULL);
	}
	Physics* physics = NULL;
	/*
	if (!isServer && NetConnection::getLocalClientConnection())
		return NULL;//*/
	physics = isServer ? mServerPhysics : mClientPhysics;


	return physics;
};
PhysLib Physics::getPhysicsLib()
{
	return mPhysLib;
};

PhysShape* Physics::createPhysShapeSoft(const PhysSoftInfo& descr)
{
	PhysShape* ps = new PhysShapeSoftDummy(this,descr);
	return ps;
}

ConsoleFunction( getPhysicsLib, const char*, 1, 1, "" )
{
 	PhysLib lib = Physics::getPhysicsLib();
	const char* libName = NULL;
	switch(lib)
	{
	case BULLET_LIB:
		libName = "Bullet";
		break;
	case PHYSX_LIB:
		libName = "PhysX";
	    break;
	default:
		libName = "Dummy";
	    break;
	}
	return libName;
}