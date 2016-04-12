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
// Shape Editor
//------------------------------------------------------------------------------

function initializeShapeEditor()
{
   echo(" % - Initializing Shape Editor");

   exec("./gui/Profiles.ed.cs");

   exec("./gui/shapeEdPreviewWindow.ed.gui");
   exec("./gui/shapeEdAnimWindow.ed.gui");
   exec("./gui/shapeEdAdvancedWindow.ed.gui");
   exec("./gui/ShapeEditorToolbar.ed.gui");
   exec("./gui/shapeEdSelectWindow.ed.gui");
   exec("./gui/shapeEdPropWindow.ed.gui");

   exec("./scripts/shapeEditor.ed.cs");
   exec("./scripts/shapeEditorHints.ed.cs");
   exec("./scripts/shapeEditorActions.ed.cs");

   // Add windows to editor gui
   ShapeEdPreviewGui.setVisible(false);
   ShapeEdAnimWindow.setVisible(false);

   ShapeEditorToolbar.setVisible(false);
   ShapeEdSelectWindow.setVisible(false);
   ShapeEdPropWindow.setVisible(false);

   EditorGui.add(ShapeEdPreviewGui);
   EditorGui.add(ShapeEdAnimWindow);
   EditorGui.add(ShapeEdAdvancedWindow);

   EditorGui.add(ShapeEditorToolbar);
   EditorGui.add(ShapeEdSelectWindow);
   EditorGui.add(ShapeEdPropWindow);

   new ScriptObject(ShapeEditorPlugin)
   {
      superClass = "EditorPlugin";
      editorGui = ShapeEdShapeView;
   };

   %map = new ActionMap();
   %map.bindCmd( keyboard, "escape", "ToolsToolbarArray->WorldEditorInspectorPalette.performClick();", "" );
   %map.bindCmd( keyboard, "1", "ShapeEditorNoneModeBtn.performClick();", "" );
   %map.bindCmd( keyboard, "2", "ShapeEditorMoveModeBtn.performClick();", "" );
   %map.bindCmd( keyboard, "3", "ShapeEditorRotateModeBtn.performClick();", "" );
   //%map.bindCmd( keyboard, "4", "ShapeEditorScaleModeBtn.performClick();", "" ); // not needed for the shape editor
   %map.bindCmd( keyboard, "n", "ShapeEditorToolbar->showNodes.performClick();", "" );
   %map.bindCmd( keyboard, "t", "ShapeEditorToolbar->ghostMode.performClick();", "" );
   %map.bindCmd( keyboard, "r", "ShapeEditorToolbar->wireframeMode.performClick();", "" );
   %map.bindCmd( keyboard, "f", "ShapeEditorToolbar->fitToShapeBtn.performClick();", "" );
   %map.bindCmd( keyboard, "g", "ShapeEditorToolbar->showGridBtn.performClick();", "" );
   %map.bindCmd( keyboard, "h", "ShapeEdSelectWindow->tabBook.selectPage( 2 );", "" ); // Load help tab
   %map.bindCmd( keyboard, "l", "ShapeEdSelectWindow->tabBook.selectPage( 1 );", "" ); // load Library Tab
   %map.bindCmd( keyboard, "j", "ShapeEdSelectWindow->tabBook.selectPage( 0 );", "" ); // load scene object Tab
   %map.bindCmd( keyboard, "SPACE", "ShapeEdAnimWindow.togglePause();", "" );
   %map.bindCmd( keyboard, "i", "ShapeEdSequences.onEditSeqInOut(\"in\", ShapeEdSeqSlider.getValue());", "" );
   %map.bindCmd( keyboard, "o", "ShapeEdSequences.onEditSeqInOut(\"out\", ShapeEdSeqSlider.getValue());", "" );
   %map.bindCmd( keyboard, "shift -", "ShapeEdSeqSlider.setValue(ShapeEdAnimWindow-->seqIn.getText());", "" );
   %map.bindCmd( keyboard, "shift =", "ShapeEdSeqSlider.setValue(ShapeEdAnimWindow-->seqOut.getText());", "" );
   %map.bindCmd( keyboard, "=", "ShapeEdAnimWindow-->stepFwdBtn.performClick();", "" );
   %map.bindCmd( keyboard, "-", "ShapeEdAnimWindow-->stepBkwdBtn.performClick();", "" );

   ShapeEditorPlugin.map = %map;

   ShapeEditorPlugin.initSettings();
}

function destroyShapeEditor()
{
}

function SetToggleButtonValue(%ctrl, %value)
{
   if ( %ctrl.getValue() != %value )
      %ctrl.performClick();
}

