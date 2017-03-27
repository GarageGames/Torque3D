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
// This file contains Weapon and Ammo Class/"namespace" helper methods as well
// as hooks into the inventory system. These functions are not attached to a
// specific C++ class or datablock, but define a set of methods which are part
// of dynamic namespaces "class". The Items include these namespaces into their
// scope using the  ItemData and ItemImageData "className" variable.
// ----------------------------------------------------------------------------

// All ShapeBase images are mounted into one of 8 slots on a shape.  This weapon
// system assumes all primary weapons are mounted into this specified slot:
$WeaponSlot = 0;

//-----------------------------------------------------------------------------
// Weapon Class
//-----------------------------------------------------------------------------

function Weapon::onUse(%data, %obj)
{
   // Default behavior for all weapons is to mount it into the object's weapon
   // slot, which is currently assumed to be slot 0
   if (%obj.getMountedImage($WeaponSlot) != %data.image.getId())
   {
      serverPlay3D(WeaponUseSound, %obj.getTransform());

      %obj.mountImage(%data.image, $WeaponSlot);
      if (%obj.client)
      {
         if (%data.description !$= "")
            messageClient(%obj.client, 'MsgWeaponUsed', '\c0%1 selected.', %data.description);
         else
            messageClient(%obj.client, 'MsgWeaponUsed', '\c0Weapon selected');
      }
      
      // If this is a Player class object then allow the weapon to modify allowed poses
      if (%obj.isInNamespaceHierarchy("Player"))
      {
         // Start by allowing everything
         %obj.allowAllPoses();
         
         // Now see what isn't allowed by the weapon
         
         %image = %data.image;
         
         if (%image.jumpingDisallowed)
            %obj.allowJumping(false);
         
         if (%image.jetJumpingDisallowed)
            %obj.allowJetJumping(false);
         
         if (%image.sprintDisallowed)
            %obj.allowSprinting(false);
         
         if (%image.crouchDisallowed)
            %obj.allowCrouching(false);
         
         if (%image.proneDisallowed)
            %obj.allowProne(false);
         
         if (%image.swimmingDisallowed)
            %obj.allowSwimming(false);
      }
   }
}

function Weapon::onPickup(%this, %obj, %shape, %amount)
{
   // The parent Item method performs the actual pickup.
   // For player's we automatically use the weapon if the
   // player does not already have one in hand.
   if (Parent::onPickup(%this, %obj, %shape, %amount))
   {
      serverPlay3D(WeaponPickupSound, %shape.getTransform());
      if (%shape.getClassName() $= "Player" && %shape.getMountedImage($WeaponSlot) == 0)
         %shape.use(%this);
   }
}

function Weapon::onInventory(%this, %obj, %amount)
{
   // Weapon inventory has changed, make sure there are no weapons
   // of this type mounted if there are none left in inventory.
   if (!%amount && (%slot = %obj.getMountSlot(%this.image)) != -1)
      %obj.unmountImage(%slot);
}

//-----------------------------------------------------------------------------
// Weapon Image Class
//-----------------------------------------------------------------------------

function WeaponImage::onMount(%this, %obj, %slot)
{
   // Images assume a false ammo state on load.  We need to
   // set the state according to the current inventory.
   if(%this.isField("clip"))
   {
      // Use the clip system for this weapon.  Check if the player already has
      // some ammo in a clip.
      if (%obj.getInventory(%this.ammo))
      {
         %obj.setImageAmmo(%slot, true);
         %currentAmmo = %obj.getInventory(%this.ammo);
      }
      else if(%obj.getInventory(%this.clip) > 0)
      {
         // Fill the weapon up from the first clip
         %obj.setInventory(%this.ammo, %this.ammo.maxInventory);
         %obj.setImageAmmo(%slot, true);
         
         // Add any spare ammo that may be "in the player's pocket"
         %currentAmmo = %this.ammo.maxInventory;
         %amountInClips += %obj.getFieldValue( "remaining" @ %this.ammo.getName());
      }
      else
      {
         %currentAmmo = 0 + %obj.getFieldValue( "remaining" @ %this.ammo.getName());
      }
      
      %amountInClips = %obj.getInventory(%this.clip);
      %amountInClips *= %this.ammo.maxInventory;
      
      if (%obj.client !$= "" && !%obj.isAiControlled)
         %obj.client.RefreshWeaponHud(%currentAmmo, %this.item.previewImage, %this.item.reticle, %this.item.zoomReticle, %amountInClips);
   }
   else if(%this.ammo !$= "")
   {
      // Use the ammo pool system for this weapon
      if (%obj.getInventory(%this.ammo))
      {
         %obj.setImageAmmo(%slot, true);
         %currentAmmo = %obj.getInventory(%this.ammo);
      }
      else
         %currentAmmo = 0;

      if (%obj.client !$= "" && !%obj.isAiControlled)
         %obj.client.RefreshWeaponHud( 1, %this.item.previewImage, %this.item.reticle, %this.item.zoomReticle, %currentAmmo );
   }
}

