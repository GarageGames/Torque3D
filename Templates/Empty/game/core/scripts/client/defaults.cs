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

// The master server is declared with the server defaults, which is
// loaded on both clients & dedicated servers.  If the server mod
// is not loaded on a client, then the master must be defined. 
// $pref::Master[0] = "2:master.garagegames.com:28002";

$pref::Player::Name = "Visitor";
$pref::Player::defaultFov = 65;
$pref::Player::zoomSpeed = 0;

$pref::Net::LagThreshold = 400;
$pref::Net::Port = 28000;

$pref::HudMessageLogSize = 40;
$pref::ChatHudLength = 1;

$pref::Input::LinkMouseSensitivity = 1;
// DInput keyboard, mouse, and joystick prefs
$pref::Input::KeyboardEnabled = 1;
$pref::Input::MouseEnabled = 1;
$pref::Input::JoystickEnabled = 0;
$pref::Input::KeyboardTurnSpeed = 0.1;
$pref::Input::MouseWheelSpeed = 120;

$sceneLighting::cacheSize = 20000;
$sceneLighting::purgeMethod = "lastCreated";
$sceneLighting::cacheLighting = 1;

$pref::Video::displayDevice = "D3D9";
$pref::Video::disableVerticalSync = 1;
$pref::Video::mode = "1024 768 false 32 60 4";
$pref::Video::defaultFenceCount = 0;
$pref::Video::screenShotSession = 0;
$pref::Video::screenShotFormat = "PNG";

/// This disables the hardware FSAA/MSAA so that
/// we depend completely on the FXAA post effect
/// which works on all cards and in deferred mode.
///
/// Note the new Intel Hybrid graphics on laptops
/// will fail to initialize when hardware AA is
/// enabled... so you've been warned.
///
$pref::Video::disableHardwareAA = true;

$pref::Video::disableNormalmapping = false;

$pref::Video::disablePixSpecular = false;

$pref::Video::disableCubemapping = false;

///
$pref::Video::disableParallaxMapping = false;

$pref::Video::Gamma = 1.0;

// Console-friendly defaults
if($platform $= "xenon")
{
   // Save some fillrate on the X360, and take advantage of the HW scaling
   $pref::Video::Resolution = "1152 640";
   $pref::Video::mode = $pref::Video::Resolution SPC "true 32 60 0";
   $pref::Video::fullScreen = 1;
}

/// This is the path used by ShaderGen to cache procedural
/// shaders.  If left blank ShaderGen will only cache shaders
/// to memory and not to disk.
$shaderGen::cachePath = "shaders/procedural";

/// The perfered light manager to use at startup.  If blank
/// or if the selected one doesn't work on this platfom it
/// will try the defaults below.
$pref::lightManager = "";

/// This is the default list of light managers ordered from
/// most to least desirable for initialization.
$lightManager::defaults = "Advanced Lighting" NL "Basic Lighting";

/// A scale to apply to the camera view distance
/// typically used for tuning performance.
$pref::camera::distanceScale = 1.0;

/// Causes the system to do a one time autodetect
/// of an SFX provider and device at startup if the
/// provider is unset.
$pref::SFX::autoDetect = true;

/// The sound provider to select at startup.  Typically
/// this is DirectSound, OpenAL, or XACT.  There is also 
/// a special Null provider which acts normally, but 
/// plays no sound.
$pref::SFX::provider = "";

/// The sound device to select from the provider.  Each
/// provider may have several different devices.
$pref::SFX::device = "OpenAL";

/// If true the device will try to use hardware buffers
/// and sound mixing.  If not it will use software.
$pref::SFX::useHardware = false;

/// If you have a software device you have a 
/// choice of how many software buffers to
/// allow at any one time.  More buffers cost
/// more CPU time to process and mix.
$pref::SFX::maxSoftwareBuffers = 16;

/// This is the playback frequency for the primary 
/// sound buffer used for mixing.  Although most
/// providers will reformat on the fly, for best 
/// quality and performance match your sound files
/// to this setting.
$pref::SFX::frequency = 44100;

/// This is the playback bitrate for the primary 
/// sound buffer used for mixing.  Although most
/// providers will reformat on the fly, for best 
/// quality and performance match your sound files
/// to this setting.
$pref::SFX::bitrate = 32;

/// The overall system volume at startup.  Note that 
/// you can only scale volume down, volume does not
/// get louder than 1.
$pref::SFX::masterVolume = 0.8;

