//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

//==============================================================================
// INIT
//==============================================================================
function BTEditCanvas::onAdd( %this )
{
   %this.onCreateMenu();
   
   // close any invalid tab book pages
   for( %i=0; %i < BTEditorTabBook.getCount(); %i++)
   {
      %page = BTEditorTabBook.getObject(%i);
      if(!isObject(%page.rootNode) || %page.rootNode.getClassName() !$= "Root")
      {
         BTEditorTabBook.remove(%page);
         %page.delete();
         %i--;
      }           
   }
}

function BTEditCanvas::onRemove( %this )
{
   if( isObject( BehaviorTreeEditorGui.menuGroup ) )
      BehaviorTreeEditorGui.delete();
   
   //BTEditorTabBook.deleteAllObjects();
}

function BTEditCanvas::quit( %this )
{
   // we must not delete a window while in its event handler, or we foul the event dispatch mechanism
   %this.schedule(10, delete);
   
   Canvas.setContent(BTEditor.lastContent);
   $InBehaviorTreeEditor = false;
   BehaviorTreeManager.onBehaviorTreeEditor(false);
   
   // cleanup
   %this.onDestroyMenu();
   
   //Re-establish the main editor's menubar
   EditorGui.attachMenus();
}

//==============================================================================
// MENU
//==============================================================================
function BTEditCanvas::onCreateMenu(%this)
{
   if(isObject(%this.menuBar))
      return;
   
   //set up %cmdctrl variable so that it matches OS standards
   if( $platform $= "macos" )
   {
      %cmdCtrl = "cmd";
      %redoShortcut = "Cmd-Shift Z";
   }
   else
   {
      %cmdCtrl = "Ctrl";
      %redoShortcut = "Ctrl Y";
   }
   
   // Menu bar
   %this.menuBar = new GuiMenuBar(BadBehaviorMenubar)
   {
      dynamicItemInsertPos = 3;
      extent = "1024 20";
      minExtent = "320 20";
      horizSizing = "width";
      profile = "GuiMenuBarProfile";
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "File";
         internalName = "FileMenu";
         
         item[0] = "New Tree..." TAB %cmdCtrl SPC "N" TAB "BTEditor.createTree();";
         item[1] = "Open..." TAB %cmdCtrl SPC "O" TAB %this @ ".open();";
         item[2] = "Save Tree" TAB %cmdCtrl SPC "S" TAB "BTEditor.saveTree( BTEditor.getCurrentRootNode(), false );";
         item[3] = "Save Tree As..." TAB %cmdCtrl @ "-Shift S" TAB "BTEditor.saveTree( BTEditor.getCurrentRootNode(), true );";
         item[4] = "-";
         item[5] = "Close Editor" TAB "F9" TAB %this @ ".quit();";
         item[6] = "Quit" TAB %cmdCtrl SPC "Q" TAB "quit();";
      };

      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "Edit";
         internalName = "EditMenu";
         
         item[0] = "Undo" TAB %cmdCtrl SPC "Z" TAB "BTEditor.undo();";
         item[1] = "Redo" TAB %redoShortcut TAB "BTEditor.redo();";
         item[2] = "-";
         item[3] = "Delete node" TAB "" TAB "BTEditor.getCurrentViewCtrl().deleteSelection();";
         item[4] = "Excise node" TAB "" TAB "BTEditor.getCurrentViewCtrl().exciseSelection();";
      };
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         barTitle = "View";
         internalName = "ViewMenu";
         
         item[0] = "Expand All" TAB %cmdCtrl SPC "=" TAB "BTEditor.expandAll();";
         item[1] = "Collapse All" TAB %cmdCtrl SPC "-" TAB "BTEditor.collapseAll();";
      };
      
      new PopupMenu()
      {
         superClass = "MenuBuilder";
         internalName = "HelpMenu";

         barTitle = "Help";

         Item[0] = "Help will arrive soon......";
      };
   };
   %this.menuBar.attachToCanvas(Canvas, 0);
}

function BTEditCanvas::onDestroyMenu(%this)
{
   if( !isObject( %this.menuBar ) )
      return;

   // Destroy menus      
   while( %this.menuBar.getCount() != 0 )
      %this.menuBar.getObject( 0 ).delete();
   
   %this.menuBar.removeFromCanvas();
   %this.menuBar.delete();
}

