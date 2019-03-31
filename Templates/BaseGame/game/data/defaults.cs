$pref::Player::Name = "Visitor";
$pref::Player::defaultFov = 75;
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
$pref::Input::invertVerticalMouse = false;
$pref::Input::VertMouseSensitivity = 1;
$pref::Input::HorzMouseSensitivity = 1;
$pref::Input::RollMouseSensitivity = 1;
$pref::Input::ZoomVertMouseSensitivity = 0.3;
$pref::Input::ZoomHorzMouseSensitivity = 0.3;

$sceneLighting::cacheSize = 20000;
$sceneLighting::purgeMethod = "lastCreated";
$sceneLighting::cacheLighting = 1;

$pref::Video::displayDevice = "D3D11";
$pref::Video::disableVerticalSync = 1;
$pref::Video::Resolution = "1024 768";
$pref::Video::FullScreen = false;
$pref::Video::BitDepth = "32";
$pref::Video::RefreshRate = "60";
$pref::Video::AA = "4";
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

$pref::Video::Gamma = 2.2;
$pref::Video::Contrast = 1.0;
$pref::Video::Brightness = 0;

/// The perfered light manager to use at startup.  If blank
/// or if the selected one doesn't work on this platfom it
/// will try the defaults below.
$pref::lightManager = "";

/// This is the default list of light managers ordered from
/// most to least desirable for initialization.
$lightManager::defaults = "Advanced Lighting";

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

$pref::SFX::channelVolume[1] = 1;
$pref::SFX::channelVolume[2] = 1;
$pref::SFX::channelVolume[3] = 1;
$pref::SFX::channelVolume[4] = 1;

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

$PostFXManager::Settings::EnableDOF = "0";
$PostFXManager::Settings::DOF::BlurCurveFar = "";
$PostFXManager::Settings::DOF::BlurCurveNear = "";
$PostFXManager::Settings::DOF::BlurMax = "";
$PostFXManager::Settings::DOF::BlurMin = "";
$PostFXManager::Settings::DOF::EnableAutoFocus = "";
$PostFXManager::Settings::DOF::EnableDOF = "";
$PostFXManager::Settings::DOF::FocusRangeMax = "";
$PostFXManager::Settings::DOF::FocusRangeMin = "";

$PostFXManager::Settings::EnableLightRays = "0";
$PostFXManager::Settings::LightRays::brightScalar = "0.75";
$PostFXManager::Settings::LightRays::decay = "1.0";
$PostFXManager::Settings::LightRays::density = "0.94";
$PostFXManager::Settings::LightRays::numSamples = "40";
$PostFXManager::Settings::LightRays::weight = "5.65";

$PostFXManager::Settings::EnableDOF = 1;
$pref::PostFX::EnableVignette = 1;
$pref::PostFX::EnableLightRays = 1;
$pref::PostFX::EnableHDR = 1;
$pref::PostFX::EnableSSAO = 1;