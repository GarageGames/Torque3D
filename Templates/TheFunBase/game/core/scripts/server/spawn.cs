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
//-----------------------------------------------------------------------------

// Leave $Game::defaultPlayerClass and $Game::defaultPlayerDataBlock as empty strings ("")
// to spawn a the $Game::defaultCameraClass as the control object.
$Game::DefaultPlayerClass        = "Player";
$Game::DefaultPlayerDataBlock    = "DefaultPlayerData";
$Game::DefaultPlayerSpawnGroups  = "PlayerSpawnPoints";

//-----------------------------------------------------------------------------
// What kind of "camera" is spawned is either controlled directly by the
// SpawnSphere or it defaults back to the values set here. This also controls
// which SimGroups to attempt to select the spawn sphere's from by walking down
// the list of SpawnGroups till it finds a valid spawn object.
//-----------------------------------------------------------------------------
$Game::DefaultCameraClass        = "Camera";
$Game::DefaultCameraDataBlock    = "Observer";
$Game::DefaultCameraSpawnGroups  = "CameraSpawnPoints PlayerSpawnPoints";

//-----------------------------------------------------------------------------
// pickCameraSpawnPoint() is responsible for finding a valid spawn point for a
// camera.
//-----------------------------------------------------------------------------
function pickCameraSpawnPoint(%spawnGroups)
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

//-----------------------------------------------------------------------------
// pickPlayerSpawnPoint() is responsible for finding a valid spawn point for a
// player.
//-----------------------------------------------------------------------------
function pickPlayerSpawnPoint(%spawnGroups)
{
   // Walk through the groups until we find a valid object
   for (%i = 0; %i < getWordCount(%spawnGroups); %i++)
   {
      %group = getWord(%spawnGroups, %i);

      if (isObject(%group))
         %spawnPoint = %group.getRandom();

      if (isObject(%spawnPoint))
         return %spawnPoint;
   }

   // Didn't find a spawn point by looking for the groups
   // so let's return the "default" SpawnSphere
   // First create it if it doesn't already exist
   if (!isObject(DefaultPlayerSpawnSphere))
   {
      %spawn = new SpawnSphere(DefaultPlayerSpawnSphere)
      {
         dataBlock      = "SpawnSphereMarker";
         spawnClass     = $Game::DefaultPlayerClass;
         spawnDatablock = $Game::DefaultPlayerDataBlock;
      };

      // Add it to the MissionCleanup group so that it
      // doesn't get saved to the Mission (and gets cleaned
      // up of course)
      MissionCleanup.add(%spawn);
   }

   return DefaultPlayerSpawnSphere;
}

//-----------------------------------------------------------------------------
// GameConnection::spawnCamera() is responsible for spawning a camera for a
// client
//-----------------------------------------------------------------------------
//function GameConnection::spawnCamera(%this, %spawnPoint)
//{
   //// Set the control object to the default camera
   //if (!isObject(%this.camera))
   //{
      //if (isDefined("$Game::DefaultCameraClass"))
         //%this.camera = spawnObject($Game::DefaultCameraClass, $Game::DefaultCameraDataBlock);
   //}
//
   //if(!isObject(%this.PathCamera))
   //{
      //// Create path camera
      //%this.PathCamera = spawnObject("PathCamera", "LoopingCam");
      ////%this.PathCamera = new PathCamera() {
      ////dataBlock = LoopingCam;
      ////position = "0 0 300 1 0 0 0";
      ////};
   //}
   //if(isObject(%this.PathCamera))
   //{
      //%this.PathCamera.setPosition("-54.0187 1.81237 5.14039");
      //%this.PathCamera.followPath(MenuPath);
      //MissionCleanup.add( %this.PathCamera);
      //%this.PathCamera.scopeToClient(%this);
      //%this.setControlObject(%this.PathCamera);      
   //}
   //// If we have a camera then set up some properties
   //if (isObject(%this.camera))
   //{
      //MissionCleanup.add( %this.camera );
      //%this.camera.scopeToClient(%this);
      //
      ////%this.setControlObject(%this.camera);
      ////%this.setControlObject(%this.PathCamera);
//
      //if (isDefined("%spawnPoint"))
      //{
         //// Attempt to treat %spawnPoint as an object
         //if (getWordCount(%spawnPoint) == 1 && isObject(%spawnPoint))
         //{
            //%this.camera.setTransform(%spawnPoint.getTransform());
         //}
         //else
         //{
            //// Treat %spawnPoint as an AxisAngle transform
            //%this.camera.setTransform(%spawnPoint);
         //}
      //}
   //}
//}

