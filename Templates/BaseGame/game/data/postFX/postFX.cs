
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function PostFX::create( %this )
{
   echo("\n--------- Initializing PostFX Directory: scripts ---------");

   // Start up in either client, or dedicated server mode
   if (!$Server::Dedicated)
   {
      //postFX stuffs
      exec("./scripts/gui/postFxManager.gui");
      
      //init the postFX
      %pattern = "./scripts/client/*.cs";   
      %file = findFirstFile( %pattern );
      if ( %file $= "" )
      {
         // Try for DSOs next.
         %pattern = "./scripts/client/*.cs.dso";
         %file = findFirstFile( %pattern );
      }
      
      while( %file !$= "" )
      {      
         exec( %file );
         %file = findNextFile( %pattern );
      }
   }
}

function PostFX::destroy( %this )
{
}