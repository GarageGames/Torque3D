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

// RocketLauncher weapon.
// This script file contains all of the necessary datablocks needed for the
// RocketLauncher.  These datablocks include sound profiles, light descriptions,
// particle effects, explosions, projectiles, items (weapon and ammo), shell
// casings (if any), and finally the weapon image which contains the state
// machine that determines how the weapon operates.

// The main "fire" method/mode is handled in "../scripts/server/weapons.cs"
// through a "WeaponImage" namespace function.  This reduces duplicated code,
// although a unique fire method could still be implemented for this weapon.

// The "alt-fire" method/mode is handled in "../scripts/server/rocketlaucner.cs".
// Alt-fire for the Rocketlauncher allows you to "charge up" the number of
// projectiles, up to 3, that get fired.  Hold to increase the number of shots
// and release to fire.  After three shots are loaded and in the pipe, the
// weapon will automatically discharge on it's own.

// ----------------------------------------------------------------------------
// Sound profiles
// ----------------------------------------------------------------------------

datablock SFXProfile(RocketLauncherReloadSound)
{
   filename = "art/sound/weapons/Crossbow_reload";
   description = AudioClose3d;
   preload = true;
};

datablock SFXProfile(RocketLauncherFireSound)
{
   filename = "art/sound/weapons/explosion_mono_01";
   description = AudioClose3d;
   preload = true;
};

datablock SFXProfile(RocketLauncherIncLoadSound)
{
   filename = "art/sound/weapons/relbow_mono_01";
   description = AudioClose3d;
   preload = true;
};

datablock SFXProfile(RocketLauncherFireEmptySound)
{
   filename = "art/sound/weapons/Crossbow_firing_empty";
   description = AudioClose3d;
   preload = true;
};

datablock SFXProfile(RocketLauncherExplosionSound)
{
   filename = "art/sound/weapons/Crossbow_explosion";
   description = AudioDefault3d;
   preload = true;
};

// ----------------------------------------------------------------------------
// Lights for the projectile(s)
// ----------------------------------------------------------------------------

datablock LightDescription(RocketLauncherLightDesc)
{
   range = 4.0;
   color = "1 1 0";
   brightness = 5.0;
   animationType = PulseLightAnim;
   animationPeriod = 0.25;
   //flareType = SimpleLightFlare0;
};

datablock LightDescription(RocketLauncherWaterLightDesc)
{
   radius = 2.0;
   color = "1 1 1";
   brightness = 5.0;
   animationType = PulseLightAnim;
   animationPeriod = 0.25;
   //flareType = SimpleLightFlare0;
};

//----------------------------------------------------------------------------
// Debris
//----------------------------------------------------------------------------

datablock ParticleData(RocketDebrisTrailParticle)
{
   textureName = "art/shapes/particles/impact";
   dragCoeffiecient = 0;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 1200;//1000;
   lifetimeVarianceMS = 299;//500;
   useInvAlpha = true;//false;
   spinSpeed = 1;
   spinRandomMin = -300.0;
   spinRandomMax = 0;
   colors[0] = "1 0.897638 0.795276 0.4";
   colors[1] = "0.795276 0.795276 0.795276 0.6";
   colors[2] = "0 0 0 0";
   sizes[0] = 0.5;//1.0;
   sizes[1] = 2;
   sizes[2] = 1;//1.0;
   times[0] = 0.0;
   times[1] = 0.498039;
   times[2] = 1.0;
   animTexName = "art/shapes/particles/impact";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketDebrisTrailEmitter)
{
   ejectionPeriodMS = 6;//8;
   periodVarianceMS = 2;//4;
   ejectionVelocity = 1.0;
   velocityVariance = 0.5;
   thetaMin = 0.0;
   thetaMax = 180.0;
   phiReferenceVel = 0;
   phiVariance = 360;
   ejectionoffset = 0.0;//0.3;
   particles = "RocketDebrisTrailParticle";
};

datablock DebrisData(RocketDebris)
{
   shapeFile = "art/shapes/weapons/SwarmGun/rocket.dts";
   emitters[0] = RocketDebrisTrailEmitter;
   elasticity = 0.5;
   friction = 0.5;
   numBounces = 1;//2;
   bounceVariance = 1;
   explodeOnMaxBounce = true;
   staticOnMaxBounce = false;
   snapOnMaxBounce = false;
   minSpinSpeed = 400;
   maxSpinSpeed = 800;
   render2D = false;
   lifetime = 0.25;//0.5;//1;//2;
   lifetimeVariance = 0.0;//0.25;//0.5;
   velocity = 35;//30;//15;
   velocityVariance = 10;//5;
   fade = true;
   useRadiusMass = true;
   baseRadius = 0.3;
   gravModifier = 1.0;
   terminalVelocity = 45;
   ignoreWater = false;
};

// ----------------------------------------------------------------------------
// Splash effects
// ----------------------------------------------------------------------------

