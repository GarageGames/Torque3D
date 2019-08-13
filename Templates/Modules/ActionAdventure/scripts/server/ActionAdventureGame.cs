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
// What kind of "player" is spawned is either controlled directly by the
// SpawnSphere or it defaults back to the values set here. This also controls
// which SimGroups to attempt to select the spawn sphere's from by walking down
// the list of SpawnGroups till it finds a valid spawn object.
// These override the values set in core/scripts/server/spawn.cs
//-----------------------------------------------------------------------------

function ActionAdventureGame::initGameVars(%game)
{
	// Leave $Game::defaultPlayerClass and $Game::defaultPlayerDataBlock as empty strings ("")
	// to spawn a the $Game::defaultCameraClass as the control object.
	$Game::DefaultPlayerClass        = "Player";
	$Game::DefaultPlayerDataBlock    = "DefaultPlayerData";
	$Game::DefaultPlayerSpawnGroups = "CameraSpawnPoints PlayerSpawnPoints PlayerDropPoints";

	//-----------------------------------------------------------------------------
	// What kind of "camera" is spawned is either controlled directly by the
	// SpawnSphere or it defaults back to the values set here. This also controls
	// which SimGroups to attempt to select the spawn sphere's from by walking down
	// the list of SpawnGroups till it finds a valid spawn object.
	// These override the values set in core/scripts/server/spawn.cs
	//-----------------------------------------------------------------------------
	$Game::DefaultCameraClass = "Camera";
	$Game::DefaultCameraDataBlock = "Observer";
	$Game::DefaultCameraSpawnGroups = "CameraSpawnPoints PlayerSpawnPoints PlayerDropPoints";

	// Global movement speed that affects all Cameras
	$Camera::MovementSpeed = 30;
  %game.allowCycling = false;   // Is mission cycling allowed?
}

function ActionAdventureGame::onGameDurationEnd(%game)
{
   // This "redirect" is here so that we can abort the game cycle if
   // the $Game::Duration variable has been cleared, without having
   // to have a function to cancel the schedule.

   if ($Game::Duration && !(EditorIsActive() && GuiEditorIsActive()))
      Game.onGameDurationEnd();
}

//-----------------------------------------------------------------------------
// GameConnection manages the communication between the server's world and the
// client's simulation. These functions are responsible for maintaining the
// client's camera and player objects.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This is the main entry point for spawning a control object for the client.
// The control object is the actual game object that the client is responsible
// for controlling in the client and server simulations. We also spawn a
// convenient camera object for use as an alternate control object. We do not
// have to spawn this camera object in order to function in the simulation.
//
// Called for each client after it's finished downloading the mission and is
// ready to start playing.
//-----------------------------------------------------------------------------
function ActionAdventureGame::onClientEnterGame(%this, %client)
{
   // This function currently relies on some helper functions defined in
   // core/scripts/spawn.cs. For custom spawn behaviors one can either
   // override the properties on the SpawnSphere's or directly override the
   // functions themselves.

  // Sync the client's clocks to the server's
   commandToClient(%client, 'SyncClock', $Sim::Time - $Game::StartTime);
   
   //Set the player name based on the client's connection data
   %client.setPlayerName(%client.connectData);
   
   //Ubiq:
   //-----------------------------------------------
   //create a cameraGoalFollower
   %client.cameraGoalFollower = new CameraGoalFollower() {
      dataBlock = CameraGoalFollowerDB;
   };
   MissionCleanup.add( %client.cameraGoalFollower );
   %client.cameraGoalFollower.scopeToClient(%client);
   //-----------------------------------------------
   //create a cameraGoalPlayer
   %client.cameraGoalPlayer = new CameraGoalPlayer() {
      dataBlock = CameraGoalPlayerDB;
   };
   MissionCleanup.add( %client.cameraGoalPlayer );
   %client.cameraGoalPlayer.scopeToClient(%client);
   //-----------------------------------------------
   //create a cameraGoalPath
   %client.cameraGoalPath = new CameraGoalPath() {
      dataBlock = CameraGoalPathDB;
   };
   MissionCleanup.add( %client.cameraGoalPath );
   %client.cameraGoalPath.scopeToClient(%client);
   //-----------------------------------------------

   // Find a spawn point for the camera
   %cameraSpawnPoint = pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);
   // Spawn a camera for this client using the found %spawnPoint
   %client.spawnCamera(%cameraSpawnPoint);

   // Find a spawn point for the player
   %playerSpawnPoint = pickPlayerSpawnPoint($Game::DefaultPlayerSpawnGroups);
   // Spawn a camera for this client using the found %spawnPoint
   %client.spawnPlayer(%playerSpawnPoint);
}

