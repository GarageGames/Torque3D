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

// @todo:
//
// - split node transform editboxes into X Y Z and rot X Y Z with spin controls
//   to allow easier manual editing
// - add groundspeed editing ( use same format as node transform editing )
//
// Known bugs/limitations:
//
// - resizing the GuiTextListCtrl should resize the columns as well
// - modifying the from/in/out properties of a sequence will change the sequence
//   order in the shape ( since it results in remove/add sequence commands )
// - deleting a node should not delete its children as well?
//

//------------------------------------------------------------------------------
// Utility Methods
//------------------------------------------------------------------------------

if ( !isObject( ShapeEditor ) ) new ScriptObject( ShapeEditor )
{
   shape = -1;
   deletedCount = 0;
};


// Capitalise the first letter of the input string
function strcapitalise( %str )
{
   %len = strlen( %str );
   return strupr( getSubStr( %str,0,1 ) ) @ getSubStr( %str,1,%len-1 );
}

function ShapeEditor::getObjectShapeFile( %this, %obj )
{
   // Get the path to the shape file used by the given object (not perfect, but
   // works for the vast majority of object types)
   %path = "";
   if ( %obj.isMemberOfClass( "TSStatic" ) )
      %path = %obj.shapeName;
   else if ( %obj.isMemberOfClass( "PhysicsShape" ) )
      %path = %obj.getDataBlock().shapeName;
   else if ( %obj.isMemberOfClass( "GameBase" ) )
      %path = %obj.getDataBlock().shapeFile;
      
   return %path;
}

// Check if the given name already exists
function ShapeEditor::nameExists( %this, %type, %name )
{
   if ( ShapeEditor.shape == -1 )
      return false;

   if ( %type $= "node" )
      return ( ShapeEditor.shape.getNodeIndex( %name ) >= 0 );
   else if ( %type $= "sequence" )
      return ( ShapeEditor.shape.getSequenceIndex( %name ) >= 0 );
   else if ( %type $= "object" )
      return ( ShapeEditor.shape.getObjectIndex( %name ) >= 0 );
}

// Check if the given 'hint' name exists (spaces could also be underscores)
function ShapeEditor::hintNameExists( %this, %type, %name )
{
   if ( ShapeEditor.nameExists( %type, %name ) )
      return true;

   // If the name contains spaces, try replacing with underscores
   %name = strreplace( %name, " ", "_" );
   if ( ShapeEditor.nameExists( %type, %name ) )
      return true;

   return false;
}

// Generate a unique name from a given base by appending an integer
function ShapeEditor::getUniqueName( %this, %type, %name )
{
   for ( %idx = 1; %idx < 100; %idx++ )
   {
      %uniqueName = %name @ %idx;
      if ( !%this.nameExists( %type, %uniqueName ) )
         break;
   }

   return %uniqueName;
}

function ShapeEditor::getProxyName( %this, %seqName )
{
   return "__proxy__" @ %seqName;
}

function ShapeEditor::getUnproxyName( %this, %proxyName )
{
   return strreplace( %proxyName, "__proxy__", "" );
}

function ShapeEditor::getBackupName( %this, %seqName )
{
   return "__backup__" @ %seqName;
}

// Check if this mesh name is a collision hint
function ShapeEditor::isCollisionMesh( %this, %name )
{
   return ( startswith( %name, "ColBox" ) ||
            startswith( %name, "ColSphere" ) ||
            startswith( %name, "ColCapsule" ) ||
            startswith( %name, "ColConvex" ) );
}

// 
function ShapeEditor::getSequenceSource( %this, %seqName )
{
   %source = %this.shape.getSequenceSource( %seqName );

   // Use the sequence name as the source for DTS built-in sequences
   %src0 = getField( %source, 0 );
   %src1 = getField( %source, 1 );
   if ( %src0 $= %src1 )
      %source = setField( %source, 1, "" );
   if ( %src0 $= "" )
      %source = setField( %source, 0, %seqName );

   return %source;
}

// Recursively get names for a node and its children
function ShapeEditor::getNodeNames( %this, %nodeName, %names, %exclude )
{
   if ( %nodeName $= %exclude )
      return %names;

   %count = %this.shape.getNodeChildCount( %nodeName );
   for ( %i = 0; %i < %count; %i++ )
   {
      %childName = %this.shape.getNodeChildName( %nodeName, %i );
      %names = %this.getNodeNames( %childName, %names, %exclude );
   }

   %names = %names TAB %nodeName;

   return trim( %names );
}

// Get the list of meshes for a particular object
function ShapeEditor::getObjectMeshList( %this, %name )
{
   %list = "";
   %count = %this.shape.getMeshCount( %name );
   for ( %i = 0; %i < %count; %i++ )
      %list = %list TAB %this.shape.getMeshName( %name, %i );
   return trim( %list );
}

// Get the list of meshes for a particular detail level
function ShapeEditor::getDetailMeshList( %this, %detSize )
{
   %list = "";
   %objCount = ShapeEditor.shape.getObjectCount();
   for ( %i = 0; %i < %objCount; %i++ )
   {
      %objName = ShapeEditor.shape.getObjectName( %i );
      %meshCount = ShapeEditor.shape.getMeshCount( %objName );
      for ( %j = 0; %j < %meshCount; %j++ )
      {
         %size = ShapeEditor.shape.getMeshSize( %objName, %j );
         if ( %size == %detSize )
            %list = %list TAB %this.shape.getMeshName( %objName, %j );
      }
   }
   return trim( %list );
}

function ShapeEditor::isDirty( %this )
{
   return ( isObject( %this.shape ) && ShapeEdPropWindow-->saveBtn.isActive() );
}

function ShapeEditor::setDirty( %this, %dirty )
{
   if ( %dirty )
      ShapeEdSelectWindow.text = "Shapes *";
   else
      ShapeEdSelectWindow.text = "Shapes";

   ShapeEdPropWindow-->saveBtn.setActive( %dirty );
}

function ShapeEditor::saveChanges( %this )
{
   if ( isObject( ShapeEditor.shape ) )
   {
      ShapeEditor.saveConstructor( ShapeEditor.shape );
      ShapeEditor.shape.writeChangeSet();
      ShapeEditor.shape.notifyShapeChanged();      // Force game objects to reload shape
      ShapeEditor.setDirty( false );
   }
}

//------------------------------------------------------------------------------
// Shape Selection
//------------------------------------------------------------------------------

function ShapeEditor::findConstructor( %this, %path )
{
   %count = TSShapeConstructorGroup.getCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %obj = TSShapeConstructorGroup.getObject( %i );
      if ( %obj.baseShape $= %path )
         return %obj;
   }
   return -1;
}

function ShapeEditor::createConstructor( %this, %path )
{
   %name = strcapitalise( fileBase( %path ) ) @ strcapitalise( getSubStr( fileExt( %path ), 1, 3 ) );
   %name = strreplace( %name, "-", "_" );
   %name = strreplace( %name, ".", "_" );
   %name = getUniqueName( %name );
   return new TSShapeConstructor( %name ) { baseShape = %path; };
}

function ShapeEditor::saveConstructor( %this, %constructor )
{
   %savepath = filePath( %constructor.baseShape ) @ "/" @ fileBase( %constructor.baseShape ) @ ".cs";
   new PersistenceManager( shapeEd_perMan );
   shapeEd_perMan.setDirty( %constructor, %savepath );
   shapeEd_perMan.saveDirtyObject( %constructor );
   shapeEd_perMan.delete();
}

// Handle a selection in the shape selector list
function ShapeEdSelectWindow::onSelect( %this, %path )
{
   // Prompt user to save the old shape if it is dirty
   if ( ShapeEditor.isDirty() )
   {
      %cmd = "ColladaImportDlg.showDialog( \"" @ %path @ "\", \"ShapeEditor.selectShape( \\\"" @ %path @ "\\\", ";
      MessageBoxYesNoCancel( "Shape Modified", "Would you like to save your changes?", %cmd @ "true );\" );", %cmd @ "false );\" );" );
   }
   else
   {
      %cmd = "ShapeEditor.selectShape( \"" @ %path @ "\", false );";
      ColladaImportDlg.showDialog( %path, %cmd );
   }
}

function ShapeEditor::selectShape( %this, %path, %saveOld )
{
   ShapeEdShapeView.setModel( "" );

   if ( %saveOld )
   {
      // Save changes to a TSShapeConstructor script
      %this.saveChanges();
   }
   else if ( ShapeEditor.isDirty() )
   {
      // Purge all unsaved changes
      %oldPath = ShapeEditor.shape.baseShape;
      ShapeEditor.shape.delete();
      ShapeEditor.shape = 0;

      reloadResource( %oldPath );   // Force game objects to reload shape
   }

   // Initialise the shape preview window
   if ( !ShapeEdShapeView.setModel( %path ) )
   {
      MessageBoxOK( "Error", "Failed to load '" @ %path @ "'. Check the console for error messages." );
      return;
   }
   ShapeEdShapeView.fitToShape();

   ShapeEdUndoManager.clearAll();
   ShapeEditor.setDirty( false );

   // Get ( or create ) the TSShapeConstructor object for this shape
   ShapeEditor.shape = ShapeEditor.findConstructor( %path );
   if ( ShapeEditor.shape <= 0 )
   {
      ShapeEditor.shape = %this.createConstructor( %path );
      if ( ShapeEditor.shape <= 0 )
      {
         error( "ShapeEditor: Error - could not select " @ %path );
         return;
      }
   }

   // Initialise the editor windows
   ShapeEdAdvancedWindow.update_onShapeSelectionChanged();
   ShapeEdMountWindow.update_onShapeSelectionChanged();
   ShapeEdThreadWindow.update_onShapeSelectionChanged();
   ShapeEdColWindow.update_onShapeSelectionChanged();
   ShapeEdPropWindow.update_onShapeSelectionChanged();
   ShapeEdShapeView.refreshShape();

   // Update object type hints
   ShapeEdSelectWindow.updateHints();

   // Update editor status bar
   EditorGuiStatusBar.setSelection( %path );
}

// Handle a selection in the MissionGroup shape selector
function ShapeEdShapeTreeView::onSelect( %this, %obj )
{
   %path = ShapeEditor.getObjectShapeFile( %obj );
   if ( %path !$= "" )
      ShapeEdSelectWindow.onSelect( %path );

   // Set the object type (for required nodes and sequences display)
   %objClass = %obj.getClassName();
   %hintId = -1;

   %count = ShapeHintGroup.getCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %hint = ShapeHintGroup.getObject( %i );
      if ( %objClass $= %hint.objectType )
      {
         %hintId = %hint;
         break;
      }
      else if ( isMemberOfClass( %objClass, %hint.objectType ) )
      {
         %hintId = %hint;
      }
   }
   ShapeEdHintMenu.setSelected( %hintId );
}

// Find all DTS or COLLADA models. Note: most of this section was shamelessly
// stolen from creater.ed.cs => great work whoever did the original!
function ShapeEdSelectWindow::navigate( %this, %address )
{
   // Freeze the icon array so it doesn't update until we've added all of the
   // icons
   %this-->shapeLibrary.frozen = true;
   %this-->shapeLibrary.clear();
   ShapeEdSelectMenu.clear();

   %filePatterns = getFormatExtensions();
   %fullPath = findFirstFileMultiExpr( %filePatterns );

   while ( %fullPath !$= "" )
   {
      // Ignore cached DTS files
      if ( endswith( %fullPath, "cached.dts" ) )
      {
         %fullPath = findNextFileMultiExpr( %filePatterns );
         continue;
      }

      // Ignore assets in the tools folder
      %fullPath = makeRelativePath( %fullPath, getMainDotCSDir() );
      %splitPath = strreplace( %fullPath, " ", "_" );
      %splitPath = strreplace( %splitPath, "/", " " );
      if ( getWord( %splitPath, 0 ) $= "tools" )
      {
         %fullPath = findNextFileMultiExpr( %filePatterns );
         continue;
      }

      %dirCount = getWordCount( %splitPath ) - 1;
      %pathFolders = getWords( %splitPath, 0, %dirCount - 1 );

      // Add this file's path ( parent folders ) to the
      // popup menu if it isn't there yet.
      %temp = strreplace( %pathFolders, " ", "/" );
      %temp = strreplace( %temp, "_", " " );
      %r = ShapeEdSelectMenu.findText( %temp );
      if ( %r == -1 )
         ShapeEdSelectMenu.add( %temp );

      // Is this file in the current folder?
      if ( stricmp( %pathFolders, %address ) == 0 )
      {
         %this.addShapeIcon( %fullPath );
      }
      // Then is this file in a subfolder we need to add
      // a folder icon for?
      else
      {
         %wordIdx = 0;
         %add = false;

         if ( %address $= "" )
         {
            %add = true;
            %wordIdx = 0;
         }
         else
         {
            for ( ; %wordIdx < %dirCount; %wordIdx++ )
            {
               %temp = getWords( %splitPath, 0, %wordIdx );
               if ( stricmp( %temp, %address ) == 0 )
               {
                  %add = true;
                  %wordIdx++;
                  break;
               }
            }
         }

         if ( %add == true )
         {
            %folder = getWord( %splitPath, %wordIdx );

            // Add folder icon if not already present
            %ctrl = %this.findIconCtrl( %folder );
            if ( %ctrl == -1 )
               %this.addFolderIcon( %folder );
         }
      }

      %fullPath = findNextFileMultiExpr( %filePatterns );
   }

   %this-->shapeLibrary.sort( "alphaIconCompare" );
   for ( %i = 0; %i < %this-->shapeLibrary.getCount(); %i++ )
      %this-->shapeLibrary.getObject( %i ).autoSize = false;

   %this-->shapeLibrary.frozen = false;
   %this-->shapeLibrary.refresh();
   %this.address = %address;

   ShapeEdSelectMenu.sort();

   %str = strreplace( %address, " ", "/" );
   %r = ShapeEdSelectMenu.findText( %str );
   if ( %r != -1 )
      ShapeEdSelectMenu.setSelected( %r, false );
   else
      ShapeEdSelectMenu.setText( %str );
}

function ShapeEdSelectWindow::navigateDown( %this, %folder )
{
   if ( %this.address $= "" )
      %address = %folder;
   else
      %address = %this.address SPC %folder;

   // Because this is called from an IconButton::onClick command
   // we have to wait a tick before actually calling navigate, else
   // we would delete the button out from under itself.
   %this.schedule( 1, "navigate", %address );
}

function ShapeEdSelectWindow::navigateUp( %this )
{
   %count = getWordCount( %this.address );

   if ( %count == 0 )
      return;

   if ( %count == 1 )
      %address = "";
   else
      %address = getWords( %this.address, 0, %count - 2 );

   %this.navigate( %address );
}

function ShapeEdSelectWindow::findIconCtrl( %this, %name )
{
   for ( %i = 0; %i < %this-->shapeLibrary.getCount(); %i++ )
   {
      %ctrl = %this-->shapeLibrary.getObject( %i );
      if ( %ctrl.text $= %name )
         return %ctrl;
   }
   return -1;
}

function ShapeEdSelectWindow::createIcon( %this )
{
   %ctrl = new GuiIconButtonCtrl()
   {
      profile = "GuiCreatorIconButtonProfile";
      iconLocation = "Left";
      textLocation = "Right";
      extent = "348 19";
      textMargin = 8;
      buttonMargin = "2 2";
      autoSize = false;
      sizeIconToButton = true;
      makeIconSquare = true;
      buttonType = "radioButton";
      groupNum = "-1";   
   };

   return %ctrl;
}

