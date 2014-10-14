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

#ifndef _SFXSOURCE_H_
#define _SFXSOURCE_H_

#ifndef _SIMSET_H_
   #include "console/simSet.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _SFXDESCRIPTION_H_
   #include "sfx/sfxDescription.h"
#endif
#ifndef _SFXPARAMETER_H_
   #include "sfx/sfxParameter.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _TIMESOURCE_H_
   #include "core/util/timeSource.h"
#endif
#ifndef _BITSET_H_
   #include "core/bitSet.h"
#endif
#ifndef _TORQUE_LIST_
   #include "core/util/tList.h"
#endif


class SFXTrack;
class SFXDescription;
class SFXSourceGroup;
class SFXModifier;
class EaseF;


/// Baseclass for sources and controllers.
class SFXSource : public SimGroup
{
   public:

      typedef SimGroup Parent;
         
      friend class SFXSystem; // _init
         
   protected:

      typedef Torque::List< SFXModifier* > ModifierList;
      typedef GenericTimeSource< RealMSTimer > TimeSource;
      
      enum Flags
      {
         CustomVolumeFlag           = BIT( 0 ),
         CustomPitchFlag            = BIT( 1 ),
         CustomPriorityFlag         = BIT( 2 ),
         CustomRadiusFlag           = BIT( 3 ),
         CustomConeFlag             = BIT( 4 ),
         CustomFadeFlag             = BIT( 5 ),
         CustomGroupFlag            = BIT( 6 ),
      };
                        
      ///
      BitSet32 mFlags;
      
      /// @name Status
      /// @{

      /// Current playback status.
      SFXStatus mStatus;

      /// The playback status that the source actually wants to be in.
      SFXStatus mSavedStatus;

      /// Console functions to call when the source status changes.
      /// If not set, "onStatusChange" will be called on the source object.
      StringTableEntry mStatusCallback;

      /// Used internally for setting the sound status.
      virtual void _setStatus( SFXStatus newStatus );
                  
      /// Update the playback status of the source.  Meant for subclasses
      /// that need to poll a connected resource.
      virtual void _updateStatus() {}
      
      /// @}

      /// @name Datablocks
      ///
      /// The track datablock is optional but the description datablock is required.
      /// If either of the two goes away, the source will auto-delete itself.
      ///
      /// These members are SimObjectPtr so that temporary track and description
      /// objects that are set to auto-delete will work.
      ///
      /// @{
      
      /// The track being played by this source.
      /// @note Will be null for sources that have been created from SFXStreams.
      SimObjectPtr< SFXTrack > mTrack;

      /// The description holding the SFX sound configuration.
      SimObjectPtr< SFXDescription > mDescription;
      
      /// @}
      
      /// @name Volume
      ///
      /// Volume processing goes through a series of stages.  The last stage, distance attenuation,
      /// happens only for 3D sounds and is done on the device (though the attenuated volume is also
      /// computed within SFX).
      ///
      /// The succession of stages is:
      ///
      /// 1. Fade (Apply fade-in/out if currently active)
      /// 2. Modulate (Apply scale factor set on source)
      /// 3. Modulate (Apply attenuated volume of source group as scale factor)
      /// 4. Attenuate (Apply 3D distance attenuation based on current listener position)
      ///
      /// @{
      
      /// The desired sound volume.
      F32 mVolume;
      
      /// Volume before fade stage.  Used as input to volume computation.  Usually this
      /// corresponds to mVolume but when fading out from an already faded start volume,
      /// this contains the starting mFadedVolume.
      F32 mPreFadeVolume;
      
      /// "mVolume" after fade stage.  Same as "mPreFadeVolume" if no fade
      /// is active.
      F32 mFadedVolume;
      
      /// Volume scale factor imposed on this source by controller.
      F32 mModulativeVolume;

      /// Effective volume after fade and modulation but before distance attenuation.
      /// For non-3D sounds, this is the final effective volume.
      F32 mPreAttenuatedVolume;
      
      /// Effective volume after distance attenuation.  Continuously updated
      /// to match listener position.  For non-3D sounds, this is the same
      /// as mPreAttenuatedVolume.
      ///
      /// @note The distance attenuation that is computed here does not take
      ///   sound cones into account so the computed attenuated volume may be
      ///   higher than the actual effective volume on the device (never
      ///   lower though).
      F32 mAttenuatedVolume;
      
      /// Set volume without affecting CustomVolumeFlag.
      void _setVolume( F32 volume );

      /// Update the effective volume of the source.
      virtual void _updateVolume( const MatrixF& listener );

      /// @}
      
      /// @name Virtualization
      /// @{

      /// The desired sound priority.
      F32 mPriority;
      
      /// The priority scale factor imposed by controllers.
      F32 mModulativePriority;
      
