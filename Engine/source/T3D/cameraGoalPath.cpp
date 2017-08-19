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

#include "cameraGoalPath.h"
#include "scene/pathManager.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "gfx/sim/debugDraw.h"
#include "math/mathUtils.h"

IMPLEMENT_CO_NETOBJECT_V1(CameraGoalPath);

CameraGoalPath::CameraGoalPath()
{
	mNetFlags.clear(Ghostable);
	mTypeMask |= CameraObjectType;

	mPosition.set(0.0f, 0.0f, 0.0f);
	mRot.identity();

	mDelta.time = 0;
	mDelta.timeVec = 0;

	mT = 0;
   mPlayerPathIndex = SimPath::Path::NoPathIndex;
	mCameraPathIndex = SimPath::Path::NoPathIndex;
	mPlayerObject = NULL;
	mLookAtPlayer = false;

	//choose our path manager (server/client)
	if(isServerObject())
		mPathManager = gServerPathManager;
	else
		mPathManager = gClientPathManager;
}

CameraGoalPath::~CameraGoalPath()
{
}

F32 CameraGoalPath::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
	//we need a higher priority because
	//cameraGoalFollower depends on us
	return 5.0f;
}

void CameraGoalPath::processTick(const Move*)
{
	if (!mPathManager->isValidPath(mCameraPathIndex)
		|| !mPathManager->isValidPath(mPlayerPathIndex)
		|| !mPlayerObject)
		return;

	mDelta.timeVec = mT;

	Point3F playerPos = mPlayerObject->getNodePosition("Cam");
	mT = mPathManager->getClosestTimeToPoint(mPlayerPathIndex, playerPos);
	mPathManager->getPathPosition(mCameraPathIndex, mT, mPosition, mRot);

#ifdef ENABLE_DEBUGDRAW
	Point3F debugCameraPos, debugPlayerPos; QuatF dummy;
	mPathManager->getPathPosition(mCameraPathIndex, mT, debugCameraPos, dummy);
	mPathManager->getPathPosition(mPlayerPathIndex, mT, debugPlayerPos, dummy);
	DebugDrawer::get()->drawLine(playerPos, debugPlayerPos, ColorI::BLUE);
	DebugDrawer::get()->setLastTTL(TickMs);
	DebugDrawer::get()->drawLine(debugPlayerPos, debugCameraPos, ColorI::RED);
	DebugDrawer::get()->setLastTTL(TickMs);
#endif

	//look at player
	if(mLookAtPlayer)
	{
		Point3F camToPlayerVec = playerPos - mPosition;
		camToPlayerVec.normalizeSafe();
		F32 camToPlayerYaw, camToPlayerPitch;
		MathUtils::getAnglesFromVector(camToPlayerVec, camToPlayerYaw, camToPlayerPitch);

		//stuff that back into a quat
		MatrixF xRot, zRot;
		xRot.set(EulerF(-camToPlayerPitch, 0.0f, 0.0f));
		zRot.set(EulerF(0.0f, 0.0f, camToPlayerYaw));
		MatrixF temp;
		temp.mul(zRot, xRot);
		mRot.set(temp);
	}

	setPosition(mPosition, mRot);

	// Set frame interpolation
	mDelta.time = mT;
	mDelta.timeVec -= mT;
}

void CameraGoalPath::getCameraTransform(F32* pos,MatrixF* mat)
{
	getRenderEyeTransform(mat);
}

void CameraGoalPath::interpolateTick(F32 dt)
{
	if (!mPathManager->isValidPath(mCameraPathIndex)
		|| !mPathManager->isValidPath(mPlayerPathIndex)
		|| !mPlayerObject)
		return;

	Parent::interpolateTick(dt);
	MatrixF mat;
	interpolateMat(mDelta.time + (mDelta.timeVec * dt),&mat);
	Parent::setRenderTransform(mat);
}

void CameraGoalPath::setPosition(const Point3F& pos, const QuatF& rot)
{
	MatrixF mat;
	rot.setMatrix(&mat);
	mat.setColumn(3,pos);
	Parent::setTransform(mat);
}

void CameraGoalPath::setRenderPosition(const Point3F& pos, const QuatF& rot)
{
	MatrixF mat;
	rot.setMatrix(&mat);
	mat.setColumn(3,pos);
	Parent::setRenderTransform(mat);
}

