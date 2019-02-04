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

#ifndef _SDLINPUTMANAGER_H_
#define _SDLINPUTMANAGER_H_

#ifndef _PLATFORMINPUT_H_
#include "platform/platformInput.h"
#endif
#include "SDL.h"

//------------------------------------------------------------------------------
class SDLInputManager : public InputManager
{
   enum Constants {
      MaxJoysticks = 4,          // Up to 4 simultaneous joysticks
      MaxControllers = 4,        // Up to 4 simultaneous controllers
      MaxHats = 2,               // Maximum 2 hats per device
      MaxBalls = 2,              // Maximum 2 trackballs per device
      MaxControllerAxes = 7,     // From map_StringForControllerAxis[] in SDL_gamecontroller.c
      MaxControllerButtons = 16  // From map_StringForControllerButton[] in SDL_gamecontroller.c
   };

   struct controllerState
   {
      S32 sdlInstID;    // SDL device instance id
      U32 torqueInstID; // Torque device instance id
      SDL_GameController *inputDevice;
   };

   struct joystickState
   {
      S32 sdlInstID;    // SDL device instance id
      U32 torqueInstID; // Torque device instance id
      SDL_Joystick *inputDevice;
      U32 numAxes;
      U8 lastHatState[MaxHats];

      void reset();
   };

private:
   typedef InputManager Parent;

   static S32 map_EventForControllerAxis[MaxControllerAxes];
   static S32 map_EventForControllerButton[MaxControllerButtons];

   static bool smJoystickEnabled;
   static bool smJoystickSplitAxesLR;
   static bool smControllerEnabled;
   static bool smPOVButtonEvents;
   static bool smPOVMaskEvents;

   joystickState mJoysticks[MaxJoysticks];
   controllerState mControllers[MaxControllers];

   // Used to look up a torque instance based on a device inst
   HashTable<S32, joystickState*> mJoystickMap;
   HashTable<S32, controllerState*> mControllerMap;

   bool mJoystickActive;

   void deviceConnectedCallback(S32 index);
   bool closeControllerByIndex(S32 index);
   void closeController(SDL_JoystickID sdlId);
   bool closeJoystickByIndex(S32 index);
   void closeJoystick(SDL_JoystickID sdlId);

   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, S32 iValue);
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, F32 fValue);
   void buildHatEvents(U32 deviceType, U32 deviceInst, U8 lastState, U8 currentState, S32 hatIndex);

public:
   DECLARE_STATIC_CLASS(SDLInputManager);

public:
   SDLInputManager();

   bool enable();
   void disable();
   void process();

   void processEvent(SDL_Event &evt);

   static void init();

   // Console interface:
   S32 openJoystick(S32 sdlIndex, S32 requestedTID);
   S32 openController(S32 sdlIndex, S32 requestedTID);
   void closeDevice(S32 sdlIndex);
   S32 getJoystickOpenState(S32 sdlIndex);
   void getJoystickTorqueInst(S32 sdlIndex, char* instBuffer);
};

#endif  // _SDLINPUTMANAGER_H_
