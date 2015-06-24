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

#include "sfx/sfxSound.h"
#include "sfx/sfxDevice.h"
#include "sfx/sfxVoice.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxBuffer.h"
#include "sfx/sfxStream.h"
#include "sfx/sfxDescription.h"
#include "core/util/safeDelete.h"
#include "console/engineAPI.h"


//#define DEBUG_SPEW


IMPLEMENT_CONOBJECT( SFXSound );

ConsoleDocClass( SFXSound,
   "@brief A sound controller that directly plays a single sound file.\n\n"
   
   "When playing individual audio files, SFXSounds are implicitly created by the sound system.\n\n"
   
   "Each sound source has an associated play cursor that can be queried and explicitly positioned "
   "by the user.  The cursor is a floating-point value measured in seconds.\n\n"
   
   "For streamed sources, playback may not be continuous in case the streaming queue is interrupted.\n\n"
   
   "@note This class cannot be instantiated directly by the user but rather is implicitly created by the sound "
      "system when sfxCreateSource() or sfxPlayOnce() is called on a SFXProfile instance.\n\n"
   
   "@section SFXSound_virtualization Sounds and Voices\n\n"
   
   "To actually emit an audible signal, a sound must allocate a resource on the sound device through "
   "which the sound data is being played back.  This resource is called 'voice'.\n\n"
   
   "As with other types of resources, the availability of these resources may be restricted, i.e. a given "
   "sound device will usually only support a fixed number of voices that are playing at the same time.  Since, "
   "however, there may be arbitrary many SFXSounds instantiated and playing at the same time, this needs to be "
   "solved.  \n\n"

   "@see SFXDescription::priority\n"

   "@ingroup SFX"
);


//-----------------------------------------------------------------------------

SFXSound::SFXSound()
   : mVoice( NULL )
{
   // NOTE: This should never be used directly 
   // and is only here to satisfy satisfy the
   // construction needs of IMPLEMENT_CONOBJECT.
}

//-----------------------------------------------------------------------------

SFXSound::SFXSound( SFXProfile *profile, SFXDescription* desc )
   :  Parent( profile, desc ),
      mVoice( NULL )
{
}

//-----------------------------------------------------------------------------

SFXSound* SFXSound::_create( SFXDevice *device, SFXProfile *profile )
{
   AssertFatal( profile, "SFXSound::_create() - Got a null profile!" );

   SFXDescription* desc = profile->getDescription();
   if ( !desc )
   {
      Con::errorf( "SFXSound::_create() - Profile has null description!" );
      return NULL;
   }

   // Create the sound and register it.
   
   SFXSound* sound = new SFXSound( profile, desc );
   sound->registerObject();
   
   // Initialize the buffer.

   SFXBuffer* buffer = profile->getBuffer();
   if( !buffer )
   {
      sound->deleteObject();

      Con::errorf( "SFXSound::_create() - Could not create device buffer!" );
      return NULL;
   }
   
   sound->_setBuffer( buffer );
   
   // The sound is a console object... register it.
   
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSound] new sound '%i' with profile '%i' (\"%s\")",
      sound->getId(), profile->getId(), profile->getName() );
   #endif
   
   // Hook up reloading.
   
   profile->getChangedSignal().notify( sound, &SFXSound::_onProfileChanged );

   return sound;
}

//-----------------------------------------------------------------------------

SFXSound* SFXSound::_create(   SFXDevice* device, 
                               const ThreadSafeRef< SFXStream >& stream,
                               SFXDescription* description )
{
   AssertFatal( stream.ptr() != NULL, "SFXSound::_create() - Got a null stream!" );
   AssertFatal( description, "SFXSound::_create() - Got a null description!" );

   // Create the source and register it.
   
   SFXSound* source = new SFXSound( NULL, description );
   source->registerObject();
   
   // Create the buffer.

   SFXBuffer* buffer = SFX->_createBuffer( stream, description );
   if( !buffer )
   {
      source->deleteObject();

      Con::errorf( "SFXSound::_create() - Could not create device buffer!" );
      return NULL;
   }
   
   source->_setBuffer( buffer );

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSound] new source '%i' for stream", source->getId() );
   #endif

   return source;
}

//-----------------------------------------------------------------------------

void SFXSound::_reloadBuffer()
{
   SFXProfile* profile = getProfile();
   if( profile != NULL && _releaseVoice() )
   {
      SFXBuffer* buffer = profile->getBuffer();
      if( !buffer )
      {
         Con::errorf( "SFXSound::_reloadBuffer() - Could not create device buffer!" );
         return;
      }
      
      _setBuffer( buffer );
      
      if( getLastStatus() == SFXStatusPlaying )
         SFX->_assignVoice( this );
   }
}

