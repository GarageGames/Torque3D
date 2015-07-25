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
// Server Admin Commands
//-----------------------------------------------------------------------------

function SAD(%password)
{
   if (%password !$= "")
      commandToServer('SAD', %password);
}

function SADSetPassword(%password)
{
   commandToServer('SADSetPassword', %password);
}

//----------------------------------------------------------------------------
// Misc server commands
//----------------------------------------------------------------------------

function clientCmdSyncClock(%time)
{
   // Time update from the server, this is only sent at the start of a mission
   // or when a client joins a game in progress.
}

//-----------------------------------------------------------------------------
// Damage Direction Indicator
//-----------------------------------------------------------------------------

function clientCmdSetDamageDirection(%direction)
{
   eval("%ctrl = DamageHUD-->damage_" @ %direction @ ";");
   if (isObject(%ctrl))
   {
      // Show the indicator, and schedule an event to hide it again
      cancelAll(%ctrl);
      %ctrl.setVisible(true);
      %ctrl.schedule(500, setVisible, false);
   }
}

//-----------------------------------------------------------------------------
// Teleporter visual effect
//-----------------------------------------------------------------------------

function clientCmdPlayTeleportEffect(%position, %effectDataBlock)
{
   if (isObject(%effectDataBlock))
   {
      new Explosion()
      {
         position = %position;
         dataBlock = %effectDataBlock;
      };
   }
}

// ----------------------------------------------------------------------------
// WeaponHUD
// ----------------------------------------------------------------------------

// Update the Ammo Counter with current ammo, if not any then hide the counter.
function clientCmdSetAmmoAmountHud(%amount, %amountInClips)
{
   if (!%amount)
      AmmoAmount.setVisible(false);
   else
   {
      AmmoAmount.setVisible(true);
      AmmoAmount.setText("Ammo: " @ %amount @ "/" @ %amountInClips);
   }
}

// Here we update the Weapon Preview image & reticle for each weapon.  We also
// update the Ammo Counter (just so we don't have to call it separately).
// Passing an empty parameter ("") hides the HUD component.

function clientCmdRefreshWeaponHUD(%amount, %preview, %ret, %zoomRet, %amountInClips)
{
   if (!%amount)
      AmmoAmount.setVisible(false);
   else
   {
      AmmoAmount.setVisible(true);
      AmmoAmount.setText("Ammo: " @ %amount @ "/" @ %amountInClips);
   }

   if (%preview $= "")
      WeaponHUD.setVisible(false);//PreviewImage.setVisible(false);
   else
   {
      WeaponHUD.setVisible(true);//PreviewImage.setVisible(true);
      PreviewImage.setbitmap("art/gui/weaponHud/"@ detag(%preview));
   }

   if (%ret $= "")
      Reticle.setVisible(false);
   else
   {
      Reticle.setVisible(true);
      Reticle.setbitmap("art/gui/weaponHud/"@ detag(%ret));
   }

   if (isObject(ZoomReticle))
   {
      if (%zoomRet $= "")
      {
         ZoomReticle.setBitmap("");
      }
      else
      {
         ZoomReticle.setBitmap("art/gui/weaponHud/"@ detag(%zoomRet));
      }
   }
}

// ----------------------------------------------------------------------------
// Vehicle Support
// ----------------------------------------------------------------------------

function clientCmdtoggleVehicleMap(%toggle)
{
   if(%toggle)
   {
      moveMap.pop();
	  // clear movement
	  $mvForwardAction = 0;
	  $mvBackwardAction = 0;
      vehicleMap.push();
   }
   else
   {
      vehicleMap.pop();
      moveMap.push();
   }
}

// ----------------------------------------------------------------------------
// Turret Support
// ----------------------------------------------------------------------------

// Call by the Turret class when a player mounts or unmounts it.
// %turret = The turret that was mounted
// %player = The player doing the mounting
// %mounted = True if the turret was mounted, false if it was unmounted
function turretMountCallback(%turret, %player, %mounted)
{
   //echo ( "\c4turretMountCallback -> " @ %mounted );

   if (%mounted)
   {
      // Push the action map
      turretMap.push();
   }
   else
   {
      // Pop the action map
      turretMap.pop();
   }
}
