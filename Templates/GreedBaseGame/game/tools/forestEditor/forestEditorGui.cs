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


// ForestEditorGui Script Methods

function ForestEditorGui::setActiveTool( %this, %tool )
{
   if ( %tool == ForestTools->BrushTool )
      ForestEditTabBook.selectPage(0);
      
   Parent::setActiveTool( %this, %tool );
}

/// This is called by the editor when the active forest has
/// changed giving us a chance to update the GUI.
function ForestEditorGui::onActiveForestUpdated( %this, %forest, %createNew )
{
   %gotForest = isObject( %forest );

   // Give the user a chance to add a forest.
   if ( !%gotForest && %createNew )
   {
      MessageBoxYesNo(  "Forest", 
                        "There is not a Forest in this mission.  Do you want to add one?",
                        %this @ ".createForest();", "" );
      return;                                                         
   }  
}

/// Called from a message box when a forest is not found.
function ForestEditorGui::createForest( %this )
{
   %forestObject = parseMissionGroupForIds("Forest", "");
 
   if ( isObject( %forestObject ) )
   {
      error( "Cannot create a second 'theForest' Forest!" );
      return;
   }
   
   // Allocate the Forest and make it undoable.
   new Forest( theForest )
   {
      dataFile = "";
      parentGroup = "MissionGroup";
   };
   
   MECreateUndoAction::submit( theForest );

   ForestEditorGui.setActiveForest( theForest );

   //Re-initialize the editor settings so we can start using it immediately.
   %tool = ForestEditorGui.getActiveTool();      
   if ( isObject( %tool ) )
      %tool.onActivated();
	
   if ( %tool == ForestTools->SelectionTool )
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
   
   EWorldEditor.isDirty = true;
}

function ForestEditorGui::newBrush( %this )
{   
   %internalName = getUniqueInternalName( "Brush", ForestBrushGroup, true );
         
   %brush = new ForestBrush()
   {
      internalName = %internalName;
      parentGroup = ForestBrushGroup; 
   };   
   
   MECreateUndoAction::submit( %brush );
   
   ForestEditBrushTree.open( ForestBrushGroup );
   ForestEditBrushTree.buildVisibleTree(true);
   %item = ForestEditBrushTree.findItemByObjectId( %brush );
   ForestEditBrushTree.clearSelection();
   ForestEditBrushTree.addSelection( %item );
   ForestEditBrushTree.scrollVisible( %item );   
   
   ForestEditorPlugin.dirty = true;
}

function ForestEditorGui::newElement( %this )
{
   %sel = ForestEditBrushTree.getSelectedObject();
   
   if ( !isObject( %sel ) )
      %parentGroup = ForestBrushGroup;
   else
   {
      if ( %sel.getClassName() $= "ForestBrushElement" )  
         %parentGroup = %sel.parentGroup;
      else
         %parentGroup = %sel;
   }
      
   %internalName = getUniqueInternalName( "Element", ForestBrushGroup, true );   
   
   %element = new ForestBrushElement()
   {
      internalName = %internalName;
      parentGroup =  %parentGroup;
   };
   
   MECreateUndoAction::submit( %element );
   
   ForestEditBrushTree.clearSelection();      
   ForestEditBrushTree.buildVisibleTree( true );
   %item = ForestEditBrushTree.findItemByObjectId( %element.getId() );
   ForestEditBrushTree.scrollVisible( %item );
   ForestEditBrushTree.addSelection( %item );  
   
   ForestEditorPlugin.dirty = true;
}

function ForestEditorGui::deleteBrushOrElement( %this )
{
   ForestEditBrushTree.deleteSelection();
   ForestEditorPlugin.dirty = true;
}