      /// The final priority level.
      F32 mEffectivePriority;
      
      /// Set priority without affecting CustomPriorityFlag.
      void _setPriority( F32 priority );

      /// Update the effective priority of the source.
      virtual void _updatePriority();
      
      /// @}
      
      /// @name Pitch
      /// @{

      /// The desired sound pitch.
      F32 mPitch;
      
      /// The pitch scale factor imposed by controllers.
      F32 mModulativePitch;
      
      /// The final effective pitch.
      F32 mEffectivePitch;
      
      /// Set pitch without affecting CustomPitchFlag.
      void _setPitch( F32 pitch );

      /// Update the effective pitch of the source.
      virtual void _updatePitch();

      ///
      
      /// @}
            
      /// @name 3D Sound
      /// @{

      /// The transform if this is a 3d source.
      MatrixF mTransform;

      /// The last set velocity.
      VectorF mVelocity;

      /// Distance at which to begin distanced-based volume attenuation.
      F32 mMinDistance;

      /// Distance at which to stop distance-based volume attenuation.
      F32 mMaxDistance;

      /// Inside cone angle in degrees.
      F32 mConeInsideAngle;

      /// Outside cone angle in degrees.
      F32 mConeOutsideAngle;

      /// Outside cone volume.
      F32 mConeOutsideVolume;
      
      /// The distance of this source to the last 
      /// listener position.
      F32 mDistToListener;

      /// If true, the transform position has been randomized.
      bool mTransformScattered;

      /// Randomize transform based on scatter settings.
      void _scatterTransform();

      /// Set 3D min/max distance without affecting CustomRadiusFlag.
      virtual void _setMinMaxDistance( F32 min, F32 max );

