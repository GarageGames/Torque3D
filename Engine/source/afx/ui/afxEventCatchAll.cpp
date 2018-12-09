
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "gui/3d/guiTSControl.h"

class afxEventCatchAll : public GuiControl 
{
  typedef GuiControl Parent;
  
public:
  /* C */         afxEventCatchAll() { }

  virtual void    getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
  
  virtual void    onMouseUp(const GuiEvent&);
  virtual void    onMouseDown(const GuiEvent&);
  virtual void    onMouseMove(const GuiEvent&);
  virtual void    onMouseDragged(const GuiEvent&);
  virtual void    onMouseEnter(const GuiEvent&);
  virtual void    onMouseLeave(const GuiEvent&);
  
  virtual bool    onMouseWheelUp(const GuiEvent&);
  virtual bool    onMouseWheelDown(const GuiEvent&);
  
  virtual void    onRightMouseDown(const GuiEvent&);
  virtual void    onRightMouseUp(const GuiEvent&);
  virtual void    onRightMouseDragged(const GuiEvent&);
  
  DECLARE_CONOBJECT(afxEventCatchAll);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CONOBJECT(afxEventCatchAll);

ConsoleDocClass( afxEventCatchAll,
   "@brief A simple but useful GUI control that propagates all events to its parent "
   "control.\n\n"

   "afxEventCatchAll is useful when you want certain events to be handled by its parent "
   "afxTSCtrl and not get consumed by certain non-interactive controls like a text "
   "HUD.\n\n"

   "@ingroup afxGUI\n"
   "@ingroup AFX\n"
);

void afxEventCatchAll::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->getCursor(cursor, showCursor, lastGuiEvent);
}

void afxEventCatchAll::onMouseUp(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseUp(evt);
}

void afxEventCatchAll::onMouseDown(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseDown(evt);
}

void afxEventCatchAll::onMouseMove(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseMove(evt);
}

void afxEventCatchAll::onMouseDragged(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseDragged(evt);
}

void afxEventCatchAll::onMouseEnter(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseEnter(evt);
}

void afxEventCatchAll::onMouseLeave(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onMouseLeave(evt);
}

bool afxEventCatchAll::onMouseWheelUp(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  return (parent) ? parent->onMouseWheelUp(evt) : false;
}

bool afxEventCatchAll::onMouseWheelDown(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  return (parent) ? parent->onMouseWheelDown(evt) : false;
}

void afxEventCatchAll::onRightMouseDown(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onRightMouseDown(evt);
}

void afxEventCatchAll::onRightMouseUp(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onRightMouseUp(evt);
}

void afxEventCatchAll::onRightMouseDragged(const GuiEvent& evt)
{   
  GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());   
  if (parent) parent->onRightMouseDragged(evt);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//


