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

#include "platform/platform.h"
#include "gui/editor/guiMenuBar.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/controls/guiTextListCtrl.h"
#include "sim/actionMap.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"
#include "console/engineAPI.h"
#include "gui/editor/guiPopupMenuCtrl.h"

// menu bar:
// basic idea - fixed height control bar at the top of a window, placed and sized in gui editor
// menu text for menus or menu items should not begin with a digit
// all menus can be removed via the clearMenus() console command
// each menu is added via the addMenu(menuText, menuId) console command
// each menu is added with a menu id
// menu items are added to menus via that addMenuItem(menu, menuItemText, menuItemId, accelerator, checkGroup) console command
// each menu item is added with a menu item id and an optional accelerator
// menu items are initially enabled, but can be disabled/re-enabled via the setMenuItemEnable(menu,menuItem,bool)
// menu text can be set via the setMenuText(menu, newMenuText) console method
// menu item text can be set via the setMenuItemText console method
// menu items can be removed via the removeMenuItem(menu, menuItem) console command
// menu items can be cleared via the clearMenuItems(menu) console command
// menus can be hidden or shown via the setMenuVisible(menu, bool) console command
// menu items can be hidden or shown via the setMenuItemVisible(menu, menuItem, bool) console command
// menu items can be check'd via the setMenuItemChecked(menu, menuItem, bool) console command
//    if the bool is true, any other items in that menu item's check group become unchecked.
//
// menu items can have a bitmap set on them via the setMenuItemBitmap(menu, menuItem, bitmapIndex)
//    passing -1 for the bitmap index will result in no bitmap being shown
//    the index paramater is an index into the bitmap array of the associated profile
//    this can be used, for example, to display a check next to a selected menu item
//    bitmap indices are actually multiplied by 3 when indexing into the bitmap
//    since bitmaps have normal, selected and disabled states.
//
// menus can be removed via the removeMenu console command
// specification arguments for menus and menu items can be either the id or the text of the menu or menu item
// adding the menu item "-" will add an un-selectable seperator to the menu
// callbacks:
// when a menu is clicked, before it is displayed, the menu calls its onMenuSelect(menuId, menuText) method -
//    this allows the callback to enable/disable menu items, or add menu items in a context-sensitive way
// when a menu item is clicked, the menu removes itself from display, then calls onMenuItemSelect(menuId, menuText, menuItemId, menuItemText)

// the initial implementation does not support:
//    hierarchal menus
//    keyboard accelerators on menu text (i.e. via alt-key combos)

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiMenuBar);

ConsoleDocClass( GuiMenuBar,
   "@brief GUI Control which displays a horizontal bar with individual drop-down menu items. Each menu item may also have submenu items.\n\n"

   "@tsexample\n"
   "new GuiMenuBar(newMenuBar)\n"
   "{\n"
   "  Padding = \"0\";\n"
   "  //Properties not specific to this control have been omitted from this example.\n"
   "};\n\n"
   "// Add a menu to the menu bar\n"
   "newMenuBar.addMenu(0,\"New Menu\");\n\n"
   "// Add a menu item to the New Menu\n"
   "newMenuBar.addMenuItem(0,\"New Menu Item\",0,\"n\",-1);\n\n"
   "// Add a submenu item to the New Menu Item\n"
   "newMenuBar.addSubmenuItem(0,1,\"New Submenu Item\",0,\"s\",-1);\n"
   "@endtsexample\n\n"

   "@see GuiTickCtrl\n\n"

   "@ingroup GuiCore\n"
);

IMPLEMENT_CALLBACK( GuiMenuBar, onMouseInMenu, void, (bool isInMenu),( isInMenu ),
   "@brief Called whenever the mouse enters, or persists is in the menu.\n\n"
   "@param isInMenu True if the mouse has entered the menu, otherwise is false.\n"
   "@note To receive this callback, call setProcessTicks(true) on the menu bar.\n"
   "@tsexample\n"
   "// Mouse enters or persists within the menu, causing the callback to occur.\n"
   "GuiMenuBar::onMouseInMenu(%this,%hasLeftMenu)\n"
   "{\n"
   "  // Code to run when the callback occurs\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl\n\n"
);

IMPLEMENT_CALLBACK( GuiMenuBar, onMenuSelect, void, ( S32 menuId, const char* menuText ),( menuId , menuText ),
   "@brief Called whenever a menu is selected.\n\n"
   "@param menuId Index id of the clicked menu\n"
   "@param menuText Text of the clicked menu\n\n"
   "@tsexample\n"
   "// A menu has been selected, causing the callback to occur.\n"
   "GuiMenuBar::onMenuSelect(%this,%menuId,%menuText)\n"
   "{\n"
   "  // Code to run when the callback occurs\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl\n\n"
);

IMPLEMENT_CALLBACK( GuiMenuBar, onMenuItemSelect, void, ( S32 menuId, const char* menuText, S32 menuItemId, const char* menuItemText ),
                                       ( menuId, menuText, menuItemId, menuItemText ),
   "@brief Called whenever an item in a menu is selected.\n\n"
   "@param menuId Index id of the menu which contains the selected menu item\n"
   "@param menuText Text of the menu which contains the selected menu item\n\n"
   "@param menuItemId Index id of the selected menu item\n"
   "@param menuItemText Text of the selected menu item\n\n"
   "@tsexample\n"
   "// A menu item has been selected, causing the callback to occur.\n"
   "GuiMenuBar::onMenuItemSelect(%this,%menuId,%menuText,%menuItemId,%menuItemText)\n"
   "{\n"
   "  // Code to run when the callback occurs\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl\n\n"
);

//------------------------------------------------------------------------------
// console methods
//------------------------------------------------------------------------------

