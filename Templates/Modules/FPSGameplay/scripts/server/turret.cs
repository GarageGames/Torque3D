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

// Respawntime is the amount of time it takes for a static "auto-respawn"
// turret to re-appear after it's been picked up.  Any turret marked as "static"
// is automaticlly respawned.
$TurretShape::RespawnTime = 30 * 1000;

// DestroyedFadeDelay is the how long a destroyed turret sticks around before it
// fades out and is deleted.
$TurretShape::DestroyedFadeDelay = 5 * 1000;

// ----------------------------------------------------------------------------
// TurretShapeData
// ----------------------------------------------------------------------------

function TurretShapeData::onAdd(%this, %obj)
{
   %obj.setRechargeRate(%this.rechargeRate);
   %obj.setEnergyLevel(%this.MaxEnergy);
   %obj.setRepairRate(0);

   if (%obj.mountable || %obj.mountable $= "")
      %this.isMountable(%obj, true);
   else
      %this.isMountable(%obj, false);

   if (%this.nameTag !$= "")
      %obj.setShapeName(%this.nameTag);

   // Mount weapons
   for(%i = 0; %i < %this.numWeaponMountPoints; %i++)
   {
      // Handle inventory
      %obj.incInventory(%this.weapon[%i], 1);
      %obj.incInventory(%this.weaponAmmo[%i], %this.weaponAmmoAmount[%i]);
      
      // Mount the image
      %obj.mountImage(%this.weapon[%i].image, %i, %this.startLoaded);
      %obj.setImageGenericTrigger(%i, 0, false); // Used to indicate the turret is destroyed
   }

   if (%this.enterSequence !$= "")
   {
      %obj.entranceThread = 0;
      %obj.playThread(%obj.entranceThread, %this.enterSequence);
      %obj.pauseThread(%obj.entranceThread);
   }
   else
   {
      %obj.entranceThread = -1;
   }
}

function TurretShapeData::onRemove(%this, %obj)
{
   //echo("\c4TurretShapeData::onRemove("@ %this.getName() @", "@ %obj.getClassName() @")");

   // if there are passengers/driver, kick them out
   for(%i = 0; %i < %this.numMountPoints; %i++)
   {
      if (%obj.getMountNodeObject(%i))
      {
         %passenger = %obj.getMountNodeObject(%i);
         %passenger.getDataBlock().doDismount(%passenger, true);
      }
   }
}

// This is on MissionGroup so it doesn't happen when the mission has ended
function MissionGroup::respawnTurret(%this, %datablock, %className, %transform, %static, %respawn)
{
   %turret = new (%className)()
   {
      datablock = %datablock;
      static = %static;
      respawn = %respawn;
   };

   %turret.setTransform(%transform);
   MissionGroup.add(%turret);
   return %turret;
}

// ----------------------------------------------------------------------------
// TurretShapeData damage state
// ----------------------------------------------------------------------------

// This method is called by weapons fire
function TurretShapeData::damage(%this, %turret, %sourceObject, %position, %damage, %damageType)
{
   //echo("\TurretShapeData::damage(" @ %turret @ ", "@ %sourceObject @ ", " @ %position @ ", "@ %damage @ ", "@ %damageType @ ")");

   if (%turret.getState() $= "Dead")
      return;

   %turret.applyDamage(%damage);

   // Update the numerical Health HUD
   %mountedObject = %turret.getObjectMount();
   if (%mountedObject)
      %mountedObject.updateHealth();

   // Kill any occupants
   if (%turret.getState() $= "Dead")
   {
      for (%i = 0; %i < %this.numMountPoints; %i++)
      {
         %player = %turret.getMountNodeObject(%i);
         if (%player != 0)
            %player.killWithSource(%sourceObject, "InsideTurret");
      }
   }
}

function TurretShapeData::onDamage(%this, %obj, %delta)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage level changes.
}

