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
#include "cameraGoalPlayer.h"
#include "T3D/gameBase/gameConnection.h"
#include "math/mathIO.h"
#include "gui/worldEditor/editor.h"
#include "console/consoleTypes.h"
#include "T3D/player.h"
#include "gfx/sim/debugDraw.h"
//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(CameraGoalPlayerData);

CameraGoalPlayerData::CameraGoalPlayerData()
{
	manualTransitionTime = 250;
	radiusDefault = 6.0f;
	radiusManual = 4.0f;
	offCenterY = 1.0f;
	offCenterX = 1.0f;
	pitchMax = 1.5f;		// +/- about 86 degrees
	pitchMult = 0.98f;
	pitchDetectRadius = 2.0f;
}

CameraGoalPlayerData::~CameraGoalPlayerData()
{

}

void CameraGoalPlayerData::initPersistFields()
{
	Parent::initPersistFields();

	addField("manualTransitionTime", TypeF32, Offset(manualTransitionTime, CameraGoalPlayerData));
	addField("radiusDefault", TypeF32, Offset(radiusDefault, CameraGoalPlayerData));
	addField("radiusManual", TypeF32, Offset(radiusManual, CameraGoalPlayerData));
	addField("offCenterY", TypeF32, Offset(offCenterY, CameraGoalPlayerData));
	addField("offCenterX", TypeF32, Offset(offCenterX, CameraGoalPlayerData));
	addField("pitchMax", TypeF32, Offset(pitchMax, CameraGoalPlayerData));
	addField("pitchMult", TypeF32, Offset(pitchMult, CameraGoalPlayerData));
	addField("pitchDetectRadius", TypeF32, Offset(pitchDetectRadius, CameraGoalPlayerData));
}

void CameraGoalPlayerData::packData(BitStream* stream)
{
	Parent::packData(stream);

	stream->write(manualTransitionTime);
	stream->write(radiusDefault);
	stream->write(radiusManual);
	stream->write(offCenterY);
	stream->write(offCenterX);
	stream->write(pitchMax);
	stream->write(pitchMult);
	stream->write(pitchDetectRadius);
}

void CameraGoalPlayerData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);

	stream->read(&manualTransitionTime);
	stream->read(&radiusDefault);
	stream->read(&radiusManual);
	stream->read(&offCenterY);
	stream->read(&offCenterX);
	stream->read(&pitchMax);
	stream->read(&pitchMult);
	stream->read(&pitchDetectRadius);
}


//----------------------------------------------------------------------------

//for resolving occlusions, how many "slices" to divide a circle into (even only)
static const U16 sCastsPerCircle = 360;

//derived from mCastsPerCircle, how many radians each slice is
static const F32 sAngleStep = M_2PI_F / sCastsPerCircle;

IMPLEMENT_CO_NETOBJECT_V1(CameraGoalPlayer);

CameraGoalPlayer::CameraGoalPlayer()
{
	mNetFlags.clear(Ghostable);
	mTypeMask |= CameraObjectType;

	mDataBlock = 0;

	delta.pos = Point3F(0.0f, 0.0f, 0.0f);
	delta.rot = Point3F(0.0f, 0.0f, 0.0f);
	delta.posVec = delta.rotVec = VectorF(0.0f, 0.0f, 0.0f);

	mPosition.zero();
	mRot.zero();
	mObjToWorld.setColumn(3, mPosition);

	mFirstTickWithPlayer = true;
	mPlayerObject = NULL;
	mPlayerPos = Point3F(0,0,0);
	mPlayerForward = Point3F(0,0,0);
	mPlayerForwardYaw, mPlayerForwardPitch = 0;

	mVec = Point3F(0,0,0);

	mYaw = 0;
	mForcedYawOn = false;
	mForcedYaw = 0;
	mForcedYawSpeed = 0;

	mPitch = 0;
	mForcedPitchOn = false;
	mForcedPitch = 0;
	mForcedPitchSpeed = 0;

	mRadius = 0;
	mRadiusSpeed = 0;
	mForcedRadiusOn = false;
	mForcedRadius = 0;
	mForcedRadiusSpeed = 0;
	
	mOffCenterXTarget = 1.0f;	//player on right of screen
	mOffCenterXCurrent = 1.0f;

	mAutoYaw = false;
}

CameraGoalPlayer::~CameraGoalPlayer()
{

}


//----------------------------------------------------------------------------

bool CameraGoalPlayer::onAdd()
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

void CameraGoalPlayer::onRemove()
{
	removeFromScene();
	Parent::onRemove();
}

bool CameraGoalPlayer::onNewDataBlock( GameBaseData *dptr, bool reload )
{
	mDataBlock = dynamic_cast<CameraGoalPlayerData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
		return false;

	mRadius = mDataBlock->radiusDefault;

	scriptOnNewDataBlock();
	return true;
}