      /// Set 3D cone without affecting CustomConeFlag.
      virtual void _setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume );

      /// @}
      
      /// @name Fading
      ///
      /// The fade system consists of "persistent" fades placed at the beginning
      /// and end of the playback range and of "temporary" fade segments placed in the
      /// playback range when stopping/pausing/resuming a source in midst of playback.
      ///
      /// @{
      
      /// The current "persistent" fade-in time in seconds.  Taken initially from the
      /// SFXDescription and as long as not being manually set on the source, will
      /// stay with the description's "fadeInTime" property.
      F32 mFadeInTime;
      
      /// The current "persistent" fade-out time in seconds.  Taken initially from the
      /// SFXDescription and as long as not being manually set on the source, will
      /// stay with the description's "fadeOutTime" property.
      F32 mFadeOutTime;
      
      /// Type for temporary fade segments.
      enum FadeSegmentType
      {
         FadeSegmentNone,     ///< No temporary fade segment set.
         FadeSegmentPlay,     ///< Temporary fade-in segment.
         FadeSegmentStop,     ///< Temporary fade-out segment ending in stop().
         FadeSegmentPause     ///< Temporary fade-out segment ending in pause().
      };

      /// Playtime at which persistent fade-in ends.  -1 if no fade-in.
      F32 mFadeInPoint;
      
      /// Playtime at which persistent fade-out starts.  -1 if no fade-out.
      F32 mFadeOutPoint;
      
      /// Type of the current temporary fade segment.  No temporary fade segment
      /// is in place when this is FadeSegmentNone.
      FadeSegmentType mFadeSegmentType;
                  
      /// Easing curve for the current fade segment.
      EaseF* mFadeSegmentEase;

      /// Playback position where the current temporary fade segment starts.
      F32 mFadeSegmentStartPoint;

      /// Playback position where the current temporary fade segment ends.
      F32 mFadeSegmentEndPoint;

      /// Fade time to apply when transitioning to mSavedStatus.
      F32 mSavedFadeTime;
      
      ///
      virtual void _setFadeTimes( F32 fadeInTime, F32 fadeOutTime );
      
      /// Set up a temporary fade-out segment.
      void _setupFadeOutSegment( FadeSegmentType type, F32 fadeOutTime );
      
      /// @}
      
      /// @name Parameters
      /// @{
                  
      ///
      Vector< SFXParameter* > mParameters;

      ///
      void _addParameter( StringTableEntry name );

      ///
      virtual void _onParameterEvent( SFXParameter* parameter, SFXParameterEvent event );
      
      /// @}
      
      /// @name Playback
      /// @{

      /// The simulation tick count that playback was started at for this source.
      U32 mPlayStartTick;

      /// Time object used to keep track of playback.
      TimeSource mPlayTimer;
      
      /// Start playback.  For implementation by concrete subclasses.
      /// @note This method should not take fading into account.
      virtual void _play();
      
      /// Pause playback.  For implementation by concrete subclasses.
      /// @note This method should not take fading into account.
      virtual void _pause();
      
      /// Stop playback.  For implementation by concrete subclasses.
      /// @note This method should not take fading into account.
      virtual void _stop();

      /// @}
      
      /// @name Modifiers
      /// @{
            
      /// List of modifiers that are active on this source.
      ModifierList mModifiers;
      
      /// Delete all modifiers of the given type.
      template< class T > void _clearModifiers();

      /// @}
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onStatusChange, ( SFXStatus newStatus ) );
      DECLARE_CALLBACK( void, onParameterValueChange, ( SFXParameter* parameter ) );
      
      /// @}
                        
      ///
      SFXSource( SFXTrack* track, SFXDescription* description = NULL );
            
      ///
      virtual void _update();

      /// We overload this to disable creation of 
      /// a source via script 'new'.
      virtual bool processArguments( S32 argc, ConsoleValueRef *argv );
      
      // Console getters/setters.
      static bool _setDescription( void *obj, const char *index, const char *data );
      static const char* _getDescription( void* obj, const char* data );

   public:
      
      ///
      SFXSource();
      
      ~SFXSource();

      ///
      void update();
      
      /// Returns the track played by this source; NULL for sources playing directly
      /// from streams.
      SFXTrack* getTrack() const;
      
      /// Return the SFX description associated with this source.  Never NULL.
      SFXDescription* getDescription() const { return mDescription; }
      
      /// Return the source group that this source has been assigned to.
      SFXSource* getSourceGroup() const;
            
      /// @name Playback Status
      /// @{
   
      /// Returns the last known status without doing an update.
      SFXStatus getLastStatus() const { return mStatus; }

      /// Return the status that the source wants to be in.  Used for playback
      /// control in combination with source groups.
      SFXStatus getSavedStatus() const { return mSavedStatus; }

      /// Returns the sound status.
      SFXStatus getStatus() const { const_cast< SFXSource* >( this )->_updateStatus(); return mStatus; }

      /// Returns true if the source is playing.
      bool isPlaying() const { return getStatus() == SFXStatusPlaying; }

      /// Returns true if the source is stopped.
      bool isStopped() const { return getStatus() == SFXStatusStopped; }

      /// Returns true if the source has been paused.
      bool isPaused() const { return getStatus() == SFXStatusPaused; }
      
      /// Returns true if the source is currently being virtualized.
      virtual bool isVirtualized() const { return false; }
      
      /// Returns true if this is a looping source.
      bool isLooping() const { return mDescription.isValid() && mDescription->mIsLooping; }

      /// @}
      
      /// @name Playback Control
      /// @{

      /// Starts the sound from the current playback position.
      ///
      /// @param fadeInTime Seconds for sound to fade in.  If -1, fadeInTime from
      ///   SFXDescription is used.  Note that certain SFXSource classes may not
      ///   support values other than 0 and -1.
      virtual void play( F32 fadeInTime = -1.f );

      /// Stops playback and resets the playback position.
      ///
      /// @note This method is also required to release all playback-related
      ///   resources on the device.
      ///
      /// @param fadeOutTime Seconds for sound to fade out.  If -1, fadeOutTime from
      ///   SFXDescription is used.  Note that certain SFXSource classes may not support
      ///   values other than 0 and -1.
      virtual void stop( F32 fadeOutTime = -1.f );

      /// Pauses the sound playback.
      ///
      /// @param fadeOutTime Seconds for sound to fade out.  If -1, fadeOutTime from
      ///   SFXDescription is used.  Note that certain SFXSource clsases may not support
      ///   values other than 0 and -1.
      virtual void pause( F32 fadeOutTime = -1.f );
      
      /// Return the elapsed play time of the current loop cycle so far in seconds.
      virtual F32 getElapsedPlayTimeCurrentCycle() const;

      /// Return the total elapsed play time so far in seconds.
      virtual F32 getElapsedPlayTime() const;
      
      /// Return the total play time of the source in seconds.  Positive infinity by default.
      ///
      /// @note For looping sounds, this must include only the playtime of a single cycle.
      virtual F32 getTotalPlayTime() const;
      
      /// @}
            
      /// @name 3D Sound
      /// @{

      /// Returns true if this is a 3D source.
      bool is3d() const { return mDescription->mIs3D; }

      /// Returns the last set velocity.
      const VectorF& getVelocity() const { return mVelocity; }

      /// Returns the last set transform.
      const MatrixF& getTransform() const { return mTransform; }

      /// Sets the position and orientation for a 3d buffer.
      virtual void setTransform( const MatrixF& transform );

      /// Sets the velocity for a 3d buffer.
      virtual void setVelocity( const VectorF& velocity );

      /// Sets the minimum and maximum distances for 3d falloff.
      void setMinMaxDistance( F32 min, F32 max ) { _setMinMaxDistance( min, max ); mFlags.set( CustomRadiusFlag ); }

      /// Set sound cone of a 3D sound.
      ///
      /// @param innerAngle Inner cone angle in degrees.
      /// @param outerAngle Outer cone angle in degrees.
      /// @param outerVolume Outer volume factor.
      void setCone( F32 innerAngle, F32 outerAngle, F32 outerVolume ) { _setCone( innerAngle, outerAngle, outerVolume ); mFlags.set( CustomConeFlag ); }
      
      /// Returns the last distance to the listener.
      /// @note Only works when distance attenuation calculations are being triggered by SFX and
      ///   are not left exclusively to the SFX device.
      F32 getDistToListener() const { return mDistToListener; }

      /// @}
      
      /// @name Volume
      /// @{

      /// Returns the source volume at its unaltered initial setting,
      /// i.e. prior to fading, modulation, and attenuation.
      F32 getVolume() const { return mVolume; }

      /// Sets the source volume which will still be
      /// scaled by the master and group volumes.
      ///
      /// @note Note that if you set an explicit volume on a source
      void setVolume( F32 volume ) { _setVolume( volume ); mFlags.set( CustomVolumeFlag ); }

      ///
      F32 getModulativeVolume() const { return mModulativeVolume; }
      
      /// Set the per-source volume scale factor.
      void setModulativeVolume( F32 value );
      
      ///
      F32 getPreAttenuatedVolume() const { return mPreAttenuatedVolume; }

      /// Returns the volume with respect to the master 
      /// and group volumes and the listener.
      F32 getAttenuatedVolume() const { return mAttenuatedVolume; }
      
      ///
      F32 getFadeInTime() const { return mFadeInTime; }
      
      ///
      F32 getFadeOutTime() const { return mFadeOutTime; }
      
      ///
      void setFadeTimes( F32 fadeInTime, F32 fadeOutTime );
            
      /// @}
      
      /// @name Pitch
      /// @{

      /// Returns the source pitch scale.
      F32 getPitch() const { return mPitch; }

      /// Sets the source pitch scale.
      void setPitch( F32 pitch ) { _setPitch( pitch ); mFlags.set( CustomPitchFlag ); }
      
      ///
      F32 getModulativePitch() const { return mModulativePitch; }
      
      ///
      void setModulativePitch( F32 value );
      
      ///
      F32 getEffectivePitch() const { return mEffectivePitch; }
      
      /// @}
      
      /// @name Dynamic Parameters
      ///
      /// Dynamic parameters allow to pass on values from the game system to the sound system
      /// and thus implement interactive audio.
      ///
      /// It is dependent on the back-end source implementation how it will react to parameter
      /// settings.
      ///
      /// @{
      
      ///
      U32 getNumParameters() const { return mParameters.size(); }
      
      ///
      SFXParameter* getParameter( U32 index )
      {
         AssertFatal( index < getNumParameters(), "SFXSource::getParameter() - index out of range" );
         return mParameters[ index ];
      }
      
      ///
      virtual void addParameter( SFXParameter* parameter );
      
      ///
      virtual void removeParameter( SFXParameter* parameter );
      
      /// @}
      
      /// @name Modifiers
      /// @{
      
      ///
      void addModifier( SFXModifier* modifier );
      
      ///
      void addMarker( const String& name, F32 pos );
      
      /// @}

      /// @name Virtualization
      /// @{
      
      /// Returns the source priority.
      F32 getPriority() const { return mPriority; }
            
      ///
      void setPriority( F32 priority ) { _setPriority( priority ); mFlags.set( CustomPriorityFlag ); }
      
      ///
      F32 getModulativePriority() const { return mModulativePriority; }
      
      ///
      void setModulativePriority( F32 value );
      
      ///
      F32 getEffectivePriority() const { return mEffectivePriority; }

      /// @}
            
      /// @}
      
      /// @name Change Notifications
      /// @{
      
      /// Notify the source that its attached SFXDescription has changed.
      virtual void notifyDescriptionChanged();
      
      /// Notify the source that its attached SFXTrack has changed.
      virtual void notifyTrackChanged();
      
      /// @}
            
      // SimGroup.
      virtual bool onAdd();
      virtual void onRemove();
      virtual void onDeleteNotify( SimObject* object );
      virtual bool acceptsAsChild( SimObject* object );
      virtual void onGroupAdd();
      
      static void initPersistFields();
      
      DECLARE_CONOBJECT( SFXSource );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "SFX sound playback controller." );
};

/// A simple macro to automate the deletion of a source.
///
/// @see SFXSource
///
#undef  SFX_DELETE
#define SFX_DELETE( source )  \
   if( source )               \
   {                          \
      source->deleteObject(); \
      source = NULL;          \
   }                          \

#endif // !_SFXSOURCE_H_
