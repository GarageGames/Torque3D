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

#include "sfx/fmod/sfxFMODEventGroup.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/fmod/sfxFMODEvent.h"
#include "sfx/fmod/sfxFMODProject.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXFMODEventGroup );

ConsoleDocClass( SFXFMODEventGroup,
   "@brief A group of events in an imported FMOD Designer project.\n\n"
   
   ""
   
   "@note Instances of this class \n\n"
   
   "@ingroup SFXFMOD\n"
   "@ingroup Datablocks"
);

//-----------------------------------------------------------------------------

SFXFMODEventGroup::SFXFMODEventGroup()
   : mProject( NULL ),
     mHandle( NULL ),
     mParent( NULL ),
     mChildren( NULL ),
     mSibling( NULL ),
     mLoadCount( 0 ),
     mEvents( NULL ),
     mNumEvents( 0 ),
     mNumGroups( 0 ),
     mParentId( 0 ),
     mProjectId( 0 )
{
}

//-----------------------------------------------------------------------------

SFXFMODEventGroup::SFXFMODEventGroup( SFXFMODProject* project, FMOD_EVENTGROUP* handle, SFXFMODEventGroup* parent )
   : mProject( project ),
     mHandle( handle ),
     mParent( parent ),
     mChildren( NULL ),
     mSibling( NULL ),
     mLoadCount( 0 ),
     mEvents( NULL ),
     mNumEvents( 0 ),
     mNumGroups( 0 ),
     mParentId( 0 ),
     mProjectId( 0 )
{
   AssertFatal( project != NULL, "SFXFMODEventGroup::SFXFMODEventGroup - got a NULL project!" );
   AssertFatal( handle != NULL, "SFXFMODEventGroup::SFXFMODEventGroup - got a NULL group handle!" );
   
   // Fetch the name.
   
   int index;
   char* name = NULL;
   
   SFXFMODDevice::smFunc->FMOD_EventGroup_GetInfo( handle, &index, &name );
   
   mName = name;
}

//-----------------------------------------------------------------------------

