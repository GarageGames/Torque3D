//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSJOINT_H_
#define _PHYSJOINT_H_

#include "T3D/BlissGMK/physics/physShape.h"


struct PhysJointInfo
{
	enum JointType {JT_CONETWIST, JT_HINGE,JT_DOF6,JT_BALLSOCKET,JT_FIXED};
	JointType	jointType;
	VectorF		params1;
	VectorF		params2;
	PhysShape*	shape1;
	PhysShape*	shape2;
	Point3F		pos;
};

class PhysJoint : public StrongRefBase
{
public:
	PhysJoint() {};
	PhysJoint(PhysJointInfo &physJointDescr):m_physJointInfo(physJointDescr){};
	virtual ~PhysJoint(){};
protected:
	PhysJointInfo m_physJointInfo;
};

#endif