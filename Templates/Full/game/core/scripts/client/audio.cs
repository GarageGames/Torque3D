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
//    Source groups.
//-----------------------------------------------------------------------------

singleton SFXDescription( AudioMaster );
singleton SFXSource( AudioChannelMaster )
{
   description = AudioMaster;
};

singleton SFXDescription( AudioChannel )
{
   sourceGroup = AudioChannelMaster;
};

singleton SFXSource( AudioChannelDefault )
{
   description = AudioChannel;
};
singleton SFXSource( AudioChannelGui )
{
   description = AudioChannel;
};
singleton SFXSource( AudioChannelEffects )
{
   description = AudioChannel;
};
singleton SFXSource( AudioChannelMessages )
{
   description = AudioChannel;
};
singleton SFXSource( AudioChannelMusic )
{
   description = AudioChannel;
};

// Set default playback states of the channels.

AudioChannelMaster.play();
AudioChannelDefault.play();

AudioChannelGui.play();
AudioChannelMusic.play();
AudioChannelMessages.play();

// Stop in-game effects channels.
AudioChannelEffects.stop();

//-----------------------------------------------------------------------------
//    Master SFXDescriptions.
//-----------------------------------------------------------------------------

// Master description for interface audio.
singleton SFXDescription( AudioGui )
{
   volume         = 1.0;
   sourceGroup    = AudioChannelGui;
};

// Master description for game effects audio.
singleton SFXDescription( AudioEffect )
{
   volume         = 1.0;
   sourceGroup    = AudioChannelEffects;
};

// Master description for audio in notifications.
singleton SFXDescription( AudioMessage )
{
   volume         = 1.0;
   sourceGroup    = AudioChannelMessages;
};

// Master description for music.
singleton SFXDescription( AudioMusic )
{
   volume         = 1.0;
   sourceGroup    = AudioChannelMusic;
};

//-----------------------------------------------------------------------------
//    SFX Functions.
//-----------------------------------------------------------------------------

/// This initializes the sound system device from
/// the defaults in the $pref::SFX:: globals.
function sfxStartup()
{   
   // The console builds should re-detect, by default, so that it plays nicely 
   // along side a PC build in the same script directory.
   
   if( $platform $= "xenon" )
   {
      if( $pref::SFX::provider $= "DirectSound" || 
          $pref::SFX::provider $= "OpenAL" )
      {
         $pref::SFX::provider = "";
      }
      
      if( $pref::SFX::provider $= "" )
      {
         $pref::SFX::autoDetect = 1;
         
         warn( "Xbox360 is auto-detecting available sound providers..." ); 
         warn( "   - You may wish to alter this functionality before release (core/scripts/client/audio.cs)" );
      }
   }

   echo( "sfxStartup..." );
   
   // If we have a provider set, try initialize a device now.
   
   if( $pref::SFX::provider !$= "" )
   {
      if( sfxInit() )
         return;
      else
      {
         // Force auto-detection.
         $pref::SFX::autoDetect = true;
      }
   }

   // If enabled autodetect a safe device.

   if( ( !isDefined( "$pref::SFX::autoDetect" ) || $pref::SFX::autoDetect ) &&
       sfxAutodetect() )
      return;
   
   // Failure.

   error( "   Failed to initialize device!\n\n" );
   
   $pref::SFX::provider = "";
   $pref::SFX::device   = "";
   
   return;
}


