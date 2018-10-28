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

#include "sfx/sfxSoundscape.h"
#include "sfx/sfxAmbience.h"
#include "sfx/sfxEnvironment.h"
#include "sfx/sfxState.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxSystem.h"


//#define DEBUG_SPEW

//FIXME: fix reverb resetting

//=============================================================================
//    SFXSoundscape.
//=============================================================================
// MARK: ---- SFXSoundscape ----

//-----------------------------------------------------------------------------

SFXSoundscape::SFXSoundscape( SFXAmbience* ambience )
   : mAmbience( ambience )
{
   mDirtyBits.set( AmbienceDirty );
   mFlags.set( FlagUnique );
   
   dMemset( mStates, 0, sizeof( mStates ) );
}

//-----------------------------------------------------------------------------

void SFXSoundscape::setAmbience( SFXAmbience* ambience )
{
   AssertFatal( ambience != NULL, "SFXSoundscape::setAmbience - ambience cannot be NULL!" );
   mDirtyBits.set( AmbienceDirty );
   mAmbience = ambience;
}

//=============================================================================
//    SFXSoundscapeManager.
//=============================================================================
// MARK: ---- SFXSoundscapeManager ----

//-----------------------------------------------------------------------------

SFXSoundscapeManager::SFXSoundscapeManager()
   : mCurrentReverbIndex( -1 )
{
   VECTOR_SET_ASSOCIATION( mStack );
   VECTOR_SET_ASSOCIATION( mFadeStack );
   
#ifndef TORQUE_SHIPPING

   // Hook on to the ambience change signal (used
   // to respond to editing of ambiences).
   
   SFXAmbience::getChangeSignal().notify
      ( this, &SFXSoundscapeManager::_notifyAmbienceChanged );
   
#endif

   // Push the global ambience.
   
   mDefaultGlobalAmbience = new SFXAmbience;
   SFXSoundscape* global = mChunker.alloc();
   constructInPlace( global, mDefaultGlobalAmbience );
   mStack.push_back( global );
}

//-----------------------------------------------------------------------------

SFXSoundscapeManager::~SFXSoundscapeManager()
{
#ifndef TORQUE_SHIPPING

   // Remove the hook on the ambience change signal.
   
   SFXAmbience::getChangeSignal().remove
      ( this, &SFXSoundscapeManager::_notifyAmbienceChanged );
      
#endif

   mDefaultGlobalAmbience->deleteObject();
}

//-----------------------------------------------------------------------------

void SFXSoundscapeManager::update()
{
   // Make sure the topmost reverb on the stack is active.
   
   S32 reverbIndex = _findTopmostReverbOnStack( mStack );
   if( mCurrentReverbIndex != reverbIndex )
   {
      if( reverbIndex == -1 )
      {
         // No ambience on the stack has reverb settings so reset
         // to default.
         SFX->setReverb( SFXReverbProperties() );
      }
      else
      {
         SFXAmbience* ambience = mStack[ reverbIndex ]->getAmbience();
         AssertFatal( ambience->getEnvironment(), "SFXSoundscapeManager::update - Reverb lookup return ambience without reverb!" );

         SFX->setRolloffFactor( ambience->getRolloffFactor() );
         SFX->setDopplerFactor( ambience->getDopplerFactor() );
         SFX->setReverb( ambience->getEnvironment()->getReverb() );
      }

      mCurrentReverbIndex = reverbIndex;
   }
   
   // Update the active soundscapes.
   
   for( U32 i = 0; i < mStack.size(); ++ i )
   {
      SFXSoundscape* soundscape = mStack[ i ];
      
      // If the soundscape's associated ambience has changed
         
      if( soundscape->mDirtyBits.test( SFXSoundscape::AmbienceDirty ) )
      {
         SFXAmbience* ambience = soundscape->getAmbience();
         
         // Start playing the ambient audio track if it isn't
         // already playing and if the soundscape isn't overridden
         // by an instance lower down the stack.
         
         if( !soundscape->_isOverridden() )
         {
            SFXTrack* track = ambience->getSoundTrack();
            if( !soundscape->mSource || soundscape->mSource->getTrack() != track )
            {
               if( soundscape->mSource != NULL )
               {
                  soundscape->mSource->stop();
                  soundscape->mSource = NULL;
               }
               
               if( track )
                  soundscape->mSource = SFX->playOnce( track );
            }
            else if( soundscape->mSource != NULL )
            {
               // Make sure to revert a fade-out running on the source
               // if it has been taken from the fade stack.
               
               soundscape->mSource->play();
            }
         }
         
         // Activate SFXStates on the ambience.  For state slots that
         // have changed, deactivate states that we have already activated.
         
         for( U32 i = 0; i < SFXAmbience::MaxStates; ++ i )
         {
            SFXState* state = ambience->getState( i );
            if( soundscape->mStates[ i ] != state )
            {
               if( soundscape->mStates[ i ] )
                  soundscape->mStates[ i ]->deactivate();

               if( state )
                  state->activate();
                  
               soundscape->mStates[ i ] = state;
            }
         }
         
         soundscape->mDirtyBits.clear( SFXSoundscape::AmbienceDirty );
      }
   }
   
   // Clean out the fade stack.
   
   for( U32 i = 0; i < mFadeStack.size(); )
      if( mFadeStack[ i ]->mSource == NULL )
      {
         SFXSoundscape* soundscape = mFadeStack[ i ];
         mFadeStack.erase_fast( i );
         
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXSoundscapeManager] Deleting faded instance of '%s'",
            soundscape->getAmbience()->getName() );
         #endif
         
         // Free the soundscape.
         
         destructInPlace( soundscape );
         mChunker.free( soundscape );
      }
      else
         ++ i;
}

