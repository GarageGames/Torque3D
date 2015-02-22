//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _VISUALEFFECT_H_
#define _VISUALEFFECT_H_

#include "T3D/fx/explosion.h"

//--------------------------------------------------------------------------
class VisualEffectData : public GameBaseData 
{
	typedef GameBaseData Parent;
public:
	VisualEffectData();
	DECLARE_CONOBJECT(VisualEffectData);
	bool onAdd();
	bool preload(bool server, String &errorStr);
	static void  initPersistFields();
	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);


	ExplosionData*    explosion;
	S32               explosionID;

	S32		m_damagePeriod;
	F32		m_damageRadius;
	F32		m_damageForce;
	F32		m_impulseForce;
	StringTableEntry m_damageType;
};


//--------------------------------------------------------------------------
class VisualEffect : public GameBase 
{
private:
	typedef GameBase Parent;
	VisualEffectData*   mDataBlock;

public:
	DECLARE_CONOBJECT( VisualEffect );

	VisualEffect();
	virtual ~VisualEffect();

	static void initPersistFields();
	bool onNewDataBlock(GameBaseData* dptr, bool reload);

	virtual bool onAdd();
	virtual void onRemove();

	U32  packUpdate  ( NetConnection *conn, U32 mask, BitStream *stream );
	void unpackUpdate( NetConnection *conn,           BitStream *stream );

	void processTick(const Move *move);
	
	bool explode();
	void applyDamage();

protected:
	SimTime m_damageLastTime;
	U32 mCurrMS;
	U32 mEndingMS;
	U32 mDelayMS;
	bool mIsExploded;
};

#endif
