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

#include "sfx/sfxAmbience.h"
#include "sfx/sfxEnvironment.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTypes.h"
#include "math/mathTypes.h"
#include "core/stream/bitStream.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXAmbience );


ConsoleDocClass( SFXAmbience,
   "@brief A datablock that describes an ambient sound space.\n\n"
   
   "Each ambience datablock captures the properties of a unique ambient sound space.  A sound space is comprised of:\n"
   
   "- an ambient audio track that is played when the listener is inside the space,\n"
   "- a reverb environment that is active inside the space, and\n"
   "- a number of SFXStates that are activated when entering the space and deactivated when exiting it.\n"
   "\n"
   
   "Each of these properties is optional.\n\n"
   
   "An important characteristic of ambient audio spaces is that their unique nature is not determined by their location "
   "in space but rather by their SFXAmbience datablock.  This means that the same SFXAmbience datablock assigned to "
   "multiple locations in a level represents the same unique audio space to the sound system.\n\n"
   
   "This is an important distinction for the ambient sound mixer which will activate a given ambient audio space only "
   "once at any one time regardless of how many intersecting audio spaces with the same SFXAmbience datablock assigned "
   "the listener may currently be in.\n\n"
   
   "All SFXAmbience instances are automatically added to the global @c SFXAmbienceSet.\n\n"

   "At the moment, transitions between reverb environments are not blended and different reverb environments from multiple "
   "active SFXAmbiences will not be blended together.  This will be added in a future version.\n\n"
   
   "@tsexample\n"
   "singleton SFXAmbience( Underwater )\n"
   "{\n"
   "   environment = AudioEnvUnderwater;\n"
   "   soundTrack = ScubaSoundList;\n"
   "   states[ 0 ] = AudioLocationUnderwater;\n"
   "};\n"
   "@endtsexample\n\n"
      
   "@see SFXEnvironment\n"
   "@see SFXTrack\n"
   "@see SFXState\n"
   "@see LevelInfo::soundAmbience\n"
   "@see Zone::soundAmbience\n\n"
   "@ref Datablock_Networking\n"
   "@ingroup SFX\n"
   "@ingroup Datablocks\n"
);


SFXAmbience::ChangeSignal SFXAmbience::smChangeSignal;


//-----------------------------------------------------------------------------

SFXAmbience::SFXAmbience()
   : mEnvironment( NULL ),
     mSoundTrack( NULL ),
     mRolloffFactor( 1.f ),
     mDopplerFactor( 0.5f )
{
   dMemset( mState, 0, sizeof( mState ) );
}

//-----------------------------------------------------------------------------

void SFXAmbience::initPersistFields()
{
   addGroup( "Sound" );
   
      addField( "environment",            TypeSFXEnvironmentName, Offset( mEnvironment, SFXAmbience ),
         "Reverb environment active in the ambience zone.\n"
         "@ref SFX_reverb" );
      addField( "soundTrack",             TypeSFXTrackName,       Offset( mSoundTrack, SFXAmbience ),
         "Sound track to play in the ambience zone." );
      addField( "rolloffFactor",          TypeF32, Offset( mRolloffFactor, SFXAmbience ),
         "The rolloff factor to apply to distance-based volume attenuation in this space.\n"
         "Defaults to 1.0.\n\n"
         "@note This applies to the logarithmic distance model only.\n\n"
         "@ref SFXSource_volume" );
      addField( "dopplerFactor",          TypeF32, Offset( mDopplerFactor, SFXAmbience ),
         "The factor to apply to the doppler affect in this space.\n"
         "Defaults to 0.5.\n\n"
         "@ref SFXSource_doppler" );
      addField( "states",                 TypeSFXStateName,       Offset( mState, SFXAmbience ),
         MaxStates,
         "States to activate when the ambient zone is entered.\n"
         "When the ambient sound state is entered, all states associated with the state will "
         "be activated (given that they are not disabled) and deactivated when the space "
         "is exited again." );
               
   endGroup( "Sound" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXAmbience::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   Sim::getSFXAmbienceSet()->addObject( this );
      
   return true;
}

//-----------------------------------------------------------------------------

bool SFXAmbience::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   validate();
      
   // Resolve datablocks on client.
   
   if( !server )
   {
      if( !sfxResolve( &mEnvironment, errorStr ) )
         return false;
         
      if( !sfxResolve( &mSoundTrack, errorStr ) )
         return false;
         
      for( U32 i = 0; i < MaxStates; ++ i )
         if( !sfxResolve( &mState[ i ], errorStr ) )
            return false;
   }
         
   return true;
}

//-----------------------------------------------------------------------------

void SFXAmbience::packData( BitStream* stream )
{
   Parent::packData( stream );
      
   sfxWrite( stream, mEnvironment );
   sfxWrite( stream, mSoundTrack );
   
   stream->write( mRolloffFactor );
   stream->write( mDopplerFactor );

   for( U32 i = 0; i < MaxStates; ++ i )
      sfxWrite( stream, mState[ i ] );
}

//-----------------------------------------------------------------------------

void SFXAmbience::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
      
   sfxRead( stream, &mEnvironment );
   sfxRead( stream, &mSoundTrack );
   
   stream->read( &mRolloffFactor );
   stream->read( &mDopplerFactor );

   for( U32 i = 0; i < MaxStates; ++ i )
      sfxRead( stream, &mState[ i ] );
}

//-----------------------------------------------------------------------------

void SFXAmbience::inspectPostApply()
{
   Parent::inspectPostApply();
   
   validate();
   
   smChangeSignal.trigger( this );
}

//-----------------------------------------------------------------------------

void SFXAmbience::validate()
{
}
