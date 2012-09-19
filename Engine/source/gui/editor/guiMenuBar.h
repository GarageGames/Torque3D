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

#ifndef _GUIMENUBAR_H_
#define _GUIMENUBAR_H_

#ifndef _GUITEXTLISTCTRL_H_
#include "gui/controls/guiTextListCtrl.h"
#endif
#ifndef _GUITICKCTRL_H_
#include "gui/shiny/guiTickCtrl.h"
#endif

class GuiMenuBar;
class GuiMenuTextListCtrl;

class GuiMenuBackgroundCtrl : public GuiControl
{
   typedef GuiControl Parent;

protected:
   GuiMenuBar *mMenuBarCtrl;
   GuiMenuTextListCtrl *mTextList; 
public:
   GuiMenuBackgroundCtrl(GuiMenuBar *ctrl, GuiMenuTextListCtrl* textList);
   void onMouseDown(const GuiEvent &event);
   void onMouseMove(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
};

class GuiSubmenuBackgroundCtrl : public GuiMenuBackgroundCtrl
{
   typedef GuiMenuBackgroundCtrl Parent;

public:
   GuiSubmenuBackgroundCtrl(GuiMenuBar *ctrl, GuiMenuTextListCtrl* textList);
   bool pointInControl(const Point2I & parentCoordPoint);
   void onMouseDown(const GuiEvent &event);
};

//------------------------------------------------------------------------------

class GuiMenuTextListCtrl : public GuiTextListCtrl
{
   private:
      typedef GuiTextListCtrl Parent;

   protected:
      GuiMenuBar *mMenuBarCtrl;

   public:
	  bool isSubMenu; //  Indicates that this text list is in a submenu

      GuiMenuTextListCtrl(); // for inheritance
      GuiMenuTextListCtrl(GuiMenuBar *ctrl);

      // GuiControl overloads:
      bool onKeyDown(const GuiEvent &event);
		void onMouseDown(const GuiEvent &event);
      void onMouseUp(const GuiEvent &event);
      void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);

      virtual void onCellHighlighted(Point2I cell); //  Added
};

//------------------------------------------------------------------------------

class GuiMenuBar : public GuiTickCtrl //  Was: GuiControl
{
   typedef GuiTickCtrl Parent; //  Was: GuiControl Parent;
public:

	struct Menu;

	struct MenuItem   // an individual item in a pull-down menu
	{
		char *text;    // the text of the menu item
		U32 id;        // a script-assigned identifier
		char *accelerator; // the keyboard accelerator shortcut for the menu item
      U32 acceleratorIndex; // index of this accelerator
		bool enabled;        // true if the menu item is selectable
      bool visible;        // true if the menu item is visible
      S32 bitmapIndex;     // index of the bitmap in the bitmap array
      S32 checkGroup;      // the group index of the item visa vi check marks - 
                           // only one item in the group can be checked.
		MenuItem *nextMenuItem; // next menu item in the linked list

		bool isSubmenu;				//  This menu item has a submenu that will be displayed
		MenuItem *firstSubmenuItem;	//  The first menu item in the submenu

		Menu* submenuParentMenu; //  For a submenu, this is the parent menu
	};

	struct Menu
	{
		char *text;
		U32 id;
		RectI bounds;
      bool visible;

		S32 bitmapIndex;		// Index of the bitmap in the bitmap array (-1 = no bitmap)
		bool drawBitmapOnly;	// Draw only the bitmap and not the text
		bool drawBorder;		// Should a border be drawn around this menu (usually if we only have a bitmap, we don't want a border)

		Menu *nextMenu;
		MenuItem *firstMenuItem;
	};
	
	GuiMenuBackgroundCtrl *mBackground;
	GuiMenuTextListCtrl *mTextList;
	
	GuiSubmenuBackgroundCtrl *mSubmenuBackground; //  Background for a submenu
	GuiMenuTextListCtrl *mSubmenuTextList;     //  Text list for a submenu

	Menu *menuList;
   Menu *mouseDownMenu;
   Menu *mouseOverMenu;

   MenuItem* mouseDownSubmenu; //  Stores the menu item that is a submenu that has been selected
   MenuItem* mouseOverSubmenu; //  Stores the menu item that is a submenu that has been highlighted

   bool menuBarDirty;
   U32 mCurAcceleratorIndex;
   Point2I maxBitmapSize;

   S32 mCheckmarkBitmapIndex; // Index in the bitmap array to use for the check mark image

   S32 mPadding;
   S32 mHorizontalMargin; // Left and right margin around the text of each menu
   S32 mVerticalMargin;   // Top and bottom margin around the text of each menu
   S32 mBitmapMargin;     // Margin between a menu's bitmap and text

   //  Used to keep track of the amount of ticks that the mouse is hovering
   // over a menu.
   S32 mMouseOverCounter;
   bool mCountMouseOver;
   S32 mMouseHoverAmount;
	
	GuiMenuBar();
   bool onWake();
   void onSleep();

	// internal menu handling functions
	// these are used by the script manipulation functions to add/remove/change menu items

   void addMenu(const char *menuText, U32 menuId);
	Menu *findMenu(const char *menu);  // takes either a menu text or a string id
	MenuItem *findMenuItem(Menu *menu, const char *menuItem); // takes either a menu text or a string id
	void removeMenu(Menu *menu);
	void removeMenuItem(Menu *menu, MenuItem *menuItem);
	void addMenuItem(Menu *menu, const char *text, U32 id, const char *accelerator, S32 checkGroup);
	void clearMenuItems(Menu *menu);
   void clearMenus();

   //  Methods to deal with submenus
   MenuItem* findSubmenuItem(Menu *menu, const char *menuItem, const char *submenuItem);
   void addSubmenuItem(Menu *menu, MenuItem *submenu, const char *text, U32 id, const char *accelerator, S32 checkGroup);
   void removeSubmenuItem(MenuItem *menuItem, MenuItem *submenuItem);
   void clearSubmenuItems(MenuItem *menuitem);
   void onSubmenuAction(S32 selectionIndex, RectI bounds, Point2I cellSize);
   void closeSubmenu();
   void checkSubmenuMouseMove(const GuiEvent &event);
   MenuItem *findHitMenuItem(Point2I mousePoint);

   void highlightedMenuItem(S32 selectionIndex, RectI bounds, Point2I cellSize); //  Called whenever a menu item is highlighted by the mouse

	// display/mouse functions

	Menu *findHitMenu(Point2I mousePoint);

   //  Called when the GUI theme changes and a bitmap arrary may need updating
  // void onThemeChange();

   void onPreRender();
	void onRender(Point2I offset, const RectI &updateRect);

   void checkMenuMouseMove(const GuiEvent &event);
   void onMouseMove(const GuiEvent &event);
   void onMouseLeave(const GuiEvent &event);
   void onMouseDown(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   
   void onAction();
   void closeMenu();
   void buildAcceleratorMap();
   void acceleratorKeyPress(U32 index);

   void menuItemSelected(Menu *menu, MenuItem *item);

   //  Added to support 'ticks'
   void processTick();

   static void initPersistFields();

   DECLARE_CONOBJECT(GuiMenuBar);
   DECLARE_CALLBACK( void, onMouseInMenu, (bool hasLeftMenu));
   DECLARE_CALLBACK( void, onMenuSelect, (const char* menuId, const char* menuText));
   DECLARE_CALLBACK( void, onMenuItemSelect, ( const char* menuId, const char* menuText, const char* menuItemId, const char* menuItemText  ));
   DECLARE_CALLBACK( void, onSubmenuSelect, ( const char* submenuId, const char* submenuText));
};

#endif
