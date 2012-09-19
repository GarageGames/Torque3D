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

#include "sfx/dsound/sfxDSVoice.h"
#include "sfx/dsound/sfxDSDevice.h"
#include "core/util/safeRelease.h"


SFXDSVoice* SFXDSVoice::create( SFXDSDevice *device, SFXDSBuffer *buffer )
{
   AssertFatal( buffer, "SFXDSVoice::create() - Got null buffer!" );

   IDirectSoundBuffer8 *dsBuffer8 = NULL;
   if ( !buffer->createVoice( &dsBuffer8 ) || !dsBuffer8 )
      return NULL;

   // Now try to grab a 3D interface... if we don't
   // get one its probably because its not a 3d sound.
   IDirectSound3DBuffer8* dsBuffer3d8 = NULL;
   dsBuffer8->QueryInterface( IID_IDirectSound3DBuffer8, (LPVOID*)&dsBuffer3d8 );

   // Create the voice and return!
   SFXDSVoice* voice = new SFXDSVoice( device,
                                       buffer,
                                       dsBuffer8,
                                       dsBuffer3d8 );

   // Now set the voice to a default state.
   // The buffer from which we have duplicated may have been assigned different
   // properties and we don't want to inherit these.

   voice->setVolume( 1.0 );
   voice->setPitch( 1.0 );

   return voice;
}

SFXDSVoice::SFXDSVoice( SFXDSDevice *device,
                        SFXDSBuffer *buffer,
                        IDirectSoundBuffer8 *dsBuffer, 
                        IDirectSound3DBuffer8 *dsBuffer3d )
   :  Parent( buffer ),
      mDevice( device ),
      mDSBuffer( dsBuffer ),
      mDSBuffer3D( dsBuffer3d ),
      mIsLooping( false )
{
   AssertFatal( mDevice, "SFXDSVoice::SFXDSVoice() - SFXDSDevice is null!" );
   AssertFatal( mBuffer, "SFXDSVoice::SFXDSVoice() - SFXDSBuffer is null!" );
   AssertFatal( mDSBuffer, "SFXDSVoice::SFXDSVoice() - Dsound buffer is null!" );
}

SFXDSVoice::~SFXDSVoice()
{
   SAFE_RELEASE( mDSBuffer3D );

   SFXDSBuffer* dsBuffer = _getBuffer();
   if( dsBuffer )
      dsBuffer->releaseVoice( &mDSBuffer );

   mBuffer = NULL;
}

SFXStatus SFXDSVoice::_status() const
{
   DWORD status = 0;
   mDSBuffer->GetStatus( &status );

   if ( status & DSBSTATUS_PLAYING )
      return SFXStatusPlaying;
   else
      return SFXStatusStopped;
}

void SFXDSVoice::_play()
{
   DSAssert( mDSBuffer->Play( 0, 0, mIsLooping ? DSBPLAY_LOOPING : 0 ), 
      "SFXDSVoice::_play() - Playback failed!" );
}

void SFXDSVoice::_stop()
{
   DSAssert( mDSBuffer->Stop(), "SFXDSVoice::pause - stop failed!" );
   mDSBuffer->SetCurrentPosition( 0 );
}

void SFXDSVoice::_pause()
{
   DSAssert( mDSBuffer->Stop(), "SFXDSVoice::pause - stop failed!" );
}

void SFXDSVoice::_seek( U32 sample )
{
   U32 pos = mBuffer->getFormat().getBytesPerSample() * sample;
   mDSBuffer->SetCurrentPosition( pos );
}

U32 SFXDSVoice::_tell() const
{
   DWORD position = 0;
   mDSBuffer->GetCurrentPosition( &position, NULL );
   U32 samplePos = _getBuffer()->getSamplePos( position );
   return samplePos;
}

void SFXDSVoice::setMinMaxDistance( F32 min, F32 max )
{
   if ( !mDSBuffer3D )
      return;

   mDSBuffer3D->SetMinDistance( min, DS3D_DEFERRED );
   mDSBuffer3D->SetMaxDistance( max, DS3D_DEFERRED );
}

void SFXDSVoice::play( bool looping )
{
   // If this is a 3d sound then we need
   // to commit any deferred settings before
   // we start playback else we can get some
   // glitches.
   
   if ( mDSBuffer3D )
      mDevice->_commitDeferred();

   // If this is a streaming buffer,
   // force looping.
   
   const bool isStreaming = mBuffer->isStreaming();
   if( isStreaming )
      looping = true;
   mIsLooping = looping;

   Parent::play( looping );
}

void SFXDSVoice::setVelocity( const VectorF& velocity )
{
   if ( !mDSBuffer3D )
      return;

   DSAssert( mDSBuffer3D->SetVelocity( velocity.x, velocity.z, velocity.y, DS3D_DEFERRED ), 
      "SFXDSVoice::setVelocity - couldn't update buffer!" );
}

void SFXDSVoice::setTransform( const MatrixF& transform )
{
   if ( !mDSBuffer3D )
      return;

   Point3F pos, dir;
   transform.getColumn( 3, &pos );
   transform.getColumn( 1, &dir );
   DSAssert( mDSBuffer3D->SetPosition( pos.x, pos.z, pos.y, DS3D_DEFERRED ), 
      "SFXDSVoice::setTransform - couldn't set position of the buffer." );

   DSAssert( mDSBuffer3D->SetConeOrientation( dir.x, dir.z, dir.y, DS3D_DEFERRED ), 
      "SFXDSVoice::setTransform - couldn't set cone orientation of the buffer." );
}

/// Helper for converting floating point linear volume
/// to a logrithmic integer volume for dsound.
LONG SFXDSVoice::_linearToLogVolume( F32 linVolume )
{
   LONG logVolume;

   if ( linVolume <= 0.0f )
      logVolume = DSBVOLUME_MIN;
   else
   {
      logVolume = -2000.0 * mLog( 1.0f / linVolume );
      logVolume = mClamp( logVolume, DSBVOLUME_MIN, DSBVOLUME_MAX );
   }

   return logVolume;
}

void SFXDSVoice::setVolume( F32 volume )
{
   LONG logVolume = _linearToLogVolume( volume );

   HRESULT hr = mDSBuffer->SetVolume( logVolume );
   DSAssert( hr, "SFXDSVoice::setVolume - couldn't set volume!" );
}

void SFXDSVoice::setPitch( F32 pitch )
{ 
   F32 sampleRate = _getBuffer()->getFormat().getSamplesPerSecond();
   F32 frequency = mFloor( mClampF( sampleRate * pitch, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX ) );

   DSAssert( mDSBuffer->SetFrequency( ( U32 )frequency ), 
      "SFXDSVoice::setPitch - couldn't set playback frequency.");
}

void SFXDSVoice::setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   if ( !mDSBuffer3D )
      return;

   DSAssert( mDSBuffer3D->SetConeAngles(  innerAngle, 
                                          outerAngle, 
                                          DS3D_DEFERRED ), 
      "SFXDSVoice::setCone - couldn't set cone angles!" );


   LONG logVolume = _linearToLogVolume( outerVolume );

   DSAssert( mDSBuffer3D->SetConeOutsideVolume( logVolume, 
                                                DS3D_DEFERRED ), 
      "SFXDSVoice::setCone - couldn't set cone outside volume!" );
}
