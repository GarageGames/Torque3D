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

#include "platform/input/event.h"
#include "core/module.h"
#include "core/util/journal/process.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"
#include "platform/platformInput.h"
#include "math/mQuat.h"

MODULE_BEGIN( InputEventManager )

   MODULE_INIT_BEFORE( SIM )
   MODULE_SHUTDOWN_AFTER( SIM )

   MODULE_INIT
   {
      ManagedSingleton< InputEventManager >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< InputEventManager >::deleteSingleton();
   }

MODULE_END;

InputEventManager::InputEventManager()
{
   mNextDeviceTypeCode = INPUT_DEVICE_PLUGIN_DEVICES_START;
   mNextDeviceCode = INPUT_DEVICE_PLUGIN_CODES_START;

   buildVirtualMap();
}

InputEventManager::~InputEventManager()
{
}

U32 InputEventManager::getNextDeviceType()
{
   U32 code = mNextDeviceTypeCode;
   ++mNextDeviceTypeCode;
   return code;
}

U32 InputEventManager::getNextDeviceCode()
{
   U32 code = mNextDeviceCode;
   ++mNextDeviceCode;
   return code;
}

void InputEventManager::registerDevice(IInputDevice* device)
{
   // Make sure the device is not already registered
   for(U32 i=0; i<mDeviceList.size(); ++i)
   {
      if(mDeviceList[i] == device)
         return;
   }

   // Add the new device to the list
   mDeviceList.push_back(device);
}

void InputEventManager::unregisterDevice(IInputDevice* device)
{
   // Remove the device from the list
   for(U32 i=0; i<mDeviceList.size(); ++i)
   {
      if(mDeviceList[i] == device)
      {
         mDeviceList.erase(i);
         return;
      }
   }
}

bool InputEventManager::isRegisteredDevice(const char* name)
{
   for(Vector<IInputDevice*>::iterator itr = mDeviceList.begin(); itr != mDeviceList.end(); ++itr)
   {
      if((*itr)->isEnabled())
      {
         const char* deviceName = (*itr)->getDeviceName();
         if(dStrnicmp(name, deviceName, dStrlen(deviceName)) == 0)
         {
            return true;
         }
      }
   }

   return false;
}

bool InputEventManager::isRegisteredDevice(U32 type)
{
   for(Vector<IInputDevice*>::iterator itr = mDeviceList.begin(); itr != mDeviceList.end(); ++itr)
   {
      if((*itr)->isEnabled())
      {
         U32 deviceType = (*itr)->getDeviceType();
         if(deviceType == type)
         {
            return true;
         }
      }
   }

   return false;
}

bool InputEventManager::isRegisteredDeviceWithAttributes(const char* name, U32& deviceType, U32&nameLen)
{
   for(Vector<IInputDevice*>::iterator itr = mDeviceList.begin(); itr != mDeviceList.end(); ++itr)
   {
      if((*itr)->isEnabled())
      {
         const char* deviceName = (*itr)->getDeviceName();
         S32 length = dStrlen(deviceName);
         if(dStrnicmp(name, deviceName, length) == 0)
         {
            deviceType = (*itr)->getDeviceType();
            nameLen = length;
            return true;
         }
      }
   }

   return false;
}

const char* InputEventManager::getRegisteredDeviceName(U32 type)
{
   for(Vector<IInputDevice*>::iterator itr = mDeviceList.begin(); itr != mDeviceList.end(); ++itr)
   {
      if((*itr)->isEnabled())
      {
         U32 deviceType = (*itr)->getDeviceType();
         if(deviceType == type)
         {
            return (*itr)->getDeviceName();
         }
      }
   }

   return NULL;
}

void InputEventManager::start()
{
   Process::notify(this, &InputEventManager::process, PROCESS_INPUT_ORDER);
}

void InputEventManager::stop()
{
   Process::remove(this, &InputEventManager::process);
}

void InputEventManager::process()
{
   // Process each device
   for(Vector<IInputDevice*>::iterator itr = mDeviceList.begin(); itr != mDeviceList.end(); ++itr)
   {
      if((*itr)->isEnabled())
      {
         (*itr)->process();
      }
   }
}

// Used for the old virtual map table that was originally in actionMap.cpp
struct CodeMapping
{
   const char* pDescription;
   InputEventType       type;
   InputObjectInstances code;
};

