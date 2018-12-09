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
#ifndef _POPUPMENU_H_
#define _POPUPMENU_H_

#include "console/simBase.h"
#include "core/util/tVector.h"
#include "util/messaging/dispatcher.h"
#include "gui/core/guiCanvas.h"

class PopupMenu;
class GuiMenuBar;
class GuiPopupMenuTextListCtrl;
class GuiPopupMenuBackgroundCtrl;

struct MenuItem   // an individual item in a pull-down menu
{
   String mText;    // the text of the menu item
   U32 mID;        // a script-assigned identifier
   char *mAccelerator; // the keyboard accelerator shortcut for the menu item
   U32 mAcceleratorIndex; // index of this accelerator
   bool mEnabled;        // true if the menu item is selectable
   bool mVisible;        // true if the menu item is visible
   S32 mBitmapIndex;     // index of the bitmap in the bitmap array
   S32 mCheckGroup;      // the group index of the item visa vi check marks - 
                        // only one item in the group can be checked.

   bool mIsSubmenu;				//  This menu item has a submenu that will be displayed

   bool mIsChecked;

   bool mIsSpacer;

   bool mIsMenubarEntry;

   PopupMenu* mSubMenuParentMenu; //  For a submenu, this is the parent menu
   PopupMenu* mSubMenu;
   String mCMD;
};

// PopupMenu represents a menu.
// You can add menu items to the menu, but there is no torque object associated
// with these menu items, they exist only in a  platform specific manner.
class PopupMenu : public SimObject, public virtual Dispatcher::IMessageListener
{
   typedef SimObject Parent;
   friend class GuiMenuBar;
   friend class GuiPopupMenuTextListCtrl;
   friend class GuiPopupMenuBackgroundCtrl;

protected:
   Vector<MenuItem> mMenuItems;

   GuiMenuBar* mMenuBarCtrl;

   StringTableEntry mBarTitle;

   RectI mBounds;
   bool mVisible;

   S32 mBitmapIndex;		// Index of the bitmap in the bitmap array (-1 = no bitmap)
   bool mDrawBitmapOnly;	// Draw only the bitmap and not the text
   bool mDrawBorder;		// Should a border be drawn around this menu (usually if we only have a bitmap, we don't want a border)

   bool mIsSubmenu;

   //This is the gui control that renders our popup
   GuiPopupMenuTextListCtrl *mTextList;

public:
   PopupMenu();
   virtual ~PopupMenu();
   
   DECLARE_CONOBJECT(PopupMenu);

   static void initPersistFields();

   virtual bool onAdd();
   virtual void onRemove();

	static PopupMenuEvent smPopupMenuEvent;
	static bool smSelectionEventHandled; /// Set to true if any menu or submenu handles a selection event
   
   /// pass NULL for @p title to insert a separator
   /// returns the menu item's ID, or -1 on failure.
   /// implementd on a per-platform basis.
   /// TODO: factor out common code
   S32 insertItem(S32 pos, const char *title, const char* accelerator, const char* cmd);

   /// Sets the name title and accelerator for 
   /// an existing item.
   bool setItem(S32 pos, const char *title, const char* accelerator, const char* cmd);

   /// pass NULL for @p title to insert a separator
   /// returns the menu item's ID, or -1 on failure.
   /// adds the submenu to the mSubmenus vector.
   /// implemented on a per-platform basis.
   /// TODO: factor out common code
   S32 insertSubMenu(S32 pos, const char *title, PopupMenu *submenu);
   
   /// remove the menu item at @p itemPos
   /// if the item has a submenu, it is removed from the mSubmenus list.
   /// implemented on a per-platform basis.
   /// TODO: factor out common code
   void removeItem(S32 itemPos);

   /// implemented on a per-platform basis.
   void enableItem(S32 pos, bool enable);
   /// implemented on a per-platform basis.
   void checkItem(S32 pos, bool checked);

   /// All items at positions firstPos through lastPos are unchecked, and the
   /// item at checkPos is checked.
   /// implemented on a per-platform basis.
   void checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos);
   bool isItemChecked(S32 pos);

   /// Returns the number of items in the menu.
   U32 getItemCount();

   ///Clears all items
   void clearItems();

   //-----------------------------------------------------------------------------
   /// Displays this menu as a popup menu and blocks until the user has selected
   /// an item.
   /// @param canvas the owner to show this popup associated with
   /// @param x window local x coordinate at which to display the popup menu
   /// @param y window local y coordinate at which to display the popup menu
   /// implemented on a per-platform basis.
   void showPopup(GuiCanvas *owner, S32 x = -1, S32 y = -1);

   void hidePopup();
   void hidePopupSubmenus();

   /// Returns true iff this menu contains an item that matches @p iD.
   /// implemented on a per-platform basis.
   /// TODO: factor out common code
   bool canHandleID(U32 iD);

   /// A menu item in this menu has been selected by id.
   /// Submenus are given a chance to respond to the command first.
   /// If no submenu can handle the command id, this menu handles it.
   /// The script callback this::onSelectItem( position, text) is called.
   /// If @p text is null, then the text arg passed to script is the text of
   /// the selected menu item.
   /// implemented on a per-platform basis.
   /// TODO: factor out common code
   bool handleSelect(U32 command, const char *text = NULL);

   void onMenuSelect();

	/// Helper function to allow menu selections from signal events.
	/// Wraps canHandleID() and handleSelect() in one function
	/// without changing their internal functionality, so
	/// it should work regardless of platform.
	void handleSelectEvent(U32 popID, U32 command);

   virtual bool onMessageReceived(StringTableEntry queue, const char* event, const char* data );
   virtual bool onMessageObjectReceived(StringTableEntry queue, Message *msg );

   bool isVisible() { return mVisible; }
   void setVisible(bool isVis) { mVisible = isVis; }

   GuiMenuBar* getMenuBarCtrl();
};

#endif // _POPUPMENU_H_
