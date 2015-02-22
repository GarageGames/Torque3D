//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _RIGIDBODY_H_
#define _RIGIDBODY_H_

#include "collision/boxConvex.h"
// BlissGMK - guidebot >>
#include "T3D/BlissGMK/physics/physBody.h"
#include "T3D/BlissGMK/physics/physShape.h"
#include "component/simComponent.h"
// BlissGMK - guidebot <<

//----------------------------------------------------------------------------
class RigidBodyData : public PhysicsBodyData
{
	typedef PhysicsBodyData Parent;

protected:
	bool onAdd();

	//-------------------------------------- Console set variables
public:


public:
	RigidBodyData();
	~RigidBodyData();

	static void initPersistFields();
	void packData(BitStream*);
	void unpackData(BitStream*);
	bool preload(bool server, String &errorStr);

	U8 mShapeType;
	EulerF mRotation;
	VectorF mPos;
	bool mOnlyOnClient;
	
	F32 mDampOnCollisionFactorL;
	F32 mDampOnCollisionFactorA;
	
	DECLARE_CONOBJECT(RigidBodyData);
};

//----------------------------------------------------------------------------
class RigidBody: public PhysBody
{
	typedef PhysBody Parent;

private:
	RigidBodyData* mDataBlock;

protected:


	struct StateDelta 
	{
		F32 dt;                       ///< Last interpolation time
		// Interpolation data
		Point3F pos;
		Point3F posVec;
		QuatF rot[2];

	};

	StateDelta mDelta;

	StrongRefPtr<PhysShape>	mPhysShape;
	//PhysShape* mPhysShape;

	MatrixF mPhysPivot;
	MatrixF mPhysPivotInv;

	Point3F mPhysPosition;
	QuatF mPhysRotation;
	VectorF mForce;
	VectorF mTorque;
	VectorF mLinVelocity;
	VectorF mAngVelocity;

	//Saving for the tick info for the
	//most significant contact.
	bool m_haveContact;
	VectorF m_contactPos;
	F32 m_contactVel;
	F32 m_contactNormalVelDot;

	void setPosition(const Point3F& pos,const QuatF& rot);
	void setRenderPosition(const Point3F& pos,const QuatF& rot);

	void writePacketData(GameConnection * conn, BitStream *stream);
	void readPacketData (GameConnection * conn, BitStream *stream);

	virtual void createPhysShape();

	/*// BlissGMK - guidebot >>
	virtual void createWorldObject();
	virtual void destroyWorldObject();
	// BlissGMK - guidebot <<*/

public:
	RigidBody();
	~RigidBody();

	static void initPersistFields();
	void processTick(const Move *move);
	bool onAdd();
	void onRemove();
	bool onNewDataBlock(GameBaseData* dptr, bool reload);

	void interpolateTick(F32 dt);
	void setTransform(const MatrixF& mat);

	void reset();

	U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn,           BitStream *stream);

	DECLARE_CONOBJECT(RigidBody);

	void setPhysPos(const VectorF& pos);
	void addForce(const VectorF& pos);
	void applyImpulse(const Point3F &r, const Point3F &impulse);
	void setEnabled( const bool enabled );

	PhysShape* getPhysShape() {return mPhysShape;};

	virtual void onContact(const VectorF& pos, const VectorF& vel, const VectorF& normal);
};


#endif