//-----------------------------------------------------------------------------

void SFXSound::_setBuffer( SFXBuffer* buffer )
{
   mBuffer = buffer;

   // There is no telling when the device will be 
   // destroyed and the buffers deleted.
   //
   // By caching the duration now we can allow sources
   // to continue virtual playback until the device
   // is restored.
   mDuration = mBuffer->getDuration();
}

//-----------------------------------------------------------------------------

bool SFXSound::_allocVoice( SFXDevice* device )
{
   // We shouldn't have any existing voice!
   AssertFatal( !mVoice, "SFXSound::_allocVoice() - Already had a voice!" );

   // Must not assign voice to source that isn't playing.
   AssertFatal( getLastStatus() == SFXStatusPlaying,
      "SFXSound::_allocVoice() - Source is not playing!" );

   // The buffer can be lost when the device is reset 
   // or changed, so initialize it if we have to.  If
   // that fails then we cannot create the voice.
   
   if( mBuffer.isNull() )
   {
      SFXProfile* profile = getProfile();
      if( profile != NULL )
      {
         SFXBuffer* buffer = profile->getBuffer();
         if( buffer )
            _setBuffer( buffer );
      }

      if( mBuffer.isNull() )
         return false;
   }

   // Ask the device for a voice based on this buffer.
   mVoice = device->createVoice( is3d(), mBuffer );
   if( !mVoice )
      return false;
            
   // Set initial properties.
   
   mVoice->setVolume( mPreAttenuatedVolume );
   mVoice->setPitch( mEffectivePitch );
   mVoice->setPriority( mEffectivePriority );
   if( mDescription->mRolloffFactor != -1.f )
      mVoice->setRolloffFactor( mDescription->mRolloffFactor );
      
   // Set 3D parameters.
   
   if( is3d() )
   {
      // Scatter the position, if requested.  Do this only once so
      // we don't change position when resuming from virtualized
      // playback.
      
      if( !mTransformScattered )
         _scatterTransform();
      
      // Set the 3D attributes.

      setTransform( mTransform );
      setVelocity( mVelocity );
      _setMinMaxDistance( mMinDistance, mMaxDistance );
      _setCone( mConeInsideAngle, mConeOutsideAngle, mConeOutsideVolume );
   }
   
   // Set reverb, if enabled.

   if( mDescription->mUseReverb )
      mVoice->setReverb( mDescription->mReverb );
   
   // Update the duration... it shouldn't have changed, but
   // its probably better that we're accurate if it did.
   mDuration = mBuffer->getDuration();

   // If virtualized playback has been started, we transfer its position to the
   // voice and stop virtualization.

   const U32 playTime = mPlayTimer.getPosition();
   
   if( playTime > 0 )
   {
      const U32 pos = mBuffer->getFormat().getSampleCount( playTime );
      mVoice->setPosition( pos);
   }

   mVoice->play( isLooping() );
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSound] allocated voice for source '%i' (pos=%i, 3d=%i, vol=%f)",
      getId(), playTime, is3d(), mPreAttenuatedVolume );
   #endif
   
   return true;
}

//-----------------------------------------------------------------------------

void SFXSound::_onParameterEvent( SFXParameter* parameter, SFXParameterEvent event )
{
   Parent::_onParameterEvent( parameter, event );

   switch( event )
   {
      case SFXParameterEvent_ValueChanged:            
         switch( parameter->getChannel() )
         {
            case SFXChannelCursor:
               setPosition( parameter->getValue() * 1000.f );
               break;
                              
            default:
               break;
         }
         break;
         
      default:
         break;
   }
}

//-----------------------------------------------------------------------------

void SFXSound::onRemove()
{
   SFXProfile* profile = getProfile();
   if( profile != NULL )
      profile->getChangedSignal().remove( this, &SFXSound::_onProfileChanged );
      
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SFXSound::onDeleteNotify( SimObject* object )
{
   if( object == mDescription )
   {
      deleteObject();
      return;
   }
   
   Parent::onDeleteNotify( object );
}

//-----------------------------------------------------------------------------

bool SFXSound::_releaseVoice()
{
   if( !mVoice )
      return true;
      
   // Refuse to release a voice for a streaming buffer that
   // is not coming from a profile.  For streaming buffers, we will
   // have to release the buffer, too, and without a profile we don't
   // know how to recreate the stream.
   
   if( isStreaming() && !mTrack )
      return false;
      
   // If we're currently playing, transfer our playback position
   // to the playtimer so we can virtualize playback while not
   // having a voice.

   SFXStatus status = getLastStatus();
   if( status == SFXStatusPlaying || status == SFXStatusBlocked )
   {
      // Sync up the play timer with the voice's current position to make
      // sure we handle any lag that's cropped up.
      
      mPlayTimer.setPosition( mVoice->getPosition() );

      if( status == SFXStatusBlocked )
         status = SFXStatusPlaying;
   }

   mVoice = NULL;
   
   // If this is a streaming source, release our buffer, too.
   // Otherwise the voice will stick around as it is uniquely assigned to
   // the buffer.  When we get reassigned a voice, we will have to do
   // a full stream seek anyway, so it's no real loss here.
   
   if( isStreaming() )
      mBuffer = NULL;
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSound] release voice for source '%i' (status: %s)",
      getId(), SFXStatusToString( status ) );
   #endif
   
   return true;
}

