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

function initializeRiverEditor()
{
   echo(" % - Initializing River Editor");
     
   exec( "./riverEditor.cs" );
   exec( "./riverEditorGui.gui" );
   exec( "./riverEditorToolbar.gui" );
   exec( "./riverEditorGui.cs" );
   
   // Add ourselves to EditorGui, where all the other tools reside
   RiverEditorGui.setVisible( false );  
   RiverEditorToolbar.setVisible(false); 
   RiverEditorOptionsWindow.setVisible( false );
   RiverEditorTreeWindow.setVisible( false );
   
   EditorGui.add( RiverEditorGui );
   EditorGui.add( RiverEditorToolbar );
   EditorGui.add( RiverEditorOptionsWindow );
   EditorGui.add( RiverEditorTreeWindow );
   
   new ScriptObject( RiverEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = RiverEditorGui;
   };
   
   %map = new ActionMap();
   %map.bindCmd( keyboard, "backspace", "RiverEditorGui.deleteNode();", "" );
   %map.bindCmd( keyboard, "1", "RiverEditorGui.prepSelectionMode();", "" );  
   %map.bindCmd( keyboard, "2", "ToolsPaletteArray->RiverEditorMoveMode.performClick();", "" );  
   %map.bindCmd( keyboard, "3", "ToolsPaletteArray->RiverEditorRotateMode.performClick();", "" );  
   %map.bindCmd( keyboard, "4", "ToolsPaletteArray->RiverEditorScaleMode.performClick();", "" );  
   %map.bindCmd( keyboard, "5", "ToolsPaletteArray->RiverEditorAddRiverMode.performClick();", "" );  
   %map.bindCmd( keyboard, "=", "ToolsPaletteArray->RiverEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadadd", "ToolsPaletteArray->RiverEditorInsertPointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "-", "ToolsPaletteArray->RiverEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "numpadminus", "ToolsPaletteArray->RiverEditorRemovePointMode.performClick();", "" );  
   %map.bindCmd( keyboard, "z", "RiverEditorShowSplineBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "x", "RiverEditorWireframeBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "v", "RiverEditorShowRoadBtn.performClick();", "" );   
   RiverEditorPlugin.map = %map;

   RiverEditorPlugin.initSettings();
}

function destroyRiverEditor()
{
}

function RiverEditorPlugin::onWorldEditorStartup( %this )
{    
    // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "River Editor", "", RiverEditorPlugin );   
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "River Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "RiverEditorPlugin", "RiverEditorPalette", expandFilename("tools/worldEditor/images/toolbar/river-editor"), %tooltip );

   //connect editor windows   
   GuiWindowCtrl::attach( RiverEditorOptionsWindow, RiverEditorTreeWindow);
   
   // Add ourselves to the Editor Settings window
   exec( "./RiverEditorSettingsTab.gui" );
   ESettingsWindow.addTabPage( ERiverEditorSettingsPage );
}

function RiverEditorPlugin::onActivated( %this )
{
   %this.readSettings();
   
   $River::EditorOpen = true;   
   
   ToolsPaletteArray->RiverEditorAddRiverMode.performClick();
   EditorGui.bringToFront( RiverEditorGui );
   
   RiverEditorGui.setVisible(true);
   RiverEditorGui.makeFirstResponder( true );
   RiverEditorToolbar.setVisible(true);
   
   RiverEditorOptionsWindow.setVisible( true );
   RiverEditorTreeWindow.setVisible( true );
   
   RiverTreeView.open(ServerRiverSet,true);
   %this.map.push();

   // Store this on a dynamic field
   // in order to restore whatever setting
   // the user had before.
   %this.prevGizmoAlignment = GlobalGizmoProfile.alignment;
   
   // The DecalEditor always uses Object alignment.
   GlobalGizmoProfile.alignment = "Object";
   
   // Set the status bar here until all tool have been hooked up
   EditorGuiStatusBar.setInfo("River editor.");
   EditorGuiStatusBar.setSelection("");
   
   // Allow the Gui to setup.
   RiverEditorGui.onEditorActivated(); 
   
   Parent::onActivated(%this);
}

