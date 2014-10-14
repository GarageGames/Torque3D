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

#include "platform/input/leapMotion/leapMotionFrame.h"
#include "platform/input/leapMotion/leapMotionUtil.h"
#include "console/engineAPI.h"
#include "math/mAngAxis.h"
#include "math/mTransform.h"

U32 LeapMotionFrame::smNextInternalFrameId = 0;

IMPLEMENT_CONOBJECT(LeapMotionFrame);

ImplementEnumType( LeapMotionFramePointableType,
   "Leap Motion pointable type.\n\n")
   { LeapMotionFrame::PT_UNKNOWN,   "Unknown",  "Unknown pointable type.\n" },
   { LeapMotionFrame::PT_FINGER,    "Finger",   "Finger pointable type.\n" },
   { LeapMotionFrame::PT_TOOL,      "Tool",     "Tool pointable type.\n"  },
EndImplementEnumType;

LeapMotionFrame::LeapMotionFrame()
{
   clear();
}

LeapMotionFrame::~LeapMotionFrame()
{
   clear();
}


void LeapMotionFrame::initPersistFields()
{
   Parent::initPersistFields();
}

bool LeapMotionFrame::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void LeapMotionFrame::onRemove()
{
   Parent::onRemove();
}

void LeapMotionFrame::clear()
{
   mFrameValid = false;

   mHandCount = 0;
   mHandValid.clear();
   mHandId.clear();
   mHandRawPos.clear();
   mHandPos.clear();
   mHandRot.clear();
   mHandRotQuat.clear();
   mHandRotAxis.clear();
   mHandPointablesCount.clear();

   mPointableCount = 0;
   mPointableValid.clear();
   mPointableId.clear();
   mPointableHandIndex.clear();
   mPointableType.clear();
   mPointableRawPos.clear();
   mPointablePos.clear();
   mPointableRot.clear();
   mPointableRotQuat.clear();
   mPointableLength.clear();
   mPointableWidth.clear();
}

void LeapMotionFrame::copyFromFrame(const Leap::Frame& frame, const F32& maxHandAxisRadius)
{
   // This also resets all counters
   clear();

   // Retrieve frame information
   mFrameValid = frame.isValid();
   mFrameId = frame.id();
   mFrameTimeStamp = frame.timestamp();

   mFrameInternalId = smNextInternalFrameId;
   ++smNextInternalFrameId;
   mFrameSimTime = Sim::getCurrentTime();
   mFrameRealTime = Platform::getRealMilliseconds();

   if(!mFrameValid)
   {
      return;
   }

   // Retrieve hand information
   mHandCount = frame.hands().count();
   if(mHandCount > 0)
   {
      copyFromFrameHands(frame.hands(), maxHandAxisRadius);
   }

   // Retrieve pointable information
   mPointableCount = frame.pointables().count();
   if(mPointableCount > 0)
   {
      copyFromFramePointables(frame.pointables());
   }
}

void LeapMotionFrame::copyFromFrameHands(const Leap::HandList& hands, const F32& maxHandAxisRadius)
{
   // Set up Vectors
   mHandValid.increment(mHandCount);
   mHandId.increment(mHandCount);
   mHandRawPos.increment(mHandCount);
   mHandPos.increment(mHandCount);
   mHandRot.increment(mHandCount);
   mHandRotQuat.increment(mHandCount);
   mHandRotAxis.increment(mHandCount);
   mHandPointablesCount.increment(mHandCount);

   // Copy data
   for(U32 i=0; i<mHandCount; ++i)
   {
      const Leap::Hand& hand = hands[i];

      mHandValid[i] = hand.isValid();
      mHandId[i] = hand.id();
      
      // Position
      LeapMotionUtil::convertPosition(hand.palmPosition(), mHandRawPos[i]);
      mHandPos[i].x = (S32)mFloor(mHandRawPos[i].x);
      mHandPos[i].y = (S32)mFloor(mHandRawPos[i].y);
      mHandPos[i].z = (S32)mFloor(mHandRawPos[i].z);

      // Rotation
      LeapMotionUtil::convertHandRotation(hand, mHandRot[i]);
      mHandRotQuat[i].set(mHandRot[i]);

      // Thumb stick axis rotation
      LeapMotionUtil::calculateHandAxisRotation(mHandRot[i], maxHandAxisRadius, mHandRotAxis[i]);

      // Pointables
      mHandPointablesCount[i] = hand.pointables().count();
   }
}

