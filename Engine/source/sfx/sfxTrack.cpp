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

#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxSystem.h"
#include "core/stream/bitStream.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXTrack );


ConsoleDocClass( SFXTrack,
   "@brief Abstract base class for sound data that can be played back by the sound system.\n\n"
   
   "The term \"track\" is used in the sound system to refer to any entity that can be played "
   "back as a sound source.  These can be individual files (SFXProfile), patterns of other tracks "
   "(SFXPlayList), or special sound data defined by a device layer (SFXFMODEvent).\n\n"
   
   "Any track must be paired with a SFXDescription that tells the sound system how to set up "
   "playback for the track.\n\n"
   
   "All objects that are of type SFXTrack will automatically be added to @c SFXTrackSet.\n\n"

   "@note This class cannot be instantiated directly.\n\n"
      
   "@ingroup SFX\n"
   "@ingroup Datablocks\n"
);


//-----------------------------------------------------------------------------

SFXTrack::SFXTrack()
   : mDescription( NULL )
{
   dMemset( mParameters, 0, sizeof( mParameters ) );
}

//-----------------------------------------------------------------------------

SFXTrack::SFXTrack( SFXDescription* description )
   : mDescription( description )
{
   dMemset( mParameters, 0, sizeof( mParameters ) );
}

//-----------------------------------------------------------------------------

void SFXTrack::initPersistFields()
{
   addGroup( "Sound" );
   
      addField( "description",   TypeSFXDescriptionName, Offset( mDescription, SFXTrack ),
         "Playback setup description for this track.\n\n"
         "If unassigned, the description named \"AudioEffects\" will automatically be assigned to the track.  If this description "
         "is not defined, track creation will fail." );
      addField( "parameters",    TypeSFXParameterName,   Offset( mParameters, SFXTrack ), MaxNumParameters,
         "Parameters to automatically attach to SFXSources created from this track.\n"
         "Individual parameters are identified by their #internalName." );
      
   endGroup( "Sound" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXTrack::processArguments( S32 argc, ConsoleValueRef *argv )
{
   if( typeid( *this ) == typeid( SFXTrack ) )
   {
      Con::errorf( ConsoleLogEntry::Script, "SFXTrack is an abstract base class that cannot be instantiated directly!" );
      return false;
   }
   
   return Parent::processArguments( argc, argv );
}

//-----------------------------------------------------------------------------

void SFXTrack::setParameter( U32 index, const char* name )
{
   AssertFatal( index < MaxNumParameters, "SFXTrack::setParameter() - index out of range" );
   mParameters[ index ] = StringTable->insert( name );
}

//-----------------------------------------------------------------------------

void SFXTrack::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   sfxWrite( stream, mDescription );

   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->writeFlag( mParameters[ i ] ) )
         stream->writeString( mParameters[ i ] );
}

//-----------------------------------------------------------------------------

void SFXTrack::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   sfxRead( stream, &mDescription );

   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->readFlag() )
         mParameters[ i ] = stream->readSTString();
      else
         mParameters[ i ] = NULL;
}

//-----------------------------------------------------------------------------

bool SFXTrack::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   if( !server )
   {
      if( !sfxResolve( &mDescription, errorStr ) )
         return false;
   }
   
   return true;
}

//-----------------------------------------------------------------------------

bool SFXTrack::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   // If we have no SFXDescription, try to grab a default.

   if( !mDescription )
   {
      if( !Sim::findObject( "AudioEffects", mDescription ) && Sim::getSFXDescriptionSet()->size() > 0 )
         mDescription = dynamic_cast< SFXDescription* >( Sim::getSFXDescriptionSet()->at( 0 ) );
      
      if( !mDescription )
      {
         Con::errorf( 
            "SFXTrack(%s)::onAdd: The profile is missing a description!", 
            getName() );
         return false;
      }
   }
   
   Sim::getSFXTrackSet()->addObject( this );
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXTrack::inspectPostApply()
{
   Parent::inspectPostApply();
   
   if( SFX )
      SFX->notifyTrackChanged( this );
}
