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

#include "platform/input/leapMotion/leapMotionData.h"
#include "platform/input/leapMotion/leapMotionUtil.h"

LeapMotionDeviceData::LeapMotionDeviceData()
{
   reset();
}

void LeapMotionDeviceData::reset()
{
   mDataSet = false;

   mIsValid = false;

   mHasTrackingData = false;

   for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
   {
      mHandValid[i] = false;

      for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
      {
         mPointableValid[i][j] = false;
      }
   }
}

void LeapMotionDeviceData::setData(const Leap::Frame& frame, LeapMotionDeviceData* prevData, bool keepHandIndexPersistent, bool keepPointableIndexPersistent, const F32& maxHandAxisRadius)
{
   mIsValid = frame.isValid();
   if(!mIsValid)
      return;

   // Determine if there is any valid tracking data
   mHasTrackingData = frame.hands().count() > 0 || frame.pointables().count() > 0;

   const Leap::HandList hands = frame.hands();

   // Check if the hand index needs to persist between frames, but only if the
   // previous data is valid
   if(keepHandIndexPersistent && prevData && prevData->mDataSet && prevData->mIsValid)
   {
      processPersistentHands(frame.hands(), keepPointableIndexPersistent, prevData);
   }
   else
   {
      processHands(frame.hands());
   }

   // Single hand rotation as axis
   if(mHandValid[0])
   {
      Point2F axis;
      LeapMotionUtil::calculateHandAxisRotation(mHandRot[0], maxHandAxisRadius, axis);

      mHandRotAxis[0] = axis.x;
      mHandRotAxis[1] = axis.y;
   }
   else
   {
      // The first hand is not valid so we reset the axis rotation to none
      mHandRotAxis[0] = 0.0f;
      mHandRotAxis[1] = 0.0f;
   }

   // Store the current sequence number
   mSequenceNum = frame.id();

   mDataSet = true;
}

void LeapMotionDeviceData::processPersistentHands(const Leap::HandList& hands, bool keepPointableIndexPersistent, LeapMotionDeviceData* prevData)
{
   S32 numHands = hands.count();

   static S32 handDataIndex[LeapMotionConstants::MaxHands];
   static bool handIndexUsed[LeapMotionConstants::MaxHands];
   static Vector<S32> frameHandFound;

   // Clear out our lookup arrays
   for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
   {
      handDataIndex[i] = -1;
      handIndexUsed[i] = false;
   }
   frameHandFound.setSize(numHands);
   for(U32 i=0; i<numHands; ++i)
   {
      frameHandFound[i] = -1;
   }

   // Check if any hands this frame were picked up last frame
   for(U32 i=0; i<numHands; ++i)
   {
      const Leap::Hand& hand = hands[i];
      for(U32 j=0; j<LeapMotionConstants::MaxHands; ++j)
      {
         if(prevData && prevData->mHandValid[j] && hand.id() == prevData->mHandID[j])
         {
            handDataIndex[j] = i;
            frameHandFound[i] = j;
         }
      }
   }

   // Process all hands that were present in the last frame
   for(U32 i=0; i<numHands; ++i)
   {
      if(frameHandFound[i] != -1)
      {
         processHand(hands[i], frameHandFound[i], keepPointableIndexPersistent, prevData);
         handIndexUsed[frameHandFound[i]] = true;
      }
   }

   // Process all hands that were not present in the last frame
   for(U32 i=0; i<numHands; ++i)
   {
      if(frameHandFound[i] != -1)
         continue;

      // Find the first hand data that has not yet been used
      for(U32 j=0; j<LeapMotionConstants::MaxHands; ++j)
      {
         if(!handIndexUsed[j])
         {
            // Process this hand
            processHand(hands[i], j, keepPointableIndexPersistent, prevData);
            handIndexUsed[j] = true;
            break;
         }
      }
   }

   // Finally, mark all hand data that has not been processed this frame as invalid
   for(U32 i=0; i<LeapMotionConstants::MaxHands; ++i)
   {
      if(!handIndexUsed[i])
      {
         mHandValid[i] = false;
      }
   }
}

void LeapMotionDeviceData::processHands(const Leap::HandList& hands)
{
   S32 numHands = hands.count();

   // Process all valid hands
   S32 handsToProcess = getMin(numHands, LeapMotionConstants::MaxHands);
   for(U32 i=0; i<handsToProcess; ++i)
   {
      processHand(hands[i], i, false, NULL);
   }

   // Take care of any hands that do not exist this frame
   for(U32 i=handsToProcess; i<LeapMotionConstants::MaxHands; ++i)
   {
      mHandValid[i] = false;
   }
}

void LeapMotionDeviceData::processHand(const Leap::Hand& hand, U32 handIndex, bool keepPointableIndexPersistent, LeapMotionDeviceData* prevData)
{
   mHandValid[handIndex] = true;

   mHandID[handIndex] = hand.id();

   // Set the hand position
   LeapMotionUtil::convertPosition(hand.palmPosition(), mHandRawPos[handIndex][0], mHandRawPos[handIndex][1], mHandRawPos[handIndex][2]);
   mHandPos[handIndex][0] = (S32)mFloor(mHandRawPos[handIndex][0]);
   mHandPos[handIndex][1] = (S32)mFloor(mHandRawPos[handIndex][1]);
   mHandPos[handIndex][2] = (S32)mFloor(mHandRawPos[handIndex][2]);

   mHandPosPoint[handIndex].set(mHandPos[handIndex][0], mHandPos[handIndex][1], mHandPos[handIndex][2]);

   // Set the hand rotation
   LeapMotionUtil::convertHandRotation(hand, mHandRot[handIndex]);
   mHandRotQuat[handIndex].set(mHandRot[handIndex]);

   // Process the pointables associated with this hand
   if(keepPointableIndexPersistent)
   {
      processPersistentHandPointables(hand.pointables(), handIndex, prevData);
   }
   else
   {
      processHandPointables(hand.pointables(), handIndex);
   }
}

