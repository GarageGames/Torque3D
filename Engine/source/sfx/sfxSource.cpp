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

#include "sfx/sfxSource.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxModifier.h"
#include "console/engineAPI.h"
#include "math/mRandom.h"
#include "math/mEase.h"



//#define DEBUG_SPEW


IMPLEMENT_CONOBJECT( SFXSource );

ConsoleDocClass( SFXSource,
   "@brief Playback controller for a sound source.\n\n"
   
   "All sound playback is driven by SFXSources.  Each such source represents an independent playback controller "
   "that directly or indirectly affects sound output.\n\n"
   
   "While this class itself is instantiable, such an instance will not by itself emit any sound.  This is the "
   "responsibility of its subclasses.  Note, however, that none of these subclasses must be instantiated directly "
   "but must instead be instantiated indirectly through the SFX interface.\n\n"
   
   "@section SFXSource_playonce Play-Once Sources\n\n"
   
   "Often, a sound source need only exist for the duration of the sound it is playing.  In this case "
   "so-called \"play-once\" sources simplify the bookkeeping involved by leaving the deletion of "
   "sources that have expired their playtime to the sound system.\n\n"
   
   "Play-once sources can be created in either of two ways:\n"
   
   "- sfxPlayOnce(): Directly create a play-once source from a SFXTrack or file.\n"
   "- sfxDeleteWhenStopped(): Retroactively turn any source into a play-once source that will automatically "
      "be deleted when moving into stopped state.\n\n"
   
   "@see sfxPlayOnce\n"
   "@see sfxDeleteWhenStopped\n\n"
   
   "@section SFXSource_hierarchies Source Hierarchies\n\n"
   
   "Source are arranged into playback hierarchies where a parent source will scale some of the properties of its "
   "children and also hand on any play(), pause(), and stop() commands to them.  This allows to easily group sounds "
   "into logical units that can then be operated on as a whole.\n\n"
   
   "An example of this is the segregation of sounds according to their use in the game.  Volume levels of background "
   "music, in-game sound effects, and character voices will usually be controlled independently and putting their sounds "
   "into different hierarchies allows to achieve that easily.\n\n"
   
   "The source properties that are scaled by parent values are:\n"
   
   "- volume,\n"
   "- pitch, and\n"
   "- priority\n\n"
   
   "This means that if a parent has a volume of 0.5, the child will play at half the effective volume it would otherwise "
   "have.\n\n"

   "Additionally, parents affect the playback state of their children:\n\n"
   
   "- A parent that is in stopped state will force all its direct and indirect children into stopped state.\n"
   "- A parent that is in paused state will force all its direct and indirect children that are playing into paused state.  However, "
      "children that are in stopped state will not be affected.\n"
   "- A parent that is in playing state will not affect the playback state of its children.\n\n"

   "Each source maintains a state that is wants to be in which may differ from the state that is enforced on it by its "
   "parent.  If a parent changes its states in a way that allows a child to move into its desired state, the child will do "
   "so.\n\n"
      
   "For logically grouping sources, instantiate the SFXSource class directly and make other sources children to it.  A "
   "source thus instantiated will not effect any real sound output on its own but will influence the sound output of its "
   "direct and indirect children.\n\n"
   
   "@note Be aware that the property values used to scale child property values are the @b effective values.  For example, "
      "the value used to scale the volume of a child is the @b effective volume of the parent, i.e. the volume after fades, "
      "distance attenuation, etc. has been applied.\n\n"
   
   "@see SFXDescription::sourceGroup\n"
   
   "@section SFXSource_volume Volume Attenuation\n\n"
   
   "During its lifetime, the volume of a source will be continually updated.  This update process always progresses "
   "in a fixed set of steps to compute the final effective volume of the source based on the base volume level "
   "that was either assigned from the SFXDescription associated with the source (SFXDescription::volume) or manually "
   "set by the user.  The process of finding a source's final effective volume is called \"volume attenuation\".  The "
   "steps involved in attenuating a source's volume are (in order):\n"
   
   "<dl>\n"
      "<dt>Fading</dt>\n"
      "<dd>If the source currently has a fade-effect applied, the volume is interpolated along the currently active fade curve.</dd>\n"
      "<dt>Modulation</dt>\n"
      "<dd>If the source is part of a hierarchy, it's volume is scaled according to the effective volume of its parent.</dd>\n"
      "<dt>Distance Attenuation</dt>\n"
      "<dd>If the source is a 3D sound source, then the volume is interpolated according to the distance model in effect and "
         "current listener position and orientation (see @ref SFX_3d).</dd>\n"
   "</dl>\n\n"
   
   "@see SFXDescription::volume\n"
   "@see SFXDescription::is3d\n"
   
   "@section SFXSource_fades Volume Fades\n\n"
   
   "To ease-in and ease-out playback of a sound, fade effects may be applied to sources.  A fade will either go from "
   "zero volume to full effective volume (fade-in) or from full effective volume to zero volume (fade-out).\n\n"
   
   "Fading is coupled to the play(), pause(), and stop() methods as well as to loop iterations when SFXDescription::fadeLoops "
   "is true for the source.  play() and the start of a loop iteration will trigger a fade-in whereas pause(), stop() and the "
   "end of loop iterations will trigger fade-outs.\n\n"
   
   "For looping sources, if SFXDescription::fadeLoops is false, only the initial play() will trigger a fade-in and no further "
   "fading will be applied to loop iterations.\n\n"
   
   "By default, the fade durations will be governed by the SFXDescription::fadeInTime and SFXDescription::fadeOutTime properties "
   "of the SFXDescription attached to the source.  However, these may be overridden on a per-source basis by setting fade times "
   "explicitly with setFadeTimes().  Additionally, the set values may be overridden for individual play(), pause(), and stop() "
   "calls by supplying appropriate fadeInTime/fadeOutTime parameters.\n\n"
   
   "By default, volume will interpolate linearly during fades.  However, custom interpolation curves can be assigned through the "
   "SFXDescription::fadeInEase and SFXDescription::fadeOutTime properties.\n\n"
   
   "@see SFXDescription::fadeInTime\n"
   "@see SFXDescription::fadeOutTime\n"
   "@see SFXDescription::fadeInEase\n"
   "@see SFXDescription::fadeOutEase\n"
   "@see SFXDescription::fadeLoops\n"
         
   "@section SFXSource_cones Sound Cones\n\n"
   
   "@see SFXDescription::coneInsideAngle\n"
   "@see SFXDescription::coneOutsideAngle\n"
   "@see SFXDescription::coneOutsideVolume\n"
   
   "@section SFXSource_doppler Doppler Effect\n\n"
   
   "@see sfxGetDopplerFactor\n"
   "@see sfxSetDopplerFactor\n"
   "@see SFXAmbience::dopplerFactor\n"
   
   "@section SFXSource_markers Playback Markers\n\n"
   
   "Playback markers allow to attach notification triggers to specific playback positions.  Once the "
   "play cursor crosses a position for which a marker is defined, the #onMarkerPassed callback will "
   "be triggered on the SFXSource thus allowing to couple script logic to .\n\n"
   
   "Be aware that the precision with which marker callbacks are triggered are bound by global source "
   "update frequency.  Thus there may be a delay between the play cursor actually passing a marker position "
   "and the callback being triggered.\n\n"

   "@ingroup SFX\n"
);


