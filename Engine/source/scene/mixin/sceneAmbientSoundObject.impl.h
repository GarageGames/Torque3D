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

#ifndef _SCENEAMBIENTSOUNDOBJECT_IMPL_H_
#define _SCENEAMBIENTSOUNDOBJECT_IMPL_H_

#include "platform/platform.h"
#include "scene/mixin/sceneAmbientSoundObject.h"

#include "T3D/sfx/sfx3DWorld.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxAmbience.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"




//-----------------------------------------------------------------------------

template< typename Base >
SceneAmbientSoundObject< Base >::SceneAmbientSoundObject()
   : mSoundAmbience( NULL )
{
}

//-----------------------------------------------------------------------------

template< typename Base >
void SceneAmbientSoundObject< Base >::initPersistFields()
{
   Parent::addGroup( "Sound" );
      Parent::addProtectedField( "soundAmbience", TypeSFXAmbienceName, Offset( mSoundAmbience, SceneAmbientSoundObject ),
         &_setSoundAmbience, &defaultProtectedGetFn,
         "Ambient sound environment for the space." );
   Parent::endGroup( "Sound" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

template< typename Base >
U32 SceneAmbientSoundObject< Base >::packUpdate( NetConnection* connection, U32 mask, BitStream* stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );

   if( stream->writeFlag( mask & SoundMask ) )
      sfxWrite( stream, mSoundAmbience );

   return retMask;
}

//-----------------------------------------------------------------------------

template< typename Base >
void SceneAmbientSoundObject< Base >::unpackUpdate( NetConnection* connection, BitStream* stream )
{
   Parent::unpackUpdate( connection, stream );

   if( stream->readFlag() ) // SoundMask
   {
      SFXAmbience* ambience;

      String errorStr;
      if( !sfxReadAndResolve( stream, &ambience, errorStr ) )
         Con::errorf( "SceneAmbientSoundObject::unpackUpdate - bad packet: %s", errorStr.c_str() );
      else
         setSoundAmbience( ambience );
   }
}

//-----------------------------------------------------------------------------

template< typename Base >
void SceneAmbientSoundObject< Base >::setSoundAmbience( SFXAmbience* ambience )
{
   if( mSoundAmbience == ambience )
      return;

   mSoundAmbience = ambience;

   if( this->isServerObject() )
      this->setMaskBits( SoundMask );
   else if( this->isProperlyAdded() && gSFX3DWorld )
      gSFX3DWorld->notifyChanged( this );
}

//-----------------------------------------------------------------------------

template< typename Base >
bool SceneAmbientSoundObject< Base >::_setSoundAmbience( void* object, const char* index, const char* data )
{
   SceneAmbientSoundObject* p = reinterpret_cast< SceneAmbientSoundObject* >( object );
   SFXAmbience* ambience = EngineUnmarshallData< SFXAmbience* >()( data );
   p->setSoundAmbience( ambience );
   return false;
}

#endif // _SCENEAMBIENTSOUNDOBJECT_IMPL_H_
