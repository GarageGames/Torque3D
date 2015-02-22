//-----------------------------------------------------------------------------
// Copyright 2008 (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/BlissGMK/physics/physics.h"
#include "T3D/BlissGMK/physics/rigidBody.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "console/consoleTypes.h"

// BlissGMK - guidebot >>
//#include "T3D/BlissGMK/guideBot/sceneWorldObject.h"
// BlissGMK - guidebot <<

IMPLEMENT_CO_DATABLOCK_V1(RigidBodyData);
IMPLEMENT_CO_NETOBJECT_V1(RigidBody);

extern bool gFreezeSim;

RigidBodyData::RigidBodyData()
{
	mShapeType = PhysInfo::ST_BOX;
	mPos = Point3F::Zero;
	mRotation = EulerF::Zero;
	mOnlyOnClient = false;
	mDampOnCollisionFactorL = 1.f;
	mDampOnCollisionFactorA = 1.f;
}

RigidBodyData::~RigidBodyData()
{
}


//----------------------------------------------------------------------------


bool RigidBodyData::onAdd()
{
	if(!Parent::onAdd())
		return false;

	return true;
}


bool RigidBodyData::preload(bool server, String &errorStr)
{
	if (!Parent::preload(server, errorStr))
		return false;
	return true;
}   


//----------------------------------------------------------------------------
void RigidBodyData::packData(BitStream* stream)
{
	Parent::packData(stream);
	stream->write(mShapeType);
	mathWrite(*stream,mRotation);
	mathWrite(*stream,mPos);
	stream->write(mDampOnCollisionFactorL);
	stream->write(mDampOnCollisionFactorA);
	stream->writeFlag(mOnlyOnClient);	
}   

void RigidBodyData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	stream->read(&mShapeType);
	mathRead(*stream,&mRotation);
	mathRead(*stream,&mPos);
	stream->read(&mDampOnCollisionFactorL);
	stream->read(&mDampOnCollisionFactorA);
	mOnlyOnClient = stream->readFlag();
}   

//----------------------------------------------------------------------------

void RigidBodyData::initPersistFields()
{
	Parent::initPersistFields();
	addField("shapeType",    TypeS8,	Offset(mShapeType,      RigidBodyData));
	addField("rotAngles",    TypePoint3F,	Offset(mRotation,      RigidBodyData));
	addField("offset",    TypePoint3F,	Offset(mPos,      RigidBodyData));
	addField("onlyOnClient",    TypeBool,	Offset(mOnlyOnClient,      RigidBodyData));
	addField("dampOnCollisionL",    TypeF32,	Offset(mDampOnCollisionFactorL,      RigidBodyData));
	addField("dampOnCollisionA",    TypeF32,	Offset(mDampOnCollisionFactorA,      RigidBodyData));
	
}   


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

RigidBody::RigidBody()
{

	mNetFlags.set(Ghostable);


	mDataBlock = 0;
	mTypeMask |= VehicleObjectType;

	mDelta.pos = Point3F(0,0,0);
	mDelta.posVec = Point3F(0,0,0);
	mDelta.dt = 0.f;
	
	mPhysShape = NULL;
	mPhysPosition	= Point3F::Zero;
	mPhysRotation	= QuatF(0,0,0,1);
	mForce			= VectorF::Zero;
	mTorque			= VectorF::Zero;
	mLinVelocity	= VectorF::Zero;
	mAngVelocity	= VectorF::Zero;
	
	m_haveContact = false;
	m_contactPos = VectorF::Zero;
	m_contactVel = 0.f;
	m_contactNormalVelDot = 0.f;

	mPhysPivot.identity();
	mPhysPivotInv.identity();
}   

RigidBody::~RigidBody()
{

}


