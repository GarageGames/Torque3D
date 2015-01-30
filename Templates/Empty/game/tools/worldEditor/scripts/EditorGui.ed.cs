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

function EditorGui::init(%this)
{
   EWorldEditor.isDirty = false;
   ETerrainEditor.isDirty = false;
   ETerrainEditor.isMissionDirty = false;
   
   if( %this.isInitialized )
      return;

   %this.readWorldEditorSettings();

   $SelectedOperation = -1;
   $NextOperationId   = 1;
   $HeightfieldDirtyRow = -1;

   %this.buildMenus();

   if( !isObject( %this-->ToolsPaletteWindow ) )
   {
      // Load Creator/Inspector GUI
      exec("~/worldEditor/gui/ToolsPaletteGroups/init.cs");
      exec("~/worldEditor/gui/ToolsPaletteWindow.ed.gui");
      
      if( isObject( EWToolsPaletteWindow ) )
      {
         %this.add( EWToolsPaletteWindow );
         EWToolsPaletteWindow.init();
         EWToolsPaletteWindow.setVisible( false );
      }
   }
   
   if( !isObject( %this-->TreeWindow ) )
   {
      // Load Creator/Inspector GUI
      exec("~/worldEditor/gui/WorldEditorTreeWindow.ed.gui");
      if( isObject( EWTreeWindow ) )
      {
         %this.add( EWTreeWindow );
         EWTreeWindow-->EditorTree.selectPage( 0 );
         EWTreeWindow.setVisible( false );
      }
   }
   
   if( !isObject( %this-->InspectorWindow ) )
   {
      // Load Creator/Inspector GUI
      exec("~/worldEditor/gui/WorldEditorInspectorWindow.ed.gui");
      //EWInspectorWindow.resize(getWord(EWInspectorWindow.Position, 0), getWord(EWInspectorWindow.Position, 1), getWord(EWInspectorWindow.extent, 0), getWord(EWInspectorWindow.extent, 1));
      if( isObject( EWInspectorWindow ) )
      {
         %this.add( EWInspectorWindow );
         EWInspectorWindow.setVisible( false );
      }
   }   
   
   if( !isObject( %this-->WorldEditorToolbar ) )
   {
      // Load Creator/Inspector GUI
      exec("~/worldEditor/gui/WorldEditorToolbar.ed.gui");
      if( isObject( EWorldEditorToolbar ) )
      {
         %this.add( EWorldEditorToolbar );
         EWorldEditorToolbar.setVisible( false );
      }
   }  
   
   if ( !isObject( %this-->TerrainEditToolbar ) )
   {
      // Load Terrain Edit GUI
      exec("~/worldEditor/gui/TerrainEditToolbar.ed.gui");
      if( isObject( EWTerrainEditToolbar ) )
      {
         %this.add( EWTerrainEditToolbar );
         EWTerrainEditToolbar.setVisible( false );
      }
   }
   
   if( !isObject( %this-->TerrainPainter ) )
   {
      // Load Terrain Painter GUI
      exec("~/worldEditor/gui/TerrainPainterWindow.ed.gui");
      if( isObject( %guiContent ) ){
         %this.add( %guiContent->TerrainPainter );
         %this.add( %guiContent->TerrainPainterPreview );
      }
         
      exec("~/worldEditor/gui/guiTerrainMaterialDlg.ed.gui"); 
      exec("~/worldEditor/gui/TerrainBrushSoftnessCurveDlg.ed.gui");        
   }
   if ( !isObject( %this-->TerrainPainterToolbar) )
   {
      // Load Terrain Edit GUI
      exec("~/worldEditor/gui/TerrainPainterToolbar.ed.gui");
      if( isObject( EWTerrainPainterToolbar ) )
      {
         %this.add( EWTerrainPainterToolbar );
         EWTerrainPainterToolbar.setVisible( false );
      }
   }

   if( !isObject( %this-->ToolsToolbar ) )
   {
      // Load Creator/Inspector GUI
      exec("~/worldEditor/gui/ToolsToolbar.ed.gui");
      if( isObject( EWToolsToolbar ) )
      {
         %this.add( EWToolsToolbar );
         EWToolsToolbar.setVisible( true );
         
      }
   }
   
   // Visibility Layer Window
   if( !isObject( %this-->VisibilityLayerWindow ) )
   {
      %this.add( EVisibility );
      EVisibility.setVisible(false);
      EVisibilityTabBook.selectPage(0);
   }
      
   // Editor Settings Window
   if( !isObject( %this-->EditorSettingsWindow ) )
   {
      exec("~/worldEditor/gui/EditorSettingsWindow.ed.gui");
      exec("~/worldEditor/scripts/editorSettingsWindow.ed.cs");
      %this.add( ESettingsWindow );
      ESettingsWindow.setVisible(false);
      
      // Start the standard settings tabs pages
      exec( "~/worldEditor/gui/GeneralSettingsTab.ed.gui" );
      ESettingsWindow.addTabPage( EGeneralSettingsPage );
      exec("~/worldEditor/gui/ObjectEditorSettingsTab.ed.gui");
      ESettingsWindow.addTabPage( EObjectEditorSettingsPage );
      exec("~/worldEditor/gui/AxisGizmoSettingsTab.ed.gui");
      ESettingsWindow.addTabPage( EAxisGizmoSettingsPage );
      exec("~/worldEditor/gui/TerrainEditorSettingsTab.ed.gui");
      ESettingsWindow.addTabPage( ETerrainEditorSettingsPage );
      exec("~/worldEditor/gui/CameraSettingsTab.ed.gui");
      ESettingsWindow.addTabPage( ECameraSettingsPage );
   }

   // Object Snap Options Window
   if( !isObject( %this-->SnapOptionsWindow ) )
   {
      exec("~/worldEditor/gui/ObjectSnapOptionsWindow.ed.gui");
      exec("~/worldEditor/scripts/objectSnapOptions.ed.cs");
      %this.add( ESnapOptions );
      ESnapOptions.setVisible(false);
      ESnapOptionsTabBook.selectPage(0);
   }
   
   // Transform Selection Window
   if( !isObject( %this-->TransformSelectionWindow ) )
   {
      exec("~/worldEditor/gui/TransformSelectionWindow.ed.gui");
      exec("~/worldEditor/scripts/transformSelection.ed.cs");
      %this.add( ETransformSelection );
      ETransformSelection.setVisible(false);
   }
   
   // Manage Bookmarks Window
   if( !isObject( %this-->ManageBookmarksWindow ) )
   {
      %this.add( EManageBookmarks );
      EManageBookmarks.setVisible(false);
   }
   
   // Manage SFXParameters Window
   if( !isObject( %this-->ManageSFXParametersWindow ) )
   {
      %this.add( EManageSFXParameters );
      EManageSFXParameters.setVisible( false );
   }
   
   // Select Objects Window
   if( !isObject( %this->SelectObjectsWindow ) )
   {
      %this.add( ESelectObjectsWindow );
      ESelectObjectsWindow.setVisible( false );
   }

   EWorldEditor.init();
   ETerrainEditor.init();

   //Creator.init();
   EWCreatorWindow.init();
   ObjectBuilderGui.init();

   %this.setMenuDefaultState();
   
   EWorldEditorToggleCamera.setBitmap("tools/worldEditor/images/toolbar/player");
   
   /*
   EWorldEditorCameraSpeed.clear();
   EWorldEditorCameraSpeed.add("Slowest - Camera 1",0);
   EWorldEditorCameraSpeed.add("Slow - Camera 2",1);
   EWorldEditorCameraSpeed.add("Slower - Camera 3",2);
   EWorldEditorCameraSpeed.add("Normal - Camera 4",3);
   EWorldEditorCameraSpeed.add("Faster - Camera 5",4);
   EWorldEditorCameraSpeed.add("Fast - Camera 6",5);
   EWorldEditorCameraSpeed.add("Fastest - Camera 7",6);
   EWorldEditorCameraSpeed.setSelected(3);
   */
   
   EWorldEditorAlignPopup.clear();
   EWorldEditorAlignPopup.add("World",0);
   EWorldEditorAlignPopup.add("Object",1);
   EWorldEditorAlignPopup.setSelected(0);
   
   
   // sync camera gui
   EditorGui.syncCameraGui();
   
   // this will brind CameraTypesDropdown to front so that it goes over the menubar
   EditorGui.pushToBack(CameraTypesDropdown); 
   EditorGui.pushToBack(VisibilityDropdown); 
   
   // dropdowns out so that they display correctly in editor gui
   objectTransformDropdown.parentGroup = editorGui; 
   objectCenterDropdown.parentGroup = editorGui; 
   objectSnapDropdown.parentGroup = editorGui; 
   
   // make sure to show the default world editor guis
   EditorGui.bringToFront( EWorldEditor );
   EWorldEditor.setVisible( false );       
   
   // Call the startup callback on the editor plugins.   
   for ( %i = 0; %i < EditorPluginSet.getCount(); %i++ )
   {
      %obj = EditorPluginSet.getObject( %i );
      %obj.onWorldEditorStartup();      
   }

   // With everything loaded, start up the settings window
   ESettingsWindow.startup();
   
   // Start up initial editor plugin.
   
   %initialEditor = %this.currentEditor; // Read from prefs.
   %this.currentEditor = "";

   if( %initialEditor $= "" )
      %initialEditor = "WorldEditorInspectorPlugin";
   %this.setEditor( %initialEditor, true, true );
   
   // Done.
   
   %this.isInitialized = true;
}

//------------------------------------------------------------------------------
// Editor Gui's interactions with Camera Settings

function EditorGui::setupDefaultCameraSettings( %this )
{
   EditorSettings.beginGroup( "LevelInformation/levels/" @ %this.levelName );
   
   EditorSettings.setDefaultValue(  "cameraSpeedMin",         "5"         );
   EditorSettings.setDefaultValue(  "cameraSpeedMax",         "200"         );

   EditorSettings.endGroup();
}

function EditorGui::readCameraSettings( %this, %levelName )
{
   if( %levelName !$= %this.levelName )
      return;
      
   EditorCameraSpeedOptions.setupGuiControls();
}

function EditorGui::writeCameraSettings( %this )
{
   EditorSettings.beginGroup( "LevelInformation/levels/" @ %this.levelName );
   
   EditorSettings.setValue( "cameraSpeed",               $Camera::movementSpeed );

   EditorSettings.endGroup();
}

//------------------------------------------------------------------------------

function EditorGui::shutdown( %this )
{   
   // Store settings.
   %this.writeWorldEditorSettings();
   
   // Deactivate current editor.
   %this.setEditor( "" );

   // Call the shutdown callback on the editor plugins.   
   foreach( %plugin in EditorPluginSet )
      %plugin.onWorldEditorShutdown();
}

/// This is used to add an editor to the Editors menu which
/// will take over the default world editor window.
function EditorGui::addToEditorsMenu( %this, %displayName, %accel, %newPlugin )
{
   %windowMenu = %this.findMenu( "Editors" );   
   %count = %windowMenu.getItemCount();      
   
   
   %alreadyExists = false;
   for ( %i = 0; %i < %count; %i++ )
   {      
      %existingPlugins = getField(%windowMenu.Item[%i], 2);
      
      if(%newPlugin $= %existingPlugins)
         %alreadyExists = true;
   }
   
   if( %accel $= "" && %count < 9 )
      %accel = "F" @ %count + 1;
   else
      %accel = "";
         
   if(!%alreadyExists)
      %windowMenu.addItem( %count, %displayName TAB %accel TAB %newPlugin );
      
   return %accel;
}

