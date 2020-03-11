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


//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::createUndo( %this, %class, %desc )
{
   pushInstantGroup();
   %action = new UndoScriptAction()
   {
      class = %class;
      superClass = BaseDatablockEdAction;
      actionName = %desc;
      editor = DatablockEditorPlugin;
      treeview = DatablockEditorTree;
      inspector = DatablockEditorInspector;
   };
   popInstantGroup();
   return %action;
}

//---------------------------------------------------------------------------------------------

function DatablockEditorPlugin::submitUndo( %this, %action )
{
   %action.addToManager( Editor.getUndoManager() );
}

//=============================================================================================
//    BaseDatablockEdAction.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function BaseDatablockEdAction::redo( %this )
{
}

//---------------------------------------------------------------------------------------------

function BaseDatablockEdAction::undo( %this )
{
}

//=============================================================================================
//    ActionCreateDatablock.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionCreateDatablock::redo( %this )
{
   %db = %this.db;
   
   %db.name = %this.dbName;
   
   %this.editor.PM.setDirty( %db, %this.fname );
   %this.editor.addExistingItem( %db );
   %this.editor.selectDatablock( %db );
   %this.editor.flagInspectorAsDirty( true );
   
   UnlistedDatablocks.remove( %id );
}

//---------------------------------------------------------------------------------------------

function ActionCreateDatablock::undo( %this )
{
   %db = %this.db;
      
   %itemId = %this.treeview.findItemByName( %db.name );
   if( !%itemId )
      %itemId = %this.treeview.findItemByName( %db.name @ " *" );
   
   %this.treeview.removeItem( %itemId );
   %this.editor.resetSelectedDatablock();
   %this.editor.PM.removeDirty( %db );

   %this.dbName = %db.name;
   %db.name = "";
   
   UnlistedDatablocks.add( %this.db );
}

//=============================================================================================
//    ActionDeleteDatablock.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function ActionDeleteDatablock::redo( %this )
{
   %db = %this.db;
      
   %itemId = %this.treeview.findItemByName( %db.name );
   if( !%itemId )
      %itemId = %this.treeview.findItemByName( %db.name @ " *" );
      
   // Remove from tree and file.
      
   %this.treeview.removeItem( %db );
   %this.editor.resetSelectedDatablock();
   if( %db.getFileName() !$= "" )
      %this.editor.PM.removeObjectFromFile( %db );
   
   // Unassign name.

   %this.dbName = %db.name;
   %db.name = "";
   
   // Add to unlisted.
   
   UnlistedDatablocks.add( %db );
}

//---------------------------------------------------------------------------------------------

function ActionDeleteDatablock::undo( %this )
{
   %db = %this.db;
   
   // Restore name.
   
   %db.name = %this.dbName;
   
   // Add to tree and select.
   
   %this.editor.addExistingItem( %db, true );
   %this.editor.selectDatablock( %db );
   
   // Mark as dirty.
   
   %this.editor.PM.setDirty( %db, %this.fname );
   %this.editor.syncDirtyState();
   
   // Remove from unlisted.
   
   UnlistedDatablocks.remove( %id );
}
