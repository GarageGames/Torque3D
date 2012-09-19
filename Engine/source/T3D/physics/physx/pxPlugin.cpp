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
#include "console/consoleTypes.h"
#include "T3D/physics/physX/pxPlugin.h"

#include "T3D/physics/physicsShape.h"
#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physX/pxBody.h"
#include "T3D/physics/physX/pxPlayer.h"
#include "T3D/physics/physX/pxCollision.h"
#include "T3D/gameBase/gameProcess.h"
#include "core/util/tNamedFactory.h"


extern bool gPhysXLogWarnings;

AFTER_MODULE_INIT( Sim )
{
   NamedFactory<PhysicsPlugin>::add( "PhysX", &PxPlugin::create );

   #if defined(TORQUE_OS_WIN32) || defined(TORQUE_OS_XBOX) || defined(TORQUE_OS_XENON)   
      NamedFactory<PhysicsPlugin>::add( "default", &PxPlugin::create );
   #endif   

   Con::addVariable( "$PhysXLogWarnings", TypeBool, &gPhysXLogWarnings, 
      "@brief Output PhysX warnings to the console.\n\n"
	   "@ingroup Physics\n");
}


PhysicsPlugin* PxPlugin::create()
{
   // Only create the plugin if it hasn't been set up AND
   // the PhysX world is successfully initialized.
   bool success = PxWorld::restartSDK( false );
   if ( success )
      return new PxPlugin();

   return NULL;
}

PxPlugin::PxPlugin()
{
}

PxPlugin::~PxPlugin()
{
}

void PxPlugin::destroyPlugin()
{
   // Cleanup any worlds that are still kicking.
   Map<StringNoCase, PhysicsWorld*>::Iterator iter = mPhysicsWorldLookup.begin();
   for ( ; iter != mPhysicsWorldLookup.end(); iter++ )
   {
      iter->value->destroyWorld();
      delete iter->value;
   }
   mPhysicsWorldLookup.clear();

   PxWorld::restartSDK( true );

   delete this;
}

void PxPlugin::reset()
{
   // First delete all the cleanup objects.
   if ( getPhysicsCleanup() )
      getPhysicsCleanup()->deleteAllObjects();

   getPhysicsResetSignal().trigger( PhysicsResetEvent_Restore );

   // Now let each world reset itself.
   Map<StringNoCase, PhysicsWorld*>::Iterator iter = mPhysicsWorldLookup.begin();
   for ( ; iter != mPhysicsWorldLookup.end(); iter++ )
      iter->value->reset();
}

PhysicsCollision* PxPlugin::createCollision()
{
   return new PxCollision();
}

PhysicsBody* PxPlugin::createBody()
{
   return new PxBody();
}

PhysicsPlayer* PxPlugin::createPlayer()
{
   return new PxPlayer();
}

bool PxPlugin::isSimulationEnabled() const
{
   bool ret = false;
   PxWorld *world = static_cast<PxWorld*>( getWorld( smClientWorldName ) );
   if ( world )
   {
      ret = world->getEnabled();
      return ret;
   }

   world = static_cast<PxWorld*>( getWorld( smServerWorldName ) );
   if ( world )
   {
      ret = world->getEnabled();
      return ret;
   }

   return ret;
}

void PxPlugin::enableSimulation( const String &worldName, bool enable )
{
   PxWorld *world = static_cast<PxWorld*>( getWorld( worldName ) );
   if ( world )
      world->setEnabled( enable );
}

void PxPlugin::setTimeScale( const F32 timeScale )
{
   // Grab both the client and
   // server worlds and set their time
   // scales to the passed value.
   PxWorld *world = static_cast<PxWorld*>( getWorld( smClientWorldName ) );
   if ( world )
      world->setEditorTimeScale( timeScale );

   world = static_cast<PxWorld*>( getWorld( smServerWorldName ) );
   if ( world )
      world->setEditorTimeScale( timeScale );
}

