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

#include "platformWin32/platformWin32.h"
#include "platform/menus/menuBar.h"
#include "platform/menus/popupMenu.h"
#include "gui/core/guiCanvas.h"
#include "windowManager/platformWindowMgr.h"
#include "windowManager/win32/win32Window.h"
#include "core/util/safeDelete.h"

//-----------------------------------------------------------------------------
// Platform Data
//-----------------------------------------------------------------------------

// class PlatformMenuBarData
// {
// 
// };

//-----------------------------------------------------------------------------
// MenuBar Methods
//-----------------------------------------------------------------------------

void MenuBar::createPlatformPopupMenuData()
{
//    mData = new PlatformMenuBarData;

   // [tom, 6/4/2007] Nothing currently needed for win32
   mData = NULL;
}

void MenuBar::deletePlatformPopupMenuData()
{
//    SAFE_DELETE(mData);
}

//-----------------------------------------------------------------------------

void MenuBar::updateMenuBar(PopupMenu *menu /* = NULL */)
{
   if(! isAttachedToCanvas())
      return;

   if(menu == NULL)
   {
      // [tom, 6/4/2007] Kludgetastic
      GuiCanvas *oldCanvas = mCanvas;
      S32 pos = -1;
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(0));
      if(mnu)
         pos = mnu->getPosOnMenuBar();

      removeFromCanvas();
      attachToCanvas(oldCanvas, pos);

      return;
   }

   menu->removeFromMenuBar();
   SimSet::iterator itr = find(begin(), end(), menu);
   if(itr == end())
      return;

   menu->attachToMenuBar(mCanvas, itr - begin());

   Win32Window *pWindow = dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow());
   if(pWindow == NULL) 
      return;

   HWND hWindow = pWindow->getHWND();
   DrawMenuBar(hWindow);
}

//-----------------------------------------------------------------------------

void MenuBar::attachToCanvas(GuiCanvas *owner, S32 pos)
{
   if(owner == NULL || isAttachedToCanvas())
      return;

   // This is set for popup menus in the onAttachToMenuBar() callback
   mCanvas = owner;

   Win32Window *pWindow = dynamic_cast<Win32Window*>(owner->getPlatformWindow());
   if(pWindow == NULL) 
      return;

   // Setup the native menu bar
   HMENU hWindowMenu = pWindow->getMenuHandle();
	if(hWindowMenu == NULL && !Journal::IsPlaying())
   {
      hWindowMenu = CreateMenu();
      if(hWindowMenu)
      {
         pWindow->setMenuHandle( hWindowMenu);
      }
   }

   // Add the items
   for(S32 i = 0;i < size();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if(mnu == NULL)
      {
         Con::warnf("MenuBar::attachToMenuBar - Non-PopupMenu object in set");
         continue;
      }

      if(mnu->isAttachedToMenuBar())
         mnu->removeFromMenuBar();

      mnu->attachToMenuBar(owner, pos + i);
   }

   HWND hWindow = pWindow->getHWND();
   DrawMenuBar(hWindow);

}

void MenuBar::removeFromCanvas()
{
   if(mCanvas == NULL || ! isAttachedToCanvas())
      return;

   Win32Window *pWindow = dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow());
   if(pWindow == NULL) 
      return;

   // Setup the native menu bar
   HMENU hWindowMenu = pWindow->getMenuHandle();
   if(hWindowMenu == NULL)
      return;

   // Add the items
   for(S32 i = 0;i < size();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if(mnu == NULL)
      {
         Con::warnf("MenuBar::removeFromMenuBar - Non-PopupMenu object in set");
         continue;
      }

      mnu->removeFromMenuBar();
   }

   HWND hWindow = pWindow->getHWND();
   DrawMenuBar(hWindow);

   mCanvas = NULL;
}
