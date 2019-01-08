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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/actionMap.h"

#include "sdlInputManager.h"

typedef SDL_JoystickType SDLJoystickType;
DefineEnumType(SDLJoystickType);
ImplementEnumType(SDLJoystickType,
   "The type of device connected.\n\n"
   "@ingroup Input")
{ SDL_JOYSTICK_TYPE_UNKNOWN, "Unknown"},
{ SDL_JOYSTICK_TYPE_GAMECONTROLLER, "Game Controller" },
{ SDL_JOYSTICK_TYPE_WHEEL, "Wheel" },
{ SDL_JOYSTICK_TYPE_ARCADE_STICK, "Arcade Stick" },
{ SDL_JOYSTICK_TYPE_FLIGHT_STICK, "Flight Stick" },
{ SDL_JOYSTICK_TYPE_DANCE_PAD, "Dance Pad" },
{ SDL_JOYSTICK_TYPE_GUITAR, "Guitar" },
{ SDL_JOYSTICK_TYPE_DRUM_KIT, "Drum Kit" },
{ SDL_JOYSTICK_TYPE_ARCADE_PAD, "Arcade Pad" },
{ SDL_JOYSTICK_TYPE_THROTTLE, "Throttle" },
EndImplementEnumType;

typedef SDL_JoystickPowerLevel SDLPowerEnum;
DefineEnumType(SDLPowerEnum);
ImplementEnumType(SDLPowerEnum,
   "An enumeration of battery levels of a joystick.\n\n"
   "@ingroup Input")
{ SDL_JOYSTICK_POWER_UNKNOWN, "Unknown" },
{ SDL_JOYSTICK_POWER_EMPTY, "Empty" },
{ SDL_JOYSTICK_POWER_LOW, "Low" },
{ SDL_JOYSTICK_POWER_MEDIUM, "Medium" },
{ SDL_JOYSTICK_POWER_FULL, "Full" },
{ SDL_JOYSTICK_POWER_WIRED, "Wired" },
{ SDL_JOYSTICK_POWER_MAX, "Max" },
EndImplementEnumType;

IMPLEMENT_STATIC_CLASS(SDLInputManager, ,
   "@brief Static class exposing the SDL_Joystick and SDL_GameController APIs to Torque Script.\n"
   "SDLInputManager provides access to the functions of these APIs through static class "
   "functions.These functions are not required to bind or process events.By setting "
   "pref::Input::JoystickEnabled or pref::Input::sdlControllerEnabled to true, all connected "
   "devices will automatically be opened.All of the joystick and controller events defined "
   "in event.h can then be bound. For complete API documentation see the Joystick and Game "
   "Controller section of https ://wiki.libsdl.org/APIByCategory#Input_Events.\n\n"

   "@tsexample\n"
   "// Get the name and device type for all connected devices\n"
   "%sdlDevices = SDLInputManager::numJoysticks();\n"
   "for (%i = 0; %i < %sdlDevices; %i++)\n"
   "{\n"
   "   %deviceName = SDLInputManager::JoystickNameForIndex(%i);\n"
   "   %deviceType = SDLInputManager::GetDeviceType(%i);\n"
   "}\n"
   "\n"
   "// List all installed controller mappings\n"
   "%numMappings = SDLInputManager::GameControllerNumMappings();\n"
   "for (%i = 0; %i < %numMappings; %i++)\n"
   "   echo(SDLInputManager::GameControllerMappingForIndex(%i));\n"
   "@endtsexample\n\n");

IMPLEMENT_GLOBAL_CALLBACK(onSDLDeviceConnected, void, (S32 sdlIndex, const char* deviceName, const char* deviceType),
(sdlIndex, deviceName, deviceType),
"Callback that occurs when an input device is connected to the system.\n\n"
"@param sdlIndex The index that will be used by sdl to refer to the device.\n"
"@param deviceName The name that the device reports. This will be the return "
"value of SDL_JoystickNameForIndex or SDL_GameControllerNameForIndex depending on the device type.\n"
"@param deviceType The type of device connected. See SDLInputManager::getDeviceType() "
"for possible string values.\n\n");

IMPLEMENT_GLOBAL_CALLBACK(onSDLDeviceDisconnected, void, (S32 sdlIndex), (sdlIndex),
"Callback that occurs when an input device is disconnected from the system.\n\n"
"@param sdlIndex The index of the device that was removed.\n");

//------------------------------------------------------------------------------
// Static class variables:
bool SDLInputManager::smJoystickEnabled = true;
bool SDLInputManager::smJoystickSplitAxesLR = true;
bool SDLInputManager::smControllerEnabled = true;
bool SDLInputManager::smPOVButtonEvents = true;
bool SDLInputManager::smPOVMaskEvents = false;

// Map SDL controller Axis to torque input event
// Commented text from map_StringForControllerAxis[] in SDL_gamecontroller.c
S32 SDLInputManager::map_EventForControllerAxis[] = {
   SI_XAXIS, //"leftx",
   SI_YAXIS, //"lefty",
   SI_RXAXIS, //"rightx",
   SI_RYAXIS, //"righty",
   SI_ZAXIS, //"lefttrigger",
   SI_RZAXIS, //"righttrigger",
   -1 // NULL
};

// Map SDL controller button ID to torque input event
// Commented text from map_StringForControllerButton[] in SDL_gamecontroller.c
S32 SDLInputManager::map_EventForControllerButton[] = {
   XI_A, //"a",
   XI_B, //"b",
   XI_X, //"x",
   XI_Y, //"y",
   XI_BACK, //"back",
   XI_GUIDE, //"guide",
   XI_START, //"start",
   XI_LEFT_THUMB, //"leftstick",
   XI_RIGHT_THUMB, //"rightstick",
   XI_LEFT_SHOULDER, //"leftshoulder",
   XI_RIGHT_SHOULDER, //"rightshoulder",
   SI_UPOV, //"dpup",
   SI_DPOV, //"dpdown",
   SI_LPOV, //"dpleft",
   SI_RPOV, //"dpright",
   -1 // NULL
};

