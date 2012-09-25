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

#include "platform/platform.h"
#include "sfx/fmod/sfxFMODEvent.h"
#include "sfx/fmod/sfxFMODEventGroup.h"
#include "sfx/fmod/sfxFMODProject.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/sfxParameter.h"
#include "sfx/sfxDescription.h"
#include "core/stream/bitStream.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXFMODEvent );


ConsoleDocClass( SFXFMODEvent,
   "@brief A playable sound event in an FMOD Designer audio project.\n\n"
   
   "@ingroup SFXFMOD\n"
   "@ingroup Datablocks"
);


//-----------------------------------------------------------------------------

SFXFMODEvent::SFXFMODEvent()
   : mGroup( NULL ),
     mHandle( NULL ),
     mGroupId( 0 ),
     mSibling( NULL )
{
   dMemset( mParameterRanges, 0, sizeof( mParameterRanges ) );
   dMemset( mParameterValues, 0, sizeof( mParameterValues ) );
}

//-----------------------------------------------------------------------------

SFXFMODEvent::SFXFMODEvent( SFXFMODEventGroup* group, FMOD_EVENT* handle )
   : mGroup( group ),
     mHandle( handle ),
     mGroupId( 0 ),
     mSibling( NULL )
{
   dMemset( mParameterRanges, 0, sizeof( mParameterRanges ) );
   dMemset( mParameterValues, 0, sizeof( mParameterValues ) );
   
   // Fetch name.
   
   int index;
   char* name = NULL;
   
   SFXFMODDevice::smFunc->FMOD_Event_GetInfo( mHandle, &index, &name, ( FMOD_EVENT_INFO* ) 0 );
   
   mName = name;
   
   // Read out the parameter info so we can immediately create
   // the events on the client-side without having to open and
   // read all the project info there.
   
   int numParameters;
   SFXFMODDevice::smFunc->FMOD_Event_GetNumParameters( mHandle, &numParameters );
   if( numParameters > MaxNumParameters )
   {
      Con::errorf( "SFXFMODEvent::SFXFMODEvent - event '%s' has more parameters (%i) than supported per SFXTrack (%i)",
         getQualifiedName().c_str(),
         numParameters,
         MaxNumParameters );
      numParameters = MaxNumParameters;
   }
   
   for( U32 i = 0; i < numParameters; ++ i )
   {
      FMOD_EVENTPARAMETER* parameter;
      SFXFMODDevice::smFunc->FMOD_Event_GetParameterByIndex( mHandle, i, &parameter );
      
      SFXFMODDevice::smFunc->FMOD_EventParameter_GetInfo( parameter, &index, &name );
      setParameter( i, name );
         
      // Get value and range of parameter.
      
      SFXFMODDevice::smFunc->FMOD_EventParameter_GetValue( parameter, &mParameterValues[ i ] );
      SFXFMODDevice::smFunc->FMOD_EventParameter_GetRange( parameter, &mParameterRanges[ i ].x, &mParameterRanges[ i ].y );
   }
   
   // Read out the properties and create a custom SFXDescription for the event.
   
   mDescription = new SFXDescription;
   if( !group->isClientOnly() )
      mDescription->assignId();
      
   mDescription->registerObject(
      String::ToString( "%s_%s_Description",
         group->getName(),
         FMODEventPathToTorqueName( mName ).c_str()
      )
   );
   if( group->isClientOnly() )
      Sim::getRootGroup()->addObject( mDescription );
   
   FMOD_MODE modeValue;
   float floatValue;
   
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_MODE, &modeValue, true ) == FMOD_OK )
      mDescription->mIs3D = ( modeValue == FMOD_3D );
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_VOLUME, &floatValue, true ) == FMOD_OK )
      mDescription->mVolume = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_PITCH, &floatValue, true ) == FMOD_OK )
      mDescription->mPitch = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_3D_MINDISTANCE, &floatValue, true ) == FMOD_OK )
      mDescription->mMinDistance = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_3D_MAXDISTANCE, &floatValue, true ) == FMOD_OK )
      mDescription->mMaxDistance = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_3D_CONEINSIDEANGLE, &floatValue, true ) == FMOD_OK )
      mDescription->mConeInsideAngle = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_3D_CONEOUTSIDEANGLE, &floatValue, true ) == FMOD_OK )
      mDescription->mConeOutsideAngle = floatValue;
   if( SFXFMODDevice::smFunc->FMOD_Event_GetPropertyByIndex( mHandle, FMOD_EVENTPROPERTY_3D_CONEOUTSIDEVOLUME, &floatValue, true ) == FMOD_OK )
      mDescription->mConeOutsideVolume = floatValue;
      
   // Don't read out fade values as we want to leave fade-effects to
   // FMOD rather than having the fading system built into SFX pick
   // these values up.
}

//-----------------------------------------------------------------------------