datablock ParticleData(RocketSplashMist)
{
   dragCoefficient = 1.0;
   windCoefficient = 2.0;
   gravityCoefficient = 0.3;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 600;
   lifetimeVarianceMS = 100;
   useInvAlpha = false;
   spinRandomMin = -90.0;
   spinRandomMax = 500.0;
   spinSpeed = 1;
   textureName = "art/shapes/particles/smoke";
   colors[0] = "0.7 0.8 1.0 1.0";
   colors[1] = "0.7 0.8 1.0 0.5";
   colors[2] = "0.7 0.8 1.0 0.0";
   sizes[0] = 0.2;//0.5;
   sizes[1] = 0.4;//0.5;
   sizes[2] = 0.8;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketSplashMistEmitter)
{
   ejectionPeriodMS = 5;
   periodVarianceMS = 0;
   ejectionVelocity = 3.0;
   velocityVariance = 2.0;
   ejectionOffset = 0.15;
   thetaMin = 85;
   thetaMax = 85;
   phiReferenceVel = 0;
   phiVariance = 360;
   overrideAdvance = false;
   lifetimeMS = 250;
   particles = "RocketSplashMist";
};

datablock ParticleData(RocketSplashParticle)
{
   dragCoefficient = 1;
   windCoefficient = 0.9;
   gravityCoefficient = 0.3;
   inheritedVelFactor = 0.2;
   constantAcceleration = -1.4;
   lifetimeMS = 600;
   lifetimeVarianceMS = 200;
   textureName = "art/shapes/particles/droplet";
   colors[0] = "0.7 0.8 1.0 1.0";
   colors[1] = "0.7 0.8 1.0 0.5";
   colors[2] = "0.7 0.8 1.0 0.0";
   sizes[0] = 0.5;
   sizes[1] = 0.25;
   sizes[2] = 0.25;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketSplashEmitter)
{
   ejectionPeriodMS = 4;
   periodVarianceMS = 0;
   ejectionVelocity = 7.3;
   velocityVariance = 2.0;
   ejectionOffset = 0.0;
   thetaMin = 30;
   thetaMax = 80;
   phiReferenceVel = 00;
   phiVariance = 360;
   overrideAdvance = false;
   orientParticles = true;
   orientOnVelocity = true;
   lifetimeMS = 100;
   particles = "RocketSplashParticle";
};

datablock ParticleData(RocketSplashRingParticle)
{
   textureName = "art/shapes/particles/wake";
   dragCoefficient = 0.0;
   gravityCoefficient = 0.0;
   inheritedVelFactor = 0.0;
   lifetimeMS = 2500;
   lifetimeVarianceMS = 200;
   windCoefficient = 0.0;
   useInvAlpha = 1;
   spinRandomMin = 30.0;
   spinRandomMax = 30.0;
   spinSpeed = 1;
   animateTexture = true;
   framesPerSec = 1;
   animTexTiling = "2 1";
   animTexFrames = "0 1";
   colors[0] = "0.7 0.8 1.0 1.0";
   colors[1] = "0.7 0.8 1.0 0.5";
   colors[2] = "0.7 0.8 1.0 0.0";
   sizes[0] = 2.0;
   sizes[1] = 4.0;
   sizes[2] = 8.0;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketSplashRingEmitter)
{
   lifetimeMS = "100";
   ejectionPeriodMS = 200;
   periodVarianceMS = 10;
   ejectionVelocity = 0;
   velocityVariance = 0;
   ejectionOffset = 0;
   thetaMin = 89;
   thetaMax = 90;
   phiReferenceVel = 0;
   phiVariance = 1;
   alignParticles = 1;
   alignDirection = "0 0 1";
   particles = "RocketSplashRingParticle";
};

datablock SplashData(RocketSplash)
{
//    numSegments = 15;
//    ejectionFreq = 15;
//    ejectionAngle = 40;
//    ringLifetime = 0.5;
//    lifetimeMS = 300;
//    velocity = 4.0;
//    startRadius = 0.0;
//    acceleration = -3.0;
//    texWrap = 5.0;
//    texture = "art/shapes/particles/splash";

   emitter[0] = RocketSplashEmitter;
   emitter[1] = RocketSplashMistEmitter;
   emitter[2] = RocketSplashRingEmitter;

//    colors[0] = "0.7 0.8 1.0 0.0";
//    colors[1] = "0.7 0.8 1.0 0.3";
//    colors[2] = "0.7 0.8 1.0 0.7";
//    colors[3] = "0.7 0.8 1.0 0.0";
//
//    times[0] = 0.0;
//    times[1] = 0.4;
//    times[2] = 0.8;
//    times[3] = 1.0;
};

// ----------------------------------------------------------------------------
// Explosion Particle effects
// ----------------------------------------------------------------------------