function WeaponImage::onUnmount(%this, %obj, %slot)
{
   if (%obj.client !$= "" && !%obj.isAiControlled)
      %obj.client.RefreshWeaponHud(0, "", "");
}

// ----------------------------------------------------------------------------
// A "generic" weaponimage onFire handler for most weapons.  Can be overridden
// with an appropriate namespace method for any weapon that requires a custom
// firing solution.

// projectileSpread is a dynamic property declared in the weaponImage datablock
// for those weapons in which bullet skew is desired.  Must be greater than 0,
// otherwise the projectile goes straight ahead as normal.  lower values give
// greater accuracy, higher values increase the spread pattern.
// ----------------------------------------------------------------------------

function WeaponImage::onFire(%this, %obj, %slot)
{
   //echo("\c4WeaponImage::onFire( "@%this.getName()@", "@%obj.client.nameBase@", "@%slot@" )");

   // Make sure we have valid data
   if (!isObject(%this.projectile))
   {
      error("WeaponImage::onFire() - Invalid projectile datablock");
      return;
   }
   
   // Decrement inventory ammo. The image's ammo state is updated
   // automatically by the ammo inventory hooks.
   if ( !%this.infiniteAmmo )
      %obj.decInventory(%this.ammo, 1);

   // Get the player's velocity, we'll then add it to that of the projectile
   %objectVelocity = %obj.getVelocity();
   
   %numProjectiles = %this.projectileNum;
   if (%numProjectiles == 0)
      %numProjectiles = 1;
      
   for (%i = 0; %i < %numProjectiles; %i++)
   {
      if (%this.projectileSpread)
      {
         // We'll need to "skew" this projectile a little bit.  We start by
         // getting the straight ahead aiming point of the gun
         %vec = %obj.getMuzzleVector(%slot);

         // Then we'll create a spread matrix by randomly generating x, y, and z
         // points in a circle
         %matrix = "";
         for(%j = 0; %j < 3; %j++)
            %matrix = %matrix @ (getRandom() - 0.5) * 2 * 3.1415926 * %this.projectileSpread @ " ";
         %mat = MatrixCreateFromEuler(%matrix);

         // Which we'll use to alter the projectile's initial vector with
         %muzzleVector = MatrixMulVector(%mat, %vec);
      }
      else
      {
         // Weapon projectile doesn't have a spread factor so we fire it using
         // the straight ahead aiming point of the gun
         %muzzleVector = %obj.getMuzzleVector(%slot);
      }

      // Add player's velocity
      %muzzleVelocity = VectorAdd(
         VectorScale(%muzzleVector, %this.projectile.muzzleVelocity),
         VectorScale(%objectVelocity, %this.projectile.velInheritFactor));

      // Create the projectile object
      %p = new (%this.projectileType)()
      {
         dataBlock = %this.projectile;
         initialVelocity = %muzzleVelocity;
         initialPosition = %obj.getMuzzlePoint(%slot);
         sourceObject = %obj;
         sourceSlot = %slot;
         client = %obj.client;
         sourceClass = %obj.getClassName();
      };
      MissionCleanup.add(%p);
   }
}

// ----------------------------------------------------------------------------
// A "generic" weaponimage onAltFire handler for most weapons.  Can be
// overridden with an appropriate namespace method for any weapon that requires
// a custom firing solution.
// ----------------------------------------------------------------------------