function ShapeEdSelectWindow::addFolderIcon( %this, %text )
{
   %ctrl = %this.createIcon();

   %ctrl.altCommand = "ShapeEdSelectWindow.navigateDown( \"" @ %text @ "\" );";
   %ctrl.iconBitmap = "tools/gui/images/folder.png";
   %ctrl.text = %text;
   %ctrl.tooltip = %text;
   %ctrl.class = "CreatorFolderIconBtn";
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";
   
   %this-->shapeLibrary.addGuiControl( %ctrl );
}

function ShapeEdSelectWindow::addShapeIcon( %this, %fullPath )
{
   %ctrl = %this.createIcon();

   %ext = fileExt( %fullPath );
   %file = fileBase( %fullPath );
   %fileLong = %file @ %ext;
   %tip = %fileLong NL
          "Size: " @ fileSize( %fullPath ) / 1000.0 SPC "KB" NL
          "Date Created: " @ fileCreatedTime( %fullPath ) NL
          "Last Modified: " @ fileModifiedTime( %fullPath );

   %ctrl.altCommand = "ShapeEdSelectWindow.onSelect( \"" @ %fullPath @ "\" );";
   %ctrl.iconBitmap = ( ( %ext $= ".dts" ) ? EditorIconRegistry::findIconByClassName( "TSStatic" ) : "tools/gui/images/iconCollada" );
   %ctrl.text = %file;
   %ctrl.class = "CreatorStaticIconBtn";
   %ctrl.tooltip = %tip;
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";

   // Check if a shape specific icon is available
   %formats = ".png .jpg .dds .bmp .gif .jng .tga";
   %count = getWordCount( %formats );
   for ( %i = 0; %i < %count; %i++ )
   {
      %ext = getWord( %formats, %i );
      if ( isFile( %fullPath @ %ext ) )
      {
         %ctrl.iconBitmap = %fullPath @ %ext;
         break;
      }
   }

   %this-->shapeLibrary.addGuiControl( %ctrl );
}

function ShapeEdSelectMenu::onSelect( %this, %id, %text )
{
   %split = strreplace( %text, "/", " " );
   ShapeEdSelectWindow.navigate( %split );
}

// Update the GUI in response to the shape selection changing
function ShapeEdPropWindow::update_onShapeSelectionChanged( %this )
{
   // --- NODES TAB ---
   ShapeEdNodeTreeView.removeItem( 0 );
   %rootId = ShapeEdNodeTreeView.insertItem( 0, "<root>", 0, "" );
   %count = ShapeEditor.shape.getNodeCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %name = ShapeEditor.shape.getNodeName( %i );
      if ( ShapeEditor.shape.getNodeParentName( %name ) $= "" )
         ShapeEdNodeTreeView.addNodeTree( %name );
   }
   %this.update_onNodeSelectionChanged( -1 );    // no node selected

   // --- SEQUENCES TAB ---
   ShapeEdSequenceList.clear();
   ShapeEdSequenceList.addRow( -1, "Name" TAB "Cyclic" TAB "Blend" TAB "Frames" TAB "Priority" );
   ShapeEdSequenceList.setRowActive( -1, false );
   ShapeEdSequenceList.addRow( 0, "<rootpose>" TAB "" TAB "" TAB "" TAB "" );

   %count = ShapeEditor.shape.getSequenceCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %name = ShapeEditor.shape.getSequenceName( %i );

      // Ignore __backup__ sequences (only used by editor)
      if ( !startswith( %name, "__backup__" ) )
         ShapeEdSequenceList.addItem( %name );
   }
   ShapeEdThreadWindow.onAddThread();        // add thread 0

   // --- DETAILS TAB ---
   // Add detail levels and meshes to tree
   ShapeEdDetailTree.clearSelection();
   ShapeEdDetailTree.removeItem( 0 );
   %root = ShapeEdDetailTree.insertItem( 0, "<root>", "", "" );
   %objCount = ShapeEditor.shape.getObjectCount();
   for ( %i = 0; %i < %objCount; %i++ )
   {
      %objName = ShapeEditor.shape.getObjectName( %i );
      %meshCount = ShapeEditor.shape.getMeshCount( %objName );
      for ( %j = 0; %j < %meshCount; %j++ )
      {
         %meshName = ShapeEditor.shape.getMeshName( %objName, %j );
         ShapeEdDetailTree.addMeshEntry( %meshName, 1 );
      }
   }

   // Initialise object node list
   ShapeEdDetails-->objectNode.clear();
   ShapeEdDetails-->objectNode.add( "<root>" );
   %nodeCount = ShapeEditor.shape.getNodeCount();
   for ( %i = 0; %i < %nodeCount; %i++ )
      ShapeEdDetails-->objectNode.add( ShapeEditor.shape.getNodeName( %i ) );

   // --- MATERIALS TAB ---
   ShapeEdMaterials.updateMaterialList();
}

//------------------------------------------------------------------------------
// Shape Hints
//------------------------------------------------------------------------------

function ShapeEdHintMenu::onSelect( %this, %id, %text )
{
   ShapeEdSelectWindow.updateHints();
}

function ShapeEdSelectWindow::updateHints( %this )
{
   %objectType = ShapeEdHintMenu.getText();

   ShapeEdSelectWindow-->nodeHints.freeze( true );
   ShapeEdSelectWindow-->sequenceHints.freeze( true );

   // Move all current hint controls to a holder SimGroup
   for ( %i = ShapeEdSelectWindow-->nodeHints.getCount()-1; %i >= 0; %i-- )
      ShapeHintControls.add( ShapeEdSelectWindow-->nodeHints.getObject( %i ) );
   for ( %i = ShapeEdSelectWindow-->sequenceHints.getCount()-1; %i >= 0; %i-- )
      ShapeHintControls.add( ShapeEdSelectWindow-->sequenceHints.getObject( %i ) );

   // Update node and sequence hints, modifying and/or creating gui controls as needed
   for ( %i = 0; %i < ShapeHintGroup.getCount(); %i++ )
   {
      %hint = ShapeHintGroup.getObject( %i );
      if ( ( %objectType $= %hint.objectType ) || isMemberOfClass( %objectType, %hint.objectType ) )
      {
         for ( %idx = 0; %hint.node[%idx] !$= ""; %idx++ )
            ShapeEdHintMenu.processHint( "node", %hint.node[%idx] );

         for ( %idx = 0; %hint.sequence[%idx] !$= ""; %idx++ )
            ShapeEdHintMenu.processHint( "sequence", %hint.sequence[%idx] );
      }
   }

   ShapeEdSelectWindow-->nodeHints.freeze( false );
   ShapeEdSelectWindow-->nodeHints.updateStack();
   ShapeEdSelectWindow-->sequenceHints.freeze( false );
   ShapeEdSelectWindow-->sequenceHints.updateStack();

}

function ShapeEdHintMenu::processHint( %this, %type, %hint )
{
   %name = getField( %hint, 0 );
   %desc = getField( %hint, 1 );

   // check for arrayed names (ending in 0-N or 1-N)
   %pos = strstr( %name, "0-" );
   if ( %pos == -1 )
      %pos = strstr( %name, "1-" );

   if ( %pos > 0 )
   {
      // arrayed name => add controls for each name in the array, but collapse
      // consecutive indices where possible. eg.  if the model only has nodes
      // mount1-3, we should create: mount0 (red), mount1-3 (green), mount4-31 (red)
      %base = getSubStr( %name, 0, %pos );      // array name
      %first = getSubStr( %name, %pos, 1 );     // first index
      %last = getSubStr( %name, %pos+2, 3 );    // last index

      // get the state of the first element
      %arrayStart = %first;
      %prevPresent = ShapeEditor.hintNameExists( %type, %base @ %first );

      for ( %j = %first + 1; %j <= %last; %j++ )
      {
         // if the state of this element is different to the previous one, we
         // need to add a hint
         %present = ShapeEditor.hintNameExists( %type, %base @ %j );
         if ( %present != %prevPresent )
         {
            ShapeEdSelectWindow.addObjectHint( %type, %base, %desc, %prevPresent, %arrayStart, %j-1 );
            %arrayStart = %j;
            %prevPresent = %present;
         }
      }

      // add hint for the last group
      ShapeEdSelectWindow.addObjectHint( %type, %base, %desc, %prevPresent, %arrayStart, %last );
   }
   else
   {
      // non-arrayed name
      %present = ShapeEditor.hintNameExists( %type, %name );
      ShapeEdSelectWindow.addObjectHint( %type, %name, %desc, %present );
   }
}

function ShapeEdSelectWindow::addObjectHint( %this, %type, %name, %desc, %present, %start, %end )
{
   // Get a hint gui control (create one if needed)
   if ( ShapeHintControls.getCount() == 0 )
   {
      // Create a new hint gui control
      %ctrl = new GuiIconButtonCtrl()
      {
         profile = "GuiCreatorIconButtonProfile";
         iconLocation = "Left";
         textLocation = "Right";
         extent = "348 19";
         textMargin = 8;
         buttonMargin = "2 2";
         autoSize = true;
         buttonType = "radioButton";
         groupNum = "-1";
         iconBitmap = "tools/editorClasses/gui/images/iconCancel";
         text = "hint";
         tooltip = "";
      };

      ShapeHintControls.add( %ctrl );
   }
   %ctrl = ShapeHintControls.getObject( 0 );

   // Initialise the control, then add it to the appropriate list
   %name = %name @ %start;
   if ( %end !$= %start )
      %ctrl.text = %name @ "-" @ %end;
   else
      %ctrl.text = %name;

   %ctrl.tooltip = %desc;
   %ctrl.setBitmap( "tools/editorClasses/gui/images/" @ ( %present ? "iconAccept" : "iconCancel" ) );
   %ctrl.setStateOn( false );
   %ctrl.resetState();

   switch$ ( %type )
   {
      case "node":
         %ctrl.altCommand = %present ? "" : "ShapeEdNodes.onAddNode( \"" @ %name @ "\" );";
         ShapeEdSelectWindow-->nodeHints.addGuiControl( %ctrl );
      case "sequence":
         %ctrl.altCommand = %present ? "" : "ShapeEdSequences.onAddSequence( \"" @ %name @ "\" );";
         ShapeEdSelectWindow-->sequenceHints.addGuiControl( %ctrl );
   }
}

//------------------------------------------------------------------------------

function ShapeEdSeqNodeTabBook::onTabSelected( %this, %name, %index )
{
   %this.activePage = %name;

   switch$ ( %name )
   {
      case "Seq":
         ShapeEdPropWindow-->newBtn.ToolTip = "Add new sequence";
         ShapeEdPropWindow-->newBtn.Command = "ShapeEdSequences.onAddSequence();";
         ShapeEdPropWindow-->newBtn.setActive( true );
         ShapeEdPropWindow-->deleteBtn.ToolTip = "Delete selected sequence (cannot be undone)";
         ShapeEdPropWindow-->deleteBtn.Command = "ShapeEdSequences.onDeleteSequence();";
         ShapeEdPropWindow-->deleteBtn.setActive( true );

      case "Node":
         ShapeEdPropWindow-->newBtn.ToolTip = "Add new node";
         ShapeEdPropWindow-->newBtn.Command = "ShapeEdNodes.onAddNode();";
         ShapeEdPropWindow-->newBtn.setActive( true );
         ShapeEdPropWindow-->deleteBtn.ToolTip = "Delete selected node (cannot be undone)";
         ShapeEdPropWindow-->deleteBtn.Command = "ShapeEdNodes.onDeleteNode();";
         ShapeEdPropWindow-->deleteBtn.setActive( true );

      case "Detail":
         ShapeEdPropWindow-->newBtn.ToolTip = "";
         ShapeEdPropWindow-->newBtn.Command = "";
         ShapeEdPropWindow-->newBtn.setActive( false );
         ShapeEdPropWindow-->deleteBtn.ToolTip = "Delete the selected mesh or detail level (cannot be undone)";
         ShapeEdPropWindow-->deleteBtn.Command = "ShapeEdDetails.onDeleteMesh();";
         ShapeEdPropWindow-->deleteBtn.setActive( true );

      case "Mat":
         ShapeEdPropWindow-->newBtn.ToolTip = "";
         ShapeEdPropWindow-->newBtn.Command = "";
         ShapeEdPropWindow-->newBtn.setActive( false );
         ShapeEdPropWindow-->deleteBtn.ToolTip = "";
         ShapeEdPropWindow-->deleteBtn.Command = "";
         ShapeEdPropWindow-->deleteBtn.setActive( false );

         // For some reason, the header is not resized correctly until the Materials tab has been
         // displayed at least once, so resize it here too
         ShapeEdMaterials-->materialListHeader.setExtent( getWord( ShapeEdMaterialList.extent, 0 ) SPC "19" );
   }
}

//------------------------------------------------------------------------------
// Node Editing
//------------------------------------------------------------------------------

// Update the GUI in response to the node selection changing
function ShapeEdPropWindow::update_onNodeSelectionChanged( %this, %id )
{
   if ( %id > 0 )
   {
      // Enable delete button and edit boxes
      if ( ShapeEdSeqNodeTabBook.activePage $= "Node" )
         ShapeEdPropWindow-->deleteBtn.setActive( true );
      ShapeEdNodes-->nodeName.setActive( true );
      ShapeEdNodes-->nodePosition.setActive( true );
      ShapeEdNodes-->nodeRotation.setActive( true );

      // Update the node inspection data
      %name = ShapeEdNodeTreeView.getItemText( %id );

      ShapeEdNodes-->nodeName.setText( %name );

      // Node parent list => ancestor and sibling nodes only (can't re-parent to a descendent)
      ShapeEdNodeParentMenu.clear();
      %parentNames = ShapeEditor.getNodeNames( "", "<root>", %name );
      %count = getWordCount( %parentNames );
      for ( %i = 0; %i < %count; %i++ )
         ShapeEdNodeParentMenu.add( getWord(%parentNames, %i), %i );

      %pName = ShapeEditor.shape.getNodeParentName( %name );
      if ( %pName $= "" )
         %pName = "<root>";
      ShapeEdNodeParentMenu.setText( %pName );

      if ( ShapeEdNodes-->worldTransform.getValue() )
      {
         // Global transform
         %txfm = ShapeEditor.shape.getNodeTransform( %name, 1 );
         ShapeEdNodes-->nodePosition.setText( getWords( %txfm, 0, 2 ) );
         ShapeEdNodes-->nodeRotation.setText( getWords( %txfm, 3, 6 ) );
      }
      else
      {
         // Local transform (relative to parent)
         %txfm = ShapeEditor.shape.getNodeTransform( %name, 0 );
         ShapeEdNodes-->nodePosition.setText( getWords( %txfm, 0, 2 ) );
         ShapeEdNodes-->nodeRotation.setText( getWords( %txfm, 3, 6 ) );
      }

      ShapeEdShapeView.selectedNode = ShapeEditor.shape.getNodeIndex( %name );
   }
   else
   {
      // Disable delete button and edit boxes
      if ( ShapeEdSeqNodeTabBook.activePage $= "Node" ) 
         ShapeEdPropWindow-->deleteBtn.setActive( false );
      ShapeEdNodes-->nodeName.setActive( false );
      ShapeEdNodes-->nodePosition.setActive( false );
      ShapeEdNodes-->nodeRotation.setActive( false );

      ShapeEdNodes-->nodeName.setText( "" );
      ShapeEdNodes-->nodePosition.setText( "" );
      ShapeEdNodes-->nodeRotation.setText( "" );

      ShapeEdShapeView.selectedNode = -1;
   }
}

