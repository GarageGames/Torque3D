
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function SpectatorGameplay::create( %this )
{
   //server scripts
   exec("./scripts/server/camera.cs");
   exec("./scripts/server/DefaultGame.cs");
   exec("./scripts/server/VolumetricFog.cs");
   
   //add DBs
   if(isObject(DatablockFilesList))
   {
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/camera.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/defaultParticle.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/lights.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/managedDatablocks.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/managedDecalData.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/managedForestItemData.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/managedParticleData.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/managedParticleEmitterData.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/markers.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/ribbons.cs" );
      DatablockFilesList.add( "data/spectatorGameplay/scripts/datablocks/sounds.cs" );
   }
   
   if(isObject(LevelFilesList))
   {
      for( %file = findFirstFile( "data/spectatorGameplay/levels/*.mis" );
      %file !$= "";
      %file = findNextFile( "data/spectatorGameplay/levels/*.mis" ))
      {
         LevelFilesList.add(%file);
      }
   }
   
   if (!$Server::Dedicated)
   {
      //client scripts
      $KeybindPath = "data/spectatorGameplay/scripts/client/default.keybinds.cs";
      exec($KeybindPath);
      
      %prefPath = getPrefpath();
      if(isFile(%prefPath @ "/keybinds.cs"))
         exec(%prefPath @ "/keybinds.cs");
         
      exec("data/spectatorGameplay/scripts/client/inputCommands.cs");
      
      //guis
      exec("./scripts/gui/playGui.gui");
      exec("./scripts/gui/playGui.cs");
   }
}

function SpectatorGameplay::destroy( %this )
{
   
}