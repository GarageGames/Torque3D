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

#include "platform/platform.h"
#include "sfx/xaudio/sfxXAudioVoice.h"
#include "sfx/xaudio/sfxXAudioDevice.h"
#include "sfx/xaudio/sfxXAudioBuffer.h"
#include "core/util/safeDelete.h"
#include "math/mMathFn.h"


//#define DEBUG_SPEW


static void sfxFormatToWAVEFORMATEX( const SFXFormat& format, WAVEFORMATEX *wfx )
{
   dMemset( wfx, 0, sizeof( WAVEFORMATEX ) ); 
   wfx->wFormatTag = WAVE_FORMAT_PCM; 
   wfx->nChannels = format.getChannels();
   wfx->nSamplesPerSec = format.getSamplesPerSecond();
   wfx->wBitsPerSample = format.getBitsPerChannel();
   wfx->nBlockAlign = wfx->nChannels * wfx->wBitsPerSample / 8;
   wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign; 
}


SFXXAudioVoice* SFXXAudioVoice::create(   IXAudio2 *xaudio,
                                          bool is3D,
                                          SFXXAudioBuffer *buffer,
                                          SFXXAudioVoice* inVoice )
{
   AssertFatal( xaudio, "SFXXAudioVoice::create() - Got null XAudio!" );
   AssertFatal( buffer, "SFXXAudioVoice::create() - Got null buffer!" );

   // Create the voice object first as it also the callback object.
   SFXXAudioVoice* voice = inVoice;
   if( !voice )
      voice = new SFXXAudioVoice( buffer );

   // Get the buffer format.
   WAVEFORMATEX wfx;
   sfxFormatToWAVEFORMATEX( buffer->getFormat(), &wfx );

   // We don't support multi-channel 3d sounds!
   if ( is3D && wfx.nChannels > 1 )
      return NULL;

   // Create the voice.
   IXAudio2SourceVoice *xaVoice;
   HRESULT hr = xaudio->CreateSourceVoice(   &xaVoice, 
                                             (WAVEFORMATEX*)&wfx,
                                             0, 
                                             XAUDIO2_DEFAULT_FREQ_RATIO, 
                                             voice, 
                                             NULL, 
                                             NULL );

   if( FAILED( hr ) || !voice )
   {
      if( !inVoice )
         delete voice;
      return NULL;
   }

   voice->mIs3D = is3D;
   voice->mEmitter.ChannelCount = wfx.nChannels;
   voice->mXAudioVoice = xaVoice;

   return voice;
}

SFXXAudioVoice::SFXXAudioVoice( SFXXAudioBuffer* buffer )
   :  Parent( buffer ),
      mXAudioDevice( NULL ),
      mXAudioVoice( NULL ),
      mIs3D( false ),
      mPitch( 1.0f ),
      mHasStopped( false ),
      mHasStarted( false ),
      mIsLooping( false ),
      mIsPlaying( false ),
      mNonStreamSampleStartPos( 0 ),
      mNonStreamBufferLoaded( false ),
      mSamplesPlayedOffset( 0 )
{
   dMemset( &mEmitter, 0, sizeof( mEmitter ) );
   mEmitter.DopplerScaler = 1.0f;

   InitializeCriticalSection( &mLock );
}

SFXXAudioVoice::~SFXXAudioVoice()
{
   if ( mEmitter.pVolumeCurve )
   {
      SAFE_DELETE_ARRAY( mEmitter.pVolumeCurve->pPoints );
      SAFE_DELETE( mEmitter.pVolumeCurve );
   }

   SAFE_DELETE( mEmitter.pCone );

   if ( mXAudioVoice )
      mXAudioVoice->DestroyVoice();

   DeleteCriticalSection( &mLock );
}

SFXStatus SFXXAudioVoice::_status() const
{
   if( mHasStopped )
      return SFXStatusStopped;
   else if( mHasStarted )
   {
      if( !mIsPlaying )
         return SFXStatusPaused;
      else
         return SFXStatusPlaying;
   }
   else
      return SFXStatusStopped;
}

void SFXXAudioVoice::_flush()
{
   AssertFatal( mXAudioVoice != NULL,
      "SFXXAudioVoice::_flush() - invalid voice" );

   EnterCriticalSection( &mLock );

   mXAudioVoice->Stop( 0 );
   mXAudioVoice->FlushSourceBuffers();
   mNonStreamBufferLoaded = false;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Flushed state" );
   #endif

   mIsPlaying = false;
   mHasStarted = false;
   mHasStopped = true;

   //WORKAROUND: According to the docs, SamplesPlayed reported by the
   // voice should get reset as soon as we submit a new buffer to the voice.
   // Alas it won't.  So, save the current value here and offset our future
   // play cursors.

   XAUDIO2_VOICE_STATE state;
   mXAudioVoice->GetState( &state );
   mSamplesPlayedOffset = - S32( state.SamplesPlayed );

   LeaveCriticalSection( &mLock );
}

