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


$GUI_EDITOR_DEFAULT_PROFILE_FILENAME = "art/gui/customProfiles.cs";
$GUI_EDITOR_DEFAULT_PROFILE_CATEGORY = "Other";



//=============================================================================================
//    GuiEditor.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditor::createNewProfile( %this, %name, %copySource )
{
   if( %name $= "" )
      return;
      
   // Make sure the object name is unique.
      
   if( isObject( %name ) )
      %name = getUniqueName( %name );

   // Create the profile.
   
   if( %copySource !$= "" )
      eval( "new GuiControlProfile( " @ %name @ " : " @ %copySource.getName() @ " );" );
   else
      eval( "new GuiControlProfile( " @ %name @ " );" );
   
   // Add the item and select it.
      
   %category = %this.getProfileCategory( %name );
   %group = GuiEditorProfilesTree.findChildItemByName( 0, %category );
   
   %id = GuiEditorProfilesTree.insertItem( %group, %name @ " (" @ %name.getId() @ ")", %name.getId(), "" );

   GuiEditorProfilesTree.sort( 0, true, true, false );
   GuiEditorProfilesTree.clearSelection();
   GuiEditorProfilesTree.selectItem( %id );
   
   // Mark it as needing to be saved.
   
   %this.setProfileDirty( %name, true );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::getProfileCategory( %this, %profile )
{
   if( %this.isDefaultProfile( %name ) )
      return "Default";
   else if( %profile.category !$= "" )
      return %profile.category;
   else
      return $GUI_EDITOR_DEFAULT_PROFILE_CATEGORY;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::showDeleteProfileDialog( %this, %profile )
{
   if( %profile $= "" )
      return;
      
   if( %profile.isInUse() )
   {
      MessageBoxOk( "Error",
         "The profile '" @ %profile.getName() @ "' is still used by Gui controls."
      );
      return;
   }

   MessageBoxYesNo( "Delete Profile?",
      "Do you really want to delete '" @ %profile.getName() @ "'?",
      "GuiEditor.deleteProfile( " @ %profile @ " );"
   );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::deleteProfile( %this, %profile )
{
   if( isObject( "GuiEditorProfilesPM" ) )
      new PersistenceManager( GuiEditorProfilesPM );
      
   // Clear dirty state.
   
   %this.setProfileDirty( %profile, false );

   // Remove from tree.
   
   %id = GuiEditorProfilesTree.findItemByValue( %profile.getId() );
   GuiEditorProfilesTree.removeItem( %id );
   
   // Remove from file.
   
   GuiEditorProfilesPM.removeObjectFromFile( %profile );
   
   // Delete profile object.
      
   %profile.delete();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::showSaveProfileDialog( %this, %currentFileName )
{
   getSaveFileName( "TorqueScript Files|*.cs", %this @ ".doSaveProfile", %currentFileName );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::doSaveProfile( %this, %fileName )
{
   %path = makeRelativePath( %fileName, getMainDotCsDir() );
   
   GuiEditorProfileFileName.setText( %path );
   %this.saveProfile( GuiEditorProfilesTree.getSelectedProfile(), %path );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::saveProfile( %this, %profile, %fileName )
{
   if( !isObject( "GuiEditorProfilesPM" ) )
      new PersistenceManager( GuiEditorProfilesPM );
      
   if( !GuiEditorProfilesPM.isDirty( %profile )
       && ( %fileName $= "" || %fileName $= %profile.getFileName() ) )
      return;
      
   // Update the filename, if requested.
      
   if( %fileName !$= "" )
   {
      %profile.setFileName( %fileName );
      GuiEditorProfilesPM.setDirty( %profile, %fileName );
   }
      
   // Save the object.
      
   GuiEditorProfilesPM.saveDirtyObject( %profile );
   
   // Clear its dirty state.
   
   %this.setProfileDirty( %profile, false, true );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::revertProfile( %this, %profile )
{
   // Revert changes.
   
   GuiEditorProfileChangeManager.revertEdits( %profile );
   
   // Clear its dirty state.
   
   %this.setProfileDirty( %profile, false );
   
   // Refresh inspector.
   
   if( GuiEditorProfileInspector.getInspectObject() == %profile )
      GuiEditorProfileInspector.refresh();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::isProfileDirty( %this, %profile )
{
   if( !isObject( "GuiEditorProfilesPM" ) )
      return false;
      
   return GuiEditorProfilesPM.isDirty( %profile );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::setProfileDirty( %this, %profile, %value, %noCheck )
{
   if( !isObject( "GuiEditorProfilesPM" ) )
      new PersistenceManager( GuiEditorProfilesPM );
  
   if( %value )
   {
      if( !GuiEditorProfilesPM.isDirty( %profile ) || %noCheck )
      {
         // If the profile hasn't yet been associated with a file,
         // put it in the default file.
         
         if( %profile.getFileName() $= "" )
            %profile.setFileName( $GUI_EDITOR_DEFAULT_PROFILE_FILENAME );

         // Add the profile to the dirty set.
            
         GuiEditorProfilesPM.setDirty( %profile );

         // Show the item as dirty in the tree.
   
         %id = GuiEditorProfilesTree.findItemByValue( %profile.getId() );
         GuiEditorProfilesTree.editItem( %id, GuiEditorProfilesTree.getItemText( %id ) SPC "*", %profile.getId() );
   
         // Count the number of unsaved profiles.  If this is
         // the first one, indicate in the window title that
         // we have unsaved profiles.
   
         %this.increaseNumDirtyProfiles();
      }
   }
   else
   {
      if( GuiEditorProfilesPM.isDirty( %profile ) || %noCheck )
      {
         // Remove from dirty list.
         
         GuiEditorProfilesPM.removeDirty( %profile );
         
         // Clear the dirty marker in the tree.

         %id = GuiEditorProfilesTree.findItemByValue( %profile.getId() );
         %text = GuiEditorProfilesTree.getItemText( %id );
         GuiEditorProfilesTree.editItem( %id, getSubStr( %text, 0, strlen( %text ) - 2 ), %profile.getId() );

         // Count saved profiles.  If this was the last unsaved profile,
         // remove the unsaved changes indicator from the window title.

         %this.decreaseNumDirtyProfiles();

         // Remove saved edits from the change manager.

         GuiEditorProfileChangeManager.clearEdits( %profile );
      }
   }
}

//---------------------------------------------------------------------------------------------

/// Return true if the given profile name is the default profile for a
/// GuiControl class or if it's the GuiDefaultProfile.
function GuiEditor::isDefaultProfile( %this, %name )
{
   if( %name $= "GuiDefaultProfile" )
      return true;
      
   if( !endsWith( %name, "Profile" ) )
      return false;
      
   %className = getSubStr( %name, 0, strlen( %name ) - 7 ) @ "Ctrl";
   if( !isClass( %className ) )
      return false;
      
   return true;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::increaseNumDirtyProfiles( %this )
{
   %this.numDirtyProfiles ++;
   if( %this.numDirtyProfiles == 1 )
   {
      %tab = GuiEditorTabBook-->profilesPage;
      %tab.setText( %tab.text @ " *" );
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditor::decreaseNumDirtyProfiles( %this )
{
   %this.numDirtyProfiles --;
   if( !%this.numDirtyProfiles )
   {
      %tab = GuiEditorTabBook-->profilesPage;
      %title = %tab.text;
      %title = getSubstr( %title, 0, strlen( %title ) - 2 );
      
      %tab.setText( %title );
   }
}

//=============================================================================================
//    GuiEditorProfilesTree.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::init( %this )
{
   %this.clear();
   
   %defaultGroup = %this.insertItem( 0, "Default", -1 );
   %otherGroup = %this.insertItem( 0, $GUI_EDITOR_DEFAULT_PROFILE_CATEGORY, -1 );
   
   foreach( %obj in GuiDataGroup )
   {
      if( !%obj.isMemberOfClass( "GuiControlProfile" ) )
         continue;
         
      // If it's an Editor profile, skip if showing them is not enabled.
      
      if( %obj.category $= "Editor" && !GuiEditor.showEditorProfiles )
         continue;
         
      // Create a visible name.
         
      %name = %obj.getName();
      if( %name $= "" )
         %name = "<Unnamed>";
      %text = %name @ " (" @ %obj.getId() @ ")";
      
      // Find which group to put the control in.
      
      %isDefaultProfile = GuiEditor.isDefaultProfile( %name );
      if( %isDefaultProfile )
         %group = %defaultGroup;
      else if( %obj.category !$= "" )
      {
         %group = %this.findChildItemByName( 0, %obj.category );
         if( !%group )
            %group = %this.insertItem( 0, %obj.category );
      }
      else
         %group = %otherGroup;
         
      // Insert the item.
         
      %this.insertItem( %group, %text, %obj.getId(), "" );
   }
   
   %this.sort( 0, true, true, false );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::onSelect( %this, %id )
{
   %obj = %this.getItemValue( %id );
   if( %obj == -1 )
      return;
   
   GuiEditorProfileInspector.inspect( %obj );
   
   %fileName = %obj.getFileName();
   if( %fileName $= "" )
      %fileName = $GUI_EDITOR_DEFAULT_PROFILE_FILENAME;
      
   GuiEditorProfileFileName.setText( %fileName );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::onUnselect( %this, %id )
{
   GuiEditorProfileInspector.inspect( 0 );
   GuiEditorProfileFileName.setText( "" );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::onProfileRenamed( %this, %profile, %newName )
{
   %item = %this.findItemByValue( %profile.getId() );
   if( %item == -1 )
      return;

   %newText = %newName @ " (" @ %profile.getId() @ ")";
   if( GuiEditor.isProfileDirty( %profile ) )
      %newText = %newText @ " *";
      
   %this.editItem( %item, %newText, %profile.getId() );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::getSelectedProfile( %this )
{
   return %this.getItemValue( %this.getSelectedItem() );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfilesTree::setSelectedProfile( %this, %profile )
{
   %id = %this.findItemByValue( %profile.getId() );
   %this.selectItem( %id );
}

//=============================================================================================
//    GuiEditorProfileInspector.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onFieldSelected( %this, %fieldName, %fieldTypeStr, %fieldDoc )
{
   GuiEditorProfileFieldInfo.setText( "<font:ArialBold:14>" @ %fieldName @ "<font:ArialItalic:14> (" @ %fieldTypeStr @ ") " NL "<font:Arial:14>" @ %fieldDoc );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onFieldAdded( %this, %object, %fieldName )
{
   GuiEditor.setProfileDirty( %object, true );   
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onFieldRemoved( %this, %object, %fieldName )
{
   GuiEditor.setProfileDirty( %object, true );   
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onFieldRenamed( %this, %object, %oldFieldName, %newFieldName )
{
   GuiEditor.setProfileDirty( %object, true );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   GuiEditor.setProfileDirty( %object, true );
   
   // If it's the name field, make sure to sync up the treeview.
   
   if( %fieldName $= "name" )
      GuiEditorProfilesTree.onProfileRenamed( %object, %newValue );
   
   // Add change record.
   
   GuiEditorProfileChangeManager.registerEdit( %object, %fieldName, %arrayIndex, %oldValue );
   
   // Add undo.

   pushInstantGroup();

   %nameOrClass = %object.getName();
   if ( %nameOrClass $= "" )
      %nameOrClass = %object.getClassname();
      
   %action = new InspectorFieldUndoAction()
   {
      actionName = %nameOrClass @ "." @ %fieldName @ " Change";
      
      objectId = %object.getId();
      fieldName = %fieldName;
      fieldValue = %oldValue;
      arrayIndex = %arrayIndex;
                  
      inspectorGui = %this;
   };
   
   popInstantGroup();         
   %action.addToManager( GuiEditor.getUndoManager() );   
   GuiEditor.updateUndoMenu();
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onInspectorPreFieldModification( %this, %fieldName, %arrayIndex )
{
   pushInstantGroup();
   %undoManager = GuiEditor.getUndoManager();
         
   %object = %this.getInspectObject();
      
   %nameOrClass = %object.getName();
   if( %nameOrClass $= "" )
      %nameOrClass = %object.getClassname();

   %action = new InspectorFieldUndoAction()
   {
      actionName = %nameOrClass @ "." @ %fieldName @ " Change";

      objectId = %object.getId();
      fieldName = %fieldName;
      fieldValue = %object.getFieldValue( %fieldName, %arrayIndex );
      arrayIndex = %arrayIndex;

      inspectorGui = %this;
   };
      
   %this.currentFieldEditAction = %action;
   popInstantGroup();
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onInspectorPostFieldModification( %this )
{
   %action = %this.currentFieldEditAction;
   %object = %action.objectId;
   %fieldName = %action.fieldName;
   %arrayIndex = %action.arrayIndex;
   %oldValue = %action.fieldValue;
   %newValue = %object.getFieldValue( %fieldName, %arrayIndex );

   // If it's the name field, make sure to sync up the treeview.
   
   if( %action.fieldName $= "name" )
      GuiEditorProfilesTree.onProfileRenamed( %object, %newValue );
   
   // Add change record.
   
   GuiEditorProfileChangeManager.registerEdit( %object, %fieldName, %arrayIndex, %oldValue );

   %this.currentFieldEditAction.addToManager( GuiEditor.getUndoManager() );
   %this.currentFieldEditAction = "";
   
   GuiEditor.updateUndoMenu();
   GuiEditor.setProfileDirty( %object, true );
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileInspector::onInspectorDiscardFieldModification( %this )
{
   %this.currentFieldEditAction.undo();
   %this.currentFieldEditAction.delete();
   %this.currentFieldEditAction = "";
}

//=============================================================================================
//    GuiEditorProfileChangeManager.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorProfileChangeManager::registerEdit( %this, %profile, %fieldName, %arrayIndex, %oldValue )
{
   // Early-out if we already have a registered edit on the same field.
   
   foreach( %obj in %this )
   {
      if( %obj.profile != %profile )
         continue;
         
      if( %obj.fieldName $= %fieldName
          && %obj.arrayIndex $= %arrayIndex )
         return;
   }
   
   // Create a new change record.
   
   new ScriptObject()
   {
      parentGroup = %this;
      profile = %profile;
      fieldName = %fieldName;
      arrayIndex = %arrayIndex;
      oldValue = %oldValue;
   };
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileChangeManager::clearEdits( %this, %profile )
{
   for( %i = 0; %i < %this.getCount(); %i ++ )
   {
      %obj = %this.getObject( %i );
      if( %obj.profile != %profile )
         continue;
         
      %obj.delete();
      %i --;
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileChangeManager::revertEdits( %this, %profile )
{
   for( %i = 0; %i < %this.getCount(); %i ++ )
   {
      %obj = %this.getObject( %i );
      if( %obj.profile != %profile )
         continue;

      %profile.setFieldValue( %obj.fieldName, %obj.oldValue, %obj.arrayIndex );
         
      %obj.delete();
      %i --;
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorProfileChangeManager::getEdits( %this, %profile )
{
   %set = new SimSet();
      
   foreach( %obj in %this )
      if( %obj.profile == %profile )
         %set.add( %obj );
   
   return %set;
}