/*DefineEngineMethod( GuiMenuBar, clearMenus, void, (),,
   "@brief Clears all the menus from the menu bar.\n\n"
   "@tsexample\n"
   "// Inform the GuiMenuBar control to clear all menus from itself.\n"
   "%thisGuiMenuBar.clearMenus();\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   object->clearMenus();
}

DefineEngineMethod( GuiMenuBar, setMenuMargins, void, (S32 horizontalMargin, S32 verticalMargin, S32 bitmapToTextSpacing),,
   "@brief Sets the menu rendering margins: horizontal, vertical, bitmap spacing.\n\n"
   "Detailed description\n\n"
   "@param horizontalMargin Number of pixels on the left and right side of a menu's text.\n"
   "@param verticalMargin Number of pixels on the top and bottom of a menu's text.\n"
   "@param bitmapToTextSpacing Number of pixels between a menu's bitmap and text.\n"
   "@tsexample\n"
   "// Define the horizontalMargin\n"
   "%horizontalMargin = \"5\";\n\n"
   "// Define the verticalMargin\n"
   "%verticalMargin = \"5\";\n\n"
   "// Define the bitmapToTextSpacing\n"
   "%bitmapToTextSpacing = \"12\";\n\n"
   "// Inform the GuiMenuBar control to set its margins based on the defined values.\n"
   "%thisGuiMenuBar.setMenuMargins(%horizontalMargin,%verticalMargin,%bitmapToTextSpacing);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   object->mHorizontalMargin = horizontalMargin;
   object->mVerticalMargin = verticalMargin;
   object->mBitmapMargin = bitmapToTextSpacing;
}

DefineEngineMethod(GuiMenuBar, addMenu, void, (const char* menuText, S32 menuId),,
   "@brief Adds a new menu to the menu bar.\n\n"
   "@param menuText Text to display for the new menu item.\n"
   "@param menuId ID for the new menu item.\n"
   "@tsexample\n"
   "// Define the menu text\n"
   "%menuText = \"New Menu\";\n\n"
   "// Define the menu ID.\n"
   "%menuId = \"2\";\n\n"
   "// Inform the GuiMenuBar control to add the new menu\n"
   "%thisGuiMenuBar.addMenu(%menuText,%menuId);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   if(dIsdigit(menuText[0]))
   {
      Con::errorf("Cannot add menu %s (id = %s).  First character of a menu's text cannot be a digit.", menuText, menuId);
      return;
   }
   object->addMenu(menuText, menuId);
}

DefineEngineMethod(GuiMenuBar, addMenuItem, void, (const char* targetMenu, const char* menuItemText, S32 menuItemId, const char* accelerator, int checkGroup, const char *cmd),
                                     ("","",0,nullAsType<const char*>(),-1,""),
   "@brief Adds a menu item to the specified menu.  The menu argument can be either the text of a menu or its id.\n\n"
   "@param menu Menu name or menu Id to add the new item to.\n"
   "@param menuItemText Text for the new menu item.\n"
   "@param menuItemId Id for the new menu item.\n"
   "@param accelerator Accelerator key for the new menu item.\n"
   "@param checkGroup Check group to include this menu item in.\n"
   "@tsexample\n"
   "// Define the menu we wish to add the item to\n"
   "%targetMenu = \"New Menu\";  or  %menu = \"4\";\n\n"
   "// Define the text for the new menu item\n"
   "%menuItemText = \"Menu Item\";\n\n"
   "// Define the id for the new menu item\n"
   "%menuItemId = \"3\";\n\n"
   "// Set the accelerator key to toggle this menu item with\n"
   "%accelerator = \"n\";\n\n"
   "// Define the Check Group that this menu item will be in, if we want it to be in a check group. -1 sets it in no check group.\n"
   "%checkGroup = \"4\";\n\n"
   "// Inform the GuiMenuBar control to add the new menu item with the defined fields\n"
   "%thisGuiMenuBar.addMenuItem(%menu,%menuItemText,%menuItemId,%accelerator,%checkGroup);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   if(dIsdigit(menuItemText[0]))
   {
      Con::errorf("Cannot add menu item %s (id = %s).  First character of a menu item's text cannot be a digit.", menuItemText, menuItemId);
      return;
   }
   GuiMenuBar::Menu *menu = object->findMenu(targetMenu);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for addMenuItem.", targetMenu);
      return;
   }
   object->addMenuItem(menu, menuItemText, menuItemId, accelerator != NULL ? accelerator : "", checkGroup == -1 ? -1 : checkGroup, cmd);
}

DefineEngineMethod(GuiMenuBar, setMenuItemEnable, void, (const char* menuTarget, const char* menuItemTarget, bool enabled),,
   "@brief sets the menu item to enabled or disabled based on the enable parameter.\n"
   "The specified menu and menu item can either be text or ids.\n\n"
   "Detailed description\n\n"
   "@param menuTarget Menu to work in\n"
   "@param menuItemTarget The menu item inside of the menu to enable or disable\n"
   "@param enabled Boolean enable / disable value.\n"
   "@tsexample\n"
   "// Define the menu\n"
   "%menu = \"New Menu\";  or  %menu = \"4\";\n\n"
   "// Define the menu item\n"
   "%menuItem = \"New Menu Item\";  or %menuItem = \"2\";\n\n"
   "// Define the enabled state\n"
   "%enabled = \"true\";\n\n"
   "// Inform the GuiMenuBar control to set the enabled state of the requested menu item\n"
   "%thisGuiMenuBar.setMenuItemEnable(%menu,%menuItme,%enabled);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemEnable.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setMenuItemEnable.", menuItemTarget);
      return;
   }
   menuItem->enabled = enabled;
}

DefineEngineMethod(GuiMenuBar, setCheckmarkBitmapIndex, void, (S32 bitmapindex),,
   "@brief Sets the menu bitmap index for the check mark image.\n\n"
   "@param bitmapIndex Bitmap index for the check mark image.\n"
   "@tsexample\n"
   "// Define the bitmap index\n"
   "%bitmapIndex = \"2\";\n\n"
   "// Inform the GuiMenuBar control of the proper bitmap index for the check mark image\n"
   "%thisGuiMenuBar.setCheckmarkBitmapIndex(%bitmapIndex);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   object->mCheckmarkBitmapIndex = bitmapindex;
}

DefineEngineMethod(GuiMenuBar, setMenuItemChecked, void, (const char* menuTarget, const char* menuItemTarget, bool checked),,
   "@brief Sets the menu item bitmap to a check mark, which by default is the first element in\n"
   "the bitmap array (although this may be changed with setCheckmarkBitmapIndex()).\n"
   "Any other menu items in the menu with the same check group become unchecked if they are checked.\n\n"
   "@param menuTarget Menu to work in\n"
   "@param menuItem Menu item to affect\n"
   "@param checked Whether we are setting it to checked or not\n"
   "@tsexample\n"
   ""
   "@endtsexample\n\n"
   "@return If not void, return value and description\n\n"
   "@see References")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemChecked.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setMenuItemChecked.", menuItemTarget);
      return;
   }
   if(checked && menuItem->checkGroup != -1)
   {
      // first, uncheck everything in the group:
      for(GuiMenuBar::MenuItem *itemWalk = menu->firstMenuItem; itemWalk; itemWalk = itemWalk->nextMenuItem)
         if(itemWalk->checkGroup == menuItem->checkGroup && itemWalk->bitmapIndex == object->mCheckmarkBitmapIndex)
            itemWalk->bitmapIndex = -1;
   }
   menuItem->bitmapIndex = checked ? object->mCheckmarkBitmapIndex : -1;
}

DefineEngineMethod(GuiMenuBar, setMenuText, void, (const char* menuTarget, const char* newMenuText),,
   "@brief Sets the text of the specified menu to the new string.\n\n"
   "@param menuTarget Menu to affect\n"
   "@param newMenuText New menu text\n"
   "@tsexample\n"
   "// Define the menu to affect"
   "%menu = \"New Menu\";  or %menu = \"3\";\n\n"
   "// Define the text to change the menu to\n"
   "%newMenuText = \"Still a New Menu\";\n\n"
   "// Inform the GuiMenuBar control to change the defined menu to the defined text\n"
   "%thisGuiMenuBar.setMenuText(%menu,%newMenuText);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   if(dIsdigit(menuTarget[0]))
   {
      Con::errorf("Cannot name menu %s to %s.  First character of a menu's text cannot be a digit.", menuTarget, newMenuText);
      return;
   }
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuText.", menuTarget);
      return;
   }
   dFree(menu->text);
   menu->text = dStrdup(newMenuText);
   object->menuBarDirty = true;
}

DefineEngineMethod(GuiMenuBar, setMenuBitmapIndex, void, (const char* menuTarget, S32 bitmapindex, bool bitmaponly, bool drawborder),,
   "@brief Sets the bitmap index for the menu and toggles rendering only the bitmap.\n\n"
   "@param menuTarget Menu to affect\n"
   "@param bitmapindex Bitmap index to set for the menu\n"
   "@param bitmaponly If true, only the bitmap will be rendered\n"
   "@param drawborder If true, a border will be drawn around the menu.\n"
   "@tsexample\n"
   "// Define the menuTarget to affect\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Set the bitmap index\n"
   "%bitmapIndex = \"5\";\n\n"
   "// Set if we are only to render the bitmap or not\n"
   "%bitmaponly = \"true\";\n\n"
   "// Set if we are rendering a border or not\n"
   "%drawborder = \"true\";\n\n"
   "// Inform the GuiMenuBar of the bitmap and rendering changes\n"
   "%thisGuiMenuBar.setMenuBitmapIndex(%menuTarget,%bitmapIndex,%bitmapOnly,%drawBorder);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuBitmapIndex.", menuTarget);
      return;
   }

   menu->bitmapIndex = bitmapindex;
   menu->drawBitmapOnly = bitmaponly;
   menu->drawBorder = drawborder;

   object->menuBarDirty = true;
}

DefineEngineMethod(GuiMenuBar, setMenuVisible, void, (const char* menuTarget, bool visible),,
   "@brief Sets the whether or not to display the specified menu.\n\n"
   "@param menuTarget Menu item to affect\n"
   "@param visible Whether the menu item will be visible or not\n"
   "@tsexample\n"
   "// Define the menu to work with\n"
   "%menuTarget = \"New Menu\";  or  %menuTarget = \"4\";\n\n"
   "// Define if the menu should be visible or not\n"
   "%visible = \"true\";\n\n"
   "// Inform the GuiMenuBar control of the new visibility state for the defined menu\n"
   "%thisGuiMenuBar.setMenuVisible(%menuTarget,%visible);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuVisible.", menuTarget);
      return;
   }
   menu->visible = visible;
   object->menuBarDirty = true;
   object->setUpdate();
}

DefineEngineMethod(GuiMenuBar, setMenuItemText, void, (const char* menuTarget, const char* menuItemTarget, const char* newMenuItemText),,
   "@brief Sets the text of the specified menu item to the new string.\n\n"
   "@param menuTarget Menu to affect\n"
   "@param menuItem Menu item in the menu to change the text at\n"
   "@param newMenuItemText New menu text\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or  %menuTarget = \"4\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"2\";\n\n"
   "// Define the new text for the menu item\n"
   "%newMenuItemText = \"Very New Menu Item\";\n\n"
   "// Inform the GuiMenuBar control to change the defined menu item with the new text\n"
   "%thisGuiMenuBar.setMenuItemText(%menuTarget,%menuItem,%newMenuItemText);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   if(dIsdigit(newMenuItemText[0]))
   {
      Con::errorf("Cannot name menu item %s to %s.  First character of a menu item's text cannot be a digit.", menuItemTarget, newMenuItemText);
      return;
   }
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemText.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setMenuItemText.", menuItemTarget);
      return;
   }
   dFree(menuItem->text);
   menuItem->text = dStrdup(newMenuItemText);
}

DefineEngineMethod(GuiMenuBar, setMenuItemVisible, void, (const char* menuTarget, const char* menuItemTarget, bool isVisible),,
   "@brief Brief Description.\n\n"
   "Detailed description\n\n"
   "@param menuTarget Menu to affect the menu item in\n"
   "@param menuItem Menu item to affect\n"
   "@param isVisible Visible state to set the menu item to.\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or  %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"2\";\n\n"
   "// Define the visibility state\n"
   "%isVisible = \"true\";\n\n"
   "// Inform the GuiMenuBarControl of the visibility state of the defined menu item\n"
   "%thisGuiMenuBar.setMenuItemVisible(%menuTarget,%menuItem,%isVisible);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemVisible.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setMenuItemVisible.", menuItemTarget);
      return;
   }
   menuItem->visible = isVisible;
}

DefineEngineMethod(GuiMenuBar, setMenuItemBitmap, void, (const char* menuTarget, const char* menuItemTarget, S32 bitmapIndex),,
   "@brief Sets the specified menu item bitmap index in the bitmap array.  Setting the item's index to -1 will remove any bitmap.\n\n"
   "@param menuTarget Menu to affect the menuItem in\n"
   "@param menuItem Menu item to affect\n"
   "@param bitmapIndex Bitmap index to set the menu item to\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or  %menuTarget = \"3\";\n\n"
   "// Define the menuItem\"\n"
   "%menuItem = \"New Menu Item\";  or %menuItem = \"2\";\n\n"
   "// Define the bitmapIndex\n"
   "%bitmapIndex = \"6\";\n\n"
   "// Inform the GuiMenuBar control to set the menu item to the defined bitmap\n"
   "%thisGuiMenuBar.setMenuItemBitmap(%menuTarget,%menuItem,%bitmapIndex);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemBitmap.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setMenuItemBitmap.", menuItemTarget);
      return;
   }
   menuItem->bitmapIndex = bitmapIndex;
}

DefineEngineMethod(GuiMenuBar, removeMenuItem, void, (const char* menuTarget, const char* menuItemTarget),,
   "@brief Removes the specified menu item from the menu.\n\n"
   "@param menuTarget Menu to affect the menu item in\n"
   "@param menuItem Menu item to affect\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"5\";\n\n"
   "// Request the GuiMenuBar control to remove the define menu item\n"
   "%thisGuiMenuBar.removeMenuItem(%menuTarget,%menuItem);\n\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for removeMenuItem.", menuTarget);
      return;
   }
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for removeMenuItem.", menuItemTarget);
      return;
   }
   object->removeMenuItem(menu, menuItem);
}

DefineEngineMethod(GuiMenuBar, clearMenuItems, void, (const char* menuTarget),,
   "@brief Removes all the menu items from the specified menu.\n\n"
   "@param menuTarget Menu to remove all items from\n"  
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Inform the GuiMenuBar control to clear all menu items from the defined menu\n"
   "%thisGuiMenuBar.clearMenuItems(%menuTarget);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      //Con::errorf("Cannot find menu %s for clearMenuItems.", menuTarget);
      return;
   }
   object->clearMenuItems(menu);
}

DefineEngineMethod( GuiMenuBar, removeMenu, void, (const char* menuTarget),,
   "@brief Removes the specified menu from the menu bar.\n\n"
   "@param menuTarget Menu to remove from the menu bar\n"  
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Inform the GuiMenuBar to remove the defined menu from the menu bar\n"
   "%thisGuiMenuBar.removeMenu(%menuTarget);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      //Con::errorf("Cannot find menu %s for removeMenu.", menuTarget);
      return;
   }
   object->clearMenuItems(menu);
   object->menuBarDirty = true;
}

//------------------------------------------------------------------------------
//  Submenu console methods
//------------------------------------------------------------------------------

DefineEngineMethod(GuiMenuBar, setMenuItemSubmenuState, void, (const char* menuTarget, const char* menuItem, bool isSubmenu),,
   "@brief Sets the given menu item to be a submenu.\n\n"
   "@param menuTarget Menu to affect a submenu in\n"
   "@param menuItem Menu item to affect\n"
   "@param isSubmenu Whether or not the menuItem will become a subMenu or not\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"5\";\n\n"
   "// Define whether or not the Menu Item is a sub menu or not\n"
   "%isSubmenu = \"true\";\n\n"
   "// Inform the GuiMenuBar control to set the defined menu item to be a submenu or not.\n"
   "%thisGuiMenuBar.setMenuItemSubmenuState(%menuTarget,%menuItem,%isSubmenu);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setMenuItemSubmenuState.", menuTarget);
      return;
   }

   GuiMenuBar::MenuItem *menuitem = object->findMenuItem(menu, menuItem);
   if(!menuitem)
   {
      Con::errorf("Cannot find menuitem %s for setMenuItemSubmenuState.", menuItem);
      return;
   }

   menuitem->isSubmenu = isSubmenu;
}

DefineEngineMethod(GuiMenuBar, addSubmenuItem, void, (const char* menuTarget, const char* menuItem, const char* submenuItemText, 
                                         int submenuItemId, const char* accelerator, int checkGroup),,
   "@brief Adds a menu item to the specified menu.  The menu argument can be either the text of a menu or its id.\n\n"
   "@param menuTarget Menu to affect a submenu in\n"
   "@param menuItem Menu item to affect\n"
   "@param submenuItemText Text to show for the new submenu\n"
   "@param submenuItemId Id for the new submenu\n"
   "@param accelerator Accelerator key for the new submenu\n"
   "@param checkGroup Which check group the new submenu should be in, or -1 for none.\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or  %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"5\";\n\n"
   "// Define the text for the new submenu\n"
   "%submenuItemText = \"New Submenu Item\";\n\n"
   "// Define the id for the new submenu\n"
   "%submenuItemId = \"4\";\n\n"
   "// Define the accelerator key for the new submenu\n"
   "%accelerator = \"n\";\n\n"
   "// Define the checkgroup for the new submenu\n"
   "%checkgroup = \"7\";\n\n"
   "// Request the GuiMenuBar control to add the new submenu with the defined information\n"
   "%thisGuiMenuBar.addSubmenuItem(%menuTarget,%menuItem,%submenuItemText,%submenuItemId,%accelerator,%checkgroup);\n"
   "@endtsexample\n\n"
   "@see GuiTickCtrl\n")
{
   if(dIsdigit(submenuItemText[0]))
   {
      Con::errorf("Cannot add submenu item %s (id = %s).  First character of a menu item's text cannot be a digit.", submenuItemText, submenuItemId);
      return;
   }

   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for addMenuItem.", menuTarget);
      return;
   }

   GuiMenuBar::MenuItem *menuitem = object->findMenuItem(menu, menuItem);
   if(!menuitem)
   {
      Con::errorf("Cannot find menuitem %s for addSubmenuItem.", menuItem);
      return;
   }

   object->addSubmenuItem(menu, menuitem, submenuItemText, submenuItemId, !accelerator ? "" : accelerator, checkGroup == -1 ? -1 : checkGroup);
}

DefineEngineMethod(GuiMenuBar, clearSubmenuItems, void, (const char* menuTarget, const char* menuItem),,
   "@brief Removes all the menu items from the specified submenu.\n\n"
   "@param menuTarget Menu to affect a submenu in\n"
   "@param menuItem Menu item to affect\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"5\";\n\n"
   "// Inform the GuiMenuBar to remove all submenu items from the defined menu item\n"
   "%thisGuiMenuBar.clearSubmenuItems(%menuTarget,%menuItem);\n\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for clearSubmenuItems.", menuTarget);
      return;
   }

   GuiMenuBar::MenuItem *menuitem = object->findMenuItem(menu, menuItem);
   if(!menuitem)
   {
      Con::errorf("Cannot find menuitem %s for clearSubmenuItems.", menuItem);
      return;
   }

   object->clearSubmenuItems(menuitem);
}

DefineEngineMethod(GuiMenuBar, setSubmenuItemChecked, void, (const char* menuTarget, const char* menuItemTarget, const char* submenuItemText, bool checked),,
   "@brief Sets the menu item bitmap to a check mark, which by default is the first element in the\n"
   "bitmap array (although this may be changed with setCheckmarkBitmapIndex()).\n"
   "Any other menu items in the menu with the same check group become unchecked if they are checked.\n\n"
   "@param menuTarget Menu to affect a submenu in\n"
   "@param menuItem Menu item to affect\n"
   "@param submenuItemText Text to show for submenu\n"
   "@param checked Whether or not this submenu item will be checked.\n"
   "@tsexample\n"
   "// Define the menuTarget\n"
   "%menuTarget = \"New Menu\";  or %menuTarget = \"3\";\n\n"
   "// Define the menuItem\n"
   "%menuItem = \"New Menu Item\";  or  %menuItem = \"5\";\n\n"
   "// Define the text for the new submenu\n"
   "%submenuItemText = \"Submenu Item\";\n\n"
   "// Define if this submenu item should be checked or not\n"
   "%checked = \"true\";\n\n"
   "// Inform the GuiMenuBar control to set the checked state of the defined submenu item\n"
   "%thisGuiMenuBar.setSubmenuItemChecked(%menuTarget,%menuItem,%submenuItemText,%checked);\n"
   "@endtsexample\n\n"
   "@return If not void, return value and description\n\n"
   "@see References")
{
   // Find the parent menu
   GuiMenuBar::Menu *menu = object->findMenu(menuTarget);
   if(!menu)
   {
      Con::errorf("Cannot find menu %s for setSubmenuItemChecked.", menuTarget);
      return;
   }

   // Find the parent menu item
   GuiMenuBar::MenuItem *menuItem = object->findMenuItem(menu, menuItemTarget);
   if(!menuItem)
   {
      Con::errorf("Cannot find menu item %s for setSubmenuItemChecked.", menuItemTarget);
      return;
   }

   // Find the submenu item
   GuiMenuBar::MenuItem *submenuItem = object->findSubmenuItem(menu, menuItemTarget, submenuItemText);
   if(!submenuItem)
   {
      Con::errorf("Cannot find submenu item %s for setSubmenuItemChecked.", submenuItemText);
      return;
   }

   if(checked && submenuItem->checkGroup != -1)
   {
      // first, uncheck everything in the group:
      for(GuiMenuBar::MenuItem *itemWalk = menuItem->submenu->firstMenuItem; itemWalk; itemWalk = itemWalk->nextMenuItem)
         if(itemWalk->checkGroup == submenuItem->checkGroup && itemWalk->bitmapIndex == object->mCheckmarkBitmapIndex)
            itemWalk->bitmapIndex = -1;
   }
   submenuItem->bitmapIndex = checked ? object->mCheckmarkBitmapIndex : -1;
}

//------------------------------------------------------------------------------
// menu management methods
//------------------------------------------------------------------------------
GuiMenuBar::Menu* GuiMenuBar::sCreateMenu(const char *menuText, U32 menuId)
{
   // allocate the menu
   Menu *newMenu = new Menu;
   newMenu->text = dStrdup(menuText);
   newMenu->id = menuId;
   newMenu->nextMenu = NULL;
   newMenu->firstMenuItem = NULL;
   newMenu->visible = true;

   // Menu bitmap variables
   newMenu->bitmapIndex = -1;
   newMenu->drawBitmapOnly = false;
   newMenu->drawBorder = true;

   return newMenu;
}

void GuiMenuBar::addMenu(GuiMenuBar::Menu *newMenu, S32 pos)
{
   // add it to the menu list
   menuBarDirty = true;
   if (pos == -1)
      mMenuList.push_back(newMenu);
   else
      mMenuList.insert(pos, newMenu);
}

void GuiMenuBar::addMenu(const char *menuText, U32 menuId)
{
   Menu *newMenu = sCreateMenu(menuText, menuId);
   
   addMenu(newMenu);
}

GuiMenuBar::Menu *GuiMenuBar::findMenu(const char *menu)
{
   if(dIsdigit(menu[0]))
   {
      U32 id = dAtoi(menu);
      for (U32 i = 0; i < mMenuList.size(); ++i)
         if (id == mMenuList[i].id)
            return mMenuList[i];
      return NULL;
   }
   else
   {
      for (U32 i = 0; i < mMenuList.size(); ++i)
         if (!dStricmp(menu, mMenuList[i].text))
            return mMenuList[i];
      return NULL;
   }
}

GuiMenuBar::MenuItem *GuiMenuBar::findMenuItem(Menu *menu, const char *menuItem)
{
   if(dIsdigit(menuItem[0]))
   {
      U32 id = dAtoi(menuItem);
      for(MenuItem *walk = menu->firstMenuItem; walk; walk = walk->nextMenuItem)
         if(id == walk->id)
            return walk;
      return NULL;
   }
   else
   {
      for(MenuItem *walk = menu->firstMenuItem; walk; walk = walk->nextMenuItem)
         if(!dStricmp(menuItem, walk->text))
            return walk;
      return NULL;
   }
}

void GuiMenuBar::removeMenu(Menu *menu)
{
   menuBarDirty = true;
   clearMenuItems(menu);

   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      if (mMenuList[i] == menu)
      {
         mMenuList.erase(i);
         break;
      }
   }
}

void GuiMenuBar::removeMenuItem(Menu *menu, MenuItem *menuItem)
{
   for(MenuItem **walk = &menu->firstMenuItem; *walk; walk = &(*walk)->nextMenuItem)
   {
      if(*walk == menuItem)
      {
         *walk = menuItem->nextMenuItem;
         break;
      }
   }

   //  If this is a submenu, then be sure to clear the submenu's items
   if(menuItem->isSubmenu)
   {
      clearSubmenuItems(menuItem);
   }

   dFree(menuItem->text);
   dFree(menuItem->accelerator);
   delete menuItem;
}

GuiMenuBar::MenuItem* GuiMenuBar::addMenuItem(Menu *menu, const char *text, U32 id, const char *accelerator, S32 checkGroup, const char *cmd )
{
   // allocate the new menu item
   MenuItem *newMenuItem = new MenuItem;
   newMenuItem->text = dStrdup(text);
   if(accelerator[0])
      newMenuItem->accelerator = dStrdup(accelerator);
   else
      newMenuItem->accelerator = NULL;
   newMenuItem->cmd = cmd;
   newMenuItem->id = id;
   newMenuItem->checkGroup = checkGroup;
   newMenuItem->nextMenuItem = NULL;
   newMenuItem->acceleratorIndex = 0;
   newMenuItem->enabled = text[0] != '-';
   newMenuItem->visible = true;
   newMenuItem->bitmapIndex = -1;

   //  Default to not having a submenu
   newMenuItem->isSubmenu = false;
   newMenuItem->submenu = NULL;
   newMenuItem->submenuParentMenu = NULL;

   // link it into the menu's menu item list
   if(menu)
   {
      MenuItem **walk = &menu->firstMenuItem;
      while(*walk)
         walk = &(*walk)->nextMenuItem;
      *walk = newMenuItem;
   }

   return newMenuItem;
}

GuiMenuBar::MenuItem* GuiMenuBar::addMenuItem(Menu *menu, MenuItem* newMenuItem)
{
   // link it into the menu's menu item list
   if(menu)
   {
      MenuItem **walk = &menu->firstMenuItem;
      while(*walk)
         walk = &(*walk)->nextMenuItem;
      *walk = newMenuItem;
   }

   return newMenuItem;
}

void GuiMenuBar::clearMenuItems(Menu *menu)
{
   while(menu->firstMenuItem)
      removeMenuItem(menu, menu->firstMenuItem);
}

void GuiMenuBar::clearMenus()
{
   mMenuList.clear();
}

void GuiMenuBar::attachToMenuBar(Menu* menu, S32 pos)
{
   addMenu(menu, pos);
}

void GuiMenuBar::removeFromMenuBar(Menu* menu)
{
   menuBarDirty = true;

   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      if (mMenuList[i] == menu)
      {
         mMenuList.erase(i);
         break;
      }
   }
}

//------------------------------------------------------------------------------
//  Submenu methods
//------------------------------------------------------------------------------

//  This method will return the MenuItem class of of a submenu's menu item given
// its parent menu and parent menuitem.  If the menuitem ID is used, then the submenu
// ID must also be used.
GuiMenuBar::MenuItem *GuiMenuBar::findSubmenuItem(Menu *menu, const char *menuItem, const char *submenuItem)
{
   if(dIsdigit(menuItem[0]))
   {
      //  Search by ID
      U32 id = dAtoi(menuItem);
      for(MenuItem *walk = menu->firstMenuItem; walk; walk = walk->nextMenuItem)
         if(id == walk->id)
       {
          if(walk->isSubmenu && walk->submenu)
         {
            return GuiMenuBar::findMenuItem(walk->submenu, submenuItem);
         }
         return NULL;
       }
      return NULL;
   }
   else
   {
      //  Search by name
      for(MenuItem *walk = menu->firstMenuItem; walk; walk = walk->nextMenuItem)
         if(!dStricmp(menuItem, walk->text))
       {
          if(walk->isSubmenu && walk->submenu)
         {
            return GuiMenuBar::findMenuItem(walk->submenu, submenuItem);
         }
         return NULL;
       }
      return NULL;
   }
}

GuiMenuBar::MenuItem* GuiMenuBar::findSubmenuItem(MenuItem *menuItem, const char *submenuItem)
{
   if( !menuItem->isSubmenu )
      return NULL;

   return GuiMenuBar::findMenuItem( menuItem->submenu, submenuItem );
}

//  Add a menuitem to the given submenu
void GuiMenuBar::addSubmenuItem(Menu *menu, MenuItem *submenu, const char *text, U32 id, const char *accelerator, S32 checkGroup)
{
   // Check that the given menu item supports a submenu
   if(submenu && !submenu->isSubmenu)
   {
      Con::errorf("GuiMenuBar::addSubmenuItem: Attempting to add menuitem '%s' to an invalid submenu",text);
     return;
   }

   // allocate the new menu item
   MenuItem *newMenuItem = new MenuItem;
   newMenuItem->text = dStrdup(text);
   if(accelerator[0])
      newMenuItem->accelerator = dStrdup(accelerator);
   else
      newMenuItem->accelerator = NULL;
   newMenuItem->id = id;
   newMenuItem->checkGroup = checkGroup;
   newMenuItem->nextMenuItem = NULL;
   newMenuItem->acceleratorIndex = 0;
   newMenuItem->enabled = (dStrlen(text) > 1 || text[0] != '-');
   newMenuItem->visible = true;
   newMenuItem->bitmapIndex = -1;

   //  Default to not having a submenu
   newMenuItem->isSubmenu = false;
   newMenuItem->submenu = NULL;

   //  Point back to the submenu's menu
   newMenuItem->submenuParentMenu = menu;

   // link it into the menu's menu item list
   MenuItem **walk = &submenu->submenu->firstMenuItem;
   while(*walk)
      walk = &(*walk)->nextMenuItem;
   *walk = newMenuItem;
}

void GuiMenuBar::addSubmenuItem(Menu *menu, MenuItem *submenu, MenuItem *newMenuItem )
{
   AssertFatal( submenu && newMenuItem, "");

   //  Point back to the submenu's menu
   newMenuItem->submenuParentMenu = menu;

   // link it into the menu's menu item list
   MenuItem **walk = &submenu->submenu->firstMenuItem;
   while(*walk)
      walk = &(*walk)->nextMenuItem;
   *walk = newMenuItem;
}

//  Remove a submenu item
void GuiMenuBar::removeSubmenuItem(MenuItem *menuItem, MenuItem *submenuItem)
{
   // Check that the given menu item supports a submenu
   if(menuItem && !menuItem->isSubmenu)
   {
      Con::errorf("GuiMenuBar::removeSubmenuItem: Attempting to remove submenuitem '%s' from an invalid submenu",submenuItem->text);
     return;
   }

   GuiMenuBar::removeMenuItem(menuItem->submenu, submenuItem);
}

//  Clear all menuitems from a submenu
void GuiMenuBar::clearSubmenuItems(MenuItem *menuitem)
{
   // Check that the given menu item supports a submenu
   if(menuitem && !menuitem->isSubmenu)
   {
      Con::errorf("GuiMenuBar::clearSubmenuItems: Attempting to clear an invalid submenu");
     return;
   }

   while(menuitem->submenu->firstMenuItem)
      removeSubmenuItem(menuitem, menuitem->submenu->firstMenuItem);
}
*/
//------------------------------------------------------------------------------
// initialization, input and render methods
//------------------------------------------------------------------------------

