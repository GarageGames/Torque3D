//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physBody.h"
#include "T3D/BlissGMK/physics/bullet/physicsBullet.h"
#include "T3D/BlissGMK/physics/bullet/physShapeBullet.h"
#include "T3D/BlissGMK/physics/bullet/physShapeSoftBullet.h"
#include "T3D/BlissGMK/physics/bullet/physJointBullet.h"

#include "platform/platform.h"
#include "gfx/primBuilder.h"
#include "T3D/shapeBase.h"
#include "T3D/physics/bullet/btWorld.h"

btScalar PhysicsBullet::m_mu = 1.5f;

void PhysicsBullet::initBullet()
{
	//read script options
	const char* friction =  Con::getVariable("$GMK::Physics::Bullet::friction");
	if (dStrcmp(friction,""))
	{
		m_mu = dAtof(friction);
	}
}

Physics* PhysicsBullet::createPhysicsBullet(PhysicsWorld* world)
{
	initBullet();

	Physics* physics =  (Physics*) new PhysicsBullet(world);
	return physics;
}

PhysicsBullet::PhysicsBullet(PhysicsWorld* world)
{
	BtWorld* btWorld = dynamic_cast<BtWorld*>(world);
	if (!btWorld)
		return;
	m_dynamicsWorld = dynamic_cast<btDiscreteDynamicsWorld *>(btWorld->getDynamicsWorld());
	m_softBodyWorldInfo = &btWorld->getSoftBodyWorldInfo();
	gContactAddedCallback = PhysicsBullet::contactAddedCallback;
}

PhysicsBullet::~PhysicsBullet()
{
	m_dynamicsWorld = NULL;
}

PhysShape* PhysicsBullet::createPhysShape(const PhysInfo& descr)
{
	PhysShape* physShape = (PhysShape*) new PhysShapeBullet(this,descr);
	return physShape;
}
PhysShape* PhysicsBullet::createPhysShape(void* vBuffer,int vNum,int vStride, 
						   void* iBuffer, int iNum, int triStride)
{
	PhysShape* physShape = (PhysShape*) new PhysShapeBullet(this,vBuffer,vNum,vStride,iBuffer,iNum,triStride);
	return physShape;
}

PhysShape* PhysicsBullet::createPhysShapeSoft(const PhysSoftInfo& descr)
{
	PhysShape* physShape = (PhysShape*) new PhysShapeSoftBullet(this,descr);
	return physShape;
}

// Callback contact
    bool PhysicsBullet::contactAddedCallback(btManifoldPoint& cp,  
        const btCollisionObjectWrapper * colObj0,  
        int partId0,  
        int index0,  
        const btCollisionObjectWrapper * colObj1,  
        int partId1,  
        int index1)  
    {  
        if(cp.getLifeTime() == 0)  
        {  
            btVector3 vel = colObj0->getCollisionObject()->getInterpolationLinearVelocity() - colObj1->getCollisionObject()->getInterpolationLinearVelocity();  
            //filter slow contacts even before calls to owner objects  
            if(vel.length2() > Physics::minVelCallbackThreshold *   
                Physics::minVelCallbackThreshold)  
            {  
                PhysBody* owner0 = dynamic_cast<PhysBody*>( PhysicsUserData::getObject(colObj0->getCollisionObject()->getUserPointer()));  
                PhysBody* owner1 = dynamic_cast<PhysBody*>( PhysicsUserData::getObject(colObj1->getCollisionObject()->getUserPointer()));  
                if(owner0)  
                    owner0->onContact(vectorFromBt(cp.getPositionWorldOnA()), vectorFromBt(vel), vectorFromBt(-cp.m_normalWorldOnB));  
                if(owner1)  
                    owner1->onContact(vectorFromBt(cp.getPositionWorldOnB()), vectorFromBt(-vel), vectorFromBt(cp.m_normalWorldOnB));  
            }  
        }  
        return false;  
    }  

PhysJoint* PhysicsBullet::createPhysJoint(PhysJointInfo& descr)
{
	PhysJoint* physJoint = (PhysJoint*) new PhysJointBullet(this,descr);
	return physJoint;
}

// Helper functions
Point3F vectorFromBt(const btVector3& bt)
{
	Point3F res;
	res.x = bt.x();
	res.y = bt.y();
	res.z = bt.z();
	return res;
}

btVector3 vectorToBt(const Point3F& p)
{
	btVector3 vec;
	vec.setX(p.x);
	vec.setY(p.y);
	vec.setZ(p.z);
	return vec;
}

btMatrix3x3 matrix3ToBt(const MatrixF& mat)
{
	btMatrix3x3 m;
	m.setValue( mat[0], mat[1], mat[2],
				mat[4], mat[5], mat[6],
				mat[8], mat[9], mat[10] );
	return m;
}

MatrixF matrix3FromBt(const btMatrix3x3& mat)
{
	MatrixF m(true);

	m.setRow( 0, vectorFromBt( mat[0] ) );
	m.setRow( 1, vectorFromBt( mat[1] ) );
	m.setRow( 2, vectorFromBt( mat[2] ) );

	return m;
}

MatrixF matrix4fromBt(const btTransform & tr)
{
	MatrixF mat = matrix3FromBt(tr.getBasis());
	mat.setPosition(vectorFromBt(tr.getOrigin()));
	return mat;
}

btTransform matrix4toBt(const MatrixF & mat)
{
	btTransform tr;
	tr.setBasis(matrix3ToBt(mat));
	tr.setOrigin(vectorToBt(mat.getPosition()));
	return tr;
}
