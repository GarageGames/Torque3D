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


// Damage Rate for entering Liquid
$DamageLava       = 0.01;
$DamageHotLava    = 0.01;
$DamageCrustyLava = 0.01;

//----------------------------------------------------------------------------
// DefaultPlayerData Datablock methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

function DefaultPlayerData::onAdd(%this,%obj)
{
   // Vehicle timeout
   %obj.mountVehicle = true;

   // Default dynamic armor stats
   %obj.setRechargeRate(%this.rechargeRate);
   %obj.setRepairRate(0);
   // Calling updateHealth() must be delayed now... for some reason
   %obj.schedule(50, "updateHealth");
  
}

function DefaultPlayerData::onRemove(%this, %obj)
{
   if (%obj.client.player == %obj)
      %obj.client.player = 0;
}

function DefaultPlayerData::onNewDataBlock(%this,%obj)
{
}


//----------------------------------------------------------------------------

function DefaultPlayerData::onMount(%this,%obj,%vehicle,%node)
{
   if (%node == 0)  {
      %obj.setTransform("0 0 0 0 0 1 0");
      %obj.setActionThread(%vehicle.getDatablock().mountPose[%node],true,true);
      %obj.lastWeapon = %obj.getMountedImage($WeaponSlot);

      %obj.unmountImage($WeaponSlot);
      %obj.setControlObject(%vehicle);
      %obj.client.setObjectActiveImage(%vehicle, 2);
   }
}

function DefaultPlayerData::onUnmount( %this, %obj, %vehicle, %node )
{
   if (%node == 0)
      %obj.mountImage(%obj.lastWeapon, $WeaponSlot);
}

function DefaultPlayerData::doDismount(%this, %obj, %forced)
{
   // This function is called by player.cc when the jump trigger
   // is true while mounted
   if (!%obj.isMounted())
      return;

   // Position above dismount point
   %pos    = getWords(%obj.getTransform(), 0, 2);
   %oldPos = %pos;
   %vec[0] = " 0  0  1";
   %vec[1] = " 0  0  1";
   %vec[2] = " 0  0 -1";
   %vec[3] = " 1  0  0";
   %vec[4] = "-1  0  0";
   %impulseVec  = "0 0 0";
   %vec[0] = MatrixMulVector( %obj.getTransform(), %vec[0]);

   // Make sure the point is valid
   %pos = "0 0 0";
   %numAttempts = 5;
   %success     = -1;
   for (%i = 0; %i < %numAttempts; %i++) {
      %pos = VectorAdd(%oldPos, VectorScale(%vec[%i], 3));
      if (%obj.checkDismountPoint(%oldPos, %pos)) {
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
   %obj.setTransform(%pos);
   %obj.applyImpulse(%pos, VectorScale(%impulseVec, %obj.getDataBlock().mass));
   %obj.setPilot(false);
   %obj.vehicleTurret = "";
}


//----------------------------------------------------------------------------

function DefaultPlayerData::onCollision(%this,%obj,%col)
{
   if (%obj.getState() $= "Dead")
      return;
      
   //Ubiq:
   if(%obj.dieOnNextCollision)
   {
      //kill them immediately
      %fullDamage = %this.maxDamage;
      %this.Damage(%obj, 0, %obj.getPosition(), %fullDamage, "Impact");
      return;
   }

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
         %node = 0;
         %col.mountObject(%obj, %node);
         %obj.mVehicle = %col;
      }
   }
}

function DefaultPlayerData::onImpact(%this, %obj, %collidedObject, %vec, %vecLen)
{
   //Ubiq:
   if(%obj.dieOnNextCollision)
   {
      //kill them immediately
      %fullDamage = %this.maxDamage;
      %this.Damage(%obj, 0, %obj.getPosition(), %fullDamage, "Impact");
   }
   else
   {
      %this.Damage(%obj, 0, VectorAdd(%obj.getPosition(), %vec), %vecLen * %this.speedDamageScale, "Impact");
   }
}


//----------------------------------------------------------------------------

