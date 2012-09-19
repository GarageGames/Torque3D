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

//------------------------------------------------------------------------------
// global vars
//------------------------------------------------------------------------------

$OPTIONS_MENU::NO_DEVICE      = -1; ///< Indicates there is no specified device
$OPTIONS_MENU::ROW_GFXDEVICE  = 0;  ///< Row to choose gfx device
$OPTIONS_MENU::ROW_GFXRES     = 1;  ///< Row to choose gfx resolution
$OPTIONS_MENU::ROW_FULLSCREEN = 2;  ///< Row to choose fullscreen or not
$OPTIONS_MENU::ROW_VOLUME     = 3;  ///< Row to set audio volume
$OPTIONS_MENU::ROW_AA         = 4;  ///< Row to set AA

//------------------------------------------------------------------------------
// OptionsMenu methods
//------------------------------------------------------------------------------

/// Callback when this gui is added to the sim.
function OptionsMenu::onAdd(%this)
{
   %deviceList = %this.getGfxDeviceList();
   %this.addRow("Adapter", %deviceList, true, "onGfxDeviceChange", 0, 0, false);

   %resList = %this.getGfxResList(isFullScreen());
   %this.addRow("Resolution", %resList, false, "", 0, -15);

   %yesNoList = "Yes\tNo";
   %this.addRow("Fullscreen", %yesNoList, true, "onFullscreenChange", 0, -15);

   %volumeList = %this.getSfxVolumeList();
   %this.addRow("Volume", %volumeList, false, "onVolumeChange", 0, -15, false);
   
   %aaList = %this.getAAList();
   %this.addRow("Antialias", %aaList, false, "", 0, -15);
}

/// Callback when the control wakes up.
function OptionsMenu::onWake(%this)
{
   %this.loadPrefs();
}

/// Initializes each row on this control to reflect the state of saved prefs.
function OptionsMenu::loadPrefs(%this)
{
   // init the display adapter chooser
   %this.selectOption($OPTIONS_MENU::ROW_GFXDEVICE, $pref::Video::displayDevice);
   OptionsInfoDisplay.displayGfxDevice();

   // init the display resolution chooser
   %currRes = getWords($pref::Video::mode, $WORD::RES_X, $WORD::RES_Y);
   %this.selectOption($OPTIONS_MENU::ROW_GFXRES, %currRes);

   // init the fullscreen chooser
   %fullscreen = (isFullScreen()) ? "Yes" : "No";
   %this.selectOption($OPTIONS_MENU::ROW_FULLSCREEN, %fullscreen);

   // init the master volume
   %volume = mRoundByFive($pref::Audio::masterVolume * 100);
   %this.selectOption($OPTIONS_MENU::ROW_VOLUME, %volume);
      
   %currAA = getWord($pref::Video::mode, $WORD::AA);      
   %this.selectOption($OPTIONS_MENU::ROW_AA, %currAA);
}

/// Gets a tab separated list of all the graphics devices available. The list
/// will not include the null device by default but you may request to have the
/// null device included. Devices are listed by type.
///
/// \param %includeNull (bool) [optional] Specify true to include the null
/// device in the list. Default is false.
/// \return (string) A tab separated list of the avilable graphics devices.
function OptionsMenu::getGfxDeviceList(%this, %includeNull)
{
   %count = GFXInit::getAdapterCount();
   %list = "";
   for (%i = 0; %i < %count; %i++)
   {
      %type = GFXInit::getAdapterType(%i);
      if ((%includeNull) || (%type !$= "NullDevice"))
      {
         %list = %list TAB %type;
      }
   }
   return trim(%list);
}

/// Gets a tab separated list of available graphics resolutions. The resolutions
/// will be those available for the settings currently shown on the options
/// screen.
///
/// \param %fullscreen (bool) Specify true to get a list of all resolutions or
/// false to prune out resolutions that won't fit nicely on the desktop.
/// \return (string) A tab separated list of available graphics resolutions.
function OptionsMenu::getGfxResList(%this, %fullscreen)
{
   %type = %this.getCurrentOption($OPTIONS_MENU::ROW_GFXDEVICE);
   %adapter = getGfxDeviceIndex(%type);

   %deskRes = getDesktopResolution();
   %deskResX = firstWord(%deskRes);
   %deskResY = getWord(%deskRes, 1);

   %list = "";
   %count = GFXInit::getAdapterModeCount(%adapter);
   for (%i = 0; %i < %count; %i++)
   {
      %rawRes = GFXInit::getAdapterMode(%adapter, %i);
      %resX = firstWord(%rawRes);
      %resY = getWord(%rawRes, 1);
      if (%fullscreen || ((%resX < %deskResX) && (%resY < %deskResY)))
      {
         %res = %resX SPC %resY;
         if (! listHasElement(%list, %res))
         {
            %list = %list TAB %res;
         }
      }
   }

   return trim(%list);
}

