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
#include "sfx/openal/sfxALVoice.h"
#include "sfx/openal/sfxALBuffer.h"
#include "sfx/openal/sfxALDevice.h"


#ifdef TORQUE_DEBUG
#  define AL_SANITY_CHECK() \
      AssertFatal( mOpenAL.alIsSource( mSourceName ), "AL Source Sanity Check Failed!" );
#else
#  define AL_SANITY_CHECK()
#endif


//#define DEBUG_SPEW


SFXALVoice* SFXALVoice::create( SFXALDevice* device, SFXALBuffer *buffer )
{
   AssertFatal( buffer, "SFXALVoice::create() - Got null buffer!" );
 
   ALuint sourceName;
   device->mOpenAL.alGenSources( 1, &sourceName );
   AssertFatal( device->mOpenAL.alIsSource( sourceName ), "AL Source Sanity Check Failed!" );

   // Is this 3d?
   // Okay, this looks odd, but bear with me for a moment.  AL_SOURCE_RELATIVE does NOT indicate
   // whether or not the volume of the sound should change depending on the position of the listener.
   // OpenAL assumes that the volume will ALWAYS depend on the position of the listener.  What AL_SOURCE_RELATIVE
   // does do is dictate if the position of THIS SOURCE is relative to the listener.  If AL_SOURCE_RELATIVE is AL_TRUE
   // and the source's position is 0, 0, 0, then the source is directly on top of the listener at all times, which is what
   // we want for non-3d sounds.
   device->mOpenAL.alSourcei( sourceName, AL_SOURCE_RELATIVE, ( buffer->mIs3d ? AL_FALSE : AL_TRUE ) );
   
   if( buffer->mIs3d )
      device->mOpenAL.alSourcef( sourceName, AL_ROLLOFF_FACTOR, device->mRolloffFactor );

   SFXALVoice *voice = new SFXALVoice( device->mOpenAL,
                                       buffer,
                                       sourceName );

   return voice;
}

SFXALVoice::SFXALVoice( const OPENALFNTABLE &oalft,
                        SFXALBuffer *buffer, 
                        ALuint sourceName )

   :  Parent( buffer ),
      mOpenAL( oalft ), 
      mResumeAtSampleOffset( -1.0f ),
      mSourceName( sourceName ),
      mSampleOffset( 0 )
{
   AL_SANITY_CHECK();
}

SFXALVoice::~SFXALVoice()
{
   mOpenAL.alDeleteSources( 1, &mSourceName );
}

void SFXALVoice::_lateBindStaticBufferIfNecessary()
{
   if( !mBuffer->isStreaming() )
   {
      ALint bufferId;
      mOpenAL.alGetSourcei( mSourceName, AL_BUFFER, &bufferId );
      if( !bufferId )
         mOpenAL.alSourcei( mSourceName, AL_BUFFER, _getBuffer()->mALBuffer );
   }
}


SFXStatus SFXALVoice::_status() const
{
   AL_SANITY_CHECK();

   ALint state;
   mOpenAL.alGetSourcei( mSourceName, AL_SOURCE_STATE, &state );

   switch( state )
   {
      case AL_PLAYING:  return SFXStatusPlaying;
      case AL_PAUSED:   return SFXStatusPaused;
      default:          return SFXStatusStopped;
   }
}

void SFXALVoice::_play()
{
   AL_SANITY_CHECK();

   _lateBindStaticBufferIfNecessary();

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXALVoice] Starting playback" );
   #endif
   
   mOpenAL.alSourcePlay( mSourceName );
   
   //WORKAROUND: Adjust play cursor for buggy OAL when resuming playback.  Do this after alSourcePlay
   // as it is the play function that will cause the cursor to jump.
   
   if( mResumeAtSampleOffset != -1.0f )
   {
      mOpenAL.alSourcef( mSourceName, AL_SAMPLE_OFFSET, mResumeAtSampleOffset );
      mResumeAtSampleOffset = -1.0f;
   }
}

