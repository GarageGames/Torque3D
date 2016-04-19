// =============================================================================
// KEYBINDS MENU
// =============================================================================
$RemapCount = 0;
$RemapName[$RemapCount] = "Forward";
$RemapCmd[$RemapCount] = "moveforward";
$RemapGroup[$RemapCount] = "Movement";
$RemapCount++;
$RemapName[$RemapCount] = "Backward";
$RemapCmd[$RemapCount] = "movebackward";
$RemapGroup[$RemapCount] = "Movement";
$RemapCount++;
$RemapName[$RemapCount] = "Strafe Left";
$RemapCmd[$RemapCount] = "moveleft";
$RemapGroup[$RemapCount] = "Movement";
$RemapCount++;
$RemapName[$RemapCount] = "Strafe Right";
$RemapCmd[$RemapCount] = "moveright";
$RemapGroup[$RemapCount] = "Movement";
$RemapCount++;
$RemapName[$RemapCount] = "Jump";
$RemapCmd[$RemapCount] = "jump";
$RemapGroup[$RemapCount] = "Movement";
$RemapCount++;

$RemapName[$RemapCount] = "Fire Weapon";
$RemapCmd[$RemapCount] = "mouseFire";
$RemapGroup[$RemapCount] = "Combat";
$RemapCount++;
$RemapName[$RemapCount] = "Adjust Zoom";
$RemapCmd[$RemapCount] = "setZoomFov";
$RemapGroup[$RemapCount] = "Combat";
$RemapCount++;
$RemapName[$RemapCount] = "Toggle Zoom";
$RemapCmd[$RemapCount] = "toggleZoom";
$RemapGroup[$RemapCount] = "Combat";
$RemapCount++;

$RemapName[$RemapCount] = "Free Look";
$RemapCmd[$RemapCount] = "toggleFreeLook";
$RemapGroup[$RemapCount] = "Miscellaneous";
$RemapCount++;
$RemapName[$RemapCount] = "Switch 1st/3rd";
$RemapCmd[$RemapCount] = "toggleFirstPerson";
$RemapGroup[$RemapCount] = "Miscellaneous";
$RemapCount++;
$RemapName[$RemapCount] = "Toggle Camera";
$RemapCmd[$RemapCount] = "toggleCamera";
$RemapGroup[$RemapCount] = "Miscellaneous";
$RemapCount++;

function ControlsMenu::onWake(%this)
{
   ControlSetList.clear();
   ControlSetList.add( "Movement", "Movement" );
   ControlSetList.add( "Combat", "Combat" );
   ControlSetList.add( "Miscellaneous", "Miscellaneous" );
   
   ControlSetList.setSelected( "Movement", false );
   
   ControlsMenuOptionsArray.clear();
   ControlsMenu.loadGroupKeybinds("Movement");
   ControlsMenuOptionsArray.refresh();
}

function ControlSetList::onSelect( %this, %id, %text )
{
   ControlsMenuOptionsArray.clear();
   
   if(%text $= "Movement")
      ControlsMenu.loadGroupKeybinds("Movement");
   else if(%text $= "Combat")
      ControlsMenu.loadGroupKeybinds("Combat");
   else if(%text $= "Miscellaneous")
      ControlsMenu.loadGroupKeybinds("Miscellaneous");

    ControlsMenuOptionsArray.refresh();
}

function ControlsMenuOKButton::onClick(%this)
{
    // write out the control config into the keybinds.cs file
    %prefPath = getPrefpath();
    moveMap.save( %prefPath @ "/keybinds.cs" );
   
    OptionsMenu.backOut();
}

function ControlsMenuDefaultsButton::onClick(%this)
{
   //For this to work with module-style, we have to figure that somewhere, we'll set where our default keybind script is at.
   //This can be hardcoded in your actual project.
   exec($KeybindPath);
   ControlsMenu.reload();
}

function ControlsMenu::loadGroupKeybinds(%this, %keybindGroup)
{
   %optionIndex = 0;
   for(%i=0; %i < $RemapCount; %i++)
   {
      //find and add all the keybinds for the particular group we're looking at
      if($RemapGroup[%i] $= %keybindGroup)
      {
         %temp = %this.getKeybindString(%i);
         
         %option = %this.addKeybindOption();
         %option-->nameText.setText($RemapName[%i]);
         %option-->rebindButton.setText(%temp);
         %option-->rebindButton.keybindIndex = %i;
         %option-->rebindButton.optionIndex = %optionIndex;
         %optionIndex++;
      }
   }
}

function ControlsMenu::addKeybindOption(%this)
{
    %tamlReader = new Taml();
   
    %graphicsOption = %tamlReader.read("data/ui/scripts/guis/controlsMenuSetting.taml");

    ControlsMenuOptionsArray.add(%graphicsOption);

    return %graphicsOption;
}

