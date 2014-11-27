//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
//
// Much of this code was taken directly from the PathShape
// resource: http://www.garagegames.com/community/resource/view/20385/1
// With additional improvements by Azaezel:
// http://www.garagegames.com/community/forums/viewthread/137195
//-----------------------------------------------------------------------------

#ifndef _TSPATHSHAPE_H_
#define _TSPATHSHAPE_H_

#ifndef _TSDYNAMIC_H_
#include "tsDynamic.h"
#endif

#ifndef _CAMERASPLINE_H_
#include "T3D/cameraSpline.h"
#endif

#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

/// A simple pathed shape with optional ambient animation.
class TSPathShape : public TSDynamic
{
private:
   typedef TSDynamic Parent;

public:
   /// The movement states
   enum MoveState {
      Forward,
      Backward,
      Stop
   };
   MoveState mState;

protected:
   enum MaskBits 
   {
      WindowMask                 = Parent::NextFreeMask << 0,
      PositionMask               = Parent::NextFreeMask << 1,
      TargetMask                 = Parent::NextFreeMask << 2,
      StateMask                  = Parent::NextFreeMask << 3,
      NextFreeMask               = Parent::NextFreeMask << 4
   };

private:
   struct StateDelta {
      F32 time;
      F32 timeVec;
      MoveState state;
   };
   StateDelta delta;

   enum Constants {
      NodeWindow = 128,    // Maximum number of active nodes
      MoveStateBits = 2    // 2 bits for 3 states
   };

   CameraSpline mSpline;
   S32 mNodeBase;
   S32 mNodeCount;
   F32 mPosition;
   F32 mTarget;
   bool mTargetSet;
   bool mLooping;

   void interpolateMat(F32 pos, MatrixF* mat);
   void advancePosition(S32 ms);

   // The client must be synched to the server so they both represent the shape at the same
   // location, but updating too frequently makes the shape jitter due to varying transmission
   // time. This sets the number of ticks between position updates.
   static U32 mUpdateTics;
   U32 mUpdateTickCount;

protected:

   bool onAdd();
   void onRemove();
   virtual void onStaticModified(const char* slotName, const char*newValue = NULL);

   // ProcessObject
   virtual void processTick( const Move *move );
   virtual void interpolateTick( F32 delta );

   virtual bool _getShouldTick();

public:

   TSPathShape();
   ~TSPathShape();

   DECLARE_CONOBJECT(TSPathShape);
   DECLARE_CALLBACK(void, onAdd, () );
   DECLARE_CALLBACK(void, onPathChange, () );
   DECLARE_CALLBACK(void, onNode, (S32 node));
   DECLARE_CALLBACK(void, onTargetReached, (F32 val));

   static void initPersistFields();

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   // Path setup
   SimObjectRef< SimPath::Path >  mPath;
   bool reset(F32 speed, bool makeKnot);
   void pushFront(CameraSpline::Knot *knot);
   void pushBack(CameraSpline::Knot *knot);
   void popFront();

   // Movement control
   void setPathPosition(F32 pos);
   F32 getPathPosition(void) { return mPosition; }
   void setTarget(F32 pos);
   void setMoveState(MoveState s);
   MoveState getMoveState() { return mState; }
   void setLooping(bool isLooping);
   bool getLooping() { return mLooping; }
   S32 getNodeCount() { return mNodeCount; }
};

typedef TSPathShape::MoveState PathShapeState;
DefineEnumType( PathShapeState );

#endif // _TSPATHSHAPE_H_

