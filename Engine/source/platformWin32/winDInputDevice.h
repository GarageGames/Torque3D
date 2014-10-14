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

#ifndef _WINDINPUTDEVICE_H_
#define _WINDINPUTDEVICE_H_

#ifndef _PLATFORMWIN32_H_
#include "platformWin32/platformWin32.h"
#endif
#ifndef _PLATFORMINPUT_H_
#include "platform/platformInput.h"
#endif

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


class DInputDevice : public InputDevice
{
   public:
      static LPDIRECTINPUT8 smDInputInterface;

   protected:
      enum Constants
      {
         QUEUED_BUFFER_SIZE   = 128,

         SIZEOF_BUTTON = 1,                  // size of an object's data in bytes
         SIZEOF_KEY    = 1,
         SIZEOF_AXIS   = 4,
         SIZEOF_POV    = 4,
      };

      static U8   smDeviceCount[ NUM_INPUT_DEVICE_TYPES ];
      static bool smInitialized;

      /// Are we an XInput device?
      bool mIsXInput;

      //--------------------------------------
      LPDIRECTINPUTDEVICE8 mDevice;
      DIDEVICEINSTANCE     mDeviceInstance;
      DIDEVCAPS            mDeviceCaps;
      U8                   mDeviceType;
      U8                   mDeviceID;

      bool                 mAcquired;
      bool                 mNeedSync;

      LPDIRECTINPUTEFFECT  mForceFeedbackEffect;   ///< Holds our DirectInput FF Effect
      DWORD                mNumForceFeedbackAxes;  ///< # axes (we only support 0, 1, or 2
      DWORD                mForceFeedbackAxes[2];  ///< Force Feedback axes offsets into DIOBJECTFORMAT

      //--------------------------------------
      DIDEVICEOBJECTINSTANCE* mObjInstance;
      DIOBJECTDATAFORMAT*     mObjFormat;
      ObjInfo*                mObjInfo;
      U8*                     mObjBuffer1;    // polled device input buffers
      U8*                     mObjBuffer2;
      U8*                     mPrevObjBuffer; // points to buffer 1 or 2

      // Hack for POV
      S32 mPrevPOVPos;

      U32 mObjBufferSize;                     // size of objBuffer*
      U32 mObjCount;                          // number of objects on this device
      U32 mObjEnumCount;                      // used during enumeration ONLY
      U32 mObjBufferOfs;                      // used during enumeration ONLY

      static BOOL CALLBACK EnumObjectsProc( const DIDEVICEOBJECTINSTANCE *doi, LPVOID pvRef );

      bool enumerateObjects();
      bool processAsync();
      bool processImmediate();

      DWORD findObjInstance( DWORD offset );
      bool  buildEvent( DWORD offset, S32 newData, S32 oldData );

   public:
      DInputDevice( const DIDEVICEINSTANCE* deviceInst );
      ~DInputDevice();

      static void init();
 
      bool create();
      void destroy();

      bool acquire();
      bool unacquire();

      bool isAcquired();
      bool isPolled();

      U8 getDeviceType();
      U8 getDeviceID();

      const char* getName();
      const char* getProductName();

      // Constant Effect Force Feedback
      void rumble( F32 x, F32 y );

      // Console interface functions:
      const char* getJoystickAxesString();
      static bool joystickDetected();
      //

      bool process();
};

//------------------------------------------------------------------------------
inline bool DInputDevice::isAcquired()
{
   return mAcquired;
}

//------------------------------------------------------------------------------
inline bool DInputDevice::isPolled()
{
   //return true;
   return ( mDeviceCaps.dwFlags & DIDC_POLLEDDEVICE ) != 0;
}

//------------------------------------------------------------------------------
inline U8 DInputDevice::getDeviceType()
{
   return mDeviceType;
}

//------------------------------------------------------------------------------
inline U8 DInputDevice::getDeviceID()
{
   return mDeviceID;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
InputObjectInstances DIK_to_Key( U8 dikCode );
U8  Key_to_DIK( U16 keyCode );
#endif // _H_WINDINPUTDEVICE_
