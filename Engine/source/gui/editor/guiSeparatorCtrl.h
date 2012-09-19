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
#ifndef _GUISEPARATORCTRL_H_
#define _GUISEPARATORCTRL_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif


/// Renders a separator line with optional text.
class GuiSeparatorCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;

public:
   bool  mInvisible;
   String mText;
   S32   mTextLeftMargin;
   S32   mMargin;
   S32   mSeparatorType;

   enum separatorTypeOptions
   {
      separatorTypeVertical = 0, ///< Draw Vertical Separator
      separatorTypeHorizontal    ///< Horizontal Separator
   };

   //creation methods
   DECLARE_CONOBJECT(GuiSeparatorCtrl);
   DECLARE_CATEGORY( "Gui Other" );
   DECLARE_DESCRIPTION( "A control that renders a horizontal or vertical separator with\n"
      "an optional text label (horizontal only). ");
   GuiSeparatorCtrl();

   static void initPersistFields();

   void onRender(Point2I offset, const RectI &updateRect);
};

typedef GuiSeparatorCtrl::separatorTypeOptions GuiSeparatorType;
DefineEnumType( GuiSeparatorType );

#endif
