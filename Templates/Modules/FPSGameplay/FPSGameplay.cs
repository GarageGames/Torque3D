
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function FPSGameplay::create( %this )
{
   //server scripts
   exec("./scripts/server/aiPlayer.cs");
   exec("./scripts/server/camera.cs");
   exec("./scripts/server/chat.cs");
   exec("./scripts/server/cheetah.cs");
   exec("./scripts/server/commands.cs");
   exec("./scripts/server/deathMatchGame.cs");
   exec("./scripts/server/health.cs");
   exec("./scripts/server/inventory.cs");
   exec("./scripts/server/item.cs");
   exec("./scripts/server/player.cs");
   exec("./scripts/server/projectile.cs");
   exec("./scripts/server/proximityMine.cs");
   exec("./scripts/server/radiusDamage.cs");
   exec("./scripts/server/shapeBase.cs");
   exec("./scripts/server/spawn.cs");
   exec("./scripts/server/teleporter.cs");
   exec("./scripts/server/triggers.cs");
   exec("./scripts/server/turret.cs");
   exec("./scripts/server/vehicle.cs");
   exec("./scripts/server/vehicleWheeled.cs");
   exec("./scripts/server/VolumetricFog.cs");
   exec("./scripts/server/weapon.cs");
   
   //add DBs
   if(isObject(DatablockFilesList))
   {
      for( %file = findFirstFile( "data/FPSGameplay/scripts/datablocks/*.cs.dso" );
      %file !$= "";
      %file = findNextFile( "data/FPSGameplay/scripts/datablocks/*.cs.dso" ))
      {
         // Only execute, if we don't have the source file.
         %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
         if( !isFile( %csFileName ) )
            DatablockFilesList.add(%csFileName);
      }
      
      // Load all source material files.
      for( %file = findFirstFile( "data/FPSGameplay/scripts/datablocks/*.cs" );
      %file !$= "";
      %file = findNextFile( "data/FPSGameplay/scripts/datablocks/*.cs" ))
      {
         DatablockFilesList.add(%file);
      }
   }
   
   if(isObject(LevelFilesList))
   {
      for( %file = findFirstFile( "data/FPSGameplay/levels/*.mis" );
      %file !$= "";
      %file = findNextFile( "data/FPSGameplay/levels/*.mis" ))
      {
         LevelFilesList.add(%file);
      }
   }
   
   if (!$Server::Dedicated)
   {
      exec("data/FPSGameplay/scripts/client/gameProfiles.cs");
      
      //client scripts
      $KeybindPath = "data/FPSGameplay/scripts/client/default.keybinds.cs";
      exec($KeybindPath);
      
      %prefPath = getPrefpath();
      if(isFile(%prefPath @ "/keybinds.cs"))
         exec(%prefPath @ "/keybinds.cs");
         
      exec("data/FPSGameplay/scripts/client/inputCommands.cs");
      
      //guis
      exec("./scripts/gui/chatHud.gui");
      exec("./scripts/gui/playerList.gui");
      exec("./scripts/gui/playGui.gui");
      
      exec("./scripts/gui/playGui.cs");
      
      exec("data/FPSGameplay/scripts/client/message.cs");
      exec("data/FPSGameplay/scripts/client/chatHud.cs");
      exec("data/FPSGameplay/scripts/client/clientCommands.cs");
      exec("data/FPSGameplay/scripts/client/messageHud.cs");
      exec("data/FPSGameplay/scripts/client/playerList.cs");
   }
}

function FPSGameplay::destroy( %this )
{
   
}