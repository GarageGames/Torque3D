//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPE_H_
#define _PHYSSHAPE_H_

#include "math/mMatrix.h"
#include "math/mPoint3.h"
#include "math/mQuat.h"
#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/physics/physicsUserData.h"
#include "core/util/refBase.h"
#include "core/stream/bitStream.h"

#include "collision/concretePolyList.h"
class PhysBody;
struct RayInfo;

struct PhysInfo
{
	PhysInfo::PhysInfo()
	{
		shapeType = ST_BOX;
		mass = 0.f;
		params = VectorF::Zero;
		bodyType = BT_STATIC;
		owner = NULL;
		transform.identity();
	}

	enum ShapeType {ST_BOX, ST_SPHERE, ST_CAPSULE, ST_CYLINDER, ST_SOFTMESH,ST_MESH};
	ShapeType	shapeType;
	F32			mass;
	VectorF		params;
	MatrixF		transform;

	enum BodyType {BT_STATIC, BT_DYNAMIC};
	BodyType bodyType;
	PhysBody* owner;
};

struct PhysSoftInfo: PhysInfo
{
	PhysSoftInfo() : physPolyList(NULL),poseMatchKoef(0.f) {};
	ConcretePolyList* physPolyList;
	Vector<Point3F> attachPoints;
	float			poseMatchKoef;
};

class PhysShape : public StrongRefBase
{
public:
	virtual Point3F	getPosition();
	virtual void	setPosition(const Point3F& pos);

	virtual QuatF	getRotation();
	virtual void	setRotation(const QuatF& rot);
	
	virtual void	setTransform(const MatrixF& mat);
	virtual MatrixF	getTransform();

	virtual VectorF	getLinVelocity() = 0;
	virtual void	setLinVelocity(const VectorF& vel) = 0;
	
	virtual VectorF	getAngVelocity() = 0;
	virtual void	setAngVelocity(const VectorF& vel) = 0;

	virtual void	setForce(const VectorF& force) = 0;
	virtual VectorF getForce() = 0;
	virtual void	setTorque(const VectorF& torque) = 0;
	virtual VectorF getTorque() = 0;

	virtual void	addForce(const VectorF& force) = 0;
	virtual void	addForce(const VectorF& force, const Point3F& pos) = 0;
	virtual void	reset() = 0;

	virtual void	setEnable(bool isEnabled) = 0;
	virtual bool	isEnabled() = 0;

	virtual bool	isActive() = 0;
	virtual void 	setActive(bool flg) = 0;

	virtual void	applyDampingOnCollistion(float dampFactorLinear, float dampFactorAngular) {};
	PhysBody*		getOwner() {return m_physInfo.owner;}
	PhysInfo&		getInfo() {return m_physInfo;}

	virtual void	pack(BitStream* stream);
	virtual void	unpack(BitStream* stream);

	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info) {return false;};
	virtual bool castRayLocal(const Point3F &startLocal, const Point3F &endLocal, RayInfo* info);
	
	PhysShape();
	PhysShape(const PhysInfo& physDescr );
	~PhysShape();

protected:
	virtual void	setPhysicTransform(const MatrixF&) = 0;
	virtual void	getPhysicTransform(MatrixF &) = 0;

	PhysInfo m_physInfo;
	MatrixF mTransformInv;
	bool mPrevActive;
	PhysicsUserData mUserData;
};

class PhysShapeSoft: public PhysShape
{
public:
	PhysShapeSoft();
	PhysShapeSoft(const PhysSoftInfo& physSoftInfo );
	~PhysShapeSoft();
	virtual VectorF getNodePos(int idx) = 0;
	virtual int		getNodesNum() = 0;
protected:
	PhysSoftInfo m_physSoftInfo;
	virtual Point3F getCOM() = 0;
	MatrixF m_localCOMTransform;
};

#endif