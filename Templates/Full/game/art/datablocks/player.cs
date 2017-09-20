//-----------------------------------------------------------------------------
// 3D Action Adventure Kit for T3D
// Copyright (C) 2008-2013 Ubiq Visuals, Inc. (http://www.ubiqvisuals.com/)
//
// This file also incorporates work covered by the following copyright and  
// permission notice:
//
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



//----------------------------------------------------------------------------
// Player Audio Profiles
//----------------------------------------------------------------------------

datablock SFXProfile(DeathCrySound)
{
   fileName = "art/sound/player/player_death01.ogg";
   description = AudioDefault3d;
   preload = true;
};

datablock SFXProfile(PainCrySound)
{
   fileName = "art/sound/player/player_pain.ogg";
   description = AudioDefault3d;
   preload = true;
};

datablock SFXProfile(playerWaterSplash)
{
   filename    = "art/sound/player/player_water_splash01.ogg";
   description = AudioDefault3d;
   preload = true;
};

//----------------------------------------------------------------------------
// Splash
//----------------------------------------------------------------------------

datablock ParticleData(PlayerFoamParticle)
{
   dragCoefficient      = 2.0;
   gravityCoefficient   = 1.0;
   inheritedVelFactor   = 0.1;
   constantAcceleration = 0.0;
   lifetimeMS           = 300;
   lifetimeVarianceMS   = 100;
   useInvAlpha          = false;
   spinRandomMin        = -90.0;
   spinRandomMax        =  90.0;
   textureName          = "art/shapes/particles/millsplash01";
   colors[0]     = "0.8 0.8 0.8 1.0";
   colors[1]     = "0.8 0.8 0.8 1.0";
   colors[2]     = "0.8 0.8 0.8 0.00";
   sizes[0]      = 0.4;
   sizes[1]      = 0.6;
   sizes[2]      = 0.8;
   times[0]      = 0.0;
   times[1]      = 0.5;
   times[2]      = 1.0;
};

datablock ParticleEmitterData(PlayerFoamEmitter)
{
   ejectionPeriodMS = 10;
   periodVarianceMS = 5;
   ejectionVelocity = 2.0;
   velocityVariance = 1.0;
   ejectionOffset   = 0.15;
   thetaMin         = 15;
   thetaMax         = 75;
   phiReferenceVel  = 0;
   phiVariance      = 360;
   overrideAdvance = false;
   particles = "PlayerFoamParticle";
};

datablock ParticleData( PlayerWakeParticle )
{
   textureName          = "art/shapes/particles/wake";
   dragCoefficient     = "0.0";
   gravityCoefficient   = "0.0";
   inheritedVelFactor   = "0.0";
   lifetimeMS           = "2500";
   lifetimeVarianceMS   = "200";
   windCoefficient = "0.0";
   useInvAlpha = "1";
   spinRandomMin = "30.0";
   spinRandomMax = "30.0";

   animateTexture = true;
   framesPerSec = 1;
   animTexTiling = "2 1";
   animTexFrames = "0 1";

   colors[0]     = "1 1 1 0.2";
   colors[1]     = "1 1 1 0.8";
   colors[2]     = "1 1 1 0.4";
   colors[3]     = "0.5 0.5 0.5 0";

   sizes[0]      = "1.0";
   sizes[1]      = "2.0";
   sizes[2]      = "3.0";
   sizes[3]      = "3.5";

   times[0]      = "0.0";
   times[1]      = "0.25";
   times[2]      = "0.5";
   times[3]      = "1.0";
};

datablock ParticleEmitterData( PlayerWakeEmitter )
{
   ejectionPeriodMS = "200";
   periodVarianceMS = "10";

   ejectionVelocity = "0";
   velocityVariance = "0";

   ejectionOffset   = "0";

   thetaMin         = "89";
   thetaMax         = "90";

   phiReferenceVel  = "0";
   phiVariance      = "1";

   alignParticles = "1";
   alignDirection = "0 0 1";

   particles = "PlayerWakeParticle";
};

