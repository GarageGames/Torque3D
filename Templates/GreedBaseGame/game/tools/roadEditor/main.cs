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

function initializeRoadEditor()
{
   echo( " - Initializing Road and Path Editor" );
   
   exec( "./roadEditor.cs" );
   exec( "./RoadEditorGui.gui" );
   exec( "./RoadEditorToolbar.gui");
   exec( "./roadEditorGui.cs" );
   
   // Add ourselves to EditorGui, where all the other tools reside
   RoadEditorGui.setVisible( false ); 
   RoadEditorToolbar.setVisible( false );
   RoadEditorOptionsWindow.setVisible( false );
   RoadEditorTreeWindow.setVisible( false );
   
   EditorGui.add( RoadEditorGui );
   EditorGui.add( RoadEditorToolbar );
   EditorGui.add( RoadEditorOptionsWindow );
   EditorGui.add( RoadEditorTreeWindow );
   
   new ScriptObject( RoadEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = RoadEditorGui;
   };
   
   %map = new ActionMap();
   %map.bindCmd( keyboard, "backspace", "RoadEditorGui.onDeleteKey();", "" );
   %map.bindCmd( keyboard, "1", "RoadEditorGui.prepSelectionMode();", "" );  
   %map.bindCmd( keyboard, "2", "ToolsPaletteArray->RoadEditorMoveMode.performClick();", "" );  
   %map.bindCmd( keyboard, "4", "ToolsPaletteArray->RoadEditorScaleMode.performClick();", "" );  
   %map.bindCmd( keyboard, "5", "ToolsPaletteArray->RoadEditorAddRoadMode.performClick();", "" );  
   %map.bindCmd( keyboard, "=", "ToolsPaletteArray->RoadEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadadd", "ToolsPaletteArray->RoadEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "-", "ToolsPaletteArray->RoadEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadminus", "ToolsPaletteArray->RoadEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "z", "RoadEditorShowSplineBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "x", "RoadEditorWireframeBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "v", "RoadEditorShowRoadBtn.performClick();", "" ); 
   RoadEditorPlugin.map = %map;
   
   RoadEditorPlugin.initSettings();
}

function destroyRoadEditor()
{
}

function RoadEditorPlugin::onWorldEditorStartup( %this )
{  
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Road and Path Editor", "", RoadEditorPlugin );      
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Road Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "RoadEditorPlugin", "RoadEditorPalette", expandFilename("tools/worldEditor/images/toolbar/road-path-editor"), %tooltip );
   
   //connect editor windows
   GuiWindowCtrl::attach( RoadEditorOptionsWindow, RoadEditorTreeWindow);
   
   // Add ourselves to the Editor Settings window
   exec( "./RoadEditorSettingsTab.gui" );
   ESettingsWindow.addTabPage( ERoadEditorSettingsPage );
}

function RoadEditorPlugin::onActivated( %this )
{
   %this.readSettings();
   
   ToolsPaletteArray->RoadEditorAddRoadMode.performClick();
   EditorGui.bringToFront( RoadEditorGui );
   
   RoadEditorGui.setVisible( true );
   RoadEditorGui.makeFirstResponder( true );
   RoadEditorToolbar.setVisible( true );   
   
   RoadEditorOptionsWindow.setVisible( true );
   RoadEditorTreeWindow.setVisible( true );
   
   RoadTreeView.open(ServerDecalRoadSet,true);
   
   %this.map.push();

   // Set the status bar here until all tool have been hooked up
   EditorGuiStatusBar.setInfo("Road editor.");
   EditorGuiStatusBar.setSelection("");
   
   Parent::onActivated(%this);
}

function RoadEditorPlugin::onDeactivated( %this )
{
   %this.writeSettings();
   
   RoadEditorGui.setVisible( false );
   RoadEditorToolbar.setVisible( false );   
   RoadEditorOptionsWindow.setVisible( false );
   RoadEditorTreeWindow.setVisible( false );
   %this.map.pop();
   
   Parent::onDeactivated(%this);
}

function RoadEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %hasSelection = false;
   
   if( isObject( RoadEditorGui.road ) )
      %hasSelection = true;
   
   %editMenu.enableItem( 3, false ); // Cut
   %editMenu.enableItem( 4, false ); // Copy
   %editMenu.enableItem( 5, false ); // Paste 
   %editMenu.enableItem( 6, %hasSelection ); // Delete
   %editMenu.enableItem( 8, false ); // Deselect 
}

function RoadEditorPlugin::handleDelete( %this )
{
   RoadEditorGui.onDeleteKey();
}

function RoadEditorPlugin::handleEscape( %this )
{
   return RoadEditorGui.onEscapePressed();  
}

function RoadEditorPlugin::isDirty( %this )
{
   return RoadEditorGui.isDirty;
}

function RoadEditorPlugin::onSaveMission( %this, %missionFile )
{
   if( RoadEditorGui.isDirty )
   {
      MissionGroup.save( %missionFile );
      RoadEditorGui.isDirty = false;
   }
}

function RoadEditorPlugin::setEditorFunction( %this )
{
   %terrainExists = parseMissionGroup( "TerrainBlock" );

   if( %terrainExists == false )
      MessageBoxYesNoCancel("No Terrain","Would you like to create a New Terrain?", "Canvas.pushDialog(CreateNewTerrainGui);");
   
   return %terrainExists;
}

//-----------------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------------

function RoadEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "RoadEditor", true );
   
   EditorSettings.setDefaultValue(  "DefaultWidth",         "10" );
   EditorSettings.setDefaultValue(  "HoverSplineColor",     "255 0 0 255" );
   EditorSettings.setDefaultValue(  "SelectedSplineColor",  "0 255 0 255" );
   EditorSettings.setDefaultValue(  "HoverNodeColor",       "255 255 255 255" ); //<-- Not currently used
   EditorSettings.setDefaultValue(  "MaterialName",         "DefaultDecalRoadMaterial" );
   
   EditorSettings.endGroup();
}

function RoadEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "RoadEditor", true );
   
   RoadEditorGui.DefaultWidth         = EditorSettings.value("DefaultWidth");
   RoadEditorGui.HoverSplineColor     = EditorSettings.value("HoverSplineColor");
   RoadEditorGui.SelectedSplineColor  = EditorSettings.value("SelectedSplineColor");
   RoadEditorGui.HoverNodeColor       = EditorSettings.value("HoverNodeColor");
   RoadEditorGui.materialName         = EditorSettings.value("MaterialName");   
   
   EditorSettings.endGroup();  
}

function RoadEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "RoadEditor", true );
   
   EditorSettings.setValue( "DefaultWidth",           RoadEditorGui.DefaultWidth );
   EditorSettings.setValue( "HoverSplineColor",       RoadEditorGui.HoverSplineColor );
   EditorSettings.setValue( "SelectedSplineColor",    RoadEditorGui.SelectedSplineColor );
   EditorSettings.setValue( "HoverNodeColor",         RoadEditorGui.HoverNodeColor );
   EditorSettings.setValue( "MaterialName",           RoadEditorGui.materialName );
   
   EditorSettings.endGroup();
}