GuiMenuBar::GuiMenuBar()
{
   //mMenuList.clear();
   menuBarDirty = true;
   mouseDownMenu = NULL;
   mouseOverMenu = NULL;
   mCurAcceleratorIndex = 0;
   mPadding = 0;

   mCheckmarkBitmapIndex = 0; // Default to the first image in the bitmap array for the check mark

   mHorizontalMargin = 6; // Default number of pixels on the left and right side of a manu's text
   mVerticalMargin = 1;   // Default number of pixels on the top and bottom of a menu's text
   mBitmapMargin = 2;     // Default number of pixels between a menu's bitmap and text

   mMenubarHeight = 20;

   //  Added:
   mouseDownSubmenu = NULL;
   mouseOverSubmenu = NULL;

   mMouseInMenu = false;

   setProcessTicks(false);
}

void GuiMenuBar::onRemove()
{
   GuiPopupMenuBackgroundCtrl* backgroundCtrl;
   if (Sim::findObject("PopUpMenuControl", backgroundCtrl))
   {
      if (backgroundCtrl->mMenuBarCtrl == this)
         backgroundCtrl->mMenuBarCtrl = nullptr;
   }

   Parent::onRemove();
}

void GuiMenuBar::initPersistFields()
{
   addField("padding", TypeS32, Offset( mPadding, GuiMenuBar ),"Extra padding to add to the bounds of the control.\n");

   addField("menubarHeight", TypeS32, Offset(mMenubarHeight, GuiMenuBar), "Sets the height of the menubar when attached to the canvas.\n");

   Parent::initPersistFields();
}

