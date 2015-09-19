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
// Variables used by client scripts & code.  The ones marked with (c)
// are accessed from code.  Variables preceeded by Pref:: are client
// preferences and stored automatically in the ~/client/prefs.cs file
// in between sessions.
//
//    (c) Client::MissionFile             Mission file name
//    ( ) Client::Password                Password for server join

//    (?) Pref::Player::CurrentFOV
//    (?) Pref::Player::DefaultFov
//    ( ) Pref::Input::KeyboardTurnSpeed

//    (c) pref::Master[n]                 List of master servers
//    (c) pref::Net::RegionMask
//    (c) pref::Client::ServerFavoriteCount
//    (c) pref::Client::ServerFavorite[FavoriteCount]
//    .. Many more prefs... need to finish this off

// Moves, not finished with this either...
//    (c) firstPerson
//    $mv*Action...

//-----------------------------------------------------------------------------
// These are variables used to control the shell scripts and
// can be overriden by mods:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
function initClient()
{
   echo("\n--------- Initializing " @ $appName @ ": Client Scripts ---------");

   // Make sure this variable reflects the correct state.
   $Server::Dedicated = false;

   // Game information used to query the master server
   $Client::GameTypeQuery = $appName;
   $Client::MissionTypeQuery = "Any";

   // These should be game specific GuiProfiles.  Custom profiles are saved out
   // from the Gui Editor.  Either of these may override any that already exist.
   exec("data/scripts/gui/defaultGameProfiles.cs");
   exec("data/scripts/gui/customProfiles.cs"); 
   
   // The common module provides basic client functionality
   initBaseClient();

   // Use our prefs to configure our Canvas/Window
   configureCanvas();

   // Load up the Game GUIs
   exec("data/ui/Game/playGui.gui");
   exec("data/ui/Game/chatHud.gui");
   exec("data/ui/Game/playerList.gui");
   exec("data/ui/Game/hudlessGui.gui");

   // Load up the shell GUIs
   exec("data/ui/Engine/GreedMenu.gui");
   exec("data/ui/Engine/joinServerDlg.gui");
   exec("data/ui/Engine/endGameGui.gui");
   exec("data/ui/Engine/StartupGui.gui");
   exec("data/ui/Engine/chooseLevelDlg.gui");
   exec("data/ui/Engine/loadingGui.gui");
   exec("data/ui/Engine/optionsDlg.gui");
   exec("data/ui/Engine/remapDlg.gui");
   
   // Gui scripts
   exec("data/scripts/client/playerList.cs");
   exec("data/scripts/gui/chatHud.cs");
   exec("data/scripts/client/messageHud.cs");
   exec("data/scripts/gui/playGui.cs");
   exec("data/scripts/gui/startupGui.cs");
   exec("data/scripts/gui/chooseLevelDlg.cs");
   exec("data/scripts/gui/loadingGui.cs");
   exec("data/scripts/gui/optionsDlg.cs");

   // Client scripts
   exec("data/scripts/client/client.cs");
   exec("data/scripts/client/game.cs");
   exec("data/scripts/client/missionDownload.cs");
   exec("data/scripts/client/serverConnection.cs");

   // Load useful Materials
   exec("data/scripts/client/shaders.cs");

   // Default player key bindings
   exec("data/scripts/client/default.bind.cs");

   if (isFile("data/scripts/client/config.cs"))
      exec("data/scripts/client/config.cs");

   loadMaterials();

   // Really shouldn't be starting the networking unless we are
   // going to connect to a remote server, or host a multi-player
   // game.
   setNetPort(0);

   // Copy saved script prefs into C++ code.
   setDefaultFov( $pref::Player::defaultFov );
   setZoomSpeed( $pref::Player::zoomSpeed );

   if( isScriptFile( expandFilename("data/scripts/client/audioData.cs") ) )
      exec( "data/scripts/client/audioData.cs" );

   // Start up the main menu... this is separated out into a
   // method for easier mod override.

   if ($startWorldEditor || $startGUIEditor) {
      // Editor GUI's will start up in the primary main.cs once
      // engine is initialized.
      return;
   }

   // Connect to server if requested.
   if ($JoinGameAddress !$= "") {
      // If we are instantly connecting to an address, load the
      // loading GUI then attempt the connect.
      loadLoadingGui();
      connect($JoinGameAddress, "", $Pref::Player::Name);
   }
   else {
      // Otherwise go to the splash screen.
      Canvas.setCursor("DefaultCursor");
      loadStartup();
   }   
}


//-----------------------------------------------------------------------------

function loadMainMenu()
{
   // Startup the base game menu (original t3d)
   if (isObject( GreedMenu ))
      Canvas.setContent( GreedMenu );
   
   Canvas.setCursor("DefaultCursor");

   // first check if we have a level file to load
   if ($levelToLoad !$= "")
   {
      %levelFile = "data/levels/";
      %ext = getSubStr($levelToLoad, strlen($levelToLoad) - 3, 3);
      if(%ext !$= "mis")
         %levelFile = %levelFile @ $levelToLoad @ ".mis";
      else
         %levelFile = %levelFile @ $levelToLoad;

      // Clear out the $levelToLoad so we don't attempt to load the level again
      // later on.
      $levelToLoad = "";
      
      // let's make sure the file exists
      %file = findFirstFile(%levelFile);

      if(%file !$= "")
         createAndConnectToLocalServer( "SinglePlayer", %file );
   }
}

function loadLoadingGui(%displayText)
{
   Canvas.setContent("LoadingGui");
   LoadingProgress.setValue(1);

   if (%displayText !$= "")
   {
      LoadingProgressTxt.setValue(%displayText);
   }
   else
   {
      LoadingProgressTxt.setValue("WAITING FOR SERVER");
   }

   Canvas.repaint();
}
