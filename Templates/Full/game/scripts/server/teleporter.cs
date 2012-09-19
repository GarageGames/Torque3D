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
// Trigger-derrived teleporter object. Teleports an object from it's entrance to 
// it's exit if one is defined.  

function TeleporterTrigger::onAdd( %this, %teleporter )
{
   
   // Setup default parameters.
   if ( %teleporter.exit $= "" )
      %teleporter.exit = "NameOfTeleporterExit";
   if ( %teleporter.teleporterCooldown $= "" )   
      %teleporter.teleporterCooldown = %this.teleporterCooldown;
   if ( %teleporter.exitVelocityScale $= "" )   
      %teleporter.exitVelocityScale = %this.exitVelocityScale;
   if ( %teleporter.reorientPlayer $= "" )   
      %teleporter.reorientPlayer = %this.reorientPlayer;
   if ( %teleporter.oneSided $= "" )
      %teleporter.oneSided = %this.oneSided;
   if ( %teleporter.entranceEffect $= "" )
      %teleporter.entranceEffect = %this.entranceEffect;
   if ( %teleporter.exitEffect $= "" )
      %teleporter.exitEffect = %this.exitEffect;
      
   // We do not want to save this variable between levels, 
   // clear it out every time the teleporter is added 
   // to the scene.
   %teleporter.timeOfLastTeleport = "";
}

function TeleporterTrigger::onLeaveTrigger(%this,%trigger,%obj)
{
   // This is called after onEnterTrigger for BOTH
   // The teleporter's entrance and exit
   %obj.isTeleporting = false;
}

//ARGS:
// %this - The teleporter datablock.
// %entrance - The teleporter the player has entered (The one calling this function).
// %obj - The object that entered the teleporter.
function TeleporterTrigger::onEnterTrigger(%this, %entrance, %obj)
{
   // Get the location of our target position
   %exit = nameToID(%entrance.exit);
   
   // Check if the the teleport is valid.
   %valid = %this.verifyObject(%obj, %entrance, %exit);
   
   // Bail out early if we cannot complete this teleportation
   if (!%valid)
      return;
   
   // Kill any players in the exit teleporter.
   %this.telefrag(%obj, %exit);
   
   // Create our entrance effects on all clients.
   if (isObject(%entrance.entranceEffect))
   {
      for(%idx = 0; %idx < ClientGroup.getCount(); %idx++)
         commandToClient(ClientGroup.getObject(%idx), 'PlayTeleportEffect', %entrance.position, %entrance.entranceEffect.getId());
   }
   
   // Teleport the player to the exit teleporter.
   %this.teleportPlayer(%obj, %exit);
   
   // Create our exit effects on all clients.
   if (isObject(%exit.exitEffect))
   {
      for(%idx = 0; %idx < ClientGroup.getCount(); %idx++)
         commandToClient(ClientGroup.getObject(%idx), 'PlayTeleportEffect', %exit.position, %exit.exitEffect.getId());
   }
   
   // Record what time we last teleported so we can determine if enough
   // time has elapsed to teleport again
   %entrance.timeOfLastTeleport = getSimTime();
   
   // If this is a bidirectional teleporter, log it's exit too.
   if (%exit.exit $= %entrance.name)
      %exit.timeOfLastTeleport = %entrance.timeOfLastTeleport;
   
   // Tell the client to play the 2D sound for the player that teleported.
   if (isObject(%this.teleportSound) && isObject(%obj.client))
      %obj.client.play2D(%this.teleportSound);
}

// Here we verify that the teleport is valid.
// Tests go here like if the object is of a 'teleportable' type, if the 
// given teleporter has an exit defined, etc.
function TeleporterTrigger::verifyObject(%this, %obj, %entrance, %exit)
{
   
   // Bail out early if we couldn't find an exit for this teleporter.
   if (!isObject(%exit))
   {
      logError("Cound not find an exit for " @ %entrance.name @ ".");
      return false;
   }
   
   // If the entrance is once sided, make sure the object
   // approached it from it's front.
   if (%entrance.oneSided)
   {
      %dotProduct = VectorDot(%entrance.getForwardVector(), %obj.getVelocity());
      
      if (%dotProduct > 0)
         return false;
   }
   
   // If we are coming directly from another teleporter and it happens
   // to be bidirectional, We need to avoid ending sending objects through
   // an infinite loop.
   if (%obj.isTeleporting)
      return false;
   
   // We only want to teleport players
   // So bail out early if we have found any 
   // other object.
   if (!%obj.isMemberOfClass("Player"))
      return false;
      
   if (%entrance.timeOfLastTeleport > 0 && %entrance.teleporterCooldown > 0)
   {
      // Get the current time, subtract it from the time it last teleported 
      // And compare the difference to see if enough time has elapsed to
      // activate the teleporter again.
      %currentTime = getSimTime();
      %timeDifference = %currentTime - %entrance.timeOfLastTeleport;
      %db = %entrance.getDatablock();
      if (%timeDifference <= %db.teleporterCooldown)
         return false;
   }
      
   return true;
}

