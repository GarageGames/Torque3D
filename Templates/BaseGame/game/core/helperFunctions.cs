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


//------------------------------------------------------------------------------
// Check if a script file exists, compiled or not.
function isScriptFile(%path)
{
   if( isFile(%path @ ".dso") || isFile(%path) )
      return true;
   
   return false;
}

function loadMaterials()
{
   // Load any materials files for which we only have DSOs.

   for( %file = findFirstFile( "*/materials.cs.dso" );
        %file !$= "";
        %file = findNextFile( "*/materials.cs.dso" ))
   {
      // Only execute, if we don't have the source file.
      %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
      if( !isFile( %csFileName ) )
         exec( %csFileName );
   }

   // Load all source material files.

   for( %file = findFirstFile( "*/materials.cs" );
        %file !$= "";
        %file = findNextFile( "*/materials.cs" ))
   {
      exec( %file );
   }

   // Load all materials created by the material editor if
   // the folder exists
   if( IsDirectory( "materialEditor" ) )
   {
      for( %file = findFirstFile( "materialEditor/*.cs.dso" );
           %file !$= "";
           %file = findNextFile( "materialEditor/*.cs.dso" ))
      {
         // Only execute, if we don't have the source file.
         %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
         if( !isFile( %csFileName ) )
            exec( %csFileName );
      }

      for( %file = findFirstFile( "materialEditor/*.cs" );
           %file !$= "";
           %file = findNextFile( "materialEditor/*.cs" ))
      {
         exec( %file );
      }
   }
}

function reloadMaterials()
{
   reloadTextures();
   loadMaterials();
   reInitMaterials();
}

function loadDatablockFiles( %datablockFiles, %recurse )
{
   if ( %recurse )
   {
      recursiveLoadDatablockFiles( %datablockFiles, 9999 );
      return;
   }
   
   %count = %datablockFiles.count();
   for ( %i=0; %i < %count; %i++ )
   {
      %file = %datablockFiles.getKey( %i );
      if ( !isFile(%file @ ".dso") && !isFile(%file) )
         continue;
                  
      exec( %file );
   }
      
   // Destroy the incoming list.
   //%datablockFiles.delete();
}

function recursiveLoadDatablockFiles( %datablockFiles, %previousErrors )
{
   %reloadDatablockFiles = new ArrayObject();

   // Keep track of the number of datablocks that 
   // failed during this pass.
   %failedDatablocks = 0;
   
   // Try re-executing the list of datablock files.
   %count = %datablockFiles.count();
   for ( %i=0; %i < %count; %i++ )
   {      
      %file = %datablockFiles.getKey( %i );
      if ( !isFile(%file @ ".dso") && !isFile(%file) )
         continue;
         
      // Start counting copy constructor creation errors.
      $Con::objectCopyFailures = 0;
                                       
      exec( %file );
                                    
      // If errors occured then store this file for re-exec later.
      if ( $Con::objectCopyFailures > 0 )
      {
         %reloadDatablockFiles.add( %file );
         %failedDatablocks = %failedDatablocks + $Con::objectCopyFailures;
      }
   }
            
   // Clear the object copy failure counter so that
   // we get console error messages again.
   $Con::objectCopyFailures = -1;
                  
   // Delete the old incoming list... we're done with it.
   //%datablockFiles.delete();
               
   // If we still have datablocks to retry.
   %newCount = %reloadDatablockFiles.count();
   if ( %newCount > 0 )
   {
      // If the datablock failures have not been reduced
      // from the last pass then we must have a real syntax
      // error and not just a bad dependancy.         
      if ( %previousErrors > %failedDatablocks )
         recursiveLoadDatablockFiles( %reloadDatablockFiles, %failedDatablocks );
                  
      else
      {      
         // Since we must have real syntax errors do one 
         // last normal exec to output error messages.
         loadDatablockFiles( %reloadDatablockFiles, false );
      }
      
      return;
   }
                  
   // Cleanup the empty reload list.
   %reloadDatablockFiles.delete();         
}

