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

#include "platformWin32/platformWin32.h"
#include "platformWin32/winDirectInput.h"
#include "platformWin32/winDInputDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/actionMap.h"

#include <xinput.h>

//------------------------------------------------------------------------------
// Static class variables:
bool DInputManager::smJoystickEnabled = true;
bool DInputManager::smXInputEnabled = true;

// Type definitions:
typedef HRESULT (WINAPI* FN_DirectInputCreate)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

//------------------------------------------------------------------------------
DInputManager::DInputManager()
{
   mEnabled          = false;
   mDInputLib        = NULL;
   mDInputInterface  = NULL;
   mJoystickActive = mXInputActive = true;
   mXInputLib        = NULL;

   for(S32 i=0; i<4; i++)
      mLastDisconnectTime[i] = -1;
}

//------------------------------------------------------------------------------
void DInputManager::init()
{
   Con::addVariable( "pref::Input::JoystickEnabled",  TypeBool, &smJoystickEnabled, 
      "@brief If true, the joystick is currently enabled.\n\n"
	   "@ingroup Game");
}

//------------------------------------------------------------------------------
bool DInputManager::enable()
{
   FN_DirectInputCreate fnDInputCreate;

   disable();

   // Dynamically load the XInput 9 DLL and cache function pointers to the
   // two APIs we use
#ifdef LOG_INPUT
   Input::log( "Enabling XInput...\n" );
#endif
   mXInputLib = LoadLibrary( dT("xinput9_1_0.dll") );
   if ( mXInputLib )
   {
      mfnXInputGetState = (FN_XInputGetState) GetProcAddress( mXInputLib, "XInputGetState" );
      mfnXInputSetState = (FN_XInputSetState) GetProcAddress( mXInputLib, "XInputSetState" );
      if ( mfnXInputGetState && mfnXInputSetState )
      {
#ifdef LOG_INPUT
         Input::log( "XInput detected.\n" );
#endif
         mXInputStateReset = true;
         mXInputDeadZoneOn = true;
         smXInputEnabled = true;
      }
   }
   else
   {
#ifdef LOG_INPUT
      Input::log( "XInput was not found.\n" );
      mXInputStateReset = false;
      mXInputDeadZoneOn = false;
#endif
   }

#ifdef LOG_INPUT
   Input::log( "Enabling DirectInput...\n" );
#endif
   mDInputLib = LoadLibrary( dT("DInput8.dll") );
   if ( mDInputLib )
   {
      fnDInputCreate = (FN_DirectInputCreate) GetProcAddress( mDInputLib, "DirectInput8Create" );
      if ( fnDInputCreate )
      {
         bool result = SUCCEEDED( fnDInputCreate( winState.appInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&mDInputInterface), NULL ));
         if ( result )
         {
#ifdef LOG_INPUT
            Input::log( "DirectX 8 or greater detected.\n" );
#endif
         }

         if ( result )
         {
            enumerateDevices();
            mEnabled = true;
            return true;
         }
      }
   }

   disable();

#ifdef LOG_INPUT
   Input::log( "Failed to enable DirectInput.\n" );
#endif

   return false;
}

//------------------------------------------------------------------------------

void DInputManager::disable()
{
   unacquire( SI_ANY, SI_ANY );

   DInputDevice* dptr;
   iterator ptr = begin();
   while ( ptr != end() )
   {
      dptr = dynamic_cast<DInputDevice*>( *ptr );
      if ( dptr )
      {
         removeObject( dptr );
         //if ( dptr->getManager() )
            //dptr->getManager()->unregisterObject( dptr );
         delete dptr;
         ptr = begin();
      }
      else
         ptr++;
   }

   if ( mDInputInterface )
   {
      mDInputInterface->Release();
      mDInputInterface = NULL;
   }

   if ( mDInputLib )
   {
      FreeLibrary( mDInputLib );
      mDInputLib = NULL;
   }

   if ( mfnXInputGetState )
   {
      mXInputStateReset = true;
      mfnXInputGetState = NULL;
      mfnXInputSetState = NULL;
   }

   if ( mXInputLib )
   {
      FreeLibrary( mXInputLib );
      mXInputLib = NULL;
   }

   mEnabled = false;
}

//------------------------------------------------------------------------------
void DInputManager::onDeleteNotify( SimObject* object )
{
   Parent::onDeleteNotify( object );
}

