//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/bullet/PhysShapeBullet.h"
#include "scene/sceneObject.h"
#include "gfx/sim/debugDraw.h"

#define SPHERE_RESTITUTION 0.2f
#define SPHERE_LDAMPING 0.3f
#define SPHERE_ADAMPING 0.3f

PhysShapeBullet::PhysShapeBullet(Physics* phys, const PhysInfo &physDescr) : PhysShape(physDescr)
{
	PhysicsBullet* physics = static_cast<PhysicsBullet*>(phys);
	m_world = physics->getWorld();

	btScalar friction = 0.2f;
	btScalar restitution = 0.0f;
	btScalar ldamping = 0.0f;
	btScalar adamping = 0.0f;

	m_collShape = NULL;

	if (m_physInfo.shapeType == PhysInfo::ST_BOX)
	{
		btVector3 halfSize = 0.5f * vectorToBt(m_physInfo.params);
		m_collShape = new btBoxShape(halfSize);
	}
	else
	{
		restitution = SPHERE_RESTITUTION;
		ldamping = SPHERE_LDAMPING;
		adamping = SPHERE_ADAMPING;

		if (m_physInfo.shapeType == PhysInfo::ST_SPHERE)
		{
			m_collShape = new btSphereShape(0.5f * m_physInfo.params.x);
		}
		else if (m_physInfo.shapeType == PhysInfo::ST_CAPSULE)
		{
			btVector3 sizes = vectorToBt(m_physInfo.params);
			m_collShape = new btCapsuleShapeZ(0.5f*sizes.x(), sizes.y() - sizes.x());// radius, height
		}
		else if (m_physInfo.shapeType == PhysInfo::ST_CYLINDER)
		{
			btVector3 halfSize = 0.5f * vectorToBt(m_physInfo.params);
			m_collShape = new btCylinderShapeZ(halfSize);
		}
		else
		{
			Con::errorf("PhysShapeBullet::PhysShapeBullet(): Wrong phys type %d",int(m_physInfo.shapeType));
		}
	}
	

	AssertFatal(m_collShape,"Can't create physic collision shape ");

	btScalar mass = m_physInfo.mass;
	btVector3 localInertia(0, 0, 0);
	
	m_physInfo.bodyType = PhysInfo::BT_STATIC;
	if (mass > 0.f + FLT_EPSILON)
	{
		m_collShape->calculateLocalInertia(mass, localInertia);
		m_physInfo.bodyType = PhysInfo::BT_DYNAMIC;
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, NULL, m_collShape, localInertia);
	rbInfo.m_friction = friction;
	rbInfo.m_restitution = restitution;
	rbInfo.m_linearDamping = ldamping;
	rbInfo.m_angularDamping = adamping;
	m_body = new btRigidBody(rbInfo);


	if(m_physInfo.bodyType == PhysInfo::BT_DYNAMIC)
	{
		m_body->setCollisionFlags(m_body->getCollisionFlags() |
			btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	}

	m_body->setUserPointer(&mUserData);

	m_world->addRigidBody(m_body);

	//enable CCD and set margin for small object(i.e. grenades)
	if (m_physInfo.shapeType == PhysInfo::ST_BOX)
	{
		btVector3 sizes = vectorToBt(m_physInfo.params);
		if (sizes.m_floats[sizes.maxAxis()]<0.9f)
		{
			static const float marginGrenade = 0.1f;
			static btScalar frictionGr = 0.5f;
			//static btScalar ldampingGr = 0.5f;
			//static btScalar adampingGr = 0.7f;
			
			
			btScalar treshhold = 0.5f*sizes.m_floats[sizes.minAxis()];
			m_body->setCcdMotionThreshold(treshhold);
			m_collShape->setMargin(marginGrenade);
			m_body->setCcdSweptSphereRadius(treshhold);
			
			m_body->setFriction(frictionGr);
			//m_body->setDamping(ldampingGr,adampingGr);
			
		}		
	}
}

PhysShapeBullet::PhysShapeBullet(Physics* phys,void* vBuffer,int vNum,int vStride, 
									void* iBuffer, int iNum, int triStride)
{
	PhysicsBullet* physics = static_cast<PhysicsBullet*>(phys);
	m_world = physics->getWorld();
	m_physInfo.mass = 0.f;
	m_physInfo.shapeType = PhysInfo::ST_MESH;

	//transform into bullet coords
	VectorF* it = (VectorF*)vBuffer;
	for (int i=0;i<vNum;i++)
	{
		F64 temp = it->y;
		it->y = it->z;
		it->z = temp;
		it++;
	}

	btTriangleIndexVertexArray*	indexVertexArrays = new btTriangleIndexVertexArray(iNum/3,(int *)iBuffer,triStride,
		vNum,(btScalar*)vBuffer,vStride);
	
	m_collShape  = new btBvhTriangleMeshShape(indexVertexArrays,true);
	
	btScalar mass(0.f);
	btVector3 localInertia(0,0,0);
	m_physInfo.bodyType = PhysInfo::BT_STATIC;

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,NULL,m_collShape,localInertia);
	rbInfo.m_friction = PhysicsBullet::getStaticFriction();
	rbInfo.m_restitution = 1.f;
	m_body = new btRigidBody(rbInfo);

	m_body->setUserPointer(this);
	m_world->addRigidBody(m_body);

	mTransformInv = m_physInfo.transform;
	mTransformInv.inverse();
}

PhysShapeBullet::~PhysShapeBullet()
{
	m_world->removeRigidBody(m_body);
	delete m_body;
	if (m_physInfo.shapeType==PhysInfo::ST_MESH)
	{
		btBvhTriangleMeshShape* meshShape = static_cast<btBvhTriangleMeshShape*>(m_collShape);
		btStridingMeshInterface* meshInterface = meshShape->getMeshInterface();
		if (meshInterface)
			delete meshInterface;
	}
	delete m_collShape;	
}

