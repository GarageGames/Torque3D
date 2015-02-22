//-----------------------------------------------------------------------------
// Copyright 2009 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physShapeDummy.h"
#include "T3D/BlissGMK/physics/physicsDummy.h"

#define DENSITY 5.f

PhysShapeDummy::PhysShapeDummy(Physics* phys, const PhysInfo& physDescr):PhysShape(physDescr)
{
	mObjToWorld.identity();
	m_enabled = true;
}

PhysShapeDummy::PhysShapeDummy(Physics* phys,const void* vBuffer,int vNum,int vStride, 
									const void* iBuffer, int iNum, int triStride)
{
}


PhysShapeDummy::~PhysShapeDummy()
{
}

void PhysShapeDummy::setPhysicTransform(const MatrixF& tr)
{
	mObjToWorld = tr;
}

void PhysShapeDummy::getPhysicTransform(MatrixF & tr)
{
	tr = mObjToWorld;
}

VectorF PhysShapeDummy::getLinVelocity()
{
	return VectorF::Zero;
}

void PhysShapeDummy::setLinVelocity(const VectorF& vel)
{
}

VectorF PhysShapeDummy::getAngVelocity()
{
	return VectorF::Zero;
}

void PhysShapeDummy::setAngVelocity(const VectorF& vel)
{
}

void PhysShapeDummy::addForce(const VectorF& force)
{
}
void PhysShapeDummy::addForce(const VectorF& force, const Point3F& pos)
{
}

void PhysShapeDummy::reset()
{
}


void PhysShapeDummy::setForce(const VectorF& force)
{
}

VectorF  PhysShapeDummy::getForce()
{
	return 	VectorF::Zero;
}
void  PhysShapeDummy::setTorque(const VectorF& torque)
{
}
VectorF  PhysShapeDummy::getTorque()
{
	return 	VectorF::Zero;
}

void PhysShapeDummy::setEnable(bool isEnabled)
{
	m_enabled = isEnabled;
}

bool PhysShapeDummy::isEnabled()
{
	return m_enabled;
}



//-------------------------------------------------------------------------------------
PhysShapeSoftDummy::PhysShapeSoftDummy(Physics* phys, const PhysSoftInfo& physDescr):PhysShapeSoft(physDescr)
{
	mObjToWorld.identity();
	m_enabled = true;
}


PhysShapeSoftDummy::~PhysShapeSoftDummy()
{
}

void PhysShapeSoftDummy::setPhysicTransform(const MatrixF& tr)
{
	mObjToWorld = tr;
}

void PhysShapeSoftDummy::getPhysicTransform(MatrixF & tr)
{
	tr = mObjToWorld;
}

VectorF PhysShapeSoftDummy::getLinVelocity()
{
	return VectorF::Zero;
}

void PhysShapeSoftDummy::setLinVelocity(const VectorF& vel)
{
}

VectorF PhysShapeSoftDummy::getAngVelocity()
{
	return VectorF::Zero;
}

void PhysShapeSoftDummy::setAngVelocity(const VectorF& vel)
{
}

void PhysShapeSoftDummy::addForce(const VectorF& force)
{
}
void PhysShapeSoftDummy::addForce(const VectorF& force, const Point3F& pos)
{
}

void PhysShapeSoftDummy::reset()
{
}


void PhysShapeSoftDummy::setForce(const VectorF& force)
{
}

VectorF  PhysShapeSoftDummy::getForce()
{
	return 	VectorF::Zero;
}
void  PhysShapeSoftDummy::setTorque(const VectorF& torque)
{
}
VectorF  PhysShapeSoftDummy::getTorque()
{
	return 	VectorF::Zero;
}

void PhysShapeSoftDummy::setEnable(bool isEnabled)
{
	m_enabled = isEnabled;
}

bool PhysShapeSoftDummy::isEnabled()
{
	return m_enabled;
}
Point3F PhysShapeSoftDummy::getCOM()
{
	return mObjToWorld.getPosition();
}