void CameraGoalPath::interpolateMat(F64 t, MatrixF* mat)
{
	Point3F pos; QuatF rot;
	gClientPathManager->getPathPosition(mCameraPathIndex, t, pos, rot);
	if(mLookAtPlayer)
		mRot.setMatrix(mat);
	else
		rot.setMatrix(mat);
	mat->setPosition(pos);
}

void CameraGoalPath::onDeleteNotify(SimObject *obj)
{
	Parent::onDeleteNotify(obj);

	if (obj == (SimObject*)mPlayerObject)
	{
		mPlayerObject = NULL;
	}
}

U32 CameraGoalPath::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 retMask = Parent::packUpdate(con, mask, stream);

	if (stream->writeFlag(mask & ModeMask))
	{
		stream->write(mLookAtPlayer);
	}

	if(stream->writeFlag((mask & TMask)))
	{
		stream->write(mT);
	}

	if(stream->writeFlag((mask & PlayerPathMask)))
	{
		stream->write(mPlayerPathIndex);
	}

	if(stream->writeFlag((mask & CameraPathMask)))
	{
		stream->write(mCameraPathIndex);
	}

	if(stream->writeFlag((mask & PlayerMask) && mPlayerObject))
	{
		S32 id = con->getGhostIndex(mPlayerObject);
		stream->write(id);

		//hack to keep sending updates until player is ghosted
		if(id == -1)
			setMaskBits(PlayerMask);
	}

	return retMask;
}

void CameraGoalPath::unpackUpdate(NetConnection *con, BitStream *stream)
{
	Parent::unpackUpdate(con,stream);

	//ModeMask
	if (stream->readFlag())
	{
		stream->read(&mLookAtPlayer);
	}

	//TMask
	if (stream->readFlag())
	{
		stream->read(&mT);
		mDelta.time = mT;
		mDelta.timeVec = 0;
	}

	//PlayerPathMask
	if (stream->readFlag())
	{
		stream->read(&mPlayerPathIndex);
	}

	//CameraPathMask
	if (stream->readFlag())
	{
		stream->read(&mCameraPathIndex);
	}

	//PlayerMask
	if (stream->readFlag())
	{
		S32 id;
		stream->read(&id);
		if(id > 0)
		{
			Player* playerObject = static_cast<Player*>(con->resolveGhost(id));
			setPlayerObject(playerObject);
		}
	}
}

//===============================================================================

bool CameraGoalPath::setPlayerPathObject(SimPath::Path *obj)
{
	if(!obj)
		return false;

	mPlayerPathIndex = obj->getPathIndex();
	setMaskBits(PlayerPathMask);
	return true;
}

ConsoleMethod( CameraGoalPath, setPlayerPathObject, bool, 3, 3, "(Path object)") {   
	SimPath::Path *p;
	if(!Sim::findObject(argv[2], p))
	{
		Con::errorf("CameraGoalPath::setPlayerPathObject - failed to find object '%s'", argv[2]);
		return false;
	}

	return object->setPlayerPathObject(p);
}

//===============================================================================

bool CameraGoalPath::setCameraPathObject(SimPath::Path *obj)
{
 	if(!obj)
		return false;

	mCameraPathIndex = obj->getPathIndex();
	setMaskBits(CameraPathMask);
	return true;
}

ConsoleMethod( CameraGoalPath, setCameraPathObject, bool, 3, 3, "(Path object)") {   
	SimPath::Path *p;
	if(!Sim::findObject(argv[2], p))
	{
		Con::errorf("CameraGoalPath::setCameraPathObject - failed to find object '%s'", argv[2]);
		return false;
	}

	return object->setCameraPathObject(p);
}

//===============================================================================

bool CameraGoalPath::setPlayerObject(Player *obj)
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
	setMaskBits(PlayerMask);

	if(bool(mPlayerObject))
	{
		processAfter(mPlayerObject);
		deleteNotify(mPlayerObject);
	}
	return true;
}

ConsoleMethod( CameraGoalPath, setPlayerObject, bool, 3, 3, "(Player object)") {   
	Player *gb;
	if(!Sim::findObject(argv[2], gb))
	{
		Con::errorf("CameraGoalPath::setPlayerObject - failed to find object '%s'", argv[2]);
		return false;
	}

	return object->setPlayerObject(gb);
}

//===============================================================================

void CameraGoalPath::setLookAtPlayer(bool on)
{
	mLookAtPlayer = on;
	setMaskBits(ModeMask);
}

ConsoleMethod( CameraGoalPath, setLookAtPlayer, void, 3, 3, "(bool)")
{
	object->setLookAtPlayer(dAtob(argv[2]));
}