SFXFMODEvent::~SFXFMODEvent()
{
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::initPersistFields()
{
   addGroup( "DO NOT MODIFY!!" );
   addField( "fmodGroup", TYPEID< SFXFMODEventGroup >(), Offset( mGroup, SFXFMODEvent ), "DO NOT MODIFY!!" );
   addField( "fmodName", TypeRealString, Offset( mName, SFXFMODEvent ), "DO NOT MODIFY!!" );
   addField( "fmodParameterRanges", TypePoint2F, Offset( mParameterRanges, SFXFMODEvent ), MaxNumParameters, "DO NOT MODIFY!!" );
   addField( "fmodParameterValues", TypeF32, Offset( mParameterValues, SFXFMODEvent ), MaxNumParameters, "DO NOT MODIFY!!" );
   endGroup( "DO NOT MODIFY!!" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXFMODEvent::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   if( !mGroup )
   {
      Con::errorf( "SFXFMODEvent::onAdd - no group set; this event was not properly constructed" );
      return false;
   }
   
   mGroup->_addEvent( this );
   mGroup->mProject->_addEvent( this );
   
   // For non-networked event datablocks, create the parameter
   // instances now.
   
   if( isClientOnly() )
      _createParameters();
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::onRemove()
{
   Parent::onRemove();
   
   if( !mGroup )
      return;
   
   release();
   mGroup->_removeEvent( this );
}

//-----------------------------------------------------------------------------

bool SFXFMODEvent::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   if( !server )
   {
      if( !Sim::findObject( mGroupId, mGroup ) )
      {
         errorStr = String::ToString( "SFXFMODEvent - group '%i' does not exist", mGroupId );
         return false;
      }

      // Create parameters.

      _createParameters();
   }
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   stream->write( mName );
   stream->writeRangedS32( mGroup->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast );
   
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->writeFlag( mParameters[ i ] ) )
      {
         stream->write( mParameterValues[ i ] );
         stream->write( mParameterRanges[ i ].x );
         stream->write( mParameterRanges[ i ].y );
      }
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );

   stream->read( &mName );
   mGroupId = stream->readRangedS32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   
   for( U32 i = 0; i < MaxNumParameters; ++ i )
      if( stream->readFlag() )
      {
         stream->read( &mParameterValues[ i ] );
         stream->read( &mParameterRanges[ i ].x );
         stream->read( &mParameterRanges[ i ].y );
      }
      else
      {
         mParameterValues[ i ] = 0.f;
         mParameterRanges[ i ].x = 0.f;
         mParameterRanges[ i ].y = 0.f;
      }
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::acquire()
{
   if( mHandle )
      return;
      
   mGroup->acquire();
   if( SFXFMODDevice::smFunc->FMOD_EventGroup_GetEvent(
         mGroup->mHandle, mName.c_str(), FMOD_EVENT_INFOONLY, &mHandle ) != FMOD_OK )
   {
      Con::errorf( "SFXFMODEvent::acquire() - failed to acquire event '%s'", getQualifiedName().c_str() );
      return;
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::release()
{
   if( !mHandle )
      return;
      
   SFXFMODDevice::smFunc->FMOD_Event_Release( mHandle, true, false );
   mHandle = NULL;
}

//-----------------------------------------------------------------------------

String SFXFMODEvent::getQualifiedName() const
{
   return String::ToString( "%s/%s", getEventGroup()->getQualifiedName().c_str(), mName.c_str() );
}

//-----------------------------------------------------------------------------

void SFXFMODEvent::_createParameters()
{
   const String& projectFileName = getEventGroup()->getProject()->getFileName();
   const String qualifiedGroupName = getEventGroup()->getQualifiedName();
   const String description = String::ToString( "FMOD Event Parameter (%s)", projectFileName.c_str() );
   
   for( U32 i = 0; i < MaxNumParameters; ++ i )
   {
      StringTableEntry name = getParameter( i );
      if( !name )
         continue;
         
      SFXParameter* parameter = SFXParameter::find( name );
      if( !parameter )
      {
         parameter = new SFXParameter();
         parameter->setInternalName( name );
         parameter->registerObject();
         
         // Set up parameter.
         
         parameter->setChannel( SFXChannelUser0 );
         parameter->setRange( mParameterRanges[ i ] );
         parameter->setDefaultValue( mParameterValues[ i ] );
         parameter->setValue( mParameterValues[ i ] );
         parameter->setDescription( description );
         
         // Set categories for easy filtering.
         
         static StringTableEntry sCategories = StringTable->insert( "categories" );
         parameter->setDataField( sCategories, "0", "FMOD" );
         parameter->setDataField( sCategories, "1", avar( "FMOD Project: %s", projectFileName.c_str() ) );
         parameter->setDataField( sCategories, "2", avar( "FMOD Group: %s", qualifiedGroupName.c_str() ) );
      }
   }
}