/// Refreshes the list of resolutions on the control taking into account the
/// displayed settings for device and fullscreen.
function OptionsMenu::refreshResolutions(%this)
{
   %oldRes = %this.getCurrentOption($OPTIONS_MENU::ROW_GFXRES);
   %fullscreen = (%this.getCurrentOption($OPTIONS_MENU::ROW_FULLSCREEN) $= "Yes");
   %newResList = %this.getGfxResList(%fullscreen);
   %this.setOptions($OPTIONS_MENU::ROW_GFXRES, %newResList);
   %this.selectOption($OPTIONS_MENU::ROW_GFXRES, %oldRes);
}

/// Builds a list of volume increments suitable for listing on the volume
/// control. List will be from 0 to 100, incrementing by 5.
function OptionsMenu::getSfxVolumeList(%this)
{
   %start = 0;
   %end = 100;
   %inc = 5;

   %list = "";
   for (%i = %start; %i <= %end; %i += %inc)
   {
      %list = %list TAB %i;
   }

   return trim(%list);
}

function OptionsMenu::getAAList(%this)
{
   %type = %this.getCurrentOption($OPTIONS_MENU::ROW_GFXDEVICE);
   %adapter = getGfxDeviceIndex(%type);

   %list = "";
   %count = GFXInit::getAdapterModeCount(%adapter);
   %maxAA = 0;
   for (%i = 0; %i < %count; %i++)
   {
      %rawRes = GFXInit::getAdapterMode(%adapter, %i);       
      %aa = getWord(%rawRes, $WORD::AA);
      if (%aa > %maxAA)
         %maxAA = %aa;
   }
   
   %list = "";
   for (%i = 0; %i <= %maxAA; %i++)
   {
      %list = %list TAB %i;
   }
   return trim(%list);
}

//------------------------------------------------------------------------------
// OptionsButtonHolder methods
//------------------------------------------------------------------------------

function OptionsButtonHolder::onWake(%this)
{
   %this.add(GamepadButtonsGui);

   GamepadButtonsGui.setButton($BUTTON_A, "Apply Changes", OptionsMenu.CallbackOnA);
   GamepadButtonsGui.setButton($BUTTON_B, "Go Back", OptionsMenu.CallbackOnB);
   GamepadButtonsGui.setButton($BUTTON_Y, "Revert Options", OptionsMenu.CallbackOnY);
}

//------------------------------------------------------------------------------
// OptionsInfoDisplay methods
//------------------------------------------------------------------------------

/// Updates the control to display information on the display device selected on
/// the control for choosing a display device.
function OptionsInfoDisplay::displayGfxDevice(%this)
{
   %type = OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_GFXDEVICE);
   %index = getGfxDeviceIndex(%type);
   if (%index != $OPTIONS_MENU::NO_DEVICE)
   {
      %name = GFXInit::getAdapterName(%index);
      %shader = GFXInit::getAdapterShaderModel(%index);
   }
   else
   {
      %name = "<spush><color:ff0000>No valid device of that type<spop>";
      %shader = "<spush><color:ff0000>NA<spop>";
   }
   %this.setText("");
   %this.addText("<color:666666><tab:80>", false);
   %this.addText("Graphics Device<br>", false);
   %this.addText("Type:" TAB %type @ "<br>", false);
   %this.addText("Device:" TAB %name @ "<br>", false);
   %this.addText("Shader:" TAB %shader @ "<br>", false);
   %this.forceReflow();
}

//------------------------------------------------------------------------------
// callbacks from OptionsMenu
//------------------------------------------------------------------------------

/// Callback when the graphics device is changed. This will refresh the lists of
/// resolutions and other related settings.
///
/// \param %direction (string) "LEFT" or "RIGHT" indicating the direction the
/// option changed.
function onGfxDeviceChange(%direction)
{
   OptionsInfoDisplay.displayGfxDevice();
   $ThisControl.refreshResolutions();
}