bool GuiMenuBar::onWake()
{
   if(!Parent::onWake())
      return false;
   mProfile->constructBitmapArray();  // if a bitmap was specified...
   maxBitmapSize.set(0,0);
   S32 numBitmaps = mProfile->mBitmapArrayRects.size();
   if(numBitmaps)
   {
      RectI *bitmapBounds = mProfile->mBitmapArrayRects.address();
      for(S32 i = 0; i < numBitmaps; i++)
      {
         if(bitmapBounds[i].extent.x > maxBitmapSize.x)
            maxBitmapSize.x = bitmapBounds[i].extent.x;
         if(bitmapBounds[i].extent.y > maxBitmapSize.y)
            maxBitmapSize.y = bitmapBounds[i].extent.y;
      }
   }
   return true;
}

void GuiMenuBar::addObject(SimObject* object)
{
   PopupMenu* popup = dynamic_cast<PopupMenu*>(object);

   if (!popup)
   {
      //if it's not a popup, handle it normally
      Parent::addObject(object);
   }
   else
   {
      //otherwise, if it IS a popup, don't add it as a child object, but instead just insert it as a menu entry
      insert(object, -1);
   }
}

GuiMenuBar::MenuEntry *GuiMenuBar::findHitMenu(Point2I mousePoint)
{
   Point2I pos = globalToLocalCoord(mousePoint);

   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      if (mMenuList[i].visible && mMenuList[i].bounds.pointInRect(pos))
         return &mMenuList[i];
   }

   return NULL;
}

