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

//registerComponent("FPSControls", "Component", "FPS Controls", "Input", false, "First Person Shooter-type controls");

function FPSControls::onAdd(%this)
{
   //
	%this.beginGroup("Keys");
		%this.addComponentField(forwardKey, "Key to bind to vertical thrust", keybind, "keyboard w");
		%this.addComponentField(backKey, "Key to bind to vertical thrust", keybind, "keyboard s");
		%this.addComponentField(leftKey, "Key to bind to horizontal thrust", keybind, "keyboard a");
		%this.addComponentField(rightKey, "Key to bind to horizontal thrust", keybind, "keyboard d");
		
		%this.addComponentField(jump, "Key to bind to horizontal thrust", keybind, "keyboard space");
	%this.endGroup();
	
	%this.beginGroup("Mouse");
		%this.addComponentField(pitchAxis, "Key to bind to horizontal thrust", keybind, "mouse yaxis");
		%this.addComponentField(yawAxis, "Key to bind to horizontal thrust", keybind, "mouse xaxis");
	%this.endGroup();
	
	%this.addComponentField(moveSpeed, "Horizontal thrust force", float, 300.0);
	%this.addComponentField(jumpStrength, "Vertical thrust force", float, 3.0);
   //

   %control = %this.owner.getComponent( ControlObjectComponent );
   if(!%control)
   	   return echo("SPECTATOR CONTROLS: No Control Object behavior!");
		
	//%this.Physics = %this.owner.getComponent( PlayerPhysicsComponent );
	
	//%this.Animation = %this.owner.getComponent( AnimationComponent );
	
	//%this.Camera = %this.owner.getComponent( MountedCameraComponent );
	
	//%this.Animation.playThread(0, "look");
	
	%this.setupControls(%control.getClientID());
}

function FPSControls::onRemove(%this)
{
   Parent::onBehaviorRemove(%this);
   
   commandToClient(%control.clientOwnerID, 'removeInput', %this.forwardKey);
   commandToClient(%control.clientOwnerID, 'removeInput', %this.backKey);
   commandToClient(%control.clientOwnerID, 'removeInput', %this.leftKey);
   commandToClient(%control.clientOwnerID, 'removeInput', %this.rightKey);
   
   commandToClient(%control.clientOwnerID, 'removeInput', %this.pitchAxis);
   commandToClient(%control.clientOwnerID, 'removeInput', %this.yawAxis);
}

function FPSControls::onBehaviorFieldUpdate(%this, %field)
{
   %controller = %this.owner.getBehavior( ControlObjectBehavior );
   commandToClient(%controller.clientOwnerID, 'updateInput', %this.getFieldValue(%field), %field);
}

function FPSControls::onClientConnect(%this, %client)
{
   %this.setupControls(%client);
}


function FPSControls::setupControls(%this, %client)
{
   %control = %this.owner.getComponent( ControlObjectComponent );
   if(!%control.isControlClient(%client))
   {
      echo("FPS CONTROLS: Client Did Not Match");
      return;
   }
   
   %inputCommand = "FPSControls";

   %test = %this.forwardKey;
   
   /*SetInput(%client, %this.forwardKey.x,  %this.forwardKey.y,  %inputCommand@"_forwardKey");
   SetInput(%client, %this.backKey.x,     %this.backKey.y,     %inputCommand@"_backKey");
   SetInput(%client, %this.leftKey.x,     %this.leftKey.y,     %inputCommand@"_leftKey");
   SetInput(%client, %this.rightKey.x,    %this.rightKey.y,    %inputCommand@"_rightKey");
   
   SetInput(%client, %this.jump.x,        %this.jump.y,        %inputCommand@"_jump");
      
   SetInput(%client, %this.pitchAxis.x,   %this.pitchAxis.y,   %inputCommand@"_pitchAxis");
   SetInput(%client, %this.yawAxis.x,     %this.yawAxis.y,     %inputCommand@"_yawAxis");*/

   SetInput(%client, "keyboard",  "w",  %inputCommand@"_forwardKey");
   SetInput(%client, "keyboard",  "s",     %inputCommand@"_backKey");
   SetInput(%client, "keyboard",  "a",     %inputCommand@"_leftKey");
   SetInput(%client, "keyboard",  "d",    %inputCommand@"_rightKey");
   
   SetInput(%client, "keyboard",  "space",        %inputCommand@"_jump");
      
   SetInput(%client, "mouse",   "yaxis",   %inputCommand@"_pitchAxis");
   SetInput(%client, "mouse",   "xaxis",     %inputCommand@"_yawAxis");

   SetInput(%client, "keyboard",   "f",     %inputCommand@"_flashlight");
 
}

