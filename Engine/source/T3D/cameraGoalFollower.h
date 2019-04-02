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

#ifndef _GOALCAMERA_H_
#define _GOALCAMERA_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

//----------------------------------------------------------------------------
// CameraGoalFollowerData
//----------------------------------------------------------------------------
struct CameraGoalFollowerData: public ShapeBaseData
{
private:
	typedef ShapeBaseData Parent;

public:
	S32 rotationHistorySize;
	S32 positionHistorySize;

	DECLARE_CONOBJECT(CameraGoalFollowerData);
	CameraGoalFollowerData();
	~CameraGoalFollowerData();
	static void initPersistFields();
	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);
};

//----------------------------------------------------------------------------
// CameraGoalFollower
//
// This is the camera object (the entity through which the player usually views
// the world). It's designed to provide linear interpolation between different
// "goal" objects. A goal object can be any ShapeBase object. The
// CameraGoalFollower will "follow" the goal, matching it's camera transform. If
// desired, it can also provide smoothing (easing) on it's position, rotation or
// both. It does this by averaging the last X ticks. CameraGoalFollower is aware
// of the player and sometimes (like during interpolation) overrides it's own
// rotation (normally set by the goal object) to look at player.
//----------------------------------------------------------------------------
class CameraGoalFollower: public ShapeBase
{
private:
	typedef ShapeBase Parent;

	CameraGoalFollowerData*   mDataBlock;

	enum MaskBits {
         UpdateMask = Parent::NextFreeMask,
		 ClearMask = Parent::NextFreeMask << 1,
         NextFreeMask = Parent::NextFreeMask << 2
      };

	struct StateDelta {
		Point3F pos;
		Point3F rot;
		VectorF posVec;
		VectorF rotVec;
	};
	StateDelta delta;

	Point3F mRot;
	Point3F mPosition;

	Vector<Point3F> mGoalPosHistory;
	Vector<Point3F> mGoalRotHistory;

	void setPosition(const Point3F& pos,const Point3F& viewRot);
	void setRenderPosition(const Point3F& pos,const Point3F& viewRot);

	Player* mPlayerObject;	//the player object to track

	SimObjectPtr<ShapeBase> mGoalObject;			//achieve this objects position and rotation
	SimObjectPtr<ShapeBase> mGoalObjectPrev;		//previous goal object we are interpolating from
	F32 mInterpolationT;						//current interpolation value (0-1)
	S32 mInterpolationTime;						//total time of the interpolation (ms)



	void setPosition(const Point3F& pos,const Point3F& viewRot, MatrixF *mat);
	void setTransform(const MatrixF& mat);
	void setRenderTransform(const MatrixF& mat);

public:
	DECLARE_CONOBJECT(CameraGoalFollower);

	CameraGoalFollower();
	~CameraGoalFollower();

	void onEditorEnable();
	void onEditorDisable();

	bool onAdd();
	void onRemove();

	bool onNewDataBlock( GameBaseData *dptr, bool reload );
	void processTick(const Move* move);
	void interpolateTick(F32 delta);
	void getCameraTransform(F32* pos,MatrixF* mat);

	F32 getCameraFov();

	U32  packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn, BitStream *stream);

	void onDeleteNotify(SimObject *obj);

	bool setGoalObject(ShapeBase *obj, S32 ms = 2000);
	ShapeBase * getGoalObject()      { return(mGoalObject); }

	bool setPlayerObject(Player *obj);
	Player * getPlayerObject()      { return(mPlayerObject); }
};

#endif
