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
// AIPlayer callbacks
// The AIPlayer class implements the following callbacks:
//
//    PlayerData::onStop(%this,%obj)
//    PlayerData::onMove(%this,%obj)
//    PlayerData::onReachDestination(%this,%obj)
//    PlayerData::onMoveStuck(%this,%obj)
//    PlayerData::onTargetEnterLOS(%this,%obj)
//    PlayerData::onTargetExitLOS(%this,%obj)
//    PlayerData::onAdd(%this,%obj)
//
// Since the AIPlayer doesn't implement it's own datablock, these callbacks
// all take place in the PlayerData namespace.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Demo Pathed AIPlayer.
//-----------------------------------------------------------------------------

function DemoPlayer::onReachDestination(%this,%obj)
{
   //echo( %obj @ " onReachDestination" );

   // Moves to the next node on the path.
   // Override for all player.  Normally we'd override this for only
   // a specific player datablock or class of players.
   if (%obj.path !$= "")
   {
      if (%obj.currentNode == %obj.targetNode)
         %this.onEndOfPath(%obj,%obj.path);
      else
         %obj.moveToNextNode();
   }
}

function DemoPlayer::onMoveStuck(%this,%obj)
{
   //echo( %obj @ " onMoveStuck" );
}

function DemoPlayer::onTargetExitLOS(%this,%obj)
{
   //echo( %obj @ " onTargetExitLOS" );
}

function DemoPlayer::onTargetEnterLOS(%this,%obj)
{
   //echo( %obj @ " onTargetEnterLOS" );
}

function DemoPlayer::onEndOfPath(%this,%obj,%path)
{
   %obj.nextTask();
}

function DemoPlayer::onEndSequence(%this,%obj,%slot)
{
   echo("Sequence Done!");
   %obj.stopThread(%slot);
   %obj.nextTask();
}

// -----------------------------------------------------------------------------
// ActionTactics
// -----------------------------------------------------------------------------

function ActionTacticsPlayerData::onReachDestination(%this,%obj)
{
      // If there was a decal placed, then it was
   // stored in this %obj variable (see playGui.cs)
   // Erase the decal using the decal manager
   if( %obj.decal > -1 )
      decalManagerRemoveDecal(%obj.decal);

   // Moves to the next node on the path.
   // Override for all player.  Normally we'd override this for only
   // a specific player datablock or class of players.
   if (%obj.path !$= "")
   {
      if (%obj.currentNode == %obj.targetNode)
         %this.onEndOfPath(%obj,%obj.path);
      else
         %obj.moveToNextNode();
   }
}

function ActionTacticsPlayerData::onEndOfPath(%this,%obj,%path)
{
   %obj.nextTask();
}

function ActionTacticsPlayerData::onEndSequence(%this,%obj,%slot)
{
   echo("Sequence Done!");
   %obj.stopThread(%slot);
   %obj.nextTask();
}

function ActionTacticsAiData::onReachDestination(%this,%obj)
{
      // If there was a decal placed, then it was
   // stored in this %obj variable (see playGui.cs)
   // Erase the decal using the decal manager
   if( %obj.decal > -1 )
      decalManagerRemoveDecal(%obj.decal);

   // Moves to the next node on the path.
   // Override for all player.  Normally we'd override this for only
   // a specific player datablock or class of players.
   if (%obj.path !$= "")
   {
      if (%obj.currentNode == %obj.targetNode)
         %this.onEndOfPath(%obj,%obj.path);
      else
         %obj.moveToNextNode();
   }
}

function ActionTacticsAiData::onEndOfPath(%this,%obj,%path)
{
   echo(%obj.getName() @ " path ended - " @ %obj.path @ " clear it");
   %obj.path = "";
   %obj.nextTask();
}

function ActionTacticsAiData::onEndSequence(%this,%obj,%slot)
{
   echo("Sequence Done!");
   %obj.stopThread(%slot);
   %obj.nextTask();
}

