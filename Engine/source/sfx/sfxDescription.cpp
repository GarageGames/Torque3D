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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
#include "platform/platform.h"

#include "sfx/sfxDescription.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxInternal.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTypes.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "core/stringTable.h"
#include "core/module.h"
#include "math/mathIO.h"
#include "math/mathTypes.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXDescription );

ConsoleDocClass( SFXDescription,
   "@brief A description for how a sound should be played.\n\n"
   
   "SFXDescriptions are used by the sound system to collect all parameters needed to set up a given "
   "sound for playback.  This includes information like its volume level, its pitch shift, etc. as well "
   "as more complex information like its fade behavior, 3D properties, and per-sound reverb properties.\n\n"
   
   "Any sound playback will require a valid SFXDescription.\n\n"
   
   "As datablocks, SFXDescriptions can be set up as either networked datablocks or non-networked datablocks, "
   "though it generally makes sense to keep all descriptions non-networked since they will be used exclusively "
   "by clients.\n\n"
   
   "@tsexample\n"
   "// A description for a 3D sound with a reasonable default range setting.\n"
   "// The description is set up to assign sounds to the AudioChannelEffects source group\n"
   "// (defined in the core scripts).  An alternative means to achieve this is to use the\n"
   "// AudioEffects description as a copy source (\": AudioEffects\").\n"
   "\n"
   "singleton SFXDescription( Audio3DSound )\n"
   "{\n"
   "  sourceGroup       = AudioChannelEffects;\n"
   "  is3D              = true;\n"
   "  referenceDistance = 20.0;\n"
   "  maxDistance       = 100.0;\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@ingroup SFX\n"
   "@ingroup Datablocks\n"
);


// Constants.
static const U32 sReverbSoundFlagDirectHFAuto            = 0x01;
static const U32 sReverbSoundFlagRoomAuto                = 0x02;
static const U32 sReverbSoundFlagRoomHFAuto              = 0x04;
static const U32 sReverbSoundFlagInstance0               = 0x10;
static const U32 sReverbSoundFlagInstance1               = 0x20;
static const U32 sReverbSoundFlagInstance2               = 0x40;
static const U32 sReverbSoundFlagInstance3               = 0x80;