//------------------------------------------------------------------------------
void SDLInputManager::joystickState::reset()
{
   sdlInstID = -1;
   inputDevice = NULL;
   numAxes = 0;
   lastHatState[0] = 0;
   lastHatState[1] = 0;
}

//------------------------------------------------------------------------------
SDLInputManager::SDLInputManager()
{
   mEnabled = true;
   mJoystickActive = true;

   for (S32 i = 0; i < MaxJoysticks; ++i)
   {
      mJoysticks[i].reset();
      mJoysticks[i].torqueInstID = i;
   }

   for (S32 i = 0; i < MaxControllers; ++i)
   {
      mControllers[i].sdlInstID = -1;
      mControllers[i].torqueInstID = i;
      mControllers[i].inputDevice = NULL;
   }
}

//------------------------------------------------------------------------------
void SDLInputManager::init()
{
   Con::addVariable( "pref::Input::JoystickEnabled",  TypeBool, &smJoystickEnabled, 
      "If true, Joystick devices will be automatically opened.\n\n"
	   "@ingroup Input");
   Con::addVariable("pref::Input::JoystickSplitAxesLR", TypeBool, &smJoystickSplitAxesLR,
      "Split axis inputs on 4 axis joysticks. This has no effect on any other device.\n\n"
      "4 Axis joysticks use IDs 0-3 which get mapped to xaxis, yaxis, zaxis and rxaxis. "
      "When true, this will increment IDs 2 and 3 so the inputs map to xaxis, yaxis, rxaxis and ryaxis.\n"
      "@ingroup Input");
   Con::addVariable("pref::Input::sdlControllerEnabled", TypeBool, &smControllerEnabled,
      "If true, any Joystick device that SDL recognizes as a Game Controller will be automatically opened as a game controller.\n\n"
      "@ingroup Input");
   Con::addVariable("pref::Input::JoystickPOVButtons", TypeBool, &smPOVButtonEvents,
      "If true, the pov hat will be treated as 4 buttons and make/break events will be generated for "
      "upov, dpov, lpov and rpov.\n"
      "@ingroup Input");
   Con::addVariable("pref::Input::JoystickPOVMask", TypeBool, &smPOVMaskEvents,
      "If true, the pov hat will be treated as a single input with a 4 bit mask value. The povmask "
      "event will be generated with the current mask every time the mask value changes.\n"
      "@ingroup Input");

   // POV Hat mask bits
   Con::setIntVariable("$SDLMask::HatUp", SDL_HAT_UP);
   Con::setIntVariable("$SDLMask::HatRight", SDL_HAT_RIGHT);
   Con::setIntVariable("$SDLMask::HatDown", SDL_HAT_DOWN);
   Con::setIntVariable("$SDLMask::HatLeft", SDL_HAT_LEFT);
}

//------------------------------------------------------------------------------
bool SDLInputManager::enable()
{
   disable();

   if (smControllerEnabled || smJoystickEnabled)
   {
      for (S32 i = 0; i < SDL_NumJoysticks(); ++i)
      {
         if (smControllerEnabled && SDL_IsGameController(i))
            openController(i, 0);
         else if (smJoystickEnabled)
            openJoystick(i, 0);
      }
   }
   mEnabled = true;
   return true;
}

//------------------------------------------------------------------------------
void SDLInputManager::disable()
{
   // Close any open devices
   for (S32 i = 0; i < MaxControllers; ++i)
      closeControllerByIndex(i);
   for (S32 i = 0; i < MaxJoysticks; ++i)
      closeJoystickByIndex(i);

   mEnabled = false;
}

//------------------------------------------------------------------------------
void SDLInputManager::process()
{
}

