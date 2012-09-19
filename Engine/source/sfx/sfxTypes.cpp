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

#include "sfx/sfxTypes.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxEnvironment.h"
#include "sfx/sfxState.h"
#include "sfx/sfxAmbience.h"
#include "sfx/sfxSource.h"
#include "core/stringTable.h"
#include "core/stream/bitStream.h"
#include "platform/typetraits.h"


//RDTODO: find a more optimal way to transmit names rather than using full
//  strings; don't know how to facilitate NetStrings for this as we don't
//  have access to the connection


template< class T >
inline void sWrite( BitStream* stream, T* ptr )
{
   if( stream->writeFlag( ptr != NULL ) )
   {
      if( stream->writeFlag( ptr->isClientOnly() ) )
         stream->writeString( ptr->getName() );
      else
         stream->writeRangedU32( ptr->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }
}
template< class T >
inline void sRead( BitStream* stream, T** ptr )
{
   if( !stream->readFlag() )
      *ptr = NULL;
   else
   {
      if( stream->readFlag() )
      {
         StringTableEntry name = stream->readSTString();
         
         AssertFatal( !( U32( name ) & 0x1 ), "sRead - misaligned pointer" ); // StringTableEntry pointers are always word-aligned.
         
         *( ( StringTableEntry* ) ptr ) = name;
      }
      else
         *reinterpret_cast< U32* >( ptr ) =
            ( stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast ) << 1 ) | 0x1;
   }
}
template< class T >
inline bool sResolve( T** ptr, String& errorString )
{
   if( !*ptr )
      return true;
   else if( *reinterpret_cast< U32* >( ptr ) & 0x1 )
   {
      U32 id = *reinterpret_cast< U32* >( ptr ) >> 1;
      
      T* p;
      if( !Sim::findObject( id, p ) )
      {
         errorString = String::ToString( "sfxResolve - Could not resolve networked %s datalock with id '%i'",
            T::getStaticClassRep()->getClassName(), id );
         *ptr = NULL;
         return false;
      }
      
      *ptr = p;
   }
   else
   {
      StringTableEntry name = *( ( StringTableEntry* ) ptr );
         
      T* p;
      if( !Sim::findObject( name, p ) )
      {
         errorString = String::ToString( "sfxResolve - Could not resolve local %s datablock with name '%s'",
            T::getStaticClassRep()->getClassName(), name );
         *ptr = NULL;
         return false;
      }
      
      *ptr = p;
   }
   
   return true;
}


//=============================================================================
//    TypeSFXSourceName.
//=============================================================================

ConsoleType( SFXSource, TypeSFXSourceName, SFXSource* )

ConsoleGetType( TypeSFXSourceName )
{
   SFXSource** obj = ( SFXSource** ) dptr;
   if( !*obj )
      return "";
   else
      return Con::getReturnBuffer( ( *obj )->getName() );
}

ConsoleSetType( TypeSFXSourceName )
{
   if( argc == 1 )
   {
      SFXSource** obj = ( SFXSource**) dptr;
      Sim::findObject( argv[ 0 ], *obj );
   }
   else
      Con::printf("(TypeSFXSourceName) Cannot set multiple args to a single SFXSource.");
}

//=============================================================================
//    TypeSFXParameterName.
//=============================================================================

ConsoleType( string, TypeSFXParameterName, StringTableEntry )

ConsoleGetType( TypeSFXParameterName )
{
   return *( ( const char** ) dptr );
}

ConsoleSetType( TypeSFXParameterName )
{
   if( argc == 1 )
      *( ( const char** ) dptr ) = StringTable->insert( argv[ 0 ] );
   else
      Con::errorf( "(TypeSFXParameterName) Cannot set multiple args to a single SFXParameter." );
}

//=============================================================================
//    TypeSFXDescriptionName.
//=============================================================================

ConsoleType( SFXDescription, TypeSFXDescriptionName, SFXDescription* )

ConsoleSetType( TypeSFXDescriptionName )
{
   if( argc == 1 )
   {
      SFXDescription* description;
      Sim::findObject( argv[ 0 ], description );
      *( ( SFXDescription** ) dptr ) = description;
   }
   else
      Con::errorf( "(TypeSFXDescriptionName) Cannot set multiple args to a single SFXDescription.");
}

ConsoleGetType( TypeSFXDescriptionName )
{
   SFXDescription* description = *( ( SFXDescription** ) dptr );
   if( !description || !description->getName() )
      return "";
   else
      return description->getName();
}

//=============================================================================
//    TypeSFXTrackName.
//=============================================================================

ConsoleType( SFXTrack, TypeSFXTrackName, SFXTrack* )

ConsoleSetType( TypeSFXTrackName )
{
   if( argc == 1 )
   {
      SFXTrack* track;
      Sim::findObject( argv[ 0 ], track );
      *( ( SFXTrack** ) dptr ) = track;
   }
   else
      Con::errorf( "(TypeSFXTrackName) Cannot set multiple args to a single SFXTrack.");
}