void CameraGoalPlayer::getCameraTransform(F32* pos, MatrixF* mat)
{
	Point3F finalPos = getFinalPosition(mYaw, mPitch);

	//default
	getEyeTransform(mat);
	mat->setPosition(finalPos);

	//nothing to do if we're playerless
	if(!mPlayerObject)
		return;

	//We're going to do 4 raycasts in a box around the
	//mPlayerPos to finalPos vector. We'll choose the least
	//t value of these and snap in front based on that
	//value. Using 4 traces helps prevent near-clipping
	
	const F32 d = 0.1f; //dimension of box

	VectorF forward(finalPos - mPlayerPos);
	VectorF temp(forward);
	temp.normalizeSafe();
	VectorF up(0.0f, 0.0f, 1.0f);
	VectorF side, normal;
	mCross(up, temp, &side);
	mCross(side, temp, &normal);

	normal.normalize(d);
	side.normalize(d);

	Point3F pts[4] = {
		mPlayerPos + normal + side,
		mPlayerPos + normal - side,
		mPlayerPos - normal + side,
		mPlayerPos - normal - side
	};

	//we don't want to hit ourselves or the player
	mPlayerObject->disableCollision();
	this->disableCollision();

	RayInfo rInfo;  F32 tBest = F32_MAX;
	for(U32 i = 0; i < 4; i++)
	{
		if(getContainer()->castRay(pts[i], pts[i] + forward, StaticObjectType, &rInfo) && rInfo.t > 0)
		{
			//are we *not* ignoring this object?
			if(!rInfo.object->cameraIgnores())
			{
				#ifdef ENABLE_DEBUGDRAW
				DebugDrawer::get()->drawLine(pts[i], pts[i] + forward, ColorI::RED); 
            DebugDrawer::get()->setLastTTL(TickMs);
				#endif

				//did this one hit closer?
				if(rInfo.t < tBest)
					tBest = rInfo.t;
			}
		}
		else
		{
			#ifdef ENABLE_DEBUGDRAW
			DebugDrawer::get()->drawLine(pts[i], pts[i] + forward, ColorI::GREEN); 
         DebugDrawer::get()->setLastTTL(TickMs);
			#endif
		}
	}

	//re-enable collisions
	this->enableCollision();
	mPlayerObject->enableCollision();

	if(tBest < F32_MAX)
	{
		//hit something, snap in front
		mat->setColumn(3, mPlayerPos + (forward * tBest));
	}
}