IMPLEMENT_CALLBACK( SFXSource, onStatusChange, void, ( SFXStatus newStatus ), ( newStatus ),
   "Called when the playback status of the source changes.\n"
   "@param newStatus The new playback status." );
IMPLEMENT_CALLBACK( SFXSource, onParameterValueChange, void, ( SFXParameter* parameter ), ( parameter ),
   "Called when a parameter attached to the source changes value.\n"
   "This callback will be triggered before the value change has actually been applied to the source.\n"
   "@param parameter The parameter that has changed value.\n"
   "@note This is also triggered when the parameter is first attached to the source." );


//-----------------------------------------------------------------------------

SFXSource::SFXSource()
   : mStatus( SFXStatusStopped ),
     mSavedStatus( SFXStatusNull ),
     mStatusCallback( NULL ),
     mPitch( 1.f ),
     mModulativePitch( 1.f ),
     mEffectivePitch( 1.f ),
     mVolume( 1.f ),
     mPreFadeVolume( 1.f ),
     mFadedVolume( 1.f ),
     mModulativeVolume( 1.f ),
     mPreAttenuatedVolume( 1.f ),
     mAttenuatedVolume( 1.f ),
     mPriority( 0 ),
     mModulativePriority( 1.f ),
     mEffectivePriority( 0 ),
     mVelocity( 0, 0, 0 ),
     mTransform( true ),
     mMinDistance( 1 ),
     mMaxDistance( 100 ),
     mConeInsideAngle( 360 ),
     mConeOutsideAngle( 360 ),
     mConeOutsideVolume( 1 ),
     mDescription( NULL ),
     mTransformScattered( false ),
     mPlayStartTick( 0 ),
     mFadeSegmentEase( NULL ),
     mFadeInTime( 0.f ),
     mFadeOutTime( 0.f ),
     mFadeInPoint( -1.f ),
     mFadeOutPoint( -1.f ),
     mFadeSegmentType( FadeSegmentNone ),
     mFadeSegmentStartPoint( 0.f ),
     mFadeSegmentEndPoint( 0.f ),
     mSavedFadeTime( -1.f ),
     mDistToListener( 0.f )
{
   VECTOR_SET_ASSOCIATION( mParameters );
}

//-----------------------------------------------------------------------------

SFXSource::SFXSource( SFXTrack* track, SFXDescription* description )
   : mStatus( SFXStatusStopped ),
     mSavedStatus( SFXStatusNull ),
     mTrack( track ),
     mDescription( description ),
     mStatusCallback( NULL ),
     mPitch( 1.f ),
     mModulativePitch( 1.f ),
     mEffectivePitch( 1.f ),
     mVolume( 1.f ),
     mPreFadeVolume( 1.f ),
     mFadedVolume( 1.f ),
     mModulativeVolume( 1.f ),
     mPreAttenuatedVolume( 1.f ),
     mAttenuatedVolume( 1.f ),
     mPriority( 0 ),
     mModulativePriority( 1.f ),
     mEffectivePriority( 0 ),
     mVelocity( 0, 0, 0 ),
     mTransform( true ),
     mMinDistance( 1 ),
     mMaxDistance( 100 ),
     mConeInsideAngle( 360 ),
     mConeOutsideAngle( 360 ),
     mConeOutsideVolume( 1 ),
     mTransformScattered( false ),
     mPlayStartTick( 0 ),
     mFadeInTime( 0.f ),
     mFadeOutTime( 0.f ),
     mFadeSegmentEase( NULL ),
     mFadeInPoint( -1.f ),
     mFadeOutPoint( -1.f ),
     mFadeSegmentType( FadeSegmentNone ),
     mFadeSegmentStartPoint( 0.f ),
     mFadeSegmentEndPoint( 0.f ),
     mSavedFadeTime( -1.f ),
     mDistToListener( 0.f )
{
   VECTOR_SET_ASSOCIATION( mParameters );
   
   if( !description && track )
      mDescription = track->getDescription();
      
   // Make sure we get notified when our datablocks go away
   // so we can kill this source.
      
   if( mTrack != NULL )
      deleteNotify( mTrack );
   deleteNotify( mDescription );
}

//-----------------------------------------------------------------------------

SFXSource::~SFXSource()
{
   // Disconnect from any remaining parameters.
   
   while( !mParameters.empty() )
   {
      mParameters.last()->getEventSignal().remove( this, &SFXSource::_onParameterEvent );
      mParameters.decrement();
   }
}

//-----------------------------------------------------------------------------

