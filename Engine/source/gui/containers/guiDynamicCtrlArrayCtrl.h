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

#ifndef _GUIDYNAMICCTRLARRAYCTRL_H_
#define _GUIDYNAMICCTRLARRAYCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#include "gfx/gfxDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"

class GuiDynamicCtrlArrayControl : public GuiControl
{
   typedef GuiControl Parent;

public:

   GuiDynamicCtrlArrayControl();
   virtual ~GuiDynamicCtrlArrayControl();

   DECLARE_CONOBJECT(GuiDynamicCtrlArrayControl);
   DECLARE_CATEGORY( "Gui Containers" );

   // ConsoleObject
   static void initPersistFields();

   // SimObject
   void inspectPostApply();

   // SimSet
   void addObject(SimObject *obj);

   // GuiControl
   bool resize(const Point2I &newPosition, const Point2I &newExtent);
   void childResized(GuiControl *child);

   // GuiDynamicCtrlArrayCtrl
   void refresh();

protected:

   S32 mCols;
   S32 mRows;
   S32 mRowSize;
   S32 mColSize;
   S32 mRowSpacing;
   S32 mColSpacing;
   bool mResizing;
   bool mSizeToChildren;
   bool mAutoCellSize;
   bool mFrozen;
   bool mDynamicSize;   
   bool mFillRowFirst;

   RectSpacingI mPadding;
};

#endif // _GUIDYNAMICCTRLARRAYCTRL_H_