//----------------------------------------------------------------------------
// Foot puffs
//----------------------------------------------------------------------------
datablock ParticleData(LightPuff)
{
   dragCoefficient      = 2.0;
   gravityCoefficient   = 0.4;
   inheritedVelFactor   = 0.6;
   constantAcceleration = 0.0;
   lifetimeMS           = 400;
   lifetimeVarianceMS   = 100;
   useInvAlpha          = true;
   spinRandomMin        = -35.0;
   spinRandomMax        = 35.0;
   textureName   = "art/shapes/actors/Maco/footpuff.png";
   colors[0]     = "1.0 1.0 1.0 1.0";
   colors[1]     = "1.0 1.0 1.0 0.0";
   sizes[0]      = 0.1;
   sizes[1]      = 0.6;
   times[0]      = 0.3;
   times[1]      = 1.0;
};

datablock ParticleEmitterData(LightPuffEmitter)
{
   ejectionPeriodMS = 35;
   periodVarianceMS = 10;
   ejectionVelocity = 0.2; //0.2
   velocityVariance = 0.1;
   ejectionOffset   = 0.0;
   thetaMin         = 20;
   thetaMax         = 30;
   phiReferenceVel  = 0;
   phiVariance      = 360;
   overrideAdvance = false;
   particles = "LightPuff";
};

//----------------------------------------------------------------------------
// Foot prints
//----------------------------------------------------------------------------
datablock DecalData(PlayerFootprint)
{
   Material = "DECAL_PlayerFootprint";
   size = "0.5";
   lifeSpan = "50000";
};