function FPSControls::onMoveTrigger(%this, %triggerID)
{
   //check if our jump trigger was pressed!
   if(%triggerID == 2)
   {
      %this.owner.applyImpulse("0 0 0", "0 0 " @ %this.jumpStrength);
   }
}

function FPSControls::Update(%this)
{
	return;
	
	%moveVector = %this.owner.getMoveVector();
	%moveRotation = %this.owner.getMoveRotation();
	
	%this.Physics.moveVector = "0 0 0";

   if(%moveVector.x != 0)
   {
		%fv = VectorNormalize(%this.owner.getRightVector());
      
		%forMove = VectorScale(%fv, (%moveVector.x));// * (%this.moveSpeed * 0.032)));
		
   		//%this.Physics.velocity = VectorAdd(%this.Physics.velocity, %forMove);
		
		%this.Physics.moveVector = VectorAdd(%this.Physics.moveVector, %forMove);
	
		//if(%forMove > 0)
		//	%this.Animation.playThread(1, "run");
   }
	/*else
	{
		%fv = VectorNormalize(%this.owner.getRightVector());
      
		%forMove = VectorScale(%fv, (%moveVector.x * (%this.moveSpeed * 0.032)));
	
	    if(%forMove <= 0)
			%this.Animation.stopThread(1);
		
	}*/
   
   if(%moveVector.y != 0)
   {
      %fv = VectorNormalize(%this.owner.getForwardVector());
      
      %forMove = VectorScale(%fv, (%moveVector.y));// * (%this.moveSpeed * 0.032)));
	
	  //%this.Physics.velocity = VectorAdd(%this.Physics.velocity, %forMove);
	
	  %this.Physics.moveVector = VectorAdd(%this.Physics.moveVector, %forMove);
      
	//if(VectorLen(%this.Physics.velocity) < 2)
    //  		%this.Physics.velocity = VectorAdd(%this.Physics.velocity, %forMove);
   }
   
   /*if(%moveVector.z)
   {
      %fv = VectorNormalize(%this.owner.getUpVector());
      
      %forMove = VectorScale(%fv, (%moveVector.z * (%this.moveSpeed * 0.032)));
      
      %this.Physics.velocity = VectorAdd(%this.Physics.velocity, %forMove);
   }*/
   
   if(%moveRotation.x != 0)
   {
		%look = mRadToDeg(%moveRotation.x) / 180;
		
		//%this.Animation.setThreadPos(0, %look);
		
		%this.owner.getComponent( MountedCameraComponent ).rotationOffset.x += mRadToDeg(%moveRotation.x);
		
		//%this.Camera.rotationOffset.x += mRadToDeg(%moveRotation.x);
   }
   //   %this.owner.rotation.x += mRadToDeg(%moveRotation.x);
   
   if(%moveRotation.z != 0)
   {
	  %zrot = mRadToDeg(%moveRotation.z);
	  %this.owner.getComponent( MountedCameraComponent ).rotationOffset.z += %zrot;
      //%this.owner.rotation.z += %zrot;
   }
}

//
function FPSControls_forwardKey(%val)
{
   $mvForwardAction = %val;
}

function FPSControls_backKey(%val)
{
   $mvBackwardAction = %val;
}

function FPSControls_leftKey(%val)
{
   $mvLeftAction = %val;
}

function FPSControls_rightKey(%val)
{
   $mvRightAction = %val;
}

function FPSControls_yawAxis(%val)
{
   $mvYaw += getMouseAdjustAmount(%val);
}

function FPSControls_pitchAxis(%val)
{
   $mvPitch += getMouseAdjustAmount(%val);
}

function FPSControls_jump(%val)
{
   $mvTriggerCount2++;
}

function FPSControls_flashLight(%val)
{
   $mvTriggerCount3++;
}