//-----------------------------------------------------------------------------
// AIPlayer static functions
//-----------------------------------------------------------------------------

function AIPlayer::spawnAtLocation(%name, %spawnPoint)
{
   // Create the demo player object
   %player = new AiPlayer()
   {
      dataBlock = DemoPlayer;
      path = "";
   };
   MissionCleanup.add(%player);
   %player.setShapeName(%name);
   %player.setTransform(%spawnPoint);
   return %player;
}

function AIPlayer::spawnOnPath(%name, %path)
{
   // Spawn a player and place him on the first node of the path
   if (!isObject(%path))
      return 0;
   %node = %path.getObject(0);
   %player = AIPlayer::spawnAtLocation(%name, %node.getTransform());
   return %player;
}

//-----------------------------------------------------------------------------
// AIPlayer methods
//-----------------------------------------------------------------------------

function AIPlayer::followPath(%this,%path,%node)
{
   // Start the player following a path
   if (!isObject(%path))
   {
      %this.path = "";
      return;
   }

   if (%node > %path.getCount() - 1)
      %this.targetNode = %path.getCount() - 1;
   else
      %this.targetNode = %node;

   if (%this.path $= %path)
      %this.moveToNode(%this.currentNode);
   else
   {
      %this.path = %path;
      %this.moveToNode(0);
   }
}

function AIPlayer::moveToNextNode(%this)
{
   %pathNodeCount=%this.path.getCount();
   %slowdown=0;

   %targetNode=%this.currentNode + 1;

   if (%this.path.isLooping) {
      %targetNode %= %pathNodeCount;
   } else {
      if (%targetNode >= %pathNodeCount-1) {
         %targetNode=%pathNodeCount-1;

         if (%currentNode < %targetNode)
            %slowdown=1;
      }
   }

   %this.moveToNode(%targetNode, %slowdown);
}