const F32 PxPlugin::getTimeScale() const
{
   // Grab both the client and
   // server worlds and call 
   // setEnabled( true ) on them.
   PxWorld *world = static_cast<PxWorld*>( getWorld( smClientWorldName ) );
   if ( !world )
   {
      world = static_cast<PxWorld*>( getWorld( smServerWorldName ) );
      if ( !world )
         return 0.0f;
   }
   
   return world->getEditorTimeScale();
}

bool PxPlugin::createWorld( const String &worldName )
{
   Map<StringNoCase, PhysicsWorld*>::Iterator iter = mPhysicsWorldLookup.find( worldName );
   PhysicsWorld *world = NULL;
   
   iter != mPhysicsWorldLookup.end() ? world = (*iter).value : world = NULL; 

   if ( world ) 
   {
      Con::errorf( "PxPlugin::createWorld - %s world already exists!", worldName.c_str() );
      return false;
   }

   world = new PxWorld();
   
   if ( worldName.equal( smClientWorldName, String::NoCase ) )
      world->initWorld( false, ClientProcessList::get() );
   else
      world->initWorld( true, ServerProcessList::get() );

   mPhysicsWorldLookup.insert( worldName, world );

   return world != NULL;
}

void PxPlugin::destroyWorld( const String &worldName )
{
   Map<StringNoCase, PhysicsWorld*>::Iterator iter = mPhysicsWorldLookup.find( worldName );
   if ( iter == mPhysicsWorldLookup.end() )
      return;

   PhysicsWorld *world = (*iter).value;
   world->destroyWorld();
   delete world;
   
   mPhysicsWorldLookup.erase( iter );
}

PhysicsWorld* PxPlugin::getWorld( const String &worldName ) const
{
   if ( mPhysicsWorldLookup.isEmpty() )
      return NULL;

   Map<StringNoCase, PhysicsWorld*>::ConstIterator iter = mPhysicsWorldLookup.find( worldName );

   return iter != mPhysicsWorldLookup.end() ? (*iter).value : NULL;
}

PhysicsWorld* PxPlugin::getWorld() const
{
   if ( mPhysicsWorldLookup.size() == 0 )
      return NULL;

   Map<StringNoCase, PhysicsWorld*>::ConstIterator iter = mPhysicsWorldLookup.begin();
   return iter->value;
}

U32 PxPlugin::getWorldCount() const
{ 
   return mPhysicsWorldLookup.size(); 
}

void PxPlugin::_onDebugDrawEnabled( bool enabled )
{   
   if ( !enabled )
      gPhysicsSDK->setParameter( NX_VISUALIZATION_SCALE, 0.0f );
}

ConsoleFunction( physXRemoteDebuggerConnect, bool, 1, 3, "" )
{
   if ( !gPhysicsSDK )  
   {
      Con::errorf( "PhysX SDK not initialized!" );
      return false;
   }

   NxRemoteDebugger *debugger = gPhysicsSDK->getFoundationSDK().getRemoteDebugger();

   if ( debugger->isConnected() )
   {
      Con::errorf( "RemoteDebugger already connected... call disconnect first!" );
      return false;
   }

   const UTF8 *host = "localhost";
   U32 port = 5425;

   if ( argc >= 2 )
      host = argv[1];
   if ( argc >= 3 )
      port = dAtoi( argv[2] );

   // Before we connect we need to have write access
   // to both the client and server worlds.
   PxWorld::releaseWriteLocks();

   // Connect!
   debugger->connect( host, port );
   if ( !debugger->isConnected() )
   {
      Con::errorf( "RemoteDebugger failed to connect!" );
      return false;
   }

   Con::printf( "RemoteDebugger connected to %s at port %u!", host, port );
   return true;
}

ConsoleFunction( physXRemoteDebuggerDisconnect, void, 1, 1, "" )
{
   if ( !gPhysicsSDK )  
   {
      Con::errorf( "PhysX SDK not initialized!" );
      return;
   }

   NxRemoteDebugger *debugger = gPhysicsSDK->getFoundationSDK().getRemoteDebugger();

   if ( debugger->isConnected() )
   {
      debugger->flush();
      debugger->disconnect();
      Con::printf( "RemoteDebugger disconnected!" );
   }
}
