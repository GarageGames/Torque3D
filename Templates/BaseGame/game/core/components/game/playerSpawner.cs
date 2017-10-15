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

//registerComponent("PlayerSpawner", "Component", 
//			"Player Spawner", "Game", false, "When a client connects, it spawns a player object for them and attaches them to it");

function PlayerSpawner::onAdd(%this)
{
	%this.clientCount = 1;
	%this.friendlyName = "Player Spawner";
	%this.componentType = "Spawner";
	
	%this.addComponentField("GameObjectName", "The name of the game object we spawn for the players", "gameObject", "PlayerObject");
}

function PlayerSpawner::onClientConnect(%this, %client)
{
	%playerObj = spawnGameObject(%this.GameObjectName, false);
	
	if(!isObject(%playerObj))
		return;

	%playerObj.position = %this.owner.position;
	
	%playerObj.notify("onClientConnect", %client);
	
	switchControlObject(%client, %playerObj);
	switchCamera(%client, %playerObj);
	
	%client.player = %playerObj;
	%client.camera = %playerObj;
	
	%inventory = %playerObj.getComponent(InventoryController);
	
	if(isObject(%inventory))
	{
      for(%i=0; %i<5; %i++)
      {
         %arrow = spawnGameObject(ArrowProjectile, false);
         
         %inventory.addItem(%arrow);
      }
	}
	
	%playerObj.position = %this.owner.position;
	%playerObj.rotation = "0 0 0";
	
	%this.clientCount++;
}

function PlayerSpawner::onClientDisConnect(%this, %client)
{
	
}

function PlayerSpawner::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}