void SFXXAudioVoice::_play()
{
   AssertFatal( mXAudioVoice != NULL,
      "SFXXAudioVoice::_play() - invalid voice" );

   // For non-streaming voices queue the data if we haven't yet.

   if( !mBuffer->isStreaming() && !mNonStreamBufferLoaded )
      _loadNonStreamed();

   // Start playback.

   mXAudioVoice->Start( 0, 0 );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Started playback" );
   #endif

   mIsPlaying  = true;
   mHasStarted = true;
   mHasStopped = false;
}

void SFXXAudioVoice::_pause()
{
   AssertFatal( mXAudioVoice != NULL,
      "SFXXAudioVoice::_pause() - invalid voice" );

   mXAudioVoice->Stop( 0 );
   mIsPlaying  = false;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Paused playback" );
   #endif
}

void SFXXAudioVoice::_stop()
{
   AssertFatal( mXAudioVoice != NULL,
      "SFXXAudioVoice::_stop() - invalid voice" );

   _flush();

   mIsPlaying  = false;
   mHasStarted = false;
   mHasStopped = true;

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Stopped playback" );
   #endif
}

void SFXXAudioVoice::_seek( U32 sample )
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Seeking to %i", sample );
   #endif

   mNonStreamSampleStartPos = sample;

   bool wasPlaying = mIsPlaying;
   
   _stop();
   if( wasPlaying )
      _play();
}

void SFXXAudioVoice::_loadNonStreamed()
{
   AssertFatal( !mBuffer->isStreaming(), "SFXXAudioVoice::_loadNonStreamed - must not be called on streaming voices" );
   AssertFatal( mXAudioVoice != NULL, "SFXXAudioVoice::_loadNonStreamed - invalid voice" );
   AssertWarn( !mNonStreamBufferLoaded, "SFXXAudioVoice::_nonStreamNonstreamed - Data already loaded" );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXXAudioVoice] Loading non-stream buffer at %i", mNonStreamSampleStartPos );
   #endif

   EnterCriticalSection( &mLock );

   const XAUDIO2_BUFFER& orgBuffer = _getBuffer()->mBufferQueue.front().mData;

   mNonStreamBuffer = orgBuffer;

   if( mNonStreamSampleStartPos )
   {
      mNonStreamBuffer.PlayBegin = mNonStreamSampleStartPos;
      mNonStreamBuffer.PlayLength = _getBuffer()->getNumSamples() - mNonStreamSampleStartPos;
      mSamplesPlayedOffset += mNonStreamSampleStartPos; // Add samples that we are skipping.
      mNonStreamSampleStartPos = 0;
   }

   if( mIsLooping )
   {
      mNonStreamBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
      mNonStreamBuffer.LoopLength = _getBuffer()->getNumSamples();
   }

   // Submit buffer.

   mXAudioVoice->SubmitSourceBuffer( &mNonStreamBuffer );
   mNonStreamBufferLoaded = true;

   LeaveCriticalSection( &mLock );
}

U32 SFXXAudioVoice::_tell() const
{
   XAUDIO2_VOICE_STATE state;
   mXAudioVoice->GetState( &state );

   // Workaround SamplesPlayed not getting reset.
   return ( state.SamplesPlayed + mSamplesPlayedOffset );
}

