//jeep_explosion.cs


//initial jeep explosion, jeep loses wheels and is burned
//  we use the default vehicle explosion, but add wheels debris

// Debris
////////////
datablock ParticleData(jeepTireDebrisTrailParticle)
{
	dragCoefficient		= 3.0;
	windCoefficient		= 0.0;
	gravityCoefficient	= -0.5;
	inheritedVelFactor	= 0.0;
	constantAcceleration	= 0.0;
	lifetimeMS		= 500;
	lifetimeVarianceMS	= 150;
	spinSpeed		= 10.0;
	spinRandomMin		= -50.0;
	spinRandomMax		= 50.0;
	useInvAlpha		= true;
	animateTexture		= false;
	//framesPerSec		= 1;

	textureName		= "base/data/particles/cloud";
	//animTexName		= "~/data/particles/cloud";

	// Interpolation variables
	colors[0]	= "0.0 0.0 0.0 0.0";
	colors[1]	= "0.0 0.0 0.0 0.250";
   colors[2]	= "0.0 0.0 0.0 0.0";

	sizes[0]	= 1.50;
	sizes[1]	= 2.50;
   sizes[2]	= 3.50;

	times[0]	= 0.0;
	times[1]	= 0.1;
   times[2]	= 1.0;
};

datablock ParticleEmitterData(jeepTireDebrisTrailEmitter)
{
   ejectionPeriodMS = 90;
   periodVarianceMS = 0;
   ejectionVelocity = 0.0;
   velocityVariance = 0.0;
   ejectionOffset   = 1.0;
   thetaMin         = 0;
   thetaMax         = 0;
   phiReferenceVel  = 0;
   phiVariance      = 360;
   overrideAdvance = false;
   particles = "jeepTireDebrisTrailParticle";
};
datablock DebrisData(jeepTireDebris)
{
   emitters = "JeepTireDebrisTrailEmitter";

	shapeFile = "./jeepTire.dts";
	lifetime = 2.0;
	minSpinSpeed = -400.0;
	maxSpinSpeed = 200.0;
	elasticity = 0.5;
	friction = 0.2;
	numBounces = 3;
	staticOnMaxBounce = true;
	snapOnMaxBounce = false;
	fade = true;

	gravModifier = 2;
};


// Explosion
////////////
datablock ExplosionData(jeepExplosion : vehicleExplosion)
{
   debris = jeepTireDebris;
   debrisNum = 4;
   debrisNumVariance = 0;
   debrisPhiMin = 0;
   debrisPhiMax = 360;
   debrisThetaMin = 40;
   debrisThetaMax = 85;
   debrisVelocity = 14;
   debrisVelocityVariance = 3;
};


// Projectile - you can't spawn explosions on the server, so we use a short-lived projectile
/////////////////////////////////
datablock ProjectileData(jeepExplosionProjectile : vehicleExplosionProjectile)
{
   directDamage        = 0;
   radiusDamage        = 0;
   damageRadius        = 0;
   explosion           = jeepExplosion;

   directDamageType  = $DamageType::VehicleExplosion;
   radiusDamageType  = $DamageType::VehicleExplosion;

   explodeOnDeath		= 1;

   armingDelay         = 0;
   lifetime            = 0;

   uiName = "Jeep Explosion";
};