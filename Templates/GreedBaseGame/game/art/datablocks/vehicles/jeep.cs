//-----------------------------------------------------------------------------
// Torque3D Game Engine 
// Copyright (C) DeepScratch Studios.
//-----------------------------------------------------------------------------

datablock WheeledVehicleTire(Rigid_JeepTire)
{
   shapeFile =  "art/shapes/vehicles/jeep/jeepwheel.dts";

   mass = 20;

   rollingResist = 0.2;

   staticFriction = 4.2;
   kineticFriction = 3.15;

   // Spring that generates lateral tire forces
   lateralForce = 18000;
   lateralDamping = 6000;
   lateralRelaxation = 1;

   // Spring that generates longitudinal tire forces
   longitudinalForce = 18000;
   longitudinalDamping = 4000;
   longitudinalRelaxation = 0.01;
};

datablock WheeledVehicleSpring(Rigid_JeepSpring)
{
   // Wheel suspension properties
   length = 0.6;             // Suspension travel
   force = 3000;              // Spring force
   damping = 4000;            // Spring damping
   antiSwayForce = 6;         // Lateral anti-sway force
};

/*
//----------------------------------------------------------------------------
datablock WheeledVehicleEngine(Rigid_JeepEngine)
 {
	// Default Engine info...
	MinRPM = 500.0;
	MaxRPM = 4500.0;
	
	throttleIdle = 0.1;
	
	numGears = 5;
	gearRatios[0] = 3.78;
	gearRatios[1] = 2.06;
	gearRatios[2] = 1.35;
	gearRatios[3] = 0.97;
	gearRatios[4] = 0.74;
	
	diffRatio = 3.39;
	reverseRatio = 3.60;
	
    shiftUpRPM = 2300.0;
	shiftDownRPM = 1100.0;

   minEngPitch = 0.2;
   maxEngPitch = 1.0;

	numTorqueLevels = 8;
	
	rpmValues[0] = 500.0;
	torqueLevel[0] = 70.0;
	
	rpmValues[1] = 1000.0;
	torqueLevel[1] = 100.0;
	
	rpmValues[2] = 1500.0;
	torqueLevel[2] = 130.0;
	
	rpmValues[3] = 2000.0;
	torqueLevel[3] = 150.0;

	rpmValues[4] = 2500.0;
	torqueLevel[4] = 140.0;
	
	rpmValues[5] = 3000.0;
	torqueLevel[5] = 125.0;
	
	rpmValues[6] = 3500.0;
	torqueLevel[6] = 120.0;
	
	rpmValues[7] = 4000.0;
	torqueLevel[7] = 105.0;
	
	rpmValues[8] = 4500.0;
	torqueLevel[8] = 60.0;
};
// gearbox not on yet
*/

