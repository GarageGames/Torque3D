//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/physics/physx/physShapePhysX.h"

PhysShapePhysX::PhysShapePhysX(Physics* phys, const PhysInfo &physDescr) : PhysShape(physDescr)
{
	PhysicsPhysX* physics = static_cast<PhysicsPhysX*>(phys);
	m_world = physics->getWorld();
	m_scene = physics->getPhysScene();


	NxShapeDesc* shapeDesc = NULL;
	if (m_physInfo.shapeType == PhysInfo::ST_BOX)
	{
		NxVec3 halfSize = 0.5f * vectorToPx(m_physInfo.params);
		NxBoxShapeDesc* boxShapeDesc = new NxBoxShapeDesc();
		boxShapeDesc->dimensions = halfSize;
		shapeDesc = (NxShapeDesc*) boxShapeDesc;
	}
	else if (m_physInfo.shapeType == PhysInfo::ST_SPHERE)
	{
		NxSphereShapeDesc* sphereShapeDesc = new NxSphereShapeDesc();
		sphereShapeDesc->radius = 0.5f * m_physInfo.params.x;
		shapeDesc = (NxShapeDesc*) sphereShapeDesc;
	}
	else if (m_physInfo.shapeType == PhysInfo::ST_CAPSULE || m_physInfo.shapeType == PhysInfo::ST_CYLINDER)
	{
		//rotate on pi/2
		static bool useRot = true;
		if (useRot)
		{
			MatrixF rotMat(EulerF(float(M_PI)/2.f,0.f,0.f));
			m_physInfo.transform.mul(rotMat);
			mTransformInv = m_physInfo.transform;
			mTransformInv.inverse();
		}
		


		NxVec3 sizes = vectorToPx(m_physInfo.params);
		NxCapsuleShapeDesc* capsuleShapeDesc = new NxCapsuleShapeDesc();
		capsuleShapeDesc->radius = 0.5f*sizes.x;
		capsuleShapeDesc->height = sizes.z - sizes.x > 0 ? sizes.z - sizes.x : 0.1f;
		shapeDesc = (NxShapeDesc*) capsuleShapeDesc;
	}
	else
	{
		Con::errorf("PhysShapeBullet::PhysShapeBullet(): Wrong phys type %d",int(m_physInfo.shapeType));
	}
	
	AssertFatal(shapeDesc,"Can't create physic collision shape ");

	static NxF32 adamping = 0.5f;
	
	NxBodyDesc bodyDesc;
	bodyDesc.angularDamping	= adamping;

	NxF32 mass = m_physInfo.mass;

	m_physInfo.bodyType = PhysInfo::BT_STATIC;
	if (mass > 0.f + FLT_EPSILON)
	{
		m_physInfo.bodyType = PhysInfo::BT_DYNAMIC;
		bodyDesc.mass = mass;
	}
	else
		bodyDesc.mass = 1.f;


	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(shapeDesc);
	actorDesc.body			= &bodyDesc;
	
	m_actor = m_scene->createActor(actorDesc);
	
	if (m_physInfo.bodyType == PhysInfo::BT_STATIC)
		m_actor->raiseBodyFlag(NX_BF_KINEMATIC);
	
	m_actor->userData = &mUserData;
}


PhysShapePhysX::~PhysShapePhysX()
{
	m_world->releaseWriteLock();
	m_actor->userData = NULL;
	m_scene->releaseActor(*m_actor);
	m_actor = NULL;
}

void PhysShapePhysX::setPhysicTransform(const MatrixF& tr)
{
	m_actor->setGlobalPose(matrixToPx(tr));
}

void PhysShapePhysX::getPhysicTransform(MatrixF & tr)
{
	tr = matrixFromPx(m_actor->getGlobalPose());
}

VectorF PhysShapePhysX::getLinVelocity()
{
	VectorF res = vectorFromPx(m_actor->getLinearVelocity());
	return res;
}

void PhysShapePhysX::setLinVelocity(const VectorF& vel)
{
	m_actor->setLinearVelocity(vectorToPx(vel));
}

VectorF PhysShapePhysX::getAngVelocity()
{
	VectorF res = vectorFromPx(m_actor->getAngularVelocity());
	return res;
}

void PhysShapePhysX::setAngVelocity(const VectorF& vel)
{
	m_actor->setAngularVelocity(vectorToPx(vel));
}

void PhysShapePhysX::addForce(const VectorF& f)
{
	NxVec3 force = vectorToPx(f);
	m_actor->addForce(force);
}

void PhysShapePhysX::addForce(const VectorF& f, const Point3F& p)
{
	m_world->releaseWriteLock();
	NxVec3 force = vectorToPx(f);
	NxVec3 pos = vectorToPx(p);
	static NxF32 scaler = 1.f;
	force = force*scaler;
	m_actor->addForceAtPos( force, pos);
}

void PhysShapePhysX::reset()
{

}


void PhysShapePhysX::setForce(const VectorF& force)
{
	m_actor->setLinearMomentum(vectorToPx(force));
}

VectorF  PhysShapePhysX::getForce()
{
	VectorF res = vectorFromPx(m_actor->getLinearMomentum());
	return res;
}
void  PhysShapePhysX::setTorque(const VectorF& torque)
{
	m_actor->setAngularMomentum(vectorToPx(torque));
}
VectorF  PhysShapePhysX::getTorque()
{
	VectorF res = vectorFromPx(m_actor->getAngularMomentum());
	return res;
}

void PhysShapePhysX::setEnable(bool isEnabled)
{
	m_world->releaseWriteLock();
	if (isEnabled)
	{
		m_actor->clearActorFlag( NX_AF_DISABLE_COLLISION );
		m_actor->clearBodyFlag( NX_BF_KINEMATIC );
		m_actor->wakeUp();

		NxShape *const* pShapeArray = m_actor->getShapes();
		U32 shapeCount = m_actor->getNbShapes();
		for ( U32 i = 0; i < shapeCount; i++ )      
			pShapeArray[i]->setFlag( NX_SF_DISABLE_RAYCASTING, false );

	}
	else
	{
		m_actor->raiseActorFlag( NX_AF_DISABLE_COLLISION );
		m_actor->raiseBodyFlag( NX_BF_KINEMATIC );  
		m_actor->putToSleep();

		NxShape *const* pShapeArray = m_actor->getShapes();
		U32 shapeCount = m_actor->getNbShapes();
		for ( U32 i = 0; i < shapeCount; i++ )      
			pShapeArray[i]->setFlag( NX_SF_DISABLE_RAYCASTING, true );      
	}

}

bool PhysShapePhysX::isEnabled()
{	
	return !m_actor->readActorFlag(NX_AF_DISABLE_COLLISION);
}

bool PhysShapePhysX::isActive()
{	
	return !m_actor->isSleeping();
}
void PhysShapePhysX::setActive(bool flg)
{	
	if (flg)
		m_actor->wakeUp();
	else
		m_actor->putToSleep();
}
