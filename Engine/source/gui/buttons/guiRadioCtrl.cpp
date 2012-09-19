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

#include "gui/buttons/guiRadioCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gui/core/guiCanvas.h"

IMPLEMENT_CONOBJECT(GuiRadioCtrl);

ConsoleDocClass( GuiRadioCtrl,
   "@brief A button based around the radio concept.\n\n"
   
   "GuiRadioCtrl's functionality is based around GuiButtonBaseCtrl's ButtonTypeRadio type.\n\n"

   "A button control with a radio box and a text label.\n"
   "This control is used in groups where multiple radio buttons\n"
   "present a range of options out of which one can be chosen.\n"
   "A radio button automatically signals its siblings when it is\n"
   "toggled on.\n\n"
   
   "@tsexample\n"
   "// Create a GuiCheckBoxCtrl that calls randomFunction with its current value when clicked.\n"
   "%radio = new GuiRadioCtrl()\n"
   "{\n"
   "   profile = \"GuiRadioProfile\";\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiButtons"
);

//-----------------------------------------------------------------------------

GuiRadioCtrl::GuiRadioCtrl()
{
   mButtonType = ButtonTypeRadio;
}

