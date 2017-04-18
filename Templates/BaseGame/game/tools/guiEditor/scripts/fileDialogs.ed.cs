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

$GUI::FileSpec = "Torque Gui Files (*.gui)|*.gui|All Files (*.*)|*.*|";

/// GuiBuilder::getSaveName - Open a Native File dialog and retrieve the
///  location to save the current document.
/// @arg defaultFileName   The FileName to default in the field and to be selected when a path is opened
function GuiBuilder::getSaveName( %defaultFileName )
{
   %defaultPath = GuiEditor.LastPath;
   
   if( %defaultFileName $= "" )
   {
      %prefix = "";
      if( isFunction( "isScriptPathExpando" ) )
      {
         // if we're editing a game, we want to default to the games dir.
         // if we're not, then we default to the tools directory or the base.
         if( isScriptPathExpando( "^game") )
            %prefix = "^game/";
         else if( isScriptPathExpando( "^tools" ) )
            %prefix = "^tools/";
      }
         
      %defaultFileName = expandFilename( %prefix @ "gui/untitled.gui" );
   }
   else
      %defaultPath = filePath( %defaultFileName );
      
   %dlg = new SaveFileDialog()
   {
      Filters           = $GUI::FileSpec;
      DefaultPath       = makeFullPath( %defaultPath );
      DefaultFile       = %defaultFileName;
      ChangePath        = false;
      OverwritePrompt   = true;
   };
         
   if( %dlg.Execute() )
   {
      GuiEditor.LastPath = filePath( %dlg.FileName );
      %filename = %dlg.FileName;
      if( fileExt( %filename ) !$= ".gui" )
         %filename = %filename @ ".gui";
   }
   else
      %filename = "";
      
   %dlg.delete();
      
   return %filename;
}

function GuiBuilder::getOpenName( %defaultFileName )
{
   if( %defaultFileName $= "" )
      %defaultFileName = expandFilename("^game/gui/untitled.gui");
   
   %dlg = new OpenFileDialog()
   {
      Filters        = $GUI::FileSpec;
      DefaultPath    = GuiEditor.LastPath;
      DefaultFile    = %defaultFileName;
      ChangePath     = false;
      MustExist      = true;
   };
         
   if(%dlg.Execute())
   {
      GuiEditor.LastPath = filePath( %dlg.FileName );
      %filename = %dlg.FileName;      
      %dlg.delete();
      return %filename;
   }
   
   %dlg.delete();
   return "";   
}