function EditorGui::addToToolsToolbar( %this, %pluginName, %internalName, %bitmap, %tooltip )
{   
   %count = ToolsToolbarArray.getCount();      
   
   %alreadyExists = false;
   for ( %i = 0; %i < %count; %i++ )
   {      
      %existingInternalName = ToolsToolbarArray.getObject(%i).getFieldValue("internalName");
      
      if(%internalName $= %existingInternalName)
      {
         %alreadyExists = true;
         break;
      }
   }
      
   if(!%alreadyExists)
   {
      %button = new GuiBitmapButtonCtrl() {
         canSaveDynamicFields = "0";
         internalName = %internalName;
         Enabled = "1";
         isContainer = "0";
         Profile = "ToolsGuiButtonProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         position = "180 0";
         Extent = "25 19";
         MinExtent = "8 2";
         canSave = "1";
         Visible = "1";
         Command = "EditorGui.setEditor(" @ %pluginName @ ");";
         tooltipprofile = "ToolsGuiToolTipProfile";
         ToolTip = %tooltip;
         hovertime = "750";
         bitmap = %bitmap;
         buttonType = "RadioButton";
         groupNum = "0";
         useMouseEvents = "0";
      };
      ToolsToolbarArray.add(%button);
      EWToolsToolbar.setExtent((25 + 8) * (%count + 1) + 12 SPC "33");
   }
}

//-----------------------------------------------------------------------------

function EditorGui::setDisplayType( %this, %type )
{
   %gui = %this.currentEditor.editorGui;
   if( !isObject( %gui ) )
      return;

   %this.viewTypeMenu.checkRadioItem( 0, 7, %type );

   // Store the current camera rotation so we can restore it correctly when
   // switching back to perspective view
   if ( %gui.getDisplayType() == $EditTSCtrl::DisplayTypePerspective )
      %this.lastPerspectiveCamRotation = LocalClientConnection.camera.getRotation();

   %gui.setDisplayType( %type );

   if ( %gui.getDisplayType() == $EditTSCtrl::DisplayTypePerspective )
      LocalClientConnection.camera.setRotation( %this.lastPerspectiveCamRotation );

   %this.currentDisplayType = %type;
}

//-----------------------------------------------------------------------------

function EditorGui::setEditor( %this, %newEditor, %dontActivate )
{
   if ( isObject( %this.currentEditor ) )
   {
      if( isObject( %newEditor ) && %this.currentEditor.getId() == %newEditor.getId() )
         return;

      if( %this.currentEditor.isActivated )
         %this.currentEditor.onDeactivated();
         
      if( isObject( %this.currentEditor.editorGui ) )
         %this.currentOrthoFOV = %this.currentEditor.editorGui.getOrthoFOV();
   }
   
   if( !isObject( %newEditor ) )
   {
      %this.currentEditor = "";
      return;
   }
      
   // If we have a special set editor function, run that instead
   if( %newEditor.isMethod( "setEditorFunction" ) )
   {
      if( %newEditor.setEditorFunction() ) 
      {
         %this.syncEditor( %newEditor );
         %this.currentEditor = %newEditor;
         
         if (!%dontActivate)
            %this.currentEditor.onActivated();
      }
      else
      {
         // if were falling back and were the same editor, why are we going to just shove ourself
         // into the editor position again? opt for a fallback
         if( !isObject( %this.currentEditor ) )
            %this.currentEditor = "WorldEditorInspectorPlugin";
         else if( %this.currentEditor.getId() == %newEditor.getId() )
            %this.currentEditor = "WorldEditorInspectorPlugin";
            
         %this.syncEditor( %this.currentEditor, true );
         
         if( !%dontActivate )
            %this.currentEditor.onActivated();
      }
   }
   else
   {
      %this.syncEditor( %newEditor );
      %this.currentEditor = %newEditor; 
      
      if( !%dontActivate )
         %this.currentEditor.onActivated();
   }
   
   // Sync display type.
   
   %gui = %this.currentEditor.editorGui;
   if( isObject( %gui ) )
   {
      %gui.setDisplayType( %this.currentDisplayType );
      %gui.setOrthoFOV( %this.currentOrthoFOV );
      EditorGui.syncCameraGui();
   }
}

function EditorGui::syncEditor( %this, %newEditor, %newEditorFailed )
{
   // Sync with menu bar
   %menu = %this.findMenu( "Editors" );
   %count = %menu.getItemCount();      
   for ( %i = 0; %i < %count; %i++ )
   {
      %pluginObj = getField( %menu.item[%i], 2 );
      if ( %pluginObj $= %newEditor )
      {
         %menu.checkRadioItem( 0, %count, %i );
         break;  
      }
   }   
   
   // In order to hook up a palette, the word Palette must be able to be
   // switched out in order to read correctly, if not, no palette will be used
   %paletteName = strreplace(%newEditor, "Plugin", "Palette");
   
   // Sync with ToolsToolbar
   for ( %i = 0; %i < ToolsToolbarArray.getCount(); %i++ )
   {
      %toolbarButton = ToolsToolbarArray.getObject(%i).internalName;
      if( %paletteName $= %toolbarButton )
      {
         ToolsToolbarArray.getObject(%i).setStateOn(1);
         break;
      }
   } 
   
   // Handles quit game and gui editor changes in wierd scenarios
   if( %newEditorFailed && EWToolsToolbar.isDynamic )
   {
      if( EWToolsToolbar.isClosed )
         EWToolsToolbar.reset();
      EWToolsToolbar.toggleSize();
   }
            
   // Toggle the editor specific palette; we define special cases here
   switch$ ( %paletteName )
   {
      case "MaterialEditorPalette":
         %paletteName = "WorldEditorInspectorPalette";
      case "DatablockEditorPalette":
         %paletteName = "WorldEditorInspectorPalette";
      case "ParticleEditorPalette":
         %paletteName = "WorldEditorInspectorPalette";      
   }
      
   %this-->ToolsPaletteWindow.togglePalette(%paletteName);
}

function EditorGui::onWake( %this )
{
   EHWorldEditor.setStateOn( 1 );
   
   // Notify the editor plugins that the editor has started.
   
   foreach( %plugin in EditorPluginSet )
      %plugin.onEditorWake();
   
   // Push the ActionMaps in the order that we want to have them
   // before activating an editor plugin, so that if the plugin
   // installs an ActionMap, it will be highest on the stack.
   
   MoveMap.push();
   EditorMap.push();
   
   // Active the current editor plugin.

   if( !%this.currentEditor.isActivated )
      %this.currentEditor.onActivated();

   %slashPos = 0;
   while( strpos( $Server::MissionFile, "/", %slashPos ) != -1 )
   {
      %slashPos = strpos( $Server::MissionFile, "/", %slashPos ) + 1;
   }
   %levelName = getSubStr( $Server::MissionFile , %slashPos , 99 );   
   
   if( %levelName !$= %this.levelName )
      %this.onNewLevelLoaded( %levelName );
      
   if (isObject(DemoEditorAlert) && DemoEditorAlert.helpTag<2)
      Canvas.pushDialog(DemoEditorAlert);
}

function EditorGui::onSleep( %this )
{
   // Deactivate the current editor plugin.
   
   if( %this.currentEditor.isActivated )
      %this.currentEditor.onDeactivated();
      
   // Remove the editor's ActionMaps.
      
   EditorMap.pop();
   MoveMap.pop();

   // Notify the editor plugins that the editor will be closing.
   
   foreach( %plugin in EditorPluginSet )
      %plugin.onEditorSleep();
            
   if(isObject($Server::CurrentScene))
      $Server::CurrentScene.open();
}

function EditorGui::onNewLevelLoaded( %this, %levelName )
{
   %this.levelName = %levelName;
   %this.setupDefaultCameraSettings();
   ECameraSettingsPage.init();
   EditorCameraSpeedOptions.setupDefaultState();
   
   new ScriptObject( EditorMissionCleanup )
   {
      parentGroup = "MissionCleanup";
   };
}

function EditorMissionCleanup::onRemove( %this )
{
   EditorGui.levelName = "";
   foreach( %plugin in EditorPluginSet )
      %plugin.onExitMission();
}

//-----------------------------------------------------------------------------

// Called when we have been set as the content and onWake has been called
function EditorGui::onSetContent(%this, %oldContent)
{
   %this.attachMenus();
}

// Called before onSleep when the canvas content is changed
function EditorGui::onUnsetContent(%this, %newContent)
{
   %this.detachMenus();
}

//------------------------------------------------------------------------------

function EditorGui::toggleSFXParametersWindow( %this )
{
   %window = %this-->ManageSFXParametersWindow;
   %window.setVisible( !%window.isVisible() );
}

//------------------------------------------------------------------------------

function EditorGui::addCameraBookmark( %this, %name )
{
   %obj = new CameraBookmark() {
      datablock = CameraBookmarkMarker;
      internalName = %name;
   };

   // Place into correct group
   if( !isObject(CameraBookmarks) )
   {
      %grp = new SimGroup(CameraBookmarks);
      MissionGroup.add(%grp);
   }
   CameraBookmarks.add( %obj );

   %cam = LocalClientConnection.camera.getTransform();
   %obj.setTransform( %cam );
   
   EWorldEditor.isDirty = true;
   EditorTree.buildVisibleTree(true);
}

function EditorGui::removeCameraBookmark( %this, %name )
{
   if( !isObject(CameraBookmarks) )
      return;

   %mark = CameraBookmarks.findObjectByInternalName( %name, true );
   if( %mark == 0 )
      return;

   MEDeleteUndoAction::submit( %mark );
   EWorldEditor.isDirty = true;
   EditorTree.buildVisibleTree(true);
}

function EditorGui::removeCameraBookmarkIndex( %this, %index )
{
   if( !isObject(CameraBookmarks) )
      return;

   if( %index < 0 || %index >= CameraBookmarks.getCount() )
      return;

   %obj = CameraBookmarks.getObject( %index );
   MEDeleteUndoAction::submit( %obj );
   EWorldEditor.isDirty = true;
   EditorTree.buildVisibleTree(true);
}

function EditorGui::jumpToBookmark( %this, %name )
{
   if( !isObject(CameraBookmarks) )
      return;

   %mark = CameraBookmarks.findObjectByInternalName( %name, true );
   if( %mark == 0 )
      return;
      
   LocalClientConnection.camera.setTransform( %mark.getTransform() );
   return;
}

function EditorGui::jumpToBookmarkIndex( %this, %index )
{
   if( !isObject(CameraBookmarks) )
      return;

   if( %index < 0 || %index >= CameraBookmarks.getCount() )
      return;

   %trans = CameraBookmarks.getObject( %index ).getTransform();
   LocalClientConnection.camera.setTransform( %trans );
}

function EditorGui::addCameraBookmarkByGui( %this )
{
   // look for a NewCamera name to grab
   for(%i = 0; ; %i++){
      %name = "NewCamera_" @ %i;
      if( !CameraBookmarks.findObjectByInternalName(%name) ){
         break;
      }
   }
   EditorGui.addCameraBookmark( %name );
}

