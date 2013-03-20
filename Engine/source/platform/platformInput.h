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

#ifndef _PLATFORMINPUT_H_
#define _PLATFORMINPUT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#include "platform/input/event.h"

//------------------------------------------------------------------------------
U8 TranslateOSKeyCode( U8 vcode );
U8 TranslateKeyCodeToOS(U8 keycode);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class InputDevice : public SimObject
{
protected:
   char mName[30];

public:
   struct ObjInfo
   {
      InputEventType      mType;
      InputObjectInstances  mInst;
      S32   mMin, mMax;
   };

   inline const char* getDeviceName()
   {
      return mName;
   }

   virtual bool process() = 0;
};

//------------------------------------------------------------------------------

class InputManager : public SimGroup
{
protected:
   bool  mEnabled;

public:
   inline bool  isEnabled()
   {
      return mEnabled;
   }

   virtual bool enable() = 0;
   virtual void disable() = 0;
   virtual void process() = 0;
};

enum KEY_STATE
{
   STATE_LOWER,
   STATE_UPPER,
   STATE_GOOFY
};

//------------------------------------------------------------------------------
class Input
{
protected:
   static InputManager* smManager;

   static bool smActive;

   /// Current modifier keys.
   static U8 smModifierKeys;

   static bool smLastKeyboardActivated;
   static bool smLastMouseActivated;
   static bool smLastJoystickActivated;

public:
   static void init();
   static void destroy();

   static bool enable();
   static void disable();

   static void activate();
   static void deactivate();

   static U16  getAscii( U16 keyCode, KEY_STATE keyState );
   static U16  getKeyCode( U16 asciiCode );

   static bool isEnabled();
   static bool isActive();

   static void process();

   static InputManager* getManager();

   static U8 getModifierKeys() {return smModifierKeys;}
   static void setModifierKeys(U8 mod) {smModifierKeys = mod;}
#ifdef LOG_INPUT
   static void log( const char* format, ... );
#endif

#ifdef TORQUE_OS_XENON
   static S32 getLockedController();
#endif

   /// Global input routing JournaledSignal; post input events here for
   /// processing.
   static InputEvent smInputEvent;
};

#endif // _H_PLATFORMINPUT_
