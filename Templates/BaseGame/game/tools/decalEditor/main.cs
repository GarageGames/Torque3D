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

function initializeDecalEditor()
{
   echo(" % - Initializing Decal Editor");
   
   $decalDataFile = "art/decals/managedDecalData.cs";
     
   exec( "./decalEditor.cs" );
   exec( "./decalEditorGui.gui" );
   exec( "./decalEditorGui.cs" );
   exec( "./decalEditorActions.cs" );
   
   // Add ourselves to EditorGui, where all the other tools reside
   DecalEditorGui.setVisible( false ); 
   DecalPreviewWindow.setVisible( false );  
   DecalEditorWindow.setVisible( false );
   EditorGui.add( DecalEditorGui );
   EditorGui.add( DecalEditorWindow );
   EditorGui.add( DecalPreviewWindow );
   DecalEditorTabBook.selectPage( 0 );
   
   new ScriptObject( DecalEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = DecalEditorGui;
   };
   
   %map = new ActionMap();      
   %map.bindCmd( keyboard, "5", "EDecalEditorAddDecalBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "1", "EDecalEditorSelectDecalBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "2", "EDecalEditorMoveDecalBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "3", "EDecalEditorRotateDecalBtn.performClick();", "" );  
   %map.bindCmd( keyboard, "4", "EDecalEditorScaleDecalBtn.performClick();", "" );
   
   DecalEditorPlugin.map = %map;
   
   new PersistenceManager( DecalPMan );  
    
}

function destroyDecalEditor()
{
}

// JCF: helper for during development
function reinitDecalEditor()
{
   exec( "./main.cs" );
   exec( "./decalEditor.cs" );
   exec( "./decalEditorGui.cs" );
}

function DecalEditorPlugin::onWorldEditorStartup( %this )
{      
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Decal Editor", "", DecalEditorPlugin );   
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Decal Editor (" @ %accel @ ")";   
   EditorGui.addToToolsToolbar( "DecalEditorPlugin", "DecalEditorPalette", expandFilename("tools/decalEditor/decal-editor"), %tooltip );

   //connect editor windows   
   GuiWindowCtrl::attach( DecalPreviewWindow, DecalEditorWindow );
   
   //set initial palette setting
   %this.paletteSelection = "AddDecalMode";
}

function DecalEditorPlugin::onActivated( %this )
{   
   EditorGui.bringToFront( DecalEditorGui );
   DecalEditorGui.setVisible( true );
   DecalEditorGui.makeFirstResponder( true );
   DecalPreviewWindow.setVisible( true );
   DecalEditorWindow.setVisible( true );
   
   %this.map.push();
   
   //WORKAROUND: due to the gizmo mode being stored on its profile (which may be shared),
   //  we may end up with a mismatch between the editor mode and gizmo mode here.
   //  Reset mode explicitly here to work around this.
   DecalEditorGui.setMode( DecalEditorGui.getMode() );
   
   // Set the current palette selection
   DecalEditorGui.paletteSync( %this.paletteSelection );
   
   // Store this on a dynamic field
   // in order to restore whatever setting
   // the user had before.
   %this.prevGizmoAlignment = GlobalGizmoProfile.alignment;
   
   // The DecalEditor always uses Object alignment.
   GlobalGizmoProfile.alignment = "Object";
   
   DecalEditorGui.rebuildInstanceTree();
   
   // These could perhaps be the node details like the shape editor
   //ShapeEdPropWindow.syncNodeDetails(-1);
   
   Parent::onActivated(%this);
}

function DecalEditorPlugin::onDeactivated( %this )
{   
   DecalEditorGui.setVisible(false);
   DecalPreviewWindow.setVisible( false );
   DecalEditorWindow.setVisible( false );
   
   %this.map.pop();
   
   // Remember last palette selection
   %this.paletteSelection = DecalEditorGui.getMode();
   
   // Restore the previous Gizmo
   // alignment settings.
   GlobalGizmoProfile.alignment = %this.prevGizmoAlignment; 
   
   Parent::onDeactivated(%this);  
}

function DecalEditorPlugin::isDirty( %this )
{
   %dirty = DecalPMan.hasDirty();
   
   %dirty |= decalManagerDirty();
      
   return %dirty;
}

function DecalEditorPlugin::onSaveMission( %this, %file )
{   
   DecalPMan.saveDirty();
   decalManagerSave( %file @ ".decals" );
}

function DecalEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %hasSelection = false;
   
   if ( DecalEditorGui.getSelectionCount() > 0 )
      %hasSelection = true;
            
   %editMenu.enableItem( 3, false ); // Cut
   %editMenu.enableItem( 4, false ); // Copy
   %editMenu.enableItem( 5, false ); // Paste  
   %editMenu.enableItem( 6, %hasSelection ); // Delete
   %editMenu.enableItem( 8, false ); // Deselect     
   
   // NOTE: If you want to implement Cut, Copy, Paste, or Deselect
   // for this editor simply enable the menu items when it is appropriate
   // and fill in the method stubs below.
}

function DecalEditorPlugin::handleDelete( %this )
{
   DecalEditorGui.deleteSelectedDecal();
}

function DecalEditorPlugin::handleDeselect( %this )
{   
}

function DecalEditorPlugin::handleCut( %this )
{
}

function DecalEditorPlugin::handleCopy( %this )
{
}

function DecalEditorPlugin::handlePaste( %this )
{
}

function DecalEditorPlugin::handleEscape( %this )
{
}

