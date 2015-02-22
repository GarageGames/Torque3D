//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/aiBot.h"

IMPLEMENT_CO_NETOBJECT_V1(AIBot);

#define NO_ENEMY U32(0)

//////////////////////////////////////////////////////////////////////////
AIBot::AIBot()
{
	mNetFlags |= ScopeAlways;
	m_enemyId = NO_ENEMY;
	m_enemy = NULL;

	m_thinkUpdate = 100;
	

	m_attackFov = mDegToRad(60.f);
	m_attackDist = 4;
	
	m_isChasing = false;
	m_chaseFarDist = 6;
	m_chaseCloseDist = 2;

	m_isFleeing = false;
	m_fleeFarDist = 5;
	m_fleeCloseDist = 1;

	m_movementAllowed = true;
	m_tacticalMovementAllowed = true;
	m_readyToAttack = false;

	m_isStrafing = false;
	m_strafeMinDist = 4;
	m_strafeMaxDist = 10;
	m_strafeChangeDirTime = 1000;
	m_lastStrafeTime = 0;
	m_strafeOffset = Point3F(0, 0, 0);
}

AIBot::~AIBot()
{
}

//////////////////////////////////////////////////////////////////////////
void AIBot::initPersistFields()
{
	Parent::initPersistFields();
	Con::setIntVariable("$TypeMasks::AIObjectType",	AIObjectType);
}