function AIPlayer::moveToNode(%this,%index,%slowdown)
{
   // Move to the given path node index
   %this.currentNode = %index;
   %node = %this.path.getObject(%index);
   %this.setMoveDestination(%node.getTransform(),%slowdown);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

function AIPlayer::pushTask(%this,%method)
{
   if (%this.taskIndex $= "")
   {
      %this.taskIndex = 0;
      %this.taskCurrent = -1;
   }
   %this.task[%this.taskIndex] = %method;
   %this.taskIndex++;
   if (%this.taskCurrent == -1)
      %this.executeTask(%this.taskIndex - 1);
}

function AIPlayer::clearTasks(%this)
{
   %this.taskIndex = 0;
   %this.taskCurrent = -1;
}

function AIPlayer::nextTask(%this)
{
   if (%this.taskCurrent != -1)
      if (%this.taskCurrent < %this.taskIndex - 1)
         %this.executeTask(%this.taskCurrent++);
      else
         %this.taskCurrent = -1;
}

function AIPlayer::executeTask(%this,%index)
{
   %this.taskCurrent = %index;
   eval(%this.getId() @"."@ %this.task[%index] @";");
}

//-----------------------------------------------------------------------------

function AIPlayer::singleShot(%this)
{
   // The shooting delay is used to pulse the trigger
   %this.setImageTrigger(0, true);
   %this.setImageTrigger(0, false);
   %this.trigger = %this.schedule(%this.shootingDelay, singleShot);
}

//-----------------------------------------------------------------------------

function AIPlayer::wait(%this, %time)
{
   %this.schedule(%time * 1000, "nextTask");
}

function AIPlayer::done(%this,%time)
{
   %this.schedule(0, "delete");
}

function AIPlayer::fire(%this,%bool)
{
   if (%bool)
   {
      cancel(%this.trigger);
      %this.singleShot();
   }
   else
      cancel(%this.trigger);
   %this.nextTask();
}

function AIPlayer::aimAt(%this,%object)
{
   echo("Aim: "@ %object);
   %this.setAimObject(%object);
   %this.nextTask();
}

function AIPlayer::animate(%this,%seq)
{
   //%this.stopThread(0);
   //%this.playThread(0,%seq);
   %this.setActionThread(%seq);
}

// ----------------------------------------------------------------------------
// Some handy getDistance/nearestTarget functions for the AI to use
// ----------------------------------------------------------------------------

function AIPlayer::getTargetDistance(%this, %target)
{
   echo("\c4AIPlayer::getTargetDistance("@ %this @", "@ %target @")");
   $tgt = %target;
   %tgtPos = %target.getPosition();
   %eyePoint = %this.getWorldBoxCenter();
   %distance = VectorDist(%tgtPos, %eyePoint);
   echo("Distance to target = "@ %distance);
   return %distance;
}

function AIPlayer::getNearestPlayerTarget(%this)
{
   echo("\c4AIPlayer::getNearestPlayerTarget("@ %this @")");

   %index = -1;
   %botPos = %this.getPosition();
   %count = ClientGroup.getCount();
   for(%i = 0; %i < %count; %i++)
   {
      %client = ClientGroup.getObject(%i);
      if (%client.player $= "" || %client.player == 0)
         return -1;
      %playerPos = %client.player.getPosition();

      %tempDist = VectorDist(%playerPos, %botPos);
      if (%i == 0)
      {
         %dist = %tempDist;
         %index = %i;
      }
      else
      {
         if (%dist > %tempDist)
         {
            %dist = %tempDist;
            %index = %i;
         }
      }
   }
   return %index;
}

//-----------------------------------------------------------------------------

function AIPlayer::think(%player)
{
   // Thinking allows us to consider other things...
   %player.schedule(500, think);
}

function AIPlayer::spawn(%path)
{
   %player = AIPlayer::spawnOnPath("Shootme", %path);

   if (isObject(%player))
   {
      %player.followPath(%path, -1);

      // slow this sucker down, I'm tired of chasing him!
      %player.setMoveSpeed(0.5);

      //%player.mountImage(xxxImage, 0);
      //%player.setInventory(xxxAmmo, 1000);
      //%player.think();

      return %player;
   }
   else
      return 0;
}

// -----------------------------------------------------------------------------
// ActionTactics
// -----------------------------------------------------------------------------

function AIPlayer::openFire(%this, %xyz)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	
	%muzzlePoint = %this.getMuzzlePoint(%slot);
	%searchMasks = $TypeMasks::PlayerObjectType | $TypeMasks::TerrainObjectType | $TypeMasks::StaticObjectType |  $TypeMasks::StaticShapeObjectType;
	
	%aimpoint = VectorAdd(%muzzlePoint, VectorScale(%this.getEyeVector(), 200));
	%targetsearch = containerRayCast(%muzzlePoint, %aimpoint, %searchMasks, %this);
	%target = restWords(%targetSearch);
	echo("targetSearch + target = " @ %targetsearch SPC %target);
	
	if(%target)
	{
		%aim = VectorDist(%target, %xyz);
		echo("difference between muzzleVector and targetpoint is " @ %aim);
		
		if(%aim <= 5)
		{
	
			// Tell our AI object to fire its weapon
			%this.setImageTrigger(0, true);
			
			%this.hasFired = 1;
		 
			// Stop firing in 150 milliseconds
			%this.schedule(50, "setImageTrigger", 0, 0);
			%this.schedule(100, "clearAim");
		}
		else
		{
			messageClient(%client, 'MsgNoLOS', '%1 - No Line of Sight to Aimpoint!', %this.getName());
			%this.clearAim();
			%this.setImageTrigger(0, 0);
		}
	}
	else
	{
		messageClient(%client, 'MsgNoLOS', '%1 - No Line of Sight to Aimpoint!', %this.getName());
		%this.clearAim();
		%this.setImageTrigger(0, 0);
	}
}

