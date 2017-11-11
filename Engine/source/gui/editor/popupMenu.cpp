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
#include "gui/editor/popupMenu.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "core/util/safeDelete.h"
#include "gui/editor/guiPopupMenuCtrl.h"
#include "gui/editor/guiMenuBar.h"

static U32 sMaxPopupGUID = 0;
PopupMenuEvent PopupMenu::smPopupMenuEvent;
bool PopupMenu::smSelectionEventHandled = false;

/// Event class used to remove popup menus from the event notification in a safe way
class PopUpNotifyRemoveEvent : public SimEvent
{   
public:
   void process(SimObject *object)
   {
      PopupMenu::smPopupMenuEvent.remove((PopupMenu *)object, &PopupMenu::handleSelectEvent);
   }
};

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
PopupMenu::PopupMenu()
{
   bitmapIndex = -1;

   barTitle = StringTable->EmptyString();

   mMenuBarCtrl = nullptr;
   mTextList = nullptr;

   isSubmenu = false;
}

PopupMenu::~PopupMenu()
{
   PopupMenu::smPopupMenuEvent.remove(this, &PopupMenu::handleSelectEvent);
}

IMPLEMENT_CONOBJECT(PopupMenu);

ConsoleDocClass( PopupMenu,
   "@brief PopupMenu represents a system menu.\n\n"
   "You can add menu items to the menu, but there is no torque object associated "
   "with these menu items, they exist only in a  platform specific manner.\n\n"
   "@note Internal use only\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------
void PopupMenu::initPersistFields()
{
   Parent::initPersistFields();

   addField("barTitle", TypeCaseString, Offset(barTitle, PopupMenu), "");
}

//-----------------------------------------------------------------------------
bool PopupMenu::onAdd()
{
   if(! Parent::onAdd())
      return false;

   Con::executef(this, "onAdd");
   return true;
}

void PopupMenu::onRemove()
{
   Con::executef(this, "onRemove");

   Parent::onRemove();
}

//-----------------------------------------------------------------------------
void PopupMenu::onMenuSelect()
{
   Con::executef(this, "onMenuSelect");
}

//-----------------------------------------------------------------------------
void PopupMenu::handleSelectEvent(U32 popID, U32 command)
{  
}

//-----------------------------------------------------------------------------
bool PopupMenu::onMessageReceived(StringTableEntry queue, const char* event, const char* data)
{
   return Con::executef(this, "onMessageReceived", queue, event, data);
}

bool PopupMenu::onMessageObjectReceived(StringTableEntry queue, Message *msg )
{
   return Con::executef(this, "onMessageReceived", queue, Con::getIntArg(msg->getId()));
}

//////////////////////////////////////////////////////////////////////////
// Platform Menu Data
//////////////////////////////////////////////////////////////////////////
GuiMenuBar* PopupMenu::getMenuBarCtrl()
{
   return mMenuBarCtrl;
}

//////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////
S32 PopupMenu::insertItem(S32 pos, const char *title, const char* accelerator, const char* cmd)
{
   String titleString = title;

   MenuItem newItem;
   newItem.id = pos;
   newItem.text = titleString;
   newItem.cmd = cmd;

   if (titleString.isEmpty() || titleString == String("-"))
      newItem.isSpacer = true;
   else
      newItem.isSpacer = false;

   if (accelerator[0])
      newItem.accelerator = dStrdup(accelerator);
   else
      newItem.accelerator = NULL;

   newItem.visible = true;
   newItem.isChecked = false;
   newItem.acceleratorIndex = 0;
   newItem.enabled = !newItem.isSpacer;

   newItem.isSubmenu = false;
   newItem.subMenu = nullptr;
   newItem.subMenuParentMenu = nullptr;

   mMenuItems.push_back(newItem);

   return pos;
}

S32 PopupMenu::insertSubMenu(S32 pos, const char *title, PopupMenu *submenu)
{
   S32 itemPos = insertItem(pos, title, "", "");

   mMenuItems[itemPos].isSubmenu = true;
   mMenuItems[itemPos].subMenu = submenu;
   mMenuItems[itemPos].subMenuParentMenu = this;

   submenu->isSubmenu = true;

   return itemPos;
}

bool PopupMenu::setItem(S32 pos, const char *title, const char* accelerator, const char* cmd)
{
   String titleString = title;

   for (U32 i = 0; i < mMenuItems.size(); i++)
   {
      if (mMenuItems[i].text == titleString)
      {
         mMenuItems[i].id = pos;
         mMenuItems[i].cmd = cmd;
         
         if (accelerator && accelerator[0])
            mMenuItems[i].accelerator = dStrdup(accelerator);
         else
            mMenuItems[i].accelerator = NULL;
         return true;
      }
   }
   
   return false;
}

void PopupMenu::removeItem(S32 itemPos)
{
   if (mMenuItems.size() < itemPos || itemPos < 0)
      return;

   mMenuItems.erase(itemPos);
}

//////////////////////////////////////////////////////////////////////////
void PopupMenu::enableItem(S32 pos, bool enable)
{
   if (mMenuItems.size() < pos || pos < 0)
      return;

   mMenuItems[pos].enabled = enable;
}

void PopupMenu::checkItem(S32 pos, bool checked)
{
   if (mMenuItems.size() < pos || pos < 0)
      return;

   if (checked && mMenuItems[pos].checkGroup != -1)
   {
      // first, uncheck everything in the group:
      for (U32 i = 0; i < mMenuItems.size(); i++)
         if (mMenuItems[i].checkGroup == mMenuItems[pos].checkGroup && mMenuItems[i].isChecked)
            mMenuItems[i].isChecked = false;
   }

   mMenuItems[pos].isChecked;
}

void PopupMenu::checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos)
{
   for (U32 i = 0; i < mMenuItems.size(); i++)
   {
      if (mMenuItems[i].id >= firstPos && mMenuItems[i].id <= lastPos)
      {
         mMenuItems[i].isChecked = false;
      }
   }
}