void GuiMenuBar::onPreRender()
{
   setHeight(mMenubarHeight);

   Parent::onPreRender();
   if (menuBarDirty)
   {
      menuBarDirty = false;
      U32 curX = mPadding;
      for (U32 i = 0; i < mMenuList.size(); ++i)
      {
         if (!mMenuList[i].visible)
            continue;

         // Bounds depends on if there is a bitmap to be drawn or not
         if (mMenuList[i].bitmapIndex == -1)
         {
            // Text only
            mMenuList[i].bounds.set(curX, 0, mProfile->mFont->getStrWidth(mMenuList[i].text) + (mHorizontalMargin * 2), getHeight() - (mVerticalMargin * 2));

         }
         else
         {
            // Will the bitmap and text be draw?
            if (!mMenuList[i].drawBitmapOnly)
            {
               // Draw the bitmap and the text
               RectI *bitmapBounds = mProfile->mBitmapArrayRects.address();
               mMenuList[i].bounds.set(curX, 0, bitmapBounds[mMenuList[i].bitmapIndex].extent.x + mProfile->mFont->getStrWidth(mMenuList[i].text) + (mHorizontalMargin * 2), getHeight() + (mVerticalMargin * 2));

            }
            else
            {
               // Only the bitmap will be drawn
               RectI *bitmapBounds = mProfile->mBitmapArrayRects.address();
               mMenuList[i].bounds.set(curX, 0, bitmapBounds[mMenuList[i].bitmapIndex].extent.x + mBitmapMargin + (mHorizontalMargin * 2), getHeight() + (mVerticalMargin * 2));
            }
         }

         curX += mMenuList[i].bounds.extent.x;
      }
      mouseOverMenu = NULL;
      mouseDownMenu = NULL;
   }
}