function GameConnection::spawnCamera(%this, %spawnPoint)
{
   // Set the control object to the default camera
   if (!isObject(%this.camera))
   {
      if (isDefined("$Game::DefaultCameraClass"))
         %this.camera = spawnObject($Game::DefaultCameraClass, $Game::DefaultCameraDataBlock);
   }

   // If we have a camera then set up some properties
   if (isObject(%this.camera))
   {
      MissionCleanup.add( %this.camera );
      %this.camera.scopeToClient(%this);

      %this.setControlObject(%this.camera);

      if (isDefined("%spawnPoint"))
      {
         // Attempt to treat %spawnPoint as an object
         if (getWordCount(%spawnPoint) == 1 && isObject(%spawnPoint))
         {
            %this.camera.setTransform(%spawnPoint.getTransform());
         }
         else
         {
            // Treat %spawnPoint as an AxisAngle transform
            %this.camera.setTransform(%spawnPoint);
         }
      }
   }
}

//-----------------------------------------------------------------------------
// GameConnection::spawnPlayer() is responsible for spawning a player for a
// client
//-----------------------------------------------------------------------------
function GameConnection::spawnPlayer(%this, %spawnPoint, %noControl)
{
   if (isObject(%this.player))
   {
      // The client should not already have a player. Assigning
      // a new one could result in an uncontrolled player object.
      error("Attempting to create a player for a client that already has one!");
   }

   // Attempt to treat %spawnPoint as an object
   if (getWordCount(%spawnPoint) == 1 && isObject(%spawnPoint))
   {
      // Defaults
      %spawnClass      = $Game::DefaultPlayerClass;
      %spawnDataBlock  = $Game::DefaultPlayerDataBlock;

      // Overrides by the %spawnPoint
      if (isDefined("%spawnPoint.spawnClass"))
      {
         %spawnClass = %spawnPoint.spawnClass;
         %spawnDataBlock = %spawnPoint.spawnDatablock;
      }

      // This may seem redundant given the above but it allows
      // the SpawnSphere to override the datablock without
      // overriding the default player class
      if (isDefined("%spawnPoint.spawnDatablock"))
         %spawnDataBlock = %spawnPoint.spawnDatablock;

      %spawnProperties = %spawnPoint.spawnProperties;
      %spawnScript     = %spawnPoint.spawnScript;

      // Spawn with the engine's Sim::spawnObject() function
      %player = spawnObject(%spawnClass, %spawnDatablock, "",
                            %spawnProperties, %spawnScript);

      // If we have an object do some initial setup
      if (isObject(%player))
      {
         // Set the transform to %spawnPoint's transform
         %player.setTransform(%spawnPoint.getTransform());
      }
      else
      {
         // If we weren't able to create the player object then warn the user
         if (isDefined("%spawnDatablock"))
         {
               MessageBoxOK("Spawn Player Failed",
                             "Unable to create a player with class " @ %spawnClass @ 
                             " and datablock " @ %spawnDatablock @ ".\n\nStarting as an Observer instead.",
                             %this @ ".spawnCamera();");
         }
         else
         {
               MessageBoxOK("Spawn Player Failed",
                              "Unable to create a player with class " @ %spawnClass @
                              ".\n\nStarting as an Observer instead.",
                              %this @ ".spawnCamera();");
         }
      }
   }
   else
   {
      // Create a default player
      %player = spawnObject($Game::DefaultPlayerClass, $Game::DefaultPlayerDataBlock);
      
      if (!%player.isMemberOfClass("Player"))
         warn("Trying to spawn a class that does not derive from Player.");

      // Treat %spawnPoint as a transform
      %player.setTransform(%spawnPoint);
   }

   // If we didn't actually create a player object then bail
   if (!isObject(%player))
   {
      // Make sure we at least have a camera
      %this.spawnCamera(%spawnPoint);

      return;
   }

   // Update the default camera to start with the player
   if (isObject(%this.camera))
   {
      if (%player.getClassname() $= "Player")
         %this.camera.setTransform(%player.getEyeTransform());
      else
         %this.camera.setTransform(%player.getTransform());
   }

   // Add the player object to MissionCleanup so that it
   // won't get saved into the level files and will get
   // cleaned up properly
   MissionCleanup.add(%player);

   // Store the client object on the player object for
   // future reference
   %player.client = %this;

   // Player setup...
   if (%player.isMethod("setShapeName"))
      %player.setShapeName(%this.playerName);

   if (%player.isMethod("setEnergyLevel"))
      %player.setEnergyLevel(%player.getDataBlock().maxEnergy);

   // Give the client control of the player
   %this.player = %player;

   // Give the client control of the camera if in the editor
   if( $startWorldEditor )
   {
      %control = %this.camera;
      %control.mode = toggleCameraFly;
      EditorGui.syncCameraGui();
   }
   else
      %control = %player;
      
   if(!isDefined("%noControl"))
      %this.setControlObject(%control);
}