AFTER_MODULE_INIT( SFX )
{
   Con::addConstant( "SFXDescription::REVERB_DIRECTHFAUTO", TypeS32, &sReverbSoundFlagDirectHFAuto,
      "Automatic setting of SFXDescription::reverbDirect due to distance to listener.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_ROOMAUTO", TypeS32, &sReverbSoundFlagRoomAuto,
      "Automatic setting of SFXDescription::reverbRoom due to distance to listener.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_ROOMHFAUTO", TypeS32, &sReverbSoundFlagRoomHFAuto,
      "Automatic setting of SFXDescription::reverbRoomHF due to distance to listener.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_INSTANCE0", TypeS32, &sReverbSoundFlagInstance0,
      "EAX4/SFX/GameCube/Wii: Specify channel to target reverb instance 0. Default target.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_INSTANCE1", TypeS32, &sReverbSoundFlagInstance1,
      "EAX4/SFX/GameCube/Wii: Specify channel to target reverb instance 1.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_INSTANCE2", TypeS32, &sReverbSoundFlagInstance2,
      "EAX4/SFX/GameCube/Wii: Specify channel to target reverb instance 2.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
   Con::addConstant( "SFXDescription::REVERB_INSTANCE3", TypeS32, &sReverbSoundFlagInstance3,
      "EAX4/SFX/GameCube/Wii: Specify channel to target reverb instance 3.\n"
      "@see SFXDescription::flags\n\n"
      "@ingroup SFXDescription" );
}


//-----------------------------------------------------------------------------

SFXDescription::SFXDescription()
   :  SimDataBlock(),
      mVolume( 1 ),
      mPitch( 1 ),
      mIsLooping( false ),
      mIsStreaming( false ),
      mIs3D( false ),
      mUseHardware( false ),
      mUseReverb( false ),
      mMinDistance( 1 ),
      mMaxDistance( 100 ),
      mConeInsideAngle( 360 ),
      mConeOutsideAngle( 360 ),
      mConeOutsideVolume( 1 ),
      mRolloffFactor( -1.f ), // Deactivated
      mFadeInTime( 0.0f ),
      mFadeOutTime( 0.0f ),
      mFadeLoops( false ),
      mStreamPacketSize( SFXInternal::SFXAsyncStream::DEFAULT_STREAM_PACKET_LENGTH ),
      mStreamReadAhead( SFXInternal::SFXAsyncStream::DEFAULT_STREAM_LOOKAHEAD ),
      mScatterDistance( 0.f, 0.f, 0.f ),
      mPriority( 1.0f ),
      mSourceGroup( NULL )
{
   dMemset( mParameters, 0, sizeof( mParameters ) );
}

//-----------------------------------------------------------------------------

SFXDescription::SFXDescription( const SFXDescription& desc )
   :  SimDataBlock(),
      mVolume( desc.mVolume ),
      mPitch( desc.mPitch ),
      mIsLooping( desc.mIsLooping ),
      mIsStreaming( desc.mIsStreaming ),
      mIs3D( desc.mIs3D ),
      mUseHardware( desc.mUseHardware ),
      mMinDistance( desc.mMinDistance ),
      mMaxDistance( desc.mMaxDistance ),
      mConeInsideAngle( desc.mConeInsideAngle ),
      mConeOutsideAngle( desc.mConeOutsideAngle ),
      mConeOutsideVolume( desc.mConeOutsideVolume ),
      mRolloffFactor( desc.mRolloffFactor ),
      mSourceGroup( desc.mSourceGroup ),
      mFadeInTime( desc.mFadeInTime ),
      mFadeOutTime( desc.mFadeOutTime ),
      mFadeInEase( desc.mFadeInEase ),
      mFadeOutEase( desc.mFadeOutEase ),
      mFadeLoops( desc.mFadeLoops ),
      mStreamPacketSize( desc.mStreamPacketSize ),
      mUseReverb( desc.mUseReverb ),
      mStreamReadAhead( desc.mStreamReadAhead ),
      mReverb( desc.mReverb ),
      mScatterDistance( desc.mScatterDistance ),
      mPriority( desc.mPriority )
{
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      mParameters[ i ] = desc.mParameters[ i ];
}

//-----------------------------------------------------------------------------

SFXDescription::SFXDescription(const SFXDescription& other, bool temp_clone)
   : SimDataBlock(other, temp_clone),
      mVolume( other.mVolume ),
      mPitch( other.mPitch ),
      mIsLooping( other.mIsLooping ),
      mIsStreaming( other.mIsStreaming ),
      mIs3D( other.mIs3D ),
      mUseHardware( other.mUseHardware ),
      mMinDistance( other.mMinDistance ),
      mMaxDistance( other.mMaxDistance ),
      mConeInsideAngle( other.mConeInsideAngle ),
      mConeOutsideAngle( other.mConeOutsideAngle ),
      mConeOutsideVolume( other.mConeOutsideVolume ),
      mRolloffFactor( other.mRolloffFactor ),
      mSourceGroup( other.mSourceGroup ),
      mFadeInTime( other.mFadeInTime ),
      mFadeOutTime( other.mFadeOutTime ),
      mFadeInEase( other.mFadeInEase ),
      mFadeOutEase( other.mFadeOutEase ),
      mFadeLoops( other.mFadeLoops ),
      mStreamPacketSize( other.mStreamPacketSize ),
      mStreamReadAhead( other.mStreamReadAhead ),
      mUseReverb( other.mUseReverb ),
      mReverb( other.mReverb ),
      mPriority( other.mPriority ),
      mScatterDistance( other.mScatterDistance )
{
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      mParameters[ i ] = other.mParameters[ i ];
}

void SFXDescription::initPersistFields()
{
   addGroup( "Playback" );
   
      addField( "sourceGroup",         TypeSFXSourceName, Offset( mSourceGroup, SFXDescription ),
         "Group that sources playing with this description should be put into.\n\n"
         "When a sound source is allocated, it will be made a child of the source group that is listed in its \n"
         "description.  This group will then modulate several properties of the sound as it is played.\n\n"
         "For example, one use of groups is to segregate sounds so that volume levels of different sound "
         "groups such as interface audio and game audio can be controlled independently.\n\n"
         "@ref SFXSource_hierarchies" );
      addField( "volume",              TypeF32, Offset( mVolume, SFXDescription ),
         "Base volume level for the sound.\n\n"
         "This will be the starting point for volume attenuation on the sound.  The final effective volume of "
         "a sound will be dependent on a number of parameters.\n\n"
         "Must be between 0 (mute) and 1 (full volume).  Default is 1.\n\n"
         "@ref SFXSource_volume" );
      addField( "pitch",               TypeF32, Offset( mPitch, SFXDescription ),
         "Pitch shift to apply to playback.\n\n"
         "The pitch assigned to a sound determines the speed at which it is played back.  A pitch shift of 1 plays the "
         "sound at its default speed.  A greater shift factor speeds up playback and a smaller shift factor slows it down.\n\n"
         "Must be >0.  Default is 1." );
      addField( "isLooping",           TypeBool, Offset( mIsLooping, SFXDescription ),
         "If true, the sound will be played in an endless loop.\n\n"
         "Default is false." );
      addField( "priority",            TypeF32, Offset( mPriority, SFXDescription ),
         "Priority level for virtualization of sounds (1 = base level).\n"
         "When there are more concurrently active sounds than supported by the audio mixer, some of the sounds "
         "need to be culled.  Which sounds are culled first depends primarily on total audibility of individual sounds. "
         "However, the priority of invidual sounds may be decreased or decreased through this field.\n\n"
         "@ref SFXSound_virtualization" );
      addField( "useHardware",         TypeBool, Offset( mUseHardware, SFXDescription ),
         "Whether the sound is allowed to be mixed in hardware.\n"
         "If true, the sound system will try to allocate the voice for the sound directly "
         "on the sound hardware for mixing by the hardware mixer.  Be aware that a hardware mixer "
         "may not provide all features available to sounds mixed in software.\n\n"
         "@note This flag currently only takes effect when using FMOD.\n\n"
         "@note Generally, it is preferable to let sounds be mixed in software.\n\n" );
      addField( "parameters",          TypeSFXParameterName, Offset( mParameters, SFXDescription ), MaxNumParameters,
         "Names of the parameters to which sources using this description will automatically be linked.\n\n"
         "Individual parameters are identified by their #internalName.\n\n"
         "@ref SFX_interactive" );
   
   endGroup( "Playback" );
   
   addGroup( "Fading" );
   
      addField( "fadeInTime",          TypeF32,    Offset( mFadeInTime, SFXDescription ),
         "Number of seconds to gradually fade in volume from zero when playback starts.\n"
         "Must be >= 0.\n\n"
         "@ref SFXSource_fades" );
      addField( "fadeOutTime",         TypeF32,    Offset( mFadeOutTime, SFXDescription ),
         "Number of seconds to gradually fade out volume down to zero when playback is stopped or paused.\n"
         "Must be >=0.\n\n"
         "@ref SFXSource_fades" );
      addField( "fadeInEase",          TypeEaseF,  Offset( mFadeInEase, SFXDescription ),
         "Easing curve for fade-in transition.\n"
         "Volume fade-ins will interpolate volume along this curve.\n\n"
         "@ref SFXSource_fades" );
      addField( "fadeOutEase",         TypeEaseF,  Offset( mFadeOutEase, SFXDescription ),
         "Easing curve for fade-out transition.\n"
         "Volume fade-outs will interpolate volume along this curve.\n\n"
         "@ref SFXSource_fades" );
      addField( "fadeLoops",           TypeBool,   Offset( mFadeLoops, SFXDescription ),
         "Fade each cycle of a loop in and/or out; otherwise only fade-in first cycle.\n"
         "By default, volume fading is applied to the beginning and end of the playback range, i.e. a fade-in segment "
         "is placed at the beginning of the sound and a fade-out segment is paced at the end of a sound.  However, "
         "when looping playback, this may be undesirable as each iteration of the sound will then have a fade-in and "
         "fade-out effect.\n\n"
         "To set up looping sounds such that a fade-in is applied only when the sound is first started (or playback resumed) "
         "and a fade-out is only applied when the sound is explicitly paused or stopped, set this field to true.\n\n"
         "Default is false.\n\n"
         "@ref SFXSource_fades" );
      
   endGroup( "Fading" );

   addGroup( "3D" );
   
      addField( "is3D",                TypeBool,   Offset( mIs3D, SFXDescription ),
         "If true, sounds played with this description will have a position and orientation in space.\n"
         "Unlike a non-positional sound, a 3D sound will have its volume attenuated depending on the distance to the "
         "listener in space.  The farther the sound moves away from the listener, the less audible it will be.\n\n"
         "Non-positional sounds, in contrast, will remain at their original volume regardless of where the listener is.\n\n"
         "@note Whether a sound is positional or non-positional cannot be changed once the sound was created so this field "
            "determines up front which is the case for a given sound.\n\n"
         "@ref SFX_3d\n"
         "@ref SFXSource_volume" );
      addField( "referenceDistance",   TypeF32,    Offset( mMinDistance, SFXDescription ),
         "Distance at which volume attenuation begins.\n"
         "Up to this distance, the sound retains its base volume.\n\n"
         "In the linear distance model, the volume will linearly from this distance onwards up to maxDistance where it "
         "reaches zero.\n\n"
         "In the logarithmic distance model, the reference distance determine how fast the sound volume decreases "
         "with distance.  Each referenceDistance steps (scaled by the rolloff factor), the volume halves.\n\n"
         "A rule of thumb is that for sounds that require you to be close to hear them in the real world, set the reference "
         "distance to small values whereas for sounds that are widely audible set it to larger values.\n\n"
         "Only applies to 3D sounds.\n"
         "@see LevelInfo::soundDistanceModel\n\n"
         "@ref SFX_3d\n"
         "@ref SFXSource_volume" );
      addField( "maxDistance",         TypeF32,    Offset( mMaxDistance, SFXDescription ),
         "The distance at which attenuation stops.\n"
         "In the linear distance model, the attenuated volume will be zero at this distance.\n\n"
         "In the logarithmic model, attenuation will simply stop at this distance and the sound will keep its attenuated "
         "volume from there on out.  As such, it primarily functions as a cutoff factor to exponential distance attentuation "
         "to limit the number of voices relevant to updates.\n\n"
         "Only applies to 3D sounds.\n"
         "@see LevelInfo::soundDistanceModel\n\n"
         "@ref SFX_3d\n"
         "@ref SFXSource_volume" );
      addField( "scatterDistance",     TypePoint3F,Offset( mScatterDistance, SFXDescription ),
         "Bounds on random displacement of 3D sound positions.\n"
         "When a 3D sound is created and given its initial position in space, this field is used to determine "
         "the amount of randomization applied to the actual position given to the sound system.\n\n"
         "The randomization uses the following scheme:"
         "@verbatim\n"
         "x += rand( - scatterDistance[ 0 ], scatterDistance[ 0 ] );\n"
         "y += rand( - scatterDistance[ 1 ], scatterDistance[ 1 ] );\n"
         "z += rand( - scatterDistance[ 2 ], scatterDistance[ 2 ] );\n"
         "@endverbatim\n" );
      addField( "coneInsideAngle",     TypeS32,    Offset( mConeInsideAngle, SFXDescription ),
         "Inner sound cone angle in degrees.\n"
         "This value determines the angle of the inner volume cone that protrudes out in the direction "
         "of a sound.  Within this cone, the sound source retains full volume that is unaffected by sound cone "
         "settings (though still affected by distance attenuation.)\n\n"
         "Valid values are from 0 to 360. Must be less than coneOutsideAngle. Default is 360. Only for 3D sounds.\n\n"
         "@ref SFXSource_cones" );
      addField( "coneOutsideAngle",    TypeS32,    Offset( mConeOutsideAngle, SFXDescription ),
         "Outer sound cone angle in degrees.\n"
         "This value determines the angle of the outer volume cone that protrudes out in the direction of a sound "
         "and surrounds the inner volume cone.  Within this cone, volume will linearly interpolate from the outer cone "
         "hull inwards to the inner coner hull starting with the base volume scaled by coneOutsideVolume and ramping "
         "up/down to the full base volume.\n\n"
         "Valid values are from 0 to 360.  Must be >= coneInsideAngle.  Default is 360.  Only for 3D sounds.\n\n"
         "@ref SFXSource_cones" );
      addField( "coneOutsideVolume",   TypeF32,    Offset( mConeOutsideVolume, SFXDescription ),
         "Determines the volume scale factor applied the a source's base volume level outside of the outer cone.\n"
         "In the outer cone, starting from outside the inner cone, the scale factor smoothly interpolates from 1.0 (within the inner cone) "
         "to this value.  At the moment, the allowed range is 0.0 (silence) to 1.0 (no attenuation) as amplification is only supported on "
         "XAudio2 but not on the other devices.\n\n"
         "Only for 3D sound.\n\n"
         "@ref SFXSource_cones" );
      addField( "rolloffFactor",       TypeF32,    Offset( mRolloffFactor, SFXDescription ),
         "Scale factor to apply to logarithmic distance attenuation curve.  If -1, the global rolloff setting is used.\n\n"
         "@note Per-sound rolloff is only supported on OpenAL and FMOD at the moment.  With other divices, the global rolloff setting "
            "is used for all sounds.\n"
         "@see LevelInfo::soundDistanceModel" );
      
   endGroup( "3D" );

   addGroup( "Streaming" );
   
      addField( "isStreaming",         TypeBool,   Offset( mIsStreaming, SFXDescription ),
         "If true, incrementally stream sounds; otherwise sounds are loaded in full.\n\n"
         "@ref SFX_streaming" );
      addField( "streamPacketSize",    TypeS32,    Offset( mStreamPacketSize, SFXDescription ),
         "Number of seconds of sample data per single streaming packet.\n"
         "This field allows to fine-tune streaming for individual sounds.  The streaming system "
         "processes streamed sounds in batches called packets.  Each packet will contain a set amount "
         "of sample data determined by this field.  The greater its value, the more sample data each "
         "packet contains, the more work is done per packet.\n\n"
         "@note This field only takes effect when Torque's own sound system performs the streaming. "
            "When FMOD is used, this field is ignored and streaming is performed by FMOD.\n\n"
         "@ref SFX_streaming" );
      addField( "streamReadAhead",     TypeS32,    Offset( mStreamReadAhead, SFXDescription ),
         "Number of sample packets to read and buffer in advance.\n"
         "This field determines the number of packets that the streaming system will try to keep buffered "
         "in advance.  As such it determines the number of packets that can be consumed by the sound "
         "device before the playback queue is running dry.  Greater values thus allow for more lag "
         "in the streaming pipeline.\n\n"
         "@note This field only takes effect when Torque's own sound system performs the streaming. "
            "When FMOD is used, this field is ignored and streaming is performed by FMOD.\n\n"
         "@ref SFX_streaming" );
         
   endGroup( "Streaming" );

   addGroup( "Reverb" );
   
   addField("useCustomReverb", TypeBool, Offset(mUseReverb, SFXDescription),
      "If true, use the reverb properties defined here on sounds.\n"
      "By default, sounds will be assigned a generic reverb profile.  By setting this flag to true, "
      "a custom reverb setup can be defined using the \"Reverb\" properties that will then be assigned "
      "to sounds playing with the description.\n\n"
      "@ref SFX_reverb");
   addField("reverbDensity", TypeF32, Offset(mReverb.flDensity, SFXDescription),
      "Density of reverb environment.");
   addField("reverbDiffusion", TypeF32, Offset(mReverb.flDiffusion, SFXDescription),
      "Environment diffusion.");
   addField("reverbGain", TypeF32, Offset(mReverb.flGain, SFXDescription),
      "Reverb Gain Level.");
   addField("reverbGainHF", TypeF32, Offset(mReverb.flGainHF, SFXDescription),
      "Reverb Gain to high frequencies");
   addField("reverbGainLF", TypeF32, Offset(mReverb.flGainLF, SFXDescription),
      "Reverb Gain to high frequencies");
   addField("reverbDecayTime", TypeF32, Offset(mReverb.flDecayTime, SFXDescription),
      "Decay time for the reverb.");
   addField("reverbDecayHFRatio", TypeF32, Offset(mReverb.flDecayHFRatio, SFXDescription),
      "High frequency decay time ratio.");
   addField("reverbDecayLFRatio", TypeF32, Offset(mReverb.flDecayLFRatio, SFXDescription),
      "High frequency decay time ratio.");
   addField("reflectionsGain", TypeF32, Offset(mReverb.flReflectionsGain, SFXDescription),
      "Reflection Gain.");
   addField("reflectionDelay", TypeF32, Offset(mReverb.flReflectionsDelay, SFXDescription),
      "How long to delay reflections.");
   addField("lateReverbGain", TypeF32, Offset(mReverb.flLateReverbGain, SFXDescription),
      "Late reverb gain amount.");
   addField("lateReverbDelay", TypeF32, Offset(mReverb.flLateReverbDelay, SFXDescription),
      "Late reverb delay time.");
   addField("reverbEchoTime", TypeF32, Offset(mReverb.flEchoTime, SFXDescription),
      "Reverb echo time.");
   addField("reverbEchoDepth", TypeF32, Offset(mReverb.flEchoDepth, SFXDescription),
      "Reverb echo depth.");
   addField("reverbModTime", TypeF32, Offset(mReverb.flModulationTime, SFXDescription),
      "Reverb Modulation time.");
   addField("reverbModTime", TypeF32, Offset(mReverb.flModulationDepth, SFXDescription),
      "Reverb Modulation time.");
   addField("airAbsorbtionGainHF", TypeF32, Offset(mReverb.flAirAbsorptionGainHF, SFXDescription),
      "High Frequency air absorbtion");
   addField("reverbHFRef", TypeF32, Offset(mReverb.flHFReference, SFXDescription),
      "Reverb High Frequency Reference.");
   addField("reverbLFRef", TypeF32, Offset(mReverb.flLFReference, SFXDescription),
      "Reverb Low Frequency Reference.");
   addField("roomRolloffFactor", TypeF32, Offset(mReverb.flRoomRolloffFactor, SFXDescription),
      "Rolloff factor for reverb.");
   addField("decayHFLimit", TypeS32, Offset(mReverb.iDecayHFLimit, SFXDescription),
      "High Frequency decay limit.");
   endGroup("Reverb");
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXDescription::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
      
   Sim::getSFXDescriptionSet()->addObject( this );
   
   // Convert a legacy 'channel' field, if we have one.
   
   static const char* sChannel = StringTable->insert( "channel" );
   const char* channelValue = getDataField( sChannel, NULL );
   if( channelValue && channelValue[ 0 ] )
   {
      const char* group = Con::evaluatef( "return sfxOldChannelToGroup( %s );", channelValue );
      if( !Sim::findObject( group, mSourceGroup ) )
         Con::errorf( "SFXDescription::onAdd - could not resolve channel '%s' to SFXSource", channelValue );
   }
   
   // Validate the data we'll be passing to 
   // the audio layer.
   validate();

   return true;
}

//-----------------------------------------------------------------------------

void SFXDescription::validate()
{
   // Validate the data we'll be passing to the audio layer.
   mVolume = mClampF( mVolume, 0, 1 );
   
   if( mPitch <= 0.0f )
      mPitch = 1.0f;
   if( mFadeInTime < 0.0f )
      mFadeInTime = 0.0f;
   if( mFadeOutTime < 0.0f )
      mFadeOutTime = 0.0f;
   if( mRolloffFactor < 0.f )
      mRolloffFactor = -1.f;

   if( mMinDistance < 0.f )
      mMinDistance = 0.f;
   if( mMaxDistance <= mMinDistance )
      mMaxDistance = mMinDistance + 0.01f;

   mConeInsideAngle     = mClamp( mConeInsideAngle, 0, 360 );
   mConeOutsideAngle    = mClamp( mConeOutsideAngle, mConeInsideAngle, 360 );
   mConeOutsideVolume   = mClampF( mConeOutsideVolume, 0, 1 );
   
   if( !mIs3D )
      mUseReverb = false;
   
   mReverb.validate();
}

//-----------------------------------------------------------------------------

void SFXDescription::packData( BitStream *stream )
{
   Parent::packData( stream );

   stream->writeFloat( mVolume, 6 );
   stream->write( mPitch );
   stream->write( mPriority );

   stream->writeFlag( mIsLooping );
   stream->writeFlag( mFadeLoops );

   stream->writeFlag( mIsStreaming );
   stream->writeFlag( mIs3D );
   stream->writeFlag( mUseReverb );
   stream->writeFlag( mUseHardware );
   
   sfxWrite( stream, mSourceGroup );

   if( mIs3D )
   {
      stream->write( mMinDistance );
      stream->write( mMaxDistance );
      stream->write( mRolloffFactor );
      mathWrite( *stream, mScatterDistance );

      stream->writeInt( mConeInsideAngle, 9 );
      stream->writeInt( mConeOutsideAngle, 9 );

      stream->writeFloat( mConeOutsideVolume, 6 );
      
      if( mUseReverb )
      {
         stream->write(mReverb.flDensity);
         stream->write(mReverb.flDiffusion);
         stream->write(mReverb.flGain);
         stream->write(mReverb.flGainHF);
         stream->write(mReverb.flGainLF);
         stream->write(mReverb.flDecayTime);
         stream->write(mReverb.flDecayHFRatio);
         stream->write(mReverb.flDecayLFRatio);
         stream->write(mReverb.flReflectionsGain);
         stream->write(mReverb.flReflectionsDelay);
         stream->write(mReverb.flLateReverbGain);
         stream->write(mReverb.flLateReverbDelay);
         stream->write(mReverb.flEchoTime);
         stream->write(mReverb.flEchoDepth);
         stream->write(mReverb.flModulationTime);
         stream->write(mReverb.flModulationDepth);
         stream->write(mReverb.flAirAbsorptionGainHF);
         stream->write(mReverb.flHFReference);
         stream->write(mReverb.flLFReference);
         stream->write(mReverb.flRoomRolloffFactor);
         stream->write(mReverb.iDecayHFLimit);
      }
   }

   stream->write( mFadeInTime );
   stream->write( mFadeOutTime );
   stream->writeInt( mStreamPacketSize, 8 );
   stream->writeInt( mStreamReadAhead, 8 );
   
   mathWrite( *stream, mFadeInEase );
   mathWrite( *stream, mFadeOutEase );
   
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->writeFlag( mParameters[ i ] ) )
         stream->writeString( mParameters[ i ] );
}