ConsoleGetType( TypeSFXTrackName )
{
   SFXTrack* track = *( ( SFXTrack** ) dptr );
   if( !track || !track->getName() )
      return "";
   else
      return track->getName();
}

//=============================================================================
//    TypeSFXEnvironmentName.
//=============================================================================

ConsoleType( SFXEnvironment, TypeSFXEnvironmentName, SFXEnvironment* )

ConsoleSetType( TypeSFXEnvironmentName )
{
   if( argc == 1 )
   {
      SFXEnvironment* environment;
      Sim::findObject( argv[ 0 ], environment );
      *( ( SFXEnvironment** ) dptr ) = environment;
   }
   else
      Con::errorf( "(TypeSFXEnvironmentName) Cannot set multiple args to a single SFXEnvironment.");
}

ConsoleGetType( TypeSFXEnvironmentName )
{
   SFXEnvironment* environment = *( ( SFXEnvironment** ) dptr );
   if( !environment || !environment->getName() )
      return "";
   else
      return environment->getName();
}

//=============================================================================
//    TypeSFXStateName.
//=============================================================================

ConsoleType( SFXState, TypeSFXStateName, SFXState* )

ConsoleSetType( TypeSFXStateName )
{
   if( argc == 1 )
   {
      SFXState* state;
      Sim::findObject( argv[ 0 ], state );
      *( ( SFXState** ) dptr ) = state;
   }
   else
      Con::errorf( "(TypeSFXStateName) Cannot set multiple args to a single SFXState.");
}

ConsoleGetType( TypeSFXStateName )
{
   SFXState* state = *( ( SFXState** ) dptr );
   if( !state || !state->getName() )
      return "";
   else
      return state->getName();
}

//=============================================================================
//    TypeSFXAmbienceName.
//=============================================================================

ConsoleType( SFXAmbience, TypeSFXAmbienceName, SFXAmbience* )

ConsoleSetType( TypeSFXAmbienceName )
{
   if( argc == 1 )
   {
      SFXAmbience* ambience;
      Sim::findObject( argv[ 0 ], ambience );
      *( ( SFXAmbience** ) dptr ) = ambience;
   }
   else
      Con::errorf( "(TypeSFXAmbienceName) Cannot set multiple args to a single SFXAmbience.");
}

ConsoleGetType( TypeSFXAmbienceName )
{
   SFXAmbience* ambience = *( ( SFXAmbience** ) dptr );
   if( !ambience || !ambience->getName() )
      return "";
   else
      return ambience->getName();
}

//=============================================================================
//    I/O.
//=============================================================================

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXSource* source )
{
   if( stream->writeFlag( source != NULL ) )
      stream->writeString( source->getName() );
}

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXDescription* description )
{
   sWrite( stream, description );
}

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXTrack* track )
{
   sWrite( stream, track );
}

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXEnvironment* environment )
{
   sWrite( stream, environment );
}

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXState* state )
{
   sWrite( stream, state );
}

//-----------------------------------------------------------------------------

void sfxWrite( BitStream* stream, SFXAmbience* ambience )
{
   sWrite( stream, ambience );
}

//-----------------------------------------------------------------------------

void sfxRead( BitStream* stream, SFXDescription** description )
{
   sRead( stream, description );
}

//-----------------------------------------------------------------------------

void sfxRead( BitStream* stream, SFXTrack** track )
{
   sRead( stream, track );
}

//-----------------------------------------------------------------------------

void sfxRead( BitStream* stream, SFXEnvironment** environment )
{
   sRead( stream, environment );
}

//-----------------------------------------------------------------------------

void sfxRead( BitStream* stream, SFXState** state )
{
   sRead( stream, state );
}

//-----------------------------------------------------------------------------

void sfxRead( BitStream* stream, SFXAmbience** ambience )
{
   sRead( stream, ambience );
}

//-----------------------------------------------------------------------------

bool sfxResolve( SFXDescription** description, String& errorString )
{
   return sResolve( description, errorString );
}

//-----------------------------------------------------------------------------

bool sfxResolve( SFXTrack** track, String& errorString )
{
   return sResolve( track, errorString );
}

//-----------------------------------------------------------------------------

bool sfxResolve( SFXEnvironment** environment, String& errorString )
{
   return sResolve( environment, errorString );
}

//-----------------------------------------------------------------------------

bool sfxResolve( SFXState** state, String& errorString )
{
   return sResolve( state, errorString );
}

//-----------------------------------------------------------------------------

bool sfxResolve( SFXAmbience** ambience, String& errorString )
{
   return sResolve( ambience, errorString );
}

//-----------------------------------------------------------------------------

bool sfxReadAndResolve( BitStream* stream, SFXSource** source, String& errorString )
{
   if( !stream->readFlag() )
   {
      *source = NULL;
      return true;
   }
   
   const char* name = stream->readSTString();

   SFXSource* object;
   if( !Sim::findObject( name, object ) )
   {
      errorString = String::ToString( "sfxReadAndResolve - no SFXSource '%s'", name );
      return false;
   }
   
   *source = object;
   return true;
}
