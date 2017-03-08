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
function DefaultGame::initGameVars(%this)
{
   // Leave $Game::defaultPlayerClass and $Game::defaultPlayerDataBlock as empty strings ("")
   // to spawn a the $Game::defaultCameraClass as the control object.
   $Game::DefaultPlayerClass = "";
   $Game::DefaultPlayerDataBlock = "";
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
}

//-----------------------------------------------------------------------------
// DefaultGame manages the communication between the server's world and the
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
function DefaultGame::onClientEnterGame(%this, %client)
{
   // This function currently relies on some helper functions defined in
   // core/scripts/spawn.cs. For custom spawn behaviors one can either
   // override the properties on the SpawnSphere's or directly override the
   // functions themselves.

   // Find a spawn point for the camera
   %cameraSpawnPoint = %this.pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);
   
   // Spawn a camera for this client using the found %spawnPoint
   %this.spawnCamera(%cameraSpawnPoint, %client);
}

//-----------------------------------------------------------------------------
// Clean up the client's control objects
//-----------------------------------------------------------------------------
function DefaultGame::onClientLeaveGame(%this, %client)
{
   // Cleanup the camera
   if (isObject(%this.camera))
      %this.camera.delete();
}

//-----------------------------------------------------------------------------
// The server has started up so do some game start up
//-----------------------------------------------------------------------------
function DefaultGame::onMissionStart(%this)
{
   //set up the game and game variables
   %this.initGameVars();

   $Game::Duration = %this.duration;
}

function DefaultGame::onMissionEnded(%this)
{
   cancel($Game::Schedule);
   $Game::Running = false;
   $Game::Cycling = false;
}

function DefaultGame::onMissionReset(%this)
{
   // Called by resetMission(), after all the temporary mission objects
   // have been deleted.
   %this.initGameVars();

   $Game::Duration = %this.duration;
}

//-----------------------------------------------------------------------------
// Functions that implement game-play
// These are here for backwards compatibilty only, games and/or mods should
// really be overloading the server and mission functions listed ubove.
//-----------------------------------------------------------------------------
function DefaultGame::pickCameraSpawnPoint(%this, %spawnGroups)
{
   // Walk through the groups until we find a valid object
   for (%i = 0; %i < getWordCount(%spawnGroups); %i++)
   {
      %group = getWord(%spawnGroups, %i);
      
      %count = getWordCount(%group);

      if (isObject(%group))
         %spawnPoint = %group.getRandom();

      if (isObject(%spawnPoint))
         return %spawnPoint;
   }

   // Didn't find a spawn point by looking for the groups
   // so let's return the "default" SpawnSphere
   // First create it if it doesn't already exist
   if (!isObject(DefaultCameraSpawnSphere))
   {
      %spawn = new SpawnSphere(DefaultCameraSpawnSphere)
      {
         dataBlock      = "SpawnSphereMarker";
         spawnClass     = $Game::DefaultCameraClass;
         spawnDatablock = $Game::DefaultCameraDataBlock;
      };

      // Add it to the MissionCleanup group so that it
      // doesn't get saved to the Mission (and gets cleaned
      // up of course)
      MissionCleanup.add(%spawn);
   }

   return DefaultCameraSpawnSphere;
}

function DefaultGame::spawnCamera(%this, %spawnPoint, %client)
{
   // Set the control object to the default camera
   if (!isObject(%client.camera))
   {
      %camObj = spawnObject(Camera, Observer);
      %client.camera = %camObj;
   }

   // If we have a camera then set up some properties
   if (isObject(%client.camera))
   {
      MissionCleanup.add( %client.camera );
      %client.camera.scopeToClient(%client);

      %client.setControlObject(%client.camera);

      if (isDefined("%spawnPoint"))
      {
         // Attempt to treat %spawnPoint as an object
         if (getWordCount(%spawnPoint) == 1 && isObject(%spawnPoint))
         {
            %client.camera.setTransform(%spawnPoint.getTransform());
         }
         else
         {
            // Treat %spawnPoint as an AxisAngle transform
            %client.camera.setTransform(%spawnPoint);
         }
      }
   }
}