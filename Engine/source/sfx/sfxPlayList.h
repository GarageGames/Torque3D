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

#ifndef _SFXPLAYLIST_H_
#define _SFXPLAYLIST_H_

#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _SFXTRACK_H_
   #include "sfx/sfxTrack.h"
#endif


class SFXState;


/// A playback list of SFXTracks.
///
/// Note that since SFXPlayLists are SFXTracks, play lists can be cascaded.
///
/// Play lists are comprised of a sequence of slots.  Each slot can be assigned
/// a track (SFXProfile or another SFXPlayList) as well as a number of options
/// that determine how the particular slot should behave.
///
/// In addition to playing a track, each slot can do an arbitrary combination
/// of the following operations:
///
/// - Wait: wait for the previous or all sources to stop playing
/// - Stop: stop the previous or all sources from playing
/// - Delay: wait some (optionally randomized) amount of time
/// - Shift Pitch: scale pitch when playing by optionally randomized amount
/// - Shift Volume: scale volume when playing by optionally randomized amount
/// - Fade: perform volume fade-in/out
/// - Distance: only start playing track when listener is within a certain range
/// - Loop: loop a set number of times
/// - State: play only when the given SFXState is active; transitions out of
///      slot when state is deactivated
///
/// The order in which slots are played is either sequential (NotRandom),
/// or a random selection (StrictRandom), or a random ordering (OrderedRandom).
///
/// Additionally, the list may be looped over in entirety (All) or looped
/// on single slots (useful for either manual playback control or lists that
/// exclusively use states).
///
/// Be aware that playlists are affected by SFXDescriptions the same way that an
/// SFXProfile is, i.e. fades, looping, 3D sound, etc. all take effect.
///
/// @note Playlists offer a lot of control but unfortunately they also make it
///   pretty easy at the moment to shoot yourself in the foot.
///
class SFXPlayList : public SFXTrack
{
   public:
   
      typedef SFXTrack Parent;
      
      enum
      {
         /// Number of slots in a playlist.
         ///
         /// @note To have longer playlists, simply cascade playlists and use
         ///   wait behaviors.
         NUM_SLOTS = 16,
         
         NUM_TRANSITION_MODE_BITS = 3,
         NUM_LOOP_MODE_BITS = 1,
         NUM_RANDOM_MODE_BITS = 2,
         NUM_SLOTS_TO_PLAY_BITS = 5,
         NUM_REPLAY_MODE_BITS = 3,
         NUM_STATE_MODE_BITS = 2,
      };
      
      /// Behavior when description is set to loop.
      enum ELoopMode
      {
         /// Start over after completing a cycle.
         LOOP_All,

         /// Loop a single slot over and over.
         ///
         /// @note This behavior is only useful in combination with states or manual
         ///   playback control.  To just loop over a slot for some time, set its loop
         ///   count instead.
         LOOP_Single,
      };
      
      /// Random playback mode.
      enum ERandomMode
      {
         /// No randomization of playback order.
         RANDOM_NotRandom,
         
         /// Playback order that jumps to a random slot after completing
         /// a given slot.  The slot being jumped to, however, may be any
         /// slot in the list including the slot that has just played.
         ///
         /// @note In order to ensure cycles are always finite, this mode will
         ///   also just do NUM_SLOTS number of transitions and then stop the
         ///   current cycle whether all slots have played or not.  Otherwise,
         ///   it would be dependent on the random number sequence generated when
         ///   and whether at all a given cycle finishes.
         RANDOM_StrictRandom,
         
         /// Before a cycle over the playlist starts, a random total ordering of
         /// the slots is established and then played.  No slot will be played
         /// twice in a single cycle.
         RANDOM_OrderedRandom,
      };
        
      /// Transitioning behavior when moving in and out of slots.
      enum ETransitionMode
      {
         /// No specific behavior for transitioning between slots.
         TRANSITION_None,