function getUserPath()
{
	%temp = getUserHomeDirectory();  
	echo(%temp);  
	if(!isDirectory(%temp))  
	{  
		%temp = getUserDataDirectory();  
		echo(%temp);
		if(!isDirectory(%temp)) 
		{
			%userPath = "data";  
		}
		else  
		{
			//put it in appdata/roaming
			%userPath = %temp @ "/" @ $appName;  
		}  
	}  
	else  
	{  
		//put it in user/documents  
		%userPath = %temp @ "/" @ $appName;  
	}
	return %userPath;
}

function getPrefpath()
{
   $prefPath = getUserPath() @ "/preferences";
	return $prefPath;
}

function updateTSShapeLoadProgress(%progress, %msg)
{
   // Check if the loading GUI is visible and use that instead of the
   // separate import progress GUI if possible
   if ( isObject(LoadingGui) && LoadingGui.isAwake() )
   {
      // Save/Restore load progress at the start/end of the import process
      if ( %progress == 0 )
      {
         ColladaImportProgress.savedProgress = LoadingProgress.getValue();
         ColladaImportProgress.savedText = LoadingProgressTxt.getValue();

         ColladaImportProgress.msgPrefix = "Importing " @ %msg;
         %msg = "Reading file into memory...";
      }
      else if ( %progress == 1.0 )
      {
         LoadingProgress.setValue( ColladaImportProgress.savedProgress );
         LoadingProgressTxt.setValue( ColladaImportProgress.savedText );
      }

      %msg = ColladaImportProgress.msgPrefix @ ": " @ %msg;

      %progressCtrl = LoadingProgress;
      %textCtrl = LoadingProgressTxt;
   }
   else
   {
      //it's probably the editors using it
      if(isFunction("updateToolTSShapeLoadProgress"))
      {
         updateToolTSShapeLoadProgress(%progress, %msg);
      }
   }

   // Update progress indicators
   if (%progress == 0)
   {
      %progressCtrl.setValue(0.001);
      %textCtrl.setText(%msg);
   }
   else if (%progress != 1.0)
   {
      %progressCtrl.setValue(%progress);
      %textCtrl.setText(%msg);
   }

   Canvas.repaint(33);
}

/// A helper function which will return the ghosted client object
/// from a server object when connected to a local server.
function serverToClientObject( %serverObject )
{
   assert( isObject( LocalClientConnection ), "serverToClientObject() - No local client connection found!" );
   assert( isObject( ServerConnection ), "serverToClientObject() - No server connection found!" );      
         
   %ghostId = LocalClientConnection.getGhostId( %serverObject );
   if ( %ghostId == -1 )
      return 0;
                
   return ServerConnection.resolveGhostID( %ghostId );   
}

//----------------------------------------------------------------------------
// Debug commands
//----------------------------------------------------------------------------

function netSimulateLag( %msDelay, %packetLossPercent )
{
   if ( %packetLossPercent $= "" )
      %packetLossPercent = 0;
                  
   commandToServer( 'NetSimulateLag', %msDelay, %packetLossPercent );
}

//Various client functions

function validateDatablockName(%name)
{
   // remove whitespaces at beginning and end
   %name = trim( %name );
   
   // remove numbers at the beginning
   %numbers = "0123456789";   
   while( strlen(%name) > 0 )
   {
      // the first character
      %firstChar = getSubStr( %name, 0, 1 );
      // if the character is a number remove it
      if( strpos( %numbers, %firstChar ) != -1 )
      {
         %name = getSubStr( %name, 1, strlen(%name) -1 );
         %name = ltrim( %name );
      }
      else
         break;
   }
   
   // replace whitespaces with underscores
   %name = strreplace( %name, " ", "_" );
   
   // remove any other invalid characters
   %invalidCharacters = "-+*/%$&§=()[].?\"#,;!~<>|°^{}";
   %name = stripChars( %name, %invalidCharacters );
   
   if( %name $= "" )
      %name = "Unnamed";
   
   return %name;
}