SFXFMODEventGroup::~SFXFMODEventGroup()
{
   AssertFatal( mEvents == NULL, "SFXFMODEventGroup::~SFXFMODEventGroup - group still has events attached" );
   AssertFatal( mChildren == NULL, "SFXFMODEventGroup::~SFXFMODEventGroup - group still has subgroups attached" );
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::initPersistFields()
{
   addGroup( "DO NOT MODIFY!!" );
   addField( "fmodProject", TYPEID< SFXFMODProject >(), Offset( mProject, SFXFMODEventGroup ), "DO NOT MODIFY!!" );
   addField( "fmodGroup", TYPEID< SFXFMODEventGroup >(), Offset( mParent, SFXFMODEventGroup ), "DO NOT MODIFY!!" );
   addField( "fmodName", TypeRealString, Offset( mName, SFXFMODEventGroup ), "DO NOT MODIFY!!" );
   endGroup( "DO NOT MODIFY!!" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXFMODEventGroup::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   if( !mProject )
   {
      Con::errorf( "SFXFMODEventGroup - not part of a project" );
      return false;
   }
      
   if( mParent )
      mParent->_addGroup( this );
   
   mProject->_addGroup( this );
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::onRemove()
{
   Parent::onRemove();
   
   if( !mProject )
      return;
   
   release();
   
   while( mEvents )
      mEvents->deleteObject();
   while( mChildren )
      mChildren->deleteObject();
   
   if( mParent )
      mParent->_removeGroup( this );
      
   mProject->_removeGroup( this );
}

//-----------------------------------------------------------------------------

bool SFXFMODEventGroup::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
   
   if( !server )
   {
      if( mParentId != 0 && !Sim::findObject( mParentId, mParent ) )
      {
         errorStr = String::ToString( "SFXFMODEventGroup - parent group '%i' does not exist", mParentId );
         return false;
      }
      if( !Sim::findObject( mProjectId, mProject ) )
      {
         errorStr = String::ToString( "SFXFMODEventGroup - project '%i' does not exist", mProjectId );
         return false;
      }
   }
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   stream->write( mName );
   stream->writeRangedS32( mProject->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast );
   if( stream->writeFlag( mParent ) )
      stream->writeRangedS32( mParent->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast );
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   stream->read( &mName );
   
   mProjectId = stream->readRangedS32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   if( stream->readFlag() )
      mParentId = stream->readRangedS32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   else
      mParentId = 0;
}

//-----------------------------------------------------------------------------

String SFXFMODEventGroup::getQualifiedName() const
{
   if( mParent )
      return String::ToString( "%s/%s", mParent->getQualifiedName().c_str(), mName.c_str() );
   else
      return mName;
}

//-----------------------------------------------------------------------------

bool SFXFMODEventGroup::isDataLoaded() const
{
   // Check whether we or any of our parents has triggered a load.
   
   for( const SFXFMODEventGroup* group = this; group != NULL; group = group->mParent )
      if( group->mLoadCount > 0 )
         return true;
         
   return false;
}

//-----------------------------------------------------------------------------

bool SFXFMODEventGroup::loadData( bool samples, bool streams )
{
   if( !mHandle )
      acquire();
   
   if( !mLoadCount )
   {
      FMOD_EVENT_RESOURCE resource;
      if( samples && streams )
         resource = FMOD_EVENT_RESOURCE_STREAMS_AND_SAMPLES;
      else if( samples )
         resource = FMOD_EVENT_RESOURCE_SAMPLES;
      else if( streams )
         resource = FMOD_EVENT_RESOURCE_STREAMS;
      else
         return true;
         
      FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_EventGroup_LoadEventData( mHandle, resource, FMOD_EVENT_DEFAULT );
      if( result != FMOD_OK )
      {
         Con::errorf( "SFXFMODEventGroup::loadData - could not load data: %s", FMODResultToString( result ).c_str() );
         return false;
      }
      
      SFXFMODDevice::instance()->updateMemUsageStats();
      Con::printf( "SFXFMODProject - %s: Loaded data for group '%s'", mProject->getName(), getQualifiedName().c_str() );
   }
   
   mLoadCount ++;
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::freeData( bool force )
{
   bool isLoaded = ( mLoadCount > 0 );
   
   if( !isLoaded )
      isLoaded = ( mParent ? mParent->isDataLoaded() : false );
   else
   {
      if( force )
         mLoadCount = 0;
      else
         -- mLoadCount;
   }
      
   if( !mLoadCount && isLoaded )
   {
      FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_EventGroup_FreeEventData( mHandle, ( FMOD_EVENT* ) NULL, false );
      if( result != FMOD_OK )
         Con::errorf( "SFXFMODEventGroup - failed freeing event data: %s", FMODResultToString( result ).c_str() );
         
      SFXFMODDevice::instance()->updateMemUsageStats();
      Con::printf( "SFXFMODProject - %s: Cleared data for group '%s'", mProject->getName(), getQualifiedName().c_str() );
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::acquire( bool recursive )
{
   // Make sure the project is acquired.
   
   mProject->acquire();
   
   // Acquire the group.
   
   if( !mHandle )
   {
      if( mParent )
      {
         mParent->acquire();
         SFXFMODDevice::smFunc->FMOD_EventGroup_GetGroup( mParent->mHandle, mName, true, &mHandle );
      }
      else
      {
         mProject->acquire();
         SFXFMODDevice::smFunc->FMOD_EventProject_GetGroup( mProject->mHandle, mName, true, &mHandle );
      }
   }
   
   // Acquite events and subgroups.
   
   if( recursive )
   {
      for( SFXFMODEvent* event = mEvents; event != NULL; event = event->mSibling )
         event->acquire();
         
      for( SFXFMODEventGroup* group = mChildren; group != NULL; group = group->mSibling )
         group->acquire( true );
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::release()
{
   if( !mHandle )
      return;
      
   // Free the event data if we still have it loaded.
      
   if( isDataLoaded() )
      freeData( true );
      
   // Release events.
   
   for( SFXFMODEvent* event = mEvents; event != NULL; event = event->mSibling )
      event->release();
   
   // Release children.
   
   for( SFXFMODEventGroup* child = mChildren; child != NULL; child = child->mSibling )
      child->release();
   
   // Release our handle.
   
   freeData();
   mHandle = NULL;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::_load()
{
   // Make sure we have the group open.
   
   if( !mHandle )
      acquire();
      
   // Fetch info.
   
   int numEvents;
   int numGroups;

   SFXFMODDevice::smFunc->FMOD_EventGroup_GetNumEvents( mHandle, &numEvents );
   SFXFMODDevice::smFunc->FMOD_EventGroup_GetNumGroups( mHandle, &numGroups );
   
   // Load events.
   
   for( U32 i = 0; i < numEvents; ++ i )
   {
      FMOD_EVENT* handle;
      if( SFXFMODDevice::smFunc->FMOD_EventGroup_GetEventByIndex( mHandle, i, FMOD_EVENT_INFOONLY, &handle ) == FMOD_OK )
      {
         SFXFMODEvent* event = new SFXFMODEvent( this, handle );
         if( !isClientOnly() )
            event->assignId();
            
         event->registerObject( String::ToString( "%s_%s", getName(), FMODEventPathToTorqueName( event->getEventName() ).c_str() ) );
         if( isClientOnly() )
            Sim::getRootGroup()->addObject( event );
      }
   }
   
   // Load subgroups.
   
   for( U32 i = 0; i < numGroups; ++ i )
   {
      FMOD_EVENTGROUP* handle;
      if( SFXFMODDevice::smFunc->FMOD_EventGroup_GetGroupByIndex( mHandle, i, true, &handle ) == FMOD_OK )
      {
         SFXFMODEventGroup* group = new SFXFMODEventGroup( mProject, handle, this );
         if( !isClientOnly() )
            group->assignId();
            
         group->registerObject( String::ToString( "%s_%s", getName(), FMODEventPathToTorqueName( group->getGroupName() ).c_str() ) );
         if( isClientOnly() )
            Sim::getRootGroup()->addObject( group );
         
         group->_load();
      }
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::_addEvent( SFXFMODEvent* event )
{
   event->mSibling = mEvents;
   mEvents = event;
   mNumEvents ++;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::_removeEvent( SFXFMODEvent* event )
{
   if( mEvents == event )
   {
      mEvents = event->mSibling;
      event->mSibling = NULL;
      mNumEvents --;
   }
   else
   {
      SFXFMODEvent* p = mEvents;
      while( p != NULL && p->mSibling != event )
         p = p->mSibling;
      
      if( p )
      {
         p->mSibling = event->mSibling;
         event->mSibling = NULL;
         mNumEvents --;
      }
   }
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::_addGroup( SFXFMODEventGroup* group )
{
   group->mSibling = mChildren;
   mChildren = group;
   mNumGroups ++;
}

//-----------------------------------------------------------------------------

void SFXFMODEventGroup::_removeGroup( SFXFMODEventGroup* group )
{
   if( mChildren == group )
   {
      mChildren = group->mSibling;
      group->mSibling = NULL;
      mNumGroups --;
   }
   else
   {
      SFXFMODEventGroup* p = mChildren;
      while( p != NULL && p->mSibling != group )
         p = p->mSibling;
      
      if( p )
      {
         p->mSibling = group->mSibling;
         group->mSibling = NULL;
         mNumGroups --;
      }
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXFMODEventGroup, isDataLoaded, bool, (),,
   "Test whether the resource data for this group has been loaded.\n\n"
   "@return True if the resource data for this group is currently loaded.\n" )
{
   return object->isDataLoaded();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXFMODEventGroup, loadData, bool, ( bool loadStreams, bool loadSamples ), ( true, true ),
   "Load the resource data for this group, if it has not already been loaded (either directly "
   "or indirectly through a parent group).\n"
   "This method works recursively and thus data for direct and indirect child groups to this group will be "
   "loaded as well.\n\n"
   "@param loadStreams Whether to open streams.\n"
   "@param loadSamples Whether to load sample banks.\n"
   "@return True if the data has been successfully loaded; false otherwise.\n\n"
   "@see SFXFMODProject_resources" )
{
   return object->loadData( loadSamples, loadStreams );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXFMODEventGroup, freeData, void, (),,
   "Release the resource data for this group and its subgroups.\n\n"
   "@see SFXFMODProject_resources" )
{
   object->freeData();
}