function DefaultPlayerData::Damage(%this, %obj, %sourceObject, %position, %damage, %damageType)
{
   if (%obj.getState() $= "Dead")
      return;
   %obj.applyDamage(%damage);
   %location = "Body";

   // Deal with client callbacks here because we don't have this
   // information in the onDamage or onDisable methods
   %client = %obj.client;
   %sourceClient = %sourceObject ? %sourceObject.client : 0;

   if (%obj.getState() $= "Dead" && isObject(%client))
      %client.onDeath(%sourceObject, %sourceClient, %damageType, %location);
      
   //%obj.updateHealth();
}

function DefaultPlayerData::onDamage(%this, %obj, %delta)
{
   // This method is invoked by the ShapeBase code whenever the 
   // object's damage level changes.
   if (%delta > 0 && %obj.getState() !$= "Dead") {

      // Increment the flash based on the amount.
      %flash = %obj.getDamageFlash() + ((%delta / %this.maxDamage) * 2);
      if (%flash > 0.75)
         %flash = 0.75;
      %obj.setDamageFlash(%flash);

      // If the pain is excessive, let's hear about it.
      if (%delta > 10)
         %obj.playPain();
   }
}

function DefaultPlayerData::onDisabled(%this,%obj,%state)
{
   // The player object sets the "disabled" state when damage exceeds
   // it's maxDamage value.  This is method is invoked by ShapeBase
   // state mangement code.

   // If we want to deal with the damage information that actually
   // caused this death, then we would have to move this code into
   // the script "damage" method.
   %obj.playDeathCry();
   %obj.setDamageFlash(0.75);

   // Release the main weapon trigger
   %obj.setImageTrigger(0,false);
}

//-----------------------------------------------------------------------------

function DefaultPlayerData::onLeaveMissionArea(%this, %obj)
{
   // Inform the client
   %obj.client.onLeaveMissionArea();
}

function DefaultPlayerData::onEnterMissionArea(%this, %obj)
{
   // Inform the client
   %obj.client.onEnterMissionArea();
}

//-----------------------------------------------------------------------------

function DefaultPlayerData::onEnterLiquid(%this, %obj, %coverage, %type)
{
   switch(%type)
   {
      case 0: //Water
      case 1: //Ocean Water
      case 2: //River Water
      case 3: //Stagnant Water
      case 4: //Lava
         %obj.setDamageDt(%this, $DamageLava, "Lava");
      case 5: //Hot Lava
         %obj.setDamageDt(%this, $DamageHotLava, "Lava");
      case 6: //Crusty Lava
         %obj.setDamageDt(%this, $DamageCrustyLava, "Lava");
      case 7: //Quick Sand
   }
}

function DefaultPlayerData::onLeaveLiquid(%this, %obj, %type)
{
   %obj.clearDamageDt();
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

function Player::mountVehicles(%this,%bool)
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


function Player::playDeathCry( %this )
{
   %this.playAudio(0,DeathCrySound);
}

function Player::playPain( %this )
{
   %this.playAudio(0,PainCrySound);
}

// ----------------------------------------------------------------------------
// Numerical Health Counter
// ----------------------------------------------------------------------------
function Player::setDamageLevel( %this, %damageLevel )
{
   Parent::setDamageLevel(%this,%damageLevel);
   %this.updateHealth();
}

function Player::applyDamage( %this, %damage )
{
   Parent::applyDamage(%this,%damage);
   %this.updateHealth();
}


function Player::updateHealth( %this )
{
   //echo("\c4Player::updateHealth() -> Player Health changed, updating HUD!");

   // Calcualte player health
   %maxDamage = %this.getDatablock().maxDamage;
   %damageLevel = %this.getDamageLevel();
   %curHealth = %maxDamage - %damageLevel;
   %curHealth = mceil(%curHealth);

   // Send the player object's current health level to the client, where it
   // will Update the numericalHealth HUD.
   commandToClient(%this.client, 'setNumericalHealthHUD', %curHealth);
}
