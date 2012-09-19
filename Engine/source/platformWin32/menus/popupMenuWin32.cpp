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

#include "platform/menus/popupMenu.h"
#include "platformWin32/platformWin32.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "windowManager/platformWindowMgr.h"
#include "windowManager/win32/win32Window.h"
#include "core/util/safeDelete.h"

#include "sim/actionMap.h"
#include "platform/platformInput.h"

//////////////////////////////////////////////////////////////////////////
// Platform Menu Data
//////////////////////////////////////////////////////////////////////////

struct PlatformPopupMenuData
{
   static U32 mLastPopupMenuID;
   static const U32 PopupMenuIDRange;

   HMENU mMenu;
   U32 mMenuID;
   U32 mLastID;

   Win32Window::AcceleratorList mAccelerators;
   Win32Window::AcceleratorList mDisabledAccelerators;

   PlatformPopupMenuData()
   {
      mMenu = NULL;
      mMenuID = mLastPopupMenuID++;
      mLastID = 0;
   }

   ~PlatformPopupMenuData()
   {
      if(mMenu)
         DestroyMenu(mMenu);
   }

   void insertAccelerator(EventDescriptor &desc, U32 id);
   void removeAccelerator(U32 id);
   void setAccelleratorEnabled(U32 id, bool enabled);
};

U32 PlatformPopupMenuData::mLastPopupMenuID = 0;
const U32 PlatformPopupMenuData::PopupMenuIDRange = 100;

//////////////////////////////////////////////////////////////////////////

void PlatformPopupMenuData::insertAccelerator(EventDescriptor &desc, U32 id)
{
   if(desc.eventType != SI_KEY)
      return;

   Win32Window::AcceleratorList::iterator i;
   for(i = mAccelerators.begin();i != mAccelerators.end();++i)
   {
      if(i->mID == id)
      {
         // Update existing entry
         i->mDescriptor.eventType = desc.eventType;
         i->mDescriptor.eventCode = desc.eventCode;
         i->mDescriptor.flags = desc.flags;
         return;
      }

      if(i->mDescriptor.eventType == desc.eventType && i->mDescriptor.eventCode == desc.eventCode && i->mDescriptor.flags == desc.flags)
      {
         // Already have a matching accelerator, don't add another one
         return;
      }
   }

   Win32Window::Accelerator accel;
   accel.mDescriptor = desc;
   accel.mID = id;
   mAccelerators.push_back(accel);
}

void PlatformPopupMenuData::removeAccelerator(U32 id)
{
   Win32Window::AcceleratorList::iterator i;
   for(i = mAccelerators.begin();i != mAccelerators.end();++i)
   {
      if(i->mID == id)
      {
         mAccelerators.erase(i);
         return;
      }
   }
}

