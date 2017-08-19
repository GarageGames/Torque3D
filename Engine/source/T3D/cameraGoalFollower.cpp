//-----------------------------------------------------------------------------
// Copyright (C) 2008-2013 Ubiq Visuals, Inc. (http://www.ubiqvisuals.com/)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "app/game.h"
#include "math/mMath.h"
#include "math/mathUtils.h"
#include "core/stream/bitStream.h"
#include "cameraGoalFollower.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "gui/worldEditor/editor.h"
#include "console/consoleTypes.h"
#include "T3D/fx/cameraFXMgr.h"

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(CameraGoalFollowerData);

CameraGoalFollowerData::CameraGoalFollowerData()
{
	rotationHistorySize = 16;
	positionHistorySize = 16;
}

CameraGoalFollowerData::~CameraGoalFollowerData()
{

}

void CameraGoalFollowerData::initPersistFields()
{
	Parent::initPersistFields();

	addField("rotationHistorySize", TypeS32, Offset(rotationHistorySize, CameraGoalFollowerData));
	addField("positionHistorySize", TypeS32, Offset(positionHistorySize, CameraGoalFollowerData));
}

void CameraGoalFollowerData::packData(BitStream* stream)
{
	Parent::packData(stream);

	stream->write(rotationHistorySize);
	stream->write(positionHistorySize);
}

void CameraGoalFollowerData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);

	stream->read(&rotationHistorySize);
	stream->read(&positionHistorySize);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(CameraGoalFollower);

CameraGoalFollower::CameraGoalFollower()
{
	mNetFlags.clear(Ghostable);
	mTypeMask |= CameraObjectType;

	mDataBlock = 0;

	mPosition.set(0.0f, 0.0f, 0.0f);
	mRot.set(0.0f, 0.0f, 0.0f);

	delta.pos = Point3F(0.0f, 0.0f, 0.0f);
	delta.rot = Point3F(0.0f, 0.0f, 0.0f);
	delta.posVec = delta.rotVec = VectorF(0.0f, 0.0f, 0.0f);

	mObjToWorld.setColumn(3, mPosition);

	mPlayerObject = NULL;

	mGoalObject = NULL;
	mGoalObjectPrev = NULL;
	mInterpolationT = 0.0f;
	mInterpolationTime = 0.0f;
}

CameraGoalFollower::~CameraGoalFollower()
{

}


//----------------------------------------------------------------------------

bool CameraGoalFollower::onAdd()
{
	if(!Parent::onAdd())
		return false;

	mObjBox.maxExtents = mObjScale;
	mObjBox.minExtents = mObjScale;
	mObjBox.minExtents.neg();
	resetWorldBox();

	addToScene();

	return true;
}

void CameraGoalFollower::onEditorEnable()
{
	mNetFlags.set(Ghostable);
}

void CameraGoalFollower::onEditorDisable()
{
	mNetFlags.clear(Ghostable);
}

void CameraGoalFollower::onRemove()
{
	removeFromScene();
	Parent::onRemove();
}

bool CameraGoalFollower::onNewDataBlock( GameBaseData *dptr, bool reload )
{
	mDataBlock = dynamic_cast<CameraGoalFollowerData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
		return false;

	scriptOnNewDataBlock();
	return true;
}

void CameraGoalFollower::getCameraTransform(F32* pos, MatrixF* mat)
{
	getRenderEyeTransform(mat);
}

F32 CameraGoalFollower::getCameraFov()
{
   ShapeBase * obj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mGoalObject));
   if(obj && static_cast<ShapeBaseData*>(obj->getDataBlock())->observeThroughObject)
      return(obj->getCameraFov());
   else
      return(Parent::getCameraFov());
}