//----------------------------------------------------------------------------
void RigidBody::createPhysShape()
{
	//Physics* physics = isServerObject() ? gServerPhysics : gClientPhysics;
	Physics* physics = Physics::getPhysics(isServerObject());
	if (physics)
	{
		PhysInfo physDescr;
		//transform into radian
		VectorF angleRadians = mDataBlock->mRotation/180.f*float(M_PI);
		physDescr.transform.set(angleRadians, mDataBlock->mPos);
		physDescr.owner = this;
		physDescr.shapeType = (PhysInfo::ShapeType)mDataBlock->mShapeType;
		physDescr.mass = mDataBlock->mass;
		if (physDescr.shapeType==PhysInfo::ST_SPHERE)
		{
			Box3F scaledObjBox = mObjBox;
			scaledObjBox.minExtents.convolve(mObjScale);
			scaledObjBox.maxExtents.convolve(mObjScale);
			F32 radius = (scaledObjBox.maxExtents - scaledObjBox.getCenter()).len();
			physDescr.params = VectorF(radius,0.f,0.f);
		}
		else //if (physDescr.shapeType==PhysInfo::ST_BOX)
		{
			Box3F rotBox = mObjBox;
			physDescr.transform.mul(rotBox);
			VectorF length = VectorF(rotBox.len_x(),rotBox.len_y(),rotBox.len_z());
			length.convolve(mObjScale);

			physDescr.params = length;
		}
		//physDescr.params = VectorF(1.f,1.f,1.f);
		//physDescr.shapeType = PhysInfo::ST_SPHERE;
		//physDescr.mass = 5.f;
		//physDescr.params = VectorF(0.5f,0.f,0.f);
		mPhysShape = physics->createPhysShape(physDescr);
		mPhysShape->setTransform(mObjToWorld);
		mPhysShape->setForce(mForce);
		mPhysShape->setTorque(mTorque);
		mPhysShape->setLinVelocity(mLinVelocity);
		mPhysShape->setAngVelocity(mAngVelocity);
	}
}

bool RigidBody::onAdd()
{
	if (!Parent::onAdd())
		return false;

	/*
	if (isServerObject() && Physics::getPhysics(false)==NULL)
		mHasServerPhysic = true;
	*/
	if (!mPhysShape)// && (isClientObject() || !mDataBlock->mOnlyOnClient || mHasServerPhysic) )
		createPhysShape();
	
	
	if (mPhysShape)
	{
		// Initialize interpolation vars.      
		mDelta.rot[1] = mDelta.rot[0] = mPhysShape->getRotation();
		mDelta.pos = mPhysShape->getPosition();
		mDelta.posVec = Point3F(0,0,0);
		
		if (isClientObject())
		{
			setTransform(mPhysShape->getTransform());	//for interpolation
		}
	}

	/*if (mDataBlock->mOnlyOnClient && isServerObject())
		mTypeMask &= ~VehicleObjectType;*/

	addToScene();
	
	if (isServerObject())
		scriptOnAdd();

	setEnabled(mEnabled);

	return true;
}

void RigidBody::onRemove()
{
	scriptOnRemove();
	removeFromScene();

	if (mPhysShape)
	{
		mPhysShape->setEnable(false);	//.hack for local connection: it will be destroyed only afrer removing on client
		mPhysShape->getInfo().owner = NULL;
		mPhysShape = NULL;
	}

	Parent::onRemove();
}

/*// BlissGMK - guidebot >>
void RigidBody::createWorldObject()
{
	m_worldObject = (GuideBot::WorldObject*) new SceneWorldObject(this);
	m_worldObject->registerObject();
}
void RigidBody::destroyWorldObject()
{
	m_worldObject->setDestroyed(true);
	SAFE_DELETE(m_worldObject);
}
// BlissGMK - guidebot <<*/

//----------------------------------------------------------------------------

void RigidBody::processTick(const Move* move)
{     
	Parent::processTick(move);

	if (mPhysShape)
	{
		// Save current interpolation
/*
		MatrixF curTr = getRenderTransform();
		mDelta.posVec = curTr.getPosition();
		mDelta.rot[0].set(curTr);
*/
		mDelta.posVec = mPhysPosition;
		mDelta.rot[0] = mPhysRotation;

		mPhysPosition	= mPhysShape->getPosition();
		mPhysRotation	= mPhysShape->getRotation();
		mForce			= mPhysShape->getForce();
		mTorque			= mPhysShape->getTorque();
		mLinVelocity	= mPhysShape->getLinVelocity();
		mAngVelocity	= mPhysShape->getAngVelocity();

		mDelta.pos     = mPhysPosition;
		mDelta.posVec -= mDelta.pos;
		mDelta.rot[1]  = mPhysRotation;

		// Update container database
		//setPosition(mDelta.pos, mDelta.rot[1]);
		MatrixF mat;
		mDelta.rot[1].setMatrix(&mat);
		mat.setColumn(3,mDelta.pos);
		Parent::setTransform(mat);
	}
/*
	Con::printf("ProcessTick s:%d vel: %f %f %f momentum: %f %f %f ",isServerObject(),mLinVelocity.x,mLinVelocity.y,mLinVelocity.z,
		mForce.x, mForce.y, mForce.z);
*/

	setMaskBits(PositionMask);
	updateContainer();
}


