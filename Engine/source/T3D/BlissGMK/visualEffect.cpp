//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "T3D/BlissGMK/visualEffect.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"

//----------------------------------------------------------------------------
//
IMPLEMENT_CO_DATABLOCK_V1(VisualEffectData);

VisualEffectData::VisualEffectData()
  : explosion( NULL ),
	explosionID( 0 )
{
	m_damageRadius = 1;
	m_damageForce = 0;
	m_impulseForce = 0;
	m_damageType = "Radius";

	// negative period means one time damage
	m_damagePeriod = -1;	
}


void VisualEffectData::initPersistFields()
{
	Parent::initPersistFields();

	addField("damagePeriod",	TypeS32,		Offset(m_damagePeriod,	VisualEffectData));
	addField("damageRadius",	TypeF32,		Offset(m_damageRadius,	VisualEffectData));
	addField("damageForce",		TypeF32,		Offset(m_damageForce,	VisualEffectData));
	addField("impulseForce",	TypeF32,		Offset(m_impulseForce,	VisualEffectData));
	addField("damageType",		TypeString,		Offset(m_damageType,	VisualEffectData));

    addField("explosion", TYPEID< ExplosionData >(), Offset(explosion, VisualEffectData));
}

bool VisualEffectData::onAdd()
{
	if (Parent::onAdd() == false)
		return false;
	return true;
}

void VisualEffectData::packData(BitStream* stream)
{
	Parent::packData(stream);

	if( stream->writeFlag( explosion != NULL ) )
	{
		stream->writeRangedU32( explosion->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
	}
}

void VisualEffectData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	if( stream->readFlag() )
	{
		explosionID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
	}
}

bool VisualEffectData::preload(bool server, String &errorStr)
{
	if (Parent::preload(server, errorStr) == false)
		return false;
	
	// Resolve objects transmitted from server
	if (!server) {

		if( !explosion && explosionID != 0 )
		{
			if( Sim::findObject( explosionID, explosion ) == false)
			{
				Con::errorf( ConsoleLogEntry::General, "VisualEffectData::preload: Invalid packet, bad datablockId(explosion): 0x%x", explosionID );
			}
			AssertFatal(!(explosion && ((explosionID < DataBlockObjectIdFirst) || (explosionID > DataBlockObjectIdLast))),
				"VisualEffectData::preload: invalid explosion data");
		}
	}
	return true;
}


//-----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(VisualEffect);


//////////////////////////////////////////////////////////////////////////
VisualEffect::VisualEffect()
{
	 mNetFlags.set(Ghostable | ScopeAlways);

	 mDelayMS = 0;
	 mCurrMS = 0;
	 mEndingMS = 1000;
	 mIsExploded = false;
}

VisualEffect::~VisualEffect()
{
}

bool VisualEffect::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<VisualEffectData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
		return false;

	scriptOnNewDataBlock();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void VisualEffect::initPersistFields()
{
	Parent::initPersistFields();
}


//////////////////////////////////////////////////////////////////////////
bool VisualEffect::onAdd()
{
	if(!Parent::onAdd()) return false;

	if (isClientObject())
	{
		//create explosion
		Explosion* pExplosion = new Explosion;
		pExplosion->onNewDataBlock(mDataBlock->explosion, false);
		pExplosion->setTransform(getTransform());
		if (pExplosion->registerObject() == false)
		{
			Con::errorf(ConsoleLogEntry::General, "VisualEffect(%s)::onAdd: couldn't register explosion",
				mDataBlock->getName() );
			delete pExplosion;
			pExplosion = NULL;
		}
	}

	mDelayMS = mDataBlock->explosion ? mDataBlock->explosion->delayMS : 0;
	mEndingMS = mDataBlock->explosion ?  mDataBlock->explosion->lifetimeMS : 0;

	addToScene();
	if (isServerObject())
		scriptOnAdd();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void VisualEffect::onRemove()
{
	scriptOnRemove();
	removeFromScene();
	Parent::onRemove();
}

//////////////////////////////////////////////////////////////////////////
void VisualEffect::processTick(const Move *move)
{
	if(isServerObject())
	{
		SimTime curTime = Sim::getCurrentTime();
		if(	mDataBlock->m_damagePeriod > 0 && 
			curTime - m_damageLastTime > mDataBlock->m_damagePeriod)
		{
			m_damageLastTime = curTime;
			applyDamage();
		}

		mCurrMS += TickMs;
		if( (mCurrMS > mDelayMS))
		{
			explode();
		}

		if( mEndingMS > 0 &&  mCurrMS >= mEndingMS )
		{
			deleteObject();
			return;
		}
	}	
}

U32  VisualEffect::packUpdate  ( NetConnection *conn, U32 mask, BitStream *stream )
{
	U32 retMask = Parent::packUpdate(conn, mask, stream);
	if (stream->writeFlag(mask & InitialUpdateMask))
	{
		mathWrite(*stream,mObjToWorld);
	}
	
	return retMask;
}
void VisualEffect::unpackUpdate( NetConnection *conn, BitStream *stream )
{
	Parent::unpackUpdate(conn, stream);
	if (stream->readFlag())
	{
		MatrixF mat;
		mathRead(*stream, &mat);
		setTransform(mat);
		setRenderTransform(mat);
	}	
}

void VisualEffect::applyDamage()
{
	if(mDataBlock->m_damageRadius > 0.f)
		Con::executef(mDataBlock, "doDamage", getIdString(), 
						Con::getFloatArg(mDataBlock->m_damageRadius),
						Con::getFloatArg(mDataBlock->m_damageForce), mDataBlock->m_damageType,
						Con::getFloatArg(mDataBlock->m_impulseForce));
}

bool VisualEffect::explode()
{
	if(isServerObject() && mDataBlock->m_damagePeriod < 0 && !mIsExploded)
	{
		applyDamage();
		mIsExploded = true;
	}
	return true;
}