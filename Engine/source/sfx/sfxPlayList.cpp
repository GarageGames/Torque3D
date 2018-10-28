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

#include "sfx/sfxPlayList.h"
#include "sfx/sfxState.h"
#include "sfx/sfxTypes.h"
#include "core/stream/bitStream.h"
#include "math/mRandom.h"
#include "math/mathTypes.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXPlayList );


ConsoleDocClass( SFXPlayList,
   "@brief A datablock describing a playback pattern of sounds.\n\n"
   
   "Playlists allow to define intricate playback patterns of invidual tracks and thus allow the sound system to be "
   "easily used for playing multiple sounds in single operations.\n\n"
   
   "As playlists are %SFXTracks, they can thus be used anywhere in the engine where sound data can be assigned.\n\n"
   
   "Each playlist can hold a maximum of 16 tracks.  Longer playlists may be constructed by cascading lists, i.e. "
   "by creating a playlist that references other playlists.\n\n"
      
   "Processing of a single playlist slot progresses in a fixed set of steps that are invariably "
   "iterated through for each slot (except the slot is assigned a state and its state is deactivated; in "
   "this case, the controller will exit out of the slot directly):\n\n"
   
   "<ol>\n"
      "<li><b>delayIn:</b><p>Waits a set amount of time before processing the slot. This is 0 by default and "
         "is determined by the #delayTimeIn (seconds to wait) and #delayTimeInVariance (bounds on randomization) "
         "properties.</p></li>\n"
      "<li><b>#transitionIn:</b><p>Decides what to do @b before playing the slot.  Defaults to @c None which makes "
         "this stage a no-operation.  Alternatively, the slot can be configured to wait for playback of other "
         "slots to finish (@c Wait and @c WaitAll) or to stop playback of other slots (@c Stop and @c StopAll). "
         "Note that @c Wait and @c Stop always refer to the source that was last started by the list.</p></li>\n"
      "<li><b>play:</b><p><p>Finally, the #track attached to the slot is played. However, this will only @b start "
         "playback of the track and then immediately move on to the next stage.  It will @b not wait for the "
         "track to finish playing.  Note also that depending on the @c replay setting for the slot, this "
         "stage may pick up a source that is already playing on the slot rather than starting a new one.</p> "
         "<p>Several slot properties (fade times, min/max distance, and volume/pitch scale) are used in this stage.</p></li>\n"
      "<li><b>delayOut:</b><p>Waits a set amount of time before transitioning out of the slot. This works the "
         "same as @c delayIn and is set to 0 by default (i.e. no delay).</p></li>\n"
      "<li><b>#transitionOut:</b><p>Decides what to do @b after playing the slot. This works like #transitionIn.</p></li>\n"
   "</ol>\n\n"
   
   "This is a key difference to playlists in normal music players where upon reaching a certain slot, the slot "
   "will immediately play and the player then wait for playback to finish before moving on to the next slot.\n\n"
      
   "@note Be aware that time limits set on slot delays are soft limits.  The sound system updates sound sources in discrete "
      "(and equally system update frequency dependent) intervals which thus determines the granularity at which "
      "time-outs can be handled.\n\n"
      
   "@section SFXPlayList_randomization Value Randomization\n\n"
   
   "For greater variety, many of the values for individual slots may be given a randomization limit that will "
   "trigger a dynamic variance of the specified base value.\n\n"
   
   "Any given field @c xyz that may be randomized has a corresponding field @c xyzVariance which is a two-dimensional "
   "vector.  The first number specifies the greatest value that may be subtracted from the given base value (i.e. the @c xyz field) "
   "whereas the second number specifies the greatest value that may be added to the base value.  Between these two limits, "
   "a random number is generated.\n\n"
   
   "The default variance settings of \"0 0\" will thus not allow to add or subtract anything from the base value and "
   "effectively disable randomization.\n\n"
      
   "Randomization is re-evaluated on each cycle through a list.\n\n"

   "@section SFXPlayList_states Playlists and States\n\n"
   
   "A unique aspect of playlists is that they allow their playback to be tied to the changing set of active sound states. "
   "This feature enables playlists to basically connect to an extensible state machine that can be leveraged by the game "
   "code to signal a multitude of different gameplay states with the audio system then automatically reacting to state "
   "transitions.\n\n"
   
   "Playlists react to states in three ways:\n"
   
   "- Before a controller starts processing a slot it checks whether the slot is assigned a #state.  If this is the "
      "case, the controller checks whether the particular state is active.  If it is not, the entire slot is skipped.  "
      "If it is, the controller goes on to process the slot.\n"
   "- If a controller is in one of the delay stages for a slot that has a #state assigned and the state is deactivated, "
      "the controller will stop the delay and skip any of the remaining processing stages for the slot.\n"
   "- Once the play stage has been processed for a slot that has a #state assigned, the slot's #stateMode will determine "
      "what happens with the playing sound source if the slot's state is deactivated while the sound is still playing.\n"
   "\n"
   
   "A simple example of how to make use of states in combination with playlists would be to set up a playlist for background "
   "music that reacts to the mood of the current gameplay situation.  For example, during combat, tenser music could play than "
   "during normal exploration.  To set this up, different %SFXStates would represent different moods in the game and the "
   "background music playlist would have one slot set up for each such mood.  By making use of volume fades and the "
   "@c PauseWhenDeactivated #stateMode, smooth transitions between the various audio tracks can be produced.\n\n"
   
   "@tsexample\n"
   "// Create a play list from two SFXProfiles.\n"
   "%playList = new SFXPlayList()\n"
   "{\n"
   "   // Use a looped description so the list playback will loop.\n"
   "   description = AudioMusicLoop2D;\n"
   "\n"
   "   track[ 0 ] = Profile1;\n"
   "   track[ 1 ] = Profile2;\n"
   "};\n"
   "\n"
   "// Play the list.\n"
   "sfxPlayOnce( %playList );\n"
   "@endtsexample\n\n"

   "@ref SFX_interactive\n\n"
   
   "@ingroup SFX"
);


