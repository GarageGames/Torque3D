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

#include "sfx/sfxEnvironment.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "core/module.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXEnvironment );


// Reverb flags.
static const U32 sReverbFlagDecayTimeScale         = 0x001;
static const U32 sReverbFlagReflectionsScale       = 0x002;
//static const U32 sReverbFlagReflectionsDelayScale  = 0x004; unused, but kept for doc purposes -BJR
static const U32 sReverbFlagReverbScale            = 0x008;
static const U32 sReverbFlagReverbDelayScale       = 0x010;
static const U32 sReverbFlagDecayHFLimit           = 0x020;
static const U32 sReverbFlagEchoTimeScale          = 0x040;
static const U32 sReverbFlagModulationTimeScale    = 0x080;
static const U32 sReverbFlagCore0                  = 0x100;
static const U32 sReverbFlagCore1                  = 0x200;
static const U32 sReverbFlagHighQualityReverb      = 0x400;
static const U32 sReverbFlagHighQualityDPL2Reverb  = 0x800;

AFTER_MODULE_INIT( SFX )
{
   Con::addConstant( "SFXEnvironment::REVERB_DECAYTIMESCALE", TypeS32, &sReverbFlagDecayTimeScale,
      "SFXEnvironment::envSize affects reverberation decay time.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_REFLECTIONSSCALE", TypeS32, &sReverbFlagReflectionsScale,
      "SFXEnvironment::envSize affects reflection level.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_REFLECTIONSDELAYSCALE", TypeS32, &sReverbFlagReflectionsScale,
      "SFXEnvironment::envSize affects initial reflection delay time.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_REVERBSCALE", TypeS32, &sReverbFlagReverbScale,
      "SFXEnvironment::envSize affects reflections level.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_REVERBDELAYSCALE", TypeS32, &sReverbFlagReverbDelayScale,
      "SFXEnvironment::envSize affects late reverberation delay time.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_DECAYHFLIMIT", TypeS32, &sReverbFlagDecayHFLimit,
      "SFXEnvironment::airAbsorptionHF affects SFXEnvironment::decayHFRatio.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_ECHOTIMESCALE", TypeS32, &sReverbFlagEchoTimeScale,
      "SFXEnvironment::envSize affects echo time.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_MODULATIONTIMESCALE", TypeS32, &sReverbFlagModulationTimeScale,
      "SFXEnvironment::envSize affects modulation time.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_CORE0", TypeS32, &sReverbFlagCore0,
      "PS2 Only - Reverb is applied to CORE0 (hw voices 0-23).\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_CORE1", TypeS32, &sReverbFlagCore1,
      "PS2 Only - Reverb is applied to CORE1 (hw voices 24-47).\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_HIGHQUALITYREVERB", TypeS32, &sReverbFlagHighQualityReverb,
      "GameCube/Wii Only - Use high-quality reverb.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
   Con::addConstant( "SFXEnvironment::REVERB_HIGHQUALITYDPL2REVERB", TypeS32, &sReverbFlagHighQualityDPL2Reverb,
      "GameCube/Wii Only - Use high-quality DPL2 reverb.\n"
      "@see SFXEnvironment::flags\n\n"
      "@ingroup SFXEnvironment" );
}


ConsoleDocClass( SFXEnvironment,
   "@brief Description of a reverb environment.\n\n"
   
   "A reverb environment specifies how the audio mixer should render advanced environmental audio "
   "effects.  \n\n"
   
   "To use reverb environments in your level, set up one or more ambient audio spaces, assign "
   "reverb environments appropriately, and then attach the SFXAmbiences to your LevelInfo (taking effect "
   "globally) or Zone objects (taking effect locally).\n\n"
   
   "To define your own custom reverb environments, it is usually easiest to adapt one of the pre-existing "
   "reverb definitions:\n"
   
   "@tsexample_nopar\n"
   "singleton SFXEnvironment( AudioEnvCustomUnderwater : AudioEnvUnderwater )\n"
   "{\n"
   "   // Override select properties from AudioEnvUnderwater here.\n"
   "};\n"
   "@endtsexample\n\n"
   
   "In the Datablock Editor, this can be done by selecting an existing environment to copy from when creating "
   "the SFXEnvironment datablock.\n\n"
   
   "For a precise description of reverb audio and the properties of this class, please consult the EAX "
   "documentation.\n\n"
   
   "All SFXEnvironment instances are automatically added to the global @c SFXEnvironmentSet.\n\n"
   
   "@see http://www.atc.creative.com/algorithms/eax20.pdf\n"
   "@see http://connect.creativelabs.com/developer/Gaming/Forms/AllItems.aspx\n"
   "@see SFXAmbience::environment\n\n"
   "@ref SFX_reverb\n"
   "@ingroup SFX\n"
);


//-----------------------------------------------------------------------------

SFXEnvironment::SFXEnvironment()
{
}

//-----------------------------------------------------------------------------

void SFXEnvironment::initPersistFields()
{
   addGroup( "Reverb" );
   
      addField( "envSize",             TypeF32,    Offset( mReverb.mEnvSize, SFXEnvironment ),
         "Environment size in meters." );
      addField( "envDiffusion",        TypeF32,    Offset( mReverb.mEnvDiffusion, SFXEnvironment ),
         "Environment diffusion." );
      addField( "room",                TypeS32,    Offset( mReverb.mRoom, SFXEnvironment ),
         "Room effect level at mid-frequencies." );
      addField( "roomHF",              TypeS32,    Offset( mReverb.mRoomHF, SFXEnvironment ),
         "Relative room effect level at high frequencies." );
      addField( "roomLF",              TypeS32,    Offset( mReverb.mRoomLF, SFXEnvironment ),
         "Relative room effect level at low frequencies." );
      addField( "decayTime",           TypeF32,    Offset( mReverb.mDecayTime, SFXEnvironment ),
         "Reverberation decay time at mid frequencies." );
      addField( "decayHFRatio",        TypeF32,    Offset( mReverb.mDecayHFRatio, SFXEnvironment ),
         "High-frequency to mid-frequency decay time ratio." );
      addField( "decayLFRatio",        TypeF32,    Offset( mReverb.mDecayLFRatio, SFXEnvironment ),
         "Low-frequency to mid-frequency decay time ratio." );
      addField( "reflections",         TypeS32,    Offset( mReverb.mReflections, SFXEnvironment ),
         "Early reflections level relative to room effect." );
      addField( "reflectionsDelay",    TypeF32,    Offset( mReverb.mReflectionsDelay, SFXEnvironment ),
         "Initial reflection delay time." );
      addField( "reflectionsPan",      TypeF32,    Offset( mReverb.mReflectionsPan, SFXEnvironment ), 3,
         "Early reflections panning vector." );
      addField( "reverb",              TypeS32,    Offset( mReverb.mReverb, SFXEnvironment ),
         "Late reverberation level relative to room effect." );
      addField( "reverbDelay",         TypeF32,    Offset( mReverb.mReverbDelay, SFXEnvironment ),
         "Late reverberation delay time relative to initial reflection." );
      addField( "reverbPan",           TypeF32,    Offset( mReverb.mReverbPan, SFXEnvironment ), 3,
         "Late reverberation panning vector." );
      addField( "echoTime",            TypeF32,    Offset( mReverb.mEchoTime, SFXEnvironment ),
         "Echo time." );
      addField( "echoDepth",           TypeF32,    Offset( mReverb.mEchoDepth, SFXEnvironment ),
         "Echo depth." );
      addField( "modulationTime",      TypeF32,    Offset( mReverb.mModulationTime, SFXEnvironment ),
         "Modulation time." );
      addField( "modulationDepth",     TypeF32,    Offset( mReverb.mModulationDepth, SFXEnvironment ),
         "Modulation depth." );
      addField( "airAbsorptionHF",     TypeF32,    Offset( mReverb.mAirAbsorptionHF, SFXEnvironment ),
         "Change in level per meter at high frequencies." );
      addField( "HFReference",         TypeF32,    Offset( mReverb.mHFReference, SFXEnvironment ),
         "Reference high frequency in Hertz." );
      addField( "LFReference",         TypeF32,    Offset( mReverb.mLFReference, SFXEnvironment ),
         "Reference low frequency in Hertz." );
      addField( "roomRolloffFactor",   TypeF32,    Offset( mReverb.mRoomRolloffFactor, SFXEnvironment ),
         "Logarithmic distance attenuation rolloff scale factor for reverb room size effect." );
      addField( "diffusion",           TypeF32,    Offset( mReverb.mDiffusion, SFXEnvironment ),
         "Value that controls the echo density in the late reverberation decay." );
      addField( "density",             TypeF32,    Offset( mReverb.mDensity, SFXEnvironment ),
         "Value that controls the modal density in the late reverberation decay." );
      addField( "flags",               TypeS32,    Offset( mReverb.mFlags, SFXEnvironment ),
         "A bitfield of reverb flags.\n"
         "@see REVERB_DECAYTIMESCALE\n"
         "@see REVERB_REFLECTIONSSCALE\n"
         "@see REVERB_REFLECTIONSDELAYSCALE\n"
         "@see REVERB_REVERBSCALE\n"
         "@see REVERB_REVERBDELAYSCALE\n"
         "@see REVERB_DECAYHFLIMIT\n"
         "@see REVERB_ECHOTIMESCALE\n"
         "@see REVERB_MODULATIONTIMESCALE\n"
         "@see REVERB_CORE0\n"
         "@see REVERB_CORE1\n"
         "@see REVERB_HIGHQUALITYREVERB\n"
         "@see REVERB_HIGHQUALITYDPL2REVERB\n" );

   endGroup( "Reverb" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXEnvironment::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   Sim::getSFXEnvironmentSet()->addObject( this );
      
   return true;
}

//-----------------------------------------------------------------------------

bool SFXEnvironment::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   validate();
   
   return true;
}

//-----------------------------------------------------------------------------

void SFXEnvironment::inspectPostApply()
{
   Parent::inspectPostApply();
   validate();
}

//-----------------------------------------------------------------------------

void SFXEnvironment::validate()
{
   mReverb.validate();
}

//-----------------------------------------------------------------------------

void SFXEnvironment::packData( BitStream* stream )
{
   Parent::packData( stream );

   stream->write( mReverb.mEnvSize );
   stream->write( mReverb.mEnvDiffusion );
   stream->write( mReverb.mRoom );
   stream->write( mReverb.mRoomHF );
   stream->write( mReverb.mRoomLF );
   stream->write( mReverb.mDecayTime );
   stream->write( mReverb.mDecayHFRatio );
   stream->write( mReverb.mDecayLFRatio );
   stream->write( mReverb.mReflections );
   stream->write( mReverb.mReflectionsDelay );
   stream->write( mReverb.mReflectionsPan[ 0 ] );
   stream->write( mReverb.mReflectionsPan[ 1 ] );
   stream->write( mReverb.mReflectionsPan[ 2 ] );
   stream->write( mReverb.mReverb );
   stream->write( mReverb.mReverbDelay );
   stream->write( mReverb.mReverbPan[ 0 ] );
   stream->write( mReverb.mReverbPan[ 1 ] );
   stream->write( mReverb.mReverbPan[ 2 ] );
   stream->write( mReverb.mEchoTime );
   stream->write( mReverb.mEchoDepth );
   stream->write( mReverb.mModulationTime );
   stream->write( mReverb.mModulationDepth );
   stream->write( mReverb.mAirAbsorptionHF );
   stream->write( mReverb.mHFReference );
   stream->write( mReverb.mLFReference );
   stream->write( mReverb.mRoomRolloffFactor );
   stream->write( mReverb.mDiffusion );
   stream->write( mReverb.mDensity );
   stream->write( mReverb.mFlags );
}

//-----------------------------------------------------------------------------

void SFXEnvironment::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   stream->read( &mReverb.mEnvSize );
   stream->read( &mReverb.mEnvDiffusion );
   stream->read( &mReverb.mRoom );
   stream->read( &mReverb.mRoomHF );
   stream->read( &mReverb.mRoomLF );
   stream->read( &mReverb.mDecayTime );
   stream->read( &mReverb.mDecayHFRatio );
   stream->read( &mReverb.mDecayLFRatio );
   stream->read( &mReverb.mReflections );
   stream->read( &mReverb.mReflectionsDelay );
   stream->read( &mReverb.mReflectionsPan[ 0 ] );
   stream->read( &mReverb.mReflectionsPan[ 1 ] );
   stream->read( &mReverb.mReflectionsPan[ 2 ] );
   stream->read( &mReverb.mReverb );
   stream->read( &mReverb.mReverbDelay );
   stream->read( &mReverb.mReverbPan[ 0 ] );
   stream->read( &mReverb.mReverbPan[ 1 ] );
   stream->read( &mReverb.mReverbPan[ 2 ] );
   stream->read( &mReverb.mEchoTime );
   stream->read( &mReverb.mEchoDepth );
   stream->read( &mReverb.mModulationTime );
   stream->read( &mReverb.mModulationDepth );
   stream->read( &mReverb.mAirAbsorptionHF );
   stream->read( &mReverb.mHFReference );
   stream->read( &mReverb.mLFReference );
   stream->read( &mReverb.mRoomRolloffFactor );
   stream->read( &mReverb.mDiffusion );
   stream->read( &mReverb.mDensity );
   stream->read( &mReverb.mFlags );
}