void GuiMenuBar::checkMenuMouseMove(const GuiEvent &event)
{
   MenuEntry *hit = findHitMenu(event.mousePoint);
   if(hit && hit != mouseDownMenu)
   {
      // gotta close out the current menu...
      mouseDownMenu->popupMenu->hidePopup();
      
      mouseOverMenu = mouseDownMenu = hit;
      setUpdate();
      onAction();
   }
}

void GuiMenuBar::onMouseMove(const GuiEvent &event)
{
   MenuEntry *hit = findHitMenu(event.mousePoint);

   if (mouseDownMenu != nullptr && hit != nullptr)
   {
      //we have a standing click, so just update and go
      mouseDownMenu = mouseOverMenu = hit;
      setUpdate();
      onAction();

      return;
   }

   mouseOverMenu = hit;
   setUpdate();
}

void GuiMenuBar::onMouseEnter(const GuiEvent &event)
{
   onMouseInMenu_callback(true);
   mMouseInMenu = true;
}

void GuiMenuBar::onMouseLeave(const GuiEvent &event)
{
   if(mouseOverMenu)
      setUpdate();

   mouseOverMenu = NULL;
   mMouseInMenu = false;
}

void GuiMenuBar::onMouseDragged(const GuiEvent &event)
{
}

void GuiMenuBar::onMouseDown(const GuiEvent &event)
{
}

