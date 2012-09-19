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
//    Event Handlers.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::onAdd( %this )
{
   // %this.setWindowTitle("Torque Gui Editor");

   %this.onCreateMenu();
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::onRemove( %this )
{
   if( isObject( GuiEditorGui.menuGroup ) )
      GuiEditorGui.delete();

   // cleanup
   %this.onDestroyMenu();
}

//---------------------------------------------------------------------------------------------

/// Create the Gui Editor menu bar.
function GuiEditCanvas::onCreateMenu(%this)
{
   if(isObject(%this.menuBar))
      return;
   
   //set up %cmdctrl variable so that it matches OS standards
   if( $platform $= "macos" )
   {
      %cmdCtrl = "cmd";
      %redoShortcut = "Cmd-Shift Z";
   }
   else
   {
      %cmdCtrl = "Ctrl";
      %redoShort = "Ctrl Y";
   }
   
   // Menu bar
   %this.menuBar = new MenuBar()
   {
      dynamicItemInsertPos = 3;
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "File";
         internalName = "FileMenu";
         
         item[0] = "New Gui..." TAB %cmdCtrl SPC "N" TAB %this @ ".create();";
         item[1] = "Open..." TAB %cmdCtrl SPC "O" TAB %this @ ".open();";
         item[2] = "Save" TAB %cmdCtrl SPC "S" TAB %this @ ".save( false, true );";
         item[3] = "Save As..." TAB %cmdCtrl @ "-Shift S" TAB %this @ ".save( false );";
         item[4] = "Save Selected As..." TAB %cmdCtrl @ "-Alt S" TAB %this @ ".save( true );";
         item[5] = "-";
         item[6] = "Revert Gui" TAB "" TAB %this @ ".revert();";
         item[7] = "Add Gui From File..." TAB "" TAB %this @ ".append();";
         item[8] = "-";
         item[9] = "Open Gui File in Torsion" TAB "" TAB %this @".openInTorsion();";
         item[10] = "-";
         item[11] = "Close Editor" TAB "F10" TAB %this @ ".quit();";
         item[12] = "Quit" TAB %cmdCtrl SPC "Q" TAB "quit();";
      };

      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "Edit";
         internalName = "EditMenu";
         
         item[0] = "Undo" TAB %cmdCtrl SPC "Z" TAB "GuiEditor.undo();";
         item[1] = "Redo" TAB %redoShortcut TAB "GuiEditor.redo();";
         item[2] = "-";
         item[3] = "Cut" TAB %cmdCtrl SPC "X" TAB "GuiEditor.saveSelection(); GuiEditor.deleteSelection();";
         item[4] = "Copy" TAB %cmdCtrl SPC "C" TAB "GuiEditor.saveSelection();";
         item[5] = "Paste" TAB %cmdCtrl SPC "V" TAB "GuiEditor.loadSelection();";
         item[6] = "-";
         item[7] = "Select All" TAB %cmdCtrl SPC "A" TAB "GuiEditor.selectAll();";
         item[8] = "Deselect All" TAB %cmdCtrl SPC "D" TAB "GuiEditor.clearSelection();";
         item[9] = "Select Parent(s)" TAB %cmdCtrl @ "-Alt Up" TAB "GuiEditor.selectParents();";
         item[10] = "Select Children" TAB %cmdCtrl @ "-Alt Down" TAB "GuiEditor.selectChildren();";
         item[11] = "Add Parent(s) to Selection" TAB %cmdCtrl @ "-Alt-Shift Up" TAB "GuiEditor.selectParents( true );";
         item[12] = "Add Children to Selection" TAB %cmdCtrl @ "-Alt-Shift Down" TAB "GuiEditor.selectChildren( true );";
         item[13] = "Select..." TAB "" TAB "GuiEditorSelectDlg.toggleVisibility();";
         item[14] = "-";
         item[15] = "Lock/Unlock Selection" TAB %cmdCtrl SPC "L" TAB "GuiEditor.toggleLockSelection();";
         item[16] = "Hide/Unhide Selection" TAB %cmdCtrl SPC "H" TAB "GuiEditor.toggleHideSelection();";
         item[17] = "-";
         item[18] = "Group Selection" TAB %cmdCtrl SPC "G" TAB "GuiEditor.groupSelected();";
         item[19] = "Ungroup Selection" TAB %cmdCtrl @ "-Shift G" TAB "GuiEditor.ungroupSelected();";
         item[20] = "-";
         item[21] = "Full Box Selection" TAB "" TAB "GuiEditor.toggleFullBoxSelection();";
         item[22] = "-";
         item[23] = "Grid Size" TAB %cmdCtrl SPC "," TAB "GuiEditor.showPrefsDialog();";
      };
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "Layout";
         internalName = "LayoutMenu";
         
         item[0] = "Align Left" TAB %cmdCtrl SPC "Left" TAB "GuiEditor.Justify(0);";
         item[1] = "Center Horizontally" TAB "" TAB "GuiEditor.Justify(1);";
         item[2] = "Align Right" TAB %cmdCtrl SPC "Right" TAB "GuiEditor.Justify(2);";
         item[3] = "-";
         item[4] = "Align Top" TAB %cmdCtrl SPC "Up" TAB "GuiEditor.Justify(3);";
         item[5] = "Center Vertically" TAB "" TAB "GuiEditor.Justify(7);";
         item[6] = "Align Bottom" TAB %cmdCtrl SPC "Down" TAB "GuiEditor.Justify(4);";
         item[7] = "-";
         item[8] = "Space Vertically" TAB "" TAB "GuiEditor.Justify(5);";
         item[9] = "Space Horizontally" TAB "" TAB "GuiEditor.Justify(6);";
         item[10] = "-";
         item[11] = "Fit into Parent(s)" TAB "" TAB "GuiEditor.fitIntoParents();";
         item[12] = "Fit Width to Parent(s)" TAB "" TAB "GuiEditor.fitIntoParents( true, false );";
         item[13] = "Fit Height to Parent(s)" TAB "" TAB "GuiEditor.fitIntoParents( false, true );";
         item[14] = "-";
         item[15] = "Bring to Front" TAB "" TAB "GuiEditor.BringToFront();";
         item[16] = "Send to Back" TAB "" TAB "GuiEditor.PushToBack();";
      };
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "Move";
         internalName = "MoveMenu";
            
         item[0] = "Nudge Left" TAB "Left" TAB "GuiEditor.moveSelection( -1, 0);";
         item[1] = "Nudge Right" TAB "Right" TAB "GuiEditor.moveSelection( 1, 0);";
         item[2] = "Nudge Up" TAB "Up" TAB "GuiEditor.moveSelection( 0, -1);";
         item[3] = "Nudge Down" TAB "Down" TAB "GuiEditor.moveSelection( 0, 1 );";
         item[4] = "-";
         item[5] = "Big Nudge Left" TAB "Shift Left" TAB "GuiEditor.moveSelection( - GuiEditor.snap2gridsize, 0 );";
         item[6] = "Big Nudge Right" TAB "Shift Right" TAB "GuiEditor.moveSelection( GuiEditor.snap2gridsize, 0 );";
         item[7] = "Big Nudge Up" TAB "Shift Up" TAB "GuiEditor.moveSelection( 0, - GuiEditor.snap2gridsize );";
         item[8] = "Big Nudge Down" TAB "Shift Down" TAB "GuiEditor.moveSelection( 0, GuiEditor.snap2gridsize );";
      };

      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "Snap";
         internalName = "SnapMenu";

         item[0] = "Snap Edges" TAB "Alt-Shift E" TAB "GuiEditor.toggleEdgeSnap();";
         item[1] = "Snap Centers" TAB "Alt-Shift C" TAB "GuiEditor.toggleCenterSnap();";
         item[2] = "-";
         item[3] = "Snap to Guides" TAB "Alt-Shift G" TAB "GuiEditor.toggleGuideSnap();";
         item[4] = "Snap to Controls" TAB "Alt-Shift T" TAB "GuiEditor.toggleControlSnap();";
         item[5] = "Snap to Canvas" TAB "" TAB "GuiEditor.toggleCanvasSnap();";
         item[6] = "Snap to Grid" TAB "" TAB "GuiEditor.toggleGridSnap();";
         item[7] = "-";
         item[8] = "Show Guides" TAB "" TAB "GuiEditor.toggleDrawGuides();";
         item[9] = "Clear Guides" TAB "" TAB "GuiEditor.clearGuides();";
      };

      new PopupMenu()
      {
         superClass = "MenuBuilder";
         internalName = "HelpMenu";

         barTitle = "Help";

         item[0] = "Online Documentation..." TAB "Alt F1" TAB "gotoWebPage( GuiEditor.documentationURL );";
         item[1] = "Offline User Guid..." TAB "" TAB "gotoWebPage( GuiEditor.documentationLocal );";
         item[2] = "Offline Reference Guide..." TAB "" TAB "shellExecute( GuiEditor.documentationReference );";
         item[3] = "-";
         item[4] = "Torque 3D Public Forums..." TAB "" TAB "gotoWebPage( \"http://www.garagegames.com/community/forums/73\" );";
         item[5] = "Torque 3D Private Forums..." TAB "" TAB "gotoWebPage( \"http://www.garagegames.com/community/forums/63\" );";
      };
   };
   %this.menuBar.attachToCanvas( Canvas, 0 );
}

