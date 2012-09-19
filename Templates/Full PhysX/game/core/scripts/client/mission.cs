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


// Whether the local client is currently running a mission.
$Client::missionRunning = false;

// Sequence number for currently running mission.
$Client::missionSeq = -1;


// Called when mission is started.
function clientStartMission()
{
   // The client recieves a mission start right before
   // being dropped into the game.
   physicsStartSimulation( "client" );
   
   // Start game audio effects channels.
   
   AudioChannelEffects.play();
   
   // Create client mission cleanup group.
      
   new SimGroup( ClientMissionCleanup );

   // Done.
      
   $Client::missionRunning = true;
}

// Called when mission is ended (either through disconnect or
// mission end client command).
function clientEndMission()
{
   // Stop physics simulation on client.
   physicsStopSimulation( "client" );

   // Stop game audio effects channels.
   
   AudioChannelEffects.stop();
   
   // Delete all the decals.
   decalManagerClear();
  
   // Delete client mission cleanup group. 
   if( isObject( ClientMissionCleanup ) )
      ClientMissionCleanup.delete();
      
   clearClientPaths();
      
   // Done.
   $Client::missionRunning = false;
}

//----------------------------------------------------------------------------
// Mission start / end events sent from the server
//----------------------------------------------------------------------------

function clientCmdMissionStart(%seq)
{
   clientStartMission();
   $Client::missionSeq = %seq;
}

function clientCmdMissionEnd( %seq )
{
   if( $Client::missionRunning && $Client::missionSeq == %seq )
   {
      clientEndMission();
      $Client::missionSeq = -1;
   }
}

/// Expands the name of a mission into the full 
/// mission path and extension.
function expandMissionFileName( %missionFile )
{         
   // Expand any escapes in it.
   %missionFile = expandFilename( %missionFile );
                 
   // If the mission file doesn't exist... try to fix up the string.      
   if ( !isFile( %missionFile ) ) 
   {
      // Does it need a .mis?
      if ( strStr( %missionFile, ".mis" ) == -1 )
         %newMission = %missionFile @ ".mis";

      if ( !isFile( %newMission ) )
      {
         // Attach a path to it.
         %newMission = expandFilename( "levels/" @ %newMission );
         if ( !isFile( %newMission ) )
         {
            warn( "The mission file '" @ %missionFile @ "' was not found!" );
            return "";
         }
      }

      %missionFile = %newMission;
   }
   
   return %missionFile;
}

/// Load a single player level on the local server.
function loadLevel( %missionNameOrFile )
{
   // Expand the mission name... this allows you to enter
   // just the name and not the full path and extension.
   %missionFile = expandMissionFileName( %missionNameOrFile );
   if ( %missionFile $= "" )
      return false;

   // Show the loading screen immediately.
   if ( isObject( LoadingGui ) )
   {
      Canvas.setContent("LoadingGui");
      LoadingProgress.setValue(1);
      LoadingProgressTxt.setValue("LOADING MISSION FILE");
      Canvas.repaint();
   }

   // Prepare and launch the server.
   return createAndConnectToLocalServer( "SinglePlayer", %missionFile );
}
