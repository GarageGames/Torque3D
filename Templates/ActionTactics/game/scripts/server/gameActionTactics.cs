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

// ----------------------------------------------------------------------------
// ActionTacticsGame
// ----------------------------------------------------------------------------
// Depends on methods found in gameCore.cs.  Those added here are specific to
// this game type and/or over-ride the "default" game functionaliy.
//
// The desired Game Type must be added to each mission's LevelInfo object.
//   - gameType = "ActionTactics";
// If this information is missing then the GameCore will default to ActionTactics.
// ----------------------------------------------------------------------------

function ActionTacticsGame::onMissionLoaded(%game)
{
   //echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::onMissionLoaded");

   $Server::MissionType = "ActionTactics";
   parent::onMissionLoaded(%game);
}

function ActionTacticsGame::initGameVars(%game)
{
   //echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::initGameVars");

   //-----------------------------------------------------------------------------
   // What kind of "player" is spawned is either controlled directly by the
   // SpawnSphere or it defaults back to the values set here. This also controls
   // which SimGroups to attempt to select the spawn sphere's from by walking down
   // the list of SpawnGroups till it finds a valid spawn object.
   // These override the values set in core/scripts/server/spawn.cs
   //-----------------------------------------------------------------------------
   
   // Leave $Game::defaultPlayerClass and $Game::defaultPlayerDataBlock as empty strings ("")
   // to spawn a the $Game::defaultCameraClass as the control object.
   $Game::defaultPlayerClass = "AiPlayer";
   $Game::defaultPlayerDataBlock = "ActionTacticsPlayerData";
   $Game::defaultPlayerSpawnGroups = "PlayerSpawnPoints PlayerDropPoints";

   //-----------------------------------------------------------------------------
   // What kind of "camera" is spawned is either controlled directly by the
   // SpawnSphere or it defaults back to the values set here. This also controls
   // which SimGroups to attempt to select the spawn sphere's from by walking down
   // the list of SpawnGroups till it finds a valid spawn object.
   // These override the values set in core/scripts/server/spawn.cs
   //-----------------------------------------------------------------------------
   $Game::defaultCameraClass = "Camera";
   $Game::defaultCameraDataBlock = "Observer";
   $Game::defaultCameraSpawnGroups = "CameraSpawnPoints PlayerSpawnPoints PlayerDropPoints";

   // Set the gameplay parameters
   %game.duration = 0;
   %game.endgameScore = 0;
   %game.endgamePause = 0;
   %game.allowCycling = false;   // Is mission cycling allowed?
}

function ActionTacticsGame::startGame(%game)
{
   echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::startGame");
   
	if(!isObject($PlayerTeam))
	{
		$PlayerTeam = new arrayobject();
		MissionCleanup.add($PlayerTeam);
	}
	else
	{
	   //already exists so let's clean it
	   $PlayerTeam.empty();  
	}
	
	if(!isObject($CPUteam))
	{
		$CPUteam = new arrayobject();
		MissionCleanup.add($CPUteam);
	}
	else
	{
	   //already exists so let's clean it
	   $CPUteam.empty();  
	}
	
   if(!isObject($CPUteamManager))
	{
		$CPUteamManager = new arrayobject();
		MissionCleanup.add($CPUteamManager);
	}
   else
	{
	   //already exists so let's clean it
	   $CPUteamManager.empty();  
	}
	
	if(!isObject(turnManager))
	{
		new ScriptObject(turnManager) 
		{
			enemyPhase = 0;//not the enemies turn
			turnNum = 0;//start the game at the beginning! Turns have not yet started
			victory = 0;//victory not achieved - we've only just started
		};
		MissionCleanup.add(turnManager);
		echo("Establishing Turn Management System");
	}

   parent::startGame(%game);
}

function ActionTacticsGame::endGame(%game)
{
   echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::endGame");
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   
   //reset HUD and buttons
   commandToClient(%client, 'ResetHUD', %client, true);
   
   if(isObject($PlayerTeam))
		$PlayerTeam.delete();
	
	if(isObject($CPUteam))
		$CPUteam.delete();
   
   if(isObject($CPUteamManager))
		$CPUteamManager.delete();
	
	if(isObject(turnManager))
	{	
		TurnManager.delete();
		echo("Deleting Turn Management System");
	}
	
	if(isObject($Defend))
		$Defend.delete();

   parent::endGame(%game);
}

function ActionTacticsGame::onGameDurationEnd(%game)
{
   //echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::onGameDurationEnd");

   parent::onGameDurationEnd(%game);
}

function ActionTacticsGame::onClientEnterGame(%game, %client)
{
   echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::onClientEnterGame");
   
   messageAll('MsgAllTurn', '\c4Turn %1', TurnManager.turnNum);
	
   // Sync the client's clocks to the server's
   commandToClient(%client, 'SyncClock', $Sim::Time - $Game::StartTime);

	//!camera = !view, so we better spawn one!
   %cameraSpawnPoint = pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);
   %client.spawnCamera(%cameraSpawnPoint);
   
   // Prepare the player object.
   %game.preparePlayer(%client);

   // ActionTactics gametype deals with all of this here, no need for the callback
   //parent::onClientEnterGame(%game, %client);
}

