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
#include "gui/shiny/guiTickCtrl.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT( GuiTickCtrl );

ConsoleDocClass( GuiTickCtrl,
	"@brief Brief Description.\n\n"
	"This Gui Control is designed to be subclassed to let people create controls "
	"which want to receive update ticks at a constant interval. This class was "
	"created to be the Parent class of a control which used a DynamicTexture "
	"along with a VectorField to create warping effects much like the ones found "
	"in visualization displays for iTunes or Winamp. Those displays are updated "
	"at the framerate frequency. This works fine for those effects, however for "
	"an application of the same type of effects for things like Gui transitions "
	"the framerate-driven update frequency is not desirable because it does not "
	"allow the developer to be able to have any idea of a consistent user-experience.\n\n"

	"Enter the ITickable interface. This lets the Gui control, in this case, update "
	"the dynamic texture at a constant rate of once per tick, even though it gets "
	"rendered every frame, thus creating a framerate-independent update frequency "
	"so that the effects are at a consistent speed regardless of the specifics "
	"of the system the user is on. This means that the screen-transitions will "
	"occur in the same time on a machine getting 300fps in the Gui shell as a "
	"machine which gets 150fps in the Gui shell.\n\n"

	"@ingroup GuiUtil\n");

//------------------------------------------------------------------------------

static ConsoleDocFragment _setProcessTicks(
   "This will set this object to either be processing ticks or not.\n\n"
   "@param tick (optional) True or nothing to enable ticking, false otherwise.\n\n"
   "@tsexample\n"
   "// Turn off ticking for a control, like a MenuBar (declared previously)\n"
   "%sampleMenuBar.setProcessTicks(false);\n"
   "@endtsexample\n\n",
   "GuiTickCtrl",
   "void setProcessTicks( bool tick )"
);

DefineConsoleMethod( GuiTickCtrl, setProcessTicks, void, (bool tick), (true), "( [tick = true] ) - This will set this object to either be processing ticks or not" )
{
   object->setProcessTicks(tick);
}