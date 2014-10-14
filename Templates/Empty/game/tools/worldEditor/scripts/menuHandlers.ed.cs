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

$Pref::WorldEditor::FileSpec = "Torque Mission Files (*.mis)|*.mis|All Files (*.*)|*.*|";

//////////////////////////////////////////////////////////////////////////
// File Menu Handlers
//////////////////////////////////////////////////////////////////////////

function EditorFileMenu::onMenuSelect(%this)
{
   // don't do this since it won't exist if this is a "demo"
   if(!isWebDemo())
      %this.enableItem(2, EditorIsDirty());
}

//////////////////////////////////////////////////////////////////////////

// Package that gets temporarily activated to toggle editor after mission loading.
// Deactivates itself.
package BootEditor {

function GameConnection::initialControlSet( %this )
{
   Parent::initialControlSet( %this );
   
   toggleEditor( true );
   deactivatePackage( "BootEditor" );
}

};

//////////////////////////////////////////////////////////////////////////

/// Checks the various dirty flags and returns true if the 
/// mission or other related resources need to be saved.  
function EditorIsDirty()
{
   // We kept a hard coded test here, but we could break these
   // into the registered tools if we wanted to.
   %isDirty =  ( isObject( "ETerrainEditor" ) && ( ETerrainEditor.isMissionDirty || ETerrainEditor.isDirty ) )
               || ( isObject( "EWorldEditor" ) && EWorldEditor.isDirty )
               || ( isObject( "ETerrainPersistMan" ) && ETerrainPersistMan.hasDirty() );
   
   // Give the editor plugins a chance to set the dirty flag.
   for ( %i = 0; %i < EditorPluginSet.getCount(); %i++ )
   {
      %obj = EditorPluginSet.getObject(%i);
      %isDirty |= %obj.isDirty(); 
   }
   
   return %isDirty;
}

/// Clears all the dirty state without saving.
function EditorClearDirty()
{
   EWorldEditor.isDirty = false;
   ETerrainEditor.isDirty = false;
   ETerrainEditor.isMissionDirty = false;
   ETerrainPersistMan.clearAll();
   
   for ( %i = 0; %i < EditorPluginSet.getCount(); %i++ )
   {
      %obj = EditorPluginSet.getObject(%i);
      %obj.clearDirty();      
   }
}

function EditorQuitGame()
{
   if( EditorIsDirty() && !isWebDemo())
   {
      MessageBoxYesNoCancel("Level Modified", "Would you like to save your changes before quitting?", "EditorSaveMissionMenu(); quit();", "quit();", "" );
   }
   else
      quit();
}

function EditorExitMission()
{  
   if( EditorIsDirty() && !isWebDemo() )
   {
      MessageBoxYesNoCancel("Level Modified", "Would you like to save your changes before exiting?", "EditorDoExitMission(true);", "EditorDoExitMission(false);", "");
   }
   else
      EditorDoExitMission(false);
}

function EditorDoExitMission(%saveFirst)
{
   if(%saveFirst && !isWebDemo())
   {
      EditorSaveMissionMenu();
   }
   else
   {
      EditorClearDirty();
   }

   if (isObject( MainMenuGui ))
      Editor.close("MainMenuGui");

   disconnect();
}

function EditorOpenTorsionProject( %projectFile )
{
   // Make sure we have a valid path to the Torsion installation.
   
   %torsionPath = EditorSettings.value( "WorldEditor/torsionPath" );
   if( !isFile( %torsionPath ) )
   {
      MessageBoxOK(
         "Torsion Not Found",
         "Torsion not found at '" @ %torsionPath @ "'.  Please set the correct path in the preferences."
      );
      return;
   }
   
   // Determine the path to the .torsion file.
   
   if( %projectFile $= "" )
   {
      %projectName = fileBase( getExecutableName() );
      %projectFile = makeFullPath( %projectName @ ".torsion" );
      if( !isFile( %projectFile ) )
      {
         %projectFile = findFirstFile( "*.torsion", false );
         if( !isFile( %projectFile ) )
         {
            MessageBoxOK(
               "Project File Not Found",
               "Cannot find .torsion project file in '" @ getMainDotCsDir() @ "'."
            );
            return;
         }
      }
   }
   
   // Open the project in Torsion.
   
   shellExecute( %torsionPath, "\"" @ %projectFile @ "\"" );
}