//--------------------------------------------------------------------------
// Finds location of %word in %text, starting at %start.  Works just like strPos
//--------------------------------------------------------------------------

function wordPos(%text, %word, %start)
{
   if (%start $= "") %start = 0;
   
   if (strpos(%text, %word, 0) == -1) return -1;
   %count = getWordCount(%text);
   if (%start >= %count) return -1;
   for (%i = %start; %i < %count; %i++)
   {
      if (getWord( %text, %i) $= %word) return %i;
   }
   return -1;
}

//--------------------------------------------------------------------------
// Finds location of %field in %text, starting at %start.  Works just like strPos
//--------------------------------------------------------------------------

function fieldPos(%text, %field, %start)
{
   if (%start $= "") %start = 0;
   
   if (strpos(%text, %field, 0) == -1) return -1;
   %count = getFieldCount(%text);
   if (%start >= %count) return -1;
   for (%i = %start; %i < %count; %i++)
   {
      if (getField( %text, %i) $= %field) return %i;
   }
   return -1;
}

//--------------------------------------------------------------------------
// returns the text in a file with "\n" at the end of each line
//--------------------------------------------------------------------------

function loadFileText( %file)
{
   %fo = new FileObject();
   %fo.openForRead(%file);
   %text = "";
   while(!%fo.isEOF())
   {
      %text = %text @ %fo.readLine();
      if (!%fo.isEOF()) %text = %text @ "\n";
   }

   %fo.delete();
   return %text;
}

function setValueSafe(%dest, %val)
{
   %cmd = %dest.command;
   %alt = %dest.altCommand;
   %dest.command = "";
   %dest.altCommand = "";

   %dest.setValue(%val);
   
   %dest.command = %cmd;
   %dest.altCommand = %alt;
}

function shareValueSafe(%source, %dest)
{
   setValueSafe(%dest, %source.getValue());
}

function shareValueSafeDelay(%source, %dest, %delayMs)
{
   schedule(%delayMs, 0, shareValueSafe, %source, %dest);
}


//------------------------------------------------------------------------------
// An Aggregate Control is a plain GuiControl that contains other controls, 
// which all share a single job or represent a single value.
//------------------------------------------------------------------------------

// AggregateControl.setValue( ) propagates the value to any control that has an 
// internal name.
function AggregateControl::setValue(%this, %val, %child)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if( %obj == %child )
         continue;
         
      if(%obj.internalName !$= "")
         setValueSafe(%obj, %val);
   }
}

// AggregateControl.getValue() uses the value of the first control that has an
// internal name, if it has not cached a value via .setValue
function AggregateControl::getValue(%this)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if(%obj.internalName !$= "")
      {
         //error("obj = " @ %obj.getId() @ ", " @ %obj.getName() @ ", " @ %obj.internalName );
         //error(" value = " @ %obj.getValue());
         return %obj.getValue();
      }
   }
}

// AggregateControl.updateFromChild( ) is called by child controls to propagate
// a new value, and to trigger the onAction() callback.
function AggregateControl::updateFromChild(%this, %child)
{
   %val = %child.getValue();
   if(%val == mCeil(%val)){
      %val = mCeil(%val);
   }else{
      if ( %val <= -100){
         %val = mCeil(%val);
      }else if ( %val <= -10){
         %val = mFloatLength(%val, 1);
      }else if ( %val < 0){
         %val = mFloatLength(%val, 2);
      }else if ( %val >= 1000){
         %val = mCeil(%val);
      }else if ( %val >= 100){
         %val = mFloatLength(%val, 1);
      }else if ( %val >= 10){
         %val = mFloatLength(%val, 2);
      }else if ( %val > 0){
         %val = mFloatLength(%val, 3);
      }
   }
   %this.setValue(%val, %child);
   %this.onAction();
}

