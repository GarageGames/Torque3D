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

#ifndef _GUIEASEVIEWCTRL_H_
#define _GUIEASEVIEWCTRL_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif
#ifndef _MEASE_H_
   #include "math/mEase.h"
#endif


/// Control to visualize an EaseF.
class GuiEaseViewCtrl : public GuiControl
{
   public:
   
      typedef GuiControl Parent;

   protected:

      EaseF mEase;         // ease we are visualizing
      ColorF mAxisColor;   // color to draw axis in
      ColorF mEaseColor;   // color to draw ease in
      F32 mEaseWidth;      // width of lines
   
   public:
   
      GuiEaseViewCtrl();

      bool onWake();
      void onSleep();

      void onRender( Point2I, const RectI &);
      static void initPersistFields();
      
      DECLARE_CONOBJECT( GuiEaseViewCtrl );
      DECLARE_CATEGORY( "Gui Editor" );
      DECLARE_DESCRIPTION( "Control that display an EaseF curve." );
};

#endif // !_GUIEASEVIEWCTRL_H_
