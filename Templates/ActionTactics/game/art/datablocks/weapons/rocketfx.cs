// ----------------------------------------------------------------------------
// Placeholder explosion with required sounds, debris, and particle datablocks.
// These datablocks existed in now removed scripts, but were used within some
// that remain: see cheetahCar.cs
//
// These should be made more generic or new fx created for the cheetah turret's
// projectile explosion effects.
//
// I've removed all effects that are not required for the current weapons.  On
// reflection I really went overboard when originally designing these effects!
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Sound
// ----------------------------------------------------------------------------

datablock SFXProfile(RocketLauncherExplosionSound)
{
   filename = "art/sound/weapons/Crossbow_explosion";
   description = AudioDefault3d;
   preload = true;
};

//----------------------------------------------------------------------------
// Debris
//----------------------------------------------------------------------------

datablock ParticleData(RocketDebrisTrailParticle)
{
   textureName = "art/particles/impact";
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
   animTexName = "art/particles/impact";
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
   shapeFile = "art/shapes/weapons/shared/rocket.dts";
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
   textureName = "art/particles/smoke";
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
   textureName = "art/particles/droplet";
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
   textureName = "art/particles/wake";
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
//    texture = "art/images/particles/splash";

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
   textureName = "art/particles/smoke";
   animTexName = "art/particles/smoke";
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
   textureName = "art/particles/fireball.png";
   lifetimeMS = "300";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-400";
   spinRandomMax = "0";
   animTexName = "art/particles/fireball.png";
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
   textureName = "art/particles/smoke";
   animTexName = "art/particles/smoke";
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
   textureName = "art/particles/droplet.png";
   lifetimeMS = "100";
   lifetimeVarianceMS = "50";
   animTexName = "art/particles/droplet.png";
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
   textureName = "art/particles/fireball.png";
   gravityCoefficient = "-0.202686";
   lifetimeMS = "400";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-200";
   spinRandomMax = "0";
   animTexName = "art/particles/fireball.png";
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
   textureName = "art/particles/smoke";
   gravityCoefficient = "-0.40293";
   lifetimeMS = "800";
   lifetimeVarianceMS = "299";
   spinSpeed = "1";
   spinRandomMin = "-200";
   spinRandomMax = "0";
   animTexName = "art/particles/smoke";
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
