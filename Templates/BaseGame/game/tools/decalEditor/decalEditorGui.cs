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

function DecalEditorGui::onWake( %this )
{
}

function DecalEditorGui::onSelectInstance( %this, %decalId, %lookupName )
{
   if( DecalEditorGui.selDecalInstanceId == %decalId )
      return;
   // Lets remember the new Id
   DecalEditorGui.selDecalInstanceId = %decalId;
   DecalEditorTreeView.clearSelection();
   
   %name = %decalId SPC %lookupName;
   %item = DecalEditorTreeView.findItemByName( %name );
   DecalEditorTreeView.selectItem( %item );
   DecalEditorGui.syncNodeDetails();
}

function DecalEditorGui::onCreateInstance( %this, %decalId, %lookupName )
{
   // Lets remember the new Id
   DecalEditorGui.selDecalInstanceId = %decalId;
   
   // Add the new instance to the node tree
   DecalEditorTreeView.addNodeTree( %decalId, %lookupName );
   DecalEditorTreeView.clearSelection();
   
   %name = %decalId SPC %lookupName;
   %item = DecalEditorTreeView.findItemByName( %name );
   DecalEditorTreeView.selectItem( %item );
   DecalEditorGui.syncNodeDetails();
}

function DecalEditorGui::onDeleteInstance( %this, %decalId, %lookupName )
{
   if( %decalId == DecalEditorGui.selDecalInstanceId )
      DecalEditorGui.selDecalInstanceId = -1;
   
   %id = DecalEditorTreeView.findItemByName( %decalId SPC %lookupName );
   DecalEditorTreeView.removeItem(%id);
}

function DecalEditorGui::editNodeDetails( %this )
{
   %decalId = DecalEditorGui.selDecalInstanceId;
   if( %decalId == -1 )
      return;
   
   %nodeDetails = DecalEditorDetailContainer-->nodePosition.getText();
   %nodeDetails = %nodeDetails @ " " @ DecalEditorDetailContainer-->nodeTangent.getText();
   %nodeDetails = %nodeDetails @ " " @ DecalEditorDetailContainer-->nodeSize.getText();
   
   if( getWordCount(%nodeDetails) == 7 )
      DecalEditorGui.doEditNodeDetails( %decalId, %nodeDetails, false );
   
}

// Stores the information when the gizmo is first used
function DecalEditorGui::prepGizmoTransform( %this, %decalId, %nodeDetails )
{
   DecalEditorGui.gizmoDetails = %nodeDetails;
}

// Activated in onMouseUp while gizmo is dirty
function DecalEditorGui::completeGizmoTransform( %this, %decalId, %nodeDetails )
{
   DecalEditorGui.doEditNodeDetails( %decalId, %nodeDetails, true );
}

function DecalEditorGui::onSleep( %this )
{
}

function DecalEditorGui::syncNodeDetails( %this )
{
   %decalId = DecalEditorGui.selDecalInstanceId;
   if( %decalId == -1 )
      return;
   
   %lookupName = DecalEditorGui.getDecalLookupName( %decalId );
   DecalEditorGui.updateInstancePreview( %lookupName.material );   
   
   DecalEditorDetailContainer-->instanceId.setText(%decalId @ " " @ %lookupName);
   %transformData = DecalEditorGui.getDecalTransform(%decalId);
   DecalEditorDetailContainer-->nodePosition.setText(getWords(%transformData, 0, 2));
   DecalEditorDetailContainer-->nodeTangent.setText(getWords(%transformData, 3, 5));
   DecalEditorDetailContainer-->nodeSize.setText(getWord(%transformData, 6));
}

function DecalEditorGui::paletteSync( %this, %mode )
{
   %evalShortcut = "ToolsPaletteArray-->" @ %mode @ ".setStateOn(1);";
   eval(%evalShortcut);
}

