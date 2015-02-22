//-----------------------------------------------------------------------------
// Copyright 2009 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PHYSICS_BODY_H_
#define _PHYSICS_BODY_H_

#include "T3D/shapeBase.h"

//----------------------------------------------------------------------------
class PhysicsBodyData : public ShapeBaseData
{
	typedef ShapeBaseData Parent;

protected:
	bool onAdd();

public:
	PhysicsBodyData();
	~PhysicsBodyData();

	static void initPersistFields();
	void packData(BitStream*);
	void unpackData(BitStream*);
	bool preload(bool server, String &errorStr);

	F32 m_minContactSpeed;
	DECLARE_CONOBJECT(PhysicsBodyData);
};


//----------------------------------------------------------------------------
class PhysBody: public ShapeBase
{
	typedef ShapeBase Parent;

private:
	PhysicsBodyData* mDataBlock;

protected:
	//Saving for the tick info for the
	//most significant contact.
	bool m_haveContact;
	VectorF m_contactPos;
	F32 m_contactVel;
	F32 m_contactNormalVelDot;

	bool    mHasServerPhysic;
	
	void writePacketData(GameConnection * conn, BitStream *stream);
	void readPacketData (GameConnection * conn, BitStream *stream);

public:
	PhysBody();
	~PhysBody();

	enum MaskBits {
		PositionMask = Parent::NextFreeMask << 0,
		NextFreeMask = PositionMask  << 1,
	};

	static void initPersistFields();
	void processTick(const Move *move);
	bool onAdd();
	void onRemove();
	bool onNewDataBlock( GameBaseData *dptr, bool reload);

	void interpolateTick(F32 dt);

	void reset();
	
	virtual void applyImpulse(const Point3F &r, const Point3F &impulse);
	virtual void onContact(const VectorF& pos, const VectorF& vel, const VectorF& normal);
	virtual void notifyContact();

	U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn,           BitStream *stream);

	DECLARE_CONOBJECT(PhysBody);
	
	virtual void addForce(VectorF pos);
	virtual void setEnabled( const bool enabled );
};


#endif
