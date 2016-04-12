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

function initializeMeshRoadEditor()
{
   echo(" % - Initializing Mesh Road Editor");
     
   exec( "./meshRoadEditor.cs" );
   exec( "./meshRoadEditorGui.gui" );
   exec( "./meshRoadEditorToolbar.gui");
   exec( "./meshRoadEditorGui.cs" );
   
   MeshRoadEditorGui.setVisible( false );  
   MeshRoadEditorOptionsWindow.setVisible( false );  
   MeshRoadEditorToolbar.setVisible( false ); 
   MeshRoadEditorTreeWindow.setVisible( false ); 
   
   EditorGui.add( MeshRoadEditorGui );
   EditorGui.add( MeshRoadEditorOptionsWindow );
   EditorGui.add( MeshRoadEditorToolbar );
   EditorGui.add( MeshRoadEditorTreeWindow );
      
   new ScriptObject( MeshRoadEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = MeshRoadEditorGui;
   };
   
   %map = new ActionMap();
   %map.bindCmd( keyboard, "backspace", "MeshRoadEditorGui.deleteNode();", "" );
   %map.bindCmd( keyboard, "1", "MeshRoadEditorGui.prepSelectionMode();", "" );  
   %map.bindCmd( keyboard, "2", "ToolsPaletteArray->MeshRoadEditorMoveMode.performClick();", "" );  
   %map.bindCmd( keyboard, "3", "ToolsPaletteArray->MeshRoadEditorRotateMode.performClick();", "" );  
   %map.bindCmd( keyboard, "4", "ToolsPaletteArray->MeshRoadEditorScaleMode.performClick();", "" );  
   %map.bindCmd( keyboard, "5", "ToolsPaletteArray->MeshRoadEditorAddRoadMode.performClick();", "" );  
   %map.bindCmd( keyboard, "=", "ToolsPaletteArray->MeshRoadEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadadd", "ToolsPaletteArray->MeshRoadEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "-", "ToolsPaletteArray->MeshRoadEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadminus", "ToolsPaletteArray->MeshRoadEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "z", "MeshRoadEditorShowSplineBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "x", "MeshRoadEditorWireframeBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "v", "MeshRoadEditorShowRoadBtn.performClick();", "" );  
   MeshRoadEditorPlugin.map = %map;
   
   MeshRoadEditorPlugin.initSettings();
}

function destroyMeshRoadEditor()
{
}

function MeshRoadEditorPlugin::onWorldEditorStartup( %this )
{     
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Mesh Road Editor", "", MeshRoadEditorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Mesh Road Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "MeshRoadEditorPlugin", "MeshRoadEditorPalette", expandFilename("tools/worldEditor/images/toolbar/mesh-road-editor"), %tooltip );

   //connect editor windows
   GuiWindowCtrl::attach( MeshRoadEditorOptionsWindow, MeshRoadEditorTreeWindow);
   
   // Add ourselves to the Editor Settings window
   exec( "./meshRoadEditorSettingsTab.gui" );
   ESettingsWindow.addTabPage( EMeshRoadEditorSettingsPage );
}

function MeshRoadEditorPlugin::onActivated( %this )
{
   %this.readSettings();
   
   ToolsPaletteArray->MeshRoadEditorAddRoadMode.performClick();
   EditorGui.bringToFront( MeshRoadEditorGui );
   MeshRoadEditorGui.setVisible( true );
   MeshRoadEditorGui.makeFirstResponder( true );
   MeshRoadEditorOptionsWindow.setVisible( true );
   MeshRoadEditorToolbar.setVisible( true );  
   MeshRoadEditorTreeWindow.setVisible( true );
   MeshRoadTreeView.open(ServerMeshRoadSet,true);
   %this.map.push();
   
   // Store this on a dynamic field
   // in order to restore whatever setting
   // the user had before.
   %this.prevGizmoAlignment = GlobalGizmoProfile.alignment;
   
   // The DecalEditor always uses Object alignment.
   GlobalGizmoProfile.alignment = "Object";
   
   // Set the status bar here until all tool have been hooked up
   EditorGuiStatusBar.setInfo("Mesh road editor.");
   EditorGuiStatusBar.setSelection("");
   
   Parent::onActivated(%this);
}

function MeshRoadEditorPlugin::onDeactivated( %this )
{   
   %this.writeSettings();
   
   MeshRoadEditorGui.setVisible( false );
   MeshRoadEditorOptionsWindow.setVisible( false );
   MeshRoadEditorToolbar.setVisible( false );  
   MeshRoadEditorTreeWindow.setVisible( false );
   %this.map.pop();
   
   // Restore the previous Gizmo
   // alignment settings.
   GlobalGizmoProfile.alignment = %this.prevGizmoAlignment; 
   
   Parent::onDeactivated(%this);  
}

function MeshRoadEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %hasSelection = false;
   
   if( isObject( MeshRoadEditorGui.road ) )
      %hasSelection = true;
      
   %editMenu.enableItem( 3, false ); // Cut
   %editMenu.enableItem( 4, false ); // Copy
   %editMenu.enableItem( 5, false ); // Paste  
   %editMenu.enableItem( 6, %hasSelection ); // Delete
   %editMenu.enableItem( 8, false ); // Deselect   
}

function MeshRoadEditorPlugin::handleDelete( %this )
{
   MeshRoadEditorGui.deleteNode();
}

function MeshRoadEditorPlugin::handleEscape( %this )
{
   return MeshRoadEditorGui.onEscapePressed();  
}

function MeshRoadEditorPlugin::isDirty( %this )
{
   return MeshRoadEditorGui.isDirty;
}

function MeshRoadEditorPlugin::onSaveMission( %this, %missionFile )
{
   if( MeshRoadEditorGui.isDirty )
   {
      MissionGroup.save( %missionFile );
      MeshRoadEditorGui.isDirty = false;
   }
}

//-----------------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------------

function MeshRoadEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "MeshRoadEditor", true );
   
   EditorSettings.setDefaultValue(  "DefaultWidth",         "10" );
   EditorSettings.setDefaultValue(  "DefaultDepth",         "5" );
   EditorSettings.setDefaultValue(  "DefaultNormal",        "0 0 1" );
   EditorSettings.setDefaultValue(  "HoverSplineColor",     "255 0 0 255" );
   EditorSettings.setDefaultValue(  "SelectedSplineColor",  "0 255 0 255" );
   EditorSettings.setDefaultValue(  "HoverNodeColor",       "255 255 255 255" ); //<-- Not currently used
   EditorSettings.setDefaultValue(  "TopMaterialName",      "DefaultRoadMaterialTop" );
   EditorSettings.setDefaultValue(  "BottomMaterialName",   "DefaultRoadMaterialOther" );
   EditorSettings.setDefaultValue(  "SideMaterialName",     "DefaultRoadMaterialOther" );
   
   EditorSettings.endGroup();
}

function MeshRoadEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "MeshRoadEditor", true );
   
   MeshRoadEditorGui.DefaultWidth         = EditorSettings.value("DefaultWidth");
   MeshRoadEditorGui.DefaultDepth         = EditorSettings.value("DefaultDepth");
   MeshRoadEditorGui.DefaultNormal        = EditorSettings.value("DefaultNormal");
   MeshRoadEditorGui.HoverSplineColor     = EditorSettings.value("HoverSplineColor");
   MeshRoadEditorGui.SelectedSplineColor  = EditorSettings.value("SelectedSplineColor");
   MeshRoadEditorGui.HoverNodeColor       = EditorSettings.value("HoverNodeColor");
   MeshRoadEditorGui.topMaterialName      = EditorSettings.value("TopMaterialName");
   MeshRoadEditorGui.bottomMaterialName   = EditorSettings.value("BottomMaterialName");
   MeshRoadEditorGui.sideMaterialName     = EditorSettings.value("SideMaterialName");
   
   EditorSettings.endGroup();  
}

function MeshRoadEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "MeshRoadEditor", true );
   
   EditorSettings.setValue( "DefaultWidth",           MeshRoadEditorGui.DefaultWidth );
   EditorSettings.setValue( "DefaultDepth",           MeshRoadEditorGui.DefaultDepth );
   EditorSettings.setValue( "DefaultNormal",          MeshRoadEditorGui.DefaultNormal );
   EditorSettings.setValue( "HoverSplineColor",       MeshRoadEditorGui.HoverSplineColor );
   EditorSettings.setValue( "SelectedSplineColor",    MeshRoadEditorGui.SelectedSplineColor );
   EditorSettings.setValue( "HoverNodeColor",         MeshRoadEditorGui.HoverNodeColor );
   EditorSettings.setValue( "TopMaterialName",        MeshRoadEditorGui.topMaterialName );
   EditorSettings.setValue( "BottomMaterialName",     MeshRoadEditorGui.bottomMaterialName );
   EditorSettings.setValue( "SideMaterialName",       MeshRoadEditorGui.sideMaterialName );
   
   EditorSettings.endGroup();
}