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
#ifndef _GUIDECOYCTRL_H_
#define _GUIDECOYCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GuiDecoyCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
   // Constructor/Destructor/ConObject Declaration
   GuiDecoyCtrl();
   virtual ~GuiDecoyCtrl();

   DECLARE_CONOBJECT(GuiDecoyCtrl);
   DECLARE_CATEGORY( "Gui Other" );
   
   static void initPersistFields();

   bool	mMouseOver;
   bool mIsDecoy;
   GuiControl* mDecoyReference;
   bool mMouseOverDecoy;
   Point2I mMouseDownPosition;


   virtual void onMouseUp(const GuiEvent &event);
   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseMove(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event);
   virtual void onMouseEnter(const GuiEvent &event);
   virtual void onMouseLeave(const GuiEvent &event);

   virtual bool onMouseWheelUp(const GuiEvent &event);
   virtual bool onMouseWheelDown(const GuiEvent &event);

   virtual void onRightMouseDown(const GuiEvent &event);
   virtual void onRightMouseUp(const GuiEvent &event);
   virtual void onRightMouseDragged(const GuiEvent &event);

   virtual void onMiddleMouseDown(const GuiEvent &event);
   virtual void onMiddleMouseUp(const GuiEvent &event);
   virtual void onMiddleMouseDragged(const GuiEvent &event);
};
#endif
