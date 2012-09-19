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

#include "console/simBase.h"

#ifndef _MENUBAR_H_
#define _MENUBAR_H_

// Forward Refs
class PlatformMenuBarData;
class PopupMenu;
class GuiCanvas;

class MenuBar : public SimSet
{
   typedef SimSet Parent;

protected:
   PlatformMenuBarData *mData;
   GuiCanvas *mCanvas;

   /// Update the native menu bar to ensure consistency with the set
   void updateMenuBar(PopupMenu *menu = NULL);
   
   void createPlatformPopupMenuData();
   void deletePlatformPopupMenuData();
   
public:
   MenuBar();
   virtual ~MenuBar();
   DECLARE_CONOBJECT(MenuBar);

   /// Attach this menu bar to the native menu bar
   void attachToCanvas(GuiCanvas *owner, S32 pos);
   /// Remove this menu bar from the native menu bar
   void removeFromCanvas();

   /// Returns true if this menu is attached to the menu bar
   bool isAttachedToCanvas()                      { return mCanvas != NULL; }

   virtual void insertObject(SimObject *obj, S32 pos);

   // Overridden SimSet methods to ensure menu bar consistency when attached
   virtual void addObject(SimObject *obj);
   virtual void removeObject(SimObject *obj);
   virtual void pushObject(SimObject *obj);
   virtual void popObject();

   virtual bool reOrder(SimObject *obj, SimObject *target = 0);
};

#endif // _MENUBAR_H_