void SFXSource::initPersistFields()
{
   addGroup( "Sound" );
   
      addProtectedField( "description", TypeSFXDescriptionName, Offset( mDescription, SFXSource ),
         &_setDescription, &_getDescription,
         "The playback configuration that determines the initial sound properties and setup.\n"
         "Any SFXSource must have an associated SFXDescription." );
         
      addField( "statusCallback", TypeString, Offset( mStatusCallback, SFXSource ),
         "Name of function to call when the status of the source changes.\n\n"
         "The source that had its status changed is passed as the first argument to the function and the "
         "new status of the source is passed as the second argument." );
         
   endGroup( "Sound" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXSource::processArguments( S32 argc, const char **argv )
{
   // Don't allow subclasses of this to be created via script.  Force
   // usage of the SFXSystem functions.
   
   if( typeid( *this ) != typeid( SFXSource ) )
   {
      Con::errorf( ConsoleLogEntry::Script, "Use sfxCreateSource, sfxPlay, or sfxPlayOnce!" );
      return false;
   }
   else
      return Parent::processArguments( argc, argv );
}

//-----------------------------------------------------------------------------

bool SFXSource::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   // Make sure we have a description.
                  
   if( !mDescription )
   {
      Con::errorf( "SFXSource::onAdd() - no description set on source %i (%s)", getId(), getName() );
      return false;
   }
         
   // Set our initial properties.
        
   _setVolume( mDescription->mVolume ); 
   _setPitch( mDescription->mPitch );
   _setPriority( mDescription->mPriority );
   _setFadeTimes( mDescription->mFadeInTime, mDescription->mFadeOutTime );

   _setMinMaxDistance( mDescription->mMinDistance, mDescription->mMaxDistance );
   _setCone( F32( mDescription->mConeInsideAngle ),
             F32( mDescription->mConeOutsideAngle ),
             mDescription->mConeOutsideVolume );

   // Add the parameters from the description.
   
   for( U32 i = 0; i < SFXDescription::MaxNumParameters; ++ i )
   {
      StringTableEntry name = mDescription->mParameters[ i ];
      if( name && name[ 0 ] )
         _addParameter( name );
   }

   // Add the parameters from the track.
   
   if( mTrack != NULL )
      for( U32 i = 0; i < SFXTrack::MaxNumParameters; ++ i )
      {
         StringTableEntry name = mTrack->getParameter( i );
         if( name && name[ 0 ] )
            _addParameter( name );
      }

   // Add us to the description's source group.
   
   if( mDescription->mSourceGroup )
      mDescription->mSourceGroup->addObject( this );

   // Add to source set.

   if( Sim::getSFXSourceSet() )
      Sim::getSFXSourceSet()->addObject( this );
      
   // Register with SFX system.
      
   SFX->_onAddSource( this );
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXSource::onRemove()
{
   stop();

   // Let the system know.
   SFX->_onRemoveSource( this );
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSource] removed source '%i'", getId() );
   #endif
   
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SFXSource::onDeleteNotify( SimObject* object )
{
   if( object == mTrack )
   {
      deleteObject();
      return;
   }

   Parent::onDeleteNotify( object );
}

//-----------------------------------------------------------------------------

bool SFXSource::acceptsAsChild( SimObject* object )
{
   return ( dynamic_cast< SFXSource* >( object ) != NULL );
}

//-----------------------------------------------------------------------------

void SFXSource::onGroupAdd()
{
   Parent::onGroupAdd();
   
   SFXSource* source = dynamic_cast< SFXSource* >( getGroup() );
   if( source )
   {
      if( source != mDescription->mSourceGroup )
         mFlags.set( CustomGroupFlag );
      else
         mFlags.clear( CustomGroupFlag );
   }

   //DRTODO: sync up playback state
}

//-----------------------------------------------------------------------------

SFXSource* SFXSource::getSourceGroup() const
{
   return dynamic_cast< SFXSource* >( getGroup() );
}

//-----------------------------------------------------------------------------

F32 SFXSource::getElapsedPlayTime() const
{
   return F32( mPlayTimer.getPosition() ) / 1000.f;
}

//-----------------------------------------------------------------------------

F32 SFXSource::getElapsedPlayTimeCurrentCycle() const
{
   // In this base class, we can't make assumptions about total playtimes
   // and thus cannot clamp the playtimer into range for the current cycle.
   // This needs to be done by subclasses.
   
   return F32( mPlayTimer.getPosition() ) / 1000.f;
}

//-----------------------------------------------------------------------------

F32 SFXSource::getTotalPlayTime() const
{
   return Float_Inf;
}

//-----------------------------------------------------------------------------

void SFXSource::play( F32 fadeInTime )
{
   SFXStatus status = getStatus();
   
   // Return if the source is already playing.
   
   if( status == SFXStatusPlaying )
   {
      // Revert fade-out if there is one.
      
      if( mFadeSegmentType != FadeSegmentNone && mFadeSegmentType != FadeSegmentPlay )
      {
         // Let easing curve remain in place.  Otherwise we would have to
         // search the fade-in's easing curve for the point matching the
         // current fade volume in order to prevent volume pops.
         
         mFadeSegmentType = FadeSegmentPlay;
      }
   
      return;
   }
   
   // First check the parent source.  If it is not playing,
   // only save our non-inherited state and return.

   SFXSource* sourceGroup = getSourceGroup();
   if( sourceGroup != NULL && !sourceGroup->isPlaying() )
   {
      mSavedStatus = SFXStatusPlaying;
      mSavedFadeTime = fadeInTime;

      return;
   }

   // Add fades, if required.
   
   if( fadeInTime == -1.f )
      fadeInTime = mFadeInTime;
      
   if( status == SFXStatusPaused && fadeInTime > 0.f )
   {
      // Source is paused.  Set up temporary fade-in segment.
      
      mFadeSegmentEase = &mDescription->mFadeInEase;
      mFadeSegmentType = FadeSegmentPlay;
      mFadeSegmentStartPoint = getElapsedPlayTimeCurrentCycle();
      mFadeSegmentEndPoint = mFadeSegmentStartPoint + fadeInTime;
   }
   else if( fadeInTime > 0.f )
   {
      mFadeInPoint = fadeInTime;
      
      // Add fade-out if play-time of the source is finite and
      // if it is either not looping or has fading enabled on loops.
      
      F32 totalPlayTime = getTotalPlayTime();
      if( !mIsInf_F( totalPlayTime ) && mDescription->mFadeOutTime > 0.f && ( !mDescription->mIsLooping || mDescription->mFadeLoops ) )
      {
         mFadeOutPoint = totalPlayTime - mDescription->mFadeOutTime;

         // If there's an intersection between fade-in and fade-out, move them
         // to the midpoint.
      
         if( mFadeOutPoint < mFadeInPoint )
         {
            F32 midPoint = mFadeOutPoint + ( mFadeInPoint - mFadeOutPoint / 2 );
            
            mFadeInPoint = midPoint;
            mFadeOutPoint = midPoint;
         }
      }
   }
   else
   {
      mFadeInPoint = -1.f;
      mFadeOutPoint = -1.f;
   }
   
   // Start playback.
      
   if( status != SFXStatusPaused )
   {
      mPlayStartTick = Platform::getVirtualMilliseconds();
      mPlayTimer.reset();
   }
      
   _setStatus( SFXStatusPlaying );
   _play();

   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXSource] Started playback of source '%i'", getId() );
   #endif
}

//-----------------------------------------------------------------------------

void SFXSource::pause( F32 fadeOutTime )
{
   SFXStatus status = getStatus();
   if( status != SFXStatusPlaying )
      return;
   
   // Pause playback.
      
   if( fadeOutTime == -1.f )
      fadeOutTime = mFadeOutTime;
      
   if( fadeOutTime > 0.f )
      _setupFadeOutSegment( FadeSegmentPause, fadeOutTime );
   else
   {
      _setStatus( SFXStatusPaused );
      _pause();
      
      mFadeSegmentType = FadeSegmentNone;
      mPreFadeVolume = mVolume;

      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXSource] Paused playback of source '%i'", getId() );
      #endif
   }
}

