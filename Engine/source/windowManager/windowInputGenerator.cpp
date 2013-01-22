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

#include "windowManager/windowInputGenerator.h"
#include "windowManager/platformWindow.h"
#include "sim/actionMap.h"
#include "component/interfaces/IProcessInput.h"


extern InputModifiers convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
WindowInputGenerator::WindowInputGenerator( PlatformWindow *window ) : 
                                             mWindow(window),
                                             mInputController(NULL),
                                             mLastCursorPos(0,0),
                                             mClampToWindow(true),
                                             mPixelsPerMickey(1.0f),
                                             mNotifyPosition(true),
                                             mFocused(false)
{
   AssertFatal(mWindow, "NULL PlatformWindow on WindowInputGenerator creation");

#ifdef TORQUE_OS_XENON
   mFocused = true;
#endif

   if (mWindow->getOffscreenRender())
      mFocused = true;

   mWindow->appEvent.notify(this, &WindowInputGenerator::handleAppEvent);
   mWindow->mouseEvent.notify(this, &WindowInputGenerator::handleMouseMove);
   mWindow->wheelEvent.notify(this, &WindowInputGenerator::handleMouseWheel);
   mWindow->buttonEvent.notify(this, &WindowInputGenerator::handleMouseButton);
   mWindow->keyEvent.notify(this, &WindowInputGenerator::handleKeyboard);
   mWindow->charEvent.notify(this, &WindowInputGenerator::handleCharInput);

   // We also want to subscribe to input events.
   Input::smInputEvent.notify(this, &WindowInputGenerator::handleInputEvent);
}

WindowInputGenerator::~WindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseEvent.remove(this, &WindowInputGenerator::handleMouseMove);
      mWindow->buttonEvent.remove(this, &WindowInputGenerator::handleMouseButton);
      mWindow->wheelEvent.remove(this, &WindowInputGenerator::handleMouseWheel);
      mWindow->keyEvent.remove(this, &WindowInputGenerator::handleKeyboard);
      mWindow->charEvent.remove(this, &WindowInputGenerator::handleCharInput);
      mWindow->appEvent.remove(this, &WindowInputGenerator::handleAppEvent);
   }

   Input::smInputEvent.remove(this, &WindowInputGenerator::handleInputEvent);
}

//-----------------------------------------------------------------------------
// Process an input event and pass it on.
// Respect the action map.
//-----------------------------------------------------------------------------
void WindowInputGenerator::generateInputEvent( InputEventInfo &inputEvent )
{
   if( !mInputController || !mFocused )
      return;

   // Give the ActionMap first shot.
   if (ActionMap::handleEventGlobal(&inputEvent))
      return;

   if( mInputController->processInputEvent( inputEvent ) )
      return;

   // If we get here we failed to process it with anything prior... so let
   // the ActionMap handle it.
   ActionMap::handleEvent(&inputEvent);

}

//-----------------------------------------------------------------------------
// Mouse Events
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleMouseMove( WindowId did, U32 modifier, S32 x, S32 y, bool isRelative )
{
   if( !mInputController || !mFocused )
      return;

   // jddTODO : Clean this up
   // CodeReview currently the Torque GuiCanvas deals with mouse input 
   //  as relative movement, even when the cursor is visible.  Because 
   //  of this there is an asinine bit of code in there that manages
   //  updating the cursor position on the class based on relative movement.
   //  Because of this we always have to generate and send off for processing
   //  relative events, even if the mouse is not locked.  
   //  I'm considering removing this in the Canvas refactor, thoughts? [7/6/2007 justind]

   // Generate a base Movement along and Axis event
   InputEventInfo event;
   event.deviceType = MouseDeviceType;
   event.deviceInst = 0;
   event.objType    = SI_AXIS;
   event.modifier   = convertModifierBits(modifier);
   event.ascii      = 0;

   // Generate delta movement along each axis
   Point2F cursDelta;
   if(isRelative)
   {
      cursDelta.x = F32(x) * mPixelsPerMickey;
      cursDelta.y = F32(y) * mPixelsPerMickey;
   }
   else
   {
      cursDelta.x = F32(x - mLastCursorPos.x);
      cursDelta.y = F32(y - mLastCursorPos.y);
   }

   // If X axis changed, generate a relative event
   if(mFabs(cursDelta.x) > 0.1)
   {
      event.objInst    = SI_XAXIS;
      event.action     = SI_MOVE;
      event.fValue     = cursDelta.x;
      generateInputEvent(event);
   }

   // If Y axis changed, generate a relative event
   if(mFabs(cursDelta.y) > 0.1)
   {
      event.objInst    = SI_YAXIS;
      event.action     = SI_MOVE;
      event.fValue     = cursDelta.y;
      generateInputEvent(event);
   }

   //  CodeReview : If we're not relative, pass along a positional update
   //  so that the canvas can update it's internal cursor tracking
   //  point. [7/6/2007 justind]
   if( !isRelative )
   {
      if( mClampToWindow )
      {
         Point2I winExtent = mWindow->getClientExtent();
         x = mClampF(x, 0.0f, F32(winExtent.x  - 1));
         y = mClampF(y, 0.0f, F32(winExtent.y  - 1));

      }

      // When the window gains focus, we send a cursor position event 
      if( mNotifyPosition )
      {
         mNotifyPosition = false;

         // We use SI_MAKE to signify that the position is being set, not relatively moved.
         event.action     = SI_MAKE;

         // X Axis
         event.objInst    = SI_XAXIS;
         event.fValue     = (F32)x;
         generateInputEvent(event);

         // Y Axis
         event.objInst = SI_YAXIS;
         event.fValue     = (F32)y;
         generateInputEvent(event);      
      }

      mLastCursorPos = Point2I(x,y);

   }
   else
   {   
      mLastCursorPos += Point2I(x,y);      
      mNotifyPosition = true;
   }
}

