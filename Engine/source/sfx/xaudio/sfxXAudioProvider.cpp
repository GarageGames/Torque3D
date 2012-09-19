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

// Note:  This must be defined before platform.h so that
// CoInitializeEx is properly included.
#define _WIN32_DCOM
#include <xaudio2.h>

#include "sfx/xaudio/sfxXAudioDevice.h"
#include "sfx/sfxProvider.h"
#include "core/util/safeRelease.h"
#include "core/strings/unicode.h"
#include "core/strings/stringFunctions.h"
#include "console/console.h"
#include "core/module.h"


class SFXXAudioProvider : public SFXProvider
{
public:

   SFXXAudioProvider()
      : SFXProvider( "XAudio" ) {}
   virtual ~SFXXAudioProvider();

protected:

   /// Extended SFXDeviceInfo to also store some 
   /// extra XAudio specific data.
   struct XADeviceInfo : SFXDeviceInfo
   {
      UINT32 deviceIndex;

      XAUDIO2_DEVICE_ROLE role;

      WAVEFORMATEXTENSIBLE format;
   };

   /// Helper for creating the XAudio engine.
   static bool _createXAudio( IXAudio2 **xaudio );

public:

   // SFXProvider
   void init();
   SFXDevice* createDevice( const String& deviceName, bool useHardware, S32 maxBuffers );

};

MODULE_BEGIN( XAudio )

   MODULE_INIT_BEFORE( SFX )
   MODULE_SHUTDOWN_AFTER( SFX )
   
   SFXXAudioProvider* mProvider;
   
   MODULE_INIT
   {
      mProvider = new SFXXAudioProvider;
   }
   
   MODULE_SHUTDOWN
   {
      delete mProvider;
   }

MODULE_END;

SFXXAudioProvider::~SFXXAudioProvider()
{
}

void SFXXAudioProvider::init()
{
   // Create a temp XAudio object for device enumeration.
   IXAudio2 *xAudio = NULL;
   if ( !_createXAudio( &xAudio ) )
   {
      Con::errorf( "SFXXAudioProvider::init() - XAudio2 failed to load!" );
      return;
   }

   // Add the devices to the info list.
   UINT32 count = 0;
   xAudio->GetDeviceCount( &count );
   for ( UINT32 i = 0; i < count; i++ )
   {
      XAUDIO2_DEVICE_DETAILS details;
      HRESULT hr = xAudio->GetDeviceDetails( i, &details );
      if ( FAILED( hr ) )
         continue;

      // Add a device to the info list.
      XADeviceInfo* info = new XADeviceInfo;
      info->deviceIndex = i;
      info->driver = String( "XAudio" );
      info->name = String( details.DisplayName );
      info->hasHardware = false;
      info->maxBuffers = 64;
      info->role = details.Role;
      info->format = details.OutputFormat;
      mDeviceInfo.push_back( info );
   }

   // We're done with XAudio for now.
   SAFE_RELEASE( xAudio );

   // If we have no devices... we're done.
   if ( mDeviceInfo.empty() )
   {
      Con::errorf( "SFXXAudioProvider::init() - No valid XAudio2 devices found!" );
      return;
   }

   // If we got this far then we should be able to
   // safely create a device for XAudio.
   regProvider( this );
}

bool SFXXAudioProvider::_createXAudio( IXAudio2 **xaudio )
{
   // In debug builds enable the debug version 
   // of the XAudio engine.
   #ifdef TORQUE_DEBUG
      #define XAUDIO_FLAGS XAUDIO2_DEBUG_ENGINE
   #else
      #define XAUDIO_FLAGS 0
   #endif

#ifndef TORQUE_OS_XENON
   // This must be called first... it doesn't hurt to 
   // call it more than once.
   CoInitialize( NULL );
#endif

   // Try creating the xaudio engine.
   HRESULT hr = XAudio2Create( xaudio, XAUDIO_FLAGS, XAUDIO2_DEFAULT_PROCESSOR );

   return SUCCEEDED( hr ) && (*xaudio);
}

SFXDevice* SFXXAudioProvider::createDevice( const String& deviceName, bool useHardware, S32 maxBuffers )
{
   String devName;

   // On the 360, ignore what the prefs say, and create the only audio device
#ifndef TORQUE_OS_XENON
   devName = deviceName;
#endif

   XADeviceInfo* info = dynamic_cast< XADeviceInfo* >( _findDeviceInfo( devName ) );

   // Do we find one to create?
   if ( info )
   {
      // Create the XAudio object to pass to the device.
      IXAudio2 *xAudio = NULL;
      if ( !_createXAudio( &xAudio ) )
      {
         Con::errorf( "SFXXAudioProvider::createDevice() - XAudio2 failed to load!" );
         return NULL;
      }

      return new SFXXAudioDevice(   this,
                                    devName,
                                    xAudio, 
                                    info->deviceIndex,
                                    info->format.dwChannelMask,
                                    maxBuffers );
   }

   // We didn't find a matching valid device.
   return NULL;
}