// default onAction stub, here only to prevent console spam warnings.
function AggregateControl::onAction(%this) 
{
}

// call a method on all children that have an internalName and that implement the method.
function AggregateControl::callMethod(%this, %method, %args)
{
   for(%i = 0; %i < %this.getCount(); %i++)
   {
      %obj = %this.getObject(%i);
      if(%obj.internalName !$= "" && %obj.isMethod(%method))
         eval(%obj @ "." @ %method @ "( " @ %args @ " );");
   }

}

//------------------------------------------------------------------------------
// Altered Version of TGB's QuickEditDropDownTextEditCtrl
//------------------------------------------------------------------------------
function QuickEditDropDownTextEditCtrl::onRenameItem( %this )
{
}

function QuickEditDropDownTextEditCtrl::updateFromChild( %this, %ctrl )
{
   if( %ctrl.internalName $= "PopUpMenu" )
   {
      %this->TextEdit.setText( %ctrl.getText() );
   }
   else if ( %ctrl.internalName $= "TextEdit" )
   {
      %popup = %this->PopupMenu;
      %popup.changeTextById( %popup.getSelected(), %ctrl.getText() );
      %this.onRenameItem();
   }
}

// Writes out all script functions to a file.
function writeOutFunctions()
{
   new ConsoleLogger(logger, "scriptFunctions.txt", false);
   dumpConsoleFunctions();
   logger.delete();
}

// Writes out all script classes to a file.
function writeOutClasses()
{
   new ConsoleLogger(logger, "scriptClasses.txt", false);
   dumpConsoleClasses();
   logger.delete();
}

//
function compileFiles(%pattern)
{  
   %path = filePath(%pattern);

   %saveDSO    = $Scripts::OverrideDSOPath;
   %saveIgnore = $Scripts::ignoreDSOs;
   
   $Scripts::OverrideDSOPath  = %path;
   $Scripts::ignoreDSOs       = false;
   %mainCsFile = makeFullPath("main.cs");

   for (%file = findFirstFileMultiExpr(%pattern); %file !$= ""; %file = findNextFileMultiExpr(%pattern))
   {
      // we don't want to try and compile the primary main.cs
      if(%mainCsFile !$= %file)      
         compile(%file, true);
   }

   $Scripts::OverrideDSOPath  = %saveDSO;
   $Scripts::ignoreDSOs       = %saveIgnore;
   
}

function displayHelp() 
{
   // Notes on logmode: console logging is written to console.log.
   // -log 0 disables console logging.
   // -log 1 appends to existing logfile; it also closes the file
   // (flushing the write buffer) after every write.
   // -log 2 overwrites any existing logfile; it also only closes
   // the logfile when the application shuts down.  (default)

   error(
      "Torque Demo command line options:\n"@
      "  -log <logmode>         Logging behavior; see main.cs comments for details\n"@
      "  -game <game_name>      Reset list of mods to only contain <game_name>\n"@
      "  <game_name>            Works like the -game argument\n"@
      "  -dir <dir_name>        Add <dir_name> to list of directories\n"@
      "  -console               Open a separate console\n"@
      "  -jSave  <file_name>    Record a journal\n"@
      "  -jPlay  <file_name>    Play back a journal\n"@
      "  -help                  Display this help message\n"
   );
}

// Execute startup scripts for each mod, starting at base and working up
function loadDir(%dir)
{
   pushback($userDirs, %dir, ";");

   if (isScriptFile(%dir @ "/main.cs"))
   exec(%dir @ "/main.cs");
}

function loadDirs(%dirPath)
{
   %dirPath = nextToken(%dirPath, token, ";");
   if (%dirPath !$= "")
      loadDirs(%dirPath);

   if(exec(%token @ "/main.cs") != true)
   {
      error("Error: Unable to find specified directory: " @ %token );
      $dirCount--;
   }
}

//------------------------------------------------------------------------------
// Utility remap functions:
//------------------------------------------------------------------------------

