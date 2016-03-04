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

#ifdef TORQUE_SDL

#include "platform/menus/popupMenu.h"
#include "platform/menus//menuBar.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "core/util/safeDelete.h"

#include "sim/actionMap.h"
#include "platform/platformInput.h"

#include "windowManager/sdl/sdlWindow.h"
#include "gui/editor/guiMenuBar.h"

#include "platformSDL/menus/PlatformSDLPopupMenuData.h"
#include "console/engineAPI.h"

//////////////////////////////////////////////////////////////////////////
// Platform Menu Data
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

void PlatformPopupMenuData::insertAccelerator(EventDescriptor &desc, U32 id)
{
   AssertFatal(0, "");
}

void PlatformPopupMenuData::removeAccelerator(U32 id)
{
   AssertFatal(0, "");
}

void PlatformPopupMenuData::setAccelleratorEnabled( U32 id, bool enabled )
{   
  AssertFatal(0, "");
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
   mData->mMenuGui = GuiMenuBar::sCreateMenu( getBarTitle(), getId() );
   PlatformPopupMenuData::mMenuMap[ mData->mMenuGui ] = this;
}


//////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////

S32 PopupMenu::insertItem(S32 pos, const char *title, const char* accelerator, const char* cmd)
{   
   GuiMenuBar::MenuItem *item = GuiMenuBar::findMenuItem( mData->mMenuGui, title );
   if(item)
   {
      setItem( pos, title, accelerator, cmd);
      return pos;
   }

   item = GuiMenuBar::addMenuItem( mData->mMenuGui, title, pos, accelerator, -1, cmd );
   item->submenuParentMenu = this->mData->mMenuGui;
   
   return pos;
}

S32 PopupMenu::insertSubMenu(S32 pos, const char *title, PopupMenu *submenu)
{  
   GuiMenuBar::MenuItem *item = GuiMenuBar::addMenuItem( mData->mMenuGui, title, pos, "", -1, "" );
   item->isSubmenu = true;
   item->submenu = submenu->mData->mMenuGui;
   item->submenuParentMenu = this->mData->mMenuGui;

   return pos;
}

bool PopupMenu::setItem(S32 pos, const char *title, const char* accelerator, const char* cmd)
{
   GuiMenuBar::MenuItem *item = NULL;

   item = GuiMenuBar::findMenuItem( mData->mMenuGui, title );
   
   if(item)
   {
      item->id = pos;
      item->cmd = cmd;
      if( accelerator && accelerator[0] )
         item->accelerator = dStrdup( accelerator );
      else
         item->accelerator = NULL;
      return true;
   }

   return false;
}

void PopupMenu::removeItem(S32 itemPos)
{
   GuiMenuBar::MenuItem *item = GuiMenuBar::findMenuItem( mData->mMenuGui, String::ToString(itemPos) );
   if(item)
   {
      GuiMenuBar::removeMenuItem( mData->mMenuGui, item);
   }
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::enableItem( S32 pos, bool enable )
{
   GuiMenuBar::MenuItem *item = NULL;
   for( item = mData->mMenuGui->firstMenuItem; item; item = item->nextMenuItem )
   {
      if( item->id == pos)
         item->enabled = enable;
   }
}

void PopupMenu::checkItem(S32 pos, bool checked)
{
   GuiMenuBar::MenuItem *item = NULL;
   for( item = mData->mMenuGui->firstMenuItem; item; item = item->nextMenuItem )   
      if(item->id == pos)
         break;   

   if( !item )
      return;

   if(checked && item->checkGroup != -1)
   {
      // first, uncheck everything in the group:
      for( GuiMenuBar::MenuItem *itemWalk = mData->mMenuGui->firstMenuItem; itemWalk; itemWalk = itemWalk->nextMenuItem )
         if( itemWalk->checkGroup == item->checkGroup && itemWalk->bitmapIndex == mData->mCheckedBitmapIdx )
            itemWalk->bitmapIndex = -1;
   }

   item->bitmapIndex = checked ? mData->mCheckedBitmapIdx : -1;   
}

void PopupMenu::checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos)
{
   GuiMenuBar::MenuItem *item = NULL;
   for( item = mData->mMenuGui->firstMenuItem; item; item = item->nextMenuItem )
   {
      if(item->id >= firstPos && item->id <= lastPos)
      {
         item->bitmapIndex = (item->id  == checkPos) ? mData->mCheckedBitmapIdx : -1;  
      }
   }
}

bool PopupMenu::isItemChecked(S32 pos)
{
   GuiMenuBar::MenuItem *item = NULL;
   for( item = mData->mMenuGui->firstMenuItem; item; item = item->nextMenuItem )   
      if(item->id == pos)      
         return item->bitmapIndex == mData->mCheckedBitmapIdx;   

   return false;
}

U32 PopupMenu::getItemCount()
{
   int count = 0;
   for( GuiMenuBar::MenuItem *item = mData->mMenuGui->firstMenuItem; item; item = item->nextMenuItem )
      ++count;

   return count;
}

//////////////////////////////////////////////////////////////////////////

bool PopupMenu::canHandleID(U32 id)
{
   return true;
}

bool PopupMenu::handleSelect(U32 command, const char *text /* = NULL */)
{
   return dAtob(Con::executef(this, "onSelectItem", Con::getIntArg(command), text ? text : ""));
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::showPopup(GuiCanvas *owner, S32 x /* = -1 */, S32 y /* = -1 */)
{
    if(owner == NULL || isAttachedToMenuBar())
      return;
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::attachToMenuBar(GuiCanvas *owner, S32 pos, const char *title)
{
   if(owner == NULL || isAttachedToMenuBar())
      return;   
}

// New version of above for use by MenuBar class. Do not use yet.
void PopupMenu::attachToMenuBar(GuiCanvas *owner, S32 pos)
{
   if(owner == NULL || isAttachedToMenuBar())
      return;

   //mData->mMenuBar = owner->setMenuBar();
}

void PopupMenu::removeFromMenuBar()
{
   if(isAttachedToMenuBar())
      return;
}

S32 PopupMenu::getPosOnMenuBar()
{
   
   return 0;
}

#endif
