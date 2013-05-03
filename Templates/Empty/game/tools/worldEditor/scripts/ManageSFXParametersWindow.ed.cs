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


//=============================================================================
//    Constants.
//=============================================================================

/// File to save newly created SFXParameters in by default.
$SFX_PARAMETER_FILE = "scripts/client/audioData.cs";

$SFX_PARAMETER_CHANNELS[  0 ] = "Volume";
$SFX_PARAMETER_CHANNELS[  1 ] = "Pitch";
$SFX_PARAMETER_CHANNELS[  2 ] = "Priority";
$SFX_PARAMETER_CHANNELS[  3 ] = "MinDistance";
$SFX_PARAMETER_CHANNELS[  4 ] = "MaxDistance";
$SFX_PARAMETER_CHANNELS[  5 ] = "ConeInsideAngle";
$SFX_PARAMETER_CHANNELS[  6 ] = "ConeOutsideAngle";
$SFX_PARAMETER_CHANNELS[  7 ] = "ConeOutsideVolume";
$SFX_PARAMETER_CHANNELS[  8 ] = "PositionX";
$SFX_PARAMETER_CHANNELS[  9 ] = "PositionY";
$SFX_PARAMETER_CHANNELS[ 10 ] = "PositionZ";
$SFX_PARAMETER_CHANNELS[ 11 ] = "RotationX";
$SFX_PARAMETER_CHANNELS[ 12 ] = "RotationY";
$SFX_PARAMETER_CHANNELS[ 13 ] = "RotationZ";
$SFX_PARAMETER_CHANNELS[ 14 ] = "VelocityX";
$SFX_PARAMETER_CHANNELS[ 15 ] = "VelocityY";
$SFX_PARAMETER_CHANNELS[ 16 ] = "VelocityZ";
$SFX_PARAMETER_CHANNELS[ 17 ] = "Cursor";
$SFX_PARAMETER_CHANNELS[ 18 ] = "User0";
$SFX_PARAMETER_CHANNELS[ 19 ] = "User1";
$SFX_PARAMETER_CHANNELS[ 20 ] = "User2";
$SFX_PARAMETER_CHANNELS[ 21 ] = "User3";

$SFX_PARAMETER_CHANNELS_COUNT = 22;

/// Interval (in milliseconds) between GUI updates.  Each update
/// syncs the displayed values to the actual parameter states.
$SFX_PARAMETERS_UPDATE_INTERVAL = 50;

//=============================================================================
//    EManageSFXParameters.
//=============================================================================

//-----------------------------------------------------------------------------

