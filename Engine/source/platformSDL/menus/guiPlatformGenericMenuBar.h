#pragma once

#include "gui/editor/guiMenuBar.h"
#include "platformSDL/menus/PlatformSDLPopupMenuData.h"
#include "platform/menus/popupMenu.h"

class GuiPlatformGenericMenuBar : public GuiMenuBar
{
   typedef GuiMenuBar Parent;
public:
   DECLARE_CONOBJECT(GuiPlatformGenericMenuBar);

   virtual void menuItemSelected(Menu *menu, MenuItem *item)
   {
      AssertFatal(menu && item, "");

      PopupMenu *popupMenu = PlatformPopupMenuData::mMenuMap[menu];
      AssertFatal(popupMenu, "");

      popupMenu->handleSelect(item->id);

      Parent::menuItemSelected(menu, item);
   }

protected:
   /// menu id / item id
   Map<CompoundKey<U32, U32>, String> mCmds;

};