// Update the GUI in response to a node being added
function ShapeEdPropWindow::update_onNodeAdded( %this, %nodeName, %oldTreeIndex )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();
   ShapeEdShapeView.updateNodeTransforms();
   ShapeEdSelectWindow.updateHints();

   // --- MOUNT WINDOW ---
   if ( ShapeEdMountWindow.isMountableNode( %nodeName ) )
   {
      ShapeEdMountWindow-->mountNode.add( %nodeName );
      ShapeEdMountWindow-->mountNode.sort();
   }

   // --- NODES TAB ---
   %id = ShapeEdNodeTreeView.addNodeTree( %nodeName );
   if ( %oldTreeIndex <= 0 )
   {
      // This is a new node => make it the current selection
      if ( %id > 0 )
      {
         ShapeEdNodeTreeView.clearSelection();
         ShapeEdNodeTreeView.selectItem( %id );
      }
   }
   else
   {
      // This node has been un-deleted. Inserting a new item puts it at the
      // end of the siblings, but we want to restore the original order as
      // if the item was never deleted, so move it up as required.
      %childIndex = ShapeEdNodeTreeView.getChildIndexByName( %nodeName );
      while ( %childIndex > %oldTreeIndex )
      {
         ShapeEdNodeTreeView.moveItemUp( %id );
         %childIndex--;
      }
   }

   // --- DETAILS TAB ---
   ShapeEdDetails-->objectNode.add( %nodeName );
}

// Update the GUI in response to a node(s) being removed
function ShapeEdPropWindow::update_onNodeRemoved( %this, %nameList, %nameCount )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();
   ShapeEdShapeView.updateNodeTransforms();
   ShapeEdSelectWindow.updateHints();

   // Remove nodes from the mountable list, and any shapes mounted to the node
   for ( %i = 0; %i < %nameCount; %i++ )
   {
      %nodeName = getField( %nameList, %i );
      ShapeEdMountWindow-->mountNode.clearEntry( ShapeEdMountWindow-->mountNode.findText( %nodeName ) );

      for ( %j = ShapeEdMountWindow-->mountList.rowCount()-1; %j >= 1; %j-- )
      {
         %text = ShapeEdMountWindow-->mountList.getRowText( %j );
         if ( getField( %text, 1 ) $= %nodeName )
         {
            ShapeEdShapeView.unmountShape( %j-1 );
            ShapeEdMountWindow-->mountList.removeRow( %j );
         }
      }
   }

   // --- NODES TAB ---
   %lastName = getField( %nameList, %nameCount-1 );
   %id = ShapeEdNodeTreeView.findItemByName( %lastName );   // only need to remove the parent item
   if ( %id > 0 )
   {
      ShapeEdNodeTreeView.removeItem( %id );
      if ( ShapeEdNodeTreeView.getSelectedItem() <= 0 )
         ShapeEdPropWindow.update_onNodeSelectionChanged( -1 );
   }

   // --- DETAILS TAB ---
   for ( %i = 0; %i < %nameCount; %i++ )
   {
      %nodeName = getField( %nameList, %i );
      ShapeEdDetails-->objectNode.clearEntry( ShapeEdDetails-->objectNode.findText( %nodeName ) );
   }
}

// Update the GUI in response to a node being renamed
function ShapeEdPropWindow::update_onNodeRenamed( %this, %oldName, %newName )
{
   // --- MISC ---
   ShapeEdSelectWindow.updateHints();

   // --- MOUNT WINDOW ---
   // Update entries for any shapes mounted to this node
   %rowCount = ShapeEdMountWindow-->mountList.rowCount();
   for ( %i = 1; %i < %rowCount; %i++ )
   {
      %text = ShapeEdMountWindow-->mountList.getRowText( %i );
      if ( getField( %text, 1 ) $= %oldName )
      {
         %text = setField( %text, 1, %newName );
         ShapeEdMountWindow-->mountList.setRowById( ShapeEdMountWindow-->mountList.getRowId( %i ), %text );
      }
   }

   // Update list of mountable nodes
   ShapeEdMountWindow-->mountNode.clearEntry( ShapeEdMountWindow-->mountNode.findText( %oldName ) );
   if ( ShapeEdMountWindow.isMountableNode( %newName ) )
   {
      ShapeEdMountWindow-->mountNode.add( %newName );
      ShapeEdMountWindow-->mountNode.sort();
   }

   // --- NODES TAB ---
   %id = ShapeEdNodeTreeView.findItemByName( %oldName );
   ShapeEdNodeTreeView.editItem( %id, %newName, 0 );
   if ( ShapeEdNodeTreeView.getSelectedItem() == %id )
      ShapeEdNodes-->nodeName.setText( %newName );

   // --- DETAILS TAB ---
   %id = ShapeEdDetails-->objectNode.findText( %oldName );
   if ( %id != -1 )
   {
      ShapeEdDetails-->objectNode.clearEntry( %id );
      ShapeEdDetails-->objectNode.add( %newName, %id );
      ShapeEdDetails-->objectNode.sortID();
      if ( ShapeEdDetails-->objectNode.getText() $= %oldName )
         ShapeEdDetails-->objectNode.setText( %newName );
   }
}

// Update the GUI in response to a node's parent being changed
function ShapeEdPropWindow::update_onNodeParentChanged( %this, %nodeName )
{
   // --- MISC ---
   ShapeEdShapeView.updateNodeTransforms();

   // --- NODES TAB ---
   %isSelected = 0;
   %id = ShapeEdNodeTreeView.findItemByName( %nodeName );
   if ( %id > 0 )
   {
      %isSelected = ( ShapeEdNodeTreeView.getSelectedItem() == %id );
      ShapeEdNodeTreeView.removeItem( %id );
   }
   ShapeEdNodeTreeView.addNodeTree( %nodeName );
   if ( %isSelected )
      ShapeEdNodeTreeView.selectItem( ShapeEdNodeTreeView.findItemByName( %nodeName ) );
}

function ShapeEdPropWindow::update_onNodeTransformChanged( %this, %nodeName )
{
   // Default to the selected node if none is specified
   if ( %nodeName $= "" )
   {
      %id = ShapeEdNodeTreeView.getSelectedItem();
      if ( %id > 0 )
         %nodeName = ShapeEdNodeTreeView.getItemText( %id );
      else
         return;
   }

   // --- MISC ---
   ShapeEdShapeView.updateNodeTransforms();
   if ( ShapeEdNodes-->objectTransform.getValue() )
      GlobalGizmoProfile.setFieldValue(alignment, Object);
   else
      GlobalGizmoProfile.setFieldValue(alignment, World);

   // --- NODES TAB ---
   // Update the node transform fields if necessary
   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( ( %id > 0 ) && ( ShapeEdNodeTreeView.getItemText( %id ) $= %nodeName ) )
   {
      %isWorld = ShapeEdNodes-->worldTransform.getValue();
      %transform = ShapeEditor.shape.getNodeTransform( %nodeName, %isWorld );
      ShapeEdNodes-->nodePosition.setText( getWords( %transform, 0, 2 ) );
      ShapeEdNodes-->nodeRotation.setText( getWords( %transform, 3, 6 ) );
   }
}

function ShapeEdNodeTreeView::onClearSelection( %this )
{
   ShapeEdPropWindow.update_onNodeSelectionChanged( -1 );
}

function ShapeEdNodeTreeView::onSelect( %this, %id )
{
   // Update the node name and transform controls
   ShapeEdPropWindow.update_onNodeSelectionChanged( %id );

   // Update orbit position if orbiting the selected node
   if ( ShapeEdShapeView.orbitNode )
   {
      %name = %this.getItemText( %id );
      %transform = ShapeEditor.shape.getNodeTransform( %name, 1 );
      ShapeEdShapeView.setOrbitPos( getWords( %transform, 0, 2 ) );
   }
}

function ShapeEdShapeView::onNodeSelected( %this, %index )
{
   ShapeEdNodeTreeView.clearSelection();
   if ( %index > 0 )
   {
      %name = ShapeEditor.shape.getNodeName( %index );
      %id = ShapeEdNodeTreeView.findItemByName( %name );
      if ( %id > 0 )
         ShapeEdNodeTreeView.selectItem( %id );
   }
}

function ShapeEdNodes::onAddNode( %this, %name )
{
   // Add a new node, using the currently selected node as the initial parent
   if ( %name $= "" )
      %name = ShapeEditor.getUniqueName( "node", "myNode" );

   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( %id <= 0 )
      %parent = "";
   else
      %parent = ShapeEdNodeTreeView.getItemText( %id );

   ShapeEditor.doAddNode( %name, %parent, "0 0 0 0 0 1 0" );
}

function ShapeEdNodes::onDeleteNode( %this )
{
   // Remove the node and all its children from the shape
   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( %id > 0 )
   {
      %name = ShapeEdNodeTreeView.getItemText( %id );
      ShapeEditor.doRemoveShapeData( "Node", %name );
   }
}

// Determine the index of a node in the tree relative to its parent
function ShapeEdNodeTreeView::getChildIndexByName( %this, %name )
{
   %id = %this.findItemByName( %name );
   %parentId = %this.getParentItem( %id );
   %childId = %this.getChild( %parentId );
   if ( %childId <= 0 )
      return 0;   // bad!

   %index = 0;
   while ( %childId != %id )
   {
      %childId = %this.getNextSibling( %childId );
      %index++;
   }

   return %index;
}

// Add a node and its children to the node tree view
function ShapeEdNodeTreeView::addNodeTree( %this, %nodeName )
{
   // Abort if already added => something dodgy has happened and we'd end up
   // recursing indefinitely
   if ( %this.findItemByName( %nodeName ) )
   {
      error( "Recursion error in ShapeEdNodeTreeView::addNodeTree" );
      return 0;
   }

   // Find parent and add me to it
   %parentName = ShapeEditor.shape.getNodeParentName( %nodeName );
   if ( %parentName $= "" )
      %parentName = "<root>";

   %parentId = %this.findItemByName( %parentName );
   %id = %this.insertItem( %parentId, %nodeName, 0, "" );

   // Add children
   %count = ShapeEditor.shape.getNodeChildCount( %nodeName );
   for ( %i = 0; %i < %count; %i++ )
      %this.addNodeTree( ShapeEditor.shape.getNodeChildName( %nodeName, %i ) );

   return %id;
}

function ShapeEdNodes::onEditName( %this )
{
   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( %id > 0 )
   {
      %oldName = ShapeEdNodeTreeView.getItemText( %id );
      %newName = %this-->nodeName.getText();
      if ( %newName !$= "" )
         ShapeEditor.doRenameNode( %oldName, %newName );
   }
}

function ShapeEdNodeParentMenu::onSelect( %this, %id, %text )
{
   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( %id > 0 )
   {
      %name = ShapeEdNodeTreeView.getItemText( %id );
      ShapeEditor.doSetNodeParent( %name, %text );
   }
}

function ShapeEdNodes::onEditTransform( %this )
{
   %id = ShapeEdNodeTreeView.getSelectedItem();
   if ( %id > 0 )
   {
      %name = ShapeEdNodeTreeView.getItemText( %id );

      // Get the node transform from the gui
      %pos = %this-->nodePosition.getText();
      %rot = %this-->nodeRotation.getText();
      %txfm = %pos SPC %rot;
      %isWorld = ShapeEdNodes-->worldTransform.getValue();

      // Do a quick sanity check to avoid setting wildly invalid transforms
      for ( %i = 0; %i < 7; %i++ )    // "x y z aa.x aa.y aa.z aa.angle"
      {
         if ( getWord( %txfm, %i ) $= "" )
            return;
      }

      ShapeEditor.doEditNodeTransform( %name, %txfm, %isWorld, -1 );
   }
}

function ShapeEdShapeView::onEditNodeTransform( %this, %node, %txfm, %gizmoID )
{
   ShapeEditor.doEditNodeTransform( %node, %txfm, 1, %gizmoID );
}

//------------------------------------------------------------------------------
// Sequence Editing
//------------------------------------------------------------------------------

function ShapeEdPropWindow::onWake( %this )
{
   ShapeEdTriggerList.triggerId = 1;

   ShapeEdTriggerList.addRow( -1, "-1" TAB "Frame" TAB "Trigger" TAB "State" );
   ShapeEdTriggerList.setRowActive( -1, false );
}

function ShapeEdPropWindow::update_onSeqSelectionChanged( %this )
{
   // Sync the Thread window sequence selection
   %row = ShapeEdSequenceList.getSelectedRow();
   if ( ShapeEdThreadWindow-->seqList.getSelectedRow() != ( %row-1 ) )
   {
      ShapeEdThreadWindow-->seqList.setSelectedRow( %row-1 );
      return;  // selecting a sequence in the Thread window will re-call this function
   }

   ShapeEdSeqFromMenu.clear();
   ShapeEdSequences-->blendSeq.clear();

   // Clear the trigger list
   ShapeEdTriggerList.removeAll();

   // Update the active sequence data
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      // Enable delete button and edit boxes
      if ( ShapeEdSeqNodeTabBook.activePage $= "Seq" ) 
         ShapeEdPropWindow-->deleteBtn.setActive( true );
      ShapeEdSequences-->seqName.setActive( true );
      ShapeEdSequences-->blendFlag.setActive( true );
      ShapeEdSequences-->cyclicFlag.setActive( true );
      ShapeEdSequences-->priority.setActive( true );
      ShapeEdSequences-->addTriggerBtn.setActive( true );
      ShapeEdSequences-->deleteTriggerBtn.setActive( true );

      // Initialise the sequence properties
      %blendData = ShapeEditor.shape.getSequenceBlend( %seqName );
      ShapeEdSequences-->seqName.setText( %seqName );
      ShapeEdSequences-->cyclicFlag.setValue( ShapeEditor.shape.getSequenceCyclic( %seqName ) );
      ShapeEdSequences-->blendFlag.setValue( getField( %blendData, 0 ) );
      ShapeEdSequences-->priority.setText( ShapeEditor.shape.getSequencePriority( %seqName ) );

      // 'From' and 'Blend' sequence menus
      ShapeEdSeqFromMenu.add( "Browse..." );
      %count = ShapeEdSequenceList.rowCount();
      for ( %i = 2; %i < %count; %i++ )  // skip header row and <rootpose>
      {
         %name = ShapeEdSequenceList.getItemName( %i );
         if ( %name !$= %seqName )
         {
            ShapeEdSeqFromMenu.add( %name );
            ShapeEdSequences-->blendSeq.add( %name );
         }
      }
      ShapeEdSequences-->blendSeq.setText( getField( %blendData, 1 ) );
      ShapeEdSequences-->blendFrame.setText( getField( %blendData, 2 ) );

      %this.syncPlaybackDetails();

      // Triggers (must occur after syncPlaybackDetails is called so the slider range is correct)
      %count = ShapeEditor.shape.getTriggerCount( %seqName );
      for ( %i = 0; %i < %count; %i++ )
      {
         %trigger = ShapeEditor.shape.getTrigger( %seqName, %i );
         ShapeEdTriggerList.addItem( getWord( %trigger, 0 ), getWord( %trigger, 1 ) );
      }
   }
   else
   {
      // Disable delete button and edit boxes
      if ( ShapeEdSeqNodeTabBook.activePage $= "Seq" ) 
         ShapeEdPropWindow-->deleteBtn.setActive( false );
      ShapeEdSequences-->seqName.setActive( false );
      ShapeEdSequences-->blendFlag.setActive( false );
      ShapeEdSequences-->cyclicFlag.setActive( false );
      ShapeEdSequences-->priority.setActive( false );
      ShapeEdSequences-->addTriggerBtn.setActive( false );
      ShapeEdSequences-->deleteTriggerBtn.setActive( false );

      // Clear sequence properties
      ShapeEdSequences-->seqName.setText( "" );
      ShapeEdSequences-->cyclicFlag.setValue( 0 );
      ShapeEdSequences-->blendSeq.setText( "" );
      ShapeEdSequences-->blendFlag.setValue( 0 );
      ShapeEdSequences-->priority.setText( 0 );

      %this.syncPlaybackDetails();
   }

   %this.onTriggerSelectionChanged();

   ShapeEdSequences-->sequenceListHeader.setExtent( getWord( ShapeEdSequenceList.extent, 0 ) SPC "19" );

   // Reset current frame
   //ShapeEdAnimWindow.setKeyframe( ShapeEdAnimWindow-->seqIn.getText() );
}