function RiverEditorPlugin::onDeactivated( %this )
{
   %this.writeSettings();
   
   $River::EditorOpen = false;   
   
   RiverEditorGui.setVisible(false);
   RiverEditorToolbar.setVisible(false);
   RiverEditorOptionsWindow.setVisible( false );
   RiverEditorTreeWindow.setVisible( false );
   %this.map.pop();
   
   // Restore the previous Gizmo
   // alignment settings.
   GlobalGizmoProfile.alignment = %this.prevGizmoAlignment;  
   
   // Allow the Gui to cleanup.
   RiverEditorGui.onEditorDeactivated(); 
   
   Parent::onDeactivated(%this);
}

function RiverEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %hasSelection = false;
   
   if( isObject( RiverEditorGui.river ) )
      %hasSelection = true;
   
   %editMenu.enableItem( 3, false ); // Cut
   %editMenu.enableItem( 4, false ); // Copy
   %editMenu.enableItem( 5, false ); // Paste  
   %editMenu.enableItem( 6, %hasSelection ); // Delete
   %editMenu.enableItem( 8, false ); // Deselect 
}

function RiverEditorPlugin::handleDelete( %this )
{
   RiverEditorGui.deleteNode();
}

function RiverEditorPlugin::handleEscape( %this )
{
   return RiverEditorGui.onEscapePressed();  
}

function RiverEditorPlugin::isDirty( %this )
{
   return RiverEditorGui.isDirty;
}

function RiverEditorPlugin::onSaveMission( %this, %missionFile )
{
   if( RiverEditorGui.isDirty )
   {
      MissionGroup.save( %missionFile );
      RiverEditorGui.isDirty = false;
   }
}

//-----------------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------------

function RiverEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "RiverEditor", true );
   
   EditorSettings.setDefaultValue(  "DefaultWidth",         "10" );
   EditorSettings.setDefaultValue(  "DefaultDepth",         "5" );
   EditorSettings.setDefaultValue(  "DefaultNormal",        "0 0 1" );
   EditorSettings.setDefaultValue(  "HoverSplineColor",     "255 0 0 255" );
   EditorSettings.setDefaultValue(  "SelectedSplineColor",  "0 255 0 255" );
   EditorSettings.setDefaultValue(  "HoverNodeColor",       "255 255 255 255" ); //<-- Not currently used
   
   EditorSettings.endGroup();
}

function RiverEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "RiverEditor", true );
   
   RiverEditorGui.DefaultWidth         = EditorSettings.value("DefaultWidth");
   RiverEditorGui.DefaultDepth         = EditorSettings.value("DefaultDepth");
   RiverEditorGui.DefaultNormal        = EditorSettings.value("DefaultNormal");
   RiverEditorGui.HoverSplineColor     = EditorSettings.value("HoverSplineColor");
   RiverEditorGui.SelectedSplineColor  = EditorSettings.value("SelectedSplineColor");
   RiverEditorGui.HoverNodeColor       = EditorSettings.value("HoverNodeColor");
   
   EditorSettings.endGroup();  
}

function RiverEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "RiverEditor", true );
   
   EditorSettings.setValue( "DefaultWidth",           RiverEditorGui.DefaultWidth );
   EditorSettings.setValue( "DefaultDepth",           RiverEditorGui.DefaultDepth );
   EditorSettings.setValue( "DefaultNormal",          RiverEditorGui.DefaultNormal );
   EditorSettings.setValue( "HoverSplineColor",       RiverEditorGui.HoverSplineColor );
   EditorSettings.setValue( "SelectedSplineColor",    RiverEditorGui.SelectedSplineColor );
   EditorSettings.setValue( "HoverNodeColor",         RiverEditorGui.HoverNodeColor );

   EditorSettings.endGroup();
}
