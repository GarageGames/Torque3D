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
	mMenuItems = NULL;
	mMenuBarCtrl = nullptr;

	mBarTitle = StringTable->EmptyString();
	mBounds = RectI(0, 0, 64, 64);
	mVisible = true;

	mBitmapIndex = -1;
	mDrawBitmapOnly = false;
	mDrawBorder = false;

	mTextList = nullptr;

	mIsSubmenu = false;
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

   addField("barTitle", TypeCaseString, Offset(mBarTitle, PopupMenu), "");
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
   newItem.mID = pos;
   newItem.mText = titleString;
   newItem.mCMD = cmd;

   if (titleString.isEmpty() || titleString == String("-"))
      newItem.mIsSpacer = true;
   else
      newItem.mIsSpacer = false;

   if (accelerator[0])
      newItem.mAccelerator = dStrdup(accelerator);
   else
      newItem.mAccelerator = NULL;

   newItem.mVisible = true;
   newItem.mIsChecked = false;
   newItem.mAcceleratorIndex = 0;
   newItem.mEnabled = !newItem.mIsSpacer;

   newItem.mIsSubmenu = false;
   newItem.mSubMenu = nullptr;
   newItem.mSubMenuParentMenu = nullptr;

   mMenuItems.push_back(newItem);

   return pos;
}

S32 PopupMenu::insertSubMenu(S32 pos, const char *title, PopupMenu *submenu)
{
   S32 itemPos = insertItem(pos, title, "", "");

   mMenuItems[itemPos].mIsSubmenu = true;
   mMenuItems[itemPos].mSubMenu = submenu;
   mMenuItems[itemPos].mSubMenuParentMenu = this;

   submenu->mIsSubmenu = true;

   return itemPos;
}

bool PopupMenu::setItem(S32 pos, const char *title, const char* accelerator, const char* cmd)
{
   String titleString = title;

   for (U32 i = 0; i < mMenuItems.size(); i++)
   {
      if (mMenuItems[i].mText == titleString)
      {
         mMenuItems[i].mID = pos;
         mMenuItems[i].mCMD = cmd;
         
         if (accelerator && accelerator[0])
            mMenuItems[i].mAccelerator = dStrdup(accelerator);
         else
            mMenuItems[i].mAccelerator = NULL;
         return true;
      }
   }
   
   return false;
}

void PopupMenu::removeItem(S32 itemPos)
{
   if (mMenuItems.empty() || mMenuItems.size() < itemPos || itemPos < 0)
      return;

   mMenuItems.erase(itemPos);
}

//////////////////////////////////////////////////////////////////////////
void PopupMenu::enableItem(S32 pos, bool enable)
{
   if (mMenuItems.empty() || mMenuItems.size() < pos || pos < 0)
      return;

   mMenuItems[pos].mEnabled = enable;
}

void PopupMenu::checkItem(S32 pos, bool checked)
{
   if (mMenuItems.empty() || mMenuItems.size() < pos || pos < 0)
      return;

   if (checked && mMenuItems[pos].mCheckGroup != -1)
   {
      // first, uncheck everything in the group:
      for (U32 i = 0; i < mMenuItems.size(); i++)
         if (mMenuItems[i].mCheckGroup == mMenuItems[pos].mCheckGroup && mMenuItems[i].mIsChecked)
            mMenuItems[i].mIsChecked = false;
   }

   mMenuItems[pos].mIsChecked = checked;
}

void PopupMenu::checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos)
{
   for (U32 i = 0; i < mMenuItems.size(); i++)
   {
      if (mMenuItems[i].mID >= firstPos && mMenuItems[i].mID <= lastPos)
      {
         mMenuItems[i].mIsChecked = false;
      }
   }
}

bool PopupMenu::isItemChecked(S32 pos)
{
   if (mMenuItems.empty() || mMenuItems.size() < pos || pos < 0)
      return false;

   return mMenuItems[pos].mIsChecked;
}

U32 PopupMenu::getItemCount()
{
   return mMenuItems.size();
}

