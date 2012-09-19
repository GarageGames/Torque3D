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

#ifndef _GUIPROGRESSCTRL_H_
#define _GUIPROGRESSCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _GUITEXTCTRL_H_
#include "gui/controls/guiTextCtrl.h"
#endif


class GuiProgressCtrl : public GuiTextCtrl
{
private:
   typedef GuiTextCtrl Parent;

   F32 mProgress;

public:
   //creation methods
   DECLARE_CONOBJECT(GuiProgressCtrl);
   DECLARE_CATEGORY( "Gui Values" );
   DECLARE_DESCRIPTION( "A control that display a horizontal progress bar.  The bar is\n"
      "rendered using as a filled rectangle (properties determined by profile)." );
   GuiProgressCtrl();

   //console related methods
   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect);
};

#endif
