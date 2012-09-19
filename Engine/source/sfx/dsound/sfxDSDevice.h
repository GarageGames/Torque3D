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

#ifndef _SFXDSDEVICE_H_
#define _SFXDSDEVICE_H_

#ifndef _STRINGFUNCTIONS_H_
#  include "core/strings/stringFunctions.h"
#endif
#ifndef _SFXDEVICE_H_
   #include "sfx/sfxDevice.h"
#endif
#ifndef _SFXDSVOICE_H_
   #include "sfx/dsound/sfxDSVoice.h"
#endif
#ifndef OS_DLIBRARY_H
   #include "platform/platformDlibrary.h"
#endif

#include <dsound.h>

// Typedefs
#define DS_FUNCTION(fn_name, fn_return, fn_args) \
   typedef fn_return (WINAPI *DSFNPTR##fn_name##)##fn_args##;
#include "sfx/dsound/dsFunctions.h"
#undef DS_FUNCTION

// Function table
struct DSoundFNTable
{
   DSoundFNTable() : isLoaded( false ) {};
   bool isLoaded;
   DLibraryRef dllRef;

#define DS_FUNCTION(fn_name, fn_return, fn_args) \
   DSFNPTR##fn_name fn_name;
#include "sfx/dsound/dsFunctions.h"
#undef DS_FUNCTION
};


/// Helper for asserting on dsound HRESULTS.
inline void DSAssert( HRESULT hr, const char *info ) 
{
   #ifdef TORQUE_DEBUG

      if( FAILED( hr ) ) 
      {
         char buf[256];
         dSprintf( buf, 256, "Error code: %x\n%s", hr, info );
         AssertFatal( false, buf );
      }

   #endif
}


/// The DirectSound device implementation exposes a couple
/// of settings to script that you should be aware of:
///
///   $DirectSound::dopplerFactor - This controls the scale of 
///   the doppler effect.  Valid factors are 0.0 to 10.0 and it
///   defaults to 0.75.
///
///   $DirectSound::distanceFactor - This sets the unit conversion
///   for 
///
///   $DirectSound::rolloffFactor - ;
///
///
class SFXDSDevice : public SFXDevice
{
      typedef SFXDevice Parent;
   
      friend class SFXDSVoice;
      friend class SFXDSProvider; // _init

   public:

      //explicit SFXDSDevice();

      SFXDSDevice(   SFXProvider* provider, 
                     DSoundFNTable *dsFnTbl, 
                     GUID* guid, 
                     String name, 
                     bool useHardware, 
                     S32 maxBuffers );

      virtual ~SFXDSDevice();

   protected:

      IDirectSound8 *mDSound;

      IDirectSound3DListener8 *mListener;

      IDirectSoundBuffer *mPrimaryBuffer;

      DSoundFNTable *mDSoundTbl;

      DSCAPS mCaps;

      GUID* mGUID;

      bool _init();

      /// Called from SFXDSVoice to commit any deferred
      /// settings before playback begins.
      void _commitDeferred();

   public:

      // SFXDevice
      virtual SFXBuffer* createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      virtual SFXVoice* createVoice( bool is3D, SFXBuffer *buffer );
      virtual void update();
      virtual void setListener( U32 index, const SFXListenerProperties& listener );
      virtual void setDistanceModel( SFXDistanceModel mode );
      virtual void setDopplerFactor( F32 factor );
      virtual void setRolloffFactor( F32 factor );
};

#endif // _SFXDSDEVICE_H_
