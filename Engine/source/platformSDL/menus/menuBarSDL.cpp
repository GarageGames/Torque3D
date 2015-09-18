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

#include "platform/menus/menuBar.h"
#include "platform/menus/popupMenu.h"
#include "gui/core/guiCanvas.h"
#include "windowManager/platformWindowMgr.h"
#include "core/util/safeDelete.h"

#include "windowManager/sdl/sdlWindow.h"
#include "gui/editor/guiMenuBar.h"

#include "platformSDL/menus/PlatformSDLPopupMenuData.h"

#ifdef TORQUE_SDL

//-----------------------------------------------------------------------------
// Platform Data
//-----------------------------------------------------------------------------

// class PlatformMenuBarData
// {
// 
// };

Map<GuiMenuBar::Menu*, PopupMenu*> PlatformPopupMenuData::mMenuMap;

class GuiPlatformGenericMenuBar : public GuiMenuBar
{
   typedef GuiMenuBar Parent;
public:
   DECLARE_CONOBJECT(GuiPlatformGenericMenuBar);

   virtual void menuItemSelected(Menu *menu, MenuItem *item)
   {
      AssertFatal(menu && item, "");

      PopupMenu *popupMenu = PlatformPopupMenuData::mMenuMap[ menu ];
      AssertFatal(popupMenu, "");

      popupMenu->handleSelect( item->id );

      Parent::menuItemSelected(menu, item);
   }

protected:
   /// menu id / item id
   Map<CompoundKey<U32, U32>, String> mCmds;

};

IMPLEMENT_CONOBJECT(GuiPlatformGenericMenuBar);

//-----------------------------------------------------------------------------
// MenuBar Methods
//-----------------------------------------------------------------------------

void MenuBar::createPlatformPopupMenuData()
{
   mData = NULL;
}

void MenuBar::deletePlatformPopupMenuData()
{
//    SAFE_DELETE(mData);
}

//-----------------------------------------------------------------------------

GuiPlatformGenericMenuBar* _FindMenuBarCtrl()
{
   GuiControl* control;
   Sim::findObject("PlatformGenericMenubar", control);
   AssertFatal(control, "");
   if( !control )      
      return NULL;   

   GuiPlatformGenericMenuBar* menuBar;
   menuBar = dynamic_cast<GuiPlatformGenericMenuBar*>( control->findObjectByInternalName(  StringTable->insert("menubar"), true) );
   AssertFatal(menuBar, "");
   return menuBar;
}


void MenuBar::updateMenuBar(PopupMenu *popupMenu /* = NULL */)
{
   //if(! isAttachedToCanvas())
   //   return;   

   if(!popupMenu)
      return;

   GuiPlatformGenericMenuBar* menuBarGui = _FindMenuBarCtrl();
   popupMenu->mData->mMenuBar = this;

   String menuTitle = popupMenu->getBarTitle();

   //Next, find out if we're still in the list of entries
   SimSet::iterator itr = find(begin(), end(), popupMenu);

   GuiMenuBar::Menu* menuGui = menuBarGui->findMenu(menuTitle);
   if (!menuGui)
   {
      //This is our first time setting this particular menu up, so we'll OK it.
      if (itr == end())
         menuBarGui->attachToMenuBar(popupMenu->mData->mMenuGui);
      else
         menuBarGui->attachToMenuBar(popupMenu->mData->mMenuGui, itr - begin());
   }
   else
   {
      //Not our first time through, so we're really updating it.

      //So, first, remove it from the menubar
      menuBarGui->removeFromMenuBar(menuGui);

      //Next, find out if we're still in the list of entries
      SimSet::iterator itr = find(begin(), end(), popupMenu);

      //if we're no longer in the list, we're pretty much done here
      if (itr == end())
         return;

      //We're still here, so this is a valid menu for our current bar configuration, so add us back in.
      menuBarGui->attachToMenuBar(menuGui, itr - begin());
   }
}

//-----------------------------------------------------------------------------

void MenuBar::attachToCanvas(GuiCanvas *owner, S32 pos)
{
   if(owner == NULL || isAttachedToCanvas())
      return;

   // This is set for popup menus in the onAttachToMenuBar() callback
   mCanvas = owner;

   PlatformWindowSDL *pWindow = dynamic_cast<PlatformWindowSDL*>(owner->getPlatformWindow());
   if(pWindow == NULL) 
      return;

   // Setup the native menu bar
   GuiMenuBar *hWindowMenu = static_cast<GuiMenuBar*>( pWindow->getMenuHandle() );
	if( hWindowMenu == NULL && !Journal::IsPlaying() )
      hWindowMenu = _FindMenuBarCtrl();

   if(hWindowMenu)
   {
      pWindow->setMenuHandle( hWindowMenu );
      GuiControl *base = hWindowMenu->getParent();
         
      while( base->getParent() )
      {
         base = base->getParent();
      }         

      mCanvas->setMenuBar( base );
   }

   for (S32 i = 0; i < size(); ++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if (mnu == NULL)
      {
         Con::warnf("MenuBar::attachToMenuBar - Non-PopupMenu object in set");
         continue;
      }

      if (mnu->isAttachedToMenuBar())
         mnu->removeFromMenuBar();

      mnu->attachToMenuBar(owner, pos + i);
   }
   
}

void MenuBar::removeFromCanvas()
{
   if (mCanvas == NULL || !isAttachedToCanvas())
      return;

   //_FindMenuBarCtrl()->clearMenus();

   // Add the items
   for (S32 i = 0; i < size(); ++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if (mnu == NULL)
      {
         Con::warnf("MenuBar::removeFromMenuBar - Non-PopupMenu object in set");
         continue;
      }

      mnu->removeFromMenuBar();
   }

   mCanvas->setMenuBar(NULL);

   mCanvas = NULL;
}

#endif