void SFXALVoice::_pause()
{   
   AL_SANITY_CHECK();

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXALVoice] Pausing playback" );
   #endif

   mOpenAL.alSourcePause( mSourceName );
   
   //WORKAROUND: Another workaround for buggy OAL.  Resuming playback of a paused source will cause the 
   // play cursor to jump.  Save the cursor so we can manually move it into position in _play().  Sigh.
   
   mOpenAL.alGetSourcef( mSourceName, AL_SAMPLE_OFFSET, &mResumeAtSampleOffset );
}

void SFXALVoice::_stop()
{
   AL_SANITY_CHECK();
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXALVoice] Stopping playback" );
   #endif

   mOpenAL.alSourceStop( mSourceName );
   mSampleOffset = 0;
   
   mResumeAtSampleOffset = -1.0f;
}

void SFXALVoice::_seek( U32 sample )
{
   AL_SANITY_CHECK();
   
   _lateBindStaticBufferIfNecessary();
   mOpenAL.alSourcei( mSourceName, AL_SAMPLE_OFFSET, sample );

   mResumeAtSampleOffset = -1.0f;
}

U32 SFXALVoice::_tell() const
{
   // Flush processed buffers as AL_SAMPLE_OFFSET will snap back to zero as soon
   // as the queue is processed in whole.

   if( mBuffer->isStreaming() )
      mBuffer->write( NULL, 0 );

   ALint pos;
   mOpenAL.alGetSourcei( mSourceName, AL_SAMPLE_OFFSET, &pos );
   return ( pos + mSampleOffset );
}

void SFXALVoice::setMinMaxDistance( F32 min, F32 max )
{
   AL_SANITY_CHECK();

   mOpenAL.alSourcef( mSourceName, AL_REFERENCE_DISTANCE, min );
   mOpenAL.alSourcef( mSourceName, AL_MAX_DISTANCE, max );
}

void SFXALVoice::play( bool looping )
{
   AL_SANITY_CHECK();

   mOpenAL.alSourceStop( mSourceName );
   if( !mBuffer->isStreaming() )
      mOpenAL.alSourcei( mSourceName, AL_LOOPING, ( looping ? AL_TRUE : AL_FALSE ) );

   Parent::play( looping );
}

void SFXALVoice::setVelocity( const VectorF& velocity )
{
   AL_SANITY_CHECK();

   // Torque and OpenAL are both right handed 
   // systems, so no coordinate flipping is needed.

   mOpenAL.alSourcefv( mSourceName, AL_VELOCITY, velocity );
}

void SFXALVoice::setTransform( const MatrixF& transform )
{
   AL_SANITY_CHECK();

   // Torque and OpenAL are both right handed 
   // systems, so no coordinate flipping is needed.

   Point3F pos, dir;
   transform.getColumn( 3, &pos );
   transform.getColumn( 1, &dir );

   mOpenAL.alSourcefv( mSourceName, AL_POSITION, pos );
   mOpenAL.alSourcefv( mSourceName, AL_DIRECTION, dir );
}

void SFXALVoice::setVolume( F32 volume )
{
   AL_SANITY_CHECK();

   mOpenAL.alSourcef( mSourceName, AL_GAIN, volume );
}

void SFXALVoice::setPitch( F32 pitch )
{ 
   AL_SANITY_CHECK();

   mOpenAL.alSourcef( mSourceName, AL_PITCH, pitch );
}

void SFXALVoice::setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   AL_SANITY_CHECK();

   mOpenAL.alSourcef( mSourceName, AL_CONE_INNER_ANGLE, innerAngle );
   mOpenAL.alSourcef( mSourceName, AL_CONE_OUTER_ANGLE, outerAngle );
   mOpenAL.alSourcef( mSourceName, AL_CONE_OUTER_GAIN, outerVolume );
}

void SFXALVoice::setRolloffFactor( F32 factor )
{
   mOpenAL.alSourcef( mSourceName, AL_ROLLOFF_FACTOR, factor );
}