function EditorOpenFileInTorsion( %file, %line )
{
   // Make sure we have a valid path to the Torsion installation.
   
   %torsionPath = EditorSettings.value( "WorldEditor/torsionPath" );
   if( !isFile( %torsionPath ) )
   {
      MessageBoxOK(
         "Torsion Not Found",
         "Torsion not found at '" @ %torsionPath @ "'.  Please set the correct path in the preferences."
      );
      return;
   }
   
   // If no file was specified, take the current mission file.
   
   if( %file $= "" )
      %file = makeFullPath( $Server::MissionFile );
  
   // Open the file in Torsion.
   
   %args = "\"" @ %file;
   if( %line !$= "" )
      %args = %args @ ":" @ %line;
   %args = %args @ "\"";
   
   shellExecute( %torsionPath, %args );
}

function EditorOpenDeclarationInTorsion( %object )
{
   %fileName = %object.getFileName();
   if( %fileName $= "" )
      return;
      
   EditorOpenFileInTorsion( makeFullPath( %fileName ), %object.getDeclarationLine() );
}

function EditorNewLevel( %file )
{
   if(isWebDemo())
      return;
      
   %saveFirst = false;
   if ( EditorIsDirty() )
   {
      error(knob);
      %saveFirst = MessageBox("Mission Modified", "Would you like to save changes to the current mission \"" @
         $Server::MissionFile @ "\" before creating a new mission?", "SaveDontSave", "Question") == $MROk;
   }
      
   if(%saveFirst)
      EditorSaveMission();

   // Clear dirty flags first to avoid duplicate dialog box from EditorOpenMission()
   if( isObject( Editor ) )
   {
      EditorClearDirty();
      Editor.getUndoManager().clearAll();
   }

   if( %file $= "" )
      %file = EditorSettings.value( "WorldEditor/newLevelFile" );

   if( !$missionRunning )
   {
      activatePackage( "BootEditor" );
      StartLevel( %file );
   }
   else
      EditorOpenMission(%file);

   //EWorldEditor.isDirty = true;
   //ETerrainEditor.isDirty = true;
   EditorGui.saveAs = true;
}

function EditorSaveMissionMenu()
{
   if(!$Pref::disableSaving && !isWebDemo())
   {
      if(EditorGui.saveAs)
         EditorSaveMissionAs();
      else
         EditorSaveMission();
   }
   else
   {
      EditorSaveMissionMenuDisableSave();
   }
}

