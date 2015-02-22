//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSICS_H_
#define _PHYSICS_H_
#include "math/mPoint3.h"
#include "core/color.h"
#include "sim/netConnection.h"

#define PHYSICS_BULLET
//#define PHYSICS_PHYSX

class PhysShape;
struct PhysInfo;
struct PhysSoftInfo;
class PhysJoint;
struct PhysJointInfo;
class PhysicsWorld;

enum PhysLib {DUMMY_PHYSICS_LIB, ODE_LIB, BULLET_LIB,PHYSX_LIB};

class Physics
{
public:
	static void init(const char * library = NULL);
	static void destroy();

	static void	createPhysics(bool isServer, PhysicsWorld* world);
	static void	destroyPhysics(bool isServer);
	static Physics* getPhysics(bool isServer);

	virtual PhysShape* createPhysShape(const PhysInfo& descr) = 0;
	virtual PhysShape* createPhysShape(void* vBuffer,int vNum,int vStride, 
										void* iBuffer, int iNum, int triStride) = 0;

	virtual PhysShape* createPhysShapeSoft(const PhysSoftInfo& descr);

	virtual PhysJoint* createPhysJoint(PhysJointInfo& descr) = 0;
	
	static PhysLib getPhysicsLib();

protected:
	static Physics* mServerPhysics;
	static Physics* mClientPhysics;

	static PhysLib mPhysLib;
	
	static const F32 minVelCallbackThreshold;
	static bool m_inited;
};

#endif