function WeaponImage::onAltFire(%this, %obj, %slot)
{
   //echo("\c4WeaponImage::onAltFire("@%this.getName()@", "@%obj.client.nameBase@", "@%slot@")");

   // Decrement inventory ammo. The image's ammo state is updated
   // automatically by the ammo inventory hooks.
   %obj.decInventory(%this.ammo, 1);

   // Get the player's velocity, we'll then add it to that of the projectile
   %objectVelocity = %obj.getVelocity();
   
   %numProjectiles = %this.altProjectileNum;
   if (%numProjectiles == 0)
      %numProjectiles = 1;
      
   for (%i = 0; %i < %numProjectiles; %i++)
   {
      if (%this.altProjectileSpread)
      {
         // We'll need to "skew" this projectile a little bit.  We start by
         // getting the straight ahead aiming point of the gun
         %vec = %obj.getMuzzleVector(%slot);

         // Then we'll create a spread matrix by randomly generating x, y, and z
         // points in a circle
         %matrix = "";
         for(%i = 0; %i < 3; %i++)
            %matrix = %matrix @ (getRandom() - 0.5) * 2 * 3.1415926 * %this.altProjectileSpread @ " ";
         %mat = MatrixCreateFromEuler(%matrix);

         // Which we'll use to alter the projectile's initial vector with
         %muzzleVector = MatrixMulVector(%mat, %vec);
      }
      else
      {
         // Weapon projectile doesn't have a spread factor so we fire it using
         // the straight ahead aiming point of the gun.
         %muzzleVector = %obj.getMuzzleVector(%slot);
      }

      // Add player's velocity
      %muzzleVelocity = VectorAdd(
         VectorScale(%muzzleVector, %this.altProjectile.muzzleVelocity),
         VectorScale(%objectVelocity, %this.altProjectile.velInheritFactor));

      // Create the projectile object
      %p = new (%this.projectileType)()
      {
         dataBlock = %this.altProjectile;
         initialVelocity = %muzzleVelocity;
         initialPosition = %obj.getMuzzlePoint(%slot);
         sourceObject = %obj;
         sourceSlot = %slot;
         client = %obj.client;
      };
      MissionCleanup.add(%p);
   }
}

// ----------------------------------------------------------------------------
// A "generic" weaponimage onWetFire handler for most weapons.  Can be
// overridden with an appropriate namespace method for any weapon that requires
// a custom firing solution.
// ----------------------------------------------------------------------------

function WeaponImage::onWetFire(%this, %obj, %slot)
{
   //echo("\c4WeaponImage::onWetFire("@%this.getName()@", "@%obj.client.nameBase@", "@%slot@")");

   // Decrement inventory ammo. The image's ammo state is updated
   // automatically by the ammo inventory hooks.
   %obj.decInventory(%this.ammo, 1);

   // Get the player's velocity, we'll then add it to that of the projectile
   %objectVelocity = %obj.getVelocity();
   
   %numProjectiles = %this.projectileNum;
   if (%numProjectiles == 0)
      %numProjectiles = 1;
      
   for (%i = 0; %i < %numProjectiles; %i++)
   {
      if (%this.wetProjectileSpread)
      {
         // We'll need to "skew" this projectile a little bit.  We start by
         // getting the straight ahead aiming point of the gun
         %vec = %obj.getMuzzleVector(%slot);

         // Then we'll create a spread matrix by randomly generating x, y, and z
         // points in a circle
         %matrix = "";
         for(%j = 0; %j < 3; %j++)
         %matrix = %matrix @ (getRandom() - 0.5) * 2 * 3.1415926 * %this.wetProjectileSpread @ " ";
         %mat = MatrixCreateFromEuler(%matrix);

         // Which we'll use to alter the projectile's initial vector with
         %muzzleVector = MatrixMulVector(%mat, %vec);
      }
      else
      {
         // Weapon projectile doesn't have a spread factor so we fire it using
         // the straight ahead aiming point of the gun.
         %muzzleVector = %obj.getMuzzleVector(%slot);
      }
      
      // Add player's velocity
      %muzzleVelocity = VectorAdd(
         VectorScale(%muzzleVector, %this.wetProjectile.muzzleVelocity),
         VectorScale(%objectVelocity, %this.wetProjectile.velInheritFactor));

      // Create the projectile object
      %p = new (%this.projectileType)()
      {
         dataBlock = %this.wetProjectile;
         initialVelocity = %muzzleVelocity;
         initialPosition = %obj.getMuzzlePoint(%slot);
         sourceObject = %obj;
         sourceSlot = %slot;
         client = %obj.client;
      };
      MissionCleanup.add(%p);
   }
}

//-----------------------------------------------------------------------------
// Clip Management
//-----------------------------------------------------------------------------

