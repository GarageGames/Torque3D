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

function initializeForestEditor()
{
   echo(" % - Initializing Forest Editor");   
  
   exec( "./forestEditor.cs" );
   exec( "./forestEditorGui.gui" );
   exec( "./forestEditToolbar.ed.gui" );

   exec( "./forestEditorGui.cs" );
   exec( "./tools.cs" );
   
   ForestEditorGui.setVisible( false );   
   ForestEditorPalleteWindow.setVisible( false );
   ForestEditorPropertiesWindow.setVisible( false );
   ForestEditToolbar.setVisible( false );
   
   EditorGui.add( ForestEditorGui );
   EditorGui.add( ForestEditorPalleteWindow );
   EditorGui.add( ForestEditorPropertiesWindow );
   EditorGui.add( ForestEditToolbar );
            
   new ScriptObject( ForestEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = ForestEditorGui;
   };
   
   new SimSet(ForestTools)
   {
      new ForestBrushTool()
      {
         internalName = "BrushTool";
         toolTip = "Paint Tool";
         buttonImage = "tools/forest/images/brushTool";      
      };

      new ForestSelectionTool()
      {
         internalName = "SelectionTool";      
         toolTip = "Selection Tool";
         buttonImage = "tools/forest/images/selectionTool";      
      };
   };      
   
   %map = new ActionMap();
   %map.bindCmd( keyboard, "1", "ForestEditorSelectModeBtn.performClick();", "" ); // Select
   %map.bindCmd( keyboard, "2", "ForestEditorMoveModeBtn.performClick();", "" );   // Move
   %map.bindCmd( keyboard, "3", "ForestEditorRotateModeBtn.performClick();", "" ); // Rotate
   %map.bindCmd( keyboard, "4", "ForestEditorScaleModeBtn.performClick();", "" );  // Scale
   %map.bindCmd( keyboard, "5", "ForestEditorPaintModeBtn.performClick();", "" );  // Paint
   %map.bindCmd( keyboard, "6", "ForestEditorEraseModeBtn.performClick();", "" );  // Erase
   %map.bindCmd( keyboard, "7", "ForestEditorEraseSelectedModeBtn.performClick();", "" );  // EraseSelected   
   //%map.bindCmd( keyboard, "backspace", "ForestEditorGui.onDeleteKey();", "" );
   //%map.bindCmd( keyboard, "delete", "ForestEditorGui.onDeleteKey();", "" );   
   ForestEditorPlugin.map = %map;   
}

function destroyForestEditor()
{
}

// NOTE: debugging helper.
function reinitForest()
{
   exec( "./main.cs" );
   exec( "./forestEditorGui.cs" );   
   exec( "./tools.cs" );
}

function ForestEditorPlugin::onWorldEditorStartup( %this )
{       
   new PersistenceManager( ForestDataManager );
   
   %brushPath = "art/forest/brushes.cs";
   if ( !isFile( %brushPath ) )   
      createPath( %brushPath );      
      
   // This creates the ForestBrushGroup, all brushes, and elements.
   exec( %brushpath );         
   
   if ( !isObject( ForestBrushGroup ) )
   {
      new SimGroup( ForestBrushGroup );
      %this.showError = true;      
   }
      
   ForestEditBrushTree.open( ForestBrushGroup );   
            
   if ( !isObject( ForestItemDataSet ) )
      new SimSet( ForestItemDataSet );
      
   ForestEditMeshTree.open( ForestItemDataSet );

   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Forest Editor", "", ForestEditorPlugin );    
   
   // Add ourselves to the tools menu.
   %tooltip = "Forest Editor (" @ %accel @ ")";  
   EditorGui.addToToolsToolbar( "ForestEditorPlugin", "ForestEditorPalette", expandFilename("tools/forestEditor/images/forest-editor-btn"), %tooltip );   
   
   //connect editor windows
   GuiWindowCtrl::attach( ForestEditorPropertiesWindow, ForestEditorPalleteWindow );
   ForestEditTabBook.selectPage(0);        
}

function ForestEditorPlugin::onWorldEditorShutdown( %this )
{
   if ( isObject( ForestBrushGroup ) )   
      ForestBrushGroup.delete();
   if ( isObject( ForestDataManager ) )
      ForestDataManager.delete();
}

