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

//-----------------------------------------------------------------------------
// CameraBookmark class methods.  Located here so they won't fire without
// the tools in place.

function CameraBookmark::onAdd( %this )
{
}

function CameraBookmark::onRemove( %this )
{
   if( isObject(EditorCameraBookmarks) )
   {
      %pos = CameraBookmarks.getObjectIndex( %this );
      if( %pos != -1 )
      {
         EditorCameraBookmarks.deleteItem( %pos );
         EManageBookmarks.deleteBookmark( %this, %pos );
      }
   }
}

function CameraBookmark::onGroupAdd( %this )
{
   // If we're added to the CameraBookmarks group, then also add us
   // to the menu and Manage Bookmarks window.
   if( isObject(CameraBookmarks) )
   {
      %pos = CameraBookmarks.getObjectIndex( %this );
      if( %pos != -1 )
      {
         EditorCameraBookmarks.addItem( %pos, %this.internalName );
         EManageBookmarks.addBookmark( %this, %pos );
      }
   }
}

function CameraBookmark::onGroupRemove( %this )
{
   // If we're part of the CameraBookmarks group, then also remove us from
   // the menu and Manage Bookmarks window.
   if( isObject(CameraBookmarks) )
   {
      %pos = CameraBookmarks.getObjectIndex( %this );
      if( %pos != -1 )
      {
         EditorCameraBookmarks.deleteItem( %pos );
         EManageBookmarks.deleteBookmark( %this, %pos );
      }
   }
}

function CameraBookmark::onInspectPostApply( %this )
{
   EditorCameraBookmarks.rebuildBookmarks();
}

//-----------------------------------------------------------------------------

function EditorCameraBookmarksMenu::onAdd( %this )
{
   if(! isObject(%this.canvas))
      %this.canvas = Canvas;
   
   // Add any existing bookmarks
   %this.rebuildBookmarks();
}

function EditorCameraBookmarksMenu::addItem( %this, %pos, %name )
{
   if( %this.NoneItem == true )
   {
      %this.NoneItem = false;
      %this.removeItem( 0 );
   }
   
   %accel = "";
   %this.insertItem(%pos, %name !$= "-" ? %name : "", %accel);
}

function EditorCameraBookmarksMenu::deleteItem( %this, %pos )
{
   %this.removeItem( %pos );
   if( %this.getItemCount() == 0 && %this.NoneItem != true )
   {
      %this.addItem( 0, "None" );
      %this.enableItem( 0, false );
      %this.NoneItem = true;
   }
}

function EditorCameraBookmarksMenu::onSelectItem( %this, %pos, %text )
{
   if( %pos >= 0 && %pos < CameraBookmarks.getCount() )
   {
      %mark = CameraBookmarks.getObject( %pos );
      EditorGui.jumpToBookmark( %mark.internalName );
      return true;
   }

   return false;
}

function EditorCameraBookmarksMenu::rebuildBookmarks( %this )
{
   // Delete all current items
   while( %this.getItemCount() > 0)
   {
      %this.removeItem( 0 );
   }
   
   // Add back in all of the bookmarks
   if( isObject(CameraBookmarks) && CameraBookmarks.getCount() > 0 )
   {
      for( %i=0; %i<CameraBookmarks.getCount(); %i++ )
      {
         %mark = CameraBookmarks.getObject( %i );
         %this.addItem( %i, %mark.internalName );
      }
      %this.NoneItem = false;
   }
   else
   {
      %this.addItem( 0, "None" );
      %this.enableItem( 0, false );
      %this.NoneItem = true;
   }
}

//-----------------------------------------------------------------------------

function ManageBookmarksContainer::onOK( %this )
{
   %name = EAddBookmarkWindowName.getText();
   EAddBookmarkWindowName.clearFirstResponder();
   
   if( %name $= "" )
   {
      // look for a NewCamera name to grab
      for(%i = 0; ; %i++){
         %name = "NewCamera_" @ %i;
         if( !CameraBookmarks.findObjectByInternalName(%name) ){
            break;
         }
      }
   }
   
   // Check if the new bookmark name already exists
   if( isObject(CameraBookmarks) && CameraBookmarks.findObjectByInternalName(%name) )
   { 
      %userName = %name;
      for(%i = 0; ; %i++){
         %name = %userName @ "_" @ %i;
         if( !CameraBookmarks.findObjectByInternalName(%name) ){
            break;
         }
      }
   }
   
   EditorGui.addCameraBookmark( %name );
   EAddBookmarkWindowName.text = "";
   //%this.CloseWindow();
}

function EAddBookmarkWindowName::onReturn( %this )
{
   // Same as clicking the Create Bookmark button
   ManageBookmarksContainer.onOK();
}

//-----------------------------------------------------------------------------

function EManageBookmarks::hideDialog( %this )
{
   %this.setVisible(false);
}

