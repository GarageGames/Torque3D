//-----------------------------------------------------------------------------
// new jeep from a cheetah (for debuguing sake)
//-----------------------------------------------------------------------------

datablock SFXProfile(rjeepEngine)
{
   preload = "1";
   description = "AudioCloseLoop3D";
   fileName = "art/sound/cheetah/cheetah_engine.ogg";
};

datablock SFXProfile(rjeepSqueal)
{
   preload = "1";
   description = "AudioDefault3D";
   fileName = "art/sound/cheetah/cheetah_squeal.ogg";
};

datablock SFXProfile(hardImpact)
{
   preload = "1";
   description = "AudioDefault3D";
   fileName = "art/sound/cheetah/hardImpact.ogg";
};

datablock SFXProfile(softImpact)
{
   preload = "1";
   description = "AudioDefault3D";
   fileName = "art/sound/cheetah/softImpact.ogg";
};

datablock SFXProfile(DirtKickup)
{
   preload = "1";
   description = "AudioDefault3D";
   fileName = "art/sound/cheetah/softImpact.ogg";
};

datablock ParticleData(RjeepTireParticle)
{
   textureName          = "data/materials/particles/dustParticle";
   dragCoefficient      = "1.99902";
   gravityCoefficient   = "-0.100122";
   inheritedVelFactor   = "0.0998043";
   constantAcceleration = 0.0;
   lifetimeMS           = 1000;
   lifetimeVarianceMS   = 400;
   colors[0]            = "0.456693 0.354331 0.259843 1";
   colors[1]            = "0.456693 0.456693 0.354331 0";
   sizes[0]             = "0.997986";
   sizes[1]             = "3.99805";
   sizes[2]             = "1.0";
   sizes[3]             = "1.0";
   times[0]             = "0.0";
   times[1]             = "1";
   times[2]             = "1";
   times[3]             = "1";
};

datablock ParticleEmitterData(RjeepTireEmitter)
{
   ejectionPeriodMS = 20;
   periodVarianceMS = 10;
   ejectionVelocity = "14.57";
   velocityVariance = 1.0;
   ejectionOffset   = 0.0;
   thetaMin         = 0;
   thetaMax         = 60;
   phiReferenceVel  = 0;
   phiVariance      = 360;
   overrideAdvance = false;
   particles = "RjeepTireParticle";
   blendStyle = "ADDITIVE";
};

//-----------------------------------------------------------------------------
// Information extacted from the shape.
//
// Wheel Sequences
//    spring#        Wheel spring motion: time 0 = wheel fully extended,
//                   the hub must be displaced, but not directly animated
//                   as it will be rotated in code.
// Other Sequences
//    steering       Wheel steering: time 0 = full right, 0.5 = center
//    breakLight     Break light, time 0 = off, 1 = breaking
//
// Wheel Nodes
//    hub#           Wheel hub, the hub must be in it's upper position
//                   from which the springs are mounted.
//
// The steering and animation sequences are optional.
// The center of the shape acts as the center of mass for the car.

//-----------------------------------------------------------------------------
datablock WheeledVehicleTire(RjeepCarTire)
{
   // Tires act as springs and generate lateral and longitudinal
   // forces to move the vehicle. These distortion/spring forces
   // are what convert wheel angular velocity into forces that
   // act on the rigid body.
   shapeFile = "art/shapes/vehicles/jeepwheel.dts";
   staticFriction = 4.2;
   kineticFriction = "3.15";//"1";

   // Spring that generates lateral tire forces
   lateralForce = 18000;
   lateralDamping = 6000;
   lateralRelaxation = 1;

   // Spring that generates longitudinal tire forces
   longitudinalForce = 18000;
   longitudinalDamping = 4000;
   longitudinalRelaxation = 1;
   radius = "0.609998";
};

datablock WheeledVehicleTire(RjeepCarTireRear)
{
   // Tires act as springs and generate lateral and longitudinal
   // forces to move the vehicle. These distortion/spring forces
   // are what convert wheel angular velocity into forces that
   // act on the rigid body.
   shapeFile = "art/shapes/Cheetah/wheelBack.DAE";
   staticFriction = "7.2";
   kineticFriction = "1";

   // Spring that generates lateral tire forces
   lateralForce = "19000";
   lateralDamping = 6000;
   lateralRelaxation = 1;

   // Spring that generates longitudinal tire forces
   longitudinalForce = 18000;
   longitudinalDamping = 4000;
   longitudinalRelaxation = 1;
   radius = "0.840293";
};

datablock WheeledVehicleSpring(RjeepCarSpring)
{
   // Wheel suspension properties
   length = 0.5;             // Suspension travel
   force = 2800;              // Spring force
   damping = 3600;             // Spring damping
   antiSwayForce = 3;         // Lateral anti-sway force
};

datablock WheeledVehicleData(RjeepCar)
{
   category = "Vehicles";
   shapeFile = "art/shapes/Cheetah/Cheetah_Body.DAE";
   emap = 1;

   mountPose[0] = sitting;
   numMountPoints = 5; // 6

   useEyePoint = true;  // Use the vehicle's camera node rather than the player's

   maxSteeringAngle = 0.585;  // Maximum steering angle, should match animation

   // 3rd person camera settings
   cameraRoll = false;        // Roll the camera with the vehicle
   cameraMaxDist = 7.8;       // Far distance from vehicle
   cameraOffset = 1.0;        // Vertical offset from camera mount point
   cameraLag = "0.3";           // Velocity lag of camera
   cameraDecay = 1.25;        // Decay per sec. rate of velocity lag

   // Rigid Body
   mass = "400";
   massCenter = "0 0.5 0";    // Center of mass for rigid body
   massBox = "0 0 0";         // Size of box used for moment of inertia,
                              // if zero it defaults to object bounding box
   drag = 0.6;                // Drag coefficient
   bodyFriction = 0.6;
   bodyRestitution = 0.4;
   minImpactSpeed = 5;        // Impacts over this invoke the script callback
   softImpactSpeed = 5;       // Play SoftImpact Sound
   hardImpactSpeed = 15;      // Play HardImpact Sound
   integration = 8;           // Physics integration: TickSec/Rate
   collisionTol = "0.1";        // Collision distance tolerance
   contactTol = "0.4";          // Contact velocity tolerance

   // Engine
   engineTorque = 4300;       // Engine power
   engineBrake = "5000";         // Braking when throttle is 0
   brakeTorque = "10000";        // When brakes are applied
   maxWheelSpeed = 50;        // Engine scale by current speed / max speed

   // Energy
   maxEnergy = 100;
   jetForce = 3000;
   minJetEnergy = 30;
   jetEnergyDrain = 2;

   // Sounds
   engineSound = cheetahEngine;
   //squealSound = cheetahSqueal;
   softImpactSound = softImpact;
   hardImpactSound = hardImpact;

   // Dynamic fields accessed via script
   nameTag = 'Rjeep';
   maxDismountSpeed = 10;
   maxMountSpeed = 5;
   mountPose0 = "sitting";
   tireEmitter = "RjeepTireEmitter";
   dustEmitter = "RjeepTireEmitter";
   dustHeight = "1";

   // Mount slots
   turretSlot = 1;
   rightBrakeSlot = 2;
   leftBrakeSlot = 3;
};
