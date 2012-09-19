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

#include "sfx/fmod/sfxFMODProject.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/fmod/sfxFMODEvent.h"
#include "sfx/fmod/sfxFMODEventGroup.h"
#include "sfx/sfxDescription.h"
#include "core/stringTable.h"
#include "core/volume.h"
#include "core/util/path.h"
#include "core/stream/fileStream.h"
#include "core/stream/bitStream.h"
#include "core/util/safeDelete.h"


IMPLEMENT_CO_DATABLOCK_V1( SFXFMODProject );


ConsoleDocClass( SFXFMODProject,
   "@brief An FMOD Designer project loaded into Torque.\n\n"
   
   "@section SFXFMODProject_resources Resource Loading\n\n"
   
   "@ingroup SFXFMOD\n"
   "@ingroup Datablocks"
);


//-----------------------------------------------------------------------------

SFXFMODProject::SFXFMODProject()
   : mHandle( NULL ),
     mRootGroups( NULL )
{
   VECTOR_SET_ASSOCIATION( mGroups );
   VECTOR_SET_ASSOCIATION( mEvents );
   
   SFX->getEventSignal().notify( this, &SFXFMODProject::_onSystemEvent );
}

//-----------------------------------------------------------------------------

SFXFMODProject::~SFXFMODProject()
{
   AssertFatal( mGroups.empty(), "SFXFMODProject::~SFXFMODProject - project still has groups attached" );
   AssertFatal( mEvents.empty(), "SFXFMODProject::~SFXFMODProject - project still has events attached" );
   
   if( SFX )
      SFX->getEventSignal().remove( this, &SFXFMODProject::_onSystemEvent );
}

//-----------------------------------------------------------------------------