function TurretShapeData::onDestroyed(%this, %obj, %lastState)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage state changes.

   // Fade out the destroyed object.  Then schedule a return.
   %obj.startFade(1000, $TurretShape::DestroyedFadeDelay, true);
   %obj.schedule($TurretShape::DestroyedFadeDelay + 1000, "delete");

   if (%obj.doRespawn())
   {
      MissionGroup.schedule($TurretShape::RespawnTime, "respawnTurret", %this, %obj.getClassName(), %obj.getTransform(), true, true);
   }
}

function TurretShapeData::onDisabled(%this, %obj, %lastState)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage state changes.
}

function TurretShapeData::onEnabled(%this, %obj, %lastState)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage state changes.
}

// ----------------------------------------------------------------------------
// TurretShapeData player mounting and dismounting
// ----------------------------------------------------------------------------

function TurretShapeData::isMountable(%this, %obj, %val)
{
   %obj.mountable = %val;
}

function TurretShapeData::onMountObject(%this, %turret, %player, %node)
{
   if (%turret.entranceThread >= 0)
   {
      %turret.setThreadDir(%turret.entranceThread, true);
      %turret.setThreadPosition(%turret.entranceThread, 0);
      %turret.playThread(%turret.entranceThread, "");
   }
}

function TurretShapeData::onUnmountObject(%this, %turret, %player)
{
   if (%turret.entranceThread >= 0)
   {
      // Play the entrance thread backwards for an exit
      %turret.setThreadDir(%turret.entranceThread, false);
      %turret.setThreadPosition(%turret.entranceThread, 1);
      %turret.playThread(%turret.entranceThread, "");
   }
}

function TurretShapeData::mountPlayer(%this, %turret, %player)
{
   //echo("\c4TurretShapeData::mountPlayer("@ %this.getName() @", "@ %turret @", "@ %player.client.nameBase @")");

   if (isObject(%turret) && %turret.getDamageState() !$= "Destroyed")
   {
      //%player.startFade(1000, 0, true);
      //%this.schedule(1000, "setMountTurret", %turret, %player);
      //%player.schedule(1500, "startFade", 1000, 0, false);
      %this.setMountTurret(%turret, %player);
   }
}

function TurretShapeData::setMountTurret(%this, %turret, %player)
{
   //echo("\c4TurretShapeData::setMountTurret("@ %this.getName() @", "@ %turret @", "@ %player.client.nameBase @")");

   if (isObject(%turret) && %turret.getDamageState() !$= "Destroyed")
   {
      %node = %this.findEmptySeat(%turret, %player);
      if (%node >= 0)
      {
         //echo("\c4Mount Node: "@ %node);
         %turret.mountObject(%player, %node);
         //%player.playAudio(0, MountVehicleSound);
         %player.mVehicle = %turret;
      }
   }
}

function TurretShapeData::findEmptySeat(%this, %turret, %player)
{
   //echo("\c4This turret has "@ %this.numMountPoints @" mount points.");

   for (%i = 0; %i < %this.numMountPoints; %i++)
   {
      %node = %turret.getMountNodeObject(%i);
      if (%node == 0)
         return %i;
   }
   return -1;
}

function TurretShapeData::switchSeats(%this, %turret, %player)
{
   for (%i = 0; %i < %this.numMountPoints; %i++)
   {
      %node = %turret.getMountNodeObject(%i);
      if (%node == %player || %node > 0)
         continue;

      if (%node == 0)
         return %i;
   }
   return -1;
}

function TurretShapeData::onMount(%this, %turret, %player, %node)
{
   //echo("\c4TurretShapeData::onMount("@ %this.getName() @", "@ %turret @", "@ %player.client.nameBase @")");

   %player.client.RefreshVehicleHud(%turret, %this.reticle, %this.zoomReticle);
   //%player.client.UpdateVehicleHealth(%turret);
}

