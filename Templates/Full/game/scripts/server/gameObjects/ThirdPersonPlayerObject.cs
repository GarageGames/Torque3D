function ThirdPersonPlayerObject::onAdd(%this)
{
   %this.turnRate = 0.3;

   %this.phys = %this.getComponent("PlayerControllerComponent");
   %this.collision = %this.getComponent("CollisionComponent");
   %this.cam = %this.getComponent("CameraComponent");
   %this.camArm = %this.getComponent("CameraOrbiterComponent");
   %this.animation = %this.getComponent("AnimationComponent");
   %this.stateMachine = %this.getComponent("StateMachineComponent");
   %this.mesh = %this.getComponent("MeshComponent");

   %this.stateMachine.forwardVector = 0;

   %this.crouch = false;
   
   %this.firstPerson = false;
   
   %this.crouchSpeedMod = 0.5;
   
   %this.aimOrbitDist = 1.5;
   %this.regularOrbitDist = 5;
   
   %this.regularOrbitMaxPitch = 70;
   %this.regularOrbitMinPitch = -10;
   
   %this.aimedMaxPitch = 90;
   %this.aimedMinPitch = -90;
}

function ThirdPersonPlayerObject::onRemove(%this)
{

}

function ThirdPersonPlayerObject::moveVectorEvent(%this)
{
    %moveVector = %this.getMoveVector();

    // forward of the camera on the x-z plane
    %cameraForward = %this.cam.getForwardVector();

    %cameraRight = %this.cam.getRightVector();

    %moveVec = VectorAdd(VectorScale(%cameraRight, %moveVector.x), VectorScale(%cameraForward, %moveVector.y));

   if(%this.aiming || %this.firstPerson)
   {
      %forMove = "0 0 0";
      
      if(%moveVector.x != 0)
      {
         %this.phys.inputVelocity.x = %moveVector.x * 10;
      }
      else
      {
         %this.phys.inputVelocity.x = 0;
      }

      if(%moveVector.y != 0)
      {

         %this.phys.inputVelocity.y = %moveVector.y * 10;
      }
      else
      {
         %this.phys.inputVelocity.y = 0;
      }
   }
   else
   {
      if(%moveVec.x == 0 && %moveVec.y == 0)
      {
         %this.phys.inputVelocity = "0 0 0";
         %this.stateMachine.forwardVector = 0;
      }
      else
      {
         %moveVec.z = 0;

         %curForVec = %this.getForwardVector();

         %newForVec = VectorLerp(%curForVec, %moveVec, %this.turnRate);

         %this.setForwardVector(%newForVec);
         
         %this.phys.inputVelocity.y = 10;

         %this.stateMachine.forwardVector = 1;
      }
   }
   
   if(%this.crouch)
      %this.phys.inputVelocity = VectorScale(%this.phys.inputVelocity, %this.crouchSpeedMod);
}

function ThirdPersonPlayerObject::moveYawEvent(%this)
{
   %moveRotation = %this.getMoveRotation();

    %camOrb = %this.getComponent("CameraOrbiterComponent");
    
    if(%this.aiming || %this.firstPerson)
    {
      %this.rotation.z += %moveRotation.z * 10;
    }

    %camOrb.rotation.z += %moveRotation.z * 10;
}

function ThirdPersonPlayerObject::movePitchEvent(%this)
{
   %moveRotation = %this.getMoveRotation();

    %camOrb = %this.getComponent("CameraOrbiterComponent");

    %camOrb.rotation.x += %moveRotation.x * 10;
}

function ThirdPersonPlayerObject::moveRollEvent(%this){}

