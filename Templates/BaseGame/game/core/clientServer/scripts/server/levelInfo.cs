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
// Loading info is text displayed on the client side while the mission
// is being loaded.  This information is extracted from the mission file
// and sent to each the client as it joins.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// clearLoadInfo
//
// Clears the mission info stored
//------------------------------------------------------------------------------
function clearLoadInfo() 
{
   if (isObject(theLevelInfo))
      theLevelInfo.delete();
}

//------------------------------------------------------------------------------
// buildLoadInfo
//
// Extract the map description from the .mis file
//------------------------------------------------------------------------------
function buildLoadInfo( %mission ) 
{
	clearLoadInfo();

	%infoObject = "";
	%file = new FileObject();

	if ( %file.openForRead( %mission ) ) {
		%inInfoBlock = false;
		
		while ( !%file.isEOF() ) {
			%line = %file.readLine();
			%line = trim( %line );
			
			if( %line $= "new ScriptObject(MissionInfo) {" )
				%inInfoBlock = true;
         else if( %line $= "new LevelInfo(theLevelInfo) {" )
				%inInfoBlock = true;
			else if( %inInfoBlock && %line $= "};" ) {
				%inInfoBlock = false;
				%infoObject = %infoObject @ %line; 
				break;
			}
			
			if( %inInfoBlock )
			   %infoObject = %infoObject @ %line @ " ";
		}
		
		%file.close();
	}
	else
	   error("Level file " @ %mission @ " not found.");

   // Will create the object "MissionInfo"
	eval( %infoObject );
	%file.delete();
}

//------------------------------------------------------------------------------
// dumpLoadInfo
//
// Echo the mission information to the console
//------------------------------------------------------------------------------
function dumpLoadInfo()
{
	echo( "Level Name: " @ theLevelInfo.name );
   echo( "Level Description:" );
   
   for( %i = 0; theLevelInfo.desc[%i] !$= ""; %i++ )
      echo ("   " @ theLevelInfo.desc[%i]);
}

//------------------------------------------------------------------------------
// sendLoadInfoToClient
//
// Sends mission description to the client
//------------------------------------------------------------------------------
function sendLoadInfoToClient( %client )
{
   messageClient( %client, 'MsgLoadInfo', "", theLevelInfo.levelName );

   // Send Mission Description a line at a time
   for( %i = 0; theLevelInfo.desc[%i] !$= ""; %i++ )
     messageClient( %client, 'MsgLoadDescripition', "", theLevelInfo.desc[%i] );

   messageClient( %client, 'MsgLoadInfoDone' );
}

// A function used in order to easily parse the MissionGroup for classes . I'm pretty 
// sure at this point the function can be easily modified to search the any group as well.
function parseMissionGroup( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = getScene(0);
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

//
function parseMissionGroupForIds( %className, %childGroup )
{
   if( getWordCount( %childGroup ) == 0)
      %currentGroup = getScene(0);
   else
      %currentGroup = %childGroup;
      
   for(%i = 0; %i < (%currentGroup).getCount(); %i++)
   {      
      if( (%currentGroup).getObject(%i).getClassName() $= %className )
         %classIds = %classIds @ (%currentGroup).getObject(%i).getId() @ " ";
      
      if( (%currentGroup).getObject(%i).getClassName() $= "SimGroup" )
         %classIds = %classIds @ parseMissionGroupForIds( %className, (%currentGroup).getObject(%i).getId());
   } 
   return %classIds;
}

function getLevelInfo( %missionFile ) 
{
   clearLoadInfo();
   
   %file = new FileObject();
   
   %LevelInfoObject = "";
   
   if ( %file.openForRead( %missionFile ) ) {
		%inInfoBlock = false;
		
		while ( !%file.isEOF() ) {
			%line = %file.readLine();
			%line = trim( %line );
			
			if( %line $= "new ScriptObject(LevelInfo) {" )
				%inInfoBlock = true;
         else if( %line $= "new LevelInfo(theLevelInfo) {" )
				%inInfoBlock = true;
			else if( %inInfoBlock && %line $= "};" ) {
				%inInfoBlock = false;
				%LevelInfoObject = %LevelInfoObject @ %line; 
				break;
			}
			
			if( %inInfoBlock )
			   %LevelInfoObject = %LevelInfoObject @ %line @ " "; 	
		}
		
		%file.close();
	}
   %file.delete();

	if( %LevelInfoObject !$= "" )
	{
	   %LevelInfoObject = "%LevelInfoObject = " @ %LevelInfoObject;
	   eval( %LevelInfoObject );

      return %LevelInfoObject;
	}
	
	// Didn't find our LevelInfo
   return 0; 
}