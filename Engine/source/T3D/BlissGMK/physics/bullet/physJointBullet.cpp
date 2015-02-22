//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/bullet/physJointBullet.h"

PhysJointBullet::PhysJointBullet(): m_world(NULL),m_constrained(NULL)
{

}

PhysJointBullet::PhysJointBullet(Physics* phys,PhysJointInfo &physJointDescr): PhysJoint(physJointDescr)
{
	PhysicsBullet* physics = static_cast<PhysicsBullet*>(phys);
	m_world = physics->getWorld();

	PhysShapeBullet * physShape1 = (static_cast<PhysShapeBullet*>(physJointDescr.shape1));
	PhysShapeBullet * physShape2 = (static_cast<PhysShapeBullet*>(physJointDescr.shape2));
	btRigidBody* body1 = physShape1->getBody();
	btRigidBody* body2 = physShape2->getBody();
	VectorF jointPos =physJointDescr.pos;//0.5f*(physShape1->getPosition() + physShape2->getPosition());
	//find pivotes in local spaces
	MatrixF trA = physShape1->getTransform();
	MatrixF origTrA = trA;
	trA.fullInverse();
	VectorF pA = jointPos;
	trA.mulP(pA);
	btVector3 pivotInA = vectorToBt(pA);
	MatrixF trB = physShape2->getTransform();
	trB.fullInverse();
	VectorF pB = jointPos;
	trB.mulP(pB);
	btVector3 pivotInB = vectorToBt(pB);

	btTransform localA,localB;
	localA.setIdentity(); localB.setIdentity();
	localA.setOrigin(pivotInA);
	localB.setOrigin(pivotInB);
	btMatrix3x3 matrix = matrix3ToBt(trB*origTrA);
	localB.setBasis(matrix);

	m_constrained = NULL;

	btMatrix3x3 localJointRot;
	localJointRot.setIdentity();

	switch(physJointDescr.jointType)
	{
	case PhysJointInfo::JT_CONETWIST:
		{
			localJointRot.setEulerZYX(0, Float_HalfPi, 0); //0,0,Float_HalfPi
			localA.setBasis(localA.getBasis()*localJointRot);
			localB.setBasis(localB.getBasis()*localJointRot);
			btConeTwistConstraint* coneC = new btConeTwistConstraint(*body1, *body2, localA, localB);
			coneC->setLimit(physJointDescr.params1.x,physJointDescr.params1.y,physJointDescr.params1.z);//_swingSpan1,_swingSpan2,_twistSpan,
			m_constrained = coneC;
			break;
		}
	case PhysJointInfo::JT_HINGE:
		{
			localJointRot.setEulerZYX(0, Float_HalfPi, 0); //0,-Float_HalfPi,0
			localA.setBasis(localA.getBasis()*localJointRot);
			localB.setBasis(localB.getBasis()*localJointRot);
			btHingeConstraint* hingeC =  new btHingeConstraint(*body1, *body2, localA, localB);
			hingeC->setLimit(physJointDescr.params1.x,physJointDescr.params1.y);//low high
			m_constrained = hingeC;
			break;
		}
	case PhysJointInfo::JT_DOF6:
		{
			//generic dof 6 joint
			btGeneric6DofConstraint* dof6joint = new btGeneric6DofConstraint(*body1, *body2, localA, localB,true);
			dof6joint->setAngularLowerLimit(btVector3(-SIMD_PI*0.4f,-SIMD_EPSILON,-SIMD_PI*0.4f));
			dof6joint->setAngularUpperLimit(btVector3(SIMD_PI*0.4f,SIMD_EPSILON,SIMD_PI*0.4f));
			m_constrained = dof6joint;
			break;
		}
	case PhysJointInfo::JT_BALLSOCKET:
		{
			btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body1, *body2,localA.getOrigin(),localB.getOrigin());
			m_constrained = p2p;
			break;
		}
	}

	m_world->addConstraint(m_constrained,true);

	
	// set some damping (for ragdolls)
	body1->setDamping(0.05f, 0.85f);
	body1->setDeactivationTime(0.8f);
	body1->setSleepingThresholds(1.6f, 2.5f);
	body2->setDamping(0.05f, 0.85f);
	body2->setDeactivationTime(0.8f);
	body2->setSleepingThresholds(1.6f, 2.5f);
	
}


PhysJointBullet::~PhysJointBullet()
{
	if (m_constrained)
	{
		m_world->removeConstraint(m_constrained);
		delete m_constrained;
		m_constrained = NULL;
	}
	
}