//-----------------------------------------------------------------------------

void SFXSound::_play()
{
   Parent::_play();
   
   if( mVoice )
      mVoice->play( isLooping() );
   else
   {
      // To ensure the fastest possible reaction 
      // to this playback let the system reassign
      // voices immediately.
      SFX->_assignVoice( this );
      
      // If we did not get assigned a voice, we'll be
      // running virtualized.

      #ifdef DEBUG_SPEW
      if( !mVoice )
         Platform::outputDebugString( "[SFXSound] virtualizing playback of source '%i'", getId() );
      #endif
   }
}

//-----------------------------------------------------------------------------

void SFXSound::_stop()
{
   Parent::_stop();
   
   if( mVoice )
      mVoice->stop();
}

//-----------------------------------------------------------------------------

void SFXSound::_pause()
{
   Parent::_pause();
   
   if( mVoice )
      mVoice->pause();
}

//-----------------------------------------------------------------------------

void SFXSound::_updateStatus()
{
   // If we have a voice, use its status.
   
   if( mVoice )
   {
      SFXStatus voiceStatus = mVoice->getStatus();
      
      // Filter out SFXStatusBlocked.
      
      if( voiceStatus == SFXStatusBlocked )
         _setStatus( SFXStatusPlaying );
      else
         _setStatus( voiceStatus );
      
      return;
   }

   // If we're not in a playing state or we're a looping
   // sound then we don't need to calculate the status.
   
   if( isLooping() || mStatus != SFXStatusPlaying )
      return;

   // If we're playing and don't have a voice we
   // need to decide if the sound is done playing
   // to ensure proper virtualization of the sound.

   if( mPlayTimer.getPosition() > mDuration )
   {
      _stop();
      _setStatus( SFXStatusStopped );
   }
}

//-----------------------------------------------------------------------------

void SFXSound::_updateVolume( const MatrixF& listener )
{
   F32 oldPreAttenuatedVolume = mPreAttenuatedVolume;
   Parent::_updateVolume( listener );
   
   // If we have a voice and the pre-attenuated volume has
   // changed, pass it on to the voice.  Attenuation itself will
   // happen on the device.
   
   if( mVoice != NULL && oldPreAttenuatedVolume != mPreAttenuatedVolume )
      mVoice->setVolume( mPreAttenuatedVolume );
}

//-----------------------------------------------------------------------------

void SFXSound::_updatePitch()
{
   F32 oldEffectivePitch = mEffectivePitch;
   Parent::_updatePitch();
   
   if( mVoice != NULL && oldEffectivePitch != mEffectivePitch )
      mVoice->setPitch( mEffectivePitch );
}

//-----------------------------------------------------------------------------

void SFXSound::_updatePriority()
{
   F32 oldEffectivePriority = mEffectivePriority;
   Parent::_updatePriority();
   
   if( mVoice != NULL && oldEffectivePriority != mEffectivePriority )
      mVoice->setPriority( mEffectivePriority );
}

//-----------------------------------------------------------------------------

U32 SFXSound::getPosition() const
{
   if( mVoice )
      return mVoice->getFormat().getDuration( mVoice->getPosition() );
   else
      return ( mPlayTimer.getPosition() % mDuration ); // Clamp for looped sounds.
}

//-----------------------------------------------------------------------------

void SFXSound::setPosition( U32 ms )
{
   AssertFatal( ms < getDuration(), "SFXSound::setPosition() - position out of range" );
   if( mVoice )
      mVoice->setPosition( mVoice->getFormat().getSampleCount( ms ) );
   else
      mPlayTimer.setPosition( ms );
}

//-----------------------------------------------------------------------------

void SFXSound::setVelocity( const VectorF& velocity )
{
   Parent::setVelocity( velocity );

   if( mVoice && is3d() )
      mVoice->setVelocity( velocity );      
}

//-----------------------------------------------------------------------------