void RigidBody::interpolateTick(F32 dt)
{     
	Parent::interpolateTick(dt);
	//setRenderPosition(mDelta.pos, mDelta.rot[1]);

	if(dt == 0.0f)
	{
		setRenderPosition(mDelta.pos, mDelta.rot[1]);
	}
	else
	{
		QuatF rot;
		rot.interpolate(mDelta.rot[1], mDelta.rot[0], dt);
		Point3F pos = mDelta.pos + mDelta.posVec * dt;
		setRenderPosition(pos,rot);	
	}

	

	mDelta.dt = dt;
}


//----------------------------------------------------------------------------

bool RigidBody::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<RigidBodyData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
		return false;

	// Update Rigid Info
	
	scriptOnNewDataBlock();

	return true;
}


void RigidBody::applyImpulse(const Point3F &pos, const Point3F &impulse)
{
	//Con::printf("RigidBody::applyImpulse: %p isServer :%d tick: %d",this,isServerObject(),isServerObject()? gServerProcessList.getTotalTicks():gClientProcessList.getTotalTicks());
	//Con::printf("applyImpulse");
	if(mEnabled && mPhysShape && (isServerObject() || !mHasServerPhysic))
	{
		mPhysShape->addForce(impulse,pos);
	}
}

//----------------------------------------------------------------------------

void RigidBody::setPosition(const Point3F& pos,const QuatF& rot)
{
	MatrixF mat;
	rot.setMatrix(&mat);
	mat.setColumn(3,pos);
	setTransform(mat);
}

void RigidBody::setRenderPosition(const Point3F& pos, const QuatF& rot)
{
	MatrixF mat;
	rot.setMatrix(&mat);
	mat.setColumn(3,pos);
	setRenderTransform(mat);
}

void RigidBody::setTransform(const MatrixF& newMat)
{
	Parent::setTransform(newMat);
	
	// for GMK editor, when physics simulation is frozen
	//if(gFreezeSim)
	{
		if(mPhysShape) 
			mPhysShape->setTransform(newMat);
		
		mDelta.pos = newMat.getPosition();
		mDelta.rot[0] = QuatF(newMat);
		mDelta.rot[1] = mDelta.rot[0];
		mPhysPosition = mDelta.pos;
		mPhysRotation = mDelta.rot[0];
	}
}


void RigidBody::writePacketData(GameConnection *connection, BitStream *stream)
{
	Parent::writePacketData(connection, stream);
	if (stream->writeFlag(mPhysShape))
	{
		mathWrite(*stream, mPhysShape->getPosition());
		mathWrite(*stream, mPhysShape->getRotation());
		mathWrite(*stream, mPhysShape->getLinVelocity());
		mathWrite(*stream, mPhysShape->getAngVelocity());
	}
	
	//stream->setCompressionPoint(mRigid.linPosition);
}

void RigidBody::readPacketData(GameConnection *connection, BitStream *stream)
{
	Parent::readPacketData(connection, stream);
	if (stream->readFlag())
	{
		Point3F pos;
		QuatF q;
		VectorF linVel,angVel;
		mathRead(*stream, &pos);
		mathRead(*stream, &q);
		mathRead(*stream, &linVel);
		mathRead(*stream, &angVel);
	}
	
	///mDisableMove = stream->readFlag();
	//stream->setCompressionPoint(mRigid.linPosition);
}   

//----------------------------------------------------------------------------

U32 RigidBody::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 retMask = Parent::packUpdate(con, mask, stream);
	bool hasServerPhysics = !Physics::getPhysics(false) && con->isLocalConnection();

	//set hasServerPhysic flag
	if ((mask&InitialUpdateMask))
	{
		if (!hasServerPhysics && mDataBlock->mOnlyOnClient)
		{
			setScopeAlways();
		}
	}
	//special cases:
	if (stream->writeFlag((mask&InitialUpdateMask) && (hasServerPhysics || mDataBlock->mOnlyOnClient)))
	{
		if (stream->writeFlag(hasServerPhysics))
		{
			PhysShape* shape = mPhysShape;
			stream->writeBits(8*sizeof(shape),&shape);
		}
		else
		{
			mathWrite(*stream,getTransform());
		}
		
	}

	if (stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
	{
		return retMask;
	}

	if (!hasServerPhysics && !mDataBlock->mOnlyOnClient && stream->writeFlag(mask & PositionMask))
	{
		/*
		VectorF linVel = mPhysShape->getLinVelocity();
		VectorF force = mPhysShape->getForce();
		Con::printf("Pack vel: %f %f %f momentum: %f %f %f ",linVel.x,linVel.y,linVel.z,
			force.x, force.y, force.z);*/

		mPhysShape->pack(stream);
	}

	return retMask;
}   

