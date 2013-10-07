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
// Misc server commands avialable to clients
//-----------------------------------------------------------------------------

function serverCmdSuicide(%client)
{
   if (isObject(%client.player))
      %client.player.kill("Suicide");
}

function serverCmdPlayCel(%client,%anim)
{
   if (isObject(%client.player))
      %client.player.playCelAnimation(%anim);
}

function serverCmdTestAnimation(%client, %anim)
{
   if (isObject(%client.player))
      %client.player.playTestAnimation(%anim);
}

function serverCmdPlayDeath(%client)
{
   if (isObject(%client.player))
      %client.player.playDeathAnimation();
}

// ----------------------------------------------------------------------------
// Throw/Toss
// ----------------------------------------------------------------------------

function serverCmdThrow(%client, %data)
{
   %player = %client.player;
   if(!isObject(%player) || %player.getState() $= "Dead" || !$Game::Running)
      return;
   switch$ (%data)
   {
      case "Weapon":
         %item = (%player.getMountedImage($WeaponSlot) == 0) ? "" : %player.getMountedImage($WeaponSlot).item;
         if (%item !$="")
            %player.throw(%item);
      case "Ammo":
         %weapon = (%player.getMountedImage($WeaponSlot) == 0) ? "" : %player.getMountedImage($WeaponSlot);
         if (%weapon !$= "")
         {
            if(%weapon.ammo !$= "")
               %player.throw(%weapon.ammo);
         }
      default:
         if(%player.hasInventory(%data.getName()))
            %player.throw(%data);
   }
}

// ----------------------------------------------------------------------------
// Force game end and cycle
// Probably don't want this in a final game without some checks.  Anyone could
// restart a game.
// ----------------------------------------------------------------------------

function serverCmdFinishGame()
{
   cycleGame();
}

// ----------------------------------------------------------------------------
// Cycle weapons
// ----------------------------------------------------------------------------

function serverCmdCycleWeapon(%client, %direction)
{
   %client.getControlObject().cycleWeapon(%direction);
}

// ----------------------------------------------------------------------------
// Unmount current weapon
// ----------------------------------------------------------------------------

function serverCmdUnmountWeapon(%client)
{
   %client.getControlObject().unmountImage($WeaponSlot);
}

// ----------------------------------------------------------------------------
// Weapon reloading
// ----------------------------------------------------------------------------

function serverCmdReloadWeapon(%client)
{
   %player = %client.getControlObject();
   %image = %player.getMountedImage($WeaponSlot);
   
   // Don't reload if the weapon's full.
   if (%player.getInventory(%image.ammo) == %image.ammo.maxInventory)
      return;
      
   if (%image > 0)
      %image.clearAmmoClip(%player, $WeaponSlot);
}

// -----------------------------------------------------------------------------
// ActionTactics
// -----------------------------------------------------------------------------

function serverCmdtacticsCam(%client)
{
   //%pi = 3.14159;
	//mDegToRad(20) = 0.349066
	%posneg = getword(%client.player.getTransform(), 5);
	%deg = getword(%client.player.getTransform(), 6);

	if(%posneg == -1)
		%deg = %deg - %deg - %deg;
		
	%client.camera.setOrbitObject(%client.player, "0.349066 0 " @ %deg, 0, 1.5, 1.5);
	%client.camera.camDist = 1.5;
}

function serverCmdadjustCamera(%client, %adjustment)
{
		if(%adjustment == 1)
			%n = %client.camera.camDist + 0.5;
		else
			%n = %client.camera.camDist - 0.5;
			
		if(%n < 0.5)
			%n = 0.5;
			
		if(%n > 5)
			%n = 5;
			
		%client.camera.setOrbitObject(%client.player, %client.camera.getRotation(), 0, %n, %n);
		%client.camera.camDist = %n;
}

function serverCmdSetActionMove(%client)
{
	%client.player.action = 1;
   
   commandToClient(%client, 'SetClientMove');
}

