
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function BlankGame::create( %this )
{
   exec("./scripts/guis/playGui.gui");
   
   datablock CameraData(Observer) {};
   
   // Create a local game server and connect to it.
   new SimGroup(ServerGroup);
   new GameConnection(ServerConnection);

   // This calls GameConnection::onConnect.
   ServerConnection.connectLocal();

   //-----------------------------------------------------------------------------
   // Add a material to give the ground some colour (even if it's just white).
   singleton Material(BlankWhite) {
       diffuseColor[0] = "White";
   };

   // Create objects!
   new SimGroup(MissionGroup)
   {
      new LevelInfo(TheLevelInfo) 
      {
         canvasClearColor = "0 0 0";
      };
      new GroundPlane(TheGround) 
      {
         position = "0 0 0";
         material = BlankWhite;
      };
      new Sun(TheSun) 
      {
         azimuth = 230;
         elevation = 45;
         color = "1 1 1";
         ambient = "0.1 0.1 0.1";
         castShadows = true;
      };
   };
   
   new SimGroup(MissionCleanup);

   // Allow us to exit the game...
   GlobalActionMap.bind("keyboard", "escape", "quit");
}

function BlankGame::destroy( %this )
{
   // Clean up ghosts.
   ServerConnection.delete();

   // Delete the objects we created.
   MissionGroup.delete();
   MissionCleanup.delete();

   // Delete server-side objects and datablocks.
   ServerGroup.delete();
   deleteDataBlocks();
}

//-----------------------------------------------------------------------------
// Called when all datablocks have been transmitted.
function GameConnection::onEnterGame(%client) {
   // Create a camera for the client.
   new Camera(TheCamera) {
      datablock = Observer;
   };
   TheCamera.setTransform("0 0 2 1 0 0 0");
   
   // Cameras are not ghosted (sent across the network) by default; we need to
   // do it manually for the client that owns the camera or things will go south
   // quickly.
   TheCamera.scopeToClient(%client);
   
   // And let the client control the camera.
   %client.setControlObject(TheCamera);
   
   // Add the camera to the group of game objects so that it's cleaned up when
   // we close the game.
   MissionGroup.add(TheCamera);
   
   LocalClientConnection.camera = TheCamera;
   
   // Activate HUD which allows us to see the game. This should technically be
   // a commandToClient, but since the client and server are on the same
   // machine...
   Canvas.setContent(PlayGui);
   activateDirectInput();
}

// Called when we connect to the local game.
function GameConnection::onConnect(%this) {
   %this.transmitDataBlocks(0);
}

// Called when all datablocks from above have been transmitted.
function GameConnection::onDataBlocksDone(%this) {
   closeSplashWindow();
   Canvas.showWindow();

   // Start sending ghosts to the client.
   %this.activateGhosting();
   %this.onEnterGame();
}

//stubs to keep the console clean. These aren't needed for such a lightweight implementation.
function serverCmdsetClientAspectRatio(){}
function onDataBlockObjectReceived(){}
function onGhostAlwaysObjectReceived(){}
function onGhostAlwaysStarted(){}
function onGhostAlwaysObjectReceived(){}