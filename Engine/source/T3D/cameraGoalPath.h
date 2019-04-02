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

#ifndef _CAMERAGOALPATH_H_
#define _CAMERAGOALPATH_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

#ifndef _PATHMANAGER_H_
#include "scene/pathManager.h"
#endif

//----------------------------------------------------------------------------
// CameraGoalPath
//
// This camera goal provides a view of the player from a designer-specified
// path. There are actually 2 paths required: 1) the player path, to which the
// players current position is compared to determine his "progress" (t) along the
// path, and 2) the camera path, along which the camera is positioned at an
// equal value of t. For rotation, the camera can automatically look at the
// player, or use the rotation specified in the nodes of the camera path.
//----------------------------------------------------------------------------
class CameraGoalPath: public ShapeBase
{
private:
	typedef ShapeBase Parent;

	struct StateDelta {
		F64 time;
		F64 timeVec;
	};
	StateDelta mDelta;

	enum MaskBits {
		TMask			= Parent::NextFreeMask,
		PlayerPathMask	= Parent::NextFreeMask + 1,
		CameraPathMask	= Parent::NextFreeMask + 2,
		PlayerMask		= Parent::NextFreeMask + 3,
		ModeMask		= Parent::NextFreeMask + 4,
		NextFreeMask	= Parent::NextFreeMask << 1
	};

	PathManager* mPathManager;

	QuatF mRot;
	Point3F mPosition;

	void setPosition(const Point3F& pos, const QuatF& rot);
	void setRenderPosition(const Point3F& pos, const QuatF& rot);

	
	F64 mT;					//current t value for this camera
	U32 mPlayerPathIndex;	//the path used to read player position
	U32 mCameraPathIndex;	//the path used to move this camera
	Player* mPlayerObject;	//the player object to track
	bool mLookAtPlayer;

	void interpolateMat(F64 t, MatrixF* mat);
	//void setT(F32 t);



public:
	DECLARE_CONOBJECT(CameraGoalPath);

	CameraGoalPath();
	~CameraGoalPath();

	F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);

	bool setPlayerPathObject(SimPath::Path* obj);
	bool setCameraPathObject(SimPath::Path* obj);
	bool setPlayerObject(Player *obj);
	void setLookAtPlayer(bool on);

	void processTick(const Move*);
	void interpolateTick(F32 dt);
	void getCameraTransform(F32* pos,MatrixF* mat);
	U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *, BitStream *stream);
	void onDeleteNotify(SimObject *obj);
};

#endif