//------------------------------------------------------------------------------
void SDLInputManager::processEvent(SDL_Event &evt)
{
   switch (evt.type)
   {
   case SDL_JOYAXISMOTION:
   {
      joystickState* torqueMapping;
      if (mJoystickMap.isEmpty() || !mJoystickMap.find(evt.jaxis.which, torqueMapping))
         break;
      // SDL axis value inputs are in (range: -32768 to 32767)
      // Torque axis values are -1.0 to 1.0
      F32 value = ((F32)evt.jaxis.value) / (F32) (evt.jaxis.value > 0 ? SDL_JOYSTICK_AXIS_MAX : -SDL_JOYSTICK_AXIS_MIN);
      S32 mapAxis = SI_XAXIS + evt.jaxis.axis;
      if (evt.jaxis.axis > 1 && torqueMapping->numAxes == 4 && smJoystickSplitAxesLR)
         mapAxis += 1; // On a 4 axis, we'll shift the second two so we use LX LY RX RY instead of LX LY LZ RX
      buildInputEvent(JoystickDeviceType, torqueMapping->torqueInstID, SI_AXIS, mapAxis, SI_MOVE, value);
      break;
   }

   case SDL_JOYBALLMOTION:
   {
      joystickState* torqueMapping;
      if (mJoystickMap.isEmpty() || !mJoystickMap.find(evt.jball.which, torqueMapping) || evt.jball.ball >= MaxBalls)
         break;
      if (evt.jball.xrel != 0)
         buildInputEvent(JoystickDeviceType, torqueMapping->torqueInstID, SI_INT, evt.jball.ball ? SI_XBALL2 : SI_XBALL, SI_MOVE, (S32) evt.jball.xrel);
      if (evt.jball.yrel != 0)
         buildInputEvent(JoystickDeviceType, torqueMapping->torqueInstID, SI_INT, evt.jball.ball ? SI_YBALL2 : SI_YBALL, SI_MOVE, (S32) evt.jball.yrel);
      break;
   }

   case SDL_JOYHATMOTION:
   {
      joystickState* torqueMapping;
      if (mJoystickMap.isEmpty() || !mJoystickMap.find(evt.jball.which, torqueMapping) || evt.jhat.hat >= MaxHats)
         break;
      if (torqueMapping->lastHatState[evt.jhat.hat] != evt.jhat.value)
      {
         buildHatEvents(JoystickDeviceType, torqueMapping->torqueInstID, torqueMapping->lastHatState[evt.jhat.hat], evt.jhat.value, evt.jhat.hat);
         torqueMapping->lastHatState[evt.jhat.hat] = evt.jhat.value;
      }
      break;
   }

   case SDL_JOYBUTTONDOWN:
   case SDL_JOYBUTTONUP:
   {
      joystickState* torqueMapping;
      if (mJoystickMap.isEmpty() || !mJoystickMap.find(evt.jbutton.which, torqueMapping))
         break;
      buildInputEvent(JoystickDeviceType, torqueMapping->torqueInstID, SI_BUTTON, KEY_BUTTON0 + evt.jbutton.button,
         evt.cbutton.state == SDL_PRESSED ? SI_MAKE : SI_BREAK, evt.cbutton.state == SDL_PRESSED ? 1.0f : 0.0f);
      break;
   }

   case SDL_JOYDEVICEADDED:
   {
      deviceConnectedCallback(evt.jdevice.which);
      if (smControllerEnabled && SDL_IsGameController(evt.jdevice.which))
         break;   // This device will be added as a controller

      if (smJoystickEnabled)
         openJoystick(evt.jdevice.which, 0);
      break;
   }

   case SDL_JOYDEVICEREMOVED:
   {
      onSDLDeviceDisconnected_callback(evt.jdevice.which);
      closeJoystick(evt.jdevice.which);
   }

   case SDL_CONTROLLERAXISMOTION:
   {
      controllerState* torqueMapping;
      if (mControllerMap.isEmpty() || !mControllerMap.find(evt.caxis.which, torqueMapping))
         break;
      // SDL axis value inputs are in (range: -32768 to 32767)
      // Torque axis values are -1.0 to 1.0
      F32 value = ((F32)evt.caxis.value) / (F32) (evt.caxis.value > 0 ? SDL_JOYSTICK_AXIS_MAX : -SDL_JOYSTICK_AXIS_MIN);
      buildInputEvent(GamepadDeviceType, torqueMapping->torqueInstID, SI_AXIS, map_EventForControllerAxis[evt.caxis.axis], SI_MOVE, value);
      break;
   }

   case SDL_CONTROLLERBUTTONDOWN:
   case SDL_CONTROLLERBUTTONUP:
   {
      controllerState* torqueMapping;
      if (mControllerMap.isEmpty() || !mControllerMap.find(evt.cbutton.which, torqueMapping))
         break;
      buildInputEvent(GamepadDeviceType, torqueMapping->torqueInstID, SI_BUTTON, map_EventForControllerButton[evt.cbutton.button],
         evt.cbutton.state == SDL_PRESSED ? SI_MAKE : SI_BREAK, evt.cbutton.state == SDL_PRESSED ? 1.0f : 0.0f);
      break;
   }

   case SDL_CONTROLLERDEVICEADDED:
   {
      if (smControllerEnabled)
         openController(evt.cdevice.which, 0);
      break;
   }

   case SDL_CONTROLLERDEVICEREMOVED:
   {
      closeController(evt.cdevice.which);
      break;
   }

   case SDL_CONTROLLERDEVICEREMAPPED:
      break;

   default:
#ifdef TORQUE_DEBUG
      Con::warnf("Unhandled SDL input event: 0x%04x", evt.type);
#endif
      break;
   }
}

//------------------------------------------------------------------------------
void SDLInputManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, S32 iValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType = deviceType;
   newEvent.deviceInst = deviceInst;
   newEvent.objType = objType;
   newEvent.objInst = objInst;
   newEvent.action = action;
   newEvent.iValue = iValue;

   newEvent.postToSignal(Input::smInputEvent);
}

//------------------------------------------------------------------------------
void SDLInputManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, F32 fValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType = deviceType;
   newEvent.deviceInst = deviceInst;
   newEvent.objType = objType;
   newEvent.objInst = objInst;
   newEvent.action = action;
   newEvent.fValue = fValue;

   newEvent.postToSignal(Input::smInputEvent);
}

//------------------------------------------------------------------------------
void SDLInputManager::buildHatEvents(U32 deviceType, U32 deviceInst, U8 lastState, U8 currentState, S32 hatIndex)
{
   if (smPOVButtonEvents)
   {
      if ((lastState & SDL_HAT_UP) != (currentState & SDL_HAT_UP))
      {
         buildInputEvent(deviceType, deviceInst, SI_POV, hatIndex ? SI_UPOV2 : SI_UPOV,
            (currentState & SDL_HAT_UP) ? SI_MAKE : SI_BREAK, (currentState & SDL_HAT_UP) ? 1.0f : 0.0f);
      }

      if ((lastState & SDL_HAT_DOWN) != (currentState & SDL_HAT_DOWN))
      {
         buildInputEvent(deviceType, deviceInst, SI_POV, hatIndex ? SI_DPOV2 : SI_DPOV,
            (currentState & SDL_HAT_DOWN) ? SI_MAKE : SI_BREAK, (currentState & SDL_HAT_DOWN) ? 1.0f : 0.0f);
      }

      if ((lastState & SDL_HAT_LEFT) != (currentState & SDL_HAT_LEFT))
      {
         buildInputEvent(deviceType, deviceInst, SI_POV, hatIndex ? SI_LPOV2 : SI_LPOV,
            (currentState & SDL_HAT_LEFT) ? SI_MAKE : SI_BREAK, (currentState & SDL_HAT_LEFT) ? 1.0f : 0.0f);
      }

      if ((lastState & SDL_HAT_RIGHT) != (currentState & SDL_HAT_RIGHT))
      {
         buildInputEvent(deviceType, deviceInst, SI_POV, hatIndex ? SI_RPOV2 : SI_RPOV,
            (currentState & SDL_HAT_RIGHT) ? SI_MAKE : SI_BREAK, (currentState & SDL_HAT_RIGHT) ? 1.0f : 0.0f);
      }
   }

   if (smPOVMaskEvents)
   {
      buildInputEvent(deviceType, deviceInst, SI_INT, hatIndex ? SI_POVMASK2 : SI_POVMASK, SI_VALUE, (S32) currentState);
   }
}