void GuiMenuBar::onMouseUp(const GuiEvent &event)
{
   mouseDownMenu = mouseOverMenu = findHitMenu(event.mousePoint);
   setUpdate();
   onAction();
}

void GuiMenuBar::onRender(Point2I offset, const RectI &updateRect)
{
   Point2I extent = getExtent();

   RectI ctrlRect(offset, extent);

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   //if opaque, fill the update rect with the fill color
   if (mProfile->mOpaque)
      drawUtil->drawRectFill(RectI(offset, extent), mProfile->mFillColor);

   //if there's a border, draw the border
   if (mProfile->mBorder)
      renderBorder(ctrlRect, mProfile);

   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      if (!mMenuList[i].visible)
         continue;

      ColorI fontColor = mProfile->mFontColor;
      RectI bounds = mMenuList[i].bounds;
      bounds.point += offset;

      Point2I start;

      start.x = mMenuList[i].bounds.point.x + mHorizontalMargin;
      start.y = mMenuList[i].bounds.point.y + (mMenuList[i].bounds.extent.y - mProfile->mFont->getHeight()) / 2;

      // Draw the border
      if (mMenuList[i].drawBorder)
      {
         RectI highlightBounds = bounds;
         highlightBounds.inset(1, 1);
         if (&mMenuList[i] == mouseDownMenu)
            renderFilledBorder(highlightBounds, mProfile->mBorderColorHL, mProfile->mFillColorHL);
         else if (&mMenuList[i] == mouseOverMenu && mouseDownMenu == NULL)
            renderFilledBorder(highlightBounds, mProfile->mBorderColorHL, mProfile->mFillColorHL);
      }

      // Do we draw a bitmap?
      if (mMenuList[i].bitmapIndex != -1)
      {
         S32 index = mMenuList[i].bitmapIndex * 3;
         if (&mMenuList[i] == mouseDownMenu)
            ++index;
         else if (&mMenuList[i] == mouseOverMenu && mouseDownMenu == NULL)
            index += 2;

         RectI rect = mProfile->mBitmapArrayRects[index];

         Point2I bitmapstart(start);
         bitmapstart.y = mMenuList[i].bounds.point.y + (mMenuList[i].bounds.extent.y - rect.extent.y) / 2;

         drawUtil->clearBitmapModulation();
         drawUtil->drawBitmapSR(mProfile->mTextureObject, offset + bitmapstart, rect);

         // Should we also draw the text?
         if (!mMenuList[i].drawBitmapOnly)
         {
            start.x += mBitmapMargin;
            drawUtil->setBitmapModulation(fontColor);
            drawUtil->drawText(mProfile->mFont, start + offset, mMenuList[i].text, mProfile->mFontColors);
         }
      }
      else
      {
         drawUtil->setBitmapModulation(fontColor);
         drawUtil->drawText(mProfile->mFont, start + offset, mMenuList[i].text, mProfile->mFontColors);
      }
   }

   renderChildControls(offset, updateRect);
}