$GUI_EDITOR_MENU_EDGESNAP_INDEX = 0;
$GUI_EDITOR_MENU_CENTERSNAP_INDEX = 1;
$GUI_EDITOR_MENU_GUIDESNAP_INDEX = 3;
$GUI_EDITOR_MENU_CONTROLSNAP_INDEX = 4;
$GUI_EDITOR_MENU_CANVASSNAP_INDEX = 5;
$GUI_EDITOR_MENU_GRIDSNAP_INDEX = 6;
$GUI_EDITOR_MENU_DRAWGUIDES_INDEX = 8;
$GUI_EDITOR_MENU_FULLBOXSELECT_INDEX = 21;

//---------------------------------------------------------------------------------------------

/// Called before onSleep when the canvas content is changed
function GuiEditCanvas::onDestroyMenu(%this)
{
   if( !isObject( %this.menuBar ) )
      return;

   // Destroy menus      
   while( %this.menuBar.getCount() != 0 )
      %this.menuBar.getObject( 0 ).delete();
   
   %this.menuBar.removeFromCanvas();
   %this.menuBar.delete();
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::onWindowClose(%this)
{
   %this.quit();
}

//=============================================================================================
//    Menu Commands.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::create( %this )
{
   GuiEditorNewGuiDialog.init( "NewGui", "GuiControl" );
      
   Canvas.pushDialog( GuiEditorNewGuiDialog );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::load( %this, %filename )
{
   %newRedefineBehavior = "replaceExisting";
   if( isDefined( "$GuiEditor::loadRedefineBehavior" ) )
   {
      // This trick allows to choose different redefineBehaviors when loading
      // GUIs.  This is useful, for example, when loading GUIs that would lead to
      // problems when loading with their correct names because script behavior
      // would immediately attach.
      //
      // This allows to also edit the GUI editor's own GUI inside itself.
      
      %newRedefineBehavior = $GuiEditor::loadRedefineBehavior;
   }
   
   // Allow stomping objects while exec'ing the GUI file as we want to
   // pull the file's objects even if we have another version of the GUI
   // already loaded.
   
   %oldRedefineBehavior = $Con::redefineBehavior;
   $Con::redefineBehavior = %newRedefineBehavior;
   
   // Load up the gui.
   exec( %fileName );
   
   $Con::redefineBehavior = %oldRedefineBehavior;
   
   // The GUI file should have contained a GUIControl which should now be in the instant
   // group. And, it should be the only thing in the group.
   if( !isObject( %guiContent ) )
   {
      MessageBox( getEngineName(),
         "You have loaded a Gui file that was created before this version.  It has been loaded but you must open it manually from the content list dropdown",
         "Ok", "Information" );   
      return 0;
   }

   GuiEditor.openForEditing( %guiContent );
   
   GuiEditorStatusBar.print( "Loaded '" @ %filename @ "'" );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::openInTorsion( %this )
{
   if( !GuiEditorContent.getCount() )
      return;
      
   %guiObject = GuiEditorContent.getObject( 0 );
   EditorOpenDeclarationInTorsion( %guiObject );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::open( %this )
{
   %openFileName = GuiBuilder::getOpenName();
   if( %openFileName $= "" )
      return;

   // Make sure the file is valid.
   if ((!isFile(%openFileName)) && (!isFile(%openFileName @ ".dso")))
      return;

   %this.load( %openFileName );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::save( %this, %selectedOnly, %noPrompt )
{
   // Get the control we should save.
   
   if( %selectedOnly )
   {
      %selected = GuiEditor.getSelection();
      if( !%selected.getCount() )
         return;
      else if( %selected.getCount() > 1 )
      {
         MessageBox( "Invalid selection", "Only a single control hierarchy can be saved to a file.  Make sure you have selected only one control in the tree view." );
         return;
      }
         
      %currentObject = %selected.getObject( 0 );
   }
   else if( GuiEditorContent.getCount() > 0 )
      %currentObject = GuiEditorContent.getObject( 0 );
   else
      return;
      
   // Store the current guide set on the control.
   
   GuiEditor.writeGuides( %currentObject );
   %currentObject.canSaveDynamicFields = true; // Make sure the guides get saved out.
   
   // Construct a base filename.
   
   if( %currentObject.getName() !$= "" )
      %name =  %currentObject.getName() @ ".gui";
   else
      %name = "Untitled.gui";
      
   // Construct a path.
   
   if( %selectedOnly
       && %currentObject != GuiEditorContent.getObject( 0 )
       && %currentObject.getFileName() $= GuiEditorContent.getObject( 0 ).getFileName() )
   {
      // Selected child control that hasn't been yet saved to its own file.
      
      %currentFile = GuiEditor.LastPath @ "/" @ %name;
      %currentFile = makeRelativePath( %currentFile, getMainDotCsDir() );
   }
   else
   {
      %currentFile = %currentObject.getFileName();
      if( %currentFile $= "")
      {
         // No file name set on control.  Force a prompt.
         %noPrompt = false;
         
         if( GuiEditor.LastPath !$= "" )
         {
            %currentFile = GuiEditor.LastPath @ "/" @ %name;
            %currentFile = makeRelativePath( %currentFile, getMainDotCsDir() );
         }
         else
            %currentFile = expandFileName( %name );
      }
      else
         %currentFile = expandFileName( %currentFile );
   }
   
   // Get the filename.
   
   if( !%noPrompt )
   {
      %filename = GuiBuilder::getSaveName( %currentFile );
      if( %filename $= "" )
         return;
   }
   else
      %filename = %currentFile;
      
   // Save the Gui.
   
   if( isWriteableFileName( %filename ) )
   {
      //
      // Extract any existent TorqueScript before writing out to disk
      //
      %fileObject = new FileObject();
      %fileObject.openForRead( %filename );      
      %skipLines = true;
      %beforeObject = true;
      // %var++ does not post-increment %var, in torquescript, it pre-increments it,
      // because ++%var is illegal. 
      %lines = -1;
      %beforeLines = -1;
      %skipLines = false;
      while( !%fileObject.isEOF() )
      {
         %line = %fileObject.readLine();
         if( %line $= "//--- OBJECT WRITE BEGIN ---" )
            %skipLines = true;
         else if( %line $= "//--- OBJECT WRITE END ---" )
         {
            %skipLines = false;
            %beforeObject = false;
         }
         else if( %skipLines == false )
         {
            if(%beforeObject)
               %beforeNewFileLines[ %beforeLines++ ] = %line;
            else
               %newFileLines[ %lines++ ] = %line;
         }
      }      
      %fileObject.close();
      %fileObject.delete();
     
      %fo = new FileObject();
      %fo.openForWrite(%filename);
      
      // Write out the captured TorqueScript that was before the object before the object
      for( %i = 0; %i <= %beforeLines; %i++)
         %fo.writeLine( %beforeNewFileLines[ %i ] );
         
      %fo.writeLine("//--- OBJECT WRITE BEGIN ---");
      %fo.writeObject(%currentObject, "%guiContent = ");
      %fo.writeLine("//--- OBJECT WRITE END ---");
      
      // Write out captured TorqueScript below Gui object
      for( %i = 0; %i <= %lines; %i++ )
         %fo.writeLine( %newFileLines[ %i ] );
               
      %fo.close();
      %fo.delete();
      
      %currentObject.setFileName( makeRelativePath( %filename, getMainDotCsDir() ) );
      
      GuiEditorStatusBar.print( "Saved file '" @ %currentObject.getFileName() @ "'" );
   }
   else
      MessageBox( "Error writing to file", "There was an error writing to file '" @ %currentFile @ "'. The file may be read-only.", "Ok", "Error" );   
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::append( %this )
{
   // Get filename.
   
   %openFileName = GuiBuilder::getOpenName();
   if( %openFileName $= ""
       || ( !isFile( %openFileName )
            && !isFile( %openFileName @ ".dso" ) ) )
      return;
   
   // Exec file.

   %oldRedefineBehavior = $Con::redefineBehavior;
   $Con::redefineBehavior = "renameNew";
   exec( %openFileName );
   $Con::redefineBehavior = %oldRedefineBehavior;
   
   // Find guiContent.
   
   if( !isObject( %guiContent ) )
   {
      MessageBox( "Error loading GUI file", "The GUI content controls could not be found.  This function can only be used with files saved by the GUI editor.", "Ok", "Error" );
      return;
   }
   
   if( !GuiEditorContent.getCount() )
      GuiEditor.openForEditing( %guiContent );
   else
   {
      GuiEditor.getCurrentAddSet().add( %guiContent );
      GuiEditor.readGuides( %guiContent );
      GuiEditor.onAddNewCtrl( %guiContent );
      GuiEditor.onHierarchyChanged();
   }
   
   GuiEditorStatusBar.print( "Appended controls from '" @ %openFileName @ "'" );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::revert( %this )
{
   if( !GuiEditorContent.getCount() )
      return;
      
   %gui = GuiEditorContent.getObject( 0 );
   %filename = %gui.getFileName();
   if( %filename $= "" )
      return;
      
   if( MessageBox( "Revert Gui", "Really revert the current Gui?  This cannot be undone.", "OkCancel", "Question" ) == $MROk )
      %this.load( %filename );
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::close( %this )
{
}

//---------------------------------------------------------------------------------------------

function GuiEditCanvas::quit( %this )
{
   %this.close();
   GuiGroup.add(GuiEditorGui);
   // we must not delete a window while in its event handler, or we foul the event dispatch mechanism
   %this.schedule(10, delete);
   
   Canvas.setContent(GuiEditor.lastContent);
   $InGuiEditor = false;

   //Temp fix to disable MLAA when in GUI editor
   if( isObject(MLAAFx) && $MLAAFxGuiEditorTemp==true )
   {
	 MLAAFx.isEnabled = true;
         $MLAAFxGuiEditorTemp = false;
   }
}