function ControlsMenu::getKeybindString(%this, %index )
{
   %name       = $RemapName[%index];
   %cmd        = $RemapCmd[%index];

   %temp = moveMap.getBinding( %cmd );
   if ( %temp $= "" )
      return %name TAB "";

   %mapString = "";

   %count = getFieldCount( %temp );
   for ( %i = 0; %i < %count; %i += 2 )
   {
      %device = getField( %temp, %i + 0 );
      %object = getField( %temp, %i + 1 );
      
      %displayName = %this.getMapDisplayName( %device, %object );
      
      if(%displayName !$= "")
      {
         %tmpMapString = %this.getMapDisplayName( %device, %object );
         
         if(%mapString $= "")
         {
            %mapString = %tmpMapString;
         }
         else
         {
            if ( %tmpMapString !$= "")
            {
               %mapString = %mapString @ ", " @ %tmpMapString;
            }
         }
      }
   }

   return %mapString; 
}

function ControlsMenu::redoMapping( %device, %action, %cmd, %oldIndex, %newIndex )
{
	//%actionMap.bind( %device, %action, $RemapCmd[%newIndex] );
	moveMap.bind( %device, %action, %cmd );
	
   %remapList = %this-->OptRemapList;
	%remapList.setRowById( %oldIndex, buildFullMapString( %oldIndex ) );
	%remapList.setRowById( %newIndex, buildFullMapString( %newIndex ) );
}

function ControlsMenu::getMapDisplayName( %this, %device, %action )
{
	if ( %device $= "keyboard" )
		return( %action );		
	else if ( strstr( %device, "mouse" ) != -1 )
	{
		// Substitute "mouse" for "button" in the action string:
		%pos = strstr( %action, "button" );
		if ( %pos != -1 )
		{
			%mods = getSubStr( %action, 0, %pos );
			%object = getSubStr( %action, %pos, 1000 );
			%instance = getSubStr( %object, strlen( "button" ), 1000 );
			return( %mods @ "mouse" @ ( %instance + 1 ) );
		}
		else
			error( "Mouse input object other than button passed to getDisplayMapName!" );
	}
	else if ( strstr( %device, "joystick" ) != -1 )
	{
		// Substitute "joystick" for "button" in the action string:
		%pos = strstr( %action, "button" );
		if ( %pos != -1 )
		{
			%mods = getSubStr( %action, 0, %pos );
			%object = getSubStr( %action, %pos, 1000 );
			%instance = getSubStr( %object, strlen( "button" ), 1000 );
			return( %mods @ "joystick" @ ( %instance + 1 ) );
		}
		else
	   { 
	      %pos = strstr( %action, "pov" );
         if ( %pos != -1 )
         {
            %wordCount = getWordCount( %action );
            %mods = %wordCount > 1 ? getWords( %action, 0, %wordCount - 2 ) @ " " : "";
            %object = getWord( %action, %wordCount - 1 );
            switch$ ( %object )
            {
               case "upov":   %object = "POV1 up";
               case "dpov":   %object = "POV1 down";
               case "lpov":   %object = "POV1 left";
               case "rpov":   %object = "POV1 right";
               case "upov2":  %object = "POV2 up";
               case "dpov2":  %object = "POV2 down";
               case "lpov2":  %object = "POV2 left";
               case "rpov2":  %object = "POV2 right";
               default:       %object = "";
            }
            return( %mods @ %object );
         }
         else
            error( "Unsupported Joystick input object passed to getDisplayMapName!" );
      }
	}
		
	return( "" );		
}

function ControlsMenu::buildFullMapString( %this, %index )
{
   %name       = $RemapName[%index];
   %cmd        = $RemapCmd[%index];

   %temp = moveMap.getBinding( %cmd );
   if ( %temp $= "" )
      return %name TAB "";

   %mapString = "";

   %count = getFieldCount( %temp );
   for ( %i = 0; %i < %count; %i += 2 )
   {
      if ( %mapString !$= "" )
         %mapString = %mapString @ ", ";

      %device = getField( %temp, %i + 0 );
      %object = getField( %temp, %i + 1 );
      %mapString = %mapString @ %this.getMapDisplayName( %device, %object );
   }

   return %name TAB %mapString; 
}

function ControlsMenu::fillRemapList( %this )
{
   %remapList = %this-->OptRemapList;
   
	%remapList.clear();
   for ( %i = 0; %i < $RemapCount; %i++ )
      %remapList.addRow( %i, %this.buildFullMapString( %i ) );
}

function ControlsMenu::doRemap( %this )
{
   %remapList = %this-->OptRemapList;
   
	%selId = %remapList.getSelectedId();
   %name = $RemapName[%selId];

	RemapDlg-->OptRemapText.setValue( "Re-bind \"" @ %name @ "\" to..." );
	OptRemapInputCtrl.index = %selId;
	Canvas.pushDialog( RemapDlg );
}

function ControlsMenuRebindButton::onClick(%this)
{
   %name = $RemapName[%this.keybindIndex];
   RemapDlg-->OptRemapText.setValue( "Re-bind \"" @ %name @ "\" to..." );
   
   OptRemapInputCtrl.index = %this.keybindIndex;
   OptRemapInputCtrl.optionIndex = %this.optionIndex;
   Canvas.pushDialog( RemapDlg );
}