//-----------------------------------------------------------------------------

void SFXDescription::unpackData( BitStream *stream )
{
   Parent::unpackData( stream );

   mVolume        = stream->readFloat( 6 );
   stream->read( &mPitch );
   stream->read( &mPriority );

   mIsLooping     = stream->readFlag();
   mFadeLoops     = stream->readFlag();

   mIsStreaming   = stream->readFlag();
   mIs3D          = stream->readFlag();
   mUseReverb     = stream->readFlag();
   mUseHardware   = stream->readFlag();
   
   String errorStr;
   if( !sfxReadAndResolve( stream, &mSourceGroup, errorStr ) )
      Con::errorf( "SFXDescription::unpackData: %s", errorStr.c_str() );

   if( mIs3D )
   {
      stream->read( &mMinDistance );
      stream->read( &mMaxDistance );
      stream->read( &mRolloffFactor );
      mathRead( *stream, &mScatterDistance );

      mConeInsideAngle     = stream->readInt( 9 );
      mConeOutsideAngle    = stream->readInt( 9 );

      mConeOutsideVolume   = stream->readFloat( 6 );
      
      if( mUseReverb )
      {
         stream->read(&mReverb.flDensity);
         stream->read(&mReverb.flDiffusion);
         stream->read(&mReverb.flGain);
         stream->read(&mReverb.flGainHF);
         stream->read(&mReverb.flGainLF);
         stream->read(&mReverb.flDecayTime);
         stream->read(&mReverb.flDecayHFRatio);
         stream->read(&mReverb.flDecayLFRatio);
         stream->read(&mReverb.flReflectionsGain);
         stream->read(&mReverb.flReflectionsDelay);
         stream->read(&mReverb.flLateReverbGain);
         stream->read(&mReverb.flLateReverbDelay);
         stream->read(&mReverb.flEchoTime);
         stream->read(&mReverb.flEchoDepth);
         stream->read(&mReverb.flModulationTime);
         stream->read(&mReverb.flModulationDepth);
         stream->read(&mReverb.flAirAbsorptionGainHF);
         stream->read(&mReverb.flHFReference);
         stream->read(&mReverb.flLFReference);
         stream->read(&mReverb.flRoomRolloffFactor);
         stream->read(&mReverb.iDecayHFLimit);
      }
   }

   stream->read( &mFadeInTime );
   stream->read( &mFadeOutTime );
   mStreamPacketSize = stream->readInt( 8 );
   mStreamReadAhead = stream->readInt( 8 );
   
   mathRead( *stream, &mFadeInEase );
   mathRead( *stream, &mFadeOutEase );
   
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->readFlag() )
         mParameters[ i ] = stream->readSTString();
      else
         mParameters[ i ] = NULL;
}

//-----------------------------------------------------------------------------

void SFXDescription::inspectPostApply()
{
   Parent::inspectPostApply();
   
   validate();
   
   if( SFX )
      SFX->notifyDescriptionChanged( this );
}
// This allows legacy AudioDescription datablocks to be recognized as an alias
// for SFXDescription. It is intended to ease the transition from older scripts
// especially those that still need to support pre-1.7 applications.
// (This maybe removed in future releases so treat as deprecated.)
class AudioDescription : public SFXDescription
{
	typedef SFXDescription Parent;
public:
	DECLARE_CONOBJECT(AudioDescription);
};

IMPLEMENT_CO_DATABLOCK_V1(AudioDescription);

ConsoleDocClass( AudioDescription,
   "@brief Allows legacy AudioDescription datablocks to be treated as SFXDescription datablocks.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