datablock ParticleData(RocketExpFire)
{
   gravityCoefficient = "-0.50061";
   lifetimeMS = "400";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-200";
   spinRandomMax = "0";
   textureName = "art/shapes/particles/smoke";
   animTexName = "art/shapes/particles/smoke";
   colors[0] = "1 0.897638 0.795276 1";
   colors[1] = "0.795276 0.393701 0 0.6";
   colors[2] = "0 0 0 0";
   sizes[0] = "1.99902";
   sizes[1] = "7.99915";
   sizes[2] = "3.99805";
   times[1] = "0.392157";
   times[2] = "1";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketExpFireEmitter)
{
   ejectionPeriodMS = "10";
   periodVarianceMS = "5";
   ejectionVelocity = "3";
   velocityVariance = "2";
   particles = "RocketExpFire";
   blendStyle = "NORMAL";
};

datablock ParticleData(RocketExpFireball)
{
   textureName = "art/shapes/particles/fireball.png";
   lifetimeMS = "300";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-400";
   spinRandomMax = "0";
   animTexName = "art/shapes/particles/fireball.png";
   colors[0] = "1 0.897638 0.795276 0.2";
   colors[1] = "1 0.496063 0 0.6";
   colors[2] = "0.0944882 0.0944882 0.0944882 0";
   sizes[0] = "0.997986";
   sizes[1] = "1.99902";
   sizes[2] = "2.99701";
   times[1] = "0.498039";
   times[2] = "1";
   times[3] = "1";
   gravityCoefficient = "-1";
};

datablock ParticleEmitterData(RocketExpFireballEmitter)
{
   particles = "RocketExpFireball";
   blendStyle = "ADDITIVE";
   ejectionPeriodMS = "10";
   periodVarianceMS = "5";
   ejectionVelocity = "4";
   velocityVariance = "2";
   ejectionOffset = "2";
   thetaMax = "120";
};

datablock ParticleData(RocketExpSmoke)
{
   lifetimeMS = 1200;//"1250";
   lifetimeVarianceMS = 299;//200;//"250";
   textureName = "art/shapes/particles/smoke";
   animTexName = "art/shapes/particles/smoke";
   useInvAlpha = "1";
   gravityCoefficient = "-0.100122";
   spinSpeed = "1";
   spinRandomMin = "-100";
   spinRandomMax = "0";
   colors[0] = "0.897638 0.795276 0.692913 0.4";//"0.192157 0.192157 0.192157 0.0944882";
   colors[1] = "0.897638 0.897638 0.897638 0.8";//"0.454902 0.454902 0.454902 0.897638";
   colors[2] = "0.4 0.4 0.4 0";//"1 1 1 0";
   sizes[0] = "1.99597";
   sizes[1] = "3.99805";
   sizes[2] = "7.99915";
   times[1] = "0.494118";
   times[2] = "1";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketExpSmokeEmitter)
{
   ejectionPeriodMS = "15";
   periodVarianceMS = "5";
   //ejectionOffset = "1";
   thetaMax = "180";
   particles = "RocketExpSmoke";
   blendStyle = "NORMAL";
};

datablock ParticleData(RocketExpSparks)
{
   textureName = "art/shapes/particles/droplet.png";
   lifetimeMS = "100";
   lifetimeVarianceMS = "50";
   animTexName = "art/shapes/particles/droplet.png";
   inheritedVelFactor = "0.391389";
   sizes[0] = "1.99902";
   sizes[1] = "2.49954";
   sizes[2] = "0.997986";
   colors[0] = "1.0 0.9 0.8 0.2";
   colors[1] = "1.0 0.9 0.8 0.8";
   colors[2] = "0.8 0.4 0.0 0.0";
   times[0] = "0";
   times[1] = "0.34902";
   times[2] = "1";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketExpSparksEmitter)
{
   particles = "RocketExpSparks";
   blendStyle = "NORMAL";
   ejectionPeriodMS = "10";
   periodVarianceMS = "5";
   ejectionVelocity = "60";
   velocityVariance = "10";
   thetaMax = "120";
   phiReferenceVel = 0;
   phiVariance = "360";
   ejectionOffset = "0";
   orientParticles = true;
   orientOnVelocity = true;
};

datablock ParticleData(RocketExpSubFireParticles)
{
   textureName = "art/shapes/particles/fireball.png";
   gravityCoefficient = "-0.202686";
   lifetimeMS = "400";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-200";
   spinRandomMax = "0";
   animTexName = "art/shapes/particles/fireball.png";
   colors[0] = "1 0.897638 0.795276 0.2";
   colors[1] = "1 0.496063 0 1";
   colors[2] = "0.0944882 0.0944882 0.0944882 0";
   sizes[0] = "0.997986";
   sizes[1] = "1.99902";
   sizes[2] = "2.99701";
   times[1] = "0.498039";
   times[2] = "1";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketExpSubFireEmitter)
{
   particles = "RocketExpSubFireParticles";
   blendStyle = "ADDITIVE";
   ejectionPeriodMS = "10";
   periodVarianceMS = "5";
   ejectionVelocity = "4";
   velocityVariance = "2";
   thetaMax = "120";
};

