
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
#include "gfx/gfxDrawUtil.h"

#include "afx/ui/afxProgressBase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxSpellCastBar : public GuiControl, public afxProgressBase
{
  typedef GuiControl Parent;

  bool              want_border;
  bool              want_background;
  bool              use_alt_final_color;
  ColorF            rgba_background;
  ColorF            rgba_border;
  ColorF            rgba_fill;
  ColorF            rgba_fill_final;

  F32               fraction;

public:
  /*C*/             afxSpellCastBar();

  virtual void      onRender(Point2I, const RectI&);

  void              setFraction(F32 frac);
  F32               getFraction() const { return fraction; }

  virtual void      setProgress(F32 value) { setFraction(value); }
  virtual void      onStaticModified(const char* slotName, const char* newValue = NULL);

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxSpellCastBar);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CONOBJECT(afxSpellCastBar);

ConsoleDocClass( afxSpellCastBar,
   "@brief A GUI progress bar useful as a spell casting bar.\n\n"

   "@ingroup afxGUI\n"
   "@ingroup AFX\n"
);

afxSpellCastBar::afxSpellCastBar()
{
  want_border = true;
  want_background = true;
  use_alt_final_color = false;
  rgba_background.set(0.0f, 0.0f, 0.0f, 0.5f);
  rgba_border.set(0.5f, 0.5f, 0.5f, 1.0f);
  rgba_fill.set(0.0f, 1.0f, 1.0f, 1.0f);
  rgba_fill_final.set(0.0f, 1.0f, 1.0f, 1.0f);

  fraction = 0.5f;
}

void afxSpellCastBar::setFraction(F32 frac)
{
  fraction = mClampF(frac, 0.0f, 1.0f);
}

void afxSpellCastBar::onStaticModified(const char* slotName, const char* newValue)
{
   Parent::onStaticModified(slotName, newValue);
   if (dStricmp(slotName, "fillColorFinal") == 0)
      use_alt_final_color = true;
}

// STATIC 
void afxSpellCastBar::initPersistFields()
{
  addGroup("Colors");
  addField( "backgroundColor",  TypeColorF, Offset(rgba_background, afxSpellCastBar),
    "...");
  addField( "borderColor",      TypeColorF, Offset(rgba_border, afxSpellCastBar),
    "...");
  addField( "fillColor",        TypeColorF, Offset(rgba_fill, afxSpellCastBar),
    "...");
  addField( "fillColorFinal",   TypeColorF, Offset(rgba_fill_final, afxSpellCastBar),
    "...");
  endGroup("Colors");

  Parent::initPersistFields();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxSpellCastBar::onRender(Point2I offset, const RectI &updateRect)
{
  ColorF color;

  // draw the background
  if (want_background)
  {
    color.set(rgba_background.red, rgba_background.green, rgba_background.blue, rgba_background.alpha*fade_amt); 
    GFX->getDrawUtil()->drawRectFill(updateRect, color);
  }

  // calculate the rectangle dimensions
  RectI rect(updateRect);
  rect.extent.x = (S32)(rect.extent.x * fraction);

  // draw the filled part of bar
  if (fraction >= 1.0f && use_alt_final_color)
    color.set(rgba_fill_final.red, rgba_fill_final.green, rgba_fill_final.blue, rgba_fill_final.alpha*fade_amt);
  else
    color.set(rgba_fill.red, rgba_fill.green, rgba_fill.blue, rgba_fill.alpha*fade_amt);

  GFX->getDrawUtil()->drawRectFill(rect, color);

  // draw the border
  if (want_border)
  {
    color.set(rgba_border.red, rgba_border.green, rgba_border.blue, rgba_border.alpha*fade_amt);
    GFX->getDrawUtil()->drawRect(updateRect, color);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxSpellCastBar, setProgress, void, (float percentDone),,
                   "Set the progress percentage on the progress-bar.\n\n"
                   "@ingroup AFX")
{
  object->setFraction(percentDone);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
