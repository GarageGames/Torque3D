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

// Functions dealing with connecting to a server


//-----------------------------------------------------------------------------
// Server connection error
//-----------------------------------------------------------------------------

addMessageCallback( 'MsgConnectionError', handleConnectionErrorMessage );

function handleConnectionErrorMessage(%msgType, %msgString, %msgError)
{
   // On connect the server transmits a message to display if there
   // are any problems with the connection.  Most connection errors
   // are game version differences, so hopefully the server message
   // will tell us where to get the latest version of the game.
   $ServerConnectionErrorMessage = %msgError;
}


//----------------------------------------------------------------------------
// GameConnection client callbacks
//----------------------------------------------------------------------------

function GameConnection::initialControlSet(%this)
{
   echo ("*** Initial Control Object");

   // The first control object has been set by the server
   // and we are now ready to go.
   
   // first check if the editor is active
   if (!isToolBuild() || !Editor::checkActiveLoadDone())
   {
      if (Canvas.getContent() != PlayGui.getId())
         Canvas.setContent(PlayGui);
   }
}

function GameConnection::onControlObjectChange(%this)
{
   echo ("*** Control Object Changed");
   
   // Reset the current FOV to match the new object
   // and turn off any current zoom.
   resetCurrentFOV();
   turnOffZoom();
}

// Called on the new connection object after connect() succeeds.
function GameConnection::onConnectionAccepted(%this)
{
   // Startup the physX world on the client before any
   // datablocks and objects are ghosted over.
   physicsInitWorld( "client" );   
}

function GameConnection::onConnectionError(%this, %msg)
{
   // General connection error, usually raised by ghosted objects
   // initialization problems, such as missing files.  We'll display
   // the server's connection error message.
   disconnectedCleanup();
   MessageBoxOK( "DISCONNECT", $ServerConnectionErrorMessage @ " (" @ %msg @ ")" );
}

//-----------------------------------------------------------------------------
// Disconnect
//-----------------------------------------------------------------------------

function disconnect()
{
   // We need to stop the client side simulation
   // else physics resources will not cleanup properly.
   physicsStopSimulation( "client" );

   // Delete the connection if it's still there.
   if (isObject(ServerConnection))
      ServerConnection.delete();
      
   disconnectedCleanup();

   // Call destroyServer in case we're hosting
   destroyServer();
}

function disconnectedCleanup()
{
   // End mission, if it's running.
   
   if( $Client::missionRunning )
      clientEndMission();
      
   // Disable mission lighting if it's going, this is here
   // in case we're disconnected while the mission is loading.
   
   $lightingMission = false;
   $sceneLighting::terminateLighting = true;

   // Back to the launch screen
   if (isObject( MainMenuGui ))
      Canvas.setContent( MainMenuGui );

   // Before we destroy the client physics world
   // make sure all ServerConnection objects are deleted.
   if(isObject(ServerConnection))
   {
      ServerConnection.deleteAllObjects();
   }
   
   // We can now delete the client physics simulation.
   physicsDestroyWorld( "client" );                 
}
