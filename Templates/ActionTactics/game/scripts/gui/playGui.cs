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
// PlayGui is the main TSControl through which the game is viewed.
// The PlayGui also contains the hud controls.
//-----------------------------------------------------------------------------

function PlayGui::onWake(%this)
{
   // Turn off any shell sounds...
   // sfxStop( ... );
   
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	echo("\c4client ID = " @ %client);

   $enableDirectInput = "1";
   activateDirectInput();

   // Message hud dialog
   if ( isObject( MainChatHud ) )
   {
      Canvas.pushDialog( MainChatHud );
      chatHud.attach(HudMessageVector);
   }      
   
   // just update the action map here
   moveMap.push();

   // hack city - these controls are floating around and need to be clamped
   if ( isFunction( "refreshCenterTextCtrl" ) )
      schedule(0, 0, "refreshCenterTextCtrl");
   if ( isFunction( "refreshBottomTextCtrl" ) )
      schedule(0, 0, "refreshBottomTextCtrl");
      
   //ActionTactics - reset hud
   commandToClient(%client, 'ResetHUD', %client, true);
}

function PlayGui::onSleep(%this)
{
   if ( isObject( MainChatHud ) )
      Canvas.popDialog( MainChatHud );
   
   // pop the keymaps
   moveMap.pop();
}

function PlayGui::clearHud( %this )
{
   Canvas.popDialog( MainChatHud );

   while ( %this.getCount() > 0 )
      %this.getObject( 0 ).delete();
}

//-----------------------------------------------------------------------------

function refreshBottomTextCtrl()
{
   BottomPrintText.position = "0 0";
}

function refreshCenterTextCtrl()
{
   CenterPrintText.position = "0 0";
}

// -----------------------------------------------------------------------------
// ActionTactics
// -----------------------------------------------------------------------------

function PlayGui::onMouseDown(%this, %pos, %start, %ray)
{
   //do not allow player interference during enemy active phase
   if(TurnManager.enemyPhase == 1)
      return;
      
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	echo("\c4client ID = " @ %client);

	//actions
	//0 = no action
	//1 = move action
	//2 = shoot action

	// Get access to the AI player we control
	%ai = %client.player;
	
	//if they are already moving, 
	//interrupt and stop them, 
	//also remove any marker they may have
	//and abort this action sequence as the Client may want to give new orders
	
	if(%ai.getVelocity() !$="0 0 0")
	{
		if( %ai.decal > -1 )
			decalManagerRemoveDecal( %ai.decal );
			
		%ai.stop();
		return;
	}

	echo("ai has action = " @ %ai.action);
	
	if(%ai.action == 0)
		return;
		
	//find end of search vector from the cursors point on screen
	// make it no more 100 units from the camera
	//anymore and have it miss
	%ray = VectorScale(%ray, 100);
	%end = VectorAdd(%start, %ray);
 
	if(%ai.action == 1)//1 is move
	{
		//we can only move if we have energy!
		if(%ai.getEnergyLevel() < 1)
		{
			messageClient(%client, 'MsgPlayer', '\c0%1 - Cannot Move - Out Of Energy!', %ai.getName());
			return;
		}
		
		// only care about terrain objects for now
		%searchMasks = $TypeMasks::TerrainObjectType;

		// search!
		%scanTarg = ContainerRayCast( %start, %end, %searchMasks );
   
		// If the terrain object was found in the scan
		if( %scanTarg )
		{
			// Get the X,Y,Z position of where we clicked
			%pos = getWords(%scanTarg, 1, 3);
      
			// Get the normal of the location we clicked on
			%norm = getWords(%scanTarg, 4, 6);
			
			//if we are not moving already, start to deplete energy/action points
			if(%ai.getVelocity() $="0 0 0")
				%ai.schedule(500, "decEnergy");

			// Set the destination for the AI player
			%ai.setMoveDestination( %pos );
			
			//clear his aim so he looks where he is running
			%ai.clearAim();
		
			// If the AI player already has a decal (0 or greater)
			// tell the decal manager to delete the instance of the torque_decal
			if( %ai.decal > -1 )
			{
				decalManagerRemoveDecal( %ai.decal );
		
				if(%ai.getVelocity() !$="0 0 0")
				{
					%ai.stop();
					return;
				}
			}

			// Create a new decal using the decal manager
			// arguments are (Position, Normal, Rotation, Scale, Datablock, Permanent)
			// AddDecal will return an ID of the new decal, which we will
			// store in the player
			%ai.decal = decalManagerAddDecal( %pos, %norm, 0, 1, "torque_decal", true );
		}
		else
		{
			//raycast was too far, more than 100 units from the camera
			messageClient(%client, 'MsgPlayer', '\c0%1 - Location Out Of Range!', %ai.getName());
		}
   
	}
	else//action 2 is shoot
	{
		if(%ai.hasFired == true)
		{
			// Can only shoot once per turn
			messageClient(%client, 'MsgPlayer', '\c0%1 Has Already Fired This Turn!', %ai.getName());
			return;
		}
   
		%searchMasks = $TypeMasks::PlayerObjectType | $TypeMasks::TerrainObjectType | $TypeMasks::StaticObjectType |  $TypeMasks::StaticShapeObjectType;
   
		// Search!
		%scanTarg = ContainerRayCast( %start, %end, %searchMasks, %ai );
   
		if( %scanTarg )
		{
			// Get the enemy ID
			%target = firstWord(%scanTarg);
			%xyz = restWords(%scanTarg);
			
			//aim at the location we hit
			%ai.setAimLocation(%xyz);
				
			//pause for 200m/s to give time to aim and then shoot
			%ai.schedule(200, "OpenFire", %xyz);
		}
		else
		{
			//aimpoint is more than 100 units abort
			messageClient(%client, 'MsgPlayer', '\c0%1 - AimPoint Out Of Range!', %ai.getName());
			
			//clear aim and if he's shooting stop it
			%ai.clearAim();
			%ai.setImageTrigger(0, 0);
		}
   }
}