function AIPlayer::tacticsSpawn(%name, %spawnPoint, %team, %path)
{
	if(%team == 1)
		%playerType = ActionTacticsPlayerData;//the player
	else
		%playerType = ActionTacticsAiData;//the Ai enemy

   // Create the demo player object
   %player = new AiPlayer(%name)
   {
      dataBlock = %playerType;
      path = %path;
   };
   MissionCleanup.add(%player);
   
   %player.team = %team;
   %player.action = 0;
   %player.hasFired = 0;
   
	%player.path = %path;
	%player.startPath = 0;
   
   %rand = GetRandom(1000, 1500);
   %player.timer = %rand;
   
	if(isObject(%spawnpoint))
		%player.setTransform(%spawnPoint.getTransform());
	else
		echo(%spawnpoint @ " is not an object for " @ %name @ "'s spawnpoint");
		
	%player.setInventory(Lurker, 1);
	%player.setInventory(LurkerAmmo, %player.maxInventory(LurkerAmmo));
	%player.mountImage(LurkerWeaponImage, 0);
		
	if(isObject($PlayerTeam) && %team == 1)
	{
	   %player.setShapeName(%name);
		$PlayerTeam.add(%player, %playerType);
		
		%player.setSkinName("teal");
		
		if(isObject(turnManager))
			if(turnManager.enemyPhase == 1)
				%player.schedule(%rand, "passiveThink");
		
	}
		
	if(isObject($CPUteam) && %team != 1)
	{
		$CPUteam.add(%player, %playerType);
		%player.setSkinName("orange");

		if(isObject(turnManager))
			if(turnManager.enemyPhase == 0)
				%player.schedule(%rand, "passiveThink");
	}
		
   return %player;
}

function AIPlayer::checkOtherLOS(%this, %target)
{
   %found = 0;
   %test = 0;
   %index = 0;
   %count = $PlayerTeam.count();

   while(%test == 0) 
   {
      %key = $PlayerTeam.getKey(%index);
      if(%key != %this)
      {
         if(%key.targetClearView(%target) == true)
         {
            %found = %key;
            %test = 1;  
         }
      }
      
      %index++;
      
      if(%index == %count)
         %test = 1;
   }
   
   return %found;
}

function AIPlayer::passiveThink(%this)
{
	if(%this.getState() $="Dead")
		return;
		
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   %player = %client.player;
   
   %turn = turnManager;
		
   //uncomment this to see it report to the console every loop
	//echo(%this.getname() @ " passiveThink");

	if(%this.team != 1)
	{
		//must be enemy team member
		
		//check if the turn has changed
		//for AI enemy team they need to go into activeThink mode
		if(%turn.enemyPhase == true)
			return;
		
		//we only attack the active player
		%target = %player;
		
		//can only shoot at active hostile playerObject in action = move
		if(%target.action != 1)
			%target = 0;
	}
	else
	{
		//must be an allied team member
		
		//check if the turn has changed
		//for Human Ally team they need to go back to being Client Controlled
		if(%turn.enemyPhase == false)
			return;
			
		%target = %turn.activeEnemy;
		
		if(isObject(%target))
		{
			if(%target.getVelocity() $="0 0 0")
				%target = 0;
         else
         {
            if(%this == %player)
            {
               %view = %this.targetClearView(%target);
               echo(%this @ " player view of enemy = " @ %view);
               if(%view == false)
               {
                  //check for other player troops and see if they can see the enemy
                  %los = %this.checkOtherLOS(%target);
                  echo(%this @ " los returns " @ %los);
                  if(%los != 0)
                  {
                     %client.player = %los;
                     
                     //point at the active AI from one of the players troops
		               commandToServer('AttackerCam', %los, %target);
                  }
               }
            }
         }
		}
		else
		{
			%target = 0;
		}
	}
	
	%wpn = %this.getMountedImage(%slot);
	
	if(%wpn != 0)
	{
      %state = %this.getImageState(%slot);
      if(%state $="Ready")
      {
         if(%target != 0)
         {
            %view = %this.targetClearView(%target);
            if(%view == true)
            {
               //distance check - Client can only shoot to 100 units so Enemy Ai is the same
               %dist = VectorDist(%me, %you);
               
               if(%dist < 100)
               {
                  %this.setAimObject(%target, "0 0 1.5");//aim a little up
                  %this.schedule(200, "autoShoot");//to help aiming delay
               }
            }
            else
            {
               //not able to shoot
               %this.clearAim();
            }
         }
      }
      else
      {
         %reload = %this.AiReload();
         
         if(%reload == 0)
            echo("\c2AiPlayer has no weapon to reload!");
      }
         
      //And loop
      %this.schedule(%this.timer, "passiveThink");
	}
	else
	{
	   echo("\c2AiPlayer has no weapon! Abort passiveThink routine");  
	}
}

