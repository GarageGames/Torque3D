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

#include "sfx/sfxController.h"
#include "sfx/sfxPlayList.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxState.h"
#include "sfx/sfxDescription.h"
#include "console/engineAPI.h"
#include "math/mRandom.h"



IMPLEMENT_CONOBJECT( SFXController );


ConsoleDocClass( SFXController,
   "@brief A sound source that drives multi-source playback.\n\n"
   
   "This class acts as an interpreter for SFXPlayLists.  It goes through the slots of the playlist it is "
   "attached to and performs the actions described by each of the slots in turn.\n"
   
   "As SFXControllers are created implicitly by the SFX system when instantiating a source for a play list it is "
   "in most cases not necessary to directly deal with the class.\n"
   
   "The following example demonstrates how a controller would commonly be created.\n"
   
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
   "// Play the list.  This will implicitly create a controller.\n"
   "sfxPlayOnce( %playList );\n"
   "@endtsexample\n\n"
      
   "@note Play lists are updated at regular intervals by the sound system.  This processing determines the granularity at "
      "which playlist action timing takes place.\n"
   "@note This class cannot be instantiated directly.  Use sfxPlayOnce() or sfxCreateSource() with the playlist "
      "you want to play to create an instance of this class.\n"
      
   "@see SFXPlayList\n"
   "@ingroup SFX\n"
);


//-----------------------------------------------------------------------------

SFXController::SFXController( SFXPlayList* playList )
   : Parent( playList ),
     mTrace( playList->trace() )
{
   VECTOR_SET_ASSOCIATION( mInsns );
   VECTOR_SET_ASSOCIATION( mSources );
   VECTOR_SET_ASSOCIATION( mParameters );
   
   _compileList( playList );
}

//-----------------------------------------------------------------------------

SFXController::~SFXController()
{
}

//-----------------------------------------------------------------------------

