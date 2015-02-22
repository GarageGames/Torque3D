//-----------------------------------------------------------------------------
// Copyright 2009 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPE_DUMMY_H_
#define _PHYSSHAPE_DUMMY_H_

#include "T3D/BlissGMK/physics/physicsDummy.h"
#include "T3D/BlissGMK/physics/physShape.h"


class PhysShapeDummy: PhysShape
{
public:
	PhysShapeDummy(Physics* phys, const PhysInfo& physDescr);
	PhysShapeDummy(Physics* phys, const void* vBuffer, int vNum, int vStride, 
		const void* iBuffer, int iNum, int triStride);
	virtual ~PhysShapeDummy();

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
	virtual bool	isActive() {return isEnabled();};
	virtual void 	setActive(bool flg){};
protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);
private:
	MatrixF mObjToWorld;
	bool m_enabled;
};

class PhysShapeSoftDummy:  public PhysShapeSoft
{
public:
	PhysShapeSoftDummy(Physics* phys, const PhysSoftInfo& physDescr );
	~PhysShapeSoftDummy();
	virtual VectorF getNodePos(int idx) {return VectorF::Zero;};
	virtual int		getNodesNum(){return 0;};

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
	virtual bool	isActive() {return isEnabled();};
	virtual void 	setActive(bool flg){};
protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);

	virtual Point3F getCOM();
	MatrixF m_localCOM;

private:
	MatrixF mObjToWorld;
	bool m_enabled;
};

#endif