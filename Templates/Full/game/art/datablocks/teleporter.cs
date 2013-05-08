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
// DefaultTrigger is used by the mission editor.  This is also an example
// of trigger methods and callbacks.

// A 3D sound played by the server at the location of the 
// teleporter after an object has teleported.
datablock SFXProfile(TeleportEntrance)  
{  
   fileName = "art/sound/orc_pain"; 
   description = AudioDefault3D;  
   preload = true;
}; 

// The 2D sound played by the client after a teleport.
datablock SFXProfile(TeleportSound)  
{  
   fileName = "art/sound/orc_death"; 
   description = Audio2D;  
   preload = true;  
};

datablock ParticleData(TeleporterFlash : DefaultParticle)
{
   dragCoefficient = "5";
   inheritedVelFactor = "0";
   constantAcceleration = "0";
   lifetimeMS = "500";
   spinRandomMin = "-90";
   spinRandomMax = "90";
   textureName = "art/particles/flare.png";
   animTexName = "art/particles/flare.png";
   colors[0] = "0.678431 0.686275 0.913726 0.207";
   colors[1] = "0 0.543307 1 0.759";
   colors[2] = "0.0472441 0.181102 0.92126 0.838";
   colors[3] = "0.141732 0.0393701 0.944882 0";
   sizes[0] = "0";
   sizes[1] = "0";
   sizes[2] = "4";
   sizes[3] = "0.1";
   times[1] = "0.166667";
   times[2] = "0.666667";
   lifetimeVarianceMS = "0";
   gravityCoefficient = "-9";
};

datablock ParticleEmitterData(TeleportFlash_Emitter : DefaultEmitter)
{
   ejectionVelocity = "0.1";
   particles = "TeleporterFlash";
   thetaMax = "180";
   softnessDistance = "1";
   ejectionOffset = "0.417";
};

// Particles to use for the emitter at the teleporter
datablock ParticleData(TeleporterParticles)
{
   lifetimeMS = "750";
   lifetimeVarianceMS = "100";
   textureName = "art/particles/Streak.png";
   useInvAlpha = "0";
   gravityCoefficient = "-1";
   spinSpeed = "0";
   spinRandomMin = "0";
   spinRandomMax = "0";
   colors[0]     = "0.0980392 0.788235 0.92549 1";
   colors[1]     = "0.0627451 0.478431 0.952941 1";
   colors[2]     = "0.0509804 0.690196 0.964706 1";
   sizes[0]      = "1";
   sizes[1]      = "1";
   sizes[2]      = "1";
   times[0]      = 0.0;
   times[1]      = "0.415686";
   times[2]      = "0.74902";
   animTexName = "art/particles/Streak.png";
   inheritedVelFactor = "0.0998043";
   constantAcceleration = "-2";
   colors[3] = "0.694118 0.843137 0.945098 0";
   sizes[3] = "1";
};

// Particle Emitter to be played when a teleport occours.
datablock ParticleEmitterData(TeleportEmitter)
{
   ejectionPeriodMS = "25";
   periodVarianceMS = "2";
   ejectionVelocity = "0.25";
   velocityVariance = "0.1";
   ejectionOffset   = "0.25";
   thetaMin         = "90";
   thetaMax         = "90";
   phiReferenceVel  = 0;
   phiVariance      = 360;
   overrideAdvance = false;
   lifetimeMS       = "1000";
   particles = "TeleporterParticles";
   blendStyle = "ADDITIVE";
};

// Ignore the name "explosion" for this. An Explosion in T3D
// is really an effect that plays a SFXProfile, particle emitters,
// point light, debris, and camera shake. Things normally associated with
// an explosion, such as damage and pushback, are calculated outside of the
// explosion object itself. Because of this, we use an ExplosionDatablock to 
// attach visual effects to our teleporter.
datablock ExplosionData(EntranceEffect)
{
   soundProfile = TeleportEntrance;

   particleEmitter = "TeleportEmitter";
   
   lifeTimeMS = "288";

   lightStartRadius = "0";
   lightEndRadius = "2.82353";
   lightStartColor = "0.992126 0.992126 0.992126 1";
   lightEndColor = "0 0.102362 0.992126 1";
   lightStartBrightness = "0.784314";
   lightEndBrightness = "4";
   lightNormalOffset = "0";
   emitter[0] = "RocketSplashEmitter";
   times[0] = "0.247059";
   particleRadius = "0.1";
   particleDensity = "10";
   playSpeed = "1";
};

datablock TriggerData(TeleporterTrigger : DefaultTrigger)
{
   // Amount of time, in milliseconds, to wait before allowing another
   // object to use this teleportat.
	teleporterCooldown = 0;
	
	// Amount to scale the object's exit velocity. Larger values will
	// propel the object with greater force.
	exitVelocityScale = 0;
	
	// If true, the object will be oriented to the front
	// of the exit teleporter. Otherwise the player will retain their original
	// orientation.
	reorientPlayer = true;
	
	// If true, the teleporter will only trigger if the object
	// enters the front of the teleporter.
	oneSided = false;
	
	// Effects to play at the entrance of the teleporter.
	entranceEffect = EntranceEffect;
	exiteffect = EntranceEffect;
	
	// 2D Sound to play for the client being teleported.
	teleportSound = TeleportSound;
	
};
