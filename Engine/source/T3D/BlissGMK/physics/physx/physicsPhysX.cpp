//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/physics/physBody.h"
#include "T3D/logickingMechanics/physics/physx/physicsPhysX.h"
#include "T3D/logickingMechanics/physics/physx/physShapePhysX.h"
#include "T3D/logickingMechanics/physics/physx/physJointPhysX.h"
#include "T3D/logickingMechanics/physics/physx/physShapeSoftPhysX.h"

float PhysicsPhysX::m_restitution = 0.0f;
float PhysicsPhysX::m_staticFriction = 0.4f;
float PhysicsPhysX::m_dynamicFriction = 0.4f;

Physics* PhysicsPhysX::createPhysicsX(PhysicsWorld* world)
{
	Physics* physics =  (Physics*) new PhysicsPhysX(world);
	return physics;
}

void PhysicsPhysX::initPhysX()
{
	const char* restitution =  Con::getVariable("$GMK::Physics::PhysX::restitution");
	const char* staticFriction =  Con::getVariable("$GMK::Physics::PhysX::staticFriction");
	const char* dynamicFriction =  Con::getVariable("$GMK::Physics::PhysX::dynamicFriction");
	m_restitution = dStrcmp(restitution,"") ? dAtof(restitution) : m_restitution;
	m_staticFriction = dStrcmp(staticFriction,"") ? dAtof(staticFriction) : m_staticFriction;
	m_dynamicFriction = dStrcmp(dynamicFriction,"") ? dAtof(dynamicFriction) : m_dynamicFriction;
}

PhysicsPhysX::PhysicsPhysX(PhysicsWorld* world)
{
	initPhysX();

	PxWorld* pxWorld = static_cast<PxWorld*>(world);
	mWorld = pxWorld;
	mScene = pxWorld->getScene();

	//set default material params
	NxMaterial* defaultMaterial = mScene->getMaterialFromIndex(0); 	
	defaultMaterial->setRestitution(m_restitution);
	defaultMaterial->setStaticFriction(m_staticFriction);
	defaultMaterial->setDynamicFriction(m_dynamicFriction);
}

PhysicsPhysX::~PhysicsPhysX()
{
	mWorld = NULL;
	mScene = NULL;
}

PhysShape* PhysicsPhysX::createPhysShape(const PhysInfo& descr)
{
	PhysShape* physShape = (PhysShape*) new PhysShapePhysX(this,descr);
	return physShape;
}
PhysShape* PhysicsPhysX::createPhysShape(void* vBuffer,int vNum,int vStride, 
						   void* iBuffer, int iNum, int triStride)
{
	return NULL;
}
PhysJoint* PhysicsPhysX::createPhysJoint(PhysJointInfo& descr)
{
	PhysJoint* physJoint = (PhysJoint*) new PhysJointPhysX(this,descr);
	return physJoint;
}

PhysShape* PhysicsPhysX::createPhysShapeSoft(const PhysSoftInfo& descr)
{
	PhysShape* physShape = (PhysShape*) new PhysShapeSoftPhysX(this,descr);
	return physShape;
}
VectorF vectorFromPx(const NxVec3 & v)
{
	return VectorF(v.x,v.y,v.z);
}

NxVec3 vectorToPx(const VectorF & v)
{
	return NxVec3(v.x,v.y,v.z);
}

MatrixF matrixFromPx(const NxMat34 & m)
{
	MatrixF res;
	m.getRowMajor44( res );
	return res;
}

NxMat34 matrixToPx(const MatrixF & m)
{
	NxMat34 res;
	res.setRowMajor44( m );
	return res;
}