void LeapMotionDeviceData::processPersistentHandPointables(const Leap::PointableList& pointables, U32 handIndex, LeapMotionDeviceData* prevData)
{
   S32 numPointables = pointables.count();

   static S32 pointableDataIndex[LeapMotionConstants::MaxPointablesPerHand];
   static bool pointableIndexUsed[LeapMotionConstants::MaxPointablesPerHand];
   static Vector<S32> framePointableFound;

   // Clear out our lookup arrays
   for(U32 i=0; i<LeapMotionConstants::MaxPointablesPerHand; ++i)
   {
      pointableDataIndex[i] = -1;
      pointableIndexUsed[i] = false;
   }
   framePointableFound.setSize(numPointables);
   for(U32 i=0; i<numPointables; ++i)
   {
      framePointableFound[i] = -1;
   }

   // Check if any pointables for this hand during this frame were picked
   // up last frame
   for(U32 i=0; i<numPointables; ++i)
   {
      const Leap::Pointable& pointable = pointables[i];
      for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
      {
         if(prevData && prevData->mPointableValid[handIndex][j] && pointable.id() == prevData->mPointableID[handIndex][j])
         {
            pointableDataIndex[j] = i;
            framePointableFound[i] = j;
         }
      }
   }

   // Process all hand pointables that were present in the last frame
   for(U32 i=0; i<numPointables; ++i)
   {
      if(framePointableFound[i] != -1)
      {
         processHandPointable(pointables[i], handIndex, framePointableFound[i]);
         pointableIndexUsed[framePointableFound[i]] = true;
      }
   }

   // Process all hand pointables that were not present in the last frame
   for(U32 i=0; i<numPointables; ++i)
   {
      if(framePointableFound[i] != -1)
         continue;

      // Find the first hand pointable data that has not yet been used
      for(U32 j=0; j<LeapMotionConstants::MaxPointablesPerHand; ++j)
      {
         if(!pointableIndexUsed[j])
         {
            // Process the pointable
            processHandPointable(pointables[i], handIndex, j);
            pointableIndexUsed[j] = true;
            break;
         }
      }
   }

   // Finally, mark all hand pointable data that has not been process this frame as invalid
   for(U32 i=0; i<LeapMotionConstants::MaxPointablesPerHand; ++i)
   {
      if(!pointableIndexUsed[i])
      {
         mPointableValid[handIndex][i] = false;
      }
   }
}

void LeapMotionDeviceData::processHandPointables(const Leap::PointableList& pointables, U32 handIndex)
{
   // Process all pointables attached to the hand
   S32 numPointables = pointables.count();
   S32 pointablesToProcess = getMin(numPointables, LeapMotionConstants::MaxPointablesPerHand);
   for(U32 i=0; i<pointablesToProcess; ++i)
   {
      processHandPointable(pointables[i], handIndex, i);
   }

   // Take care of any pointables that do not exist this frame
   for(U32 i=pointablesToProcess; i<LeapMotionConstants::MaxPointablesPerHand; ++i)
   {
      mPointableValid[handIndex][i] = false;
   }
}

void LeapMotionDeviceData::processHandPointable(const Leap::Pointable& pointable, U32 handIndex, U32 handPointableIndex)
{
   mPointableValid[handIndex][handPointableIndex] = true;

   mPointableID[handIndex][handPointableIndex] = pointable.id();
   mPointableLength[handIndex][handPointableIndex] = pointable.length();
   mPointableWidth[handIndex][handPointableIndex] = pointable.width();

   // Set the pointable position
   LeapMotionUtil::convertPosition(pointable.tipPosition(), mPointableRawPos[handIndex][handPointableIndex][0], mPointableRawPos[handIndex][handPointableIndex][1], mPointableRawPos[handIndex][handPointableIndex][2]);
   mPointablePos[handIndex][handPointableIndex][0] = (S32)mFloor(mPointableRawPos[handIndex][handPointableIndex][0]);
   mPointablePos[handIndex][handPointableIndex][1] = (S32)mFloor(mPointableRawPos[handIndex][handPointableIndex][1]);
   mPointablePos[handIndex][handPointableIndex][2] = (S32)mFloor(mPointableRawPos[handIndex][handPointableIndex][2]);

   mPointablePosPoint[handIndex][handPointableIndex].set(mPointablePos[handIndex][handPointableIndex][0], mPointablePos[handIndex][handPointableIndex][1], mPointablePos[handIndex][handPointableIndex][2]);

   // Set the pointable rotation
   LeapMotionUtil::convertPointableRotation(pointable, mPointableRot[handIndex][handPointableIndex]);
   mPointableRotQuat[handIndex][handPointableIndex].set(mPointableRot[handIndex][handPointableIndex]);
}

U32 LeapMotionDeviceData::compare(LeapMotionDeviceData* other)
{
   S32 result = DIFF_NONE;

   // Check hand rotation as axis
   if(mHandRotAxis[0] != other->mHandRotAxis[0] || !mDataSet)
   {
      result |= DIFF_HANDROTAXISX;
   }
   if(mHandRotAxis[1] != other->mHandRotAxis[1] || !mDataSet)
   {
      result |= DIFF_HANDROTAXISY;
   }

   return result;
}

U32 LeapMotionDeviceData::compareMeta(LeapMotionDeviceData* other)
{
   S32 result = DIFF_NONE;

   if(mHasTrackingData != other->mHasTrackingData || !mDataSet)
   {
      result |= METADIFF_FRAME_VALID_DATA;
   }

   return result;
}
