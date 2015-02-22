//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _SOFTBODY_H_
#define _SOFTBODY_H_

#include "T3D/BlissGMK/physics/rigidBody.h"

class SceneRenderState;

//----------------------------------------------------------------------------
class SoftBodyData : public RigidBodyData
{
	typedef RigidBodyData Parent;
public:
	SoftBodyData();
	~SoftBodyData();

	enum SoftBodyConsts 
	{
		MaxAttachedPoints    = 8,
	};

	static void initPersistFields();
	void packData(BitStream*);
	void unpackData(BitStream*);

	S8 attachedPointsNum;
	VectorF attachedPoints[MaxAttachedPoints];
	float poseMatchKoef;
	DECLARE_CONOBJECT(SoftBodyData);
};


//----------------------------------------------------------------------------
class SoftBody: public RigidBody
{
	typedef RigidBody Parent;
public:
	SoftBody();
	~SoftBody();

	DECLARE_CONOBJECT(SoftBody);

	void prepBatchRender(SceneRenderState* state, S32 mountedImageIndex );
	void processTick(const Move* move);
	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
	
	U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn,           BitStream *stream);

	void setTransform(const MatrixF& mat);
private:
	SoftBodyData* mDataBlock; 

protected:

	bool onNewDataBlock(GameBaseData* dptr, bool reload);
	bool onAdd();
	void onRemove();
	void createPhysShape();

	ConcretePolyList* mPhysPolyList;
	Vector<U32> mIndexBuffer;

	PhysShapeSoft* m_physShapeSoft;

	ConcretePolyList::IndexList m_vertexBindingVec;

	void createUniqVertexList();

	void onEditorEnable();
	void onEditorDisable();

	bool m_stopSimulation;
	MatrixF m_initTransform;
};


#endif