function ForestEditorGui::newMesh( %this )
{
   %spec = "All Mesh Files|*.dts;*.dae|DTS|*.dts|DAE|*.dae";
   
   %dlg = new OpenFileDialog()
   {
      Filters        = %spec;
      DefaultPath    = $Pref::WorldEditor::LastPath;
      DefaultFile    = "";
      ChangePath     = true;
   };
         
   %ret = %dlg.Execute();
   
   if ( %ret )
   {
      $Pref::WorldEditor::LastPath = filePath( %dlg.FileName );
      %fullPath = makeRelativePath( %dlg.FileName, getMainDotCSDir() );
      %file = fileBase( %fullPath );
   }   
   
   %dlg.delete();
   
   if ( !%ret )
      return;
         
   %name = getUniqueName( %file );
      
   %str = "datablock TSForestItemData( " @ %name @ " ) { shapeFile = \"" @ %fullPath @ "\"; };";            
   eval( %str );
   
   if ( isObject( %name ) )
   {
      ForestEditMeshTree.clearSelection();
      ForestEditMeshTree.buildVisibleTree( true );
      %item = ForestEditMeshTree.findItemByObjectId( %name.getId() );
      ForestEditMeshTree.scrollVisible( %item );
      ForestEditMeshTree.addSelection( %item );
      
      ForestDataManager.setDirty( %name, "art/forest/managedItemData.cs" );  
      
      %element = new ForestBrushElement()
      {
         internalName = %name;
         forestItemData = %name;  
         parentGroup = ForestBrushGroup;
      };                       

      ForestEditBrushTree.clearSelection();      
      ForestEditBrushTree.buildVisibleTree( true );
      %item = ForestEditBrushTree.findItemByObjectId( %element.getId() );
      ForestEditBrushTree.scrollVisible( %item );
      ForestEditBrushTree.addSelection( %item );    
            
      pushInstantGroup();      
      %action = new MECreateUndoAction()
      {
         actionName = "Create TSForestItemData";
      };      
      popInstantGroup();
            
      %action.addObject( %name );
      %action.addObject( %element );            
      %action.addToManager( Editor.getUndoManager() );   
      
      ForestEditorPlugin.dirty = true;   
   }         
}

function ForestEditorGui::deleteMesh( %this )
{
   %obj = ForestEditMeshTree.getSelectedObject();   
   
   // Can't delete itemData's that are in use without
   // crashing at the moment...
      
   if ( isObject( %obj ) )
   {
      MessageBoxOKCancel( "Warning", 
                          "Deleting this mesh will also delete BrushesElements and ForestItems referencing it.", 
                          "ForestEditorGui.okDeleteMesh(" @ %obj @ ");",
                          "" );      
   }   
}

function ForestEditorGui::okDeleteMesh( %this, %mesh )
{
   // Remove mesh from file
   ForestDataManager.removeObjectFromFile( %mesh, "art/forest/managedItemData.cs" );  

   // Submitting undo actions is handled in code.
   %this.deleteMeshSafe( %mesh );   
   
   // Update TreeViews.
   ForestEditBrushTree.buildVisibleTree( true );
   ForestEditMeshTree.buildVisibleTree( true );
   
   ForestEditorPlugin.dirty = true;
}

function ForestEditorGui::validateBrushSize( %this )
{
   %minBrushSize = 1;
   %maxBrushSize = getWord(ETerrainEditor.maxBrushSize, 0);

   %val = $ThisControl.getText();
   if(%val < %minBrushSize)
      $ThisControl.setValue(%minBrushSize);
   else if(%val > %maxBrushSize)
      $ThisControl.setValue(%maxBrushSize);
}



// Child-control Script Methods


function ForestEditMeshTree::onSelect( %this, %obj )
{
   ForestEditorInspector.inspect( %obj );
}

function ForestEditBrushTree::onRemoveSelection( %this, %obj )
{   
   %this.buildVisibleTree( true );
   ForestTools->BrushTool.collectElements();
   
   if ( %this.getSelectedItemsCount() == 1 )
      ForestEditorInspector.inspect( %obj );
   else
      ForestEditorInspector.inspect( "" );
}

function ForestEditBrushTree::onAddSelection( %this, %obj )
{
   %this.buildVisibleTree( true );
   ForestTools->BrushTool.collectElements();
   
   if ( %this.getSelectedItemsCount() == 1 )
      ForestEditorInspector.inspect( %obj );
   else
      ForestEditorInspector.inspect( "" );
}

function ForestEditTabBook::onTabSelected( %this, %text, %idx )
{
   %bbg = ForestEditorPalleteWindow.findObjectByInternalName("BrushButtonGroup");
   %mbg = ForestEditorPalleteWindow.findObjectByInternalName("MeshButtonGroup");
   
   %bbg.setVisible( false );
   %mbg.setVisible( false );
      
   if ( %text $= "Brushes" )
   {   
      %bbg.setVisible( true );
      %obj = ForestEditBrushTree.getSelectedObject();      
      ForestEditorInspector.inspect( %obj );
   }
   else if ( %text $= "Meshes" )
   {
      %mbg.setVisible( true );
      %obj = ForestEditMeshTree.getSelectedObject();
      ForestEditorInspector.inspect( %obj );      
   }
}

function ForestEditBrushTree::onDeleteSelection( %this )
{
   %list = ForestEditBrushTree.getSelectedObjectList();               
   
   MEDeleteUndoAction::submit( %list, true );
   
   ForestEditorPlugin.dirty = true;
} 

function ForestEditBrushTree::onDragDropped( %this )
{
   ForestEditorPlugin.dirty = true;
}

function ForestEditMeshTree::onDragDropped( %this )
{
   ForestEditorPlugin.dirty = true;
}