         /// Wait for single slot to stop playing.  If transitioning into slot,
         /// this is the slot being transitioned from.  If transitioning out of slot,
         /// this is the current slot. 
         TRANSITION_Wait,
         
         /// Wait for all slots to stop playing.
         TRANSITION_WaitAll,

         /// Stop single slot before proceeding.  If transitioning into slot, this
         /// is the slot being transitioned from.  If transitioning out of slot,
         /// this is the current slot.
         TRANSITION_Stop,
         
         /// Stop all playing slots before proceeding.
         TRANSITION_StopAll,
      };
      
      /// Behavior when hitting play() on a slot that is still playing from
      /// a previous cycle.
      enum EReplayMode
      {
         /// Do not check if a source is already playing on the slot.
         REPLAY_IgnorePlaying,
         
         /// Stop the currently playing source and start playing it from the
         /// beginning.
         REPLAY_RestartPlaying,
         
         /// Move the currently playing source to the top of the stack and pretend
         /// it was started by this cycle.
         ///
         /// When using STATE_PauseInactive, it is usally best to also use REPLAY_KeepPlaying
         /// as otherwise a new source will be spawned when the state becomes active again.
         ///
         /// @note When the currently playing source is paused, KeepPlaying will
         ///   resume playback.
         REPLAY_KeepPlaying,
         
         /// Let the old source play and start a new one on the same slot.
         REPLAY_StartNew,

         /// If there is a source currently playing on this slot, skip the play() stage.
         REPLAY_SkipIfPlaying,
      };
      
      /// State-reaction behavior of slot once a source has started playing.
      enum EStateMode
      {
         /// Stop and remove source when state becomes inactive.
         STATE_StopInactive,
         
         /// Pause source when state becomes inactive and resume playback
         /// when state becomes active again.
         STATE_PauseInactive,
         
         /// Once a source has started to play, it will not be stopped due to
         /// state changes.  A source will, however, still be prevented from starting
         /// to play when its assigned state is not active.
         STATE_IgnoreInactive,
      };
            
      // All structures here are laid out as structures of arrays instead of arrays of structures
      // to allow them to be used as fixed-size TorqueScript arrays.

      struct VariantFloat : SFXVariantFloat< NUM_SLOTS >
      {
         VariantFloat()
         {
            dMemset( mValue, 0, sizeof( mValue ) );
            dMemset( mVariance, 0, sizeof( mVariance ) );
         }
      };
      
      /// Settings for the playback slots.
      struct SlotData
      {
         /// Behavior when a sound is already playing on a slot from a previous cycle.
         EReplayMode mReplayMode[ NUM_SLOTS ];
         
         /// Behavior when transitioning into the slot.
         ETransitionMode mTransitionIn[ NUM_SLOTS ];
         
         /// Behavior when transitioning out of the slot.
         ETransitionMode mTransitionOut[ NUM_SLOTS ];
         
         /// Seconds to fade sound in.  -1 to leave at default.
         VariantFloat mFadeTimeIn;
         
         /// Seconds to fade sound out.  -1 to leave at default.
         VariantFloat mFadeTimeOut;
         
         /// Time to delay before mTransitionIn.
         VariantFloat mDelayTimeIn;
         
         /// Time to delay before mTransitionOut.
         VariantFloat mDelayTimeOut;
         
         /// Volume scale factor.
         VariantFloat mVolumeScale;
         
         /// Pitch scale factor.
         VariantFloat mPitchScale;
         
         /// Min distance for 3D sounds.
         VariantFloat mMinDistance;
         
         /// Max distance for 3D sounds.
         VariantFloat mMaxDistance;
                  
         /// Number of times to loop over this slot.
         /// @note Each iteration will do a full transition as if proceeding
         ///   to a different slot.
         U32 mRepeatCount[ NUM_SLOTS ];
         
         /// State restriction for this slot.  Slot will only play when the given
         /// state is active and will be automatically transitioned from
         /// if the state becomes inactive.
         SFXState* mState[ NUM_SLOTS ];
         
