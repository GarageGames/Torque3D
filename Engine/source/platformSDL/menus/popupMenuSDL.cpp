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
#include "platform/menus/menuBar.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "core/util/safeDelete.h"

#include "sim/actionMap.h"
#include "platform/platformInput.h"

#include "windowManager/sdl/sdlWindow.h"
#include "gui/editor/guiMenuBar.h"

#include "platformSDL/menus/PlatformSDLPopupMenuData.h"
#include "console/engineAPI.h"

#include "platformSDL/menus/guiPlatformGenericMenuBar.h"
#include "gui/editor/guiPopupMenuCtrl.h"

//////////////////////////////////////////////////////////////////////////
// Platform Menu Data
//////////////////////////////////////////////////////////////////////////
GuiPlatformGenericMenuBar* findMenuBarCtrl()
{
   GuiControl* control;
   Sim::findObject("PlatformGenericMenubar", control);
   AssertFatal(control, "");
   if (!control)
      return NULL;

   GuiPlatformGenericMenuBar* menuBar;
   menuBar = dynamic_cast<GuiPlatformGenericMenuBar*>(control->findObjectByInternalName(StringTable->insert("menubar"), true));
   AssertFatal(menuBar, "");
   return menuBar;
}

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
   
   //We'll make a special exception for the spacer items
   if(item && dStrcmp(title, ""))
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
    if(owner == NULL)
      return;

    GuiControl* editorGui;
    Sim::findObject("EditorGui", editorGui);

    if (editorGui)
    {
       GuiPopupMenuTextListCtrl* textList;
       GuiPopupMenuBackgroundCtrl* backgroundCtrl;
       Sim::findObject("PopUpMenuControl", backgroundCtrl);

       GuiControlProfile* profile;
       Sim::findObject("GuiMenubarProfile", profile);

       if (!profile)
          return;

       if (!backgroundCtrl)
       {
          textList = new GuiPopupMenuTextListCtrl();

          textList->registerObject();

          backgroundCtrl = new GuiPopupMenuBackgroundCtrl(textList);

          backgroundCtrl->registerObject("PopUpMenuControl");

          textList->setControlProfile(profile);

          backgroundCtrl->addObject(textList);
       }
       else
       {
          textList = dynamic_cast<GuiPopupMenuTextListCtrl*>(backgroundCtrl->first());
       }

       if (!backgroundCtrl || !textList)
          return;

       owner->pushDialogControl(backgroundCtrl, 10);

       backgroundCtrl->setExtent(editorGui->getExtent());

       textList->clear();
       textList->mMenu = mData->mMenuGui;
       textList->mMenuBar = findMenuBarCtrl();
       textList->mPopup = this;

       S32 textWidth = 0, width = 0;
       S32 acceleratorWidth = 0;
       GFont *font = profile->mFont;

       Point2I maxBitmapSize = Point2I(0, 0);

       S32 numBitmaps = profile->mBitmapArrayRects.size();
       if (numBitmaps)
       {
          RectI *bitmapBounds = profile->mBitmapArrayRects.address();
          for (S32 i = 0; i < numBitmaps; i++)
          {
             if (bitmapBounds[i].extent.x > maxBitmapSize.x)
                maxBitmapSize.x = bitmapBounds[i].extent.x;
             if (bitmapBounds[i].extent.y > maxBitmapSize.y)
                maxBitmapSize.y = bitmapBounds[i].extent.y;
          }
       }

       for (GuiMenuBar::MenuItem *walk = mData->mMenuGui->firstMenuItem; walk; walk = walk->nextMenuItem)
       {
          if (!walk->visible)
             continue;

          S32 iTextWidth = font->getStrWidth(walk->text);
          S32 iAcceleratorWidth = walk->accelerator ? font->getStrWidth(walk->accelerator) : 0;

          if (iTextWidth > textWidth)
             textWidth = iTextWidth;
          if (iAcceleratorWidth > acceleratorWidth)
             acceleratorWidth = iAcceleratorWidth;
       }
       width = textWidth + acceleratorWidth + maxBitmapSize.x * 2 + 2 + 4;

       textList->setCellSize(Point2I(width, font->getHeight() + 2));
       textList->clearColumnOffsets();
       textList->addColumnOffset(-1); // add an empty column in for the bitmap index.
       textList->addColumnOffset(maxBitmapSize.x + 1);
       textList->addColumnOffset(maxBitmapSize.x + 1 + textWidth + 4);

       U32 entryCount = 0;

       for (GuiMenuBar::MenuItem *walk = mData->mMenuGui->firstMenuItem; walk; walk = walk->nextMenuItem)
       {
          if (!walk->visible)
             continue;

          char buf[512];

          //  If this menu item is a submenu, then set the isSubmenu to 2 to indicate
          // an arrow should be drawn.  Otherwise set the isSubmenu normally.
          char isSubmenu = 1;
          if (walk->isSubmenu)
             isSubmenu = 2;

          char bitmapIndex = 1;
          if (walk->bitmapIndex >= 0 && (walk->bitmapIndex * 3 <= profile->mBitmapArrayRects.size()))
             bitmapIndex = walk->bitmapIndex + 2;
          dSprintf(buf, sizeof(buf), "%c%c\t%s\t%s", bitmapIndex, isSubmenu, walk->text, walk->accelerator ? walk->accelerator : "");
          textList->addEntry(entryCount, buf);

          if (!walk->enabled)
             textList->setEntryActive(entryCount, false);

          entryCount++;
       }

       Point2I pos = owner->getCursorPos();
       textList->setPosition(pos);

       //nudge in if we'd overshoot the screen
       S32 widthDiff = (textList->getPosition().x + textList->getExtent().x) - backgroundCtrl->getWidth();
       if (widthDiff > 0)
       {
          Point2I popupPos = textList->getPosition();
          textList->setPosition(popupPos.x - widthDiff, popupPos.y);
       }
    }
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
