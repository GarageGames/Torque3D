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

function DecalEditorGui::createAction(%this, %class, %desc)
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = %class;
      superClass = BaseDecalEdAction;
      actionName = %desc;
      tree = DecalEditorTreeView;
   };
   popInstantGroup();
   return %action;
}

function DecalEditorGui::doAction(%this, %action)
{
   if (%action.doit())
      %action.addToManager(Editor.getUndoManager());
}

function BaseDecalEdAction::redo(%this)
{
   // Default redo action is the same as the doit action
   %this.doit();
}

function BaseDecalEdAction::undo(%this)
{
}

//------------------------------------------------------------------------------
// Edit node
function DecalEditorGui::doEditNodeDetails(%this, %instanceId, %transformData, %gizmo)
{
   %action = %this.createAction(ActionEditNodeDetails, "Edit Decal Transform");
   %action.instanceId = %instanceId;
   %action.newTransformData = %transformData;
   
   if( %gizmo )
      %action.oldTransformData = %this.gizmoDetails;
   else
      %action.oldTransformData = %this.getDecalTransform(%instanceId);
   
   %this.doAction(%action);
}

function ActionEditNodeDetails::doit(%this)
{
   %count = getWordCount(%this.newTransformData);
   if(%this.instanceId !$= "" && %count == 7)
   {
      DecalEditorGui.editDecalDetails( %this.instanceId, %this.newTransformData );
      DecalEditorGui.syncNodeDetails();
      DecalEditorGui.selectDecal( %this.instanceId );
      return true;
   }
   return false;
}

function ActionEditNodeDetails::undo(%this)
{
   %count = getWordCount(%this.oldTransformData);
   if(%this.instanceId !$= "" && %count == 7)
   {
      DecalEditorGui.editDecalDetails( %this.instanceId, %this.oldTransformData );
      DecalEditorGui.syncNodeDetails();
      DecalEditorGui.selectDecal( %this.instanceId );
   }
}

//------------------------------------------------------------------------------
// Delete Decal Datablocks

// This functionality solely depends on the undo/redo datablock callbacks in 
// source.

function DecalEditorGui::redoDeleteDecalDatablock( %this, %datablock )
{
   // Remove the object from file and place a filter
   if( %datablock.getFilename() !$= "" )
   {
      DecalPMan.removeDirty( %datablock );
      DecalPMan.removeObjectFromFile( %datablock );  
   }
   
   DecalDataList.addFilteredItem( %datablock );
}

function DecalEditorGui::undoDeleteDecalDatablock( %this, %datablock )
{
   // Replace the object in file and remove the filter
   %filename = %datablock.getFilename();
   if( %datablock.getFilename() !$= "" )
   {
      DecalPMan.setDirty( %datablock, %filename );
      DecalPMan.saveDirty();
   }
   
   DecalDataList.removeFilteredItem( %datablock );
}
