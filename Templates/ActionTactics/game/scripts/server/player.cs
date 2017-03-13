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

// Timeouts for corpse deletion.
$CorpseTimeoutValue = 45 * 1000;


//----------------------------------------------------------------------------
// Player Datablock methods
//----------------------------------------------------------------------------

function PlayerData::onAdd(%this, %obj)
{
   // Vehicle timeout
   %obj.mountVehicle = false;//no vehicles in ActionTactics Template

   // Default dynamic armor stats
   %obj.setRechargeRate(0);//stop auto-recharge (%this.rechargeRate);
   %obj.setRepairRate(0);
   
   // Calling updates must have a short delay whilst object spawns
   %obj.schedule(50, "restoreEnergy");
}

function PlayerData::onRemove(%this, %obj)
{
   if (%obj.client.player == %obj)
      %obj.client.player = 0;
}

function PlayerData::onNewDataBlock(%this, %obj)
{
}

//----------------------------------------------------------------------------

function PlayerData::onMount(%this, %obj, %vehicle, %node)
{
   // Node 0 is the pilot's position, we need to dismount his weapon.
   if (%node == 0)
   {
      %obj.setTransform("0 0 0 0 0 1 0");
      %obj.setActionThread(%vehicle.getDatablock().mountPose[%node], true, true);

      %obj.lastWeapon = %obj.getMountedImage($WeaponSlot);
      %obj.unmountImage($WeaponSlot);

      %obj.setControlObject(%vehicle);
      
      if(%obj.getClassName() $= "Player")
         commandToClient(%obj.client, 'toggleVehicleMap', true);
   }
   else
   {
      if (%vehicle.getDataBlock().mountPose[%node] !$= "")
         %obj.setActionThread(%vehicle.getDatablock().mountPose[%node]);
      else
         %obj.setActionThread("root", true);
   }
}

function PlayerData::onUnmount(%this, %obj, %vehicle, %node)
{
   if (%node == 0)
   {
      %obj.mountImage(%obj.lastWeapon, $WeaponSlot);
      %obj.setControlObject("");
   }
}

function PlayerData::doDismount(%this, %obj, %forced)
{
   //echo("\c4PlayerData::doDismount(" @ %this @", "@ %obj.client.nameBase @", "@ %forced @")");

   // This function is called by player.cc when the jump trigger
   // is true while mounted
   %vehicle = %obj.mVehicle;
   if (!%obj.isMounted() || !isObject(%vehicle))
      return;

   // Vehicle must be at rest!
   if ((VectorLen(%vehicle.getVelocity()) <= %vehicle.getDataBlock().maxDismountSpeed ) || %forced)
   {
      // Position above dismount point
      %pos = getWords(%obj.getTransform(), 0, 2);
      %rot = getWords(%obj.getTransform(), 3, 6);
      %oldPos = %pos;
      %vec[0] = " -1 0 0";
      %vec[1] = " 0 0 1";
      %vec[2] = " 0 0 -1";
      %vec[3] = " 1 0 0";
      %vec[4] = "0 -1 0";
      %impulseVec = "0 0 0";
      %vec[0] = MatrixMulVector(%obj.getTransform(), %vec[0]);

      // Make sure the point is valid
      %pos = "0 0 0";
      %numAttempts = 5;
      %success = -1;
      for (%i = 0; %i < %numAttempts; %i++)
      {
         %pos = VectorAdd(%oldPos, VectorScale(%vec[%i], 3));
         if (%obj.checkDismountPoint(%oldPos, %pos))
         {
            %success = %i;
            %impulseVec = %vec[%i];
            break;
         }
      }
      if (%forced && %success == -1)
         %pos = %oldPos;

      %obj.mountVehicle = false;
      %obj.schedule(4000, "mountVehicles", true);

      // Position above dismount point
      %obj.unmount();
      %obj.setTransform(%pos SPC %rot);//%obj.setTransform(%pos);
      //%obj.playAudio(0, UnmountVehicleSound);
      %obj.applyImpulse(%pos, VectorScale(%impulseVec, %obj.getDataBlock().mass));

      // Set player velocity when ejecting
      %vel = %obj.getVelocity();
      %vec = vectorDot( %vel, vectorNormalize(%vel));
      if(%vec > 50)
      {
         %scale = 50 / %vec;
         %obj.setVelocity(VectorScale(%vel, %scale));
      }

      //%obj.vehicleTurret = "";
   }
   else
      messageClient(%obj.client, 'msgUnmount', '\c2Cannot exit %1 while moving.', %vehicle.getDataBlock().nameTag);
}