function WeaponImage::onClipEmpty(%this, %obj, %slot)
{
   //echo("WeaponImage::onClipEmpty: " SPC %this SPC %obj SPC %slot);

   // Attempt to automatically reload.  Schedule this so it occurs
   // outside of the current state that called this method
   %this.schedule(0, "reloadAmmoClip", %obj, %slot);
}

function WeaponImage::reloadAmmoClip(%this, %obj, %slot)
{
   //echo("WeaponImage::reloadAmmoClip: " SPC %this SPC %obj SPC %slot);

   // Make sure we're indeed the currect image on the given slot
   if (%this != %obj.getMountedImage(%slot))
      return;

   if ( %this.isField("clip") )
   {
      if (%obj.getInventory(%this.clip) > 0)
      {
         %obj.decInventory(%this.clip, 1);
         %obj.setInventory(%this.ammo, %this.ammo.maxInventory);
         %obj.setImageAmmo(%slot, true);
      }
      else
      {
         %amountInPocket = %obj.getFieldValue( "remaining" @ %this.ammo.getName());
         if ( %amountInPocket )
         {
            %obj.setFieldValue( "remaining" @ %this.ammo.getName(), 0);
            %obj.setInventory( %this.ammo, %amountInPocket );
            %obj.setImageAmmo( %slot, true );
         }
      }
      
   }
}

function WeaponImage::clearAmmoClip( %this, %obj, %slot )
{
   //echo("WeaponImage::clearAmmoClip: " SPC %this SPC %obj SPC %slot);
   
   // if we're not empty put the remaining bullets from the current clip
   // in to the player's "pocket".

   if ( %this.isField( "clip" ) )
   {
      // Commenting out this line will use a "hard clip" system, where
      // A player will lose any ammo currently in the gun when reloading.
      %pocketAmount = %this.stashSpareAmmo( %obj );
      
      if ( %obj.getInventory( %this.clip ) > 0 || %pocketAmount != 0 )
         %obj.setImageAmmo(%slot, false);
   }
}
function WeaponImage::stashSpareAmmo( %this, %player )
{
   // If the amount in our pocket plus what we are about to add from the clip
   // Is over a clip, add a clip to inventory and keep the remainder
   // on the player
   if (%player.getInventory( %this.ammo ) < %this.ammo.maxInventory )
   {
      %nameOfAmmoField = "remaining" @ %this.ammo.getName();
      
      %amountInPocket = %player.getFieldValue( %nameOfAmmoField );
      
      %amountInGun = %player.getInventory( %this.ammo );
      
      %combinedAmmo = %amountInGun + %amountInPocket;
      
      // Give the player another clip if the amount in our pocket + the 
      // Amount in our gun is over the size of a clip.
      if ( %combinedAmmo >= %this.ammo.maxInventory )
      {
         %player.setFieldValue( %nameOfAmmoField, %combinedAmmo - %this.ammo.maxInventory );
         %player.incInventory( %this.clip, 1 );
      }
      else if ( %player.getInventory(%this.clip) > 0 )// Only put it back in our pocket if we have clips.
         %player.setFieldValue( %nameOfAmmoField, %combinedAmmo );
         
      return %player.getFieldValue( %nameOfAmmoField );
      
   }
   
   return 0;

}

//-----------------------------------------------------------------------------
// Clip Class
//-----------------------------------------------------------------------------

function AmmoClip::onPickup(%this, %obj, %shape, %amount)
{
   // The parent Item method performs the actual pickup.
   if (Parent::onPickup(%this, %obj, %shape, %amount))
      serverPlay3D(AmmoPickupSound, %shape.getTransform());

   // The clip inventory state has changed, we need to update the
   // current mounted image using this clip to reflect the new state.
   if ((%image = %shape.getMountedImage($WeaponSlot)) > 0)
   {
      // Check if this weapon uses the clip we just picked up and if
      // there is no ammo.
      if (%image.isField("clip") && %image.clip.getId() == %this.getId())
      {
         %outOfAmmo = !%shape.getImageAmmo($WeaponSlot);
         
         %currentAmmo = %shape.getInventory(%image.ammo);

         if ( isObject( %image.clip ) )
            %amountInClips = %shape.getInventory(%image.clip);
            
         %amountInClips *= %image.ammo.maxInventory;
         %amountInClips += %obj.getFieldValue( "remaining" @ %this.ammo.getName() );
         
         %shape.client.setAmmoAmountHud(%currentAmmo, %amountInClips );
         
         if (%outOfAmmo)
         {
            %image.onClipEmpty(%shape, $WeaponSlot);
         }
      }
   }
}