function serverCmdSetActionShoot(%client)
{

	%client.player.action = 2;

   //if the playerObject is moving stop it and remove any destination marker
	if(%client.player.getVelocity() !$="0 0 0")
	{
		if( %client.player.decal > -1 )
			decalManagerRemoveDecal( %client.player.decal );
			
		%client.player.stop();
	}
	
	commandToClient(%client, 'SetClientShoot');
}

function serverCmdClearActions(%client)
{
	%client.player.action = 0;

   //if the playerObject is moving stop it and remove any destination marker
	if(%client.player.getVelocity() !$="0 0 0")
	{
		if( %client.player.decal > -1 )
			decalManagerRemoveDecal( %client.player.decal );
			
		%client.player.stop();
	}
	
	commandToClient(%client, 'ResetHud', %client, true);
}

function serverCmdTurnOver(%client)
{
   //reset the current troops action
	%client.player.action = 0;

   //if the playerObject is moving stop it and remove any destination marker
	if(%client.player.getVelocity() !$="0 0 0")
	{
		if( %client.player.decal > -1 )
			decalManagerRemoveDecal( %client.player.decal );
			
		%client.player.stop();
	}
	
	%count = $PlayerTeam.count();
	if(%count > 0)
	{
	
		if(isObject(TurnManager))
		{
			TurnManager.enemyPhase = 1;
			echo("Enemy Team Turn");

			messageAll('MsgNewTurn', '\c4Turn %1 Enemy Phase', TurnManager.turnNum);
			
			if(turnManager.victory == 1)
			{
				MessageBoxOK( "You Have Defeated The Enemy", "Huzzar! You Won The Game!", "disconnect();");
				return;
			}
		}
		else
		{
			echo("\c2error: There is no Turn Management System in place - something has gone horribly wrong!");
			MessageBoxOK( "There is no Turn Management System in place - something has gone horribly wrong!", "Quit", "disconnect();");
		}
	
		//sort out the player
		for(%i=0; %i < %count; %i++)
		{
			%bot = $PlayerTeam.getkey(%i);
			
			if(%bot.getVelocity() !$="0 0 0")//this should never happen but safety first ;)
				%bot.stop();
		
			if( %bot.decal > -1 )//just checking
				decalManagerRemoveDecal( %bot.decal );
				
			%bot.schedule(%bot.timer, "passiveThink");
		}
		
		commandToClient(%client, 'ResetHud', %client, false);
		
		//sort out the enemy turn
		%count = $CPUteam.count();
		if(%count > 0)
		{
			if(!isObject($CPUteamManager))//this should never happen
			{
				$CPUteamManager = new arrayobject();
				MissionCleanup.add($CPUteamManager);
			}
			
			for(%i=0; %i < %count; %i++)
			{
				%ai = $CPUteam.getkey(%i);
	
				%ai.action = 0;
				%ai.hasFired = 0;
				%ai.restoreEnergy();
				
				$CPUteamManager.add(%ai, %ai.goal);
			}

			schedule(2000, turnManager, "commandToServer", 'enemyPhase');
		}
		else
		{
			//make a slight pause before starting a fresh turn
			schedule(2000, turnManager, "commandToServer", 'NewTurn');
		}
	}
	else
	{
	    MessageBoxOK( "You Have Been Defeated", "You Lost The Game", "disconnect();");
	}
}