// Replace the command field in an Editor PopupMenu item (returns the original value)
function ShapeEditorPlugin::replaceMenuCmd(%this, %menuTitle, %id, %newCmd)
{
   %menu = EditorGui.findMenu( %menuTitle );
   %cmd = getField( %menu.item[%id], 2 );
   %menu.setItemCommand( %id, %newCmd );

   return %cmd;
}

function ShapeEditorPlugin::onWorldEditorStartup(%this)
{
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu("Shape Editor", "", ShapeEditorPlugin);

   // Add ourselves to the ToolsToolbar
   %tooltip = "Shape Editor (" @ %accel @ ")";
   EditorGui.addToToolsToolbar( "ShapeEditorPlugin", "ShapeEditorPalette", expandFilename("tools/worldEditor/images/toolbar/shape-editor"), %tooltip );

   // Add ourselves to the Editor Settings window
   exec( "./gui/ShapeEditorSettingsTab.gui" );
   ESettingsWindow.addTabPage( EShapeEditorSettingsPage );

   GuiWindowCtrl::attach(ShapeEdPropWindow, ShapeEdSelectWindow);
   ShapeEdAnimWindow.resize( -1, 526, 593, 53 );
   
   // Initialise gui
   ShapeEdSeqNodeTabBook.selectPage(0);
   ShapeEdAdvancedWindow-->tabBook.selectPage(0);
   ShapeEdSelectWindow-->tabBook.selectPage(0);
   ShapeEdSelectWindow.navigate("");

   SetToggleButtonValue( ShapeEditorToolbar-->orbitNodeBtn, 0 );
   SetToggleButtonValue( ShapeEditorToolbar-->ghostMode, 0 );

   // Initialise hints menu
   ShapeEdHintMenu.clear();
   %count = ShapeHintGroup.getCount();
   for (%i = 0; %i < %count; %i++)
   {
      %hint = ShapeHintGroup.getObject(%i);
      ShapeEdHintMenu.add(%hint.objectType, %hint);
   }
}

function ShapeEditorPlugin::open(%this, %filename)
{
   if ( !%this.isActivated )
   {
      // Activate the Shape Editor
      EditorGui.setEditor( %this, true );

      // Get editor settings (note the sun angle is not configured in the settings
      // dialog, so apply the settings here instead of in readSettings)
      %this.readSettings();
      ShapeEdShapeView.sunAngleX = EditorSettings.value("ShapeEditor/SunAngleX");
      ShapeEdShapeView.sunAngleZ = EditorSettings.value("ShapeEditor/SunAngleZ");
      EWorldEditor.forceLoadDAE = EditorSettings.value("forceLoadDAE");

      $wasInWireFrameMode = $gfx::wireframe;
      ShapeEditorToolbar-->wireframeMode.setStateOn($gfx::wireframe);

      if ( GlobalGizmoProfile.getFieldValue(alignment) $= "Object" )
         ShapeEdNodes-->objectTransform.setStateOn(1);
      else
         ShapeEdNodes-->worldTransform.setStateOn(1);

      // Initialise and show the shape editor
      ShapeEdShapeTreeView.open(MissionGroup);
      ShapeEdShapeTreeView.buildVisibleTree(true);

      ShapeEdPreviewGui.setVisible(true);
      ShapeEdSelectWindow.setVisible(true);
      ShapeEdPropWindow.setVisible(true);
      ShapeEdAnimWindow.setVisible(true);
      ShapeEdAdvancedWindow.setVisible(ShapeEditorToolbar-->showAdvanced.getValue());
      ShapeEditorToolbar.setVisible(true);
      EditorGui.bringToFront(ShapeEdPreviewGui);

      ToolsPaletteArray->WorldEditorMove.performClick();
      %this.map.push();

      // Switch to the ShapeEditor UndoManager
      %this.oldUndoMgr = Editor.getUndoManager();
      Editor.setUndoManager( ShapeEdUndoManager );

      ShapeEdShapeView.setDisplayType( EditorGui.currentDisplayType );
      %this.initStatusBar();

      // Customise menu bar
      %this.oldCamFitCmd = %this.replaceMenuCmd( "Camera", 8, "ShapeEdShapeView.fitToShape();" );
      %this.oldCamFitOrbitCmd = %this.replaceMenuCmd( "Camera", 9, "ShapeEdShapeView.fitToShape();" );

      Parent::onActivated(%this);
   }

   // Select the new shape
   if (isObject(ShapeEditor.shape) && (ShapeEditor.shape.baseShape $= %filename))
   {
      // Shape is already selected => re-highlight the selected material if necessary
      ShapeEdMaterials.updateSelectedMaterial(ShapeEdMaterials-->highlightMaterial.getValue());
   }
   else if (%filename !$= "")
   {
      ShapeEditor.selectShape(%filename, ShapeEditor.isDirty());

      // 'fitToShape' only works after the GUI has been rendered, so force a repaint first
      Canvas.repaint();
      ShapeEdShapeView.fitToShape();
   }
}