/// The startup sound channel volumes.  These are 
/// used to control the overall volume of different 
/// classes of sounds.
$pref::SFX::channelVolume1 = 1;
$pref::SFX::channelVolume2 = 1;
$pref::SFX::channelVolume3 = 1;
$pref::SFX::channelVolume4 = 1;
$pref::SFX::channelVolume5 = 1;
$pref::SFX::channelVolume6 = 1;
$pref::SFX::channelVolume7 = 1;
$pref::SFX::channelVolume8 = 1;

$pref::PostEffect::PreferedHDRFormat = "GFXFormatR8G8B8A8";

/// This is an scalar which can be used to reduce the 
/// reflection textures on all objects to save fillrate.
$pref::Reflect::refractTexScale = 1.0;

/// This is the total frame in milliseconds to budget for
/// reflection rendering.  If your CPU bound and have alot
/// of smaller reflection surfaces try reducing this time.
$pref::Reflect::frameLimitMS = 10;

/// Set true to force all water objects to use static cubemap reflections.
$pref::Water::disableTrueReflections = false;

// A global LOD scalar which can reduce the overall density of placed GroundCover.
$pref::GroundCover::densityScale = 1.0;

/// An overall scaler on the lod switching between DTS models.
/// Smaller numbers makes the lod switch sooner.
$pref::TS::detailAdjust = 1.0;

///
$pref::Decals::enabled = true;

///
$pref::Decals::lifeTimeScale = "1";

/// The number of mipmap levels to drop on loaded textures
/// to reduce video memory usage.  
///
/// It will skip any textures that have been defined as not 
/// allowing down scaling.
///
$pref::Video::textureReductionLevel = 0;

///
$pref::Shadows::textureScalar = 1.0;

///
$pref::Shadows::disable = false;

/// Sets the shadow filtering mode.
///
/// None - Disables filtering.
///
/// SoftShadow - Does a simple soft shadow
///
/// SoftShadowHighQuality 
///
$pref::Shadows::filterMode = "SoftShadow";

///
$pref::Video::defaultAnisotropy = 1;

/// Radius in meters around the camera that ForestItems are affected by wind.
/// Note that a very large number with a large number of items is not cheap.
$pref::windEffectRadius = 25;

/// AutoDetect graphics quality levels the next startup.
$pref::Video::autoDetect = 1;

//-----------------------------------------------------------------------------
// Graphics Quality Groups
//-----------------------------------------------------------------------------

// The graphics quality groups are used by the options dialog to
// control the state of the $prefs.  You should overload these in
// your game specific defaults.cs file if they need to be changed.

if ( isObject( MeshQualityGroup ) )
   MeshQualityGroup.delete();
if ( isObject( TextureQualityGroup ) )
   TextureQualityGroup.delete();
if ( isObject( LightingQualityGroup ) )
   LightingQualityGroup.delete();
if ( isObject( ShaderQualityGroup ) )
   ShaderQualityGroup.delete();
 
new SimGroup( MeshQualityGroup )
{ 
   new ArrayObject( [Lowest] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::TS::detailAdjust"] = 0.5;
      key["$pref::TS::skipRenderDLs"] = 1;      
      key["$pref::Terrain::lodScale"] = 2.0;
      key["$pref::decalMgr::enabled"] = false;
      key["$pref::GroundCover::densityScale"] = 0.5;
   };
   
   new ArrayObject( [Low] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
            
      key["$pref::TS::detailAdjust"] = 0.75;
      key["$pref::TS::skipRenderDLs"] = 0;      
      key["$pref::Terrain::lodScale"] = 1.5;
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::GroundCover::densityScale"] = 0.75;
   };
   
   new ArrayObject( [Normal] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;

      key["$pref::TS::detailAdjust"] = 1.0;
      key["$pref::TS::skipRenderDLs"] = 0;      
      key["$pref::Terrain::lodScale"] = 1.0;
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::GroundCover::densityScale"] = 1.0;
   };

   new ArrayObject( [High] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;

      key["$pref::TS::detailAdjust"] = 1.5;
      key["$pref::TS::skipRenderDLs"] = 0;      
      key["$pref::Terrain::lodScale"] = 0.75;
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::GroundCover::densityScale"] = 1.0;
   };   
};


new SimGroup( TextureQualityGroup )
{
   new ArrayObject( [Lowest] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::Video::textureReductionLevel"] = 2;
      key["$pref::Reflect::refractTexScale"] = 0.5;
      key["$pref::Terrain::detailScale"] = 0.5;      
   };
   
   new ArrayObject( [Low] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
            
      key["$pref::Video::textureReductionLevel"] = 1;
      key["$pref::Reflect::refractTexScale"] = 0.75;
      key["$pref::Terrain::detailScale"] = 0.75;      
   };
   
   new ArrayObject( [Normal] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1;
      key["$pref::Terrain::detailScale"] = 1;      
   };

   new ArrayObject( [High] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1.25;
      key["$pref::Terrain::detailScale"] = 1.5;      
   };   
};

