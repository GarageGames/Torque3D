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


//=============================================================================================
//    Activation.
//=============================================================================================

$InGuiEditor = false;
$MLAAFxGuiEditorTemp = false;

function GuiEdit( %val )
{
   if (Canvas.isFullscreen())
   {
      MessageBoxOK("Windowed Mode Required", "Please switch to windowed mode to access the GUI Editor.");
      return;
   }

   if(%val != 0)
      return;

   if (!$InGuiEditor)
   {
      GuiEditContent(Canvas.getContent());
      
      //Temp fix to disable MLAA when in GUI editor
      if( isObject(MLAAFx) && MLAAFx.isEnabled==true )
      {
	 MLAAFx.isEnabled = false;
         $MLAAFxGuiEditorTemp = true;
      }

   }
   else
   {
      GuiEditCanvas.quit();
   }

}

function GuiEditContent( %content )
{
   if( !isObject( GuiEditCanvas ) )
      new GuiControl( GuiEditCanvas, EditorGuiGroup );

   GuiEditor.openForEditing( %content );
   
   $InGuiEditor = true;
}

function toggleGuiEditor( %make )
{
   if( %make )
   {
      if( EditorIsActive() && !GuiEditor.toggleIntoEditorGui )
         toggleEditor( true );
         
      GuiEdit();
      
	  // Cancel the scheduled event to prevent
	  // the level from cycling after it's duration
	  // has elapsed.
      cancel($Game::Schedule);
   }
}

GlobalActionMap.bind( keyboard, "f10", toggleGuiEditor );

//=============================================================================================
//    Methods.
//=============================================================================================

package GuiEditor_BlockDialogs
{
   function GuiCanvas::pushDialog() {}
   function GuiCanvas::popDialog() {}
};

//---------------------------------------------------------------------------------------------