function OptRemapInputCtrl::onInputEvent( %this, %device, %action )
{
   //error( "** onInputEvent called - device = " @ %device @ ", action = " @ %action @ " **" );
   Canvas.popDialog( RemapDlg );

   // Test for the reserved keystrokes:
   if ( %device $= "keyboard" )
   {
      // Cancel...
      if ( %action $= "escape" )
      {
         // Do nothing...
         return;
      }
   }

   %cmd  = $RemapCmd[%this.index];
   %name = $RemapName[%this.index];

   // Grab the friendly display name for this action
   // which we'll use when prompting the user below.
   %mapName = ControlsMenu.getMapDisplayName( %device, %action );
   
   // Get the current command this action is mapped to.
   %prevMap = moveMap.getCommand( %device, %action );

   // If nothing was mapped to the previous command 
   // mapping then it's easy... just bind it.
   if ( %prevMap $= "" )
   {
      ControlsMenu.unbindExtraActions( %cmd, 1 );
      moveMap.bind( %device, %action, %cmd );
      
      //ControlsMenu.reload();
      %newCommands = getField(ControlsMenu.buildFullMapString( %this.index ), 1);
      ControlsMenuOptionsArray.getObject(%this.optionIndex)-->rebindButton.setText(%newCommands);
      return;
   }

   // If the previous command is the same as the 
   // current then they hit the same input as what
   // was already assigned.
   if ( %prevMap $= %cmd )
   {
      ControlsMenu.unbindExtraActions( %cmd, 0 );
      moveMap.bind( %device, %action, %cmd );

      //ControlsMenu.reload();
      %newCommands = getField(ControlsMenu.buildFullMapString( %this.index ), 1);
      ControlsMenuOptionsArray.getObject(%this.optionIndex)-->rebindButton.setText(%newCommands);
      return;   
   }

   // Look for the index of the previous mapping.
   %prevMapIndex = ControlsMenu.findRemapCmdIndex( %prevMap );
   
   // If we get a negative index then the previous 
   // mapping was to an item that isn't included in
   // the mapping list... so we cannot unmap it.
   if ( %prevMapIndex == -1 )
   {
      MessageBoxOK( "Remap Failed", "\"" @ %mapName @ "\" is already bound to a non-remappable command!" );
      return;
   }

   // Setup the forced remapping callback command.
   %callback = "redoMapping(" @ %device @ ", \"" @ %action @ "\", \"" @
                              %cmd @ "\", " @ %prevMapIndex @ ", " @ %this.index @ ");";
   
   // Warn that we're about to remove the old mapping and
   // replace it with another.
   %prevCmdName = $RemapName[%prevMapIndex];
   Canvas.pushDialog( RemapConfirmDlg );
   
   RemapConfirmationText.setText("\"" @ %mapName @ "\" is already bound to \""
      @ %prevCmdName @ "\"! Do you wish to replace this mapping?");
   RemapConfirmationYesButton.command = "ControlsMenu.redoMapping(" @ %device @ ", \"" @ %action @ "\", \"" @
                              %cmd @ "\", " @ %prevMapIndex @ ", " @ %this.index @ "); Canvas.popDialog();";
   RemapConfirmationNoButton.command = "Canvas.popDialog();";
   
   /*MessageBoxYesNo( "Warning",
      "\"" @ %mapName @ "\" is already bound to \""
      @ %prevCmdName @ "\"!\nDo you wish to replace this mapping?",
       %callback, "" );*/
}

function ControlsMenu::findRemapCmdIndex( %this, %command )
{
	for ( %i = 0; %i < $RemapCount; %i++ )
	{
		if ( %command $= $RemapCmd[%i] )
			return( %i );			
	}
	return( -1 );	
}

/// This unbinds actions beyond %count associated to the
/// particular moveMap %commmand.
function ControlsMenu::unbindExtraActions( %this, %command, %count )
{
   %temp = moveMap.getBinding( %command );
   if ( %temp $= "" )
      return;

   %count = getFieldCount( %temp ) - ( %count * 2 );
   for ( %i = 0; %i < %count; %i += 2 )
   {
      %device = getField( %temp, %i + 0 );
      %action = getField( %temp, %i + 1 );
      
      moveMap.unbind( %device, %action );
   }
}

function ControlsMenu::redoMapping( %this, %device, %action, %cmd, %oldIndex, %newIndex )
{
	//%actionMap.bind( %device, %action, $RemapCmd[%newIndex] );
	moveMap.bind( %device, %action, %cmd );
	
   %remapList = %this-->OptRemapList;
	%remapList.setRowById( %oldIndex, %this.buildFullMapString( %oldIndex ) );
	%remapList.setRowById( %newIndex, %this.buildFullMapString( %newIndex ) );
	
	%this.changeSettingsPage();
}