/// This initializes the sound system device from
/// the defaults in the $pref::SFX:: globals.
function sfxInit()
{
   // If already initialized, shut down the current device first.
   
   if( sfxGetDeviceInfo() !$= "" )
      sfxShutdown();
      
   // Start it up!
   %maxBuffers = $pref::SFX::useHardware ? -1 : $pref::SFX::maxSoftwareBuffers;
   if ( !sfxCreateDevice( $pref::SFX::provider, $pref::SFX::device, $pref::SFX::useHardware, %maxBuffers ) )
      return false;

   // This returns a tab seperated string with
   // the initialized system info.
   %info = sfxGetDeviceInfo();
   $pref::SFX::provider       = getField( %info, 0 );
   $pref::SFX::device         = getField( %info, 1 );
   $pref::SFX::useHardware    = getField( %info, 2 );
   %useHardware               = $pref::SFX::useHardware ? "Yes" : "No";
   %maxBuffers                = getField( %info, 3 );
   
   echo( "   Provider: "    @ $pref::SFX::provider );
   echo( "   Device: "      @ $pref::SFX::device );
   echo( "   Hardware: "    @ %useHardware );
   echo( "   Buffers: "      @ %maxBuffers );
   
   if( isDefined( "$pref::SFX::distanceModel" ) )
      sfxSetDistanceModel( $pref::SFX::distanceModel );
   if( isDefined( "$pref::SFX::dopplerFactor" ) )
      sfxSetDopplerFactor( $pref::SFX::dopplerFactor );
   if( isDefined( "$pref::SFX::rolloffFactor" ) )
      sfxSetRolloffFactor( $pref::SFX::rolloffFactor );

   // Restore master volume.
   
   sfxSetMasterVolume( $pref::SFX::masterVolume );

   // Restore channel volumes.
   
   for( %channel = 0; %channel <= 8; %channel ++ )
      sfxSetChannelVolume( %channel, $pref::SFX::channelVolume[ %channel ] );
      
   return true;
}


/// Destroys the current sound system device.
function sfxShutdown()
{
   // Store volume prefs.
   
   $pref::SFX::masterVolume = sfxGetMasterVolume();
   
   for( %channel = 0; %channel <= 8; %channel ++ )
      $pref::SFX::channelVolume[ %channel ] = sfxGetChannelVolume( %channel );
   
   // We're assuming here that a null info 
   // string means that no device is loaded.
   if( sfxGetDeviceInfo() $= "" )
      return;

   sfxDeleteDevice();
}


/// Determines which of the two SFX providers is preferable.
function sfxCompareProvider( %providerA, %providerB )
{
   if( %providerA $= %providerB )
      return 0;
      
   switch$( %providerA )
   {
      // Always prefer FMOD over anything else.
      case "FMOD":
         return 1;
         
      // Prefer OpenAL over anything but FMOD.
      case "OpenAL":
         if( %providerB $= "FMOD" )
            return -1;
         else
            return 1;
            
      // As long as the XAudio SFX provider still has issues,
      // choose stable DSound over it.
      case "DirectSound":
         if( %providerB $= "FMOD" || %providerB $= "OpenAL" )
            return -1;
         else
            return 0;
            
      case "XAudio":
         if( %providerB !$= "FMOD" && %providerB !$= "OpenAL" && %providerB !$= "DirectSound" )
            return 1;
         else
            return -1;
         
      default:
         return -1;
   }
}


/// Try to detect and initalize the best SFX device available.
function sfxAutodetect()
{
   // Get all the available devices.
   
   %devices = sfxGetAvailableDevices();

   // Collect and sort the devices by preferentiality.
   
   %deviceTrySequence = new ArrayObject();
   %bestMatch = -1;
   %count = getRecordCount( %devices );
   for( %i = 0; %i < %count; %i ++ )
   {
      %info = getRecord( %devices, %i );
      %provider = getField( %info, 0 );
         
      %deviceTrySequence.push_back( %provider, %info );
   }
   
   %deviceTrySequence.sortfkd( "sfxCompareProvider" );
         
   // Try the devices in order.
   
   %count = %deviceTrySequence.count();
   for( %i = 0; %i < %count; %i ++ )
   {
      %provider = %deviceTrySequence.getKey( %i );
      %info = %deviceTrySequence.getValue( %i );
      
      $pref::SFX::provider       = %provider;
      $pref::SFX::device         = getField( %info, 1 );
      $pref::SFX::useHardware    = getField( %info, 2 );
      
      // By default we've decided to avoid hardware devices as
      // they are buggy and prone to problems.
      $pref::SFX::useHardware = false;

      if( sfxInit() )
      {
         $pref::SFX::autoDetect = false;
         %deviceTrySequence.delete();
         return true;
      }
   }
   
   // Found no suitable device.
   
   error( "sfxAutodetect - Could not initialize a valid SFX device." );
   
   $pref::SFX::provider = "";
   $pref::SFX::device = "";
   $pref::SFX::useHardware = "";
   
   %deviceTrySequence.delete();
   
   return false;
}