// Function to teleport object %player to teleporter %exit.
function TeleporterTrigger::teleportPlayer(%this, %player, %exit)
{
   // Teleport our player to the exit teleporter.
   if (%exit.reorientPlayer)
      %targetPosition = %exit.getTransform();
   else
   {
      %pos = %exit.getPosition();
      %rot = getWords(%player.getTransform(), 3, 6);
      %targetPosition = %pos SPC %rot;
   }

   %player.setTransform(%targetPosition);
   
   // Adjust the player's velocity by the Exit location's scale.  
   %player.setVelocity(vectorScale(%player.getVelocity(), %exit.exitVelocityScale)); 
   
   // Prevent the object from doing an immediate second teleport
   // In the case of a bidirectional teleporter
   %player.isTeleporting = true;
}

// Telefrag is a term used in multiplayer gaming when a player takes a teleporter
// while another player is occupying it's exit. The player at the exit location 
// is killed, allowing the original player to arrive at the teleporter.
function TeleporterTrigger::teleFrag(%this, %player, %exit)
{
   // When a telefrag happens, there are two cases we have to consider.
   // The first case occurs when the player's bounding box is much larger than the exit location, 
   // it is possible to have players colide even though a player is not within the bounds 
   // of the trigger Because of this we first check a radius the size of a player's bounding 
   // box around the exit location.
     
   // Get the bounding box of the player
   %boundingBoxSize = %player.getDatablock().boundingBox;
   %radius = getWord(%boundingBoxSize, 0);
   %boxSizeY = getWord(%boundingBoxSize, 1);
   %boxSizeZ = getWord(%boundingBoxSize, 2);
   
   // Use the largest dimention as the radius to check
   if (%boxSizeY > %radius)
      %radius = %boxSizeY;
   if (%boxSizeZ > %radius)
      %radius = %boxSizeZ;
   
   %position = %exit.getPosition();
   %mask = $TypeMasks::PlayerObjectType;
   
   // Check all objects within the found radius of the exit location, and telefrag
   // any players that meet the conditions.
   initContainerRadiusSearch( %position, %radius, %mask );
   while ( (%objectNearExit = containerSearchNext()) != 0 )
   {
      if (!%objectNearExit.isMemberOfClass("Player"))
         continue;
         
      // Avoid killing the player that is teleporting in the case of two
      // Teleporters near eachother.
      if (%objectNearExit == %player)
         continue;
         
      %objectNearExit.damage(%player, %exit.getTransform(), 10000, "Telefrag");  
   }

   // The second case occurs when the bounds of the trigger are much larger
   // than the bounding box of the player. (So multiple players can exist within the
   // same trigger). For this case we check all objects contained within the trigger
   // and telefrag all players.
   %objectsInExit = %exit.getNumObjects();
   
   // Loop through all objects in the teleporter exit
   // And kill any players
   for(%i = 0; %i < %objectsInExit; %i++) 
   {
      %objectInTeleporter = %exit.getObject(%i);
      
      if (!%objectInTeleporter.isMemberOfClass("Player"))
         continue;
         
      // Avoid killing the player that is teleporting in the case of two
      // Teleporters near eachother.
      if (%objectInTeleporter == %player)
         continue;
         
      %objectInTeleporter.damage(%player, %exit.getTransform(), 10000, "Telefrag");  
   }
}

// Customized kill message for telefrag deaths
function sendMsgClientKilled_Telefrag(%msgType, %client, %sourceClient, %damLoc)
{
   messageAll(%msgType, '%1 was telefragged by %2!', %client.playerName, %sourceClient.playerName);
}