         /// Bahavior when state of this slot is deactivated and the slot's track
         /// is playing.
         EStateMode mStateMode[ NUM_SLOTS ];
         
         /// Track to play in this slot.
         SFXTrack* mTrack[ NUM_SLOTS ];
         
         SlotData()
         {
            dMemset( mReplayMode, 0, sizeof( mReplayMode ) );
            dMemset( mTransitionIn, 0, sizeof( mTransitionIn ) );
            dMemset( mTransitionOut, 0, sizeof( mTransitionOut ) );
            dMemset( mRepeatCount, 0, sizeof( mRepeatCount ) );
            dMemset( mState, 0, sizeof( mState ) );
            dMemset( mTrack, 0, sizeof( mTrack ) );
            dMemset( mStateMode, 0, sizeof( mStateMode ) );
            
            for( U32 i = 0; i < NUM_SLOTS; ++ i )
            {
               mTransitionOut[ i ]        = TRANSITION_Wait;
               mVolumeScale.mValue[ i ]   = 1.f;
               mPitchScale.mValue[ i ]    = 1.f;
               mFadeTimeIn.mValue[ i ]    = -1.f;  // Don't touch by default.
               mFadeTimeOut.mValue[ i ]   = -1.f;  // Don't touch by default.
               mMinDistance.mValue[ i ]   = -1.f;  // Don't touch by default.
               mMaxDistance.mValue[ i ]   = -1.f;  // Don't touch by default.
            }
         }
      };
                        
   protected:
   
      /// Trace interpreter execution.  This field is not networked.
      bool mTrace;
   
      /// Select slots at random.
      ERandomMode mRandomMode;
         
      /// Loop over slots in this list.
      ELoopMode mLoopMode;
      
      /// Number of slots to play from list.  This can be used, for example,
      /// to create a list of tracks where only a single track is selected and
      /// played for each cycle.
      U32 mNumSlotsToPlay;
      
      /// Data for each of the playlist slots.
      SlotData mSlots;
               
   public:
   
      SFXPlayList();
      
      /// Make all settings conform to constraints.
      void validate();
      
      /// Return true if execution tracing is enabled on this list.
      bool trace() const { return mTrace; }
      
      /// Return the number of slots to play from this list in a single cycle.
      U32 getNumSlotsToPlay() const { return mNumSlotsToPlay; }
      
      /// Return the slot order randomization behavior.
      ERandomMode getRandomMode() const { return mRandomMode; }
      
      /// Return the loop mode (only relevant if this is a looped playlist).
      ELoopMode getLoopMode() const { return mLoopMode; }
      
      /// Return the total number of slots in the list.
      U32 getNumSlots() const { return NUM_SLOTS; }
      
      /// Return the slot data for this list.
      const SlotData& getSlots() const { return mSlots; }
                  
      DECLARE_CONOBJECT( SFXPlayList );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "A playback list of SFXProfiles or nested SFXPlayLists." );
      
      // SimDataBlock.
      virtual bool preload( bool server, String& errorStr );
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      virtual void inspectPostApply();
      
      static void initPersistFields();
};


typedef SFXPlayList::ELoopMode SFXPlayListLoopMode;
typedef SFXPlayList::ETransitionMode SFXPlayListTransitionMode;
typedef SFXPlayList::EStateMode SFXPlayListStateMode;
typedef SFXPlayList::ERandomMode SFXPlayListRandomMode;
typedef SFXPlayList::EReplayMode SFXPlayListReplayMode;

DefineEnumType( SFXPlayListLoopMode );
DefineEnumType( SFXPlayListTransitionMode );
DefineEnumType( SFXPlayListStateMode );
DefineEnumType( SFXPlayListRandomMode );
DefineEnumType( SFXPlayListReplayMode );

#endif // _SFXPLAYLIST_H_
