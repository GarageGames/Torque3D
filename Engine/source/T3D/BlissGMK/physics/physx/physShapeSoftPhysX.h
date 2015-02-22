//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSSHAPESOFTPHYSX_H_
#define _PHYSSHAPESOFTPHYSX_H_

#include "T3D/logickingMechanics/physics/physX/physicsPhysX.h"
#include "T3D/logickingMechanics/physics/physShape.h"


class PhysShapeSoftPhysX: public PhysShapeSoft
{
public:
	PhysShapeSoftPhysX(Physics* phys, const PhysSoftInfo &physDescr);
	virtual ~PhysShapeSoftPhysX();

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

	//btSoftBody*		getBody(){return m_softBody;};
	virtual VectorF getNodePos(int idx);
	virtual int		getNodesNum();

	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
protected:
	virtual void	setPhysicTransform(const MatrixF&);
	virtual void	getPhysicTransform(MatrixF &);
	size_t getNearestNode(VectorF pos);

	PxWorld* m_world;
	NxScene* m_scene;

	NxSoftBodyMesh* m_softBodyMesh;
	NxSoftBody* m_softBody;
	
	NxClothMesh* m_clothMesh;
	NxCloth* m_cloth;
	NxU32    m_vertNum;
	Vector<U32> mIndexBuffer;

	virtual Point3F getCOM();

	Vector<U32> m_attachedVertexIdx;
	ConcretePolyList::VertexList	m_initLocalPos;

};

#endif