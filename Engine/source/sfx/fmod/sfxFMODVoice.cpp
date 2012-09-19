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
#include "sfx/fmod/sfxFMODVoice.h"

#include "sfx/fmod/sfxFMODBuffer.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "core/tAlgorithm.h"


SFXFMODVoice* SFXFMODVoice::create( SFXFMODDevice *device,
                                    SFXFMODBuffer *buffer )
{
   AssertFatal( device, "SFXFMODVoice::create() - Got null device!" );
   AssertFatal( buffer, "SFXFMODVoice::create() - Got null buffer!" );

   return new SFXFMODVoice( device, buffer );
}

SFXFMODVoice::SFXFMODVoice(   SFXFMODDevice *device, 
                              SFXFMODBuffer *buffer )
   :  Parent( buffer ),
      mDevice( device ),
      mChannel( NULL )
{
   AssertFatal( device, "SFXFMODVoice::SFXFMODVoice() - No device assigned!" );
   AssertFatal( buffer, "SFXFMODVoice::SFXFMODVoice() - No buffer assigned!" );
   AssertFatal( _getBuffer()->mSound != NULL, "SFXFMODVoice::SFXFMODVoice() - No sound assigned!" );
}

SFXFMODVoice::~SFXFMODVoice()
{
	_stop();
}

SFXStatus SFXFMODVoice::_status() const
{
   if( mChannel )
   {
      FMOD_BOOL isTrue = false;
      SFXFMODDevice::smFunc->FMOD_Channel_GetPaused( mChannel, &isTrue );
      if ( isTrue )
         return SFXStatusPaused;

      SFXFMODDevice::smFunc->FMOD_Channel_IsPlaying( mChannel, &isTrue );
      if ( isTrue )
         return SFXStatusPlaying;
   }
   
   SFXFMODDevice::smFunc->FMOD_Channel_Stop( mChannel );
   mChannel = NULL;

   return SFXStatusStopped;
}

void SFXFMODVoice::_play()
{
   if( !mChannel )
      _assignChannel();

   SFXFMODDevice::smFunc->FMOD_Channel_SetPaused( mChannel, false );
}

void SFXFMODVoice::_pause()
{
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_SetPaused( mChannel, true );
}

void SFXFMODVoice::_stop()
{
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_Stop(mChannel);
      
	mChannel = NULL;
}

void SFXFMODVoice::_seek( U32 sample )
{
   if( !mChannel )
      _assignChannel();

   SFXFMODDevice::smFunc->FMOD_Channel_SetPosition
      ( mChannel, sample, FMOD_TIMEUNIT_PCM );
}

bool SFXFMODVoice::_assignChannel()
{
   AssertFatal( _getBuffer()->mSound != NULL, "SFXFMODVoice::_assignChannel() - No sound assigned!" );

   // we start playing it now in the paused state, so that we can immediately set attributes that
   // depend on having a channel (position, volume, etc).  According to the FMod docs
   // it is ok to do this.
   bool success = SFXFMODDevice::smFunc->FMOD_System_PlaySound(
      SFXFMODDevice::smSystem, 
      FMOD_CHANNEL_FREE, 
      _getBuffer()->mSound, 
      true, 
      &mChannel ) == FMOD_OK;
      
   if( success )
   {
      SFXFMODDevice::smFunc->FMOD_Channel_SetMode( mChannel, mMode );
      SFXFMODDevice::smFunc->FMOD_Channel_SetLoopCount( mChannel, mMode & FMOD_LOOP_NORMAL ? -1 : 0 );

      if( mSetFlags.test( SET_Velocity ) )
         SFXFMODDevice::smFunc->FMOD_Channel_Set3DAttributes( mChannel, ( const FMOD_VECTOR* ) NULL, &mVelocity );
      if( mSetFlags.test( SET_MinMaxDistance ) )
         SFXFMODDevice::smFunc->FMOD_Channel_Set3DMinMaxDistance(mChannel, mMinDistance, mMaxDistance);
      if( mSetFlags.test( SET_Transform ) )
      {
         SFXFMODDevice::smFunc->FMOD_Channel_Set3DAttributes( mChannel, &mPosition, ( const FMOD_VECTOR* ) NULL );
         SFXFMODDevice::smFunc->FMOD_Channel_Set3DConeOrientation( mChannel, &mDirection );
      }
      if( mSetFlags.test( SET_Volume ) )
         SFXFMODDevice::smFunc->FMOD_Channel_SetVolume(mChannel, mVolume);
      if( mSetFlags.test( SET_Pitch ) )
         SFXFMODDevice::smFunc->FMOD_Channel_SetFrequency( mChannel, mFrequency );
      if( mSetFlags.test( SET_Cone ) )
         SFXFMODDevice::smFunc->FMOD_Channel_Set3DConeSettings( 
            mChannel, 
            mConeInnerAngle,
            mConeOuterAngle,
            mConeOuterVolume );
      if( mSetFlags.test( SET_Priority ) )
         SFXFMODDevice::smFunc->FMOD_Channel_SetPriority( mChannel, TorquePriorityToFMODPriority( mPriority ) );
      if( mSetFlags.test( SET_Reverb ) )
         SFXFMODDevice::smFunc->FMOD_Channel_SetReverbProperties( mChannel, &mReverb );
   }
   
   return success;
}