function DecalDataList::onSelect( %this, %id, %text )
{
   %obj = %this.getItemObject( %id );   
   DecalEditorGui.currentDecalData = %obj;
   
   %itemNum = DecalDataList.getSelectedItem();
   if ( %itemNum == -1 )
      return;
         
   %data = DecalDataList.getItemObject( %itemNum );      
   
   // Update the materialEditorList
   $Tools::materialEditorList = %data.getId();
   
   //Canvas.pushDialog( DecalEditDlg );
   DecalInspector.inspect( %data );
   DecalEditorGui.updateDecalPreview( %data.material );
}

function RetargetDecalButton::onClick( %this )
{  
   %id = DecalDataList.getSelectedItem();
   %datablock = DecalDataList.getItemText(%id );
   
   if( !isObject(%datablock) )
   {
      MessageBoxOK("Error", "A valid Decal Template must be selected.");
      return;
   }

   // This is the first place IODropdown is used. The # in the function passed replaced with the output 
   // of the preset menu.
   
   IODropdown("Retarget Decal Instances", 
               "Retarget DecalInstances from " @ %datablock.getName() @ " over to....",
               "decalDataSet", 
               "DecalEditorGui.retargetDecalDatablock(" @ %datablock.getName() @ ", #);", 
               "");
   DecalEditorGui.rebuildInstanceTree();
}

function NewDecalButton::onClick( %this )
{  
   %name = getUniqueName( "NewDecalData" );
   
   %str = "datablock DecalData( " @ %name @ " ) { Material = \"WarningMaterial\"; };";            
   eval( %str );
   
   DecalPMan.setDirty( %name, $decalDataFile );
   
   if ( strchr(LibraryTabControl.text, "*") $= ""  )
      LibraryTabControl.text = LibraryTabControl.text @ "*";
   
   DecalDataList.doMirror();
   %id = DecalDataList.findItemText( %name );
   DecalDataList.setSelected( %id, true );
      
   Canvas.pushDialog( DecalEditDlg );
   DecalInspector.inspect( %name );
}

function DeleteDecalButton::onClick( %this )
{
   
   if( DecalEditorTabBook.getSelectedPage() == 0 ) // library
   {
      %id = DecalDataList.getSelectedItem();
      %datablock = DecalDataList.getItemText(%id );
   
      MessageBoxYesNoCancel("Delete Decal Datablock?", 
         "Are you sure you want to delete<br><br>" @ %datablock @ "<br><br> Datablock deletion won't take affect until the engine is quit.", 
         "DecalEditorGui.deleteSelectedDecalDatablock();", 
         "", 
         "" ); 
   }
   else // instances
   {
      DecalEditorGui.deleteSelectedDecal();
   }
}

// Intended for gui use. The undo/redo functionality for deletion of datablocks
// will enable itself automatically after using this function.
function DecalEditorGui::deleteSelectedDecalDatablock()
{
   %id = DecalDataList.getSelectedItem();
   %datablock = DecalDataList.getItemText(%id );
   
   DecalEditorGui.deleteDecalDatablock( %datablock );
   
   if( %datablock.getFilename() !$= "" )
   {
      DecalPMan.removeDirty( %datablock );
      DecalPMan.removeObjectFromFile( %datablock );  
   }
   
   DecalDataList.addFilteredItem( %datablock );
}

function DecalEditorTabBook::onTabSelected( %this, %text, %idx )
{
   if( %idx == 0)
   {
      DecalPreviewWindow.text = "Template Properties";
      DecalEditorLibraryProperties.setVisible(true);
      DecalEditorTemplateProperties.setVisible(false);
      RetargetDecalButton.setVisible( true );
      SaveDecalsButton.setVisible( true );
      NewDecalButton.setVisible( true );
      DeleteDecalButton.tabSelected = %idx;
   }
   else
   {
      DecalPreviewWindow.text = "Instance Properties";
      RetargetDecalButton.setVisible( false );
      NewDecalButton.setVisible( false );
      SaveDecalsButton.setVisible( false );
      DeleteDecalButton.tabSelected = %idx;
      DecalEditorLibraryProperties.setVisible(false);
      DecalEditorTemplateProperties.setVisible(true);
   }
}