function ForestEditMeshTree::onDeleteObject( %this, %obj )
{
   // return true - skip delete.
   return true;
}

function ForestEditMeshTree::onDoubleClick( %this )
{
   %obj = %this.getSelectedObject();     
   
   %name = getUniqueInternalName( %obj.getName(), ForestBrushGroup, true );
   
   %element = new ForestBrushElement()
   {
      internalName = %name;
      forestItemData = %obj.getName();  
      parentGroup = ForestBrushGroup;
   };          
   
   //ForestDataManager.setDirty( %element, "art/forest/brushes.cs" );                 

   ForestEditBrushTree.clearSelection();      
   ForestEditBrushTree.buildVisibleTree( true );
   %item = ForestEditBrushTree.findItemByObjectId( %element );
   ForestEditBrushTree.scrollVisible( %item );
   ForestEditBrushTree.addSelection( %item );  
   
   ForestEditorPlugin.dirty = true;
}

function ForestEditBrushTree::handleRenameObject( %this, %name, %obj )
{   
   if ( %name !$= "" )
   {       
      %found = ForestBrushGroup.findObjectByInternalName( %name );
      if ( isObject( %found ) && %found.getId() != %obj.getId() )
      {
         MessageBoxOK( "Error", "Brush or Element with that name already exists.", "" );   
         
         // true as in, we handled it, don't rename the object.      
         return true;   
      }
   }      
   
   // Since we aren't showing any groups whens inspecting a ForestBrushGroup
   // we can't push this event off to the inspector to handle.

   //return GuiTreeViewCtrl::handleRenameObject( %this, %name, %obj );      
   
   
   // The instant group will try to add our
   // UndoAction if we don't disable it.   
   pushInstantGroup();

   %nameOrClass = %obj.getName();
   if ( %nameOrClass $= "" )
      %nameOrClass = %obj.getClassname();

   %action = new InspectorFieldUndoAction()
   {
      actionName = %nameOrClass @ "." @ "internalName" @ " Change";
      
      objectId = %obj.getId();
      fieldName = "internalName";
      fieldValue = %obj.internalName;
      arrayIndex = 0;
                  
      inspectorGui = "";
   };
   
   // Restore the instant group.
   popInstantGroup();
         
   %action.addToManager( Editor.getUndoManager() );
   EWorldEditor.isDirty = true;   
   
   return false;   
}

function ForestEditorInspector::inspect( %this, %obj )
{
   if ( isObject( %obj ) )
      %class = %obj.getClassName();
   
   %this.showObjectName = false;
   %this.showCustomFields = false;
   
   switch$ ( %class )
   {
      case "ForestBrush":   
         %this.groupFilters = "+NOTHING,-Ungrouped";      

      case "ForestBrushElement":
         %this.groupFilters = "+ForestBrushElement,-Ungrouped";

      case "TSForestItemData":   
         %this.groupFilters = "+Media,+Wind";

      default:
         %this.groupFilters = "";
   }
   
   Parent::inspect( %this, %obj );  
}

function ForestEditorInspector::onInspectorFieldModified( %this, %object, %fieldName, %oldValue, %newValue )
{
   // The instant group will try to add our
   // UndoAction if we don't disable it.   
   %instantGroup = $InstantGroup;
   $InstantGroup = 0;

   %nameOrClass = %object.getName();
   if ( %nameOrClass $= "" )
      %nameOrClass = %object.getClassname();

   %action = new InspectorFieldUndoAction()
   {
      actionName = %nameOrClass @ "." @ %fieldName @ " Change";
      
      objectId = %object.getId();
      fieldName = %fieldName;
      fieldValue = %oldValue;
                  
      inspectorGui = %this;
   };
   
   // Restore the instant group.
   $InstantGroup = %instantGroup; 
         
   %action.addToManager( Editor.getUndoManager() );
   
   if ( %object.getClassName() $= "TSForestItemData" )
      ForestDataManager.setDirty( %object );
      
   ForestEditorPlugin.dirty = true;
}

function ForestEditorInspector::onFieldSelected( %this, %fieldName, %fieldTypeStr, %fieldDoc )
{
   //FieldInfoControl.setText( "<font:ArialBold:14>" @ %fieldName @ "<font:ArialItalic:14> (" @ %fieldTypeStr @ ") " NL "<font:Arial:14>" @ %fieldDoc );
}

function ForestBrushSizeSliderCtrlContainer::onWake(%this)
{
   %this-->slider.range = "1" SPC getWord(ETerrainEditor.maxBrushSize, 0);
   %this-->slider.setValue(ForestBrushSizeTextEditContainer-->textEdit.getValue());
}