void CameraGoalFollower::processTick(const Move* move)
{
	Parent::processTick(move);

	// Update delta
	delta.rotVec = mRot;
	delta.posVec = mPosition;

	//check if we have a goal object
	ShapeBase* goalObj = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mGoalObject));
	if (goalObj)
	{
		F32 pos; MatrixF goalTrans;
		goalObj->getCameraTransform(&pos, &goalTrans);

		Point3F goalPos;
		goalTrans.getColumn(3, &goalPos);

		VectorF goalForward;
		goalTrans.getColumn(1, &goalForward);

		//if we have a goalObjPrev and an interpolation time, perform the interpolation
		ShapeBase* goalObjPrev = dynamic_cast<ShapeBase*>(static_cast<SimObject*>(mGoalObjectPrev));
		if(goalObjPrev && mInterpolationTime > 0)
		{
			F32 posPrev; MatrixF goalTransPrev;
			goalObjPrev->getCameraTransform(&posPrev, &goalTransPrev);

			Point3F goalPosPrev;
			goalTransPrev.getColumn(3, &goalPosPrev);

			VectorF goalForwardPrev;
			goalTransPrev.getColumn(1, &goalForwardPrev);

			//interpolate between goalObjPrev and goalObj
			mInterpolationT += (F32)TickMs / (F32)mInterpolationTime;
			mInterpolationT = mClampF(mInterpolationT, 0, 1);
			goalPos.interpolate(goalPosPrev, goalPos, mInterpolationT);

			//override rotation during interpolation
			if(!mPlayerObject)
			{
				//no player object, just interpolate between goalForwardPrev & goalForward
				//this may result in the player going offscreen temporarily
				goalForward.interpolate(goalForwardPrev, goalForward, mInterpolationT);
			}
			else
			{
				//it's assumed that both goals provide a nice view of the player
				//but sometimes linear interpolation between them like above
				//causes the player to go off-screen. So (rotationally) we're
				//going to interpolate to a view of the player, and then to the goal

				Point3F playerPos = mPlayerObject->getNodePosition("Cam");
				VectorF vec = playerPos - mPosition; vec.normalizeSafe();
				MatrixF lookAt = MathUtils::createOrientFromDir(vec);

				QuatF q1(goalTransPrev);
				QuatF q2(lookAt);
				QuatF q3(goalTrans);
				QuatF final;

				//first half of the transition
				if(mInterpolationT < 0.5f)
				{
					//interpolate between prev goal and lookAt
					F32 t = mInterpolationT * 2.0f;
					final.interpolate(q1, q2, t);
				}

				//second half of the transition
				else
				{
					//interpolate between lookat and new goal
					F32 t =  (mInterpolationT - 0.5f) * 2.0f;
					final.interpolate(q2, q3, t);
				}

				MatrixF tmp;
				final.setMatrix(&tmp);
				tmp.getColumn(1, &goalForward);
			}

			//have we finished this interpolation?
			if(mInterpolationT == 1)
				mGoalObjectPrev = NULL;
		}

		//-----------------------------------------------------------
		// Position
		//-----------------------------------------------------------
		//add current goalPos to the history
		mGoalPosHistory.push_back(goalPos);

		//maintain history size
		if(mGoalPosHistory.size() > mDataBlock->positionHistorySize)
			mGoalPosHistory.pop_front();

		//determine current position by averaging goal history
		mPosition.zero();
		for( U32 i = 0; i < mGoalPosHistory.size(); i++ )
		{
			mPosition += mGoalPosHistory[i];
		}
		mPosition /= mGoalPosHistory.size();

		//-----------------------------------------------------------
		// Rotation
		//-----------------------------------------------------------
		//add current goalForward to the history
		mGoalRotHistory.push_back(goalForward);

		//maintain history size
		if(mGoalRotHistory.size() > mDataBlock->rotationHistorySize)
			mGoalRotHistory.pop_front();

		//determine current rotation by averaging goal history
		goalForward.zero();
		for( U32 i = 0; i < mGoalRotHistory.size(); i++ )
		{
			goalForward += mGoalRotHistory[i];
		}
		goalForward /= mGoalRotHistory.size();

		//look in goalForward direction
		F32 yaw, pitch;
		MathUtils::getAnglesFromVector( goalForward, yaw, pitch );

		//there is a *tiny* glitch at 180 yaw if we just use yaw and pitch
		//so we do a little pre-processing and everything is perfect
		F32 yawDiff = yaw - mRot.z;
		F32 pitchDiff = -pitch - mRot.x;

		while( yawDiff > M_PI_F )
			yawDiff -= M_2PI_F;
		while( yawDiff < -M_PI_F )
			yawDiff += M_2PI_F;

		while( pitchDiff > M_PI_F )
			pitchDiff -= M_2PI_F;
		while( pitchDiff < -M_PI_F )
			pitchDiff += M_2PI_F;

		mRot.x += pitchDiff;
		mRot.z += yawDiff;
	}

	// If on the client, calc delta for interpolation
	if (isClientObject()) 
	{
		delta.pos = mPosition;
		delta.rot = mRot;
		delta.posVec = delta.posVec - delta.pos;
		delta.rotVec = delta.rotVec - delta.rot;
	}

	setPosition(mPosition, mRot);
}