void SFXSound::setTransform( const MatrixF& transform )
{
   Parent::setTransform( transform );

   if( mVoice && is3d() )
      mVoice->setTransform( mTransform );      
}

//-----------------------------------------------------------------------------

void SFXSound::_setMinMaxDistance( F32 min, F32 max )
{
   Parent::_setMinMaxDistance( min, max );

   if( mVoice && is3d() )
      mVoice->setMinMaxDistance( mMinDistance, mMaxDistance );
}

//-----------------------------------------------------------------------------

void SFXSound::_setCone(   F32 innerAngle,
                           F32 outerAngle,
                           F32 outerVolume )
{
   Parent::_setCone( innerAngle, outerAngle, outerVolume );

   if( mVoice && is3d() )
      mVoice->setCone(  mConeInsideAngle,
                        mConeOutsideAngle,
                        mConeOutsideVolume );
}

//-----------------------------------------------------------------------------

bool SFXSound::isReady() const
{
   return ( mBuffer != NULL && mBuffer->isReady() );
}

//-----------------------------------------------------------------------------

bool SFXSound::isVirtualized() const
{
   return ( ( mVoice == NULL && isPlaying() ) ||
            ( mVoice != NULL && mVoice->isVirtual() ) );
}
 
//-----------------------------------------------------------------------------

SFXProfile* SFXSound::getProfile() const
{
   return dynamic_cast< SFXProfile* >( mTrack.getPointer() );
}

//-----------------------------------------------------------------------------

F32 SFXSound::getElapsedPlayTimeCurrentCycle() const
{
   return F32( getPosition() ) / 1000.f;
}

//-----------------------------------------------------------------------------

F32 SFXSound::getTotalPlayTime() const
{
   return F32( mDuration ) / 1000.f;
}

//-----------------------------------------------------------------------------

// Let the user define a priority value for each channel
// in script.  We assign it in the system init and use
// it when doleing out hardware handles.

S32 QSORT_CALLBACK SFXSound::qsortCompare( const void* item1, const void* item2 )
{
   const SFXSound* source1 = *( ( SFXSound** ) item1 );
   const SFXSound* source2 = *( ( SFXSound** ) item2 );

	// Sounds that are playing are always sorted 
	// closer than non-playing sounds.
   
   const bool source1IsPlaying = source1->isPlaying();
   const bool source2IsPlaying = source2->isPlaying();
   
   if( !source1IsPlaying && !source2IsPlaying )
      return 0;
	else if( !source1IsPlaying && source2IsPlaying )
		return 1;
	else if( source1IsPlaying && !source2IsPlaying )
		return -1;
      
   // Louder attenuated volumes take precedence but adjust them
   // by priority so that less audible sounds with higher priority
   // become more important.
   
	F32 volume1 = source1->getAttenuatedVolume();
	F32 volume2 = source2->getAttenuatedVolume();
   
   volume1 += volume1 * source1->mEffectivePriority;
   volume2 += volume2 * source2->mEffectivePriority;
   
   if( volume1 < volume2 )
      return 1;
   if( volume1 > volume2 )
      return -1;

   // If we got this far then the source that was 
   // played last has the higher priority.
   
   if( source1->mPlayStartTick > source2->mPlayStartTick )
      return -1;
   if( source1->mPlayStartTick < source2->mPlayStartTick )
      return 1;

   // These are sorted the same!
   return 0;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSound, isReady, bool, (),,
   "Test whether the sound data associated with the sound has been fully loaded and is ready for playback.\n"
   "For streamed sounds, this will be false during playback when the stream queue for the sound is starved and "
   "waiting for data.  For buffered sounds, only an initial loading phase will potentially cause isReady to "
   "return false.\n\n"
   "@return True if the sound is ready for playback." )
{
   return object->isReady();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSound, getPosition, F32, (),,
   "Get the current playback position in seconds.\n"
   "@return The current play cursor offset." )
{
   return F32( object->getPosition() ) * 0.001f;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSound, setPosition, void, ( F32 position ),,
   "Set the current playback position in seconds.\n"
   "If the source is currently playing, playback will jump to the new position.  If playback is stopped or paused, "
   "playback will resume at the given position when play() is called.\n\n"
   "@param position The new position of the play cursor (in seconds).\n" )
{
   if( position >= 0 && position <= object->getDuration() )
      object->setPosition( position * 1000.0f );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSound, getDuration, F32, (),,
   "Get the total play time (in seconds) of the sound data attached to the sound.\n"
   "@return \n\n"
   "@note Be aware that for looped sounds, this will not return the total playback time of the sound.\n" )
{
   return F32( object->getDuration() ) * 0.001f;
}
