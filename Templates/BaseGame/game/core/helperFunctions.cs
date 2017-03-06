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

// A function used in order to easily parse the MissionGroup for classes . I'm pretty 
// sure at this point the function can be easily modified to search the any group as well.
function parseMissionGroup( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = "MissionGroup";
   else
      %currentGroup = %childGroup;
      
   for(%i = 0; %i < (%currentGroup).getCount(); %i++)
   {      
      if( (%currentGroup).getObject(%i).getClassName() $= %className )
         return true;
      
      if( (%currentGroup).getObject(%i).getClassName() $= "SimGroup" )
      {
         if( parseMissionGroup( %className, (%currentGroup).getObject(%i).getId() ) )
            return true;         
      }
   } 
}

// A variation of the above used to grab ids from the mission group based on classnames
function parseMissionGroupForIds( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = "MissionGroup";
   else
      %currentGroup = %childGroup;
      
   for(%i = 0; %i < (%currentGroup).getCount(); %i++)
   {      
      if( (%currentGroup).getObject(%i).getClassName() $= %className )
         %classIds = %classIds @ (%currentGroup).getObject(%i).getId() @ " ";
      
      if( (%currentGroup).getObject(%i).getClassName() $= "SimGroup" )
         %classIds = %classIds @ parseMissionGroupForIds( %className, (%currentGroup).getObject(%i).getId());
   } 
   return trim( %classIds );
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