function PlayGui::onRightMouseDown(%this, %pos, %start, %ray)
{   
   if(Canvas.isCursorOn())
      hideCursor();
   else
      showCursor();
}

function tacticsMove::onAction(%this)
{
//move has been pressed
//if move's pressed variable is not active, make it so now ->SetActionMove
//if shoot is on, turn it off
//if move is already pressed the client must be toggling it off by pressing it again -> clearActions

	if(tacticsMove.pressed == 0)
		commandToServer('SetActionMove');
	else
		commandToServer('ClearActions');
}

function tacticsShoot::onAction(%this)
{
//shoot has been pressed
//if shoot's pressed variable is not active, make it so now ->SetActionShoot
//if move is on, turn it off
//if shoot is already pressed the client must be toggling it off by pressing it again -> clearActions

	if(tacticsShoot.pressed == 0)
		commandToServer('SetActionShoot');
	else
		commandToServer('ClearActions');
}

function EndTurn::onAction(%this)
{
	commandToServer('TurnOver');
}

function tacticsTeam::onAction(%this)
{
   // As this is single player only it will be the first and only object in ClientGroup
   %client = ClientGroup.getObject(0);
	echo("\c4client ID = " @ %client);
	
	%bot = %client.player;
	
	if(isObject(%bot))
	{
		if(%bot.getVelocity() !$="0 0 0")//this is a safety feature to make so that they stop
		{
			%bot.stop();		
			if( %bot.decal > -1 )
				decalManagerRemoveDecal( %bot.decal );
		}
	}

	%num = $PlayerTeam.count();
	echo("number of team members = " @ %num);
	
	if(%num > 1)
	{
		//reset that playerObject's action to zero when we control another
		%bot.action = 0;

		echo($PlayerTeam.getCurrent());
	
		if($PlayerTeam.getCurrent() == %num -1 || $PlayerTeam.getCurrent() == %num)
			%id = $PlayerTeam.getKey($PlayerTeam.moveFirst());
		else
			%id = $PlayerTeam.getKey($PlayerTeam.moveNext());

		//change the camera FIRST!
		commandToServer('tacticsCam');
		%client.player = %id;
	}
	else
	{
		if(%num == 1)
		{
			%id = $PlayerTeam.getKey($PlayerTeam.moveFirst());	
			
			if(%client.player != %id)
			{
			//change the camera FIRST!
				commandToServer('tacticsCam');
				%client.player = %id;
			}
			else
				echo("No more Ally players to swap control to!");
		}
		else
		{
			echo("No Players left!");
		}
	}
	
	commandToClient(%client, 'ResetHud', %client, true);
}