function ActionMap::copyBind( %this, %otherMap, %command )
{
   if ( !isObject( %otherMap ) )
   {
      error( "ActionMap::copyBind - \"" @ %otherMap @ "\" is not an object!" );
      return;
   }

   %bind = %otherMap.getBinding( %command );
   if ( %bind !$= "" )
   {
      %device = getField( %bind, 0 );
      %action = getField( %bind, 1 );
      %flags = %otherMap.isInverted( %device, %action ) ? "SDI" : "SD";
      %deadZone = %otherMap.getDeadZone( %device, %action );
      %scale = %otherMap.getScale( %device, %action );
      %this.bind( %device, %action, %flags, %deadZone, %scale, %command );
   }
}

//------------------------------------------------------------------------------
function ActionMap::blockBind( %this, %otherMap, %command )
{
   if ( !isObject( %otherMap ) )
   {
      error( "ActionMap::blockBind - \"" @ %otherMap @ "\" is not an object!" );
      return;
   }

   %bind = %otherMap.getBinding( %command );
   if ( %bind !$= "" )
      %this.bind( getField( %bind, 0 ), getField( %bind, 1 ), "" );
}

//Dev helpers
/// Shortcut for typing dbgSetParameters with the default values torsion uses.
function dbgTorsion()
{
   dbgSetParameters( 6060, "password", false );
}

/// Reset the input state to a default of all-keys-up.
/// A helpful remedy for when Torque misses a button up event do to your breakpoints
/// and can't stop shooting / jumping / strafing.
function mvReset()
{
   for ( %i = 0; %i < 6; %i++ )
      setVariable( "mvTriggerCount" @ %i, 0 );
      
   $mvUpAction = 0;
   $mvDownAction = 0;
   $mvLeftAction = 0;
   $mvRightAction = 0;
   
   // There are others.
}

//Persistance Manager tests

new PersistenceManager(TestPManager);

function runPManTest(%test)
{
   if (!isObject(TestPManager))
      return;

   if (%test $= "")
      %test = 100;

   switch(%test)
   {
      case 0:
         TestPManager.testFieldUpdates();
      case 1:
         TestPManager.testObjectRename();
      case 2:
         TestPManager.testNewObject();
      case 3:
         TestPManager.testNewGroup();
      case 4:
         TestPManager.testMoveObject();
      case 5:
         TestPManager.testObjectRemove();
      case 100:
         TestPManager.testFieldUpdates();
         TestPManager.testObjectRename();
         TestPManager.testNewObject();
         TestPManager.testNewGroup();
         TestPManager.testMoveObject();
         TestPManager.testObjectRemove();
   }
}

