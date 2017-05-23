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

// Code for the main Gui Editor tree view that shows the hierarchy of the
// current GUI being edited.


//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::init(%this)
{
   if( !isObject( %this.contextMenu ) )
      %this.contextMenu = new PopupMenu()
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Rename" TAB "" TAB "GuiEditorTreeView.showItemRenameCtrl( GuiEditorTreeView.findItemByObjectId( %this.object ) );";
         item[ 1 ] = "Delete" TAB "" TAB "GuiEditor.deleteControl( %this.object );";
         item[ 2 ] = "-";
         item[ 3 ] = "Locked" TAB "" TAB "%this.object.setLocked( !%this.object.locked ); GuiEditorTreeView.update();";
         item[ 4 ] = "Hidden" TAB "" TAB "%this.object.setVisible( !%this.object.isVisible() ); GuiEditorTreeView.update();";
         item[ 5 ] = "-";
         item[ 6 ] = "Add New Controls Here" TAB "" TAB "GuiEditor.setCurrentAddSet( %this.object );";
         item[ 7 ] = "Add Child Controls to Selection" TAB "" TAB "GuiEditor.selectAllControlsInSet( %this.object, false );";
         item[ 8 ] = "Remove Child Controls from Selection" TAB "" TAB "GuiEditor.selectAllControlsInSet( %this.object, true );";
         
         object = -1;
      };
      
   if( !isObject( %this.contextMenuMultiSel ) )
      %this.contextMenuMultiSel = new PopupMenu()
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Delete" TAB "" TAB "GuiEditor.deleteSelection();";
      };
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::update( %this )
{
   %obj = GuiEditorContent.getObject( 0 );
   
   if( !isObject( %obj ) )
      GuiEditorTreeView.clear();
   else
   {
      // Open inspector tree.
      
      GuiEditorTreeView.open( %obj );
      
      // Sync selection with GuiEditor.
      
      GuiEditorTreeView.clearSelection();
      
      %selection = GuiEditor.getSelection();
      %count = %selection.getCount();
      
      for( %i = 0; %i < %count; %i ++ )
         GuiEditorTreeView.addSelection( %selection.getObject( %i ) );
   }
}

//=============================================================================================
//    Event Handlers.
//=============================================================================================

//---------------------------------------------------------------------------------------------

/// Defines the icons to be used in the tree view control.
/// Provide the paths to each icon minus the file extension.
/// Seperate them with ':'.
/// The order of the icons must correspond to the bit array defined
/// in the GuiTreeViewCtrl.h.
function GuiEditorTreeView::onDefineIcons(%this)
{
   %icons = ":" @       // Default1
            ":" @       // SimGroup1
            ":" @       // SimGroup2
            ":" @       // SimGroup3
            ":" @       // SimGroup4
            "tools/gui/images/treeview/hidden:" @
            "tools/worldEditor/images/lockedHandle";

   GuiEditorTreeView.buildIconTable( %icons );
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onRightMouseDown( %this, %item, %pts, %obj )
{
   if( %this.getSelectedItemsCount() > 1 )
   {
      %popup = %this.contextMenuMultiSel;
      %popup.showPopup( Canvas );
   }
   else if( %obj )
   {
      %popup = %this.contextMenu;
      
      %popup.checkItem( 3, %obj.locked );
      %popup.checkItem( 4, !%obj.isVisible() );
      
      %popup.enableItem( 6, %obj.isContainer );
      %popup.enableItem( 7, %obj.getCount() > 0 );
      %popup.enableItem( 8, %obj.getCount() > 0 );
      
      %popup.object = %obj;      
      %popup.showPopup( Canvas );
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onAddSelection(%this,%ctrl)
{   
   GuiEditor.dontSyncTreeViewSelection = true;
   GuiEditor.addSelection( %ctrl );
   GuiEditor.dontSyncTreeViewSelection = false;
   GuiEditor.setFirstResponder();
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onRemoveSelection( %this, %ctrl )
{
   GuiEditor.dontSyncTreeViewSelection = true;
   GuiEditor.removeSelection( %ctrl );
   GuiEditor.dontSyncTreeViewSelection = false;
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onDeleteSelection(%this)
{ 
   GuiEditor.clearSelection();
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onSelect( %this, %obj )
{
   if( isObject( %obj ) )
   {
      GuiEditor.dontSyncTreeViewSelection = true;
      GuiEditor.select( %obj );
      GuiEditor.dontSyncTreeViewSelection = false;
      GuiEditorInspectFields.update( %obj );
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::isValidDragTarget( %this, %id, %obj )
{
   return ( %obj.isContainer || %obj.getCount() > 0 );
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onBeginReparenting( %this )
{
   if( isObject( %this.reparentUndoAction ) )
      %this.reparentUndoAction.delete();
      
   %action = UndoActionReparentObjects::create( %this );
   
   %this.reparentUndoAction = %action;
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onReparent( %this, %obj, %oldParent, %newParent )
{
   %this.reparentUndoAction.add( %obj, %oldParent, %newParent );
}

//---------------------------------------------------------------------------------------------

function GuiEditorTreeView::onEndReparenting( %this )
{
   %action = %this.reparentUndoAction;
   %this.reparentUndoAction = "";

   if( %action.numObjects > 0 )
   {
      if( %action.numObjects == 1 )
         %action.actionName = "Reparent Control";
      else
         %action.actionName = "Reparent Controls";
         
      %action.addToManager( GuiEditor.getUndoManager() );

      GuiEditor.updateUndoMenu();
   }
   else
      %action.delete();
}
