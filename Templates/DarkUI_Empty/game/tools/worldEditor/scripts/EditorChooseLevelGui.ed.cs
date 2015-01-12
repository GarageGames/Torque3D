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

function EditorChooseLevelGui::onWake()
{
   // first check if we have a level file to load, then we'll bypass this
   if ($levelToLoad !$= "")
   {      
      // First try using the file path raw... it may already be good.
      %file = findFirstFile( $levelToLoad );
      if ( %file $= "" )
      {
         %levelFile = "levels/";
         %ext = getSubStr($levelToLoad, strlen($levelToLoad) - 3, 3);
         if(%ext !$= "mis")
            %levelFile = %levelFile @ $levelToLoad @ ".mis";
         else
            %levelFile = %levelFile @ $levelToLoad;
                       
         // let's make sure the file exists
         %file = findFirstFile(%levelFile);
      }

      // Clear out the $levelToLoad so we don't attempt to load the level again
      // later on.
      $levelToLoad = "";

      if(%file !$= "")
      {
         WE_EditLevel(%file);
         return;      
      }
   }
   
   //If no valid name, then push the level chooser
   Canvas.pushDialog(EditorChooseLevelContainer);
}  
 
function EditorChooseLevelContainer::onWake(%this)
{
   // Build the text lists
   WE_LevelList.clear();
   WE_TemplateList.clear();

   %leveltext = "<linkcolor:0000FF><linkcolorhl:FF0000>";
   %templatetext = "<linkcolor:0000FF><linkcolorhl:FF0000>";
   for(%file = findFirstFile($Server::MissionFileSpec); %file !$= ""; %file = findNextFile($Server::MissionFileSpec))
   {
      %name = getLevelDisplayName(%file);
      %n = strlwr(%name);
      if(strstr(%n, "template") == -1)
      {
         %leveltext = %leveltext @ "<a:gamelink:" @ %file @ ">" @ %name @ "</a><br>";
      }
      else
      {
         %templatetext = %templatetext @ "<a:gamelink:" @ %file @ ">" @ %name @ "</a><br>";
      }
   }

   WE_LevelList.setText(%leveltext);
   WE_LevelList.forceReflow();
   WE_LevelList.scrollToTop();

   WE_TemplateList.setText(%templatetext);
   WE_TemplateList.forceReflow();
   WE_TemplateList.scrollToTop();
}

function WE_EditLevel(%levelFile)
{
   EditorOpenMission( %levelFile );
}

function WE_ReturnToMainMenu()
{
   loadMainMenu();
}

function WE_LevelList::onURL(%this, %url)
{
   // Remove 'gamelink:' from front
   %levelFile = getSubStr(%url, 9, 1024);
   WE_EditLevel(%levelFile);
}

function WE_TemplateList::onURL(%this, %url)
{
   // Remove 'gamelink:' from front
   %levelFile = getSubStr(%url, 9, 1024);
   WE_EditLevel(%levelFile);
   EditorGui.saveAs = true;
}

function getLevelDisplayName( %levelFile ) 
{
   %file = new FileObject();
   
   %MissionInfoObject = "";
   
   if ( %file.openForRead( %levelFile ) ) {
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
				%MissionInfoObject = %MissionInfoObject @ %line; 
				break;
			}
			
			if( %inInfoBlock )
			   %MissionInfoObject = %MissionInfoObject @ %line @ " "; 	
		}
		
		%file.close();
	}
	%MissionInfoObject = "%MissionInfoObject = " @ %MissionInfoObject;
	eval( %MissionInfoObject );
	
   %file.delete();
   if( %MissionInfoObject.levelName !$= "" )
      %name = %MissionInfoObject.levelName;
   else
      %name = fileBase(%levelFile); 
      
   %MissionInfoObject.delete();
   
   return %name;
}