function EditorSaveMission()
{
   // just save the mission without renaming it
   if(isFunction("getObjectLimit") && MissionGroup.getFullCount() >= getObjectLimit())
   {
      MessageBoxOKBuy( "Object Limit Reached", "You have exceeded the object limit of " @ getObjectLimit() @ " for this demo. You can remove objects if you would like to add more.", "", "Canvas.showPurchaseScreen(\"objectlimit\");" );
      return;
   }
   
   // first check for dirty and read-only files:
   if((EWorldEditor.isDirty || ETerrainEditor.isMissionDirty) && !isWriteableFileName($Server::MissionFile))
   {
      MessageBox("Error", "Mission file \""@ $Server::MissionFile @ "\" is read-only.  Continue?", "Ok", "Stop");
      return false;
   }
   if(ETerrainEditor.isDirty)
   {
      // Find all of the terrain files
      initContainerTypeSearch($TypeMasks::TerrainObjectType);

      while ((%terrainObject = containerSearchNext()) != 0)
      {
         if (!isWriteableFileName(%terrainObject.terrainFile))
         {
            if (MessageBox("Error", "Terrain file \""@ %terrainObject.terrainFile @ "\" is read-only.  Continue?", "Ok", "Stop") == $MROk)
               continue;
            else
               return false;
         }
      }
   }
  
   // now write the terrain and mission files out:

   if(EWorldEditor.isDirty || ETerrainEditor.isMissionDirty)
      MissionGroup.save($Server::MissionFile);
   if(ETerrainEditor.isDirty)
   {
      // Find all of the terrain files
      initContainerTypeSearch($TypeMasks::TerrainObjectType);

      while ((%terrainObject = containerSearchNext()) != 0)
         %terrainObject.save(%terrainObject.terrainFile);
   }

   ETerrainPersistMan.saveDirty();
      
   // Give EditorPlugins a chance to save.
   for ( %i = 0; %i < EditorPluginSet.getCount(); %i++ )
   {
      %obj = EditorPluginSet.getObject(%i);
      if ( %obj.isDirty() )
         %obj.onSaveMission( $Server::MissionFile );      
   } 
   
   EditorClearDirty();
   
   EditorGui.saveAs = false;
   
   return true;
}

function EditorSaveMissionMenuDisableSave()
{
   GenericPromptDialog-->GenericPromptWindow.text = "Warning";
   GenericPromptDialog-->GenericPromptText.setText("Saving disabled in demo mode."); 
   Canvas.pushDialog( GenericPromptDialog ); 
}

function EditorSaveMissionAs( %missionName )
{
   if(isFunction("getObjectLimit") && MissionGroup.getFullCount() >= getObjectLimit())
   {
      MessageBoxOKBuy( "Object Limit Reached", "You have exceeded the object limit of " @ getObjectLimit() @ " for this demo. You can remove objects if you would like to add more.", "", "Canvas.showPurchaseScreen(\"objectlimit\");" );
      return;
   }
   
   if(!$Pref::disableSaving && !isWebDemo())
   {
      // If we didn't get passed a new mission name then
      // prompt the user for one.
      if ( %missionName $= "" )
      {
         %dlg = new SaveFileDialog()
         {
            Filters        = $Pref::WorldEditor::FileSpec;
            DefaultPath    = EditorSettings.value("LevelInformation/levelsDirectory");
            ChangePath     = false;
            OverwritePrompt   = true;
         };

         %ret = %dlg.Execute();
         if(%ret)
         {
            // Immediately override/set the levelsDirectory
            EditorSettings.setValue( "LevelInformation/levelsDirectory", collapseFilename(filePath( %dlg.FileName )) );
            
            %missionName = %dlg.FileName;
         }
         
         %dlg.delete();
         
         if(! %ret)
            return;
      }
                  
      if( fileExt( %missionName ) !$= ".mis" )
         %missionName = %missionName @ ".mis";

      EWorldEditor.isDirty = true;
      %saveMissionFile = $Server::MissionFile;

      $Server::MissionFile = %missionName;

      %copyTerrainsFailed = false;

      // Rename all the terrain files.  Save all previous names so we can
      // reset them if saving fails.
      %newMissionName = fileBase(%missionName);
      %oldMissionName = fileBase(%saveMissionFile);
      
      initContainerTypeSearch( $TypeMasks::TerrainObjectType );
      %savedTerrNames = new ScriptObject();
      for( %i = 0;; %i ++ )
      {
         %terrainObject = containerSearchNext();
         if( !%terrainObject )
            break;

         %savedTerrNames.array[ %i ] = %terrainObject.terrainFile;
         
         %terrainFilePath = makeRelativePath( filePath( %terrainObject.terrainFile ), getMainDotCsDir() );
         %terrainFileName = fileName( %terrainObject.terrainFile );
                  
         // Workaround to have terrains created in an unsaved "New Level..." mission
         // moved to the correct place.
         
         if( EditorGui.saveAs && %terrainFilePath $= "tools/art/terrains" )
            %terrainFilePath = "art/terrains";
         
         // Try and follow the existing naming convention.
         // If we can't, use systematic terrain file names.
         if( strstr( %terrainFileName, %oldMissionName ) >= 0 )
            %terrainFileName = strreplace( %terrainFileName, %oldMissionName, %newMissionName );
         else
            %terrainFileName = %newMissionName @ "_" @ %i @ ".ter";

         %newTerrainFile = %terrainFilePath @ "/" @ %terrainFileName;

         if (!isWriteableFileName(%newTerrainFile))
         {
            if (MessageBox("Error", "Terrain file \""@ %newTerrainFile @ "\" is read-only.  Continue?", "Ok", "Stop") == $MROk)
               continue;
            else
            {
               %copyTerrainsFailed = true;
               break;
            }
         }
         
         if( !%terrainObject.save( %newTerrainFile ) )
         {
            error( "Failed to save '" @ %newTerrainFile @ "'" );
            %copyTerrainsFailed = true;
            break;
         }
         
         %terrainObject.terrainFile = %newTerrainFile;
      }

      ETerrainEditor.isDirty = false;
      
      // Save the mission.
      if(%copyTerrainsFailed || !EditorSaveMission())
      {
         // It failed, so restore the mission and terrain filenames.
         
         $Server::MissionFile = %saveMissionFile;

         initContainerTypeSearch( $TypeMasks::TerrainObjectType );
         for( %i = 0;; %i ++ )
         {
            %terrainObject = containerSearchNext();
            if( !%terrainObject )
               break;
               
            %terrainObject.terrainFile = %savedTerrNames.array[ %i ];
         }
      }
      
      %savedTerrNames.delete();
   }
   else
   {
      EditorSaveMissionMenuDisableSave();
   }
   
}

