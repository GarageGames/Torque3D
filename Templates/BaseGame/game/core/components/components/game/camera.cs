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

function CameraComponent::onAdd(%this) 
{
   %this.addComponentField(clientOwner, "The client that views this camera", "int", "1", "");

   %test = %this.clientOwner;

   %barf = ClientGroup.getCount();

   %clientID = %this.getClientID();
   if(%clientID && !isObject(%clientID.camera))
   {
      %this.scopeToClient(%clientID);
      %this.setDirty();

      %clientID.setCameraObject(%this.owner);
      %clientID.setControlCameraFov(%this.FOV);
      
      %clientID.camera = %this.owner;
   }

   %res = $pref::Video::mode;
   %derp = 0;
}

function CameraComponent::onRemove(%this)
{
   %clientID = %this.getClientID();
   if(%clientID)
      %clientID.clearCameraObject();
}

function CameraComponent::onInspectorUpdate(%this)
{
   //if(%this.clientOwner)
      //%this.clientOwner.setCameraObject(%this.owner);
}

function CameraComponent::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}

function CameraComponent::isClientCamera(%this, %client)
{
	%clientID = ClientGroup.getObject(%this.clientOwner-1);
	
	if(%client.getID() == %clientID)
		return true;
	else
	    return false;
}

function CameraComponent::onClientConnect(%this, %client)
{
   //if(%this.isClientCamera(%client) && !isObject(%client.camera))
   //{
      %this.scopeToClient(%client);
      %this.setDirty();
      
      %client.setCameraObject(%this.owner);
      %client.setControlCameraFov(%this.FOV);
      
      %client.camera = %this.owner;
   //}
   //else
   //{
   //   echo("CONNECTED CLIENT IS NOT CAMERA OWNER!");
   //}
}

function CameraComponent::onClientDisconnect(%this, %client)
{
   Parent::onClientDisconnect(%this, %client);
   
   if(isClientCamera(%client)){
      %this.clearScopeToClient(%client);
      %client.clearCameraObject();
   }
}

///
///
///

function VRCameraComponent::onAdd(%this) 
{
   %this.addComponentField(clientOwner, "The client that views this camera", "int", "1", "");

   %test = %this.clientOwner;

   %barf = ClientGroup.getCount();

   %clientID = %this.getClientID();
   if(%clientID && !isObject(%clientID.camera))
   {
      %this.scopeToClient(%clientID);
      %this.setDirty();

      %clientID.setCameraObject(%this.owner);
      %clientID.setControlCameraFov(%this.FOV);
      
      %clientID.camera = %this.owner;
   }

   %res = $pref::Video::mode;
   %derp = 0;
}

function VRCameraComponent::onRemove(%this)
{
   %clientID = %this.getClientID();
   if(%clientID)
      %clientID.clearCameraObject();
}

function CameraComponent::onInspectorUpdate(%this)
{
   //if(%this.clientOwner)
      //%this.clientOwner.setCameraObject(%this.owner);
}

function VRCameraComponent::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}

function VRCameraComponent::isClientCamera(%this, %client)
{
	%clientID = ClientGroup.getObject(%this.clientOwner-1);
	
	if(%client.getID() == %clientID)
		return true;
	else
	    return false;
}

function VRCameraComponent::onClientConnect(%this, %client)
{
   //if(%this.isClientCamera(%client) && !isObject(%client.camera))
   //{
      %this.scopeToClient(%client);
      %this.setDirty();
      
      %client.setCameraObject(%this.owner);
      %client.setControlCameraFov(%this.FOV);
      
      %client.camera = %this.owner;
   //}
   //else
   //{
   //   echo("CONNECTED CLIENT IS NOT CAMERA OWNER!");
   //}
}

function VRCameraComponent::onClientDisconnect(%this, %client)
{
   Parent::onClientDisconnect(%this, %client);
   
   if(isClientCamera(%client)){
      %this.clearScopeToClient(%client);
      %client.clearCameraObject();
   }
}