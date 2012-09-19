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

#include "gui/core/guiControl.h"

//------------------------------------------------------------------------------
class GuiNoMouseCtrl : public GuiControl
{
   typedef GuiControl Parent;
   public:

      // GuiControl
      bool pointInControl(const Point2I &)   { return(false); }
      DECLARE_CONOBJECT(GuiNoMouseCtrl);
      DECLARE_CATEGORY( "Gui Other" );
};
IMPLEMENT_CONOBJECT(GuiNoMouseCtrl);

ConsoleDocClass( GuiNoMouseCtrl,
				"@brief No known usage, possibly Legacy.\n\n"
				"Not used at all, internal until deprecated\n\n"
				"@internal");