function EditorOpenMission(%filename)
{
   if( EditorIsDirty() && !isWebDemo() )
   {
      // "EditorSaveBeforeLoad();", "getLoadFilename(\"*.mis\", \"EditorDoLoadMission\");"
      if(MessageBox("Mission Modified", "Would you like to save changes to the current mission \"" @
         $Server::MissionFile @ "\" before opening a new mission?", SaveDontSave, Question) == $MROk)
      {
         if(! EditorSaveMission())
            return;
      }
   }

   if(%filename $= "")
   {
      %dlg = new OpenFileDialog()
      {
         Filters        = $Pref::WorldEditor::FileSpec;
         DefaultPath    = EditorSettings.value("LevelInformation/levelsDirectory");
         ChangePath     = false;
         MustExist      = true;
      };
            
      %ret = %dlg.Execute();
      if(%ret)
      {
         // Immediately override/set the levelsDirectory
         EditorSettings.setValue( "LevelInformation/levelsDirectory", collapseFilename(filePath( %dlg.FileName )) );
         %filename = %dlg.FileName;
      }
      
      %dlg.delete();
      
      if(! %ret)
         return;
   }
      
   // close the current editor, it will get cleaned up by MissionCleanup
   if( isObject( "Editor" ) )
      Editor.close( LoadingGui );

   EditorClearDirty();

   // If we haven't yet connnected, create a server now.
   // Otherwise just load the mission.

   if( !$missionRunning )
   {
      activatePackage( "BootEditor" );
      StartLevel( %filename );
   }
   else
   {
      loadMission( %filename, true ) ;
   
      pushInstantGroup();

      // recreate and open the editor
      Editor::create();
      MissionCleanup.add( Editor );
      MissionCleanup.add( Editor.getUndoManager() );
      EditorGui.loadingMission = true;
      Editor.open();
   
      popInstantGroup();
   }
}

