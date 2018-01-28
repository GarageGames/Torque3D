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

#ifndef _GUITICKCTRL_H_
#include "gui/shiny/guiTickCtrl.h"
#endif

#ifndef _POPUPMENU_H_
#include "gui/editor/popupMenu.h"
#endif

class GuiMenuBar;
class WindowInputGenerator;

//------------------------------------------------------------------------------
class GuiMenuBar : public GuiTickCtrl //  Was: GuiControl
{
   typedef GuiTickCtrl Parent; //  Was: GuiControl Parent;
public:

   struct MenuEntry
   {
      U32 pos;
      RectI bounds;

      bool visible;

      S32 bitmapIndex;
      bool drawBitmapOnly;

      bool drawBorder;

      StringTableEntry text;
      PopupMenu* popupMenu;
   };

   Vector<MenuEntry> mMenuList;

   MenuEntry *mouseDownMenu;
   MenuEntry *mouseOverMenu;

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

   U32 mMenubarHeight;

   bool mMouseInMenu;
	
	GuiMenuBar();

   void onRemove();
   bool onWake();
   void onSleep();

   virtual void addObject(SimObject* object);

	MenuEntry *findHitMenu(Point2I mousePoint);

   void onPreRender();
	void onRender(Point2I offset, const RectI &updateRect);

   void checkMenuMouseMove(const GuiEvent &event);
   void onMouseMove(const GuiEvent &event);
   void onMouseEnter(const GuiEvent &event);
   void onMouseLeave(const GuiEvent &event);
   void onMouseDown(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   
   void onAction();
   void closeMenu();
   void buildWindowAcceleratorMap( WindowInputGenerator &inputGenerator );
   void removeWindowAcceleratorMap( WindowInputGenerator &inputGenerator );
   void acceleratorKeyPress(U32 index);

   //  Added to support 'ticks'
   void processTick();

   void insert(SimObject* pObject, S32 pos);

   static void initPersistFields();

   U32 getMenuListCount() { return mMenuList.size(); }

   PopupMenu* getMenu(U32 index);
   PopupMenu* findMenu(StringTableEntry barTitle);

   DECLARE_CONOBJECT(GuiMenuBar);
   DECLARE_CALLBACK( void, onMouseInMenu, ( bool hasLeftMenu ));
   DECLARE_CALLBACK( void, onMenuSelect, ( S32 menuId, const char* menuText ));
   DECLARE_CALLBACK( void, onMenuItemSelect, ( S32 menuId, const char* menuText, S32 menuItemId, const char* menuItemText  ));
};

#endif