//-----------------------------------------------------------------------------
//    Backwards-compatibility with old channel system.
//-----------------------------------------------------------------------------

// Volume channel IDs for backwards-compatibility.

$GuiAudioType        = 1;  // Interface.
$SimAudioType        = 2;  // Game.
$MessageAudioType    = 3;  // Notifications.
$MusicAudioType      = 4;  // Music.

$AudioChannels[ 0 ] = AudioChannelDefault;
$AudioChannels[ $GuiAudioType ] = AudioChannelGui;
$AudioChannels[ $SimAudioType ] = AudioChannelEffects;
$AudioChannels[ $MessageAudioType ] = AudioChannelMessages;
$AudioChannels[ $MusicAudioType ] = AudioChannelMusic;

function sfxOldChannelToGroup( %channel )
{
   return $AudioChannels[ %channel ];
}

function sfxGroupToOldChannel( %group )
{
   %id = %group.getId();
   for( %i = 0;; %i ++ )
      if( !isObject( $AudioChannels[ %i ] ) )
         return -1;
      else if( $AudioChannels[ %i ].getId() == %id )
         return %i;
         
   return -1;
}

function sfxSetMasterVolume( %volume )
{
   AudioChannelMaster.setVolume( %volume );
}

function sfxGetMasterVolume( %volume )
{
   return AudioChannelMaster.getVolume();
}

function sfxStopAll( %channel )
{
   // Don't stop channel itself since that isn't quite what the function
   // here intends.
   
   %channel = sfxOldChannelToGroup( %channel );
   if (isObject(%channel))
   {
      foreach( %source in %channel )
         %source.stop();
   }
}

function sfxGetChannelVolume( %channel )
{
   %obj = sfxOldChannelToGroup( %channel );
   if( isObject( %obj ) )
      return %obj.getVolume();
}

function sfxSetChannelVolume( %channel, %volume )
{
   %obj = sfxOldChannelToGroup( %channel );
   if( isObject( %obj ) )
      %obj.setVolume( %volume );
}

singleton SimSet( SFXPausedSet );


/// Pauses the playback of active sound sources.
/// 
/// @param %channels    An optional word list of channel indices or an empty 
///                     string to pause sources on all channels.
/// @param %pauseSet    An optional SimSet which is filled with the paused 
///                     sources.  If not specified the global SfxSourceGroup 
///                     is used.
///
/// @deprecated
///
function sfxPause( %channels, %pauseSet )
{
   // Did we get a set to populate?
   if ( !isObject( %pauseSet ) )
      %pauseSet = SFXPausedSet;
      
   %count = SFXSourceSet.getCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %source = SFXSourceSet.getObject( %i );

      %channel = sfxGroupToOldChannel( %source.getGroup() );
      if( %channels $= "" || findWord( %channels, %channel ) != -1 )
      {
         %source.pause();
         %pauseSet.add( %source );
      }
   }
}


/// Resumes the playback of paused sound sources.
/// 
/// @param %pauseSet    An optional SimSet which contains the paused sound 
///                     sources to be resumed.  If not specified the global 
///                     SfxSourceGroup is used.
/// @deprecated
///
function sfxResume( %pauseSet )
{
   if ( !isObject( %pauseSet ) )
      %pauseSet = SFXPausedSet;
                  
   %count = %pauseSet.getCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %source = %pauseSet.getObject( %i );
      %source.play();
   }

   // Clear our pause set... the caller is left
   // to clear his own if he passed one.
   %pauseSet.clear();
}
