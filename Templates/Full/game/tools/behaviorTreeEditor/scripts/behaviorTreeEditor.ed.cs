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

$inBehaviorTreeEditor = false;

//==============================================================================
// INIT
//==============================================================================
function BTEdit()
{
   if (!$InBehaviorTreeEditor)
   {
      if(!isObject(BTEditCanvas))
         new GuiControl(BTEditCanvas, EditorGuiGroup);
      
      BTEditor.startUp(Canvas.getContent());
      
      $InBehaviorTreeEditor = true;
      BehaviorTreeManager.onBehaviorTreeEditor(true);
   }
   else
   {
      BTEditCanvas.quit();
   }
}

function toggleBehaviorTreeEditor( %make )
{
   if( %make )
   {
      BTEdit();
      cancel($Game::Schedule);
   }
}

GlobalActionMap.bind( keyboard, "f9", toggleBehaviorTreeEditor );


function BTEditor::startUp(%this, %content)
{
   %this.lastContent=%content;
   Canvas.setContent( BTEditor );
   
   if(!isObject(BehaviorTreeManager))
      // This isn't pretty, but we need to load up existing trees
      exec("scripts/server/BadBehavior/behaviorTreeManager.cs");
   
   if(BehaviorTreeGroup.getCount() == 0)
   {
      %this.createTree();
   }
   else
   {
      BTEditorContentList.refresh();
      if(BTEditorTabBook.getCount() == 0)
         BTEditorContentList.setFirstSelected();
   }
   
   %this.updateUndoMenu();
   %this.updateNodeTypes();
}


function BTEditor::updateNodeTypes(%this)
{
   if(isObject(BTNodeTypes))
      BTNodeTypes.delete();
   
   new SimSet(BTNodeTypes);
   %set = new SimSet() {
      internalName = "Composite";
   };
   %set.add( new ScriptObject() { nodeType = "ActiveSelector"; } );
   %set.add( new ScriptObject() { nodeType = "Parallel"; } );
   %set.add( new ScriptObject() { nodeType = "RandomSelector"; } );
   %set.add( new ScriptObject() { nodeType = "Selector"; } );
   %set.add( new ScriptObject() { nodeType = "Sequence"; } );
   BTNodeTypes.add(%set);
   
   %set = new SimSet() {
      internalName = "Decorator";
   };
   %set.add( new ScriptObject() { nodeType = "FailAlways"; });
   %set.add( new ScriptObject() { nodeType = "Inverter"; } );
   %set.add( new ScriptObject() { nodeType = "Loop"; } );
   %set.add( new ScriptObject() { nodeType = "Monitor"; } );
   %set.add( new ScriptObject() { nodeType = "SucceedAlways"; } );
   %set.add( new ScriptObject() { nodeType = "Ticker"; } );
   BTNodeTypes.add(%set);
   
   %set = new SimSet() {
      internalName = "Leaf";
   };
   %set.add( new ScriptObject() { nodeType = "RandomWait"; } );
   %set.add( new ScriptObject() { nodeType = "ScriptedBehavior"; } );   
   %set.add( new ScriptObject() { nodeType = "ScriptEval"; } );
   %set.add( new ScriptObject() { nodeType = "ScriptFunc"; } );
   %set.add( new ScriptObject() { nodeType = "SubTree"; } );
   %set.add( new ScriptObject() { nodeType = "Wait"; } );
   %set.add( new ScriptObject() { nodeType = "WaitForSignal"; } );
   BTNodeTypes.add(%set);
}

function BTEditor::getBaseNodeType(%this, %type)
{
   foreach(%baseType in BTNodeTypes)
   {
      if(%baseType.internalName $= %type) // supplied basetype
         return %type;
         
      foreach(%derivedType in %baseType) // supplied derived type
      {
         if(%derivedType.nodeType $= %type)
            return %baseType.internalName;
      }
   }
   return "";
}

function BTEditor::viewTree(%this, %tree)
{
   %viewPage = -1;
   foreach(%page in BTEditorTabBook)
   {
      if(%page.rootNode == %tree)
      {
         %viewPage = %page;
         break;
      }
   }
   
   if(isObject(%viewPage))
   {
      %viewPage.select();
   }
   else
   {
      %newPage = BTEditor::newPage();
      %newPage.setText(%tree.name);
      %newPage.rootNode = %tree;
      BTEditorTabBook.addGuiControl(%newPage);
      %newPage-->BTView.open(%tree);
      %newPage-->BTView.refresh();
   }
   %this.updateUndoMenu();
}

function BTEditor::createTree(%this)
{
   %list = BTEditorCreatePrompt-->CopySourceDropdown;
   %list.clear();
   foreach (%tree in BehaviorTreeGroup)
      %list.add(%tree.getName(), %tree);
   Canvas.pushDialog(BTEditorCreatePrompt);
}

function BTEditor::getCurrentViewCtrl(%this)
{
   %pageId = BTEditorTabBook.getSelectedPage();
   if(%pageId >= 0)
      return BTEditorTabBook.getObject(%pageId)-->BTView;
   
   return %pageId;
}


function BTEditor::getTreeRoot(%this, %node)
{
   %current = %node;
   while(%current.getClassName() !$= "Root" && isObject(%current))
   {
      %current = %current.getGroup();
   }
   return %current;
}

