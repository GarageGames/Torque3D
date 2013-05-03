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

#ifndef _RAZERHYDRAFRAME_H_
#define _RAZERHYDRAFRAME_H_

#include "platform/input/razerHydra/razerHydraConstants.h"
#include "console/simObject.h"
#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "sixense.h"

class RazerHydraFrame : public SimObject
{
   typedef SimObject Parent;

protected:
   struct ControllerData
   {
      // Position
      Point3F mRawPos;
      Point3I mPos;

      // Rotation
      MatrixF mRot;
      QuatF   mRotQuat;

      // Controller rotation as axis x, y
      Point2F mRotAxis;

      // Thumb stick x, y and trigger
      Point2F mThumbStick;
      F32 mTrigger;

      // Buttons
      bool mShoulder;
      bool mThumb;
      bool mStart;
      bool mButton1;
      bool mButton2;
      bool mButton3;
      bool mButton4;

      // Other data
      U8 mSequenceNum;
      bool mEnabled;
      bool mIsDocked;
   };

   static U32 smNextInternalFrameId;

   // Sixense Frame
   bool  mFrameValid;

   // Torque 3D frame information
   U32   mFrameInternalId;
   S32   mFrameSimTime;
   S32   mFrameRealTime;

   // Controller data for the frame
   ControllerData mControllerData[RazerHydraConstants::MaxControllers];

public:
   RazerHydraFrame();
   virtual ~RazerHydraFrame();

   static void initPersistFields();

   virtual bool onAdd();
   virtual void onRemove();

   void clear();

   /// Copy a Leap Frame into our data structures
   void copyFromFrame(const sixenseAllControllerData& frame, const F32& maxAxisRadius);

   // Frame
   bool isFrameValid() const { return mFrameValid; }
   U32 getFrameInternalId() const { return mFrameInternalId; }
   S32 getFrameSimTime() const { return mFrameSimTime; }
   S32 getFrameRealTime() const { return mFrameRealTime; }

   // Controller
   const Point3F& getRawPos(U32 index) const;
   const Point3I& getPos(U32 index) const;
   const MatrixF& getRot(U32 index) const;
   const QuatF& getRotQuat(U32 index) const;
   const Point2F& getRotAxis(U32 index) const;

   // Controller variable controls
   const Point2F& getThumbStick(U32 Index) const;
   F32 getTrigger(U32 index) const;

   // Controller buttons
   bool getShoulder(U32 index) const;
   bool getThumb(U32 index) const;
   bool getStart(U32 index) const;
   bool getButton1(U32 index) const;
   bool getButton2(U32 index) const;
   bool getButton3(U32 index) const;
   bool getButton4(U32 index) const;

   // Controller other
   bool getEnabled(U32 index) const;
   bool getDocked(U32 index) const;
   S32 getSequenceNum(U32 index) const;

   DECLARE_CONOBJECT(RazerHydraFrame);
};

//-----------------------------------------------------------------------------

inline const Point3F& RazerHydraFrame::getRawPos(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? Point3F::Zero : mControllerData[index].mRawPos;
}

inline const Point3I& RazerHydraFrame::getPos(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? Point3I::Zero : mControllerData[index].mPos;
}

inline const MatrixF& RazerHydraFrame::getRot(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? MatrixF::Identity : mControllerData[index].mRot;
}

inline const QuatF& RazerHydraFrame::getRotQuat(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? QuatF::Identity : mControllerData[index].mRotQuat;
}

inline const Point2F& RazerHydraFrame::getRotAxis(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? Point2F::Zero : mControllerData[index].mRotAxis;
}

inline const Point2F& RazerHydraFrame::getThumbStick(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? Point2F::Zero : mControllerData[index].mThumbStick;
}

inline F32 RazerHydraFrame::getTrigger(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? 0.0f : mControllerData[index].mTrigger;
}

inline bool RazerHydraFrame::getShoulder(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mShoulder;
}

inline bool RazerHydraFrame::getThumb(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mThumb;
}

inline bool RazerHydraFrame::getStart(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mStart;
}

inline bool RazerHydraFrame::getButton1(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mButton1;
}

inline bool RazerHydraFrame::getButton2(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mButton2;
}

inline bool RazerHydraFrame::getButton3(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mButton3;
}

inline bool RazerHydraFrame::getButton4(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mButton4;
}

inline bool RazerHydraFrame::getEnabled(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mEnabled;
}

inline bool RazerHydraFrame::getDocked(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? false : mControllerData[index].mIsDocked;
}

inline S32 RazerHydraFrame::getSequenceNum(U32 index) const
{
   return (index >= RazerHydraConstants::MaxControllers) ? -1 : mControllerData[index].mSequenceNum;
}

#endif   // _RAZERHYDRAFRAME_H_