void GuiMenuBar::buildWindowAcceleratorMap(WindowInputGenerator &inputGenerator)
{
   // ok, accelerator map is cleared...
   // add all our keys:
   mCurAcceleratorIndex = 1;

   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      for (U32 item = 0; item < mMenuList[i].popupMenu->mMenuItems.size(); item++)
      {
         if (!mMenuList[i].popupMenu->mMenuItems[item].mAccelerator)
         {
            mMenuList[i].popupMenu->mMenuItems[item].mAccelerator = 0;
            continue;
         }

         EventDescriptor accelEvent;
         ActionMap::createEventDescriptor(mMenuList[i].popupMenu->mMenuItems[item].mAccelerator, &accelEvent);

         //now we have a modifier, and a key, add them to the canvas
         inputGenerator.addAcceleratorKey(this, mMenuList[i].popupMenu->mMenuItems[item].mCMD, accelEvent.eventCode, accelEvent.flags);

         mMenuList[i].popupMenu->mMenuItems[item].mAcceleratorIndex = mCurAcceleratorIndex;
         mCurAcceleratorIndex++;
      }
   }
}

void GuiMenuBar::removeWindowAcceleratorMap( WindowInputGenerator &inputGenerator )
{
    inputGenerator.removeAcceleratorKeys( this );
}

void GuiMenuBar::acceleratorKeyPress(U32 index)
{
   // loop through all the menus
   // and find the item that corresponds to the accelerator index
   for (U32 i = 0; i < mMenuList.size(); ++i)
   {
      if (!mMenuList[i].visible)
         continue;

      for(U32 item = 0; item < mMenuList[i].popupMenu->mMenuItems.size(); item++)
      {
         if(mMenuList[i].popupMenu->mMenuItems[item].mAcceleratorIndex == index)
         {
            // first, call the script callback for menu selection:
            onMenuSelect_callback(mMenuList[i].popupMenu->getId(), mMenuList[i].text);
            return;
         }
      }
   }
}

void GuiMenuBar::onSleep()
{
   Parent::onSleep();
}

//------------------------------------------------------------------------------
void GuiMenuBar::onAction()
{
   if(!mouseDownMenu)
      return;

   mouseDownMenu->popupMenu->hidePopup();

   // first, call the script callback for menu selection:
   onMenuSelect_callback(mouseDownMenu->popupMenu->getId(), mouseDownMenu->text);

   mouseDownMenu->popupMenu->mMenuBarCtrl = this;

   GuiCanvas *root = getRoot();
   Point2I pos = Point2I(mouseDownMenu->bounds.point.x, mouseDownMenu->bounds.point.y + mouseDownMenu->bounds.extent.y);
   mouseDownMenu->popupMenu->showPopup(root, pos.x, pos.y);
}

//  Process a tick
void GuiMenuBar::processTick()
{
   if(mMouseInMenu)
      onMouseInMenu_callback(true);
}

void GuiMenuBar::insert(SimObject* pObject, S32 pos)
{
   PopupMenu* menu = dynamic_cast<PopupMenu*>(pObject);
   if (menu == nullptr)
      return;

   MenuEntry newMenu;
   newMenu.pos = pos >= mMenuList.size() || pos == -1 ? pos = mMenuList.size() : pos;
   newMenu.drawBitmapOnly = false;
   newMenu.drawBorder = true;
   newMenu.bitmapIndex = -1;
   newMenu.text = menu->mBarTitle;
   newMenu.visible = true;
   newMenu.popupMenu = menu;

   if (pos >= mMenuList.size() || pos == -1)
      mMenuList.push_back(newMenu);
   else
      mMenuList.insert(pos, newMenu);
}

PopupMenu* GuiMenuBar::getMenu(U32 index)
{
   if (index >= mMenuList.size())
      return nullptr;

   return mMenuList[index].popupMenu;
}

PopupMenu* GuiMenuBar::findMenu(String barTitle)
{
   for (U32 i = 0; i < mMenuList.size(); i++)
   {
      if (String::ToLower(mMenuList[i].text) == String::ToLower(barTitle))
         return mMenuList[i].popupMenu;
   }

   return nullptr;
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------
DefineEngineMethod(GuiMenuBar, attachToCanvas, void, (const char *canvas, S32 pos), , "(GuiCanvas, pos)")
{
   GuiCanvas* canv = dynamic_cast<GuiCanvas*>(Sim::findObject(canvas));
   if (canv)
   {
      canv->setMenuBar(object);
   }
}

DefineEngineMethod(GuiMenuBar, removeFromCanvas, void, (), , "()")
{
   GuiCanvas* canvas = object->getRoot();

   if(canvas)
      canvas->setMenuBar(nullptr);
}

DefineEngineMethod(GuiMenuBar, getMenuCount, S32, (), , "()")
{
   return object->getMenuListCount();
}

DefineEngineMethod(GuiMenuBar, getMenu, S32, (S32 index), (0), "(Index)")
{
   return object->getMenu(index)->getId();
}

//-----------------------------------------------------------------------------
DefineEngineMethod(GuiMenuBar, insert, void, (SimObject* pObject, S32 pos), (nullAsType<SimObject*>(), -1), "(object, pos) insert object at position")
{
   object->insert(pObject, pos);
}

DefineEngineMethod(GuiMenuBar, findMenu, S32, (const char* barTitle), (""), "(barTitle)")
{
   PopupMenu* menu = object->findMenu(barTitle);

   if (menu)
      return menu->getId();
   else
      return 0;
}