function EManageBookmarks::ToggleVisibility( %this )
{
   if ( %this.visible  )
   {
      %this.setVisible(false);
      EWorldEditor.EManageBookmarksDisplayed = false;
   }
   else
   {
      %this.setVisible(true);
      %this.selectWindow();
      %this.setCollapseGroup(false);
      EWorldEditor.EManageBookmarksDisplayed = true;
   }
}

function EManageBookmarks::addBookmark( %this, %mark, %index )
{
   %gui = new GuiControl() {
      internalName = %mark.getInternalName();
      Enabled = "1";
      Profile = "ToolsGuiDefaultProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "0 0";
      Extent = "300 20";
      MinExtent = "78 20";
      Visible = "1";
      Bookmark = %mark;

      new GuiBitmapButtonCtrl() {
         class = "EManageBookmarksGoToButton";
         bitmap = "tools/gui/images/camera-btn";
         groupNum = "-1";
         buttonType = "PushButton";
         useMouseEvents = "0";
         isContainer = "0";
         Profile = "ToolsGuiButtonProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         position = "2 2";
         Extent = "17 17";
         MinExtent = "8 2";
         canSave = "1";
         Visible = "1";
         Command = "";
         tooltipprofile = "ToolsGuiToolTipProfile";
         ToolTip = "Go to bookmark";
         hovertime = "1000";
         internalName = "goToBookmark";
         canSaveDynamicFields = "0";
      };
      
      new GuiTextEditCtrl() {
         class = "EManageBookmarksTextEdit";
         internalName = "BookmarkName";
         profile="ToolsGuiTextEditProfile";
         HorizSizing = "width";
         VertSizing = "bottom";
         position = "22 2";
         Extent = "260 18";
         text = %mark.getInternalName();
         maxLength = "1024";
         AltCommand = "";
      };

      new GuiBitmapButtonCtrl() {
         class = "EManageBookmarksDeleteButton";
         bitmap = "tools/gui/images/delete";
         groupNum = "-1";
         buttonType = "PushButton";
         useMouseEvents = "0";
         isContainer = "0";
         Profile = "ToolsGuiButtonProfile";
         HorizSizing = "left";
         VertSizing = "bottom";
         position = "284 3";
         Extent = "16 16";
         MinExtent = "8 2";
         canSave = "1";
         Visible = "1";
         Command = "";
         tooltipprofile = "ToolsGuiToolTipProfile";
         ToolTip = "Delete camera bookmark";
         hovertime = "1000";
         internalName = "deleteBookmark";
         canSaveDynamicFields = "0";
      };
   };
   
   EManageBookmarks-->ManageBookmarksWindowStack.addGuiControl( %gui );
}

function EManageBookmarks::deleteBookmark( %this, %mark, %index )
{
   %gui = EManageBookmarks-->ManageBookmarksWindowStack.findObjectByInternalName( %mark.getInternalName() );
   if( %gui != 0 )
      %gui.delete();
   else
      warn("EManageBookmarks::deleteBookmark(): Could not find bookmark " @ %mark @ " at index " @ %index);
}

function EManageBookmarksGoToButton::onClick( %this )
{
   %mark = %this.getParent().Bookmark;
   EditorGui.jumpToBookmark( %mark.getInternalName() );
}

function EManageBookmarksDeleteButton::onClick( %this )
{
   %mark = %this.getParent().Bookmark;
   EditorGui.schedule( 0, removeCameraBookmark, %mark.getInternalName() );
}

function EManageBookmarksTextEdit::onGainFirstResponder( %this )
{
   if( %this.isActive() )
   {
      %this.selectAllText();
   }
}

function EManageBookmarksTextEdit::onReturn( %this )
{
   %this.onValidate();
}

function EManageBookmarksTextEdit::onValidate( %this )
{
   %mark = %this.getParent().Bookmark;
   %oldname = %mark.getInternalName();
   %newname = %this.getText();
   
   // If the new name is the same as the old, do nothing
   if( %newname $= %oldname )
      return;
   
   // Make sure the new name doesn't conflict with a current bookmark
   if( isObject(CameraBookmarks) && CameraBookmarks.findObjectByInternalName(%newname) )
   {
      %id = %this.getId();
      %callback = %id @ ".setText(\"" @ %oldname @ "\"); " @ %id @ ".makeFirstResponder(true); " @ %id @ ".selectAllText();";
      MessageBoxOK("Create Bookmark", "You must provide a unique name for the new bookmark.", %callback);
      return;
   }
   
   // Rename the bookmark and update
   %this.getParent().setInternalName( %newname );
   %mark.setInternalName( %newname );
   if( Inspector.getInspectObject() == %mark.getId() )
   {
      Inspector.inspect( %mark );
      Inspector.apply();
   }
   else
   {
      // User is not inspecting the bookmark, so manually
      // update the menu.
      %mark.onInspectPostApply();
   }

}
