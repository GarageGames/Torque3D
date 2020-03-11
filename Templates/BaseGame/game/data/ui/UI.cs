
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function UI::create( %this )
{
   if ($Server::Dedicated)
      return;
   
   // Use our prefs to configure our Canvas/Window
   configureCanvas();
   
   //Load UI stuff
   //we need to load this because some of the menu profiles use the sounds here
   exec("./scripts/datablocks/guiSounds.cs");
   
   //Profiles
   exec("./scripts/profiles.cs");
   
   //Now gui files
   exec("./scripts/guis/mainMenu.gui");
   exec("./scripts/guis/chooseLevelDlg.gui");
   exec("./scripts/guis/joinServerMenu.gui");
   exec("./scripts/guis/loadingGui.gui");
   exec("./scripts/guis/optionsMenu.gui");
   exec("./scripts/guis/pauseMenu.gui");
   exec("./scripts/guis/remapDlg.gui");
   exec("./scripts/guis/remapConfirmDlg.gui");
   
   exec("./scripts/guis/profiler.gui");
   exec("./scripts/guis/netGraphGui.gui");
   exec("./scripts/guis/FileDialog.gui");
   exec("./scripts/guis/guiMusicPlayer.gui");
   exec("./scripts/guis/startupGui.gui");
   
   //Load gui companion scripts
   exec("./scripts/chooseLevelDlg.cs");
   exec("./scripts/optionsList.cs");
   exec("./scripts/optionsMenu.cs");
   exec("./scripts/graphicsMenu.cs");
   exec("./scripts/controlsMenu.cs");
   exec("./scripts/chooseLevelDlg.cs");
   exec("./scripts/mainMenu.cs");
   exec("./scripts/joinServerMenu.cs");
   exec("./scripts/pauseMenu.cs");
   exec("./scripts/messageBoxes.cs");
   exec("./scripts/help.cs");
   exec("./scripts/cursors.cs");
   exec("./scripts/profiler.cs");
   exec("./scripts/FileDialog.cs");
   exec("./scripts/GuiTreeViewCtrl.cs");
   exec("./scripts/guiMusicPlayer.cs");
   exec("./scripts/startupGui.cs");
   
   %dbList = new ArrayObject(LevelFilesList);
   
   loadStartup();
   //Canvas.pushDialog(MainMenuGui);
}

function Game::destroy( %this )
{
   
}