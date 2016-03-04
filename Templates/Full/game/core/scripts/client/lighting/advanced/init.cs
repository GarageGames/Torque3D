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

///////////////////////////////////////////////////////////////////////////////
// Default Prefs

/*
$pref::LightManager::sgAtlasMaxDynamicLights = "16";
$pref::LightManager::sgDynamicShadowDetailSize = "0";
$pref::LightManager::sgDynamicShadowQuality = "0";
$pref::LightManager::sgLightingProfileAllowShadows = "1";
$pref::LightManager::sgLightingProfileQuality = "0";
$pref::LightManager::sgMaxBestLights = "10";
$pref::LightManager::sgMultipleDynamicShadows = "1";
$pref::LightManager::sgShowCacheStats = "0";
$pref::LightManager::sgUseBloom = "";
$pref::LightManager::sgUseDRLHighDynamicRange = "0";
$pref::LightManager::sgUseDynamicRangeLighting = "0";
$pref::LightManager::sgUseDynamicShadows = "1";
$pref::LightManager::sgUseToneMapping = "";
*/

exec( "./shaders.cs" );
exec( "./lightViz.cs" );
exec( "./shadowViz.cs" );
exec( "./shadowViz.gui" );
exec( "./deferredShading.cs" );

function onActivateAdvancedLM()
{
   // Don't allow the offscreen target on OSX.
   if ( $platform $= "macos" )
      return;
                  
   // On the Xbox360 we know what will be enabled so don't do any trickery to
   // disable MSAA
   if ( $platform $= "xenon" )
      return;
      
   // Enable the offscreen target so that AL will work
   // with MSAA back buffers and for HDR rendering.   
   AL_FormatToken.enable();
   
   // Activate Deferred Shading
   AL_DeferredShading.enable();
}

function onDeactivateAdvancedLM()
{
   // Disable the offscreen render target.
   AL_FormatToken.disable();
   
   // Deactivate Deferred Shading
   AL_DeferredShading.disable();
}

function setAdvancedLighting()
{
   setLightManager( "Advanced Lighting" );   
}

