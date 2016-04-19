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
      if ( %lastFailures > %failedDatablocks )
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

function getPrefpath()
{
	%temp = getUserHomeDirectory();  
	echo(%temp);  
	if(!isDirectory(%temp))  
	{  
		%temp = getUserDataDirectory();  
		echo(%temp);
		if(!isDirectory(%temp)) 
		{
			$prefpath = "data";  
		}
		else  
		{
			//put it in appdata/roaming
			$prefpath = %temp @ "/" @ $appName @ "/preferences";  
		}  
	}  
	else  
	{  
		//put it in user/documents  
		$prefPath = %temp @ "/" @ $appName @ "/preferences";  
	}
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