function EManageSFXParameters::createNewParameter( %this, %name )
{
   %parameter = new SFXParameter()
   {
      internalName = %name;
   };
   
   if( !isObject( %parameter ) )
      return;
   
   %parameter.setFilename( $SFX_PARAMETER_FILE );
   %this.persistenceMgr.setDirty( %parameter );
   %this.persistenceMgr.saveDirty();
   
   %this.addParameter( %parameter );
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::showDeleteParameterDlg( %this, %parameter )
{
   MessageBoxOkCancel( "Confirmation",
      "Really delete '" @ %parameter.getInternalName() @ "'?" NL
      "" NL
      "The parameter will be removed from the file '" @ %parameter.getFileName() @ "'.",
      %this @ ".deleteParameter( " @ %parameter @ " );"
   );
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::deleteParameter( %this, %parameter )
{
   %this.removeParameter( %parameter );
   if( %parameter.getFilename() !$= "" )
      %this.persistenceMgr.removeObjectFromFile( %parameter );
      
   %parameter.delete();
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::saveParameter( %this, %parameter )
{
   if( %parameter.getFilename() !$= "" )
   {
      if( !%this.persistenceMgr.isDirty( %parameter ) )
         %this.persistenceMgr.setDirty( %parameter );
         
      %this.persistenceMgr.saveDirty();
   }
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::onWake( %this )
{
   // If the parameter list is empty, add all SFXParameters in the
   // SFXParameterGroup to the list.
   
   if( %this-->SFXParametersStack.getCount() == 0 )
      %this.initList();
      
   if( !isObject( %this.persistenceMgr ) )
      %this.persistenceMgr = new PersistenceManager();
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::onVisible( %this, %value )
{
   if( %value )
   {
      // Schedule an update.

      %this.schedule( %SFX_PARAMETERS_UPDATE_INTERVAL, "update" );      
   }
}

//-----------------------------------------------------------------------------

/// Populate the parameter list with the currently defined SFXParameters.
function EManageSFXParameters::initList( %this, %filter )
{
   // Clear the current lists.
   
   %this-->SFXParametersStack.clear();
   
   // Add each SFXParameter in SFXParameterGroup.
   
   foreach( %obj in SFXParameterGroup )
   {
      if( !isMemberOfClass( %obj.getClassName(), "SFXParameter" ) )
         continue;
         
      // If we have a filter, search for it in the parameter's
      // categories.
         
      %matchesFilter = true;
      if( %filter !$= "" )
      {
         %matchesFilter = false;
         
         for( %idx = 0; %obj.categories[ %idx ] !$= ""; %idx ++  )
         {
            if( %obj.categories[ %idx ] $= %filter )
            {
               %matchesFilter = true;
               break;
            }
         }
      }
         
      if( %matchesFilter )
         %this.addParameter( %obj );
   }
   
   // Init the filters.
   
   %this.initFilterList( %filter );
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::initFilterList( %this, %selectFilter )
{
   %filterList = %this-->SFXParameterFilter;
   %filterList.clear();
   %filterList.add( "", 0 );

   foreach( %obj in SFXParameterGroup )
   {
      if( !isMemberOfClass( %obj.getClassName(), "SFXParameter" ) )
         continue;
         
      for( %idx = 0; %obj.categories[ %idx ] !$= ""; %idx ++ )
      {
         %category = %obj.categories[ %idx ];
         if( %filterList.findText( %category ) == -1 )
            %filterList.add( %category, %filterList.size() );
      }
   }

   // Sort the filters.
   
   %filterList.sort();
   %filterList.setSelected( %filterList.findText( %selectFilter ), false );
}

//-----------------------------------------------------------------------------

/// Parse the categories for the parameter from the given comma-separated list.
function EManageSFXParameters::updateParameterCategories( %this, %parameter, %list )
{
   %this.persistenceMgr.setDirty( %parameter );

   // Parse the list.

   %len = strlen( %list );
   %pos = 0;
   
   %idx = 0;
   while( %pos < %len )
   {
      %startPos = %pos;
      %pos = strchrpos( %list, ",", %pos );
      if( %pos == -1 )
         %pos = %len;
         
      if( %pos > %startPos )
      {
         %category = getSubStr( %list, %startPos, %pos - %startPos );
         %category = trim( %category );
         %parameter.categories[ %idx ] = %category;
         %idx ++;
      }
      
      %pos ++;
   }
   
   // Clear out excess categories existing from before.
   
   while( %parameter.categories[ %idx ] !$= "" )
   {
      %parameter.categories[ %idx ] = "";
      %this.persistenceMgr.removeField( %parameter, "categories" @ %idx );
      %idx ++;
   }
   
   // Save the parameter.
   
   %this.saveParameter( %parameter );
   
   // Re-initialize the filter list.
   
   %this.initFilterList( %this-->SFXParameterFilter.getText() );
}

//-----------------------------------------------------------------------------

/// Add a new SFXParameter to the list.
function EManageSFXParameters::addParameter( %this, %parameter )
{
   %ctrl = new GuiRolloutCtrl() {
      Margin = "0 0 0 0";
      DefaultHeight = "40";
      Expanded = "1";
      ClickCollapse = "1";
      HideHeader = "0";
      isContainer = "1";
      Profile = "GuiRolloutProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      position = "0 0";
      Extent = "421 114";
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      tooltipprofile = "ToolsGuiToolTipProfile";
      hovertime = "1000";
      canSaveDynamicFields = "0";
      caption = %parameter.getInternalName();

      new GuiControl() {
         isContainer = "1";
         Profile = "ToolsGuiDefaultProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         position = "0 17";
         Extent = "421 94";
         MinExtent = "421 94";
         canSave = "1";
         Visible = "1";
         tooltipprofile = "ToolsGuiToolTipProfile";
         hovertime = "1000";
         canSaveDynamicFields = "0";

         new GuiTextCtrl() {
            text = "Value";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "right";
            VertSizing = "bottom";
            position = "7 4";
            Extent = "27 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Channel";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "right";
            VertSizing = "bottom";
            position = "7 27";
            Extent = "45 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Comment";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "right";
            VertSizing = "bottom";
            position = "7 50";
            Extent = "47 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Tags";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "right";
            VertSizing = "bottom";
            position = "7 73";
            Extent = "25 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Min";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "205 27";
            Extent = "17 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Max";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "271 27";
            Extent = "21 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiTextCtrl() {
            text = "Initial";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiAutoSizeTextProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "340 27";
            Extent = "24 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            canSaveDynamicFields = "0";
         };
         new GuiSliderCtrl() {
            range = "0 1";
            ticks = "0";
            snap = "0";
            value = "0.5";
            isContainer = "0";
            Profile = "ToolsGuiSliderProfile";
            HorizSizing = "width";
            VertSizing = "bottom";
            position = "65 5";
            Extent = "263 15";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "valueSlider";
            canSaveDynamicFields = "0";
            command = %parameter @ ".value = $thisControl.getValue();";
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "336 4";
            Extent = "39 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "valueField";
            canSaveDynamicFields = "0";
            altCommand = %parameter @ ".value = $thisControl.getValue();";
         };
         new GuiBitmapButtonCtrl() {
            bitmap = "tools/gui/images/reset-icon";
            autoFit = "0";
            groupNum = "-1";
            buttonType = "PushButton";
            useMouseEvents = "0";
            isContainer = "0";
            Profile = "ToolsGuiDefaultProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "381 4";
            Extent = "17 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "resetButton";
            canSaveDynamicFields = "0";
            command = %parameter @ ".reset();";
         };
         new GuiBitmapButtonCtrl() {
            bitmap = "tools/gui/images/delete";
            autoFit = "0";
            groupNum = "-1";
            buttonType = "PushButton";
            useMouseEvents = "0";
            isContainer = "0";
            Profile = "ToolsGuiDefaultProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "398 4";
            Extent = "17 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "deleteButton";
            canSaveDynamicFields = "0";
            command = "EManageSFXParameters.showDeleteParameterDlg( " @ %parameter @ ");";
         };
         new GuiPopUpMenuCtrl() {
            maxPopupHeight = "200";
            sbUsesNAColor = "0";
            reverseTextList = "0";
            bitmapBounds = "16 16";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiPopUpMenuProfile";
            HorizSizing = "width";
            VertSizing = "bottom";
            position = "65 26";
            Extent = "135 18";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "channelDropdown";
            canSaveDynamicFields = "0";
            command = %parameter @ ".channel = $ThisControl.getText(); EManageSFXParameters.saveParameter( " @ %parameter @ ");"; //RDTODO: update range
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "width";
            VertSizing = "bottom";
            position = "65 50";
            Extent = "350 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "descriptionField";
            canSaveDynamicFields = "0";
            altCommand = %parameter @ ".description = $ThisControl.getText(); EManageSFXParameters.saveParameter( " @ %parameter @ ");";
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "width";
            VertSizing = "bottom";
            position = "65 73";
            Extent = "230 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "tagsField";
            canSaveDynamicFields = "0";
            altCommand = "EManageSFXParameters.updateParameterCategories( " @ %parameter @ ", $ThisControl.getText() );";
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "372 27";
            Extent = "43 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "defaultField";
            canSaveDynamicFields = "0";
            command = %parameter @ ".defaultValue = $ThisControl.getValue(); EManageSFXParameters.saveParameter( " @ %parameter @ ");";
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "297 27";
            Extent = "39 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "rangeMaxField";
            canSaveDynamicFields = "0";
            altCommand = %parameter @ ".range = " @ %parameter @ ".range.x SPC $ThisControl.getValue(); $ThisControl.parentGroup-->valueSlider.range = " @ %parameter @ ".range; EManageSFXParameters.saveParameter( " @ %parameter @ ");";
         };
         new GuiTextEditCtrl() {
            historySize = "0";
            password = "0";
            tabComplete = "0";
            sinkAllKeyEvents = "0";
            passwordMask = "•";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiTextEditProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "229 27";
            Extent = "39 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "rangeMinField";
            canSaveDynamicFields = "0";
            altCommand = %parameter @ ".range = $ThisControl.getValue() SPC " @ %parameter @ ".range.y; $ThisControl.parentGroup-->valueSlider.range = " @ %parameter @ ".range; EManageSFXParameters.saveParameter( " @ %parameter @ ");";
         };
         new GuiCheckBoxCtrl() {
            useInactiveState = "0";
            text = "Local";
            groupNum = "-1";
            buttonType = "ToggleButton";
            useMouseEvents = "0";
            isContainer = "0";
            Profile = "ToolsGuiCheckBoxProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "302 73";
            Extent = "45 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "localCheckbox";
            canSaveDynamicFields = "0";
         };
         new GuiPopUpMenuCtrl() {
            maxPopupHeight = "200";
            sbUsesNAColor = "0";
            reverseTextList = "0";
            bitmapBounds = "16 16";
            maxLength = "1024";
            Margin = "0 0 0 0";
            Padding = "0 0 0 0";
            AnchorTop = "1";
            AnchorBottom = "0";
            AnchorLeft = "1";
            AnchorRight = "0";
            isContainer = "0";
            Profile = "ToolsGuiPopUpMenuProfile";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = "349 73";
            Extent = "64 17";
            MinExtent = "8 2";
            canSave = "1";
            Visible = "1";
            tooltipprofile = "ToolsGuiToolTipProfile";
            hovertime = "1000";
            internalName = "sourceDropdown";
            canSaveDynamicFields = "0";
         };
      };
   };
   
   %ctrl.sfxParameter = %parameter;
   
   // Deactivate the per-source controls for now as these are not
   // yet implemented in SFX.
   
   %ctrl-->localCheckbox.setActive( false );
   %ctrl-->sourceDropdown.setActive( false );
   
   // Set the fields to reflect the parameter's current settings.
   
   %ctrl-->valueField.setValue( %paramter.value );
   %ctrl-->rangeMinField.setText( %parameter.range.x );
   %ctrl-->rangeMaxField.setText( %parameter.range.y );
   %ctrl-->defaultField.setValue( %parameter.defaultValue );
   %ctrl-->descriptionField.setText( %parameter.description );
   
   %ctrl-->valueSlider.range = %parameter.range;
   %ctrl-->valueSlider.setValue( %parameter.value );
   
   // Set up the channels dropdown.
   
   %list = %ctrl-->channelDropdown;
   for( %i = 0; %i < $SFX_PARAMETER_CHANNELS_COUNT; %i ++ )
      %list.add( $SFX_PARAMETER_CHANNELS[ %i ], %i );
   %list.sort();
   %list.setSelected( %list.findText( %parameter.channel ) );
   
   %this-->SFXParametersStack.addGuiControl( %ctrl );
   
   // Fill tagging field.
   
   %tags = "";
   %isFirst = true;
   for( %i = 0; %parameter.categories[ %i ] !$= ""; %i ++ )
   {
      if( !%isFirst )
         %tags = %tags @ ", ";
         
      %tags = %tags @ %parameter.categories[ %i ];
      
      %isFirst = false;
   }
   
   %ctrl-->tagsField.setText( %tags );
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::removeParameter( %this, %parameter )
{
   foreach( %ctrl in %this-->SFXParametersStack )
   {
      if( %ctrl.sfxParameter == %parameter )
      {
         %ctrl.delete();
         break;
      }
   }
}

//-----------------------------------------------------------------------------

function EManageSFXParameters::update( %this )
{   
   foreach( %ctrl in %this-->SFXParametersStack )
   {
      // If either the value field or the slider are currently being
      // edited, don't update the value in order to not interfere with
      // user editing.
      
      if( %ctrl-->valueField.isFirstResponder() || %ctrl-->valueSlider.isThumbBeingDragged() )
         continue;
      
      %parameter = %ctrl.sfxParameter;
      
      %ctrl-->valueField.setValue( %parameter.value );
      %ctrl-->valueSlider.setValue( %parameter.value );
   }
   
   // If the control is still awake, schedule another
   // update.
   
   if( %this.isVisible() )
      %this.schedule( $SFX_PARAMETERS_UPDATE_INTERVAL, "update" );
}
