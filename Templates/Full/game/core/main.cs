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

// Constants for referencing video resolution preferences
$WORD::RES_X = 0;
$WORD::RES_Y = 1;
$WORD::FULLSCREEN = 2;
$WORD::BITDEPTH = 3;
$WORD::REFRESH = 4;
$WORD::AA = 5;

//---------------------------------------------------------------------------------------------
// CorePackage
// Adds functionality for this mod to some standard functions.
//---------------------------------------------------------------------------------------------
package CorePackage
{
//---------------------------------------------------------------------------------------------
// onStart
// Called when the engine is starting up. Initializes this mod.
//---------------------------------------------------------------------------------------------
function onStart()
{
   Parent::onStart();

   // Here is where we will do the video device stuff, so it overwrites the defaults
   // First set the PCI device variables (yes AGP/PCI-E works too)
   $isFirstPersonVar = 1;

   // Uncomment to enable AdvancedLighting on the Mac (T3D 2009 Beta 3)
   //$pref::machax::enableAdvancedLighting = true;

   // Uncomment to disable ShaderGen, useful when debugging
   //$ShaderGen::GenNewShaders = false;
   
   // Uncomment to dump disassembly for any shader that is compiled to disk.
   // These will appear as shadername_dis.txt in the same path as the
   // hlsl or glsl shader.   
   //$gfx::disassembleAllShaders = true;

   // Uncomment useNVPerfHud to allow you to start up correctly
   // when you drop your executable onto NVPerfHud
   //$Video::useNVPerfHud = true;
   
   // Uncomment these to allow you to force your app into using
   // a specific pixel shader version (0 is for fixed function)
   //$pref::Video::forcePixVersion = true;
   //$pref::Video::forcedPixVersion = 0;

   if ($platform $= "macos")
      $pref::Video::displayDevice = "OpenGL";
   //else
      //$pref::Video::displayDevice = "D3D9";
   
   // Initialise stuff.
   exec("./scripts/client/core.cs");
   initializeCore();

   exec("./scripts/client/client.cs");
   exec("./scripts/server/server.cs");
   
   exec("./scripts/gui/guiTreeViewCtrl.cs");
   exec("./scripts/gui/messageBoxes/messageBox.ed.cs");
   
   echo(" % - Initialized Core");
}

//---------------------------------------------------------------------------------------------
// onExit
// Called when the engine is shutting down. Shutdowns this mod.
//---------------------------------------------------------------------------------------------
function onExit()
{   
   // Shutdown stuff.
   shutdownCore();

   Parent::onExit();
}

function loadKeybindings()
{
   $keybindCount = 0;
   // Load up the active projects keybinds.
   if(isFunction("setupKeybinds"))
      setupKeybinds();
}

//---------------------------------------------------------------------------------------------
// displayHelp
// Prints the command line options available for this mod.
//---------------------------------------------------------------------------------------------
function displayHelp() {
   // Let the parent do its stuff.
   Parent::displayHelp();

   error("Core Mod options:\n" @
         "  -fullscreen            Starts game in full screen mode\n" @
         "  -windowed              Starts game in windowed mode\n" @
         "  -autoVideo             Auto detect video, but prefers OpenGL\n" @
         "  -openGL                Force OpenGL acceleration\n" @
         "  -directX               Force DirectX acceleration\n" @
         "  -voodoo2               Force Voodoo2 acceleration\n" @
         "  -prefs <configFile>    Exec the config file\n");
}

//---------------------------------------------------------------------------------------------
// parseArgs
// Parses the command line arguments and processes those valid for this mod.
//---------------------------------------------------------------------------------------------
function parseArgs()
{
   // Let the parent grab the arguments it wants first.
   Parent::parseArgs();

   // Loop through the arguments.
   for (%i = 1; %i < $Game::argc; %i++)
   {
      %arg = $Game::argv[%i];
      %nextArg = $Game::argv[%i+1];
      %hasNextArg = $Game::argc - %i > 1;
   
      switch$ (%arg)
      {
         case "-fullscreen":
            $cliFullscreen = true;
            $argUsed[%i]++;

         case "-windowed":
            $cliFullscreen = false;
            $argUsed[%i]++;

         case "-openGL":
            $pref::Video::displayDevice = "OpenGL";
            $argUsed[%i]++;

         case "-directX":
            $pref::Video::displayDevice = "D3D";
            $argUsed[%i]++;

         case "-voodoo2":
            $pref::Video::displayDevice = "Voodoo2";
            $argUsed[%i]++;

         case "-autoVideo":
            $pref::Video::displayDevice = "";
            $argUsed[%i]++;

         case "-prefs":
            $argUsed[%i]++;
            if (%hasNextArg) {
               exec(%nextArg, true, true);
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -prefs <path/script.cs>");
      }
   }
}

};

activatePackage(CorePackage);