datablock ParticleData(RocketExpSubSmoke)
{
   textureName = "art/shapes/particles/smoke";
   gravityCoefficient = "-0.40293";
   lifetimeMS = "800";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-200";
   spinRandomMax = "0";
   animTexName = "art/shapes/particles/smoke";
   colors[0] = "0.4 0.35 0.3 0.393701";
   colors[1] = "0.45 0.45 0.45 0.795276";
   colors[2] = "0.4 0.4 0.4 0";
   sizes[0] = "1.99902";
   sizes[1] = "3.99805";
   sizes[2] = "7.99915";
   times[1] = "0.4";
   times[2] = "1";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketExpSubSmokeEmitter)
{
   particles = "RocketExpSubSmoke";
   ejectionPeriodMS = "30";
   periodVarianceMS = "10";
   ejectionVelocity = "2";
   velocityVariance = "1";
   ejectionOffset = 1;//"2";
   blendStyle = "NORMAL";
};

// ----------------------------------------------------------------------------
// Water Explosion
// ----------------------------------------------------------------------------

datablock ParticleData(RLWaterExpDust)
{
   textureName = "art/shapes/particles/steam";
   dragCoefficient = 1.0;
   gravityCoefficient = -0.01;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 2500;
   lifetimeVarianceMS = 250;
   useInvAlpha = false;
   spinSpeed = 1;
   spinRandomMin = -90.0;
   spinRandomMax = 500.0;
   colors[0] = "0.6 0.6 1.0 0.5";
   colors[1] = "0.6 0.6 1.0 0.3";
   sizes[0] = 0.25;
   sizes[1] = 1.5;
   times[0] = 0.0;
   times[1] = 1.0;
};

datablock ParticleEmitterData(RLWaterExpDustEmitter)
{
   ejectionPeriodMS = 1;
   periodVarianceMS = 0;
   ejectionVelocity = 10;
   velocityVariance = 0.0;
   ejectionOffset = 0.0;
   thetaMin = 85;
   thetaMax = 85;
   phiReferenceVel = 0;
   phiVariance = 360;
   overrideAdvances = false;
   lifetimeMS = 75;
   particles = "RLWaterExpDust";
};

datablock ParticleData(RLWaterExpSparks)
{
   textureName = "art/shapes/particles/spark_wet";
   dragCoefficient = 1;
   gravityCoefficient = 0.0;
   inheritedVelFactor = 0.2;
   constantAcceleration = 0.0;
   lifetimeMS = 500;
   lifetimeVarianceMS = 250;
   colors[0] = "0.6 0.6 1.0 1.0";
   colors[1] = "0.6 0.6 1.0 1.0";
   colors[2] = "0.6 0.6 1.0 0.0";
   sizes[0] = 0.5;
   sizes[1] = 0.5;
   sizes[2] = 0.75;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RLWaterExpSparkEmitter)
{
   ejectionPeriodMS = 2;
   periodVarianceMS = 0;
   ejectionVelocity = 12;
   velocityVariance = 6.75;
   ejectionOffset = 0.0;
   thetaMin = 0;
   thetaMax = 60;
   phiReferenceVel = 0;
   phiVariance = 360;
   overrideAdvances = false;
   orientParticles = true;
   lifetimeMS = 100;
   particles = "RLWaterExpSparks";
};

datablock ParticleData(RLWaterExpSmoke)
{
   textureName = "art/shapes/particles/smoke";
   dragCoeffiecient = 0.4;
   gravityCoefficient = -0.25;
   inheritedVelFactor = 0.025;
   constantAcceleration = -1.1;
   lifetimeMS = 1250;
   lifetimeVarianceMS = 0;
   useInvAlpha = false;
   spinSpeed = 1;
   spinRandomMin = -200.0;
   spinRandomMax = 200.0;
   colors[0] = "0.1 0.1 1.0 1.0";
   colors[1] = "0.4 0.4 1.0 1.0";
   colors[2] = "0.4 0.4 1.0 0.0";
   sizes[0] = 2.0;
   sizes[1] = 6.0;
   sizes[2] = 2.0;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RLWaterExpSmokeEmitter)
{
   ejectionPeriodMS = 15;
   periodVarianceMS = 0;
   ejectionVelocity = 6.25;
   velocityVariance = 0.25;
   thetaMin = 0.0;
   thetaMax = 90.0;
   lifetimeMS = 250;
   particles = "RLWaterExpSmoke";
};

datablock ParticleData(RLWaterExpBubbles)
{
   textureName = "art/shapes/particles/millsplash01";
   dragCoefficient = 0.0;
   gravityCoefficient = -0.05;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 1500;
   lifetimeVarianceMS = 250;
   useInvAlpha = false;
   spinRandomMin = -100.0;
   spinRandomMax = 100.0;
   spinSpeed = 1;
   colors[0] = "0.7 0.8 1.0 0.0";
   colors[1] = "0.7 0.8 1.0 0.4";
   colors[2] = "0.7 0.8 1.0 0.0";
   sizes[0] = 0.2;
   sizes[1] = 0.4;
   sizes[2] = 0.8;
   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RLWaterExpBubbleEmitter)
{
   ejectionPeriodMS = 5;
   periodVarianceMS = 0;
   ejectionVelocity = 1.0;
   ejectionOffset = 3.0;
   velocityVariance = 0.5;
   thetaMin = 0;
   thetaMax = 80;
   phiReferenceVel = 0;
   phiVariance = 360;
   overrideAdvances = false;
   particles = "RLWaterExpBubbles";
};

