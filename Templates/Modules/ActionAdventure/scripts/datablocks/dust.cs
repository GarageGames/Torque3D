datablock ParticleData(dust_cloud)
{
   textureName = "data/ActionAdventure/art/particles/dust.png";
   lifetimeMS = 8000;
   lifetimeVaranceMS = 100;
   colors[0] = "1 1 1 0";
   colors[1] = "1 1 1 .5";
   colors[2] = "1 1 1 .5";
   colors[3] = "1 1 1 0";
   sizes[0] = 30.0;
   sizes[1] = 33.0;
   sizes[2] = 36.0;
   sizes[3] = 40.0;
   times[0] = 0.0;
   times[1] = 0.3;
   times[2] = 0.6;
   times[3] = 1;
   spinSpeed = 1;
   spinRandomMin = -5;
   spinRandomMax = 5;
   gravityCoefficient = 0;
   constantAcceleration = 0;
   dragCoefficient = 0;
   useInvAlpha = true;
};

datablock ParticleEmitterData(dust_cloud_emitter)
{
   particles = dust_cloud;
   ejectionPeriodMS = 2047;
   periodVarianceMS = 0;
   ejectionVelocity = 3;
   velocityVariance = 1;
   thetaMax = 90;
   thetaMin = 90;
   phiReferenceVal = 90;
   phiVariance = 90;
   lifeTimeMS = 0;
};

datablock ParticleEmitterNodeData(dust_cloud_source)
{
   timeMultiple = 1;
};