void CameraGoalFollower::onDeleteNotify(SimObject *obj)
{
	Parent::onDeleteNotify(obj);

	if (obj == (SimObject*)mPlayerObject)
		mPlayerObject = NULL;

	if (obj == (SimObject*)mGoalObject)
		mGoalObject = NULL;

	if (obj == (SimObject*)mGoalObjectPrev)
		mGoalObjectPrev = NULL;
}

void CameraGoalFollower::interpolateTick(F32 dt)
{
	Parent::interpolateTick(dt);
	Point3F rot = delta.rot + delta.rotVec * dt;
	Point3F pos = delta.pos + delta.posVec * dt;
	setRenderPosition(pos,rot);

	//apply camera effects
	MatrixF curTrans = getRenderTransform();
	curTrans.mul( gCamFXMgr.getTrans() );
	Parent::setRenderTransform( curTrans );
}

void CameraGoalFollower::setPosition(const Point3F& pos, const Point3F& rot)
{
	MatrixF xRot, zRot;
	xRot.set(EulerF(rot.x, 0.0f, 0.0f));
	zRot.set(EulerF(0.0f, 0.0f, rot.z));

	MatrixF temp;
	temp.mul(zRot, xRot);
	temp.setColumn(3, pos);
	Parent::setTransform(temp);
	mRot = rot;
}

void CameraGoalFollower::setRenderPosition(const Point3F& pos,const Point3F& rot)
{
	MatrixF xRot, zRot;
	xRot.set(EulerF(rot.x, 0, 0));
	zRot.set(EulerF(0, 0, rot.z));
	MatrixF temp;
	temp.mul(zRot, xRot);
	temp.setColumn(3, pos);
	Parent::setRenderTransform(temp);
}

//----------------------------------------------------------------------------

U32 CameraGoalFollower::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
	U32 retMask = Parent::packUpdate(conn, mask, stream);

	if(!mGoalObject || !mPlayerObject)
	{
		stream->writeFlag(false);
		stream->writeFlag(false);
		return retMask;
	}

	S32 goalObjectId = conn->getGhostIndex(mGoalObject);
	S32 playerObjectId = conn->getGhostIndex(mPlayerObject);
	
	if(goalObjectId < 0 || playerObjectId < 0)
	{
		stream->writeFlag(false);
		stream->writeFlag(false);
		setMaskBits(UpdateMask);	//keep running packUpdate until the goal & player are ghosted
		if(mask & ClearMask) setMaskBits(ClearMask);
		return retMask;
	}

	if(stream->writeFlag(mask & UpdateMask))
	{
		stream->write(mInterpolationTime);
		stream->writeRangedU32(U32(goalObjectId), 0, NetConnection::MaxGhostCount);
		stream->writeRangedU32(U32(playerObjectId), 0, NetConnection::MaxGhostCount);
	}

	if(stream->writeFlag(mask & ClearMask))
	{		
		//clear history
		mGoalPosHistory.clear();
		mGoalRotHistory.clear();
	}

	return retMask;
}

void CameraGoalFollower::unpackUpdate(NetConnection *conn, BitStream *stream)
{
	Parent::unpackUpdate(conn, stream);

	if( stream->readFlag() )
	{
		S32 ms;
		stream->read(&ms);

		//goal object
		S32 goalObjectId = stream->readRangedU32(0, NetConnection::MaxGhostCount);
		if(goalObjectId >= 0)
		{
			ShapeBase* goalObject = static_cast<ShapeBase*>(conn->resolveGhost(goalObjectId));
			setGoalObject(goalObject, ms);
		}

		//player object
		S32 playerObjectId = stream->readRangedU32(0, NetConnection::MaxGhostCount);
		if(playerObjectId >= 0)
		{
			Player* playerObject = static_cast<Player*>(conn->resolveGhost(playerObjectId));
			setPlayerObject(playerObject);
		}
	}

	if( stream->readFlag() )
	{
		//clear history
		mGoalPosHistory.clear();
		mGoalRotHistory.clear();
	}
}

