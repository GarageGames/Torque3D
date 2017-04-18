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

// Initialization and shutdown code for particle editor plugin.


//---------------------------------------------------------------------------------------------

function initializeParticleEditor()
{
   echo( " % - Initializing Particle Editor" );
     
   exec( "./ParticleEditor.ed.gui" );
   exec( "./particleEditor.ed.cs" );
   exec( "./particleEditorUndo.ed.cs" );   
   exec( "./particleEmitterEditor.ed.cs" );
   exec( "./particleParticleEditor.ed.cs" );
   
   PE_Window.setVisible( false );
   EditorGui.add( PE_Window );
   
   new ScriptObject( ParticleEditorPlugin )
   {
      superClass = "WorldEditorPlugin";
      editorGui = EWorldEditor;
   };
      
   %map = new ActionMap();
   %map.bindCmd( keyboard, "1", "EWorldEditorNoneModeBtn.performClick();", "" );  // Select
   %map.bindCmd( keyboard, "2", "EWorldEditorMoveModeBtn.performClick();", "" );  // Move
   %map.bindCmd( keyboard, "3", "EWorldEditorRotateModeBtn.performClick();", "" );  // Rotate
   %map.bindCmd( keyboard, "4", "EWorldEditorScaleModeBtn.performClick();", "" );  // Scale

   ParticleEditorPlugin.map = %map;

   new ScriptObject( ParticleEditor );

   new PersistenceManager( PE_EmitterSaver );
   new PersistenceManager( PE_ParticleSaver );

   new SimSet( PE_UnlistedParticles );
   new SimSet( PE_UnlistedEmitters );
}

//---------------------------------------------------------------------------------------------

function destroyParticleEditor()
{
}

//=============================================================================================
//    ParticleEditorPlugin.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::onWorldEditorStartup( %this )
{     
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Particle Editor", "", ParticleEditorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Particle Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "ParticleEditorPlugin", "ParticleEditorPalette", expandFilename("tools/worldEditor/images/toolbar/particleeditor"), %tooltip );
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::onActivated( %this )
{
   if( !ParticleEditor.isInitialized )
   {
      ParticleEditor.initEditor();
      ParticleEditor.isInitialized = true;
   }
   
   EditorGui-->WorldEditorToolbar.setVisible( true );
   EditorGui.bringToFront( PE_Window);
   PE_Window.setVisible( true );
   PE_Window.makeFirstResponder( true );
   
   %this.map.push();
      
   ParticleEditor.resetEmitterNode();
   
   // Set the status bar here
   EditorGuiStatusBar.setInfo( "Particle editor." );
   EditorGuiStatusBar.setSelection( "" );
   
   Parent::onActivated( %this );
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::onDeactivated( %this )
{   
   EditorGui-->WorldEditorToolbar.setVisible( false );
   PE_Window.setVisible( false );
   
   if( isObject( $ParticleEditor::emitterNode) )
      $ParticleEditor::emitterNode.delete();
        
   %this.map.pop(); 
   
   Parent::onDeactivated( %this );
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::onExitMission( %this )
{
   // Force Particle Editor to re-initialize.
   ParticleEditor.isInitialized = false;

   Parent::onExitMission( %this );
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup( "ParticleEditor", true );
   
      EditorSettings.setDefaultValue( "selectedTab", 0 );
   
   EditorSettings.endGroup();
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup( "ParticleEditor", true );
   
      %selectedEmitter = EditorSettings.value( "selectedEmitter" );
      if( isObject( %selectedEmitter ) )
         PEE_EmitterSelector.setSelected( %selectedEmitter.getId() );

      %selectedParticle = EditorSettings.value( "selectedParticle" );
      if( isObject( %selectedParticle ) )
         PEP_ParticleSelector.setSelected( %selectedParticle.getId() );
   
      PE_TabBook.selectPage( EditorSettings.value( "selectedPage" ) );

   EditorSettings.endGroup();
}

//---------------------------------------------------------------------------------------------

function ParticleEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "ParticleEditor", true );
   
      EditorSettings.setValue( "selectedEmitter", PEE_EmitterSelector.getText() );
      EditorSettings.setValue( "selectedParticle", PEP_ParticleSelector.getText() );
      EditorSettings.setValue( "selectedTab", PE_TabBook.getSelectedPage() );

   EditorSettings.endGroup();
}