function GuiEditor::openForEditing( %this, %content )
{   
   Canvas.setContent( GuiEditorGui );
   while( GuiEditorContent.getCount() )
      GuiGroup.add( GuiEditorContent.getObject( 0 ) ); // get rid of anything being edited
      
   // Clear the current guide set and add the guides
   // from the control.
   
   %this.clearGuides();
   %this.readGuides( %content );
   
   // Enumerate GUIs and put them into the content list.

   GuiEditorContentList.init();
   
   GuiEditorScroll.scrollToTop();
   activatePackage( GuiEditor_BlockDialogs );
   GuiEditorContent.add( %content );
   deactivatePackage( GuiEditor_BlockDialogs );
   GuiEditorContentList.sort();
         
   if(%content.getName() $= "")
      %name = "(unnamed) - " @ %content;
   else
      %name = %content.getName() @ " - " @ %content;
   
   GuiEditorContentList.setText(%name);

   %this.setContentControl(%content);
   
   // Initialize the preview resolution list and select the current
   // preview resolution.

   GuiEditorResList.init();

   %res = %this.previewResolution;
   if( %res $= "" )
      %res = "1024 768";
   GuiEditorResList.selectFormat( %res );

   // Initialize the treeview and expand the first level.

   GuiEditorTreeView.init();
   GuiEditorTreeView.open( %content );
   GuiEditorTreeView.expandItem( 1 );
   
   // Initialize profiles tree.
   
   if( !GuiEditorProfilesTree.isInitialized )
   {
      GuiEditorProfilesTree.init();
      GuiEditorProfilesTree.isInitialized = true;
   }
   
   // Create profile change manager if we haven't already.
   
   if( !isObject( GuiEditorProfileChangeManager ) )
      new SimGroup( GuiEditorProfileChangeManager );
   
   // clear the undo manager if we're switching controls.
   if( %this.lastContent != %content )
      GuiEditor.getUndoManager().clearAll();
      
   GuiEditor.setFirstResponder();
   
   %this.updateUndoMenu();
   %this.lastContent = %content;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::switchToWorldEditor( %this )
{
   %editingWorldEditor = false;
   if( GuiEditorContent.getObject( 0 ) == EditorGui.getId() )
      %editingWorldEditor = true;
      
   GuiEdit();
   
   if( !$missionRunning )
      EditorNewLevel();
   else if( !%editingWorldEditor )
      toggleEditor( true );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::enableMenuItems(%this, %val)
{
   %menu = GuiEditCanvas.menuBar->EditMenu.getID();
   
   %menu.enableItem( 3, %val ); // cut
   %menu.enableItem( 4, %val ); // copy
   %menu.enableItem( 5, %val ); // paste
   //%menu.enableItem( 7, %val ); // selectall
   //%menu.enableItem( 8, %val ); // deselectall
   %menu.enableItem( 9, %val ); // selectparents
   %menu.enableItem( 10, %val ); // selectchildren
   %menu.enableItem( 11, %val ); // addselectparents
   %menu.enableItem( 12, %val ); // addselectchildren
   %menu.enableItem( 15, %val ); // lock
   %menu.enableItem( 16, %val ); // hide
   %menu.enableItem( 18, %val ); // group
   %menu.enableItem( 19, %val ); // ungroup
   
   GuiEditCanvas.menuBar->LayoutMenu.enableAllItems( %val );
   GuiEditCanvas.menuBar->MoveMenu.enableAllItems( %val );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::showPrefsDialog(%this)
{
   Canvas.pushDialog(GuiEditorPrefsDlg);
}

//---------------------------------------------------------------------------------------------

function GuiEditor::getUndoManager( %this )
{
   if( !isObject( GuiEditorUndoManager ) )
      new UndoManager( GuiEditorUndoManager );
   
   return GuiEditorUndoManager;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::undo(%this)
{
   %action = %this.getUndoManager().getNextUndoName();
   
   %this.getUndoManager().undo();
   %this.updateUndoMenu();
   //%this.clearSelection();
   
   GuiEditorStatusBar.print( "Undid '" @ %action @ "'" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::redo(%this)
{
   %action = %this.getUndoManager().getNextRedoName();

   %this.getUndoManager().redo();
   %this.updateUndoMenu();
   //%this.clearSelection();

   GuiEditorStatusBar.print( "Redid '" @ %action @ "'" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::updateUndoMenu(%this)
{
   %uman = %this.getUndoManager();
   %nextUndo = %uman.getNextUndoName();
   %nextRedo = %uman.getNextRedoName();
   
   %editMenu = GuiEditCanvas.menuBar->editMenu;
   
   %editMenu.setItemName( 0, "Undo " @ %nextUndo );
   %editMenu.setItemName( 1, "Redo " @ %nextRedo );
   
   %editMenu.enableItem( 0, %nextUndo !$= "" );
   %editMenu.enableItem( 1, %nextRedo !$= "" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::isFilteredClass( %this, %className )
{
   // Filter out all the internal GuiInspector classes.
   
   if( startsWith( %className, "GuiInspector" ) && %className !$= "GuiInspector" )
      return true;
      
   // Filter out GuiEditor classes.
   
   if( startsWith( %className, "GuiEditor" ) )
      return true;
      
   // Filter out specific classes.
      
   switch$( %className )
   {
      case "GuiCanvas":             return true;
      case "GuiAviBitmapCtrl":      return true; // For now.  Probably removed altogether.
      case "GuiArrayCtrl":          return true; // Abstract base class really.
      case "GuiScintillaTextCtrl":  return true; // Internal class.
      case "GuiNoMouseCtrl":        return true; // Too odd.
      case "GuiEditCtrl":           return true;
      case "GuiBackgroundCtrl":     return true; // Just plain useless.
      case "GuiTSCtrl":             return true; // Abstract base class.
      case "GuiTickCtrl":           return true; // Abstract base class.
      case "GuiWindowCollapseCtrl": return true; // Legacy.
   }
   
   return false;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::editProfile( %this, %profile )
{
   GuiEditorTabBook->profilesPage.select();
   GuiEditorProfilesTree.setSelectedProfile( %profile );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::createControl( %this, %className )
{
   %ctrl = eval( "return new " @ %className @ "();" );
   if( !isObject( %ctrl ) )
      return;
      
   // Add the control.
   
   %this.addNewCtrl( %ctrl );
}

//---------------------------------------------------------------------------------------------

/// Group all GuiControls in the currenct selection set under a new GuiControl.
function GuiEditor::groupSelected( %this )
{
   %selection = %this.getSelection();
   if( %selection.getCount() < 2 )
      return;
         
   // Create action.
   
   %action = GuiEditorGroupAction::create( %selection, GuiEditor.getContentControl() );   
   %action.groupControls();

   // Update editor tree.
   
   %this.clearSelection();
   %this.addSelection( %action.group[ 0 ].groupObject );
   GuiEditorTreeView.update();
   
   // Update undo state.

   %action.addtoManager( %this.getUndoManager() );
   %this.updateUndoMenu();
}

//---------------------------------------------------------------------------------------------

/// Take all direct GuiControl instances in the selection set and reparent their child controls
/// to each of the group's parents.  The GuiControl group objects are deleted.
function GuiEditor::ungroupSelected( %this )
{
   %action = GuiEditorUngroupAction::create( %this.getSelection() );
   %action.ungroupControls();
   
   // Update editor tree.
   
   %this.clearSelection();
   GuiEditorTreeView.update();
   
   // Update undo state.
   
   %action.addToManager( %this.getUndoManager() );
   %this.updateUndoMenu();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::deleteControl( %this, %ctrl )
{
   // Unselect.
   
   GuiEditor.removeSelection( %ctrl );
   
   // Record undo.
   
   %set = new SimSet() { parentGroup = RootGroup; };
   %set.add( %ctrl );
   
   %action = UndoActionDeleteObject::create( %set, %this.getTrash(), GuiEditorTreeView );
   %action.addToManager( %this.getUndoManager() );
   %this.updateUndoMenu();
   
   GuiEditorTreeView.update();
   %set.delete();
   
   // Remove.
   
   %this.getTrash().add( %ctrl );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::setPreviewResolution( %this, %width, %height )
{
   GuiEditorRegion.resize( 0, 0, %width, %height );
   GuiEditorContent.getObject( 0 ).resize( 0, 0, %width, %height );
   
   GuiEditor.previewResolution = %width SPC %height;
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleEdgeSnap( %this )
{
   %this.snapToEdges = !%this.snapToEdges;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_EDGESNAP_INDEX, %this.snapToEdges );
   GuiEditorEdgeSnapping_btn.setStateOn( %this.snapToEdges );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleCenterSnap( %this )
{
   %this.snapToCenters = !%this.snapToCenters;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CENTERSNAP_INDEX, %this.snapToCenters );
   GuiEditorCenterSnapping_btn.setStateOn( %this.snapToCenters );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleFullBoxSelection( %this )
{
   %this.fullBoxSelection = !%this.fullBoxSelection;
   GuiEditCanvas.menuBar->EditMenu.checkItem( $GUI_EDITOR_MENU_FULLBOXSELECT_INDEX, %this.fullBoxSelection );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleDrawGuides( %this )
{
   %this.drawGuides= !%this.drawGuides;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_DRAWGUIDES_INDEX, %this.drawGuides );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleGuideSnap( %this )
{
   %this.snapToGuides = !%this.snapToGuides;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_GUIDESNAP_INDEX, %this.snapToGuides );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleControlSnap( %this )
{
   %this.snapToControls = !%this.snapToControls;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CONTROLSNAP_INDEX, %this.snapToControls );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleCanvasSnap( %this )
{
   %this.snapToCanvas = !%this.snapToCanvas;
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CANVASSNAP_INDEX, %this.snapToCanvas );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleGridSnap( %this )
{
   %this.snap2Grid = !%this.snap2Grid;
   if( !%this.snap2Grid )
      %this.setSnapToGrid( 0 );
   else
      %this.setSnapToGrid( %this.snap2GridSize );

   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_GRIDSNAP_INDEX, %this.snap2Grid );
   GuiEditorSnapCheckBox.setStateOn( %this.snap2Grid );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleLockSelection( %this )
{
   %this.toggleFlagInAllSelectedObjects( "locked" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleHideSelection( %this )
{
   %this.toggleFlagInAllSelectedObjects( "hidden" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::selectAllControlsInSet( %this, %set, %deselect )
{
   if( !isObject( %set ) )
      return;
      
   foreach( %obj in %set )
   {
      if( !%obj.isMemberOfClass( "GuiControl" ) )
         continue;
         
      if( !%deselect )
         %this.addSelection( %obj );
      else
         %this.removeSelection( %obj );
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditor::toggleFlagInAllSelectedObjects( %this, %flagFieldName )
{
   // Use the inspector's code here to record undo information
   // for the field edits.
   
   GuiEditorInspectFields.onInspectorPreFieldModification( %flagFieldName );
   
   %selected = %this.getSelection();
   foreach( %object in %selected )
      %object.setFieldValue( %flagFieldName, !%object.getFieldValue( %flagFieldName ) );
   
   GuiEditorInspectFields.onInspectorPostFieldModification();
   GuiEditorInspectFields.refresh();
}

//=============================================================================================
//    Event Handlers.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditor::onDelete(%this)
{
   GuiEditorTreeView.update();
   // clear out the gui inspector.
   GuiEditorInspectFields.update(0);
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onSelectionMoved( %this, %ctrl )
{
   GuiEditorInspectFields.update( %ctrl );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onSelectionResized( %this, %ctrl )
{
   GuiEditorInspectFields.update( %ctrl );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onSelect(%this, %ctrl)
{
   if( !%this.dontSyncTreeViewSelection )
   {
      GuiEditorTreeView.clearSelection();
      GuiEditorTreeView.addSelection( %ctrl );
   }
   
   GuiEditorInspectFields.update( %ctrl );
   
   GuiEditorSelectionStatus.setText( "1 Control Selected" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onAddSelected( %this, %ctrl )
{
   if( !%this.dontSyncTreeViewSelection )
   {
      GuiEditorTreeView.addSelection( %ctrl );
      GuiEditorTreeView.scrollVisibleByObjectId( %ctrl );
   }
   
   GuiEditorSelectionStatus.setText( %this.getNumSelected() @ " Controls Selected" );

   // Add to inspection set.
   
   GuiEditorInspectFields.addInspect( %ctrl );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onRemoveSelected( %this, %ctrl )
{
   if( !%this.dontSyncTreeViewSelection )
      GuiEditorTreeView.removeSelection( %ctrl );
      
   GuiEditorSelectionStatus.setText( %this.getNumSelected() @ " Controls Selected" );
   
   // Remove from inspection set.
   
   GuiEditorInspectFields.removeInspect( %ctrl );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onClearSelected( %this )
{ 
   if( !%this.dontSyncTreeViewSelection )
      GuiEditorTreeView.clearSelection();
      
   GuiEditorInspectFields.update( 0 );
   GuiEditorSelectionStatus.setText( "" );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onControlDragged( %this, %payload, %position )
{
   // Make sure we have the right kind of D&D.
   
   if( !%payload.parentGroup.isInNamespaceHierarchy( "GuiDragAndDropControlType_GuiControl" ) )
      return;
      
   // use the position under the mouse cursor, not the payload position.
   %position = VectorSub( %position, GuiEditorContent.getGlobalPosition() );
   %x = getWord( %position, 0 );
   %y = getWord( %position, 1 );
   %target = GuiEditor.getContentControl().findHitControl( %x, %y );
   
   // Make sure the target is a valid parent for our payload.
   
   while(    ( !%target.isContainer || !%target.acceptsAsChild( %payload ) )
          && %target != GuiEditor.getContentControl() )
      %target = %target.getParent();
      
   if( %target != %this.getCurrentAddSet() )
      %this.setCurrentAddSet( %target );
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onControlDropped(%this, %payload, %position)
{  
   // Make sure we have the right kind of D&D.
   
   if( !%payload.parentGroup.isInNamespaceHierarchy( "GuiDragAndDropControlType_GuiControl" ) )
      return;

   %pos = %payload.getGlobalPosition();
   %x = getWord(%pos, 0);
   %y = getWord(%pos, 1);

   %this.addNewCtrl(%payload);
   
   %payload.setPositionGlobal(%x, %y);
   %this.setFirstResponder();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onGainFirstResponder(%this)
{
   %this.enableMenuItems(true);
   
   // JCF: don't just turn them all on!
   // Undo/Redo is only enabled if those actions exist.
   %this.updateUndoMenu();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onLoseFirstResponder(%this)
{
   %this.enableMenuItems(false);
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onHierarchyChanged( %this )
{
   GuiEditorTreeView.update();
}

//---------------------------------------------------------------------------------------------

function GuiEditor::onMouseModeChange( %this )
{
   GuiEditorStatusBar.setText( GuiEditorStatusBar.getMouseModeHelp() );
}

//=============================================================================================
//    Resolution List.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorResList::init( %this )
{
   %this.clear();
   
   // Non-widescreen formats.
   
   %this.add( "640x480 (VGA, 4:3)", 640 );
   %this.add( "800x600 (SVGA, 4:3)", 800 );
   %this.add( "1024x768 (XGA, 4:3)", 1024 );
   %this.add( "1280x1024 (SXGA, 4:3)", 1280 );
   %this.add( "1600x1200 (UXGA, 4:3)", 1600 );
   
   // Widescreen formats.
   
   %this.add( "1280x720 (WXGA, 16:9)", 720 );
   %this.add( "1600x900 (16:9)", 900 );
   %this.add( "1920x1080 (16:9)", 1080 );
   %this.add( "1440x900 (WXGA+, 16:10)", 900 );
   %this.add( "1680x1050 (WSXGA+, 16:10)", 1050 );
   %this.add( "1920x1200 (WUXGA, 16:10)", 1200 );   
}

//---------------------------------------------------------------------------------------------

function GuiEditorResList::selectFormat( %this, %format )
{
   %width = getWord( %format, 0 );
   %height = getWord( %format, 1 );
   
   switch( %height )
   {
      case 720:
         %this.setSelected( 720 );
         
      case 900:
         %this.setSelected( 900 );
         
      case 1050:
         %this.setSelected( 1050 );
         
      case 1080:
         %this.setSelected( 1080 );
         
      default:
      
         switch( %width )
         {
            case 640:
               %this.setSelected( 640 );
               
            case 800:
               %this.setSelected( 800 );
               
            case 1024:
               %this.setSelected( 1024 );
               
            case 1280:
               %this.setSelected( 1280 );
               
            case 1600:
               %this.setSelected( 1600 );
               
            default:
               %this.setSelected( 1200 );
         }
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorResList::onSelect( %this, %id )
{
   switch( %id )
   {
      case 640:
         GuiEditor.setPreviewResolution( 640, 480 );
         
      case 800:
         GuiEditor.setPreviewResolution( 800, 600 );
         
      case 1024:
         GuiEditor.setPreviewResolution( 1024, 768 );
         
      case 1280:
         GuiEditor.setPreviewResolution( 1280, 1024 );
         
      case 1600:
         GuiEditor.setPreviewResolution( 1600, 1200 );
         
      case 720:
         GuiEditor.setPreviewResolution( 1280, 720 );
         
      case 900:
         GuiEditor.setPreviewResolution( 1440, 900 );
         
      case 1050:
         GuiEditor.setPreviewResolution( 1680, 1050 );
         
      case 1080:
         GuiEditor.setPreviewResolution( 1920, 1080 );
         
      case 1200:
         GuiEditor.setPreviewResolution( 1920, 1200 );
   }
}

//=============================================================================================
//    Sidebar.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorTabBook::onWake( %this )
{
   if( !isObject( "GuiEditorTabBookLibraryPopup" ) )
      new PopupMenu( GuiEditorTabBookLibraryPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Alphabetical View" TAB "" TAB "GuiEditorToolbox.setViewType( \"Alphabetical\" );";
         item[ 1 ] = "Categorized View" TAB "" TAB "GuiEditorToolbox.setViewType( \"Categorized\" );";
      };
}

//---------------------------------------------------------------------------------------------

function GuiEditorTabBook::onTabSelected( %this, %text, %index )
{
   %sidebar = GuiEditorSidebar;
   %name = %this.getObject( %index ).getInternalName();
   
   switch$( %name )
   {
      case "guiPage":
      
         %sidebar-->button1.setVisible( false );
         %sidebar-->button2.setVisible( false );
         %sidebar-->button3.setVisible( true );
         %sidebar-->button4.setVisible( true );
         
         %sidebar-->button4.setBitmap( "tools/gui/images/delete" );
         %sidebar-->button4.command = "GuiEditor.deleteSelection();";
         %sidebar-->button4.tooltip = "Delete Selected Control(s)";
         
         %sidebar-->button3.setBitmap( "tools/gui/images/visible" );
         %sidebar-->button3.command = "GuiEditor.toggleHideSelection();";
         %sidebar-->button3.tooltip = "Hide Selected Control(s)";
                  
      case "profilesPage":

         %sidebar-->button1.setVisible( true );
         %sidebar-->button2.setVisible( true );
         %sidebar-->button3.setVisible( true );
         %sidebar-->button4.setVisible( true );
      
         %sidebar-->button4.setBitmap( "tools/gui/images/delete" );
         %sidebar-->button4.command = "GuiEditor.showDeleteProfileDialog( GuiEditorProfilesTree.getSelectedProfile() );";
         %sidebar-->button4.tooltip = "Delete Selected Profile";
      
         %sidebar-->button3.setBitmap( "tools/gui/images/new" );
         %sidebar-->button3.command = "GuiEditor.createNewProfile( \"Unnamed\" );";
         %sidebar-->button3.tooltip = "Create New Profile with Default Values";
         
         %sidebar-->button2.setBitmap( "tools/gui/images/copy-btn" );
         %sidebar-->button2.command = "GuiEditor.createNewProfile( GuiEditorProfilesTree.getSelectedProfile().getName(), GuiEditorProfilesTree.getSelectedProfile() );";
         %sidebar-->button2.tooltip = "Create New Profile by Copying the Selected Profile";

         %sidebar-->button1.setBitmap( "tools/gui/images/reset-icon" );
         %sidebar-->button1.command = "GuiEditor.revertProfile( GuiEditorProfilesTree.getSelectedProfile() );";
         %sidebar-->button1.tooltip = "Revert Changes to the Selected Profile";
         
      case "toolboxPage":
      
         //TODO
         
         %sidebar-->button1.setVisible( false );
         %sidebar-->button2.setVisible( false );
         %sidebar-->button3.setVisible( false );
         %sidebar-->button4.setVisible( false );
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorTabBook::onTabRightClick( %this, %text, %index )
{
   %name = %this.getObject( %index ).getInternalName();
   
   switch$( %name )
   {
      case "toolboxPage":
      
         // Open toolbox popup.
         
         %popup = GuiEditorTabBookLibraryPopup;
         
         %currentViewType = GuiEditorToolbox.getViewType();
         switch$( %currentViewType )
         {
            case "Alphabetical":
               %popup.checkRadioItem( 0, 1, 0 );
               
            case "Categorized":
               %popup.checkRadioItem( 0, 1, 1 );
         }
         
         %popup.showPopup( Canvas );
   }
}

//=============================================================================================
//    Toolbar.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorSnapCheckBox::onWake(%this)
{
   %snap = GuiEditor.snap2grid * GuiEditor.snap2gridsize;
   %this.setValue( %snap );
   GuiEditor.setSnapToGrid( %snap );
}

//---------------------------------------------------------------------------------------------

function GuiEditorSnapCheckBox::onAction(%this)
{
   %snap = GuiEditor.snap2gridsize * %this.getValue();
   GuiEditor.snap2grid = %this.getValue();
   GuiEditor.setSnapToGrid(%snap);
}

//=============================================================================================
//    GuiEditorGui.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorGui::onWake( %this )
{
   GHGuiEditor.setStateOn( 1 );
   
   if( !isObject( %this->SelectControlsDlg ) )
   {
      %this.add( GuiEditorSelectDlg );
      GuiEditorSelectDlg.setVisible( false );
   }
         
   // Attach our menus.
   
   if( isObject( %this.menuGroup ) )
      for( %i = 0; %i < %this.menuGroup.getCount(); %i ++ )
         %this.menuGroup.getObject( %i ).attachToMenuBar();
         
   // Read settings.

   %this.initSettings();
   %this.readSettings();
   
   // Initialize toolbox.

   if( !GuiEditorToolbox.isInitialized )
      GuiEditorToolbox.initialize();

   // Set up initial menu toggle states.
   
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_EDGESNAP_INDEX, GuiEditor.snapToEdges );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CENTERSNAP_INDEX, GuiEditor.snapToCenters );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_GUIDESNAP_INDEX, GuiEditor.snapToGuides );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CONTROLSNAP_INDEX, GuiEditor.snapToControls );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_CANVASSNAP_INDEX, GuiEditor.snapToCanvas );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_GRIDSNAP_INDEX, GuiEditor.snap2Grid );
   GuiEditCanvas.menuBar->SnapMenu.checkItem( $GUI_EDITOR_MENU_DRAWGUIDES_INDEX, GuiEditor.drawGuides );
   GuiEditCanvas.menuBar->EditMenu.checkItem( $GUI_EDITOR_MENU_FULLBOXSELECT_INDEX, GuiEditor.fullBoxSelection );

   // Sync toolbar buttons.
   
   GuiEditorSnapCheckBox.setStateOn( GuiEditor.snap2Grid );
   GuiEditorEdgeSnapping_btn.setStateOn( GuiEditor.snapToEdges );
   GuiEditorCenterSnapping_btn.setStateOn( GuiEditor.snapToCenters );
}

//---------------------------------------------------------------------------------------------

function GuiEditorGui::onSleep( %this)
{
   // If we are editing a control, store its guide state.
   
   %content = GuiEditor.getContentControl();
   if( isObject( %content ) )
      GuiEditor.writeGuides( %content );

   // Remove our menus.
   
   if( isObject( %this.menuGroup ) )
      for( %i = 0; %i < %this.menuGroup.getCount(); %i ++ )
         %this.menuGroup.getObject( %i ).removeFromMenuBar();
         
   // Store our preferences.
   
   %this.writeSettings();
}

//---------------------------------------------------------------------------------------------

function GuiEditorGui::initSettings( %this )
{
   EditorSettings.beginGroup( "GuiEditor", true );
   
      EditorSettings.setDefaultValue( "lastPath", "" );
      EditorSettings.setDefaultValue( "previewResolution", "1024 768" );
      
      EditorSettings.beginGroup( "EngineDevelopment" );
      EditorSettings.setDefaultValue( "toggleIntoEditor", 0 );
      EditorSettings.setDefaultValue( "showEditorProfiles", 0 );
      EditorSettings.setDefaultValue( "showEditorGuis", 0 );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Library" );
      EditorSettings.setDefaultValue( "viewType", "Categorized" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Snapping" );
      EditorSettings.setDefaultValue( "snapToControls", "1" );
      EditorSettings.setDefaultValue( "snapToGuides", "1" );
      EditorSettings.setDefaultValue( "snapToCanvas", "1" );
      EditorSettings.setDefaultValue( "snapToEdges", "1" );
      EditorSettings.setDefaultValue( "snapToCenters", "1" );
      EditorSettings.setDefaultValue( "sensitivity", "2" );
      EditorSettings.setDefaultValue( "snap2Grid", "0" );
      EditorSettings.setDefaultValue( "snap2GridSize", $GuiEditor::defaultGridSize );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Selection" );
      EditorSettings.setDefaultValue( "fullBox", "0" );
      EditorSettings.endGroup();
      
      EditorSettings.beginGroup( "Rendering" );
      EditorSettings.setDefaultValue( "drawBorderLines", "1" );
      EditorSettings.setDefaultValue( "drawGuides", "1" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Help" );
      EditorSettings.setDefaultValue( "documentationURL", "http://www.garagegames.com/products/torque-3d/documentation/user" ); //RDTODO: make this point to Gui Editor docs when available

      // Create a path to the local documentation.  This is a bit of guesswork here.
      // It assumes that the project is located in a folder of the SDK root directory
      // (e.g. "Examples/" or "Demos/") and that from there the path to the game
      // folder is "<project>/game".
      EditorSettings.setDefaultValue("documentationLocal", "../../../Documentation/Official Documentation.html"  );
      
      EditorSettings.setDefaultValue("documentationReference", "../../../Documentation/Torque 3D - Script Manual.chm"  );
      
      EditorSettings.endGroup();

   EditorSettings.endGroup();
}

//---------------------------------------------------------------------------------------------

function GuiEditorGui::readSettings( %this )
{
   EditorSettings.read();
   
   EditorSettings.beginGroup( "GuiEditor", true );

      GuiEditor.lastPath = EditorSettings.value( "lastPath" );
      GuiEditor.previewResolution = EditorSettings.value( "previewResolution" );

      EditorSettings.beginGroup( "EngineDevelopment" );
      GuiEditor.toggleIntoEditor = EditorSettings.value( "toggleIntoEditor" );
      GuiEditor.showEditorProfiles = EditorSettings.value( "showEditorProfiles" );
      GuiEditor.showEditorGuis = EditorSettings.value( "showEditorGuis" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Library" );
      GuiEditorToolbox.currentViewType = EditorSettings.value( "viewType" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Snapping" );
      GuiEditor.snapToGuides = EditorSettings.value( "snapToGuides" );
      GuiEditor.snapToControls = EditorSettings.value( "snapToControls" );
      GuiEditor.snapToCanvas = EditorSettings.value( "snapToCanvas" );
      GuiEditor.snapToEdges = EditorSettings.value( "snapToEdges" );
      GuiEditor.snapToCenters = EditorSettings.value( "snapToCenters" );
      GuiEditor.snapSensitivity = EditorSettings.value( "sensitivity" );
      GuiEditor.snap2Grid = EditorSettings.value( "snap2Grid" );
      GuiEditor.snap2GridSize = EditorSettings.value( "snap2GridSize" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Selection" );
      GuiEditor.fullBoxSelection = EditorSettings.value( "fullBox" );
      EditorSettings.endGroup();
   
      EditorSettings.beginGroup( "Rendering" );
      GuiEditor.drawBorderLines = EditorSettings.value( "drawBorderLines" );
      GuiEditor.drawGuides = EditorSettings.value( "drawGuides" );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Help" );
      GuiEditor.documentationURL = EditorSettings.value( "documentationURL" );
      GuiEditor.documentationLocal = EditorSettings.value( "documentationLocal" );
      GuiEditor.documentationReference = EditorSettings.value( "documentationReference" );
      EditorSettings.endGroup();

   EditorSettings.endGroup();

   if( GuiEditor.snap2Grid )
      GuiEditor.setSnapToGrid( GuiEditor.snap2GridSize );
}

//---------------------------------------------------------------------------------------------

function GuiEditorGui::writeSettings( %this )
{
   EditorSettings.beginGroup( "GuiEditor", true );
   
      EditorSettings.setValue( "lastPath", GuiEditor.lastPath );
      EditorSettings.setValue( "previewResolution", GuiEditor.previewResolution );

      EditorSettings.beginGroup( "EngineDevelopment" );
      EditorSettings.setValue( "toggleIntoEditor", GuiEditor.toggleIntoEditor );
      EditorSettings.setValue( "showEditorProfiles", GuiEditor.showEditorProfiles );
      EditorSettings.setValue( "showEditorGuis", GuiEditor.showEditorGuis );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Library" );
      EditorSettings.setValue( "viewType", GuiEditorToolbox.currentViewType );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Snapping" );
      EditorSettings.setValue( "snapToControls", GuiEditor.snapToControls );
      EditorSettings.setValue( "snapToGuides", GuiEditor.snapToGuides );
      EditorSettings.setValue( "snapToCanvas", GuiEditor.snapToCanvas );
      EditorSettings.setValue( "snapToEdges", GuiEditor.snapToEdges );
      EditorSettings.setValue( "snapToCenters", GuiEditor.snapToCenters );
      EditorSettings.setValue( "sensitivity", GuiEditor.snapSensitivity );
      EditorSettings.setValue( "snap2Grid", GuiEditor.snap2Grid );
      EditorSettings.setValue( "snap2GridSize", GuiEditor.snap2GridSize );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Selection" );
      EditorSettings.setValue( "fullBox", GuiEditor.fullBoxSelection );
      EditorSettings.endGroup();
      
      EditorSettings.beginGroup( "Rendering" );
      EditorSettings.setValue( "drawBorderLines", GuiEditor.drawBorderLines );
      EditorSettings.setValue( "drawGuides", GuiEditor.drawGuides );
      EditorSettings.endGroup();

      EditorSettings.beginGroup( "Help" );
      EditorSettings.setValue( "documentationURL", GuiEditor.documentationURL );
      EditorSettings.setValue( "documentationLocal", GuiEditor.documentationLocal );
      EditorSettings.setValue( "documentationReference", GuiEditor.documentationReference );
      EditorSettings.endGroup();

   EditorSettings.endGroup();

   EditorSettings.write();
}
