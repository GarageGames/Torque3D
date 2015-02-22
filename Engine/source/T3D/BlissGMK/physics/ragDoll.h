//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _RAGDOLL_H_
#define _RAGDOLL_H_


#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/BlissGMK/physics/physBody.h"
#include "T3D/BlissGMK/physics/physJoint.h"
#include "ts/tsShapeInstance.h"

#include <string>
#include <map>
#include <set>


//----------------------------------------------------------------------------
struct RagDollData: public PhysicsBodyData 
{
	typedef PhysicsBodyData Parent;

	RagDollData();
	DECLARE_CONOBJECT(RagDollData);
	
	static void initPersistFields();
	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);


	enum Constants 
	{
		MaxBones    = 32,
	};

	const char*				boneParentNodeName	[MaxBones];
	const char*				boneNodeName		[MaxBones];
	VectorF					boneSize			[MaxBones];
	float					boneMass			[MaxBones];
	VectorF					boneOffset			[MaxBones];
	VectorF					boneRotAngles		[MaxBones];
	U8						boneShape			[MaxBones];
	U8					boneJointType		[MaxBones];
	VectorF					boneJointParam		[MaxBones];
	VectorF					boneJointParam2		[MaxBones];

	bool m_manualBoneRotations;	//bone rotations  are defined in scripts 
};


//----------------------------------------------------------------------------

class RagDoll : public PhysBody
{
	typedef PhysBody Parent;
	RagDollData* mDataBlock;
    
public:
	DECLARE_CONOBJECT(RagDoll);

	RagDoll();
	virtual ~RagDoll();

	static void initPersistFields();

	bool onAdd();
	void onRemove();
	bool onNewDataBlock(GameBaseData* dptr, bool reload);

	void processTick(const Move *move);
	void interpolateTick(F32 delta);

	U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn,           BitStream *stream);

	virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);

	struct boneInfo
	{
		boneInfo():physShape(NULL),constraint(NULL){};
		StrongRefPtr<PhysShape> physShape;
		StrongRefPtr<PhysJoint> constraint;
		MatrixF pivot;
		MatrixF invPivot;
		MatrixF model_transform;
		MatrixF model_inverse_transform;
		//Current and previous local transformations.
		QuatF curRot;
		Point3F curPos;
		QuatF prevRot;
		Point3F prevPos;

		std::string node_name;
			
		S32 bode_idx;
		S32 node_idx;
	};

	MatrixF mPrevToCur;

	typedef std::map<S32, boneInfo> boneInfoMap;
	virtual void applyImpulse(const Point3F& pos, const VectorF& vec);


protected:
	S32 mRootBoneNodeIdx;

	boneInfoMap*	mpBones;
	boneInfoMap&	getBones() {AssertFatal(mpBones, "mpBones == NULL!"); return *mpBones;}

	void createRagDoll();
	void destroyRagDoll();

	

	std::size_t addBone(S32 bode_idx, const char* parent_node_name, 
		const char* node_name,
		const VectorF& size,
		const VectorF& offset,const VectorF& rotAngels,
		float mass,int shapeType,int jointType,VectorF jointParam,VectorF jointParam2);

	void setRagDoll();
public:
	void initRagDoll(ShapeBase* object);
	void updateRagDoll();
	
protected:
	MatrixF calcNodeTransform(S32 node_idx);
	std::set<S32> mCalculatedNones;
	Physics* mPhysics;

	bool m_interpolatingBones;
	F32 m_interpolateTime;
};

#endif