function TurretShapeData::onUnmount(%this, %turret, %player, %node)
{
   //echo("\c4TurretShapeData::onUnmount(" @ %this.getName() @ ", " @ %turret @ ", " @ %player.client.nameBase @ ")");

   %player.client.RefreshVehicleHud(0, "", "");
}

// ----------------------------------------------------------------------------
// TurretShape damage
// ----------------------------------------------------------------------------

// This method is called by weapons fire
function TurretShape::damage(%this, %sourceObject, %position, %damage, %damageType)
{
   //echo("\TurretShape::damage(" @ %this @ ", "@ %sourceObject @ ", " @ %position @ ", "@ %damage @ ", "@ %damageType @ ")");

   %this.getDataBlock().damage(%this, %sourceObject, %position, %damage, %damageType);
}

// ----------------------------------------------------------------------------
// TurretDamage
// ----------------------------------------------------------------------------

// Customized kill message for deaths caused by turrets
function sendMsgClientKilled_TurretDamage( %msgType, %client, %sourceClient, %damLoc )
{
   if ( %sourceClient $= "" )             // editor placed turret
      messageAll( %msgType, '%1 was shot down by a turret!', %client.playerName );
   else if ( %sourceClient == %client )   // own mine
      messageAll( %msgType, '%1 kill by his own turret!', %client.playerName );
   else                                   // enemy placed mine
      messageAll( %msgType, '%1 was killed by a turret of %2!', %client.playerName, %sourceClient.playerName );
}

// ----------------------------------------------------------------------------
// AITurretShapeData
// ----------------------------------------------------------------------------

function AITurretShapeData::onAdd(%this, %obj)
{
   Parent::onAdd(%this, %obj);

   %obj.mountable = false;
}

// Player has thrown a deployable turret.  This copies from ItemData::onThrow()
function AITurretShapeData::onThrow(%this, %user, %amount)
{
   // Remove the object from the inventory
   if (%amount $= "")
      %amount = 1;
   if (%this.maxInventory !$= "")
      if (%amount > %this.maxInventory)
         %amount = %this.maxInventory;
   if (!%amount)
      return 0;
   %user.decInventory(%this,%amount);

   // Construct the actual object in the world, and add it to
   // the mission group so it's cleaned up when the mission is
   // done.  The turret's rotation matches the player's.
   %rot = %user.getEulerRotation();
   %obj = new AITurretShape()
   {
      datablock = %this;
      rotation = "0 0 1 " @ getWord(%rot, 2);
      count = 1;
      sourceObject = %user;
      client = %user.client;
      isAiControlled = true;
   };
   MissionGroup.add(%obj);
   
   // Let the turret know that we're a firend
   %obj.addToIgnoreList(%user);

   // We need to add this turret to a list on the client so that if we die,
   // the turret will still ignore our player.
   %client = %user.client;
   if (%client)
   {
      if (!%client.ownedTurrets)
      {
         %client.ownedTurrets = new SimSet();
      }
      
      // Go through the client's owned turret list.  Make sure we're
      // a friend of every turret and every turret is a friend of ours.
      // Commence hugging!
      for (%i=0; %i<%client.ownedTurrets.getCount(); %i++)
      {
         %turret = %client.ownedTurrets.getObject(%i);
         %turret.addToIgnoreList(%obj);
         %obj.addToIgnoreList(%turret);
      }
      
      // Add ourselves to the client's owned list.
      %client.ownedTurrets.add(%obj);
   }
   
   return %obj;
}

function AITurretShapeData::onDestroyed(%this, %turret, %lastState)
{
   // This method is invoked by the ShapeBase code whenever the
   // object's damage state changes.

   %turret.playAudio(0, TurretDestroyed);
   %turret.setAllGunsFiring(false);
   %turret.resetTarget();
   %turret.setTurretState( "Destroyed", true );

   // Set the weapons to destoryed
   for(%i = 0; %i < %this.numWeaponMountPoints; %i++)
   {
      %turret.setImageGenericTrigger(%i, 0, true);
   }

   Parent::onDestroyed(%this, %turret, %lastState);
}

