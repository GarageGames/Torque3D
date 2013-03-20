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

#ifndef _LEAPMOTIONDATA_H_
#define _LEAPMOTIONDATA_H_

#include "console/consoleTypes.h"
#include "math/mMathFn.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "platform/input/leapMotion/leapMotionConstants.h"
#include "Leap.h"

struct LeapMotionDeviceData
{
   enum DataDifferences {
      DIFF_NONE            = 0,
      DIFF_HANDROTAXISX    = (1<<1),
      DIFF_HANDROTAXISY    = (1<<2),

      DIFF_HANDROTAXIS = (DIFF_HANDROTAXISX | DIFF_HANDROTAXISY),
   };

   enum MetaDataDifferences {
      METADIFF_NONE              = 0,
      METADIFF_FRAME_VALID_DATA  = (1<<0),
   };

protected:
   void processPersistentHands(const Leap::HandList& hands, bool keepPointableIndexPersistent, LeapMotionDeviceData* prevData);
   void processHands(const Leap::HandList& hands);
   void processHand(const Leap::Hand& hand, U32 handIndex, bool keepPointableIndexPersistent, LeapMotionDeviceData* prevData);

   void processPersistentHandPointables(const Leap::PointableList& pointables, U32 handIndex, LeapMotionDeviceData* prevData);
   void processHandPointables(const Leap::PointableList& pointables, U32 handIndex);
   void processHandPointable(const Leap::Pointable& pointable, U32 handIndex, U32 handPointableIndex);

public:
   bool mDataSet;

   // Frame Data Set
   bool mIsValid;
   bool mHasTrackingData;

   // Hand Data Set
   bool mHandValid[LeapMotionConstants::MaxHands];
   S32 mHandID[LeapMotionConstants::MaxHands];

   // Hand Position
   F32 mHandRawPos[LeapMotionConstants::MaxHands][3];
   S32 mHandPos[LeapMotionConstants::MaxHands][3];
   Point3F mHandPosPoint[LeapMotionConstants::MaxHands];

   // Hand Rotation
   MatrixF mHandRot[LeapMotionConstants::MaxHands];
   QuatF mHandRotQuat[LeapMotionConstants::MaxHands];

   // Hand rotation as axis x, y
   F32 mHandRotAxis[2];

   // Pointable Data Set
   bool mPointableValid[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];
   S32 mPointableID[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];
   F32 mPointableLength[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];
   F32 mPointableWidth[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];

   // Pointable Position
   F32 mPointableRawPos[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand][3];
   S32 mPointablePos[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand][3];
   Point3F mPointablePosPoint[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];

   // Pointable Rotation
   MatrixF mPointableRot[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];
   QuatF mPointableRotQuat[LeapMotionConstants::MaxHands][LeapMotionConstants::MaxPointablesPerHand];

   // Sequence number from device
   U64 mSequenceNum;

   LeapMotionDeviceData();

   /// Reset device data
   void reset();

   /// Set data based on Leap Motion device data
   void setData(const Leap::Frame& frame, LeapMotionDeviceData* prevData, bool keepHandIndexPersistent, bool keepPointableIndexPersistent, const F32& maxHandAxisRadius);

   /// Compare this data and given and return differences
   U32 compare(LeapMotionDeviceData* other);

   /// Compare meta data between this and given and return differences
   U32 compareMeta(LeapMotionDeviceData* other);
};

#endif   // _LEAPMOTIONDATA_H_
