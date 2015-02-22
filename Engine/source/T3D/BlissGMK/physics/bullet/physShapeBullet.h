//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPEBULLET_H_
#define _PHYSSHAPEBULLET_H_

#include "T3D/BlissGMK/physics/bullet/physicsBullet.h"
#include "T3D/BlissGMK/physics/physShape.h"


class PhysShapeBullet: public PhysShape
{
public:
	PhysShapeBullet(Physics* phys, const PhysInfo &physDescr);
	PhysShapeBullet(Physics* phys, void* vBuffer,int vNum,int vStride, 
				void* iBuffer, int iNum, int triStride);
	virtual ~PhysShapeBullet();

	virtual VectorF	getLinVelocity();
	virtual void	setLinVelocity(const VectorF& vel);

	virtual VectorF	getAngVelocity();
	virtual void	setAngVelocity(const VectorF& vel);

	virtual void	setForce(const VectorF& force);
	virtual VectorF getForce();
	virtual void	setTorque(const VectorF& torque);
	virtual VectorF getTorque();

	virtual void	addForce(const VectorF& force);
	virtual void	addForce(const VectorF& force, const Point3F& pos);
	virtual void	reset();

	btRigidBody* getBody(){return m_body;};

	virtual void	setEnable(bool isEnabled);
	virtual bool	isEnabled();
	virtual bool	isActive();
	virtual void	setActive(bool flg);

	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);

	virtual void	applyDampingOnCollistion(float dampFactorLinear, float dampFactorAngular);

protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);
private:
	btDiscreteDynamicsWorld* m_world;
	btCollisionShape* m_collShape;
	btRigidBody* m_body;
};

#endif