//-----------------------------------------------------------------------------

void SFXSource::stop( F32 fadeOutTime )
{
   SFXStatus status = getStatus();
   if( status == SFXStatusStopped )
      return;

   if( status == SFXStatusPaused )
   {
      _setStatus( SFXStatusStopped );
      _stop();
      return;
   }
   
   if( fadeOutTime == -1.f )
      fadeOutTime = mFadeOutTime;
      
   if( fadeOutTime > 0.f )
      _setupFadeOutSegment( FadeSegmentStop, fadeOutTime );
   else
   {
      _setStatus( SFXStatusStopped );
      _stop();

      mFadeSegmentType = FadeSegmentNone;
      mPreFadeVolume = mVolume;

      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXSource] Stopped playback of source '%i'", getId() );
      #endif
   }
}

//-----------------------------------------------------------------------------

void SFXSource::update()
{
   if( !isPlaying() )
      return;
      
   _update();      

   // Update our modifiers, if any.
   
   for( ModifierList::Iterator iter = mModifiers.begin();
        iter != mModifiers.end(); )
   {
      ModifierList::Iterator next = iter;
      next ++;

      if( !( *iter )->update() )
      {
         delete *iter;
         mModifiers.erase( iter );
      }

      iter = next;
   }
}

//-----------------------------------------------------------------------------

void SFXSource::_play()
{
   mPlayTimer.start();

   // Resume playback of children that want to be in
   // playing state.
   
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );
      if( source && source->mSavedStatus == SFXStatusPlaying )
         source->play( source->mSavedFadeTime );
   }
}

//-----------------------------------------------------------------------------

void SFXSource::_pause()
{
   // Pause playing child sources.

   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );
      if( source && source->isPlaying() )
      {
         source->pause( 0.f );

         // Save info for resuming playback.

         source->mSavedStatus = SFXStatusPlaying;
         source->mSavedFadeTime = 0.f;
      }
   }
   
   mPlayTimer.pause();
}

//-----------------------------------------------------------------------------

void SFXSource::_stop()
{  
   // Stop all child sources.

   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SFXSource* source = dynamic_cast< SFXSource* >( *iter );
      if( source )
         source->stop( 0.f );
   }

   mPlayTimer.stop();
}

//-----------------------------------------------------------------------------

void SFXSource::_update()
{      
   /// Update our modulated properties.
      
   _updateVolume( SFX->getListener().getTransform() );
   _updatePitch();
   _updatePriority();         
}

//-----------------------------------------------------------------------------

bool SFXSource::_setDescription( void* obj, const char* index, const char* data )
{
   SFXSource* source = reinterpret_cast< SFXSource* >( obj );

   source->mDescription = EngineUnmarshallData< SFXDescription* >()( data );
   if( !source->mDescription )
   {
      Con::errorf( "SFXSource::_setDescription - No SFXDescription '%s'", data );
      return false;
   }

   source->notifyDescriptionChanged();
   
   return false;
}

//-----------------------------------------------------------------------------

const char* SFXSource::_getDescription( void* obj, const char* data )
{
   SFXSource* source = reinterpret_cast< SFXSource* >( obj );
   SFXDescription* description = source->mDescription;
   
   if( !description )
      return "";

   return description->getName();
}

//-----------------------------------------------------------------------------

void SFXSource::_setStatus( SFXStatus status )
{
   if( mStatus == status )
      return;

   mStatus = status;

   // Clear saved status.

   mSavedStatus = SFXStatusNull;

   // Do the callback if we have it.

   if( mStatusCallback && mStatusCallback[0] )
   {
      const char* statusString = SFXStatusToString( mStatus );
      Con::executef( mStatusCallback, getIdString(), statusString );
   }
   else if( getNamespace() )
      onStatusChange_callback( status );
}

//-----------------------------------------------------------------------------

void SFXSource::_updateVolume( const MatrixF& listener )
{
   // Handle fades (compute mFadedVolume).
      
   mFadedVolume = mPreFadeVolume;
   if( mFadeSegmentType != FadeSegmentNone )
   {
      // We have a temporary fade segment.
      
      F32 elapsed;
      if( mDescription->mIsLooping && !mDescription->mFadeLoops )
         elapsed = getElapsedPlayTime();
      else
         elapsed = getElapsedPlayTimeCurrentCycle();
      
      if( elapsed < mFadeSegmentEndPoint )
      {
         const F32 duration = mFadeSegmentEndPoint - mFadeSegmentStartPoint;
         
         if( mFadeSegmentType == FadeSegmentPlay )
         {
            const F32 time = elapsed - mFadeSegmentStartPoint;
            mFadedVolume = mFadeSegmentEase->getValue
               ( time, 0.f, mPreFadeVolume, duration );
         }
         else
         {
            // If the end-point to the ease functions is 0,
            // we'll always get out the start point.  Thus do the whole
            // thing backwards here.
            
            const F32 time = mFadeSegmentEndPoint - elapsed;
            mFadedVolume = mFadeSegmentEase->getValue
               ( time, 0.f, mPreFadeVolume, duration );
         }
      }
      else
      {
         // The fade segment has played.  Remove it.
         
         switch( mFadeSegmentType )
         {
            case FadeSegmentStop:
               stop( 0.f );
               break;
               
            case FadeSegmentPause:
               pause( 0.f );
               break;
               
            case FadeSegmentPlay: // Nothing to do.               
            default:
               break;
         }
         
         mFadeSegmentType = FadeSegmentNone;
         mPreFadeVolume = mVolume;
      }
   }
   else if( mFadeInPoint != -1 || mFadeOutPoint != -1 )
   {
      F32 elapsed;
      if( mDescription->mIsLooping && !mDescription->mFadeLoops )
         elapsed = getElapsedPlayTime();
      else
         elapsed = getElapsedPlayTimeCurrentCycle();
      
      // Check for fade-in.
      
      if( mFadeInPoint != -1 )
      {
         if( elapsed < mFadeInPoint )
            mFadedVolume = mDescription->mFadeInEase.getValue( elapsed, 0.f, mPreFadeVolume, mFadeInPoint );
         else if( mDescription->mIsLooping && !mDescription->mFadeLoops )
         {
            // Deactivate fade-in so we don't see it on further loops.
            mFadeInPoint = -1;
         }
      }
      
      // Check for fade-out.
      
      if( mFadeOutPoint != -1 && elapsed > mFadeOutPoint )
      {
         const F32 totalPlayTime = getTotalPlayTime();
         const F32 duration = totalPlayTime - mFadeOutPoint;
         const F32 time = totalPlayTime - elapsed;
         
         mFadedVolume = mDescription->mFadeOutEase.getValue( time, 0.f, mPreFadeVolume, duration );
      }
   }

   // Compute the pre-attenuated volume.
   
   mPreAttenuatedVolume =
        mFadedVolume
      * mModulativeVolume;
      
   SFXSource* group = getSourceGroup();
   if( group )
      mPreAttenuatedVolume *= group->getAttenuatedVolume();

   if( !is3d() ) 
   {
      mDistToListener = 0.0f;
      mAttenuatedVolume = mPreAttenuatedVolume; 
	}
   else
   {
      // For 3D sounds, compute distance attenuation.

      Point3F pos, lpos;
      mTransform.getColumn( 3, &pos );
      listener.getColumn( 3, &lpos );

      mDistToListener = ( pos - lpos ).len();
      mAttenuatedVolume = SFXDistanceAttenuation(
         SFX->getDistanceModel(),
         mMinDistance,
         mMaxDistance,
         mDistToListener,
         mPreAttenuatedVolume,
         SFX->getRolloffFactor() );
   }
}