function EditorGui::toggleCameraBookmarkWindow( %this )
{
   EManageBookmarks.ToggleVisibility();
}

function EditorGui::toggleObjectSelectionsWindow( %this )
{
   ESelectObjectsWindow.toggleVisibility();
}

function EditorGui::toggleOrthoGrid( %this )
{
   EWorldEditor.renderOrthoGrid = !EWorldEditor.renderOrthoGrid;
}

//------------------------------------------------------------------------------

function EditorGui::syncCameraGui( %this )
{
   if( !EditorIsActive() )
      return;

   // Sync projection type
   %displayType = %this.currentEditor.editorGui.getDisplayType();
   %this.viewTypeMenu.checkRadioItem( 0, 7, %displayType );

   // Set the camera object's mode and rotation so that it moves correctly
   // based on the current editor mode
   if( %displayType != $EditTSCtrl::DisplayTypePerspective )
   {
      switch( %displayType )
      {
         case $EditTSCtrl::DisplayTypeTop:       %name = "Top View";          %camRot = "0 0 0";
         case $EditTSCtrl::DisplayTypeBottom:    %name = "Bottom View";       %camRot = "3.14159 0 0";
         case $EditTSCtrl::DisplayTypeLeft:      %name = "Left View";         %camRot = "-1.571 0 1.571";
         case $EditTSCtrl::DisplayTypeRight:     %name = "Right View";        %camRot = "-1.571 0 -1.571";
         case $EditTSCtrl::DisplayTypeFront:     %name = "Front View";        %camRot = "-1.571 0 3.14159";
         case $EditTSCtrl::DisplayTypeBack:      %name = "Back View";         %camRot = "-1.571 0 0";
         case $EditTSCtrl::DisplayTypeIsometric: %name = "Isometric View";    %camRot = "0 0 0";
      }

      LocalClientConnection.camera.controlMode = "Fly";
      LocalClientConnection.camera.setRotation( %camRot );
      EditorGuiStatusBar.setCamera( %name );
      return;
   }

   // Sync camera settings.
   %flyModeRadioItem = -1;
   if(LocalClientConnection.getControlObject() != LocalClientConnection.player)
   {
      %mode = LocalClientConnection.camera.getMode();

      if(%mode $= "Fly" && LocalClientConnection.camera.newtonMode)
      {
         if(LocalClientConnection.camera.newtonRotation == true)
         {
            EditorGui-->NewtonianRotationCamera.setStateOn(true);
            EWorldEditorToggleCamera.setBitmap("tools/gui/images/menubar/smooth-cam-rot");
            %flyModeRadioItem = 4;
            EditorGuiStatusBar.setCamera("Smooth Rot Camera");
         }
         else
         {
            EditorGui-->NewtonianCamera.setStateOn(true);
            EWorldEditorToggleCamera.setBitmap("tools/gui/images/menubar/smooth-cam");
            %flyModeRadioItem = 3;
            EditorGuiStatusBar.setCamera("Smooth Camera");
         }
      }
      else if(%mode $= "EditOrbit")
      {
         EditorGui-->OrbitCamera.setStateOn(true);
         EWorldEditorToggleCamera.setBitmap("tools/gui/images/menubar/orbit-cam");
         %flyModeRadioItem = 1;
         EditorGuiStatusBar.setCamera("Orbit Camera");
      }
		else // default camera mode
      {
         EditorGui-->StandardCamera.setStateOn(true);
         EWorldEditorToggleCamera.setBitmap("tools/worldEditor/images/toolbar/camera");
         %flyModeRadioItem = 0;
         EditorGuiStatusBar.setCamera("Standard Camera");
      }
      
      //quick way select menu bar options
      %this.findMenu( "Camera" ).checkRadioItem( 0, 1, 0 );
      EditorFreeCameraTypeOptions.checkRadioItem( 0, 4, %flyModeRadioItem);
      EditorPlayerCameraTypeOptions.checkRadioItem( 0, 4, -1);
   }
   else if (!$isFirstPersonVar) // if 3rd person
   {
      EditorGui-->trdPersonCamera.setStateOn(true);
      EWorldEditorToggleCamera.setBitmap("tools/worldEditor/images/toolbar/3rd-person-camera");
      %flyModeRadioItem = 1;
      //quick way select menu bar options
      %this.findMenu( "Camera" ).checkRadioItem( 0, 1, 1 );
      EditorPlayerCameraTypeOptions.checkRadioItem( 0, 2, %flyModeRadioItem);
      EditorGuiStatusBar.setCamera("3rd Person Camera");
   }
   else if ($isFirstPersonVar) // if 1st Person
   {
      EditorGui-->PlayerCamera.setStateOn(true);
      EWorldEditorToggleCamera.setBitmap("tools/worldEditor/images/toolbar/player");
      %flyModeRadioItem = 0;
      //quick way select menu bar options
      %this.findMenu( "Camera" ).checkRadioItem( 0, 1, 1 );
      EditorPlayerCameraTypeOptions.checkRadioItem( 0, 2, %flyModeRadioItem);
      EditorFreeCameraTypeOptions.checkRadioItem( 0, 4, -1);
      EditorGuiStatusBar.setCamera("1st Person Camera");
   }      
 }
  
/// @name EditorPlugin Methods 
/// @{
 
//------------------------------------------------------------------------------
// WorldEditorPlugin
//------------------------------------------------------------------------------

function WorldEditorPlugin::onActivated( %this )
{
   EditorGui.bringToFront( EWorldEditor );
   EWorldEditor.setVisible(true);
   EditorGui.menuBar.insert( EditorGui.worldMenu, EditorGui.menuBar.dynamicItemInsertPos );
   EWorldEditor.makeFirstResponder(true);
   EditorTree.open(MissionGroup,true);
   EWCreatorWindow.setNewObjectGroup(MissionGroup);

   EWorldEditor.syncGui();

   EditorGuiStatusBar.setSelectionObjectsByCount(EWorldEditor.getSelectionSize());
   
   // Should the Transform Selection window open?
   if( EWorldEditor.ETransformSelectionDisplayed )
   {
      ETransformSelection.setVisible(true);
   }
   
   Parent::onActivated(%this);
}

function WorldEditorPlugin::onDeactivated( %this )
{
   // Hide the Transform Selection window from other editors
   ETransformSelection.setVisible(false);
   
   EWorldEditor.setVisible( false );            
   EditorGui.menuBar.remove( EditorGui.worldMenu );
   
   Parent::onDeactivated(%this);    
}

//------------------------------------------------------------------------------
// WorldEditorInspectorPlugin
//------------------------------------------------------------------------------

function WorldEditorInspectorPlugin::onWorldEditorStartup( %this )
{
   Parent::onWorldEditorStartup( %this );
   
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Object Editor", "", WorldEditorInspectorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Object Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "WorldEditorInspectorPlugin", "WorldEditorInspectorPalette", expandFilename("tools/worldEditor/images/toolbar/transform-objects"), %tooltip );
   
   //connect editor windows
   GuiWindowCtrl::attach( EWInspectorWindow, EWTreeWindow);
   
   %map = new ActionMap();   
   %map.bindCmd( keyboard, "1", "EWorldEditorNoneModeBtn.performClick();", "" );  // Select
   %map.bindCmd( keyboard, "2", "EWorldEditorMoveModeBtn.performClick();", "" );  // Move
   %map.bindCmd( keyboard, "3", "EWorldEditorRotateModeBtn.performClick();", "" );  // Rotate
   %map.bindCmd( keyboard, "4", "EWorldEditorScaleModeBtn.performClick();", "" );  // Scale
   %map.bindCmd( keyboard, "f", "FitToSelectionBtn.performClick();", "" );// Fit Camera to Selection
   %map.bindCmd( keyboard, "z", "EditorGuiStatusBar.setCamera(\"Standard Camera\");", "" );// Free camera
   %map.bindCmd( keyboard, "n", "ToggleNodeBar->renderHandleBtn.performClick();", "" );// Render Node
   %map.bindCmd( keyboard, "shift n", "ToggleNodeBar->renderTextBtn.performClick();", "" );// Render Node Text
   %map.bindCmd( keyboard, "g", "ESnapOptions-->GridSnapButton.performClick();" ); // Grid Snappping
   %map.bindCmd( keyboard, "t", "SnapToBar->objectSnapDownBtn.performClick();", "" );// Terrain Snapping
   %map.bindCmd( keyboard, "b", "SnapToBar-->objectSnapBtn.performClick();" ); // Soft Snappping
   %map.bindCmd( keyboard, "v", "EWorldEditorToolbar->boundingBoxColBtn.performClick();", "" );// Bounds Selection
   %map.bindCmd( keyboard, "o", "objectCenterDropdown->objectBoxBtn.performClick(); objectCenterDropdown.toggle();", "" );// Object Center
   %map.bindCmd( keyboard, "p", "objectCenterDropdown->objectBoundsBtn.performClick(); objectCenterDropdown.toggle();", "" );// Bounds Center
   %map.bindCmd( keyboard, "k", "objectTransformDropdown->objectTransformBtn.performClick(); objectTransformDropdown.toggle();", "" );// Object Transform
   %map.bindCmd( keyboard, "l", "objectTransformDropdown->worldTransformBtn.performClick(); objectTransformDropdown.toggle();", "" );// World Transform
   
   WorldEditorInspectorPlugin.map = %map; 
}

function WorldEditorInspectorPlugin::onActivated( %this )
{   
   Parent::onActivated( %this );

   EditorGui-->InspectorWindow.setVisible( true );   
   EditorGui-->TreeWindow.setVisible( true );
   EditorGui-->WorldEditorToolbar.setVisible( true );
   %this.map.push();
}

function WorldEditorInspectorPlugin::onDeactivated( %this )
{   
   Parent::onDeactivated( %this );

   EditorGui-->InspectorWindow.setVisible( false );  
   EditorGui-->TreeWindow.setVisible( false ); 
   EditorGui-->WorldEditorToolbar.setVisible( false );
   %this.map.pop();
}

function WorldEditorInspectorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %canCutCopy = EWorldEditor.getSelectionSize() > 0;
   %editMenu.enableItem( 3, %canCutCopy ); // Cut
   %editMenu.enableItem( 4, %canCutCopy ); // Copy      
   %editMenu.enableItem( 5, EWorldEditor.canPasteSelection() ); // Paste
   
   %selSize = EWorldEditor.getSelectionSize();
   %lockCount = EWorldEditor.getSelectionLockCount();
   %hideCount = EWorldEditor.getSelectionHiddenCount();   
   %editMenu.enableItem( 6, %selSize > 0 && %lockCount != %selSize ); // Delete Selection
   
   %editMenu.enableItem( 8, %canCutCopy ); // Deselect  
}

function WorldEditorInspectorPlugin::handleDelete( %this )
{
   // The tree handles deletion and notifies the
   // world editor to clear its selection.  
   //
   // This is because non-SceneObject elements like
   // SimGroups also need to be destroyed.
   //
   // See EditorTree::onObjectDeleteCompleted().
   %selSize = EWorldEditor.getSelectionSize();
   if( %selSize > 0 )   
      EditorTree.deleteSelection();   
}

function WorldEditorInspectorPlugin::handleDeselect()
{
   EWorldEditor.clearSelection();
}

function WorldEditorInspectorPlugin::handleCut()
{
   EWorldEditor.cutSelection();
}

