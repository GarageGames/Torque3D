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

// onAdd creates the base menu's and document controller
function BaseEditorCanvas::createMenuBar( %this )
{
   if(isObject(%this.menuBar))
      return;
     
   // Menu bar
   %this.menuBar = new MenuBar()
   {
      dynamicItemInsertPos = 3;
      
      // File Menu
      new PopupMenu()
      {
         superClass = "MenuBuilder"; 
         class = "BaseEditorFileMenu";
         internalName = "FileMenu";      
         
         barTitle = "File";
         
         item[0] = "New..." TAB "Ctrl N" TAB  "[this].onNew();";
         item[1] = "Open..." TAB "Ctrl O" TAB "[this].onOpen();";
         item[2] = "-";
         item[3] = "Save" TAB "Ctrl S" TAB "[this].onSave();";
         item[4] = "Save As" TAB "Ctrl-Alt S" TAB "[this].onSaveAs();";
         item[5] = "Save All" TAB "Ctrl-Shift S" TAB "[this].onSaveAll();";
         item[6] = "-";
         item[7] = "Import..." TAB "Ctrl-Shift I" TAB "[this].onImport();";
         item[8] = "Export..." TAB "Ctrl-Shift E" TAB "[this].onExport();";         
         item[9] = "-";
         item[10] = "Revert" TAB "Ctrl R" TAB "[this].onRevert();";
         item[11] = "-";
         item[12] = "Close" TAB "Ctrl W" TAB "[this].onClose();";
      };      
   };
}

function BaseEditorCanvas::destroyMenuBar( %this )
{
   if( isObject( %this.menuBar ) )
      %this.menuBar.delete();
}

function BaseEditorCanvas::onCreateMenu(%this)
{
   if( !isObject( %this.menuBar ) )
      %this.createMenuBar();
      
   %this.menuBar.attachToCanvas( %this, 0 );
}

function BaseEditorCanvas::onDestroyMenu(%this)
{
   if( isObject( %this.menuBar ) )
   {
      %this.destroyMenuBar();
      %this.menuBar.removeFromCanvas( %this );
   }
}
