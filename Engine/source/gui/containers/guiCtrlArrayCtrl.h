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

#ifndef _GUICTRLARRAYCTRL_H_
#define _GUICTRLARRAYCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#include "gfx/gfxDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"

class GuiControlArrayControl : public GuiControl
{
private:
   typedef GuiControl Parent;

   bool mResizing;

   S32 mCols;
   Vector<S32> mColumnSizes;
   S32 mRowSize;
   S32 mRowSpacing;
   S32 mColSpacing;

public:
   GuiControlArrayControl();

   bool resize(const Point2I &newPosition, const Point2I &newExtent);

   bool onWake();
   void onSleep();
   void inspectPostApply();

   bool updateArray();

   void addObject(SimObject *obj);
   void removeObject(SimObject *obj);

   bool reOrder(SimObject* obj, SimObject* target = 0);

   static void initPersistFields();
   DECLARE_CONOBJECT(GuiControlArrayControl);
   DECLARE_CATEGORY( "Gui Containers" );
};

#endif