U32 SFXFMODVoice::_tell() const
{
   if( !mChannel )
      return 0;

   U32 pos;
   SFXFMODDevice::smFunc->FMOD_Channel_GetPosition( mChannel, &pos, ( FMOD_TIMEUNIT ) FMOD_TIMEUNIT_PCMBYTES );
   return _getBuffer()->getSamplePos( pos );
}

void SFXFMODVoice::setMinMaxDistance( F32 min, F32 max )
{
	if ( !( _getBuffer()->mMode & FMOD_3D ) )
		return;
      
   mMinDistance = min;
   mMaxDistance = max;
   
   mSetFlags.set( SET_MinMaxDistance );

   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_Set3DMinMaxDistance(mChannel, mMinDistance, mMaxDistance);
}

void SFXFMODVoice::play( bool looping )
{
   if( mBuffer->isStreaming() )
      looping = true;
   
   mMode = mDevice->get3dRollOffMode();
   mMode |= (looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

   Parent::play( looping );
}

void SFXFMODVoice::setVelocity( const VectorF& velocity )
{
	if( !( _getBuffer()->mMode & FMOD_3D ) )
		return;

	// Note we have to do a handedness swap; see the 
   // listener update code in SFXFMODDevice for details.
	mVelocity.x = velocity.x;
   mVelocity.y = velocity.z;
	mVelocity.z = velocity.y;
   
   mSetFlags.set( SET_Velocity );

   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_Set3DAttributes( mChannel, ( const FMOD_VECTOR* ) NULL, &mVelocity );
}

void SFXFMODVoice::setTransform( const MatrixF& transform )
{
	if ( !( _getBuffer()->mMode & FMOD_3D ) )
		return;

   transform.getColumn( 3, (Point3F*)&mPosition );
   transform.getColumn( 1, (Point3F*)&mDirection );

   // Note we have to do a handedness swap; see the 
   // listener update code in SFXFMODDevice for details.
   swap( mPosition.y, mPosition.z );
   swap( mDirection.y, mDirection.z );
   
   mSetFlags.set( SET_Transform );

   if( mChannel )
   {
      // This can fail safe, so don't assert if it fails.
      SFXFMODDevice::smFunc->FMOD_Channel_Set3DAttributes( mChannel, &mPosition, ( const FMOD_VECTOR* ) NULL );
      SFXFMODDevice::smFunc->FMOD_Channel_Set3DConeOrientation( mChannel, &mDirection );
   }
}

void SFXFMODVoice::setVolume( F32 volume )
{
   mVolume = volume;
   mSetFlags.set( SET_Volume );
   
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_SetVolume( mChannel, volume );
}

void SFXFMODVoice::setPriority( F32 priority )
{
   mPriority = priority;
   mSetFlags.set( SET_Priority );
   
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_SetPriority( mChannel, TorquePriorityToFMODPriority( priority ) );
}

void SFXFMODVoice::setPitch( F32 pitch )
{
   // if we do not know the frequency, we cannot change the pitch
   F32 frequency = _getBuffer()->getFormat().getSamplesPerSecond();
   if ( frequency == 0 )
      return;
      
   mFrequency = frequency * pitch;
   
   mSetFlags.set( SET_Pitch );

	// Scale the original frequency by the pitch factor.
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_SetFrequency(mChannel, mFrequency);
}

void SFXFMODVoice::setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   mConeInnerAngle = innerAngle;
   mConeOuterAngle = outerAngle;
   mConeOuterVolume = outerVolume;
   
   mSetFlags.set( SET_Cone );

   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_Set3DConeSettings( 
         mChannel, 
         mConeInnerAngle,
         mConeOuterAngle,
         mConeOuterVolume );
}

void SFXFMODVoice::setReverb( const SFXSoundReverbProperties& reverb )
{
   dMemset( &mReverb, 0, sizeof( mReverb ) );
   
   mReverb.Direct                = reverb.mDirect;
   mReverb.Room                  = reverb.mRoom;
   mReverb.Flags                 = reverb.mFlags;
   
   mSetFlags.set( SET_Reverb );
   
   if( mChannel )
      SFXFMODDevice::smFunc->FMOD_Channel_SetReverbProperties( mChannel, &mReverb );
}

bool SFXFMODVoice::isVirtual() const
{
   if( mChannel )
   {
      FMOD_BOOL result;
      SFXFMODDevice::smFunc->FMOD_Channel_IsVirtual( mChannel, &result );
      return result;
   }
   else
      return false;
}