//----------------------------------------------------------------------------
datablock WheeledVehicleData(Rigid_Jeep)
{
   category = "Vehicles";
   shapeFile = "data/mesh/jeep/jeep.dts";

   // Engine
   engineTorque = 1500;       // Engine power
   engineBrake = 1000;         // Braking when throttle is 0
   brakeTorque = 2500;        // When brakes are applied
   maxWheelSpeed = 25;        // Engine scale by current speed / max speed

   // Rigid Body
   mass = 200;
   massCenter = "0 -0.2 1";    // Center of mass for rigid body
   massBox = "0 0 0";         // Size of box used for moment of inertia,
   
   //chassis
	vehiclePartNodeName[0] = "Bone_root";
	vehiclePartSize[0] = "2 1.5 5";//up/down side/side front/back 
	vehiclePartMass[0] = 100;
	vehiclePartShape[0] = $ShapeType::Box;
	vehiclePartOffset[0] = "0.5 0 1.2";// front/back side/side up/down
	vehiclePartJointBreakable[0] = true;
	//vehiclePartJointBreakThreshold[0] = 0.1;
	vehiclePartJointLockThreshold[0] = 1.0;
	vehiclePartJointLockLimits[0] = "0 0 0";
	
   //Physics updates  
   // Dynamic fields accessed via script
   nameTag = "[-O-]";
   maxDismountSpeed = 10000;
   maxMountSpeed = 5;
   tireDB = "Rigid_JeepTire";
   springDB = "Rigid_JeepSpring";
   
   emap = 1;
   cubeReflectorDesc = MirrorCubeDesc;
   
   gyroForce = 0.01; // Puts force on vehicle to keep it upright

   hitForce = 0.5;

   gyroForceDampen = 0.01; //Dampens the force to not have too extreme movement
   leanForce = 0.01; //angular force on vehicle when turning

   directDamage = 50;
   radiusDamage  = 10;
   damageRadius  = 50;
   areaImpulse = 50;
 
    maxDamage = 750.0;
    destroyedLevel = 750.0; 

//   decalData   = CarWheelTreadMark;
   decalOffset = 0.92;

    // Headlights and BrakeLights
    headlightLeft   = 0;
    headlightRight  = 0;
    brakelightLeft  = 0;
    brakelightRight = 0;

   numMountPoints      = 10;

   actionMapForMountPoint[0] = "wheeledVehicleDriverMap";
   mountPose[0]              = "sitting";
   /*
   mountPointTransform[0]    = "0 0 0 0 0 1 0";

   actionMapForMountPoint[1] = "wheeledVehicleGunnerMap";
   mountPose[1]              = "Sitting";
   mountPointTransform[1]    = "0 0 0 0 0 1 0";

    // When more mounts are added they will use this map.
   actionMapForMountPoint[2] = "wheeledVehiclePassengerMap";
   mountPose[2]              = "Sitting";
   mountPointTransform[2]    = "0 0 0 0 0 1 0";

   actionMapForMountPoint[3] = "wheeledVehiclePassengerMap";
   mountPose[3]              = "Sitting";
   mountPointTransform[3]    = "0 0 0 0 0 1 0";
*/
   useEyePoint = false;

   //steering
   steeringRTC = 0.001;
   speedDependentRTC = true;
   
   maxSteeringAngle = 0.350;  // Maximum steering angle, should match animation
   steeringIncrements = 0.9;  // Change between 0.1(turn fast) and 0.9(turn slow). 
   
   engineExhaustEmitter = carExhaustEmitter;
   tireSplashEmitter = VehicleTireSplashEmitter;
   tireEmitter = dirtSkidEmitter; // All the tires use the same dust emitter
   //dustEmitter = LiftoffDustEmitter;

   // 3rd person camera settings
   cameraRoll = false;         // Roll the camera with the vehicle
   cameraMaxDist = 3;         // Far distance from vehicle
   cameraOffset = 2.5;        // Vertical offset from camera mount point
   cameraLag = 0.01;           // Velocity lag of camera
   cameraDecay = 1.25;        // Decay per sec. rate of velocity lag

   freeCamPitch = false; 
   minCamPitchAngle = -1.0;
   maxCamPitchAngle = 1.2;
   freeCamYaw = false; 
   minCamYawAngle = -2.0;
   maxCamYawAngle = 2.0;

   drag = 0.6;                // Drag coefficient
   bodyFriction = 0.6;
   bodyRestitution = 0.4;
   minImpactSpeed = 0.1;        // Impacts over this invoke the script callback
   softImpactSpeed = 5;       // Play SoftImpact Sound
   hardImpactSpeed = 15;      // Play HardImpact Sound

   integration = 5;//was 4           // Physics integration: TickSec/Rate

   collisionTol = 0.6;        // Collision distance tolerance
   contactTol = 0.1;          // Contact velocity tolerance

   downForce = 0.3;           // downward force as speed increases, to keep you on the ground, like a wing
   
   // Energy
   maxEnergy = 100;
   jetForce = 3000;
   minJetEnergy = 30;
   jetEnergyDrain = 2;

   // Sounds start
   enterVehicleSound = enterCarSound;
   exitVehicleSound  = exitCarSound;
   startEngineSound  = StartCarEngineSound;
   stopEngineSound   = StopCarEngineSound;
   exhaustSound      = cheetahEngine;
   gearUpSound       = carGearUpSound;
   gearDownSound     = carGearDownSound;
   reverseSound      = carReverseGearSound; 
   GearSound1st      = car1stGearSound;
   GearSound2nd      = car2ndGearSound;    
   GearSound3rd      = car3rdGearSound;
   GearSound4th      = car4thGearSound;       
   GearSound5th      = car5thGearSound;    
   GearSound6th      = car6thGearSound;
   GearSound7th      = car7thGearSound;
   GearSound8th      = car8thGearSound;
   GearSound9th      = car9thGearSound;
   GearSound10th     = car10thGearSound;
   engineSound       = jeepEngineSound;
   squealSound       = SkidTarmacSound;
   softImpactSound   = MediumImpact01Sound;
   hardImpactSound   = HeavyImpact01Sound;
   wheelImpactSound  = SkidRoadSound;
   // Sounds end

    // Damage Emitters, Offsets, etc.
    damageEmitter[0]        = SuperficialDamageEmitter;
    damageEmitter[1]        = SevereDamageEmitter;
    damageEmitter[2]        = OnDestroyedEmitter;
    damageEmitterOffset[0]  = "0.2 1.5 1.0";   // right/left forward/backward, up/down
    damageEmitterOffset[1]  = "0.0 0.0 0.5";   // right/left forward/backward, up/down
    damageEmitterOffset[2]  = "0.0 0.0 0.0";   // right/left forward/backward, up/down
    damageLevelTolerance[0] = 0.3;
    damageLevelTolerance[1] = 0.7;
    numDmgEmitterAreas      = 3;

    splash = VehicleSplash;
    splashVelocity = 4.0;
    splashAngle = 67.0;
    splashFreqMod = 300.0;
    splashVelEpsilon = 0.60;
    bubbleEmitTime = 0.4;
    splashEmitter[0] = VehicleWakeEmitter;
    splashEmitter[1] = VehicleFoamEmitter;
    splashEmitter[2] = VehicleBubbleEmitter;

    // Explosion
    explosion       = JeepMainExplosion;

    //For Heat seeking weapons
    minimumHeatSig = 60.0;
    heatReduceIncr = 0;
   
    // Make sure we don't render when
    // we are destroyed
    renderWhenDestroyed = false;
};

