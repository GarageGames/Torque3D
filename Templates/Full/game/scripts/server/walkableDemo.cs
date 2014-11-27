//-----------------------------------------------------------------------------
// walkableDemo.cs - This is a platform-jumping mini-game designed to test the
// PathShape, TSAttachable and WalkableShape object classes.
// Author: Michael A. Reino
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CustomCheetah is a cheetah with a WalkableShape mounted to the roof where
// the turret normally mounts.
//-----------------------------------------------------------------------------
datablock WheeledVehicleData(CustomCheetah : CheetahCar)
{
   nameTag = 'Custom Cheetah';
};

function CustomCheetah::onAdd(%this, %obj)
{
   // Do standard cheetah init
   CheetahCar::onAdd(%this, %obj);

   // Remove the turret from the roof
   %obj.unmountImage(%this.turretSlot);

   // Create our walkable platform
   %platform = new WalkableShape(CarPlatform) {
      UseAutoAttach = "1";
      RayLength = "1.5";
      shapeName = "art/shapes/rocks/boulder.dts";
      scale = "1 1 0.1";
   };
   %obj.platform = %platform;
   %platform.car = %obj;

   // Mount the platform to the Turret slot
   %obj.mountObject(%platform, %this.turretSlot);
}

function CustomCheetah::onRemove(%this, %obj)
{
   %obj.unmountObject(%obj.platform);
   if( isObject(%obj.platform) )
      %obj.platform.delete();

   CheetahCar::onRemove(%this, %obj);
}

function CarPlatform::onObjectAttached(%this, %obj)
{  // Give them a message when they get on the platform
   if ( !isObject(%obj.client) )
      return;

   %driverAvailable = (ClientGroup.getCount() > 1);
   if ( %driverAvailable )
   {
      %driver = %this.car.getMountNodeObject(0);         
      if( isObject(%driver) )
         messageClient(%obj.client, 'MsgItemPickup', '\c1Hang On!');
      else
         messageAllExcept(%obj.client, -1, 'MsgItemPickup', '\c2%1 is on the Cheetah and needs a driver...', %obj.client.playerName);
   }
   else
      messageClient(%obj.client, 'MsgItemPickup', '\c2Good luck finding a driver...');
}

//-----------------------------------------------------------------------------
// FirstPlatform is the platform the user steps on to take them up to the game.
// The first time they get on, Let them know the objective and record their
// start time. When they get off, return to the start position.
//-----------------------------------------------------------------------------
function FirstPlatform::onAdd(%this)
{
   Parent::onAdd(%this);

   // This is not a looping path. It travels between two fixed positions.
   %this.setLooping(false);
}

function FirstPlatform::onObjectAttached(%this, %obj)
{  // Start moving forward whenever we get a rider
   %this.setMoveState("Forward");

   if ( isObject(%obj.client) )
   {
      if ( !%obj.client.gotInstructions )
      {
         messageClient(%obj.client, 'MsgItemPickup', '\c1Hop to the center grey platform to win.');
         %obj.client.gotInstructions = true;
      }
      if ( %obj.client.miniStartTime $= "" )
      {  // Record this as their start time and clear the tracking variables
         %obj.client.miniStartTime = getSimTime();
         for (%i = 1; %i < 4; %i++)
            %obj.client.touchedPlatform[%i] = false;
      }
   }

   Parent::onObjectAttached(%this, %obj);
}

function FirstPlatform::onObjectDetached(%this, %obj)
{  // If no riders are left, return to the start position
   if ( !%this.hasHumanRider() )
      %this.setMoveState("Backward");

   Parent::onObjectDetached(%this, %obj);
}

//-----------------------------------------------------------------------------
// TargetPlatform is the center platform. Reaching this platform is the goal of
// the mini game.
//-----------------------------------------------------------------------------
function TargetPlatform::onAdd(%this)
{
   Parent::onAdd(%this);
   %this.setLooping(false);

   // Put some lights on it
   %this.mountObject(RedLight, 0, "0.15 0 9 1 0 0 64");
   %this.mountObject(BlueLight, 0, "-0.15 0 9 1 0 0 90");
}

function TargetPlatform::onNode(%this, %node)
{  // Run back and forth between nodes 0 and 4 unless someone is on the platform
   if ( !%this.hasHumanRider() )
   {
      if ( %node == 0 )
         %this.setMoveState("Forward");
      else if ( %node == 4 )
         %this.setMoveState("Backward");
   }
}

function TargetPlatform::onObjectAttached(%this, %obj)
{  // Go to the custom cheetah and give the 'win' message
   if ( isObject(%obj.client) )
   {
      %this.setMoveState("Forward");
      %this.checkWinner(%obj.client);
   }

   Parent::onObjectAttached(%this, %obj);
}

function TargetPlatform::onObjectDetached(%this, %obj)
{  // If no riders are left, return to the start
   if ( !%this.hasHumanRider() )
      %this.setMoveState("Backward");

   Parent::onObjectDetached(%this, %obj);
}

function TargetPlatform::checkWinner(%this, %client)
{
   for (%i = 1; %i < 4; %i++)
      if ( !%client.touchedPlatform[%i] )
         return;  // Skipped a platform, no winner message for you!

   // Get the total time it took them
   %totalTime = getSimTime() - %client.miniStartTime;
   %timeStr = timeToText(%totalTime) @ ".";
   
   // See if they beat the best time
   if ( $Pref::Server::miniWalkableBest $= "" )
      $Pref::Server::miniWalkableBest = %totalTime;
   else if ( %totalTime < $Pref::Server::miniWalkableBest )
   {
      $Pref::Server::miniWalkableBest = %totalTime;
      %timeStr = %timeStr @ "\nA new best time!";
   }

   messageClient(%client, 'MsgItemPickup',
         '\c1Congratulations! You reached the platform in %1', %timeStr);
   messageAllExcept(%client, -1, 'MsgItemPickup',
         '\c1%1 reached the platform in %2', %client.playerName, %timeStr);

   // Reset so they can play again
   %client.miniStartTime = "";
}

function timeToText(%timeMS)
{  // Convert ms to text description
   %totalSeconds = mFloor(%timeMS / 1000);
   %minutes = mFloor(%totalSeconds / 60);
   %seconds = %totalSeconds % 60;

   %timeStr = %minutes SPC "Minute";
   if ( %minutes != 1 )
      %timeStr = %timeStr @ "s";
   %timeStr = %timeStr @ ", " @ %seconds SPC "Second";
   if ( %seconds != 1 )
      %timeStr = %timeStr @ "s";

   return %timeStr;
}

//-----------------------------------------------------------------------------
// Override the default onObjectAttached() so we can keep track of platforms
// touched.
//-----------------------------------------------------------------------------
function TSAttachable::onObjectAttached(%this, %obj)
{
   if ( isObject(%obj.client) && (%this.platformNum !$= "") )
      %obj.client.touchedPlatform[%this.platformNum] = true;
}

//-----------------------------------------------------------------------------
// StartTrigger is located at the player spawn point so we can give them some 
// instructions the first time they spawn.
//-----------------------------------------------------------------------------
datablock TriggerData(StartTrigger : DefaultTrigger)
{
   tickPeriodMS = 500;
};

function StartTrigger::onEnterTrigger(%this,%trigger,%obj)
{
   if ( isObject(%obj.client) && !%obj.client.welcomed )
   {
       messageClient(%obj.client, 'MsgItemPickup', '\c1There\'s a platform in-front of you...Jump on.');
       %obj.client.welcomed = true;
   }
}