function EditorExportToCollada()
{
   if ( !$Pref::disableSaving && !isWebDemo() )
   {
      %dlg = new SaveFileDialog()
      {
         Filters        = "COLLADA Files (*.dae)|*.dae|";
         DefaultPath    = $Pref::WorldEditor::LastPath;
         DefaultFile    = "";
         ChangePath     = false;
         OverwritePrompt   = true;
      };

      %ret = %dlg.Execute();
      if ( %ret )
      {
         $Pref::WorldEditor::LastPath = filePath( %dlg.FileName );
         %exportFile = %dlg.FileName;
      }

      if( fileExt( %exportFile ) !$= ".dae" )
         %exportFile = %exportFile @ ".dae";

      %dlg.delete();

      if ( !%ret )
         return;

      if ( EditorGui.currentEditor.getId() == ShapeEditorPlugin.getId() )
         ShapeEdShapeView.exportToCollada( %exportFile );
      else
         EWorldEditor.colladaExportSelection( %exportFile );
   }
}

function EditorMakePrefab()
{
   // Should this be protected or not?
   if ( !$Pref::disableSaving && !isWebDemo() )
   {
      %dlg = new SaveFileDialog()
      {
         Filters        = "Prefab Files (*.prefab)|*.prefab|";
         DefaultPath    = $Pref::WorldEditor::LastPath;
         DefaultFile    = "";
         ChangePath     = false;
         OverwritePrompt   = true;
      };
            
      %ret = %dlg.Execute();
      if ( %ret )
      {
         $Pref::WorldEditor::LastPath = filePath( %dlg.FileName );
         %saveFile = %dlg.FileName;
      }
      
      if( fileExt( %saveFile ) !$= ".prefab" )
         %saveFile = %saveFile @ ".prefab";
      
      %dlg.delete();
      
      if ( !%ret )
         return;
      
      EWorldEditor.makeSelectionPrefab( %saveFile );    
      
      EditorTree.buildVisibleTree( true );  
   }
}

function EditorExplodePrefab()
{
   //echo( "EditorExplodePrefab()" );  
   EWorldEditor.explodeSelectedPrefab();
   EditorTree.buildVisibleTree( true );
}

function EditorMount()
{
   echo( "EditorMount" );
   
   %size = EWorldEditor.getSelectionSize();
   if ( %size != 2 )
      return;
      
   %a = EWorldEditor.getSelectedObject(0);
   %b = EWorldEditor.getSelectedObject(1);
   
   //%a.mountObject( %b, 0 );
   EWorldEditor.mountRelative( %a, %b );
}

function EditorUnmount()
{
   echo( "EditorUnmount" );
   
   %obj = EWorldEditor.getSelectedObject(0);
   %obj.unmount();   
}

//////////////////////////////////////////////////////////////////////////
// View Menu Handlers
//////////////////////////////////////////////////////////////////////////

function EditorViewMenu::onMenuSelect( %this )
{
   %this.checkItem( 1, EWorldEditor.renderOrthoGrid );
}

//////////////////////////////////////////////////////////////////////////
// Edit Menu Handlers
//////////////////////////////////////////////////////////////////////////

function EditorEditMenu::onMenuSelect( %this )
{      
   // UndoManager is in charge of enabling or disabling the undo/redo items.
   Editor.getUndoManager().updateUndoMenu( %this );
   
   // SICKHEAD: It a perfect world we would abstract 
   // cut/copy/paste with a generic selection object 
   // which would know how to process itself.         
   
   // Give the active editor a chance at fixing up
   // the state of the edit menu.
   // Do we really need this check here?
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.onEditMenuSelect( %this );   
}

//////////////////////////////////////////////////////////////////////////

function EditorMenuEditDelete()
{
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.handleDelete();      
}

function EditorMenuEditDeselect()
{
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.handleDeselect();  
}

function EditorMenuEditCut()
{
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.handleCut();  
}

function EditorMenuEditCopy()
{
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.handleCopy();  
}

function EditorMenuEditPaste()
{
   if ( isObject( EditorGui.currentEditor ) )
      EditorGui.currentEditor.handlePaste();  
}



//////////////////////////////////////////////////////////////////////////
// Window Menu Handler
//////////////////////////////////////////////////////////////////////////