void RigidBody::unpackUpdate(NetConnection *con, BitStream *stream)
{
	//Con::printf("RigidBody::unpackUpdate: %p isServer :%d tick: %d",this,isServerObject(),isServerObject()? gServerProcessList.getTotalTicks():gClientProcessList.getTotalTicks());
	Parent::unpackUpdate(con,stream);

	// Initial update
	if (stream->readFlag())
	{
		if (stream->readFlag())
		{
			mHasServerPhysic = true;
			PhysShape* serverShape = NULL;
			stream->readBits(8*sizeof(serverShape),&serverShape);
			mPhysShape = serverShape;
		}
		else
		{
			MatrixF tr;
			mathRead(*stream,&tr);
			setTransform(tr);
		}
		mDelta.dt  = 0;
	}

	if (stream->readFlag())
		return;

	if (!mHasServerPhysic && !mDataBlock->mOnlyOnClient && stream->readFlag()) 
	{
		// Read in new position and momentum values
		if (!mPhysShape)
			createPhysShape();
		
		{
			mPhysShape->unpack(stream);

			mPhysPosition	= mPhysShape->getPosition();
			mPhysRotation	= mPhysShape->getRotation();
			mForce			= mPhysShape->getForce();
			mTorque			= mPhysShape->getTorque();
			mLinVelocity	= mPhysShape->getLinVelocity();
			mAngVelocity	= mPhysShape->getAngVelocity();
		}
				
		/*
		Con::printf("Unpack vel: %f %f %f momentum: %f %f %f ",mLinVelocity.x,mLinVelocity.y,mLinVelocity.z,
			mForce.x, mForce.y, mForce.z);*/

		if (mDelta.dt > 0.f) 
		{
			Point3F curPos = mDelta.pos + mDelta.posVec * mDelta.dt;
		
			QuatF curRotate;
			curRotate.interpolate(mDelta.rot[1],mDelta.rot[0],mDelta.dt);
			mDelta.pos = mPhysPosition;
			mDelta.posVec = (curPos - mDelta.pos) / mDelta.dt;
			mDelta.rot[0] = curRotate;
			mDelta.rot[1] = mPhysRotation;
		}
		else
		{
			//Con::printf("Unpack pos dt0:");
			mDelta.pos = mPhysPosition;
			mDelta.posVec.set(0,0,0);
			mDelta.rot[1] = mDelta.rot[0] = mPhysRotation;
		}

	}
}


//----------------------------------------------------------------------------
void RigidBody::setEnabled(const bool enabled)
{
	Parent::setEnabled(enabled);
	if(mPhysShape)
		mPhysShape->setEnable(enabled);

	// When enabling add a tiniest force, to
	// prevent body from flying motionless in the air.
	if(enabled)
	{
		addForce(VectorF(0.0001f, 0.0001f, 0.0001f));
	}
}
//----------------------------------------------------------------------------
void RigidBody::initPersistFields()
{
	Parent::initPersistFields();
}

void RigidBody::reset()
{
	Parent::reset();
}

void RigidBody::setPhysPos(const VectorF& pos)
{
	if (mPhysShape)
	{
		mPhysShape->setPosition(pos);
		mPhysShape->reset();
	}
}
void RigidBody::addForce(const VectorF& pos)
{
	Parent::addForce(pos);
	if (mPhysShape)
	{
		mPhysShape->addForce(pos);
	}
}

void RigidBody::onContact(const VectorF& pos, const VectorF& vel, const VectorF& normal)
{
	Parent::onContact(pos,normal,vel);
	if (mPhysShape)
	{
		mPhysShape->applyDampingOnCollistion(mDataBlock->mDampOnCollisionFactorL,mDataBlock->mDampOnCollisionFactorA);
	}
}


//----------------------------------------------------------------------------
ConsoleMethod(RigidBody, setPhysPos, void, 3, 3, "")
{
	VectorF physPos(0.f,0.f,0.f);
	dSscanf(argv[2], "%g %g %g", &physPos.x, &physPos.y, &physPos.z);
	object->setPhysPos(physPos);
}

/*
#include "T3D/logickingMechanics/physics/bullet/physShapeBullet.h"
ConsoleMethod(RigidBody, damping, void, 2, 2, "")
{
	PhysShape* physShape = object->getPhysShape();
	PhysShapeBullet* physShapeBullet = dynamic_cast<PhysShapeBullet*>(physShape);
	if (physShapeBullet)
	{
		btRigidBody* body = physShapeBullet->getBody();
		body->setDamping(0.9f,0.9f);
	}
}
*/