//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/BlissGMK/physics/physBody.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"

IMPLEMENT_CO_DATABLOCK_V1(PhysicsBodyData);
IMPLEMENT_CO_NETOBJECT_V1(PhysBody);



PhysicsBodyData::PhysicsBodyData()
{
	m_minContactSpeed = 10.f;
}

PhysicsBodyData::~PhysicsBodyData()
{
}

//----------------------------------------------------------------------------


bool PhysicsBodyData::onAdd()
{
	if(!Parent::onAdd())
		return false;

	return true;
}


bool PhysicsBodyData::preload(bool server, String &errorStr)
{
	if (!Parent::preload(server, errorStr))
		return false;
	return true;
}   


//----------------------------------------------------------------------------
void PhysicsBodyData::packData(BitStream* stream)
{
	Parent::packData(stream);
	stream->write(m_minContactSpeed);
}   

void PhysicsBodyData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	stream->read(&m_minContactSpeed);
}   

//----------------------------------------------------------------------------
void PhysicsBodyData::initPersistFields()
{
	Parent::initPersistFields();
	addField("minContactSpeed", TypeF32, Offset(m_minContactSpeed,	PhysicsBodyData));
}   


//----------------------------------------------------------------------------

PhysBody::PhysBody()
{
	m_haveContact = false;
	m_contactPos = VectorF::Zero;
	m_contactVel = 0.f;
	m_contactNormalVelDot = 0.f;

	mHasServerPhysic = false;
}   

PhysBody::~PhysBody()
{

}


//----------------------------------------------------------------------------

bool PhysBody::onAdd()
{
	if (!Parent::onAdd())
		return false;
	return true;
}

void PhysBody::onRemove()
{
	Parent::onRemove();
}


bool PhysBody::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<PhysicsBodyData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
		return false;
	scriptOnNewDataBlock();
	return true;
}
//----------------------------------------------------------------------------

void PhysBody::processTick(const Move* move)
{     
	Parent::processTick(move);
	notifyContact();
}


void PhysBody::interpolateTick(F32 dt)
{     
	Parent::interpolateTick(dt);
}


//----------------------------------------------------------------------------
void PhysBody::applyImpulse(const Point3F &pos, const Point3F &impulse)
{
}

//----------------------------------------------------------------------------

void PhysBody::writePacketData(GameConnection *connection, BitStream *stream)
{
	Parent::writePacketData(connection, stream);
}

void PhysBody::readPacketData(GameConnection *connection, BitStream *stream)
{
	Parent::readPacketData(connection, stream);
}   


//----------------------------------------------------------------------------

U32 PhysBody::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 retMask = Parent::packUpdate(con, mask, stream);
	return retMask;
}   

void PhysBody::unpackUpdate(NetConnection *con, BitStream *stream)
{
	Parent::unpackUpdate(con,stream);
}

//----------------------------------------------------------------------------
void PhysBody::setEnabled(const bool enabled)
{
	Parent::setEnabled(enabled);

	// When enabling add a tiniest force, to
	// prevent body from flying motionless in the air.
	if(enabled)
	{
		addForce(VectorF(0.0001f, 0.0001f, 0.0001f));
	}
}

//----------------------------------------------------------------------------
void PhysBody::onContact(const VectorF& pos, const VectorF& vel, const VectorF& normal)
{
	if(isGhost()) return;

	if(vel.lenSquared() < mDataBlock->m_minContactSpeed * mDataBlock->m_minContactSpeed)
		return;

	// velNormDot ~ 1 means we're colliding obstacle,
	// velNormDot ~ 0 means we're sliding on some surface,
	F32 velLen = vel.len();
	VectorF velN = vel;
	velN.normalize();
	F32 velNormDot = mFabs(mDot(velN, normal));

	if(m_haveContact)
	{
		if(velNormDot > m_contactNormalVelDot)
		{
			m_contactPos = pos;
			m_contactVel = velLen;
			m_contactNormalVelDot = velNormDot;
		}
	}
	else
	{
		m_haveContact = true;
		m_contactPos = pos;
		m_contactVel = velLen;
		m_contactNormalVelDot = velNormDot;
	}
}

void PhysBody::notifyContact()
{
	if(!m_haveContact) return;

	char buff0[256];
	char buff1[32];
	char buff2[32];

	dSprintf(buff0, sizeof(buff0),"%g %g %g", m_contactPos.x, m_contactPos.y, m_contactPos.z);
	dSprintf(buff1, sizeof(buff1),"%g", m_contactVel);
	dSprintf(buff2, sizeof(buff2),"%g", m_contactNormalVelDot);

	Con::executef(mDataBlock, "onContact", getIdString(), buff0, buff1, buff2);

	m_haveContact = false;
}

//----------------------------------------------------------------------------

void PhysBody::initPersistFields()
{
	Parent::initPersistFields();
}

void PhysBody::reset()
{
}

void PhysBody::addForce(VectorF pos)
{
}

ConsoleMethod(PhysBody, reset, void, 2, 2, "")
{
	object->reset();
}

ConsoleMethod(PhysBody, addForce, void, 3, 3, "")
{
	VectorF force(0.f,0.f,0.f);
	dSscanf(argv[2], "%g %g %g", &force.x, &force.y, &force.z);
	object->addForce(force);
}

ConsoleMethod(PhysBody, setEnabled, void, 3, 3, "")
{
	int isEnabled;
	dSscanf(argv[2], "%d", &isEnabled);
	object->setEnabled((bool)isEnabled);
}