function ActionTacticsGame::onClientLeaveGame(%game, %client)
{
   //echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::onClientLeaveGame");

   parent::onClientLeaveGame(%game, %client);

}

function ActionTacticsGame::preparePlayer(%game, %client)
{
   echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::preparePlayer");
   
   %turn = turnManager;
   
   //spawn our allied team at the start only
	if(isObject(%turn) && %turn.turnNum == 0)
	{
		if(isObject(PlayerSpawn1))
			%player1 = AiPlayer::tacticsSpawn(Tom, PlayerSpawn1, 1);
		else
			echo("\c2error: No SpawnPoint to spawn Tom!");
		
		if(isObject(PlayerSpawn2))
			%player2 = AiPlayer::tacticsSpawn(Dick, PlayerSpawn2, 1);
		else
			echo("\c2error: No SpawnPoint to spawn Dick!");
		
		if(isObject(PlayerSpawn1))
			%player3 = AiPlayer::tacticsSpawn(Harry, PlayerSpawn3, 1);
		else
			echo("\c2error: No SpawnPoint to spawn Harry!");
	}
	else
	{
		echo("\c2error: Not spawning an allied team as either there is no Turn Management System Active or we are not at the beginning of the game");
	}
		
	%count = $PlayerTeam.count();
	if(%count > 0)
	{
		%start = $PlayerTeam.getKey($PlayerTeam.moveFirst());
		%client.player = %start.getID();//should return ID anyway but check
		%client.player.updateEnergy();
		%client.player.updateHealth();
		%wpn = %client.player.getMountedImage(%slot);
		%wpn.UpdateWeaponHud(%client.player, %slot);
	}
	else
		echo("AllyList is empty!");
		
	commandToServer('tacticsCam');
	commandToServer('NewTurn');//start turn one!
}

function ActionTacticsGame::GameSpecificDeath(%game, %obj)
{
   echo (%game @"\c4 -> "@ %game.class @" -> ActionTacticsGame::GameSpecificDeath");
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	echo("\c4client ID = " @ %client);
   
	if(%obj.team == 1)
	{
		messageClient(%client, 'MsgCPUKill', '\c2Oh No! They got %1!', %obj.getName());
		
      if($PlayerTeam.countKey(%obj) != 0)
		{
			%remove = $PlayerTeam.getIndexfromKey(%obj);
			echo("remove index = " @ %remove);
			$PlayerTeam.erase(%remove);
		}
		$PlayerTeam.echo();//just for debugging in the console
	}
	else
	{
		messageClient(%client, 'MsgPlayerKill', '\c2Bad Guy Down!');
		
      if($CPUteam.countKey(%obj) != 0)
		{
			%remove = $CPUteam.getIndexfromKey(%obj);
			echo("remove index = " @ %remove);
			$CPUteam.erase(%remove);
		}
		$CPUteam.echo();//just for debugging in the console
	}
}

function final_Spawn()
{
   if(!isObject(bot13))
      aiplayer::tacticsSpawn(bot13, botspawn7 , 0);
   if(!isObject(bot14))
      aiplayer::tacticsSpawn(bot14, botspawn8 , 0);
   if(!isObject(bot15))
      aiplayer::tacticsSpawn(bot15, botspawn9 , 0);
   if(!isObject(bot16))
      aiplayer::tacticsSpawn(bot16, botspawn10, 0);
   if(!isObject(bot17))
      aiplayer::tacticsSpawn(bot17, path3start, 0);
   if(!isObject(bot18))
      aiplayer::tacticsSpawn(bot18, path4start, 0);
   if(!isObject(bot19))
      aiplayer::tacticsSpawn(bot19, path5start, 0, botpath5);
   if(!isObject(bot20))
      aiplayer::tacticsSpawn(bot20, path6start, 0, botpath6);
   start_victory_check();
}

function start_Victory_Check()
{
	if(!isObject($Defend))
	{
		$Defend = new arrayobject();

		if(isObject(bot13))
			$Defend.add(bot13, bot13.getID());
		if(isObject(bot14))
			$Defend.add(bot14, bot14.getID());
		if(isObject(bot15))
			$Defend.add(bot15, bot15.getID());
		if(isObject(bot16))
			$Defend.add(bot16, bot16.getID());
		if(isObject(bot17))
			$Defend.add(bot17, bot17.getID());
		if(isObject(bot18))
			$Defend.add(bot18, bot18.getID());
		if(isObject(bot19))
			$Defend.add(bot19, bot19.getID());
		if(isObject(bot20))
			$Defend.add(bot20, bot20.getID());
		MissionCleanup.add($CPUteamManager);
	}
	schedule(1000, turnManager, "victory_Check");
}

function victory_Check()
{
   %count = $Defend.count();

   if(%count > 0)
   {
      for(%i=0; %i < %count; %i++)
      {
         %bot = $Defend.getkey(%i);
         if(!isObject(%bot))
         {
            %k = $Defend.getindexfromkey(%bot);
               $Defend.erase(%k);
         }
         else
         {
            if(%bot.getState() $= "Dead")
            {
               %k = $Defend.getindexfromkey(%bot);
               $Defend.erase(%k);
            }
         }
      }
      schedule(2000, turnManager, "victory_Check");
	}
	else
	{
		echo("Victory Conditions have been met! No Defenders left!");
		turnManager.victory = 1;
	}
}