// Update the GUI in response to a sequence being added
function ShapeEdPropWindow::update_onSequenceAdded( %this, %seqName, %oldIndex )
{
   // --- MISC ---
   ShapeEdSelectWindow.updateHints();

   // --- SEQUENCES TAB ---
   if ( %oldIndex == -1 )
   {
      // This is a brand new sequence => add it to the list and make it the
      // current selection
      %row = ShapeEdSequenceList.insertItem( %seqName, ShapeEdSequenceList.rowCount() );
      ShapeEdSequenceList.scrollVisible( %row );
      ShapeEdSequenceList.setSelectedRow( %row );
   }
   else
   {
      // This sequence has been un-deleted => add it back to the list at the
      // original position
      ShapeEdSequenceList.insertItem( %seqName, %oldIndex );
   }
}

function ShapeEdPropWindow::update_onSequenceRemoved( %this, %seqName )
{
   // --- MISC ---
   ShapeEdSelectWindow.updateHints();

   // --- SEQUENCES TAB ---
   %isSelected = ( ShapeEdSequenceList.getSelectedName() $= %seqName );
   ShapeEdSequenceList.removeItem( %seqName );
   if ( %isSelected )
      ShapeEdPropWindow.update_onSeqSelectionChanged();

   // --- THREADS WINDOW ---
   ShapeEdShapeView.refreshThreadSequences();
}

function ShapeEdPropWindow::update_onSequenceRenamed( %this, %oldName, %newName )
{
   // --- MISC ---
   ShapeEdSelectWindow.updateHints();

   // Rename the proxy sequence as well
   %oldProxy = ShapeEditor.getProxyName( %oldName );
   %newProxy = ShapeEditor.getProxyName( %newName );
   if ( ShapeEditor.shape.getSequenceIndex( %oldProxy ) != -1 )
      ShapeEditor.shape.renameSequence( %oldProxy, %newProxy );

   // --- SEQUENCES TAB ---
   ShapeEdSequenceList.editColumn( %oldName, 0, %newName );
   if ( ShapeEdSequenceList.getSelectedName() $= %newName )
      ShapeEdSequences-->seqName.setText( %newName );

   // --- THREADS WINDOW ---
   // Update any threads that use this sequence
   %active = ShapeEdShapeView.activeThread;
   for ( %i = 0; %i < ShapeEdShapeView.getThreadCount(); %i++ )
   {
      ShapeEdShapeView.activeThread = %i;
      if ( ShapeEdShapeView.getThreadSequence() $= %oldName )
         ShapeEdShapeView.setThreadSequence( %newName, 0, ShapeEdShapeView.threadPos, 0 );
      else if ( ShapeEdShapeView.getThreadSequence() $= %oldProxy )
         ShapeEdShapeView.setThreadSequence( %newProxy, 0, ShapeEdShapeView.threadPos, 0 );
   }
   ShapeEdShapeView.activeThread = %active;
}

function ShapeEdPropWindow::update_onSequenceCyclicChanged( %this, %seqName, %cyclic )
{
   // --- MISC ---
   // Apply the same transformation to the proxy animation if necessary
   %proxyName = ShapeEditor.getProxyName( %seqName );
   if ( ShapeEditor.shape.getSequenceIndex( %proxyName ) != -1 )
      ShapeEditor.shape.setSequenceCyclic( %proxyName, %cyclic );

   // --- SEQUENCES TAB ---
   ShapeEdSequenceList.editColumn( %seqName, 1, %cyclic ? "yes" : "no" );
   if ( ShapeEdSequenceList.getSelectedName() $= %seqName )
      ShapeEdSequences-->cyclicFlag.setStateOn( %cyclic );
}

function ShapeEdPropWindow::update_onSequenceBlendChanged( %this, %seqName, %blend,
                              %oldBlendSeq, %oldBlendFrame, %blendSeq, %blendFrame )
{
   // --- MISC ---
   // Apply the same transformation to the proxy animation if necessary
   %proxyName = ShapeEditor.getProxyName( %seqName );
   if ( ShapeEditor.shape.getSequenceIndex( %proxyName ) != -1 )
   {
      if ( %blend && %oldBlend )
         ShapeEditor.shape.setSequenceBlend( %proxyName, false, %oldBlendSeq, %oldBlendFrame );
      ShapeEditor.shape.setSequenceBlend( %proxyName, %blend, %blendSeq, %blendFrame );
   }
   ShapeEdShapeView.updateNodeTransforms();

   // --- SEQUENCES TAB ---
   ShapeEdSequenceList.editColumn( %seqName, 2, %blend ? "yes" : "no" );
   if ( ShapeEdSequenceList.getSelectedName() $= %seqName )
   {
      ShapeEdSequences-->blendFlag.setStateOn( %blend );
      ShapeEdSequences-->blendSeq.setText( %blendSeq );
      ShapeEdSequences-->blendFrame.setText( %blendFrame );
   }
}

function ShapeEdPropWindow::update_onSequencePriorityChanged( %this, %seqName )
{
   // --- SEQUENCES TAB ---
   %priority = ShapeEditor.shape.getSequencePriority( %seqName );
   ShapeEdSequenceList.editColumn( %seqName, 4, %priority );
   if ( ShapeEdSequenceList.getSelectedName() $= %seqName )
      ShapeEdSequences-->priority.setText( %priority );
}

function ShapeEdPropWindow::update_onSequenceGroundSpeedChanged( %this, %seqName )
{
   // nothing to do yet
}

function ShapeEdPropWindow::syncPlaybackDetails( %this )
{
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      // Show sequence in/out bars
      ShapeEdAnimWindow-->seqInBar.setVisible( true );
      ShapeEdAnimWindow-->seqOutBar.setVisible( true );

      // Sync playback controls
      %sourceData = ShapeEditor.getSequenceSource( %seqName );
      %seqFrom = rtrim( getFields( %sourceData, 0, 1 ) );
      %seqStart = getField( %sourceData, 2 );
      %seqEnd = getField( %sourceData, 3 );
      %seqFromTotal = getField( %sourceData, 4 );

      // Display the original source for edited sequences
      if ( startswith( %seqFrom, "__backup__" ) )
      {
         %backupData = ShapeEditor.getSequenceSource( getField( %seqFrom, 0 ) );
         %seqFrom = rtrim( getFields( %backupData, 0, 1 ) );
      }

      ShapeEdSeqFromMenu.setText( %seqFrom );
      ShapeEdSeqFromMenu.tooltip = ShapeEdSeqFromMenu.getText();   // use tooltip to show long names
      ShapeEdSequences-->startFrame.setText( %seqStart );
      ShapeEdSequences-->endFrame.setText( %seqEnd );

      %val = ShapeEdSeqSlider.getValue() / getWord( ShapeEdSeqSlider.range, 1 );
      ShapeEdSeqSlider.range = "0" SPC ( %seqFromTotal-1 );
      ShapeEdSeqSlider.setValue( %val * getWord( ShapeEdSeqSlider.range, 1 ) );
      ShapeEdThreadSlider.range = ShapeEdSeqSlider.range;
      ShapeEdThreadSlider.setValue( ShapeEdSeqSlider.value );

      ShapeEdAnimWindow.setSequence( %seqName );
      ShapeEdAnimWindow.setPlaybackLimit( "in", %seqStart );
      ShapeEdAnimWindow.setPlaybackLimit( "out", %seqEnd );
   }
   else
   {
      // Hide sequence in/out bars
      ShapeEdAnimWindow-->seqInBar.setVisible( false );
      ShapeEdAnimWindow-->seqOutBar.setVisible( false );

      ShapeEdSeqFromMenu.setText( "" );
      ShapeEdSeqFromMenu.tooltip = "";
      ShapeEdSequences-->startFrame.setText( 0 );
      ShapeEdSequences-->endFrame.setText( 0 );

      ShapeEdSeqSlider.range = "0 1";
      ShapeEdSeqSlider.setValue( 0 );
      ShapeEdThreadSlider.range = ShapeEdSeqSlider.range;
      ShapeEdThreadSlider.setValue( ShapeEdSeqSlider.value );
      ShapeEdAnimWindow.setPlaybackLimit( "in", 0 );
      ShapeEdAnimWindow.setPlaybackLimit( "out", 1 );
      ShapeEdAnimWindow.setSequence( "" );
   }
}

function ShapeEdSequences::onEditSeqInOut( %this, %type, %val )
{
   %frameCount = getWord( ShapeEdSeqSlider.range, 1 );

   // Force value to a frame index within the slider range
   %val = mRound( %val );
   if ( %val < 0 )
      %val = 0;
   if ( %val > %frameCount )
      %val = %frameCount;

   // Enforce 'in' value must be < 'out' value
   if ( %type $= "in" )
   {
      if ( %val >= %this-->endFrame.getText() )
         %val = %this-->endFrame.getText() - 1;
      %this-->startFrame.setText( %val );
   }
   else
   {
      if ( %val <= %this-->startFrame.getText() )
         %val = %this-->startFrame.getText() + 1;
      %this-->endFrame.setText( %val );
   }

   %this.onEditSequenceSource( "" );
}

function ShapeEdSequences::onEditSequenceSource( %this, %from )
{
   // ignore for shapes without sequences
   if (ShapeEditor.shape.getSequenceCount() == 0)
      return;

   %start = %this-->startFrame.getText();
   %end = %this-->endFrame.getText();

   if ( ( %start !$= "" ) && ( %end !$= "" ) )
   {
      %seqName = ShapeEdSequenceList.getSelectedName();
      %oldSource = ShapeEditor.getSequenceSource( %seqName );

      if ( %from $= "" )
         %from = rtrim( getFields( %oldSource, 0, 0 ) );

      if ( getFields( %oldSource, 0, 3 ) !$= ( %from TAB "" TAB %start TAB %end ) )
         ShapeEditor.doEditSeqSource( %seqName, %from, %start, %end );
   }
}

function ShapeEdSequences::onToggleCyclic( %this )
{
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      %cyclic = %this-->cyclicFlag.getValue();
      ShapeEditor.doEditCyclic( %seqName, %cyclic );
   }
}

function ShapeEdSequences::onEditPriority( %this )
{
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      %newPriority = %this-->priority.getText();
      if ( %newPriority !$= "" )
         ShapeEditor.doEditSequencePriority( %seqName, %newPriority );
   }
}

function ShapeEdSequences::onEditBlend( %this )
{
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      // Get the blend flags (current and new)
      %oldBlendData = ShapeEditor.shape.getSequenceBlend( %seqName );
      %oldBlend = getField( %oldBlendData, 0 );
      %blend = %this-->blendFlag.getValue();

      // Ignore changes to the blend reference for non-blend sequences
      if ( !%oldBlend && !%blend )
         return;

      // OK - we're trying to change the blend properties of this sequence. The
      // new reference sequence and frame must be set.
      %blendSeq = %this-->blendSeq.getText();
      %blendFrame = %this-->blendFrame.getText();
      if ( ( %blendSeq $= "" ) || ( %blendFrame $= "" ) )
      {
         MessageBoxOK( "Blend reference not set", "The blend reference sequence and " @
            "frame must be set before changing the blend flag or frame." );
         ShapeEdSequences-->blendFlag.setStateOn( %oldBlend );
         return;
      }

      // Get the current blend properties (use new values if not specified)
      %oldBlendSeq = getField( %oldBlendData, 1 );
      if ( %oldBlendSeq $= "" )
         %oldBlendSeq = %blendSeq;
      %oldBlendFrame = getField( %oldBlendData, 2 );
      if ( %oldBlendFrame $= "" )
         %oldBlendFrame = %blendFrame;

      // Check if there is anything to do
      if ( ( %oldBlend TAB %oldBlendSeq TAB %oldBlendFrame ) !$= ( %blend TAB %blendSeq TAB %blendFrame ) )
         ShapeEditor.doEditBlend( %seqName, %blend, %blendSeq, %blendFrame );
   }
}

function ShapeEdSequences::onAddSequence( %this, %name )
{
   if ( %name $= "" )
      %name = ShapeEditor.getUniqueName( "sequence", "mySequence" );

   // Use the currently selected sequence as the base
   %from = ShapeEdSequenceList.getSelectedName();
   %row = ShapeEdSequenceList.getSelectedRow();
   if ( ( %row < 2 ) && ( ShapeEdSequenceList.rowCount() > 2 ) )
      %row = 2;
   if ( %from $= "" )
   {
      // No sequence selected => open dialog to browse for one
      getLoadFormatFilename( %this @ ".onAddSequenceFromBrowse", ShapeEdFromMenu.lastPath );
      return;
   }
   else
   {
      // Add the new sequence
      %start = ShapeEdSequences-->startFrame.getText();
      %end = ShapeEdSequences-->endFrame.getText();
      ShapeEditor.doAddSequence( %name, %from, %start, %end );
   }
}

function ShapeEdSequences::onAddSequenceFromBrowse( %this, %path )
{
   // Add a new sequence from the browse path
   %path = makeRelativePath( %path, getMainDotCSDir() );
   ShapeEdFromMenu.lastPath = %path;

   %name = ShapeEditor.getUniqueName( "sequence", "mySequence" );
   ShapeEditor.doAddSequence( %name, %path, 0, -1 );
}

// Delete the selected sequence
function ShapeEdSequences::onDeleteSequence( %this )
{
   %row = ShapeEdSequenceList.getSelectedRow();
   if ( %row != -1 )
   {
      %seqName = ShapeEdSequenceList.getItemName( %row );
      ShapeEditor.doRemoveShapeData( "Sequence", %seqName );
   }
}

// Get the name of the currently selected sequence
function ShapeEdSequenceList::getSelectedName( %this )
{
   %row = %this.getSelectedRow();
   return ( %row > 1 ) ? %this.getItemName( %row ) : "";    // ignore header row
}

// Get the sequence name from the indexed row
function ShapeEdSequenceList::getItemName( %this, %row )
{
   return getField( %this.getRowText( %row ), 0 );
}

// Get the index in the list of the sequence with the given name
function ShapeEdSequenceList::getItemIndex( %this, %name )
{
   for ( %i = 1; %i < %this.rowCount(); %i++ )  // ignore header row
   {
      if ( %this.getItemName( %i ) $= %name )
         return %i;
   }
   return -1;
}

// Change one of the fields in the sequence list
function ShapeEdSequenceList::editColumn( %this, %name, %col, %text )
{
   %row = %this.getItemIndex( %name );
   %rowText = setField( %this.getRowText( %row ), %col, %text );

   // Update the Properties and Thread sequence lists
   %id = %this.getRowId( %row );
   if ( %col == 0 )
      ShapeEdThreadWindow-->seqList.setRowById( %id, %text );   // Sync name in Thread window
   %this.setRowById( %id, %rowText );
}

function ShapeEdSequenceList::addItem( %this, %name )
{
   return %this.insertItem( %name, %this.rowCount() );
}

function ShapeEdSequenceList::insertItem( %this, %name, %index )
{
   %cyclic = ShapeEditor.shape.getSequenceCyclic( %name ) ? "yes" : "no";
   %blend = getField( ShapeEditor.shape.getSequenceBlend( %name ), 0 ) ? "yes" : "no";
   %frameCount = ShapeEditor.shape.getSequenceFrameCount( %name );
   %priority = ShapeEditor.shape.getSequencePriority( %name );

   // Add the item to the Properties and Thread sequence lists
   %this.seqId++; // use this to keep the row IDs synchronised
   ShapeEdThreadWindow-->seqList.addRow( %this.seqId, %name, %index-1 );   // no header row
   return %this.addRow( %this.seqId, %name TAB %cyclic TAB %blend TAB %frameCount TAB %priority, %index );
}