bool PopupMenu::isItemChecked(S32 pos)
{
   if (mMenuItems.size() < pos || pos < 0)
      return false;

   return mMenuItems[pos].isChecked;
}

U32 PopupMenu::getItemCount()
{
   return mMenuItems.size();
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
   if (owner == NULL)
      return;

   GuiControl* editorGui;
   Sim::findObject("EditorGui", editorGui);

   if (editorGui)
   {
      GuiPopupMenuBackgroundCtrl* backgroundCtrl;
      Sim::findObject("PopUpMenuControl", backgroundCtrl);

      GuiControlProfile* profile;
      Sim::findObject("GuiMenubarProfile", profile);

      if (!profile)
         return;

      if (mTextList == nullptr)
      {
         mTextList = new GuiPopupMenuTextListCtrl();
         mTextList->registerObject();
         mTextList->setControlProfile(profile);

         mTextList->mPopup = this;
         mTextList->mMenuBar = getMenuBarCtrl();
      }

      if (!backgroundCtrl)
      {
         backgroundCtrl = new GuiPopupMenuBackgroundCtrl();

         backgroundCtrl->registerObject("PopUpMenuControl");
      }

      if (!backgroundCtrl || !mTextList)
         return;

      if (!isSubmenu)
      {
         //if we're a 'parent' menu, then tell the background to clear out all existing other popups

         backgroundCtrl->clearPopups();
      }

      //find out if we're doing a first-time add
      S32 popupIndex = backgroundCtrl->findPopupMenu(this);

      if (popupIndex == -1)
      {
         backgroundCtrl->addObject(mTextList);
         backgroundCtrl->mPopups.push_back(this);
      }

      mTextList->mBackground = backgroundCtrl;

      owner->pushDialogControl(backgroundCtrl, 10);

      //Set the background control's menubar, if any, and if it's not already set
      if(backgroundCtrl->mMenuBarCtrl == nullptr)
         backgroundCtrl->mMenuBarCtrl = getMenuBarCtrl();

      backgroundCtrl->setExtent(editorGui->getExtent());

      mTextList->clear();

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

      for (U32 i = 0; i < mMenuItems.size(); i++)
      {
         if (!mMenuItems[i].visible)
            continue;

         S32 iTextWidth = font->getStrWidth(mMenuItems[i].text.c_str());
         S32 iAcceleratorWidth = mMenuItems[i].accelerator ? font->getStrWidth(mMenuItems[i].accelerator) : 0;

         if (iTextWidth > textWidth)
            textWidth = iTextWidth;
         if (iAcceleratorWidth > acceleratorWidth)
            acceleratorWidth = iAcceleratorWidth;
      }

      width = textWidth + acceleratorWidth + maxBitmapSize.x * 2 + 2 + 4;

      mTextList->setCellSize(Point2I(width, font->getHeight() + 2));
      mTextList->clearColumnOffsets();
      mTextList->addColumnOffset(-1); // add an empty column in for the bitmap index.
      mTextList->addColumnOffset(maxBitmapSize.x + 1);
      mTextList->addColumnOffset(maxBitmapSize.x + 1 + textWidth + 4);

      U32 entryCount = 0;

      for (U32 i = 0; i < mMenuItems.size(); i++)
      {
         if (!mMenuItems[i].visible)
            continue;

         char buf[512];

         //  If this menu item is a submenu, then set the isSubmenu to 2 to indicate
         // an arrow should be drawn.  Otherwise set the isSubmenu normally.
         char isSubmenu = 1;
         if (mMenuItems[i].isSubmenu)
            isSubmenu = 2;

         char bitmapIndex = 1;
         if (mMenuItems[i].bitmapIndex >= 0 && (mMenuItems[i].bitmapIndex * 3 <= profile->mBitmapArrayRects.size()))
            bitmapIndex = mMenuItems[i].bitmapIndex + 2;

         dSprintf(buf, sizeof(buf), "%c%c\t%s\t%s", bitmapIndex, isSubmenu, mMenuItems[i].text.c_str(), mMenuItems[i].accelerator ? mMenuItems[i].accelerator : "");
         mTextList->addEntry(entryCount, buf);

         if (!mMenuItems[i].enabled)
            mTextList->setEntryActive(entryCount, false);

         entryCount++;
      }

      Point2I pos = Point2I::Zero;

      if (x == -1 && y == -1)
         pos = owner->getCursorPos();
      else
         pos = Point2I(x, y);

      mTextList->setPosition(pos);

      //nudge in if we'd overshoot the screen
      S32 widthDiff = (mTextList->getPosition().x + mTextList->getExtent().x) - backgroundCtrl->getWidth();
      if (widthDiff > 0)
      {
         Point2I popupPos = mTextList->getPosition();
         mTextList->setPosition(popupPos.x - widthDiff, popupPos.y);
      }

      mTextList->setHidden(false);
   }
}

