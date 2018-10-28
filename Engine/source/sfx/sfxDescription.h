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

#ifndef _SFXDESCRIPTION_H_
#define _SFXDESCRIPTION_H_

#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif
#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _MPOINT3_H_
   #include "math/mPoint3.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _MEASE_H_
   #include "math/mEase.h"
#endif


class SFXSource;


/// The SFXDescription defines how a sound should be played.
///
/// If mConeInsideAngle and mConeOutsideAngle are not both
/// 360 then the sound will be directional and facing out
/// the Y axis.  To reorient the cones, reorient the emitter
/// object.
///
/// A few tips:
///
/// Make sure that server SFXDescription are defined with the 
/// datablock keyword, and that client SFXDescription are defined
/// with the 'new' or 'singleton' keyword.
///
class SFXDescription : public SimDataBlock
{
      typedef SimDataBlock Parent;

   public:
   
      enum
      {
         MaxNumParameters = 8,
      };

      /// The 0 to 1 volume scale.
      F32 mVolume;
      
      /// The pitch scale.
      F32 mPitch;

      /// If true the sound will loop.
      bool mIsLooping;

      /// If true the sound data will be streamed from
      /// disk and not loaded completely into memory.
      bool mIsStreaming;

      /// If true the sound will be 3D positional.
      bool mIs3D;
      
      /// If this sound is allowed to be mixed in hardware.
      bool mUseHardware;

      /// If true the sound uses custom reverb properties.
      bool mUseReverb;
      
      /// The distance from the emitter at which the
      /// sound volume is unchanged.  Beyond this distance
      /// the volume begins to falloff.
      ///
      /// This is only valid for 3D sounds.
      F32 mMinDistance;

      /// The distance from the emitter at which the
      /// sound volume becomes zero.
      ///
      /// This is only valid for 3D sounds.
      F32 mMaxDistance;

      /// The angle in degrees of the inner part of
      /// the cone.  It must be within 0 to 360.
      ///
      /// This is only valid for 3D sounds.
      U32 mConeInsideAngle;

      /// The angle in degrees of the outer part of
      /// the cone.  It must be greater than mConeInsideAngle
      /// and less than to 360.
      ///
      /// This is only valid for 3D sounds.
      U32 mConeOutsideAngle;

      /// The volume scalar for on/beyond the outside angle.
      ///
      /// This is only valid for 3D sounds.
      F32 mConeOutsideVolume;
      
      /// Local logarithmic distance attenuation rolloff factor.  -1 to use global setting.
      /// Per-sound rolloff settings only supported with some SFX providers.
      F32 mRolloffFactor;
      
      /// Max distance in both directions along each axis by which the
      /// sound position of a 3D sound will be randomly scattered.
      ///
      /// Example: if you set x=5, then the final x coordinate of the 3D
      ///   sound will be (OriginalX + randF( -5, +5 )).
      Point3F mScatterDistance;

      /// The source to which sources playing with this description will
      /// be added.
      SFXSource* mSourceGroup;
      
      /// Number of seconds until playback reaches full volume after starting/resuming.
      /// Zero to deactivate (default).
      F32 mFadeInTime;
      
      /// Number of seconds to fade out fading before stopping/pausing.
      /// Zero to deactivate (default).
      F32 mFadeOutTime;
      
      /// Easing curve for fade-in.
      EaseF mFadeInEase;
      
      /// Easing curve for fade-out.
      EaseF mFadeOutEase;
      
      /// When mIsLooping is true, the fades will apply to each cycle.  Otherwise, only
      /// the first cycle will have a fade-in applied and no fade-out happens when a cycle
      /// ends.
      ///
      /// This also affects the playtime that is used to place fades.  If mFadeLoops is
      /// false, the total playtime so far will be used rather than the playtime of the
      /// current cycle.  This makes it possible to apply fades that extend across two or
      /// more loops of the sound, i.e. are longer than the actual sound duration.
      bool mFadeLoops;

      /// The number of seconds of sound data to read per streaming
      /// packet.  Only relevant if "isStreaming" is true.
      U32 mStreamPacketSize;

      /// The number of streaming packets to read and buffer in advance.
      /// Only relevant if "isStreaming" is true.
      U32 mStreamReadAhead;

      /// Reverb properties for sound playback.
      SFXSoundReverbProperties mReverb;
            
      /// Parameters to which sources playing with this description should automatically
      /// connect when created.
      StringTableEntry mParameters[ MaxNumParameters ];
      
      /// Priority level for sounds.  Higher priority sounds are less likely to be
      /// culled.
      F32 mPriority;

      SFXDescription();
      SFXDescription( const SFXDescription& desc );
      DECLARE_CONOBJECT( SFXDescription );
      static void initPersistFields();

      // SimDataBlock.
      virtual bool onAdd();
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      virtual void inspectPostApply();

      /// Validates the description fixing any
      /// parameters that are out of range.
      void validate();
};


#endif // _SFXDESCRIPTION_H_