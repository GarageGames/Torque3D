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

#include "platform/input/razerHydra/razerHydraData.h"
#include "platform/input/razerHydra/razerHydraUtil.h"

RazerHyrdaControllerData::RazerHyrdaControllerData()
{
   reset();
}

void RazerHyrdaControllerData::reset()
{
   mDataSet = false;

   mShoulder = false;
   mThumb = false;
   mStart = false;
   mButton1 = false;
   mButton2 = false;
   mButton3 = false;
   mButton4 = false;

   mIsDocked = false;
}

void RazerHyrdaControllerData::setData(const sixenseControllerData& data, const F32& maxAxisRadius)
{
   // Controller position
   RazerHydraUtil::convertPosition(data.pos, mRawPos[0], mRawPos[1], mRawPos[2]);
   mPos[0] = (S32)mFloor(mRawPos[0]);
   mPos[1] = (S32)mFloor(mRawPos[1]);
   mPos[2] = (S32)mFloor(mRawPos[2]);

   mPosPoint.set(mPos[0], mPos[1], mPos[2]);

   // Controller rotation
   RazerHydraUtil::convertRotation(data.rot_mat, mRot);
   mRotQuat.set(mRot);

   // Controller rotation as axis, but only if not docked
   if(!data.is_docked)
   {
      RazerHydraUtil::calculateAxisRotation(mRot, maxAxisRadius, mRotAxis);
   }
   else
   {
      mRotAxis.x = 0.0f;
      mRotAxis.y = 0.0f;
   }

   // Thumb stick
   mThumbStick[0] = data.joystick_x;
   mThumbStick[1] = data.joystick_y;

   // Trigger
   mTrigger = data.trigger;

   //Buttons
   mShoulder = data.buttons & SIXENSE_BUTTON_BUMPER;
   mThumb = data.buttons & SIXENSE_BUTTON_JOYSTICK;
   mStart = data.buttons & SIXENSE_BUTTON_START;
   mButton1 = data.buttons & SIXENSE_BUTTON_1;
   mButton2 = data.buttons & SIXENSE_BUTTON_2;
   mButton3 = data.buttons & SIXENSE_BUTTON_3;
   mButton4 = data.buttons & SIXENSE_BUTTON_4;

   // Other data
   mIsDocked = data.is_docked;

   // Store the current sequence number
   mSequenceNum = data.sequence_number;

   mDataSet = true;
}

U32 RazerHyrdaControllerData::compare(RazerHyrdaControllerData* other)
{
   S32 result = DIFF_NONE;

   // Check position
   if(mDataSet)
   {
      if(mPos[0] != other->mPos[0])
         result |= DIFF_POSX;

      if(mPos[1] != other->mPos[1])
         result |= DIFF_POSY;

      if(mPos[2] != other->mPos[2])
         result |= DIFF_POSZ;
   }
   else
   {
      result |= DIFF_POS;
   }

   // Check rotation
   if(mRotQuat != other->mRotQuat || !mDataSet)
   {
      result |= DIFF_ROT;
   }

   // Check rotation as axis
   if(mRotAxis.x != other->mRotAxis.x || !mDataSet)
   {
      result |= DIFF_ROTAXISX;
   }
   if(mRotAxis.y != other->mRotAxis.y || !mDataSet)
   {
      result |= DIFF_ROTAXISY;
   }

   // Check thumb stick
   if(mThumbStick[0] != other->mThumbStick[0] || !mDataSet)
   {
      result |= DIFF_AXISX;
   }
   if(mThumbStick[1] != other->mThumbStick[1] || !mDataSet)
   {
      result |= DIFF_AXISY;
   }

   // Check trigger
   if(mTrigger != other->mTrigger || !mDataSet)
   {
      result |= DIFF_TRIGGER;
   }

   // Check buttons
   if(mShoulder != other->mShoulder)
   {
      result |= DIFF_BUTTON_SHOULDER;
   }
   if(mThumb != other->mThumb)
   {
      result |= DIFF_BUTTON_THUMB;
   }
   if(mStart != other->mStart)
   {
      result |= DIFF_BUTTON_START;
   }
   if(mButton1 != other->mButton1)
   {
      result |= DIFF_BUTTON1;
   }
   if(mButton2 != other->mButton2)
   {
      result |= DIFF_BUTTON2;
   }
   if(mButton3 != other->mButton3)
   {
      result |= DIFF_BUTTON3;
   }
   if(mButton4 != other->mButton4)
   {
      result |= DIFF_BUTTON4;
   }

   return result;
}

U32 RazerHyrdaControllerData::compareMeta(RazerHyrdaControllerData* other)
{
   S32 result = DIFF_NONE;

   if(mIsDocked != other->mIsDocked || !mDataSet)
   {
      result |= METADIFF_DOCKED;
   }

   return result;
}
