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
#include "platform/menus/menuBar.h"
#include "platform/menus/popupMenu.h"
#include "gui/core/guiCanvas.h"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------

MenuBar::MenuBar()
{
   createPlatformPopupMenuData();

   mCanvas = NULL;
}

MenuBar::~MenuBar()
{
   removeFromCanvas();

   deletePlatformPopupMenuData();
}

IMPLEMENT_CONOBJECT(MenuBar);

ConsoleDocClass( MenuBar,
   "@brief Used for rendering platform menu bars\n\n"
   "Internal use only\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------

void MenuBar::addObject(SimObject *obj)
{
   Parent::addObject(obj);
   updateMenuBar(dynamic_cast<PopupMenu *>(obj));
}

void MenuBar::removeObject(SimObject *obj)
{
   Parent::removeObject(obj);
   updateMenuBar(dynamic_cast<PopupMenu *>(obj));
}

void MenuBar::insertObject(SimObject *obj, S32 pos)
{
   Parent::addObject(obj);
   
   if(pos >= size())
      pos = size() - 1;

   if(pos < size())
   {
      if(pos < 0) pos = 0;
      Parent::reOrder(obj, at(pos));
   }
   updateMenuBar(dynamic_cast<PopupMenu *>(obj));
}

void MenuBar::pushObject(SimObject *obj)
{
   Parent::pushObject(obj);
   updateMenuBar(dynamic_cast<PopupMenu *>(obj));
}

void MenuBar::popObject()
{
   Parent::popObject();
   updateMenuBar();
}

bool MenuBar::reOrder(SimObject *obj, SimObject *target /*= 0*/)
{
   bool ret = Parent::reOrder(obj, target);
   if(ret)
      updateMenuBar(dynamic_cast<PopupMenu *>(obj));
   return ret;
}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

ConsoleMethod(MenuBar, attachToCanvas, void, 4, 4, "(GuiCanvas, pos)")
{
   object->attachToCanvas(dynamic_cast<GuiCanvas*>(Sim::findObject(argv[2])), dAtoi(argv[3]));
}

ConsoleMethod(MenuBar, removeFromCanvas, void, 2, 2, "()")
{
   object->removeFromCanvas();
}

//-----------------------------------------------------------------------------

ConsoleMethod(MenuBar, insert, void, 4, 4,"(object, pos) insert object at position")
{
   SimObject* pObject = Sim::findObject(argv[2]);

   if(pObject)
      object->insertObject(pObject, dAtoi(argv[3]));
}