void SFXFMODProject::initPersistFields()
{
   addGroup( "FMOD" );
   
      addField( "fileName", TypeStringFilename, Offset( mFileName, SFXFMODProject ), "The compiled .fev file from FMOD Designer." );
      addField( "mediaPath", TypeStringFilename, Offset( mMediaPath, SFXFMODProject ), "Path to the media files; if unset, defaults to project directory." );
   
   endGroup( "FMOD" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXFMODProject::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   // If this is a non-networked datablock, load the
   // project data now.
      
   if( isClientOnly() && !_load() )
      return false;
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODProject::onRemove()
{
   Parent::onRemove();
   
   _clear();
}

//-----------------------------------------------------------------------------

bool SFXFMODProject::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;
      
   if( server )
   {
      if( mFileName.isEmpty() )
      {
         errorStr = String::ToString( "SFXFMODProject::preload - no filename set on %i (%s)",
            getId(), getName() );
         return false;
      }
      
      if( mGroups.empty() || mEvents.empty() )
         _load();
         
      release();
   }
   
   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODProject::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   stream->write( mFileName );
   stream->write( mMediaPath );
}

//-----------------------------------------------------------------------------

void SFXFMODProject::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   stream->read( &mFileName );
   stream->read( &mMediaPath );
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_onSystemEvent( SFXSystemEventType event )
{
   switch( event )
   {
      case SFXSystemEvent_DestroyDevice:
      
         // If the FMOD device is being destroyed,
         // release all our data.
         
         if( SFXFMODDevice::instance() )
            release();
            
         break;
         
      default:
         break;
   }
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_clear()
{
   release();
   
   for( U32 i = 0; i < mGroups.size(); ++ i )
      if( !mGroups[ i ]->isRemoved() )
         mGroups[ i ]->deleteObject();
      
   mGroups.clear();
   mEvents.clear();
   
   mRootGroups = NULL;
}

//-----------------------------------------------------------------------------

bool SFXFMODProject::_load()
{
   const Torque::Path eventScriptFileName = mFileName + ".cs";
   const Torque::Path eventScriptFileNameDSO = eventScriptFileName + ".dso";
   const bool eventScriptFileExists = Torque::FS::IsFile( eventScriptFileName );
   const bool eventScriptFileDSOExists = Torque::FS::IsFile( eventScriptFileNameDSO );
      
   // Check if we need to (re-)generate the event script file.
   
   bool needToGenerateEventScriptFile = false;
   if(    ( !eventScriptFileExists && !eventScriptFileDSOExists )
       || ( Torque::FS::CompareModifiedTimes( mFileName, eventScriptFileName ) > 0
            || Torque::FS::CompareModifiedTimes( mFileName, eventScriptFileNameDSO ) > 0 ) )
      needToGenerateEventScriptFile = true;
      
   // If we need to generate, check if we can.
   
   SFXFMODDevice* fmodDevice = SFXFMODDevice::instance();
   if( needToGenerateEventScriptFile && !fmodDevice )
   {
      // If we have neither FMOD nor the event scripts (even if outdated),
      // there's nothing we can do.
      
      if( !eventScriptFileExists && !eventScriptFileDSOExists )
      {
         Con::errorf( "SFXFMODProject::_load() - event script for '%s' does not exist and device is not FMOD; load this project under FMOD first",
            mFileName.c_str() );
         return false;
      }
      
      // Use the oudated versions.
      
      Con::warnf( "SFXMODProject::_load() - event script for '%s' is outdated and device is not FMOD; event data may not match .fev contents",
         mFileName.c_str() );
      needToGenerateEventScriptFile = false;
   }
   
   // If we don't need to regenerate, try executing the event script now.
   
   if( !needToGenerateEventScriptFile )
   {
      if(    ( eventScriptFileExists || eventScriptFileDSOExists )
          && !Con::evaluatef( "exec( \"%s\" );", eventScriptFileName.getFullPath().c_str() ) )
      {
         Con::errorf( "SFXFMODProject::_load() - failed to execute event script for '%s'%s",
            mFileName.c_str(),
            fmodDevice != NULL ? "; trying to regenerate" : ""
         );
         
         if( !fmodDevice )
            return false;
            
         needToGenerateEventScriptFile = true;
      }
      else
         Con::printf( "SFXFMODProject - %s: Loaded event script", getName() );
   }
      
   // If we need to generate the event script file,
   // load the FMOD project now and then emit the file.
   
   if( needToGenerateEventScriptFile )
   {
      // Try to load the project.
      
      acquire();
      
      if( !mHandle )
         return false;
         
      // Get the project info.

      FMOD_EVENT_PROJECTINFO info;
      
      int numEvents;
      int numGroups;
      
      SFXFMODDevice::smFunc->FMOD_EventProject_GetInfo( mHandle, &info );
      SFXFMODDevice::smFunc->FMOD_EventProject_GetNumEvents( mHandle, &numEvents );
      SFXFMODDevice::smFunc->FMOD_EventProject_GetNumGroups( mHandle, &numGroups );
      
      Con::printf( "SFXFMODProject - %s: Loading '%s' from '%s' (index: %i, events: %i, groups: %i)",
         getName(), info.name, mFileName.c_str(), info.index, numEvents, numGroups );
         
      // Load the root groups.

      for( U32 i = 0; i < numGroups; ++ i )
      {
         FMOD_EVENTGROUP* group;
         if( SFXFMODDevice::smFunc->FMOD_EventProject_GetGroupByIndex( mHandle, i, true, &group ) == FMOD_OK )
         {
            SFXFMODEventGroup* object = new SFXFMODEventGroup( this, group );
            
            object->mSibling = mRootGroups;
            mRootGroups = object;
            
            String qualifiedName = FMODEventPathToTorqueName( object->getQualifiedName() );
            
            if( !isClientOnly() )
               object->assignId();
               
            object->registerObject( String::ToString( "%s_%s", getName(), qualifiedName.c_str() ) );
            if( isClientOnly() )
               Sim::getRootGroup()->addObject( object );
            
            object->_load();
         }
      }

      // Create the event script file.
      
      FileStream stream;
      if( !stream.open( eventScriptFileName.getFullPath(), Torque::FS::File::Write ) )
      {
         Con::errorf( "SFXFMODProject::_load - could not create event script file for '%s'", mFileName.c_str() );
         return true; // Don't treat as failure.
      }
      
      // Write a header.
      
      stream.writeText( String::ToString( "// This file has been auto-generated from '%s'\n", mFileName.c_str() ) );
      stream.writeText( "// Do not edit this file manually and do not move it away from the Designer file.\n\n" );
      
      // Write the group objects.
      
      for( U32 i = 0; i < mGroups.size(); ++ i )
      {
         mGroups[ i ]->write( stream, 0 );
         stream.writeText( "\n" );
      }

      // Write the event objects along with their
      // SFXDescriptions.
      
      for( U32 i = 0; i < mEvents.size(); ++ i )
      {
         mEvents[ i ]->getDescription()->write( stream, 0 );
         mEvents[ i ]->write( stream, 0 );
         stream.writeText( "\n" );
      }
      
      Con::printf( "SFXFMODProject - %s: Generated event script '%s'", getName(), eventScriptFileName.getFullPath().c_str() );
   }

   return true;
}

//-----------------------------------------------------------------------------

void SFXFMODProject::acquire( bool recursive )
{      
   // Load the project file.
   
   if( !mHandle )
   {
      FMOD_RESULT result = SFXFMODDevice::smFunc->FMOD_EventSystem_Load(
         SFXFMODDevice::smEventSystem,
         mFileName.c_str(),
         ( FMOD_EVENT_LOADINFO* ) 0,
         &mHandle
      );
      
      if( result != FMOD_OK )
      {
         Con::errorf( "SFXFMODProject::acquire - could not load '%s' (%s)",
            mFileName.c_str(), FMODResultToString( result ).c_str() );
         mHandle = NULL;
         return;
      }
      
      Con::printf( "SFXFMODProject - %s: Opened project '%s'", getName(), mFileName.c_str() );
      
      // Set the media path.
      
      String mediaPath;
      if( !mMediaPath.isEmpty() )
      {
         mediaPath = mMediaPath;
         if( mediaPath[ mediaPath.length() - 1 ] != '/' )
            mediaPath += '/';
      }
      else
      {
         // Set to project directory.
         
         Torque::Path path = mFileName;
         if( path.getRoot().isEmpty() )
            path.setRoot( "game" );
         path.setFileName( "" );
         path.setExtension( "" );       
         
         mediaPath = path.getFullPath() + '/';
      }

      SFXFMODDevice::smFunc->FMOD_EventSystem_SetMediaPath(
         SFXFMODDevice::smEventSystem,
         mediaPath.c_str()
      );
   }
   
   // Acquire the root groups.
   
   if( recursive )
      for( SFXFMODEventGroup* group = mRootGroups; group != NULL; group = group->mSibling )
         group->acquire( true );
         
   SFXFMODDevice::instance()->updateMemUsageStats();
}

//-----------------------------------------------------------------------------

void SFXFMODProject::release()
{
   if( !mHandle )
      return;
      
   Con::printf( "SFXFMODProject - %s: Closing project '%s'",
      getName(), mFileName.c_str() );
   
   // Clear media path.
   
   SFXFMODDevice::smFunc->FMOD_EventSystem_SetMediaPath(
      SFXFMODDevice::smEventSystem, "" );
      
   // Release the root groups.
   
   for( SFXFMODEventGroup* group = mRootGroups; group != NULL; group = group->mSibling )
      group->release();
 
   // Release the project.
   
   SFXFMODDevice::smFunc->FMOD_EventProject_Release( mHandle );
   mHandle = NULL;

   SFXFMODDevice::instance()->updateMemUsageStats();
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_addEvent( SFXFMODEvent* event )
{
   mEvents.push_back( event );
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_addGroup( SFXFMODEventGroup* group )
{
   mGroups.push_back( group );
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_removeEvent( SFXFMODEvent* event )
{
   for( U32 i = 0; i < mEvents.size(); ++ i )
      if( mEvents[ i ] == event )
      {
         mEvents.erase( i );
         break;
      }
}

//-----------------------------------------------------------------------------

void SFXFMODProject::_removeGroup( SFXFMODEventGroup* group )
{
   // Remove from group array.
   
   for( U32 i = 0; i < mGroups.size(); ++ i )
      if( mGroups[ i ] == group )
      {
         mGroups.erase( i );
         break;;
      }
      
   // Unlink if it's a root group.
   
   if( !group->mParent )
   {
      if( group == mRootGroups )
      {
         mRootGroups = group->mSibling;
         group->mSibling = NULL;
      }
      else
      {
         SFXFMODEventGroup* p = mRootGroups;
         while( p && p->mSibling != group )
            p = p->mSibling;
            
         if( p )
         {
            p->mSibling = group->mSibling;
            group->mSibling = NULL;
         }
      }
   }
}
