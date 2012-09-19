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
#include "T3D/physics/physicsPlugin.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/simSet.h"
#include "core/strings/stringFunctions.h"
#include "scene/sceneObject.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "T3D/physics/physicsObject.h"
#include "T3D/physics/physicsWorld.h"
#include "core/util/tNamedFactory.h"


PhysicsPlugin* PhysicsPlugin::smSingleton = NULL;
PhysicsResetSignal PhysicsPlugin::smPhysicsResetSignal;
bool PhysicsPlugin::smSinglePlayer = false;
U32 PhysicsPlugin::smThreadCount = 2;


String PhysicsPlugin::smServerWorldName( "server" );
String PhysicsPlugin::smClientWorldName( "client" );

AFTER_MODULE_INIT( Sim )
{
   Con::addVariable( "$Physics::isSinglePlayer", TypeBool, &PhysicsPlugin::smSinglePlayer, 
      "@brief Informs the physics simulation if only a single player exists.\n\n"
      "If true, optimizations will be implemented to better cater to a single player environmnent.\n\n"
	   "@ingroup Physics\n");
   Con::addVariable( "$pref::Physics::threadCount", TypeS32, &PhysicsPlugin::smThreadCount, 
      "@brief Number of threads to use in a single pass of the physics engine.\n\n"
      "Defaults to 2 if not set.\n\n"
	   "@ingroup Physics\n");
}

bool PhysicsPlugin::activate( const char *library )
{
   // Cleanup any previous plugin.
   if ( smSingleton )
   {
      smSingleton->destroyPlugin();
      AssertFatal( smSingleton == NULL, 
         "PhysicsPlugin::activate - destroyPlugin didn't delete the plugin!" );
   }

   // Create it thru the factory.
   PhysicsPlugin *plugin = NamedFactory<PhysicsPlugin>::create( library );
   if ( !plugin )
   {
      // One last try... try the first available one.
      plugin = NamedFactory<PhysicsPlugin>::create();
      if ( !plugin )
         return false;
   }

   smSingleton = plugin;
   return true;
}

PhysicsPlugin::PhysicsPlugin()
{
   mPhysicsCleanup = new SimSet();
   mPhysicsCleanup->assignName( "PhysicsCleanupSet" );
   mPhysicsCleanup->registerObject();
   Sim::getRootGroup()->addObject( mPhysicsCleanup );   
}

PhysicsPlugin::~PhysicsPlugin()
{
   AssertFatal( smSingleton == this, "PhysicsPlugin::~PhysicsPlugin() - Wrong active plugin!" );
   
   if ( mPhysicsCleanup )
      mPhysicsCleanup->deleteObject();

   smSingleton = NULL;
}

void PhysicsPlugin::enableDebugDraw( bool enabled )
{
   if ( enabled )
      SceneManager::getPostRenderSignal().notify( &PhysicsPlugin::_debugDraw );
   else
      SceneManager::getPostRenderSignal().remove( &PhysicsPlugin::_debugDraw );

   _onDebugDrawEnabled( enabled );
}

void PhysicsPlugin::_debugDraw( SceneManager *graph, const SceneRenderState *state )
{
   // We only debug draw in the diffuse pass if we have a physics object.
   if ( !PHYSICSMGR || !state->isDiffusePass() )
      return;

   // Render the server by default... else the client.
   PhysicsWorld *world = PHYSICSMGR->getWorld( smServerWorldName );
   if ( !world )
      world = PHYSICSMGR->getWorld( smClientWorldName );

   if ( world )
      world->onDebugDraw( state );
}

ConsoleFunction( physicsPluginPresent, bool, 1, 1, "physicsPluginPresent()\n"
   "@brief Returns true if a physics plugin exists and is initialized.\n\n"
   "@ingroup Physics" )
{
   return PHYSICSMGR != NULL;
}

ConsoleFunction( physicsInit, bool, 1, 2, "physicsInit( [string library] )" )
{
   const char *library = "default";
   if ( argc > 1 )
      library = argv[1];

   return PhysicsPlugin::activate( library );
}

ConsoleFunction( physicsDestroy, void, 1, 1, "physicsDestroy()" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->destroyPlugin();
}

ConsoleFunction( physicsInitWorld, bool, 2, 2, "physicsInitWorld( String worldName )" )
{
   return PHYSICSMGR && PHYSICSMGR->createWorld( String( argv[1] ) );
}

ConsoleFunction( physicsDestroyWorld, void, 2, 2, "physicsDestroyWorld( String worldName )" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->destroyWorld( String( argv[1] ) );
}


// Control/query of the stop/started state
// of the currently running simulation.
ConsoleFunction( physicsStartSimulation, void, 2, 2, "physicsStartSimulation( String worldName )" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->enableSimulation( String( argv[1] ), true );
}

ConsoleFunction( physicsStopSimulation, void, 2, 2, "physicsStopSimulation( String worldName )" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->enableSimulation( String( argv[1] ), false );
}

ConsoleFunction( physicsSimulationEnabled, bool, 1, 1, "physicsSimulationEnabled()" )
{
   return PHYSICSMGR && PHYSICSMGR->isSimulationEnabled();
}

// Used for slowing down time on the
// physics simulation, and for pausing/restarting
// the simulation.
ConsoleFunction( physicsSetTimeScale, void, 2, 2, "physicsSetTimeScale( F32 scale )" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->setTimeScale( dAtof( argv[1] ) );
}

// Get the currently set time scale.
ConsoleFunction( physicsGetTimeScale, F32, 1, 1, "physicsGetTimeScale()" )
{
   return PHYSICSMGR && PHYSICSMGR->getTimeScale();
}

// Used to send a signal to objects in the
// physics simulation that they should store
// their current state for later restoration,
// such as when the editor is closed.
ConsoleFunction( physicsStoreState, void, 1, 1, "physicsStoreState()" )
{
   PhysicsPlugin::getPhysicsResetSignal().trigger( PhysicsResetEvent_Store );
}

// Used to send a signal to objects in the
// physics simulation that they should restore
// their saved state, such as when the editor is opened.
ConsoleFunction( physicsRestoreState, void, 1, 1, "physicsRestoreState()" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->reset();
}

ConsoleFunction( physicsDebugDraw, void, 2, 2, "physicsDebugDraw( bool enable )" )
{
   if ( PHYSICSMGR )
      PHYSICSMGR->enableDebugDraw( dAtoi( argv[1] ) );
}