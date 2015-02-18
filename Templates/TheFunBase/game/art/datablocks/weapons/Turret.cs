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

datablock SFXProfile(TargetAquiredSound)
{
   filename = "";
   description = AudioClose3D;
   preload = false;
};

datablock SFXProfile(TargetLostSound)
{
   filename = "";
   description = AudioClose3D;
   preload = false;
};

datablock SFXProfile(TurretDestroyed)
{
   filename = "";
   description = AudioClose3D;
   preload = false;
};

datablock SFXProfile(TurretThrown)
{
   filename = "";
   description = AudioClose3D;
   preload = false;
};

datablock SFXProfile(TurretFireSound)
{
   filename = "art/sound/turret/wpn_turret_fire";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(TurretActivatedSound)
{
   filename = "art/sound/turret/wpn_turret_deploy";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile(TurretScanningSound)
{
   filename = "art/sound/turret/wpn_turret_scan";
   description = AudioCloseLoop3D;
   preload = true;
};

datablock SFXProfile(TurretSwitchinSound)
{
   filename = "art/sound/turret/wpn_turret_switchin";
   description = AudioClose3D;
   preload = true;
};

//-----------------------------------------------------------------------------
// Turret Bullet Projectile
//-----------------------------------------------------------------------------

datablock ProjectileData( TurretBulletProjectile )
{
   projectileShapeName = "";

   directDamage        = 5;
   radiusDamage        = 0;
   damageRadius        = 0.5;
   areaImpulse         = 0.5;
   impactForce         = 1;
   
   damageType          = "TurretDamage";  // Type of damage applied by this weapon

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

function TurretBulletProjectile::onCollision(%this,%obj,%col,%fade,%pos,%normal)
{
   // Apply impact force from the projectile.
   
   // Apply damage to the object all shape base objects
   if ( %col.getType() & $TypeMasks::GameBaseObjectType )
      %col.damage(%obj,%pos,%this.directDamage,%this.damageType);
}

//-----------------------------------------------------------------------------
// Turret Bullet Ammo
//-----------------------------------------------------------------------------

datablock ItemData(AITurretAmmo)
{
   // Mission editor category
   category = "Ammo";

   // Add the Ammo namespace as a parent.  The ammo namespace provides
   // common ammo related functions and hooks into the inventory system.
   className = "Ammo";

   // Basic Item properties
   shapeFile = "art/shapes/weapons/Turret/Turret_Legs.DAE";
   mass = 1;
   elasticity = 0.2;
   friction = 0.6;

   // Dynamic properties defined by the scripts
   pickUpName = "turret ammo";
};

//-----------------------------------------------------------------------------
// AI Turret Weapon
//-----------------------------------------------------------------------------

datablock ItemData(AITurretHead)
{
   // Mission editor category
   category = "Weapon";

   // Hook into Item Weapon class hierarchy. The weapon namespace
   // provides common weapon handling functions in addition to hooks
   // into the inventory system.
   className = "Weapon";

   // Basic Item properties
   shapeFile = "art/shapes/weapons/Turret/Turret_Head.DAE";
   mass = 1;
   elasticity = 0.2;
   friction = 0.6;
   emap = true;

   // Dynamic properties defined by the scripts
   pickUpName = "an AI turret head";
   description = "AI Turret Head";
   image = AITurretHeadImage;
   reticle = "crossHair";
};
datablock ShapeBaseImageData(AITurretHeadImage)
{
   // Basic Item properties
   shapeFile = "art/shapes/weapons/Turret/Turret_Head.DAE";
   emap = true;

   // Specify mount point
   mountPoint = 0;

   // Add the WeaponImage namespace as a parent, WeaponImage namespace
   // provides some hooks into the inventory system.
   class = "WeaponImage";
   className = "WeaponImage";

   // Projectiles and Ammo.
   item = AITurretHead;
   ammo = AITurretAmmo;

   projectile = TurretBulletProjectile;
   projectileType = Projectile;
   projectileSpread = "0.02";

   casing = BulletShell;
   shellExitDir        = "1.0 0.3 1.0";
   shellExitOffset     = "0.15 -0.56 -0.1";
   shellExitVariance   = 15.0;
   shellVelocity       = 3.0;

   // Weapon lights up while firing
   lightType = "WeaponFireLight";
   lightColor = "0.992126 0.968504 0.708661 1";
   lightRadius = "4";
   lightDuration = "100";
   lightBrightness = 2;

   // Shake camera while firing.
   shakeCamera = false;
   camShakeFreq = "0 0 0";
   camShakeAmp = "0 0 0";

   // Images have a state system which controls how the animations
   // are run, which sounds are played, script callbacks, etc. This
   // state system is downloaded to the client so that clients can
   // predict state changes and animate accordingly.  The following
   // system supports basic ready->fire->reload transitions as
   // well as a no-ammo->dryfire idle state.

   useRemainderDT = true;

   // Initial start up state
   stateName[0]                     = "Preactivate";
   stateIgnoreLoadedForReady[0]     = false;
   stateTransitionOnLoaded[0]       = "Activate";
   stateTransitionOnNotLoaded[0]    = "WaitingDeployment";  // If the turret weapon is not loaded then it has not yet been deployed
   stateTransitionOnNoAmmo[0]       = "NoAmmo";

   // Activating the gun.  Called when the weapon is first
   // mounted and there is ammo.
   stateName[1]                     = "Activate";
   stateTransitionGeneric0In[1]     = "Destroyed";
   stateTransitionOnTimeout[1]      = "Ready";
   stateTimeoutValue[1]             = 0.5;
   stateSequence[1]                 = "Activate";

   // Ready to fire, just waiting for the trigger
   stateName[2]                     = "Ready";
   stateTransitionGeneric0In[2]     = "Destroyed";
   stateTransitionOnNoAmmo[2]       = "NoAmmo";
   stateTransitionOnTriggerDown[2]  = "Fire";
   stateSequence[2]                 = "scan";

   // Fire the weapon. Calls the fire script which does
   // the actual work.
   stateName[3]                     = "Fire";
   stateTransitionGeneric0In[3]     = "Destroyed";
   stateTransitionOnTimeout[3]      = "Reload";
   stateTimeoutValue[3]             = 0.2;
   stateFire[3]                     = true;
   stateRecoil[3]                   = "LightRecoil";
   stateAllowImageChange[3]         = false;
   stateSequence[3]                 = "fire";
   stateSequenceRandomFlash[3]      = true;        // use muzzle flash sequence
   stateScript[3]                   = "onFire";
   stateSound[3]                    = TurretFireSound;
   stateEmitter[3]                  = GunFireSmokeEmitter;
   stateEmitterTime[3]              = 0.025;
   stateEjectShell[3]               = true;

   // Play the reload animation, and transition into
   stateName[4]                     = "Reload";
   stateTransitionGeneric0In[4]     = "Destroyed";
   stateTransitionOnNoAmmo[4]       = "NoAmmo";
   stateTransitionOnTimeout[4]      = "Ready";
   stateWaitForTimeout[4]           = "0";
   stateTimeoutValue[4]             = 0.0;
   stateAllowImageChange[4]         = false;
   stateSequence[4]                 = "Reload";

   // No ammo in the weapon, just idle until something
   // shows up. Play the dry fire sound if the trigger is
   // pulled.
   stateName[5]                     = "NoAmmo";
   stateTransitionGeneric0In[5]     = "Destroyed";
   stateTransitionOnAmmo[5]         = "Reload";
   stateSequence[5]                 = "NoAmmo";
   stateTransitionOnTriggerDown[5]  = "DryFire";

   // No ammo dry fire
   stateName[6]                     = "DryFire";
   stateTransitionGeneric0In[6]     = "Destroyed";
   stateTimeoutValue[6]             = 1.0;
   stateTransitionOnTimeout[6]      = "NoAmmo";
   stateScript[6]                   = "onDryFire";

   // Waiting for the turret to be deployed
   stateName[7]                     = "WaitingDeployment";
   stateTransitionGeneric0In[7]     = "Destroyed";
   stateTransitionOnLoaded[7]       = "Deployed";
   stateSequence[7]                 = "wait_deploy";

   // Turret has been deployed
   stateName[8]                     = "Deployed";
   stateTransitionGeneric0In[8]     = "Destroyed";
   stateTransitionOnTimeout[8]      = "Ready";
   stateWaitForTimeout[8]           = true;
   stateTimeoutValue[8]             = 2.5;   // Same length as turret base's Deploy state
   stateSequence[8]                 = "deploy";

   // Turret has been destroyed
   stateName[9]                     = "Destroyed";
   stateSequence[9]                 = "destroyed";
};

//-----------------------------------------------------------------------------
// AI Turret
//-----------------------------------------------------------------------------

datablock AITurretShapeData(AITurret)
{
   category = "Turrets";
   shapeFile = "art/shapes/weapons/Turret/Turret_Legs.DAE";

   maxDamage = 70;
   destroyedLevel = 70;
   explosion = GrenadeExplosion;
   
   simpleServerCollision = false;

   zRotOnly = false;
   
   // Rotation settings
   minPitch = 15;
   maxPitch = 80;
   maxHeading = 90;
   headingRate = 50;
   pitchRate = 50;

   // Scan settings
   maxScanPitch = 10;
   maxScanHeading = 30;
   maxScanDistance = 20;
   trackLostTargetTime = 2;

   maxWeaponRange = 30;

   weaponLeadVelocity = 0;

   // Weapon mounting
   numWeaponMountPoints = 1;

   weapon[0] = AITurretHead;
   weaponAmmo[0] = AITurretAmmo;
   weaponAmmoAmount[0] = 10000;

   maxInv[AITurretHead] = 1;
   maxInv[AITurretAmmo] = 10000;

   // Initial start up state
   stateName[0]                     = "Preactivate";
   stateTransitionOnAtRest[0]       = "Scanning";
   stateTransitionOnNotAtRest[0]    = "Thrown";
   
   // Scan for targets
   stateName[1]                     = "Scanning";
   stateScan[1]                     = true;
   stateTransitionOnTarget[1]       = "Target";
   stateSequence[1]                 = "scan";
   stateScript[1]                   = "OnScanning";

   // Have a target
   stateName[2]                     = "Target";
   stateTransitionOnNoTarget[2]     = "NoTarget";
   stateTransitionOnTimeout[2]      = "Firing";
   stateTimeoutValue[2]             = 2.0;
   stateScript[2]                   = "OnTarget";

   // Fire at target
   stateName[3]                     = "Firing";
   stateFire[3]                     = true;
   stateTransitionOnNoTarget[3]     = "NoTarget";
   stateScript[3]                   = "OnFiring";

   // Lost target
   stateName[4]                     = "NoTarget";
   stateTransitionOnTimeout[4]      = "Scanning";
   stateTimeoutValue[4]             = 2.0;
   stateScript[4]                   = "OnNoTarget";

   // Player thrown turret
   stateName[5]                     = "Thrown";
   stateTransitionOnAtRest[5]       = "Deploy";
   stateSequence[5]                 = "throw";
   stateScript[5]                   = "OnThrown";

   // Player thrown turret is deploying
   stateName[6]                     = "Deploy";
   stateTransitionOnTimeout[6]      = "Scanning";
   stateTimeoutValue[6]             = 2.5;
   stateSequence[6]                 = "deploy";
   stateScaleAnimation[6]           = true;
   stateScript[6]                   = "OnDeploy";

   // Special state that is set when the turret is destroyed.
   // This state is set in the onDestroyed() callback.
   stateName[7]                     = "Destroyed";
   stateSequence[7]                 = "destroyed";
};

//-----------------------------------------------------------------------------
// Deployable AI Turret
//-----------------------------------------------------------------------------
datablock AITurretShapeData(DeployableTurret : AITurret)
{
   // Mission editor category
   category = "Weapon";

   className = "DeployableTurretWeapon";

   startLoaded = false;
   
   // Basic Item properties
   mass = 1.5;
   elasticity = 0.1;
   friction = 0.6;
   simpleServerCollision = false;

   // Dynamic properties defined by the scripts
   PreviewImage = 'turret.png';
   pickUpName = "a deployable turret";
   description = "Deployable Turret";
   image = DeployableTurretImage;
   reticle = "blank";
   zoomReticle = 'blank';
};

datablock ShapeBaseImageData(DeployableTurretImage)
{
   // Basic Item properties
   shapeFile = "art/shapes/weapons/Turret/TP_Turret.DAE";
   shapeFileFP = "art/shapes/weapons/Turret/FP_Turret.DAE";
   emap = true;

   imageAnimPrefix = "Turret";
   imageAnimPrefixFP = "Turret";

   // Specify mount point & offset for 3rd person, and eye offset
   // for first person rendering.
   mountPoint = 0;
   firstPerson = true;
   useEyeNode = true;

   // Don't allow a player to sprint with a turret
   sprintDisallowed = true;
   
   class = "DeployableTurretWeaponImage";
   className = "DeployableTurretWeaponImage";

   // Projectiles and Ammo.
   item = DeployableTurret;

   // Shake camera while firing.
   shakeCamera = false;
   camShakeFreq = "0 0 0";
   camShakeAmp = "0 0 0";

   // Images have a state system which controls how the animations
   // are run, which sounds are played, script callbacks, etc. This
   // state system is downloaded to the client so that clients can
   // predict state changes and animate accordingly.  The following
   // system supports basic ready->fire->reload transitions as
   // well as a no-ammo->dryfire idle state.

   // Initial start up state
   stateName[0]                     = "Preactivate";
   stateTransitionOnLoaded[0]       = "Activate";
   stateTransitionOnNoAmmo[0]       = "Activate";

   // Activating the gun.  Called when the weapon is first
   // mounted and there is ammo.
   stateName[1]                     = "Activate";
   stateTransitionGeneric0In[1]     = "SprintEnter";
   stateTransitionOnTimeout[1]      = "Ready";
   stateTimeoutValue[1]             = 0.66;
   stateSequence[1]                 = "switch_in";
   stateSound[1]                    = TurretSwitchinSound;

   // Ready to fire, just waiting for the trigger
   stateName[2]                     = "Ready";
   stateTransitionGeneric0In[2]     = "SprintEnter";
   stateTransitionOnMotion[2]       = "ReadyMotion";
   stateTransitionOnTriggerDown[2]  = "Fire";
   stateScaleAnimation[2]           = false;
   stateScaleAnimationFP[2]         = false;
   stateSequence[2]                 = "idle";

   // Ready to fire with player moving
   stateName[3]                     = "ReadyMotion";
   stateTransitionGeneric0In[3]     = "SprintEnter";
   stateTransitionOnNoMotion[3]     = "Ready";
   stateScaleAnimation[3]           = false;
   stateScaleAnimationFP[3]         = false;
   stateSequenceTransitionIn[3]     = true;
   stateSequenceTransitionOut[3]    = true;
   stateTransitionOnTriggerDown[3]  = "Fire";
   stateSequence[3]                 = "run";

   // Wind up to throw the Turret
   stateName[4]                     = "Fire";
   stateTransitionGeneric0In[4]     = "SprintEnter";
   stateTransitionOnTimeout[4]      = "Fire2";
   stateTimeoutValue[4]             = 0.66;
   stateFire[4]                     = true;
   stateRecoil[4]                   = "";
   stateAllowImageChange[4]         = false;
   stateSequence[4]                 = "Fire";
   stateSequenceNeverTransition[4]  = true;
   stateShapeSequence[4]            = "Recoil";

   // Throw the actual Turret
   stateName[5]                     = "Fire2";
   stateTransitionGeneric0In[5]     = "SprintEnter";
   stateTransitionOnTriggerUp[5]    = "Reload";
   stateTimeoutValue[5]             = 0.1;
   stateAllowImageChange[5]         = false;
   stateScript[5]                   = "onFire";
   stateShapeSequence[5]            = "Fire_Release";

   // Play the reload animation, and transition into
   stateName[6]                     = "Reload";
   stateTransitionGeneric0In[6]     = "SprintEnter";
   stateTransitionOnTimeout[6]      = "Ready";
   stateWaitForTimeout[6]           = true;
   stateTimeoutValue[6]             = 0.66;
   stateSequence[6]                 = "switch_in";

   // Start Sprinting
   stateName[7]                     = "SprintEnter";
   stateTransitionGeneric0Out[7]    = "SprintExit";
   stateTransitionOnTimeout[7]      = "Sprinting";
   stateWaitForTimeout[7]           = false;
   stateTimeoutValue[7]             = 0.25;
   stateWaitForTimeout[7]           = false;
   stateScaleAnimation[7]           = true;
   stateScaleAnimationFP[7]         = true;
   stateSequenceTransitionIn[7]     = true;
   stateSequenceTransitionOut[7]    = true;
   stateAllowImageChange[7]         = false;
   stateSequence[7]                 = "sprint";

   // Sprinting
   stateName[8]                     = "Sprinting";
   stateTransitionGeneric0Out[8]    = "SprintExit";
   stateWaitForTimeout[8]           = false;
   stateScaleAnimation[8]           = false;
   stateScaleAnimationFP[8]         = false;
   stateSequenceTransitionIn[8]     = true;
   stateSequenceTransitionOut[8]    = true;
   stateAllowImageChange[8]         = false;
   stateSequence[8]                 = "sprint";
   
   // Stop Sprinting
   stateName[9]                     = "SprintExit";
   stateTransitionGeneric0In[9]     = "SprintEnter";
   stateTransitionOnTimeout[9]      = "Ready";
   stateWaitForTimeout[9]           = false;
   stateTimeoutValue[9]             = 0.5;
   stateSequenceTransitionIn[9]     = true;
   stateSequenceTransitionOut[9]    = true;
   stateAllowImageChange[9]         = false;
   stateSequence[9]                 = "sprint";
};