//-----------------------------------------------------------------------------
// Ammmo Class
//-----------------------------------------------------------------------------

function Ammo::onPickup(%this, %obj, %shape, %amount)
{
   // The parent Item method performs the actual pickup.
   if (Parent::onPickup(%this, %obj, %shape, %amount))
      serverPlay3D(AmmoPickupSound, %shape.getTransform());
}

function Ammo::onInventory(%this, %obj, %amount)
{
   // The ammo inventory state has changed, we need to update any
   // mounted images using this ammo to reflect the new state.
   for (%i = 0; %i < 8; %i++)
   {
      if ((%image = %obj.getMountedImage(%i)) > 0)
         if (isObject(%image.ammo) && %image.ammo.getId() == %this.getId())
         {
            %obj.setImageAmmo(%i, %amount != 0);
            %currentAmmo = %obj.getInventory(%this);
            
            if (%obj.getClassname() $= "Player")
            {
               if ( isObject( %this.clip ) )
               {
                  %amountInClips = %obj.getInventory(%this.clip);
                  %amountInClips *= %this.maxInventory;
                  %amountInClips += %obj.getFieldValue( "remaining" @ %this.getName() );
               }
               else //Is a single fire weapon, like the grenade launcher.
               {
                  %amountInClips = %currentAmmo;
                  %currentAmmo = 1;
               }
               
               if (%obj.client !$= "" && !%obj.isAiControlled)
                  %obj.client.setAmmoAmountHud(%currentAmmo, %amountInClips);
            }
         }
   }
}

// ----------------------------------------------------------------------------
// Weapon cycling
// ----------------------------------------------------------------------------

function ShapeBase::clearWeaponCycle(%this)
{
   %this.totalCycledWeapons = 0;
}

function ShapeBase::addToWeaponCycle(%this, %weapon)
{
   %this.cycleWeapon[%this.totalCycledWeapons++ - 1] = %weapon;
}

function ShapeBase::cycleWeapon(%this, %direction)
{
   // Can't cycle what we don't have
   if (%this.totalCycledWeapons == 0)
      return;
      
   // Find out the index of the current weapon, if any (not all
   // available weapons may be part of the cycle)
   %currentIndex = -1;
   if (%this.getMountedImage($WeaponSlot) != 0)
   {
      %curWeapon = %this.getMountedImage($WeaponSlot).item.getName();
      for (%i=0; %i<%this.totalCycledWeapons; %i++)
      {
         if (%this.cycleWeapon[%i] $= %curWeapon)
         {
            %currentIndex = %i;
            break;
         }
      }
   }

   // Get the next weapon index
   %nextIndex = 0;
   %dir = 1;
   if (%currentIndex != -1)
   {
      if (%direction $= "prev")
      {
         %dir = -1;
         %nextIndex = %currentIndex - 1;
         if (%nextIndex < 0)
         {
            // Wrap around to the end
            %nextIndex = %this.totalCycledWeapons - 1;
         }
      }
      else
      {
         %nextIndex = %currentIndex + 1;
         if (%nextIndex >= %this.totalCycledWeapons)
         {
            // Wrap back to the beginning
            %nextIndex = 0;
         }
      }
   }
   
   // We now need to check if the next index is a valid weapon.  If not,
   // then continue to cycle to the next weapon, in the appropriate direction,
   // until one is found.  If nothing is found, then do nothing.
   %found = false;
   for (%i=0; %i<%this.totalCycledWeapons; %i++)
   {
      %weapon = %this.cycleWeapon[%nextIndex];
      if (%weapon !$= "" && %this.hasInventory(%weapon) && %this.hasAmmo(%weapon))
      {
         // We've found out weapon
         %found = true;
         break;
      }
      
      %nextIndex = %nextIndex + %dir;
      if (%nextIndex < 0)
      {
         %nextIndex = %this.totalCycledWeapons - 1;
      }
      else if (%nextIndex >= %this.totalCycledWeapons)
      {
         %nextIndex = 0;
      }
   }
   
   if (%found)
   {
      %this.use(%this.cycleWeapon[%nextIndex]);
   }
}
