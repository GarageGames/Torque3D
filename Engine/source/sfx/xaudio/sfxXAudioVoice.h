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

#ifndef _SFXXAUDIOVOICE_H_
#define _SFXXAUDIOVOICE_H_

#include <xaudio2.h>
#include <x3daudio.h>

#include "sfx/sfxVoice.h"


class SFXXAudioBuffer;


class SFXXAudioVoice :  public SFXVoice,
                        public IXAudio2VoiceCallback
{
   public:

      typedef SFXVoice Parent;

      friend class SFXXAudioDevice;
      friend class SFXXAudioBuffer;

   protected:

      /// This constructor does not create a valid voice.
      /// @see SFXXAudioVoice::create
      SFXXAudioVoice( SFXXAudioBuffer* buffer );

      /// The device that created us.
      SFXXAudioDevice *mXAudioDevice;

      /// The XAudio voice.
      IXAudio2SourceVoice *mXAudioVoice;

      ///
      CRITICAL_SECTION mLock;

      /// Used to know what sounds need positional updates.
      bool mIs3D;

      /// Whether the voice has stopped playing.
      mutable bool mHasStopped;

      /// Whether we have started playback.
      bool mHasStarted;

      /// Whether playback is currently running.
      bool mIsPlaying;

      /// Whether playback is looping.
      bool mIsLooping;

      /// Since 3D sounds are pitch shifted for doppler
      /// effect we need to track the users base pitch.
      F32 mPitch;

      /// The cached X3DAudio emitter data.
      X3DAUDIO_EMITTER mEmitter;

      /// XAudio does not reset the SamplesPlayed count as is stated in the docs.  To work around
      /// that, we offset the values reported by XAudio through this field.
      S32 mSamplesPlayedOffset;

      /// @name Data for Non-Streaming Voices
      /// @{

      /// Whether we have loaded our non-streaming data.  We use an explicit
      /// flag here as we really can't rely on XAUDIO2_VOICE_STATE.
      bool mNonStreamBufferLoaded;

      /// Audio buffer for non-streaming voice.
      XAUDIO2_BUFFER mNonStreamBuffer;

      /// Sample to start playing at when seting up #mNonStreamBuffer.
      U32 mNonStreamSampleStartPos;

      /// @}

      // IXAudio2VoiceCallback
      void STDMETHODCALLTYPE OnStreamEnd();      
      void STDMETHODCALLTYPE OnVoiceProcessingPassStart( UINT32 BytesRequired ) {}
      void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() {}
      void STDMETHODCALLTYPE OnBufferEnd( void *bufferContext );
      void STDMETHODCALLTYPE OnBufferStart( void *bufferContext ) {}
      void STDMETHODCALLTYPE OnLoopEnd( void *bufferContext ) {}
      void STDMETHODCALLTYPE OnVoiceError( void * bufferContext, HRESULT error ) {}

      /// @deprecated This is only here for compatibility with
      /// the March 2008 SDK version of IXAudio2VoiceCallback.
      void STDMETHODCALLTYPE OnVoiceProcessingPassStart() {} 

      void _flush();
      void _loadNonStreamed();

      // SFXVoice.
      virtual SFXStatus _status() const;
      virtual void _play();
      virtual void _pause();
      virtual void _stop();
      virtual void _seek( U32 sample );
      virtual U32 _tell() const;

      SFXXAudioBuffer* _getBuffer() const { return ( SFXXAudioBuffer* ) mBuffer.getPointer(); }

   public:

      ///
      static SFXXAudioVoice* create(   IXAudio2 *xaudio,
                                       bool is3D,
                                       SFXXAudioBuffer *buffer,
                                       SFXXAudioVoice* inVoice = NULL );

      ///
      virtual ~SFXXAudioVoice();

      // SFXVoice
      void setMinMaxDistance( F32 min, F32 max );
      void play( bool looping );
      void setVelocity( const VectorF& velocity );
      void setTransform( const MatrixF& transform );
      void setVolume( F32 volume );
      void setPitch( F32 pitch );
      void setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );

      /// Is this a 3D positional voice.
      bool is3D() const { return mIs3D; }

      ///
      const X3DAUDIO_EMITTER& getEmitter() const { return mEmitter; }
};

#endif // _SFXXAUDIOVOICE_H_