void CameraGoalPlayer::processTick(const Move* move)
{
	Parent::processTick(move);

	if (!move)
		move = &NullMove;

	// Update delta
	delta.rotVec = mRot;
	delta.posVec = mPosition;

	//check if we have a player object
	if (mPlayerObject)
	{
		//grab current player position
		mPlayerPos = mPlayerObject->getNodePosition("Cam");

		//grab current player forward vector
		mPlayerObject->getRenderTransform().getColumn(1, &mPlayerForward);
		MathUtils::getAnglesFromVector(mPlayerForward, mPlayerForwardYaw, mPlayerForwardPitch);

		//if this is the first tick we have a player
		if(mFirstTickWithPlayer)
		{
			mFirstTickWithPlayer = false;

			//warp instantly to perfect position behind player
			orbitToYaw(mPlayerForwardYaw + M_PI_F);
			orbitToPitch(0.0f);
		}

		//--------------------------------------------
		// update trigger states
		//--------------------------------------------
		const U16 trigNum = 4;
		bool trigPressed = move->trigger[trigNum];
		bool trigJustPressed = move->trigger[trigNum] && (move->trigger[trigNum] != delta.move.trigger[trigNum]);
		bool trigJustReleased = !move->trigger[trigNum] && (move->trigger[trigNum] != delta.move.trigger[trigNum]);

		if(trigJustPressed)
		{
			//set our radius speed so we'll arrive in exactly manualTransitionTime ms
			F32 radiusDiff = mRadius - mDataBlock->radiusManual;
			mRadiusSpeed = mFabs(radiusDiff / (mDataBlock->manualTransitionTime / 1000.0f));
			mRadiusSpeed = mClampF(mRadiusSpeed, 0, F32_MAX);
		}
		if(trigJustReleased)
		{
			//set our radius speed so we'll arrive in exactly manualTransitionTime ms
			F32 radiusDiff = mRadius - mDataBlock->radiusDefault;
			mRadiusSpeed = mFabs(radiusDiff / (mDataBlock->manualTransitionTime / 1000.0f));
			mRadiusSpeed = mClampF(mRadiusSpeed, 0, F32_MAX);

			//update our forced speeds (so we'll arrive at the
			//forced position in exactly manualTransitionTime ms)
			if(mForcedRadiusOn)
				setForcedRadius(mForcedRadius, mDataBlock->manualTransitionTime);
			if(mForcedYawOn)
				setForcedYaw(mForcedYaw, mDataBlock->manualTransitionTime);
			if(mForcedPitchOn)
				setForcedPitch(mForcedPitch, mDataBlock->manualTransitionTime);
		}


		//--------------------------------------------
		// update left/right offset
		//--------------------------------------------
		updateOffset();


		//--------------------------------------------
		// update radius
		//--------------------------------------------

		//manual orbit
		if(trigPressed)
		{
			zoomToRadius(mDataBlock->radiusManual, mRadiusSpeed);
		}

		//forced radius
		else if(mForcedRadiusOn)
		{
			zoomToRadius(mForcedRadius, mForcedRadiusSpeed);
		}

		//default radius
		else
		{
			zoomToRadius(mDataBlock->radiusDefault, mRadiusSpeed);
		}


		//--------------------------------------------
		// update yaw & pitch
		//--------------------------------------------

		//remember unlocked yaw & pitch in case we need them
		VectorF unlockedVec = mPosition - mPlayerPos;
		F32 unlockedYaw, unlockedPitch;
		MathUtils::getAnglesFromVector(unlockedVec, unlockedYaw, unlockedPitch);

		//--------------------------------------------
		// yaw
		//--------------------------------------------
		{
			//manual orbit
			if(trigPressed)
			{
				orbitToYaw(mYaw + move->yaw);
			}

			//orbit to forced yaw angle
			else if(mForcedYawOn)
			{
				orbitToYaw(mForcedYaw, mForcedYawSpeed);
			}

			// orbit to player's back
			else if(mPlayerObject->mClimbState.active
				|| mPlayerObject->mWallHugState.active
				|| mPlayerObject->mLedgeState.active)
			{
				orbitToYaw(mPlayerForwardYaw + M_PI_F, 3.0f);
			}

			//experimental "autoYaw"
			else if(mAutoYaw)
			{
				orbitToYaw(findAutoYaw(), 0.5f);
			}

			//allow yaw to change freely
			else
			{
				orbitToYaw(unlockedYaw);
			}
		}

		//--------------------------------------------
		// pitch
		//--------------------------------------------
		{
			//manual orbit
			if(trigPressed)
			{
				orbitToPitch(mPitch + move->pitch);
			}

			//orbit to forced pitch angle, keep yaw
			else if(mForcedPitchOn)
			{
				orbitToPitch(mForcedPitch, mForcedPitchSpeed);
			}

			// orbit to player's back
			else if(mPlayerObject->mClimbState.active ||
				mPlayerObject->mWallHugState.active ||
				mPlayerObject->mLedgeState.active)
			{
				orbitToPitch(0.0f, 3.0f);
			}

			//allow pitch to change freely
			else
			{
				orbitToPitch(unlockedPitch);
				autoPitch();
			}
		}


		//--------------------------------------------
		// avoid obstructions
		//--------------------------------------------
		if((!mPlayerObject->mClimbState.active
			&& !mPlayerObject->mWallHugState.active
			&& !mPlayerObject->mLedgeState.active
			&& !trigPressed)
			&& (!mForcedYawOn || !mForcedPitchOn))
		{
			//at least one axis is unlocked, so we could
			//try to resolve any potential obstructions

			//check for obstructions
			Point3F finalPos = getFinalPosition(mYaw, mPitch);
			if(!viewClear(finalPos))
			{
				//okay something is between us and the player
				if(!mForcedYawOn && mForcedPitchOn)
				{
					//we can only solve with yaw
					F32 newYaw;
					if(findClearYaw(&newYaw))
						orbitToYaw(newYaw);
				}

				if(!mForcedPitchOn && mForcedYawOn)
				{
					//we can only solve with pitch
					F32 newPitch;
					if(findClearPitch(&newPitch))
						orbitToPitch(newPitch);
				}
				
				if(!mForcedYawOn && !mForcedPitchOn)
				{
					//we have full freedom to resolve

					F32 newYaw, newPitch;
					bool foundClearYaw = findClearYaw(&newYaw);
					bool foundClearPitch = findClearPitch(&newPitch);

					//determine if camera itself is inside geometry
					RayInfo rInfo;
					bool camInside = getContainer()->castRay(finalPos, mPlayerPos, StaticObjectType, &rInfo) && rInfo.t == 0;

					//if camera is inside geometry, prefer to solve with pitch (I'm not
					//sure why this heuristic is "right" - but pitch seems to resolve
					//the back-into-wall scenario in a more pleasing way than yaw)
					if(camInside && foundClearPitch)
					{
						orbitToPitch(newPitch);
					}

					//if both methods found solutions, choose
					//whichever solution requires the least change
					if(foundClearYaw && foundClearPitch)
					{
						F32 yawDiff = mYaw - newYaw;
						if (yawDiff < -M_PI_F)
							yawDiff += M_2PI_F;
						if (yawDiff > M_PI_F)
							yawDiff -= M_2PI_F;

						F32 pitchDiff = mPitch - newPitch;
						if (pitchDiff < -M_PI_F)
							pitchDiff += M_2PI_F;
						if (pitchDiff > M_PI_F)
							pitchDiff -= M_2PI_F;

						if(mFabs(pitchDiff) <= mFabs(yawDiff))
						{
							//use pitch
							orbitToPitch(newPitch);
						}
						else
						{
							//use yaw
							orbitToYaw(newYaw);
						}
					}
					
					//otherwise just solve with the one that found a solution
					else if(foundClearPitch)
					{
						orbitToPitch(newPitch);
					}
					else if(foundClearYaw)
					{
						orbitToYaw(newYaw);
					}
					else
					{
						//we failed to find a solution

						//this can happen when the player is surrounded
						//by geometry such that neither yaw (on current pitch)
						//or pitch (on current yaw) can fix it. We could try
						//searching the whole sphere of solutions here...
					}
				}
			}
		}

		//--------------------------------------------
		//look toward player
		//--------------------------------------------
		mRot.x = mPitch;
		mRot.z = mYaw + M_PI_F;


		#ifdef ENABLE_DEBUGDRAW
		DebugDrawer::get()->drawBox(mPlayerPos - Point3F(.1f,.1f,.1f), mPlayerPos + Point3F(.1f,.1f,.1f), ColorI::GREEN); 
      DebugDrawer::get()->setLastTTL(TickMs);
		DebugDrawer::get()->drawLine(mPlayerPos, mPosition, ColorI::GREEN);
      DebugDrawer::get()->setLastTTL(TickMs);
		#endif
	}

	// If on the client, calc delta for backstepping
	if (isClientObject()) 
	{
		delta.pos = mPosition;
		delta.rot = mRot;
		delta.posVec = delta.posVec - delta.pos;
		delta.rotVec = delta.rotVec - delta.rot;
	}

	delta.move = *move;

	setPosition(mPosition, mRot);
}