//-----------------------------------------------------------------------------

function Rigid_Jeep::onAdd(%this,%obj)
{
   Parent::onAdd(%this, %obj);
   %obj.mountable = true;

   %obj.dashLightMat = JeepDashMat;
   %obj.headLightMat = Jeep_lights;

   %obj.setEngine(Rigid_JeepEngine);
   %obj.setGear(0);   
   %obj.parkingBrake = true;
   %obj.autoTrans = true; 

   //%obj.playThread(0,"idle");

   // add some lights
   addLights(%this, %obj);
    
   // Setup the car with some tires & springs
   for (%i = %obj.getWheelCount() - 1; %i >= 0; %i--)
   {
      %obj.setWheelTire(%i, %this.tireDB);
      %obj.setWheelSpring(%i, %this.springDB);
      %obj.setWheelPowered(%i, false);
   }

   // Steer with the front tires
   %obj.setWheelSteering(0, 1);
   %obj.setWheelSteering(1, 1);

   // Only power the two rear wheels...4X4
   %obj.setWheelPowered(0,true);//front
   %obj.setWheelPowered(1,true);//front
   %obj.setWheelPowered(2,true);//rear
   %obj.setWheelPowered(3,true);//rear
   
   // mesh hiding
   %obj.setMeshHidden("driver", true);
      
   %obj.setMeshHidden("roof", false);//HBa
   %obj.setMeshHidden("roof_dam", true);

   %obj.setMeshHidden("chassis", false);////
   %obj.setMeshHidden("chassis_dam", true);
   
   %obj.setMeshHidden("hood", false);////
   %obj.setMeshHidden("hood_dam", true);

   %obj.setMeshHidden("front_left_body", false);//HBi
   %obj.setMeshHidden("front_left_body_dam", true);

   %obj.setMeshHidden("front_right_body", false);//HBj
   %obj.setMeshHidden("front_right_body_dam", true);
   
   %obj.setMeshHidden("rear_left_body", false);//HBk
   %obj.setMeshHidden("rear_left_body_dam", true);
   
   %obj.setMeshHidden("rear_right_body", false);//HBl
   %obj.setMeshHidden("rear_right_body_dam", true);
   
   %obj.setMeshHidden("front_body", false);//HBm
   %obj.setMeshHidden("front_body_dam", true);

   %obj.setMeshHidden("rear_body", false);//HBm
   %obj.setMeshHidden("rear_body_dam", true);
      
   %obj.setMeshHidden("front_window", false);////
   %obj.setMeshHidden("front_window_dam", true);
}
//-----------------------------------------------------------------------------
function Rigid_Jeep::onRemove(%this, %obj)
{
   Parent::onRemove(%this, %obj);
   echo("\c4VehicleData::onRemove("@ %this.getName() @", "@ %obj.getClassName() @")");

   // if there are passengers/driver, kick them out
   for(%i = 0; %i < %obj.getDataBlock().numMountPoints; %i++)
   {
      if (%obj.getMountNodeObject(%i))
      {
         %passenger = %obj.getMountNodeObject(%i);
         if( isObject(%passenger) && %passenger.isMemberOfClass("Player") )
            %passenger.getDataBlock().doDismount(%passenger, true);
      }
   }
}
//-----------------------------------------------------------------------------
function Rigid_Jeep::onCollision(%this,%obj,%col,%vec,%speed)
{

   %damageAmt = %obj.getDamageLevel();

   if (%damageAmt >= %this.destroyedLevel)
      return;

   // Try and pickup all items
   if (%col.getClassName() $= "Item")
   {
      %obj.pickup(%col, 1);
      return;
   }

    if (%client.canMount == 1)   
    {   
    %obj.mountObject(%col, 0);   
    }  

     // %directDamage = 10;
   if ( %speed >= 1 )   
   {   
       if(%col.getType() & $TypeMasks::PlayerObjectType)
        {
         %db = %col.getDataBlock();

          if(%db.getName() $= DefaultPlayerData)
          {
          %directDamage = 0;
   	    %force = VectorScale(%normal,100);
	       %force = VectorSub("0 0 0",%force);
	       %col.applyImpulse(%pos,%force);
          return;
         }
      else
         echo("npcPlayerData");
         //%col.damage(%obj,%pos,%this.directDamage*10, "");
         %directDamage = 0;
         %position = %col.getPosition();   
         %impulse = %this.areaImpulse;   
         %fwVec = %obj.getForwardVector();   
         %impulseVec = VectorSub(%col.getWorldBoxCenter(), %position);   
         %impulseVec = VectorNormalize(%impulseVec);   
         %impulseVec = VectorScale(%impulseVec, %impulse *10); //* %fwVec   
         %impulseVec = VectorScale(%impulseVec, 0.5);   
         %col.applyImpulse(%position, %impulseVec);
         //%col.setActionThread("death10");
       }

    if(%col.getType() & $TypeMasks::VehicleObjectType)
         {
          echo("VehicleObjectType");
          %directDamage = 10;
         %position = %col.getPosition();   
         %impulse = %this.areaImpulse;   
         %fwVec = %obj.getForwardVector();   
         %impulseVec = VectorSub(%col.getWorldBoxCenter(), %position);   
         %impulseVec = VectorNormalize(%impulseVec);   
         %impulseVec = VectorScale(%impulseVec, %impulse *10); //* %fwVec   
         %impulseVec = VectorScale(%impulseVec, 0.5);   
         %col.applyImpulse(%position, %impulseVec);
         }
         
    if(%col.getType() & $TypeMasks::TerrainObjectType)
         {
          %directDamage = 0;
   	    %force = VectorScale(%normal,0.1);
	       %force = VectorSub("0 0 0",%force);
	       %col.applyImpulse(%pos,%force);
	       echo("xxxxx-VEHICLE-COLLISION-WITH-TERRAIN-xxxxx");//WHY DOESN'T ECHO???
         }

    if(%col.getType() & $TypeMasks::PhysicsObjectType)
      {
       %db = %col.getDataBlock();

        if(%db.getClassName() $= GMK_RigidBody)
         {
          echo("RigidBodyData");
          %directDamage = 0;
          %force = VectorScale(%normal,100);
	       %force = VectorSub("0 0 0",%force);
	       %col.applyImpulse(%pos,%force);
         }

        if(%db.getClassName() $= PhysicsShapeData)
         {
          echo("PhysicsShapeData");
          %col.damage(%obj,%pos,%this.directDamage/10, "mp5Damage");
   	    %force = VectorScale(%normal,3);
	       %force = VectorSub("0 0 0",%force);
	       %col.applyImpulse(%pos,%force);
         }

        if(%db.getClassName() $= PxMultiActorData)
         {
          echo("PxMultiActorData");
          %directDamage = 0;
   	    %force = VectorScale(%normal,2);
	       %force = VectorSub("0 0 0",%force);
	       %col.applyImpulse(%pos,%force);
         }
      }
   } 
  
} 