//------------------------------------------------------------------------------
bool DInputManager::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   acquire( SI_ANY, SI_ANY );
   return true;
}

//------------------------------------------------------------------------------
void DInputManager::onRemove()
{
   unacquire( SI_ANY, SI_ANY );
   Parent::onRemove();
}

//------------------------------------------------------------------------------
bool DInputManager::acquire( U8 deviceType, U8 deviceID )
{
   bool anyActive = false;
   DInputDevice* dptr;
   for ( iterator ptr = begin(); ptr != end(); ptr++ )
   {
      dptr = dynamic_cast<DInputDevice*>( *ptr );
      if ( dptr
        && ( ( deviceType == SI_ANY ) || ( dptr->getDeviceType() == deviceType ) )
        && ( ( deviceID == SI_ANY ) || ( dptr->getDeviceID() == deviceID ) ) )
      {
         if ( dptr->acquire() )
            anyActive = true;
      }
   }

   return anyActive;
}

//------------------------------------------------------------------------------
void DInputManager::unacquire( U8 deviceType, U8 deviceID )
{
   DInputDevice* dptr;
   for ( iterator ptr = begin(); ptr != end(); ptr++ )
   {
      dptr = dynamic_cast<DInputDevice*>( *ptr );
      if ( dptr
        && ( ( deviceType == SI_ANY ) || ( dptr->getDeviceType() == deviceType ) )
        && ( ( deviceID == SI_ANY ) || ( dptr->getDeviceID() == deviceID ) ) )
         dptr->unacquire();
   }
}

//------------------------------------------------------------------------------
void DInputManager::process()
{
   // Because the XInput APIs manage everything for all four controllers trivially,
   // we don't need the abstraction of a DInputDevice for an unknown number of devices
   // with varying buttons & whistles... nice and easy!
   if ( isXInputActive() )
      processXInput();

   DInputDevice* dptr;
   for ( iterator ptr = begin(); ptr != end(); ptr++ )
   {
      dptr = dynamic_cast<DInputDevice*>( *ptr );
      if ( dptr )
         dptr->process();
   }
}

