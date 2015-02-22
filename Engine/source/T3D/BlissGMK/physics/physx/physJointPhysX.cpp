//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/physics/physx/PhysJointPhysX.h"

PhysJointPhysX::PhysJointPhysX(): m_world(NULL),m_scene(NULL), m_joint(NULL)
{

}

PhysJointPhysX::PhysJointPhysX(Physics* phys,PhysJointInfo &physJointDescr): PhysJoint(physJointDescr)
{
	PhysicsPhysX* physics = static_cast<PhysicsPhysX*>(phys);
	m_world = physics->getWorld();
	m_scene = physics->getPhysScene();

	PhysShapePhysX * physShape1 = (static_cast<PhysShapePhysX*>(physJointDescr.shape1));
	PhysShapePhysX * physShape2 = (static_cast<PhysShapePhysX*>(physJointDescr.shape2));
	NxActor* actor1 = physShape1->getActor();
	NxActor* actor2 = physShape2->getActor();
	VectorF jointPos =physJointDescr.pos;//0.5f*(physShape1->getPosition() + physShape2->getPosition());//
	

	NxMat33 tr1 =  actor1->getGlobalOrientation();
	//MatrixF trA = physShape1->getTransform();

/*
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
	//localB.setBasis(matrix3ToBt(trB*origTrA));
	btMatrix3x3 matrix = matrix3ToBt(trB*origTrA);
	localB.setBasis(matrix);//.transpose()

	m_constrained = NULL;

	btMatrix3x3 localJointRot;
	localJointRot.setIdentity();
*/
	NxJointDesc* jointDesc = NULL;
	


	switch(physJointDescr.jointType)
	{
	case PhysJointInfo::JT_HINGE:
		{
			NxVec3 axis(1.f,0.f,0.f);
			tr1.multiply(axis,axis);

			NxRevoluteJointDesc* revJointDesc = new NxRevoluteJointDesc;

			revJointDesc->actor[0] = actor1;
			revJointDesc->actor[1] = actor2;
			revJointDesc->setGlobalAnchor(vectorToPx(jointPos));

			revJointDesc->setGlobalAxis(axis);
			
			
			revJointDesc->flags |= NX_RJF_LIMIT_ENABLED;
			NxJointLimitPairDesc limitDesc;
			limitDesc.low.value = (NxReal)physJointDescr.params1.x;
			limitDesc.high.value = (NxReal)physJointDescr.params1.y;
			revJointDesc->limit = limitDesc;
			
			jointDesc = (NxJointDesc*)revJointDesc;

			break;
		}
	case PhysJointInfo::JT_CONETWIST:
		{
			NxVec3 axis(0.f,1.f,0.f);
			tr1.multiply(axis,axis);

			NxSphericalJointDesc *sphericalDesc = new NxSphericalJointDesc;

			sphericalDesc->actor[0] = actor1;
			sphericalDesc->actor[1] = actor2;
			sphericalDesc->setGlobalAnchor(vectorToPx(jointPos));
			sphericalDesc->setGlobalAxis(axis);

			//sphericalDesc->swingAxis = NxVec3(0.f,-1.0,0.f);

			sphericalDesc->flags |= NX_SJF_TWIST_LIMIT_ENABLED | NX_SJF_SWING_LIMIT_ENABLED;


			NxJointLimitDesc swingLimitDesc;
			swingLimitDesc.value = (NxReal)physJointDescr.params1.x;
			sphericalDesc->swingLimit = swingLimitDesc;
			NxJointLimitPairDesc twistLimitDesc;
			twistLimitDesc.low.value  = (NxReal)physJointDescr.params1.y;
			twistLimitDesc.high.value = (NxReal)physJointDescr.params1.z;
			sphericalDesc->twistLimit = twistLimitDesc;

			jointDesc = (NxJointDesc*)sphericalDesc;
			break;
		}
		/*
	case PhysJointInfo::JT_DOF6:
		{
			//generic dof 6 joint
			btGeneric6DofConstraint* dof6joint = new btGeneric6DofConstraint(*body1, *body2, localA, localB,true);
			//dof6joint->setAngularLowerLimit(btVector3(-SIMD_EPSILON,-SIMD_EPSILON,-SIMD_EPSILON));
			//dof6joint->setAngularUpperLimit(btVector3(SIMD_EPSILON,SIMD_EPSILON,SIMD_EPSILON));
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
		*/
	}
	
	AssertFatal(jointDesc,"Can't create physic joint");


	jointDesc->jointFlags |= NX_JF_VISUALIZATION;

	m_joint = m_scene->createJoint(*jointDesc);
}


PhysJointPhysX::~PhysJointPhysX()
{
	if (m_joint)
	{
		m_world->releaseWriteLock();
		m_scene->releaseJoint(*m_joint);
		m_joint = NULL;
	}
	
}