//------------------------------------------------------------------------------
S32 SDLInputManager::openController(S32 sdlIndex, S32 requestedTID)
{
   if ((sdlIndex < 0) || (sdlIndex >= SDL_NumJoysticks()) || (requestedTID < 0) || (requestedTID >= MaxControllers))
      return -1;

   if (SDL_IsGameController(sdlIndex))
   {
      SDL_GameController *inputDevice = SDL_GameControllerOpen(sdlIndex);
      if (inputDevice)
      {
         SDL_JoystickID sdlId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(inputDevice));

         // See if the device is already open as a joystick
         for (S32 i = 0; i < MaxJoysticks; ++i)
         {
            if (mJoysticks[i].sdlInstID == sdlId)
            {
               if (!closeJoystickByIndex(i))
               {
                  SDL_GameControllerClose(inputDevice);
                  return -1;
               }
            }
         }

         controllerState* torqueMapping = NULL;
         if (mControllerMap.find(sdlId, torqueMapping))
         {
            if (torqueMapping->torqueInstID == (U32) requestedTID)
            {
               SDL_GameControllerClose(inputDevice);
               return requestedTID;  // Already open at the requested ID
            }
            closeControllerByIndex(torqueMapping->torqueInstID);
         }

         S32 gamepadSlot = -1;
         if (!mControllers[requestedTID].inputDevice)
            gamepadSlot = requestedTID;
         else
         {
            // Find the first available gamepad device slot
            for (S32 i = 0; i < MaxControllers; ++i)
            {
               if (!mControllers[i].inputDevice)
               {
                  gamepadSlot = i;
                  break;
               }
            }
         }

         if (gamepadSlot == -1)
         {
            Con::errorf("Unable to open Game Controller %s. Too many devices present.", SDL_GameControllerName(inputDevice));
            SDL_GameControllerClose(inputDevice);
            return -1;
         }

         mControllers[gamepadSlot].inputDevice = inputDevice;
         mControllers[gamepadSlot].sdlInstID = sdlId;
         mControllerMap.insertUnique(sdlId, &mControllers[gamepadSlot]);

         return gamepadSlot;
      }
   }
   return -1;
}

//------------------------------------------------------------------------------
void SDLInputManager::closeController(SDL_JoystickID sdlId)
{
   controllerState* torqueMapping = NULL;
   if (mControllerMap.find(sdlId, torqueMapping))
      closeControllerByIndex(torqueMapping->torqueInstID);
}

//------------------------------------------------------------------------------
bool SDLInputManager::closeControllerByIndex(S32 index)
{
   if (index < 0 || index >= MaxControllers)
      return false;

   if (mControllers[index].inputDevice && mControllers[index].sdlInstID != -1)
   {
      SDL_GameControllerClose(mControllers[index].inputDevice);
      mControllerMap.erase(mControllers[index].sdlInstID);
      mControllers[index].sdlInstID = -1;
      mControllers[index].inputDevice = NULL;
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
S32 SDLInputManager::openJoystick(S32 sdlIndex, S32 requestedTID)
{
   if ((sdlIndex < 0) || (sdlIndex >= SDL_NumJoysticks()) || (requestedTID < 0) || (requestedTID >= MaxJoysticks))
      return -1;

   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      SDL_JoystickID sdlId = SDL_JoystickInstanceID(inputDevice);

      // See if the device is already open as a controller
      for (S32 i = 0; i < MaxControllers; ++i)
      {
         if (mControllers[i].sdlInstID == sdlId)
         {
            if (!closeControllerByIndex(i))
            {
               SDL_JoystickClose(inputDevice);
               return -1;
            }
         }
      }

      joystickState* torqueMapping = NULL;
      if (mJoystickMap.find(sdlId, torqueMapping))
      {
         if (torqueMapping->torqueInstID == (U32) requestedTID)
         {
            SDL_JoystickClose(inputDevice);
            return requestedTID;  // Already open at the requested ID
         }
         closeJoystickByIndex(torqueMapping->torqueInstID);
      }

      S32 joystickSlot = -1;
      if (!mJoysticks[requestedTID].inputDevice)
         joystickSlot = requestedTID;
      else
      {
         // Find the first available joystick device slot
         for (S32 i = 0; i < MaxJoysticks; ++i)
         {
            if (!mJoysticks[i].inputDevice)
            {
               joystickSlot = i;
               break;
            }
         }
      }

      if (joystickSlot == -1)
      {
         Con::errorf("Unable to open Joystick %s. Too many devices present.", SDL_JoystickName(inputDevice));
         SDL_JoystickClose(inputDevice);
         return -1;
      }

      mJoysticks[joystickSlot].inputDevice = inputDevice;
      mJoysticks[joystickSlot].sdlInstID = sdlId;
      mJoysticks[joystickSlot].numAxes = SDL_JoystickNumAxes(inputDevice);
      mJoystickMap.insertUnique(sdlId, &mJoysticks[joystickSlot]);

      return joystickSlot;
   }
   return -1;
}

//------------------------------------------------------------------------------
void SDLInputManager::closeJoystick(SDL_JoystickID sdlId)
{
   joystickState* torqueMapping = NULL;
   if (mJoystickMap.find(sdlId, torqueMapping))
      closeJoystickByIndex(torqueMapping->torqueInstID);
}

//------------------------------------------------------------------------------
bool SDLInputManager::closeJoystickByIndex(S32 index)
{
   if (index < 0 || index >= MaxJoysticks)
      return false;

   if (mJoysticks[index].inputDevice && mJoysticks[index].sdlInstID != -1)
   {
      SDL_JoystickClose(mJoysticks[index].inputDevice);
      mJoystickMap.erase(mJoysticks[index].sdlInstID);
      mJoysticks[index].reset();
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
void SDLInputManager::closeDevice(S32 sdlIndex)
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return;

   SDL_JoystickID sdlId = -1;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      sdlId = SDL_JoystickInstanceID(inputDevice);
      SDL_JoystickClose(inputDevice);
   }

   if (sdlId < 0)
      return;

   for (S32 i = 0; i < MaxControllers; ++i)
   {
      if (mControllers[i].sdlInstID == sdlId)
      {
         closeControllerByIndex(i);
         return;
      }
   }

   for (S32 i = 0; i < MaxJoysticks; ++i)
   {
      if (mJoysticks[i].sdlInstID == sdlId)
      {
         closeJoystickByIndex(i);
         return;
      }
   }
}

//------------------------------------------------------------------------------
void SDLInputManager::deviceConnectedCallback(S32 index)
{
   // This will generate the script callback:
   // onSDLDeviceConnected(%sdlIndex, %isController, %deviceName)
   bool isController = SDL_IsGameController(index);
   const char *deviceName = isController ? SDL_GameControllerNameForIndex(index) : SDL_JoystickNameForIndex(index);
   SDL_JoystickType deviceType = SDL_JoystickGetDeviceType(index);
   onSDLDeviceConnected_callback(index, deviceName, castConsoleTypeToString(deviceType));
}

//------------------------------------------------------------------------------
// Console interface

//------------------------------------------------------------------------------
// Get the N'th SDL device state -1=doesn't exist, 0=closed, 1=open joystick, 2=open controller
S32 SDLInputManager::getJoystickOpenState(S32 sdlIndex)
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return -1;

   S32 currentState = 0;
   // We need to open the joystick to get the sdl instanceID
   // This will increase the refcount on the joystick if it was already open.
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      SDL_JoystickID sdlId = SDL_JoystickInstanceID(inputDevice);
      controllerState* controllerMapping = NULL;
      joystickState* joystickMapping = NULL;
      if (!mControllerMap.isEmpty() && mControllerMap.find(sdlId, controllerMapping))
         currentState = 2;
      else if (!mJoystickMap.isEmpty() && mJoystickMap.find(sdlId, joystickMapping))
         currentState = 1;

      // Close the joystick to return the refcount to the previouse state
      SDL_JoystickClose(inputDevice);
   }

   return currentState;
}