function WorldEditorInspectorPlugin::handleCopy()
{
   EWorldEditor.copySelection();
}

function WorldEditorInspectorPlugin::handlePaste()
{
   EWorldEditor.pasteSelection();
}

//------------------------------------------------------------------------------
// TerrainEditorPlugin
//------------------------------------------------------------------------------

function TerrainEditorPlugin::onWorldEditorStartup( %this )
{
   Parent::onWorldEditorStartup( %this );
   
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Terrain Editor", "", TerrainEditorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Terrain Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "TerrainEditorPlugin", "TerrainEditorPalette", expandFilename("tools/worldEditor/images/toolbar/sculpt-terrain"), %tooltip );
   
   %map = new ActionMap();   
   %map.bindCmd( keyboard, "1", "ToolsPaletteArray->brushAdjustHeight.performClick();", "" );    //Grab Terrain
   %map.bindCmd( keyboard, "2", "ToolsPaletteArray->raiseHeight.performClick();", "" );     // Raise Height
   %map.bindCmd( keyboard, "3", "ToolsPaletteArray->lowerHeight.performClick();", "" );     // Lower Height
   %map.bindCmd( keyboard, "4", "ToolsPaletteArray->smoothHeight.performClick();", "" );    // Average Height
   %map.bindCmd( keyboard, "5", "ToolsPaletteArray->smoothSlope.performClick();", "" );    // Smooth Slope
   %map.bindCmd( keyboard, "6", "ToolsPaletteArray->paintNoise.performClick();", "" );      // Noise
   %map.bindCmd( keyboard, "7", "ToolsPaletteArray->flattenHeight.performClick();", "" );   // Flatten
   %map.bindCmd( keyboard, "8", "ToolsPaletteArray->setHeight.performClick();", "" );       // Set Height
   %map.bindCmd( keyboard, "9", "ToolsPaletteArray->setEmpty.performClick();", "" );    // Clear Terrain
   %map.bindCmd( keyboard, "0", "ToolsPaletteArray->clearEmpty.performClick();", "" );  // Restore Terrain
   %map.bindCmd( keyboard, "v", "EWTerrainEditToolbarBrushType->ellipse.performClick();", "" );// Circle Brush
   %map.bindCmd( keyboard, "b", "EWTerrainEditToolbarBrushType->box.performClick();", "" );// Box Brush
   %map.bindCmd( keyboard, "=", "TerrainEditorPlugin.keyboardModifyBrushSize(1);", "" );// +1 Brush Size
   %map.bindCmd( keyboard, "+", "TerrainEditorPlugin.keyboardModifyBrushSize(1);", "" );// +1 Brush Size
   %map.bindCmd( keyboard, "-", "TerrainEditorPlugin.keyboardModifyBrushSize(-1);", "" );// -1 Brush Size
   %map.bindCmd( keyboard, "[", "TerrainEditorPlugin.keyboardModifyBrushSize(-5);", "" );// -5 Brush Size
   %map.bindCmd( keyboard, "]", "TerrainEditorPlugin.keyboardModifyBrushSize(5);", "" );// +5 Brush Size
   /*%map.bindCmd( keyboard, "]", "TerrainBrushPressureTextEditContainer->textEdit.text += 5", "" );// +5 Pressure
   %map.bindCmd( keyboard, "[", "TerrainBrushPressureTextEditContainer->textEdit.text -= 5", "" );// -5 Pressure
   %map.bindCmd( keyboard, "'", "TerrainBrushSoftnessTextEditContainer->textEdit.text += 5", "" );// +5 Softness
   %map.bindCmd( keyboard, ";", "TerrainBrushSoftnessTextEditContainer->textEdit.text -= 5", "" );// -5 Softness*/
   
   TerrainEditorPlugin.map = %map;  
   
   %this.terrainMenu = new PopupMenu()
   {
      superClass = "MenuBuilder";

      barTitle = "Terrain";
               
      item[0] = "Smooth Heightmap" TAB "" TAB "ETerrainEditor.onSmoothHeightmap();";
   };   
}

function TerrainEditorPlugin::onActivated( %this )
{
   Parent::onActivated( %this );

   EditorGui.readTerrainEditorSettings();
   
   %action = EditorSettings.value("TerrainEditor/currentAction");
   ETerrainEditor.switchAction( %action );
   ToolsPaletteArray.findObjectByInternalName( %action, true ).setStateOn( true );

   EWTerrainEditToolbarBrushType->ellipse.performClick(); // Circle Brush
   
   EditorGui.menuBar.insert( %this.terrainMenu, EditorGui.menuBar.dynamicItemInsertPos );
         
   EditorGui.bringToFront( ETerrainEditor );
   ETerrainEditor.setVisible( true );
   ETerrainEditor.attachTerrain();
   ETerrainEditor.makeFirstResponder( true );
        
   EWTerrainEditToolbar.setVisible( true );
   ETerrainEditor.onBrushChanged();
   ETerrainEditor.setup();
   TerrainEditorPlugin.syncBrushInfo();

   EditorGuiStatusBar.setSelection("");
   %this.map.push();
}

function TerrainEditorPlugin::onDeactivated( %this )
{
   Parent::onDeactivated( %this );

   endToolTime("TerrainEditor");
   EditorGui.writeTerrainEditorSettings();

   EWTerrainEditToolbar.setVisible( false );
   ETerrainEditor.setVisible( false );

   EditorGui.menuBar.remove( %this.terrainMenu );

   %this.map.pop();
}

function TerrainEditorPlugin::syncBrushInfo( %this )
{
   // Update gui brush info
   TerrainBrushSizeTextEditContainer-->textEdit.text = getWord(ETerrainEditor.getBrushSize(), 0);
   TerrainBrushPressureTextEditContainer-->textEdit.text = ETerrainEditor.getBrushPressure()*100;
   TerrainBrushSoftnessTextEditContainer-->textEdit.text = ETerrainEditor.getBrushSoftness()*100;
   TerrainSetHeightTextEditContainer-->textEdit.text = ETerrainEditor.setHeightVal;

   %brushType = ETerrainEditor.getBrushType();
   eval( "EWTerrainEditToolbar-->" @ %brushType @ ".setStateOn(1);" );
}

function TerrainEditorPlugin::validateBrushSize( %this )
{
   %minBrushSize = 1;
   %maxBrushSize = getWord(ETerrainEditor.maxBrushSize, 0);

   %val = $ThisControl.getText();
   if(%val < %minBrushSize)
      $ThisControl.setValue(%minBrushSize);
   else if(%val > %maxBrushSize)
      $ThisControl.setValue(%maxBrushSize);
}

function TerrainEditorPlugin::keyboardModifyBrushSize( %this, %amt)
{
   %val = TerrainBrushSizeTextEditContainer-->textEdit.getText();
   %val += %amt;
   TerrainBrushSizeTextEditContainer-->textEdit.setValue(%val);
   TerrainBrushSizeTextEditContainer-->textEdit.forceValidateText();
   ETerrainEditor.setBrushSize( TerrainBrushSizeTextEditContainer-->textEdit.getText() );
}

//------------------------------------------------------------------------------
// TerrainTextureEditorTool
//------------------------------------------------------------------------------

function TerrainTextureEditorTool::onActivated( %this )
{
   EditorGui.bringToFront( ETerrainEditor );
   ETerrainEditor.setVisible( true );
   ETerrainEditor.attachTerrain();
   ETerrainEditor.makeFirstResponder( true );
   
   EditorGui-->TextureEditor.setVisible(true);

   EditorGuiStatusBar.setSelection("");
}

function TerrainTextureEditorTool::onDeactivated( %this )
{
   EditorGui-->TextureEditor.setVisible(false); 
       
   ETerrainEditor.setVisible( false );
}

//------------------------------------------------------------------------------
// TerrainPainterPlugin
//------------------------------------------------------------------------------

function TerrainPainterPlugin::onWorldEditorStartup( %this )
{
   Parent::onWorldEditorStartup( %this );
   
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Terrain Painter", "", TerrainPainterPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Terrain Painter (" @ %accel @ ")"; 
   EditorGui.addToToolsToolbar( "TerrainPainterPlugin", "TerrainPainterPalette", expandFilename("tools/worldEditor/images/toolbar/paint-terrain"), %tooltip );

   %map = new ActionMap();   
   %map.bindCmd( keyboard, "v", "EWTerrainPainterToolbarBrushType->ellipse.performClick();", "" );// Circle Brush
   %map.bindCmd( keyboard, "b", "EWTerrainPainterToolbarBrushType->box.performClick();", "" );// Box Brush
   %map.bindCmd( keyboard, "=", "TerrainPainterPlugin.keyboardModifyBrushSize(1);", "" );// +1 Brush Size
   %map.bindCmd( keyboard, "+", "TerrainPainterPlugin.keyboardModifyBrushSize(1);", "" );// +1 Brush Size
   %map.bindCmd( keyboard, "-", "TerrainPainterPlugin.keyboardModifyBrushSize(-1);", "" );// -1 Brush Size
   %map.bindCmd( keyboard, "[", "TerrainPainterPlugin.keyboardModifyBrushSize(-5);", "" );// -5 Brush Size
   %map.bindCmd( keyboard, "]", "TerrainPainterPlugin.keyboardModifyBrushSize(5);", "" );// +5 Brush Size
   /*%map.bindCmd( keyboard, "]", "PaintBrushSlopeControl->SlopeMinAngle.text += 5", "" );// +5 SlopeMinAngle
   %map.bindCmd( keyboard, "[", "PaintBrushSlopeControl->SlopeMinAngle.text -= 5", "" );// -5 SlopeMinAngle
   %map.bindCmd( keyboard, "'", "PaintBrushSlopeControl->SlopeMaxAngle.text += 5", "" );// +5 SlopeMaxAngle
   %map.bindCmd( keyboard, ";", "PaintBrushSlopeControl->SlopeMaxAngle.text -= 5", "" );// -5 Softness*/

   for(%i=1; %i<10; %i++)
   {
      %map.bindCmd( keyboard, %i, "TerrainPainterPlugin.keyboardSetMaterial(" @ (%i-1) @ ");", "" );
   }
   %map.bindCmd( keyboard, 0, "TerrainPainterPlugin.keyboardSetMaterial(10);", "" );
   
   TerrainPainterPlugin.map = %map;  
   GuiWindowCtrl::attach( EPainter, EPainterPreview);
}

function TerrainPainterPlugin::onActivated( %this )
{
   Parent::onActivated( %this );

   EditorGui.readTerrainEditorSettings();

   EWTerrainPainterToolbarBrushType->ellipse.performClick();// Circle Brush
   %this.map.push();

   EditorGui.bringToFront( ETerrainEditor );
   ETerrainEditor.setVisible( true );
   ETerrainEditor.attachTerrain();
   ETerrainEditor.makeFirstResponder( true );

   EditorGui-->TerrainPainter.setVisible(true);
   EditorGui-->TerrainPainterPreview.setVisible(true);
   EWTerrainPainterToolbar.setVisible(true);
   ETerrainEditor.onBrushChanged();
   EPainter.setup();
   TerrainPainterPlugin.syncBrushInfo();

   EditorGuiStatusBar.setSelection("");
}

