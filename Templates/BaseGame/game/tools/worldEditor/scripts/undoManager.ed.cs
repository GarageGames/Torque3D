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

function EUndoManager::onUndo( %this )
{
}

function EUndoManager::onRedo( %this )
{
}

function EUndoManager::onAddUndo( %this )
{
}

function EUndoManager::onRemoveUndo( %this )
{
}

function EUndoManager::onClear( %this )
{
}

function EUndoManager::updateUndoMenu( %this, %editMenu )
{
   // TODO: If we ever fix the TerrainEditor and WorldEditor
   // to have descriptive UndoAction names then we can change
   // the text as part of the menu update.
      
   %undoName = %this.getNextUndoName();
   %redoName = %this.getNextRedoName();
   
   %editMenu.setItemName( 0, "Undo " @ %undoName );
   %editMenu.setItemName( 1, "Redo " @ %redoName );
   
   %editMenu.enableItem( 0, %undoName !$= "" );
   %editMenu.enableItem( 1, %redoName !$= "" );
}


/// A helper for submitting a creation undo action.
function MECreateUndoAction::submit( %undoObject )
{
   // The instant group will try to add our
   // UndoAction if we don't disable it.   
   pushInstantGroup();
   
   // Create the undo action.     
   %action = new MECreateUndoAction()
   {
      actionName = "Create " @ %undoObject.getClassName();
   };
   
   // Restore the instant group.
   popInstantGroup();
   
   // Set the object to undo.
   %action.addObject( %undoObject );
   
   // Submit it.
   %action.addToManager( Editor.getUndoManager() );
}

function MECreateUndoAction::onUndone( %this )
{
   EWorldEditor.syncGui();
}

function MECreateUndoAction::onRedone( %this )
{
   EWorldEditor.syncGui();
}


/// A helper for submitting a delete undo action.
/// If %wordSeperated is not specified or is false it is assumed %deleteObjects
/// is tab sperated.
function MEDeleteUndoAction::submit( %deleteObjects, %wordSeperated )
{
   // The instant group will try to add our
   // UndoAction if we don't disable it.   
   pushInstantGroup();
   
   // Create the undo action.     
   %action = new MEDeleteUndoAction()
   {
      actionName = "Delete";
   };

   // Restore the instant group.
   popInstantGroup();
   
   // Add the deletion objects to the action which
   // will take care of properly deleting them.
   %deleteObjects = trim( %deleteObjects );   
   
   if ( %wordSeperated )
   {
      %count = getWordCount( %deleteObjects );
      for ( %i = 0; %i < %count; %i++ )
      {
         %object = getWord( %deleteObjects, %i );
         %action.deleteObject( %object );
      }
   }
   else
   {
      %count = getFieldCount( %deleteObjects );
      for ( %i = 0; %i < %count; %i++ )
      {
         %object = getField( %deleteObjects, %i );
         %action.deleteObject( %object );
      }
   }
   
   // Submit it.
   %action.addToManager( Editor.getUndoManager() );
}

function MEDeleteUndoAction::onUndone( %this )
{
   EWorldEditor.syncGui();
}

function MEDeleteUndoAction::onRedone( %this )
{
   EWorldEditor.syncGui();
}
