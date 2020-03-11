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

function ProximityMineData::onThrow( %this, %user, %amount )
{
   // Remove the object from the inventory
   %user.decInventory( %this, 1 );

   // Construct the actual object in the world, and add it to
   // the mission group so its cleaned up when the mission is
   // done.  The object is given a random z rotation.
   %obj = new ProximityMine()
   {
      datablock = %this;
      sourceObject = %user;
      rotation = "0 0 1 "@ (getRandom() * 360);
      static = false;
      client = %user.client;
   };
   MissionCleanup.add(%obj);
   return %obj;
}


function ProximityMineData::onTriggered( %this, %obj, %target )
{
   //echo(%this.name SPC "triggered by " @ %target.getClassName());
}

function ProximityMineData::onExplode( %this, %obj, %position )
{
   // Damage objects within the mine's damage radius
   if ( %this.damageRadius > 0 )
      radiusDamage( %obj, %position, %this.damageRadius, %this.radiusDamage, %this.damageType, %this.areaImpulse );
}

function ProximityMineData::damage( %this, %obj, %position, %source, %amount, %damageType )
{
   // Explode if any damage is applied to the mine
   %obj.schedule(50 + getRandom(50), explode);
}

// Customized kill message for deaths caused by proximity mines
function sendMsgClientKilled_MineDamage( %msgType, %client, %sourceClient, %damLoc )
{
   if ( %sourceClient $= "" )             // editor placed mine
      messageAll( %msgType, '%1 was blown up!', %client.playerName );
   else if ( %sourceClient == %client )   // own mine
      messageAll( %msgType, '%1 stepped on his own mine!', %client.playerName );
   else                                   // enemy placed mine
      messageAll( %msgType, '%1 was blown up by %2!', %client.playerName, %sourceClient.playerName );
}

// ----------------------------------------------------------------------------
// Player deployable proximity mine
// ----------------------------------------------------------------------------

// Cannot use the Weapon class for ProximityMineData datablocks as it is already tied
// to ItemData.

function ProxMine::onUse(%this, %obj)
{
   // Act like a weapon on use
   Weapon::onUse( %this, %obj );
}

function ProxMine::onPickup( %this, %obj, %shape, %amount )
{
   // Act like a weapon on pickup
   Weapon::onPickup( %this, %obj, %shape, %amount );
}

function ProxMine::onInventory( %this, %obj, %amount )
{
   %obj.client.setAmmoAmountHud( 1, %amount );

   // Cycle weapons if we are out of ammo
   if ( !%amount && ( %slot = %obj.getMountSlot( %this.image ) ) != -1 )
      %obj.cycleWeapon( "prev" );
}

function ProxMineImage::onMount( %this, %obj, %slot )
{
   // The mine doesn't use ammo from a player's perspective.
   %obj.setImageAmmo( %slot, true );
   %numMines = %obj.getInventory(%this.item);
   %obj.client.RefreshWeaponHud( 1, %this.item.previewImage, %this.item.reticle, %this.item.zoomReticle, %numMines  );
}

function ProxMineImage::onUnmount( %this, %obj, %slot )
{
   %obj.client.RefreshWeaponHud( 0, "", "" );
}

function ProxMineImage::onFire( %this, %obj, %slot )
{
   // To fire a deployable mine is to throw it
   %obj.throw( %this.item );
}