function TerrainPainterPlugin::onDeactivated( %this )
{
   Parent::onDeactivated( %this );

   EditorGui.writeTerrainEditorSettings();

   %this.map.pop();
   EditorGui-->TerrainPainter.setVisible(false);
   EditorGui-->TerrainPainterPreview.setVisible(false);
   EWTerrainPainterToolbar.setVisible(false);
   ETerrainEditor.setVisible( false );
}

function TerrainPainterPlugin::syncBrushInfo( %this )
{
   // Update gui brush info
   PaintBrushSizeTextEditContainer-->textEdit.text = getWord(ETerrainEditor.getBrushSize(), 0);
   PaintBrushSlopeControl-->SlopeMinAngle.text = ETerrainEditor.getSlopeLimitMinAngle();
   PaintBrushSlopeControl-->SlopeMaxAngle.text = ETerrainEditor.getSlopeLimitMaxAngle();
   PaintBrushPressureTextEditContainer-->textEdit.text = ETerrainEditor.getBrushPressure()*100;
   %brushType = ETerrainEditor.getBrushType();
   eval( "EWTerrainPainterToolbar-->" @ %brushType @ ".setStateOn(1);" );
}

function TerrainPainterPlugin::validateBrushSize( %this )
{
   %minBrushSize = 1;
   %maxBrushSize = getWord(ETerrainEditor.maxBrushSize, 0);

   %val = $ThisControl.getText();
   if(%val < %minBrushSize)
      $ThisControl.setValue(%minBrushSize);
   else if(%val > %maxBrushSize)
      $ThisControl.setValue(%maxBrushSize);
}

function TerrainPainterPlugin::validateSlopeMaxAngle( %this )
{
   %maxval = ETerrainEditor.getSlopeLimitMaxAngle();
   PaintBrushSlopeControl-->SlopeMaxAngle.setText(%maxval); 
}

function TerrainPainterPlugin::validateSlopeMinAngle( %this )
{
   %minval = ETerrainEditor.getSlopeLimitMinAngle();
   PaintBrushSlopeControl-->SlopeMinAngle.setText(%minval);  
}

function TerrainPainterPlugin::keyboardModifyBrushSize( %this, %amt)
{
   %val = PaintBrushSizeTextEditContainer-->textEdit.getText();
   %val += %amt;
   PaintBrushSizeTextEditContainer-->textEdit.setValue(%val);
   PaintBrushSizeTextEditContainer-->textEdit.forceValidateText();
   ETerrainEditor.setBrushSize( PaintBrushSizeTextEditContainer-->textEdit.getText() );
}

function TerrainPainterPlugin::keyboardSetMaterial( %this, %mat)
{
   %name = "EPainterMaterialButton" @ %mat;
   %ctrl = EPainter.findObjectByInternalName(%name, true);
   if(%ctrl)
   {
      %ctrl.performClick();
   }
}
   
/// @} End of EditorPlugin Methods


function objectTransformDropdown::toggle()
{
   if ( objectTransformDropdown.visible  )
   {
      EWorldEditorToolbar-->objectTransform.setStateOn(false);
      objectTransformDropdownDecoy.setVisible(false);
      objectTransformDropdownDecoy.setActive(false);
      objectTransformDropdown.setVisible(false);
   }
   else
   {
      EWorldEditorToolbar-->objectTransform.setStateOn(true);
      objectTransformDropdown.setVisible(true);
      objectTransformDropdownDecoy.setActive(true);
      objectTransformDropdownDecoy.setVisible(true);
   }
}

function CameraTypesDropdownToggle()
{
   if ( CameraTypesDropdown.visible  )
   {
      EWorldEditorToggleCamera.setStateOn(0);
      CameraTypesDropdownDecoy.setVisible(false);
      CameraTypesDropdownDecoy.setActive(false);
      CameraTypesDropdown.setVisible(false);
   }
   else
   {
      CameraTypesDropdown.setVisible(true);
      CameraTypesDropdownDecoy.setVisible(true);
      CameraTypesDropdownDecoy.setActive(true);
      EWorldEditorToggleCamera.setStateOn(1);
   }
}

function VisibilityDropdownToggle()
{
   if ( EVisibility.visible  )
   {
      EVisibility.setVisible(false);
      visibilityToggleBtn.setStateOn(0);
   }
   else
   {
      EVisibility.setVisible(true);
      visibilityToggleBtn.setStateOn(1);
   }
}

function CameraTypesDropdownDecoy::onMouseLeave()
{
   CameraTypesDropdownToggle();
}

//-----------------------------------------------------------------------------

function EWorldEditor::getGridSnap( %this )
{
   return %this.gridSnap;
}

function EWorldEditor::setGridSnap( %this, %value )
{
   %this.gridSnap = %value;
   GlobalGizmoProfile.snapToGrid = %value;
   %this.syncGui();
}

function EWorldEditor::getGridSize( %this )
{
   return %this.gridSize;
}

function EWorldEditor::setGridSize( %this, %value )
{
   GlobalGizmoProfile.gridSize = %value SPC %value SPC %value;
   %this.gridSize = %value;
   
   %this.syncGui();
}

//-----------------------------------------------------------------------------

function EWorldEditor::areAllSelectedObjectsOfType( %this, %className )
{
   %activeSelection = %this.getActiveSelection();
   if( !isObject( %activeSelection ) )
      return false;
      
   %count = %activeSelection.getCount();
   for( %i = 0; %i < %count; %i ++ )
   {
      %obj = %activeSelection.getObject( %i );
      if( !%obj.isMemberOfClass( %className ) )
         return false;
   }
      
   return true;
}

//-----------------------------------------------------------------------------
function EWorldEditorToggleCamera::toggleBitmap(%this)
{
   %currentImage = %this.bitmap;

   if ( %currentImage $= "tools/worldEditor/images/toolbar/player" )
      %image = "tools/worldEditor/images/toolbar/camera";
   else
      %image = "tools/worldEditor/images/toolbar/player";

   %this.setBitmap( %image );
}

function EWorldEditorCameraSpeed::updateMenuBar(%this, %editorBarCtrl)
{
   // Update Toolbar TextEdit
   if( %editorBarCtrl.getId() == CameraSpeedDropdownCtrlContainer-->slider.getId() )
   {
      %value = %editorBarCtrl.getValue();
      EWorldEditorCameraSpeed.setText( %value );
      $Camera::movementSpeed = %value;
   }

   // Update Toolbar Slider
   if( %editorBarCtrl.getId() == EWorldEditorCameraSpeed.getId() )
   {
      %value = %editorBarCtrl.getText();
      if ( %value !$= "" )
      {
         if ( %value <= 0 )    // camera speed must be >= 0
         {
            %value = 1;
            %editorBarCtrl.setText( %value );
         }
         CameraSpeedDropdownCtrlContainer-->slider.setValue( %value );
         $Camera::movementSpeed = %value;
      }
   }
   
   // Update Editor
   EditorCameraSpeedOptions.checkRadioItem(0, 6, -1);
}

//-----------------------------------------------------------------------------