//----------------------------------------------------------------------------

function PlayerData::onCollision(%this, %obj, %col)
{
   if (!isObject(%col) || %obj.getState() $= "Dead")
      return;

   // Try and pickup all items
   if (%col.getClassName() $= "Item")
   {
      %obj.pickup(%col);
      return;
   }

   // Mount vehicles
   if (%col.getType() & $TypeMasks::GameBaseObjectType)
   {
      %db = %col.getDataBlock();
      if ((%db.getClassName() $= "WheeledVehicleData" ) && %obj.mountVehicle && %obj.getState() $= "Move" && %col.mountable)
      {
         // Only mount drivers for now.
         ServerConnection.setFirstPerson(0);
         
         // For this specific example, only one person can fit
         // into a vehicle
         %mount = %col.getMountNodeObject(0);         
         if(%mount)
            return;
         
         // For this specific FPS Example, always mount the player
         // to node 0
         %node = 0;
         %col.mountObject(%obj, %node);
         %obj.mVehicle = %col;
      }
   }
}

function PlayerData::onImpact(%this, %obj, %collidedObject, %vec, %vecLen)
{
   %obj.damage(0, VectorAdd(%obj.getPosition(), %vec), %vecLen * %this.speedDamageScale, "Impact");
}

//----------------------------------------------------------------------------

function PlayerData::damage(%this, %obj, %sourceObject, %position, %damage, %damageType)
{
   if (!isObject(%obj) || %obj.getState() $= "Dead" || !%damage)
      return;
      
   %location = %obj.getDamageLocation(%position);
   %bodyPart = getWord(%location, 0);
   %region = getWord(%location, 1);
   echo("\c4DAMAGELOCATION:  bodyPart = "@ %bodyPart @" || REGION = "@ %region);
   // BODYPARTS:  HEAD | TORSO | LEGS
   // REGION (legs/Torso):  front_left | front_right | back_left | back_right
   // REGION (head):  left_back   | left_middle   | left front
   //                 middle_back | middle_middle | middle_front
   //                 right_back  | right_middle  | right_front
   
   //make sure that the damage is from a projectile, else damage is full
   if (%damageType $= "BulletProjectile")
   {
      //headshots do most damage
      if(%bodyPart $="head")
         %damage=%damage * 2;
      //torso does standard damage
      if(%bodyPart $="torso")
         %damage=%damage;
      //legs/low does half damage
      if(%bodyPart $="legs")
         %damage=%damage / 2;
   }
      
   //make certain that damage is a round number - no fractions
   %damage = mceil(%damage);

   %obj.applyDamage(%damage);

   //%location = "Body";
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   
   /*
   // Deal with client callbacks here because we don't have this
   // information in the onDamage or onDisable methods
   %client = %obj.client;
   %sourceClient = %sourceObject ? %sourceObject.client : 0;
   
   if (isObject(%client))
   {
      // Determine damage direction
      if (%damageType !$= "Suicide")
         %obj.setDamageDirection(%sourceObject, %position);

      if (%obj.getState() $= "Dead")
         %client.onDeath(%sourceObject, %sourceClient, %damageType, %location);
   }
   */
   
   // ActionTactics Update the numerical Health HUD
   if(%client.player == %obj)
		%obj.updateHealth();
   
   // ActionTactics call the gametype specific death in gameActionTactics.cs
   if(%obj.getState() $="Dead")
      game.GameSpecificDeath(%obj);
}

function PlayerData::onDamage(%this, %obj, %delta)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage level changes.
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   if(%client.player == %obj)
   {
      %damage = %obj.getDamageLevel();
      if(%damage > 0)
      {
         %obj.playPain();
         %obj.setDamageFlash(0.75);
      }
   }
   else
   {
      %obj.playDeathCry();
   }
}

// ----------------------------------------------------------------------------
// The player object sets the "disabled" state when damage exceeds it's
// maxDamage value. This is method is invoked by ShapeBase state mangement code.

// If we want to deal with the damage information that actually caused this
// death, then we would have to move this code into the script "damage" method.