//-----------------------------------------------------------------------------

void SFXSource::_updatePitch()
{
   mEffectivePitch = mModulativePitch * mPitch;
}

//-----------------------------------------------------------------------------

void SFXSource::_updatePriority()
{      
   mEffectivePriority = mPriority * mModulativePriority;

   SFXSource* group = getSourceGroup();
   if( group )
      mEffectivePriority *= group->getEffectivePriority();
}

//-----------------------------------------------------------------------------

void SFXSource::setTransform( const MatrixF& transform )
{
   mTransform = transform;
}

//-----------------------------------------------------------------------------

void SFXSource::setVelocity( const VectorF& velocity )
{
   mVelocity = velocity;
}

//-----------------------------------------------------------------------------

void SFXSource::setFadeTimes( F32 fadeInTime, F32 fadeOutTime )
{
   _setFadeTimes( fadeInTime, fadeOutTime );
   
   if( mFadeInTime >= 0.f || mFadeOutTime >= 0.f )
      mFlags.set( CustomFadeFlag );
}

//-----------------------------------------------------------------------------

void SFXSource::_setFadeTimes( F32 fadeInTime, F32 fadeOutTime )
{
   if( fadeInTime >= 0.f )
      mFadeInTime = getMax( 0.f, fadeInTime );
   else
      mFadeInTime = mDescription->mFadeInTime;
      
   if( fadeOutTime >= 0.f )
      mFadeOutTime = getMax( 0.f, fadeOutTime );
   else
      mFadeOutTime = mDescription->mFadeOutTime;
}

//-----------------------------------------------------------------------------

void SFXSource::_setupFadeOutSegment( FadeSegmentType type, F32 fadeOutTime )
{
   // Get the current faded volume as the start volume so
   // that we correctly fade when starting in a fade.
   
   _updateVolume( SFX->getListener().getTransform() );
   mPreFadeVolume = mFadedVolume;

   mFadeSegmentEase = &mDescription->mFadeOutEase;
   mFadeSegmentType = type;

   if( mDescription->mIsLooping && mDescription->mFadeLoops )
   {
      mFadeSegmentStartPoint = getElapsedPlayTimeCurrentCycle();
      mFadeSegmentEndPoint = getMin( mFadeSegmentStartPoint + fadeOutTime, getTotalPlayTime() );
   }
   else
   {
      mFadeSegmentStartPoint = getElapsedPlayTime();
      mFadeSegmentEndPoint = mFadeSegmentStartPoint + fadeOutTime;
   }
}

//-----------------------------------------------------------------------------

void SFXSource::_setMinMaxDistance( F32 min, F32 max )
{
   mMinDistance = getMax( 0.0f, min );
   mMaxDistance = getMax( mMinDistance, max );
}

//-----------------------------------------------------------------------------