void LeapMotionFrame::copyFromFramePointables(const Leap::PointableList& pointables)
{
   // Set up Vectors
   mPointableValid.increment(mPointableCount);
   mPointableId.increment(mPointableCount);
   mPointableHandIndex.increment(mPointableCount);
   mPointableType.increment(mPointableCount);
   mPointableRawPos.increment(mPointableCount);
   mPointablePos.increment(mPointableCount);
   mPointableRot.increment(mPointableCount);
   mPointableRotQuat.increment(mPointableCount);
   mPointableLength.increment(mPointableCount);
   mPointableWidth.increment(mPointableCount);

   // Copy data
   for(U32 i=0; i<mPointableCount; ++i)
   {
      const Leap::Pointable& pointable = pointables[i];

      mPointableValid[i] = pointable.isValid();
      mPointableId[i] = pointable.id();
      mPointableLength[i] = pointable.length();
      mPointableWidth[i] = pointable.width();

      mPointableType[i] = PT_UNKNOWN;
      if(pointable.isFinger())
      {
         mPointableType[i] = PT_FINGER;
      }
      else if(pointable.isTool())
      {
         mPointableType[i] = PT_TOOL;
      }

      // Which hand, if any
      const Leap::Hand& hand = pointable.hand();
      if(hand.isValid())
      {
         bool found = false;
         S32 handId = hand.id();
         for(U32 j=0; j<mHandCount; ++j)
         {
            if(mHandId[j] == handId)
            {
               mPointableHandIndex[i] = j;
               found = true;
               break;
            }
         }

         if(!found)
         {
            mPointableHandIndex[i] = -1;
         }
      }
      else
      {
         mPointableHandIndex[i] = -1;
      }

      // Position
      LeapMotionUtil::convertPosition(pointable.tipPosition(), mPointableRawPos[i]);
      mPointablePos[i].x = (S32)mFloor(mPointableRawPos[i].x);
      mPointablePos[i].y = (S32)mFloor(mPointableRawPos[i].y);
      mPointablePos[i].z = (S32)mFloor(mPointableRawPos[i].z);

      // Rotation
      LeapMotionUtil::convertPointableRotation(pointable, mPointableRot[i]);
      mPointableRotQuat[i].set(mPointableRot[i]);
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( LeapMotionFrame, isFrameValid, bool, ( ),,
   "@brief Checks if this frame is valid.\n\n"
   "@return True if the frame is valid.\n\n")
{
   return object->isFrameValid();
}

DefineEngineMethod( LeapMotionFrame, getFrameInternalId, S32, ( ),,
   "@brief Provides the internal ID for this frame.\n\n"
   "@return Internal ID of this frame.\n\n")
{
   return object->getFrameInternalId();
}

DefineEngineMethod( LeapMotionFrame, getFrameSimTime, S32, ( ),,
   "@brief Get the sim time that this frame was generated.\n\n"
   "@return Sim time of this frame in milliseconds.\n\n")
{
   return object->getFrameSimTime();
}

DefineEngineMethod( LeapMotionFrame, getFrameRealTime, S32, ( ),,
   "@brief Get the real time that this frame was generated.\n\n"
   "@return Real time of this frame in milliseconds.\n\n")
{
   return object->getFrameRealTime();
}

DefineEngineMethod( LeapMotionFrame, getHandCount, S32, ( ),,
   "@brief Get the number of hands defined in this frame.\n\n"
   "@return The number of defined hands.\n\n")
{
   return object->getHandCount();
}

DefineEngineMethod( LeapMotionFrame, getHandValid, bool, ( S32 index ),,
   "@brief Check if the requested hand is valid.\n\n"
   "@param index The hand index to check.\n"
   "@return True if the hand is valid.\n\n")
{
   return object->getHandValid(index);
}

DefineEngineMethod( LeapMotionFrame, getHandId, S32, ( S32 index ),,
   "@brief Get the ID of the requested hand.\n\n"
   "@param index The hand index to check.\n"
   "@return ID of the requested hand.\n\n")
{
   return object->getHandId(index);
}

DefineEngineMethod( LeapMotionFrame, getHandRawPos, Point3F, ( S32 index ),,
   "@brief Get the raw position of the requested hand.\n\n"
   "The raw position is the hand's floating point position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The hand index to check.\n"
   "@return Raw position of the requested hand.\n\n")
{
   return object->getHandRawPos(index);
}

DefineEngineMethod( LeapMotionFrame, getHandPos, Point3I, ( S32 index ),,
   "@brief Get the position of the requested hand.\n\n"
   "The position is the hand's integer position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The hand index to check.\n"
   "@return Integer position of the requested hand (in millimeters).\n\n")
{
   return object->getHandPos(index);
}

DefineEngineMethod( LeapMotionFrame, getHandRot, AngAxisF, ( S32 index ),,
   "@brief Get the rotation of the requested hand.\n\n"
   "The Leap Motion hand rotation as converted into the Torque 3D"
   "coordinate system.\n"
   "@param index The hand index to check.\n"
   "@return Rotation of the requested hand.\n\n")
{
   AngAxisF aa(object->getHandRot(index));
   return aa;
}

DefineEngineMethod( LeapMotionFrame, getHandRawTransform, TransformF, ( S32 index ),,
   "@brief Get the raw transform of the requested hand.\n\n"
   "@param index The hand index to check.\n"
   "@return The raw position and rotation of the requested hand (in Torque 3D coordinates).\n\n")
{
   const Point3F& pos = object->getHandRawPos(index);
   const QuatF& qa = object->getHandRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans(pos, aa);
   return trans;
}

DefineEngineMethod( LeapMotionFrame, getHandTransform, TransformF, ( S32 index ),,
   "@brief Get the transform of the requested hand.\n\n"
   "@param index The hand index to check.\n"
   "@return The position and rotation of the requested hand (in Torque 3D coordinates).\n\n")
{
   const Point3I& pos = object->getHandPos(index);
   const QuatF& qa = object->getHandRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans;
   trans.mPosition = Point3F(pos.x, pos.y, pos.z);
   trans.mOrientation = aa;

   return trans;
}

DefineEngineMethod( LeapMotionFrame, getHandRotAxis, Point2F, ( S32 index ),,
   "@brief Get the axis rotation of the requested hand.\n\n"
   "This is the axis rotation of the hand as if the hand were a gamepad thumb stick.  "
   "Imagine a stick coming out the top of the hand and tilting the hand front, back, "
   "left and right controls that stick.  The values returned along the x and y stick "
   "axis are normalized from -1.0 to 1.0 with the maximum hand tilt angle for these "
   "values as defined by $LeapMotion::MaximumHandAxisAngle.\n"
   "@param index The hand index to check.\n"
   "@return Axis rotation of the requested hand.\n\n"
   "@see LeapMotion::MaximumHandAxisAngle\n")
{
   return object->getHandRotAxis(index);
}

DefineEngineMethod( LeapMotionFrame, getHandPointablesCount, S32, ( S32 index ),,
   "@brief Get the number of pointables associated with this hand.\n\n"
   "@param index The hand index to check.\n"
   "@return Number of pointables that belong with this hand.\n\n")
{
   return object->getHandPointablesCount(index);
}

DefineEngineMethod( LeapMotionFrame, getPointablesCount, S32, ( ),,
   "@brief Get the number of pointables defined in this frame.\n\n"
   "@return The number of defined pointables.\n\n")
{
   return object->getPointablesCount();
}

DefineEngineMethod( LeapMotionFrame, getPointableValid, bool, ( S32 index ),,
   "@brief Check if the requested pointable is valid.\n\n"
   "@param index The pointable index to check.\n"
   "@return True if the pointable is valid.\n\n")
{
   return object->getPointableValid(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableId, S32, ( S32 index ),,
   "@brief Get the ID of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return ID of the requested pointable.\n\n")
{
   return object->getPointableId(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableHandIndex, S32, ( S32 index ),,
   "@brief Get the index of the hand that this pointable belongs to, if any.\n\n"
   "@param index The pointable index to check.\n"
   "@return Index of the hand this pointable belongs to, or -1 if there is no associated hand.\n\n")
{
   return object->getPointableHandIndex(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableType, LeapMotionFramePointableType, ( S32 index ),,
   "@brief Get the type of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return Type of the requested pointable.\n\n")
{
   return object->getPointableType(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableRawPos, Point3F, ( S32 index ),,
   "@brief Get the raw position of the requested pointable.\n\n"
   "The raw position is the pointable's floating point position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The pointable index to check.\n"
   "@return Raw position of the requested pointable.\n\n")
{
   return object->getPointableRawPos(index);
}

DefineEngineMethod( LeapMotionFrame, getPointablePos, Point3I, ( S32 index ),,
   "@brief Get the position of the requested pointable.\n\n"
   "The position is the pointable's integer position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The pointable index to check.\n"
   "@return Integer position of the requested pointable (in millimeters).\n\n")
{
   return object->getPointablePos(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableRot, AngAxisF, ( S32 index ),,
   "@brief Get the rotation of the requested pointable.\n\n"
   "The Leap Motion pointable rotation as converted into the Torque 3D"
   "coordinate system.\n"
   "@param index The pointable index to check.\n"
   "@return Rotation of the requested pointable.\n\n")
{
   AngAxisF aa(object->getPointableRot(index));
   return aa;
}

DefineEngineMethod( LeapMotionFrame, getPointableRawTransform, TransformF, ( S32 index ),,
   "@brief Get the raw transform of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return The raw position and rotation of the requested pointable (in Torque 3D coordinates).\n\n")
{
   const Point3F& pos = object->getPointableRawPos(index);
   const QuatF& qa = object->getPointableRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans(pos, aa);
   return trans;
}

DefineEngineMethod( LeapMotionFrame, getPointableTransform, TransformF, ( S32 index ),,
   "@brief Get the transform of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return The position and rotation of the requested pointable (in Torque 3D coordinates).\n\n")
{
   const Point3I& pos = object->getPointablePos(index);
   const QuatF& qa = object->getPointableRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans;
   trans.mPosition = Point3F(pos.x, pos.y, pos.z);
   trans.mOrientation = aa;

   return trans;
}

DefineEngineMethod( LeapMotionFrame, getPointableLength, F32, ( S32 index ),,
   "@brief Get the length of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return Length of the requested pointable (in millimeters).\n\n")
{
   return object->getPointableLength(index);
}

DefineEngineMethod( LeapMotionFrame, getPointableWidth, F32, ( S32 index ),,
   "@brief Get the width of the requested pointable.\n\n"
   "@param index The pointable index to check.\n"
   "@return Width of the requested pointable (in millimeters).\n\n")
{
   return object->getPointableWidth(index);
}
