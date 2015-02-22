//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSICSPHYSX_H_
#define _PHYSICSPHYSX_H_

#include "T3D/logickingMechanics/physics/physics.h"
#undef min
#undef max
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physx/pxWorld.h"

class PhysicsPhysX: public Physics
{
public:
	static Physics* createPhysicsX(PhysicsWorld* world);

	PhysicsPhysX(PhysicsWorld* world);
	virtual ~PhysicsPhysX();

	virtual PhysShape* createPhysShape(const PhysInfo& descr);
	virtual PhysShape* createPhysShape(void* vBuffer,int vNum,int vStride, 
											void* iBuffer, int iNum, int triStride);

	virtual PhysJoint* createPhysJoint(PhysJointInfo& descr);
	virtual PhysShape* createPhysShapeSoft(const PhysSoftInfo& descr);

	NxScene *getPhysScene() {return mScene;};
	PxWorld *getWorld() {return mWorld;};

private:
	PxWorld *mWorld;
	NxScene *mScene;

	static void initPhysX();
	static float m_restitution;
	static float m_staticFriction;
	static float m_dynamicFriction;
};

NxVec3 vectorToPx(const VectorF & v);
VectorF vectorFromPx(const NxVec3 & v);
NxMat34 matrixToPx(const MatrixF & m);
MatrixF matrixFromPx(const NxMat34 & m);


#endif