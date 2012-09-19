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

#ifndef _GUIBUBBLETEXTCTRL_H_
#define _GUIBUBBLETEXTCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "gui/controls/guiTextCtrl.h"
#endif
#ifndef _GUIMLTEXTCTRL_H_
#include "gui/controls/guiMLTextCtrl.h"
#endif

/// A single-line text control that displays its text in a multi-line popup when
/// clicked.
class GuiBubbleTextCtrl : public GuiTextCtrl
{
   private:
   
      typedef GuiTextCtrl Parent;

  protected:
      bool mInAction;
      GuiControl *mDlg;
      GuiControl *mPopup;
      GuiMLTextCtrl *mMLText;

      void popBubble();

  public:
  
      DECLARE_CONOBJECT(GuiBubbleTextCtrl);
      DECLARE_DESCRIPTION( "A single-line text control that displays its text in a multi-line\n"
         "popup when clicked." );

      GuiBubbleTextCtrl() { mInAction = false; }

      virtual void onMouseDown(const GuiEvent &event);
};

#endif /* _GUI_BUBBLE_TEXT_CONTROL_H_ */
