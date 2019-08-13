//-----------------------------------------------------------------------------
// 3D Action Adventure Kit for T3D
// Copyright (C) 2008-2013 Ubiq Visuals, Inc. (http://www.ubiqvisuals.com/)
//
// This file also incorporates work covered by the following copyright and  
// permission notice:
//
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

//Ubiq: Calling Parent::onEnterTrigger() Parent::onLeaveTrigger() etc. seem
//to result in errors now, so I'm commenting them out to avoid console spam.

//-----------------------------------------------------------------------------
// DefaultTrigger is used by the mission editor.  This is also an example
// of trigger methods and callbacks.

datablock TriggerData(DefaultTrigger)
{
   // The period is value is used to control how often the console
   // onTriggerTick callback is called while there are any objects
   // in the trigger.  The default value is 100 MS.
   tickPeriodMS = 100;
};

datablock TriggerData(ClientTrigger : DefaultTrigger)
{
   clientSide = true;
};


//-----------------------------------------------------------------------------
// SaveTrigger - writes a quick autosave file (only the player transform / health)
//-----------------------------------------------------------------------------
datablock TriggerData(SaveTrigger)
{
   tickPeriodMS = 100;
};

function SaveTrigger::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
   
   if(!%trigger.disabled)
   {
      if(%obj.getClassName() $= "Player")
      {
         %client = %obj.getControllingClient();
         
         if(!isObject(saveData))
            new ScriptObject(saveData);
            
         %file = fileBase($Server::MissionFile);
         %path = "saves/"@%file@"-autosave.sav";   
         
         echo("Saving to "@%path@"...");
         
         saveData.playerTransform = %obj.getTransform();
         saveData.playerDamage = %obj.getDamageLevel();
         saveData.cameraGoalPlayerTransform = %client.cameraGoalPlayer.getTransform();
         saveData.cameraGoalFollowerTransform = %client.cameraGoalFollower.getTransform();
         
         saveData.save(%path);
         
         echo("Done!");
         
         //disable ourselves after one use
         %trigger.disabled = true;
      }
   }
}

function SaveTrigger::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
}

function SaveTrigger::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);
}

//-----------------------------------------------------------------------------
// DamageTrigger is used to apply damage to objects (typically players). While
// inside, an object will continually sustain damage at the rate specified
//-----------------------------------------------------------------------------
datablock TriggerData(DamageTrigger)
{
   tickPeriodMS = 100;
};

function DamageTrigger::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
}

function DamageTrigger::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
}

function DamageTrigger::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);
   
   %count = %trigger.getNumObjects();
   for(%i = 0; %i < %count; %i++)
   {
      %obj = %trigger.getObject(%i);
      %obj.applyDamage(%trigger.damage);
   }
}

//-----------------------------------------------------------------------------
// KillTrigger is used to kill players. By default a player will die immediately
// on entering. If "nextCollision" is 1, the player doesn't die until they collide
// with something. This can be useful for "faking" a death-by-fall.
//-----------------------------------------------------------------------------
datablock TriggerData(KillTrigger)
{
   tickPeriodMS = 100;
};

function KillTrigger::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
   
   if(%obj.getClassName() $= "Player")
   {
      if(%trigger.nextCollision)
      {
         //kill them on next collision
         %obj.dieOnNextCollision = true;
      }
      else
      {
         //kill them immediately
         %playerDatablock = %obj.getDatablock();
         %fullDamage = %playerDatablock.maxDamage;
         %playerDatablock.Damage(%obj, 0, %obj.getPosition(), %fullDamage, "KillTrigger");
      }
   }
}

function KillTrigger::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
}

function KillTrigger::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);
}

//-----------------------------------------------------------------------------
// ClimbTrigger is used to designate climbable areas
// If a player is inside 1 (or more) ClimbTriggers, he can climb any appropriate surface
//-----------------------------------------------------------------------------
datablock TriggerData(ClimbTrigger)
{
   tickPeriodMS = 100;
};

function ClimbTrigger::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
   %obj.climbTriggerCount++;
}

function ClimbTrigger::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
   %obj.climbTriggerCount--;
}