void PopupMenu::clearItems()
{
	mMenuItems.clear();
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

      if (!mIsSubmenu)
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
         if (!mMenuItems[i].mVisible)
            continue;

         S32 iTextWidth = font->getStrWidth(mMenuItems[i].mText.c_str());
         S32 iAcceleratorWidth = mMenuItems[i].mAccelerator ? font->getStrWidth(mMenuItems[i].mAccelerator) : 0;

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
         if (!mMenuItems[i].mVisible)
            continue;

         char buf[512];

         //  If this menu item is a submenu, then set the isSubmenu to 2 to indicate
         // an arrow should be drawn.  Otherwise set the isSubmenu normally.
         char isSubmenu = 1;
         if (mMenuItems[i].mIsSubmenu)
            isSubmenu = 2;

         char bitmapIndex = 1;
         if (mMenuItems[i].mBitmapIndex >= 0 && (mMenuItems[i].mBitmapIndex * 3 <= profile->mBitmapArrayRects.size()))
            bitmapIndex = mMenuItems[i].mBitmapIndex + 2;

         dSprintf(buf, sizeof(buf), "%c%c\t%s\t%s", bitmapIndex, isSubmenu, mMenuItems[i].mText.c_str(), mMenuItems[i].mAccelerator ? mMenuItems[i].mAccelerator : "");
         mTextList->addEntry(entryCount, buf);

         if (!mMenuItems[i].mEnabled)
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
      if (mMenuItems[i].mSubMenu != nullptr)
         mMenuItems[i].mSubMenu->hidePopup();
   }
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------
DefineEngineMethod(PopupMenu, insertItem, S32, (S32 pos, const char * title, const char * accelerator, const char* cmd), ("", "", ""), "(pos[, title][, accelerator][, cmd])")
{
   return object->insertItem(pos, title, accelerator, cmd);
}

DefineEngineMethod(PopupMenu, removeItem, void, (S32 pos), , "(pos)")
{
   object->removeItem(pos);
}

DefineEngineMethod(PopupMenu, insertSubMenu, S32, (S32 pos, String title, String subMenu), , "(pos, title, subMenu)")
{
   PopupMenu *mnu = dynamic_cast<PopupMenu *>(Sim::findObject(subMenu));
   if(mnu == NULL)
   {
      Con::errorf("PopupMenu::insertSubMenu - Invalid PopupMenu object specified for submenu");
      return -1;
   }
   return object->insertSubMenu(pos, title, mnu);
}

DefineEngineMethod(PopupMenu, setItem, bool, (S32 pos, const char * title, const char * accelerator, const char *cmd), (""), "(pos, title[, accelerator][, cmd])")
{
   return object->setItem(pos, title, accelerator, cmd);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(PopupMenu, enableItem, void, (S32 pos, bool enabled), , "(pos, enabled)")
{
   object->enableItem(pos, enabled);
}

DefineEngineMethod(PopupMenu, checkItem, void, (S32 pos, bool checked), , "(pos, checked)")
{
   object->checkItem(pos, checked);
}

DefineEngineMethod(PopupMenu, checkRadioItem, void, (S32 firstPos, S32 lastPos, S32 checkPos), , "(firstPos, lastPos, checkPos)")
{
   object->checkRadioItem(firstPos, lastPos, checkPos);
}

DefineEngineMethod(PopupMenu, isItemChecked, bool, (S32 pos), , "(pos)")
{
   return object->isItemChecked(pos);
}

DefineEngineMethod(PopupMenu, getItemCount, S32, (), , "()")
{
   return object->getItemCount();
}

DefineEngineMethod(PopupMenu, clearItems, void, (), , "()")
{
	return object->clearItems();
}

//-----------------------------------------------------------------------------
DefineEngineMethod(PopupMenu, showPopup, void, (const char * canvasName, S32 x, S32 y), ( -1, -1), "(Canvas,[x, y])")
{
   GuiCanvas *pCanvas = dynamic_cast<GuiCanvas*>(Sim::findObject(canvasName));
   object->showPopup(pCanvas, x, y);
}
