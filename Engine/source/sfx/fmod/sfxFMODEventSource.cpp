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
#include "sfx/fmod/sfxFMODEventSource.h"
#include "sfx/fmod/sfxFMODEvent.h"
#include "sfx/fmod/sfxFMODEventGroup.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/sfxDescription.h"


IMPLEMENT_CONOBJECT( SFXFMODEventSource );

ConsoleDocClass( SFXFMODEventSource,
   "@brief A sound source controller playing an %FMOD Designer event (SFXFMODEvent).\n\n"
   
   "%FMOD event sources are internally created by the sound system to play events from imported %FMOD Designer projects.\n\n"
   
   "@note This class cannot be instantiated directly by the user.  Instead, instances of SFXFMODEventSource will be "
      "implicitly created by the sound system when playing an SFXFMODEvent.\n\n"
   
   "@ingroup SFXFMOD\n"
);


//-----------------------------------------------------------------------------

SFXFMODEventSource::SFXFMODEventSource()
   : mHandle( NULL )
{
   SFXFMODDevice::instance()->smStatNumEventSources ++;
}

//-----------------------------------------------------------------------------

SFXFMODEventSource::SFXFMODEventSource( SFXFMODEvent* event )
   : Parent( event ),
     mHandle( NULL )
{
   SFXFMODDevice::instance()->smStatNumEventSources ++;

   // Make sure the group has its data loaded.
      
   SFXFMODEventGroup* group = event->getEventGroup();
   if( !group->loadData() )
      return;
      
   // Create an event instance.

   if( SFXFMODDevice::smFunc->FMOD_EventGroup_GetEvent(
         event->getEventGroup()->mHandle,
         event->getEventName(),
         FMOD_EVENT_DEFAULT,
         &mHandle ) != FMOD_OK )
   {
      Con::errorf( "SFXFMODEventSource::SFXFMODEventSource - failed to open event '%s'", event->getQualifiedName().c_str() );
      mHandle = NULL;
   }
}

//-----------------------------------------------------------------------------

SFXFMODEventSource::~SFXFMODEventSource()
{
   SFXFMODDevice::instance()->smStatNumEventSources --;

   if( mHandle )
      SFXFMODDevice::smFunc->FMOD_Event_Release( mHandle, true, true );
      
   if( getEvent() )
      getEvent()->getEventGroup()->freeData();
}

//-----------------------------------------------------------------------------