ImplementEnumType( SFXPlayListLoopMode,
   "Playlist behavior when description is set to loop.\n\n"
   "@see SFXDescription::isLooping\n\n"
   "@see SFXPlayList::loopMode\n\n"
   "@ingroup SFX" )
   { SFXPlayList::LOOP_All,                  "All",
      "Loop over all slots, i.e. jump from last to first slot after all slots have played." },
   { SFXPlayList::LOOP_Single,               "Single",
      "Loop infinitely over the current slot.  Only useful in combination with either states or manual playlist control." },
EndImplementEnumType;

ImplementEnumType( SFXPlayListRandomMode,
   "Randomization pattern to apply to playlist slot playback order.\n\n"
   "@see SFXPlayList::random\n\n"
   "@ingroup SFX" )
   { SFXPlayList::RANDOM_NotRandom,          "NotRandom",
      "Play slots in sequential order.  No randomization." },
   { SFXPlayList::RANDOM_StrictRandom,       "StrictRandom",
      "Play a strictly random selection of slots.\n\n"
      "In this mode, a set of numSlotsToPlay random numbers between 0 and numSlotsToPlay-1 (inclusive), i.e. in the range of valid slot indices, is "
      "generated and playlist slots are played back in the order of this list.  This allows the same slot to occur multiple times in a list and, "
      "consequentially, allows for other slots to not appear at all in a given slot ordering." },
   { SFXPlayList::RANDOM_OrderedRandom,      "OrderedRandom",
      "Play all slots in the list in a random order.\n\n"
      "In this mode, the @c numSlotsToPlay slots from the list with valid tracks assigned are put into a random order and played.  This guarantees "
      "that each slots is played exactly once albeit at a random position in the total ordering." },
EndImplementEnumType;

ImplementEnumType( SFXPlayListTransitionMode,
   "Playlist behavior when transitioning in and out of invididual slots.\n\n"
   "Transition behaviors apply when the playback controller starts processing a playlist slot and when it ends processing a slot.  Using transition "
   "behaviors, playback can be synchronized.\n\n"
   "@see SFXPlayList::transitionIn\n\n"
   "@see SFXPlayList::transitionOut\n\n"
   "@ingroup SFX" )
   { SFXPlayList::TRANSITION_None,           "None",
      "No transition.  Immediately move on to processing the slot or immediately move on to the next slot." },
   { SFXPlayList::TRANSITION_Wait,           "Wait",
      "Wait for the sound source spawned last by this playlist to finish playing.  Then proceed." },
   { SFXPlayList::TRANSITION_WaitAll,        "WaitAll",
      "Wait for all sound sources currently spawned by the playlist to finish playing.  Then proceed." },
   { SFXPlayList::TRANSITION_Stop,           "Stop",
      "Stop the sound source spawned last by this playlist.  Then proceed." },
   { SFXPlayList::TRANSITION_StopAll,        "StopAll",
      "Stop all sound sources spawned by the playlist.  Then proceed." },
EndImplementEnumType;

