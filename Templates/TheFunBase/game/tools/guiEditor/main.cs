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

function initializeGuiEditor()
{
   echo( " % - Initializing Gui Editor" );
   
   // GUIs.
   
   exec( "./gui/guiEditor.ed.gui" );
   exec( "./gui/guiEditorNewGuiDialog.ed.gui" );
   exec( "./gui/guiEditorPrefsDlg.ed.gui" );
   exec( "./gui/guiEditorSelectDlg.ed.gui" );
   exec( "./gui/EditorChooseGUI.ed.gui" );
   
   // Scripts.

   exec( "./scripts/guiEditor.ed.cs" );
   exec( "./scripts/guiEditorTreeView.ed.cs" );
   exec( "./scripts/guiEditorInspector.ed.cs" );
   exec( "./scripts/guiEditorProfiles.ed.cs" );
   exec( "./scripts/guiEditorGroup.ed.cs" );
   exec( "./scripts/guiEditorUndo.ed.cs" );
   exec( "./scripts/guiEditorCanvas.ed.cs" );
   exec( "./scripts/guiEditorContentList.ed.cs" );
   exec( "./scripts/guiEditorStatusBar.ed.cs" );
   exec( "./scripts/guiEditorToolbox.ed.cs" );
   exec( "./scripts/guiEditorSelectDlg.ed.cs" );
   
   exec( "./scripts/guiEditorNewGuiDialog.ed.cs" );
   exec( "./scripts/fileDialogs.ed.cs" );
   exec( "./scripts/guiEditorPrefsDlg.ed.cs" );
   exec( "./scripts/EditorChooseGUI.ed.cs" );
}

function destroyGuiEditor()
{
}
