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

// Game duration in secs, no limit if the duration is set to 0
$Game::Duration = 20 * 60;

// When a client score reaches this value, the game is ended.
$Game::EndGameScore = 30;

// Pause while looking over the end game screen (in secs)
$Game::EndGamePause = 10;

//-----------------------------------------------------------------------------

function onServerCreated()
{
   // Server::GameType is sent to the master server.
   // This variable should uniquely identify your game and/or mod.
   $Server::GameType = $appName;

   // Server::MissionType sent to the master server.  Clients can
   // filter servers based on mission type.
   $Server::MissionType = "Deathmatch";

   // GameStartTime is the sim time the game started. Used to calculated
   // game elapsed time.
   $Game::StartTime = 0;

   // Create the server physics world.
   physicsInitWorld( "server" );

   // Load up any objects or datablocks saved to the editor managed scripts
   %datablockFiles = new ArrayObject();
   %datablockFiles.add( "art/ribbons/ribbonExec.cs" );   
   %datablockFiles.add( "art/particles/managedParticleData.cs" );
   %datablockFiles.add( "art/particles/managedParticleEmitterData.cs" );
   %datablockFiles.add( "art/decals/managedDecalData.cs" );
   %datablockFiles.add( "art/datablocks/managedDatablocks.cs" );
   %datablockFiles.add( "art/forest/managedItemData.cs" );
   %datablockFiles.add( "art/datablocks/datablockExec.cs" );   
   loadDatablockFiles( %datablockFiles, true );

   // Run the other gameplay scripts in this folder
   exec("./scriptExec.cs");

   // Keep track of when the game started
   $Game::StartTime = $Sim::Time;
}

function onServerDestroyed()
{
   // This function is called as part of a server shutdown.

   physicsDestroyWorld( "server" );

   // Clean up the GameCore package here as it persists over the
   // life of the server.
   if (isPackage(GameCore))
   {
      deactivatePackage(GameCore);
   }
}

//-----------------------------------------------------------------------------

function onGameDurationEnd()
{
   // This "redirect" is here so that we can abort the game cycle if
   // the $Game::Duration variable has been cleared, without having
   // to have a function to cancel the schedule.

   if ($Game::Duration && !(EditorIsActive() && GuiEditorIsActive()))
      Game.onGameDurationEnd();
}

//-----------------------------------------------------------------------------

function cycleGame()
{
   // This is setup as a schedule so that this function can be called
   // directly from object callbacks.  Object callbacks have to be
   // carefull about invoking server functions that could cause
   // their object to be deleted.

   if (!$Game::Cycling)
   {
      $Game::Cycling = true;
      $Game::Schedule = schedule(0, 0, "onCycleExec");
   }
}

function onCycleExec()
{
   // End the current game and start another one, we'll pause for a little
   // so the end game victory screen can be examined by the clients.

   //endGame();
   endMission();
   $Game::Schedule = schedule($Game::EndGamePause * 1000, 0, "onCyclePauseEnd");
}

function onCyclePauseEnd()
{
   $Game::Cycling = false;

   // Just cycle through the missions for now.

   %search = $Server::MissionFileSpec;
   %oldMissionFile = makeRelativePath( $Server::MissionFile );
      
   for( %file = findFirstFile( %search ); %file !$= ""; %file = findNextFile( %search ) )
   {
      if( %file $= %oldMissionFile )
      {
         // Get the next one, back to the first if there is no next.
         %file = findNextFile( %search );
         if( %file $= "" )
            %file = findFirstFile(%search);
         break;
      }
   }

   if( %file $= "" )
      %file = findFirstFile( %search );

   loadMission(%file);
}

//-----------------------------------------------------------------------------
// GameConnection Methods
// These methods are extensions to the GameConnection class. Extending
// GameConnection makes it easier to deal with some of this functionality,
// but these could also be implemented as stand-alone functions.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function GameConnection::onLeaveMissionArea(%this)
{
   // The control objects invoke this method when they
   // move out of the mission area.

   messageClient(%this, 'MsgClientJoin', '\c2Now leaving the mission area!');
}

function GameConnection::onEnterMissionArea(%this)
{
   // The control objects invoke this method when they
   // move back into the mission area.

   messageClient(%this, 'MsgClientJoin', '\c2Now entering the mission area.');
}

//-----------------------------------------------------------------------------

function GameConnection::onDeath(%this, %sourceObject, %sourceClient, %damageType, %damLoc)
{
   game.onDeath(%this, %sourceObject, %sourceClient, %damageType, %damLoc);
}

// ----------------------------------------------------------------------------
// weapon HUD
// ----------------------------------------------------------------------------
function GameConnection::setAmmoAmountHud(%client, %amount, %amountInClips )
{
   commandToClient(%client, 'SetAmmoAmountHud', %amount, %amountInClips);
}

function GameConnection::RefreshWeaponHud(%client, %amount, %preview, %ret, %zoomRet, %amountInClips)
{
   commandToClient(%client, 'RefreshWeaponHud', %amount, %preview, %ret, %zoomRet, %amountInClips);
}
