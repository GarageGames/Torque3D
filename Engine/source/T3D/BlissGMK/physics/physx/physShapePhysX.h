//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPEBULLET_H_
#define _PHYSSHAPEBULLET_H_

#include "T3D/logickingMechanics/physics/physX/physicsPhysX.h"
#include "T3D/logickingMechanics/physics/physShape.h"


class PhysShapePhysX: public PhysShape
{
public:
	PhysShapePhysX(Physics* phys, const PhysInfo &physDescr);
	virtual ~PhysShapePhysX();

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

	virtual void	setEnable(bool isEnabled);
	virtual bool	isEnabled();
	virtual bool	isActive();
	virtual void	setActive(bool flg);

	NxActor* getActor(){return m_actor;};
protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);
private:
	MatrixF mTm;
	PxWorld *m_world;
	NxScene* m_scene;
	NxActor *m_actor;
};

#endif