void SFXSource::_setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume )
{
   mConeInsideAngle = mClampF( innerAngle, 0.0, 360.0 );
   mConeOutsideAngle = mClampF( outerAngle, mConeInsideAngle, 360.0 );
   mConeOutsideVolume = mClampF( outerVolume, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------

void SFXSource::_setVolume( F32 volume )
{
   mVolume = mClampF( volume, 0.f, 1.f );
   mPreFadeVolume = mVolume;
   _updateVolume( SFX->getListener( 0 ).getTransform() );
}

//-----------------------------------------------------------------------------

void SFXSource::setModulativeVolume( F32 value )
{
   mModulativeVolume = mClampF( value, 0.f, 1.f );
   _updateVolume( SFX->getListener( 0 ).getTransform() );
}

//-----------------------------------------------------------------------------

void SFXSource::_setPitch( F32 pitch )
{
   mPitch = mClampF( pitch, 0.001f, 2.0f );
   _updatePitch();
}

//-----------------------------------------------------------------------------

void SFXSource::setModulativePitch( F32 value )
{
   mModulativePitch = mClampF( value, 0.001f, 2.0f );
   _updatePitch();
}

//-----------------------------------------------------------------------------

void SFXSource::_setPriority( F32 priority )
{
   mPriority = priority;
   _updatePriority();
}

//-----------------------------------------------------------------------------

void SFXSource::setModulativePriority( F32 value )
{
   mModulativePriority = value;
   _updatePriority();
}

//-----------------------------------------------------------------------------

SFXTrack* SFXSource::getTrack() const
{
   return mTrack;
}

//-----------------------------------------------------------------------------

void SFXSource::notifyDescriptionChanged()
{
   if( !mFlags.test( CustomVolumeFlag ) )
      _setVolume( mDescription->mVolume );
   if( !mFlags.test( CustomPitchFlag ) )
      _setPitch( mDescription->mPitch );
   if( !mFlags.test( CustomPriorityFlag ) )
      _setPriority( mDescription->mPriority );
   if( !mFlags.test( CustomRadiusFlag ) )
      _setMinMaxDistance( mDescription->mMinDistance, mDescription->mMaxDistance );
   if( !mFlags.test( CustomConeFlag ) )
      _setCone( mDescription->mConeInsideAngle, mDescription->mConeOutsideAngle, mDescription->mConeOutsideVolume );
   if( !mFlags.test( CustomFadeFlag ) )
      _setFadeTimes( mDescription->mFadeInTime, mDescription->mFadeOutTime );
   if( !mFlags.test( CustomGroupFlag ) && mDescription->mSourceGroup != NULL && getGroup() != mDescription->mSourceGroup )
      mDescription->mSourceGroup->addObject( this );
}

//-----------------------------------------------------------------------------

void SFXSource::notifyTrackChanged()
{
   //RDTODO
}

//-----------------------------------------------------------------------------

template< class T >
void SFXSource::_clearModifiers()
{
   for( ModifierList::Iterator iter = mModifiers.begin();
        iter != mModifiers.end(); )
   {
      ModifierList::Iterator next = iter;
      next ++;

      if( dynamic_cast< T* >( *iter ) )
      {
         delete *iter;
         mModifiers.erase( iter );
      }

      iter = next;
   }
}

//-----------------------------------------------------------------------------

void SFXSource::addModifier( SFXModifier* modifier )
{
   mModifiers.pushBack( modifier );
}

//-----------------------------------------------------------------------------

void SFXSource::addMarker( const String& name, F32 pos )
{
   addModifier( new SFXMarkerModifier( this, name, pos ) );
}

//-----------------------------------------------------------------------------

void SFXSource::addParameter( SFXParameter* parameter )
{
   for( U32 i = 0; i < mParameters.size(); ++ i )
      if( mParameters[ i ] == parameter )
         return;
         
   mParameters.push_back( parameter );
   parameter->getEventSignal().notify( this, &SFXSource::_onParameterEvent );
   
   // Trigger an initial value-changed event so the
   // current parameter value becomes effective.
   
   _onParameterEvent( parameter, SFXParameterEvent_ValueChanged );
}

//-----------------------------------------------------------------------------

void SFXSource::removeParameter( SFXParameter* parameter )
{
   for( U32 i = 0; i < mParameters.size(); ++ i )
      if( mParameters[ i ] == parameter )
      {
         mParameters[ i ]->getEventSignal().remove( this, &SFXSource::_onParameterEvent );
         mParameters.erase( i );
         break;
      }
}

//-----------------------------------------------------------------------------

void SFXSource::_addParameter( StringTableEntry name )
{
   SFXParameter* parameter = dynamic_cast< SFXParameter* >(
      Sim::getSFXParameterGroup()->findObjectByInternalName( name )
   );
   
   if( !parameter )
   {
      Con::errorf( "SFXSource::_addParameter - no parameter called '%s'", name );
      return;
   }
   
   addParameter( parameter );
}

//-----------------------------------------------------------------------------

void SFXSource::_onParameterEvent( SFXParameter* parameter, SFXParameterEvent event )
{
   switch( event )
   {
      case SFXParameterEvent_ValueChanged:
      
         onParameterValueChange_callback( parameter );
            
         switch( parameter->getChannel() )
         {
            case SFXChannelStatus:
            {
               F32 value = parameter->getValue();
               if( value >= F32( SFXStatusPlaying ) && value <= F32( SFXStatusStopped ) )
               {
                  SFXStatus status = ( SFXStatus ) U32( value );
                  switch( status )
                  {
                     case SFXStatusPlaying:     play(); break;
                     case SFXStatusPaused:      pause(); break;
                     case SFXStatusStopped:     stop(); break;
                     
                     default:
                        break;
                  }
               }
               break;
            }
               
            case SFXChannelVolume:
               setVolume( parameter->getValue() );
               break;
               
            case SFXChannelPitch:
               setPitch( parameter->getValue() );
               break;
               
            case SFXChannelPriority:
               setPriority( parameter->getValue() );
               break;
               
            case SFXChannelMinDistance:
               setMinMaxDistance( parameter->getValue(), mMaxDistance );
               break;
               
            case SFXChannelMaxDistance:
               setMinMaxDistance( mMinDistance, parameter->getValue() );
               break;
               
            case SFXChannelPositionX:
            {
               Point3F position = mTransform.getPosition();
               position.x = parameter->getValue();
               mTransform.setPosition( position );
               setTransform( mTransform );
               break;
            }

            case SFXChannelPositionY:
            {
               Point3F position = mTransform.getPosition();
               position.y = parameter->getValue();
               mTransform.setPosition( position );
               setTransform( mTransform );
               break;
            }

            case SFXChannelPositionZ:
            {
               Point3F position = mTransform.getPosition();
               position.z = parameter->getValue();
               mTransform.setPosition( position );
               setTransform( mTransform );
               break;
            }
            
            case SFXChannelRotationX:
            {
               EulerF angles = mTransform.toEuler();
               angles.x = parameter->getValue();
               MatrixF transform( angles );
               transform.setPosition( mTransform.getPosition() );
               setTransform( transform );
               break;
            }
            
            case SFXChannelRotationY:
            {
               EulerF angles = mTransform.toEuler();
               angles.y = parameter->getValue();
               MatrixF transform( angles );
               transform.setPosition( mTransform.getPosition() );
               setTransform( transform );
               break;
            }

            case SFXChannelRotationZ:
            {
               EulerF angles = mTransform.toEuler();
               angles.z = parameter->getValue();
               MatrixF transform( angles );
               transform.setPosition( mTransform.getPosition() );
               setTransform( transform );
               break;
            }

            case SFXChannelVelocityX:
            {
               mVelocity.x = parameter->getValue();
               setVelocity( mVelocity );
               break;
            }

            case SFXChannelVelocityY:
            {
               mVelocity.y = parameter->getValue();
               setVelocity( mVelocity );
               break;
            }

            case SFXChannelVelocityZ:
            {
               mVelocity.z = parameter->getValue();
               setVelocity( mVelocity );
               break;
            }
               
            default:
               break;
         }
         break;
         
      case SFXParameterEvent_Deleted:
         removeParameter( parameter );
         break;
   }
}

//-----------------------------------------------------------------------------

void SFXSource::_scatterTransform()
{
   if( mDescription )
   {
      Point3F position = mTransform.getPosition();
      for( U32 i = 0; i < 3; ++ i )
      {
         F32 scatterDist = mDescription->mScatterDistance[ i ];
         if( scatterDist != 0.f )
            position[ 0 ] += gRandGen.randF( - scatterDist, scatterDist );
      }
      mTransform.setPosition( position );
   }
   
   mTransformScattered = true;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, play, void, ( F32 fadeInTime ), ( -1.f ),
   "Start playback of the source.\n"
   "If the sound data for the source has not yet been fully loaded, there will be a delay after calling "
   "play and playback will start after the data has become available.\n\n"
   "@param fadeInTime Seconds for the sound to reach full volume.  If -1, the SFXDescription::fadeInTime "
      "set in the source's associated description is used.  Pass 0 to disable a fade-in effect that may "
      "be configured on the description." )
{
   object->play( fadeInTime );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, stop, void, ( F32 fadeOutTime ), ( -1.f ),
   "Stop playback of the source.\n"
   "@param fadeOutTime Seconds for the sound to fade down to zero volume.  If -1, the SFXDescription::fadeOutTime "
      "set in the source's associated description is used.  Pass 0 to disable a fade-out effect that may be "
      "configured on the description.\n"
      "Be aware that if a fade-out effect is used, the source will not immediately transtion to stopped state but "
      "will rather remain in playing state until the fade-out time has expired." )
{
   object->stop( fadeOutTime );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, pause, void, ( F32 fadeOutTime ), ( -1.f ),
   "Pause playback of the source.\n"
   "@param fadeOutTime Seconds for the sound to fade down to zero volume.  If -1, the SFXDescription::fadeOutTime "
      "set in the source's associated description is used.  Pass 0 to disable a fade-out effect that may be "
      "configured on the description.\n"
      "Be aware that if a fade-out effect is used, the source will not immediately to paused state but will "
      "rather remain in playing state until the fade-out time has expired.." )
{
   object->pause( fadeOutTime );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, isPlaying, bool, (),,
   "Test whether the source is currently playing.\n"
   "@return True if the source is in playing state, false otherwise.\n\n"
   "@see play\n"
   "@see getStatus\n"
   "@see SFXStatus" )
{
   return object->isPlaying();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, isPaused, bool, (),,
   "Test whether the source is currently paused.\n"
   "@return True if the source is in paused state, false otherwise.\n\n"
   "@see pause\n"
   "@see getStatus\n"
   "@see SFXStatus" )
{
   return object->isPaused();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, isStopped, bool, (),,
   "Test whether the source is currently stopped.\n"
   "@return True if the source is in stopped state, false otherwise.\n\n"
   "@see stop\n"
   "@see getStatus\n"
   "@see SFXStatus" )
{
   return object->isStopped();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getStatus, SFXStatus, (),,
   "Get the current playback status.\n"
   "@return Te current playback status\n" )
{
   return object->getStatus();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getVolume, F32, (),,
   "Get the current base volume level of the source.\n"
   "This is not the final effective volume that the source is playing at but rather the starting "
   "volume level before source group modulation, fades, or distance-based volume attenuation are applied.\n\n"
   "@return The current base volume level.\n\n"
   "@see setVolume\n"
   "@see SFXDescription::volume\n\n"
   "@ref SFXSource_volume" )
{
   return object->getVolume();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, setVolume, void, ( F32 volume ),,
   "Set the base volume level for the source.\n"
   "This volume will be the starting point for source group volume modulation, fades, and distance-based "
   "volume attenuation.\n\n"
   "@param volume The new base volume level for the source.  Must be 0>=volume<=1.\n\n"
   "@see getVolume\n\n"
   "@ref SFXSource_volume" )
{
   object->setVolume( volume );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getAttenuatedVolume, F32, (),,
   "Get the final effective volume level of the source.\n\n"
   "This method returns the volume level as it is after source group volume modulation, fades, and distance-based "
   "volume attenuation have been applied to the base volume level.\n\n"
   "@return The effective volume of the source.\n\n"
   "@ref SFXSource_volume" )
{
   return object->getAttenuatedVolume();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getFadeInTime, F32, (),,
   "Get the fade-in time set on the source.\n"
   "This will initially be SFXDescription::fadeInTime.\n\n"
   "@return The fade-in time set on the source in seconds.\n\n"
   "@see SFXDescription::fadeInTime\n\n"
   "@ref SFXSource_fades" )
{
   return object->getFadeInTime();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getFadeOutTime, F32, (),,
   "Get the fade-out time set on the source.\n"
   "This will initially be SFXDescription::fadeOutTime.\n\n"
   "@return The fade-out time set on the source in seconds.\n\n"
   "@see SFXDescription::fadeOutTime\n\n"
   "@ref SFXSource_fades" )
{
   return object->getFadeOutTime();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, setFadeTimes, void, ( F32 fadeInTime, F32 fadeOutTime ),,
   "Set the fade time parameters of the source.\n"
   "@param fadeInTime The new fade-in time in seconds.\n"
   "@param fadeOutTime The new fade-out time in seconds.\n\n"
   "@see SFXDescription::fadeInTime\n"
   "@see SFXDescription::fadeOutTime\n\n"
   "@ref SFXSource_fades" )
{
   object->setFadeTimes( fadeInTime, fadeOutTime );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getPitch, F32, (),,
   "Get the pitch scale of the source.\n"
   "Pitch determines the playback speed of the source (default: 1).\n\n"
   "@return The current pitch scale factor of the source.\n\n"
   "@see setPitch\n"
   "@see SFXDescription::pitch" )
{
   return object->getPitch();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, setPitch, void, ( F32 pitch ),,
   "Set the pitch scale of the source.\n"
   "Pitch determines the playback speed of the source (default: 1).\n\n"
   "@param pitch The new pitch scale factor.\n\n"
   "@see getPitch\n"
   "@see SFXDescription::pitch" )
{
   object->setPitch( pitch );
}

//-----------------------------------------------------------------------------

// Need an overload here as we can't use a default parameter to signal omission of the direction argument
// and we need to properly detect the omission to leave the currently set direction on the source as is.

DEFINE_CALLIN( fnSFXSoure_setTransform1, setTransform, SFXSource, void, ( SFXSource* source, const VectorF& position ),,,
   "Set the position of the source's 3D sound.\n\n"
   "@param position The new position in world space.\n"
   "@note This method has no effect if the source is not a 3D source." )
{
   MatrixF mat = source->getTransform();
   mat.setPosition( position );
   source->setTransform( mat );
}
DEFINE_CALLIN( fnSFXSoure_setTransform2, setTransform, SFXSource, void, ( SFXSource* source, const VectorF& position, const VectorF& direction  ),,,
   "Set the position and orientation of the source's 3D sound.\n\n"
   "@param position The new position in world space.\n"
   "@param direction The forward vector." )
{
   MatrixF mat = source->getTransform();
   mat.setPosition( position );
   mat.setColumn( 1, direction );
   source->setTransform( mat );
}

// Console interop version.

static ConsoleDocFragment _sSetTransform1(
   "Set the position of the source's 3D sound.\n\n"
   "@param position The new position in world space.\n"
   "@note This method has no effect if the source is not a 3D source.",
   "SFXSource",
   "void setTransform( Point3F position )"
);
static ConsoleDocFragment _sSetTransform2(
   "Set the position and orientation of the source's 3D sound.\n\n"
   "@param position The new position in world space.\n"
   "@param direction The forward vector.",
   "SFXSource",
   "void setTransform( Point3F position, Point3F direction )"
);

ConsoleMethod( SFXSource, setTransform, void, 3, 4,
   "( vector position [, vector direction ] ) "
   "Set the position and orientation of a 3D sound source.\n"
   "@hide" )
{
   MatrixF mat = object->getTransform();

   Point3F pos;
   dSscanf( argv[2], "%g %g %g", &pos.x, &pos.y, &pos.z );
   mat.setPosition( pos );
   
   if( argc > 3 )
   {
      Point3F dir;
      dSscanf( argv[ 3 ], "%g %g %g", &dir.x, &dir.y, &dir.z );
      mat.setColumn( 1, dir );
   }
   
   object->setTransform( mat );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, setCone, void, ( F32 innerAngle, F32 outerAngle, F32 outsideVolume ),,
   "Set up the 3D volume cone for the source.\n\n"
   "@param innerAngle Angle of the inner sound cone in degrees (@ref SFXDescription::coneInsideAngle).  Must be 0<=innerAngle<=360.\n"
   "@param outerAngle Angle of the outer sound cone in degrees (@ref SFXDescription::coneOutsideAngle).  Must be 0<=outerAngle<=360.\n"
   "@param outsideVolume Volume scale factor outside of outer cone (@ref SFXDescription::coneOutsideVolume).  Must be 0<=outsideVolume<=1.\n"
   "@note This method has no effect on the source if the source is not 3D.\n\n" )
{
   bool isValid = true;
   
   if( innerAngle < 0.0 || innerAngle > 360.0 )
   {
      Con::errorf( "SFXSource.setCone() - 'innerAngle' must be between 0 and 360" );
      isValid = false;
   }
   if( outerAngle < 0.0 || outerAngle > 360.0 )
   {
      Con::errorf( "SFXSource.setCone() - 'outerAngle' must be between 0 and 360" );
      isValid = false;
   }
   if( outsideVolume < 0.0 || outsideVolume > 1.0 )
   {
      Con::errorf( "SFXSource.setCone() - 'outsideVolume' must be between 0 and 1" );
      isValid = false;
   }
   
   if( !isValid )
      return;
      
   object->setCone( innerAngle, outerAngle, outsideVolume );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getParameterCount, S32, (),,
   "Get the number of SFXParameters that are attached to the source.\n"
   "@return The number of parameters attached to the source.\n\n"
   "@tsexample\n"
      "// Print the name ofo each parameter attached to %source.\n"
      "%numParams = %source.getParameterCount();\n"
      "for( %i = 0; %i < %numParams; %i ++ )\n"
      "   echo( %source.getParameter( %i ).getParameterName() );\n"
   "@endtsexample\n\n"
   "@see getParameter\n"
   "@see addParameter\n" )
{
   return object->getNumParameters();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, addParameter, void, ( SFXParameter* parameter ),,
   "Attach @a parameter to the source,\n\n"
   "Once attached, the source will react to value changes of the given @a parameter.  Attaching a parameter "
   "will also trigger an initial read-out of the parameter's current value.\n\n"
   "@param parameter The parameter to attach to the source." )
{
   if( parameter )
      object->addParameter( parameter );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, removeParameter, void, ( SFXParameter* parameter ),,
   "Detach @a parameter from the source.\n\n"
   "Once detached, the source will no longer react to value changes of the given @a parameter.\n\n"
   "If the parameter is not attached to the source, the method will do nothing.\n\n"
   "@param parameter The parameter to detach from the source.\n" )
{
   if( parameter )
      object->removeParameter( parameter );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, getParameter, SFXParameter*, ( S32 index ),,
   "Get the parameter at the given index.\n"
   "@param index Index of the parameter to fetch.  Must be 0<=index<=getParameterCount().\n"
   "@return The parameter at the given @a index or null if @a index is out of range.\n\n"
   "@tsexample\n"
      "// Print the name ofo each parameter attached to %source.\n"
      "%numParams = %source.getParameterCount();\n"
      "for( %i = 0; %i < %numParams; %i ++ )\n"
      "   echo( %source.getParameter( %i ).getParameterName() );\n"
   "@endtsexample\n\n"
   "@see getParameterCount" )
{
   if( index >= object->getNumParameters() )
   {
      Con::errorf( "SFXSource::getParameter - index out of range: %i", index );
      return 0;
   }
   
   return object->getParameter( index );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXSource, addMarker, void, ( String name, F32 pos ),,
   "Add a notification marker called @a name at @a pos seconds of playback.\n\n"
   "@param name Symbolic name for the marker that will be passed to the onMarkerPassed() callback.\n"
   "@param pos Playback position in seconds when the notification should trigger.  Note that this is a soft limit and there "
      "may be a delay between the play cursor actually passing the position and the callback being triggered.\n\n"
   "@note For looped sounds, the marker will trigger on each iteration.\n\n"
   "@tsexample\n"
   "// Create a new source.\n"
   "$source = sfxCreateSource( AudioMusicLoop2D, \"art/sound/backgroundMusic\" );\n"
   "\n"
   "// Assign a class to the source.\n"
   "$source.class = \"BackgroundMusic\";\n"
   "\n"
   "// Add a playback marker at one minute into playback.\n"
   "$source.addMarker( \"first\", 60 );\n"
   "\n"
   "// Define the callback function.  This function will be called when the playback position passes the one minute mark.\n"
   "function BackgroundMusic::onMarkerPassed( %this, %markerName )\n"
   "{\n"
   "   if( %markerName $= \"first\" )\n"
   "      echo( \"Playback has passed the 60 seconds mark.\" );\n"
   "}\n"
   "\n"
   "// Play the sound.\n"
   "$source.play();\n"
   "@endtsexample" )
{
   object->addMarker( name, pos );
}
