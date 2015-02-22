//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _AIBOT_H_
#define _AIBOT_H_

#include "T3D/aiPlayer.h"

class AIBot : public AIPlayer 
{
	typedef AIPlayer Parent;
public:
	DECLARE_CONOBJECT( AIBot );

	AIBot();
	virtual ~AIBot();

	static void initPersistFields();

	virtual bool onAdd();
	virtual void onRemove();

	void processTick(const Move *move);
	F32	calcDestantion(const VectorF &pos);

	GameBase*	getEnemy();
	void		setEnemy(U32 enemy);
	U32			getEnemyId();
	F32			getDistToEnemy();

	void		updateTacticalMovement();
	void		enableChase(bool isChasing, F32 closeChaseDist, F32 farChaseDist);
	void		enableFlee(bool isFleeing, F32 closeFleeDist, F32 farFleeDist);
	void		enableStrafe(bool isStrafing, F32 minStrafeDist, F32 maxStrafeDist, SimTime changeDirTime);
	void		attackParams(F32 dist, F32 fov);
	void		allowMovement(bool isAllowed);
	void		allowTacticalMovement(bool isAllowed);
	
	bool		getAIMove(Move *movePtr);

	void		applyImpulse(const Point3F& pos,const VectorF& vec);

	bool		isDead();

protected:
	void think(SimTime dt);
	bool updateEnemy();

	void wallAvoindance(const VectorF& wallNormal);
	
	U32 m_enemyId;
	GameBase* m_enemy;
	F32 m_distToEnemy;
	
	F32 m_attackFov;
	F32 m_attackDist;

	// Chase
	bool m_isChasing;
	// if enemy is further than m_chaseFarDist, begin chasing
	F32 m_chaseFarDist;
	// if enemy is closer than m_chaseCloseDist, stop chasing
	F32 m_chaseCloseDist;
	
	// Flee
	bool m_isFleeing;
	// if enemy is closer than m_fleeCloseDist, start fleeing
	F32 m_fleeCloseDist;
	// if enemy is further than m_fleeFarDist, stop fleeing
	F32 m_fleeFarDist;

	enum TacticalMovement {ReduceDist, HoldDist, IncreaseDist};
	TacticalMovement m_tacticalMovement;
	

	// Strafe
	bool m_isStrafing;
	F32 m_strafeMinDist;
	F32 m_strafeMaxDist;
	SimTime m_strafeChangeDirTime;
	VectorF m_strafeOffset;
	SimTime m_lastStrafeTime;


	SimTime m_lastThinkTime;
	SimTime m_thinkUpdate;

	bool m_movementAllowed;
	bool m_tacticalMovementAllowed;

	bool m_readyToAttack;
};

#endif