void SFXController::initPersistFields()
{
   addGroup( "Debug" );
      addField( "trace", TypeBool, Offset( mTrace, SFXController ),
         "If true, the controller logs its operation to the console.\n"
         "This is a non-networked field that will work locally only." );
   endGroup( "Debug" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

SFXController* SFXController::_create( SFXPlayList* playList )
{
   AssertFatal( playList != NULL, "SFXController::_create() - got a NULL playlist!" );
   
   SFXController* controller = new SFXController( playList );
   controller->registerObject();
   
   return controller;
}

//-----------------------------------------------------------------------------

void SFXController::_compileList( SFXPlayList* playList )
{
   mInsns.clear();
   const bool isLooping = playList->getDescription()->mIsLooping;
   
   // Create a slot list that determines the order the slots will be
   // played in.
   
   U32 slotList[ SFXPlayList::NUM_SLOTS ];
   bool isOrderedRandom = false;
   switch( playList->getRandomMode() )
   {
      case SFXPlayList::RANDOM_OrderedRandom:
         isOrderedRandom = true;
         /* fallthrough */
         
      case SFXPlayList::RANDOM_NotRandom:
         // Generate sequence 1-NUM_SLOTS.
         for( U32 i = 0; i < SFXPlayList::NUM_SLOTS; ++ i )
            slotList[ i ] = i;
            
         if( isOrderedRandom )
         {
            // Randomly exchange slots in the list.
            for( U32 i = 0; i < SFXPlayList::NUM_SLOTS; ++ i )
               swap( slotList[ gRandGen.randI( 0, SFXPlayList::NUM_SLOTS - 1 ) ], slotList[ i ] );
         }
         break;
         
      case SFXPlayList::RANDOM_StrictRandom:
         // Randomly generate NUM_SLOTS slot indices.
         for( U32 i = 0; i < SFXPlayList::NUM_SLOTS; ++ i )
            slotList[ i ] = gRandGen.randI( 0, SFXPlayList::NUM_SLOTS - 1 );
         break;
   }
   
   // Generate the instruction list.
   
   U32 slotCount = 0;
   for( U32 i = 0; i < SFXPlayList::NUM_SLOTS; ++ i )
   {
      const U32 slotIndex = slotList[ i ];
      const U32 slotStartIp = mInsns.size();
      
      SFXState* state = playList->getSlots().mState[ slotIndex ];
      
      // If there's no track in this slot, ignore it.
      
      if( !playList->getSlots().mTrack[ slotIndex ] )
         continue;
         
      // If this is a looped slot and the list is not set to loop
      // indefinitly on single slots, start a loop.
      
      S32 loopStartIp = -1;
      if( playList->getSlots().mRepeatCount[ slotIndex ] > 0
          && ( !isLooping || playList->getLoopMode() != SFXPlayList::LOOP_Single ) )
      {
         Insn insn( OP_LoopBegin, slotIndex, state );
         insn.mArg.mLoopCount = playList->getSlots().mRepeatCount[ slotIndex ];
         mInsns.push_back( insn );
         
         loopStartIp = mInsns.size();
      }
         
      // Add in-delay, if any.
      
      if( playList->getSlots().mDelayTimeIn.mValue[ slotIndex ] > 0.0f )
      {
         Insn insn( OP_Delay, slotIndex, state );
         insn.mArg.mDelayTime.mValue[ 0 ] = playList->getSlots().mDelayTimeIn.mValue[ slotIndex ];
         insn.mArg.mDelayTime.mVariance[ 0 ][ 0 ] = playList->getSlots().mDelayTimeIn.mVariance[ slotIndex ][ 0 ];
         insn.mArg.mDelayTime.mVariance[ 0 ][ 1 ] = playList->getSlots().mDelayTimeIn.mVariance[ slotIndex ][ 1 ];
         
         mInsns.push_back( insn );
      }
      
      // Add the in-transition.
      
      const SFXPlayList::ETransitionMode transitionIn = playList->getSlots().mTransitionIn[ slotIndex ];
      if( transitionIn != SFXPlayList::TRANSITION_None )
      {
         Insn insn( slotIndex, state );
         _genTransition( insn, transitionIn );
         mInsns.push_back( insn );
      }
      
      // Add the play instruction.
      
      {
         Insn insn( OP_Play, slotIndex, state );
         mInsns.push_back( insn );
      }
      
      // Add out-delay, if any.
      
      if( playList->getSlots().mDelayTimeOut.mValue[ slotIndex ] > 0.0f )
      {
         Insn insn( OP_Delay, slotIndex, state );
         insn.mArg.mDelayTime.mValue[ 0 ] = playList->getSlots().mDelayTimeOut.mValue[ slotIndex ];
         insn.mArg.mDelayTime.mVariance[ 0 ][ 0 ] = playList->getSlots().mDelayTimeOut.mVariance[ slotIndex ][ 0 ];
         insn.mArg.mDelayTime.mVariance[ 0 ][ 1 ] = playList->getSlots().mDelayTimeOut.mVariance[ slotIndex ][ 1 ];
         
         mInsns.push_back( insn );
      }
      
      // Add the out-transition.
      
      const SFXPlayList::ETransitionMode transitionOut = playList->getSlots().mTransitionOut[ slotIndex ];
      if( transitionOut != SFXPlayList::TRANSITION_None )
      {
         Insn insn( slotIndex, state );
         _genTransition( insn, transitionOut );
         mInsns.push_back( insn );
      }
      
      // Loop, if necessary.
      
      if( loopStartIp != -1 )
      {
         Insn insn( OP_LoopEnd, slotIndex, state );
         insn.mArg.mJumpIp = loopStartIp;
         mInsns.push_back( insn );
      }
      
      // If the list is on repeat-single, unconditionally
      // loop over the instruction sequence of each slot.
      
      if( isLooping && playList->getLoopMode() == SFXPlayList::LOOP_Single )
      {
         Insn insn( OP_Jump, slotIndex, state );
         insn.mArg.mJumpIp = slotStartIp;
         mInsns.push_back( insn );
      }
      
      // If we have reached the limit of slots to play,
      // stop generating.
      
      slotCount ++;
      if( playList->getNumSlotsToPlay() == slotCount )
         break;
   }
      
   // Set up for execution.

   mIp = 0;
   if( !mInsns.empty() )
      _initInsn();
}

//-----------------------------------------------------------------------------

void SFXController::_genTransition( Insn& insn, SFXPlayList::ETransitionMode transition )
{
   switch( transition )
   {
      case SFXPlayList::TRANSITION_Wait:
         insn.mOpcode = OP_WaitSingle;
         break;
         
      case SFXPlayList::TRANSITION_WaitAll:
         insn.mOpcode = OP_WaitAll;
         break;
         
      case SFXPlayList::TRANSITION_Stop:
         insn.mOpcode = OP_StopSingle;
         break;

      case SFXPlayList::TRANSITION_StopAll:
         insn.mOpcode = OP_StopAll;
         break;
         
      default:
         AssertFatal( false, "SFXController::_addTransition() - should not reach here" );
   }
}

//-----------------------------------------------------------------------------

void SFXController::_initInsn()
{
   Insn& insn = mInsns[ mIp ];
   switch( insn.mOpcode )
   {
      case OP_Delay:
         mDelayEndTime = Platform::getVirtualMilliseconds()
            + U32( insn.mArg.mDelayTime.getValue( 0, 0.0f ) * 1000.f );
         break;
         
      default:
         break;
   }
   
   if( mTrace )
      _printInsn(insn );
}

//-----------------------------------------------------------------------------

void SFXController::_printInsn( Insn& insn)
{
   switch( insn.mOpcode )
   {
      case OP_Delay:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: Delay %f:%f:%f",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "",
            insn.mArg.mDelayTime.mValue[ 0 ],
            insn.mArg.mDelayTime.mVariance[ 0 ],
            insn.mArg.mDelayTime.mVariance[ 1 ]
         );
         break;
         
      case OP_WaitSingle:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: WaitSingle",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;

      case OP_WaitAll:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: WaitAll",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;

      case OP_StopSingle:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: StopSingle",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;

      case OP_StopAll:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: StopAll",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;
         
      case OP_Play:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: Play",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;
         
      case OP_Jump:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: Jump %i",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "", insn.mArg.mJumpIp );
         break;
         
      case OP_LoopBegin:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: LoopBegin %i",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "", insn.mArg.mLoopCount );
         break;
         
      case OP_LoopEnd:
         Con::printf( "[SFXController] ip=%d: slot=%d: state=%s: LoopEnd",
            mIp, insn.mSlotIndex, insn.mState ? insn.mState->getName() : "" );
         break;
   }
}

//-----------------------------------------------------------------------------

bool SFXController::_execInsn()
{
   bool endUpdate = false;
   Insn& insn = mInsns[ mIp ];
   
   switch( insn.mOpcode )
   {
      case OP_Delay:
      {
         if( Platform::getVirtualMilliseconds() < mDelayEndTime )
            endUpdate = true;
         else
            _advanceIp();
         break;
      }
         
      case OP_Play:
      {
         SFXPlayList* playList = getPlayList();
         SFXTrack* track = playList->getSlots().mTrack[ insn.mSlotIndex ];
         
         // Handle existing sources playing on this slot and find
         // whether we need to start a new source.
         //
         // Go through the list top-down so we can push sources we re-use
         // to the top of the list.  A side-effect of doing it this way is
         // that the order of the sources that are preserved gets reversed,
         // i.e. older sources will end up higher up the stack.
         
         bool startNew = true;
         SFXPlayList::EReplayMode replayMode = playList->getSlots().mReplayMode[ insn.mSlotIndex ];
         if( replayMode != SFXPlayList::REPLAY_IgnorePlaying )
            for( S32 i = mSources.size() - 1; i >= 0; -- i )
            {
               Source& source = mSources[ i ];
               if( source.mSlotIndex != insn.mSlotIndex )
                  continue;
                  
               // If the play-once source has expired, remove the entry
               // and go on.
                  
               if( source.mPtr == NULL )
               {
                  mSources.erase( i );
                  ++ i;
                  continue;
               }
               
               // Decide what to do with the still-playing source.
               
               if(    replayMode == SFXPlayList::REPLAY_RestartPlaying
                   || replayMode == SFXPlayList::REPLAY_KeepPlaying )
               {
                  // Restart the source or keep playing it.
                  // Either way, move it to the top of the stack.
                  
                  startNew = false;
                  
                  Source src = mSources[ i ];
                  mSources.erase( i );
                  
                  //RDTODO: add a method to restart cleanly in the presence of fades; this here
                  //    just cuts the current playback short

                  if( replayMode == SFXPlayList::REPLAY_RestartPlaying )
                     src.mPtr->stop( 0.f );
                     
                  src.mPtr->play();
                  
                  // Move the source to the top of the stack.
                  
                  mSources.increment();
                  mSources.last() = src;
               }
               else if( replayMode == SFXPlayList::REPLAY_StartNew )
               {
                  // Kill off existing source.
                  
                  source.mPtr->stop();
                  mSources.erase( i );
                  ++ i;
               }
               else if( replayMode == SFXPlayList::REPLAY_SkipIfPlaying )
               {
                  startNew = false;
                  break;
               }
            }
                  
         if( startNew )
         {               
            // Create a new source.

            SFXSource* source = SFX->createSource(
               track,
               &getTransform(),
               &getVelocity()
            );

            // Append the source to the list of playing sources.

            if( source )
            {
               mSources.increment();
               Source& src = mSources.last();

               // Determine fade times.
               
               F32 fadeInTime = -1;
               F32 fadeOutTime = -1;
               
               if( playList->getSlots().mFadeTimeIn.mValue[ insn.mSlotIndex ] != -1 )
                  fadeInTime = playList->getSlots().mFadeTimeIn.getValue( insn.mSlotIndex, 0.f );
               if( playList->getSlots().mFadeTimeOut.mValue[ insn.mSlotIndex ] != -1 )
                  fadeOutTime = playList->getSlots().mFadeTimeOut.getValue( insn.mSlotIndex, 0.f );
                  
               if( fadeInTime != -1 || fadeOutTime != -1 )
                  source->setFadeTimes( fadeInTime, fadeOutTime );

               // Set up source record.
               
               src.mPtr = source;
               src.mSlotIndex = insn.mSlotIndex;
               src.mVolumeScale = playList->getSlots().mVolumeScale.getValue( insn.mSlotIndex, 0.f, 1.f );
               src.mPitchScale = playList->getSlots().mPitchScale.getValue( insn.mSlotIndex );
               src.mFadeInTime = fadeInTime;
               src.mFadeOutTime = fadeOutTime;
                              
               SFXPlayList::EStateMode stateMode = playList->getSlots().mStateMode[ insn.mSlotIndex ];
               if( stateMode != SFXPlayList::STATE_IgnoreInactive )
                  src.mState = insn.mState;
                                 
               // Set the source's volume and pitch.  Either is scaled by our own
               // assigned value and the scale factors from the playlist slot.
               
               source->setModulativeVolume( mAttenuatedVolume * src.mVolumeScale );
               source->setModulativePitch( mEffectivePitch * src.mPitchScale );
               
               // Set min and max range.
               
               const SFXPlayList::VariantFloat& minDistance = playList->getSlots().mMinDistance;
               const SFXPlayList::VariantFloat& maxDistance = playList->getSlots().mMaxDistance;
               
               if(    minDistance.mValue[ insn.mSlotIndex ] >= 0.f
                   && maxDistance.mValue[ insn.mSlotIndex ] >= 0.f )
                  source->setMinMaxDistance(
                     minDistance.getValue( insn.mSlotIndex, 0.f ),
                     maxDistance.getValue( insn.mSlotIndex, 0.f )
                  );
                  
               // Start the source.
               
               source->play();
               SFX->deleteWhenStopped( source );
            }
         }
         
         _advanceIp();
         break;
      }
      
      case OP_WaitSingle:
      {
         if( !mSources.empty() && mSources.last().mPtr != NULL && mSources.last().mPtr->isPlaying() )
            endUpdate = true;
         else
         {
            if( !mSources.empty() )
               mSources.decrement();
            _advanceIp();
         }
         break;
      }

      case OP_WaitAll:
      {
         for( U32 i = 0; i < mSources.size(); ++ i )
            if( mSources[ i ].mPtr != NULL && mSources[ i ].mPtr->isStopped() )
            {
               mSources.erase( i );
               -- i;
            }
            
         if( !mSources.empty() )
            endUpdate = true;
         else
            _advanceIp();
            
         break;
      }
      
      case OP_StopSingle:
      {
         if( !mSources.empty() )
         {
            if( mSources.last().mPtr != NULL )
               mSources.last().mPtr->stop();
            mSources.decrement();
         }
         
         _advanceIp();
         break;
      }
      
      case OP_StopAll:
      {
         while( !mSources.empty() )
         {
            if( mSources.last().mPtr != NULL )
               mSources.last().mPtr->stop();
            mSources.decrement();
         }
         
         _advanceIp();
         break;
      }
      
      case OP_Jump:
      {
         mIp = insn.mArg.mJumpIp;
         _initInsn();
         break;
      }
      
      case OP_LoopBegin:
      {
         mLoopCounter = insn.mArg.mLoopCount;
         _advanceIp();
         break;
      }
      
      case OP_LoopEnd:
      {
         -- mLoopCounter;
         if( mLoopCounter > 0 )
         {
            mIp = insn.mArg.mJumpIp;
            _initInsn();
         }
         else
            _advanceIp();
            
         break;
      }
   }
   
   return endUpdate;
}

//-----------------------------------------------------------------------------

void SFXController::_advanceIp()
{
   mIp ++;
   if( mIp < mInsns.size() )
      _initInsn();
}

//-----------------------------------------------------------------------------

void SFXController::_onParameterEvent( SFXParameter* parameter, SFXParameterEvent event )
{
   Parent::_onParameterEvent( parameter, event );
   
   // Implement cursor semantic.
   
   if(    event == SFXParameterEvent_ValueChanged
       && parameter->getChannel() == SFXChannelCursor )
   {
      U32 slot = U32( mFloor( parameter->getValue() ) );
      if( slot != getCurrentSlot() )
         setCurrentSlot( slot );
   }
}

//-----------------------------------------------------------------------------

SFXPlayList* SFXController::getPlayList() const
{
   return static_cast< SFXPlayList* >( mTrack.getPointer() );
}

//-----------------------------------------------------------------------------

U32 SFXController::getCurrentSlot() const
{
   if( mIp >= mInsns.size() )
      return 0;
   else
      return mInsns[ mIp ].mSlotIndex;
}

//-----------------------------------------------------------------------------

void SFXController::setCurrentSlot( U32 index )
{
   mIp = 0;
   while( mIp < mInsns.size() && mInsns[ mIp ].mSlotIndex != index )
      ++ mIp;
      
   if( mIp >= mInsns.size() )
      mIp = 0;
    
   if( !mInsns.empty() )
      _initInsn();
}

//-----------------------------------------------------------------------------

void SFXController::_play()
{
   Parent::_play();

   // Unpause sources, if we are paused.
   
   if( mStatus == SFXStatusPaused )
      for( U32 i = 0; i < mSources.size(); ++ i )
         if( mSources[ i ].mPtr != NULL )
            mSources[ i ].mPtr->play( 0.f ); // We want our fade values to take effect.
         else
         {
            mSources.erase( i );
            -- i;
         }
}

//-----------------------------------------------------------------------------

void SFXController::_pause()
{
   Parent::_pause();

   // Pause all playing sources.
   
   for( U32 i = 0; i < mSources.size(); ++ i )
      if( mSources[ i ].mPtr != NULL )
         mSources[ i ].mPtr->pause( 0.f ); // We want our fade values to take effect.
      else
      {
         mSources.erase( i );
         -- i;
      }      
}

//-----------------------------------------------------------------------------

void SFXController::_stop()
{
   Parent::_stop();

   // Stop all playing sources.
   
   while( !mSources.empty() )
   {
      if( mSources.last().mPtr != NULL )
         mSources.last().mPtr->stop( 0.f ); // We want our fade values to take effect.
      mSources.decrement();
   }
   
   // Reset execution.
   
   mIp = 0;
   if( !mInsns.empty() )
      _initInsn();      
}

//-----------------------------------------------------------------------------

void SFXController::_updateVolume( const MatrixF& listener )
{
   F32 oldAttenuatedVolume = mAttenuatedVolume;
   Parent::_updateVolume( listener );
   
   // If the attenuated volume has changed, pass it off
   // as the modulative volume to all our sources.
   
   if( oldAttenuatedVolume != mAttenuatedVolume )
      for( U32 i = 0; i < mSources.size(); ++ i )
      {
         Source& source = mSources[ i ];
         if( source.mPtr != NULL )
            source.mPtr->setModulativeVolume( mAttenuatedVolume * source.mVolumeScale );
         else
         {
            mSources.erase( i );
            -- i;
         }
      }
}

//-----------------------------------------------------------------------------

void SFXController::_updatePitch()
{
   F32 oldEffectivePitch = mEffectivePitch;
   Parent::_updatePitch();
   
   if( mEffectivePitch != oldEffectivePitch )
      for( U32 i = 0; i < mSources.size(); ++ i )
      {
         Source& source = mSources[ i ];
         if( source.mPtr != NULL )
            source.mPtr->setModulativePitch( mEffectivePitch * source.mPitchScale );
         else
         {
            mSources.erase( i );
            -- i;
         }
      }
}

//-----------------------------------------------------------------------------

void SFXController::_updatePriority()
{
   F32 oldEffectivePriority = mEffectivePriority;
   Parent::_updatePriority();
   
   if( mEffectivePriority != oldEffectivePriority )
      for( U32 i = 0; i < mSources.size(); ++ i )
      {
         Source& source = mSources[ i ];
         if( source.mPtr != NULL )
            source.mPtr->setModulativePriority( mEffectivePriority );
         else
         {
            mSources.erase( i );
            -- i;
         }
      }
}

//-----------------------------------------------------------------------------

void SFXController::_update()
{
   Parent::_update();
   
   SFXPlayList* playList = getPlayList();

   // Check all sources against the current state setup and
   // take appropriate actions.
 
   for( U32 i = 0; i < mSources.size(); ++ i )
   {
      Source& source = mSources[ i ];
      
      // If the source has already stopped playing,
      // remove it.
      
      if( !source.mPtr )
      {
         mSources.erase( i );
         -- i;
         continue;
      }
      
      if( !source.mState )
         continue;
         
      SFXPlayList::EStateMode stateMode = playList->getSlots().mStateMode[ mSources[ i ].mSlotIndex ];
      if( !source.mState->isActive() )
      {
         if( source.mPtr->isPlaying() )
         {
            // The source is playing in an incompatible state.
            
            if( stateMode == SFXPlayList::STATE_PauseInactive )
               source.mPtr->pause();
            else if( stateMode == SFXPlayList::STATE_StopInactive )
            {
               source.mPtr->stop();
               mSources.erase( i );
            
               -- i;
            }
         }
      }
      else
      {
         // Unpause a source that had its state become active again.
         
         if( source.mPtr->isPaused() && stateMode == SFXPlayList::STATE_PauseInactive )
            source.mPtr->play();
      }
   }
      
   // Update interpreter.
      
   bool endUpdate = false;
   while( !endUpdate )
   {
      if( mIp >= mInsns.size() )
      {
         // End of list reached.
         
         if( playList->getDescription()->mIsLooping &&
             playList->getLoopMode() == SFXPlayList::LOOP_All )
         {
            // The play list is set to repeat-all.
            // If it is also random, generate a new instruction list
            // so we get a new playing order.  Otherwise just reset.
            
            if( playList->getRandomMode() != SFXPlayList::RANDOM_NotRandom )
               _compileList( playList );
            else
            {
               mIp = 0;
               if( !mInsns.empty() )
                  _initInsn();
            }
            
            // Reset play timer.
            
            mPlayTimer.reset();
            mPlayTimer.start();
         }
         else
         {
            // Moved to stopped state.
            
            mPlayTimer.stop();
            _setStatus( SFXStatusStopped );
            mIp = 0;
         }
         
         // End this update.  This limits playlist to at most one complete
         // cycle per update.
         
         break;
      }
         
      Insn& insn = mInsns[ mIp ];
      
      if( insn.mState && !insn.mState->isActive() )
      {
         // The state associated with the slot is inactive.  Skip
         // the instructions.
         _advanceIp();
      }
      else
         endUpdate = _execInsn();
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXController, getCurrentSlot, S32, (),,
   "Get the index of the playlist slot currently processed by the controller.\n"
   "@return The slot index currently being played.\n"
   "@see SFXPlayList" )
{
   return object->getCurrentSlot();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXController, setCurrentSlot, void, ( S32 index ),,
   "Set the index of the playlist slot to play by the controller.  This can be used to seek in the playlist.\n"
   "@param index Index of the playlist slot." )
{
   object->setCurrentSlot( index );
}
