
//----------------------------------------------------------------------------------------------------------------------------------------------------
//JeepWreckage
//----------------------------------------------------------------------------------------------------------------------------------------------------
datablock RigidBodyData( JeepWreckageBody )
{	
   category = "RigidBody";
   shapeFile = "data/mesh/jeep/jeep.dts";
   shapeType = $ShapeType::Box;
   mass = 50;
   
   allowTransDecal = true;

   damageOn = true;

   minContactSpeed = 1.0;
   slidingThreshold = 0.1;
   collisionSoundsCount = 3;
   collisionSound[0] = boxFall0;
   collisionSound[1] = boxFall1;
   collisionSound[2] = boxFall2;

//   softImpactSound = SoftImpactSound;
//   hardImpactSound = HardImpactSound;
//   wheelImpactSound = WheelImpactSound;

    // Damage Emitters, Offsets, etc.
    damageEmitter[0]        = smokeEmitter;
    //damageEmitter[1]        = SevereDamageEmitter;
    //damageEmitter[2]        = OnDestroyedEmitter;
    damageEmitterOffset[0]  = "0.2 1.5 0.5";   // right/left forward/backward, up/down
    damageEmitterOffset[1]  = "0.0 0.0 0.0";   // right/left forward/backward, up/down
    damageEmitterOffset[2]  = "0.2 1.5 1.0";   // right/left forward/backward, up/down
    //damageLevelTolerance[0] = 0.3;
    //damageLevelTolerance[1] = 0.7;
    numDmgEmitterAreas      = 3;

    dustEmitter = LiftoffDustEmitter;

   splash = VehicleSplash;
   splashVelocity = 4.0;
   splashAngle = 67.0;
   splashFreqMod = 300.0;
   splashVelEpsilon = 0.60;
   bubbleEmitTime = 0.4;
   splashEmitter[0] = VehicleWakeEmitter;
   splashEmitter[1] = VehicleFoamEmitter;
   splashEmitter[2] = VehicleBubbleEmitter;
};

datablock RigidBodyData( JeepWreckageWheel )
{	
   category = "RigidBody";
   shapeFile =  "art/shapes/vehicles/jeep/jeepwheel.dts";
   shapeType = $ShapeType::Cylinder;
   mass = 2;
   rotAngles = "90 0 0 0";
   offset = "0 0 0 0";

   allowTransDecal = true;

   damageOn = true;

   slidingThreshold = 0.7;
   minContactSpeed = 1.0;
   collisionSoundsCount = 1;
   collisionSound[0] = wheelFall0;
   collisionSound[1] = wheelFall1;
   collisionSound[2] = wheelFall2;
   
   slideSoundsCount = 1;
   slideSound[0] = wheelFall0;

//   softImpactSound = SoftImpactSound;
//   hardImpactSound = HardImpactSound;
//   wheelImpactSound = WheelImpactSound;

    // Damage Emitters, Offsets, etc.
    damageEmitter[0]        = smokeEmitter;
    //damageEmitter[1]        = SevereDamageEmitter;
    //damageEmitter[2]        = OnDestroyedEmitter;
    damageEmitterOffset[0]  = "0.0 0.0 0.0";   // right/left forward/backward, up/down
    damageEmitterOffset[1]  = "0.0 0.0 0.5";   // right/left forward/backward, up/down
    damageEmitterOffset[2]  = "0.0 0.0 1.0";   // right/left forward/backward, up/down
    //damageLevelTolerance[0] = 0.3;
    //damageLevelTolerance[1] = 0.7;
    numDmgEmitterAreas      = 2;

    dustEmitter = LiftoffDustEmitter;

   splash = VehicleSplash;
   splashVelocity = 4.0;
   splashAngle = 67.0;
   splashFreqMod = 300.0;
   splashVelEpsilon = 0.60;
   bubbleEmitTime = 0.4;
   splashEmitter[0] = VehicleWakeEmitter;
   splashEmitter[1] = VehicleFoamEmitter;
   splashEmitter[2] = VehicleBubbleEmitter;
};
//-------------------------------------------
function JeepBodyWreckage::onAdd(%this, %obj)
{
   Parent::onAdd(%this, %obj);

   //%smoke.setTransform(%obj.position);
   // Delete the particle emitter as soon as it is done
   //schedule(%smoke.emitter.lifetimeMS + 10, 0, "deleteEmitterNode", %smoke);
}
