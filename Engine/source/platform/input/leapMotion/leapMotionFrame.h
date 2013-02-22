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
//-----------------------------------------------------------------------------

#ifndef _LEAPMOTIONFRAME_H_
#define _LEAPMOTIONFRAME_H_

#include "console/simObject.h"
#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "Leap.h"

class LeapMotionFrame : public SimObject
{
   typedef SimObject Parent;

public:
   enum PointableType
   {
      PT_UNKNOWN = -1,
      PT_FINGER = 0,
      PT_TOOL,
   };

protected:
   static U32 smNextInternalFrameId;

   // Frame
   bool  mFrameValid;
   U64   mFrameId;
   U64   mFrameTimeStamp;

   // Torque 3D frame information
   U32   mFrameInternalId;
   S32   mFrameSimTime;
   S32   mFrameRealTime;

   // Hands
   U32 mHandCount;
   Vector<bool>      mHandValid;
   Vector<S32>       mHandId;
   Vector<Point3F>   mHandRawPos;
   Vector<Point3I>   mHandPos;
   Vector<MatrixF>   mHandRot;
   Vector<QuatF>     mHandRotQuat;
   Vector<Point2F>   mHandRotAxis;
   Vector<U32>       mHandPointablesCount;

   // Pointables
   U32 mPointableCount;
   Vector<bool>      mPointableValid;
   Vector<S32>       mPointableId;
   Vector<S32>       mPointableHandIndex;
   Vector<PointableType>   mPointableType;
   Vector<Point3F>   mPointableRawPos;
   Vector<Point3I>   mPointablePos;
   Vector<MatrixF>   mPointableRot;
   Vector<QuatF>     mPointableRotQuat;
   Vector<F32>       mPointableLength;
   Vector<F32>       mPointableWidth;

protected:
   void copyFromFrameHands(const Leap::HandList& hands, const F32& maxHandAxisRadius);
   void copyFromFramePointables(const Leap::PointableList& pointables);

public:
   LeapMotionFrame();
   virtual ~LeapMotionFrame();

   static void initPersistFields();

   virtual bool onAdd();
   virtual void onRemove();

   void clear();

   /// Copy a Leap Frame into our data structures
   void copyFromFrame(const Leap::Frame& frame, const F32& maxHandAxisRadius);

   // Frame
   bool isFrameValid() const { return mFrameValid; }
   U32 getFrameInternalId() const { return mFrameInternalId; }
   S32 getFrameSimTime() const { return mFrameSimTime; }
   S32 getFrameRealTime() const { return mFrameRealTime; }

   // Hands
   U32 getHandCount() const { return mHandCount; }
   bool getHandValid(U32 index) const;
   S32 getHandId(U32 index) const;
   const Point3F& getHandRawPos(U32 index) const;
   const Point3I& getHandPos(U32 index) const;
   const MatrixF& getHandRot(U32 index) const;
   const QuatF& getHandRotQuat(U32 index) const;
   const Point2F& getHandRotAxis(U32 index) const;
   U32 getHandPointablesCount(U32 index) const;

   // Pointables
   U32 getPointablesCount() const { return mPointableCount; }
   bool getPointableValid(U32 index) const;
   S32 getPointableId(U32 index) const;
   S32 getPointableHandIndex(U32 index) const;
   PointableType getPointableType(U32 index) const;
   const Point3F& getPointableRawPos(U32 index) const;
   const Point3I& getPointablePos(U32 index) const;
   const MatrixF& getPointableRot(U32 index) const;
   const QuatF& getPointableRotQuat(U32 index) const;
   F32 getPointableLength(U32 index) const;
   F32 getPointableWidth(U32 index) const;

   DECLARE_CONOBJECT(LeapMotionFrame);
};

typedef LeapMotionFrame::PointableType LeapMotionFramePointableType;
DefineEnumType( LeapMotionFramePointableType );

//-----------------------------------------------------------------------------

inline bool LeapMotionFrame::getHandValid(U32 index) const
{
   return (index < mHandCount && mHandValid[index]);
}

inline S32 LeapMotionFrame::getHandId(U32 index) const
{
   return (index >= mHandCount) ? -1 : mHandId[index];
}

inline const Point3F& LeapMotionFrame::getHandRawPos(U32 index) const
{
   return (index >= mHandCount) ? Point3F::Zero : mHandRawPos[index];
}

inline const Point3I& LeapMotionFrame::getHandPos(U32 index) const
{
   return (index >= mHandCount) ? Point3I::Zero : mHandPos[index];
}

inline const MatrixF& LeapMotionFrame::getHandRot(U32 index) const
{
   return (index >= mHandCount) ? MatrixF::Identity : mHandRot[index];
}

inline const QuatF& LeapMotionFrame::getHandRotQuat(U32 index) const
{
   return (index >= mHandCount) ? QuatF::Identity : mHandRotQuat[index];
}

inline const Point2F& LeapMotionFrame::getHandRotAxis(U32 index) const
{
   return (index >= mHandCount) ? Point2F::Zero : mHandRotAxis[index];
}

inline U32 LeapMotionFrame::getHandPointablesCount(U32 index) const
{
   return (index >= mHandCount) ? 0 : mHandPointablesCount[index];
}

inline bool LeapMotionFrame::getPointableValid(U32 index) const
{
   return (index < mPointableCount && mPointableValid[index]);
}

inline S32 LeapMotionFrame::getPointableId(U32 index) const
{
   return (index >= mPointableCount) ? -1 : mPointableId[index];
}

inline S32 LeapMotionFrame::getPointableHandIndex(U32 index) const
{
   return (index >= mPointableCount) ? -1 : mPointableHandIndex[index];
}

inline LeapMotionFrame::PointableType LeapMotionFrame::getPointableType(U32 index) const
{
   return (index >= mPointableCount) ? PT_UNKNOWN : mPointableType[index];
}

inline const Point3F& LeapMotionFrame::getPointableRawPos(U32 index) const
{
   return (index >= mPointableCount) ? Point3F::Zero : mPointableRawPos[index];
}

inline const Point3I& LeapMotionFrame::getPointablePos(U32 index) const
{
   return (index >= mPointableCount) ? Point3I::Zero : mPointablePos[index];
}

inline const MatrixF& LeapMotionFrame::getPointableRot(U32 index) const
{
   return (index >= mPointableCount) ? MatrixF::Identity : mPointableRot[index];
}

inline const QuatF& LeapMotionFrame::getPointableRotQuat(U32 index) const
{
   return (index >= mPointableCount) ? QuatF::Identity : mPointableRotQuat[index];
}

inline F32 LeapMotionFrame::getPointableLength(U32 index) const
{
   return (index >= mPointableCount) ? 0.0f : mPointableLength[index];
}

inline F32 LeapMotionFrame::getPointableWidth(U32 index) const
{
   return (index >= mPointableCount) ? 0.0f : mPointableWidth[index];
}

#endif   // _LEAPMOTIONFRAME_H_
