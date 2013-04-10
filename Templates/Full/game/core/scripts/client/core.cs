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

//---------------------------------------------------------------------------------------------
// initializeCore
// Initializes core game functionality.
//---------------------------------------------------------------------------------------------
function initializeCore()
{
   // Not Reentrant
   if( $coreInitialized == true )
      return;
      
   // Core keybindings.
   GlobalActionMap.bind(keyboard, tilde, toggleConsole);
   GlobalActionMap.bind(keyboard, "ctrl p", doScreenShot);
   GlobalActionMap.bindcmd(keyboard, "alt enter", "Canvas.attemptFullscreenToggle();","");
   GlobalActionMap.bindcmd(keyboard, "alt k", "cls();",  "");
//   GlobalActionMap.bindCmd(keyboard, "escape", "", "handleEscape();");
   
   
   
   // Very basic functions used by everyone.
   exec("./audio.cs");
   exec("./canvas.cs");
   exec("./cursor.cs");
   exec("./persistenceManagerTest.cs");

   // Content.
   exec("~/art/gui/profiles.cs");
   exec("~/scripts/gui/cursors.cs");
   
   exec( "./audioEnvironments.cs" );
   exec( "./audioDescriptions.cs" );
   exec( "./audioStates.cs" );
   exec( "./audioAmbiences.cs" );

   // Input devices
   exec("~/scripts/client/oculusVR.cs");

   // Seed the random number generator.
   setRandomSeed();
   
   // Set up networking.
   setNetPort(0);
  
   // Initialize the canvas.
   initializeCanvas();

   // Start processing file change events.   
   startFileChangeNotifications();
      
   // Core Guis.
   exec("~/art/gui/console.gui");
   exec("~/art/gui/consoleVarDlg.gui");
   exec("~/art/gui/netGraphGui.gui");
   
   // Gui Helper Scripts.
   exec("~/scripts/gui/help.cs");

   // Random Scripts.
   exec("~/scripts/client/screenshot.cs");
   exec("~/scripts/client/scriptDoc.cs");
   //exec("~/scripts/client/keybindings.cs");
   exec("~/scripts/client/helperfuncs.cs");
   exec("~/scripts/client/commands.cs");
   
   // Client scripts
   exec("~/scripts/client/devHelpers.cs");
   exec("~/scripts/client/metrics.cs");
   exec("~/scripts/client/recordings.cs");
   exec("~/scripts/client/centerPrint.cs");

   // Materials and Shaders for rendering various object types
   loadCoreMaterials();

   exec("~/scripts/client/commonMaterialData.cs");
   exec("~/scripts/client/shaders.cs");
   exec("~/scripts/client/materials.cs");
   exec("~/scripts/client/terrainBlock.cs");
   exec("~/scripts/client/water.cs");
   exec("~/scripts/client/imposter.cs");
   exec("~/scripts/client/scatterSky.cs");
   exec("~/scripts/client/clouds.cs");
   
   // Initialize all core post effects.   
   exec("~/scripts/client/postFx.cs");
   initPostEffects();
   
   // Initialize the post effect manager.
   exec("~/scripts/client/postFx/postFXManager.gui");
   exec("~/scripts/client/postFx/postFXManager.gui.cs");
   exec("~/scripts/client/postFx/postFXManager.gui.settings.cs");
   exec("~/scripts/client/postFx/postFXManager.persistance.cs");
   
   PostFXManager.settingsApplyDefaultPreset();  // Get the default preset settings   
   
   // Set a default cursor.
   Canvas.setCursor(DefaultCursor);
   
   loadKeybindings();

   $coreInitialized = true;
}

//---------------------------------------------------------------------------------------------
// shutdownCore
// Shuts down core game functionality.
//---------------------------------------------------------------------------------------------
function shutdownCore()
{      
   // Stop file change events.
   stopFileChangeNotifications();
   
   sfxShutdown();
}

//---------------------------------------------------------------------------------------------
// dumpKeybindings
// Saves of all keybindings.
//---------------------------------------------------------------------------------------------
function dumpKeybindings()
{
   // Loop through all the binds.
   for (%i = 0; %i < $keybindCount; %i++)
   {
      // If we haven't dealt with this map yet...
      if (isObject($keybindMap[%i]))
      {
         // Save and delete.
         $keybindMap[%i].save(getPrefsPath("bind.cs"), %i == 0 ? false : true);
         $keybindMap[%i].delete();
      }
   }
}

function handleEscape()
{

   if (isObject(EditorGui))
   {
      if (Canvas.getContent() == EditorGui.getId())
      {
         EditorGui.handleEscape();
         return;
      }
      else if ( EditorIsDirty() )
      {
         MessageBoxYesNoCancel( "Level Modified", "Level has been modified in the Editor. Save?",
                           "EditorDoExitMission(1);",
                           "EditorDoExitMission();",
                           "");
         return;
      }
   }

   if (isObject(GuiEditor))
   {
      if (GuiEditor.isAwake())
      {
         GuiEditCanvas.quit();
         return; 
      }
   }

   if (PlayGui.isAwake())	
      escapeFromGame();	
}

//-----------------------------------------------------------------------------
// loadMaterials - load all materials.cs files
//-----------------------------------------------------------------------------
function loadCoreMaterials()
{
   // Load any materials files for which we only have DSOs.

   for( %file = findFirstFile( "core/materials.cs.dso" );
        %file !$= "";
        %file = findNextFile( "core/materials.cs.dso" ))
   {
      // Only execute, if we don't have the source file.
      %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
      if( !isFile( %csFileName ) )
         exec( %csFileName );
   }

   // Load all source material files.

   for( %file = findFirstFile( "core/materials.cs" );
        %file !$= "";
        %file = findNextFile( "core/materials.cs" ))
   {
      exec( %file );
   }
}

function reloadCoreMaterials()
{
   reloadTextures();
   loadCoreMaterials();
   reInitMaterials();
}

//-----------------------------------------------------------------------------
// loadMaterials - load all materials.cs files
//-----------------------------------------------------------------------------
function loadMaterials()
{
   // Load any materials files for which we only have DSOs.

   for( %file = findFirstFile( "*/materials.cs.dso" );
        %file !$= "";
        %file = findNextFile( "*/materials.cs.dso" ))
   {
      // Only execute, if we don't have the source file.
      %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
      if( !isFile( %csFileName ) )
         exec( %csFileName );
   }

   // Load all source material files.

   for( %file = findFirstFile( "*/materials.cs" );
        %file !$= "";
        %file = findNextFile( "*/materials.cs" ))
   {
      exec( %file );
   }

   // Load all materials created by the material editor if
   // the folder exists
   if( IsDirectory( "materialEditor" ) )
   {
      for( %file = findFirstFile( "materialEditor/*.cs.dso" );
           %file !$= "";
           %file = findNextFile( "materialEditor/*.cs.dso" ))
      {
         // Only execute, if we don't have the source file.
         %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
         if( !isFile( %csFileName ) )
            exec( %csFileName );
      }

      for( %file = findFirstFile( "materialEditor/*.cs" );
           %file !$= "";
           %file = findNextFile( "materialEditor/*.cs" ))
      {
         exec( %file );
      }
   }
}

function reloadMaterials()
{
   reloadTextures();
   loadMaterials();
   reInitMaterials();
}