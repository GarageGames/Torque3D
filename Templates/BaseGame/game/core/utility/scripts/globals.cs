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
// DInput keyboard, mouse, and joystick prefs
// ----------------------------------------------------------------------------

$pref::Input::MouseEnabled = 1;
$pref::Input::LinkMouseSensitivity = 1;
$pref::Input::KeyboardEnabled = 1;
$pref::Input::KeyboardTurnSpeed = 0.1;
$pref::Input::JoystickEnabled = 0;

// ----------------------------------------------------------------------------
// Video Preferences
// ----------------------------------------------------------------------------

// Set directory paths for various data or default images.
$pref::Video::ProfilePath = "core/rendering/scripts/gfxprofile";

$pref::Video::disableVerticalSync = 1;
$pref::Video::mode = "800 600 false 32 60 4";
$pref::Video::defaultFenceCount = 0;

// This disables the hardware FSAA/MSAA so that we depend completely on the FXAA
// post effect which works on all cards and in deferred mode.  Note that the new
// Intel Hybrid graphics on laptops will fail to initialize when hardware AA is
// enabled... so you've been warned.
$pref::Video::disableHardwareAA = true;

$pref::Video::disableNormalmapping = false;
$pref::Video::disablePixSpecular = false;
$pref::Video::disableCubemapping = false;
$pref::Video::disableParallaxMapping = false;

// The number of mipmap levels to drop on loaded textures to reduce video memory
// usage.  It will skip any textures that have been defined as not allowing down
// scaling.
$pref::Video::textureReductionLevel = 0;

$pref::Video::defaultAnisotropy = 1;
//$pref::Video::Gamma = 1.0;

/// AutoDetect graphics quality levels the next startup.
$pref::Video::autoDetect = 1;

// ----------------------------------------------------------------------------
// Shader stuff
// ----------------------------------------------------------------------------

// This is the path used by ShaderGen to cache procedural shaders.  If left
// blank ShaderGen will only cache shaders to memory and not to disk.
$shaderGen::cachePath = "data/shaderCache";

// Uncomment to disable ShaderGen, useful when debugging
//$ShaderGen::GenNewShaders = false;
   
// Uncomment to dump disassembly for any shader that is compiled to disk.  These
// will appear as shadername_dis.txt in the same path as the shader file.   
//$gfx::disassembleAllShaders = true;

// ----------------------------------------------------------------------------
// Lighting and shadowing
// ----------------------------------------------------------------------------

// Uncomment to enable AdvancedLighting on the Mac (T3D 2009 Beta 3)
//$pref::machax::enableAdvancedLighting = true;

$sceneLighting::cacheSize = 20000;
$sceneLighting::purgeMethod = "lastCreated";
$sceneLighting::cacheLighting = 1;

$pref::Shadows::textureScalar = 1.0;
$pref::Shadows::disable = false;

// Sets the shadow filtering mode.
//  None - Disables filtering.
//  SoftShadow - Does a simple soft shadow
//  SoftShadowHighQuality 
$pref::Shadows::filterMode = "SoftShadow";