void SFXXAudioVoice::setMinMaxDistance( F32 min, F32 max )
{
   // Set the overall volume curve scale.
   mEmitter.CurveDistanceScaler = max;

   // The curve uses normalized distances, so 
   // figure out the normalized min distance.
   F32 normMin = 0.0f;
   if ( min > 0.0f )
      normMin = min / max;

   // See what type of curve we are supposed to generate.
   const bool linear = ( mXAudioDevice->mDistanceModel == SFXDistanceModelLinear );

   // Have we setup the curve yet?
   if( !mEmitter.pVolumeCurve
       || ( linear && mEmitter.pVolumeCurve->PointCount != 2 )
       || ( !linear && mEmitter.pVolumeCurve->PointCount != 6 ) )
   {
      if( !mEmitter.pVolumeCurve )
         mEmitter.pVolumeCurve = new X3DAUDIO_DISTANCE_CURVE;
      else
         SAFE_DELETE_ARRAY( mEmitter.pVolumeCurve->pPoints );

      // We use 6 points for logarithmic volume curves and 2 for linear volume curves.
      if( linear )
      {
         mEmitter.pVolumeCurve->pPoints = new X3DAUDIO_DISTANCE_CURVE_POINT[ 2 ];
         mEmitter.pVolumeCurve->PointCount = 2;
      }
      else
      {
         mEmitter.pVolumeCurve->pPoints = new X3DAUDIO_DISTANCE_CURVE_POINT[ 6 ];
         mEmitter.pVolumeCurve->PointCount = 6;
      }

      // The first and last points are known 
      // and will not change.
      mEmitter.pVolumeCurve->pPoints[ 0 ].Distance = 0.0f;
      mEmitter.pVolumeCurve->pPoints[ 0 ].DSPSetting = 1.0f;
      mEmitter.pVolumeCurve->pPoints[ linear ? 1 : 5 ].Distance = 1.0f;
      mEmitter.pVolumeCurve->pPoints[ linear ? 1 : 5 ].DSPSetting = 0.0f;
   }

   if( !linear )
   {
      // Set the second point of the curve.
      mEmitter.pVolumeCurve->pPoints[1].Distance = normMin;
      mEmitter.pVolumeCurve->pPoints[1].DSPSetting = 1.0f;

      // The next three points are calculated to 
      // give the sound a rough logarithmic falloff.
      F32 distStep = ( 1.0f - normMin ) / 4.0f;
      for ( U32 i=0; i < 3; i++ )
      {
         U32 index = 2 + i;
         F32 dist = normMin + ( distStep * (F32)( i + 1 ) );

         mEmitter.pVolumeCurve->pPoints[index].Distance = dist;
         mEmitter.pVolumeCurve->pPoints[index].DSPSetting = 1.0f - log10( dist * 10.0f );
      }
   }
}

void SFXXAudioVoice::OnBufferEnd( void* bufferContext )
{
   if( mBuffer->isStreaming() )
      SFXInternal::TriggerUpdate();
}

void SFXXAudioVoice::OnStreamEnd()
{
   // Warning:  This is being called within the XAudio 
   // thread, so be sure you're thread safe!

   mHasStopped = true;

   if( mBuffer->isStreaming() )
      SFXInternal::TriggerUpdate();
   else
      _stop();
}

void SFXXAudioVoice::play( bool looping )
{
   // Give the device a chance to calculate our positional
   // audio settings before we start playback... this is 
   // important else we get glitches.
   if( mIs3D )
      mXAudioDevice->_setOutputMatrix( this );

   mIsLooping = looping;
   Parent::play( looping );
}

void SFXXAudioVoice::setVelocity( const VectorF& velocity )
{
   mEmitter.Velocity.x = velocity.x;
   mEmitter.Velocity.y = velocity.y;

   // XAudio and Torque use opposite handedness, so
   // flip the z coord to account for that.
   mEmitter.Velocity.z = -velocity.z;
}

void SFXXAudioVoice::setTransform( const MatrixF& transform )
{
   transform.getColumn( 3, (Point3F*)&mEmitter.Position );
   transform.getColumn( 1, (Point3F*)&mEmitter.OrientFront );
   transform.getColumn( 2, (Point3F*)&mEmitter.OrientTop );

   // XAudio and Torque use opposite handedness, so
   // flip the z coord to account for that.
   mEmitter.Position.z     *= -1.0f;
   mEmitter.OrientFront.z  *= -1.0f;
   mEmitter.OrientTop.z    *= -1.0f;
}

void SFXXAudioVoice::setVolume( F32 volume )
{
   mXAudioVoice->SetVolume( volume );
}

void SFXXAudioVoice::setPitch( F32 pitch )
{
   mPitch = mClampF( pitch, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_DEFAULT_FREQ_RATIO );
   mXAudioVoice->SetFrequencyRatio( mPitch );
}

void SFXXAudioVoice::setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   // If the cone is set to 360 then the
   // cone is null and doesn't need to be
   // set on the voice.
   if ( mIsEqual( innerAngle, 360 ) )
   {
      SAFE_DELETE( mEmitter.pCone );
      return;
   }

   if ( !mEmitter.pCone )
   {
      mEmitter.pCone = new X3DAUDIO_CONE;

      // The inner volume is always 1... the overall
      // volume is what scales it.
      mEmitter.pCone->InnerVolume = 1.0f; 

      // We don't use these yet.
      mEmitter.pCone->InnerLPF = 0.0f; 
      mEmitter.pCone->OuterLPF = 0.0f; 
      mEmitter.pCone->InnerReverb = 0.0f; 
      mEmitter.pCone->OuterReverb = 0.0f; 
   }

   mEmitter.pCone->InnerAngle = mDegToRad( innerAngle );
   mEmitter.pCone->OuterAngle = mDegToRad( outerAngle );
   mEmitter.pCone->OuterVolume = outerVolume;
}
