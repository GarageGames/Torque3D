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

//--------------------------------------------------------------------------
// Sounds
//--------------------------------------------------------------------------
// Added for Lesson 5 - Adding Weapons - 23 Sep 11
datablock SFXProfile(WeaponTemplateFireSound)
{
   filename = "art/sound/weapons/wpn_fire";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(WeaponTemplateReloadSound)
{
   filename = "art/sound/weapons/wpn_reload";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(WeaponTemplateSwitchinSound)
{
   filename = "art/sound/weapons/wpn_switchin";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(WeaponTemplateIdleSound)
{
   filename = "art/sound/weapons/wpn_idle";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(WeaponTemplateGrenadeSound)
{
   filename = "art/sound/weapons/wpn_grenadelaunch";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(WeaponTemplateMineSwitchinSound)
{
   filename = "art/sound/weapons/wpn_mine_switchin";
   description = AudioClose3D;
   preload = true;
};
//-----------------------------------------------------------------------------
// Added 28 Sep 11
datablock LightDescription( BulletProjectileLightDesc )
{
   color  = "0.0 0.5 0.7";
   range = 3.0;
};

datablock ProjectileData( BulletProjectile )
{
   projectileShapeName = "";

   directDamage        = 5;
   radiusDamage        = 0;
   damageRadius        = 0.5;
   areaImpulse         = 0.5;
   impactForce         = 1;

   explosion           = BulletDirtExplosion;
   decal               = BulletHoleDecal;

   muzzleVelocity      = 120;
   velInheritFactor    = 1;

   armingDelay         = 0;
   lifetime            = 992;
   fadeDelay           = 1472;
   bounceElasticity    = 0;
   bounceFriction      = 0;
   isBallistic         = false;
   gravityMod          = 1;
};

function BulletProjectile::onCollision(%this,%obj,%col,%fade,%pos,%normal)
{
   // Apply impact force from the projectile.
   
   // Apply damage to the object all shape base objects
   if ( %col.getType() & $TypeMasks::GameBaseObjectType )
      %col.damage(%obj,%pos,%this.directDamage,"BulletProjectile");
}
//-----------------------------------------------------------------------------

datablock ProjectileData( WeaponTemplateProjectile )
{
   projectileShapeName              = "";

   directDamage                     = 5;
   radiusDamage                     = 0;
   damageRadius                     = 0.5;
   areaImpulse                      = 0.5;
   impactForce                      = 1;

   muzzleVelocity                   = 120;
   velInheritFactor                 = 1;

   armingDelay                      = 0;
   lifetime                         = 992;
   fadeDelay                        = 1472;
   bounceElasticity                 = 0;
   bounceFriction                   = 0;
   isBallistic                      = false;
   gravityMod                       = 1;
};

function WeaponTemplateProjectile::onCollision(%this,%obj,%col,%fade,%pos,%normal)
{
   // Apply impact force from the projectile.
   
   // Apply damage to the object all shape base objects
   if ( %col.getType() & $TypeMasks::GameBaseObjectType )
      %col.damage(%obj,%pos,%this.directDamage,"WeaponTemplateProjectile");
}

//-----------------------------------------------------------------------------
// Ammo Item
//-----------------------------------------------------------------------------
datablock ItemData(WeaponTemplateAmmo)
{
   // Mission editor category
   category                         = "Ammo";

   // Add the Ammo namespace as a parent.  The ammo namespace provides
   // common ammo related functions and hooks into the inventory system.
   className                        = "Ammo";

   // Basic Item properties
   shapeFile                        = "";
   mass                             = 1;
   elasticity                       = 0.2;
   friction                         = 0.6;

   // Dynamic properties defined by the scripts
   pickUpName                       = "";
   maxInventory                     = 1000;
};

//--------------------------------------------------------------------------
// Weapon Item.  This is the item that exists in the world, i.e. when it's
// been dropped, thrown or is acting as re-spawnable item.  When the weapon
// is mounted onto a shape, the NewWeaponTemplate is used.
//-----------------------------------------------------------------------------
datablock ItemData(WeaponTemplateItem)
{
   // Mission editor category
   category                         = "Weapon";

   // Hook into Item Weapon class hierarchy. The weapon namespace
   // provides common weapon handling functions in addition to hooks
   // into the inventory system.
   className                        = "Weapon";

   // Basic Item properties
   shapeFile                        = "";
   mass                             = 1;
   elasticity                       = 0.2;
   friction                         = 0.6;
   emap                             = true;

   // Dynamic properties defined by the scripts
   pickUpName                       = "A basic weapon";
   description                      = "Weapon";
   image                            = WeaponTemplateImage;
   reticle                          = "crossHair";
};

datablock ShapeBaseImageData(WeaponTemplateImage)
{
   // FP refers to first person specific features
   
   // Defines what art file to use.
   shapeFile = "art/shapes/weapons/Lurker/TP_Lurker.DAE";
   shapeFileFP = "art/shapes/weapons/Lurker/FP_Lurker.DAE";
   
   // Whether or not to enable environment mapping
   //emap                           = true;

   //imageAnimPrefixFP              = "";

   // Specify mount point & offset for 3rd person, and eye offset
   // for first person rendering.
   mountPoint                       = 0;
   //firstPerson                    = true;
   //eyeOffset                      = "0.001 -0.05 -0.065";
   
   // When firing from a point offset from the eye, muzzle correction
   // will adjust the muzzle vector to point to the eye LOS point.
   // Since this weapon doesn't actually fire from the muzzle point,
   // we need to turn this off.
   //correctMuzzleVector            = true;

   // Add the WeaponImage namespace as a parent, WeaponImage namespace
   // provides some hooks into the inventory system.
   class                            = "WeaponImage";
   className                        = "WeaponImage";

   // Projectiles and Ammo.
   item                             = WeaponTemplateItem;
   ammo                             = WeaponTemplateAmmo;

   //projectile                     = BulletProjectile;
   //projectileType                 = Projectile;
   //projectileSpread               = "0.005";
   
   // Properties associated with the shell casing that gets ejected during firing
   //casing = BulletShell;
   //shellExitDir                   = "1.0 0.3 1.0";
   //shellExitOffset                = "0.15 -0.56 -0.1";
   //shellExitVariance              = 15.0;
   //shellVelocity                  = 3.0;

   // Properties associated with a light that occurs when the weapon fires
   //lightType                      = "";
   //lightColor                     = "0.992126 0.968504 0.708661 1";
   //lightRadius                    = "4";
   //lightDuration                  = "100";
   //lightBrightness                = 2;

   // Properties associated with shaking the camera during firing
   //shakeCamera                    = false;
   //camShakeFreq                   = "0 0 0";
   //camShakeAmp                    = "0 0 0";

   // Images have a state system which controls how the animations
   // are run, which sounds are played, script callbacks, etc. This
   // state system is downloaded to the client so that clients can
   // predict state changes and animate accordingly.  The following
   // system supports basic ready->fire->reload transitions as
   // well as a no-ammo->dryfire idle state.

   // If true, allow multiple timeout transitions to occur within a single tick
   // useful if states have a very small timeout
   //useRemainderDT = true;

   // Initial start up state
   stateName[0]                     = "Preactivate";
   stateTransitionOnLoaded[0]       = "Activate";
   stateTransitionOnNoAmmo[0]       = "NoAmmo";

   // Activating the gun.  Called when the weapon is first
   // mounted and there is ammo.
   stateName[1]                     = "Activate";
   stateTransitionGeneric0In[1]     = "SprintEnter";
   stateTransitionOnTimeout[1]      = "Ready";
   stateTimeoutValue[1]             = 0.5;
   stateSequence[1]                 = "switch_in";

   // Ready to fire, just waiting for the trigger
   stateName[2]                     = "Ready";
   stateTransitionGeneric0In[2]     = "SprintEnter";
   stateTransitionOnMotion[2]       = "ReadyMotion";
   stateTransitionOnTimeout[2]      = "ReadyFidget";
   stateTimeoutValue[2]             = 10;
   stateWaitForTimeout[2]           = false;
   stateScaleAnimation[2]           = false;
   stateScaleAnimationFP[2]         = false;
   stateTransitionOnNoAmmo[2]       = "NoAmmo";
   stateTransitionOnTriggerDown[2]  = "Fire";
   stateSequence[2]                 = "idle";

   // Same as Ready state but plays a fidget sequence
   stateName[3]                     = "ReadyFidget";
   stateTransitionGeneric0In[3]     = "SprintEnter";
   stateTransitionOnMotion[3]       = "ReadyMotion";
   stateTransitionOnTimeout[3]      = "Ready";
   stateTimeoutValue[3]             = 6;
   stateWaitForTimeout[3]           = false;
   stateTransitionOnNoAmmo[3]       = "NoAmmo";
   stateTransitionOnTriggerDown[3]  = "Fire";
   stateSequence[3]                 = "idle_fidget1";

   // Ready to fire with player moving
   stateName[4]                     = "ReadyMotion";
   stateTransitionGeneric0In[4]     = "SprintEnter";
   stateTransitionOnNoMotion[4]     = "Ready";
   stateWaitForTimeout[4]           = false;
   stateScaleAnimation[4]           = false;
   stateScaleAnimationFP[4]         = false;
   stateSequenceTransitionIn[4]     = true;
   stateSequenceTransitionOut[4]    = true;
   stateTransitionOnNoAmmo[4]       = "NoAmmo";
   stateTransitionOnTriggerDown[4]  = "Fire";
   stateSequence[4]                 = "run";

   // Fire the weapon. Calls the fire script which does
   // the actual work.
   stateName[5]                     = "Fire";
   stateTransitionGeneric0In[5]     = "SprintEnter";
   stateTransitionOnTimeout[5]      = "Reload";
   stateTimeoutValue[5]             = 0.15;
   stateFire[5]                     = true;
   stateRecoil[5]                   = "";
   stateAllowImageChange[5]         = false;
   stateSequence[5]                 = "fire";
   stateScaleAnimation[5]           = false;
   stateSequenceNeverTransition[5]  = true;
   stateSequenceRandomFlash[5]      = true;        // use muzzle flash sequence
   stateScript[5]                   = "onFire";
   stateSound[5]                    = WeaponTemplateFireSound;
   stateEmitter[5]                  = GunFireSmokeEmitter;
   stateEmitterTime[5]              = 0.025;

   // Play the reload animation, and transition into
   stateName[6]                     = "Reload";
   stateTransitionGeneric0In[6]     = "SprintEnter";
   stateTransitionOnNoAmmo[6]       = "NoAmmo";
   stateTransitionOnTimeout[6]      = "Ready";
   stateWaitForTimeout[6]           = "0";
   stateTimeoutValue[6]             = 0.05;
   stateAllowImageChange[6]         = false;
   //stateSequence[6]                 = "reload";
   stateEjectShell[6]               = true;

   // No ammo in the weapon, just idle until something
   // shows up. Play the dry fire sound if the trigger is
   // pulled.
   stateName[7]                     = "NoAmmo";
   stateTransitionGeneric0In[7]     = "SprintEnter";
   stateTransitionOnAmmo[7]         = "ReloadClip";
   stateScript[7]                   = "onClipEmpty";

   // No ammo dry fire
   stateName[8]                     = "DryFire";
   stateTransitionGeneric0In[8]     = "SprintEnter";
   stateTimeoutValue[8]             = 1.0;
   stateTransitionOnTimeout[8]      = "NoAmmo";
   stateScript[8]                   = "onDryFire";

   // Play the reload clip animation
   stateName[9]                     = "ReloadClip";
   stateTransitionGeneric0In[9]     = "SprintEnter";
   stateTransitionOnTimeout[9]      = "Ready";
   stateWaitForTimeout[9]           = true;
   stateTimeoutValue[9]             = 3.0;
   stateReload[9]                   = true;
   stateSequence[9]                 = "reload";
   stateShapeSequence[9]            = "Reload";
   stateScaleShapeSequence[9]       = true;

   // Start Sprinting
   stateName[10]                    = "SprintEnter";
   stateTransitionGeneric0Out[10]   = "SprintExit";
   stateTransitionOnTimeout[10]     = "Sprinting";
   stateWaitForTimeout[10]          = false;
   stateTimeoutValue[10]            = 0.5;
   stateWaitForTimeout[10]          = false;
   stateScaleAnimation[10]          = false;
   stateScaleAnimationFP[10]        = false;
   stateSequenceTransitionIn[10]    = true;
   stateSequenceTransitionOut[10]   = true;
   stateAllowImageChange[10]        = false;
   stateSequence[10]                = "sprint";

   // Sprinting
   stateName[11]                    = "Sprinting";
   stateTransitionGeneric0Out[11]   = "SprintExit";
   stateWaitForTimeout[11]          = false;
   stateScaleAnimation[11]          = false;
   stateScaleAnimationFP[11]        = false;
   stateSequenceTransitionIn[11]    = true;
   stateSequenceTransitionOut[11]   = true;
   stateAllowImageChange[11]        = false;
   stateSequence[11]                = "sprint";
   
   // Stop Sprinting
   stateName[12]                    = "SprintExit";
   stateTransitionGeneric0In[12]    = "SprintEnter";
   stateTransitionOnTimeout[12]     = "Ready";
   stateWaitForTimeout[12]          = false;
   stateTimeoutValue[12]            = 0.5;
   stateSequenceTransitionIn[12]    = true;
   stateSequenceTransitionOut[12]   = true;
   stateAllowImageChange[12]        = false;
   stateSequence[12]                = "sprint";
};