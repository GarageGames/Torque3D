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
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "core/util/safeDelete.h"

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

PopupMenu::PopupMenu() : mCanvas(NULL)
{
   createPlatformPopupMenuData();

   mSubmenus = new SimSet;
   mSubmenus->registerObject();

   mBarTitle = StringTable->insert("");
   mIsPopup = false;

	mPopupGUID = sMaxPopupGUID++;
}

PopupMenu::~PopupMenu()
{
   // This searches the menu bar so is safe to call for menus
   // that aren't on it, since nothing will happen.
   removeFromMenuBar();

   SimSet::iterator i;
   while((i = mSubmenus->begin()) != mSubmenus->end())
   {
      (*i)->deleteObject();
   }

   mSubmenus->deleteObject();
   deletePlatformPopupMenuData();

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
   addField("isPopup",     TypeBool,         Offset(mIsPopup, PopupMenu),  "true if this is a pop-up/context menu. defaults to false.");
   addField("barTitle",    TypeCaseString,   Offset(mBarTitle, PopupMenu), "the title of this menu when attached to a menu bar");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool PopupMenu::onAdd()
{
   if(! Parent::onAdd())
      return false;

   createPlatformMenu();

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
	if (popID == mPopupGUID && canHandleID(command))	
		if (handleSelect(command))
			smSelectionEventHandled = true;
}

//-----------------------------------------------------------------------------

void PopupMenu::onAttachToMenuBar(GuiCanvas *canvas, S32 pos, const char *title)
{
   mCanvas = canvas;

	// Attached menus must be notified of menu events
	smPopupMenuEvent.notify(this, &PopupMenu::handleSelectEvent);
   
   // Pass on to sub menus
   for(SimSet::iterator i = mSubmenus->begin();i != mSubmenus->end();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(*i);
      if(mnu == NULL)
         continue;

      mnu->onAttachToMenuBar(canvas, pos, title);
   }

   // Call script
   if(isProperlyAdded())
      Con::executef(this, "onAttachToMenuBar", Con::getIntArg(canvas ? canvas->getId() : 0), Con::getIntArg(pos), title);
}

void PopupMenu::onRemoveFromMenuBar(GuiCanvas *canvas)
{
   mCanvas = NULL;

	// We are no longer interested in select events, remove ourselves from the notification list in a safe way
	Sim::postCurrentEvent(this, new PopUpNotifyRemoveEvent());
      
   // Pass on to sub menus
   for(SimSet::iterator i = mSubmenus->begin();i != mSubmenus->end();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(*i);
      if(mnu == NULL)
         continue;

      mnu->onRemoveFromMenuBar(canvas);
   }

   // Call script
   if(isProperlyAdded())
      Con::executef(this, "onRemoveFromMenuBar", Con::getIntArg(canvas ? canvas->getId() : 0));
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

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

DefineConsoleMethod(PopupMenu, insertItem, S32, (S32 pos, const char * title, const char * accelerator), ("", ""), "(pos[, title][, accelerator])")
{
   return object->insertItem(pos, title, accelerator);
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

DefineConsoleMethod(PopupMenu, setItem, bool, (S32 pos, const char * title, const char * accelerator), (""), "(pos, title[, accelerator])")
{
   return object->setItem(pos, title, accelerator);
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

DefineConsoleMethod(PopupMenu, attachToMenuBar, void, (const char * canvasName, S32 pos, const char * title), , "(GuiCanvas, pos, title)")
{
   object->attachToMenuBar(dynamic_cast<GuiCanvas*>(Sim::findObject(canvasName)), pos, title);
}

DefineConsoleMethod(PopupMenu, removeFromMenuBar, void, (), , "()")
{
   object->removeFromMenuBar();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod(PopupMenu, showPopup, void, (const char * canvasName, S32 x, S32 y), ( -1, -1), "(Canvas,[x, y])")
{
   GuiCanvas *pCanvas = dynamic_cast<GuiCanvas*>(Sim::findObject(canvasName));
   object->showPopup(pCanvas, x, y);
}
