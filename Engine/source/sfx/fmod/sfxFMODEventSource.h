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

#ifndef _SFXFMODEVENTSOURCE_H_
#define _SFXFMODEVENTSOURCE_H_

#ifndef _SFXSOURCE_H_
   #include "sfx/sfxSource.h"
#endif

#include "fmod_event.h"


class SFXFMODEvent;


/// An SFXSource that controls the playback of an SFXFMODEvent.
///
/// SFXFMODEvents can be played back directly through their console methods.
/// However, this class integrates them with the remaining SFX system and makes
/// events usable wherever SFX tracks are usable though with the important
/// distinction that there can only ever be a single source for a given event.
///
/// Note that calling playback methods directly on an event will cause a source
/// for the event to be created if there is not already one.
///
/// Be aware that using fade-outs in events in combination with play-once sources
/// does not work well at the moment.
///
class SFXFMODEventSource : public SFXSource
{
   public:
   
      typedef SFXSource Parent;
      
   protected:
   
      /// The event instance handle for this source.
      FMOD_EVENT* mHandle;
   
      ///
      SFXFMODEventSource( SFXFMODEvent* event );
            
      /// Update 3D position, velocity, and orientation from current source transform.
      void _update3DAttributes();
      
      // SFXSource.
      virtual void _updateStatus();
      virtual void _updateVolume( const MatrixF& listener );
      virtual void _updatePitch();
      virtual void _updatePriority();
      virtual void _onParameterEvent( SFXParameter* parameter, SFXParameterEvent event );
      virtual void _setMinMaxDistance( F32 min, F32 max );
      virtual void _setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );
      virtual void _setFadeTimes( F32 fadeInTime, F32 fadeOutTime );
   
   public:
   
      ///
      SFXFMODEventSource();
      
      virtual ~SFXFMODEventSource();
   
      /// Return the FMOD event object that is being played back by this source.
      SFXFMODEvent* getEvent() const { return ( SFXFMODEvent* ) mTrack.getPointer(); }

      /// Create a new source for the given event.
      static SFXFMODEventSource* create( SFXFMODEvent* event );
      
      // SFXSource.
      virtual void play( F32 fadeInTime = -1.f ); // fadeInTime ignored when resuming from paused
      virtual void stop( F32 fadeOutTime = -1.f ); // fadeOutTime!=0 ignored
      virtual void pause( F32 fadeOutTime = -1.f ); // fadeOutTime currently ignored
      virtual void setTransform( const MatrixF& transform );
      virtual void setVelocity( const VectorF& velocity );
      
      DECLARE_CONOBJECT( SFXFMODEventSource );
      DECLARE_CATEGORY( "SFX FMOD" );
      DECLARE_DESCRIPTION( "An SFX source controlling the playback of an FMOD Designer event." );
};

#endif // !_SFXFMODEVENTSOURCE_H_