//-------------------------------------------------------
function Rigid_Jeep::onImpact(%this, %obj, %col, %vec, %vecLen)
{
   //echo("\c4VehicleData::onImpact("@ %this.getName() @", "@ %obj.getClassName() @", "@ %col @", "@ %vec @", "@ %vecLen @")");
         
   if (%vecLen > %this.minImpactSpeed)
   %obj.damage(0, VectorAdd(%obj.getPosition(), %vec), %vecLen * %this.speedDamageScale * 10, "Impact");

   // associated "crash" sounds
   if (%vecLen > %this.hardImpactSpeed)
      %obj.playAudio(0, %this.hardImpactSound);
   else if (%vecLen > %this.softImpactSpeed)
      %obj.playAudio(0, %this.softImpactSound);
}

//---------------------------------------------------------
function Rigid_Jeep::damage(%this, %obj, %sourceObj, %position, %damage, %damageType, %hitbox)
{
   %obj.applyDamage(%damage); 
                     
    // VEHICLE SPARK EFFECT WHEN HIT
    %sparks = new ParticleEmitterNode()
    {
        position = %position;
        rotation = "1 0 0 0";
        scale = "1 1 1";
        dataBlock = "DamageFxNode";
        emitter = "BulletSparks";
        velocity = "1";
    };
    MissionCleanup.add(%sparks);
}
//-----------------------------------------------------------------------------
function Rigid_Jeep::onDamage(%this, %obj, %pos, %delta)
{
   %damageAmt = %obj.getDamageLevel();
   
   %location = %obj.getDamageLocation(%pos);
   %bodyPart = getWord(%location, 0);
   %quadrant = getWord(%location, 1);

   switch$ (bodyPart)
   {
      case "top": %damage = %damage*2;
      case "chassis": %damage = %damage;
      case "wheels":  %damage = %damage/2;
   }

  //initial light damage, scratch the paint
  if (%damageAmt >= %this.destroyedLevel/5)
   {
   if (isObject(%obj))
    {
     //jeep_body.diffuseMap[0] = "art/shapes/vehicles/jeep/jeep_body.dds";//before damage
     jeep_body.diffuseMap[0] = "art/shapes/vehicles/jeep/jeep_body_dam.dds";
     jeep_body.detailScale[0] = "3 3";
     jeep_body.detailNormalMapStrength[0] = "10";
     jeep_body.reload();
    }
   }

  //more damage, so swap out meshes.
  if (%damageAmt >= %this.destroyedLevel/3)
   {
  if (isObject(%obj))
   {     
   if(%bodyPart $= "top") 
    {
      if(%quadrant $= "left_front" ) //1
      {
        %obj.setMeshHidden("front_left_body", true);
        %obj.setMeshHidden("front_left_body_dam", false);
      }
      else if(%quadrant $= "right_front") //2
      {
        %obj.setMeshHidden("front_right_body", true);
        %obj.setMeshHidden("front_right_body_dam", false);
      }
      else if(%quadrant $= "left_back" ) //3
      {
        %obj.setMeshHidden("rear_left_body", true);
        %obj.setMeshHidden("rear_left_body_dam", false);
      }
      else if(%quadrant $= "right_back") //4
      {
        %obj.setMeshHidden("rear_right_body", true);
        %obj.setMeshHidden("rear_right_body_dam", false);
      }
      else if(%quadrant $= "left_middle" ) //5
      {
      }
      else if(%quadrant $= "right_middle") //6
      {
      }
      else if(%quadrant $= "middle_front" ) //7
      {
        %obj.setMeshHidden("hood", true);
        %obj.setMeshHidden("hood_dam", false);
        %obj.setMeshHidden("front_right_body", true);
        %obj.setMeshHidden("front_right_body_dam", false);
        %obj.setMeshHidden("front_left_body", true);
        %obj.setMeshHidden("front_left_body_dam", false);
        %obj.setMeshHidden("front_window", true);
        %obj.setMeshHidden("front_window_dam", false);
      }
      else if(%quadrant $= "middle_back") //8
      {
        %obj.setMeshHidden("hood", true);
        %obj.setMeshHidden("hood_dam", false);
        %obj.setMeshHidden("rear_left_body", true);
        %obj.setMeshHidden("rear_left_body_dam", false);
        %obj.setMeshHidden("rear_right_body", true);
        %obj.setMeshHidden("rear_right_body_dam", false);
      }
      else if(%quadrant $= "middle_middle") //9
      {
        %obj.setMeshHidden("front_window", true);
        %obj.setMeshHidden("front_window_dam", false);
      }
    }
   else if(%bodyPart $= "chassis") 
    {
      if(%quadrant $= "left_front" ) //1
      {
        %obj.setMeshHidden("front_left_body", true);
        %obj.setMeshHidden("front_left_body_dam", false);
      }
      else if(%quadrant $= "right_front") //2
      {
        %obj.setMeshHidden("front_right_body", true);
        %obj.setMeshHidden("front_right_body_dam", false);
      }
      else if(%quadrant $= "left_back" ) //3
      {
        %obj.setMeshHidden("rear_left_body", true);
        %obj.setMeshHidden("rear_left_body_dam", false);
      }
      else if(%quadrant $= "right_back") //4
      {
        %obj.setMeshHidden("rear_right_body", true);
        %obj.setMeshHidden("rear_right_body_dam", false);
      }
      else if(%quadrant $= "left_middle" ) //5
      {
      }
      else if(%quadrant $= "right_middle") //6
      {
      }
      else if(%quadrant $= "middle_front" ) //7
      {
        %obj.setMeshHidden("hood", true);
        %obj.setMeshHidden("hood_dam", false);
        %obj.setMeshHidden("front_right_body", true);
        %obj.setMeshHidden("front_right_body_dam", false);
        %obj.setMeshHidden("front_left_body", true);
        %obj.setMeshHidden("front_left_body_dam", false);
        %obj.setMeshHidden("front_window", true);
        %obj.setMeshHidden("front_window_dam", false);
      }
      else if(%quadrant $= "middle_back") //8
      {
        %obj.setMeshHidden("hood", true);
        %obj.setMeshHidden("hood_dam", false);
        %obj.setMeshHidden("rear_left_body", true);
        %obj.setMeshHidden("rear_left_body_dam", false);
        %obj.setMeshHidden("rear_right_body", true);
        %obj.setMeshHidden("rear_right_body_dam", false);
        %obj.setMeshHidden("rear_window", true);
        %obj.setMeshHidden("rear_window_dam", false);
      }
    }
  }
 }

  //recieved enough damage to lose control
  if (%damageAmt >= %this.destroyedLevel/2)
   {
   if (!%this.getFieldValue(isPuppet))
    {
     %this.blendToPuppet(%obj);
    }
   }
   
   //and a time to die
   if (%damageAmt >= %this.destroyedLevel)
   {
      // Cloak our main object while
      // our explosion and debris are active
      // make the object unmountable
      %obj.mountable = false;
      //%obj.setDamageState(Destroyed);
      //%obj.schedule(200, "delete");
      //spawnJeepWreck (%obj.position);
      
   %client = LocalClientConnection;
   %player = %client.player;
   %driver = %obj.getMountedObject(0);

   %obj.setMeshHidden("driver", true);
   %obj.setMeshHidden("helmet", true);
   %obj.setMeshHidden("visor", true);

   %obj.schedule(0, "applyImpulse", %obj, "-10 0 5");//push forward and upward

  if(%driver)
   {
    %driver.setAllMeshesHidden(false);  
    %driver.schedule(10, "unMount");
    %driver.schedule(20, "applyImpulse", %driver,"-10 0 5");//push forward and upward
    %driver.getDatablock().setFieldValue("isPuppet", true);
                
    LocalClientConnection.setControlObject(%driver); 
 
    //commandToServer('AttachCamToObject', %driver);
   
    //%driver.getDatablock().setFieldValue("isPuppet", false); 
    //%driver.setActionThread("root", true, true);
   }
  }
}
//-----------------------------------------------------------------------------
function Rigid_Jeep::onDestroyed(%data, %obj, %prevState, %position)
{
   // Loop through all of our mounted objects, and 
   // if any are players, unmount them to avoid control
   // object being lost and getting pink screen
   %totalMountedObjects = %obj.getMountedObjectCount();

   // If we don't have anything or anyone mounted then just delete
   // or destroy the vehicle
   if(%totalMountedObjects == 0)
   {
      // Schedule delete for parent afte 3 seconds
      %obj.schedule(300, "delete");
      return;
   }

   %driver = %obj.getMountNodeObject(0);
/*
   // Everything by convention that is mounted to the iav should be
   // either a player (an object) or mounted as an image like the cannon,
   // cage, machine gun, plow, etc.
   for(%objCount = %totalMountedObjects; %objCount > -1; %objCount--) 
   {		
		//get the object at index %objCount
		%mountedObject = %obj.getMountedObject(%objCount);

        // If we have a driver then release so that
        // that client's player is the control object
        // and not the vehicle.
        if(isObject(%driver))
			%driver.client.setControlObject(%driver);

        // set mountVehicle to false
		if (%mountedObject.mVehicle)
			%mountedObject.mountVehicle = false;

        // unmount our object
        %mountedObject.unMount();
        // schedule a time so we can remount shortly
        %mountedObject.schedule(600, "MountVehicles", true);
       
            // unhide our player crosshair
        //unhideCrossHair();

        //%obj.jeepRD.stopThread(0);

        // Player ejection
		// Position above dismount point
		%ejectpos = %mountedObject.getPosition();
		%ejectpos = VectorAdd(%ejectpos, "2 0 2");
		%mountedObject.setTransform(%ejectpos);

        // get the vehicles velocity
		%ejectvel = %mountedObject.mVehicle.getVelocity();
        // add this velocity to our hard coded velocity here
		%ejectvel = VectorAdd(%ejectvel, "0 0 10");
        // scale the vector by the mass of the player
		%ejectvel = VectorScale(%ejectvel, 100);
        // apply impulse to throw us out of vehicle
		%mountedObject.applyImpulse(%ejectpos, %ejectvel);
   }
*/
   // Schedule delete for parent after 3 seconds
   %obj.schedule(300, "delete");
}