/// Callback when the fullscreen setting is changed. Refreshes the list of
/// resolutions to reflect what is available.
///
/// \param %direction (string) "LEFT" or "RIGHT" indicating the direction the
/// option changed.
function onFullscreenChange(%direction)
{
   $ThisControl.refreshResolutions();
}

/// Callback when the volume setting is changed.
///
/// \param %direction (string) "LEFT" or "RIGHT" indicating the direction the
/// option changed.
function onVolumeChange(%direction)
{
   %volume = (OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_VOLUME) / 100);
   sfxSetMasterVolume(%volume);
   // TODO: play sample audio blip for user feedback
}

//------------------------------------------------------------------------------
// global methods
//------------------------------------------------------------------------------

/// Applies the options that have been set.
function applyOptions()
{
   // set the audio options
   %rawVolume = OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_VOLUME);
   $pref::Audio::masterVolume = %rawVolume / 100;

   // set the new video mode.
   %newRes = OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_GFXRES);
   %newFs = (OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_FULLSCREEN) $= "Yes");
   %newBpp = "32";
   %rate = getWord($pref::Video::mode, $WORD::REFRESH);
   %aa = OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_AA);

   // even though pref is set by "setVideoMode" we must set it here to handle
   // the case of a restart to apply new driver settings
   $pref::Video::mode = %newRes SPC %newFs SPC %newBpp SPC %rate SPC %aa;
   
   // check if Torque will require a restart for the new driver
   %oldDriver = $pref::Video::displayDevice;
   $pref::Video::displayDevice = OptionsMenu.getCurrentOption($OPTIONS_MENU::ROW_GFXDEVICE);
   if (%oldDriver !$= $pref::Video::displayDevice)
   {
      MessageBoxOKCancel("Change Video Device Now?",
         "Changing your video device requires Torque to be restarted. Selecting \"Ok\" will restart Torque with the new settings. Selecting \"Cancel\" will keep the new video settings and apply them next time you restart Torque.",
         "restartInstance();", "");
   }

   $pref::Video::mode = %newRes SPC %newFs SPC %newBpp SPC %rate SPC %aa;
   configureCanvas();
}

/// Reverts all options to the saved preferences.
function revertOptions()
{
   OptionsMenu.loadPrefs();
}

/// Looks up the system index of the gfx device from the type.
///
/// \param %type (string) A string representing the system type of the adapter.
/// \return (int) The index of the adapter if it is found or if there is no
/// device with that type it returns $OPTIONS_MENU::NO_DEVICE
function getGfxDeviceIndex(%type)
{
   %count = GFXInit::getAdapterCount();
   for (%i = 0; %i < %count; %i++)
   {
      %otherType = GFXInit::getAdapterType(%i);
      if (%type $= %otherType)
      {
         return %i;
      }
   }

   return $OPTIONS_MENU::NO_DEVICE;
}

/// Determines if the list contains the given element.
///
/// \return (bool) True if the element is found in the list, false if it is not.
function listHasElement(%list, %element)
{
   %count = getFieldCount(%list);
   for (%i = 0; %i < %count; %i++)
   {
      %word = getField(%list, %i);
      if (%word $= %element)
      {
         return true;
      }
   }

   return false;
}

/// Rounds %n to the nearest whole integer ending in a multiple of five.
///
/// \param %n (int or float) The number to round.
/// \return (int) %n rounded to the nearest multiple of five.
function mRoundByFive(%n)
{
   return (mFloor((%n + 2.5) / 5) * 5);
}

/// Determines if the saved preferences indicate running in fullscreen mode.
///
/// \return (bool) True if the preferences are saved to run in fullscreen or
/// false if saved to run windowed.
function isFullScreen()
{
   %fullscreen = getWord($pref::Video::mode, $WORD::FULLSCREEN);
   return (%fullscreen $= "true");
}

/// Sets the preference for fullscreen to the indicated value.
///
/// \param %bool (bool) Specify true to set the preference to fullscreen, false
/// to set it to windowed.
function setFullScreen(%bool)
{
   $pref::Video::resolution = setWord($pref::Video::mode, $WORD::FULLSCREEN, (%bool ? "true" : "false"));
}
