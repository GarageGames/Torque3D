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

#ifndef _CAMERAGOALPLAYER_H_
#define _CAMERAGOALPLAYER_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

//----------------------------------------------------------------------------
// CameraGoalPlayerData
//----------------------------------------------------------------------------
struct CameraGoalPlayerData: public ShapeBaseData
{
private:
	typedef ShapeBaseData Parent;

public:
	F32 manualTransitionTime;	//transition time in/out of manual mode (ms)
	F32 radiusDefault;			//default radius from player (world units)
	F32 radiusManual;			//radius from player during manual mode (world units)
	F32 offCenterY;				//screen-space Y (height) offset 
	F32 offCenterX;				//screen-space X (width) offset
	F32 pitchMax;				//max allowed pitch above or below the horizion (radians)
	F32 pitchMult;				//pitch multiplier applied every tick (returns pitch to 0)
	F32 pitchDetectRadius;		//radius around player where pitch detection raycasts are performed

	DECLARE_CONOBJECT(CameraGoalPlayerData);
	CameraGoalPlayerData();
	~CameraGoalPlayerData();

	static void initPersistFields();
	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);
};

//----------------------------------------------------------------------------
// CameraGoalPlayer
//
// This camera goal provides the default view of the player. It's essentially
// a follow-cam that maintains a specific distance from the player at all
// times. It has some 'intelligence' to detect & remedy player occlusion, and
// tries to keep itself out of walls etc. During specific moves, (or at the
// players request), it can orbit to a locked position behind the player.
// Designers can request that a specific angle be maintained (forcedYaw/pitch),
// or request that the camera remain parallel to the normals of nearby geometry
// (autoYaw), which can be useful for prototyping or tracking the player around
// oddly shaped walls / towers. If desired, CameraGoalPlayer can also produce
// a nice "rule of thirds" effect by keeping the player off-center.
//----------------------------------------------------------------------------
class CameraGoalPlayer: public ShapeBase
{
	typedef ShapeBase Parent;

	CameraGoalPlayerData*   mDataBlock;

	enum MaskBits {
		MoveMask = Parent::NextFreeMask,
		PlayerMask =   MoveMask << 1,
		ModeMask =   MoveMask << 2,
		NextFreeMask = ModeMask << 1
	};

	struct StateDelta {
		Point3F pos;
		Point3F rot;
		VectorF posVec;
		VectorF rotVec;
		Move move;
	};
	StateDelta delta;

	Point3F mRot;				//rotation of this camera goal
	Point3F mPosition;			//position of this camera goal

	bool mFirstTickWithPlayer;	//is this the first tick with our player?
	Player* mPlayerObject;		//player object to follow
	Point3F mPlayerPos;			//position of the player "Cam" node
	Point3F mPlayerForward;		//player forward vector
	F32 mPlayerForwardYaw, mPlayerForwardPitch;

	//player to camera vector
	Point3F mVec;

	F32 mYaw;					//current yaw angle (radians)
	bool mForcedYawOn;			//forcedYaw on/off
	F32 mForcedYaw;				//target yaw angle (radians)
	F32 mForcedYawSpeed;		//speed toward target yaw (radians / second)

	F32 mPitch;					//current pitch angle (radians)
	bool mForcedPitchOn;		//forcedPitch on/off
	F32 mForcedPitch;			//target pitch angle (radians)
	F32 mForcedPitchSpeed;		//speed toward target pitch (radians / second)

	F32 mRadius;				//current radius from the player (world units)
	F32 mRadiusSpeed;			//regular speed toward target radius (world units / second)
	bool mForcedRadiusOn;		//forcedRadius on/off
	F32 mForcedRadius;			//target radius from the player (world units)
	F32 mForcedRadiusSpeed;		//speed toward target radius (world units / second)
	
	F32 mOffCenterXTarget;		//offset target, 1.0 = offset player right, -1.0 = offset player left
	F32 mOffCenterXCurrent;		//offset current, 1.0 = offset player right, -1.0 = offset player left

	bool mAutoYaw;				//if true, camera will orbit automatically to a "good" view

	

	void setPosition(const Point3F& pos,const Point3F& viewRot);
	void setRenderPosition(const Point3F& pos,const Point3F& viewRot);

	void setTransform(const MatrixF& mat);
	void setRenderTransform(const MatrixF& mat);


	bool viewClear(Point3F from);
	bool findClearYaw(F32* yaw);
	bool findClearPitch(F32* pitch);
	bool canPitchUp();
	bool canPitchDown();
	Point3F getFinalPosition(F32 yaw, F32 pitch, F32 offCenterX);
	Point3F getFinalPosition(F32 yaw, F32 pitch);
	void updateOffset();
	void autoPitch();

	void orbitToYaw(F32 yaw, F32 speed = F32_MAX);
	void orbitToPitch(F32 pitch, F32 speed = F32_MAX);
	void zoomToRadius(F32 radius, F32 speed = F32_MAX);

	F32 findAutoYaw();
	
public:
	DECLARE_CONOBJECT(CameraGoalPlayer);

	CameraGoalPlayer();
	~CameraGoalPlayer();

	bool onAdd();
	void onRemove();

	bool onNewDataBlock( GameBaseData *dptr, bool reload );
	void processTick(const Move* move);
	void interpolateTick(F32 delta);
	void getCameraTransform(F32* pos,MatrixF* mat);

	U32  packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn, BitStream *stream);

	void onDeleteNotify(SimObject *obj);

	bool setPlayerObject(Player *obj);
	Player * getPlayerObject()      { return(mPlayerObject); }

	void setForcedYaw(F32 yaw, S32 ms = 0);
	void clearForcedYaw();			//no transition occurs
	
	void setForcedPitch(F32 pitch, S32 ms = 0);
	void clearForcedPitch();		//no transition occurs

	void setForcedRadius(F32 radius, S32 ms = 0);
	void clearForcedRadius(S32 ms);	//transition occurs, needs time

	void setAutoYaw(bool on);
};

#endif