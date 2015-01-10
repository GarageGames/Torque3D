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
// This script function is called before a client connection
// is accepted.  Returning "" will accept the connection,
// anything else will be sent back as an error to the client.
// All the connect args are passed also to onConnectRequest
//
function GameConnection::onConnectRequest( %client, %netAddress, %name )
{
   echo("Connect request from: " @ %netAddress);
   if($Server::PlayerCount >= $pref::Server::MaxPlayers)
      return "CR_SERVERFULL";
   return "";
}

//-----------------------------------------------------------------------------
// This script function is the first called on a client accept
//
function GameConnection::onConnect( %client, %name )
{
   // Send down the connection error info, the client is
   // responsible for displaying this message if a connection
   // error occures.
   messageClient(%client,'MsgConnectionError',"",$Pref::Server::ConnectionError);

   // Send mission information to the client
   sendLoadInfoToClient( %client );

   // Simulated client lag for testing...
   // %client.setSimulatedNetParams(0.1, 30);

   // Get the client's unique id:
   // %authInfo = %client.getAuthInfo();
   // %client.guid = getField( %authInfo, 3 );
   %client.guid = 0;
   addToServerGuidList( %client.guid );
   
   // Set admin status
   if (%client.getAddress() $= "local") {
      %client.isAdmin = true;
      %client.isSuperAdmin = true;
   }
   else {
      %client.isAdmin = false;
      %client.isSuperAdmin = false;
   }

   // Save client preferences on the connection object for later use.
   %client.gender = "Male";
   %client.armor = "Light";
   %client.race = "Human";
   %client.skin = addTaggedString( "base" );
   %client.setPlayerName(%name);
   %client.team = "";
   %client.score = 0;

   // 
   echo("CADD: " @ %client @ " " @ %client.getAddress());

   // Inform the client of all the other clients
   %count = ClientGroup.getCount();
   for (%cl = 0; %cl < %count; %cl++) {
      %other = ClientGroup.getObject(%cl);
      if ((%other != %client)) {
         // These should be "silent" versions of these messages...
         messageClient(%client, 'MsgClientJoin', "", 
               %other.playerName,
               %other,
               %other.sendGuid,
               %other.team,
               %other.score, 
               %other.isAIControlled(),
               %other.isAdmin, 
               %other.isSuperAdmin);
      }
   }

   // Inform the client we've joined up
   messageClient(%client,
      'MsgClientJoin', 'Welcome to a Torque application %1.', 
      %client.playerName, 
      %client,
      %client.sendGuid,
      %client.team,
      %client.score,
      %client.isAiControlled(), 
      %client.isAdmin, 
      %client.isSuperAdmin);

   // Inform all the other clients of the new guy
   messageAllExcept(%client, -1, 'MsgClientJoin', '\c1%1 joined the game.', 
      %client.playerName, 
      %client,
      %client.sendGuid,
      %client.team,
      %client.score,
      %client.isAiControlled(), 
      %client.isAdmin, 
      %client.isSuperAdmin);

   // If the mission is running, go ahead download it to the client
   if ($missionRunning)
   {
      %client.loadMission();
   }
   else if ($Server::LoadFailMsg !$= "")
   {
      messageClient(%client, 'MsgLoadFailed', $Server::LoadFailMsg);
   }
   $Server::PlayerCount++;
}

//-----------------------------------------------------------------------------
// A player's name could be obtained from the auth server, but for
// now we use the one passed from the client.
// %realName = getField( %authInfo, 0 );
//
function GameConnection::setPlayerName(%client,%name)
{
   %client.sendGuid = 0;

   // Minimum length requirements
   %name = trim( strToPlayerName( %name ) );
   if ( strlen( %name ) < 3 )
      %name = "Poser";

   // Make sure the alias is unique, we'll hit something eventually
   if (!isNameUnique(%name))
   {
      %isUnique = false;
      for (%suffix = 1; !%isUnique; %suffix++)  {
         %nameTry = %name @ "." @ %suffix;
         %isUnique = isNameUnique(%nameTry);
      }
      %name = %nameTry;
   }

   // Tag the name with the "smurf" color:
   %client.nameBase = %name;
   %client.playerName = addTaggedString("\cp\c8" @ %name @ "\co");
}

function isNameUnique(%name)
{
   %count = ClientGroup.getCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %test = ClientGroup.getObject( %i );
      %rawName = stripChars( detag( getTaggedString( %test.playerName ) ), "\cp\co\c6\c7\c8\c9" );
      if ( strcmp( %name, %rawName ) == 0 )
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
// This function is called when a client drops for any reason
//
function GameConnection::onDrop(%client, %reason)
{
   %client.onClientLeaveGame();
   
   removeFromServerGuidList( %client.guid );
   messageAllExcept(%client, -1, 'MsgClientDrop', '\c1%1 has left the game.', %client.playerName, %client);

   removeTaggedString(%client.playerName);
   echo("CDROP: " @ %client @ " " @ %client.getAddress());
   $Server::PlayerCount--;
   
   // Reset the server if everyone has left the game
   if( $Server::PlayerCount == 0 && $Server::Dedicated)
      schedule(0, 0, "resetServerDefaults");
}


//-----------------------------------------------------------------------------

function GameConnection::startMission(%this)
{
   // Inform the client the mission starting
   commandToClient(%this, 'MissionStart', $missionSequence);
}


function GameConnection::endMission(%this)
{
   // Inform the client the mission is done.  Note that if this is
   // called as part of the server destruction routine, the client will
   // actually never see this comment since the client connection will
   // be destroyed before another round of command processing occurs.
   // In this case, the client will only see the disconnect from the server
   // and should manually trigger a mission cleanup.
   commandToClient(%this, 'MissionEnd', $missionSequence);
}


//--------------------------------------------------------------------------
// Sync the clock on the client.

function GameConnection::syncClock(%client, %time)
{
   commandToClient(%client, 'syncClock', %time);
}


//--------------------------------------------------------------------------
// Update all the clients with the new score

function GameConnection::incScore(%this,%delta)
{
   %this.score += %delta;
   messageAll('MsgClientScoreChanged', "", %this.score, %this);
}