function ForestEditorPlugin::onActivated( %this )
{
   EditorGui.bringToFront( ForestEditorGui );
   ForestEditorGui.setVisible( true );
   ForestEditorPalleteWindow.setVisible( true );
   ForestEditorPropertiesWindow.setVisible( true );
   ForestEditorGui.makeFirstResponder( true );
   //ForestEditToolbar.setVisible( true );
   
   %this.map.push();
   Parent::onActivated(%this);   
   
   ForestEditBrushTree.open( ForestBrushGroup );   
   ForestEditMeshTree.open( ForestItemDataSet );
   
   // Open the Brush tab.
   ForestEditTabBook.selectPage(0);
   
   // Sync the pallete button state
   
   // And toolbar.
   %tool = ForestEditorGui.getActiveTool();      
   if ( isObject( %tool ) )
      %tool.onActivated();
   
   if ( !isObject( %tool ) )
   {
      ForestEditorPaintModeBtn.performClick();
      
      if ( ForestEditBrushTree.getItemCount() > 0 )
      {
         ForestEditBrushTree.selectItem( 0, true );  
      }
   }
   else if ( %tool == ForestTools->SelectionTool )
   {
      %mode = GlobalGizmoProfile.mode;
      switch$ (%mode)
      {
         case "None":
            ForestEditorSelectModeBtn.performClick();
         case "Move":
            ForestEditorMoveModeBtn.performClick();
         case "Rotate":
            ForestEditorRotateModeBtn.performClick();
         case "Scale":
            ForestEditorScaleModeBtn.performClick();
      }
   }
   else if ( %tool == ForestTools->BrushTool )
   {
      %mode = ForestTools->BrushTool.mode;
      switch$ (%mode)
      {
         case "Paint":
            ForestEditorPaintModeBtn.performClick();
         case "Erase":
            ForestEditorEraseModeBtn.performClick();
         case "EraseSelected":
            ForestEditorEraseSelectedModeBtn.performClick();
      }
   }   
   
   if ( %this.showError )
      MessageBoxOK( "Error", "Your art/forest folder does not contain a valid brushes.cs. Brushes you create will not be saved!" );
}

function ForestEditorPlugin::onDeactivated( %this )
{  
   ForestEditorGui.setVisible( false );
   ForestEditorPalleteWindow.setVisible( false );
   ForestEditorPropertiesWindow.setVisible( false );
   
   %tool = ForestEditorGui.getActiveTool();
   if ( isObject( %tool ) )
      %tool.onDeactivated();
   
   // Also take this opportunity to save.
   ForestDataManager.saveDirty();
   
   %this.map.pop();
   
   Parent::onDeactivated(%this);
}

function ForestEditorPlugin::isDirty( %this )
{
   %dirty = %this.dirty || ForestEditorGui.isDirty();
   return %dirty;
}

function ForestEditorPlugin::clearDirty( %this )
{   
   %this.dirty = false;
}

function ForestEditorPlugin::onSaveMission( %this, %missionFile )
{
   ForestDataManager.saveDirty();
   
   if ( isObject( theForest ) )                     
      theForest.saveDataFile();
      
   ForestBrushGroup.save( "art/forest/brushes.cs" );
}

function ForestEditorPlugin::onEditorSleep( %this )
{
}

function ForestEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %hasSelection = false;
   
   %selTool = ForestTools->SelectionTool;
   if ( ForestEditorGui.getActiveTool() == %selTool )
      if ( %selTool.getSelectionCount() > 0 )      
         %hasSelection = true;
      
   %editMenu.enableItem( 3, %hasSelection ); // Cut
   %editMenu.enableItem( 4, %hasSelection ); // Copy
   %editMenu.enableItem( 5, %hasSelection ); // Paste  
   %editMenu.enableItem( 6, %hasSelection ); // Delete
   %editMenu.enableItem( 8, %hasSelection ); // Deselect     
}

function ForestEditorPlugin::handleDelete( %this )
{
   ForestTools->SelectionTool.deleteSelection();   
}

function ForestEditorPlugin::handleDeselect( %this )
{
   ForestTools->SelectionTool.clearSelection();
}

function ForestEditorPlugin::handleCut( %this )
{
   ForestTools->SelectionTool.cutSelection();
}

function ForestEditorPlugin::handleCopy( %this )
{
   ForestTools->SelectionTool.copySelection();
}

function ForestEditorPlugin::handlePaste( %this )
{
   ForestTools->SelectionTool.pasteSelection();
}