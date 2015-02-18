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

datablock SFXProfile( MineArmedSound )
{
   filename = "art/sound/weapons/mine_armed";
   description = AudioClose3d;
   preload = true;
};

datablock SFXProfile( MineSwitchinSound )
{
   filename = "art/sound/weapons/wpn_proximitymine_switchin";
   description = AudioClose3D;
   preload = true;
};

datablock SFXProfile( MineTriggeredSound )
{
   filename = "art/sound/weapons/mine_trigger";
   description = AudioClose3d;
   preload = true;
};

datablock ProximityMineData( ProxMine )
{
   // ShapeBaseData fields
   category = "Weapon";
   shapeFile = "art/shapes/weapons/ProxMine/TP_ProxMine.DAE";
   explosion = GrenadeLauncherExplosion;

   // ItemData fields
   sticky = true;
   mass = 2;
   elasticity = 0.2;
   friction = 0.6;
   simpleServerCollision = false;

   // ProximityMineData fields
   armingDelay = 3.5;
   armingSound = MineArmedSound;

   autoTriggerDelay = 0;
   triggerOnOwner = true;
   triggerRadius = 3.0;
   triggerDelay = 0.45;
   triggerSound = MineTriggeredSound;

   explosionOffset = 0.1;
   
   // dynamic fields
   pickUpName = "a proximity mine";
   description = "Proximity Mine";
   maxInventory = 20;
   image = ProxMineImage;

   previewImage = 'mine.png';
   reticle = 'blank';
   zoomReticle = 'blank';

   damageType = "MineDamage";   // type of damage applied to objects in radius
   radiusDamage = 300;           // amount of damage to apply to objects in radius
   damageRadius = 8;            // search radius to damage objects when exploding
   areaImpulse = 2000;          // magnitude of impulse to apply to objects in radius
};

datablock ShapeBaseImageData( ProxMineImage )
{
   // Basic Item properties
   shapeFile = "art/shapes/weapons/ProxMine/TP_ProxMine.DAE";
   shapeFileFP = "art/shapes/weapons/ProxMine/FP_ProxMine.DAE";

   imageAnimPrefix = "ProxMine";
   imageAnimPrefixFP = "ProxMine";

   // Specify mount point & offset for 3rd person, and eye offset
   // for first person rendering.
   mountPoint = 0;
   firstPerson = true;
   useEyeNode = true;

   // When firing from a point offset from the eye, muzzle correction
   // will adjust the muzzle vector to point to the eye LOS point.
   // Since this weapon doesn't actually fire from the muzzle point,
   // we need to turn this off.
   correctMuzzleVector = false;

   // Add the WeaponImage namespace as a parent, WeaponImage namespace
   // provides some hooks into the inventory system.
   class = "WeaponImage";
   className = "WeaponImage";

   // Projectiles and Ammo.
   item = ProxMine;

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
   stateTimeoutValue[1]             = 3.0;
   stateSequence[1]                 = "switch_in";
   stateShapeSequence[1]            = "Reload";
   stateSound[1]                    = MineSwitchinSound;

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

   // Wind up to throw the ProxMine
   stateName[4]                     = "Fire";
   stateTransitionGeneric0In[4]     = "SprintEnter";
   stateTransitionOnTimeout[4]      = "Fire2";
   stateTimeoutValue[4]             = 0.8;
   stateFire[4]                     = true;
   stateRecoil[4]                   = "";
   stateAllowImageChange[4]         = false;
   stateSequence[4]                 = "Fire";
   stateSequenceNeverTransition[4]  = true;
   stateShapeSequence[4]            = "Fire";

   // Throw the actual mine
   stateName[5]                     = "Fire2";
   stateTransitionGeneric0In[5]     = "SprintEnter";
   stateTransitionOnTriggerUp[5]    = "Reload";
   stateTimeoutValue[5]             = 0.7;
   stateAllowImageChange[5]         = false;
   stateSequenceNeverTransition[5]  = true;
   stateSequence[5]                 = "fire_release";
   stateShapeSequence[5]            = "Fire_Release";
   stateScript[5]                   = "onFire";

   // Play the reload animation, and transition into
   stateName[6]                     = "Reload";
   stateTransitionGeneric0In[6]     = "SprintEnter";
   stateTransitionOnTimeout[6]      = "Ready";
   stateWaitForTimeout[6]           = true;
   stateTimeoutValue[6]             = 3.0;
   stateSequence[6]                 = "switch_in";
   stateShapeSequence[6]            = "Reload";
   stateSound[6]                    = MineSwitchinSound;

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
   stateSequence[7]                 = "run2sprint";

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
   stateSequence[9]                 = "sprint2run";
};
