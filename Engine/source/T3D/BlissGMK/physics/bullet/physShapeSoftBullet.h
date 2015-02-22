//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPESOFTBULLET_H_
#define _PHYSSHAPESOFTBULLET_H_

#include "T3D/BlissGMK/physics/bullet/physicsBullet.h"
#include "T3D/BlissGMK/physics/physShape.h"
#include "T3D/physics/bullet/bt.h"

class PhysShapeSoftBullet: public PhysShapeSoft
{
public:
	PhysShapeSoftBullet(Physics* phys, const PhysSoftInfo &physDescr);
	virtual ~PhysShapeSoftBullet();

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

	btSoftBody*		getBody(){return m_softBody;};
	virtual VectorF getNodePos(int idx);
	virtual int		getNodesNum();

	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);
	size_t getNearestNode(VectorF pos);

	virtual Point3F getCOM();

	btSoftRigidDynamicsWorld* m_world;

	btSoftBody* m_softBody;
	Vector<U32> mIndexBuffer;
};

#endif