//------------------------------------------------------------------------------
// Fills in the torque device instance string from an sdl joystick index number
void SDLInputManager::getJoystickTorqueInst(S32 sdlIndex, char* instBuffer)
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return;

   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      SDL_JoystickID sdlId = SDL_JoystickInstanceID(inputDevice);
      controllerState* controllerMapping = NULL;
      joystickState* joystickMapping = NULL;
      if (!mControllerMap.isEmpty() && mControllerMap.find(sdlId, controllerMapping))
         ActionMap::getDeviceName(GamepadDeviceType, controllerMapping->torqueInstID, instBuffer);
      else if (!mJoystickMap.isEmpty() && mJoystickMap.find(sdlId, joystickMapping))
         ActionMap::getDeviceName(JoystickDeviceType, joystickMapping->torqueInstID, instBuffer);

      SDL_JoystickClose(inputDevice);
   }
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, numJoysticks, S32, (), ,
   "@brief Returns the number of currently connected joystick devices.\n\n"
   "Game Controllers are a sub-set of joysticks and are included in the joystick count. "
   "See https://wiki.libsdl.org/SDL_NumJoysticks for more details.\n"
   "@ingroup Input")
{
   return SDL_NumJoysticks();
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, getDeviceOpenState, S32, ( S32 sdlIndex ), ( 0 ),
   "@brief Used to determine the current state of the N'th item in the SDL device list.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return values:\n"
   "-1 if the device does not exist (invalid sdlIndex passed)\n"
   "0 The device is closed\n"
   "1 The device is open as a Joystick\n"
   "2 The device is open as a Game Controller\n"
   "@ingroup Input")
{
   SDLInputManager* mgr = dynamic_cast<SDLInputManager*>(Input::getManager());
   if (mgr && mgr->isEnabled())
      return mgr->getJoystickOpenState(sdlIndex);
   return -1;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, openAsJoystick, S32, ( S32 sdlIndex, S32 torqueInstId ), ( 0, 0 ),
   "@brief Used to open the device as a Joystick.\n\n"
   "If the device is currently open as a Game Controller, it will be closed and opened as "
   "a Joystick. If it is currently opened as a Joystick with a different T3D instance ID, "
   "it will be changed to the requested ID if that ID is available.\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@param  torqueInstId Is the requested T3D device instance ID. If there is already an open Joystick with "
   "the requested ID, The first available ID will be assigned.\n"
   "@return The T3D device instance ID assigned, or -1 if the device could not be opened.")
{
   SDLInputManager* mgr = dynamic_cast<SDLInputManager*>(Input::getManager());
   if (mgr && mgr->isEnabled())
      return mgr->openJoystick(sdlIndex, torqueInstId);
   return -1;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, openAsController, S32, (S32 sdlIndex, S32 torqueInstId), (0, 0),
   "@brief Used to open the device as a Game Controller.\n\n"
   "If the device is currently open as a Joystick, it will be closed and opened as "
   "a Game Controller. If it is currently opened as a Game Controller with a different "
   "T3D instance ID, it will be changed to the requested ID if that ID is available.\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@param  torqueInstId Is the requested T3D device instance ID. If there is already an "
   "open Game Controller with the requested ID, The first available ID will be assigned.\n"
   "@return The T3D device instance ID assigned, or -1 if the device could not be opened.")
{
   SDLInputManager* mgr = dynamic_cast<SDLInputManager*>(Input::getManager());
   if (mgr && mgr->isEnabled())
      return mgr->openController(sdlIndex, torqueInstId);
   return -1;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, closeDevice, void, (S32 sdlIndex), (0),
   "@brief Used to close the N'th item in the SDL device list.\n\n"
   "This will close a Joystick or Game Controller.\n"
   "@param sdlIndex The SDL index for this device.\n")
{
   SDLInputManager* mgr = dynamic_cast<SDLInputManager*>(Input::getManager());
   if (mgr && mgr->isEnabled())
      mgr->closeDevice(sdlIndex);
   return;
}