function AIPlayer::autoShoot(%this)//automated shooting
{
	if(%this.getState() $="Dead")
		return;
		
	echo(%this.getname() @ " autoShoot");
	// Tell our passive AI object to fire its weapon
	%this.setImageTrigger(0, true);
			
	// Stop firing in 50 milliseconds
	%this.schedule(50, "setImageTrigger", 0, 0);
	%this.schedule(100, "clearAim");
}

function AIPlayer::TargetClearView(%this, %target)
{
   //target is the ID of the object which we are aiming at, not the name
	if(%this.getState() $="Dead")
		return;
		
	if(!isObject(%target))
		return false;
		
	if(%target.getState() $="Dead")
		return;
   
	%searchMasks = $TypeMasks::PlayerObjectType | $TypeMasks::TerrainObjectType | $TypeMasks::StaticObjectType | $TypeMasks::StaticShapeObjectType;
		
	%me = %this.getEyePoint();
	%you = %target.getEyePoint();
		
	//check for Line OF Sight
	%clear = containerRayCast(%me, %you, %searchMasks, %this);
	
	//get the id of what the LOS has hit
	%id = firstWord(%clear);
	
	//echo("targetClearView " @ %clear SPC %id SPC %id.getClassName());
			
	if(%clear == 0 || %clear == %target) 
		return true;   // path is clear
	else 
		return false;
}