function EditorToolsMenu::onSelectItem(%this, %id)
{
   %toolName = getField( %this.item[%id], 2 );  

   EditorGui.setEditor(%toolName, %paletteName  );
   
   %this.checkRadioItem(0, %this.getItemCount(), %id);
   return true;
}

function EditorToolsMenu::setupDefaultState(%this)
{
   Parent::setupDefaultState(%this);
}

//////////////////////////////////////////////////////////////////////////
// Camera Menu Handler
//////////////////////////////////////////////////////////////////////////

function EditorCameraMenu::onSelectItem(%this, %id, %text)
{
   if(%id == 0 || %id == 1)
   {
      // Handle the Free Camera/Orbit Camera toggle
      %this.checkRadioItem(0, 1, %id);
   }

   return Parent::onSelectItem(%this, %id, %text);
}

function EditorCameraMenu::setupDefaultState(%this)
{
   // Set the Free Camera/Orbit Camera check marks
   %this.checkRadioItem(0, 1, 0);
   Parent::setupDefaultState(%this);
}

function EditorFreeCameraTypeMenu::onSelectItem(%this, %id, %text)
{
   // Handle the camera type radio
   %this.checkRadioItem(0, 2, %id);

   return Parent::onSelectItem(%this, %id, %text);
}

function EditorFreeCameraTypeMenu::setupDefaultState(%this)
{
   // Set the camera type check marks
   %this.checkRadioItem(0, 2, 0);
   Parent::setupDefaultState(%this);
}

function EditorCameraSpeedMenu::onSelectItem(%this, %id, %text)
{   
   // Grab and set speed
   %speed = getField( %this.item[%id], 2 ); 
   $Camera::movementSpeed = %speed;
   
   // Update Editor
   %this.checkRadioItem(0, 6, %id);
   
   // Update Toolbar TextEdit
   EWorldEditorCameraSpeed.setText( $Camera::movementSpeed );
   
   // Update Toolbar Slider
   CameraSpeedDropdownCtrlContainer-->Slider.setValue( $Camera::movementSpeed );
   
   return true;
}
function EditorCameraSpeedMenu::setupDefaultState(%this)
{
   // Setup camera speed gui's. Both menu and editorgui
   %this.setupGuiControls();
   
   //Grab and set speed
   %defaultSpeed = EditorSettings.value("LevelInformation/levels/" @ EditorGui.levelName @ "/cameraSpeed");
   if( %defaultSpeed $= "" )
   {
      // Update Editor with default speed
      %defaultSpeed = 25;
   }
   $Camera::movementSpeed = %defaultSpeed;
   
   // Update Toolbar TextEdit
   EWorldEditorCameraSpeed.setText( %defaultSpeed );
   
   // Update Toolbar Slider
   CameraSpeedDropdownCtrlContainer-->Slider.setValue( %defaultSpeed );
   
   Parent::setupDefaultState(%this);
}

function EditorCameraSpeedMenu::setupGuiControls(%this)
{
   // Default levelInfo params
   %minSpeed = 5;
   %maxSpeed = 200;
   
   %speedA = EditorSettings.value("LevelInformation/levels/" @ EditorGui.levelName @ "/cameraSpeedMin");
   %speedB = EditorSettings.value("LevelInformation/levels/" @ EditorGui.levelName @ "/cameraSpeedMax");
   if( %speedA < %speedB )
   {         
      if( %speedA == 0 )
      {
         if( %speedB > 1 )
            %minSpeed = 1;
         else
            %minSpeed = 0.1;
      }
      else
      {
         %minSpeed = %speedA;
      }

      %maxSpeed = %speedB;
   }
   
   // Set up the camera speed items
   %inc = ( (%maxSpeed - %minSpeed) / (%this.getItemCount() - 1) );
   for( %i = 0; %i < %this.getItemCount(); %i++)
      %this.item[%i] = setField( %this.item[%i], 2, (%minSpeed + (%inc * %i)));
   
   // Set up min/max camera slider range
   eval("CameraSpeedDropdownCtrlContainer-->Slider.range = \"" @ %minSpeed @ " " @ %maxSpeed @ "\";");
}
//////////////////////////////////////////////////////////////////////////
// World Menu Handler Object Menu
//////////////////////////////////////////////////////////////////////////