CodeMapping gVirtualMap[] =
{
   //-------------------------------------- KEYBOARD EVENTS
   //
   { "backspace",     SI_KEY,    KEY_BACKSPACE   },
   { "tab",           SI_KEY,    KEY_TAB         },

   { "return",        SI_KEY,    KEY_RETURN      },
   { "enter",         SI_KEY,    KEY_RETURN      },

   { "shift",         SI_KEY,    KEY_SHIFT       },
   { "ctrl",          SI_KEY,    KEY_CONTROL     },
   { "alt",           SI_KEY,    KEY_ALT         },
   { "pause",         SI_KEY,    KEY_PAUSE       },
   { "capslock",      SI_KEY,    KEY_CAPSLOCK    },

   { "escape",        SI_KEY,    KEY_ESCAPE      },

   { "space",         SI_KEY,    KEY_SPACE       },
   { "pagedown",      SI_KEY,    KEY_PAGE_DOWN   },
   { "pageup",        SI_KEY,    KEY_PAGE_UP     },
   { "end",           SI_KEY,    KEY_END         },
   { "home",          SI_KEY,    KEY_HOME        },
   { "left",          SI_KEY,    KEY_LEFT        },
   { "up",            SI_KEY,    KEY_UP          },
   { "right",         SI_KEY,    KEY_RIGHT       },
   { "down",          SI_KEY,    KEY_DOWN        },
   { "print",         SI_KEY,    KEY_PRINT       },
   { "insert",        SI_KEY,    KEY_INSERT      },
   { "delete",        SI_KEY,    KEY_DELETE      },
   { "help",          SI_KEY,    KEY_HELP        },

   { "win_lwindow",   SI_KEY,    KEY_WIN_LWINDOW },
   { "win_rwindow",   SI_KEY,    KEY_WIN_RWINDOW },
   { "win_apps",      SI_KEY,    KEY_WIN_APPS    },

   { "cmd",           SI_KEY,    KEY_ALT         },
   { "opt",           SI_KEY,    KEY_MAC_OPT     },
   { "lopt",          SI_KEY,    KEY_MAC_LOPT    },
   { "ropt",          SI_KEY,    KEY_MAC_ROPT    },

   { "numpad0",       SI_KEY,    KEY_NUMPAD0     },
   { "numpad1",       SI_KEY,    KEY_NUMPAD1     },
   { "numpad2",       SI_KEY,    KEY_NUMPAD2     },
   { "numpad3",       SI_KEY,    KEY_NUMPAD3     },
   { "numpad4",       SI_KEY,    KEY_NUMPAD4     },
   { "numpad5",       SI_KEY,    KEY_NUMPAD5     },
   { "numpad6",       SI_KEY,    KEY_NUMPAD6     },
   { "numpad7",       SI_KEY,    KEY_NUMPAD7     },
   { "numpad8",       SI_KEY,    KEY_NUMPAD8     },
   { "numpad9",       SI_KEY,    KEY_NUMPAD9     },
   { "numpadmult",    SI_KEY,    KEY_MULTIPLY    },
   { "numpadadd",     SI_KEY,    KEY_ADD         },
   { "numpadsep",     SI_KEY,    KEY_SEPARATOR   },
   { "numpadminus",   SI_KEY,    KEY_SUBTRACT    },
   { "numpaddecimal", SI_KEY,    KEY_DECIMAL     },
   { "numpaddivide",  SI_KEY,    KEY_DIVIDE      },
   { "numpadenter",   SI_KEY,    KEY_NUMPADENTER },

   { "f1",            SI_KEY,    KEY_F1          },
   { "f2",            SI_KEY,    KEY_F2          },
   { "f3",            SI_KEY,    KEY_F3          },
   { "f4",            SI_KEY,    KEY_F4          },
   { "f5",            SI_KEY,    KEY_F5          },
   { "f6",            SI_KEY,    KEY_F6          },
   { "f7",            SI_KEY,    KEY_F7          },
   { "f8",            SI_KEY,    KEY_F8          },
   { "f9",            SI_KEY,    KEY_F9          },
   { "f10",           SI_KEY,    KEY_F10         },
   { "f11",           SI_KEY,    KEY_F11         },
   { "f12",           SI_KEY,    KEY_F12         },
   { "f13",           SI_KEY,    KEY_F13         },
   { "f14",           SI_KEY,    KEY_F14         },
   { "f15",           SI_KEY,    KEY_F15         },
   { "f16",           SI_KEY,    KEY_F16         },
   { "f17",           SI_KEY,    KEY_F17         },
   { "f18",           SI_KEY,    KEY_F18         },
   { "f19",           SI_KEY,    KEY_F19         },
   { "f20",           SI_KEY,    KEY_F20         },
   { "f21",           SI_KEY,    KEY_F21         },
   { "f22",           SI_KEY,    KEY_F22         },
   { "f23",           SI_KEY,    KEY_F23         },
   { "f24",           SI_KEY,    KEY_F24         },

   { "numlock",       SI_KEY,    KEY_NUMLOCK     },
   { "scrolllock",    SI_KEY,    KEY_SCROLLLOCK  },

   { "lshift",        SI_KEY,    KEY_LSHIFT      },
   { "rshift",        SI_KEY,    KEY_RSHIFT      },
   { "lcontrol",      SI_KEY,    KEY_LCONTROL    },
   { "rcontrol",      SI_KEY,    KEY_RCONTROL    },
   { "lalt",          SI_KEY,    KEY_LALT        },
   { "ralt",          SI_KEY,    KEY_RALT        },
   { "tilde",         SI_KEY,    KEY_TILDE       },

   { "minus",         SI_KEY,    KEY_MINUS       },
   { "equals",        SI_KEY,    KEY_EQUALS      },
   { "lbracket",      SI_KEY,    KEY_LBRACKET    },
   { "rbracket",      SI_KEY,    KEY_RBRACKET    },
   { "backslash",     SI_KEY,    KEY_BACKSLASH   },
   { "semicolon",     SI_KEY,    KEY_SEMICOLON   },
   { "apostrophe",    SI_KEY,    KEY_APOSTROPHE  },
   { "comma",         SI_KEY,    KEY_COMMA       },
   { "period",        SI_KEY,    KEY_PERIOD      },
   { "slash",         SI_KEY,    KEY_SLASH       },
   { "lessthan",      SI_KEY,    KEY_OEM_102     },

   //-------------------------------------- BUTTON EVENTS
   // Joystick/Mouse buttons
   { "button0",       SI_BUTTON, KEY_BUTTON0    },
   { "button1",       SI_BUTTON, KEY_BUTTON1    },
   { "button2",       SI_BUTTON, KEY_BUTTON2    },
   { "button3",       SI_BUTTON, KEY_BUTTON3    },
   { "button4",       SI_BUTTON, KEY_BUTTON4    },
   { "button5",       SI_BUTTON, KEY_BUTTON5    },
   { "button6",       SI_BUTTON, KEY_BUTTON6    },
   { "button7",       SI_BUTTON, KEY_BUTTON7    },
   { "button8",       SI_BUTTON, KEY_BUTTON8    },
   { "button9",       SI_BUTTON, KEY_BUTTON9    },
   { "button10",      SI_BUTTON, KEY_BUTTON10   },
   { "button11",      SI_BUTTON, KEY_BUTTON11   },
   { "button12",      SI_BUTTON, KEY_BUTTON12   },
   { "button13",      SI_BUTTON, KEY_BUTTON13   },
   { "button14",      SI_BUTTON, KEY_BUTTON14   },
   { "button15",      SI_BUTTON, KEY_BUTTON15   },
   { "button16",      SI_BUTTON, KEY_BUTTON16   },
   { "button17",      SI_BUTTON, KEY_BUTTON17   },
   { "button18",      SI_BUTTON, KEY_BUTTON18   },
   { "button19",      SI_BUTTON, KEY_BUTTON19   },
   { "button20",      SI_BUTTON, KEY_BUTTON20   },
   { "button21",      SI_BUTTON, KEY_BUTTON21   },
   { "button22",      SI_BUTTON, KEY_BUTTON22   },
   { "button23",      SI_BUTTON, KEY_BUTTON23   },
   { "button24",      SI_BUTTON, KEY_BUTTON24   },
   { "button25",      SI_BUTTON, KEY_BUTTON25   },
   { "button26",      SI_BUTTON, KEY_BUTTON26   },
   { "button27",      SI_BUTTON, KEY_BUTTON27   },
   { "button28",      SI_BUTTON, KEY_BUTTON28   },
   { "button29",      SI_BUTTON, KEY_BUTTON29   },
   { "button30",      SI_BUTTON, KEY_BUTTON30   },
   { "button31",      SI_BUTTON, KEY_BUTTON31   },
   { "button32",      SI_BUTTON, KEY_BUTTON32   },
   { "button33",      SI_BUTTON, KEY_BUTTON33   },
   { "button34",      SI_BUTTON, KEY_BUTTON34   },
   { "button35",      SI_BUTTON, KEY_BUTTON35   },
   { "button36",      SI_BUTTON, KEY_BUTTON36   },
   { "button37",      SI_BUTTON, KEY_BUTTON37   },
   { "button38",      SI_BUTTON, KEY_BUTTON38   },
   { "button39",      SI_BUTTON, KEY_BUTTON39   },
   { "button40",      SI_BUTTON, KEY_BUTTON40   },
   { "button41",      SI_BUTTON, KEY_BUTTON41   },
   { "button42",      SI_BUTTON, KEY_BUTTON42   },
   { "button43",      SI_BUTTON, KEY_BUTTON43   },
   { "button44",      SI_BUTTON, KEY_BUTTON44   },
   { "button45",      SI_BUTTON, KEY_BUTTON45   },
   { "button46",      SI_BUTTON, KEY_BUTTON46   },
   { "button47",      SI_BUTTON, KEY_BUTTON47   },

   //-------------------------------------- MOVE EVENTS
   // Mouse/Joystick axes:
   { "xaxis",         SI_AXIS,   SI_XAXIS       },
   { "yaxis",         SI_AXIS,   SI_YAXIS       },
   { "zaxis",         SI_AXIS,   SI_ZAXIS       },
   { "rxaxis",        SI_AXIS,   SI_RXAXIS      },
   { "ryaxis",        SI_AXIS,   SI_RYAXIS      },
   { "rzaxis",        SI_AXIS,   SI_RZAXIS      },
   { "slider",        SI_AXIS,   SI_SLIDER      },

   //-------------------------------------- POV EVENTS
   // Joystick POV:
   { "xpov",          SI_POV,    SI_XPOV        },
   { "ypov",          SI_POV,    SI_YPOV        },
   { "upov",          SI_POV,    SI_UPOV        },
   { "dpov",          SI_POV,    SI_DPOV        },
   { "lpov",          SI_POV,    SI_LPOV        },
   { "rpov",          SI_POV,    SI_RPOV        },
   { "xpov2",         SI_POV,    SI_XPOV2       },
   { "ypov2",         SI_POV,    SI_YPOV2       },
   { "upov2",         SI_POV,    SI_UPOV2       },
   { "dpov2",         SI_POV,    SI_DPOV2       },
   { "lpov2",         SI_POV,    SI_LPOV2       },
   { "rpov2",         SI_POV,    SI_RPOV2       },

#if defined( TORQUE_OS_WIN32 ) || defined( TORQUE_OS_XENON )
   //-------------------------------------- XINPUT EVENTS
   // Controller connect / disconnect:
   { "connect",       SI_BUTTON, XI_CONNECT     },
   
   // L & R Thumbsticks:
   { "thumblx",       SI_AXIS,   XI_THUMBLX     },
   { "thumbly",       SI_AXIS,   XI_THUMBLY     },
   { "thumbrx",       SI_AXIS,   XI_THUMBRX     },
   { "thumbry",       SI_AXIS,   XI_THUMBRY     },

   // L & R Triggers:
   { "triggerl",      SI_AXIS,   XI_LEFT_TRIGGER  },
   { "triggerr",      SI_AXIS,   XI_RIGHT_TRIGGER },

   // DPAD Buttons:
   { "dpadu",         SI_BUTTON, SI_UPOV     },
   { "dpadd",         SI_BUTTON, SI_DPOV   },
   { "dpadl",         SI_BUTTON, SI_LPOV   },
   { "dpadr",         SI_BUTTON, SI_RPOV  },

   // START & BACK Buttons:
   { "btn_start",     SI_BUTTON, XI_START       },
   { "btn_back",      SI_BUTTON, XI_BACK        },

   // L & R Thumbstick Buttons:
   { "btn_lt",        SI_BUTTON, XI_LEFT_THUMB  },
   { "btn_rt",        SI_BUTTON, XI_RIGHT_THUMB },

   // L & R Shoulder Buttons:
   { "btn_l",         SI_BUTTON, XI_LEFT_SHOULDER  },
   { "btn_r",         SI_BUTTON, XI_RIGHT_SHOULDER },

   // Primary buttons:
   { "btn_a",         SI_BUTTON, XI_A           },
   { "btn_b",         SI_BUTTON, XI_B           },
   { "btn_x",         SI_BUTTON, XI_X           },
   { "btn_y",         SI_BUTTON, XI_Y           },
#endif

   //-------------------------------------- MISCELLANEOUS EVENTS
   //

   { "anykey",        SI_KEY,      KEY_ANYKEY },
   { "nomatch",       SI_UNKNOWN,  (InputObjectInstances)0xFFFFFFFF }
};

