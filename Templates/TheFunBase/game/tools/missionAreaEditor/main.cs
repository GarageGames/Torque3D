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

function initializeMissionAreaEditor()
{
   echo(" % - Initializing Mission Area Editor");
     
   exec( "./missionAreaEditor.ed.cs" );
   exec( "./missionAreaEditorGui.ed.gui" );
   exec( "./missionAreaEditorGui.ed.cs" );
   
   // Add ourselves to EditorGui, where all the other tools reside
   MissionAreaEditorGui.setVisible( false );  
   MissionAreaEditorTerrainWindow.setVisible( false );
   MissionAreaEditorPropertiesWindow.setVisible( false );
   
   EditorGui.add( MissionAreaEditorGui );
   EditorGui.add( MissionAreaEditorTerrainWindow );
   EditorGui.add( MissionAreaEditorPropertiesWindow );
   
   new ScriptObject( MissionAreaEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = MissionAreaEditorGui;
   };

   MissionAreaEditorPlugin.initSettings();
}

function destroyMissionAreaEditor()
{
}

function MissionAreaEditorPlugin::onWorldEditorStartup( %this )
{    
    // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Mission Area Editor", "", MissionAreaEditorPlugin );   
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Mission Area Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "MissionAreaEditorPlugin", "MissionAreaEditorPalette", expandFilename("tools/missionAreaEditor/images/mission-area"), %tooltip );

   //connect editor windows   
   GuiWindowCtrl::attach( MissionAreaEditorPropertiesWindow, MissionAreaEditorTerrainWindow);
}

function MissionAreaEditorPlugin::onActivated( %this )
{
   %this.readSettings();
   
   EditorGui.bringToFront( MissionAreaEditorGui );
   
   MissionAreaEditorGui.setVisible(true);
   MissionAreaEditorGui.makeFirstResponder( true );
   
   MissionAreaEditorTerrainWindow.setVisible( true );
   MissionAreaEditorPropertiesWindow.setVisible( true );
   
   // Set the status bar here until all tool have been hooked up
   EditorGuiStatusBar.setInfo("Mission Area Editor.");
   EditorGuiStatusBar.setSelection("");
   
   // Allow the Gui to setup.
   MissionAreaEditorGui.onEditorActivated(); 
   
   Parent::onActivated(%this);
}

function MissionAreaEditorPlugin::onDeactivated( %this )
{
   %this.writeSettings();
   
   MissionAreaEditorGui.setVisible(false);
   MissionAreaEditorTerrainWindow.setVisible( false );
   MissionAreaEditorPropertiesWindow.setVisible( false );
   
   // Allow the Gui to cleanup.
   MissionAreaEditorGui.onEditorDeactivated(); 
   
   Parent::onDeactivated(%this);
}

//-----------------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------------

function MissionAreaEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "MissionAreaEditor", true );
   
   EditorSettings.setDefaultValue(  "MissionBoundsColor",   "255 255 255" );
   
   EditorSettings.endGroup();
}

function MissionAreaEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "MissionAreaEditor", true );
   
   MissionAreaEditorTerrainEditor.missionBoundsColor     = EditorSettings.value("MissionBoundsColor");
   
   EditorSettings.endGroup();  
}

function MissionAreaEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "MissionAreaEditor", true );
   
   EditorSettings.setValue( "MissionBoundsColor",     MissionAreaEditorTerrainEditor.missionBoundsColor );

   EditorSettings.endGroup();
}