function ShapeEdSequenceList::removeItem( %this, %name )
{
   %index = %this.getItemIndex( %name );
   if ( %index >= 0 )
   {
      %this.removeRow( %index );
      ShapeEdThreadWindow-->seqList.removeRow( %index-1 );   // no header row
   }
}

function ShapeEdSeqFromMenu::onSelect( %this, %id, %text )
{
   if ( %text $= "Browse..." )
   {
      // Reset menu text
      %seqName = ShapeEdSequenceList.getSelectedName();
      %seqFrom = rtrim( getFields( ShapeEditor.getSequenceSource( %seqName ), 0, 1 ) );
      %this.setText( %seqFrom );

      // Allow the user to browse for an external source of animation data
      getLoadFormatFilename( %this @ ".onBrowseSelect", %this.lastPath );
   }
   else
   {
      ShapeEdSequences.onEditSequenceSource( %text );
   }
}

function ShapeEdSeqFromMenu::onBrowseSelect( %this, %path )
{
   %path = makeRelativePath( %path, getMainDotCSDir() );
   %this.lastPath = %path;
   %this.setText( %path );
   ShapeEdSequences.onEditSequenceSource( %path );
}

//------------------------------------------------------------------------------
// Threads and Animation
//------------------------------------------------------------------------------

function ShapeEdThreadWindow::onWake( %this )
{
   %this-->useTransitions.setValue( 1 );
   %this-->transitionTime.setText( "0.5" );

   %this-->transitionTo.clear();
   %this-->transitionTo.add( "synched position", 0 );
   %this-->transitionTo.add( "slider position", 1 );
   %this-->transitionTo.setSelected( 0 );

   %this-->transitionTarget.clear();
   %this-->transitionTarget.add( "plays during transition", 0 );
   %this-->transitionTarget.add( "pauses during transition", 1 );
   %this-->transitionTarget.setSelected( 0 );
}

// Update the GUI in response to the shape selection changing
function ShapeEdThreadWindow::update_onShapeSelectionChanged( %this )
{
   ShapeEdThreadList.clear();
   %this-->seqList.clear();
   %this-->seqList.addRow( 0, "<rootpose>" );
}

function ShapeEdAnimWIndow::threadPosToKeyframe( %this, %pos )
{
   if ( %this.usingProxySeq )
   {
      %start = getWord( ShapeEdSeqSlider.range, 0 );
      %end = getWord( ShapeEdSeqSlider.range, 1 );
   }
   else
   {
      %start = ShapeEdAnimWindow.seqStartFrame;
      %end = ShapeEdAnimWindow.seqEndFrame;
   }

   return %start + ( %end - %start ) * %pos;
}

function ShapeEdAnimWindow::keyframeToThreadPos( %this, %frame )
{
   if ( %this.usingProxySeq )
   {
      %start = getWord( ShapeEdSeqSlider.range, 0 );
      %end = getWord( ShapeEdSeqSlider.range, 1 );
   }
   else
   {
      %start = ShapeEdAnimWindow.seqStartFrame;
      %end = ShapeEdAnimWindow.seqEndFrame;
   }

   return ( %frame - %start ) / ( %end - %start );
}

function ShapeEdAnimWindow::setKeyframe( %this, %frame )
{
   ShapeEdSeqSlider.setValue( %frame );
   if ( ShapeEdThreadWindow-->transitionTo.getText() $= "synched position" )
      ShapeEdThreadSlider.setValue( %frame );

   // Update the position of the active thread => if outside the in/out range,
   // need to switch to the proxy sequence
   if ( !%this.usingProxySeq )
   {
      if ( ( %frame < %this.seqStartFrame ) || ( %frame > %this.seqEndFrame) )
      {
         %this.usingProxySeq = true;
         %proxyName = ShapeEditor.getProxyName( ShapeEdShapeView.getThreadSequence() );
         ShapeEdShapeView.setThreadSequence( %proxyName, 0, 0, false );
      }
   }

   ShapeEdShapeView.threadPos = %this.keyframeToThreadPos( %frame );
}

function ShapeEdAnimWindow::setNoProxySequence( %this )
{
   // no need to use the proxy sequence during playback
   if ( %this.usingProxySeq )
   {
      %this.usingProxySeq = false;
      %seqName = ShapeEditor.getUnproxyName( ShapeEdShapeView.getThreadSequence() );
      ShapeEdShapeView.setThreadSequence( %seqName, 0, 0, false );
      ShapeEdShapeView.threadPos = %this.keyframeToThreadPos( ShapeEdSeqSlider.getValue() );
   }
}

function ShapeEdAnimWindow::togglePause( %this )
{
   if ( %this-->pauseBtn.getValue() == 0 )
   {
      %this.lastDirBkwd = %this-->playBkwdBtn.getValue();
      %this-->pauseBtn.performClick();
   }
   else
   {
      %this.setNoProxySequence();
      if ( %this.lastDirBkwd )
         %this-->playBkwdBtn.performClick();
      else
         %this-->playFwdBtn.performClick();
   }
}

function ShapeEdAnimWindow::togglePingPong( %this )
{
   ShapeEdShapeView.threadPingPong = %this-->pingpong.getValue();
   if ( %this-->playFwdBtn.getValue() )
      %this-->playFwdBtn.performClick();
   else if ( %this-->playBkwdBtn.getValue() )
      %this-->playBkwdBtn.performClick();
}

function ShapeEdSeqSlider::onMouseDragged( %this )
{
   // Pause the active thread when the slider is dragged
   if ( ShapeEdAnimWindow-->pauseBtn.getValue() == 0 )
      ShapeEdAnimWindow-->pauseBtn.performClick();

   ShapeEdAnimWindow.setKeyframe( %this.getValue() );
}

function ShapeEdThreadSlider::onMouseDragged( %this )
{
   if ( ShapeEdThreadWindow-->transitionTo.getText() $= "synched position" )
   {
      // Pause the active thread when the slider is dragged
      if ( ShapeEdAnimWindow-->pauseBtn.getValue() == 0 )
         ShapeEdAnimWindow-->pauseBtn.performClick();

      ShapeEdAnimWindow.setKeyframe( %this.getValue() );
   }
}

function ShapeEdShapeView::onThreadPosChanged( %this, %pos, %inTransition )
{
   // Update sliders
   %frame = ShapeEdAnimWindow.threadPosToKeyframe( %pos );
   ShapeEdSeqSlider.setValue( %frame );

   if ( ShapeEdThreadWindow-->transitionTo.getText() $= "synched position" )
   {
      ShapeEdThreadSlider.setValue( %frame );

      // Highlight the slider during transitions
      if ( %inTransition )
         ShapeEdThreadSlider.profile = GuiShapeEdTransitionSliderProfile;
      else
         ShapeEdThreadSlider.profile = ToolsGuiSliderProfile;
   }
}

// Set the direction of the current thread (-1: reverse, 0: paused, 1: forward)
function ShapeEdAnimWindow::setThreadDirection( %this, %dir )
{
   // Update thread direction
   ShapeEdShapeView.threadDirection = %dir;

   // Sync the controls in the thread window
   switch ( %dir )
   {
      case -1: ShapeEdThreadWindow-->playBkwdBtn.setStateOn( 1 );
      case 0:  ShapeEdThreadWindow-->pauseBtn.setStateOn( 1 );
      case 1:  ShapeEdThreadWindow-->playFwdBtn.setStateOn( 1 );
   }
}

// Set the sequence to play
function ShapeEdAnimWindow::setSequence( %this, %seqName )
{
   %this.usingProxySeq = false;

   if ( ShapeEdThreadWindow-->useTransitions.getValue() )
   {
      %transTime = ShapeEdThreadWindow-->transitionTime.getText();
      if ( ShapeEdThreadWindow-->transitionTo.getText() $= "synched position" )
         %transPos = -1;
      else
         %transPos = %this.keyframeToThreadPos( ShapeEdThreadSlider.getValue() );
      %transPlay = ( ShapeEdThreadWindow-->transitionTarget.getText() $= "plays during transition" );
   }
   else
   {
      %transTime = 0;
      %transPos = 0;
      %transPlay = 0;
   }

   // No transition when sequence is not changing
   if ( %seqName $= ShapeEdShapeView.getThreadSequence() )
      %transTime = 0;

   if ( %seqName !$= "" )
   {
      // To be able to effectively scrub through the animation, we need to have all
      // frames available, even if it was added with only a subset. If that is the
      // case, then create a proxy sequence that has all the frames instead.
      %sourceData = ShapeEditor.getSequenceSource( %seqName );
      %from = rtrim( getFields( %sourceData, 0, 1 ) );
      %startFrame = getField( %sourceData, 2 );
      %endFrame = getField( %sourceData, 3 );
      %frameCount = getField( %sourceData, 4 );

      if ( ( %startFrame != 0 ) || ( %endFrame != ( %frameCount-1 ) ) )
      {
         %proxyName = ShapeEditor.getProxyName( %seqName );
         if ( ShapeEditor.shape.getSequenceIndex( %proxyName ) != -1 )
         {
            ShapeEditor.shape.removeSequence( %proxyName );
            ShapeEdShapeView.refreshThreadSequences();
         }
         ShapeEditor.shape.addSequence( %from, %proxyName );

         // Limit the transition position to the in/out range
         %transPos = mClamp( %transPos, 0, 1 );
      }
   }

   ShapeEdShapeView.setThreadSequence( %seqName, %transTime, %transPos, %transPlay );
}

function ShapeEdAnimWindow::getTimelineBitmapPos( %this, %val, %width )
{
   %frameCount = getWord( ShapeEdSeqSlider.range, 1 );
   %pos_x = getWord( ShapeEdSeqSlider.getPosition(), 0 );
   %len_x = getWord( ShapeEdSeqSlider.getExtent(), 0 ) - %width;
   return %pos_x + ( ( %len_x * %val / %frameCount ) );
}

// Set the in or out sequence limit
function ShapeEdAnimWindow::setPlaybackLimit( %this, %limit, %val )
{
   // Determine where to place the in/out bar on the slider
   %thumbWidth = 8;    // width of the thumb bitmap
   %pos_x = %this.getTimelineBitmapPos( %val, %thumbWidth );

   if ( %limit $= "in" )
   {
      %this.seqStartFrame = %val;
      %this-->seqIn.setText( %val );
      %this-->seqInBar.setPosition( %pos_x, 0 );
   }
   else
   {
      %this.seqEndFrame = %val;
      %this-->seqOut.setText( %val );
      %this-->seqOutBar.setPosition( %pos_x, 0 );
   }
}

function ShapeEdThreadWindow::onAddThread( %this )
{
   ShapeEdShapeView.addThread();
   ShapeEdThreadList.addRow( %this.threadID++, ShapeEdThreadList.rowCount() );
   ShapeEdThreadList.setSelectedRow( ShapeEdThreadList.rowCount()-1 );
}

function ShapeEdThreadWindow::onRemoveThread( %this )
{
   if ( ShapeEdThreadList.rowCount() > 1 )
   {
      // Remove the selected thread
      %row = ShapeEdThreadList.getSelectedRow();
      ShapeEdShapeView.removeThread( %row );
      ShapeEdThreadList.removeRow( %row );

      // Update list (threads are always numbered 0-N)
      %rowCount = ShapeEdThreadList.rowCount();
      for ( %i = %row; %i < %rowCount; %i++ )
         ShapeEdThreadList.setRowById( ShapeEdThreadList.getRowId( %i ), %i );

      // Select the next thread
      if ( %row >= %rowCount )
         %row = %rowCount - 1;

      ShapeEdThreadList.setSelectedRow( %row );
   }
}

function ShapeEdThreadList::onSelect( %this, %row, %text )
{
   ShapeEdShapeView.activeThread = ShapeEdThreadList.getSelectedRow();

   // Select the active thread's sequence in the list
   %seqName = ShapeEdShapeView.getThreadSequence();
   if ( %seqName $= "" )
      %seqName = "<rootpose>";
   else if ( startswith( %seqName, "__proxy__" ) )
      %seqName = ShapeEditor.getUnproxyName( %seqName );

   %seqIndex = ShapeEdSequenceList.getItemIndex( %seqName );
   ShapeEdSequenceList.setSelectedRow( %seqIndex );

   // Update the playback controls
   switch ( ShapeEdShapeView.threadDirection )
   {
      case -1: ShapeEdAnimWindow-->playBkwdBtn.performClick();
      case 0:  ShapeEdAnimWindow-->pauseBtn.performClick();
      case 1:  ShapeEdAnimWindow-->playFwdBtn.performClick();
   }
   SetToggleButtonValue( ShapeEdAnimWindow-->pingpong, ShapeEdShapeView.threadPingPong );
}

//------------------------------------------------------------------------------
// Trigger Editing
//------------------------------------------------------------------------------

function ShapeEdPropWindow::onTriggerSelectionChanged( %this )
{
   %row = ShapeEdTriggerList.getSelectedRow();
   if ( %row > 0 )  // skip header row
   {
      %text = ShapeEdTriggerList.getRowText( %row );

      ShapeEdSequences-->triggerFrame.setActive( true );
      ShapeEdSequences-->triggerNum.setActive( true );
      ShapeEdSequences-->triggerOnOff.setActive( true );

      ShapeEdSequences-->triggerFrame.setText( getField( %text, 1 ) );
      ShapeEdSequences-->triggerNum.setText( getField( %text, 2 ) );
      ShapeEdSequences-->triggerOnOff.setValue( getField( %text, 3 ) $= "on" );
   }
   else
   {
      // No trigger selected
      ShapeEdSequences-->triggerFrame.setActive( false );
      ShapeEdSequences-->triggerNum.setActive( false );
      ShapeEdSequences-->triggerOnOff.setActive( false );

      ShapeEdSequences-->triggerFrame.setText( "" );
      ShapeEdSequences-->triggerNum.setText( "" );
      ShapeEdSequences-->triggerOnOff.setValue( 0 );
   }
}

function ShapeEdSequences::onEditName( %this )
{
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      %newName = %this-->seqName.getText();
      if ( %newName !$= "" )
         ShapeEditor.doRenameSequence( %seqName, %newName );
   }
}

function ShapeEdPropWindow::update_onTriggerAdded( %this, %seqName, %frame, %state )
{
   // --- SEQUENCES TAB ---
   // Add trigger to list if this sequence is selected
   if ( ShapeEdSequenceList.getSelectedName() $= %seqName )
      ShapeEdTriggerList.addItem( %frame, %state );
}

function ShapeEdPropWindow::update_onTriggerRemoved( %this, %seqName, %frame, %state )
{
   // --- SEQUENCES TAB ---
   // Remove trigger from list if this sequence is selected
   if ( ShapeEdSequenceList.getSelectedName() $= %seqName )
      ShapeEdTriggerList.removeItem( %frame, %state );
}

function ShapeEdTriggerList::getTriggerText( %this, %frame, %state )
{
   // First column is invisible and used only for sorting
   %sortKey = ( %frame * 1000 ) + ( mAbs( %state ) * 10 ) + ( ( %state > 0 ) ? 1 : 0 );
   return %sortKey TAB %frame TAB mAbs( %state ) TAB ( ( %state > 0 ) ? "on" : "off" );
}

