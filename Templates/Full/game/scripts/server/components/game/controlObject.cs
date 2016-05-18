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

//registerComponent("ControlObjectComponent", "Component", "Control Object", "Game", false, "Allows the behavior owner to operate as a camera.");

function ControlObjectComponent::onAdd(%this)
{
   %this.addComponentField(clientOwner, "The shape to use for rendering", "int", "1", "");

   %clientID = %this.getClientID();

   if(%clientID && !isObject(%clientID.getControlObject()))
      %clientID.setControlObject(%this.owner);
}

function ControlObjectComponent::onRemove(%this)
{
   %clientID = %this.getClientID();
	
   if(%clientID)
      %clientID.setControlObject(0);
}

function ControlObjectComponent::onClientConnect(%this, %client)
{
   if(%this.isControlClient(%client) && !isObject(%client.getControlObject()))
      %client.setControlObject(%this.owner);
}

function ControlObjectComponent::onClientDisconnect(%this, %client)
{
   if(%this.isControlClient(%client))
      %client.setControlObject(0);
}

function ControlObjectComponent::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}

function ControlObjectComponent::isControlClient(%this, %client)
{
	%clientID = ClientGroup.getObject(%this.clientOwner-1);
	
	if(%client.getID() == %clientID)
		return true;
	else
	    return false;
}

function ControlObjectComponent::onInspectorUpdate(%this, %field)
{
   %clientID = %this.getClientID();
	
   if(%clientID && !isObject(%clientID.getControlObject()))
      %clientID.setControlObject(%this.owner);
}

function switchControlObject(%client, %newControlEntity)
{
	if(!isObject(%client) || !isObject(%newControlEntity))
		return error("SwitchControlObject: No client or target controller!");
		
	%control = %newControlEntity.getComponent(ControlObjectComponent);
		
	if(!isObject(%control))
		return error("SwitchControlObject: Target controller has no conrol object behavior!");
		
    %client.setControlObject(%newControlEntity);
}