function TestPManager::testFieldUpdates(%doNotSave)
{
   // Set some objects as dirty
   TestPManager.setDirty(AudioGui);
   TestPManager.setDirty(AudioSim);
   TestPManager.setDirty(AudioMessage);

   // Alter some of the existing fields
   AudioEffect.isLooping         = true;
   AudioMessage.isLooping     = true;
   AudioEffect.is3D              = true;

   // Test removing a field
   TestPManager.removeField(AudioGui, "isLooping");

   // Alter some of the persistent fields
   AudioGui.referenceDistance     = 0.8;
   AudioMessage.referenceDistance = 0.8;

   // Add some new dynamic fields
   AudioGui.foo = "bar";
   AudioEffect.foo = "bar";

   // Remove an object from the dirty list
   // It shouldn't get updated in the file
   TestPManager.removeDirty(AudioEffect);

   // Dirty an object in another file as well
   TestPManager.setDirty(WarningMaterial);

   // Update a field that doesn't exist
   WarningMaterial.glow[0] = true;

   // Drity another object to test for crashes
   // when a dirty object is deleted
   TestPManager.setDirty(SFXPausedSet);

   // Delete the object
   SFXPausedSet.delete();

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testObjectRename(%doNotSave)
{
   // Flag an object as dirty
   if (isObject(AudioGui))
      TestPManager.setDirty(AudioGui);
   else if (isObject(AudioGuiFoo))
      TestPManager.setDirty(AudioGuiFoo);

   // Rename it
   if (isObject(AudioGui))
      AudioGui.setName(AudioGuiFoo);
   else if (isObject(AudioGuiFoo))
      AudioGuiFoo.setName(AudioGui);

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testNewObject(%doNotSave)
{
   // Test adding a new named object
   new SFXDescription(AudioNew)
   {
      volume = 0.5;
      isLooping = true;
      channel  = $GuiAudioType;
      foo = 2;
   };

   // Flag it as dirty
   TestPManager.setDirty(AudioNew, "core/scripts/client/audio.cs");

   // Test adding a new unnamed object
   %obj = new SFXDescription()
   {
      volume = 0.75;
      isLooping = true;
      bar = 3;
   };

   // Flag it as dirty
   TestPManager.setDirty(%obj, "core/scripts/client/audio.cs");

   // Test adding an "empty" object
   new SFXDescription(AudioEmpty);

   TestPManager.setDirty(AudioEmpty, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testNewGroup(%doNotSave)
{
   // Test adding a new named SimGroup
   new SimGroup(TestGroup)
   {
      foo = "bar";

      new SFXDescription(TestObject)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 1;
      };
      new SimGroup(SubGroup)
      {
         foo = 2;

         new SFXDescription(SubObject)
         {
            volume = 0.5;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 3;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(TestGroup, "core/scripts/client/audio.cs");

   // Test adding a new unnamed SimGroup
   %group = new SimGroup()
   {
      foo = "bar";

      new SFXDescription()
      {
         volume = 0.75;
         channel  = $GuiAudioType;
         foo = 4;
      };
      new SimGroup()
      {
         foo = 5;

         new SFXDescription()
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 6;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(%group, "core/scripts/client/audio.cs");

   // Test adding a new unnamed SimSet
   %set = new SimSet()
   {
      foo = "bar";

      new SFXDescription()
      {
         volume = 0.75;
         channel  = $GuiAudioType;
         foo = 7;
      };
      new SimGroup()
      {
         foo = 8;

         new SFXDescription()
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 9;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(%set, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testMoveObject(%doNotSave)
{
   // First add a couple of groups to the file
   new SimGroup(MoveGroup1)
   {
      foo = "bar";

      new SFXDescription(MoveObject1)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 1;
      };

      new SimSet(SubGroup1)
      {
         new SFXDescription(SubObject1)
         {
            volume = 0.75;
            isLooping = true;
            channel  = $GuiAudioType;
            foo = 2;
         };
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(MoveGroup1, "core/scripts/client/audio.cs");

   new SimGroup(MoveGroup2)
   {
      foo = "bar";

      new SFXDescription(MoveObject2)
      {
         volume = 0.5;
         isLooping = true;
         channel  = $GuiAudioType;
         foo = 3;
      };
   };

   // Flag this as dirty
   TestPManager.setDirty(MoveGroup2, "core/scripts/client/audio.cs");

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();

   // Set them as dirty again
   TestPManager.setDirty(MoveGroup1);
   TestPManager.setDirty(MoveGroup2);

   // Give the subobject an new value
   MoveObject1.foo = 4;

   // Move it into the other group
   MoveGroup1.add(MoveObject2);

   // Switch the other subobject
   MoveGroup2.add(MoveObject1);

   // Also add a new unnamed object to one of the groups
   %obj = new SFXDescription()
   {
      volume = 0.75;
      isLooping = true;
      bar = 5;
   };

   MoveGroup1.add(%obj);

   // Unless %doNotSave is set (by a batch/combo test)
   // then go ahead and save now
   if (!%doNotSave)
      TestPManager.saveDirty();
}

function TestPManager::testObjectRemove(%doNotSave)
{
   TestPManager.removeObjectFromFile(AudioSim);
}