function BTEditor::getCurrentRootNode(%this)
{
   return %this.getCurrentViewCtrl().getRootNode();  
}


function BTEditor::createPromptNameCheck(%this)
{
   %name = BTEditorCreatePrompt-->CreateTreeName.getText();
   if( !Editor::validateObjectName( %name, true ) )
      return;
      
   // Fetch the copy source and clear the list.
   
   %copySource = BTEditorCreatePrompt-->copySourceDropdown.getText();
   BTEditorCreatePrompt-->copySourceDropdown.clear();
   
   // Remove the dialog and create the tree.
   
   canvas.popDialog( BTEditorCreatePrompt );
   %this.createTreeFinish( %name, %copySource );
}

function BTEditor::createTreeFinish( %this, %name, %copySource )
{
   %newTree = -1;
   pushInstantGroup(BehaviorTreeGroup);   
   if(%copySource !$= "")
   {
      %newTree = %copySource.deepClone();
   }
   else
   {
      %newTree = new Root();  
   }
   popInstantGroup();
   
   %newTree.setName(%name);
   %newTree.setFilename("");

   BTEditorContentList.refresh();
   BTEditorContentList.setSelected(BTEditorContentList.findText(%newTree.name));   
}

function BTEditor::saveTree(%this, %tree, %prompt)
{
   // check we actually have something to save
   if(!isObject(%tree))
      return;
      
   if((%file = %tree.getFileName()) !$= "")
   {
      %path = filePath(%file);  
   }
   else
   {
      %path = "scripts/server/BadBehavior/behaviorTrees";
      %file = %path @ "/" @ %tree.name;
      
      if(!isDirectory(%path))
         createPath(%path @ "/");
      
      %prompt = true;
   }
   
   if(%prompt || !isFile(%file))
   {
      %dlg = new SaveFileDialog()
      {
         filters = "Torque script files (*.cs)|*.cs|";
         defaultPath = %path;
         defaultFile = %file;
         changePath = true;
         overwritePrompt = true;
      };
   
      if(%dlg.execute())
      {
         %file = %dlg.fileName;         
         %dlg.delete();
      }
      else
      {
         return;
      }
   }

   %tree.save(%file);
   %tree.setFileName(collapseFilename(%file));
   BTEditorStatusBar.setText("Saved '" @ %tree.name @ "' to file" SPC %tree.getFileName());
}

//==============================================================================
// VIEW
//==============================================================================

function BTEditor::expandAll(%this)
{
   %this.getCurrentViewCtrl().expandAll();
}

function BTEditor::collapseAll(%this)
{
   %this.getCurrentViewCtrl().collapseAll();
}


function BTEditorTabBook::onTabClose(%this, %index)
{
   if(%index == %this.selectedPage)
   {
      BehaviorTreeInspector.inspect(-1);
      if(%this.getCount() > 1)
         %this.getObject(0).select();
      else
         BTEditor.ResetUndoMenu();
   }
   
   %this.getObject(%index).delete();
}

function BTEditorTabBook::onTabSelected(%this, %text, %index)
{
   //echo("onTabSelected" TAB %text);
   BTEditor.updateUndoMenu();
   BehaviorTreeInspector.inspect(BTEditor.getCurrentViewCtrl().getSelectedObject());
}

function BTEditorTabBook::onTabRightClick(%this, %text, %index)
{
   if(isObject(BTEditorTabBookPopup))
      BTEditorTabBookPopup.delete();
      
   %popup = new PopupMenu( BTEditorTabBookPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         item[ 0 ] = "Close" SPC %text SPC "tab" TAB "" TAB "BTEditorTabBook.onTabClose(" SPC %index SPC ");";
      };
      
   %popup.showPopup( Canvas ); 
}

//==============================================================================
// UNDO
//==============================================================================
function BTEditor::getUndoManager( %this )
{
   return %this.getCurrentViewCtrl().getUndoManager();
}


function BTEditor::updateUndoMenu(%this)
{
   %uman = %this.getUndoManager();
   %nextUndo = %uman.getNextUndoName();
   %nextRedo = %uman.getNextRedoName();
   
   %editMenu = BTEditCanvas.menuBar->editMenu;
   
   %editMenu.setItemName( 0, "Undo " @ %nextUndo );
   %editMenu.setItemName( 1, "Redo " @ %nextRedo );
   
   %editMenu.enableItem( 0, %nextUndo !$= "" );
   %editMenu.enableItem( 1, %nextRedo !$= "" );
}

function BTEditor::ResetUndoMenu(%this)
{
   %editMenu = BTEditCanvas.menuBar->editMenu;
   %editMenu.setItemName( 0, "Undo" );
   %editMenu.setItemName( 1, "Redo" );
   %editMenu.enableItem( 0, false );
   %editMenu.enableItem( 1, false );
}

function BTEditor::undo(%this)
{
   %action = %this.getUndoManager().getNextUndoName();
   
   %this.getUndoManager().undo();
   %this.updateUndoMenu();
   
   BTEditorStatusBar.print( "Undid '" @ %action @ "'" );
}

function BTEditor::redo(%this)
{
   %action = %this.getUndoManager().getNextRedoName();

   %this.getUndoManager().redo();
   %this.updateUndoMenu();
   
   BTEditorStatusBar.print( "Redid '" @ %action @ "'" );
}