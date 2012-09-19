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

#ifndef _SFXCONTROLLER_H_
#define _SFXCONTROLLER_H_

#ifndef _SFXSOURCE_H_
   #include "sfx/sfxSource.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _SFXSOURCE_H_
   #include "sfx/sfxSource.h"
#endif
#ifndef _SFXPLAYLIST_H_
   #include "sfx/sfxPlayList.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif


class SFXTrack;
class SFXProfile;
class SFXState;


/// SFXSource that drives multi-source playback.
///
/// Basically, this class is an interpreter for the instruction slots in
/// SFXPlayLists.
///
/// Controllers can be switched between states.  When no state is set, all
/// tracks from playlists that do not have a state set will be played.  When
/// setting a state, only tracks with the given state will be played.  If
/// currently tracks with a different state are playing, the respective
/// controllers will transition out of their respective slots.
///
class SFXController : public SFXSource
{
   public:
   
      typedef SFXSource Parent;
      friend class SFXSystem; // _create
   
   protected:
   
      typedef SFXVariantFloat< 1 > VariantFloat;
   
      enum EOp
      {
         OP_Delay,
         OP_WaitSingle,
         OP_WaitAll,
         OP_StopSingle,
         OP_StopAll,
         OP_Play,
         OP_Jump,
         OP_LoopBegin,
         OP_LoopEnd,
      };
      
      struct Insn
      {
         EOp mOpcode;
         U32 mSlotIndex;
         SFXState* mState;
         
         union
         {
            VariantFloat mDelayTime;
            U32 mJumpIp;
            U32 mLoopCount;
         } mArg;
         
         Insn() {}
         Insn( EOp opcode )
            : mOpcode( opcode ), mSlotIndex( U32_MAX ), mState( NULL ) {}
         Insn( U32 slotIndex, SFXState* state )
            : mSlotIndex( slotIndex ), mState( state ) {}
         Insn( EOp opcode, U32 slotIndex, SFXState* state )
            : mOpcode( opcode ), mSlotIndex( slotIndex ), mState( state ) {}
      };
      
      ///
      struct Source
      {
         /// The play-once source.
         SimObjectPtr< SFXSource > mPtr;
         
         /// The state to which the source is tied.  Only taken over from
         /// the instruction if the state mode is not set to ignored.
         SFXState* mState;
         
         /// Index of slot in playlist that this source was spawned on.
         U32 mSlotIndex;
         
         /// Volume scale factor to apply to the source.  Saved as it may have been
         /// randomly generated.
         F32 mVolumeScale;
         
         /// Pitch scale factor to apply to the source.  Saved as it may have been
         /// randomly generated.
         F32 mPitchScale;
         
         ///
         F32 mFadeInTime;
         
         ///
         F32 mFadeOutTime;
         
         Source()
            : mState( 0 ) {}
      };
            
      /// The current instruction in "mInsns".
      U32 mIp;
      
      /// The instruction list.  This is compiled from the playlist and then executed
      /// in the controller's update.
      Vector< Insn > mInsns;
      
      /// The stack of currently playing sources.
      ///
      /// All sources on this list are play-once sources so we can leave their lifetime
      /// management to the SFX system.  This is especially convenient in combination
      /// with fade-outs where a source cannot be immediately deleted.
      Vector< Source > mSources;
      
      ///
      bool mTrace;
      
      ///
      U32 mDelayEndTime;
      
      ///
      U32 mLoopCounter;
      
      ///
      SFXController( SFXPlayList* playList );
      
      ///
      void _printInsn( Insn& insn );
      
      ///
      void _compileList( SFXPlayList* playList );
      
      ///
      void _genTransition( Insn& insn, SFXPlayList::ETransitionMode transition );
      
      ///
      void _dumpInsns();
      
      ///
      void _initInsn();
      
      ///
      bool _execInsn();
      
      ///
      void _advanceIp();
      
      ///
      static SFXController* _create( SFXPlayList* playList );
      
      // SFXSource.
      virtual void _play();
      virtual void _pause();
      virtual void _stop();
      virtual void _onParameterEvent( SFXParameter* parameter, SFXParameterEvent event );
      virtual void _updateVolume( const MatrixF& listener );
      virtual void _updatePitch();
      virtual void _updatePriority();
      virtual void _update();
      
   public:
   
      ~SFXController();
   
      /// Constructor for the sake of ConsoleObject.
      explicit SFXController() {}
               
      /// Return the playlist being played back by the controller.
      SFXPlayList* getPlayList() const;
      
      /// Return the index of the playlist slot being processed by the controller.
      U32 getCurrentSlot() const;
      
      /// Set the index of the playlist slot to process.
      void setCurrentSlot( U32 index );
      
      // SFXSource.
      static void initPersistFields();
            
      DECLARE_CONOBJECT( SFXController );
      DECLARE_DESCRIPTION( "Controls the playback of an SFXPlayList." );
};

#endif // !_SFXCONTROLLER_H_