function ShapeEdTriggerList::addItem( %this, %frame, %state )
{
   // Add to text list
   %row = %this.addRow( %this.triggerId, %this.getTriggerText( %frame, %state ) );
   %this.sortNumerical( 0, true );

   // Add marker to animation timeline
   %pos = ShapeEdAnimWindow.getTimelineBitmapPos( ShapeEdAnimWindow-->seqIn.getText() + %frame, 2 );
   %ctrl = new GuiBitmapCtrl()
   {
      internalName = "trigger" @ %this.triggerId;
      Profile = "ToolsGuiDefaultProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      position = %pos SPC "0";
      Extent = "2 12";
      bitmap = "tools/shapeEditor/images/trigger_marker";
   };
   ShapeEdAnimWindow.getObject(0).addGuiControl( %ctrl );
   %this.triggerId++;
}

function ShapeEdTriggerList::removeItem( %this, %frame, %state )
{
   // Remove from text list
   %row = %this.findTextIndex( %this.getTriggerText( %frame, %state ) );
   if ( %row > 0 )
   {
      eval( "ShapeEdAnimWindow-->trigger" @ %this.getRowId( %row ) @ ".delete();" );
      %this.removeRow( %row );
   }
}

function ShapeEdTriggerList::removeAll( %this )
{
   %count = %this.rowCount();
   for ( %row = %count-1; %row > 0; %row-- )
   {
      eval( "ShapeEdAnimWindow-->trigger" @ %this.getRowId( %row ) @ ".delete();" );
      %this.removeRow( %row );
   }
}

function ShapeEdTriggerList::updateItem( %this, %oldFrame, %oldState, %frame, %state )
{
   // Update text list entry
   %oldText = %this.getTriggerText( %oldFrame, %oldState );
   %row = %this.getSelectedRow();
   if ( ( %row <= 0 ) || ( %this.getRowText( %row ) !$= %oldText ) )
      %row = %this.findTextIndex( %oldText );
   if ( %row > 0 )
   {
      %updatedId = %this.getRowId( %row );
      %newText = %this.getTriggerText( %frame, %state );
      %this.setRowById( %updatedId, %newText );

      // keep selected row the same
      %selectedId = %this.getSelectedId();
      %this.sortNumerical( 0, true );
      %this.setSelectedById( %selectedId );

      // Update animation timeline marker
      if ( %frame != %oldFrame )
      {
         %pos = ShapeEdAnimWindow.getTimelineBitmapPos( ShapeEdAnimWindow-->seqIn.getText() + %frame, 2 );
         eval( "%ctrl = ShapeEdAnimWindow-->trigger" @ %updatedId @ ";" );
         %ctrl.position = %pos SPC "0";
      }
   }
}

function ShapeEdSequences::onAddTrigger( %this )
{
    // Can only add triggers if a sequence is selected
    %seqName = ShapeEdSequenceList.getSelectedName();
    if ( %seqName !$= "" )
    {
        // Add a new trigger at the current frame
        %frame = mRound( ShapeEdSeqSlider.getValue() ) - %this-->startFrame.getText();
        if ((%frame < 0) || (%frame > %this-->endFrame.getText() - %this-->startFrame.getText()))
        {
            MessageBoxOK( "Error", "Trigger out of range of the selected animation." );
        }
        else
        {
        %state = ShapeEdTriggerList.rowCount() % 30;
        ShapeEditor.doAddTrigger( %seqName, %frame, %state );
        }
    }
}

function ShapeEdTriggerList::onDeleteSelection( %this )
{
   // Can only delete a trigger if a sequence and trigger are selected
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      %row = %this.getSelectedRow();
      if ( %row > 0 )
      {
         %text = %this.getRowText( %row );
         %frame = getWord( %text, 1 );
         %state = getWord( %text, 2 );
         %state *= ( getWord( %text, 3 ) $= "on" ) ? 1 : -1;
         ShapeEditor.doRemoveTrigger( %seqName, %frame, %state );
      }
   }
}

function ShapeEdTriggerList::onEditSelection( %this )
{
   // Can only edit triggers if a sequence and trigger are selected
   %seqName = ShapeEdSequenceList.getSelectedName();
   if ( %seqName !$= "" )
   {
      %row = ShapeEdTriggerList.getSelectedRow();
      if ( %row > 0 )
      {
         %text = %this.getRowText( %row );
         %oldFrame = getWord( %text, 1 );
         %oldState = getWord( %text, 2 );
         %oldState *= ( getWord( %text, 3 ) $= "on" ) ? 1 : -1;

         %frame = mRound( ShapeEdSequences-->triggerFrame.getText() );
         %state = mRound( mAbs( ShapeEdSequences-->triggerNum.getText() ) );
         %state *= ShapeEdSequences-->triggerOnOff.getValue() ? 1 : -1;

         if ( ( %frame >= 0 ) && ( %state != 0 ) )
            ShapeEditor.doEditTrigger( %seqName, %oldFrame, %oldState, %frame, %state );
      }
   }
}

//------------------------------------------------------------------------------
// Material Editing
//------------------------------------------------------------------------------

function ShapeEdMaterials::updateMaterialList( %this )
{
   // --- MATERIALS TAB ---
   ShapeEdMaterialList.clear();
   ShapeEdMaterialList.addRow( -2, "Name" TAB "Mapped" );
   ShapeEdMaterialList.setRowActive( -2, false );
   ShapeEdMaterialList.addRow( -1, "<none>" );
   %count = ShapeEditor.shape.getTargetCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %matName = ShapeEditor.shape.getTargetName( %i );
      %mapped = getMaterialMapping( %matName );
      if ( %mapped $= "" )
         ShapeEdMaterialList.addRow( WarningMaterial.getID(), %matName TAB "unmapped" );
      else
         ShapeEdMaterialList.addRow( %mapped.getID(), %matName TAB %mapped );
   }

   ShapeEdMaterials-->materialListHeader.setExtent( getWord( ShapeEdMaterialList.extent, 0 ) SPC "19" );
}

function ShapeEdMaterials::updateSelectedMaterial( %this, %highlight )
{
   // Remove the highlight effect from the old selection
   if ( isObject( %this.selectedMaterial ) )
   {
      %this.selectedMaterial.diffuseMap[1] = %this.savedMap;
      %this.selectedMaterial.reload();
   }

   // Apply the highlight effect to the new selected material
   %this.selectedMapTo = getField( ShapeEdMaterialList.getRowText( ShapeEdMaterialList.getSelectedRow() ), 0 );
   %this.selectedMaterial = ShapeEdMaterialList.getSelectedId();
   %this.savedMap = %this.selectedMaterial.diffuseMap[1];
   if ( %highlight && isObject( %this.selectedMaterial ) )
   {
      %this.selectedMaterial.diffuseMap[1] = "tools/shapeEditor/images/highlight_material";
      %this.selectedMaterial.reload();
   }
}

function ShapeEdMaterials::editSelectedMaterial( %this )
{
   if ( isObject( %this.selectedMaterial ) )
   {
      // Remove the highlight effect from the selected material, then switch
      // to the Material Editor
      %this.updateSelectedMaterial( false );

      // Create a temporary TSStatic so the MaterialEditor can query the model's
      // materials.
      pushInstantGroup();
      %this.tempShape = new TSStatic() {
         shapeName = ShapeEditor.shape.baseShape;
         collisionType = "None";
      };
      popInstantGroup();

      MaterialEditorGui.currentMaterial = %this.selectedMaterial;
      MaterialEditorGui.currentObject = $Tools::materialEditorList = %this.tempShape;

      ShapeEdSelectWindow.setVisible( false );
      ShapeEdPropWindow.setVisible( false );
      
      EditorGui-->MatEdPropertiesWindow.setVisible( true );
      EditorGui-->MatEdPreviewWindow.setVisible( true );
      
      MatEd_phoBreadcrumb.setVisible( true );
      MatEd_phoBreadcrumb.command = "ShapeEdMaterials.editSelectedMaterialEnd();";
      
      advancedTextureMapsRollout.Expanded = false;
      materialAnimationPropertiesRollout.Expanded = false;
      materialAdvancedPropertiesRollout.Expanded = false;
   
      MaterialEditorGui.open();
      MaterialEditorGui.setActiveMaterial( %this.selectedMaterial );

      %id = SubMaterialSelector.findText( %this.selectedMapTo );
      if( %id != -1 )
         SubMaterialSelector.setSelected( %id );
   }
}

function ShapeEdMaterials::editSelectedMaterialEnd( %this, %closeEditor )
{   
   MatEd_phoBreadcrumb.setVisible( false );
   MatEd_phoBreadcrumb.command = "";
   
   MaterialEditorGui.quit();
   EditorGui-->MatEdPropertiesWindow.setVisible( false );
   EditorGui-->MatEdPreviewWindow.setVisible( false );

   // Delete the temporary TSStatic
   %this.tempShape.delete();

   if( !%closeEditor )
   {
      ShapeEdSelectWindow.setVisible( true );
      ShapeEdPropWindow.setVisible( true );
   }
}

//------------------------------------------------------------------------------
// Detail/Mesh Editing
//------------------------------------------------------------------------------

function ShapeEdDetails::onWake( %this )
{
   // Initialise popup menus
   %this-->bbType.clear();
   %this-->bbType.add( "None", 0 );
   %this-->bbType.add( "Billboard", 1 );
   %this-->bbType.add( "Z Billboard", 2 );

   %this-->addGeomTo.clear();
   %this-->addGeomTo.add( "current detail", 0 );
   %this-->addGeomTo.add( "new detail", 1 );
   %this-->addGeomTo.setSelected( 0, false );

   ShapeEdDetailTree.onDefineIcons();
}

function ShapeEdDetailTree::onDefineIcons(%this)
{
   // Set the tree view icon indices and texture paths
   %this._imageNone = 0;
   %this._imageHidden = 1;

   %icons = ":" @                                        // no icon
            "tools/gui/images/visible_i:";               // hidden

   %this.buildIconTable( %icons );
}

// Return true if the item in the details tree view is a detail level (false if
// a mesh)
function ShapeEdDetailTree::isDetailItem( %this, %id )
{
   return ( %this.getParentItem( %id ) == 1 );
}

// Get the detail level index from the ID of an item in the details tree view
function ShapeEdDetailTree::getDetailLevelFromItem( %this, %id )
{
   if ( %this.isDetailItem( %id ) )
      %detSize = %this.getItemValue( %id );
      
   else
      %detSize = %this.getItemValue( %this.getParentItem( %id ) );
   return ShapeEditor.shape.getDetailLevelIndex( %detSize );
}

function ShapeEdDetailTree::addMeshEntry( %this, %name, %noSync )
{
   // Add new detail level if required
   %size = getTrailingNumber( %name );
   %detailID = %this.findItemByValue( %size );
   if ( %detailID <= 0 )
   {
      %dl = ShapeEditor.shape.getDetailLevelIndex( %size );
      %detName = ShapeEditor.shape.getDetailLevelName( %dl );
      %detailID = ShapeEdDetailTree.insertItem( 1, %detName, %size, "" );

      // Sort details by decreasing size
      for ( %sibling = ShapeEdDetailTree.getPrevSibling( %detailID );
            ( %sibling > 0 ) && ( ShapeEdDetailTree.getItemValue( %sibling ) < %size );
            %sibling = ShapeEdDetailTree.getPrevSibling( %detailID ) )
         ShapeEdDetailTree.moveItemUp( %detailID );

      if ( !%noSync )
         ShapeEdDetails.update_onDetailsChanged();
   }
   return %this.insertItem( %detailID, %name, "", "" );
}

function ShapeEdDetailTree::removeMeshEntry( %this, %name, %size )
{
   %size = getTrailingNumber( %name );
   %id = ShapeEdDetailTree.findItemByName( %name );
   if ( ShapeEditor.shape.getDetailLevelIndex( %size ) < 0 )
   {
      // Last mesh of a detail level has been removed => remove the detail level
      %this.removeItem( %this.getParentItem( %id ) );
      ShapeEdDetails.update_onDetailsChanged();
   }
   else
      %this.removeItem( %id );
}

function ShapeEdAdvancedWindow::update_onShapeSelectionChanged( %this )
{
   ShapeEdShapeView.currentDL = 0;
   ShapeEdShapeView.onDetailChanged();
}

function ShapeEdPropWindow::update_onDetailRenamed( %this, %oldName, %newName )
{
   // --- DETAILS TAB ---
   // Rename detail entry
   %id = ShapeEdDetailTree.findItemByName( %oldName );
   if ( %id > 0 )
   {
      %size = ShapeEdDetailTree.getItemValue( %id );
      ShapeEdDetailTree.editItem( %id, %newName, %size );

      // Sync text if item is selected
      if ( ShapeEdDetailTree.isItemSelected( %id ) &&
           ( ShapeEdDetails-->meshName.getText() !$= %newName ) )
         ShapeEdDetails-->meshName.setText( stripTrailingNumber( %newName ) );
   }
}

function ShapeEdPropWindow::update_onDetailSizeChanged( %this, %oldSize, %newSize )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();
   %dl = ShapeEditor.shape.getDetailLevelIndex( %newSize );
   if ( ShapeEdAdvancedWindow-->detailSize.getText() $= %oldSize )
   {
      ShapeEdShapeView.currentDL = %dl;
      ShapeEdAdvancedWindow-->detailSize.setText( %newSize );
      ShapeEdDetails-->meshSize.setText( %newSize );
   }

   // --- DETAILS TAB ---
   // Update detail entry then resort details by size
   %id = ShapeEdDetailTree.findItemByValue( %oldSize );
   %detName = ShapeEditor.shape.getDetailLevelName( %dl );
   ShapeEdDetailTree.editItem( %id, %detName, %newSize );

   for ( %sibling = ShapeEdDetailTree.getPrevSibling( %id );
         ( %sibling > 0 ) && ( ShapeEdDetailTree.getItemValue( %sibling ) < %newSize );
         %sibling = ShapeEdDetailTree.getPrevSibling( %id ) )
      ShapeEdDetailTree.moveItemUp( %id );
   for ( %sibling = ShapeEdDetailTree.getNextSibling( %id );
         ( %sibling > 0 ) && ( ShapeEdDetailTree.getItemValue( %sibling ) > %newSize );
         %sibling = ShapeEdDetailTree.getNextSibling( %id ) )
      ShapeEdDetailTree.moveItemDown( %id );

   // Update size values for meshes of this detail
   for ( %child = ShapeEdDetailTree.getChild( %id );
         %child > 0;
         %child = ShapeEdDetailTree.getNextSibling( %child ) )
   {
      %meshName = stripTrailingNumber( ShapeEdDetailTree.getItemText( %child ) );
      ShapeEdDetailTree.editItem( %child, %meshName SPC %newSize, "" );
   }
}

function ShapeEdDetails::update_onDetailsChanged( %this )
{
   %detailCount = ShapeEditor.shape.getDetailLevelCount();
   ShapeEdAdvancedWindow-->detailSlider.range = "0" SPC ( %detailCount-1 );
   if ( %detailCount >= 2 )
      ShapeEdAdvancedWindow-->detailSlider.ticks = %detailCount - 2;
   else
      ShapeEdAdvancedWindow-->detailSlider.ticks = 0;

   // Initialise imposter settings
   ShapeEdAdvancedWindow-->bbUseImposters.setValue( ShapeEditor.shape.getImposterDetailLevel() != -1 );

   // Update detail parameters
   if ( ShapeEdShapeView.currentDL < %detailCount )
   {
      %settings = ShapeEditor.shape.getImposterSettings( ShapeEdShapeView.currentDL );
      %isImposter = getWord( %settings, 0 );

      ShapeEdAdvancedWindow-->imposterInactive.setVisible( !%isImposter );

      ShapeEdAdvancedWindow-->bbEquatorSteps.setText( getField( %settings, 1 ) );
      ShapeEdAdvancedWindow-->bbPolarSteps.setText( getField( %settings, 2 ) );
      ShapeEdAdvancedWindow-->bbDetailLevel.setText( getField( %settings, 3 ) );
      ShapeEdAdvancedWindow-->bbDimension.setText( getField( %settings, 4 ) );
      ShapeEdAdvancedWindow-->bbIncludePoles.setValue( getField( %settings, 5 ) );
      ShapeEdAdvancedWindow-->bbPolarAngle.setText( getField( %settings, 6 ) );
   }
}

