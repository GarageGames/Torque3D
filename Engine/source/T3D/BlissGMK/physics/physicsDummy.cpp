//-----------------------------------------------------------------------------
// Copyright 2009 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physicsDummy.h"
#include "T3D/BlissGMK/physics/physShapeDummy.h"

void PhysicsDummy::initDummy()
{
}

void PhysicsDummy::closeDummy()
{
}

Physics* PhysicsDummy::createPhysicsDummy()
{
	Physics* physics = (Physics*) new PhysicsDummy();
	return physics;
}

PhysicsDummy::PhysicsDummy()
{
}

PhysicsDummy::~PhysicsDummy()
{
}

void PhysicsDummy::update()
{
}

PhysShape* PhysicsDummy::createPhysShape(const PhysInfo& physDescr)
{
	PhysShape* physShape =(PhysShape*) new PhysShapeDummy(this,physDescr);
	return physShape;
}
PhysShape* PhysicsDummy::createPhysShape(void* vBuffer,int vNum,int vStride, 
						   void* iBuffer, int iNum, int triStride)
{
	PhysShape* physShape = (PhysShape*) new PhysShapeDummy(this,vBuffer,vNum,vStride,iBuffer,iNum,triStride);
	return physShape;
}
PhysJoint* PhysicsDummy::createPhysJoint(PhysJointInfo& descr)
{
	PhysJoint* physJoint = new PhysJointDummy();
	return physJoint;
}