datablock ExplosionData(RocketLauncherWaterExplosion)
{
   //soundProfile = RLWaterExplosionSound;

   emitter[0] = RLWaterExpDustEmitter;
   emitter[1] = RLWaterExpSparkEmitter;
   emitter[2] = RLWaterExpSmokeEmitter;
   emitter[3] = RLWaterExpBubbleEmitter;

   shakeCamera = true;
   camShakeFreq = "10.0 11.0 9.0";
   camShakeAmp = "20.0 20.0 20.0";
   camShakeDuration = 1.5;
   camShakeRadius = 20.0;

   lightStartRadius = 20.0;
   lightEndRadius = 0.0;
   lightStartColor = "0.9 0.9 0.8";
   lightEndColor = "0.6 0.6 1.0";
   lightStartBrightness = 2.0;
   lightEndBrightness = 0.0;
};

// ----------------------------------------------------------------------------
// Dry/Air Explosion Objects
// ----------------------------------------------------------------------------

datablock ExplosionData(RocketSubExplosion)
{
   lifeTimeMS = 100;
   offset = 0.4;
   emitter[0] = RocketExpSubFireEmitter;
   emitter[1] = RocketExpSubSmokeEmitter;
};


datablock ExplosionData(RocketLauncherExplosion)
{
   soundProfile = RocketLauncherExplosionSound;
   lifeTimeMS = 200; // I want a quick bang and dissipation, not a slow burn-out

   // Volume particles
   particleEmitter = RocketExpSmokeEmitter;
   particleDensity = 10;//20;
   particleRadius = 1;//2;

   // Point emission
   emitter[0] = RocketExpFireEmitter;
   emitter[1] = RocketExpSparksEmitter;
   emitter[2] = RocketExpSparksEmitter;
   emitter[3] = RocketExpFireballEmitter;

   // Sub explosion objects
   subExplosion[0] = RocketSubExplosion;

   // Camera Shaking
   shakeCamera = true;
   camShakeFreq = "10.0 11.0 9.0";
   camShakeAmp = "15.0 15.0 15.0";
   camShakeDuration = 1.5;
   camShakeRadius = 20;

   // Exploding debris
   debris = RocketDebris;
   debrisThetaMin = 0;//10;
   debrisThetaMax = 90;//80;
   debrisNum = 5;
   debrisNumVariance = 2;
   debrisVelocity = 1;//2;
   debrisVelocityVariance = 0.2;//0.5;

   lightStartRadius = 6.0;
   lightEndRadius = 0.0;
   lightStartColor = "1.0 0.7 0.2";
   lightEndColor = "0.9 0.7 0.0";
   lightStartBrightness = 2.5;
   lightEndBrightness = 0.0;
   lightNormalOffset = 3.0;
};

// ----------------------------------------------------------------------------
// Underwater Rocket projectile trail
// ----------------------------------------------------------------------------

datablock ParticleData(RocketTrailWaterParticle)
{
   textureName = "art/shapes/particles/bubble";
   dragCoefficient = 0.0;
   gravityCoefficient = 0.1;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 1500;
   lifetimeVarianceMS = 600;
   useInvAlpha = false;
   spinRandomMin = -100.0;
   spinRandomMax = 100.0;
   spinSpeed = 1;

   colors[0] = "0.7 0.8 1.0 1.0";
   colors[1] = "0.7 0.8 1.0 0.4";
   colors[2] = "0.7 0.8 1.0 0.0";

   sizes[0] = 0.05;
   sizes[1] = 0.05;
   sizes[2] = 0.05;

   times[0] = 0.0;
   times[1] = 0.5;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketTrailWaterEmitter)
{
   ejectionPeriodMS = 5;
   periodVarianceMS = 0;
   ejectionVelocity = 1.0;
   ejectionOffset = 0.1;
   velocityVariance = 0.5;
   thetaMin = 0.0;
   thetaMax = 80.0;
   phiReferenceVel = 0;
   phiVariance = 360;
   overrideAdvances = false;
   particles = RocketTrailWaterParticle;
};

// ----------------------------------------------------------------------------
// Normal-fire Projectile Object
// ----------------------------------------------------------------------------

datablock ParticleData(RocketProjSmokeTrail)
{
   textureName = "art/shapes/particles/smoke";
   dragCoeffiecient = 0;
   gravityCoefficient = -0.202686;
   inheritedVelFactor = 0.0;
   constantAcceleration = 0.0;
   lifetimeMS = 750;
   lifetimeVarianceMS = 749;
   useInvAlpha = true;
   spinRandomMin = -60;
   spinRandomMax = 0;
   spinSpeed = 1;

   colors[0] = "0.3 0.3 0.3 0.598425";
   colors[1] = "0.9 0.9 0.9 0.897638";
   colors[2] = "0.9 0.9 0.9 0";

   sizes[0] = 0.247207;
   sizes[1] = 0.497467;
   sizes[2] = 0.747726;

   times[0] = 0.0;
   times[1] = 0.4;
   times[2] = 1.0;
   animTexName = "art/shapes/particles/smoke";
   times[3] = "1";
};

