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

#ifndef _SFXSOUNDSCAPE_H_
#define _SFXSOUNDSCAPE_H_

#ifndef _SFXCOMMON_H_
#include "sfx/sfxCommon.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif

#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif

#ifndef _TRESPONSECURVE_H_
#include "math/util/tResponseCurve.h"
#endif

#ifndef _SFXAMBIENCE_H_
#include "sfx/sfxAmbience.h"
#endif


/// @file
/// The soundscape system is responsible for controlling ambient audio.
/// It is largely driven by SFXWorld's listener tracking.


class SFXSource;




/// An instance of an SFXAmbience on the soundscape mixer stack.
///
class SFXSoundscape
{
   public:
   
      typedef void Parent;
      friend class SFXSoundscapeManager;
      
      enum Flags
      {
         FlagOverridden    = BIT( 0 ),    ///< Same ambience is pushed lower down onto the stack.
         FlagUnique        = BIT( 1 ),    ///< No other instance of this ambience on stack.
      };
      
      enum DirtyBits
      {
         AmbienceDirty = BIT( 0 ),        ///< Associated ambience has changed.
         AllDirty = 0xFFFFFFFF
      };
      
   protected:
   
      ///
      BitSet32 mFlags;
   
      ///
      BitSet32 mDirtyBits;

      /// The current soundtrack playing in this soundscape.  This is either the
      /// ambient track or the surround sound track depending on whether
      SimObjectPtr< SFXSource > mSource;
      
      /// The ambient space assigned to this soundscape.
      SFXAmbience* mAmbience;
      
      /// States activated by this soundscape.
      SFXState* mStates[ SFXAmbience::MaxStates ];
      
      /// Return true if another soundscape lower down the stack is using the same
      /// ambient space as this soundscape.
      bool _isOverridden() const { return mFlags.test( FlagOverridden ); }
      
      /// Return true if this soundscape is the only soundscape using the assigned
      /// ambient space.
      bool _isUnique() const { return mFlags.test( FlagUnique ); }

   public:
   
      /// Create a soundscape associated with the given ambient space.
      SFXSoundscape( SFXAmbience* ambience );

      /// Return the ambient space associated with this soundscape.
      SFXAmbience* getAmbience() const { return mAmbience; }
      
      /// Set the ambient space associated with this soundscape.  Triggers corresponding
      /// recomputations on the next soundscape manager update.
      void setAmbience( SFXAmbience* ambience );
};


/// The soundscape manager produces a dynamic mix between multiple active soundscapes.
///
/// @note The more layered soundscapes there are, the costlier ambient sound updates will get.
///
class SFXSoundscapeManager
{
   public:
   
      typedef void Parent;
      
   protected:

      /// Linear stack of soundscapes.  Evaluated bottom to top.  Scapes
      /// being faded out are always last on the stack.
      Vector< SFXSoundscape* > mStack;
      
      /// Stack of soundscape that are currently being faded out.  These are
      /// kept around so that when their ambient spaces are activated again,
      /// they can be brought back with their fades simply being reversed.
      ///
      /// All soundscapes on this stack are unique.
      Vector< SFXSoundscape* > mFadeStack;

      /// Index into #mStack for the soundscape that defines the current
      /// reverb settings.  -1 if no current ambience has reverb settings.
      S32 mCurrentReverbIndex;

      /// Memory manager of soundscape instances.
      FreeListChunker< SFXSoundscape > mChunker;
      
      /// Default global SFXAmbience.  Not a registered object.
      SFXAmbience* mDefaultGlobalAmbience;
      
      /// Found the topmost instance on the given stack associated with the given
      /// ambient space.
      S32 _findOnStack( SFXAmbience* ambience, const Vector< SFXSoundscape* >& stack );
      
      /// Find the topmost soundscape on the given stack that has a reverb environment
      /// defined on its ambient space.
      S32 _findTopmostReverbOnStack( const Vector< SFXSoundscape* >& stack );
      
      /// Method hooked up to the SFXAmbience change signal to automatically
      /// make soundscapes using the given ambience as dirty and trigger
      /// a recomputation of their properties.
      void _notifyAmbienceChanged( SFXAmbience* ambience );

   public:
   
      ///
      SFXSoundscapeManager();
      
      ///
      ~SFXSoundscapeManager();
   
      /// Update the current soundscape mix.
      void update();
      
      /// Return the total number of soundscape instances currently on the stack.
      /// Always >=1.
      U32 getNumTotalSoundscapes() const { return mStack.size(); }
   
      /// Insert a new soundscape instance associated with the given ambient space
      /// at the given stack index.
      SFXSoundscape* insertSoundscape( U32 index, SFXAmbience* ambience );
      
      /// Remove the given soundscape from the stack.
      void removeSoundscape( SFXSoundscape* soundscape );
      
      /// Return the topmost soundscape.  This soundscape is always defined and cannot
      /// be removed.
      SFXSoundscape* getGlobalSoundscape() const { return mStack[ 0 ]; }
};

#endif // !_SFXSOUNDSCAPE_H_