function EWorldEditorAlignPopup::onSelect(%this, %id, %text)
{
   if ( GlobalGizmoProfile.mode $= "Scale" && %text $= "World" )
   {
      EWorldEditorAlignPopup.setSelected(1);
      return;
   }
   
   GlobalGizmoProfile.alignment = %text;   
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

function EWorldEditorNoneModeBtn::onClick(%this)
{
   GlobalGizmoProfile.mode = "None";
   
   EditorGuiStatusBar.setInfo("Selection arrow.");
}

function EWorldEditorMoveModeBtn::onClick(%this)
{
   GlobalGizmoProfile.mode = "Move";
   
   %cmdCtrl = "CTRL";
   if( $platform $= "macos" )
      %cmdCtrl = "CMD";
   
   EditorGuiStatusBar.setInfo( "Move selection.  SHIFT while dragging duplicates objects.  " @ %cmdCtrl @ " to toggle soft snap.  ALT to toggle grid snap." );
}

function EWorldEditorRotateModeBtn::onClick(%this)
{
   GlobalGizmoProfile.mode = "Rotate";
   
   EditorGuiStatusBar.setInfo("Rotate selection.");
}

function EWorldEditorScaleModeBtn::onClick(%this)
{
   GlobalGizmoProfile.mode = "Scale";
   
   EditorGuiStatusBar.setInfo("Scale selection.");
}

//-----------------------------------------------------------------------------

function EditorTree::onDeleteSelection( %this )
{
   %this.undoDeleteList = "";   
}

function EditorTree::onDeleteObject( %this, %object )
{
   // Don't delete locked objects
   if( %object.locked )
      return true;
   
   if( %object == EWCreatorWindow.objectGroup )
      EWCreatorWindow.setNewObjectGroup( MissionGroup );

   // Append it to our list.
   %this.undoDeleteList = %this.undoDeleteList TAB %object;
              
   // We're gonna delete this ourselves in the
   // completion callback.
   return true;
}

function EditorTree::onObjectDeleteCompleted( %this )
{
   // This can be called when a deletion is attempted but nothing was
   // actually deleted ( cannot delete the root of the tree ) so only submit
   // the undo if we really deleted something.
   if ( %this.undoDeleteList !$= "" )   
      MEDeleteUndoAction::submit( %this.undoDeleteList );
   
   // Let the world editor know to 
   // clear its selection.
   EWorldEditor.clearSelection();
   EWorldEditor.isDirty = true;
}

function EditorTree::onClearSelected(%this)
{
   WorldEditor.clearSelection();
}

function EditorTree::onInspect(%this, %obj)
{
   Inspector.inspect(%obj);   
}

function EditorTree::toggleLock( %this )
{
   if( EWTreeWindow-->LockSelection.command $= "EWorldEditor.lockSelection(true); EditorTree.toggleLock();" )
   {
      EWTreeWindow-->LockSelection.command = "EWorldEditor.lockSelection(false); EditorTree.toggleLock();";
      EWTreeWindow-->DeleteSelection.command = "";
   }
   else
   {
      EWTreeWindow-->LockSelection.command = "EWorldEditor.lockSelection(true); EditorTree.toggleLock();";
      EWTreeWindow-->DeleteSelection.command = "EditorMenuEditDelete();";
   }
}

function EditorTree::onAddSelection(%this, %obj, %isLastSelection)
{
   EWorldEditor.selectObject( %obj );
   
   %selSize = EWorldEditor.getSelectionSize();
   %lockCount = EWorldEditor.getSelectionLockCount();
   
   if( %lockCount < %selSize )
   {
      EWTreeWindow-->LockSelection.setStateOn(0);
      EWTreeWindow-->LockSelection.command = "EWorldEditor.lockSelection(true); EditorTree.toggleLock();";
   }
   else if ( %lockCount > 0 )
   {
      EWTreeWindow-->LockSelection.setStateOn(1);
      EWTreeWindow-->LockSelection.command = "EWorldEditor.lockSelection(false); EditorTree.toggleLock();";
   }
   
   if( %selSize > 0 && %lockCount == 0 )
      EWTreeWindow-->DeleteSelection.command = "EditorMenuEditDelete();";
   else
      EWTreeWindow-->DeleteSelection.command = "";
   
   if( %isLastSelection )
      Inspector.addInspect( %obj );
   else  
      Inspector.addInspect( %obj, false );
      
}
function EditorTree::onRemoveSelection(%this, %obj)
{
   EWorldEditor.unselectObject(%obj);
   Inspector.removeInspect( %obj );
}
function EditorTree::onSelect(%this, %obj)
{
}

function EditorTree::onUnselect(%this, %obj)
{
   EWorldEditor.unselectObject(%obj);
}

function EditorTree::onDragDropped(%this)
{
   EWorldEditor.isDirty = true;
}

function EditorTree::onAddGroupSelected(%this, %group)
{
   EWCreatorWindow.setNewObjectGroup(%group);
}

function EditorTree::onRightMouseUp( %this, %itemId, %mouse, %obj )
{
   %haveObjectEntries = false;
   %haveLockAndHideEntries = true;
   
   // Handle multi-selection.
   if( %this.getSelectedItemsCount() > 1 )
   {
      %popup = ETMultiSelectionContextPopup;
      if( !isObject( %popup ) )
         %popup = new PopupMenu( ETMultiSelectionContextPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Delete" TAB "" TAB "EditorMenuEditDelete();";
            item[ 1 ] = "Group" TAB "" TAB "EWorldEditor.addSimGroup( true );";
         };
   }

   // Open context menu if this is a CameraBookmark
   else if( %obj.isMemberOfClass( "CameraBookmark" ) )
   {
      %popup = ETCameraBookmarkContextPopup;
      if( !isObject( %popup ) )
         %popup = new PopupMenu( ETCameraBookmarkContextPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Go To Bookmark" TAB "" TAB "EditorGui.jumpToBookmark( %this.bookmark.getInternalName() );";

            bookmark = -1;
         };

      ETCameraBookmarkContextPopup.bookmark = %obj;
   }
   
   // Open context menu if this is set CameraBookmarks group.
   else if( %obj.name $= "CameraBookmarks" )
   {
      %popup = ETCameraBookmarksGroupContextPopup;
      if( !isObject( %popup ) )
         %popup = new PopupMenu( ETCameraBookmarksGroupContextPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Add Camera Bookmark" TAB "" TAB "EditorGui.addCameraBookmarkByGui();";
         };
   }

   // Open context menu if this is a SimGroup
   else if( %obj.isMemberOfClass( "SimGroup" ) )
   {
      %popup = ETSimGroupContextPopup;
      if( !isObject( %popup ) )
         %popup = new PopupMenu( ETSimGroupContextPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Rename" TAB "" TAB "EditorTree.showItemRenameCtrl( EditorTree.findItemByObjectId( %this.object ) );";
            item[ 1 ] = "Delete" TAB "" TAB "EWorldEditor.deleteMissionObject( %this.object );";
            item[ 2 ] = "Inspect" TAB "" TAB "inspectObject( %this.object );";
            item[ 3 ] = "-";
            item[ 4 ] = "Toggle Lock Children" TAB "" TAB "EWorldEditor.toggleLockChildren( %this.object );";
            item[ 5 ] = "Toggle Hide Children" TAB "" TAB "EWorldEditor.toggleHideChildren( %this.object );";
            item[ 6 ] = "-";
            item[ 7 ] = "Group" TAB "" TAB "EWorldEditor.addSimGroup( true );";
            item[ 8 ] = "-";
            item[ 9 ] = "Add New Objects Here" TAB "" TAB "EWCreatorWindow.setNewObjectGroup( %this.object );";
            item[ 10 ] = "Add Children to Selection" TAB "" TAB "EWorldEditor.selectAllObjectsInSet( %this.object, false );";
            item[ 11 ] = "Remove Children from Selection" TAB "" TAB "EWorldEditor.selectAllObjectsInSet( %this.object, true );";

            object = -1;
         };

      %popup.object = %obj;
      
      %hasChildren = %obj.getCount() > 0;
      %popup.enableItem( 10, %hasChildren );
      %popup.enableItem( 11, %hasChildren );
      
      %haveObjectEntries = true;
      %haveLockAndHideEntries = false;
   }
   
   // Open generic context menu.
   else
   {
      %popup = ETContextPopup;      
      if( !isObject( %popup ) )
         %popup = new PopupMenu( ETContextPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Rename" TAB "" TAB "EditorTree.showItemRenameCtrl( EditorTree.findItemByObjectId( %this.object ) );";
            item[ 1 ] = "Delete" TAB "" TAB "EWorldEditor.deleteMissionObject( %this.object );";
            item[ 2 ] = "Inspect" TAB "" TAB "inspectObject( %this.object );";
            item[ 3 ] = "-";
            item[ 4 ] = "Locked" TAB "" TAB "%this.object.setLocked( !%this.object.locked ); EWorldEditor.syncGui();";
            item[ 5 ] = "Hidden" TAB "" TAB "EWorldEditor.hideObject( %this.object, !%this.object.hidden ); EWorldEditor.syncGui();";
            item[ 6 ] = "-";
            item[ 7 ] = "Group" TAB "" TAB "EWorldEditor.addSimGroup( true );";

            object = -1;
         };
     
      // Specialized version for ConvexShapes. 
      if( %obj.isMemberOfClass( "ConvexShape" ) )
      {
         %popup = ETConvexShapeContextPopup;      
         if( !isObject( %popup ) )
            %popup = new PopupMenu( ETConvexShapeContextPopup : ETContextPopup )
            {
               superClass = "MenuBuilder";
               isPopup = "1";

               item[ 8 ] = "-";
               item[ 9 ] = "Convert to Zone" TAB "" TAB "EWorldEditor.convertSelectionToPolyhedralObjects( \"Zone\" );";
               item[ 10 ] = "Convert to Portal" TAB "" TAB "EWorldEditor.convertSelectionToPolyhedralObjects( \"Portal\" );";
               item[ 11 ] = "Convert to Occluder" TAB "" TAB "EWorldEditor.convertSelectionToPolyhedralObjects( \"OcclusionVolume\" );";
               item[ 12 ] = "Convert to Sound Space" TAB "" TAB "EWorldEditor.convertSelectionToPolyhedralObjects( \"SFXSpace\" );";
            };
      }
      
      // Specialized version for polyhedral objects.
      else if( %obj.isMemberOfClass( "Zone" ) ||
               %obj.isMemberOfClass( "Portal" ) ||
               %obj.isMemberOfClass( "OcclusionVolume" ) ||
               %obj.isMemberOfClass( "SFXSpace" ) )
      {
         %popup = ETPolyObjectContextPopup;      
         if( !isObject( %popup ) )
            %popup = new PopupMenu( ETPolyObjectContextPopup : ETContextPopup )
            {
               superClass = "MenuBuilder";
               isPopup = "1";

               item[ 8 ] = "-";
               item[ 9 ] = "Convert to ConvexShape" TAB "" TAB "EWorldEditor.convertSelectionToConvexShape();";
            };
      }

      %popup.object = %obj;
      %haveObjectEntries = true;
   }

   if( %haveObjectEntries )
   {         
      %popup.enableItem( 0, %obj.isNameChangeAllowed() && %obj.getName() !$= "MissionGroup" );
      %popup.enableItem( 1, %obj.getName() !$= "MissionGroup" );
      if( %haveLockAndHideEntries )
      {
         %popup.checkItem( 4, %obj.locked );
         %popup.checkItem( 5, %obj.hidden );
      }
      %popup.enableItem( 7, %this.isItemSelected( %itemId ) );
   }
   
   %popup.showPopup( Canvas );
}

function EditorTree::positionContextMenu( %this, %menu )
{
   if( (getWord(%menu.position, 0) + getWord(%menu.extent, 0)) > getWord(EWorldEditor.extent, 0) )
   {
      %posx = getWord(%menu.position, 0);
      %offset = getWord(EWorldEditor.extent, 0) - (%posx + getWord(%menu.extent, 0)) - 5;
      %posx += %offset;
      %menu.position = %posx @ " " @ getWord(%menu.position, 1);
   }
}

function EditorTree::isValidDragTarget( %this, %id, %obj )
{
   if( %obj.isMemberOfClass( "Path" ) )
      return EWorldEditor.areAllSelectedObjectsOfType( "Marker" );
   if( %obj.name $= "CameraBookmarks" )
      return EWorldEditor.areAllSelectedObjectsOfType( "CameraBookmark" );
   else
      return ( %obj.getClassName() $= "SimGroup" );
}

function EditorTree::onBeginReparenting( %this )
{
   if( isObject( %this.reparentUndoAction ) )
      %this.reparentUndoAction.delete();
      
   %action = UndoActionReparentObjects::create( %this );
   
   %this.reparentUndoAction = %action;
}

function EditorTree::onReparent( %this, %obj, %oldParent, %newParent )
{
   %this.reparentUndoAction.add( %obj, %oldParent, %newParent );
}

function EditorTree::onEndReparenting( %this )
{
   %action = %this.reparentUndoAction;
   %this.reparentUndoAction = "";

   if( %action.numObjects > 0 )
   {
      if( %action.numObjects == 1 )
         %action.actionName = "Reparent Object";
      else
         %action.actionName = "Reparent Objects";
         
      %action.addToManager( Editor.getUndoManager() );
      
      EWorldEditor.syncGui();
   }
   else
      %action.delete();
}

function EditorTree::update( %this )
{
   %this.buildVisibleTree( false );
}

//------------------------------------------------------------------------------

// Tooltip for TSStatic
function EditorTree::GetTooltipTSStatic( %this, %obj )
{
   return "Shape: " @ %obj.shapeName;
}

// Tooltip for ShapeBase
function EditorTree::GetTooltipShapeBase( %this, %obj )
{
   return "Datablock: " @ %obj.dataBlock;
}

// Tooltip for StaticShape
function EditorTree::GetTooltipStaticShape( %this, %obj )
{
   return "Datablock: " @ %obj.dataBlock;
}

// Tooltip for Item
function EditorTree::GetTooltipItem( %this, %obj )
{
   return "Datablock: " @ %obj.dataBlock;
}

// Tooltip for RigidShape
function EditorTree::GetTooltipRigidShape( %this, %obj )
{
   return "Datablock: " @ %obj.dataBlock;
}

// Tooltip for Prefab
function EditorTree::GetTooltipPrefab( %this, %obj )
{
   return "File: " @ %obj.filename;
}

// Tooltip for GroundCover
function EditorTree::GetTooltipGroundCover( %this, %obj )
{
   %text = "Material: " @ %obj.material;
   for(%i=0; %i<8; %i++)
   {
      if(%obj.probability[%i] > 0 && %obj.shapeFilename[%i] !$= "")
      {
         %text = %text NL "Shape " @ %i @ ": " @ %obj.shapeFilename[%i];
      }
   }
   return %text;
}

// Tooltip for SFXEmitter
function EditorTree::GetTooltipSFXEmitter( %this, %obj )
{
   if(%obj.fileName $= "")
      return "Track: " @ %obj.track;
   else
      return "File: " @ %obj.fileName;
}

// Tooltip for ParticleEmitterNode
function EditorTree::GetTooltipParticleEmitterNode( %this, %obj )
{
   %text = "Datablock: " @ %obj.dataBlock;
   %text = %text NL "Emitter: " @ %obj.emitter;
   return %text;
}

// Tooltip for WorldEditorSelection
function EditorTree::GetTooltipWorldEditorSelection( %this, %obj )
{
   %text = "Objects: " @ %obj.getCount();
   
   if( !%obj.getCanSave() )
      %text = %text NL "Persistent: No";
   else
      %text = %text NL "Persistent: Yes";
      
   return %text;
}

//------------------------------------------------------------------------------

function EditorTreeTabBook::onTabSelected( %this )
{
   if( EditorTreeTabBook.getSelectedPage() == 0)
   {
      EWTreeWindow-->DeleteSelection.visible = true;
      EWTreeWindow-->LockSelection.visible = true;
      EWTreeWindow-->AddSimGroup.visible = true;
   }
   else
   {
      EWTreeWindow-->DeleteSelection.visible = false;
      EWTreeWindow-->LockSelection.visible = false;
      EWTreeWindow-->AddSimGroup.visible = false;
   }
}

//------------------------------------------------------------------------------

function Editor::open(%this)
{
   // prevent the mission editor from opening while the GuiEditor is open.
   if(Canvas.getContent() == GuiEditorGui.getId())
      return;
      
   if( !EditorGui.isInitialized )
      EditorGui.init();

   %this.editorEnabled();
   Canvas.setContent(EditorGui);   
   EditorGui.syncCameraGui();
}

function Editor::close(%this, %gui)
{
   %this.editorDisabled();
   Canvas.setContent(%gui);
   if(isObject(MessageHud))
      MessageHud.close();   
   EditorGui.writeCameraSettings();
}

$RelightCallback = "";

function EditorLightingComplete()
{
   $lightingMission = false;
   RelightStatus.visible = false;
   
   if ($RelightCallback !$= "")
   {
      eval($RelightCallback);
   }
   
   $RelightCallback = "";
}

function updateEditorLightingProgress()
{
   RelightProgress.setValue(($SceneLighting::lightingProgress));
   if ($lightingMission)
      $lightingProgressThread = schedule(1, 0, "updateEditorLightingProgress");
}

function Editor::lightScene(%this, %callback, %forceAlways)
{
   if ($lightingMission)
      return;
      
   $lightingMission = true;
   $RelightCallback = %callback;
   RelightStatus.visible = true;
   RelightProgress.setValue(0);
   Canvas.repaint();  
   lightScene("EditorLightingComplete", %forceAlways);
   updateEditorLightingProgress();
} 

//------------------------------------------------------------------------------

function EditorGui::handleEscape( %this )
{
   %result = false;
   if ( isObject( %this.currentEditor ) )
      %result = %this.currentEditor.handleEscape();
      
   if ( !%result )
   {
     Editor.close("PlayGui");
   }
}

function EditTSCtrl::updateGizmoMode( %this, %mode )
{
   // Called when the gizmo mode is changed from C++
   
   if ( %mode $= "None" )
      EditorGuiToolbar->NoneModeBtn.performClick();
   else if ( %mode $= "Move" )   
      EditorGuiToolbar->MoveModeBtn.performClick();
   else if ( %mode $= "Rotate" )
      EditorGuiToolbar->RotateModeBtn.performClick();
   else if ( %mode $= "Scale" )
      EditorGuiToolbar->ScaleModeBtn.performClick();
}

//------------------------------------------------------------------------------

function EWorldEditor::syncGui( %this )
{
   %this.syncToolPalette();
   
   EditorTree.update();
   Editor.getUndoManager().updateUndoMenu( EditorGui.menuBar-->EditMenu );
   EditorGuiStatusBar.setSelectionObjectsByCount( %this.getSelectionSize() );
   
   EWTreeWindow-->LockSelection.setStateOn( %this.getSelectionLockCount() > 0 );
   
   EWorldEditorToolbar-->boundingBoxColBtn.setStateOn( EWorldEditor.boundingBoxCollision );
      
   if( EWorldEditor.objectsUseBoxCenter )
   {
      EWorldEditorToolbar-->centerObject.setBitmap("tools/gui/images/menubar/bounds-center");
      objectCenterDropdown-->objectBoundsBtn.setStateOn( 1 );
   }
   else
   {
      EWorldEditorToolbar-->centerObject.setBitmap("tools/gui/images/menubar/object-center");
      objectCenterDropdown-->objectBoxBtn.setStateOn( 1 );
   }
   
   if( GlobalGizmoProfile.getFieldValue(alignment) $= "Object" )
   {
      EWorldEditorToolbar-->objectTransform.setBitmap("tools/gui/images/menubar/object-transform");
      objectTransformDropdown-->objectTransformBtn.setStateOn( 1 );
      
   }
   else
   {
      EWorldEditorToolbar-->objectTransform.setBitmap("tools/gui/images/menubar/world-transform");
      objectTransformDropdown-->worldTransformBtn.setStateOn( 1 );
   }
   
   EWorldEditorToolbar-->renderHandleBtn.setStateOn( EWorldEditor.renderObjHandle );
   EWorldEditorToolbar-->renderTextBtn.setStateOn( EWorldEditor.renderObjText );

   SnapToBar-->objectSnapBtn.setStateOn( EWorldEditor.getSoftSnap() );
   EWorldEditorToolbar-->softSnapSizeTextEdit.setText( EWorldEditor.getSoftSnapSize() );
   ESnapOptions-->SnapSize.setText( EWorldEditor.getSoftSnapSize() );
   ESnapOptions-->GridSize.setText( EWorldEditor.getGridSize() );
   
   ESnapOptions-->GridSnapButton.setStateOn( %this.getGridSnap() );
   SnapToBar-->objectGridSnapBtn.setStateOn( %this.getGridSnap() );
   ESnapOptions-->NoSnapButton.setStateOn( !%this.stickToGround && !%this.getSoftSnap() && !%this.getGridSnap() );
}

function EWorldEditor::syncToolPalette( %this )
{
   switch$ ( GlobalGizmoProfile.mode )
   {
      case "None":
         EWorldEditorNoneModeBtn.performClick();
      case "Move":
         EWorldEditorMoveModeBtn.performClick();
      case "Rotate":
         EWorldEditorRotateModeBtn.performClick();
      case "Scale":
         EWorldEditorScaleModeBtn.performClick();
   }
}

function EWorldEditor::addSimGroup( %this, %groupCurrentSelection )
{
   %activeSelection = %this.getActiveSelection();
   if ( %activeSelection.getObjectIndex( MissionGroup ) != -1 )
   {
      MessageBoxOK( "Error", "Cannot add MissionGroup to a new SimGroup" );
      return;
   }

   // Find our parent.

   %parent = MissionGroup;
   if( !%groupCurrentSelection && isObject( %activeSelection ) && %activeSelection.getCount() > 0 )
   {
      %firstSelectedObject = %activeSelection.getObject( 0 );
      if( %firstSelectedObject.isMemberOfClass( "SimGroup" ) )
         %parent = %firstSelectedObject;
      else if( %firstSelectedObject.getId() != MissionGroup.getId() )
         %parent = %firstSelectedObject.parentGroup;
   }
   
   // If we are about to do a group-selected as well,
   // starting recording an undo compound.
   
   if( %groupCurrentSelection )
      Editor.getUndoManager().pushCompound( "Group Selected" );
   
   // Create the SimGroup.
   
   %object = new SimGroup()
   {
      parentGroup = %parent;
   };
   MECreateUndoAction::submit( %object );
   
   // Put selected objects into the group, if requested.
   
   if( %groupCurrentSelection && isObject( %activeSelection ) )
   {
      %undo = UndoActionReparentObjects::create( EditorTree );
      
      %numObjects = %activeSelection.getCount();
      for( %i = 0; %i < %numObjects; %i ++ )
      {
         %sel = %activeSelection.getObject( %i );
         %undo.add( %sel, %sel.parentGroup, %object );
         %object.add( %sel );
      }
      
      %undo.addToManager( Editor.getUndoManager() );
   }
      
   // Stop recording for group-selected.
   
   if( %groupCurrentSelection )
      Editor.getUndoManager().popCompound();
   
   // When not grouping selection, make the newly created SimGroup the
   // current selection.
   
   if( !%groupCurrentSelection )
   {
      EWorldEditor.clearSelection();
      EWorldEditor.selectObject( %object );
   }

   // Refresh the Gui.
   
   %this.syncGui();
}

function EWorldEditor::toggleLockChildren( %this, %simGroup )
{
   foreach( %child in %simGroup )
   {
      if( %child.isMemberOfClass( "SimGroup" ) )
         %this.toggleLockChildren( %child );
      else
         %child.setLocked( !%child.locked );
   }
   
   EWorldEditor.syncGui();
}

function EWorldEditor::toggleHideChildren( %this, %simGroup )
{
   foreach( %child in %simGroup )
   {
      if( %child.isMemberOfClass( "SimGroup" ) )
         %this.toggleHideChildren( %child );
      else
         %this.hideObject( %child, !%child.hidden );
   }
   
   EWorldEditor.syncGui();
}

function EWorldEditor::convertSelectionToPolyhedralObjects( %this, %className )
{
   %group = %this.getNewObjectGroup();
   %undoManager = Editor.getUndoManager();
   
   %activeSelection = %this.getActiveSelection();
   while( %activeSelection.getCount() != 0 )
   {
      %oldObject = %activeSelection.getObject( 0 );
      %newObject = %this.createPolyhedralObject( %className, %oldObject );
      if( isObject( %newObject ) )
      {
         %undoManager.pushCompound( "Convert ConvexShape to " @ %className );
         %newObject.parentGroup = %oldObject.parentGroup;
         MECreateUndoAction::submit( %newObject );
         MEDeleteUndoAction::submit( %oldObject );
         %undoManager.popCompound();
      }
   }
}

function EWorldEditor::convertSelectionToConvexShape( %this )
{
   %group = %this.getNewObjectGroup();
   %undoManager = Editor.getUndoManager();
   
   %activeSelection = %this.getActiveSelection();
   while( %activeSelection.getCount() != 0 )
   {
      %oldObject = %activeSelection.getObject( 0 );
      %newObject = %this.createConvexShapeFrom( %oldObject );
      if( isObject( %newObject ) )
      {
         %undoManager.pushCompound( "Convert " @ %oldObject.getClassName() @ " to ConvexShape" );
         %newObject.parentGroup = %oldObject.parentGroup;
         MECreateUndoAction::submit( %newObject );
         MEDeleteUndoAction::submit( %oldObject );
         %undoManager.popCompound();
      }
   }
}

function EWorldEditor::getNewObjectGroup( %this )
{
   return EWCreatorWindow.getNewObjectGroup();
}

function EWorldEditor::deleteMissionObject( %this, %object )
{
   // Unselect in editor tree.
   
   %id = EditorTree.findItemByObjectId( %object );   
   EditorTree.selectItem( %id, false );
   
   // Delete object.
   
   MEDeleteUndoAction::submit( %object );
   EWorldEditor.isDirty = true;
   EditorTree.buildVisibleTree( true );
}

function EWorldEditor::selectAllObjectsInSet( %this, %set, %deselect )
{
   if( !isObject( %set ) )
      return;
      
   foreach( %obj in %set )
   {
      if( %deselect )
         %this.unselectObject( %obj );
      else
         %this.selectObject( %obj );
   }
}

function toggleSnappingOptions( %var )
{
   if( SnapToBar->objectSnapDownBtn.getValue() && SnapToBar->objectSnapBtn.getValue() )
   {
      if( %var $= "terrain" )
      {
         EWorldEditor.stickToGround = 1;
         EWorldEditor.setSoftSnap(false);
         ESnapOptionsTabBook.selectPage(0);
         SnapToBar->objectSnapBtn.setStateOn(0);
      }
      else
      { 
         // soft snapping
         EWorldEditor.stickToGround = 0;
         EWorldEditor.setSoftSnap(true);
         ESnapOptionsTabBook.selectPage(1);
         SnapToBar->objectSnapDownBtn.setStateOn(0);
      }
   }
   else if( %var $= "terrain" && EWorldEditor.stickToGround == 0 )
   {
      // Terrain Snapping
      EWorldEditor.stickToGround = 1;
      EWorldEditor.setSoftSnap(false);
      ESnapOptionsTabBook.selectPage(0);
      SnapToBar->objectSnapDownBtn.setStateOn(1);
      SnapToBar->objectSnapBtn.setStateOn(0);
      
   }
   else if( %var $= "soft" && EWorldEditor.getSoftSnap() == false )
   { 
      // Object Snapping
      EWorldEditor.stickToGround = 0;
      EWorldEditor.setSoftSnap(true);
      ESnapOptionsTabBook.selectPage(1);
      SnapToBar->objectSnapBtn.setStateOn(1);
      SnapToBar->objectSnapDownBtn.setStateOn(0);
      
   }
   else if( %var $= "grid" )
   {
      EWorldEditor.setGridSnap( !EWorldEditor.getGridSnap() );
   }
   else
   { 
      // No snapping.
      
      EWorldEditor.stickToGround = false;
      EWorldEditor.setGridSnap( false );
      EWorldEditor.setSoftSnap( false );
      
      SnapToBar->objectSnapDownBtn.setStateOn(0);
      SnapToBar->objectSnapBtn.setStateOn(0);
   }
   
   EWorldEditor.syncGui();
}

function objectCenterDropdown::toggle()
{
   if ( objectCenterDropdown.visible  )
   {
      EWorldEditorToolbar-->centerObject.setStateOn(false);
      objectCenterDropdownDecoy.setVisible(false);
      objectCenterDropdownDecoy.setActive(false);
      objectCenterDropdown.setVisible(false);
   }
   else
   {
      EWorldEditorToolbar-->centerObject.setStateOn(true);
      objectCenterDropdown.setVisible(true);
      objectCenterDropdownDecoy.setActive(true);
      objectCenterDropdownDecoy.setVisible(true);
   }
}

function objectTransformDropdown::toggle()
{
   if ( objectTransformDropdown.visible  )
   {
      EWorldEditorToolbar-->objectTransform.setStateOn(false);
      objectTransformDropdownDecoy.setVisible(false);
      objectTransformDropdownDecoy.setActive(false);
      objectTransformDropdown.setVisible(false);
   }
   else
   {
      EWorldEditorToolbar-->objectTransform.setStateOn(true);
      objectTransformDropdown.setVisible(true);
      objectTransformDropdownDecoy.setActive(true);
      objectTransformDropdownDecoy.setVisible(true);
   }
}

function objectSnapDropdownDecoy::onMouseLeave()
{
   objectSnapDropdown.toggle();
}

function objectCenterDropdownDecoy::onMouseLeave()
{
   objectCenterDropdown.toggle();
}

function objectTransformDropdownDecoy::onMouseLeave()
{
   objectTransformDropdown.toggle();
}

//------------------------------------------------------------------------------

function EWAddSimGroupButton::onDefaultClick( %this )
{
   EWorldEditor.addSimGroup();
}

function EWAddSimGroupButton::onCtrlClick( %this )
{
   EWorldEditor.addSimGroup( true );
}

//------------------------------------------------------------------------------

function EWToolsToolbar::reset( %this )
{
   %count = ToolsToolbarArray.getCount();
   for( %i = 0 ; %i < %count; %i++ )
      ToolsToolbarArray.getObject(%i).setVisible(true);

   %this.setExtent((29 + 4) * %count + 12, 33);
   %this.isClosed = 0;
   EWToolsToolbar.isDynamic = 0;
      
   EWToolsToolbarDecoy.setVisible(false);
   EWToolsToolbarDecoy.setExtent((29 + 4) * %count + 4, 31);

  %this-->resizeArrow.setBitmap( "tools/gui/images/collapse-toolbar" );
}

function EWToolsToolbar::toggleSize( %this, %useDynamics )
{
   // toggles the size of the tooltoolbar. also goes through 
   // and hides each control not currently selected. we hide the controls
   // in a very neat, spiffy way

   if ( %this.isClosed == 0 )
   {
      %image = "tools/gui/images/expand-toolbar";
      
      for( %i = 0 ; %i < ToolsToolbarArray.getCount(); %i++ )
      {
         if( ToolsToolbarArray.getObject(%i).getValue() != 1 )
            ToolsToolbarArray.getObject(%i).setVisible(false);
      }
         
      %this.setExtent(43, 33);
      %this.isClosed = 1;
      
      if(!%useDynamics)
      {
         EWToolsToolbarDecoy.setVisible(true);
         EWToolsToolbar.isDynamic = 1;
      }
         
      EWToolsToolbarDecoy.setExtent(35, 31);
   }
   else
   {
      %image = "tools/gui/images/collapse-toolbar";

      %count = ToolsToolbarArray.getCount();
      for( %i = 0 ; %i < %count; %i++ )
         ToolsToolbarArray.getObject(%i).setVisible(true);
      
      %this.setExtent((29 + 4) * %count + 12, 33);
      %this.isClosed = 0;
      
      if(!%useDynamics)
      {
         EWToolsToolbarDecoy.setVisible(false);
         EWToolsToolbar.isDynamic = 0;
      }

      EWToolsToolbarDecoy.setExtent((29 + 4) * %count + 4, 32);
   }

  %this-->resizeArrow.setBitmap( %image );
  
}

function EWToolsToolbarDecoy::onMouseEnter( %this )
{
   EWToolsToolbar.toggleSize(true);
}

function EWToolsToolbarDecoy::onMouseLeave( %this )
{
   EWToolsToolbar.toggleSize(true);
}

//------------------------------------------------------------------------------

function EditorGuiStatusBar::reset( %this )
{
   EWorldEditorStatusBarInfo.clearInfo();
}

function EditorGuiStatusBar::getInfo( %this )
{
   return EWorldEditorStatusBarInfo.getValue();
}

function EditorGuiStatusBar::setInfo( %this, %text )
{
   EWorldEditorStatusBarInfo.setText(%text);
}

function EditorGuiStatusBar::clearInfo( %this )
{
   EWorldEditorStatusBarInfo.setText("");
}

function EditorGuiStatusBar::getSelection( %this )
{
   return EWorldEditorStatusBarSelection.getValue();
}

function EditorGuiStatusBar::setSelection( %this, %text )
{
   EWorldEditorStatusBarSelection.setText(%text);
}

function EditorGuiStatusBar::setSelectionObjectsByCount( %this, %count )
{
   %text = " objects selected";
   if(%count == 1)
      %text = " object selected";

   EWorldEditorStatusBarSelection.setText(%count @ %text);
}

function EditorGuiStatusBar::clearSelection( %this )
{
   EWorldEditorStatusBarSelection.setText("");
}

function EditorGuiStatusBar::getCamera( %this )
{
   return EWorldEditorStatusBarCamera.getText();
}

function EditorGuiStatusBar::setCamera( %this, %text )
{
   %id = EWorldEditorStatusBarCamera.findText( %text );
   if( %id != -1 )
   {
      if ( EWorldEditorStatusBarCamera.getSelected() != %id )
         EWorldEditorStatusBarCamera.setSelected( %id, true );
   }
}

function EWorldEditorStatusBarCamera::onWake( %this )
{
   %this.add( "Standard Camera" );
   %this.add( "1st Person Camera" );
   %this.add( "3rd Person Camera" );
   %this.add( "Orbit Camera" );
   %this.add( "Top View" );
   %this.add( "Bottom View" );
   %this.add( "Left View" );
   %this.add( "Right View" );
   %this.add( "Front View" );
   %this.add( "Back View" );
   %this.add( "Isometric View" );
   %this.add( "Smooth Camera" );
   %this.add( "Smooth Rot Camera" );
}

function EWorldEditorStatusBarCamera::onSelect( %this, %id, %text )
{
   switch$( %text )
   {
      case "Top View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeTop );

      case "Bottom View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeBottom );

      case "Left View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeLeft );

      case "Right View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeRight );

      case "Front View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeFront );

      case "Back View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeBack );

      case "Isometric View":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypeIsometric );

      case "Standard Camera":
         commandToServer( 'SetEditorCameraStandard' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );

      case "1st Person Camera":
         commandToServer( 'SetEditorCameraPlayer' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );

      case "3rd Person Camera":
         commandToServer( 'SetEditorCameraPlayerThird' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );

      case "Orbit Camera":
         commandToServer( 'SetEditorOrbitCamera' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );

      case "Smooth Camera":
         commandToServer( 'SetEditorCameraNewton' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );

      case "Smooth Rot Camera":
         commandToServer( 'SetEditorCameraNewtonDamped' );
         EditorGui.setDisplayType( $EditTsCtrl::DisplayTypePerspective );
   }
}