function ThirdPersonPlayerObject::moveTriggerEvent(%this, %triggerNum, %triggerValue)
{
   if(%triggerNum == 3 && %triggerValue)
   {
      if(%triggerValue)
      {
        %this.firstPerson = !%this.firstPerson;
        
        if(%this.firstPerson)
        {
            %this.rotation.z = %this.cam.rotationOffset.z;
            %this.camArm.orbitDistance = 0;
            %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
            %this.camArm.minPitchAngle = %this.aimedMinPitch;
            
            %this.cam.positionOffset = "0 0 0";
            %this.cam.rotationOffset = "0 0 0";
        }
        else if(%this.aiming)
        {
            %this.camArm.orbitDistance = %this.aimOrbitDist;
            
            %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
            %this.camArm.minPitchAngle = %this.aimedMinPitch;
        }
        else
        {
            %this.camArm.orbitDistance = %this.regularOrbitDist;
            
            %this.camArm.maxPitchAngle = %this.regularOrbitMaxPitch;
            %this.camArm.minPitchAngle = %this.regularOrbitMinPitch;
        }
        
        commandToClient(localclientConnection, 'SetClientRenderShapeVisibility', 
            localclientConnection.getGhostID(%this.getComponent("MeshComponent")), !%this.firstPerson);
      }
   }
	else if(%triggerNum == 2 && %triggerValue == true)
	{
	   //get our best collision assuming up is 0 0 1
	   %collisionAngle = %this.collision.getBestCollisionAngle("0 0 1");
	   
	   if(%collisionAngle >= 80)
	   {
	      %surfaceNormal = %this.collision.getCollisionNormal(0);
	      %jumpVector = VectorScale(%surfaceNormal, 200);
	      echo("Jump surface Angle is at: " @ %surfaceNormal);
	      
	      %this.phys.applyImpulse(%this.position, %jumpVector);
	      %this.setForwardVector(%jumpVector);
	   }
      else
         %this.phys.applyImpulse(%this.position, "0 0 300");
	}
	else if(%triggerNum == 4)
	{
      %this.crouch = %triggerValue;
	}
	else if(%triggerNum == 1)
	{
	   %this.aiming = %triggerValue;  
	   
	   if(%this.aiming)
      {
         %this.rotation.z = %this.cam.rotationOffset.z;
         %this.camArm.orbitDistance = %this.aimOrbitDist;
         %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
         %this.camArm.minPitchAngle = %this.aimedMinPitch;
      }
      else
      {
         %this.camArm.orbitDistance = %this.regularOrbitDist;
         %this.camArm.maxPitchAngle = %this.regularOrbitMaxPitch;
         %this.camArm.minPitchAngle = %this.regularOrbitMinPitch;
      }
	}
}

function ThirdPersonPlayerObject::onCollisionEvent(%this, %colObject, %colNormal, %colPoint, %colMatID, %velocity)
{
   if(!%this.phys.isContacted())
    echo(%this @ " collided with " @ %colObject);
}

function ThirdPersonPlayerObject::processTick(%this)
{
   %moveVec = %this.getMoveVector();
   %bestFit = "";
   
   if(%this.crouch)
   {
      if(%moveVec.x != 0 || %moveVec.y != 0)
         %bestFit = "Crouch_Forward";
      else
         %bestFit = "Crouch_Root";
   }
   else
   {
      if(%moveVec.x != 0 || %moveVec.y != 0)
         %bestFit = "Run";
      else
         %bestFit = "Root";
   }
   
   if(%this.animation.getThreadAnimation(0) !$= %bestFit)
      %this.animation.playThread(0, %bestFit);
}

//Used for first person mode
function clientCmdSetClientRenderShapeVisibility(%id, %visiblilty)
{
   %localID = ServerConnection.resolveGhostID(%id); 
   %localID.enabled = %visiblilty;
}

function serverToClientObject( %serverObject )
{
   assert( isObject( LocalClientConnection ), "serverToClientObject() - No local client connection found!" );
   assert( isObject( ServerConnection ), "serverToClientObject() - No server connection found!" );      
         
   %ghostId = LocalClientConnection.getGhostId( %serverObject );
   if ( %ghostId == -1 )
      return 0;
                
   return ServerConnection.resolveGhostID( %ghostId );   
}