ImplementEnumType( SFXPlayListReplayMode,
   "Behavior when hitting the play stage of a slot that is still playing from a previous cycle.\n\n"
   "@see SFXPlayList::replay\n\n"
   "@ingroup SFX" )
   { SFXPlayList::REPLAY_IgnorePlaying,      "IgnorePlaying",
      "Ignore any sources that may already be playing on the slot and just create a new source." },
   { SFXPlayList::REPLAY_RestartPlaying,     "RestartPlaying",
      "Restart all sources that was last created for the slot." },
   { SFXPlayList::REPLAY_KeepPlaying,        "KeepPlaying",
      "Keep playing the current source(s) as if the source started last on the slot was created in this cycle.  For this, "
      "the sources associated with the slot are brought to the top of the play stack." },
   { SFXPlayList::REPLAY_StartNew,           "StartNew",
      "Stop all sources currently playing on the slot and then create a new source." },
   { SFXPlayList::REPLAY_SkipIfPlaying,      "SkipIfPlaying",
      "If there are sources already playing on the slot, skip the play stage." },
EndImplementEnumType;

ImplementEnumType( SFXPlayListStateMode,
   "Reaction behavior when a state is changed incompatibly on a slot that has already started playing.\n\n"
   "@see SFXPlayList::stateMode\n\n"
   "@ingroup SFX" )
   { SFXPlayList::STATE_StopInactive,        "StopWhenDeactivated",
      "Stop the sources playing on the slot when a state changes to a setting that is incompatible with "
      "the slot's state setting." },
   { SFXPlayList::STATE_PauseInactive,       "PauseWhenDeactivated",
      "Pause all sources playing on the slot when a state changes to a setting that is incompatible with the "
      "slot's state setting.\n\n"
      "When the slot's state is reactivated again, the sources will resume playback." },
   { SFXPlayList::STATE_IgnoreInactive,      "IgnoreWhenDeactivated",
      "Ignore when a state changes to a setting incompatible with the slot's state setting and just keep "
      "playing sources attached to the slot." },
EndImplementEnumType;


//-----------------------------------------------------------------------------

SFXPlayList::SFXPlayList()
   : mRandomMode( RANDOM_NotRandom ),
     mLoopMode( LOOP_All ),
     mNumSlotsToPlay( NUM_SLOTS ),
     mTrace( false )
{
}

//-----------------------------------------------------------------------------

