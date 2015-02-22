//-----------------------------------------------------------------------------
// Copyright 2009 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// A 'dummy' physics implementation. For the case when user don't want to 
// have any phys library.
//-----------------------------------------------------------------------------

#ifndef _PHYSICS_DUMMY_H_
#define _PHYSICS_DUMMY_H_

#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/BlissGMK/physics/physJoint.h"

class PhysicsDummy: Physics
{
public:
	static void initDummy();
	static void closeDummy();
	static Physics* createPhysicsDummy();

	PhysicsDummy();
	virtual ~PhysicsDummy();

	virtual PhysShape* createPhysShape(const PhysInfo& physDescr);
	virtual PhysShape* createPhysShape(void* vBuffer,int vNum,int vStride, 
											void* iBuffer, int iNum, int triStride);

	virtual PhysJoint* createPhysJoint(PhysJointInfo& descr);

	virtual void update();
protected:
	virtual void drawPhysics(){};
private:
};


class PhysJointDummy : public PhysJoint
{
public:
	PhysJointDummy() {};
};

#endif