void PopupMenu::hidePopup()
{
   if (mTextList)
   {
      mTextList->setHidden(true);
   }

   hidePopupSubmenus();
}

void PopupMenu::hidePopupSubmenus()
{
   for (U32 i = 0; i < mMenuItems.size(); i++)
   {
      if (mMenuItems[i].subMenu != nullptr)
         mMenuItems[i].subMenu->hidePopup();
   }
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------
DefineConsoleMethod(PopupMenu, insertItem, S32, (S32 pos, const char * title, const char * accelerator, const char* cmd), ("", "", ""), "(pos[, title][, accelerator][, cmd])")
{
   return object->insertItem(pos, title, accelerator, cmd);
}

DefineConsoleMethod(PopupMenu, removeItem, void, (S32 pos), , "(pos)")
{
   object->removeItem(pos);
}

DefineConsoleMethod(PopupMenu, insertSubMenu, S32, (S32 pos, String title, String subMenu), , "(pos, title, subMenu)")
{
   PopupMenu *mnu = dynamic_cast<PopupMenu *>(Sim::findObject(subMenu));
   if(mnu == NULL)
   {
      Con::errorf("PopupMenu::insertSubMenu - Invalid PopupMenu object specified for submenu");
      return -1;
   }
   return object->insertSubMenu(pos, title, mnu);
}

DefineConsoleMethod(PopupMenu, setItem, bool, (S32 pos, const char * title, const char * accelerator, const char *cmd), (""), "(pos, title[, accelerator][, cmd])")
{
   return object->setItem(pos, title, accelerator, cmd);
}

//-----------------------------------------------------------------------------

DefineConsoleMethod(PopupMenu, enableItem, void, (S32 pos, bool enabled), , "(pos, enabled)")
{
   object->enableItem(pos, enabled);
}

DefineConsoleMethod(PopupMenu, checkItem, void, (S32 pos, bool checked), , "(pos, checked)")
{
   object->checkItem(pos, checked);
}

DefineConsoleMethod(PopupMenu, checkRadioItem, void, (S32 firstPos, S32 lastPos, S32 checkPos), , "(firstPos, lastPos, checkPos)")
{
   object->checkRadioItem(firstPos, lastPos, checkPos);
}

DefineConsoleMethod(PopupMenu, isItemChecked, bool, (S32 pos), , "(pos)")
{
   return object->isItemChecked(pos);
}

DefineConsoleMethod(PopupMenu, getItemCount, S32, (), , "()")
{
   return object->getItemCount();
}

//-----------------------------------------------------------------------------
DefineConsoleMethod(PopupMenu, showPopup, void, (const char * canvasName, S32 x, S32 y), ( -1, -1), "(Canvas,[x, y])")
{
   GuiCanvas *pCanvas = dynamic_cast<GuiCanvas*>(Sim::findObject(canvasName));
   object->showPopup(pCanvas, x, y);
}
