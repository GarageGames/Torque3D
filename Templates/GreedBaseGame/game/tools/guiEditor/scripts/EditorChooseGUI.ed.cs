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

function GE_ReturnToMainMenu()
{
   loadMainMenu();
}

function GE_OpenGUIFile()
{
   %openFileName = GuiBuilder::getOpenName();
   if( %openFileName $= "" )
      return;   

   // Make sure the file is valid.
   if ((!isFile(%openFileName)) && (!isFile(%openFileName @ ".dso")))
      return;
      
   // Allow stomping objects while exec'ing the GUI file as we want to
   // pull the file's objects even if we have another version of the GUI
   // already loaded.
   
   %oldRedefineBehavior = $Con::redefineBehavior;
   $Con::redefineBehavior = "replaceExisting";
   
   // Load up the level.
   exec( %openFileName );
   
   $Con::redefineBehavior = %oldRedefineBehavior;
   
   // The level file should have contained a scenegraph, which should now be in the instant
   // group. And, it should be the only thing in the group.
   if( !isObject( %guiContent ) )
   {
      MessageBox( getEngineName(),
         "You have loaded a Gui file that was created before this version.  It has been loaded but you must open it manually from the content list dropdown",
         "Ok", "Information" );   
      GuiEditContent( Canvas.getContent() );
      return 0;
   }

   GuiEditContent( %guiContent );
}

function GE_GUIList::onURL(%this, %url)
{
   // Remove 'gamelink:' from front
   %gui = getSubStr(%url, 9, 1024);
   GuiEditContent(%gui);
}

function EditorChooseGUI::onWake()
{
   // Build the text list
   GE_GUIList.clear();

   %list = "<linkcolor:0000FF><linkcolorhl:FF0000>";
   %list = GE_ScanGroupForGuis(GuiGroup, %list);
   GE_GUIList.setText(%list);
   GE_GUIList.forceReflow();
   GE_GUIList.scrollToTop();
}   

function GE_ScanGroupForGuis(%group, %text)
{
   %count = %group.getCount();
   for(%i=0; %i < %count; %i++)
   {
      %obj = %group.getObject(%i);
      if(%obj.getClassName() $= "GuiCanvas")
      {
         %text = %text @ GE_ScanGroupForGuis(%obj, %text);
      }
      else 
      {
         if(%obj.getName() $= "")
            %name = "(unnamed) - " @ %obj;
         else
            %name = %obj.getName() @ " - " @ %obj;

         %text = %text @ "<a:gamelink:" @ %obj @ ">" @ %name @ "</a><br>";
      }
   }

   return %text;
}