datablock ParticleEmitterData(RocketProjSmokeTrailEmitter)
{
   ejectionPeriodMS = 1;
   periodVarianceMS = 0;
   ejectionVelocity = 0.75;
   velocityVariance = 0;
   thetaMin = 0.0;
   thetaMax = 0.0;
   phiReferenceVel = 90;
   phiVariance = 0;
   particles = "RocketProjSmokeTrail";
};

datablock ProjectileData(RocketLauncherProjectile)
{
   projectileShapeName = "art/shapes/weapons/SwarmGun/rocket.dts";
   directDamage = 30;
   radiusDamage = 30;
   damageRadius = 5;
   areaImpulse = 2500;

   explosion = RocketLauncherExplosion;
   waterExplosion = RocketLauncherWaterExplosion;

   decal = ScorchRXDecal;
   splash = RocketSplash;

   particleEmitter = RocketProjSmokeTrailEmitter;
   particleWaterEmitter = RocketTrailWaterEmitter;

   muzzleVelocity = 100;
   velInheritFactor = 0.3;

   armingDelay = 0;
   lifetime = 5000; //(500m / 100m/s = 5000ms)
   fadeDelay = 4500;

   bounceElasticity = 0;
   bounceFriction = 0;
   isBallistic = false;
   gravityMod = 0.80;

   lightDesc = RocketLauncherLightDesc;

   damageType = "RocketDamage";
};

// ----------------------------------------------------------------------------
// Underwater Projectile
// ----------------------------------------------------------------------------

datablock ProjectileData(RocketWetProjectile)
{
   projectileShapeName = "art/shapes/weapons/SwarmGun/rocket.dts";
   directDamage = 20;
   radiusDamage = 10;
   damageRadius = 10;
   areaImpulse = 2000;

   explosion = RocketLauncherWaterExplosion;

   particleEmitter = RocketProjSmokeTrailEmitter;
   particleWaterEmitter = RocketTrailWaterEmitter;

   muzzleVelocity = 20;
   velInheritFactor = 0.3;

   armingDelay = 0;
   lifetime = 5000; //(500m / 100m/s = 5000ms)
   fadeDelay = 4500;

   bounceElasticity = 0.2;
   bounceFriction = 0.4;
   isBallistic = true;
   gravityMod = 0.80;

   lightDesc = RocketLauncherWaterLightDesc;

   damageType = "RocketDamage";
};

// ----------------------------------------------------------------------------
// Shell that's ejected during reload.
// ----------------------------------------------------------------------------

datablock DebrisData(RocketlauncherShellCasing)
{
   shapeFile = "art/shapes/weapons/SwarmGun/rocket.dts";
   lifetime = 6.0;
   minSpinSpeed = 300.0;
   maxSpinSpeed = 400.0;
   elasticity = 0.65;
   friction = 0.05;
   numBounces = 5;
   staticOnMaxBounce = true;
   snapOnMaxBounce = false;
   fade = true;
};

// ----------------------------------------------------------------------------
// Particle Emitter played when firing.
// ----------------------------------------------------------------------------

