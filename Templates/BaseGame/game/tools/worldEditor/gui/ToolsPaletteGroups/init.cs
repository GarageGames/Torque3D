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

function EWToolsPaletteWindow::loadToolsPalettes()
{
   %filespec = "tools/worldEditor/gui/ToolsPaletteGroups/*.ed.gui";
   
   // were executing each gui file and adding them to the ToolsPaletteArray
   for( %file = findFirstFile(%filespec); %file !$= ""; %file = findNextFile(%filespec))
   {
      exec( %file );
      %paletteGroup = 0;      
      
      %i = %paletteId.getCount();
      for( ; %i != 0; %i--)
      {
         %paletteId.getObject(0).visible = 0;
         %paletteId.getObject(0).groupNum = %paletteGroup;
         %paletteId.getObject(0).paletteName = %paletteId.getName();
         ToolsPaletteArray.addGuiControl(%paletteId.getObject(0));
      }
      %paletteGroup++;
   }

   %filespec = "tools/worldEditor/gui/ToolsPaletteGroups/*.ed.gui.edso";
   
   // were executing each gui file and adding them to the ToolsPaletteArray
   for( %file = findFirstFile(%filespec); %file !$= ""; %file = findNextFile(%filespec))
   {
      exec( %file );
      %paletteGroup = 0;      
      
      %i = %paletteId.getCount();
      for( ; %i != 0; %i--)
      {
         %paletteId.getObject(0).visible = 0;
         %paletteId.getObject(0).groupNum = %paletteGroup;
         %paletteId.getObject(0).paletteName = %paletteId.getName();
         ToolsPaletteArray.addGuiControl(%paletteId.getObject(0));
      }
      %paletteGroup++;
   }
}

function EWToolsPaletteWindow::init()
{
   EWToolsPaletteWindow.loadToolsPalettes();
}

function EWToolsPaletteWindow::togglePalette(%this, %paletteName)
{
   // since the palette window ctrl auto adjusts to child ctrls being visible,
   // loop through the array and pick out the children that belong to a certain tool
   // and label them visible or not visible
   
   for( %i = 0; %i < ToolsPaletteArray.getCount(); %i++ )
      ToolsPaletteArray.getObject(%i).visible = 0;
   
   %windowMultiplier = 0;
   %paletteNameWordCount = getWordCount( %paletteName );
   for(%pallateNum = 0; %pallateNum < %paletteNameWordCount; %pallateNum++)
   {
      %currentPalette = getWord(%paletteName, %pallateNum);
      for( %i = 0; %i < ToolsPaletteArray.getCount(); %i++ )
      {
         if( ToolsPaletteArray.getObject(%i).paletteName $= %currentPalette)
         {
            ToolsPaletteArray.getObject(%i).visible = 1;
            %windowMultiplier++;
         }
      }
   }
   
   // auto adjust the palette window extent according to how many 
   // children controls we found; if none found, the palette window becomes invisible
   if( %windowMultiplier == 0 || %paletteName $= "")
      EWToolsPaletteWindow.visible = 0;
   else
   {
      EWToolsPaletteWindow.visible = 1;
      EWToolsPaletteWindow.extent = getWord(EWToolsPaletteWindow.extent, 0) SPC (16 + 26 * %windowMultiplier);
   }
}