//------------------------------------------------------------------------------
void DInputManager::enumerateDevices()
{
   if ( mDInputInterface )
   {
#ifdef LOG_INPUT
      Input::log( "Enumerating input devices...\n" );
#endif

      DInputDevice::init();
      DInputDevice::smDInputInterface = mDInputInterface;
      mDInputInterface->EnumDevices( DI8DEVTYPE_KEYBOARD, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
      mDInputInterface->EnumDevices( DI8DEVTYPE_MOUSE,    EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
      mDInputInterface->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
   }
}

//------------------------------------------------------------------------------
BOOL CALLBACK DInputManager::EnumDevicesProc( const DIDEVICEINSTANCE* pddi, LPVOID pvRef )
{
   DInputManager* manager = (DInputManager*) pvRef;
   DInputDevice* newDevice = new DInputDevice( pddi );
   manager->addObject( newDevice );
   if ( !newDevice->create() )
   {
      manager->removeObject( newDevice );
      delete newDevice;
   }

   return (DIENUM_CONTINUE);
}

//------------------------------------------------------------------------------
bool DInputManager::enableJoystick()
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( !mgr || !mgr->isEnabled() )
      return( false );

   if ( smJoystickEnabled && mgr->isJoystickActive() )
      return( true );

   smJoystickEnabled = true;
   if ( Input::isActive() )
      smJoystickEnabled = mgr->activateJoystick();

   if ( smJoystickEnabled )
   {
      Con::printf( "DirectInput joystick enabled." );
#ifdef LOG_INPUT
      Input::log( "Joystick enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "DirectInput joystick failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Joystick failed to enable!\n" );
#endif
   }

   return( smJoystickEnabled );
}

//------------------------------------------------------------------------------
void DInputManager::disableJoystick()
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( !mgr || !mgr->isEnabled() || !smJoystickEnabled )
      return;

   mgr->deactivateJoystick();
   smJoystickEnabled = false;
   Con::printf( "DirectInput joystick disabled." );
#ifdef LOG_INPUT
   Input::log( "Joystick disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool DInputManager::isJoystickEnabled()
{
   return( smJoystickEnabled );
}

//------------------------------------------------------------------------------
bool DInputManager::activateJoystick()
{
   if ( !mEnabled || !Input::isActive() || !smJoystickEnabled )
      return( false );

   mJoystickActive = acquire( JoystickDeviceType, SI_ANY );
#ifdef LOG_INPUT
   Input::log( mJoystickActive ? "Joystick activated.\n" : "Joystick failed to activate!\n" );
#endif
   return( mJoystickActive );
}

//------------------------------------------------------------------------------
void DInputManager::deactivateJoystick()
{
   if ( mEnabled && mJoystickActive )
   {
      unacquire( JoystickDeviceType, SI_ANY );
      mJoystickActive = false;
#ifdef LOG_INPUT
      Input::log( "Joystick deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
const char* DInputManager::getJoystickAxesString( U32 deviceID )
{
   DInputDevice* dptr;
   for ( iterator ptr = begin(); ptr != end(); ptr++ )
   {
      dptr = dynamic_cast<DInputDevice*>( *ptr );
      if ( dptr && ( dptr->getDeviceType() == JoystickDeviceType ) && ( dptr->getDeviceID() == deviceID ) )
         return( dptr->getJoystickAxesString() );
   }

   return( "" );
}

//------------------------------------------------------------------------------
bool DInputManager::enableXInput()
{
   // Largely, this series of functions is identical to the Joystick versions,
   // except that XInput cannot be "activated" or "deactivated". You either have
   // the DLL or you don't. Beyond that, you have up to four controllers
   // connected at any given time

   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( !mgr || !mgr->isEnabled() )
      return( false );

   if ( mgr->isXInputActive() )
      return( true );

   if ( Input::isActive() )
      mgr->activateXInput();

   if ( smXInputEnabled )
   {
      Con::printf( "XInput enabled." );
#ifdef LOG_INPUT
      Input::log( "XInput enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "XInput failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "XInput failed to enable!\n" );
#endif
   }

   return( smXInputEnabled );
}

//------------------------------------------------------------------------------
void DInputManager::disableXInput()
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( !mgr || !mgr->isEnabled())
      return;

   mgr->deactivateXInput();
   Con::printf( "XInput disabled." );
#ifdef LOG_INPUT
   Input::log( "XInput disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool DInputManager::isXInputEnabled()
{
   return( smXInputEnabled );
}

//------------------------------------------------------------------------------
bool DInputManager::isXInputConnected(int controllerID)
{
   return( mXInputStateNew[controllerID].bConnected );
}

int DInputManager::getXInputState(int controllerID, int property, bool current)
{
   int retVal;

   switch(property)
   {
#define CHECK_PROP_ANALOG(prop, stateTest) \
   case prop:        (current) ? retVal = mXInputStateNew[controllerID].state.Gamepad.##stateTest : retVal = mXInputStateOld[controllerID].state.Gamepad.##stateTest; return retVal;

      CHECK_PROP_ANALOG(XI_THUMBLX, sThumbLX)
      CHECK_PROP_ANALOG(XI_THUMBLY, sThumbLY)
      CHECK_PROP_ANALOG(XI_THUMBRX, sThumbRX)
      CHECK_PROP_ANALOG(XI_THUMBRY, sThumbRY)
      CHECK_PROP_ANALOG(XI_LEFT_TRIGGER, bLeftTrigger)
      CHECK_PROP_ANALOG(XI_RIGHT_TRIGGER, bRightTrigger)

#undef CHECK_PROP_ANALOG

#define CHECK_PROP_DIGITAL(prop, stateTest) \
   case prop:           (current) ? retVal = (( mXInputStateNew[controllerID].state.Gamepad.wButtons & stateTest ) != 0 ) : retVal = (( mXInputStateOld[controllerID].state.Gamepad.wButtons & stateTest ) != 0 ); return retVal;

      CHECK_PROP_DIGITAL(SI_UPOV, XINPUT_GAMEPAD_DPAD_UP)
      CHECK_PROP_DIGITAL(SI_DPOV, XINPUT_GAMEPAD_DPAD_DOWN)
      CHECK_PROP_DIGITAL(SI_LPOV, XINPUT_GAMEPAD_DPAD_LEFT)
      CHECK_PROP_DIGITAL(SI_RPOV, XINPUT_GAMEPAD_DPAD_RIGHT)
      CHECK_PROP_DIGITAL(XI_START, XINPUT_GAMEPAD_START)
      CHECK_PROP_DIGITAL(XI_BACK, XINPUT_GAMEPAD_BACK)
      CHECK_PROP_DIGITAL(XI_LEFT_THUMB, XINPUT_GAMEPAD_LEFT_THUMB)
      CHECK_PROP_DIGITAL(XI_RIGHT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB)
      CHECK_PROP_DIGITAL(XI_LEFT_SHOULDER, XINPUT_GAMEPAD_LEFT_SHOULDER)
      CHECK_PROP_DIGITAL(XI_RIGHT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER)
      CHECK_PROP_DIGITAL(XI_A, XINPUT_GAMEPAD_A)
      CHECK_PROP_DIGITAL(XI_B, XINPUT_GAMEPAD_B)
      CHECK_PROP_DIGITAL(XI_X, XINPUT_GAMEPAD_X)
      CHECK_PROP_DIGITAL(XI_Y, XINPUT_GAMEPAD_Y)

#undef CHECK_PROP_DIGITAL
   }

   return -1;
}

//------------------------------------------------------------------------------
bool DInputManager::activateXInput()
{
   if ( !mEnabled || !Input::isActive())
      return( false );

   mXInputActive = true; //acquire( GamepadDeviceType, SI_ANY );
#ifdef LOG_INPUT
   Input::log( mXInputActive ? "XInput activated.\n" : "XInput failed to activate!\n" );
#endif
   return( mXInputActive );
}

//------------------------------------------------------------------------------
void DInputManager::deactivateXInput()
{
   if ( mEnabled && mXInputActive )
   {
      unacquire( GamepadDeviceType, SI_ANY );
      mXInputActive = false;
#ifdef LOG_INPUT
      Input::log( "XInput deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
bool DInputManager::rumble( const char *pDeviceName, float x, float y )
{
   // Determine the device
   U32 deviceType;
   U32 deviceInst;

   // Find the requested device
   if ( !ActionMap::getDeviceTypeAndInstance(pDeviceName, deviceType, deviceInst) )
   {
      Con::printf("DInputManager::rumble: unknown device: %s", pDeviceName);
      return false;
   }

   // clamp (x, y) to the range of [0 ... 1] each
   x = mClampF(x, 0.f, 1.f);
   y = mClampF(y, 0.f, 1.f);

   //  Easy path for xinput devices.
   if(deviceType == GamepadDeviceType)
   {
      XINPUT_VIBRATION vibration;
      vibration.wLeftMotorSpeed = static_cast<int>(x * 65535);
      vibration.wRightMotorSpeed = static_cast<int>(y * 65535);
      return ( mfnXInputSetState(deviceInst, &vibration) == ERROR_SUCCESS );
   }

   switch ( deviceType )
   {
   case JoystickDeviceType:

      // Find the device and shake it!
      DInputDevice* dptr;
      for ( iterator ptr = begin(); ptr != end(); ptr++ )
      {
         dptr = dynamic_cast<DInputDevice*>( *ptr );
         if ( dptr )
         {
            if (deviceType == dptr->getDeviceType() && deviceInst == dptr->getDeviceID())
            {
               dptr->rumble(x, y);
               return true;
            }
         }
      }

      // We should never get here... something's really messed up
      Con::errorf( "DInputManager::rumbleJoystick - Couldn't find device to rumble! This should never happen!\n" );
      return false;

   default:
      Con::printf("DInputManager::rumble - only supports joystick and xinput/gamepad devices");
      return false;
   }
}

void DInputManager::buildXInputEvent( U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, float fValue )
{
   InputEventInfo newEvent;

   newEvent.deviceType = GamepadDeviceType;
   newEvent.deviceInst = deviceInst;
   newEvent.objType = objType;
   newEvent.objInst = objInst;
   newEvent.action = action;
   newEvent.fValue = fValue;

   newEvent.postToSignal(Input::smInputEvent);
}

// The next three functions: fireXInputConnectEvent, fireXInputMoveEvent, and fireXInputButtonEvent
// determine whether a "delta" has occurred between the last captured controller state and the
// currently captured controller state and only if so, do we fire an event. The shortcutter
// "mXInputStateReset" is the exception and is true whenever DirectInput gets reset (because 
// the user ALT-TABBED away, for example). That means that after every context switch,
// you will get a full set of updates on the "true" state of the controller.
inline void DInputManager::fireXInputConnectEvent( int controllerID, bool condition, bool connected )
{
   if ( mXInputStateReset || condition )
   {
#ifdef LOG_INPUT
      Input::log( "EVENT (XInput): xinput%d CONNECT %s\n", controllerID, connected ? "make" : "break" );
#endif
      buildXInputEvent( controllerID, SI_BUTTON, XI_CONNECT, connected ? SI_MAKE : SI_BREAK, 0);
   }
}

inline void DInputManager::fireXInputMoveEvent( int controllerID, bool condition, InputObjectInstances objInst, float fValue )
{
   if ( mXInputStateReset || condition )
   {
#ifdef LOG_INPUT
      char *objName;
      switch (objInst)
      {
      case XI_THUMBLX:       objName = "THUMBLX"; break;
      case XI_THUMBLY:       objName = "THUMBLY"; break;
      case XI_THUMBRX:       objName = "THUMBRX"; break;
      case XI_THUMBRY:       objName = "THUMBRY"; break;
      case XI_LEFT_TRIGGER:  objName = "LEFT_TRIGGER"; break;
      case XI_RIGHT_TRIGGER: objName = "RIGHT_TRIGGER"; break;
      default:               objName = "UNKNOWN"; break;
      }

      Input::log( "EVENT (XInput): xinput%d %s MOVE %.1f.\n", controllerID, objName, fValue );
#endif
      buildXInputEvent( controllerID, SI_AXIS, objInst, SI_MOVE, fValue );
   }
}

inline void DInputManager::fireXInputButtonEvent( int controllerID, bool forceFire, int button, InputObjectInstances objInst )
{
   if ( mXInputStateReset || forceFire || ((mXInputStateNew[controllerID].state.Gamepad.wButtons & button) != (mXInputStateOld[controllerID].state.Gamepad.wButtons & button)) )
   {
#ifdef LOG_INPUT
      char *objName;
      switch (objInst)
      {      
      /*
      case XI_DPAD_UP:        objName = "DPAD_UP"; break;
      case XI_DPAD_DOWN:      objName = "DPAD_DOWN"; break;
      case XI_DPAD_LEFT:      objName = "DPAD_LEFT"; break;
      case XI_DPAD_RIGHT:     objName = "DPAD_RIGHT"; break;
      */      
      case XI_START:          objName = "START"; break;
      case XI_BACK:           objName = "BACK"; break;
      case XI_LEFT_THUMB:     objName = "LEFT_THUMB"; break;
      case XI_RIGHT_THUMB:    objName = "RIGHT_THUMB"; break;
      case XI_LEFT_SHOULDER:  objName = "LEFT_SHOULDER"; break;
      case XI_RIGHT_SHOULDER: objName = "RIGHT_SHOULDER"; break;
      case XI_A:              objName = "A"; break;
      case XI_B:              objName = "B"; break;
      case XI_X:              objName = "X"; break;
      case XI_Y:              objName = "Y"; break;
      default:                objName = "UNKNOWN"; break;
      }

      Input::log( "EVENT (XInput): xinput%d %s %s\n", controllerID, objName, ((mXInputStateNew[controllerID].state.Gamepad.wButtons & button) != 0) ? "make" : "break" );
#endif
      InputActionType action = ((mXInputStateNew[controllerID].state.Gamepad.wButtons & button) != 0) ? SI_MAKE : SI_BREAK;
      buildXInputEvent( controllerID, SI_BUTTON, objInst, action, ( action == SI_MAKE ? 1 : 0 ) );
   }
}

void DInputManager::processXInput( void )
{
   const U32 curTime = Platform::getRealMilliseconds();

   // We only want to check one disconnected device per frame.
   bool foundDisconnected = false;

   if ( mfnXInputGetState )
   {
      for ( int i=0; i<4; i++ )
      {
         // Calling XInputGetState on a disconnected controller takes a fair 
         // amount of time (probably because it tries to locate it), so we 
         // add a delay - only check every 250ms or so.
         if(mLastDisconnectTime[i] != -1)
         {
            // If it's not -1, then it was disconnected list time we checked.
            // So skip until it's time.
            if((curTime-mLastDisconnectTime[i]) < csmDisconnectedSkipDelay)
            {
               continue;
            }

            // If we already checked a disconnected controller, don't try any
            // further potentially disconnected devices.
            if(foundDisconnected)
            {
               // If we're skipping this, defer it out by the skip delay
               // so we don't get clumped checks.
               mLastDisconnectTime[i] += csmDisconnectedSkipDelay;
               continue;
            }
         }

         mXInputStateOld[i] = mXInputStateNew[i];
         mXInputStateNew[i].bConnected = ( mfnXInputGetState( i, &mXInputStateNew[i].state ) == ERROR_SUCCESS );

         // Update the last connected time.
         if(mXInputStateNew[i].bConnected)
            mLastDisconnectTime[i] = -1;
         else
         {
            foundDisconnected = true;
            mLastDisconnectTime[i] = curTime;
         }

         // trim the controller's thumbsticks to zero if they are within the deadzone
         if( mXInputDeadZoneOn )
         {
            // Zero value if thumbsticks are within the dead zone 
            if( (mXInputStateNew[i].state.Gamepad.sThumbLX < XINPUT_DEADZONE && mXInputStateNew[i].state.Gamepad.sThumbLX > -XINPUT_DEADZONE) && 
                (mXInputStateNew[i].state.Gamepad.sThumbLY < XINPUT_DEADZONE && mXInputStateNew[i].state.Gamepad.sThumbLY > -XINPUT_DEADZONE) ) 
            {
               mXInputStateNew[i].state.Gamepad.sThumbLX = 0;
               mXInputStateNew[i].state.Gamepad.sThumbLY = 0;
            }

            if( (mXInputStateNew[i].state.Gamepad.sThumbRX < XINPUT_DEADZONE && mXInputStateNew[i].state.Gamepad.sThumbRX > -XINPUT_DEADZONE) && 
                (mXInputStateNew[i].state.Gamepad.sThumbRY < XINPUT_DEADZONE && mXInputStateNew[i].state.Gamepad.sThumbRY > -XINPUT_DEADZONE) ) 
            {
               mXInputStateNew[i].state.Gamepad.sThumbRX = 0;
               mXInputStateNew[i].state.Gamepad.sThumbRY = 0;
            }
         }

         // this controller was connected or disconnected
         bool bJustConnected = ( ( mXInputStateOld[i].bConnected != mXInputStateNew[i].bConnected ) && ( mXInputStateNew[i].bConnected ) );
         fireXInputConnectEvent( i, (mXInputStateOld[i].bConnected != mXInputStateNew[i].bConnected), mXInputStateNew[i].bConnected );

         // If this controller is disconnected, stop reporting events for it
         if ( !mXInputStateNew[i].bConnected )
            continue;

         // == LEFT THUMBSTICK ==
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.sThumbLX != mXInputStateOld[i].state.Gamepad.sThumbLX), XI_THUMBLX, (mXInputStateNew[i].state.Gamepad.sThumbLX / 32768.0f) );
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.sThumbLY != mXInputStateOld[i].state.Gamepad.sThumbLY), XI_THUMBLY, (mXInputStateNew[i].state.Gamepad.sThumbLY / 32768.0f) );

         // == RIGHT THUMBSTICK ==
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.sThumbRX != mXInputStateOld[i].state.Gamepad.sThumbRX), XI_THUMBRX, (mXInputStateNew[i].state.Gamepad.sThumbRX / 32768.0f) );
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.sThumbRY != mXInputStateOld[i].state.Gamepad.sThumbRY), XI_THUMBRY, (mXInputStateNew[i].state.Gamepad.sThumbRY / 32768.0f) );

         // == LEFT & RIGHT REAR TRIGGERS ==
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.bLeftTrigger != mXInputStateOld[i].state.Gamepad.bLeftTrigger), XI_LEFT_TRIGGER, (mXInputStateNew[i].state.Gamepad.bLeftTrigger / 255.0f) );
         fireXInputMoveEvent( i, ( bJustConnected ) || (mXInputStateNew[i].state.Gamepad.bRightTrigger != mXInputStateOld[i].state.Gamepad.bRightTrigger), XI_RIGHT_TRIGGER, (mXInputStateNew[i].state.Gamepad.bRightTrigger / 255.0f) );

         // == BUTTONS: DPAD ==
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_DPAD_UP, SI_UPOV );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_DPAD_DOWN, SI_DPOV );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_DPAD_LEFT, SI_LPOV );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_DPAD_RIGHT, SI_RPOV );

         // == BUTTONS: START & BACK ==
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_START, XI_START );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_BACK, XI_BACK );

         // == BUTTONS: LEFT AND RIGHT THUMBSTICK ==
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_LEFT_THUMB, XI_LEFT_THUMB );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_RIGHT_THUMB, XI_RIGHT_THUMB );

         // == BUTTONS: LEFT AND RIGHT SHOULDERS (formerly WHITE and BLACK on Xbox 1) ==
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_LEFT_SHOULDER, XI_LEFT_SHOULDER );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_RIGHT_SHOULDER, XI_RIGHT_SHOULDER );

         // == BUTTONS: A, B, X, and Y ==
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_A, XI_A );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_B, XI_B );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_X, XI_X );
         fireXInputButtonEvent( i, bJustConnected, XINPUT_GAMEPAD_Y, XI_Y );
      }

      if ( mXInputStateReset ) 
         mXInputStateReset = false;
   }
}
ConsoleFunction( enableJoystick, bool, 1, 1, "()"
             "@brief Enables use of the joystick.\n\n"
             "@note DirectInput must be enabled and active to use this function.\n\n"
             "@ingroup Input")
{
   argc; argv;
   return( DInputManager::enableJoystick() );
}