function AIPlayer::activeThink(%this)
{
	//Ai Enemy's turn to be active
	echo(%this.getname() @ " Enemy activeThink");
	
	// As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	
	//need to check for:
	//can we move? Energy > 0
	//do we want to move?
	//are we already moving?
	//can we shoot? hasFired == 0
	//do we have a target?
	
	if(%this.getState() $="Dead")
	{
		%k = $CPUteamManager.getindexfromkey(%this);
   		$CPUteamManager.erase(%k);
		
		turnManager.activeEnemy = 0;
		schedule(1000, turnManager, "commandToServer", 'enemyPhase');
		
		return;
	}
	
	if(turnManager.activeEnemy != %this)
	{
		echo("\c0******** " @ %this.getname @ " ends active turn");
		schedule(1000, turnManager, "commandToServer", 'enemyPhase');
		return;
	}
	
	%energy = %this.getEnergyLevel();
	
	if(%energy > 0)
	{
		//we have energy to move

		//do we have a goal to move to?
		if(isObject(%this.goal))
		{
			%range = VectorDist(%this.getPosition(), %this.goal.getPosition());
		
			if(%range > 1)
			{
				echo(%this.getname() SPC %this.getenergyLevel());
				%this.setmovedestination(%this.goal.getposition());
				
				//if the client's playerObject usese up energy, so does the Ai
				%this.decEnergy();
				
				%this.schedule(%this.timer, "activeThink");
				return;
			}
			else
				%this.goal = 0;
		}
		
      //do we have a path?
		if(%this.path !$="")
		{
			//if the client's playerObject uses up energy, so does the Ai
				%this.decEnergy();
				
			if(%this.startPath == 0)
			{
				echo("start path for the first time!");
				%this.startPath = 1;
				%this.getID().followPath("MissionGroup/bot_Paths/" @ %this.path, 99);
				echo("our path = " @ %this.path);
			}
			else
			{
				echo("already has a path! - path = " @ %this.path);
				%this.moveToNode(%this.currentNode);
			}

				%this.schedule(%this.timer, "activeThink");
				return;
		}
		else
		{
			echo("we have no path - path = " @ %this.path);
		}
	}
	
	if(%this.hasFired == 0)
	{
		%target = %this.AttackTarget();
		echo("activeThink target = " @ %target);
		
		if(%target != 0)
		{
			if(%target != %client.player)
				%client.player = %target;
				
		   //point at the attacker from the target which is one of the player's troops
		   commandToServer('AttackerCam', %target, %this);
			
			//%this.setAimObject(%target, "0 0 1.5");//aim a little up
			%this.setAimLocation(%target.getEyePoint());
			%state = %this.getImageState(%slot);
			if(%state $="Ready")
			{
            %this.schedule(200, "autoShoot");//to help aiming delay
            %this.hasFired = 1;
			}
			else
			{
			   // CPU team member needs to reload if they cannot they will be returned here and their phase will end
			   %reload = %this.AiReload();
			   if(%reload == 1)
			   {
			      echo("reloaded!");
			      %this.schedule(%this.timer, "activeThink");
			      return;
			   }
			}
		}
	}
	
	//if we get down here we must be done!
	//remove us from the EnemyManager and get the next Ai
	%k = $CPUteamManager.getindexfromkey(%this);
   $CPUteamManager.erase(%k);
	turnManager.activeEnemy = 0;
	
	%this.schedule(%this.timer, "activeThink");
}

function AIPlayer::Targeting(%this)
{
	%dist = 0;
	%index = -1;
	%mypos = %this.getposition();

	%count = $PlayerTeam.count();
	for(%i=0; %i < %count; %i++)
	{
		%bot = $PlayerTeam.getkey(%i);
		if(isObject(%bot))
		{
			if(%bot.getstate() !$="Dead")		
			{
				%tgtpos = %bot.getposition();		
				%tempdist = vectorDist(%tgtpos, %mypos);

				if(%tempdist < 100)//100 is max range for Client so same for Ai
					if(%tempdist < %dist || %dist == 0)
					{
					   if(%this.targetClearView(%bot) == true)
						{
							%dist = %tempdist;
							%index = %i;
						}
					}
			}
		}
	}
	echo("targeting returns index " @ %index);
	if(%index != -1)
	{
		%tgt = $PlayerTeam.getkey(%index);
	 	return %tgt;
	}

	return -1;
}

function AIPlayer::AttackTarget(%this)
{
	%tgtid = %this.Targeting(%obj);
	
	if(%tgtid != -1 && isObject(%tgtid))
		return %tgtid;
   else
      return 0;
}

function AIPlayer::AiReload(%this)
{
   // we never want the CPU Ai enemy troops to run out of ammo
   %wpn = %this.getMountedImage(%slot);
   
   if(%wpn == 0)
      return false;//bail - no weapon
   
   // get the type of ammo
   %ammo = %wpn.ammo;
   %max = %this.maxInventory(%ammo);
   
   echo(%ammo SPC %max);
   
   // reload
   %this.setInventory(%ammo, %max);

   return true;
}

function AIPlayer::getHealth(%this)
{
   %max = %this.getDatablock().maxDamage;
   %damage = %this.getDamageLevel();
   echo("max health and damage = " @ %max SPC %damage);
   if(%damage == 0)
      return %max;
   else
   {
      %result = %max - %damage;
      return %result;
   }
}