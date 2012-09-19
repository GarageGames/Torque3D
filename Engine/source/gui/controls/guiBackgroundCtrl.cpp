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

#include "console/console.h"
#include "gui/controls/guiBackgroundCtrl.h"

IMPLEMENT_CONOBJECT(GuiBackgroundCtrl);

ConsoleDocClass( GuiBackgroundCtrl,
   "@brief Renders a background, so you can have a backdrop for your GUI.\n\n"

   "Deprecated\n\n"

   "@ingroup GuiImages\n"

   "@internal"
);

//--------------------------------------------------------------------------
GuiBackgroundCtrl::GuiBackgroundCtrl() : GuiControl()
{
   mDraw = false;
   mIsContainer = true;
}

//--------------------------------------------------------------------------
void GuiBackgroundCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if ( mDraw )
      Parent::onRender( offset, updateRect );

   renderChildControls(offset, updateRect);
}


