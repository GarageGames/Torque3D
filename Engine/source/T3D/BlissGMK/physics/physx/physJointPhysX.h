//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSJOINTPHYSX_H_
#define _PHYSJOINTPHYSX_H_

#include "T3D/logickingMechanics/physics/physJoint.h"
#include "T3D/logickingMechanics/physics/physX/physShapePhysX.h"

class PhysJointPhysX : public PhysJoint
{
public:
	PhysJointPhysX();
	PhysJointPhysX(Physics* phys,PhysJointInfo &physJointDescr);
	virtual ~PhysJointPhysX();
protected:
	PxWorld* m_world;
	NxScene* m_scene;
	NxJoint* m_joint;
};

#endif