function PlayerData::onDisabled(%this, %obj, %state)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);

   // Release the main weapon trigger
   %obj.setImageTrigger(0, false);

   // Toss current mounted weapon and ammo if any
   %item = %obj.getMountedImage($WeaponSlot).item;
   if (isObject(%item))
   {
      %amount = %obj.getInventory(%item.image.ammo);

      if(%amount)
         %obj.throw(%item.image.ammo, %amount);
   }

   // Toss out a health patch
   %obj.tossPatch();

   %obj.playDeathCry();
   %obj.playDeathAnimation();
   %obj.unMountImage(%slot);

   // Disable any vehicle map
   //commandToClient(%obj.client, 'toggleVehicleMap', false);

   // Schedule corpse removal. Just keeping the place clean.
   %obj.schedule($CorpseTimeoutValue - 1000, "startFade", 1000, 0, true);
   %obj.schedule($CorpseTimeoutValue, "delete");
   
   //ActionTactics
   if(%client.player == %obj)
	{
		%obj.setEnergyLevel(0);
		%obj.updateEnergy();
		%obj.setDamageFlash(0.75);
	}
	
	%obj.setShapeName("");
	
	if( %obj.decal > -1 )
	   decalManagerRemoveDecal( %client.player.decal );
}

//-----------------------------------------------------------------------------

function PlayerData::onLeaveMissionArea(%this, %obj)
{
   //echo("\c4Leaving Mission Area at POS:"@ %obj.getPosition());

   // Inform the client
   %obj.client.onLeaveMissionArea();

   // Damage over time and kill the coward!
   //%obj.setDamageDt(0.2, "MissionAreaDamage");
}

function PlayerData::onEnterMissionArea(%this, %obj)
{
   //echo("\c4Entering Mission Area at POS:"@ %obj.getPosition());

   // Inform the client
   %obj.client.onEnterMissionArea();

   // Stop the punishment
   //%obj.clearDamageDt();
}

//-----------------------------------------------------------------------------

function PlayerData::onEnterLiquid(%this, %obj, %coverage, %type)
{
   //echo("\c4this:"@ %this @" object:"@ %obj @" just entered water of type:"@ %type @" for "@ %coverage @"coverage");
}

function PlayerData::onLeaveLiquid(%this, %obj, %type)
{
   //
}

//-----------------------------------------------------------------------------

function PlayerData::onTrigger(%this, %obj, %triggerNum, %val)
{
   // This method is invoked when the player receives a trigger move event.
   // The player automatically triggers slot 0 and slot one off of triggers #
   // 0 & 1.  Trigger # 2 is also used as the jump key.
}

//-----------------------------------------------------------------------------

function PlayerData::onPoseChange(%this, %obj, %oldPose, %newPose)
{
   // Set the script anim prefix to be that of the current pose
   %obj.setImageScriptAnimPrefix( $WeaponSlot, addTaggedString(%newPose) );
}

//-----------------------------------------------------------------------------

function PlayerData::onStartSprintMotion(%this, %obj)
{
   %obj.setImageGenericTrigger($WeaponSlot, 0, true);
}

function PlayerData::onStopSprintMotion(%this, %obj)
{
   %obj.setImageGenericTrigger($WeaponSlot, 0, false);
}

//-----------------------------------------------------------------------------
// Player methods
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------

function Player::kill(%this, %damageType)
{
   %this.damage(0, %this.getPosition(), 10000, %damageType);
}

//----------------------------------------------------------------------------

function Player::mountVehicles(%this, %bool)
{
   // If set to false, this variable disables vehicle mounting.
   %this.mountVehicle = %bool;
}

function Player::isPilot(%this)
{
   %vehicle = %this.getObjectMount();
   // There are two "if" statements to avoid a script warning.
   if (%vehicle)
      if (%vehicle.getMountNodeObject(0) == %this)
         return true;
   return false;
}

//----------------------------------------------------------------------------

function Player::playDeathAnimation(%this)
{
   %numDeathAnimations = %this.getNumDeathAnimations();
   if ( %numDeathAnimations > 0 )
   {
      if (isObject(%this.client))
      {
         if (%this.client.deathIdx++ > %numDeathAnimations)
            %this.client.deathIdx = 1;
         %this.setActionThread("Death" @ %this.client.deathIdx);
      }
      else
      {
         %rand = getRandom(1, %numDeathAnimations);
         %this.setActionThread("Death" @ %rand);
      }
   }
}