function ClimbTrigger::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);   
}

//-----------------------------------------------------------------------------
// CameraTrigger_Reset is used to return the camera to the default mode (player
// cam with nothing forced). It's an "onLeave" trigger so a large area can be
// surrounded and when the player leaves, the camera will return to default.
//-----------------------------------------------------------------------------
datablock TriggerData(CameraTrigger_Reset)
{
   tickPeriodMS = 100;
};

function CameraTrigger_Reset::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
}

function CameraTrigger_Reset::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
   
   %con = %obj.getControllingClient();
   
   //reset cameraGoalPlayer
   %con.cameraGoalPlayer.setForcedYaw("", "");
   %con.cameraGoalPlayer.setForcedPitch("", "");
   %con.cameraGoalPlayer.setForcedRadius("", %trigger.time);
   %con.cameraGoalPlayer.setAutoYaw(false);
   
   //set cameraGoalFollower to follow cameraGoalPlayer
   %con.cameraGoalFollower.setGoalObject(%con.cameraGoalPlayer, %trigger.time);
}

function CameraTrigger_Reset::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);   
}

//-----------------------------------------------------------------------------
// CameraTrigger_Player is used to "activate" the cameraGoalPlayer camera mode
//-----------------------------------------------------------------------------
datablock TriggerData(CameraTrigger_Player)
{
   tickPeriodMS = 100;
};

function CameraTrigger_Player::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
   %con = %obj.getControllingClient();
   
   %currentGoal = %con.cameraGoalFollower.getGoalObject();
   if(%currentGoal == %con.cameraGoalPlayer)
   {
      //transitioning from ourselves (smooth)
      %con.cameraGoalPlayer.setForcedYaw(%trigger.forcedYaw, %trigger.time);
      %con.cameraGoalPlayer.setForcedPitch(%trigger.forcedPitch, %trigger.time);
      %con.cameraGoalPlayer.setForcedRadius(%trigger.forcedRadius, %trigger.time);
      
      %con.cameraGoalFollower.setGoalObject(%con.cameraGoalPlayer);
   }
   else
   {
      //transitioning from something else (instant)
      %con.cameraGoalPlayer.setForcedYaw(%trigger.forcedYaw, "");
      %con.cameraGoalPlayer.setForcedPitch(%trigger.forcedPitch, "");
      %con.cameraGoalPlayer.setForcedRadius(%trigger.forcedRadius, "");
      
      %con.cameraGoalFollower.setGoalObject(%con.cameraGoalPlayer, %trigger.time);
   }
   
   %con.cameraGoalPlayer.setAutoYaw(%trigger.autoYaw);   
}

function CameraTrigger_Player::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
}

function CameraTrigger_Player::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);   
}

//-----------------------------------------------------------------------------
// CameraTrigger_Path is used to "activate" the cameraGoalPath camera mode
//-----------------------------------------------------------------------------
datablock TriggerData(CameraTrigger_Path)
{
   tickPeriodMS = 100;
};

function CameraTrigger_Path::onEnterTrigger(%this,%trigger,%obj)
{
   //Parent::onEnterTrigger(%this,%trigger,%obj);
   %con = %obj.getControllingClient();
      
   //setup cameraGoalPath
   %con.cameraGoalPath.setPlayerPathObject(%trigger.playerPath);
   %con.cameraGoalPath.setCameraPathObject(%trigger.cameraPath);
   %con.cameraGoalPath.setLookAtPlayer(%trigger.lookAtPlayer);
   
   //set cameraGoalFollower to follow cameraGoalPath
   if(%trigger.time $= "")
      %con.cameraGoalFollower.setGoalObject(%con.cameraGoalPath);
   else
      %con.cameraGoalFollower.setGoalObject(%con.cameraGoalPath, %trigger.time);
}

function CameraTrigger_Path::onLeaveTrigger(%this,%trigger,%obj)
{
   //Parent::onLeaveTrigger(%this,%trigger,%obj);
}

function CameraTrigger_Path::onTickTrigger(%this,%trigger)
{
   //Parent::onTickTrigger(%this,%trigger);   
}