//----------------------------------------------------------------------------
// PlayerData
//----------------------------------------------------------------------------
datablock PlayerData(DefaultPlayerData)
{
   renderFirstPerson = false;

   computeCRC = false;

   // Third person shape
   shapeFile = "art/shapes/actors/Maco/player.dts";
   cameraMaxDist = 11;
   cameraMinDist = 3;
   cameraOffset = "0 0 0.7"; //this offsets our "Cam" node position for easy tweaking
   allowImageStateAnimation = true;

   // First person arms
   //imageAnimPrefixFP = "soldier";
   //shapeNameFP[0] = "art/shapes/actors/Soldier/FP/FP_SoldierArms.DAE";

   canObserve = 1;
   cmdCategory = "Clients";

   cameraDefaultFov = 55.0;
   cameraMinFov = 5.0;
   cameraMaxFov = 65.0;

   //debrisShapeName = "art/shapes/actors/common/debris_player.dts";
   //debris = playerDebris;
   
   throwForce = 30;

   aiAvoidThis = 1;

   minLookAngle = "-1.5";
   maxLookAngle = "1.5";
   maxFreelookAngle = 1.5;

   mass = 40;
   drag = 0.1;  //wind friction applied
   maxdrag = 0.4;
   groundFriction = 1.5;  //friction applied while on ground: 0 = no friction
   density = 1.1;
   maxDamage = 100;
   maxEnergy =  60;
   repairRate = 0.33;
   energyPerDamagePoint = 75;

   rechargeRate = 1.0;

   runForce = 400;  //affects how fast player achieve max speed
   runEnergyDrain = 0;
   minRunEnergy = 0;
   maxForwardSpeed = 5.2;
   maxBackwardSpeed = 3.6;
   maxSideSpeed = 3.6;

   sprintForce = 600;
   sprintEnergyDrain = 0;
   minSprintEnergy = 0;
   maxSprintForwardSpeed = 8.0;
   maxSprintBackwardSpeed = 5.5;
   maxSprintSideSpeed = 5.5;
   sprintStrafeScale = 0.25;
   sprintYawScale = 0.05;
   sprintPitchScale = 0.05;
   sprintCanJump = true;

   crouchForce = 400;
   maxCrouchForwardSpeed = 2.0;
   maxCrouchBackwardSpeed = 1.4;
   maxCrouchSideSpeed = 1.4;
   
   proneForce = 400;
   maxProneForwardSpeed = 1.0;
   maxProneBackwardSpeed = 0.69;
   maxProneSideSpeed = 0.69;
   

   groundTurnRate = 6.25;          //bigger numbers = faster turn
   airTurnRate = 3.125;
   
   maxStepHeight = 0.5;  

   walkRunAnimVelocity = 2.5;    //velocity at which player switches between walk and run animations

   maxUnderwaterForwardSpeed = 2.0;
   maxUnderwaterBackwardSpeed = 2.0;
   maxUnderwaterSideSpeed = 2.0;

   //jump
   jumpForce = 224;
   jumpEnergyDrain = 0;
   minJumpEnergy = 0;
   jumpDelay = 5;          //how long after landing before player can jump again (ticks), keep this low
   airControl = 1.0;
   standJumpCrouchDelay = 384;   //how long to delay a stand-jump while the crouch portion of the animation plays (ms)
   runJumpCrouchDelay = 160;     //how long to delay a run-jump while the crouch portion of the animation plays (ms)

   fallingSpeedThreshold = -7;     //if the players velocity is less than this, he is considered falling (plays fall animation)   landSequenceTime = 0.33;

   //landSequenceTime = 0;   //Ubiq: removing these - we use our own landing system
   //transitionToLand = false;
   //recoverDelay = 0;
   //recoverRunForceScale = 0;

   minImpactSpeed = 20;    //if player collides (with ground or wall etc.) faster than this he may get hurt, camera may shake
   minLateralImpactSpeed = 20;
   speedDamageScale = 1.0; //specifies how much impact damage the player receives based on impact speed. Not in C++, but used in player.cs server script.

   boundingBox = "0.4 0.4 1.45";
   crouchBoundingBox = "0.4 0.4 0.8";
   swimBoundingBox = "0.4 0.4 1.45";
   pickupRadius = 0.75;

	//climb
	climbHeightMin = 0.4;
	climbHeightMax = 1.6;
	climbSpeedUp = 0.35;     //how fast player moves up a climb surface
	climbSpeedDown = 0.35;   //how fast player moves down a climb surface
	climbSpeedSide = 0.35;   //how fast player moves sideways on a climb surface
	climbScrapeSpeed = -3.0; //if player falling faster than this, he can't climb but will scrape (should be less than -climbSpeedDown)
	climbScrapeFriction = 2.0;   //how much friction when trying to climb but falling too fast: 0 = slide forever, 1 = instant grab surface
   
	//ledge grab
	grabHeightMin = 1.5;             //player will detect & grab nearby ledges in this height range (relative to players feet). If player is "missing" ledges when falling quickly, make the range larger.
	grabHeightMax = 1.7;
	grabHeight = 1.6;                //upon finding a ledge, player will snap up/down so the ledge is at this height (relative to players feet). Must be between (or equal) to min/max above. Adjust until grab animations look right.
	grabSpeedSide = 0.48;             //how fast player moves sideways while ledge-grabbing
	grabSpeedUp = 0.4;               //how fast player pulls himself up a ledge. Animation will speed up/slow down as necessary to match
	grabUpForwardOffset = 0.55;      //applies a forward offset to the "teleport" that occurs at the end of the ledge up animation (match to animation)
	grabUpUpwardOffset = 0.015;       //applies an upward offset to the "teleport" that occurs at the end of the ledge up animation (should be positive, near 0). Allows climb onto sloped surface
	grabUpTestBox = "0.4 0.4 1.45";//box used to test if there is enough room above for player to pull himself up (adjust until animation has no penetrations)

	//wall hug
	wallHugSpeed = 0.5;			//how fast player moves sideways while wall hugging
	wallHugHeightMin = 0.9;		//player will detect nearby walls in this height range (minimum, relative to players feet)
	wallHugHeightMax = 1.1;		//player will detect nearby walls in this height range (maximum, relative to players feet)
	
   
   //jet jump
   jetTime = 320;       //how long after jumping the jet lasts (ms)
   jetJumpForce = 812.5;
   jetJumpEnergyDrain = 0;
   jetMinJumpEnergy = 0;
   jetJumpSurfaceAngle = 78;
   jetMinJumpSpeed = 3;
   jetMaxJumpSpeed = 100;

   //Ground Snap
   //a ray is cast down from player. If a sloped surface is found the player is moved down (visually) to
   //stand on it. This helps on slopes where otherwise the player appears to float above the surface
   groundSnapSpeed = 0.02;       //how fast to move player down (units/tick). Use 0 to turn this feature off
   groundSnapRayLength = 0.6;    //length of ray (units down from player origin - his feet). Make sure it's long enough to work on steep slopes
   groundSnapRayOffset = -0.05;   //forward offset for ray in case feet are forward or back of origin (units). Positive = forward, Negative = backward
	
   landDuration = 100;        //the land animation will play for at least this long (in ms). If not trying to move, it will play longer
   landSpeedFactor = 0.5;     //during the land, the run force has this multiplier applied (0 = no force, 1 = regular force)

   // Damage location details
   boxHeadPercentage       = 0.83;
   boxTorsoPercentage      = 0.49;
   boxHeadLeftPercentage         = 0.30;
   boxHeadRightPercentage        = 0.60;
   boxHeadBackPercentage         = 0.30;
   boxHeadFrontPercentage        = 0.60;

   // Foot Prints
   decalData   = PlayerFootprint;
   decalOffset = 0.15;

   footPuffEmitter = "LightPuffEmitter";
   footPuffNumParts = 10;
   footPuffRadius = "0.25";

   //dustEmitter = LiftoffDustEmitter;

   //splash = PlayerSplash;	//Ubiq: the Splash class doesn't seem to render anything (useless!) - removing this
   splashVelocity = 4.0;
   splashAngle = 67.0;
   splashFreqMod = 300.0;
   splashVelEpsilon = 0.60;
   bubbleEmitTime = 0.4;
   splashEmitter[0] = PlayerWakeEmitter;	//Ubiq: these are the only "splash" effects in use
   splashEmitter[1] = PlayerFoamEmitter;
   //splashEmitter[2] = PlayerBubbleEmitter;
   
   impactWaterEasy = playerWaterSplash;
   
   mediumSplashSoundVelocity = 10.0;
   impactWaterMedium = playerWaterSplash;
   
   hardSplashSoundVelocity = 20.0;
   impactWaterHard = playerWaterSplash;
   
   exitSplashSoundVelocity = 5.0;
   exitingWater = playerWaterSplash;
   
   //waterBreathSound = playerWaterSplash;	//always plays when submerged
   //movingBubblesSound = playerWaterSplash;	//plays when submerged and moving
   FootShallowSound = playerWaterSplash;	//plays when walking in water and coverage is less than footSplashHeight
   FootWadingSound = playerWaterSplash;	//plays when walking in water and coverage is less than 1 and > footSplashHeight
   
   footstepSplashHeight = 0.4;

   // Controls over slope of runnable/jumpable surfaces
   runSurfaceAngle  = 45;
   jumpSurfaceAngle = 45;

   minJumpSpeed = 0;
   maxJumpSpeed = 30;

   horizMaxSpeed = 50;
   horizResistSpeed = 0;
   horizResistFactor = 0.4;

   upMaxSpeed = 50;
   upResistSpeed = 0;
   upResistFactor = 0.4;

   //Ubiq: Player Sounds
   stopSound               = playerStop;
   jumpCrouchSound         = playerJumpCrouch;
   jumpSound               = playerJump;
   landSound               = playerLand;
   climbIdleSound          = playerClimbIdle;
   climbUpSound            = playerClimbUp;
   climbDownSound          = playerClimbDown;
   climbLeftRightSound     = playerClimbLeftRight;
   ledgeIdleSound          = playerLedgeIdle;
   ledgeUpSound            = playerLedgeUp;
   ledgeLeftRightSound     = playerLedgeLeftRight;
   slideSound              = playerSlide;
   
   //camera shake
   groundImpactMinSpeed    = 15.0;
   groundImpactShakeFreq   = "4.0 4.0 4.0";
   groundImpactShakeAmp    = "1.0 1.0 1.0";
   groundImpactShakeDuration = 0.75;
   groundImpactShakeFalloff = 10.0;

   observeParameters = "0.5 4.5 4.5";
   // available skins (see materials.cs in model folder)
   availableSkins =  "base	blue";
};