void InputEventManager::buildVirtualMap()
{
   char desc[256];
   VirtualMapData* data;

   for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
   {
      // Make sure the description is lower case
      desc[0] = 0;
      dStrncpy(desc, gVirtualMap[j].pDescription, 255);
      dStrlwr(desc);

      data = new VirtualMapData();
      data->type = gVirtualMap[j].type;
      data->code = gVirtualMap[j].code;
      data->desc = StringTable->insert(desc);

      mVirtualMap.insert(data, desc);
      mActionCodeMap.insertUnique(data->code, *data);
   }
}

void InputEventManager::addVirtualMap(const char* description, InputEventType type, InputObjectInstances code)
{
   // Make sure the description is lower case
   char desc[256];
   desc[0] = 0;
   dStrncpy(desc, description, 255);
   dStrlwr(desc);

   VirtualMapData* data = new VirtualMapData();
   data->type = type;
   data->code = code;
   data->desc = StringTable->insert(desc);

   mVirtualMap.insert(data, desc);
   mActionCodeMap.insertUnique(data->code, *data);
}

InputEventManager::VirtualMapData* InputEventManager::findVirtualMap(const char* description)
{
   char desc[256];
   desc[0] = 0;
   dStrncpy(desc, description, 255);
   dStrlwr(desc);

   return mVirtualMap.retreive(desc);
}