void PlatformPopupMenuData::setAccelleratorEnabled( U32 id, bool enabled )
{   
   Win32Window::AcceleratorList *src = NULL;
   Win32Window::AcceleratorList *dst = NULL;
   
   if ( enabled )
   {
      src = &mDisabledAccelerators;
      dst = &mAccelerators;
   }
   else
   {
      src = &mAccelerators;
      dst = &mDisabledAccelerators;
   }

   Win32Window::AcceleratorList::iterator i;
   for ( i = src->begin(); i != src->end(); ++i )
   {
      if ( i->mID == id )
      {
         Win32Window::Accelerator tmp = *i;
         src->erase( i );
         dst->push_back( tmp );
         return;
      }
   }
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::createPlatformPopupMenuData()
{
   mData = new PlatformPopupMenuData;
}

void PopupMenu::deletePlatformPopupMenuData()
{
   SAFE_DELETE(mData);
}
void PopupMenu::createPlatformMenu()
{
   mData->mMenu = mIsPopup ? CreatePopupMenu() : CreateMenu();
   AssertFatal(mData->mMenu, "Unable to create menu");

   MENUINFO mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIM_MENUDATA;
   mi.dwMenuData = (ULONG_PTR)this;
   SetMenuInfo(mData->mMenu, &mi);
}

//////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////

S32 PopupMenu::insertItem(S32 pos, const char *title, const char* accelerator)
{
   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   bool isAttached = isAttachedToMenuBar();
   if(isAttached && pWindow == NULL)
      return -1;

   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_ID|MIIM_TYPE;
   mi.wID = (mData->mMenuID * PlatformPopupMenuData::PopupMenuIDRange) + mData->mLastID + 1;
   mData->mLastID++;
   if(title && *title)
      mi.fType = MFT_STRING;
   else
      mi.fType = MFT_SEPARATOR;
   
   char buf[1024];
   if(accelerator && *accelerator)
   {
      dSprintf(buf, sizeof(buf), "%s\t%s", title, accelerator);

      if(isAttached)
         pWindow->removeAccelerators(mData->mAccelerators);

      // Build entry for accelerator table
      EventDescriptor accelDesc;
      if(ActionMap::createEventDescriptor(accelerator, &accelDesc))
         mData->insertAccelerator(accelDesc, mi.wID);
      else
         Con::errorf("PopupMenu::insertItem - Could not create event descriptor for accelerator \"%s\"", accelerator);

      if(isAttached)
         pWindow->addAccelerators(mData->mAccelerators);
   }
   else
      dSprintf(buf, sizeof(buf), "%s", title);

   mi.dwTypeData = (LPSTR)buf;

   if(InsertMenuItemA(mData->mMenu, pos, TRUE, &mi))
   {
      if(isAttached)
      {
         HWND hWindow = pWindow->getHWND();
         DrawMenuBar(hWindow);
      }
      return mi.wID;
   }

   return -1;
}

S32 PopupMenu::insertSubMenu(S32 pos, const char *title, PopupMenu *submenu)
{
   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   bool isAttached = isAttachedToMenuBar();
   if(isAttached && pWindow == NULL)
      return -1;

   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      if(submenu == (*mSubmenus)[i])
      {
         Con::errorf("PopupMenu::insertSubMenu - Attempting to add submenu twice");
         return -1;
      }
   }

   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_ID|MIIM_TYPE|MIIM_SUBMENU|MIIM_DATA;
   mi.wID = (mData->mMenuID * PlatformPopupMenuData::PopupMenuIDRange) + mData->mLastID + 1;
   if(title && *title)
      mi.fType = MFT_STRING;
   else
      mi.fType = MFT_SEPARATOR;
   mi.dwTypeData = (LPSTR)title;
   mi.hSubMenu = submenu->mData->mMenu;
   mi.dwItemData = (ULONG_PTR)submenu;
   if(InsertMenuItemA(mData->mMenu, pos, TRUE, &mi))
   {
      mSubmenus->addObject(submenu);

      if(isAttached)
      {
         pWindow->addAccelerators(submenu->mData->mAccelerators);

         HWND hWindow = pWindow->getHWND();
         DrawMenuBar(hWindow);
      }
      return mi.wID;
   }

   return -1;
}

bool PopupMenu::setItem(S32 pos, const char *title, const char* accelerator)
{
   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   bool isAttached = isAttachedToMenuBar();
   if(isAttached && pWindow == NULL)
      return false;

   // Are we out of range?
   if ( pos >= getItemCount() )
      return false;

   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_TYPE;

   if(title && *title)
      mi.fType = MFT_STRING;
   else
      mi.fType = MFT_SEPARATOR;

   char buf[1024];
   if(accelerator && *accelerator)
   {
      dSprintf(buf, sizeof(buf), "%s\t%s", title, accelerator);

      if(isAttached)
         pWindow->removeAccelerators(mData->mAccelerators);

      // Build entry for accelerator table
      EventDescriptor accelDesc;
      if(ActionMap::createEventDescriptor(accelerator, &accelDesc))
         mData->insertAccelerator(accelDesc, pos);
      else
         Con::errorf("PopupMenu::setItem - Could not create event descriptor for accelerator \"%s\"", accelerator);

      if(isAttached)
         pWindow->addAccelerators(mData->mAccelerators);
   }
   else
      dSprintf(buf, sizeof(buf), "%s", title);

   mi.dwTypeData = (LPSTR)buf;

   if(SetMenuItemInfoA(mData->mMenu, pos, TRUE, &mi))
   {
      if(isAttached)
      {
         HWND hWindow = pWindow->getHWND();
         DrawMenuBar(hWindow);
      }

      return true;
   }

   return false;
}

