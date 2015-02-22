//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSICSBULLET_H_
#define _PHYSICSBULLET_H_

#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/physics/bullet/bt.h"

class PhysicDrawer;

class PhysicsBullet: public Physics
{
public:
	static void initBullet();
	static Physics* createPhysicsBullet(PhysicsWorld* world);

	PhysicsBullet(PhysicsWorld* world);
	virtual ~PhysicsBullet();

	virtual PhysShape* createPhysShape(const PhysInfo& descr);
	virtual PhysShape* createPhysShape(void* vBuffer,int vNum,int vStride, 
											void* iBuffer, int iNum, int triStride);
	virtual PhysShape* createPhysShapeSoft(const PhysSoftInfo& descr);

	virtual PhysJoint* createPhysJoint(PhysJointInfo& descr);

	btDiscreteDynamicsWorld* getWorld() {return m_dynamicsWorld;};
	static btScalar getStaticFriction(){ return m_mu;};
    static bool contactAddedCallback(btManifoldPoint& cp,  
            const btCollisionObjectWrapper * colObj0,  
            int partId0,  
            int index0,  
            const btCollisionObjectWrapper * colObj1,  
            int partId1,  
            int index1);  

	btSoftBodyWorldInfo* getSoftBodyWorldInfo() { return m_softBodyWorldInfo; };
private:
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btCollisionDispatcher* m_dispatcher;
	btAxisSweep3* m_overlappingPairCache;
	btSequentialImpulseConstraintSolver* m_solver;
	
	btDiscreteDynamicsWorld* m_dynamicsWorld;
	btSoftBodyWorldInfo*	m_softBodyWorldInfo;
	

	static btScalar m_mu;

};

extern Point3F vectorFromBt(const btVector3& bt);
extern btVector3 vectorToBt(const Point3F& p);
btMatrix3x3 matrix3ToBt(const MatrixF& mat);
btTransform matrix4toBt(const MatrixF & mat);
MatrixF matrix4fromBt(const btTransform & tr);

#endif