function ShapeEditorPlugin::onActivated(%this)
{
   %this.open("");

   // Try to start with the shape selected in the world editor
   %count = EWorldEditor.getSelectionSize();
   for (%i = 0; %i < %count; %i++)
   {
      %obj = EWorldEditor.getSelectedObject(%i);
      %shapeFile = ShapeEditor.getObjectShapeFile(%obj);
      if (%shapeFile !$= "")
      {
         if (!isObject(ShapeEditor.shape) || (ShapeEditor.shape.baseShape !$= %shapeFile))
         {
            // Call the 'onSelect' method directly if the object is not in the
            // MissionGroup tree (such as a Player or Projectile object).
            ShapeEdShapeTreeView.clearSelection();
            if (!ShapeEdShapeTreeView.selectItem(%obj))
               ShapeEdShapeTreeView.onSelect(%obj);

            // 'fitToShape' only works after the GUI has been rendered, so force a repaint first
            Canvas.repaint();
            ShapeEdShapeView.fitToShape();
         }
         break;
      }
   }
}

function ShapeEditorPlugin::initStatusBar(%this)
{
   EditorGuiStatusBar.setInfo("Shape editor ( Shift Click ) to speed up camera.");
   EditorGuiStatusBar.setSelection( ShapeEditor.shape.baseShape );
}

function ShapeEditorPlugin::onDeactivated(%this)
{
   %this.writeSettings();

   // Notify game objects if shape has been modified
   if ( ShapeEditor.isDirty() )
      ShapeEditor.shape.notifyShapeChanged();

   $gfx::wireframe = $wasInWireFrameMode;

   ShapeEdMaterials.updateSelectedMaterial(false);
   ShapeEditorToolbar.setVisible(false);

   ShapeEdPreviewGui.setVisible(false);
   ShapeEdSelectWindow.setVisible(false);
   ShapeEdPropWindow.setVisible(false);
   ShapeEdAnimWindow.setVisible(false);
   ShapeEdAdvancedWindow.setVisible(false);
   
   if( EditorGui-->MatEdPropertiesWindow.visible )
   {
      ShapeEdMaterials.editSelectedMaterialEnd( true );
   }

   %this.map.pop();

   // Restore the original undo manager
   Editor.setUndoManager( %this.oldUndoMgr );

   // Restore menu bar
   %this.replaceMenuCmd( "Camera", 8, %this.oldCamFitCmd );
   %this.replaceMenuCmd( "Camera", 9, %this.oldCamFitOrbitCmd );

   Parent::onDeactivated(%this);
}

function ShapeEditorPlugin::onExitMission( %this )
{
   // unselect the current shape
   ShapeEdShapeView.setModel( "" );
   if (ShapeEditor.shape != -1)
      ShapeEditor.shape.delete();
   ShapeEditor.shape = 0;
   ShapeEdUndoManager.clearAll();
   ShapeEditor.setDirty( false );

   ShapeEdSequenceList.clear();
   ShapeEdNodeTreeView.removeItem( 0 );
   ShapeEdPropWindow.update_onNodeSelectionChanged( -1 );
   ShapeEdDetailTree.removeItem( 0 );
   ShapeEdMaterialList.clear();

   ShapeEdMountWindow-->mountList.clear();
   ShapeEdThreadWindow-->seqList.clear();
   ShapeEdThreadList.clear();
}

function ShapeEditorPlugin::openShape( %this, %path, %discardChangesToCurrent )
{   
   EditorGui.setEditor( ShapeEditorPlugin );
   
   if( ShapeEditor.isDirty() && !%discardChangesToCurrent )
   {
      MessageBoxYesNo( "Save Changes?",
         "Save changes to current shape?",
         "ShapeEditor.saveChanges(); ShapeEditorPlugin.openShape(\"" @ %path @ "\");",
         "ShapeEditorPlugin.openShape(\"" @ %path @ "\");" );
      return;
   }
   
   ShapeEditor.selectShape( %path );
   ShapeEdShapeView.fitToShape();
}

function shapeEditorWireframeMode()
{
   $gfx::wireframe = !$gfx::wireframe;
   ShapeEditorToolbar-->wireframeMode.setStateOn($gfx::wireframe);
}

//-----------------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------------

function ShapeEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "ShapeEditor", true );

   // Display options
   EditorSettings.setDefaultValue( "BackgroundColor",    "0 0 0 100" );
   EditorSettings.setDefaultValue( "HighlightMaterial", 1 );
   EditorSettings.setDefaultValue( "ShowNodes", 1 );
   EditorSettings.setDefaultValue( "ShowBounds", 0 );
   EditorSettings.setDefaultValue( "ShowObjBox", 1 );
   EditorSettings.setDefaultValue( "RenderMounts", 1 );
   EditorSettings.setDefaultValue( "RenderCollision", 0 );

   // Grid
   EditorSettings.setDefaultValue( "ShowGrid", 1 );
   EditorSettings.setDefaultValue( "GridSize", 0.1 );
   EditorSettings.setDefaultValue( "GridDimension", "40 40" );

   // Sun
   EditorSettings.setDefaultValue( "SunDiffuseColor",    "255 255 255 255" );
   EditorSettings.setDefaultValue( "SunAmbientColor",    "180 180 180 255" );
   EditorSettings.setDefaultValue( "SunAngleX",          "45" );
   EditorSettings.setDefaultValue( "SunAngleZ",          "135" );

   // Sub-windows
   EditorSettings.setDefaultValue( "AdvancedWndVisible",   "1" );

   EditorSettings.endGroup();
}

function ShapeEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "ShapeEditor", true );

   // Display options
   ShapeEdPreviewGui-->previewBackground.color = ColorIntToFloat( EditorSettings.value("BackgroundColor") );
   SetToggleButtonValue( ShapeEdMaterials-->highlightMaterial, EditorSettings.value( "HighlightMaterial" ) );
   SetToggleButtonValue( ShapeEditorToolbar-->showNodes, EditorSettings.value( "ShowNodes" ) );
   SetToggleButtonValue( ShapeEditorToolbar-->showBounds, EditorSettings.value( "ShowBounds" ) );
   SetToggleButtonValue( ShapeEditorToolbar-->showObjBox, EditorSettings.value( "ShowObjBox" ) );
   SetToggleButtonValue( ShapeEditorToolbar-->renderColMeshes, EditorSettings.value( "RenderCollision" ) );
   SetToggleButtonValue( ShapeEdMountWindow-->renderMounts, EditorSettings.value( "RenderMounts" ) );

   // Grid
   SetToggleButtonValue( ShapeEditorToolbar-->showGridBtn, EditorSettings.value( "ShowGrid" ) );
   ShapeEdShapeView.gridSize = EditorSettings.value( "GridSize" );
   ShapeEdShapeView.gridDimension = EditorSettings.value( "GridDimension" );

   // Sun
   ShapeEdShapeView.sunDiffuse = EditorSettings.value("SunDiffuseColor");
   ShapeEdShapeView.sunAmbient = EditorSettings.value("SunAmbientColor");

   // Sub-windows
   SetToggleButtonValue( ShapeEditorToolbar-->showAdvanced, EditorSettings.value( "AdvancedWndVisible" ) );

   EditorSettings.endGroup();
}

function ShapeEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "ShapeEditor", true );

   // Display options
   EditorSettings.setValue( "BackgroundColor",     ColorFloatToInt( ShapeEdPreviewGui-->previewBackground.color ) );
   EditorSettings.setValue( "HighlightMaterial",   ShapeEdMaterials-->highlightMaterial.getValue() );
   EditorSettings.setValue( "ShowNodes",           ShapeEditorToolbar-->showNodes.getValue() );
   EditorSettings.setValue( "ShowBounds",          ShapeEditorToolbar-->showBounds.getValue() );
   EditorSettings.setValue( "ShowObjBox",          ShapeEditorToolbar-->showObjBox.getValue() );
   EditorSettings.setValue( "RenderCollision",     ShapeEditorToolbar-->renderColMeshes.getValue() );
   EditorSettings.setValue( "RenderMounts",        ShapeEdMountWindow-->renderMounts.getValue() );

   // Grid
   EditorSettings.setValue( "ShowGrid",            ShapeEditorToolbar-->showGridBtn.getValue() );
   EditorSettings.setValue( "GridSize",            ShapeEdShapeView.gridSize );
   EditorSettings.setValue( "GridDimension",       ShapeEdShapeView.gridDimension );

   // Sun
   EditorSettings.setValue( "SunDiffuseColor",     ShapeEdShapeView.sunDiffuse );
   EditorSettings.setValue( "SunAmbientColor",     ShapeEdShapeView.sunAmbient );
   EditorSettings.setValue( "SunAngleX",           ShapeEdShapeView.sunAngleX );
   EditorSettings.setValue( "SunAngleZ",           ShapeEdShapeView.sunAngleZ );

   // Sub-windows
   EditorSettings.setValue( "AdvancedWndVisible",    ShapeEditorToolbar-->showAdvanced.getValue() );

   EditorSettings.endGroup();
}