void PopupMenu::removeItem(S32 itemPos)
{
   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   bool isAttached = isAttachedToMenuBar();
   if(isAttached && pWindow == NULL)
      return;
   
   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_DATA|MIIM_ID;
   if(GetMenuItemInfoA(mData->mMenu, itemPos, TRUE, &mi))
   {
      bool submenu = false;

      // Update list of submenus if this is a submenu
      if(mi.fMask & MIIM_DATA)
      {
         PopupMenu *mnu = (PopupMenu *)mi.dwItemData;
         if( mnu != NULL )
         {
            if(isAttached)
               pWindow->removeAccelerators(mnu->mData->mAccelerators);
            mSubmenus->removeObject(mnu);

            submenu = true;
         }
      }

      if(! submenu)
      {
         // Update accelerators if this has an accelerator and wasn't a sub menu
         for(S32 i = 0;i < mData->mAccelerators.size();++i)
         {
            if(mData->mAccelerators[i].mID == mi.wID)
            {
               if(isAttached)
                  pWindow->removeAccelerators(mData->mAccelerators);

               mData->mAccelerators.erase(i);

               if(isAttached)
                  pWindow->addAccelerators(mData->mAccelerators);

               break;
            }
         }
      }
   }
   else
      return;

   RemoveMenu(mData->mMenu, itemPos, MF_BYPOSITION);

   if(isAttached)
   {
      HWND hWindow = pWindow->getHWND();
      DrawMenuBar(hWindow);
   }
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::enableItem( S32 pos, bool enable )
{
   U32 flags = enable ? MF_ENABLED : MF_GRAYED;
   EnableMenuItem( mData->mMenu, pos, MF_BYPOSITION|flags );

   // Update accelerators.

   // NOTE: This really DOES need to happen. A disabled menu item
   // should not still have an accelerator mapped to it. 
   //
   // Unfortunately, the editors currently only set menu items 
   // enabled/disabled when the menu itself is selected which means our 
   // accelerators would be out of synch.

   /*
   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>( mCanvas->getPlatformWindow() ) : NULL;
   bool isAttached = isAttachedToMenuBar();
   if ( isAttached && pWindow == NULL )
      return;

   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_DATA|MIIM_ID;
   if ( !GetMenuItemInfoA( mData->mMenu, pos, TRUE, &mi) )
      return;

   if ( isAttached )
      pWindow->removeAccelerators( mData->mAccelerators );

   mData->setAccelleratorEnabled( mi.wID, enable );

   if ( isAttached )
      pWindow->addAccelerators( mData->mAccelerators );
   */
}

void PopupMenu::checkItem(S32 pos, bool checked)
{
//    U32 flags = checked ? MF_CHECKED : MF_UNCHECKED;
//    CheckMenuItem(mData->mMenu, pos, MF_BYPOSITION|flags);

   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_STATE;
   mi.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
   SetMenuItemInfoA(mData->mMenu, pos, TRUE, &mi);
}

void PopupMenu::checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos)
{
   CheckMenuRadioItem(mData->mMenu, firstPos, lastPos, checkPos, MF_BYPOSITION);
}

bool PopupMenu::isItemChecked(S32 pos)
{
   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.fMask = MIIM_STATE;
   if(GetMenuItemInfoA(mData->mMenu, pos, TRUE, &mi) && (mi.fState & MFS_CHECKED))
      return true;
   return false;
}

U32 PopupMenu::getItemCount()
{
   return GetMenuItemCount( mData->mMenu );
}

//////////////////////////////////////////////////////////////////////////

bool PopupMenu::canHandleID(U32 id)
{
   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      PopupMenu *subM = dynamic_cast<PopupMenu *>((*mSubmenus)[i]);
      if(subM == NULL)
         continue;

      if(subM->canHandleID(id))
         return true;
   }

   if(id >= mData->mMenuID * PlatformPopupMenuData::PopupMenuIDRange &&
      id < (mData->mMenuID+1) * PlatformPopupMenuData::PopupMenuIDRange)
   {
      return true;
   }

   return false;
}

