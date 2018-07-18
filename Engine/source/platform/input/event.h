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
/// @file
/// Library-wide input events
///
/// All external events are converted into system events, which are defined
/// in this file.

///
#ifndef _EVENT_H_
#define _EVENT_H_

#include "platform/types.h"
#include "platform/input/IInputDevice.h"
#include "core/util/journal/journaledSignal.h"
#include "core/util/tSingleton.h"
#include "core/util/tDictionary.h"
#include "core/tSimpleHashTable.h"
#include "SDL_keycode.h"
#include "SDL_scancode.h"

#define AddInputVirtualMap( description, type, code )         \
   INPUTMGR->addVirtualMap( #description, type, code );

/// @defgroup input_constants Input system constants
/// @{

/// Wildcard match used by the input system.
#define SI_ANY SDL_NUM_SCANCODES - 1   //0x01ff

/// Input event constants:
typedef U32 InputObjectInstances;
enum InputObjectInstancesEnum
{
   KEY_NULL          = 0x000,     ///< Invalid KeyCode
   KEY_BACKSPACE     = SDL_SCANCODE_BACKSPACE,
   KEY_TAB           = SDL_SCANCODE_TAB,
   KEY_RETURN        = SDL_SCANCODE_RETURN,
   //KEY_CONTROL       = 0x004,
   //KEY_ALT           = 0x005,
   //KEY_SHIFT         = 0x006,
   KEY_PAUSE         = SDL_SCANCODE_PAUSE,
   KEY_CAPSLOCK      = SDL_SCANCODE_CAPSLOCK,
   KEY_ESCAPE        = SDL_SCANCODE_ESCAPE,
   KEY_SPACE         = SDL_SCANCODE_SPACE,
   KEY_PAGE_DOWN     = SDL_SCANCODE_PAGEDOWN,
   KEY_PAGE_UP       = SDL_SCANCODE_PAGEUP,
   KEY_END           = SDL_SCANCODE_END,
   KEY_HOME          = SDL_SCANCODE_HOME,
   KEY_LEFT          = SDL_SCANCODE_LEFT,
   KEY_UP            = SDL_SCANCODE_UP,
   KEY_RIGHT         = SDL_SCANCODE_RIGHT,
   KEY_DOWN          = SDL_SCANCODE_DOWN,
   KEY_PRINT         = SDL_SCANCODE_PRINTSCREEN,
   KEY_INSERT        = SDL_SCANCODE_INSERT,
   KEY_DELETE        = SDL_SCANCODE_DELETE,
   KEY_HELP          = SDL_SCANCODE_HELP,

   KEY_0             = SDL_SCANCODE_0,
   KEY_1             = SDL_SCANCODE_1,
   KEY_2             = SDL_SCANCODE_2,
   KEY_3             = SDL_SCANCODE_3,
   KEY_4             = SDL_SCANCODE_4,
   KEY_5             = SDL_SCANCODE_5,
   KEY_6             = SDL_SCANCODE_6,
   KEY_7             = SDL_SCANCODE_7,
   KEY_8             = SDL_SCANCODE_8,
   KEY_9             = SDL_SCANCODE_9,

   KEY_A             = SDL_SCANCODE_A,
   KEY_B             = SDL_SCANCODE_B,
   KEY_C             = SDL_SCANCODE_C,
   KEY_D             = SDL_SCANCODE_D,
   KEY_E             = SDL_SCANCODE_E,
   KEY_F             = SDL_SCANCODE_F,
   KEY_G             = SDL_SCANCODE_G,
   KEY_H             = SDL_SCANCODE_H,
   KEY_I             = SDL_SCANCODE_I,
   KEY_J             = SDL_SCANCODE_J,
   KEY_K             = SDL_SCANCODE_K,
   KEY_L             = SDL_SCANCODE_L,
   KEY_M             = SDL_SCANCODE_M,
   KEY_N             = SDL_SCANCODE_N,
   KEY_O             = SDL_SCANCODE_O,
   KEY_P             = SDL_SCANCODE_P,
   KEY_Q             = SDL_SCANCODE_Q,
   KEY_R             = SDL_SCANCODE_R,
   KEY_S             = SDL_SCANCODE_S,
   KEY_T             = SDL_SCANCODE_T,
   KEY_U             = SDL_SCANCODE_U,
   KEY_V             = SDL_SCANCODE_V,
   KEY_W             = SDL_SCANCODE_W,
   KEY_X             = SDL_SCANCODE_X,
   KEY_Y             = SDL_SCANCODE_Y,
   KEY_Z             = SDL_SCANCODE_Z,

   KEY_TILDE         = SDL_SCANCODE_GRAVE,
   KEY_MINUS         = SDL_SCANCODE_MINUS,
   KEY_EQUALS        = SDL_SCANCODE_EQUALS,
   KEY_LBRACKET      = SDL_SCANCODE_LEFTBRACKET,
   KEY_RBRACKET      = SDL_SCANCODE_RIGHTBRACKET,
   KEY_BACKSLASH     = SDL_SCANCODE_BACKSLASH,
   KEY_SEMICOLON     = SDL_SCANCODE_SEMICOLON,
   KEY_APOSTROPHE    = SDL_SCANCODE_APOSTROPHE,
   KEY_COMMA         = SDL_SCANCODE_COMMA,
   KEY_PERIOD        = SDL_SCANCODE_PERIOD,
   KEY_SLASH         = SDL_SCANCODE_SLASH,
   KEY_NUMPAD0       = SDL_SCANCODE_KP_0,
   KEY_NUMPAD1       = SDL_SCANCODE_KP_1,
   KEY_NUMPAD2       = SDL_SCANCODE_KP_2,
   KEY_NUMPAD3       = SDL_SCANCODE_KP_3,
   KEY_NUMPAD4       = SDL_SCANCODE_KP_4,
   KEY_NUMPAD5       = SDL_SCANCODE_KP_5,
   KEY_NUMPAD6       = SDL_SCANCODE_KP_6,
   KEY_NUMPAD7       = SDL_SCANCODE_KP_7,
   KEY_NUMPAD8       = SDL_SCANCODE_KP_8,
   KEY_NUMPAD9       = SDL_SCANCODE_KP_9,
   KEY_MULTIPLY      = SDL_SCANCODE_KP_MULTIPLY,
   KEY_ADD           = SDL_SCANCODE_KP_PLUS,
   KEY_SEPARATOR     = SDL_SCANCODE_KP_VERTICALBAR,
   KEY_SUBTRACT      = SDL_SCANCODE_KP_MINUS,
   KEY_DECIMAL       = SDL_SCANCODE_KP_PERIOD,
   KEY_DIVIDE        = SDL_SCANCODE_KP_DIVIDE,
   KEY_NUMPADENTER   = SDL_SCANCODE_KP_ENTER,
   KEY_NUMPADCLEAR   = SDL_SCANCODE_CLEAR,
   KEY_NUMPADEQUALS  = SDL_SCANCODE_KP_EQUALS,

   KEY_F1            = SDL_SCANCODE_F1,
   KEY_F2            = SDL_SCANCODE_F2,
   KEY_F3            = SDL_SCANCODE_F3,
   KEY_F4            = SDL_SCANCODE_F4,
   KEY_F5            = SDL_SCANCODE_F5,
   KEY_F6            = SDL_SCANCODE_F6,
   KEY_F7            = SDL_SCANCODE_F7,
   KEY_F8            = SDL_SCANCODE_F8,
   KEY_F9            = SDL_SCANCODE_F9,
   KEY_F10           = SDL_SCANCODE_F10,
   KEY_F11           = SDL_SCANCODE_F11,
   KEY_F12           = SDL_SCANCODE_F12,
   KEY_F13           = SDL_SCANCODE_F13,
   KEY_F14           = SDL_SCANCODE_F14,
   KEY_F15           = SDL_SCANCODE_F15,
   KEY_F16           = SDL_SCANCODE_F16,
   KEY_F17           = SDL_SCANCODE_F17,
   KEY_F18           = SDL_SCANCODE_F18,
   KEY_F19           = SDL_SCANCODE_F19,
   KEY_F20           = SDL_SCANCODE_F20,
   KEY_F21           = SDL_SCANCODE_F21,
   KEY_F22           = SDL_SCANCODE_F22,
   KEY_F23           = SDL_SCANCODE_F23,
   KEY_F24           = SDL_SCANCODE_F24,

   KEY_NUMLOCK       = SDL_SCANCODE_NUMLOCKCLEAR,
   KEY_SCROLLLOCK    = SDL_SCANCODE_SCROLLLOCK,
   KEY_LCONTROL      = SDL_SCANCODE_LCTRL,
   KEY_RCONTROL      = SDL_SCANCODE_RCTRL,
   KEY_LALT          = SDL_SCANCODE_LALT,
   KEY_RALT          = SDL_SCANCODE_RALT,
   KEY_LSHIFT        = SDL_SCANCODE_LSHIFT,
   KEY_RSHIFT        = SDL_SCANCODE_RSHIFT,
   //KEY_WIN_LWINDOW   = 0x077,
   //KEY_WIN_RWINDOW   = 0x078,
   KEY_WIN_APPS      = SDL_SCANCODE_APPLICATION,
   KEY_OEM_102       = SDL_SCANCODE_NONUSBACKSLASH,

   //KEY_MAC_OPT       = 0x090,
   KEY_MAC_LOPT      = SDL_SCANCODE_LGUI,
   KEY_MAC_ROPT      = SDL_SCANCODE_RGUI,

   KEY_ANYKEY        = SI_ANY,
   // SDL scancode values can range 4 - 0x1ff. See SDL_NUM_SCANCODES in SDL_scancode.h

   /// Mouse/Joystick button event codes.
   KEY_BUTTON0       = 0x0200,
   KEY_BUTTON1       = 0x0201,
   KEY_BUTTON2       = 0x0202,
   KEY_BUTTON3       = 0x0203,
   KEY_BUTTON4       = 0x0204,
   KEY_BUTTON5       = 0x0205,
   KEY_BUTTON6       = 0x0206,
   KEY_BUTTON7       = 0x0207,
   KEY_BUTTON8       = 0x0208,
   KEY_BUTTON9       = 0x0209,
   KEY_BUTTON10      = 0x020A,
   KEY_BUTTON11      = 0x020B,
   KEY_BUTTON12      = 0x020C,
   KEY_BUTTON13      = 0x020D,
   KEY_BUTTON14      = 0x020E,
   KEY_BUTTON15      = 0x020F,
   KEY_BUTTON16      = 0x0210,
   KEY_BUTTON17      = 0x0211,
   KEY_BUTTON18      = 0x0212,
   KEY_BUTTON19      = 0x0213,
   KEY_BUTTON20      = 0x0214,
   KEY_BUTTON21      = 0x0215,
   KEY_BUTTON22      = 0x0216,
   KEY_BUTTON23      = 0x0217,
   KEY_BUTTON24      = 0x0218,
   KEY_BUTTON25      = 0x0219,
   KEY_BUTTON26      = 0x021A,
   KEY_BUTTON27      = 0x021B,
   KEY_BUTTON28      = 0x021C,
   KEY_BUTTON29      = 0x021D,
   KEY_BUTTON30      = 0x021E,
   KEY_BUTTON31      = 0x021F,
   KEY_BUTTON32      = 0x0220,
   KEY_BUTTON33      = 0x0221,
   KEY_BUTTON34      = 0x0222,
   KEY_BUTTON35      = 0x0223,
   KEY_BUTTON36      = 0x0224,
   KEY_BUTTON37      = 0x0225,
   KEY_BUTTON38      = 0x0226,
   KEY_BUTTON39      = 0x0227,
   KEY_BUTTON40      = 0x0228,
   KEY_BUTTON41      = 0x0229,
   KEY_BUTTON42      = 0x022A,
   KEY_BUTTON43      = 0x022B,
   KEY_BUTTON44      = 0x022C,
   KEY_BUTTON45      = 0x022D,
   KEY_BUTTON46      = 0x022E,
   KEY_BUTTON47      = 0x022F,

   /// Axis event codes
   SI_XAXIS          = 0x0300,
   SI_YAXIS          = 0x0301,
   SI_ZAXIS          = 0x0302,
   SI_RXAXIS         = 0x0303,
   SI_RYAXIS         = 0x0304,
   SI_RZAXIS         = 0x0305,
   SI_SLIDER         = 0x0306,

   /// DPad/Hat event codes.
   SI_UPOV           = 0x0310,
   SI_DPOV           = 0x0311,
   SI_LPOV           = 0x0312,
   SI_RPOV           = 0x0313,
   SI_UPOV2          = 0x0314,
   SI_DPOV2          = 0x0315,
   SI_LPOV2          = 0x0316,
   SI_RPOV2          = 0x0317,
   SI_POVMASK        = 0x0318,
   SI_POVMASK2       = 0x0319,

   /// Trackball event codes.
   SI_XBALL          = 0x0320,
   SI_YBALL          = 0x0321,
   SI_XBALL2         = 0x0322,
   SI_YBALL2         = 0x0323,

   /// Gamepad button event codes.
   XI_A              = 0x0330,
   XI_B              = 0x0331,
   XI_X              = 0x0332,
   XI_Y              = 0x0333,

   XI_BACK           = 0x0334,
   XI_GUIDE          = 0x0335,
   XI_START          = 0x0336,
   XI_LEFT_THUMB     = 0x0337,
   XI_RIGHT_THUMB    = 0x0338,
   XI_LEFT_SHOULDER  = 0x0339,
   XI_RIGHT_SHOULDER = 0x033A,

   INPUT_DEVICE_PLUGIN_CODES_START = 0x400,
};

#define XI_THUMBLX SI_XAXIS
#define XI_THUMBLY SI_YAXIS
#define XI_THUMBRX SI_RXAXIS
#define XI_THUMBRY SI_RYAXIS
#define XI_LEFT_TRIGGER SI_ZAXIS
#define XI_RIGHT_TRIGGER SI_RZAXIS

/// Input device types
typedef U32 InputDeviceTypes;
enum InputDeviceTypesEnum
{
   UnknownDeviceType,
   MouseDeviceType,
   KeyboardDeviceType,
   JoystickDeviceType,
   GamepadDeviceType,

   NUM_INPUT_DEVICE_TYPES,

   INPUT_DEVICE_PLUGIN_DEVICES_START = NUM_INPUT_DEVICE_TYPES,
};

/// Device Event Action Types
enum InputActionType
{
   /// Button was depressed.
   SI_MAKE    = 0x01,

   /// Button was released.
   SI_BREAK   = 0x02,

   /// An axis moved.
   SI_MOVE    = 0x03,

   /// A key repeat occurred. Happens in between a SI_MAKE and SI_BREAK.
   SI_REPEAT  = 0x04,

   /// A value of some type.  Matched with SI_FLOAT or SI_INT.
   SI_VALUE   = 0x05,
};

///Device Event Types
enum InputEventType
{
   SI_UNKNOWN = 0x01,
   SI_BUTTON  = 0x02,   // Button press/release
   SI_POV     = 0x03,   // Point of View hat
   SI_AXIS    = 0x04,   // Axis in range -1.0..1.0
   SI_POS     = 0x05,   // Absolute position value (Point3F)
   SI_ROT     = 0x06,   // Absolute rotation value (QuatF)
   SI_INT     = 0x07,   // Integer value (S32)
   SI_FLOAT   = 0x08,   // Float value (F32)
   SI_KEY     = 0x0A,   // Keyboard key
};

// Modifier Keys
enum InputModifiers
{
   /// shift and ctrl are the same between platforms.
   SI_LSHIFT = KMOD_LSHIFT,
   SI_RSHIFT = KMOD_RSHIFT,
   SI_SHIFT  = (SI_LSHIFT|SI_RSHIFT),
   SI_LCTRL  = KMOD_LCTRL,
   SI_RCTRL  = KMOD_RCTRL,
   SI_CTRL   = (SI_LCTRL|SI_RCTRL),

   /// win altkey, mapped to mac cmdkey.
   SI_LALT = KMOD_LALT,
   SI_RALT = KMOD_RALT,
   SI_ALT = (SI_LALT|SI_RALT),

   /// mac optionkey
   SI_MAC_LOPT  = KMOD_LGUI,
   SI_MAC_ROPT  = KMOD_RGUI,
   SI_MAC_OPT   = (SI_MAC_LOPT|SI_MAC_ROPT),

   /// modifier keys used for common operations
#if defined(TORQUE_OS_MAC)
   SI_COPYPASTE = SI_ALT,
   SI_MULTISELECT = SI_ALT,
   SI_RANGESELECT = SI_SHIFT,
   SI_PRIMARY_ALT = SI_MAC_OPT,  ///< Primary key used for toggling into alternates of commands.
   SI_PRIMARY_CTRL = SI_ALT,     ///< Primary key used for triggering commands.
#else
   SI_COPYPASTE = SI_CTRL,
   SI_MULTISELECT = SI_CTRL,
   SI_RANGESELECT = SI_SHIFT,
   SI_PRIMARY_ALT = SI_ALT,
   SI_PRIMARY_CTRL = SI_CTRL,
#endif
   /// modfier key used in conjunction w/ arrow keys to move cursor to next word
#if defined(TORQUE_OS_MAC)
   SI_WORDJUMP = SI_MAC_OPT,
#else
   SI_WORDJUMP = SI_CTRL,
#endif
   /// modifier key used in conjunction w/ arrow keys to move cursor to beginning / end of line
   SI_LINEJUMP = SI_ALT,

   /// modifier key used in conjunction w/ home & end to jump to the top or bottom of a document
#if defined(TORQUE_OS_MAC)
   SI_DOCJUMP = SI_ANY,
#else
   SI_DOCJUMP = SI_CTRL,
#endif
};

/// @}


/// Generic input event.
struct InputEventInfo
{
   InputEventInfo()
   {
      deviceInst = 0;
      fValue     = 0.f;
      fValue2    = 0.f;
      fValue3    = 0.f;
      fValue4    = 0.f;
      iValue     = 0;
      deviceType = (InputDeviceTypes)0;
      objType    = (InputEventType)0;
      ascii      = 0;
      objInst    = (InputObjectInstances)0;
      action     = (InputActionType)0;
      modifier   = (InputModifiers)0;
   }

   /// Device instance: joystick0, joystick1, etc
   U32 deviceInst;

   /// Value typically ranges from -1.0 to 1.0, but doesn't have to.
   /// It depends on the context.
   F32 fValue;

   /// Extended float values (often used for absolute rotation Quat)
   F32 fValue2;
   F32 fValue3;
   F32 fValue4;

   /// Signed integer value
   S32 iValue;

   /// What was the action? (MAKE/BREAK/MOVE)
   InputActionType      action;
   InputDeviceTypes     deviceType;
   InputEventType       objType;
   InputObjectInstances objInst;

   /// ASCII character code if this is a keyboard event.
   U16 ascii;
   
   /// Modifiers to action: SI_LSHIFT, SI_LCTRL, etc.
   U32 modifier;

   inline void postToSignal(InputEvent &ie)
   {
      ie.trigger(deviceInst, fValue, fValue2, fValue3, fValue4, iValue, deviceType, objType, ascii, objInst, action, modifier);
   }
};

class Point3F;
class QuatF;

/// Handles input device plug-ins
class InputEventManager
{
public:
   struct VirtualMapData
   {
      StringTableEntry     desc;
      InputEventType       type;
      InputObjectInstances code;
   };

public:
   InputEventManager();
   virtual ~InputEventManager();

   /// Get the next device type code
   U32 getNextDeviceType();

   /// Get the next device action code
   U32 getNextDeviceCode();

   void registerDevice(IInputDevice* device);
   void unregisterDevice(IInputDevice* device);

   /// Check if the given device name is a registered device.
   /// The given name can optionally include an instance number on the end.
   bool isRegisteredDevice(const char* name);

   /// Check if the given device type is a registered device.
   bool isRegisteredDevice(U32 type);

   /// Same as above but also provides the found device type and actual
   // device name length.  Used by ActionMap::getDeviceTypeAndInstance()
   bool isRegisteredDeviceWithAttributes(const char* name, U32& deviceType, U32&nameLen);

   /// Returns the name of a registered device given its type
   const char* getRegisteredDeviceName(U32 type);

   void start();
   void stop();

   void process();

   // Add to the virtual map table
   void addVirtualMap(const char* description, InputEventType type, InputObjectInstances code);

   // Find a virtual map entry based on the text description
   VirtualMapData* findVirtualMap(const char* description);

   // Find a keyboard map entry based on the text description
   VirtualMapData* findKeyboardMap(const char* description);

   // Find a virtual map entry's description based on the action code
   const char* findVirtualMapDescFromCode(U32 code);

   // Find a keyboard map entry's description based on the scan code
   const char* findKeyboardMapDescFromCode(U32 code);

   /// Build an input event based on a single iValue
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, S32 iValue);

   /// Build an input event based on a single fValue
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, F32 fValue);

   /// Build an input event based on a Point3F
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, Point3F& pValue);

   /// Build an input event based on a QuatF
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, QuatF& qValue);

   /// Build an input event based on a AngAxisF
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, AngAxisF& qValue);

protected:
   U32 mNextDeviceTypeCode;
   U32 mNextDeviceCode;

   Vector<IInputDevice*> mDeviceList;

   // Holds description to VirtualMapData struct
   SimpleHashTable<VirtualMapData> mVirtualMap;
   SimpleHashTable<VirtualMapData> mKeyboardMap;

   // Used to look up a description based on a VirtualMapData.code
   HashTable<U32, VirtualMapData> mActionCodeMap;
   HashTable<U32, VirtualMapData> mScanCodeMap;

protected:
   void buildVirtualMap();
   void buildKeyboardMap();

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "InputEventManager"; }   
};

/// Returns the InputEventManager singleton.
#define INPUTMGR ManagedSingleton<InputEventManager>::instance()


#endif