SFXFMODEventSource* SFXFMODEventSource::create( SFXFMODEvent* event )
{
   AssertFatal( event != NULL, "SFXFMODEventSource::create - got a NULL event!" );
   
   // Create the source.
   
   SFXFMODEventSource* source = new SFXFMODEventSource( event );
   if( source->mHandle )
      source->registerObject();
   else
   {
      delete source;
      source = NULL;
   }

   return source;
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::play( F32 fadeInTime )
{
   if( getStatus() == SFXStatusPlaying )
      return;
      
   if( isPaused() )
      SFXFMODDevice::smFunc->FMOD_Event_SetPaused( mHandle, false );
   else
   {
      AssertFatal( getEvent()->getEventGroup()->isDataLoaded(), "SFXFMODEventSource::play() - event data for group not loaded" );
                     
      if( fadeInTime != -1.f )
      {
         U32 fade = U32( fadeInTime * 1000.f );
         SFXFMODDevice::smFunc->FMOD_Event_SetPropertyByIndex(
            mHandle, FMOD_EVENTPROPERTY_FADEIN,
            &fade, true
         );
      }
      
      FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_Event_Start( mHandle );
      if( result != FMOD_OK )
      {
         Con::errorf( "SFXFMODEventSoure::play() - failed to start event: %s", FMODResultToString( result ).c_str() );
         return;
      }
   }
   
   mPlayTimer.start();
   _setStatus( SFXStatusPlaying );
   
   _play();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::stop( F32 fadeOutTime )
{
   if( getStatus() == SFXStatusStopped )
      return;
      
   AssertFatal( mHandle, "SFXFMODEvent::stop() - event not acquired" );
   
   bool immediate = ( fadeOutTime == 0.f );
   
   FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_Event_Stop( mHandle, immediate );
   if( result != FMOD_OK )
      Con::errorf( "SFXFMODEventSource::stop() - failed to stop event: %s", FMODResultToString( result ).c_str() );

   mPlayTimer.stop();
   _setStatus( SFXStatusStopped );
   
   // Reset fade-in to default in case it got overwritten
   // in play().

   U32 fade = U32( mFadeInTime * 1000.f );
   SFXFMODDevice::smFunc->FMOD_Event_SetPropertyByIndex(
      mHandle, FMOD_EVENTPROPERTY_FADEIN,
      &fade, true
   );
   
   _stop();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::pause( F32 fadeOutTime )
{
   if( getStatus() != SFXStatusPlaying )
      return;

   SFXFMODDevice::smFunc->FMOD_Event_SetPaused( mHandle, true );

   mPlayTimer.pause();
   _setStatus( SFXStatusPaused );
   
   _pause();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::setTransform( const MatrixF& transform )
{
   Parent::setTransform( transform );
   _update3DAttributes();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::setVelocity( const VectorF& velocity )
{
   Parent::setVelocity( velocity );
   _update3DAttributes();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_update3DAttributes()
{
   FMOD_VECTOR position;
   FMOD_VECTOR velocity;
   FMOD_VECTOR orientation;
   
   Point3F direction;
   getTransform().getColumn( 1, &direction );
   
   TorqueVectorToFMODVector( getTransform().getPosition(), position );
   TorqueVectorToFMODVector( getVelocity(), velocity );
   TorqueVectorToFMODVector( direction, orientation );
   
   SFXFMODDevice::smFunc->FMOD_Event_Set3DAttributes( mHandle, &position, &velocity, &orientation );
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_updateStatus()
{
   if( mStatus == SFXStatusPlaying )
   {
      if( !getEvent() )
         _setStatus( SFXStatusStopped );
      else
      {
         FMOD_EVENT_STATE state;
         SFXFMODDevice::smFunc->FMOD_Event_GetState( mHandle, &state );
         
         if( !( state & FMOD_EVENT_STATE_PLAYING ) )
            _setStatus( SFXStatusStopped );
      }
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_updateVolume( const MatrixF& listener )
{
   F32 oldPreAttenuatedVolume = mPreAttenuatedVolume;
   Parent::_updateVolume( listener );
   
   if( oldPreAttenuatedVolume != mPreAttenuatedVolume )
      SFXFMODDevice::smFunc->FMOD_Event_SetVolume( mHandle, mPreAttenuatedVolume );
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_updatePitch()
{
   F32 oldEffectivePitch = mEffectivePitch;
   Parent::_updatePitch();
   
   if( mEffectivePitch != oldEffectivePitch )
      SFXFMODDevice::smFunc->FMOD_Event_SetPitch( mHandle, mEffectivePitch - 1.0f, FMOD_EVENT_PITCHUNITS_RAW );
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_updatePriority()
{
   //TODO
   Parent::_updatePriority();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_setMinMaxDistance( F32 min, F32 max )
{
   Parent::_setMinMaxDistance( min, max );
   _update3DAttributes();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_setFadeTimes( F32 fadeInTime, F32 fadeOutTime )
{
   Parent::_setFadeTimes( fadeInTime, fadeOutTime );

   U32 fadeIn = U32( mFadeInTime * 1000.f );
   SFXFMODDevice::smFunc->FMOD_Event_SetPropertyByIndex(
      mHandle, FMOD_EVENTPROPERTY_FADEIN,
      &fadeIn, true
   );

   U32 fadeOut = U32( mFadeOutTime * 1000.f );
   SFXFMODDevice::smFunc->FMOD_Event_SetPropertyByIndex(
      mHandle, FMOD_EVENTPROPERTY_FADEOUT,
      &fadeOut, true
   );
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   Parent::_setCone( innerAngle, outerAngle, outerVolume );
   _update3DAttributes();
}

//-----------------------------------------------------------------------------

void SFXFMODEventSource::_onParameterEvent( SFXParameter* parameter, SFXParameterEvent event )
{
   Parent::_onParameterEvent( parameter, event );
   
   // If it's a value-change on a custom parameter,
   // pass it along to FMOD.
   
   if(    getEvent()
       && event == SFXParameterEvent_ValueChanged
       && parameter->getChannel() == SFXChannelUser0 )
   {
      const char* name = parameter->getInternalName();
      
      FMOD_EVENTPARAMETER* fmodParameter;
      if( SFXFMODDevice::smFunc->FMOD_Event_GetParameter( mHandle, name, &fmodParameter ) != FMOD_OK )
      {
         Con::errorf( "SFXFMODEventSource::_onParameterEvent - could not access parameter '%s' of event '%s'",
            name, getEvent()->getQualifiedName().c_str() );
         return;
      }
      
      SFXFMODDevice::smFunc->FMOD_EventParameter_SetValue( fmodParameter, parameter->getValue() );
   }
}