function ShapeEdPropWindow::update_onObjectNodeChanged( %this, %objName )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();

   // --- DETAILS TAB ---
   // Update the node popup menu if this object is selected
   if ( ShapeEdDetails-->meshName.getText() $= %objName )
   {
      %nodeName = ShapeEditor.shape.getObjectNode( %objName );
      if ( %nodeName $= "" )
         %nodeName = "<root>";
      %id = ShapeEdDetails-->objectNode.findText( %nodeName );
      ShapeEdDetails-->objectNode.setSelected( %id, false );
   }
}

function ShapeEdPropWindow::update_onObjectRenamed( %this, %oldName, %newName )
{
   // --- DETAILS TAB ---
   // Rename tree entries for this object
   %count = ShapeEditor.shape.getMeshCount( %newName );
   for ( %i = 0; %i < %count; %i++ )
   {
      %size = getTrailingNumber( ShapeEditor.shape.getMeshName( %newName, %i ) );
      %id = ShapeEdDetailTree.findItemByName( %oldName SPC %size );
      if ( %id > 0 )
      {
         ShapeEdDetailTree.editItem( %id, %newName SPC %size, "" );

         // Sync text if item is selected
         if ( ShapeEdDetailTree.isItemSelected( %id ) &&
              ( ShapeEdDetails-->meshName.getText() !$= %newName ) )
            ShapeEdDetails-->meshName.setText( %newName );
      }
   }
}

function ShapeEdPropWindow::update_onMeshAdded( %this, %meshName )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();
   ShapeEdShapeView.updateNodeTransforms();

   // --- COLLISION WINDOW ---
   // Add object to target list if it does not already exist
   if ( !ShapeEditor.isCollisionMesh( %meshName ) )
   {
      %objName = stripTrailingNumber( %meshName );
      %id = ShapeEdColWindow-->colTarget.findText( %objName );
      if ( %id == -1 )
         ShapeEdColWindow-->colTarget.add( %objName );
   }

   // --- DETAILS TAB ---
   %id = ShapeEdDetailTree.addMeshEntry( %meshName );
   ShapeEdDetailTree.clearSelection();
   ShapeEdDetailTree.selectItem( %id );
}

function ShapeEdPropWindow::update_onMeshSizeChanged( %this, %meshName, %oldSize, %newSize )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();

   // --- DETAILS TAB ---
   // Move the mesh to the new location in the tree
   %selected = ShapeEdDetailTree.getSelectedItem();
   %id = ShapeEdDetailTree.findItemByName( %meshName SPC %oldSize );
   ShapeEdDetailTree.removeMeshEntry( %meshName SPC %oldSize );
   %newId = ShapeEdDetailTree.addMeshEntry( %meshName SPC %newSize );

   // Re-select the new entry if it was selected
   if ( %selected == %id )
   {
      ShapeEdDetailTree.clearSelection();
      ShapeEdDetailTree.selectItem( %newId );
   }
}

function ShapeEdPropWindow::update_onMeshRemoved( %this, %meshName )
{
   // --- MISC ---
   ShapeEdShapeView.refreshShape();

   // --- COLLISION WINDOW ---
   // Remove object from target list if it no longer exists
   %objName = stripTrailingNumber( %meshName );
   if ( ShapeEditor.shape.getObjectIndex( %objName ) == -1 )
   {
      %id = ShapeEdColWindow-->colTarget.findText( %objName );
      if ( %id != -1 )
         ShapeEdColWindow-->colTarget.clearEntry( %id );
   }

   // --- DETAILS TAB ---
   // Determine which item to select next
   %id = ShapeEdDetailTree.findItemByName( %meshName );
   if ( %id > 0 )
   {
      %nextId = ShapeEdDetailTree.getPrevSibling( %id );
      if ( %nextId <= 0 )
      {
         %nextId = ShapeEdDetailTree.getNextSibling( %id );
         if ( %nextId <= 0 )
            %nextId = 2;
      }

      // Remove the entry from the tree
      %meshSize = getTrailingNumber( %meshName );
      ShapeEdDetailTree.removeMeshEntry( %meshName, %meshSize );

      // Change selection if needed
      if ( ShapeEdDetailTree.getSelectedItem() == -1 )
         ShapeEdDetailTree.selectItem( %nextId );
   }
}

function ShapeEdDetailTree::onSelect( %this, %id )
{
   %name = %this.getItemText( %id );
   %baseName = stripTrailingNumber( %name );
   %size = getTrailingNumber( %name );

   ShapeEdDetails-->meshName.setText( %baseName );
   ShapeEdDetails-->meshSize.setText( %size );

   // Select the appropriate detail level
   %dl = %this.getDetailLevelFromItem( %id );
   ShapeEdShapeView.currentDL = %dl;

   if ( %this.isDetailItem( %id ) )
   {
      // Selected a detail => disable mesh controls
      ShapeEdDetails-->editMeshInactive.setVisible( true );
      ShapeEdShapeView.selectedObject = -1;
      ShapeEdShapeView.selectedObjDetail = 0;
   }
   else
   {
      // Selected a mesh => sync mesh controls
      ShapeEdDetails-->editMeshInactive.setVisible( false );

      switch$ ( ShapeEditor.shape.getMeshType( %name ) )
      {
         case "normal":          ShapeEdDetails-->bbType.setSelected( 0, false );
         case "billboard":       ShapeEdDetails-->bbType.setSelected( 1, false );
         case "billboardzaxis":  ShapeEdDetails-->bbType.setSelected( 2, false );
      }

      %node = ShapeEditor.shape.getObjectNode( %baseName );
      if ( %node $= "" )
         %node = "<root>";
      ShapeEdDetails-->objectNode.setSelected( ShapeEdDetails-->objectNode.findText( %node ), false );
      ShapeEdShapeView.selectedObject = ShapeEditor.shape.getObjectIndex( %baseName );
      ShapeEdShapeView.selectedObjDetail = %dl;
   }
}

function ShapeEdDetailTree::onRightMouseUp( %this, %itemId, %mouse )
{
   // Open context menu if this is a Mesh item
   if ( !%this.isDetailItem( %itemId ) )
   {
      if( !isObject( "ShapeEdMeshPopup" ) )
      {
         new PopupMenu( ShapeEdMeshPopup )
         {
            superClass = "MenuBuilder";
            isPopup = "1";

            item[ 0 ] = "Hidden" TAB "" TAB "ShapeEdDetailTree.onHideMeshItem( %this._objName, !%this._itemHidden );";
            item[ 1 ] = "-";
            item[ 2 ] = "Hide all" TAB "" TAB "ShapeEdDetailTree.onHideMeshItem( \"\", true );";
            item[ 3 ] = "Show all" TAB "" TAB "ShapeEdDetailTree.onHideMeshItem( \"\", false );";
         };
      }

      ShapeEdMeshPopup._objName = stripTrailingNumber( %this.getItemText( %itemId ) );
      ShapeEdMeshPopup._itemHidden = ShapeEdShapeView.getMeshHidden( ShapeEdMeshPopup._objName );

      ShapeEdMeshPopup.checkItem( 0, ShapeEdMeshPopup._itemHidden );
      ShapeEdMeshPopup.showPopup( Canvas );
   }
}

function ShapeEdDetailTree::onHideMeshItem( %this, %objName, %hide )
{
   if ( %hide )
      %imageId = %this._imageHidden;
   else
      %imageId = %this._imageNone;

   if ( %objName $= "" )
   {
      // Show/hide all
      ShapeEdShapeView.setAllMeshesHidden( %hide );
      for ( %parent = %this.getChild(%this.getFirstRootItem()); %parent > 0; %parent = %this.getNextSibling(%parent) )
         for ( %child = %this.getChild(%parent); %child > 0; %child = %this.getNextSibling(%child) )
            %this.setItemImages( %child, %imageId, %imageId );
   }
   else
   {
      // Show/hide all meshes for this object
      ShapeEdShapeView.setMeshHidden( %objName, %hide );
      %count = ShapeEditor.shape.getMeshCount( %objName );
      for ( %i = 0; %i < %count; %i++ )
      {
         %meshName = ShapeEditor.shape.getMeshName( %objName, %i );
         %id = ShapeEdDetailTree.findItemByName( %meshName );
         if ( %id > 0 )
            %this.setItemImages( %id, %imageId, %imageId );
      }
   }
}

function ShapeEdShapeView::onDetailChanged( %this )
{
   // Update slider
   if ( mRound( ShapeEdAdvancedWindow-->detailSlider.getValue() ) != %this.currentDL )
      ShapeEdAdvancedWindow-->detailSlider.setValue( %this.currentDL );
   ShapeEdAdvancedWindow-->detailSize.setText( %this.detailSize );

   ShapeEdDetails.update_onDetailsChanged();

   %id = ShapeEdDetailTree.getSelectedItem();
   if ( ( %id <= 0 ) || ( %this.currentDL != ShapeEdDetailTree.getDetailLevelFromItem( %id ) ) )
   {
      %id = ShapeEdDetailTree.findItemByValue( %this.detailSize );
      if ( %id > 0 )
      {
         ShapeEdDetailTree.clearSelection();
         ShapeEdDetailTree.selectItem( %id );
      }
   }
}

function ShapeEdAdvancedWindow::onEditDetailSize( %this )
{
   // Change the size of the current detail level
   %oldSize = ShapeEditor.shape.getDetailLevelSize( ShapeEdShapeView.currentDL );
   %detailSize = %this-->detailSize.getText();
   ShapeEditor.doEditDetailSize( %oldSize, %detailSize );
}

function ShapeEdDetails::onEditName( %this )
{
   %newName = %this-->meshName.getText();

   // Check if we are renaming a detail or a mesh
   %id = ShapeEdDetailTree.getSelectedItem();
   %oldName = ShapeEdDetailTree.getItemText( %id );

   if ( ShapeEdDetailTree.isDetailItem( %id ) )
   {
      // Rename the selected detail level
      %oldSize = getTrailingNumber( %oldName );
      ShapeEditor.doRenameDetail( %oldName, %newName @ %oldSize );
   }
   else
   {
      // Rename the selected mesh
      ShapeEditor.doRenameObject( stripTrailingNumber( %oldName ), %newName );
   }
}

function ShapeEdDetails::onEditSize( %this )
{
   %newSize = %this-->meshSize.getText();

   // Check if we are changing the size for a detail or a mesh
   %id = ShapeEdDetailTree.getSelectedItem();
   if ( ShapeEdDetailTree.isDetailItem( %id ) )
   {
      // Change the size of the selected detail level
      %oldSize = ShapeEdDetailTree.getItemValue( %id );
      ShapeEditor.doEditDetailSize( %oldSize, %newSize );
   }
   else
   {
      // Change the size of the selected mesh
      %meshName = ShapeEdDetailTree.getItemText( %id );
      ShapeEditor.doEditMeshSize( %meshName, %newSize );
   }
}

function ShapeEdDetails::onEditBBType( %this )
{
   // This command is only valid for meshes (not details)
   %id = ShapeEdDetailTree.getSelectedItem();
   if ( !ShapeEdDetailTree.isDetailItem( %id ) )
   {
      %meshName = ShapeEdDetailTree.getItemText( %id );
      %bbType = ShapeEdDetails-->bbType.getText();
      switch$ ( %bbType )
      {
         case "None":         %bbType = "normal";
         case "Billboard":    %bbType = "billboard";
         case "Z Billboard":  %bbType = "billboardzaxis";
      }
      ShapeEditor.doEditMeshBillboard( %meshName, %bbType );
   }
}

function ShapeEdDetails::onSetObjectNode( %this )
{
   // This command is only valid for meshes (not details)
   %id = ShapeEdDetailTree.getSelectedItem();
   if ( !ShapeEdDetailTree.isDetailItem( %id ) )
   {
      %meshName = ShapeEdDetailTree.getItemText( %id );
      %objName = stripTrailingNumber( %meshName );
      %node = %this-->objectNode.getText();
      if ( %node $= "<root>" )
         %node = "";
      ShapeEditor.doSetObjectNode( %objName, %node );
   }
}

function ShapeEdDetails::onAddMeshFromFile( %this, %path )
{
   if ( %path $= "" )
   {
      getLoadFormatFilename( %this @ ".onAddMeshFromFile", %this.lastPath );
      return;
   }

   %path = makeRelativePath( %path, getMainDotCSDir() );
   %this.lastPath = %path;

   // Determine the detail level to use for the new geometry
   if ( %this-->addGeomTo.getText() $= "current detail" )
   {
      %size = ShapeEditor.shape.getDetailLevelSize( ShapeEdShapeView.currentDL );
   }
   else
   {
      // Check if the file has an LODXXX hint at the end of it
      %base = fileBase( %path );
      %pos = strstr( %base, "_LOD" );
      if ( %pos > 0 )
         %size = getSubStr( %base, %pos + 4, strlen( %base ) ) + 0;
      else
         %size = 2;

      // Make sure size is not in use
      while ( ShapeEditor.shape.getDetailLevelIndex( %size ) != -1 )
         %size++;
   }

   ShapeEditor.doAddMeshFromFile( %path, %size );
}

function ShapeEdDetails::onDeleteMesh( %this )
{
   %id = ShapeEdDetailTree.getSelectedItem();
   if ( ShapeEdDetailTree.isDetailItem( %id ) )
   {
      %detSize = ShapeEdDetailTree.getItemValue( %id );
      ShapeEditor.doRemoveShapeData( "Detail", %detSize );
   }
   else
   {
      %name = ShapeEdDetailTree.getItemText( %id );
      ShapeEditor.doRemoveShapeData( "Mesh", %name );
   }
}

function ShapeEdDetails::onToggleImposter( %this, %useImposter )
{
   %hasImposterDetail = ( ShapeEditor.shape.getImposterDetailLevel() != -1 );
   if ( %useImposter == %hasImposterDetail )
      return;

   if ( %useImposter )
   {
      // Determine an unused detail size
      for ( %detailSize = 0; %detailSize < 50; %detailSize++ )
      {
         if ( ShapeEditor.shape.getDetailLevelIndex( %detailSize ) == -1 )
            break;
      }

      // Set some initial values for the imposter
      %bbEquatorSteps = 6;
      %bbPolarSteps = 0;
      %bbDetailLevel = 0;
      %bbDimension = 128;
      %bbIncludePoles = 0;
      %bbPolarAngle = 0;

      // Add a new imposter detail level to the shape
      ShapeEditor.doEditImposter( -1, %detailSize, %bbEquatorSteps, %bbPolarSteps,
         %bbDetailLevel, %bbDimension, %bbIncludePoles, %bbPolarAngle );
   }
   else
   {
      // Remove the imposter detail level
      ShapeEditor.doRemoveImposter();
   }
}

function ShapeEdDetails::onEditImposter( %this )
{
   // Modify the parameters of the current imposter detail level
   %detailSize = ShapeEditor.shape.getDetailLevelSize( ShapeEdShapeView.currentDL );
   %bbDimension = ShapeEdAdvancedWindow-->bbDimension.getText();
   %bbDetailLevel = ShapeEdAdvancedWindow-->bbDetailLevel.getText();
   %bbEquatorSteps = ShapeEdAdvancedWindow-->bbEquatorSteps.getText();
   %bbIncludePoles = ShapeEdAdvancedWindow-->bbIncludePoles.getValue();
   %bbPolarSteps = ShapeEdAdvancedWindow-->bbPolarSteps.getText();
   %bbPolarAngle = ShapeEdAdvancedWindow-->bbPolarAngle.getText();

   ShapeEditor.doEditImposter( ShapeEdShapeView.currentDL, %detailSize,
      %bbEquatorSteps, %bbPolarSteps, %bbDetailLevel, %bbDimension,
      %bbIncludePoles, %bbPolarAngle );
}


