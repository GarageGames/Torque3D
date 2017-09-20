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
// Numerical Health Counter
//-----------------------------------------------------------------------------

function clientCmdSetNumericalHealthHUD(%curHealth)
{
   // Skip if the hud is missing.
   if (!isObject(numericalHealthHUD))
      return;

   // The server has sent us our current health, display it on the HUD
   numericalHealthHUD.setValue(%curHealth);

   // Ensure the HUD is set to visible while we have health / are alive
   if (%curHealth)
      HealthHUD.setVisible(true);
   else
      HealthHUD.setVisible(false);
}

// ----------------------------------------------------------------------------
// WeaponHUD
// ----------------------------------------------------------------------------

// Update the Ammo Counter with current ammo, if not any then hide the counter.

function clientCmdSetAmmoAmountHud(%amount)
{
   if (!%amount)
      AmmoAmount.setVisible(false);
   else
   {
      AmmoAmount.setVisible(true);
      AmmoAmount.setText("Ammo: "@%amount);
   }
}

// Here we update the Weapon Preview image & reticle for each weapon.  We also
// update the Ammo Counter (just so we don't have to call it separately).
// Passing an empty parameter ("") hides the HUD component.

function clientCmdRefreshWeaponHUD(%amount, %preview, %ret)
{
   if (!%amount)
      AmmoAmount.setVisible(false);
   else
   {
      AmmoAmount.setVisible(true);
      AmmoAmount.setText("Ammo: "@ %amount);
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
}

// ----------------------------------------------------------------------------
// Zoom reticle
// ----------------------------------------------------------------------------

// Here we turn the normal reticle off and show the zoom reticle.  The "reticle"
// parameter is the "zoomReticle" field in the weapon's datablock.


function clientCmdsetZoom(%reticle)
{
   $ZoomOn = true;
   setFov($Pref::Player::CurrentFOV);
   zoomReticle.setBitmap("art/gui/weaponHud/"@ detag(%reticle));
   Reticle.setVisible(false);
   zoomReticle.setVisible(true);

   DOFPostEffect.setAutoFocus( true );
   DOFPostEffect.setFocusParams( 0.5, 0.5, 50, 500, -5, 5 );
   DOFPostEffect.enable();
}

function clientCmdUnSetZoom(%reticle)
{
   $ZoomOn = false;
   setFov($Pref::Player::defaultFOV);
   Reticle.setVisible(true);
   zoomReticle.setVisible(false);

   DOFPostEffect.disable();
}
