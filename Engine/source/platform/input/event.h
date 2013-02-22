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

#define AddInputVirtualMap( description, type, code )         \
   INPUTMGR->addVirtualMap( #description, type, code );

/// @defgroup input_constants Input system constants
/// @{

/// Input event constants:
typedef U32 InputObjectInstances;
enum InputObjectInstancesEnum
{
   KEY_NULL          = 0x000,     ///< Invalid KeyCode
   KEY_BACKSPACE     = 0x001,
   KEY_TAB           = 0x002,
   KEY_RETURN        = 0x003,
   KEY_CONTROL       = 0x004,
   KEY_ALT           = 0x005,
   KEY_SHIFT         = 0x006,
   KEY_PAUSE         = 0x007,
   KEY_CAPSLOCK      = 0x008,
   KEY_ESCAPE        = 0x009,
   KEY_SPACE         = 0x00a,
   KEY_PAGE_DOWN     = 0x00b,
   KEY_PAGE_UP       = 0x00c,
   KEY_END           = 0x00d,
   KEY_HOME          = 0x00e,
   KEY_LEFT          = 0x00f,
   KEY_UP            = 0x010,
   KEY_RIGHT         = 0x011,
   KEY_DOWN          = 0x012,
   KEY_PRINT         = 0x013,
   KEY_INSERT        = 0x014,
   KEY_DELETE        = 0x015,
   KEY_HELP          = 0x016,

   KEY_0             = 0x017,
   KEY_1             = 0x018,
   KEY_2             = 0x019,
   KEY_3             = 0x01a,
   KEY_4             = 0x01b,
   KEY_5             = 0x01c,
   KEY_6             = 0x01d,
   KEY_7             = 0x01e,
   KEY_8             = 0x01f,
   KEY_9             = 0x020,

   KEY_A             = 0x021,
   KEY_B             = 0x022,
   KEY_C             = 0x023,
   KEY_D             = 0x024,
   KEY_E             = 0x025,
   KEY_F             = 0x026,
   KEY_G             = 0x027,
   KEY_H             = 0x028,
   KEY_I             = 0x029,
   KEY_J             = 0x02a,
   KEY_K             = 0x02b,
   KEY_L             = 0x02c,
   KEY_M             = 0x02d,
   KEY_N             = 0x02e,
   KEY_O             = 0x02f,
   KEY_P             = 0x030,
   KEY_Q             = 0x031,
   KEY_R             = 0x032,
   KEY_S             = 0x033,
   KEY_T             = 0x034,
   KEY_U             = 0x035,
   KEY_V             = 0x036,
   KEY_W             = 0x037,
   KEY_X             = 0x038,
   KEY_Y             = 0x039,
   KEY_Z             = 0x03a,

   KEY_TILDE         = 0x03b,
   KEY_MINUS         = 0x03c,
   KEY_EQUALS        = 0x03d,
   KEY_LBRACKET      = 0x03e,
   KEY_RBRACKET      = 0x03f,
   KEY_BACKSLASH     = 0x040,
   KEY_SEMICOLON     = 0x041,
   KEY_APOSTROPHE    = 0x042,
   KEY_COMMA         = 0x043,
   KEY_PERIOD        = 0x044,
   KEY_SLASH         = 0x045,
   KEY_NUMPAD0       = 0x046,
   KEY_NUMPAD1       = 0x047,
   KEY_NUMPAD2       = 0x048,
   KEY_NUMPAD3       = 0x049,
   KEY_NUMPAD4       = 0x04a,
   KEY_NUMPAD5       = 0x04b,
   KEY_NUMPAD6       = 0x04c,
   KEY_NUMPAD7       = 0x04d,
   KEY_NUMPAD8       = 0x04e,
   KEY_NUMPAD9       = 0x04f,
   KEY_MULTIPLY      = 0x050,
   KEY_ADD           = 0x051,
   KEY_SEPARATOR     = 0x052,
   KEY_SUBTRACT      = 0x053,
   KEY_DECIMAL       = 0x054,
   KEY_DIVIDE        = 0x055,
   KEY_NUMPADENTER   = 0x056,

   KEY_F1            = 0x057,
   KEY_F2            = 0x058,
   KEY_F3            = 0x059,
   KEY_F4            = 0x05a,
   KEY_F5            = 0x05b,
   KEY_F6            = 0x05c,
   KEY_F7            = 0x05d,
   KEY_F8            = 0x05e,
   KEY_F9            = 0x05f,
   KEY_F10           = 0x060,
   KEY_F11           = 0x061,
   KEY_F12           = 0x062,
   KEY_F13           = 0x063,
   KEY_F14           = 0x064,
   KEY_F15           = 0x065,
   KEY_F16           = 0x066,
   KEY_F17           = 0x067,
   KEY_F18           = 0x068,
   KEY_F19           = 0x069,
   KEY_F20           = 0x06a,
   KEY_F21           = 0x06b,
   KEY_F22           = 0x06c,
   KEY_F23           = 0x06d,
   KEY_F24           = 0x06e,

   KEY_NUMLOCK       = 0x06f,
   KEY_SCROLLLOCK    = 0x070,
   KEY_LCONTROL      = 0x071,
   KEY_RCONTROL      = 0x072,
   KEY_LALT          = 0x073,
   KEY_RALT          = 0x074,
   KEY_LSHIFT        = 0x075,
   KEY_RSHIFT        = 0x076,
   KEY_WIN_LWINDOW   = 0x077,
   KEY_WIN_RWINDOW   = 0x078,
   KEY_WIN_APPS      = 0x079,
   KEY_OEM_102       = 0x080,

   KEY_MAC_OPT       = 0x090,
   KEY_MAC_LOPT      = 0x091,
   KEY_MAC_ROPT      = 0x092,

   KEY_BUTTON0       = 0x0100,
   KEY_BUTTON1       = 0x0101,
   KEY_BUTTON2       = 0x0102,
   KEY_BUTTON3       = 0x0103,
   KEY_BUTTON4       = 0x0104,
   KEY_BUTTON5       = 0x0105,
   KEY_BUTTON6       = 0x0106,
   KEY_BUTTON7       = 0x0107,
   KEY_BUTTON8       = 0x0108,
   KEY_BUTTON9       = 0x0109,
   KEY_BUTTON10      = 0x010A,
   KEY_BUTTON11      = 0x010B,
   KEY_BUTTON12      = 0x010C,
   KEY_BUTTON13      = 0x010D,
   KEY_BUTTON14      = 0x010E,
   KEY_BUTTON15      = 0x010F,
   KEY_BUTTON16      = 0x0110,
   KEY_BUTTON17      = 0x0111,
   KEY_BUTTON18      = 0x0112,
   KEY_BUTTON19      = 0x0113,
   KEY_BUTTON20      = 0x0114,
   KEY_BUTTON21      = 0x0115,
   KEY_BUTTON22      = 0x0116,
   KEY_BUTTON23      = 0x0117,
   KEY_BUTTON24      = 0x0118,
   KEY_BUTTON25      = 0x0119,
   KEY_BUTTON26      = 0x011A,
   KEY_BUTTON27      = 0x011B,
   KEY_BUTTON28      = 0x011C,
   KEY_BUTTON29      = 0x011D,
   KEY_BUTTON30      = 0x011E,
   KEY_BUTTON31      = 0x011F,
   KEY_BUTTON32      = 0x0120,
   KEY_BUTTON33      = 0x0121,
   KEY_BUTTON34      = 0x0122,
   KEY_BUTTON35      = 0x0123,
   KEY_BUTTON36      = 0x0124,
   KEY_BUTTON37      = 0x0125,
   KEY_BUTTON38      = 0x0126,
   KEY_BUTTON39      = 0x0127,
   KEY_BUTTON40      = 0x0128,
   KEY_BUTTON41      = 0x0129,
   KEY_BUTTON42      = 0x012A,
   KEY_BUTTON43      = 0x012B,
   KEY_BUTTON44      = 0x012C,
   KEY_BUTTON45      = 0x012D,
   KEY_BUTTON46      = 0x012E,
   KEY_BUTTON47      = 0x012F,
   KEY_ANYKEY        = 0xfffe,

   /// Joystick event codes.
   SI_XPOV           = 0x204,
   SI_YPOV           = 0x205,
   SI_UPOV           = 0x206,
   SI_DPOV           = 0x207,
   SI_LPOV           = 0x208,
   SI_RPOV           = 0x209,
   SI_XAXIS          = 0x20B,
   SI_YAXIS          = 0x20C,
   SI_ZAXIS          = 0x20D,
   SI_RXAXIS         = 0x20E,
   SI_RYAXIS         = 0x20F,
   SI_RZAXIS         = 0x210,
   SI_SLIDER         = 0x211,
   SI_XPOV2          = 0x212,
   SI_YPOV2          = 0x213,
   SI_UPOV2          = 0x214,
   SI_DPOV2          = 0x215,
   SI_LPOV2          = 0x216,
   SI_RPOV2          = 0x217,

   XI_CONNECT        = 0x300,
   XI_THUMBLX        = 0x301,
   XI_THUMBLY        = 0x302,
   XI_THUMBRX        = 0x303,
   XI_THUMBRY        = 0x304,
   XI_LEFT_TRIGGER   = 0x305,
   XI_RIGHT_TRIGGER  = 0x306,

   /*XI_DPAD_UP        = 0x307,
   XI_DPAD_DOWN      = 0x308,
   XI_DPAD_LEFT      = 0x309,
   XI_DPAD_RIGHT     = 0x310,*/
   
   XI_START          = 0x311,
   XI_BACK           = 0x312,
   XI_LEFT_THUMB     = 0x313,
   XI_RIGHT_THUMB    = 0x314,
   XI_LEFT_SHOULDER  = 0x315,
   XI_RIGHT_SHOULDER = 0x316,

   XI_A              = 0x317,
   XI_B              = 0x318,
   XI_X              = 0x319,
   XI_Y              = 0x320,

   INPUT_DEVICE_PLUGIN_CODES_START = 0x400,
};

/// Input device types
typedef U32 InputDeviceTypes;
enum InputDeviceTypesEnum
{
   UnknownDeviceType,
   MouseDeviceType,
   KeyboardDeviceType,
   JoystickDeviceType,
   GamepadDeviceType,
   XInputDeviceType,

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

/// Wildcard match used by the input system.
#define SI_ANY       0xff

// Modifier Keys
enum InputModifiers
{
   /// shift and ctrl are the same between platforms.
   SI_LSHIFT = BIT(0),
   SI_RSHIFT = BIT(1),
   SI_SHIFT  = (SI_LSHIFT|SI_RSHIFT),
   SI_LCTRL  = BIT(2),
   SI_RCTRL  = BIT(3),
   SI_CTRL   = (SI_LCTRL|SI_RCTRL),

   /// win altkey, mapped to mac cmdkey.
   SI_LALT = BIT(4),
   SI_RALT = BIT(5),
   SI_ALT = (SI_LALT|SI_RALT),

   /// mac optionkey
   SI_MAC_LOPT  = BIT(6),
   SI_MAC_ROPT  = BIT(7),
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
   InputModifiers modifier;

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

   // Find a virtual map entry's description based on the action code
   const char* findVirtualMapDescFromCode(U32 code);

   /// Build an input event based on a single iValue
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, S32 iValue);

   /// Build an input event based on a single fValue
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, float fValue);

   /// Build an input event based on a Point3F
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, Point3F& pValue);

   /// Build an input event based on a QuatF
   void buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, QuatF& qValue);

protected:
   U32 mNextDeviceTypeCode;
   U32 mNextDeviceCode;

   Vector<IInputDevice*> mDeviceList;

   // Holds description to VirtualMapData struct
   SimpleHashTable<VirtualMapData> mVirtualMap;

   // Used to look up a description based on a VirtualMapData.code
   HashTable<U32, VirtualMapData> mActionCodeMap;

protected:
   void buildVirtualMap();

public:
   // For ManagedSingleton.
   static const char* getSingletonName() { return "InputEventManager"; }   
};

/// Returns the InputEventManager singleton.
#define INPUTMGR ManagedSingleton<InputEventManager>::instance()


#endif