//////////////////////////////////////////////////////////////////////////
bool AIBot::onAdd()
{
	if(!Parent::onAdd()) return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void AIBot::onRemove()
{
	Parent::onRemove();
}

//////////////////////////////////////////////////////////////////////////
void AIBot::processTick(const Move *move)
{
	Parent::processTick(move);
	if(isServerObject() && !isDead())
	{
		SimTime dt = Sim::getCurrentTime() - m_lastThinkTime;
		if(dt > m_thinkUpdate)
		{
			think(dt);
			m_lastThinkTime = Sim::getCurrentTime();
		}
	}
}

bool AIBot::getAIMove(Move *movePtr)
{
	*movePtr = NullMove;
	if(!isDead() && m_movementAllowed)
	{
		Parent::getAIMove(movePtr);
	}
	return true;
}

void AIBot::think(SimTime dt)
{
	updateEnemy();
	if(getEnemy())
		updateTacticalMovement();
}

bool AIBot::isDead()
{
	if (mDamageState != Enabled) 
		return true;
	return false;
}
   

// Chasing selected enemy
void AIBot::updateTacticalMovement()
{
	if(!m_tacticalMovementAllowed) return;

	if(m_isStrafing)
	{
		if(Sim::getCurrentTime() - m_lastStrafeTime > m_strafeChangeDirTime)
		{
			m_lastStrafeTime = Sim::getCurrentTime();
			VectorF left;
			getTransform().getColumn(0, &left);
			VectorF strafeDir = left * 
				(mRandI(0, 1) ? 1.f : -1.f) * 
				(m_strafeMinDist + mRandF(0.f, m_strafeMaxDist - m_strafeMinDist));
			m_strafeOffset = strafeDir;
		}
	}
	else
		m_strafeOffset = Point3F(0,0,0);

	bool needToStop = false;
	bool needToMove = false;
	VectorF moveDest;

	if(m_isChasing)
	{
		// approaching the enemy
		if(m_distToEnemy > m_chaseCloseDist)
		{
			if(m_distToEnemy > m_chaseFarDist)
			{
				m_tacticalMovement = ReduceDist;
				//Con::printf(" chase: %f, ReduceDist", m_distToEnemy);
			}

			if(m_tacticalMovement == ReduceDist)
			{
				moveDest = getEnemy()->getPosition() + m_strafeOffset;
				needToMove = true;
				//Con::printf(" chase: %f ", m_distToEnemy);
			}
		}
		else if(m_distToEnemy < m_chaseCloseDist)
		{
			if(m_tacticalMovement == ReduceDist)
			{
				m_tacticalMovement = HoldDist;
				needToStop = true;
			}
		}
	}

	if(m_isFleeing)
	{
		if(m_distToEnemy < m_fleeFarDist)
		{
			if(m_distToEnemy < m_fleeCloseDist)
			{
				m_tacticalMovement = IncreaseDist;
				//Con::printf(" flee: %f, IncreaseDist", m_distToEnemy);
				//Con::printf("chase stop ");
			}

			if(m_tacticalMovement == IncreaseDist)
			{
				VectorF fleeDir = getPosition() - getEnemy()->getPosition();
				F32 fleeDist = (m_fleeFarDist - m_distToEnemy) * 2.f;
				fleeDir.normalize(fleeDist);
				moveDest = getPosition() + fleeDir + m_strafeOffset;
				needToMove = true;
				//Con::printf(" flee: %f ", m_distToEnemy);
			}
		}
		else if (m_distToEnemy > m_fleeFarDist)
		{
			if(m_tacticalMovement == IncreaseDist)
			{
				m_tacticalMovement = HoldDist;
				needToStop = true;
				//Con::printf("flee stop ");
			}
		}
	}


	if(m_isStrafing && !needToMove && m_tacticalMovement == HoldDist)
	{
		needToMove = true;
		moveDest = getPosition() + m_strafeOffset;
	}

	if(needToMove)
		setMoveDestination(moveDest, true);
	else if(needToStop)
		stopMove();
}

void AIBot::enableChase(bool isChasing, F32 closeChaseDist, F32 farChaseDist)
{
	m_isChasing = isChasing;
	m_chaseFarDist = farChaseDist;
	m_chaseCloseDist = closeChaseDist;
}

void AIBot::enableFlee(bool isFleeing, F32 closeFleeDist, F32 farFleeDist)
{
	m_isFleeing = isFleeing;
	m_fleeFarDist = farFleeDist;
	m_fleeCloseDist = closeFleeDist;
}

void AIBot::enableStrafe(bool isStrafing, F32 minStrafeDist, F32 maxStrafeDist, SimTime changeDirTime)
{
	m_isStrafing = isStrafing;
	m_strafeMinDist = minStrafeDist;
	m_strafeMaxDist = maxStrafeDist;
	m_strafeChangeDirTime = changeDirTime;

	m_strafeOffset = Point3F(0, 0, 0);
}

void AIBot::allowMovement(bool isAllowed)
{
	m_movementAllowed = isAllowed;
}

void AIBot::allowTacticalMovement(bool isAllowed)
{
	m_tacticalMovementAllowed = isAllowed;
}


void AIBot::applyImpulse(const Point3F& pos,const VectorF& vec)
{
	//Parent::applyImpulse(pos, vec);
	return;
}

//////////////////////////////////////////////////////////////////////////
GameBase* AIBot::getEnemy() 
{ 
	return m_enemy;
}

void AIBot::setEnemy(U32 enemy)
{ 
	// Can't set ourself as enemy
	if(enemy == getId()) enemy = NO_ENEMY;
	
	U32 oldEnemyId = m_enemyId;
	m_enemyId = enemy;

	if(m_enemyId != NO_ENEMY)
		updateEnemy();
	else
		m_enemy = NULL;
	
	if(m_enemy)
	{
		if(m_enemyId != oldEnemyId)
		{
			throwCallback("onNewEnemy");
			m_readyToAttack = false;
		}
	}
	else
	{
		if(oldEnemyId != NO_ENEMY)
		{
			throwCallback("onNoEnemy");
			m_readyToAttack = false;
		}
	}
}


U32 AIBot::getEnemyId() 
{ 
	return m_enemyId;
}

F32 AIBot::getDistToEnemy()
{
	return m_distToEnemy;
}

bool AIBot::updateEnemy()
{
	if(m_enemyId == NO_ENEMY) return false;

	Sim::findObject(m_enemyId, m_enemy);

	Player* playerEnemy = dynamic_cast<Player*>(m_enemy);
	if(!playerEnemy || playerEnemy->getDamageState() == Disabled)
	{
		setEnemy(NO_ENEMY);
		return false;
	}

	VectorF dir = m_enemy->getPosition() - this->getPosition();
	m_distToEnemy = dir.len();

	if(m_distToEnemy < m_attackDist)
	{
		dir.normalize();
		Point3F lookFwd;
		getTransform().getColumn(1, &lookFwd);

		F32 dot = mDot(dir, lookFwd);
		dot = mClampF(dot, -1, 1);
		F32 objFov = mAcos(dot);
		if(objFov < m_attackFov)
		{
			if(!m_readyToAttack)
			{
				throwCallback("onReadyToAttack");
				m_readyToAttack = true;
			}
			return true;
		}
	}

	if(m_readyToAttack)
	{
		m_readyToAttack = false;
		throwCallback("onNotReadyToAttack");
		return false;
	}
	return true;
}

void AIBot::wallAvoindance(const VectorF& collidedWallNormal)
{
	if(mFabs(collidedWallNormal.z) > 0.4f) return;

	VectorF dir = mVelocity;
	VectorF wallNormal = collidedWallNormal;
	wallNormal.z = 0.f;
	wallNormal.normalizeSafe();
	dir.z = 0.f;
	dir.normalizeSafe();
	if (mDot(dir, wallNormal) < 0.f)
	{
		VectorF vec;
		mCross(dir, wallNormal, &vec);
		mCross(vec, wallNormal, &vec);
		vec.z = 0.f;
		vec.normalizeSafe();
		dir = vec;
		dir.z = 0;
		dir.normalizeSafe();
		dir *= getMaxForwardVelocity();
		mVelocity = -dir;
	}
}

void AIBot::attackParams(F32 dist, F32 fov)
{
	m_attackDist = dist;
	m_attackFov = mDegToRad(fov);
}

// --------------------------------------------------------------------------------------------
// Console Functions
// --------------------------------------------------------------------------------------------
ConsoleMethod( AIBot, getEnemy, S32, 2, 2, "()")
{
	return object->getEnemyId();
}

ConsoleMethod( AIBot, setEnemy, void, 3, 3, "(GameBase obj)")
{
	// Find the target
	GameBase *targetObject;
	if( Sim::findObject( argv[2], targetObject ) )
	{
		object->setEnemy(targetObject->getId());
	}
}

ConsoleMethod( AIBot, hasEnemy, bool, 2, 2, "()")
{
	return object->getEnemy() ? true : false;
}

ConsoleMethod( AIBot, distToEnemy, F32, 2, 2, "()")
{
	return object->getDistToEnemy();
}

ConsoleMethod( AIBot, enableChase, void, 5, 5, "()")
{
	return object->enableChase(dAtob(argv[2]), dAtof(argv[3]), dAtof(argv[4]));
}

ConsoleMethod( AIBot, enableFlee, void, 5, 5, "()")
{
	return object->enableFlee(dAtob(argv[2]), dAtof(argv[3]), dAtof(argv[4]));
}

ConsoleMethod( AIBot, enableStrafe, void, 6, 6, "()")
{
	return object->enableStrafe(dAtob(argv[2]), dAtof(argv[3]), dAtof(argv[4]), dAtoi(argv[5]));
}


ConsoleMethod( AIBot, attackParams, void, 4, 4, "()")
{
	return object->attackParams(dAtof(argv[2]), dAtof(argv[3]));
}


ConsoleMethod( AIBot, allowMovement, void, 3, 3, "()")
{
	return object->allowMovement(dAtob(argv[2]));
}

ConsoleMethod( AIBot, allowTacticalMovement, void, 3, 3, "()")
{
	return object->allowTacticalMovement(dAtob(argv[2]));
}


ConsoleMethod( AIBot, isDead, bool, 2, 2, "()")
{
	return object->isDead();
}