function AITurretShapeData::OnScanning(%this, %turret)
{
   //echo("AITurretShapeData::OnScanning: " SPC %this SPC %turret);

   %turret.startScanForTargets();
   %turret.playAudio(0, TurretScanningSound);
}

function AITurretShapeData::OnTarget(%this, %turret)
{
   //echo("AITurretShapeData::OnTarget: " SPC %this SPC %turret);

   %turret.startTrackingTarget();
   %turret.playAudio(0, TargetAquiredSound);
}

function AITurretShapeData::OnNoTarget(%this, %turret)
{
   //echo("AITurretShapeData::OnNoTarget: " SPC %this SPC %turret);

   %turret.setAllGunsFiring(false);
   %turret.recenterTurret();
   %turret.playAudio(0, TargetLostSound);
}

function AITurretShapeData::OnFiring(%this, %turret)
{
   //echo("AITurretShapeData::OnFiring: " SPC %this SPC %turret);

   %turret.setAllGunsFiring(true);
}

function AITurretShapeData::OnThrown(%this, %turret)
{
   //echo("AITurretShapeData::OnThrown: " SPC %this SPC %turret);

   %turret.playAudio(0, TurretThrown);
}

function AITurretShapeData::OnDeploy(%this, %turret)
{
   //echo("AITurretShapeData::OnDeploy: " SPC %this SPC %turret);

   // Set the weapons to loaded
   for(%i = 0; %i < %this.numWeaponMountPoints; %i++)
   {
      %turret.setImageLoaded(%i, true);
   }
   
   %turret.playAudio(0, TurretActivatedSound);
}


// ----------------------------------------------------------------------------
// Player deployable turret
// ----------------------------------------------------------------------------

// Cannot use the Weapon class for deployable turrets as it is already tied
// to ItemData.

function DeployableTurretWeapon::onUse(%this, %obj)
{
   Weapon::onUse(%this, %obj);
}

function DeployableTurretWeapon::onPickup(%this, %obj, %shape, %amount)
{
   Weapon::onPickup(%this, %obj, %shape, %amount);
}

function DeployableTurretWeapon::onInventory(%this, %obj, %amount)
{
   if (%obj.client !$= "" && !%obj.isAiControlled)
   {
      %obj.client.setAmmoAmountHud( 1, %amount );
   }

   // Cycle weapons if we are out of ammo
   if ( !%amount && ( %slot = %obj.getMountSlot( %this.image ) ) != -1 )
      %obj.cycleWeapon( "prev" );
}

function DeployableTurretWeaponImage::onMount(%this, %obj, %slot)
{
   // The turret doesn't use ammo from a player's perspective.
   %obj.setImageAmmo(%slot, true);
   %numTurrets = %obj.getInventory(%this.item);
   
   if (%obj.client !$= "" && !%obj.isAiControlled)
      %obj.client.RefreshWeaponHud( 1, %this.item.previewImage, %this.item.reticle, %this.item.zoomReticle, %numTurrets);
}

function DeployableTurretWeaponImage::onUnmount(%this, %obj, %slot)
{
   if (%obj.client !$= "" && !%obj.isAiControlled)
      %obj.client.RefreshWeaponHud(0, "", "");
}

function DeployableTurretWeaponImage::onFire(%this, %obj, %slot)
{
   //echo("\DeployableTurretWeaponImage::onFire( "@%this.getName()@", "@%obj.client.nameBase@", "@%slot@" )");

   // To fire a deployable turret is to throw it.  Schedule the throw
   // so that it doesn't happen during this ShapeBaseImageData's state machine.
   // If we throw the last one then we end up unmounting while the state machine
   // is still being processed.
   %obj.schedule(0, "throw", %this.item);
}