void WindowInputGenerator::handleMouseButton( WindowId did, U32 modifiers, U32 action, U16 button )
{
   if( !mInputController || !mFocused )
      return;

   InputEventInfo event;
   event.deviceType = MouseDeviceType;
   event.deviceInst = 0;
   event.objType    = SI_BUTTON;
   event.objInst    = (InputObjectInstances)(KEY_BUTTON0 + button);
   event.modifier   = convertModifierBits(modifiers);
   event.ascii      = 0;
   event.action     = (action==IA_MAKE) ? SI_MAKE : SI_BREAK;
   event.fValue     = (action==IA_MAKE) ? 1.0 : 0.0;

   generateInputEvent(event);
}

void WindowInputGenerator::handleMouseWheel( WindowId did, U32 modifiers, S32 wheelDeltaX, S32 wheelDeltaY )
{
   if( !mInputController || !mFocused )
      return;

   InputEventInfo event;
   event.deviceType = MouseDeviceType;
   event.deviceInst = 0;
   event.objType    = SI_AXIS;
   event.modifier   = convertModifierBits(modifiers);
   event.ascii      = 0;
   event.action     = SI_MOVE;

   if( wheelDeltaY ) // Vertical
   {
      event.objInst    = SI_ZAXIS;
      event.fValue     = (F32)wheelDeltaY;

      generateInputEvent(event);
   }
   if( wheelDeltaX ) // Horizontal
   {
      event.objInst    = SI_RZAXIS;
      event.fValue     = (F32)wheelDeltaX;

      generateInputEvent(event);
   }
}

//-----------------------------------------------------------------------------
// Key/Character Input
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleCharInput( WindowId did, U32 modifier, U16 key )
{
   if( !mInputController || !mFocused )
      return;

   InputEventInfo event;
   event.deviceType  = KeyboardDeviceType;
   event.deviceInst  = 0;
   event.objType     = SI_KEY;
   event.objInst     = KEY_NULL;
   event.modifier    = convertModifierBits(modifier);
   event.ascii       = key;
   event.action      = SI_MAKE;
   event.fValue      = 1.0;
   generateInputEvent(event);

   event.action = SI_BREAK;
   event.fValue = 0.f;
   generateInputEvent(event);
}


void WindowInputGenerator::handleKeyboard( WindowId did, U32 modifier, U32 action, U16 key )
{
   if( !mInputController || !mFocused )
      return;

   InputEventInfo event;
   event.deviceType  = KeyboardDeviceType;
   event.deviceInst  = 0;
   event.objType     = SI_KEY;
   event.objInst     = (InputObjectInstances)key;
   event.modifier    = convertModifierBits(modifier);
   event.ascii       = 0;

   switch(action)
   {
   case IA_MAKE:
      event.action = SI_MAKE;
      event.fValue = 1.f;
      break;

   case IA_REPEAT:
      event.action = SI_REPEAT;
      event.fValue = 1.f;
      break;

   case IA_BREAK:
      event.action = SI_BREAK;
      event.fValue = 0.f;
      break;

      // If we encounter an unknown don't submit the event.
   default:
      //Con::warnf("GuiCanvas::handleKeyboard - got an unknown action type %d!", action);
      return;
   }

   generateInputEvent(event);
}

//-----------------------------------------------------------------------------
// Raw input 
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleInputEvent( U32 deviceInst, F32 fValue, F32 fValue2, F32 fValue3, F32 fValue4, S32 iValue, U16 deviceType, U16 objType, U16 ascii, U16 objInst, U8 action, U8 modifier )
{
   // Skip it if we don't have focus.
   if(!mInputController || !mFocused)
      return;

   // Convert to an InputEventInfo and pass it around for processing.
   InputEventInfo event;
   event.deviceInst  = deviceInst;
   event.fValue      = fValue;
   event.fValue2     = fValue2;
   event.fValue3     = fValue3;
   event.fValue4     = fValue4;
   event.iValue      = iValue;
   event.deviceType  = (InputDeviceTypes)deviceType;
   event.objType     = (InputEventType)objType;
   event.ascii       = ascii;
   event.objInst     = (InputObjectInstances)objInst;
   event.action      = (InputActionType)action;
   event.modifier    = (InputModifiers)modifier;
   
   generateInputEvent(event);
}

//-----------------------------------------------------------------------------
// Window Events
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleAppEvent( WindowId did, S32 event )
{
   if(event == LoseFocus)
   {
      // Fire all breaks; this will prevent issues with dangling keys.
      ActionMap::clearAllBreaks();
      mFocused = false;
   }
   else if(event == GainFocus)
   {
      // Set an update flag to notify the consumer of the absolute mouse position next move
      mNotifyPosition = true;
      mFocused = true;
   }

   // always focused with offscreen rendering
   if (mWindow->getOffscreenRender())
      mFocused = true;
}

//-----------------------------------------------------------------------------
// Character Input Mapping
//-----------------------------------------------------------------------------

bool WindowInputGenerator::wantAsKeyboardEvent( U32 modifiers, U32 keyCode )
{
   // Disallow translation on keys that are bound in the global action map.
   
   return ActionMap::getGlobalMap()->isAction(
      KeyboardDeviceType,
      0,
      modifiers,
      keyCode
   );
}