function TextureQualityGroup::onApply( %this, %level )
{
   // Note that this can be a slow operation.  
   reloadTextures();
}


new SimGroup( LightingQualityGroup )
{ 
   new ArrayObject( [Lowest] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::lightManager"] = "Basic Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.5;
      key["$pref::Shadows::filterMode"] = "None";     
   };
   
   new ArrayObject( [Low] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
                  
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.5;
      key["$pref::Shadows::filterMode"] = "SoftShadow";     
   };
   
   new ArrayObject( [Normal] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;

      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 1.0;
      key["$pref::Shadows::filterMode"] = "SoftShadowHighQuality";     
   };

   new ArrayObject( [High] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 2.0;
      key["$pref::Shadows::filterMode"] = "SoftShadowHighQuality";          
   };   
};

function LightingQualityGroup::onApply( %this, %level )
{
   // Set the light manager.  This should do nothing 
   // if its already set or if its not compatible.   
   setLightManager( $pref::lightManager );
}


// TODO: Reduce shader complexity of water and the scatter sky here!
new SimGroup( ShaderQualityGroup )
{
   new ArrayObject( [Lowest] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::Video::disablePixSpecular"] = true;
      key["$pref::Video::disableNormalmapping"] = true;
      key["$pref::Video::disableParallaxMapping"] = true;
      key["$pref::Water::disableTrueReflections"] = true;
   };
   
   new ArrayObject( [Low] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::Video::disablePixSpecular"] = false;
      key["$pref::Video::disableNormalmapping"] = false;
      key["$pref::Video::disableParallaxMapping"] = true;
      key["$pref::Water::disableTrueReflections"] = true;
   };
   
   new ArrayObject( [Normal] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::Video::disablePixSpecular"] = false;
      key["$pref::Video::disableNormalmapping"] = false;
      key["$pref::Video::disableParallaxMapping"] = false;   
      key["$pref::Water::disableTrueReflections"] = false;   
   };
   
   new ArrayObject( [High] )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      key["$pref::Video::disablePixSpecular"] = false;
      key["$pref::Video::disableNormalmapping"] = false;
      key["$pref::Video::disableParallaxMapping"] = false;     
      key["$pref::Water::disableTrueReflections"] = false;          
   };   
};


function GraphicsQualityAutodetect()
{
   $pref::Video::autoDetect = false;
   
   %shaderVer = getPixelShaderVersion();
   %intel = ( strstr( strupr( getDisplayDeviceInformation() ), "INTEL" ) != -1 ) ? true : false;
   %videoMem = GFXCardProfilerAPI::getVideoMemoryMB();
   
   return GraphicsQualityAutodetect_Apply( %shaderVer, %intel, %videoMem );
}

function GraphicsQualityAutodetect_Apply( %shaderVer, %intel, %videoMem )
{
   if ( %shaderVer < 2.0 )
   {      
      return "Your video card does not meet the minimum requirment of shader model 2.0.";
   }
   
   if ( %shaderVer < 3.0 || %intel )
   {
      // Allow specular and normals for 2.0a and 2.0b
      if ( %shaderVer > 2.0 )
      {
         MeshQualityGroup-->Lowest.apply();
         TextureQualityGroup-->Lowest.apply();
         LightingQualityGroup-->Lowest.apply();
         ShaderQualityGroup-->Low.apply();   
      }
      else
      {
         MeshQualityGroup-->Lowest.apply();
         TextureQualityGroup-->Lowest.apply();
         LightingQualityGroup-->Lowest.apply();
         ShaderQualityGroup-->Lowest.apply();   
      }
   }   
   else
   {
      if ( %videoMem > 1000 )
      {
         MeshQualityGroup-->High.apply();
         TextureQualityGroup-->High.apply();
         LightingQualityGroup-->High.apply();
         ShaderQualityGroup-->High.apply();
      }
      else if ( %videoMem > 400 || %videoMem == 0 )
      {
         MeshQualityGroup-->Normal.apply();
         TextureQualityGroup-->Normal.apply();
         LightingQualityGroup-->Normal.apply();
         ShaderQualityGroup-->Normal.apply();
         
         if ( %videoMem == 0 )
            return "Torque was unable to detect available video memory. Applying 'Normal' quality.";
      }
      else
      {
         MeshQualityGroup-->Low.apply();
         TextureQualityGroup-->Low.apply();
         LightingQualityGroup-->Low.apply();
         ShaderQualityGroup-->Low.apply();
      }
   }
   
   return "Graphics quality settings have been auto detected.";
}