//-----------------------------------------------------------------------------
// Clean up the client's control objects
//-----------------------------------------------------------------------------
function ActionAdventureGame::onClientLeaveGame(%this)
{
   // Cleanup the camera
   if (isObject(%this.camera))
      %this.camera.delete();
   // Cleanup the player
   if (isObject(%this.player))
      %this.player.delete();
}

//-----------------------------------------------------------------------------
// Handle a player's death
//-----------------------------------------------------------------------------
function ActionAdventureGame::onDeath(%this, %sourceObject, %sourceClient, %damageType, %damLoc)
{
   // Clear out the name on the corpse
   if (isObject(%this.player))
   {
      if (%this.player.isMethod("setShapeName"))
         %this.player.setShapeName("");
   }

    // Switch the client over to the death cam
    if (isObject(%this.camera) && isObject(%this.player))
    {
        %this.camera.setMode("Corpse", %this.player);
        %this.setControlObject(%this.camera);
    }

    // Unhook the player object
    //Ubiq: Lorne: we need to keep this reference so we can delete the corpse when respawning
    //%this.player = 0;
}

//-----------------------------------------------------------------------------
// The server has started up so do some game start up
//-----------------------------------------------------------------------------
function ActionAdventureGame::onMissionStart(%this)
{

  //set up the game and game variables
   %this.initGameVars();
   
   //echo (%game @"\c4 -> "@ %game.class @" -> GameCore::onStartGame");
   if ($Game::Running)
   {
      error("startGame: End the game first!");
      return;
   }

   // Inform the client we're starting up
   for (%clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++)
   {
      %cl = ClientGroup.getObject(%clientIndex);
      commandToClient(%cl, 'GameStart');

      // Other client specific setup..
      %cl.score = 0;
      %cl.kills = 0;
      %cl.deaths = 0;
   }
      
   $Game::Running = true;
   
   $Game = %this;
}

function ActionAdventureGame::onMissionEnded(%this)
{
   if (!$Game::Running)
   {
      error("endGame: No game running!");
      return;
   }

   // Stop any game timers
   cancel($Game::Schedule);

   for (%clientIndex = 0; %clientIndex < ClientGroup.getCount(); %clientIndex++)
   {
      %cl = ClientGroup.getObject(%clientIndex);
      commandToClient(%cl, 'GameEnd', $Game::EndGamePause);
   }

   $Game::Running = false;
   $Game::Cycling = false;
   $Game = "";
}

function ActionAdventureGame::onMissionReset(%this)
{
   // Called by resetMission(), after all the temporary mission objects
   // have been deleted.
    %this.initGameVars();
}

// ----------------------------------------------------------------------------
// Server
// ----------------------------------------------------------------------------

// Called by GameCore::cycleGame() when we need to destroy the server
// because we're done playing.  We don't want to call destroyServer()
// directly so we can first check that we're about to destroy the
// correct server session.
function ActionAdventureGame::DestroyServer(%serverSession)
{
   if (%serverSession == $Server::Session)
   {
      if (isObject(LocalClientConnection))
      {
         // We're a local connection so issue a disconnect.  The server will
         // be automatically destroyed for us.
         disconnect();
      }
      else
      {
         // We're a stand alone server
         destroyServer();
      }
   }
}