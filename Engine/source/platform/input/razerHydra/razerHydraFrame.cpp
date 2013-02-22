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

#include "platform/input/razerHydra/razerHydraFrame.h"
#include "platform/input/razerHydra/razerHydraUtil.h"
#include "console/engineAPI.h"
#include "math/mAngAxis.h"
#include "math/mTransform.h"

U32 RazerHydraFrame::smNextInternalFrameId = 0;

IMPLEMENT_CONOBJECT(RazerHydraFrame);

RazerHydraFrame::RazerHydraFrame()
{
   clear();
}

RazerHydraFrame::~RazerHydraFrame()
{
   clear();

   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      mControllerData[i].mEnabled = false;
   }
}


void RazerHydraFrame::initPersistFields()
{
   Parent::initPersistFields();
}

bool RazerHydraFrame::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void RazerHydraFrame::onRemove()
{
   Parent::onRemove();
}

void RazerHydraFrame::clear()
{
   mFrameValid = false;
}

void RazerHydraFrame::copyFromFrame(const sixenseAllControllerData& frame, const F32& maxAxisRadius)
{
   clear();

   // Calling this method automatically makes this a valid frame
   mFrameValid = true;

   mFrameInternalId = smNextInternalFrameId;
   ++smNextInternalFrameId;
   mFrameSimTime = Sim::getCurrentTime();
   mFrameRealTime = Platform::getRealMilliseconds();

   // Process the controllers
   for(U32 i=0; i<RazerHydraConstants::MaxControllers; ++i)
   {
      const sixenseControllerData& controller = frame.controllers[i];
      ControllerData& data = mControllerData[i];

      // General controller data
      data.mEnabled = controller.enabled;
      data.mIsDocked = controller.is_docked;
      data.mSequenceNum = controller.sequence_number;

      // Controller position
      RazerHydraUtil::convertPosition(controller.pos, data.mRawPos);
      data.mPos.x = (S32)mFloor(data.mRawPos.x);
      data.mPos.y = (S32)mFloor(data.mRawPos.y);
      data.mPos.z = (S32)mFloor(data.mRawPos.z);

      // Controller rotation
      RazerHydraUtil::convertRotation(controller.rot_mat, data.mRot);
      data.mRotQuat.set(data.mRot);

      // Controller as axis rotation
      RazerHydraUtil::calculateAxisRotation(data.mRot, maxAxisRadius, data.mRotAxis);

      // Controller thumb stick
      data.mThumbStick.x = controller.joystick_x;
      data.mThumbStick.y = controller.joystick_y;

      // Trigger
      data.mTrigger = controller.trigger;

      // Controller buttons
      data.mShoulder = controller.buttons & SIXENSE_BUTTON_BUMPER;
      data.mThumb = controller.buttons & SIXENSE_BUTTON_JOYSTICK;
      data.mStart = controller.buttons & SIXENSE_BUTTON_START;
      data.mButton1 = controller.buttons & SIXENSE_BUTTON_1;
      data.mButton2 = controller.buttons & SIXENSE_BUTTON_2;
      data.mButton3 = controller.buttons & SIXENSE_BUTTON_3;
      data.mButton4 = controller.buttons & SIXENSE_BUTTON_4;
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( RazerHydraFrame, isFrameValid, bool, ( ),,
   "@brief Checks if this frame is valid.\n\n"
   "@return True if the frame is valid.\n\n")
{
   return object->isFrameValid();
}

DefineEngineMethod( RazerHydraFrame, getFrameInternalId, S32, ( ),,
   "@brief Provides the internal ID for this frame.\n\n"
   "@return Internal ID of this frame.\n\n")
{
   return object->getFrameInternalId();
}

DefineEngineMethod( RazerHydraFrame, getFrameSimTime, S32, ( ),,
   "@brief Get the sim time that this frame was generated.\n\n"
   "@return Sim time of this frame in milliseconds.\n\n")
{
   return object->getFrameSimTime();
}

DefineEngineMethod( RazerHydraFrame, getFrameRealTime, S32, ( ),,
   "@brief Get the real time that this frame was generated.\n\n"
   "@return Real time of this frame in milliseconds.\n\n")
{
   return object->getFrameRealTime();
}

DefineEngineMethod( RazerHydraFrame, getControllerCount, S32, ( ),,
   "@brief Get the number of controllers defined in this frame.\n\n"
   "@return The number of defined controllers.\n\n")
{
   return RazerHydraConstants::MaxControllers;
}

DefineEngineMethod( RazerHydraFrame, getControllerEnabled, bool, ( S32 index ),,
   "@brief Get the enabled state of the controller.\n\n"
   "@param index The controller index to check.\n"
   "@return True if the requested controller is enabled.\n\n")
{
   return object->getEnabled(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerDocked, bool, ( S32 index ),,
   "@brief Get the docked state of the controller.\n\n"
   "@param index The controller index to check.\n"
   "@return True if the requested controller is docked.\n\n")
{
   return object->getDocked(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerSequenceNum, S32, ( S32 index ),,
   "@brief Get the controller sequence number.\n\n"
   "@param index The controller index to check.\n"
   "@return The sequence number of the requested controller.\n\n")
{
   return object->getSequenceNum(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerRawPos, Point3F, ( S32 index ),,
   "@brief Get the raw position of the requested controller.\n\n"
   "The raw position is the controller's floating point position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The controller index to check.\n"
   "@return Raw position of the requested controller (in millimeters).\n\n")
{
   return object->getRawPos(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerPos, Point3I, ( S32 index ),,
   "@brief Get the position of the requested controller.\n\n"
   "The position is the controller's integer position converted to "
   "Torque 3D coordinates (in millimeters).\n"
   "@param index The controller index to check.\n"
   "@return Integer position of the requested controller (in millimeters).\n\n")
{
   return object->getPos(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerRot, AngAxisF, ( S32 index ),,
   "@brief Get the rotation of the requested controller.\n\n"
   "The Razer Hydra controller rotation as converted into the Torque 3D"
   "coordinate system.\n"
   "@param index The controller index to check.\n"
   "@return Rotation of the requested controller.\n\n")
{
   AngAxisF aa(object->getRot(index));
   return aa;
}

DefineEngineMethod( RazerHydraFrame, getControllerRawTransform, TransformF, ( S32 index ),,
   "@brief Get the raw transform of the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return The raw position and rotation of the requested controller (in Torque 3D coordinates).\n\n")
{
   const Point3F& pos = object->getRawPos(index);
   const QuatF& qa = object->getRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans(pos, aa);
   return trans;
}

DefineEngineMethod( RazerHydraFrame, getControllerTransform, TransformF, ( S32 index ),,
   "@brief Get the transform of the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return The position and rotation of the requested controller (in Torque 3D coordinates).\n\n")
{
   const Point3I& pos = object->getPos(index);
   const QuatF& qa = object->getRotQuat(index);

   AngAxisF aa(qa);
   aa.axis.normalize();

   TransformF trans;
   trans.mPosition = Point3F(pos.x, pos.y, pos.z);
   trans.mOrientation = aa;

   return trans;
}

DefineEngineMethod( RazerHydraFrame, getControllerRotAxis, Point2F, ( S32 index ),,
   "@brief Get the axis rotation of the requested controller.\n\n"
   "This is the axis rotation of the controller as if the controller were a gamepad thumb stick.  "
   "Imagine a stick coming out the top of the controller and tilting the controller front, back, "
   "left and right controls that stick.  The values returned along the x and y stick "
   "axis are normalized from -1.0 to 1.0 with the maximum controller tilt angle for these "
   "values as defined by $RazerHydra::MaximumAxisAngle.\n"
   "@param index The controller index to check.\n"
   "@return Axis rotation of the requested controller.\n\n"
   "@see RazerHydra::MaximumAxisAngle\n")
{
   return object->getRotAxis(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerThumbStick, Point2F, ( S32 index ),,
   "@brief Get the thumb stick values of the requested controller.\n\n"
   "The thumb stick values are in the range of -1.0..1.0\n"
   "@param index The controller index to check.\n"
   "@return Thumb stick values of the requested controller.\n\n")
{
   return object->getThumbStick(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerTrigger, F32, ( S32 index ),,
   "@brief Get the trigger value for the requested controller.\n\n"
   "The trigger value is in the range of -1.0..1.0\n"
   "@param index The controller index to check.\n"
   "@return Trigger value of the requested controller.\n\n")
{
   return object->getTrigger(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerShoulderButton, bool, ( S32 index ),,
   "@brief Get the shoulder button state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Shoulder button state requested controller as true or false.\n\n")
{
   return object->getShoulder(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerThumbButton, bool, ( S32 index ),,
   "@brief Get the thumb button state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Thumb button state requested controller as true or false.\n\n")
{
   return object->getThumb(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerStartButton, bool, ( S32 index ),,
   "@brief Get the start button state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Start button state requested controller as true or false.\n\n")
{
   return object->getStart(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerButton1, bool, ( S32 index ),,
   "@brief Get the button 1 state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Button 1 state requested controller as true or false.\n\n")
{
   return object->getButton1(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerButton2, bool, ( S32 index ),,
   "@brief Get the button 2 state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Button 2 state requested controller as true or false.\n\n")
{
   return object->getButton2(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerButton3, bool, ( S32 index ),,
   "@brief Get the button 3 state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Button 3 state requested controller as true or false.\n\n")
{
   return object->getButton3(index);
}

DefineEngineMethod( RazerHydraFrame, getControllerButton4, bool, ( S32 index ),,
   "@brief Get the button 4 state for the requested controller.\n\n"
   "@param index The controller index to check.\n"
   "@return Button 4 state requested controller as true or false.\n\n")
{
   return object->getButton4(index);
}