void PhysShapeBullet::setPhysicTransform(const MatrixF& tr)
{
	btTransform& physTr = m_body->getWorldTransform();
	physTr = matrix4toBt(tr);
}

void PhysShapeBullet::getPhysicTransform(MatrixF & tr)
{
	btTransform& physTr = m_body->getWorldTransform();
	tr = matrix4fromBt(physTr);
}

VectorF PhysShapeBullet::getLinVelocity()
{
	btVector3 vel = m_body->getLinearVelocity();
	VectorF v = vectorFromBt(vel);
	
	return v;	
}

void PhysShapeBullet::setLinVelocity(const VectorF& vel)
{
	btVector3 v = vectorToBt(vel);
	m_body->setLinearVelocity(v);
}

VectorF PhysShapeBullet::getAngVelocity()
{
	btVector3 vel = m_body->getAngularVelocity();
	VectorF v = vectorFromBt(vel);

	return v;	
}

void PhysShapeBullet::setAngVelocity(const VectorF& vel)
{
	btVector3 v = vectorToBt(vel);
	m_body->setAngularVelocity(v);
}

void PhysShapeBullet::addForce(const VectorF& force)
{
	m_body->activate(true);
	btVector3 f = vectorToBt(force);
	//m_body->applyCentralImpulse(f);
	m_body->applyCentralForce(f);
}

void PhysShapeBullet::addForce(const VectorF& forceAdd, const Point3F& pos)
{
	m_body->activate(true);
	static F32 scaler = 1.f;
	VectorF force = scaler * forceAdd;

	btVector3 f = vectorToBt(force);
	btVector3 p = vectorToBt(pos);
	btVector3 relPos = p - m_body->getWorldTransform().getOrigin();
	m_body->applyForce(f, relPos);
}

void PhysShapeBullet::applyDampingOnCollistion(float dampFactorLinear, float dampFactorAngular)
{
	m_body->setLinearVelocity(dampFactorLinear*m_body->getLinearVelocity());
	m_body->setAngularVelocity(dampFactorAngular*m_body->getAngularVelocity());
}

void PhysShapeBullet::reset()
{
	m_body->clearForces();
	setLinVelocity(VectorF::Zero);
	setAngVelocity(VectorF::Zero);
}


void PhysShapeBullet::setForce(const VectorF& force)
{
	m_body->activate(true);
	m_body->clearForces();
	btVector3 f = vectorToBt(force);
	m_body->applyCentralForce(f);
}

VectorF  PhysShapeBullet::getForce()
{
	btVector3 f = m_body->getTotalForce();
	VectorF force = vectorFromBt(f);
	return force;	
}
void  PhysShapeBullet::setTorque(const VectorF& torque)
{
	m_body->activate(true);
	btVector3 t = vectorToBt(torque);
	m_body->applyTorque(t);
}
VectorF  PhysShapeBullet::getTorque()
{
	btVector3 t = m_body->getTotalTorque();
	VectorF torque = vectorFromBt(t);
	return torque;
}

void PhysShapeBullet::setEnable(bool isEnabled)
{
	if(isEnabled)
	{
		m_body->forceActivationState(ACTIVE_TAG);
		m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);

	}
	else
	{
		m_body->setActivationState(DISABLE_SIMULATION);
		m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

bool PhysShapeBullet::isEnabled()
{	
	return m_body->getActivationState()!=DISABLE_SIMULATION;
}

bool PhysShapeBullet::isActive()
{	
	return m_body->getActivationState()!=ISLAND_SLEEPING;
}
void PhysShapeBullet::setActive(bool flg)
{	
	if (flg)
		m_body->forceActivationState(ACTIVE_TAG);
	else
		m_body->forceActivationState(ISLAND_SLEEPING);
}


bool PhysShapeBullet::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
	btVector3 from  = vectorToBt(start);
	btVector3 to  = vectorToBt(end);
	btTransform fromTr;
	fromTr.setIdentity(); 
	fromTr.setOrigin(from);
	btTransform toTr;
	toTr.setIdentity(); 
	toTr.setOrigin(to);

	btCollisionWorld::ClosestRayResultCallback result(from,to);

	btCollisionWorld::rayTestSingle(fromTr,toTr,(btCollisionObject*)m_body,
		m_body->getCollisionShape(),m_body->getWorldTransform(),(btCollisionWorld::RayResultCallback&)result);
	if (result.hasHit())
	{
		if (info)
		{
			info->t        = result.m_closestHitFraction; // finally divide...
			info->setContactPoint( start, end );

			info->material = NULL;
			info->object = (SceneObject*)m_physInfo.owner;
			//force will be applying in direction opposite to normal
			
			info->normal   = start-end;//vectorFromBt(result.m_hitNormalWorld);
			info->normal.normalize();
			

		}

		return true;
	}
	return false;
}

class DrawTriangleCallback: btTriangleCallback
{
	DebugDrawer* m_drawer;
public:
	DrawTriangleCallback(){ 
		m_drawer = DebugDrawer::get();
	};
	virtual ~DrawTriangleCallback(){};
	virtual void processTriangle(btVector3* triangle, int partId, int triangleIndex)
	{
		VectorF a = vectorFromBt(triangle[0]);
		VectorF b = vectorFromBt(triangle[1]);
		VectorF c = vectorFromBt(triangle[2]);
		m_drawer->drawTri(a,b,c);
		m_drawer->setLastTTL(DebugDrawer::DD_INFINITE);
	};
};