//------------------------------------------------------------------------------------
// Each a gui slider bar is pushed on the editor gui, it maps itself with value
// located in its connected text control
//------------------------------------------------------------------------------------
function softSnapSizeSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(EWorldEditorToolbar-->softSnapSizeTextEdit.getValue());
}
function softSnapSizeSliderCtrlContainer::onSliderChanged(%this)
{
   EWorldEditor.setSoftSnapSize( %this-->slider.value );
   EWorldEditor.syncGui();
}
//------------------------------------------------------------------------------------

function PaintBrushSizeSliderCtrlContainer::onWake(%this)
{
   %this-->slider.range = "1" SPC getWord(ETerrainEditor.maxBrushSize, 0);
   %this-->slider.setValue(PaintBrushSizeTextEditContainer-->textEdit.getValue());
}

function PaintBrushPressureSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(PaintBrushPressureTextEditContainer-->textEdit.getValue() / 100);
}

function PaintBrushSoftnessSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(PaintBrushSoftnessTextEditContainer-->textEdit.getValue() / 100);
}

//------------------------------------------------------------------------------------

function TerrainBrushSizeSliderCtrlContainer::onWake(%this)
{
   %this-->slider.range = "1" SPC getWord(ETerrainEditor.maxBrushSize, 0);
   %this-->slider.setValue(TerrainBrushSizeTextEditContainer-->textEdit.getValue());
}

function TerrainBrushPressureSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(TerrainBrushPressureTextEditContainer-->textEdit.getValue() / 100.0);
}

function TerrainBrushSoftnessSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(TerrainBrushSoftnessTextEditContainer-->textEdit.getValue() / 100.0);
}

function TerrainSetHeightSliderCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(TerrainSetHeightTextEditContainer-->textEdit.getValue());
}
//------------------------------------------------------------------------------------
function CameraSpeedDropdownCtrlContainer::onWake(%this)
{
   %this-->slider.setValue(CameraSpeedDropdownContainer-->textEdit.getText());
}

//------------------------------------------------------------------------------------
// Callbacks to close the dropdown slider controls like the camera speed,
// that are marked with this class name.

function EditorDropdownSliderContainer::onMouseDown(%this)
{
   Canvas.popDialog(%this);
}

function EditorDropdownSliderContainer::onRightMouseDown(%this)
{
   Canvas.popDialog(%this);
}