// ---------------------------------------------------------------------------- 
function Rigid_Jeep::onRepaired(%this, %obj)
{

    %this.blendFromPuppet(%obj);
}

//----------------------------------------------------------------------------
function Rigid_Jeep::blendToPuppet(%this, %obj, %pos, %client, %damage)
{
   %this.isPuppet = true;
   %this.schedule(1000, %this.ownPuppet = false);
        
   %obj.setRepairRate(0.9);

   %client = LocalClientConnection;
   %player = %client.player;
   %driver = %obj.getMountedObject(0);
   
   %driver.setAllMeshesHidden(false);
      
   %obj.setMeshHidden("driver", true);

   %obj.schedule(0, "applyImpulse", %obj, "-10 0 5");//push forward and upward
   %driver.getDatablock().setFieldValue("isPuppet", true);
   %driver.schedule(0, "unMount");
   %driver.schedule(0, "applyImpulse", %driver,"10 0 7.5");//push forward and upward
             
   LocalClientConnection.setControlObject(%driver);
   //LocalClientConnection.camera.setTransform(%driver.eye);
   
   %this.schedule(5000, "recoverFromPuppet");    
}

//----------------------------------------------------------------------------
function Rigid_Jeep::blendFromPuppet(%this, %obj, %free)
{
   %client = LocalClientConnection;
   %player = %client.player;
   %driver = %obj.getMountedObject(0);
   
   %driver.getDatablock().setFieldValue("isPuppet", false);
 
   LocalClientConnection.setControlObject(%driver);    
   %driver.getEyeTransform();
   LocalClientConnection.camera.setTransform(%driver.getEyeTransform());
              
   //%driver.setAllMeshesHidden(true);
   %obj.setMeshHidden("driver", false);
            
    %this.ownPuppet = true;

   if (%obj.getMountedObject(0))
   {
     %this.ownPuppet = true;  
     %this.isPuppet  = true;
     %this.isPuppet  = false;
   }
   else
   {
     %this.ownPuppet = false;  
     %this.isPuppet  = true;
   }

    //%this.schedule(10, "recoverFromPuppet");
    //%obj.schedule(10, "mountObject", %player, 0);
}

