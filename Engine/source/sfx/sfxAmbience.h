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

#ifndef _SFXAMBIENCE_H_
#define _SFXAMBIENCE_H_

#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif
#ifndef _TSIGNAL_H_
   #include "core/util/tSignal.h"
#endif


class SFXEnvironment;
class SFXTrack;
class SFXState;


/// Datablock for describing an ambient audio space.
///
class SFXAmbience : public SimDataBlock
{
   public:
   
      typedef SimDataBlock Parent;
      typedef Signal< void( SFXAmbience* ) > ChangeSignal;
      
      enum
      {
         /// Maximum number of states that can be tied to an ambient space.
         MaxStates = 4
      };
         
   protected:
   
      /// Doppler shift factor for this space.
      F32 mDopplerFactor;
      
      /// Rolloff factor for this space.  Only applies to logarithmic distance model.
      F32 mRolloffFactor;
   
      /// Sound track to play when inside the ambient space.
      SFXTrack* mSoundTrack;

      /// Reverb environment to apply when inside the ambient space.
      SFXEnvironment* mEnvironment;
      
      /// SFXStates to activate when in this ambient space.
      SFXState* mState[ MaxStates ];
      
      ///
      static ChangeSignal smChangeSignal;

   public:
   
      SFXAmbience();
            
      /// Ensure all properties of this ambience adhere to their value contraints.
      void validate();
      
      /// Return the rolloff factor to apply to distance-based volume attenuation in this space (logarithmic distance model only).
      F32 getRolloffFactor() const { return mRolloffFactor; }
      
      /// Return the doppler shift factor to apply in this space.
      F32 getDopplerFactor() const { return mDopplerFactor; }
            
      /// Return the reverb environment of the ambient space.
      SFXEnvironment* getEnvironment() const { return mEnvironment; }
      
      /// Return the ambient soundtrack of this ambient space.
      SFXTrack* getSoundTrack() const { return mSoundTrack; }
      
      /// Return the given state bound to this ambient space.
      SFXState* getState( U32 i ) const
      {
         AssertFatal( i < MaxStates, "SFXState::getState() - index out of range" );
         return mState[ i ];
      }
      
      ///
      static ChangeSignal& getChangeSignal() { return smChangeSignal; }
                  
      // SimDataBlock.
      virtual bool onAdd();
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      virtual bool preload( bool server, String& errorStr );
      virtual void inspectPostApply();
      
      static void initPersistFields();
   
      DECLARE_CONOBJECT( SFXAmbience );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "An ambient sound environment." );
};

#endif // !_SFXAMBIENCE_H_
