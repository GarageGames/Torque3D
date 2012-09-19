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

#ifndef _SFXMODIFIER_H_
#define _SFXMODIFIER_H_

#ifndef _TSTREAM_H_
   #include "core/stream/tStream.h"
#endif


class SFXSource;


/// An SFXModifier modifies the properties of an SFXSource while its playback
/// is running.
class SFXModifier : public IPolled
{
   protected:
      
      /// The source that this effect works on.
      SFXSource* mSource;
      
      /// If true, the effect is removed from the effects stack
      bool mRemoveWhenDone;
            
   public:
   
      /// Create an effect that operates on "source".
      SFXModifier( SFXSource* source, bool removeWhenDone = false )
         : mSource( source ) {}
   
      virtual ~SFXModifier() {}
};

/// An SFXModifier that is triggered once after passing a certain playback position.
class SFXOneShotModifier : public SFXModifier
{
   public:
   
      typedef SFXModifier Parent;
      
   protected:
   
      /// Playback position that triggers the effect.
      F32 mTriggerPos;
      
      ///
      virtual void _onTrigger() = 0;
      
   public:
   
      /// Create an effect that triggers when playback of "source" passes "triggerPos".
      SFXOneShotModifier( SFXSource* source, F32 triggerPos, bool removeWhenDone = false );
   
      // IPolled.
      virtual bool update();
};

/// An SFXModifier that is spans a certain range of playback time.
class SFXRangeModifier : public SFXModifier
{
   public:

      typedef SFXModifier Parent;
      
   protected:
   
      /// If true, the effect is currently being applied to the source.
      bool mIsActive;
         
      /// Playback position in milliseconds when this effect becomes active.
      F32 mStartTime;
      
      /// Playback position in milliseconds when this effect becomes inactive.
      F32 mEndTime;
      
      /// Called when the play cursor passes mStartTime.
      /// @note There may be latency between the cursor actually passing mStartTime
      ///   and this method being called.
      virtual void _onStart() {}
      
      /// Called on each update() while the play cursor is in range.
      virtual void _onUpdate() {}
      
      /// Called when the play cursor passes mEndTime.
      /// @note There may be latency between the cursor actually passing mEndTime
      ///   and this method being called.
      virtual void _onEnd() {}
      
   public:
   
      /// Create an effect that operates on "source" between "startTime" seconds
      /// (inclusive) and "endTime" seconds (exclusive).
      SFXRangeModifier( SFXSource* source, F32 startTime, F32 endTime, bool removeWhenDone = false );
         
      ///
      bool isActive() const { return mIsActive; }
      
      // IPolled.
      virtual bool update();
};

/// A volume fade effect (fade-in or fade-out).
class SFXFadeModifier : public SFXRangeModifier
{
   public:
   
      typedef SFXRangeModifier Parent;
      
      enum EOnEnd
      {
         ON_END_Nop,       ///< Do nothing with source when fade is complete.
         ON_END_Stop,      ///< Stop source when fade is complete.
         ON_END_Pause,     ///< Pause source when fade is complete.
      };
      
   protected:
      
      /// Volume when beginning fade.  Set when effect is activated.
      F32 mStartVolume;
      
      /// Volume when ending fade.
      F32 mEndVolume;
      
      /// Current volume level.
      F32 mCurrentVolume;
      
      /// Action to perform when the fade has been completed.  Defaults to no action.
      EOnEnd mOnEnd;
      
      // SFXModifier.
      virtual void _onStart();
      virtual void _onUpdate();
      virtual void _onEnd();
      
   public:
   
      /// Create an effect that fades the volume of "source" to "endVolume" over the
      /// period of  "time" seconds.  The fade will start at "referenceTime" using the
      /// source's current volume at the time as the start.
      SFXFadeModifier( SFXSource* source, F32 time, F32 endVolume, F32 startTime, EOnEnd onEndDo = ON_END_Nop, bool removeWhenDone = false );
      
      virtual ~SFXFadeModifier();
};

/// A modifer that calls a method on the SFXSource when a particular playback position
/// is passed.
///
/// @note At the moment, doing a setPosition() on a source will not cause markers that have
///   been jumped over in the operation to be ignored.  Instead they will trigger on the
///   next update.
class SFXMarkerModifier : public SFXOneShotModifier
{
   public:
   
      typedef SFXOneShotModifier Parent;
      
   protected:
   
      /// Symbolic marker name that is passed to the "onMarkerPassed" callback.
      String mMarkerName;
      
      // SFXOneShotModifier
      virtual void _onTrigger();
   
   public:
   
      SFXMarkerModifier( SFXSource* source, const String& name, F32 pos, bool removeWhenDone = false );
};

#endif // !_SFXMODIFIER_H_