function Player::playCelAnimation(%this, %anim)
{
   if (%this.getState() !$= "Dead")
      %this.setActionThread("cel"@%anim);
}


//----------------------------------------------------------------------------

function Player::playDeathCry(%this)
{
   %this.playAudio(0, DeathCrySound);
}

function Player::playPain(%this)
{
   %this.playAudio(0, PainCrySound);
}

// ----------------------------------------------------------------------------

function Player::setDamageDirection(%player, %sourceObject, %damagePos)
{
   if (isObject(%sourceObject))
   {
      if (%sourceObject.isField(initialPosition))
      {
         // Projectiles have this field set to the muzzle point of
         // the firing weapon at the time the projectile was created.
         // This gives a damage direction towards the firing player,
         // turret, vehicle, etc.  Bullets and weapon fired grenades
         // are examples of projectiles.
         %damagePos = %sourceObject.initialPosition;
      }
      else
      {
         // Other objects that cause damage, such as mines, use their own
         // location as the damage position.  This gives a damage direction
         // towards the explosive origin rather than the person that lay the
         // explosives.
         %damagePos = %sourceObject.getPosition();
      }
   }

   // Rotate damage vector into object space
   %damageVec = VectorSub(%damagePos, %player.getWorldBoxCenter());
   %damageVec = VectorNormalize(%damageVec);
   %damageVec = MatrixMulVector(%player.client.getCameraObject().getInverseTransform(), %damageVec);

   // Determine largest component of damage vector to get direction
   %vecComponents = -%damageVec.x SPC %damageVec.x SPC -%damageVec.y SPC %damageVec.y SPC -%damageVec.z SPC %damageVec.z;
   %vecDirections = "Left"        SPC "Right"      SPC "Bottom"      SPC "Front"      SPC "Bottom"      SPC "Top";

   %max = -1;
   for (%i = 0; %i < 6; %i++)
   {
      %value = getWord(%vecComponents, %i);
      if (%value > %max)
      {
         %max = %value;
         %damageDir = getWord(%vecDirections, %i);
      }
   }
   commandToClient(%player.client, 'setDamageDirection', %damageDir);
}

function Player::use(%player, %data)
{
   // No mounting/using weapons when you're driving!
   if (%player.isPilot())
      return(false);

   Parent::use(%player, %data);
}

// -----------------------------------------------------------------------------
// ActionTactics
// -----------------------------------------------------------------------------

function Player::updateHealth(%player)
{
   //echo("\c4Player::updateHealth() -> Player Health changed, updating HUD!");
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);

   // Calcualte player health
   %maxDamage = %player.getDatablock().maxDamage;
   %damageLevel = %player.getDamageLevel();
   %curHealth = %maxDamage - %damageLevel;
   %curHealth = mceil(%curHealth);
   
   //this fixes an issue with how healthpacks work on a schedule
   //so it only displays the changes on the healed playerObject when you switch team members
   if(%player == %client.player)
      commandToClient(%client, 'setNumericalHealthHUD', %curHealth);
}

function Player::updateEnergy(%player)
{
   //echo("\c4Player::updateEnergy() -> Player Energy changed, updating HUD!");
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);

   %energyLevel = %player.getEnergyLevel();
   %curEnergy = mceil(%energyLevel);

   // Send the player object's current energy level to the client, where it
   // will Update the numericalEnergy HUD.
   if(%player == %client.player)
      commandToClient(%client, 'setNumericalEnergyHUD', %curEnergy);
}

function Player::restoreEnergy(%player)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   
   %maxEnergy = %player.getDatablock().maxEnergy;
   %player.setEnergyLevel(%maxEnergy);
   
   if(%player == %client.player)
      commandToClient(%client, 'setNumericalEnergyHUD', %maxEnergy);
}

function Player::decEnergy(%player)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
   
	if(%player.getVelocity() !$="0 0 0")
	{
		%curEnergy = %player.getEnergyLevel() -1;
		%player.setEnergyLevel(%curEnergy);
		%player.updateEnergy();
		
		if(%curEnergy < 1)
		{
			%player.stop();
			messageClient(%client, 'MsgPlayer', '\c0%1 - Cannot Move - Out Of Energy!', %player.getName());
			
			if( %player.decal > -1 )
				decalManagerRemoveDecal( %player.decal );
			return;
		}
		
		%player.schedule(250, "decEnergy");
	}
}