bool PopupMenu::handleSelect(U32 command, const char *text /* = NULL */)
{
   // [tom, 8/20/2006] Pass off to a sub menu if it's for them
   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      PopupMenu *subM = dynamic_cast<PopupMenu *>((*mSubmenus)[i]);
      if(subM == NULL)
         continue;

      if(subM->canHandleID(command))
      {
         return subM->handleSelect(command, text);
      }
   }

   // [tom, 8/21/2006] Cheesey hack to find the position based on ID
   char buf[512];
   MENUITEMINFOA mi;
   mi.cbSize = sizeof(mi);
   mi.dwTypeData = NULL;

   S32 numItems = GetMenuItemCount(mData->mMenu);
   S32 pos = -1;
   for(S32 i = 0;i < numItems;i++)
   {
      mi.fMask = MIIM_ID|MIIM_STRING|MIIM_STATE;
      if(GetMenuItemInfoA(mData->mMenu, i, TRUE, &mi))
      {
         if(mi.wID == command)
         {
            if(text == NULL)
            {
               mi.dwTypeData = buf;
               mi.cch++;
               GetMenuItemInfoA(mData->mMenu, i, TRUE, &mi);
               
               // [tom, 5/11/2007] Don't do anything if the menu item is disabled
               if(mi.fState & MFS_DISABLED)
                  return false;

               text = StringTable->insert(mi.dwTypeData);
            }
            pos = i;
            break;
         }
      }
   }

   if(pos == -1)
   {
      Con::errorf("PopupMenu::handleSelect - Could not find menu item position for ID %d ... this shouldn't happen!", command);
      return false;
   }

   // [tom, 8/20/2006] Wasn't handled by a submenu, pass off to script
   return dAtob(Con::executef(this, "onSelectItem", Con::getIntArg(pos), text ? text : ""));
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::showPopup(GuiCanvas *owner, S32 x /* = -1 */, S32 y /* = -1 */)
{
   if( owner == NULL )
   {
      Con::warnf("PopupMenu::showPopup - Invalid canvas supplied!");
      return;
   }

   // [tom, 6/4/2007] showPopup() blocks until the menu is closed by the user, 
   // so the canvas pointer is not needed beyond the scope of this function
   // when working with context menus. Setting mCanvas here will cause undesired
   // behavior in relation to the menu bar.

   Win32Window *pWindow = dynamic_cast<Win32Window*>(owner->getPlatformWindow());
   if(pWindow == NULL)
      return;
   HWND hWindow = pWindow->getHWND();
   POINT p;
   if(x == -1 && y == -1)
      GetCursorPos(&p);
   else
   {
      p.x = x;
      p.y = y;
      ClientToScreen(hWindow, &p);
   }

   winState.renderThreadBlocked = true;
   U32 opt = (int)TrackPopupMenu(mData->mMenu, TPM_NONOTIFY|TPM_RETURNCMD, p.x, p.y, 0, hWindow, NULL);
   if(opt > 0)
      handleSelect(opt, NULL);
   winState.renderThreadBlocked = false;
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::attachToMenuBar(GuiCanvas *owner, S32 pos, const char *title)
{
   if(owner == NULL || isAttachedToMenuBar())
      return;

   // This is set for sub-menus in the onAttachToMenuBar() callback
   mCanvas = owner;

   Win32Window *pWindow = dynamic_cast<Win32Window*>(owner->getPlatformWindow());
   if(pWindow == NULL) 
      return;

   HMENU hWindowMenu = pWindow->getMenuHandle();
   if(hWindowMenu == NULL)
   {
      hWindowMenu = CreateMenu();
      if(hWindowMenu)
      {
         pWindow->setMenuHandle( hWindowMenu);
      }
   }

   MENUITEMINFOA mii;

   mii.cbSize = sizeof(MENUITEMINFOA);

   mii.fMask = MIIM_STRING|MIIM_DATA;
   mii.dwTypeData = (LPSTR)title;
   mii.fMask |= MIIM_ID;
   mii.wID = mData->mMenuID;
   mii.fMask |= MIIM_SUBMENU;
   mii.hSubMenu = mData->mMenu;
   mii.dwItemData = (ULONG_PTR)this;

   InsertMenuItemA(hWindowMenu, pos, TRUE, &mii);

   HWND hWindow = pWindow->getHWND();
   DrawMenuBar(hWindow);

   pWindow->addAccelerators(mData->mAccelerators);

   // Add accelerators for sub menus
   for(SimSet::iterator i = mSubmenus->begin();i != mSubmenus->end();++i)
   {
      PopupMenu *submenu = dynamic_cast<PopupMenu *>(*i);
      if(submenu == NULL)
         continue;

      pWindow->addAccelerators(submenu->mData->mAccelerators);
   }

   onAttachToMenuBar(owner, pos, title);
}

// New version of above for use by MenuBar class. Do not use yet.
void PopupMenu::attachToMenuBar(GuiCanvas *owner, S32 pos)
{
   Win32Window *pWindow = dynamic_cast<Win32Window*>(owner->getPlatformWindow());
   if(pWindow == NULL) 
      return;

	//When playing a journal, the system menu is not actually shown
	if (Journal::IsPlaying())
	{
		onAttachToMenuBar(owner, pos, mBarTitle);
		return;
	}

   HMENU hWindowMenu = pWindow->getMenuHandle();

   MENUITEMINFOA mii;

   mii.cbSize = sizeof(MENUITEMINFOA);

   mii.fMask = MIIM_STRING|MIIM_DATA;
   mii.dwTypeData = (LPSTR)mBarTitle;
   mii.fMask |= MIIM_ID;
   mii.wID = mData->mMenuID;
   mii.fMask |= MIIM_SUBMENU;
   mii.hSubMenu = mData->mMenu;
   mii.dwItemData = (ULONG_PTR)this;

   InsertMenuItemA(hWindowMenu, pos, TRUE, &mii);

   pWindow->addAccelerators(mData->mAccelerators);

   // Add accelerators for sub menus (have to do this here as it's platform specific)
   for(SimSet::iterator i = mSubmenus->begin();i != mSubmenus->end();++i)
   {
      PopupMenu *submenu = dynamic_cast<PopupMenu *>(*i);
      if(submenu == NULL)
         continue;

      pWindow->addAccelerators(submenu->mData->mAccelerators);
   }

   onAttachToMenuBar(owner, pos, mBarTitle);
}

void PopupMenu::removeFromMenuBar()
{
   S32 pos = getPosOnMenuBar();
   if(pos == -1)
      return;

   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   if(pWindow == NULL)
      return;

   HMENU hMenuHandle = pWindow->getMenuHandle();
   if(!hMenuHandle)
      return;

   RemoveMenu(hMenuHandle, pos, MF_BYPOSITION);

   HWND hWindow = pWindow->getHWND();

   DrawMenuBar(hWindow);

   pWindow->removeAccelerators(mData->mAccelerators);

   // Remove accelerators for sub menus
   for(SimSet::iterator i = mSubmenus->begin();i != mSubmenus->end();++i)
   {
      PopupMenu *submenu = dynamic_cast<PopupMenu *>(*i);
      if(submenu == NULL)
         continue;

      pWindow->removeAccelerators(submenu->mData->mAccelerators);
   }

   onRemoveFromMenuBar(mCanvas);
}

S32 PopupMenu::getPosOnMenuBar()
{
   if(mCanvas == NULL)
      return -1;

   Win32Window *pWindow = mCanvas ? dynamic_cast<Win32Window*>(mCanvas->getPlatformWindow()) : NULL;
   if(pWindow == NULL) 
      return -1;

   HMENU hMenuHandle = pWindow->getMenuHandle();
   S32 numItems = GetMenuItemCount(hMenuHandle);
   S32 pos = -1;
   for(S32 i = 0;i < numItems;i++)
   {
      MENUITEMINFOA mi;
      mi.cbSize = sizeof(mi);
      mi.fMask = MIIM_DATA;
      if(GetMenuItemInfoA(hMenuHandle, i, TRUE, &mi))
      {
         if(mi.fMask & MIIM_DATA)
         {
            PopupMenu *mnu = (PopupMenu *)mi.dwItemData;
            if(mnu == this)
            {
               pos = i;
               break;
            }
         }
      }
   }

   return pos;
}

