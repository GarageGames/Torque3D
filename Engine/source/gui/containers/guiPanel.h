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
#ifndef _GUI_PANEL_H_
#define _GUI_PANEL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _GUITICKCTRL_H_
#include "gui/shiny/guiTickCtrl.h"
#endif
#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif


/// The GuiPanel panel is a container that when opaque will
/// draw a left to right gradient using its profile fill and
/// fill highlight colors.
///
/// @addtogroup gui_container_group Containers
///
/// @ingroup gui_group Gui System
/// @{
class GuiPanel : public GuiContainer
{
private:
   typedef GuiContainer Parent;

public:
   // Constructor/Destructor/ConObject Declaration
   GuiPanel();
   virtual ~GuiPanel();

   DECLARE_CONOBJECT(GuiPanel);

   // GuiControl
   void onRender(Point2I offset, const RectI &updateRect);
   void setVisible(bool value) { Parent::setVisible(value); setUpdateLayout( updateParent ); }
};
/// @}

#endif // _GUI_PANEL_H_