void CameraGoalFollower::setPosition(const Point3F& pos, const Point3F& rot, MatrixF *mat)
{
	MatrixF xRot, zRot;
	xRot.set(EulerF(rot.x, 0.0f, 0.0f));
	zRot.set(EulerF(0.0f, 0.0f, rot.z));
	mat->mul(zRot, xRot);
	mat->setColumn(3,pos);
	mRot = rot;
}

void CameraGoalFollower::setTransform(const MatrixF& mat)
{
	// This method should never be called on the client.
	if(!isServerObject())
		return;

	// This currently converts all rotation in the mat into
	// rotations around the z and x axis.
	Point3F pos,vec;
	mat.getColumn(1, &vec);
	mat.getColumn(3, &pos);
	Point3F rot(-mAtan2(vec.z, mSqrt(vec.x * vec.x + vec.y * vec.y)), 0.0f, -mAtan2(-vec.x, vec.y));
	setPosition(pos,rot);

	//clear history (so we'll move instantaneously)
	setMaskBits(ClearMask);
}

void CameraGoalFollower::setRenderTransform(const MatrixF& mat)
{
	// This method should never be called on the client.
	if(!isServerObject())
		return;

	// This currently converts all rotation in the mat into
	// rotations around the z and x axis.
	Point3F pos,vec;
	mat.getColumn(1,&vec);
	mat.getColumn(3,&pos);
	Point3F rot(-mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)),0,-mAtan2(-vec.x,vec.y));
	setRenderPosition(pos,rot);
}

//.............................................................

bool CameraGoalFollower::setGoalObject(ShapeBase *obj, S32 ms)
{
	if(!obj)
		return false;

	// reset current object if not null
	if(bool(mGoalObject))
		clearNotify(mGoalObject);

	if(bool(mGoalObjectPrev))
		clearNotify(mGoalObjectPrev);

	//if going to a different goal, let's interpolate
	if(obj != mGoalObject)
		mGoalObjectPrev = mGoalObject;

	mGoalObject = obj;
	mInterpolationTime = ms;
	mInterpolationT = 0;

	if(bool(mGoalObject))
		deleteNotify(mGoalObject);

	if(bool(mGoalObjectPrev))
		deleteNotify(mGoalObjectPrev);

	setMaskBits(UpdateMask);

	return true;
}

ConsoleMethod( CameraGoalFollower, setGoalObject, bool, 3, 4, "(ShapeBase object)") {   
	ShapeBase *gb;
	if(!Sim::findObject(argv[2], gb))
		return false;

	if(argc == 4)
		object->setGoalObject(gb, dAtoi(argv[3]));
	else
		object->setGoalObject(gb);

	return true;
}

ConsoleMethod( CameraGoalFollower, getGoalObject, S32, 2, 2, "")
{
   if(!object->getGoalObject())
      return 0;

	return object->getGoalObject()->getId();
}

//.............................................................

bool CameraGoalFollower::setPlayerObject(Player *obj)
{
	if(!obj)
		return false;

	// reset current object if not null
	if(bool(mPlayerObject))
	{
		clearProcessAfter();
		clearNotify(mPlayerObject);
	}

	mPlayerObject = obj;

	if(bool(mPlayerObject))
	{
		processAfter(mPlayerObject);
		deleteNotify(mPlayerObject);
	}

	setMaskBits(UpdateMask);

	return true;
}

ConsoleMethod( CameraGoalFollower, setPlayerObject, bool, 3, 3, "(Player object)") {   
	Player *gb;
	if(!Sim::findObject(argv[2], gb))
	{
		Con::errorf("CameraGoalFollower::setPlayerObject - failed to find object '%s'", argv[2]);
		return false;
	}

	return object->setPlayerObject(gb);
}