//-------------------------------------------------------------------
// CameraGoalPlayer::autoPitch
//
// 
//-------------------------------------------------------------------
void CameraGoalPlayer::autoPitch()
{
	const U16 castsPerCircle = 24;
	const F32 angleStep = M_2PI_F / castsPerCircle;

	Point3F playerPos = mPlayerObject->getPosition();

	F32 totalPitch = 0.0f;
	F32 totalWeight = 0.0f;

	for(U16 i = 0; i < castsPerCircle; i++)
	{
		F32 yaw = angleStep * i;
		F32 pitch = 0.0f;

		Point3F vec;
		MathUtils::getVectorFromAngles(vec, yaw, pitch);
		vec *= mDataBlock->pitchDetectRadius;
		Point3F start = playerPos + vec + Point3F(0,0,10.0f);
		Point3F end = playerPos + vec - Point3F(0,0,10.0f);

		//calculate weight, rays facing same direction as camera are higher
		VectorF vecNorm(vec); VectorF mVecNorm(VectorF(mVec.x, mVec.y, 0));
		vecNorm.normalizeSafe(); mVecNorm.normalizeSafe();
		F32 weight = -mDot(vecNorm, mVecNorm);
		weight = mClampF(weight, 0.0f, 1.0f);
		weight += 0.5f;

		//first cast from player to start (to ensure the "pit" is actually accessible)
		RayInfo rInfo;
		if(!getContainer()->castRay(mPlayerPos, mPlayerPos + vec, StaticObjectType, &rInfo))
		{
			//okay didn't hit anything, pit is accessible
			//now cast down from start to end to find the ground
			if(getContainer()->castRay(start, end, StaticObjectType, &rInfo))
			{
				/* //debug lines
				#ifdef ENABLE_DEBUGDRAW
				gDebugDraw->drawLine(start, rInfo.point, ColorF(1,0,0)); gDebugDraw->setLastTTL(TickMs);
				gDebugDraw->drawLine(rInfo.point, end, ColorF(0,0,1)); gDebugDraw->setLastTTL(TickMs);
				gDebugDraw->drawLine(playerPos, rInfo.point, ColorF(1,0,1)); gDebugDraw->setLastTTL(TickMs);
				#endif
				*/

				//make a vector between this hit point and player pos
				VectorF collToPlayerVec = playerPos - rInfo.point;
				F32 cyaw, cpitch;
				MathUtils::getAnglesFromVector(collToPlayerVec, cyaw, cpitch);
				totalPitch += cpitch * weight;
				totalWeight += weight;
			}
			else
			{
				//didn't hit anything, huge pit (count as highest possible)
				totalPitch += mDataBlock->pitchMax * weight;
				totalWeight += weight;
			}
		}
		else
		{
			//something is in the way (count as 0)
			totalPitch += 0.0f * weight;
			totalWeight += weight;
		}
	}

	if(totalWeight > 0.0f)
	{
		F32 averagePitch = totalPitch / totalWeight;

		//will we pitch up from this?
		if(mPitch <= averagePitch)
		{
			//will we hit anything if we do?
			if(canPitchUp())
			{
				//orbit to that pitch, keep yaw
				F32 diff = averagePitch - mPitch;
				diff *= mDataBlock->pitchMult;
				orbitToPitch(mPitch + diff, 0.5f);
			}
		}
	}

	//--------------------------------------------
	// return pitch to zero
	//--------------------------------------------
	if(mPitch > 0 ? canPitchDown() : canPitchUp())
	{
		mPitch *= mDataBlock->pitchMult;
		orbitToPitch(mPitch, 0.5f);
	}
}