datablock ParticleData(RocketLauncherfiring1Particle)
{
   textureName = "art/shapes/particles/Fireball";
   dragCoefficient = 100.0;
   gravityCoefficient = -0.25;//-0.5;//0.0;
   inheritedVelFactor = 0.25;//1.0;
   constantAcceleration = 0.1;
   lifetimeMS = 400;
   lifetimeVarianceMS = 100;
   useInvAlpha = false;
   spinSpeed = 1;
   spinRandomMin = -200;
   spinRandomMax = 200;
   colors[0] = "1 0.9 0.8 0.1";
   colors[1] = "1 0.5 0 0.3";
   colors[2] = "0.1 0.1 0.1 0";
   sizes[0] = 0.2;//1;
   sizes[1] = 0.25;//0.15;//0.75;
   sizes[2] = 0.3;//0.1;//0.5;
   times[0] = 0.0;
   times[1] = 0.5;//0.294118;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketLauncherfiring1Emitter)
{
   ejectionPeriodMS = 15;//75;
   periodVarianceMS = 5;
   ejectionVelocity = 1;
   ejectionOffset = 0.0;
   velocityVariance = 0;
   thetaMin = 0.0;
   thetaMax = 180;//10.0;
   particles = "RocketLauncherfiring1Particle";
   blendStyle = "ADDITIVE";
};

datablock ParticleData(RocketLauncherfiring2Particle)
{
   textureName = "art/shapes/particles/impact";
   dragCoefficient = 100.0;
   gravityCoefficient = -0.5;//0.0;
   inheritedVelFactor = 0.25;//1.0;
   constantAcceleration = 0.1;
   lifetimeMS = 1600;//400;
   lifetimeVarianceMS = 400;//100;
   useInvAlpha = false;
   spinSpeed = 1;
   spinRandomMin = -200;
   spinRandomMax = 200;
   colors[0] = "0.4 0.4 0.4 0.2";
   colors[1] = "0.4 0.4 0.4 0.1";
   colors[2] = "0.0 0.0 0.0 0.0";
   sizes[0] = 0.2;//1;
   sizes[1] = 0.15;//0.75;
   sizes[2] = 0.1;//0.5;
   times[0] = 0.0;
   times[1] = 0.5;//0.294118;
   times[2] = 1.0;
};

datablock ParticleEmitterData(RocketLauncherfiring2Emitter)
{
   ejectionPeriodMS = 15;//75;
   periodVarianceMS = 5;
   ejectionVelocity = 1;
   ejectionOffset = 0.0;
   velocityVariance = 0;
   thetaMin = 0.0;
   thetaMax = 180;//10.0;
   particles = "RocketLauncherfiring2Particle";
   blendStyle = "NORMAL";
};

// ----------------------------------------------------------------------------
// Ammo Item
// ----------------------------------------------------------------------------

datablock ItemData(RocketLauncherAmmo)
{
   // Mission editor category
   category = "Ammo";

   // Add the Ammo namespace as a parent. The ammo namespace provides
   // common ammo related functions and hooks into the inventory system.
   className = "Ammo";

   // Basic Item properties
   shapeFile = "art/shapes/weapons/SwarmGun/rocket.dts";
   mass = 2;
   elasticity = 0.2;
   friction = 0.6;

   // Dynamic properties defined by the scripts
   pickUpName = "Rockets";
   maxInventory = 20;
};

// ----------------------------------------------------------------------------
// Weapon Item. This is the item that exists in the world,
// i.e. when it's been dropped, thrown or is acting as re-spawnable item.
// When the weapon is mounted onto a shape, the Image is used.
// ----------------------------------------------------------------------------

datablock ItemData(RocketLauncher)
{
   // Mission editor category
   category = "Weapon";

   // Hook into Item Weapon class hierarchy. The weapon namespace
   // provides common weapon handling functions in addition to hooks
   // into the inventory system.
   className = "Weapon";

   // Basic Item properties
   shapefile = "art/shapes/weapons/SwarmGun/swarmgun.dts";
   mass = 5;
   elasticity = 0.2;
   friction = 0.6;
   emap = true;

   // Dynamic properties defined by the scripts
   pickUpName = "SwarmGun";
   description = "RocketLauncher";
   image = RocketLauncherImage;

   // weaponHUD
   previewImage = 'swarmer.png';
   reticle = 'reticle_rocketlauncher';
   zoomReticle = 'bino';
};

// ----------------------------------------------------------------------------
// Image which does all the work. Images do not normally exist in
// the world, they can only be mounted on ShapeBase objects.
// ----------------------------------------------------------------------------

datablock ShapeBaseImageData(RocketLauncherImage)
{
   // Basic Item properties
   shapefile = "art/shapes/weapons/SwarmGun/swarmgun.dts";
   emap = true;

   // Specify mount point & offset for 3rd person, and eye offset
   // for first person rendering.
   mountPoint = 0;
   offset = "0.0 0.15 0.025";
   eyeOffset = "0.25 0.6 -0.4"; // 0.25=right/left 0.5=forward/backward, -0.5=up/down

   // When firing from a point offset from the eye, muzzle correction
   // will adjust the muzzle vector to point to the eye LOS point.
   // Since this weapon doesn't actually fire from the muzzle point,
   // we need to turn this off.
   correctMuzzleVector = false;

   // Add the WeaponImage namespace as a parent, WeaponImage namespace
   // provides some hooks into the inventory system.
   className = "WeaponImage";

   // Projectile && Ammo.
   item = RocketLauncher;
   ammo = RocketLauncherAmmo;
   projectile = RocketLauncherProjectile;
   wetProjectile = RocketWetProjectile;
   projectileType = Projectile;

   // shell casings
   casing = RocketlauncherShellCasing;
   shellExitDir = "1.0 0.3 1.0";
   shellExitOffset = "0.15 -0.56 -0.1";
   shellExitVariance = 15.0;
   shellVelocity = 3.0;

   // Let there be light - NoLight, ConstantLight, PulsingLight, WeaponFireLight.
   lightType = "WeaponFireLight";
   lightColor = "1.0 1.0 0.9";
   lightDuration = 200;
   lightRadius = 10;

   // Images have a state system which controls how the animations
   // are run, which sounds are played, script callbacks, etc. This
   // state system is downloaded to the client so that clients can
   // predict state changes and animate accordingly. The following
   // system supports basic ready->fire->reload transitions as
   // well as a no-ammo->dryfire idle state.

   // Initial start up state
   stateName[0] = "Preactivate";
   stateTransitionOnLoaded[0] = "Activate";
   stateTransitionOnNoAmmo[0] = "NoAmmo";

   // Activating the gun.
   // Called when the weapon is first mounted and there is ammo.
   stateName[1] = "Activate";
   stateTransitionOnTimeout[1] = "Ready";
   stateTimeoutValue[1] = 0.6;
   stateSequence[1] = "Activate";

   // Ready to fire, just waiting for the trigger
   stateName[2] = "Ready";
   stateTransitionOnNoAmmo[2] = "NoAmmo";
   stateTransitionOnTriggerDown[2] = "CheckWet";
   stateTransitionOnAltTriggerDown[2] = "CheckWetAlt";
   stateSequence[2] = "Ready";

   // Fire the weapon. Calls the fire script which does the actual work.
   stateName[3] = "Fire";
   stateTransitionOnTimeout[3] = "PostFire";
   stateTimeoutValue[3] = 0.9;
   stateFire[3] = true;
   stateRecoil[3] = LightRecoil;
   stateAllowImageChange[3] = false;
   stateSequence[3] = "Fire";
   stateScript[3] = "onFire";
   stateSound[3] = RocketLauncherFireSound;
   stateEmitter[3] = RocketLauncherfiring1Emitter;
   stateEmitterTime[3] = 0.6;

   // Check ammo
   stateName[4] = "PostFire";
   stateTransitionOnAmmo[4] = "Reload";
   stateTransitionOnNoAmmo[4] = "NoAmmo";

   // Play the reload animation, and transition into
   stateName[5] = "Reload";
   stateTransitionOnTimeout[5] = "Ready";
   stateTimeoutValue[5] = 0.9;
   stateAllowImageChange[5] = false;
   stateSequence[5] = "Reload";
   stateEjectShell[5] = false; // set to true to enable shell casing eject
   stateSound[5] = RocketLauncherReloadSound;
   stateEmitter[5] = RocketLauncherfiring2Emitter;
   stateEmitterTime[5] = 2.4;

   // No ammo in the weapon, just idle until something shows up.
   // Play the dry fire sound if the trigger iS pulled.
   stateName[6] = "NoAmmo";
   stateTransitionOnAmmo[6] = "Reload";
   stateSequence[6] = "NoAmmo";
   stateTransitionOnTriggerDown[6] = "DryFire";

   // No ammo dry fire
   stateName[7] = "DryFire";
   stateTimeoutValue[7] = 1.0;
   stateTransitionOnTimeout[7] = "NoAmmo";
   stateSound[7] = RocketLauncherFireEmptySound;

   // Check if wet
   stateName[8] = "CheckWet";
   stateTransitionOnWet[8] = "WetFire";
   stateTransitionOnNotWet[8] = "Fire";

   // Check if alt wet
   stateName[9] = "CheckWetAlt";
   stateTransitionOnWet[9] = "WetFire";
   stateTransitionOnNotWet[9] = "ChargeUp1";

   // Wet fire
   stateName[10] = "WetFire";
   stateTransitionOnTimeout[10] = "PostFire";
   stateTimeoutValue[10] = 0.9;
   stateFire[10] = true;
   stateRecoil[10] = LightRecoil;
   stateAllowImageChange[10] = false;
   stateSequence[10] = "Fire";
   stateScript[10] = "onWetFire";
   stateSound[10] = RocketLauncherFireSound;

   // Begin "charge up", 1 in the pipe
   stateName[11] = "ChargeUp1";
   stateScript[11] = "readyLoad";
   stateSound[11] = RocketLauncherIncLoadSound;
   stateTransitionOnAltTriggerUp[11] = "AltFire";
   stateTransitionOnTimeout[11] = "ChargeUp2";
   stateTimeoutValue[11] = 0.8;
   stateWaitForTimeout[11] = false;

   // Charge up, 2 in the pipe
   stateName[12] = "ChargeUp2";
   stateScript[12] = "incLoad";
   stateSound[12] = RocketLauncherIncLoadSound;
   stateTransitionOnAltTriggerUp[12] = "AltFire";
   stateTransitionOnTimeout[12] = "ChargeUp3";
   stateTimeoutValue[12] = 0.8;
   stateWaitForTimeout[12] = false;

   // Charge up, 3 in the pipe
   stateName[13] = "ChargeUp3";
   stateScript[13] = "incLoad";
   stateSound[13] = RocketLauncherIncLoadSound;
   stateTransitionOnAltTriggerUp[13] = "AltFire";
   stateTransitionOnTimeout[13] = "Altfire";  // lets force them to fire
   stateTimeOutValue[13] = 1.2;
   stateWaitForTimeout[13] = false;

   // Alt-fire
   stateName[14] = "AltFire";
   stateTransitionOnTimeout[14] = "PostFire";
   stateTimeoutValue[14] = 1.2;
   stateFire[14] = true;
   stateRecoil[14] = LightRecoil;
   stateAllowImageChange[14] = false;
   stateSequence[14] = "Fire";
   stateScript[14] = "onAltFire";
   stateSound[14] = RocketLauncherFireSound;
   stateEmitter[14] = RocketLauncherfiring1Emitter;
   stateEmitterTime[14] = 1.2;
};