function serverCmdNewTurn(%client)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   
	if(isObject(TurnManager))
	{
		TurnManager.enemyPhase = 0;
		echo("Ally Turn");
		
		TurnManager.turnNum++;
		messageAll('MsgNewTurn', '\c4Turn %1 Player Phase', TurnManager.turnNum);
		echo("Turn " @ TurnManager.turnNum);
	}
	else
	{
		echo("\c2error: There is no Turn Management System in place - something has gone horribly wrong!");
		MessageBoxOK( "There is no Turn Management System in place - something has gone horribly wrong!", "Quit", "disconnect();");
		return;
	}

   commandToClient(%client, 'ResetHud', %client, true);

	%count = $PlayerTeam.count();
	if(%count > 0)
	{
		for(%i=0; %i < %count; %i++)
		{
			%bot = $PlayerTeam.getkey(%i);
			
			if(%bot.getVelocity() !$="0 0 0")//this should never happen but safety first ;)
				%bot.stop();
		
			if( %bot.decal > -1 )//just checking
				decalManagerRemoveDecal( %bot.decal );
	
			%bot.action = 0;
			%bot.hasFired = 0;
			%bot.restoreEnergy();
		}
	
	
		%start = $PlayerTeam.getKey($PlayerTeam.moveFirst());
		%start = %start.getID();
		%client.player = %start;
		
		commandToServer('tacticsCam');
		
		%start.updateEnergy();
		%start.updateHealth();
		%wpn = %start.getmountedimage(%slot);
		%wpn.UpdateWeaponHud(%start, %slot);
		
		//and startup the Enemy Team Passive Turn
		%num = $CPUteam.count();
		if(%num > 0)
		{
			for(%x=0; %x < %num; %x++)
			{
				%ai = $CPUteam.getkey(%x);
			
				if(%ai.getVelocity() !$="0 0 0")//this should never happen but safety first
					%ai.stop();
		
				%ai.schedule(%ai.timer, "passiveThink");
			}
		}
	}
	else
	{
	    MessageBoxOK( "You Have Been Defeated", "Game Over", "disconnect();");
	}
}

function serverCmdenemyPhase(%client)
{
	echo("\c2 -> command to server -> enemyPhase");
	%count = $CPUteamManager.count();
	if(%count > 0)
	{
		%select = $CPUteamManager.getKey($CPUteamManager.moveFirst());
		turnManager.activeEnemy = %select;
		%select.schedule(%select.timer, "activeThink");
	}
	else
	{
		turnManager.activeEnemy = 0;
		//make a slight pause before starting a fresh turn
		schedule(2000, turnManager, "commandToServer", 'NewTurn');
	}
}

function serverCmdAttackerCam(%client, %ai, %obj)
{
   //	%pi = 3.14159;
	//mDegToRad(10) = 0.174533
	//mDegToRad(20) = 0.349066
	//mDegToRad(30) = 0.523599
   echo("View Attacker Camera");
	if(!isObject(%ai))
      return;
	   
	if(!isObject(%obj))
	   return;

   //get the vector between the objects
   %aiPos = %ai.getTransform();
   %objPos = %obj.getTransform();
   
   %vector = pointToXYPosDegree(%aiPos, %objPos);
   %posneg = getword(%vector, 2);
	%deg = getword(%vector, 3);

	if(%posneg == -1)
		%deg = %deg - %deg - %deg;
   
	%rot = mDegToRad(%deg);

	%client.camera.setOrbitObject(%ai, "0.349066 0 " @ %rot, 0, 2.0, 2.0, 0, "0 0 0.25", 0);
	%client.camera.camDist = 2.0;
}

function pointToXYPosDegree(%posOne, %posTwo)
{
   %vec = VectorSub(%posOne, %posTwo);
   //get the angle
   %rotAngleZ = mATan( firstWord(%vec), getWord(%vec, 1) );
   //add pi to the angle
   %rotAngleZ += 3.14159;

   //make this rotation a proper torque game value, anything more than 240
   // degrees is negative
   if(%rotAngleZ > 4.18879)//you don't actually need this but if it ain't broke don't fix it
   {
      //the rotation scale is seldom negative, instead make the axis value negative
      %modifier = -1;
      //subtract 2pi from the value, then make sure its positive
      %rotAngleZ = mAbs(%rotAngleZ - 6.28319);
      //sigh, if only this were all true
   }
   else
      %modifier = 1;

   //assemble the rotation and send it back in degrees
	%rotAngleZ = mRadToDeg(%rotAngleZ);
	
   return "0 0" SPC %modifier SPC %rotAngleZ;
}