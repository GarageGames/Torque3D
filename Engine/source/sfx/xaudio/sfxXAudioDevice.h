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

#ifndef _SFXXAUDIODEVICE_H_
#define _SFXXAUDIODEVICE_H_

class SFXProvider;

#ifndef _SFXDEVICE_H_
#include "sfx/sfxDevice.h"
#endif

#ifndef _SFXPROVIDER_H_
#include "sfx/sfxProvider.h"
#endif

#ifndef _SFXXAUDIOVOICE_H_
#include "sfx/xaudio/sfxXAudioVoice.h"
#endif

#ifndef _SFXXAUDIOBUFFER_H_
#include "sfx/xaudio/sfxXAudioBuffer.h"
#endif

#include <xaudio2.h>
#include <x3daudio.h>


class SFXXAudioDevice : public SFXDevice
{
   public:

      typedef SFXDevice Parent;
      friend class SFXXAudioVoice; // mXAudio

   protected:

      /// The XAudio engine interface passed 
      /// on creation from the provider.
      IXAudio2 *mXAudio;

      /// The X3DAudio instance.
      X3DAUDIO_HANDLE mX3DAudio;

      /// The one and only mastering voice.
      IXAudio2MasteringVoice* mMasterVoice;

      /// The details of the master voice.
      XAUDIO2_VOICE_DETAILS mMasterVoiceDetails;

      /// The one listener.
      X3DAUDIO_LISTENER mListener;

      SFXDistanceModel mDistanceModel;
      F32 mRolloffFactor;
      F32 mDopplerFactor;

   public:

      SFXXAudioDevice(  SFXProvider* provider, 
                        const String& name,
                        IXAudio2 *xaudio,
                        U32 deviceIndex,
                        U32 speakerChannelMask,
                        U32 maxBuffers );

      virtual ~SFXXAudioDevice();

      // SFXDevice
      virtual SFXBuffer* createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      virtual SFXVoice* createVoice( bool is3D, SFXBuffer *buffer );
      virtual void update();
      virtual void setListener( U32 index, const SFXListenerProperties& listener );
      virtual void setDistanceModel( SFXDistanceModel model );
      virtual void setRolloffFactor( F32 factor );
      virtual void setDopplerFactor( F32 factor );

      /// Called from the voice when its about to start playback.
      void _setOutputMatrix( SFXXAudioVoice *voice );
};

#endif // _SFXXAUDIODEVICE_H_