//------------------------------------------------------------------------------
ConsoleFunction( disableJoystick, void, 1, 1,"()"
             "@brief Disables use of the joystick.\n\n"
             "@note DirectInput must be enabled and active to use this function.\n\n"
             "@ingroup Input")
{
   argc; argv;
   DInputManager::disableJoystick();
}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickEnabled, bool, 1, 1, "()"
				"@brief Queries input manager to see if a joystick is enabled\n\n"
				"@return 1 if a joystick exists and is enabled, 0 if it's not.\n"
				"@ingroup Input")
{
   argc; argv;
   return DInputManager::isJoystickEnabled();
}

//------------------------------------------------------------------------------
ConsoleFunction( enableXInput, bool, 1, 1, "()"
            "@brief Enables XInput for Xbox 360 controllers.\n\n"
            "@note XInput is enabled by default. Disable to use an Xbox 360 "
            "Controller as a joystick device.\n\n"
				"@ingroup Input")
{
   // Although I said above that you couldn't change the "activation" of XInput,
   // you can enable and disable it. It gets enabled by default if you have the
   // DLL. You would want to disable it if you have 360 controllers and want to
   // read them as joysticks... why you'd want to do that is beyond me

   argc; argv;
   return( DInputManager::enableXInput() );
}

//------------------------------------------------------------------------------
ConsoleFunction( disableXInput, void, 1, 1, "()"
            "@brief Disables XInput for Xbox 360 controllers.\n\n"
            "@ingroup Input")
{
   argc; argv;
   DInputManager::disableXInput();
}