function EditorWorldMenu::onMenuSelect(%this)
{
   %selSize = EWorldEditor.getSelectionSize();
   %lockCount = EWorldEditor.getSelectionLockCount();
   %hideCount = EWorldEditor.getSelectionHiddenCount();
   
   %this.enableItem(0, %lockCount < %selSize);  // Lock Selection
   %this.enableItem(1, %lockCount > 0);  // Unlock Selection
   %this.enableItem(3, %hideCount < %selSize);  // Hide Selection
   %this.enableItem(4, %hideCount > 0);  // Show Selection
   %this.enableItem(6, %selSize > 1 && %lockCount == 0);  // Align bounds
   %this.enableItem(7, %selSize > 1 && %lockCount == 0);  // Align center
   %this.enableItem(9, %selSize > 0 && %lockCount == 0);  // Reset Transforms
   %this.enableItem(10, %selSize > 0 && %lockCount == 0);  // Reset Selected Rotation
   %this.enableItem(11, %selSize > 0 && %lockCount == 0);  // Reset Selected Scale
   %this.enableItem(12, %selSize > 0 && %lockCount == 0);  // Transform Selection
   %this.enableItem(14, %selSize > 0 && %lockCount == 0);  // Drop Selection
   
   %this.enableItem(17, %selSize > 0); // Make Prefab
   %this.enableItem(18, %selSize > 0); // Explode Prefab   
   
   %this.enableItem(20, %selSize > 1); // Mount
   %this.enableItem(21, %selSize > 0); // Unmount
}

//////////////////////////////////////////////////////////////////////////

function EditorDropTypeMenu::onSelectItem(%this, %id, %text)
{
   // This sets up which drop script function to use when
   // a drop type is selected in the menu.
   EWorldEditor.dropType = getField(%this.item[%id], 2);
   
   %this.checkRadioItem(0, (%this.getItemCount() - 1), %id);
   
   return true;
}

function EditorDropTypeMenu::setupDefaultState(%this)
{
   // Check the radio item for the currently set drop type.
   
   %numItems = %this.getItemCount();
   
   %dropTypeIndex = 0;
   for( ; %dropTypeIndex < %numItems; %dropTypeIndex ++ )
      if( getField( %this.item[ %dropTypeIndex ], 2 ) $= EWorldEditor.dropType )
         break;
 
   // Default to screenCenter if we didn't match anything.        
   if( %dropTypeIndex > (%numItems - 1) )
      %dropTypeIndex = 4;
   
   %this.checkRadioItem( 0, (%numItems - 1), %dropTypeIndex );
      
   Parent::setupDefaultState(%this);
}

//////////////////////////////////////////////////////////////////////////

function EditorAlignBoundsMenu::onSelectItem(%this, %id, %text)
{
   // Have the editor align all selected objects by the selected bounds.
   EWorldEditor.alignByBounds(getField(%this.item[%id], 2));
   
   return true;
}

function EditorAlignBoundsMenu::setupDefaultState(%this)
{
   // Allow the parent to set the menu's default state
   Parent::setupDefaultState(%this);
}

//////////////////////////////////////////////////////////////////////////

function EditorAlignCenterMenu::onSelectItem(%this, %id, %text)
{
   // Have the editor align all selected objects by the selected axis.
   EWorldEditor.alignByAxis(getField(%this.item[%id], 2));
   
   return true;
}

function EditorAlignCenterMenu::setupDefaultState(%this)
{
   // Allow the parent to set the menu's default state
   Parent::setupDefaultState(%this);
}