function ShapeEditor::autoAddDetails( %this, %dest )
{
   // Sets of LOD files are named like:
   //
   // MyShape_LOD200.dae
   // MyShape_LOD64.dae
   // MyShape_LOD2.dae
   //
   // Determine the base name of the input file (MyShape_LOD in the example above)
   // and use that to find any other shapes in the set.
   %base = fileBase( %dest.baseShape );
   %pos = strstr( %base, "_LOD" );
   if ( %pos < 0 )
   {
      echo( "Not an LOD shape file" );
      return;
   }

   %base = getSubStr( %base, 0, %pos + 4 );

   echo( "Base is: " @ %base );

   %filePatterns = filePath( %dest.baseShape ) @ "/" @ %base @ "*" @ fileExt( %dest.baseShape );

   echo( "Pattern is: " @ %filePatterns );

   %fullPath = findFirstFileMultiExpr( %filePatterns );
   while ( %fullPath !$= "" )
   {
      %fullPath = makeRelativePath( %fullPath, getMainDotCSDir() );

      if ( %fullPath !$= %dest.baseShape )
      {
         echo( "Found LOD shape file: " @ %fullPath );

         // Determine the detail size ( number after the base name ), then add the
         // new mesh
         %size = strreplace( fileBase( %fullPath ), %base, "" );
         ShapeEditor.addLODFromFile( %dest, %fullPath, %size, 0 );
      }

      %fullPath = findNextFileMultiExpr( %filePatterns );
   }

   if ( %this.shape == %dest )
   {
      ShapeEdShapeView.refreshShape();
      ShapeEdDetails.update_onDetailsChanged();
   }
}

function ShapeEditor::addLODFromFile( %this, %dest, %filename, %size, %allowUnmatched )
{
   // Get (or create) a TSShapeConstructor object for the source shape. Need to
   // exec the script manually as the resource may not have been loaded yet
   %csPath = filePath( %filename ) @ "/" @ fileBase( %filename ) @ ".cs";
   if ( isFile( %csPath ) )
      exec( %csPath );

   %source = ShapeEditor.findConstructor( %filename );
   if ( %source == -1 )
      %source = ShapeEditor.createConstructor( %filename );
   %source.lodType = "SingleSize";
   %source.singleDetailSize = %size;

   // Create a temporary TSStatic to ensure the resource is loaded
   %temp = new TSStatic() {
      shapeName = %filename;
      collisionType = "None";
   };

   %meshList = "";
   if ( isObject( %temp ) )
   {
      // Add a new mesh for each object in the source shape
      %objCount = %source.getObjectCount();
      for ( %i = 0; %i < %objCount; %i++ )
      {
         %objName = %source.getObjectName( %i );

         echo( "Checking for object " @ %objName );

         if ( %allowUnmatched || ( %dest.getObjectIndex( %objName ) != -1 ) )
         {
            // Add the source object's highest LOD mesh to the destination shape
            echo( "Adding detail size" SPC %size SPC "for object" SPC %objName );
            %srcName = %source.getMeshName( %objName, 0 );
            %destName = %objName SPC %size;
            %dest.addMesh( %destName, %filename, %srcName );
            %meshList = %meshList TAB %destName;
         }
      }

      %temp.delete();
   }

   return trim( %meshList );
}

//------------------------------------------------------------------------------
// Collision editing
//------------------------------------------------------------------------------

function ShapeEdColWindow::onWake( %this )
{
   %this-->colType.clear();
   %this-->colType.add( "Box" );
   %this-->colType.add( "Sphere" );
   %this-->colType.add( "Capsule" );
   %this-->colType.add( "10-DOP X" );
   %this-->colType.add( "10-DOP Y" );
   %this-->colType.add( "10-DOP Z" );
   %this-->colType.add( "18-DOP" );
   %this-->colType.add( "26-DOP" );
   %this-->colType.add( "Convex Hulls" );
}

function ShapeEdColWindow::update_onShapeSelectionChanged( %this )
{
   %this.lastColSettings = "" TAB "Bounds";

   // Initialise collision mesh target list
   %this-->colTarget.clear();
   %this-->colTarget.add( "Bounds" );
   %objCount = ShapeEditor.shape.getObjectCount();
   for ( %i = 0; %i < %objCount; %i++ )
      %this-->colTarget.add( ShapeEditor.shape.getObjectName( %i ) );

   %this-->colTarget.setSelected( %this-->colTarget.findText( "Bounds" ), false );
}

function ShapeEdColWindow::update_onCollisionChanged( %this )
{
   // Sync collision settings
   %colData = %this.lastColSettings;

   %typeId = %this-->colType.findText( getField( %colData, 0 ) );
   %this-->colType.setSelected( %typeId, false );

   %targetId = %this-->colTarget.findText( getField( %colData, 1 ) );
   %this-->colTarget.setSelected( %targetId, false );

   if ( %this-->colType.getText() $= "Convex Hulls" )
   {
      %this-->hullInactive.setVisible( false );
      %this-->hullDepth.setValue( getField( %colData, 2 ) );
      %this-->hullDepthText.setText( mFloor( %this-->hullDepth.getValue() ) );
      %this-->hullMergeThreshold.setValue( getField( %colData, 3 ) );
      %this-->hullMergeText.setText( mFloor( %this-->hullMergeThreshold.getValue() ) );
      %this-->hullConcaveThreshold.setValue( getField( %colData, 4 ) );
      %this-->hullConcaveText.setText( mFloor( %this-->hullConcaveThreshold.getValue() ) );
      %this-->hullMaxVerts.setValue( getField( %colData, 5 ) );
      %this-->hullMaxVertsText.setText( mFloor( %this-->hullMaxVerts.getValue() ) );
      %this-->hullMaxBoxError.setValue( getField( %colData, 6 ) );
      %this-->hullMaxBoxErrorText.setText( mFloor( %this-->hullMaxBoxError.getValue() ) );
      %this-->hullMaxSphereError.setValue( getField( %colData, 7 ) );
      %this-->hullMaxSphereErrorText.setText( mFloor( %this-->hullMaxSphereError.getValue() ) );
      %this-->hullMaxCapsuleError.setValue( getField( %colData, 8 ) );
      %this-->hullMaxCapsuleErrorText.setText( mFloor( %this-->hullMaxCapsuleError.getValue() ) );
   }
   else
   {
      %this-->hullInactive.setVisible( true );
   }
}

function ShapeEdColWindow::editCollision( %this )
{
   // If the shape already contains a collision detail size-1, warn the user
   // that it will be removed
   if ( ( ShapeEditor.shape.getDetailLevelIndex( -1 ) >= 0 ) &&
        ( getField(%this.lastColSettings, 0) $= "" ) )
   {
      MessageBoxYesNo( "Warning", "Existing collision geometry at detail size " @
         "-1 will be removed, and this cannot be undone. Do you want to continue?",
         "ShapeEdColWindow.editCollisionOK();", "" );
   }
   else
   {
      %this.editCollisionOK();
   }
}

function ShapeEdColWindow::editCollisionOK( %this )
{
   %type = %this-->colType.getText();
   %target = %this-->colTarget.getText();
   %depth = %this-->hullDepth.getValue();
   %merge = %this-->hullMergeThreshold.getValue();
   %concavity = %this-->hullConcaveThreshold.getValue();
   %maxVerts = %this-->hullMaxVerts.getValue();
   %maxBox = %this-->hullMaxBoxError.getValue();
   %maxSphere = %this-->hullMaxSphereError.getValue();
   %maxCapsule = %this-->hullMaxCapsuleError.getValue();

   ShapeEditor.doEditCollision( %type, %target, %depth, %merge, %concavity, %maxVerts,
                                 %maxBox, %maxSphere, %maxCapsule );
}

//------------------------------------------------------------------------------
// Mounted Shapes
//------------------------------------------------------------------------------

function ShapeEdMountWindow::onWake( %this )
{
   %this-->mountType.clear();
   %this-->mountType.add( "Object", 0 );
   %this-->mountType.add( "Image", 1 );
   %this-->mountType.add( "Wheel", 2 );
   %this-->mountType.setSelected( 1, false );

   %this-->mountSeq.clear();
   %this-->mountSeq.add( "<rootpose>", 0 );
   %this-->mountSeq.setSelected( 0, false );
   %this-->mountPlayBtn.setStateOn( false );

   // Only add the Browse entry the first time so we keep any files the user has
   // set up previously
   if ( ShapeEdMountShapeMenu.size() == 0 )
   {
      ShapeEdMountShapeMenu.add( "Browse...", 0 );
      ShapeEdMountShapeMenu.setSelected( 0, false );
   }
}

function ShapeEdMountWindow::isMountableNode( %this, %nodeName )
{
   return ( startswith( %nodeName, "mount" ) || startswith( %nodeName, "hub" ) );
}

function ShapeEdMountWindow::update_onShapeSelectionChanged( %this )
{
   %this.unmountAll();

   // Initialise the dropdown menus
   %this-->mountNode.clear();
   %this-->mountNode.add( "<origin>" );
   %count = ShapeEditor.shape.getNodeCount();
   for ( %i = 0; %i < %count; %i++ )
   {
      %name = ShapeEditor.shape.getNodeName( %i );
      if ( %this.isMountableNode( %name ) )
         %this-->mountNode.add( %name );
   }
   %this-->mountNode.sort();
   %this-->mountNode.setFirstSelected();

   %this-->mountSeq.clear();
   %this-->mountSeq.add( "<rootpose>", 0 );
   %this-->mountSeq.setSelected( 0, false );
}

function ShapeEdMountWindow::update_onMountSelectionChanged( %this )
{
   %row = %this-->mountList.getSelectedRow();
   if ( %row > 0 )
   {
      %text = %this-->mountList.getRowText( %row );
      %shapePath = getField( %text, 0 );

      ShapeEdMountShapeMenu.setText( %shapePath );
      %this-->mountNode.setText( getField( %text, 2 ) );
      %this-->mountType.setText( getField( %text, 3 ) );

      // Fill in sequence list
      %this-->mountSeq.clear();
      %this-->mountSeq.add( "<rootpose>", 0 );

      %tss = ShapeEditor.findConstructor( %shapePath );
      if ( !isObject( %tss ) )
         %tss = ShapeEditor.createConstructor( %shapePath );
      if ( isObject( %tss ) )
      {
         %count = %tss.getSequenceCount();
         for ( %i = 0; %i < %count; %i++ )
            %this-->mountSeq.add( %tss.getSequenceName( %i ) );
      }

      // Select the currently playing sequence
      %slot = %row - 1;
      %seq = ShapeEdShapeView.getMountThreadSequence( %slot );
      %id = %this-->mountSeq.findText( %seq );
      if ( %id == -1 )
         %id = 0;
      %this-->mountSeq.setSelected( %id, false );

      ShapeEdMountSeqSlider.setValue( ShapeEdShapeView.getMountThreadPos( %slot ) );
      %this-->mountPlayBtn.setStateOn( ShapeEdShapeView.getMountThreadPos( %slot ) != 0 );
   }
}

function ShapeEdMountWindow::updateSelectedMount( %this )
{
   %row = %this-->mountList.getSelectedRow();
   if ( %row > 0 )
      %this.mountShape( %row-1 );
}

function ShapeEdMountWindow::setMountThreadSequence( %this )
{
   %row = %this-->mountList.getSelectedRow();
   if ( %row > 0 )
   {
      ShapeEdShapeView.setMountThreadSequence( %row-1, %this-->mountSeq.getText() );
      ShapeEdShapeView.setMountThreadDir( %row-1, %this-->mountPlayBtn.getValue() );
   }
}

function ShapeEdMountSeqSlider::onMouseDragged( %this )
{
   %row = ShapeEdMountWindow-->mountList.getSelectedRow();
   if ( %row > 0 )
   {
      ShapeEdShapeView.setMountThreadPos( %row-1, %this.getValue() );

      // Pause the sequence when the slider is dragged
      ShapeEdShapeView.setMountThreadDir( %row-1, 0 );
      ShapeEdMountWindow-->mountPlayBtn.setStateOn( false );
   }
}

function ShapeEdMountWindow::toggleMountThreadPlayback( %this )
{
   %row = %this-->mountList.getSelectedRow();
   if ( %row > 0 )
      ShapeEdShapeView.setMountThreadDir( %row-1, %this-->mountPlayBtn.getValue() );
}

function ShapeEdMountShapeMenu::onSelect( %this, %id, %text )
{
   if ( %text $= "Browse..." )
   {
      // Allow the user to browse for an external model file
      getLoadFormatFilename( %this @ ".onBrowseSelect", %this.lastPath );
   }
   else
   {
      // Modify the current mount
      ShapeEdMountWindow.updateSelectedMount();
   }
}

function ShapeEdMountShapeMenu::onBrowseSelect( %this, %path )
{
   %path = makeRelativePath( %path, getMainDotCSDir() );
   %this.lastPath = %path;
   %this.setText( %path );

   // Add entry if unique
   if ( %this.findText( %path ) == -1 )
      %this.add( %path );

   ShapeEdMountWindow.updateSelectedMount();
}

function ShapeEdMountWindow::mountShape( %this, %slot )
{
   %model = ShapeEdMountShapeMenu.getText();
   %node = %this-->mountNode.getText();
   %type = %this-->mountType.getText();

   if ( %model $= "Browse..." )
      %model = "core/art/shapes/octahedron.dts";

   if ( ShapeEdShapeView.mountShape( %model, %node, %type, %slot ) )
   {
      %rowText = %model TAB fileName( %model ) TAB %node TAB %type;
      if ( %slot == -1 )
      {
         %id = %this.mounts++;
         %this-->mountList.addRow( %id, %rowText );
      }
      else
      {
         %id = %this-->mountList.getRowId( %slot+1 );
         %this-->mountList.setRowById( %id, %rowText );
      }

      %this-->mountList.setSelectedById( %id );
   }
   else
   {
      MessageBoxOK( "Error", "Failed to mount \"" @ %model @ "\". Check the console for error messages.", "" );
   }
}

function ShapeEdMountWindow::unmountShape( %this )
{
   %row = %this-->mountList.getSelectedRow();
   if ( %row > 0 )
   {
      ShapeEdShapeView.unmountShape( %row-1 );
      %this-->mountList.removeRow( %row );

      // Select the next row (if any)
      %count = %this-->mountList.rowCount();
      if ( %row >= %count )
         %row = %count-1;
      if ( %row > 0 )
         %this-->mountList.setSelectedRow( %row );
   }
}

function ShapeEdMountWindow::unmountAll( %this )
{
   ShapeEdShapeView.unmountAll();
   %this-->mountList.clear();
   %this-->mountList.addRow( -1, "FullPath" TAB "Filename" TAB "Node" TAB "Type" );
   %this-->mountList.setRowActive( -1, false );
}

//------------------------------------------------------------------------------
// Shape Preview
//------------------------------------------------------------------------------

function ShapeEdPreviewGui::updatePreviewBackground( %color )
{
   ShapeEdPreviewGui-->previewBackground.color = %color;
   ShapeEditorToolbar-->previewBackgroundPicker.color = %color;
}

function showShapeEditorPreview()
{
   %visible = ShapeEditorToolbar-->showPreview.getValue();
   ShapeEdPreviewGui.setVisible( %visible );
}