// ---------------------------------------------------------------------------- 
function Rigid_Jeep::recoverFromPuppet(%this, %obj)
{
   %client = LocalClientConnection;
   %player = %client.player;
   
   //%obj.schedule(10, "mountObject", %player, 0);
   
   %player.getDatablock().setFieldValue("isPuppet", false); 
   %player.setActionThread("root", true, true);
       
    //%this.isPuppet = false;
    //%this.ownPuppet = true;
}

// ---------------------------------------------------------------------------- 
function Rigid_Jeep::makePuppet(%obj)
{
    %this.isPuppet = true;
    //%this.ownPuppet = false;
}

//-----------------------------------------------------------------------------
function Rigid_Jeep::hitSomething(%this,%obj,%col,%vec,%speed,%hitwhat)
{
  // classic speed-scaled-damage code instead...
  if(%speed > %this.minImpactSpeed && %speed > %this.minSpeedDamage)
     %obj.damage(%col, VectorAdd(%vec, %obj.getPosition()),
         (%speed-%this.minSpeedDamage) * %this.speedDamageScale, $DamageType::Ground);
}

function Rigid_Jeep::onEnterLiquid(%this, %obj, %coverage, %type)
{
      TurbulenceFx.enable();
//   $Drowning::Schedule = schedule($UnderwaterInitTime, 0, "checkUnderwater", %obj );
   %obj.lastWeapon = %obj.getMountedImage($WeaponSlot);
   %obj.unmountImage($WeaponSlot);
}

function Rigid_Jeep::onLeaveLiquid(%this, %obj, %type)
{
   //WaterDropPostFx.enable();
   TurbulenceFx.enable();

   //cancel($Drowning::Schedule);
   //%obj.clearDamageDt();

 %obj.mountImage(%obj.lastWeapon, $WeaponSlot);
}

function spawnJeepWreck(%pos, %obj)
{

}