//-----------------------------------------------------------------------------

SFXSoundscape* SFXSoundscapeManager::insertSoundscape( U32 index, SFXAmbience* ambience )
{
   AssertFatal( index <= mStack.size(), "SFXSoundscapeManager::insertSoundscape - index out of range" );
   AssertFatal( index != 0, "SFXSoundscapeManager::insertSoundscape - cannot insert before global soundscape" );
   AssertFatal( ambience != NULL, "SFXSoundscapeManager::insertSoundscape - got a NULL ambience" );
   
   // Look for an existing soundscape that is assigned the
   // same ambience.
   
   S32 ambientInstanceIndex = _findOnStack( ambience, mStack );
   
   // Push a soundscape unto the stack.  If there is an instance
   // on the fade stack that is tied to the given ambience, bring that
   // soundscape over to the active stack.  Otherwise create a new
   // soundscape instance.
   
   SFXSoundscape* soundscape = NULL;
   if( ambientInstanceIndex == -1 )
   {
      S32 fadeIndex = _findOnStack( ambience, mFadeStack );
      if( fadeIndex != -1 )
      {
         soundscape = mFadeStack[ fadeIndex ];
         mFadeStack.erase_fast( fadeIndex );
         
         // Make sure the soundscape will get re-evaluated
         // on the next update.
         
         soundscape->mDirtyBits.set( SFXSoundscape::AmbienceDirty );

         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXSoundscapeManager] Moving ambience '%s' from fade stack to #%i (total: %i)",
            ambience->getName(), index, mStack.size() + 1 );
         #endif
      }
   }
   
   if( !soundscape )
   {
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXSoundscapeManager] Adding new instance for '%s' at #%i (total: %i)",
         ambience->getName(), index, mStack.size() + 1 );
      #endif
      
      soundscape = mChunker.alloc();
      constructInPlace( soundscape, ambience );
   }
   
   mStack.insert( index, soundscape );
   
   // If there is an existing soundscape that is assigned the
   // same ambience and it is lower on the stack, steal its sound
   // source if it has one.  If it is higher up the stack, simply
   // mark this soundscape as being overridden.
   
   if( ambientInstanceIndex != -1 )
   {
      SFXSoundscape* existingSoundscape = mStack[ ambientInstanceIndex ];
      
      existingSoundscape->mFlags.clear( SFXSoundscape::FlagUnique );
      soundscape->mFlags.clear( SFXSoundscape::FlagUnique );
      
      if( ambientInstanceIndex < index )
      {
         existingSoundscape->mFlags.set( SFXSoundscape::FlagOverridden );

         SFXSource* source = existingSoundscape->mSource;
         existingSoundscape->mSource = NULL;
         
         if( source && source->isPlaying() )
            soundscape->mSource = source;
      }
      else
      {
         soundscape->mFlags.set( SFXSoundscape::FlagOverridden );
      }
   }
      
   return soundscape;
}

//-----------------------------------------------------------------------------

