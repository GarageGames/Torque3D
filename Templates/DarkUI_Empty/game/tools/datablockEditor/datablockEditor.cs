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

// Main code for the Datablock Editor plugin.


$DATABLOCK_EDITOR_DEFAULT_FILENAME = "art/datablocks/managedDatablocks.cs";

//=============================================================================================
//    Initialization.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::init( %this )
{
   if( !DatablockEditorTree.getItemCount() )
      %this.populateTrees();
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::onWorldEditorStartup( %this )
{
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Datablock Editor", "", DatablockEditorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Datablock Editor (" @ %accel @ ")"; 
   EditorGui.addToToolsToolbar( "DatablockEditorPlugin", "DatablockEditorPalette", expandFilename("tools/worldEditor/images/toolbar/datablock-editor"), %tooltip );

   //connect editor windows
   GuiWindowCtrl::Attach( DatablockEditorInspectorWindow, DatablockEditorTreeWindow);
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::onActivated( %this )
{
   EditorGui-->WorldEditorToolbar.setVisible(false);
   EditorGui.bringToFront( DatablockEditorPlugin );
   
   DatablockEditorTreeWindow.setVisible( true );
   DatablockEditorInspectorWindow.setVisible( true );
   DatablockEditorInspectorWindow.makeFirstResponder( true );
   
   %this.map.push();

   // Set the status bar here until all tool have been hooked up
   EditorGuiStatusBar.setInfo( "Datablock editor." );
   
   %numSelected = %this.getNumSelectedDatablocks();
   if( !%numSelected )
      EditorGuiStatusBar.setSelection( "" );
   else
      EditorGuiStatusBar.setSelection( %numSelected @ " datablocks selected" );
      
   %this.init();
   DatablockEditorPlugin.readSettings();
   
   if( EWorldEditor.getSelectionSize() == 1 )
      %this.onObjectSelected( EWorldEditor.getSelectedObject( 0 ) );

   Parent::onActivated( %this );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::onDeactivated( %this )
{
   DatablockEditorPlugin.writeSettings();
   
   DatablockEditorInspectorWindow.setVisible( false );
   DatablockEditorTreeWindow.setVisible( false );
   %this.map.pop();
   
   Parent::onDeactivated(%this);
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::onExitMission( %this )
{
   DatablockEditorTree.clear();
   DatablockEditorInspector.inspect( "" );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::openDatablock( %this, %datablock )
{
   EditorGui.setEditor( DatablockEditorPlugin );
   %this.selectDatablock( %datablock );
   DatablockEditorTreeTabBook.selectedPage = 0;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::setEditorFunction( %this )
{
   return true;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::onObjectSelected( %this, %object )
{
   // Select datablock of object if this is a GameBase object.
   
   if( %object.isMemberOfClass( "GameBase" ) )
      %this.selectDatablock( %object.getDatablock() );
   else if( %object.isMemberOfClass( "SFXEmitter" ) && isObject( %object.track ) )
      %this.selectDatablock( %object.track );
   else if( %object.isMemberOfClass( "LightBase" ) && isObject( %object.animationType ) )
      %this.selectDatablock( %object.animationType );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::populateTrees(%this)
{
   // Populate datablock tree.
      
   if( %this.excludeClientOnlyDatablocks )
      %set = DataBlockGroup;
   else
      %set = DataBlockSet;

   DatablockEditorTree.clear();
   
   foreach( %datablock in %set )
   {
      %unlistedFound = false;
      %id = %datablock.getId();
      
      foreach( %obj in UnlistedDatablocks )
         if( %obj.getId() == %id )
         {
            %unlistedFound = true;
            break;
         }
   
      if( %unlistedFound )
         continue;
         
      %this.addExistingItem( %datablock, true );
   }
   
   DatablockEditorTree.sort( 0, true, false, false );
   
   // Populate datablock type tree.
   
   %classList = enumerateConsoleClasses( "SimDatablock" );
   DatablockEditorTypeTree.clear();
   
   foreach$( %datablockClass in %classList )
   {
      if(    !%this.isExcludedDatablockType( %datablockClass )
          && DatablockEditorTypeTree.findItemByName( %datablockClass ) == 0 )
         DatablockEditorTypeTree.insertItem( 0, %datablockClass );
   }
   
   DatablockEditorTypeTree.sort( 0, false, false, false );   
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::addExistingItem( %this, %datablock, %dontSort )
{
   %tree = DatablockEditorTree;
   
   // Look up class at root level.  Create if needed.
   
   %class = %datablock.getClassName();
   %parentID = %tree.findItemByName( %class );
   if( %parentID == 0 )
      %parentID = %tree.insertItem( 0, %class );

   // If the datablock is already there, don't
   // do anything.
   
   if( %tree.findItemByValue( %datablock.getId() ) )
      return;
      
   // It doesn't exist so add it.

   %name = %datablock.getName();
   if( %this.PM.isDirty( %datablock ) )
      %name = %name @ " *";
         
   %id = DatablockEditorTree.insertItem( %parentID, %name, %datablock.getId() );
   if( !%dontSort )
      DatablockEditorTree.sort( %parentID, false, false, false );
         
   return %id;   
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::isExcludedDatablockType( %this, %className )
{
   switch$( %className )
   {
      case "SimDatablock":
         return true;
      case "SFXTrack": // Abstract.
         return true;  
      case "SFXFMODEvent": // Internally created.
         return true;
      case "SFXFMODEventGroup": // Internally created.
         return true;
   }
   return false;
}

//=============================================================================================
//    Settings.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::initSettings( %this )
{
   EditorSettings.beginGroup("DatablockEditor", true);
   
      EditorSettings.setDefaultValue("libraryTab", "0");
   
   EditorSettings.endGroup();
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::readSettings( %this )
{
   EditorSettings.beginGroup("DatablockEditor", true);
   
      DatablockEditorTreeTabBook.selectPage( EditorSettings.value( "libraryTab" ) );
      %db = EditorSettings.value( "selectedDatablock" );
      if( isObject( %db ) && %db.isMemberOfClass( "SimDatablock" ) )
         %this.selectDatablock( %db );
   
   EditorSettings.endGroup();  
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::writeSettings( %this )
{
   EditorSettings.beginGroup( "DatablockEditor", true );
   
      EditorSettings.setValue( "libraryTab", DatablockEditorTreeTabBook.getSelectedPage() );
      if( %this.getNumSelectedDatablocks() > 0 )
         EditorSettings.setValue( "selectedDatablock", %this.getSelectedDatablock().getName() );

   EditorSettings.endGroup();
}

//=============================================================================================
//    Persistence.
//=============================================================================================

//---------------------------------------------------------------------------------------------

/// Return true if there is any datablock with unsaved changes.
function DatablockEditorPlugin::isDirty( %this )
{
   return %this.PM.hasDirty();
}

//---------------------------------------------------------------------------------------------

/// Return true if any of the currently selected datablocks has unsaved changes.
function DatablockEditorPlugin::selectedDatablockIsDirty( %this )
{
   %tree = DatablockEditorTree;
   
   %count = %tree.getSelectedItemsCount();
   %selected = %tree.getSelectedItemList();
   
   foreach$( %id in %selected )
   {
      %db = %tree.getItemValue( %id );
      if( %this.PM.isDirty( %db ) )
         return true;
   }
   
   return false;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::syncDirtyState( %this )
{
   %tree = DatablockEditorTree;

   %count = %tree.getSelectedItemsCount();
   %selected = %tree.getSelectedItemList();
   %haveDirty = false;

   foreach$( %id in %selected )
   {
      %db = %tree.getItemValue( %id );
      if( %this.PM.isDirty( %db ) )
      {
         %this.flagDatablockAsDirty( %db, true );
         %haveDirty = true;
      }
      else
         %this.flagInspectorAsDirty( %db, false );
   }

   %this.flagInspectorAsDirty( %haveDirty );
}

//---------------------------------------------------------------------------------------------

///
function DatablockEditorPlugin::flagInspectorAsDirty( %this, %dirty )
{
   if( %dirty )
      DatablockEditorInspectorWindow.text = "Datablock *";
   else
      DatablockEditorInspectorWindow.text = "Datablock";
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::flagDatablockAsDirty(%this, %datablock, %dirty )
{
   %tree = DatablockEditorTree;
   
   %id = %tree.findItemByValue( %datablock.getId() );
   if( %id == 0 )
      return;

   // Tag the item caption and sync the persistence manager.
      
   if( %dirty )
   {
      DatablockEditorTree.editItem( %id, %datablock.getName() @ " *", %datablock.getId() );
      %this.PM.setDirty( %datablock );
   }
   else
   {
      DatablockEditorTree.editItem( %id, %datablock.getName(), %datablock.getId() );
      %this.PM.removeDirty( %datablock );
   }
   
   // Sync the inspector dirty state.
   
   %this.flagInspectorAsDirty( %this.PM.hasDirty() );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::showSaveNewFileDialog(%this)
{
   %currentFile = %this.getSelectedDatablock().getFilename();
   getSaveFilename( "TorqueScript Files|*.cs|All Files|*.*", %this @ ".saveNewFileFinish", %currentFile, false );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::saveNewFileFinish( %this, %newFileName )
{
   // Clear the first responder to capture any inspector changes
   %ctrl = canvas.getFirstResponder();
   if( isObject(%ctrl) )
      %ctrl.clearFirstResponder();

   %tree = DatablockEditorTree;
   %count = %tree.getSelectedItemsCount();
   %selected = %tree.getSelectedItemList();

   foreach$( %id in %selected )
   {
      %db = %tree.getItemValue( %id );
      %db = %this.getSelectedDatablock();
   
      // Remove from current file.
   
      %oldFileName = %db.getFileName();
      if( %oldFileName !$= "" )
         %this.PM.removeObjectFromFile( %db, %oldFileName );
   
      // Save to new file.

      %this.PM.setDirty( %db, %newFileName );
      if( %this.PM.saveDirtyObject( %db ) )
      {
         // Clear dirty state.
   
         %this.flagDatablockAsDirty( %db, false );
      }
   }
   
   DatablockEditorInspectorWindow-->DatablockFile.setText( %newFileName );   
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::save( %this )
{
   // Clear the first responder to capture any inspector changes
   %ctrl = canvas.getFirstResponder();
   if( isObject(%ctrl) )
      %ctrl.clearFirstResponder();

   %tree = DatablockEditorTree;
   %count = %tree.getSelectedItemsCount();
   %selected = %tree.getSelectedItemList();
   
   for( %i = 0; %i < %count; %i ++ )
   {
      %id = getWord( %selected, %i );
      %db = %tree.getItemValue( %id );
      
      if( %this.PM.isDirty( %db ) )
      {
         %this.PM.saveDirtyObject( %db );
         %this.flagDatablockAsDirty( %db, false );
      }
   }
}

//=============================================================================================
//    Selection.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::getNumSelectedDatablocks( %this )
{
   return DatablockEditorTree.getSelectedItemsCount();
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::getSelectedDatablock( %this, %index )
{
   %tree = DatablockEditorTree;
   if( !%tree.getSelectedItemsCount() )
      return 0;
      
   if( !%index )
      %id = %tree.getSelectedItem();
   else
      %id = getWord( %tree.getSelectedItemList(), %index );
      
   return %tree.getItemValue( %id );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::resetSelectedDatablock( %this )
{
   DatablockEditorTree.clearSelection();
   DatablockEditorInspector.inspect(0);   
   DatablockEditorInspectorWindow-->DatablockFile.setText("");     

   EditorGuiStatusBar.setSelection( "" );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::selectDatablockCheck( %this, %datablock )
{
   if( %this.selectedDatablockIsDirty() )
      %this.showSaveDialog( %datablock );
   else
      %this.selectDatablock( %datablock );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::selectDatablock( %this, %datablock, %add, %dontSyncTree )
{
   if( %add )
      DatablockEditorInspector.addInspect( %datablock );
   else
      DatablockEditorInspector.inspect( %datablock );
   
   if( !%dontSyncTree )
   {
      %id = DatablockEditorTree.findItemByValue( %datablock.getId() );
   
      if( !%add )
         DatablockEditorTree.clearSelection();
      
      DatablockEditorTree.selectItem( %id, true );
      DatablockEditorTree.scrollVisible( %id );
   }
   
   %this.syncDirtyState();
                  
   // Update the filename text field.
   
   %numSelected = %this.getNumSelectedDatablocks();
   %fileNameField = DatablockEditorInspectorWindow-->DatablockFile;
   
   if( %numSelected == 1 )
   {
      %fileName = %datablock.getFilename();
      if( %fileName !$= "" )
         %fileNameField.setText( %fileName );
      else
         %fileNameField.setText( $DATABLOCK_EDITOR_DEFAULT_FILENAME );
   }
   else
   {
      %fileNameField.setText( "" );
   }

   EditorGuiStatusBar.setSelection( %this.getNumSelectedDatablocks() @ " Datablocks Selected" );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::unselectDatablock( %this, %datablock, %dontSyncTree )
{
   DatablockEditorInspector.removeInspect( %datablock );
   
   if( !%dontSyncTree )
   {
      %id = DatablockEditorTree.findItemByValue( %datablock.getId() );
      DatablockEditorTree.selectItem( %id, false );
   }
   
   %this.syncDirtyState();

   // If we have exactly one selected datablock remaining, re-enable
   // the save-as button.
   
   %numSelected = %this.getNumSelectedDatablocks();
   if( %numSelected == 1 )
   {
      DatablockEditorInspectorWindow-->saveAsButton.setActive( true );

      %fileNameField = DatablockEditorInspectorWindow-->DatablockFile;
      %fileNameField.setText( %this.getSelectedDatablock().getFilename() );
      %fileNameField.setActive( true );
   }

   EditorGuiStatusBar.setSelection( %this.getNumSelectedDatablocks() @ " Datablocks Selected" );
}

//=============================================================================================
//    Creation and Deletion.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::deleteDatablock( %this )
{
   %tree = DatablockEditorTree;
   
   // If we have more than single datablock selected,
   // turn our undos into a compound undo.
   
   %numSelected = %tree.getSelectedItemsCount();
   if( %numSelected > 1 )
      Editor.getUndoManager().pushCompound( "Delete Multiple Datablocks" );
      
   for( %i = 0; %i < %numSelected; %i ++ )
   {
      %id = %tree.getSelectedItem( %i );
      %db = %tree.getItemValue( %id );
      
      %fileName = %db.getFileName();
      
      // Remove the datablock from the tree.
      
      DatablockEditorTree.removeItem( %id );
      
      // Create undo.

      %action = %this.createUndo( ActionDeleteDatablock, "Delete Datablock" ); 
      %action.db = %db;
      %action.dbName = %db.getName();
      %action.fname = %fileName;
      
      %this.submitUndo( %action );
      
      // Kill the datablock in the file.

      if( %fileName !$= "" )
         %this.PM.removeObjectFromFile( %db );

      UnlistedDatablocks.add( %db );

      // Show some confirmation.

      if( %numSelected == 1 )
         MessageBoxOk( "Datablock Deleted", "The datablock (" @ %db.getName() @ ") has been removed from " @
                       "it's file (" @ %db.getFilename() @ ") and upon restart will cease to exist" );
   }
   
   // Close compound, if we were deleting multiple datablocks.
   
   if( %numSelected > 1 )
      Editor.getUndoManager().popCompound();
   
   // Show confirmation for multiple datablocks.
   
   if( %numSelected > 1 )
      MessageBoxOk( "Datablocks Deleted", "The datablocks have been deleted and upon restart will cease to exist." );

   // Clear selection.
      
   DatablockEditorPlugin.resetSelectedDatablock();
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::createDatablock(%this)
{
   %class = DatablockEditorTypeTree.getItemText(DatablockEditorTypeTree.getSelectedItem());
   if( %class !$= "" )
   {  
      // Need to prompt for a name.
      
      DatablockEditorCreatePrompt-->CreateDatablockName.setText("Name");
      DatablockEditorCreatePrompt-->CreateDatablockName.selectAllText();
      
      // Populate the copy source dropdown.
      
      %list = DatablockEditorCreatePrompt-->CopySourceDropdown;
      %list.clear();
      %list.add( "", 0 );
      
      %set = DataBlockSet;
      %count = %set.getCount();
      for( %i = 0; %i < %count; %i ++ )
      {
         %datablock = %set.getObject( %i );
         %datablockClass = %datablock.getClassName();
         
         if( !isMemberOfClass( %datablockClass, %class ) )
            continue;
            
         %list.add( %datablock.getName(), %i + 1 );
      }
      
      // Set up state of client-side checkbox.
      
      %clientSideCheckBox = DatablockEditorCreatePrompt-->ClientSideCheckBox;
      %canBeClientSide = DatablockEditorPlugin::canBeClientSideDatablock( %class );
      %clientSideCheckBox.setStateOn( %canBeClientSide );
      %clientSideCheckBox.setActive( %canBeClientSide );
      
      // Show the dialog.
      
      canvas.pushDialog( DatablockEditorCreatePrompt, 0, true );
   }
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::createPromptNameCheck(%this)
{
   %name = DatablockEditorCreatePrompt-->CreateDatablockName.getText();
   if( !Editor::validateObjectName( %name, true ) )
      return;
      
   // Fetch the copy source and clear the list.
   
   %copySource = DatablockEditorCreatePrompt-->copySourceDropdown.getText();
   DatablockEditorCreatePrompt-->copySourceDropdown.clear();
   
   // Remove the dialog and create the datablock.
   
   canvas.popDialog( DatablockEditorCreatePrompt );
   %this.createDatablockFinish( %name, %copySource );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::createDatablockFinish( %this, %name, %copySource )
{
   %class = DatablockEditorTypeTree.getItemText(DatablockEditorTypeTree.getSelectedItem());
   if( %class !$= "" )
   {  
      %action = %this.createUndo( ActionCreateDatablock, "Create New Datablock" );
      
      if( DatablockEditorCreatePrompt-->ClientSideCheckBox.isStateOn() )
         %dbType = "singleton ";
      else
         %dbType = "datablock ";
      
      if( %copySource !$= "" )
         %eval = %dbType @ %class @ "(" @ %name @ " : " @ %copySource @ ") { canSaveDynamicFields = \"1\"; };";
      else
         %eval = %dbType @ %class @ "(" @ %name @ ") { canSaveDynamicFields = \"1\"; };";
         
      %res = eval( %eval );
      
      %action.db = %name.getId();
      %action.dbName = %name;
      %action.fname = $DATABLOCK_EDITOR_DEFAULT_FILENAME;
      
      %this.submitUndo( %action );
      
      %action.redo();
   }
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::canBeClientSideDatablock( %className )
{
   switch$( %className )
   {
      case "SFXProfile" or
           "SFXPlayList" or
           "SFXAmbience" or
           "SFXEnvironment" or
           "SFXState" or
           "SFXDescription" or
           "SFXFMODProject":
         return true;
         
      default:
         return false;
   }
}

//=============================================================================================
//    Events.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function DatablockEditorInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   // Same work to do as for the regular WorldEditor Inspector.
   Inspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue );   
   
   DatablockEditorPlugin.flagDatablockAsDirty( %object, true );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorInspector::onFieldSelected( %this, %fieldName, %fieldTypeStr, %fieldDoc )
{
   DatablockFieldInfoControl.setText( "<font:ArialBold:14>" @ %fieldName @ "<font:ArialItalic:14> (" @ %fieldTypeStr @ ") " NL "<font:Arial:14>" @ %fieldDoc );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorInspector::onBeginCompoundEdit( %this )
{
   Editor.getUndoManager().pushCompound( "Multiple Field Edit" );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorInspector::onEndCompoundEdit( %this, %discard )
{
   Editor.getUndoManager().popCompound( %discard );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorInspector::onClear( %this )
{
   DatablockFieldInfoControl.setText( "" );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onDeleteSelection( %this )
{
   %this.undoDeleteList = "";   
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onDeleteObject( %this, %object )
{
   // Append it to our list.
   %this.undoDeleteList = %this.undoDeleteList TAB %object;
              
   // We're gonna delete this ourselves in the
   // completion callback.
   return true;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onObjectDeleteCompleted( %this )
{
   //MEDeleteUndoAction::submit( %this.undoDeleteList );
   
   // Let the world editor know to 
   // clear its selection.
   //EWorldEditor.clearSelection();
   //EWorldEditor.isDirty = true;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onClearSelected(%this)
{
   DatablockEditorInspector.inspect( 0 );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onAddSelection( %this, %id )
{
   %obj = %this.getItemValue( %id );
   
   if( !isObject( %obj ) )
      %this.selectItem( %id, false );
   else
      DatablockEditorPlugin.selectDatablock( %obj, true, true );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onRemoveSelection( %this, %id )
{
   %obj = %this.getItemValue( %id );
   if( isObject( %obj ) )
      DatablockEditorPlugin.unselectDatablock( %obj, true );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTree::onRightMouseUp( %this, %id, %mousePos )
{
   %datablock = %this.getItemValue( %id );
   if( !isObject( %datablock ) )
      return;
      
   if( !isObject( DatablockEditorTreePopup ) )
      new PopupMenu( DatablockEditorTreePopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Delete" TAB "" TAB "DatablockEditorPlugin.selectDatablock( %this.datablockObject ); DatablockEditorPlugin.deleteDatablock( %this.datablockObject );";
         item[ 1 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenDeclarationInTorsion( %this.datablockObject );";
         
         datablockObject = "";
      };
      
   DatablockEditorTreePopup.datablockObject = %datablock;
   DatablockEditorTreePopup.showPopup( Canvas );
}

//---------------------------------------------------------------------------------------------

function DatablockEditorTreeTabBook::onTabSelected(%this, %text, %id)
{
   switch(%id)
   {
      case 0:
         DatablockEditorTreeWindow-->DeleteSelection.visible = true;
         DatablockEditorTreeWindow-->CreateSelection.visible = false;
      
      case 1:
         DatablockEditorTreeWindow-->DeleteSelection.visible = false;
         DatablockEditorTreeWindow-->CreateSelection.visible = true;               
   }
}