void SFXPlayList::initPersistFields()
{
   addGroup( "Sound" );
   
      addField( "random",           TYPEID< ERandomMode >(), Offset( mRandomMode, SFXPlayList ),
         "Slot playback order randomization pattern.\n"
         "By setting this field to something other than \"NotRandom\" to order in which slots of the "
         "playlist are processed can be changed from sequential to a random pattern.  This allows to "
         "to create more varied playback patterns.\n"
         "Defaults to \"NotRandom\"." );
      addField( "loopMode",         TYPEID< ELoopMode >(), Offset( mLoopMode, SFXPlayList ),
         "Behavior when description has looping enabled.\n"
         "The loop mode determines whether the list will loop over a single slot or loop over "
         "all the entire list of slots being played.\n\n"
         "@see SFXDescription::isLooping" );
      addField( "numSlotsToPlay",   TypeS32,          Offset( mNumSlotsToPlay, SFXPlayList ),
         "Number of slots to play.\n"
         "Up to a maximum of 16, this field determines the number of slots that are taken from the "
         "list for playback.  Only slots that have a valid #track assigned will be considered for "
         "this." );
   
      addArray( "slots", NUM_SLOTS );
      
         addField( "track",                  TypeSFXTrackName, Offset( mSlots.mTrack, SFXPlayList ), NUM_SLOTS,
            "Track to play in this slot.\n"
            "This must be set for the slot to be considered for playback.  Other settings for a slot "
            "will not take effect except this field is set." );
         addField( "replay",                 TYPEID< EReplayMode >(), Offset( mSlots.mReplayMode, SFXPlayList ), NUM_SLOTS,
            "Behavior when an already playing sound is encountered on this slot from a previous cycle.\n"
            "Each slot can have an arbitrary number of sounds playing on it from previous cycles.  This field determines "
            "how SFXController will handle these sources." );
         addField( "transitionIn",           TYPEID< ETransitionMode >(), Offset( mSlots.mTransitionIn, SFXPlayList ), NUM_SLOTS,
            "Behavior when moving into this slot.\n"
            "After the delayIn time has expired (if any), this slot determines what the controller "
            "will do before actually playing the slot." );
         addField( "transitionOut",          TYPEID< ETransitionMode >(), Offset( mSlots.mTransitionOut, SFXPlayList ), NUM_SLOTS,
            "Behavior when moving out of this slot.\n"
            "After the #detailTimeOut has expired (if any), this slot determines what the controller "
            "will do before moving on to the next slot." );
         addField( "delayTimeIn",            TypeF32,          Offset( mSlots.mDelayTimeIn.mValue, SFXPlayList ), NUM_SLOTS,
            "Seconds to wait after moving into slot before #transitionIn." );
         addField( "delayTimeInVariance",    TypePoint2F,      Offset( mSlots.mDelayTimeIn.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #delayTimeIn.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "delayTimeOut",           TypeF32,          Offset( mSlots.mDelayTimeOut.mValue, SFXPlayList ), NUM_SLOTS,
            "Seconds to wait before moving out of slot after #transitionOut." );
         addField( "delayTimeOutVariance",   TypePoint2F,      Offset( mSlots.mDelayTimeOut.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #delayTimeOut.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "fadeTimeIn",             TypeF32,          Offset( mSlots.mFadeTimeIn.mValue, SFXPlayList ), NUM_SLOTS,
            "Seconds to fade sound in (-1 to use the track's own fadeInTime.)\n"
            "@see SFXDescription::fadeTimeIn" );
         addField( "fadeTimeInVariance",     TypePoint2F,      Offset( mSlots.mFadeTimeIn.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #fadeInTime.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "fadeTimeOut",            TypeF32,          Offset( mSlots.mFadeTimeOut.mValue, SFXPlayList ), NUM_SLOTS,
            "Seconds to fade sound out (-1 to use the track's own fadeOutTime.)\n"
            "@see SFXDescription::fadeTimeOut" );
         addField( "fadeTimeOutVariance",    TypePoint2F,      Offset( mSlots.mFadeTimeOut.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #fadeOutTime\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "referenceDistance",      TypeF32,          Offset( mSlots.mMinDistance.mValue, SFXPlayList ), NUM_SLOTS,
            "@c referenceDistance to set for 3D sounds in this slot (<1 to use @c referenceDistance of track's own description).\n"
            "@see SFXDescription::referenceDistance" );
         addField( "referenceDistanceVariance", TypePoint2F,   Offset( mSlots.mMinDistance.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #referenceDistance.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "maxDistance",            TypeF32,          Offset( mSlots.mMaxDistance.mValue, SFXPlayList ), NUM_SLOTS,
            "@c maxDistance to apply to 3D sounds in this slot (<1 to use @c maxDistance of track's own description).\n"
            "@see SFXDescription::maxDistance" );
         addField( "maxDistanceVariance",    TypePoint2F,      Offset( mSlots.mMaxDistance.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #maxDistance.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "volumeScale",            TypeF32,          Offset( mSlots.mVolumeScale.mValue, SFXPlayList ), NUM_SLOTS,
            "Scale factor to apply to volume of sounds played on this list slot.\n"
            "This value will scale the actual volume level set on the track assigned to the slot, i.e. a value of 0.5 will "
            "cause the track to play at half-volume." );
         addField( "volumeScaleVariance",    TypePoint2F,      Offset( mSlots.mVolumeScale.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #volumeScale.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "pitchScale",             TypeF32,          Offset( mSlots.mPitchScale.mValue, SFXPlayList ), NUM_SLOTS,
            "Scale factor to apply to pitch of sounds played on this list slot.\n"
            "This value will scale the actual pitch set on the track assigned to the slot, i.e. a value of 0.5 will "
            "cause the track to play at half its assigned speed." );
         addField( "pitchScaleVariance",     TypePoint2F,      Offset( mSlots.mPitchScale.mVariance, SFXPlayList ), NUM_SLOTS,
            "Bounds on randomization of #pitchScale.\n\n"
            "@ref SFXPlayList_randomization\n" );
         addField( "repeatCount",            TypeS32,          Offset( mSlots.mRepeatCount, SFXPlayList ), NUM_SLOTS,
            "Number of times to loop this slot." );
         addField( "state",                  TypeSFXStateName, Offset( mSlots.mState, SFXPlayList ), NUM_SLOTS,
            "State that must be active for this slot to play.\n\n"
            "@ref SFXPlayList_states" );
         addField( "stateMode",              TYPEID< EStateMode >(), Offset( mSlots.mStateMode, SFXPlayList ), NUM_SLOTS,
            "Behavior when assigned state is deactivated while slot is playing.\n\n"
            "@ref SFXPlayList_states" );
   
      endArray( "slots" );
   
   endGroup( "Sound" );
   
   addGroup( "Debug" );
   
      addField( "trace", TypeBool, Offset( mTrace, SFXPlayList ),
         "Enable/disable execution tracing for this playlist (local only).\n"
         "If this is true, SFXControllers attached to the list will automatically run in trace mode." );
         
   endGroup( "Debug" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXPlayList::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   validate();
      
   // Resolve SFXTracks and SFXStates on client.
      
   if( !server )
   {
      for( U32 i = 0; i < NUM_SLOTS; ++ i )
      {
         if( !sfxResolve( &mSlots.mTrack[ i ], errorStr ) )
            return false;
            
         if( !sfxResolve( &mSlots.mState[ i ], errorStr ) )
            return false;
      }
   }
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXPlayList::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   stream->writeInt( mRandomMode, NUM_RANDOM_MODE_BITS );
   stream->writeInt( mLoopMode, NUM_LOOP_MODE_BITS );
   stream->writeInt( mNumSlotsToPlay, NUM_SLOTS_TO_PLAY_BITS );
   
   #define FOR_EACH_SLOT \
      for( U32 i = 0; i < NUM_SLOTS; ++ i )
   
   FOR_EACH_SLOT stream->writeInt( mSlots.mReplayMode[ i ], NUM_REPLAY_MODE_BITS );
   FOR_EACH_SLOT stream->writeInt( mSlots.mTransitionIn[ i ], NUM_TRANSITION_MODE_BITS );
   FOR_EACH_SLOT stream->writeInt( mSlots.mTransitionOut[ i ], NUM_TRANSITION_MODE_BITS );
   FOR_EACH_SLOT stream->writeInt( mSlots.mStateMode[ i ], NUM_STATE_MODE_BITS );
      
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeIn.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeIn.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeIn.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeOut.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeOut.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mFadeTimeOut.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeIn.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeIn.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeIn.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeOut.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeOut.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mDelayTimeOut.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mVolumeScale.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mVolumeScale.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mVolumeScale.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mPitchScale.mValue[ i ] );
   FOR_EACH_SLOT stream->write( mSlots.mPitchScale.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->write( mSlots.mPitchScale.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->write( mSlots.mRepeatCount[ i ] );
      
   FOR_EACH_SLOT sfxWrite( stream, mSlots.mState[ i ] );
   FOR_EACH_SLOT sfxWrite( stream, mSlots.mTrack[ i ] );
}

//-----------------------------------------------------------------------------

void SFXPlayList::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   mRandomMode          = ( ERandomMode ) stream->readInt( NUM_RANDOM_MODE_BITS );
   mLoopMode            = ( ELoopMode ) stream->readInt( NUM_LOOP_MODE_BITS );
   mNumSlotsToPlay      = stream->readInt( NUM_SLOTS_TO_PLAY_BITS );
   
   FOR_EACH_SLOT mSlots.mReplayMode[ i ]     = ( EReplayMode ) stream->readInt( NUM_REPLAY_MODE_BITS );
   FOR_EACH_SLOT mSlots.mTransitionIn[ i ]   = ( ETransitionMode ) stream->readInt( NUM_TRANSITION_MODE_BITS );
   FOR_EACH_SLOT mSlots.mTransitionOut[ i ]  = ( ETransitionMode ) stream->readInt( NUM_TRANSITION_MODE_BITS );
   FOR_EACH_SLOT mSlots.mStateMode[ i ]      = ( EStateMode ) stream->readInt( NUM_STATE_MODE_BITS );
      
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeIn.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeIn.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeIn.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeOut.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeOut.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mFadeTimeOut.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeIn.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeIn.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeIn.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeOut.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeOut.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mDelayTimeOut.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mVolumeScale.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mVolumeScale.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mVolumeScale.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mPitchScale.mValue[ i ] );
   FOR_EACH_SLOT stream->read( &mSlots.mPitchScale.mVariance[ i ][ 0 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mPitchScale.mVariance[ i ][ 1 ] );
   FOR_EACH_SLOT stream->read( &mSlots.mRepeatCount[ i ] );
      
   FOR_EACH_SLOT sfxRead( stream, &mSlots.mState[ i ] );
   FOR_EACH_SLOT sfxRead( stream, &mSlots.mTrack[ i ] );
   
   #undef FOR_EACH_SLOT
}

//-----------------------------------------------------------------------------

void SFXPlayList::inspectPostApply()
{
   Parent::inspectPostApply();
   validate();
}

//-----------------------------------------------------------------------------

void SFXPlayList::validate()
{
   if( mNumSlotsToPlay > NUM_SLOTS )
      mNumSlotsToPlay = NUM_SLOTS;
      
   mSlots.mFadeTimeIn.validate();
   mSlots.mFadeTimeOut.validate();
   mSlots.mDelayTimeIn.validate();
   mSlots.mDelayTimeOut.validate();
   mSlots.mVolumeScale.validate();
   mSlots.mPitchScale.validate();
}
