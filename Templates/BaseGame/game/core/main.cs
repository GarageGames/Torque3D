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

// ----------------------------------------------------------------------------
// Initialize core sub system functionality such as audio, the Canvas, PostFX,
// rendermanager, light managers, etc.
//
// Note that not all of these need to be initialized before the client, although
// the audio should and the canvas definitely needs to be.  I've put things here
// to distinguish between the purpose and functionality of the various client
// scripts.  Game specific script isn't needed until we reach the shell menus
// and start a game or connect to a server. We get the various subsystems ready
// to go, and then use initClient() to handle the rest of the startup sequence.
//
// If this is too convoluted we can reduce this complexity after futher testing
// to find exactly which subsystems should be readied before kicking things off. 
// ----------------------------------------------------------------------------

//We need to hook the missing/warn material stuff early, so do it here
$Core::MissingTexturePath = "core/images/missingTexture";
$Core::UnAvailableTexturePath = "core/images/unavailable";
$Core::WarningTexturePath = "core/images/warnMat";
$Core::CommonShaderPath = "core/shaders";

exec("./helperFunctions.cs");

// We need some of the default GUI profiles in order to get the canvas and
// other aspects of the GUI system ready.
exec("./profiles.cs");

//This is a bit of a shortcut, but we'll load the client's default settings to ensure all the prefs get initialized correctly
%prefPath = getPrefpath();
if ( isFile( %prefPath @ "/clientPrefs.cs" ) )
   exec( %prefPath @ "/clientPrefs.cs" );
else
   exec("data/defaults.cs");
   
%der = $pref::Video::displayDevice;

// Initialization of the various subsystems requires some of the preferences
// to be loaded... so do that first.
exec("./globals.cs");

exec("./canvas.cs");
exec("./cursor.cs");

exec("./renderManager.cs");
exec("./lighting.cs");

exec("./audio.cs");
exec("./sfx/audioAmbience.cs");
exec("./sfx/audioData.cs");
exec("./sfx/audioDescriptions.cs");
exec("./sfx/audioEnvironments.cs");
exec("./sfx/audioStates.cs");

exec("./parseArgs.cs");

// Materials and Shaders for rendering various object types
exec("./gfxData/commonMaterialData.cs");
exec("./gfxData/shaders.cs");
exec("./gfxData/terrainBlock.cs");
exec("./gfxData/water.cs");
exec("./gfxData/scatterSky.cs");
exec("./gfxData/clouds.cs");

// Initialize all core post effects.   
exec("./postFx.cs");

// Seed the random number generator.
setRandomSeed();