const char* InputEventManager::findVirtualMapDescFromCode(U32 code)
{
   HashTable<U32, VirtualMapData>::Iterator itr = mActionCodeMap.find(code);
   if(itr != mActionCodeMap.end())
      return itr->value.desc;

   return NULL;
}

void InputEventManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, S32 iValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType  = deviceType;
   newEvent.deviceInst  = deviceInst;
   newEvent.objType  = objType;
   newEvent.objInst  = objInst;
   newEvent.action   = action;
   newEvent.iValue   = iValue;

   newEvent.postToSignal(Input::smInputEvent);
}

void InputEventManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, float fValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType  = deviceType;
   newEvent.deviceInst  = deviceInst;
   newEvent.objType  = objType;
   newEvent.objInst  = objInst;
   newEvent.action   = action;
   newEvent.fValue   = fValue;

   newEvent.postToSignal(Input::smInputEvent);
}

void InputEventManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, Point3F& pValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType  = deviceType;
   newEvent.deviceInst  = deviceInst;
   newEvent.objType  = objType;
   newEvent.objInst  = objInst;
   newEvent.action   = action;
   newEvent.fValue   = pValue.x;
   newEvent.fValue2  = pValue.y;
   newEvent.fValue3  = pValue.z;

   newEvent.postToSignal(Input::smInputEvent);
}

void InputEventManager::buildInputEvent(U32 deviceType, U32 deviceInst, InputEventType objType, InputObjectInstances objInst, InputActionType action, QuatF& qValue)
{
   InputEventInfo newEvent;

   newEvent.deviceType  = deviceType;
   newEvent.deviceInst  = deviceInst;
   newEvent.objType  = objType;
   newEvent.objInst  = objInst;
   newEvent.action   = action;
   newEvent.fValue   = qValue.x;
   newEvent.fValue2  = qValue.y;
   newEvent.fValue3  = qValue.z;
   newEvent.fValue4  = qValue.w;

   newEvent.postToSignal(Input::smInputEvent);
}
