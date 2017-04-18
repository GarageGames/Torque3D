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


//---------------------------------------------------------------------------------------------

function initializeDatablockEditor()
{
   echo( " - Initializing Datablock Editor" );
   
   exec("./datablockEditor.cs");
   exec("./datablockEditorUndo.cs");
   exec("./DatablockEditorTreeWindow.ed.gui");
   exec("./DatablockEditorInspectorWindow.ed.gui");
   exec("./DatablockEditorCreatePrompt.ed.gui");
   
   // Add ourselves to EditorGui, where all the other tools reside
   DatablockEditorInspectorWindow.setVisible( false );
   DatablockEditorTreeWindow.setVisible( false );
   
   EditorGui.add( DatablockEditorInspectorWindow );
   EditorGui.add( DatablockEditorTreeWindow );
   
   new ScriptObject( DatablockEditorPlugin )
   {
      superClass = "WorldEditorPlugin";
      editorGui = EWorldEditor;
   };
      
   new SimSet( UnlistedDatablocks );
   
   // create our persistence manager
   DatablockEditorPlugin.PM = new PersistenceManager();
   
   %map = new ActionMap();
   %map.bindCmd( keyboard, "backspace", "DatablockEditorPlugin.onDeleteKey();", "" );
   %map.bindCmd( keyboard, "delete", "DatablockEditorPlugin.onDeleteKey();", "" );  
   DatablockEditorPlugin.map = %map;
   
   DatablockEditorPlugin.initSettings();
}

//---------------------------------------------------------------------------------------------

function destroyDatablockEditor()
{
}