void SFXSoundscapeManager::removeSoundscape( SFXSoundscape* soundscape )
{
   AssertFatal( soundscape != getGlobalSoundscape(),
      "SFXSoundscapeManager::removeSoundscape() - trying to remove the global soundscape" );
      
   // Find the soundscape on the stack.
            
   U32 index = 1;
   for( ; index < mStack.size(); ++ index )
      if( mStack[ index ] == soundscape )
         break;
         
   AssertFatal( index < mStack.size(),
      "SFXSoundscapeManager::removeSoundscape() - soundscape not on stack" );
      
   // Find out if the soundscape has the current reverb
   // environment.  If so, we need to change the reverb to
   // the next one higher up the stack.

   const bool isCurrentReverb = ( _findTopmostReverbOnStack( mStack ) == index );
   
   // Remove the soundscape from the stack.

   mStack.erase( index );
   
   // Update reverb, if necessary.
   
   if( isCurrentReverb )
   {
      S32 reverbIndex = _findTopmostReverbOnStack( mStack );
      if( reverbIndex != -1 )
         SFX->setReverb( mStack[ reverbIndex ]->getAmbience()->getEnvironment()->getReverb() );
   }
   
   // Deactivate states.
   
   for( U32 i = 0; i < SFXAmbience::MaxStates; ++ i )
      if( soundscape->mStates[ i ] )
      {
         soundscape->mStates[ i ]->deactivate();
         soundscape->mStates[ i ] = NULL;
      }
   
   // If the soundscape is the only instance of its ambience, move
   // it to the fade stack.  Otherwise delete the soundscape.
   
   if( soundscape->_isUnique() && soundscape->mSource != NULL )
   {
      #ifdef DEBUG_SPEW
      Platform::outputDebugString( "[SFXSoundscapeManager] Moving ambience '%s' at index #%i to fade stack",
         soundscape->getAmbience()->getName(), index );
      #endif
      
      soundscape->mSource->stop();
      mFadeStack.push_back( soundscape );      
   }
   else
   {
      if( !soundscape->_isUnique() )
      {
         // If this is the overriding soundscape, transfer its state
         // to the ambient instance lower down the stack.
         
         if( !soundscape->_isOverridden() )
         {
            S32 overrideeIndex = index - 1;
            for( ; overrideeIndex >= 0; -- overrideeIndex )
               if( soundscape->getAmbience() == mStack[ overrideeIndex ]->getAmbience() )
                  break;
                  
            AssertFatal( overrideeIndex >= 0,
               "SFXSoundscapeManager::removeSoundscape() - could not find ambience being overridden on stack" );
               
            // Pass the source on to the previous soundscape.
               
            mStack[ overrideeIndex ]->mSource = soundscape->mSource;
            soundscape->mSource = NULL;
         }
         
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXSoundscapeManager] Removing instance of '%s' at index #%i",
            soundscape->getAmbience()->getName(), index );
         #endif
         
         // If there's only one instance of this ambience is
         // left, mark it as being unique now.
         
         U32 numInstances = 0;
         for( U32 i = 0; i < mStack.size(); ++ i )
            if( mStack[ i ]->getAmbience() == soundscape->getAmbience() )
               ++ numInstances;
               
         if( numInstances == 1 )
            mStack[ _findOnStack( soundscape->getAmbience(), mStack ) ]->mFlags.set( SFXSoundscape::FlagUnique );
      }
         
      // Free the soundscape.
         
      destructInPlace( soundscape );
      mChunker.free( soundscape );
   }
}

//-----------------------------------------------------------------------------

S32 SFXSoundscapeManager::_findOnStack( SFXAmbience* ambience, const Vector< SFXSoundscape* >& stack )
{
   // Search the stack top to bottom so we always find
   // the uppermost instance of the ambience on the stack.
   
   for( S32 i = stack.size() - 1; i >= 0; -- i )
      if( stack[ i ]->getAmbience() == ambience )
         return i;
         
   return -1;
}

//-----------------------------------------------------------------------------

S32 SFXSoundscapeManager::_findTopmostReverbOnStack( const Vector< SFXSoundscape* >& stack )
{
   for( S32 i = stack.size() - 1; i >= 0; -- i )
      if( stack[ i ]->getAmbience()->getEnvironment() )
         return i;
         
   return -1;
}

//-----------------------------------------------------------------------------

void SFXSoundscapeManager::_notifyAmbienceChanged( SFXAmbience* ambience )
{
   //RDTODO: fade stack?
   
   // Set the ambience dirty bit on all soundscapes
   // tied to the given ambience.
   
   for( U32 i = 0; i < mStack.size(); ++ i )
      if( mStack[ i ]->getAmbience() == ambience )
      {
         mStack[ i ]->mDirtyBits.set( SFXSoundscape::AmbienceDirty );
         
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[SFXSoundscapeManager] Ambience '%s' at #%i changed",
            ambience->getName(), i );
         #endif
      }
}