function DecalEditorTreeView::onDefineIcons()
{
   %icons = "tools/gui/images/treeview/default:" @
            "tools/classIcons/decal:" @
            "tools/classIcons/decalNode:";
            
   DecalEditorTreeView.buildIconTable( %icons );
}

function DecalEditorTreeView::onSelect(%this, %id)
{
   %instanceTag = getWord( DecalEditorTreeView.getItemText(%id), 1 );
   if( !isObject( %instanceTag ) )
      return;   
   
   if( %instanceTag.getClassName() !$= "DecalData" )
      return;
   
   // Grab the id from the tree view
   %decalId = getWord( DecalEditorTreeView.getItemText(%id), 0 );
   
   if( DecalEditorGui.selDecalInstanceId == %decalId )
      return;
      
   // Set the curent decalinstances id
   DecalEditorGui.selDecalInstanceId = %decalId;
   
   DecalEditorGui.selectDecal(%decalId);
   DecalEditorGui.syncNodeDetails(%id);
}

// Creating per node in the instance tree
function DecalEditorTreeView::addNodeTree(%this, %nodeName, %parentName)
{
   // If my template isnt there...put it there
   if ( %this.findItemByName(%parentName) == 0 )
   {
      %rootId = %this.findItemByName("<root>");
      %this.insertItem( %rootId, %parentName, 0, "", 1, 1);
   }
   
   %nodeName = %nodeName SPC %parentName;
   %parentId = %this.findItemByName(%parentName);
   %id = %this.insertItem(%parentId, %nodeName, 0, "", 2);
}

function DecalInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   if( %fieldName $= "Material" )
      DecalEditorGui.updateDecalPreview( %newValue );
      
   // Same work to do as for the regular WorldEditor Inspector.
   Inspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue );   
   
   if (%oldValue != %newValue || %oldValue !$= %newValue)
      %this.setDirty(%object);
}

function DecalInspector::setDirty( %this, %object )
{
   DecalPMan.setDirty( %object );
   
   if ( strchr(LibraryTabControl.text, "*") $= ""  )
      LibraryTabControl.text = LibraryTabControl.text @ "*";
}

function DecalInspector::removeDirty()
{
   if ( strchr(LibraryTabControl.text, "*") !$= ""  )
      LibraryTabControl.text = stripChars(LibraryTabControl.text, "*");
}

function DecalEditorGui::updateDecalPreview( %this, %material )
{
   if( isObject( %material ) )
      DecalPreviewWindow-->decalPreview.setBitmap( MaterialEditorGui.searchForTexture( %material.getId(), %material.diffuseMap[0]) );
   else
      DecalPreviewWindow-->decalPreview.setBitmap("tools/materialEditor/gui/unknownImage");
}

function DecalEditorGui::updateInstancePreview( %this, %material )
{
   if( isObject( %material ) )
      DecalPreviewWindow-->instancePreview.setBitmap( MaterialEditorGui.searchForTexture( %material.getId(), %material.diffuseMap[0]) );      
   else
      DecalPreviewWindow-->instancePreview.setBitmap("tools/materialEditor/gui/unknownImage");   
}

function DecalEditorGui::rebuildInstanceTree( %this )
{
   // Initialize the instance tree when the tab is selected
   DecalEditorTreeView.removeItem(0);
   %rootId = DecalEditorTreeView.insertItem(0, "<root>", 0, "");
   %count = DecalEditorGui.getDecalCount();
   for (%i = 0; %i < %count; %i++)
   {
      %name = DecalEditorGui.getDecalLookupName(%i);
      if( %name $= "invalid" )
         continue;
         
      DecalEditorTreeView.addNodeTree(%i, %name);
   }
}