//-------------------------------------------------------------------
// CameraGoalPlayer::updateOffset
//
// Updates offset such that player will always face screen center
//-------------------------------------------------------------------
void CameraGoalPlayer::updateOffset()
{
	//--------------------------------------------
	//Step 1
	// - do we need to switch between left/right?
	//--------------------------------------------
	//calculate diff between playerForward and cameraForward
	F32 deltaYaw = mPlayerForwardYaw - mYaw;

	//fix it -PI to +PI
	while (deltaYaw < -M_PI_F)
		deltaYaw += M_2PI_F;
	while (deltaYaw > M_PI_F)
		deltaYaw -= M_2PI_F;

	//mirror it on the -PI/2 to PI/2 line
	//effectively making "facing camera" or "back to camera" the same
	if (deltaYaw < -M_PI_F / 2.0f)
		deltaYaw = -(M_PI_F + deltaYaw);
	if (deltaYaw > M_PI_F / 2.0f)
		deltaYaw = M_PI_F - deltaYaw;

	//offset left/right is only changed if we're outside the dead-zone (~12 degrees)
	//(there are really 2 dead-zones created by the mirroring above)
	if(deltaYaw > 0.2f) //TODO: datablock
		mOffCenterXTarget = 1.0f;
	else if (deltaYaw < -0.2f)
		mOffCenterXTarget = -1.0f;

	//--------------------------------------------
	//Step 2
	// - if we can avoid an obstruction by offsetting
	//   in the other direction, let's do it
	//--------------------------------------------
	//we only need to do this if forced yaw is on
	//otherwise occlusion is handled normally (by orbiting)
	if(mForcedYawOn)
	{
		//occlusion on this side?
		if(!viewClear(getFinalPosition(mYaw, mPitch, mOffCenterXTarget)))
		{
			//no occlusion on other side?
			if(viewClear(getFinalPosition(mYaw, mPitch, -mOffCenterXTarget)))
			{
				//switch sides
				mOffCenterXTarget *= -1.0f;
			}
		}
	}

	//--------------------------------------------
	//Step 3
	// - given an offset target, update our current offset
	//--------------------------------------------
	F32 diff = mOffCenterXTarget - mOffCenterXCurrent;

	//speed-limit it
	diff *= 0.03f;
	diff = mClampF(diff, -2.5f * TickSec, 2.5f * TickSec);	//TODO: datablock (1.0 is a 2 second switch)

	mOffCenterXCurrent += diff;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::viewClear
//
// Returns true if the view from the given point to the player is un-obstructed
//-------------------------------------------------------------------
bool CameraGoalPlayer::viewClear(Point3F from)
{
	//There are probably a million+1 ways to write this method, but
	//we're going to test 3 points on the player for visibility.
	//If the camera can see all 3, we'll consider him visible.

	//For a humanoid, these points very roughly correspond to:
	// - left shoulder
	// - right shoulder
	// - pelvis

	//we don't want to hit ourselves or the player
	mPlayerObject->disableCollision();
	this->disableCollision();

	Point3F vec = from - mPlayerPos;

	//calculate our base-points
	Box3F wBox = mPlayerObject->getWorldBox();
	Point3F boxMid; wBox.getCenter(&boxMid);
	Point3F boxTop(boxMid); boxTop.z += wBox.len_z() / 4.0f; // 3/4 height
	Point3F boxBot(boxMid); boxBot.z -= wBox.len_z() / 4.0f; // 1/4 height

	//calculate an offset left/right
	Point3F up = Point3F(0,0,1);
	Point3F temp(vec); temp.normalizeSafe();
	Point3F offset = mCross(temp, up);
	Box3F oBox = mPlayerObject->getObjBox();
	offset.normalize(getMin(oBox.len_x(),oBox.len_y()) / 2.0f - 0.01f);	//width of the player box (minus a bit)

	Point3F pts[3] = {
		boxTop + offset,
		boxTop - offset,
		boxBot
	};

	//okay, test the points for visibility
	RayInfo rInfo;
	for(U32 i = 0; i < 3; i++)
	{
		#ifdef ENABLE_DEBUGDRAW
		DebugDrawer::get()->drawLine(pts[i] + vec, pts[i], ColorI::GREEN);
      DebugDrawer::get()->setLastTTL(TickMs);
		#endif

		if(getContainer()->castRay(pts[i] + vec, pts[i], StaticObjectType, &rInfo))
		{
			//are we *not* ignoring this object?
			if(!rInfo.object->cameraIgnores())
			{
				//re-enable collisions
				this->enableCollision();
				mPlayerObject->enableCollision();

				//we hit something, no good
				return false;
			}
		}
	}

	//re-enable collisions
	this->enableCollision();
	mPlayerObject->enableCollision();

	//all clear!
	return true;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::orbitToYaw
//
// Orbits the camera around the player to the specified yaw
// Speed is in radians per second
//-------------------------------------------------------------------
void CameraGoalPlayer::orbitToYaw(F32 yaw, F32 speed)
{
	AssertFatal(speed >= 0.0f, "Speed must be >= 0");

	//account for tick length
	speed *= TickSec;

	F32 yawDiff = mYaw - yaw;
	while (yawDiff < -M_PI_F)
		yawDiff += M_2PI_F;
	while (yawDiff > M_PI_F)
		yawDiff -= M_2PI_F;

	//apply rate limit
	yawDiff = mClampF(yawDiff, -speed, speed);
	mYaw -= yawDiff;
	MathUtils::getVectorFromAngles(mVec, mYaw, mPitch);

	//maintain radius
	mVec.normalize(mRadius);

	//maintain position
	mPosition = mPlayerPos + mVec;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::orbitToPitch
//
// Orbits the camera around the player to the specified pitch
// Speed is in radians per second
//-------------------------------------------------------------------
void CameraGoalPlayer::orbitToPitch(F32 pitch, F32 speed)
{
	AssertFatal(speed >= 0.0f, "Speed must be >= 0");

	//account for tick length
	speed *= TickSec;

	//clamp pitch
	pitch = mClampF(pitch, -mDataBlock->pitchMax, mDataBlock->pitchMax);

	F32 pitchDiff = mPitch - pitch;
	while (pitchDiff < -M_PI_F)
		pitchDiff += M_2PI_F;
	while (pitchDiff > M_PI_F)
		pitchDiff -= M_2PI_F;

	//apply rate limit
	pitchDiff = mClampF(pitchDiff, -speed, speed);
	mPitch -= pitchDiff;
	MathUtils::getVectorFromAngles(mVec, mYaw, mPitch);

	//maintain radius
	mVec.normalize(mRadius);

	//maintain position
	mPosition = mPlayerPos + mVec;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::zoomToRadius
//
// Zooms the camera in/out to the specified distance from the player
// Speed is in world units per second
//-------------------------------------------------------------------
void CameraGoalPlayer::zoomToRadius(F32 radius, F32 speed)
{
	F32 radiusDiff = radius - mRadius;
	radiusDiff  = mClampF(radiusDiff, -speed * TickSec, speed * TickSec);
	mRadius += radiusDiff;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::getFinalPosition
//
// Given a base yaw & pitch, determines the final (actual) position of
// the camera, including any "screen space" offsets that need to occur
//-------------------------------------------------------------------
Point3F CameraGoalPlayer::getFinalPosition(F32 yaw, F32 pitch)
{
	//use mOffCenterXCurrent by default
	return getFinalPosition(yaw, pitch, mOffCenterXCurrent);
}
Point3F CameraGoalPlayer::getFinalPosition(F32 yaw, F32 pitch, F32 offCenterX)
{
	Point3F vec;
	MathUtils::getVectorFromAngles(vec, yaw, pitch);
	vec *= mRadius;
	Point3F pt = mPlayerPos + vec;

	MatrixF xRot, zRot;
	xRot.set(EulerF(pitch, 0.0f, 0.0f));
	zRot.set(EulerF(0.0f, 0.0f, yaw));
	MatrixF temp;
	temp.mul(zRot, xRot);

	//apply cosine interpolation
	F32 t = (offCenterX + 1.0f) / 2.0f; //remap from [-1 to 1] to [0 to 1]
	F32 t2 = (1.0f - mCos(t * M_PI_F)) / 2.0f;
	F32 interpolated = (-1.0f * (1.0f - t2) + 1.0f * t2);

	Point3F offset(mDataBlock->offCenterX * interpolated, 0, mDataBlock->offCenterY);
	temp.mulV(offset);

	return pt + offset;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::findClearPitch
//
// Attempts to solve an obstruction by finding the pitch value that
// results in a clear view to the player, (and requires the least change).
// Returns true on success, false on failure. If true, value is stored in pitch.
//-------------------------------------------------------------------
bool CameraGoalPlayer::findClearPitch(F32* pitch)
{
	//Start at the current camera angle and conduct visibility
	//tests in both directions (+/-) simultaneously. The first
	//test that doesn't hit anything wins!

	for(U16 i = 0; i < sCastsPerCircle / 2; i++)
	{
		//try in both directions each time
		for(U16 j = 0; j < 2; j++)
		{
			//this will be used as a multiplier below indicating
			//which direction to search in
			F32 mult = j == 0 ? -1.0f : 1.0f;

			F32 yaw = mYaw;
			*pitch = mPitch + mult*(sAngleStep * i);

			//don't bother if we're out of range
			if(*pitch < mDataBlock->pitchMax && *pitch > -mDataBlock->pitchMax)
			{
				Point3F fromPt = getFinalPosition(yaw, *pitch);
				if(viewClear(fromPt))
				{
					//we found a clear view
					return true;
				}
			}
		}
	}

	//we failed
	return false;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::findClearYaw
//
// Attempts to solve an obstruction by finding the yaw value that
// results in a clear view to the player, (and requires the least change).
// Returns true on success, false on failure. If true, value is stored in yaw.
//-------------------------------------------------------------------
bool CameraGoalPlayer::findClearYaw(F32* yaw)
{
	//Start at the current camera angle and conduct visibility
	//tests in both directions (+/-) simultaneously. The first
	//test that doesn't hit anything wins!

	for(U16 i = 0; i < sCastsPerCircle / 2; i++)
	{
		//try in both directions each time
		for(U16 j = 0; j < 2; j++)
		{
			//this will be used as a multiplier below indicating
			//which direction to search in
			F32 mult = j == 0 ? -1.0f : 1.0f;

			*yaw = mYaw + mult*(sAngleStep * i);
			F32 pitch = mPitch;

			Point3F fromPt = getFinalPosition(*yaw, pitch);
			if(viewClear(fromPt))
			{
				//we found a clear view
				return true;
			}
		}
	}

	//we failed
	return false;
}

//-------------------------------------------------------------------
// CameraGoalPlayer::canPitchUp
//
// Returns true if the camera could pitch up (move higher)
// (by mAngleStep) without causing an occlusion
//-------------------------------------------------------------------
bool CameraGoalPlayer::canPitchUp()
{
	F32 pitch = mPitch + sAngleStep;
	Point3F fromPt = getFinalPosition(mYaw, pitch);
	return viewClear(fromPt);
}

//-------------------------------------------------------------------
// CameraGoalPlayer::canPitchDown
//
// Returns true if the camera could pitch down (move lower)
// (by mAngleStep) without causing an occlusion
//-------------------------------------------------------------------
bool CameraGoalPlayer::canPitchDown()
{
	F32 pitch = mPitch - sAngleStep;
	Point3F fromPt = getFinalPosition(mYaw, pitch);
	return viewClear(fromPt);
}

//-------------------------------------------------------------------
// CameraGoalPlayer::findAutoYaw()
//
// Returns the average normal of geometry near the player. This can
// be useful for prototyping, providing 2D-style gameplay, or tracking
// the player around oddly shaped walls or towers etc (mostly experimental)
//-------------------------------------------------------------------
F32 CameraGoalPlayer::findAutoYaw()
{
	//cast rays from player out to points on camera radius
	//average the normals of every surface found

	Point3F averageNormal(0,0,0);
	F32 hits = 0;

	Point3F start, end;
	start = mPlayerPos;

	//we don't want to hit ourselves or the player
	mPlayerObject->disableCollision();
	this->disableCollision();

	const U16 castsPerCircle = 48;
	const F32 angleStep = M_2PI_F / castsPerCircle;

	for(U16 i = 0; i < castsPerCircle; i++)
	{
		F32 angle = i * angleStep;

		Point3F dir( sin(angle), cos(angle), 0);
		dir *= mRadius;

		end.set(start);
		end += dir;

		RayInfo rinfo;
		if(getContainer()->castRay(start, end, StaticObjectType, &rinfo))
		{
#ifdef ENABLE_DEBUGDRAW
         DebugDrawer::get()->drawLine(start, end, ColorI::GREEN);
			DebugDrawer::get()->setLastTTL(TickMs);
#endif

			averageNormal += rinfo.normal;
			hits++;
		}
#ifdef ENABLE_DEBUGDRAW
		DebugDrawer::get()->drawLine(start, end, ColorI::RED);
		DebugDrawer::get()->setLastTTL(TickMs);
#endif
	}

	//re-enable collisions
	this->enableCollision();
	mPlayerObject->enableCollision();

	if(hits < 4)
		return mYaw;
	else
	{
		averageNormal /= hits;
		F32 averageNormalYaw, averageNormalPitch;
		MathUtils::getAnglesFromVector(averageNormal, averageNormalYaw, averageNormalPitch);
		return averageNormalYaw;
	}
}

void CameraGoalPlayer::onDeleteNotify(SimObject *obj)
{
	Parent::onDeleteNotify(obj);

	if (obj == (SimObject*)mPlayerObject)
	{
		mPlayerObject = NULL;
		mFirstTickWithPlayer = true;
		setMaskBits(PlayerMask);
	}
}

void CameraGoalPlayer::interpolateTick(F32 dt)
{
	Parent::interpolateTick(dt);
	Point3F rot = delta.rot + delta.rotVec * dt;
	Point3F pos = delta.pos + delta.posVec * dt;
	setRenderPosition(pos,rot);
}

void CameraGoalPlayer::setPosition(const Point3F& pos, const Point3F& rot)
{
	MatrixF xRot, zRot;
	xRot.set(EulerF(rot.x, 0.0f, 0.0f));
	zRot.set(EulerF(0.0f, 0.0f, rot.z));

	MatrixF temp;
	temp.mul(zRot, xRot);
	temp.setColumn(3, pos);
	Parent::setTransform(temp);

	mPosition = pos;
	mRot = rot;
	
	setMaskBits(MoveMask);
}

void CameraGoalPlayer::setRenderPosition(const Point3F& pos,const Point3F& rot)
{
	MatrixF xRot, zRot;
	xRot.set(EulerF(rot.x, 0, 0));
	zRot.set(EulerF(0, 0, rot.z));
	MatrixF temp;
	temp.mul(zRot, xRot);
	temp.setColumn(3, pos);
	Parent::setRenderTransform(temp);
}

U32 CameraGoalPlayer::packUpdate(NetConnection *con, U32 mask, BitStream *bstream)
{
	U32 retMask = Parent::packUpdate(con, mask, bstream);

	if (bstream->writeFlag(mask & MoveMask))
	{
		bstream->write(mYaw);
		bstream->write(mPitch);
		bstream->write(mRadius);
		mathWrite(*bstream, mPosition);
		mathWrite(*bstream, mRot);
	}

	if(bstream->writeFlag((mask & PlayerMask) && mPlayerObject))
	{
		S32 id = con->getGhostIndex(mPlayerObject);
		bstream->write(id);

		//hack to keep sending updates until player is ghosted
		if(id == -1)
			setMaskBits(PlayerMask);
	}

	if (bstream->writeFlag(mask & ModeMask))
	{
		bstream->write(mForcedYawOn);
		bstream->write(mForcedYaw);
		bstream->write(mForcedYawSpeed);

		bstream->write(mForcedPitchOn);
		bstream->write(mForcedPitch);
		bstream->write(mForcedPitchSpeed);

		bstream->write(mRadiusSpeed);
		bstream->write(mForcedRadiusOn);
		bstream->write(mForcedRadius);
		bstream->write(mForcedRadiusSpeed);

		bstream->write(mAutoYaw);
	}

	return retMask;
}

void CameraGoalPlayer::unpackUpdate(NetConnection *con, BitStream *bstream)
{
	Parent::unpackUpdate(con,bstream);

	//MoveMask
	if (bstream->readFlag())
	{
		bstream->read(&mYaw);
		bstream->read(&mPitch);
		bstream->read(&mRadius);
		mathRead(*bstream, &mPosition);
		mathRead(*bstream, &mRot);

		setPosition(mPosition,mRot);
		delta.pos = mPosition;
		delta.rot = mRot;
		delta.rotVec.set(0.0f, 0.0f, 0.0f);
		delta.posVec.set(0.0f, 0.0f, 0.0f);
	}

	//PlayerMask
	if (bstream->readFlag())
	{
		S32 id;
		bstream->read(&id);
		if(id > 0)
		{
			Player* playerObject = static_cast<Player*>(con->resolveGhost(id));
			setPlayerObject(playerObject);
		}
	}

	//ModeMask
	if(bstream->readFlag())
	{	
		bstream->read(&mForcedYawOn);
		bstream->read(&mForcedYaw);
		bstream->read(&mForcedYawSpeed);

		bstream->read(&mForcedPitchOn);
		bstream->read(&mForcedPitch);
		bstream->read(&mForcedPitchSpeed);

		bstream->read(&mRadiusSpeed);
		bstream->read(&mForcedRadiusOn);
		bstream->read(&mForcedRadius);
		bstream->read(&mForcedRadiusSpeed);

		bstream->read(&mAutoYaw);
	}
}

void CameraGoalPlayer::setTransform(const MatrixF& mat)
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
}

void CameraGoalPlayer::setRenderTransform(const MatrixF& mat)
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

bool CameraGoalPlayer::setPlayerObject(Player *obj)
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

ConsoleMethod( CameraGoalPlayer, setPlayerObject, bool, 3, 3, "(Player object)") {   
	Player *gb;
	if(!Sim::findObject(argv[2], gb))
	{
		Con::errorf("CameraGoalPlayer::setPlayerObject - failed to find object '%s'", argv[2]);
		return false;
	}

	return object->setPlayerObject(gb);
}

//----------------------------------------------------------------------

void CameraGoalPlayer::setForcedYaw(F32 yaw, S32 ms)
{
	mForcedYawOn = true;
	mForcedYaw = yaw;
	F32 yawDiff = mYaw - mForcedYaw;
	while (yawDiff < -M_PI_F)
		yawDiff += M_2PI_F;
	while (yawDiff > M_PI_F)
		yawDiff -= M_2PI_F;
	mForcedYawSpeed = mFabs(yawDiff / (ms / 1000.0f));
	mForcedYawSpeed = mClampF(mForcedYawSpeed, 0, F32_MAX);

	setMaskBits(ModeMask);
}

void CameraGoalPlayer::clearForcedYaw()
{
	mForcedYawOn = false;

	setMaskBits(ModeMask);
}

ConsoleMethod( CameraGoalPlayer, setForcedYaw, void, 4, 4, "(yaw radians, ms)")
{
	if(dStrlen(argv[2]) > 0)
	{
		object->setForcedYaw(dAtof(argv[2]), dAtoi(argv[3]));
	}
	else
	{
		object->clearForcedYaw();
	}
}

//----------------------------------------------------------------------

void CameraGoalPlayer::setForcedPitch(F32 pitch, S32 ms)
{
	mForcedPitchOn = true;
	mForcedPitch = pitch;
	F32 pitchDiff = mPitch - mForcedPitch;
	while (pitchDiff < -M_PI_F)
		pitchDiff += M_2PI_F;
	while (pitchDiff > M_PI_F)
		pitchDiff -= M_2PI_F;
	mForcedPitchSpeed = mFabs(pitchDiff / (ms / 1000.0f));
	mForcedPitchSpeed = mClampF(mForcedPitchSpeed, 0, F32_MAX);

	setMaskBits(ModeMask);
}

void CameraGoalPlayer::clearForcedPitch()
{
	mForcedPitchOn = false;

	setMaskBits(ModeMask);
}

ConsoleMethod( CameraGoalPlayer, setForcedPitch, void, 4, 4, "(pitch radians, ms)")
{
	if(dStrlen(argv[2]) > 0)
	{
		object->setForcedPitch(dAtof(argv[2]), dAtoi(argv[3]));
	}
	else
	{
		object->clearForcedPitch();
	}
}

//----------------------------------------------------------------------

void CameraGoalPlayer::setForcedRadius(F32 radius, S32 ms)
{
	mForcedRadiusOn = true;
	mForcedRadius = radius;
	F32 radiusDiff = mRadius - mForcedRadius;
	mForcedRadiusSpeed = mFabs(radiusDiff / (ms / 1000.0f));
	mForcedRadiusSpeed = mClampF(mForcedRadiusSpeed, 0, F32_MAX);

	setMaskBits(ModeMask);
}

void CameraGoalPlayer::clearForcedRadius(S32 ms)
{
	mForcedRadiusOn = false;

	//set our radius speed so we'll arrive in exactly "ms" ms
	F32 radiusDiff = mRadius - mDataBlock->radiusDefault;
	mRadiusSpeed = mFabs(radiusDiff / (ms / 1000.0f));
	mRadiusSpeed = mClampF(mRadiusSpeed, 0, F32_MAX);

	setMaskBits(ModeMask);
}

ConsoleMethod( CameraGoalPlayer, setForcedRadius, void, 4, 4, "(radius, ms)")
{
	if(dStrlen(argv[2]) > 0)
	{
		object->setForcedRadius(dAtof(argv[2]), dAtoi(argv[3]));
	}
	else
	{
		object->clearForcedRadius(dAtoi(argv[3]));
	}
}

//----------------------------------------------------------------------

void CameraGoalPlayer::setAutoYaw(bool on)
{
	mAutoYaw = on;
	setMaskBits(ModeMask);
}

ConsoleMethod( CameraGoalPlayer, setAutoYaw, void, 3, 3, "(bool)")
{
	object->setAutoYaw(dAtob(argv[2]));
}