//------------------------------------------------------------------------------
ConsoleFunction( resetXInput, void, 1, 1, "()"
            "@brief Rebuilds the XInput section of the InputManager\n\n"
            "Requests a full refresh of events for all controllers. Useful when called at the beginning "
            "of game code after actionMaps are set up to hook up all appropriate events.\n\n"
            "@ingroup Input")
{
   // This function requests a full "refresh" of events for all controllers the
   // next time we go through the input processing loop. This is useful to call
   // at the beginning of your game code after your actionMap is set up to hook
   // all of the appropriate events

   argc; argv;
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() ) 
      mgr->resetXInput();
}

//------------------------------------------------------------------------------
ConsoleFunction( isXInputConnected, bool, 2, 2, "( int controllerID )"
				"@brief Checks to see if an Xbox 360 controller is connected\n\n"
				"@param controllerID Zero-based index of the controller to check.\n"
            "@return 1 if the controller is connected, 0 if it isn't, and 205 if XInput "
            "hasn't been initialized."
				"@ingroup Input")
{
   argc; argv;
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() ) return mgr->isXInputConnected( atoi( argv[1] ) );
   return false;
}

//------------------------------------------------------------------------------
ConsoleFunction( getXInputState, int, 3, 4, "( int controllerID, string property, bool current )"
				"@brief Queries the current state of a connected Xbox 360 controller.\n\n"
            "XInput Properties:\n\n"
            " - XI_THUMBLX, XI_THUMBLY - X and Y axes of the left thumbstick. \n"
            " - XI_THUMBRX, XI_THUMBRY - X and Y axes of the right thumbstick. \n"
            " - XI_LEFT_TRIGGER, XI_RIGHT_TRIGGER - Left and Right triggers. \n"
            " - SI_UPOV, SI_DPOV, SI_LPOV, SI_RPOV - Up, Down, Left, and Right on the directional pad.\n"
            " - XI_START, XI_BACK - The Start and Back buttons.\n"
            " - XI_LEFT_THUMB, XI_RIGHT_THUMB - Clicking in the left and right thumbstick.\n"
            " - XI_LEFT_SHOULDER, XI_RIGHT_SHOULDER - Left and Right bumpers.\n"
            " - XI_A, XI_B , XI_X, XI_Y - The A, B, X, and Y buttons.\n\n"
            "@param controllerID Zero-based index of the controller to return information about.\n"
            "@param property Name of input action being queried, such as \"XI_THUMBLX\".\n"
            "@param current True checks current device in action.\n"
            "@return Button queried - 1 if the button is pressed, 0 if it's not.\n"
            "@return Thumbstick queried - Int representing displacement from rest position."
            "@return %Trigger queried - Int from 0 to 255 representing how far the trigger is displaced."
            "@ingroup Input")
{
   argc; argv;
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );

   if ( !mgr || !mgr->isEnabled() ) 
      return -1;

   // Use a little bit of macro magic to simplify this otherwise monolothic
   // block of code.
