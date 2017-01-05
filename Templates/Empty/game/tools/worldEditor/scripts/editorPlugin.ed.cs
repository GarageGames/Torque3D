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


///
/// This is used to register editor extensions and tools.
///
/// There are various callbacks you can overload to hook in your
/// own functionality without changing the core editor code.
///
/// At the moment this is primarily for the World/Mission 
/// Editor and the callbacks mostly make sense in that context.
///
/// Example:
///
///   %obj = new ScriptObject()
///   {
///      superclass = "EditorPlugin";
///      class = "RoadEditor";
///   };
///   
///   EditorPlugin::register( %obj );
///
/// For an a full example see: tools/roadEditor/main.cs
///                        or: tools/riverEditor/main.cs
///                        or: tools/decalEditor/main.cs
///

/// It is not intended for the user to overload this method.
/// If you do make sure you call the parent.
function EditorPlugin::onAdd( %this )
{
   EditorPluginSet.add( %this );   
}


/// Callback when the world editor is first started.  It
/// is a good place to insert menus and menu items as well as 
/// preparing guis.
function EditorPlugin::onWorldEditorStartup( %this )
{
}

/// Callback when the world editor is about to be totally deleted.
/// At the time of this writing this occurs when the engine is shut down
/// and the editor had been initialized.
function EditorPlugin::onWorldEditorShutdown( %this )
{
}

/// Callback right before the editor is opened.
function EditorPlugin::onEditorWake( %this )
{
}

/// Callback right before the editor is closed.
function EditorPlugin::onEditorSleep( %this )
{
}

/// Callback when the tool is 'activated' by the WorldEditor
/// Push Gui's, stuff like that
function EditorPlugin::onActivated( %this )
{
   %this.isActivated = true;
}

/// Callback when the tool is 'deactivated' / closed by the WorldEditor
/// Pop Gui's, stuff like that
function EditorPlugin::onDeactivated( %this )
{
   %this.isActivated = false;
}

/// Callback when tab is pressed.
/// Used by the WorldEditor to toggle between inspector/creator, for example.
function EditorPlugin::onToggleToolWindows( %this )
{
}

/// Callback when the edit menu is clicked or prior to handling an accelerator 
/// key event mapped to an edit menu item.
/// It is up to the active editor to determine if these actions are
/// appropriate in the current state.
function EditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   %editMenu.enableItem( 3, false ); // Cut
   %editMenu.enableItem( 4, false ); // Copy
   %editMenu.enableItem( 5, false ); // Paste  
   %editMenu.enableItem( 6, false ); // Delete
   %editMenu.enableItem( 8, false ); // Deselect     
}

/// If this tool keeps track of changes that necessitate resaving the mission
/// return true in that case.
function EditorPlugin::isDirty( %this )
{
   return false;  
}

/// This gives tools a chance to clear whatever internal variables keep track of changes
/// since the last save.
function EditorPlugin::clearDirty( %this )
{
}

/// This gives tools chance to save data out when the mission is being saved.
/// This will only be called if the tool says it is dirty.
function EditorPlugin::onSaveMission( %this, %missionFile )
{
}

/// Called when during mission cleanup to notify plugins.
function EditorPlugin::onExitMission( %this )
{
}

/// Called on the active plugin when a SceneObject is selected.
///
/// @param object The object being selected.
function EditorPlugin::onObjectSelected( %this, %object )
{
}

/// Called on the active plugin when a SceneObject is deselected.
///
/// @param object The object being deselected.
function EditorPlugin::onObjectDeselected( %this, %object )
{
}

/// Called on the active plugin when the selection of SceneObjects is cleared.
function EditorPlugin::onSelectionCleared( %this )
{
}
   
/// Callback when the the delete item of the edit menu is selected or its
/// accelerator is pressed.
function EditorPlugin::handleDelete( %this )
{   
   warn( "EditorPlugin::handleDelete( " @ %this.getName() @ " )" NL
         "Was not implemented in child namespace, yet menu item was enabled." );
}

/// Callback when the the deselect item of the edit menu is selected or its
/// accelerator is pressed.
function EditorPlugin::handleDeselect( %this )
{   
   warn( "EditorPlugin::handleDeselect( " @ %this.getName() @ " )" NL
         "Was not implemented in child namespace, yet menu item was enabled." );
}

/// Callback when the the cut item of the edit menu is selected or its
/// accelerator is pressed.
function EditorPlugin::handleCut( %this )
{   
   warn( "EditorPlugin::handleCut( " @ %this.getName() @ " )" NL
         "Was not implemented in child namespace, yet menu item was enabled." );
}

/// Callback when the the copy item of the edit menu is selected or its
/// accelerator is pressed.
function EditorPlugin::handleCopy( %this )
{   
   warn( "EditorPlugin::handleCopy( " @ %this.getName() @ " )" NL
         "Was not implemented in child namespace, yet menu item was enabled." );
}

/// Callback when the the paste item of the edit menu is selected or its
/// accelerator is pressed.
function EditorPlugin::handlePaste( %this )
{   
   warn( "EditorPlugin::handlePaste( " @ %this.getName() @ " )" NL
         "Was not implemented in child namespace, yet menu item was enabled." );
}

/// Callback when the escape key is pressed.
/// Return true if this tool has handled the key event in a custom way.
/// If false is returned the WorldEditor default behavior is to return
/// to the ObjectEditor.
function EditorPlugin::handleEscape( %this )
{
   return false;
}