//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, getTorqueInstFromDevice, const char *, (S32 sdlIndex), (0),
   "@brief Gets the T3D instance identifier for an open SDL joystick.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the T3D instance ID used for mapping this device or Null if it does not exist.\n"
   "@ingroup Input")
{
   SDLInputManager* mgr = dynamic_cast<SDLInputManager*>(Input::getManager());
   if (mgr && mgr->isEnabled())
   {
      char* deviceInst = Con::getReturnBuffer(32);
      deviceInst[0] = '\0';
      mgr->getJoystickTorqueInst(sdlIndex, deviceInst);
      return deviceInst;
   }
   return NULL;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickNameForIndex, const char *, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickNameForIndex() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the name of the selected joystick or Null if it does not exist.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNameForIndex \n"
   "@ingroup Input")
{
   if (sdlIndex >= 0 && sdlIndex < SDL_NumJoysticks())
      return SDL_JoystickNameForIndex(sdlIndex);
   return NULL;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, ControllerNameForIndex, const char *, (S32 sdlIndex), (0),
   "@brief Exposes SDL_GameControllerNameForIndex() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the implementation dependent name for the game controller, "
   "or NULL if there is no name or the index is invalid.\n"
   "@see https://wiki.libsdl.org/SDL_GameControllerNameForIndex \n"
   "@ingroup Input")
{
   if (sdlIndex >= 0 && sdlIndex < SDL_NumJoysticks() || !SDL_IsGameController(sdlIndex))
      return SDL_GameControllerNameForIndex(sdlIndex);
   return NULL;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickGetGUID, const char *, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickGetDeviceGUID() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return GUID for the indexed device or Null if it does not exist.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetDeviceGUID \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return NULL;
      
   SDL_JoystickGUID guidVal = SDL_JoystickGetDeviceGUID(sdlIndex);
   char *guidStr = Con::getReturnBuffer(64);
   SDL_JoystickGetGUIDString(guidVal, guidStr, 64);

   return guidStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GetVendor, S32, (S32 sdlIndex), (0),
   "Gets the USB vendor ID of a joystick device, if available.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return The USB vendor ID. If the vendor ID isn't available this function returns 0.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetDeviceVendor \n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetVendor \n"
   "@see https://wiki.libsdl.org/SDL_GameControllerGetVendor \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   return (S32) SDL_JoystickGetDeviceVendor(sdlIndex);
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GetProduct, S32, (S32 sdlIndex), (0),
   "Gets the USB product ID of a joystick device, if available.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return The USB product ID. If the product ID isn't available this function returns 0.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetDeviceProduct \n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetProduct \n"
   "@see https://wiki.libsdl.org/SDL_GameControllerGetProduct \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   return (S32)SDL_JoystickGetDeviceProduct(sdlIndex);
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GetProductVersion, S32, (S32 sdlIndex), (0),
   "Gets the product version of a joystick device, if available.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return The product version. If the product version isn't available this function returns 0.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetDeviceProductVersion \n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetProductVersion \n"
   "@see https://wiki.libsdl.org/SDL_GameControllerGetProductVersion \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   return (S32)SDL_JoystickGetDeviceProductVersion(sdlIndex);
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GetDeviceType, SDLJoystickType, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickGetDeviceType() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return The type of device connected. Possible return strings are: \"Unknown\", "
   "\"Game Controller\", \"Wheel\", \"Arcade Stick\", \"Flight Stick\", \"Dance Pad\", "
   "\"Guitar\", \"Drum Kit\", \"Arcade Pad\" and \"Throttle\"\n"
   "@see https://wiki.libsdl.org/SDL_JoystickGetDeviceType \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return SDL_JOYSTICK_TYPE_UNKNOWN;

   return SDL_JoystickGetDeviceType(sdlIndex);
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickNumAxes, S32, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickNumAxes() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the number of axis controls/number of axes on success or zero on failure.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNumAxes \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   S32 numAxes = 0;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      numAxes = SDL_JoystickNumAxes(inputDevice);
      if (numAxes < 0)
      {
         Con::errorf("SDL Joystick error: %s", SDL_GetError());
         numAxes = 0;
      }

      SDL_JoystickClose(inputDevice);
   }

   return numAxes;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickNumBalls, S32, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickNumBalls() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the number of trackballs on success or zero on failure.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNumBalls \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   S32 numBalls = 0;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      numBalls = SDL_JoystickNumBalls(inputDevice);
      if (numBalls < 0)
      {
         Con::errorf("SDL Joystick error: %s", SDL_GetError());
         numBalls = 0;
      }

      SDL_JoystickClose(inputDevice);
   }

   return numBalls;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickNumButtons, S32, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickNumButtons() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the number of buttons on success or zero on failure.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNumButtons \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   S32 numButtons = 0;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      numButtons = SDL_JoystickNumButtons(inputDevice);
      if (numButtons < 0)
      {
         Con::errorf("SDL Joystick error: %s", SDL_GetError());
         numButtons = 0;
      }

      SDL_JoystickClose(inputDevice);
   }

   return numButtons;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickNumHats, S32, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickNumHats() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the number of POV hats on success or zero on failure.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNumHats \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return 0;

   S32 numHats = 0;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      numHats = SDL_JoystickNumHats(inputDevice);
      if (numHats < 0)
      {
         Con::errorf("SDL Joystick error: %s", SDL_GetError());
         numHats = 0;
      }

      SDL_JoystickClose(inputDevice);
   }

   return numHats;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, IsGameController, bool, (S32 sdlIndex), (0),
   "@brief Exposes SDL_IsGameController() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns true if the given joystick is supported by the game controller "
   "interface, false if it isn't or it's an invalid index.\n"
   "@see https://wiki.libsdl.org/SDL_IsGameController \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks() || !SDL_IsGameController(sdlIndex))
      return false;

   return true;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickIsHaptic, bool, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickIsHaptic() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns true if the joystick is haptic.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickIsHaptic \n"
   "@ingroup Input")
{
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return false;

   bool isHaptic = false;
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      isHaptic = (SDL_JoystickIsHaptic(inputDevice) == SDL_TRUE);
      SDL_JoystickClose(inputDevice);
   }

   return isHaptic;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickPowerLevel, SDLPowerEnum, (S32 sdlIndex), (0),
   "@brief Exposes SDL_JoystickCurrentPowerLevel() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns the current battery level or \"Wired\" if it's a connected device.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickCurrentPowerLevel \n"
   "@ingroup Input")
{
   SDL_JoystickPowerLevel powerLevel = SDL_JOYSTICK_POWER_UNKNOWN;
   if (sdlIndex >= 0 && sdlIndex < SDL_NumJoysticks())
   {
      SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
      if (inputDevice)
      {
         powerLevel = SDL_JoystickCurrentPowerLevel(inputDevice);
         SDL_JoystickClose(inputDevice);
      }
   }
   return powerLevel;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickGetSpecs, String, (S32 sdlIndex), (0),
   "@brief A convenience function to reurn all of the data for a Joystick/Game Controller "
   " packed as fields in a tab separated string.\n\n"
   "There is overhead involved in querying joystick data, especially if the device is not open. "
   "If more than one field is required, it is more efficient to call JoystickGetSpecs() and "
   "parse the data out of the return string than to call the console method for each.\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields().\n\n"
   "Field 0: Number of Axes\n"
   "      1: Number of Buttons\n"
   "      2: Number of POV Hats\n"
   "      3: Number of Trackballs\n"
   "      4: SDL_IsGameController() (Boolean)\n"
   "      5: SDL_JoystickIsHaptic() (Boolean)\n"
   "      6: Power Level (String)\n"
   "      7: Device Type (String)\n"
   "@ingroup Input")
{
   String specStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return specStr;

   bool isController = SDL_IsGameController(sdlIndex);
   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      SDL_JoystickPowerLevel powerLevel = SDL_JoystickCurrentPowerLevel(inputDevice);
      SDL_JoystickType deviceType = SDL_JoystickGetDeviceType(sdlIndex);

      specStr = String::ToString("%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t",
         SDL_JoystickNumAxes(inputDevice), SDL_JoystickNumButtons(inputDevice),
         SDL_JoystickNumHats(inputDevice), SDL_JoystickNumBalls(inputDevice),
         isController ? 1 : 0, (SDL_JoystickIsHaptic(inputDevice) == SDL_TRUE) ? 1 : 0,
         castConsoleTypeToString(powerLevel), castConsoleTypeToString(deviceType));
      SDL_JoystickClose(inputDevice);
   }

   return specStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickGetAxes, String, (S32 sdlIndex), (0),
   "@brief Gets the current value for all joystick axes.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields(). "
   "Each axis is one field, so a 4 axis device will have 4 fields.\n\n"
   "@ingroup Input")
{
   String axesStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return axesStr;

   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      S32 numAxes = SDL_JoystickNumAxes(inputDevice);
      for (S32 i = 0; i < numAxes; i++)
      {
         F32 axisVal = (F32) SDL_JoystickGetAxis(inputDevice, i);
         F32 value = axisVal / (F32)(axisVal > 0.0f ? SDL_JOYSTICK_AXIS_MAX : -SDL_JOYSTICK_AXIS_MIN);
         axesStr += String::ToString("%0.3f\t", value);
      }
      SDL_JoystickClose(inputDevice);
   }

   return axesStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickGetButtons, String, (S32 sdlIndex), (0),
   "@brief Gets the current value for all joystick buttons.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields(). "
   "Each button is one field. 0 - SDL_JoystickNumButtons() fields.\n\n"
   "@ingroup Input")
{
   String buttonStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return buttonStr;

   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      S32 numbuttons = SDL_JoystickNumButtons(inputDevice);
      for (S32 i = 0; i < numbuttons; i++)
      {
         buttonStr += String::ToString("%d\t", (S32) SDL_JoystickGetButton(inputDevice, i));
      }
      SDL_JoystickClose(inputDevice);
   }

   return buttonStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, JoystickGetHats, String, (S32 sdlIndex), (0),
   "@brief Gets the current value for all POV hats.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields(). "
   "Each hat is one field. 0 - SDL_JoystickNumHats() fields. The value is a 4 bit bitmask. "
   "If no bits are set, the hat is centered. Bit 0 is up, 1 is right, 2 is down and 3 is left.\n\n"
   "@ingroup Input")
{
   String hatStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return hatStr;

   SDL_Joystick *inputDevice = SDL_JoystickOpen(sdlIndex);
   if (inputDevice)
   {
      S32 numHats = SDL_JoystickNumHats(inputDevice);
      for (S32 i = 0; i < numHats; i++)
      {
         hatStr += String::ToString("%d\t", (S32)SDL_JoystickGetHat(inputDevice, i));
      }
      SDL_JoystickClose(inputDevice);
   }

   return hatStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, ControllerGetAxes, String, (S32 sdlIndex), (0),
   "@brief Gets the current value for all controller axes.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields(). "
   "Game controllers always have 6 axes in the following order: 0-LX, 1-LY, 2-RX, 3-RY, 4-LT, 5-RT.\n\n"
   "@ingroup Input")
{
   String axesStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return axesStr;

   bool isController = SDL_IsGameController(sdlIndex);
   if (!isController)
      return axesStr;

   SDL_GameController *inputDevice = SDL_GameControllerOpen(sdlIndex);
   if (inputDevice)
   {
      for (S32 i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; i++)
      {
         F32 axisVal = (F32)SDL_GameControllerGetAxis(inputDevice, (SDL_GameControllerAxis) i);
         F32 value = axisVal / (F32)(axisVal > 0.0f ? SDL_JOYSTICK_AXIS_MAX : -SDL_JOYSTICK_AXIS_MIN);
         axesStr += String::ToString("%0.3f\t", value);
      }
      SDL_GameControllerClose(inputDevice);
   }

   return axesStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, ControllerGetButtons, String, (S32 sdlIndex), (0),
   "@brief Gets the current value for all controller buttons.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return A tab separated string that can be parsed from script with getField()/getFields(). "
   "Game controllers always have 15 buttons in the following order: 0-A, 1-B, 2-X, 3-Y, 4-Back, "
   "5-Guide, 6-Start, 7-Left Stick, 8-Right Stick, 9-Left Shoulder, 10-Right Shoulder, "
   "11-DPad Up, 12-DPad Down, 13-DPad Left, 14-DPad Right.\n\n"
   "@ingroup Input")
{
   String buttonStr;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return buttonStr;

   bool isController = SDL_IsGameController(sdlIndex);
   if (!isController)
      return buttonStr;

   SDL_GameController *inputDevice = SDL_GameControllerOpen(sdlIndex);
   if (inputDevice)
   {
      for (S32 i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; i++)
      {
         buttonStr += String::ToString("%d\t", (S32)SDL_GameControllerGetButton(inputDevice, (SDL_GameControllerButton) i));
      }
      SDL_GameControllerClose(inputDevice);
   }

   return buttonStr;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerMapping, String, (S32 sdlIndex), (0),
   "@brief Exposes SDL_GameControllerMapping() to script.\n\n"
   "@param sdlIndex The SDL index for this device.\n"
   "@return Returns a string that has the controller's mapping or NULL if no mapping "
   "is available or it does not exist.\n"
   "@see https://wiki.libsdl.org/SDL_JoystickNameForIndex \n"
   "@ingroup Input")
{
   String mapping;
   if (sdlIndex < 0 || sdlIndex >= SDL_NumJoysticks())
      return mapping;

   SDL_GameController *inputDevice = SDL_GameControllerOpen(sdlIndex);
   if (inputDevice)
   {
      char* sdlStr = SDL_GameControllerMapping(inputDevice);
      if (sdlStr)
      {
         mapping = sdlStr;
         SDL_free(sdlStr);
      }
      else
         Con::errorf("SDL Joystick error: %s", SDL_GetError());

      SDL_GameControllerClose(inputDevice);
   }

   return mapping;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerMappingForGUID, String, (const char* guidStr), ,
   "@brief Exposes SDL_GameControllerMappingForGUID() to script.\n\n"
   "@param guidStr The GUID for which a mapping is desired.\n"
   "@return Returns a mapping string or NULL on error.\n"
   "@see https://wiki.libsdl.org/SDL_GameControllerMappingForGUID \n"
   "@ingroup Input")
{
   String mapping;
   SDL_JoystickGUID guid = SDL_JoystickGetGUIDFromString(guidStr);

   char* sdlStr = SDL_GameControllerMappingForGUID(guid);
   if (sdlStr)
   {
      mapping = sdlStr;
      SDL_free(sdlStr);
   }

   return mapping;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerAddMapping, S32, (const char* mappingString), ,
   "@brief Exposes SDL_GameControllerAddMapping() to script.\n\n"
   "Use this function to add support for controllers that SDL is unaware of or "
   "to cause an existing controller to have a different binding.\n"
   "@param mappingString The new mapping string to apply. Full details on the format of this "
   "string are available at the linked SDL wiki page.\n"
   "@return Returns 1 if a new mapping is added, 0 if an existing mapping is updated, -1 on error.\n"
   "@see https://wiki.libsdl.org/SDL_GameControllerAddMapping \n"
   "@ingroup Input")
{
   S32 retVal = SDL_GameControllerAddMapping(mappingString);
   if (retVal == -1)
      Con::errorf("SDL Joystick error: %s", SDL_GetError());

   return retVal;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerAddMappingsFromFile, S32, (const char* fileName), ,
   "@brief Exposes SDL_GameControllerAddMappingsFromFile() to script.\n\n"
   "Use this function to load a set of Game Controller mappings from a file, filtered by the "
   "current SDL_GetPlatform(). A community sourced database of controllers is available at "
   "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt \n"
   "@param fileName The file to load mappings from.\n"
   "@return Returns the number of mappings added or -1 on error.\n"
   "@see https://wiki.libsdl.org/SDL_GameControllerAddMappingsFromFile \n"
   "@ingroup Input")
{
   char torquePath[1024];
   Con::expandScriptFilename(torquePath, sizeof(torquePath), fileName);
   S32 retVal = SDL_GameControllerAddMappingsFromFile(torquePath);
   if (retVal == -1)
      Con::errorf("SDL Joystick error: %s", SDL_GetError());

   return retVal;
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerNumMappings, S32, (), ,
   "Get the number of mappings installed. Used with GameControllerMappingForIndex "
   "to iterate through all installed mappings.\n\n"
   "@ingroup Input")
{
   return SDL_GameControllerNumMappings();
}

//------------------------------------------------------------------------------
DefineEngineStaticMethod(SDLInputManager, GameControllerMappingForIndex, String, (S32 mappingIndex), ,
   "Get the mapping at a particular index.\n\n"
   "@param mappingIndex The index for which a mapping is desired.\n"
   "@return Returns a mapping string or NULL if the index is out of range.\n"
   "@ingroup Input")
{
   String mapping;
   char* sdlStr = SDL_GameControllerMappingForIndex(mappingIndex);

   if (sdlStr)
   {
      mapping = sdlStr;
      SDL_free(sdlStr);
   }

   return mapping;
}