#define GET_XI_STATE(constName) \
   if (!dStricmp(argv[2], #constName)) \
      return mgr->getXInputState( dAtoi( argv[1] ), constName, ( dAtoi ( argv[3] ) == 1) );
   
   GET_XI_STATE(XI_THUMBLX);
   GET_XI_STATE(XI_THUMBLY);
   GET_XI_STATE(XI_THUMBRX);
   GET_XI_STATE(XI_THUMBRY);
   GET_XI_STATE(XI_LEFT_TRIGGER);
   GET_XI_STATE(XI_RIGHT_TRIGGER);
   GET_XI_STATE(SI_UPOV);
   GET_XI_STATE(SI_DPOV);
   GET_XI_STATE(SI_LPOV);
   GET_XI_STATE(SI_RPOV);
   GET_XI_STATE(XI_START);
   GET_XI_STATE(XI_BACK);
   GET_XI_STATE(XI_LEFT_THUMB);
   GET_XI_STATE(XI_RIGHT_THUMB);
   GET_XI_STATE(XI_LEFT_SHOULDER);
   GET_XI_STATE(XI_RIGHT_SHOULDER);
   GET_XI_STATE(XI_A);
   GET_XI_STATE(XI_B);
   GET_XI_STATE(XI_X);
   GET_XI_STATE(XI_Y);
#undef GET_XI_STATE

   return -1;
}

//------------------------------------------------------------------------------
ConsoleFunction( echoInputState, void, 1, 1, "()"
            "@brief Prints information to the console stating if DirectInput and a Joystick are enabled and active.\n\n"
            "@ingroup Input")
{
   argc; argv;
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() )
   {
      Con::printf( "DirectInput is enabled %s.", Input::isActive() ? "and active" : "but inactive" );
      Con::printf( "- Joystick is %sabled and %sactive.",
            mgr->isJoystickEnabled() ? "en" : "dis",
            mgr->isJoystickActive() ? "" : "in" );
   }
   else
      Con::printf( "DirectInput is not enabled." );
}

ConsoleFunction( rumble, void, 4, 4, "(string device, float xRumble, float yRumble)"
      "@brief Activates the vibration motors in the specified controller.\n\n"
      "The controller will constantly at it's xRumble and yRumble intensities until "
      "changed or told to stop."
      "Valid inputs for xRumble/yRumble are [0 - 1].\n"
      "@param device Name of the device to rumble.\n"
      "@param xRumble Intensity to apply to the left motor.\n"
      "@param yRumble Intensity to apply to the right motor.\n"
      "@note in an Xbox 360 controller, the left motor is low-frequency, "
      "while the right motor is high-frequency."
      "@ingroup Input")
{
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() )
   {
      mgr->rumble(argv[1], dAtof(argv[2]), dAtof(argv[3]));
   }
   else
   {
      Con::printf( "DirectInput/XInput is not enabled." );
   }
}
