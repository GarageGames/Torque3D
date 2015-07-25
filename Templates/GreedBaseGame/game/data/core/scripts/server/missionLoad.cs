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

//-----------------------------------------------------------------------------
// Server mission loading
//-----------------------------------------------------------------------------

// On every mission load except the first, there is a pause after
// the initial mission info is downloaded to the client.
$MissionLoadPause = 5000;

//-----------------------------------------------------------------------------

function loadMission( %missionName, %isFirstMission ) 
{
   endMission();
   echo("*** LOADING MISSION: " @ %missionName);
   echo("*** Stage 1 load");

   // Reset all of these
   if (isFunction("clearCenterPrintAll"))
      clearCenterPrintAll();
   if (isFunction("clearBottomPrintAll"))
      clearBottomPrintAll();

   // increment the mission sequence (used for ghost sequencing)
   $missionSequence++;
   $missionRunning = false;
   $Server::MissionFile = %missionName;
   $Server::LoadFailMsg = "";

   // Extract mission info from the mission file,
   // including the display name and stuff to send
   // to the client.
   buildLoadInfo( %missionName );

   // Download mission info to the clients
   %count = ClientGroup.getCount();
   for( %cl = 0; %cl < %count; %cl++ ) {
      %client = ClientGroup.getObject( %cl );
      if (!%client.isAIControlled())
         sendLoadInfoToClient(%client);
   }

   // Now that we've sent the LevelInfo to the clients
   // clear it so that it won't conflict with the actual
   // LevelInfo loaded in the level
   clearLoadInfo();

   // if this isn't the first mission, allow some time for the server
   // to transmit information to the clients:
   if( %isFirstMission || $Server::ServerType $= "SinglePlayer" )
      loadMissionStage2();
   else
      schedule( $MissionLoadPause, ServerGroup, loadMissionStage2 );
}

//-----------------------------------------------------------------------------

function loadMissionStage2() 
{
   echo("*** Stage 2 load");

   // Create the mission group off the ServerGroup
   $instantGroup = ServerGroup;

   // Make sure the mission exists
   %file = $Server::MissionFile;
   
   if( !isFile( %file ) )
   {
      $Server::LoadFailMsg = "Could not find mission \"" @ %file @ "\"";
   }
   else
   {
      // Calculate the mission CRC.  The CRC is used by the clients
      // to caching mission lighting.
      $missionCRC = getFileCRC( %file );

      // Exec the mission.  The MissionGroup (loaded components) is added to the ServerGroup
      exec(%file);

      if( !isObject(MissionGroup) )
      {
         $Server::LoadFailMsg = "No 'MissionGroup' found in mission \"" @ %file @ "\".";
      }
   }

   if( $Server::LoadFailMsg !$= "" )
   {
      // Inform clients that are already connected
      for (%clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++)
         messageClient(ClientGroup.getObject(%clientIndex), 'MsgLoadFailed', $Server::LoadFailMsg);    
      return;
   }

   // Set mission name.
   
   if( isObject( theLevelInfo ) )
      $Server::MissionName = theLevelInfo.levelName;

   // Mission cleanup group.  This is where run time components will reside.  The MissionCleanup
   // group will be added to the ServerGroup.
   new SimGroup( MissionCleanup );

   // Make the MissionCleanup group the place where all new objects will automatically be added.
   $instantGroup = MissionCleanup;
   
   // Construct MOD paths
   pathOnMissionLoadDone();

   // Mission loading done...
   echo("*** Mission loaded");
   
   // Start all the clients in the mission
   $missionRunning = true;
   for( %clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++ )
      ClientGroup.getObject(%clientIndex).loadMission();

   // Go ahead and launch the game
   onMissionLoaded();
}


//-----------------------------------------------------------------------------

function endMission()
{
   if (!isObject( MissionGroup ))
      return;

   echo("*** ENDING MISSION");
   
   // Inform the game code we're done.
   onMissionEnded();

   // Inform the clients
   for( %clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++ ) {
      // clear ghosts and paths from all clients
      %cl = ClientGroup.getObject( %clientIndex );
      %cl.endMission();
      %cl.resetGhosting();
      %cl.clearPaths();
   }
   
   // Delete everything
   MissionGroup.delete();
   MissionCleanup.delete();
   
   clearServerPaths();
}


//-----------------------------------------------------------------------------

function resetMission()
{
   echo("*** MISSION RESET");

   // Remove any temporary mission objects
   MissionCleanup.delete();
   $instantGroup = ServerGroup;
   new SimGroup( MissionCleanup );
   $instantGroup = MissionCleanup;

   clearServerPaths();
   //
   onMissionReset();
}
