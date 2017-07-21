
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

#include "console/engineAPI.h"
#include "gui/core/guiControl.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDrawUtil.h"

#include "afx/ui/afxProgressBase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxStatusBar : public GuiControl, public afxProgressBase
{
  typedef GuiControl Parent;

  ColorF            rgba_fill;

  F32               fraction;
  ShapeBase*        shape;
  bool              show_energy;
  bool              monitor_player;

public:
  /*C*/             afxStatusBar();

  virtual void      onRender(Point2I, const RectI&);

  void              setFraction(F32 frac);
  F32               getFraction() const { return fraction; }

  virtual void      setProgress(F32 value) { setFraction(value); }

  void              setShape(ShapeBase* s);
  void              clearShape() { setShape(NULL); }

  virtual bool      onWake();
  virtual void      onSleep();
  virtual void      onMouseDown(const GuiEvent &event);
  virtual void      onDeleteNotify(SimObject*);

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxStatusBar);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CONOBJECT(afxStatusBar);

ConsoleDocClass( afxStatusBar,
   "@brief A GUI status bar for tracking and displaying health and energy of ShapeBase "
   "objects.\n\n"

   "@ingroup afxGUI\n"
   "@ingroup AFX\n"
);

afxStatusBar::afxStatusBar()
{
  rgba_fill.set(0.0f, 1.0f, 1.0f, 1.0f);

  fraction = 1.0f;
  shape = 0;
  show_energy = false;
  monitor_player = false;
}

void afxStatusBar::setFraction(F32 frac)
{
  fraction = mClampF(frac, 0.0f, 1.0f);
}

void afxStatusBar::setShape(ShapeBase* s) 
{ 
  if (shape)
    clearNotify(shape);
  shape = s;
  if (shape)
    deleteNotify(shape);
}

void afxStatusBar::onDeleteNotify(SimObject* obj)
{
  if (shape == (ShapeBase*)obj)
  {
    shape = NULL;
    return;
  }

  Parent::onDeleteNotify(obj);
}

bool afxStatusBar::onWake()
{
  if (!Parent::onWake())
    return false;

  return true;
}

void afxStatusBar::onSleep()
{
  //clearShape();
  Parent::onSleep();
}

// STATIC 
void afxStatusBar::initPersistFields()
{
  addField("fillColor",      TypeColorF, Offset(rgba_fill, afxStatusBar),
    "...");
  addField("displayEnergy",  TypeBool,   Offset(show_energy, afxStatusBar),
    "...");
  addField("monitorPlayer",  TypeBool,   Offset(monitor_player, afxStatusBar),
    "...");

  Parent::initPersistFields();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//


void afxStatusBar::onRender(Point2I offset, const RectI &updateRect)
{
  if (!shape)
    return;

  if (shape->getDamageState() != ShapeBase::Enabled)
    fraction = 0.0f;
  else
    fraction = (show_energy) ? shape->getEnergyValue() : (1.0f - shape->getDamageValue());

  // set alpha value for the fill area
  rgba_fill.alpha = 1.0f;

  // calculate the rectangle dimensions
  RectI rect(updateRect);
  rect.extent.x = (S32)(rect.extent.x*fraction);

  // draw the filled part of bar
  GFX->getDrawUtil()->drawRectFill(rect, rgba_fill);
}

void afxStatusBar::onMouseDown(const GuiEvent &event)
{
  GuiControl *parent = getParent();
  if (parent)
    parent->onMouseDown(event);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxStatusBar, setProgress, void, (float percentDone),,
                   "Set the progress percentage on the status-bar.\n\n"
                   "@ingroup AFX")
{
  object->setFraction(percentDone);
}

DefineEngineMethod(afxStatusBar, setShape, void, (ShapeBase* shape),,
                   "Associate a ShapeBase-derived object with the status-bar.\n\n"
                   "@ingroup AFX")
{
  object->setShape(shape);
}

DefineEngineMethod(afxStatusBar, clearShape, void, (),,
                   "Clear out any ShapeBase-derived object associated with the status-bar.\n\n"
                   "@ingroup AFX")
{
  object->clearShape();
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
