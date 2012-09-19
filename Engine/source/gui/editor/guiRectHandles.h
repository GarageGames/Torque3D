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
#ifndef _GUIRECTHANDLES_H_
#define _GUIRECTHANDLES_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GuiRectHandles : public GuiControl
{
private:
   typedef GuiControl Parent;

protected:
   RectF    mHandleRect;
   S32      mHandleSize;
   bool     mUseCustomColor;
   ColorI   mHandleColor;
   S32      mHitHandle;    // 0 = none, 1-8 = clockwise circle starting upper left, 9 = centre
   Point2I  mHitPoint;

public:
   DECLARE_CONOBJECT(GuiRectHandles);
   DECLARE_CATEGORY( "Gui Other" );
   DECLARE_DESCRIPTION( "Draws a box with handles for the user to manipulate.");

   GuiRectHandles();
   virtual ~GuiRectHandles();

   static void initPersistFields();

   virtual void onMouseUp(const GuiEvent &event);
   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event);

   virtual void onRender(Point2I offset, const RectI &updateRect);
};

#endif
