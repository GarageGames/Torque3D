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

#ifndef _GUITEXTEDITSLIDERBITMAPCTRL_H_
#define _GUITEXTEDITSLIDERBITMAPCTRL_H_

#ifndef _GUITYPES_H_
#include "gui/core/guiTypes.h"
#endif
#ifndef _GUITEXTEDITCTRL_H_
#include "gui/controls/guiTextEditCtrl.h"
#endif

class GuiTextEditSliderBitmapCtrl : public GuiTextEditCtrl
{
   typedef GuiTextEditCtrl Parent;

public:

   enum CtrlArea
   {
      None,
      Slider,
      ArrowUp,
      ArrowDown
   };

   GuiTextEditSliderBitmapCtrl();
   ~GuiTextEditSliderBitmapCtrl();

   DECLARE_CONOBJECT(GuiTextEditSliderBitmapCtrl);
   DECLARE_CATEGORY( "Gui Values" );
   DECLARE_DESCRIPTION( "A text control that display a numeric value and bitmapped up/down sliders." );

   static void initPersistFields();

   virtual void getText(char *dest);  // dest must be of size
                                      // StructDes::MAX_STRING_LEN + 1

   virtual void setText(const char *txt);

   void setValue();
   void checkRange();
   void checkIncValue();
   void timeInc(U32 elapseTime);

   virtual bool onKeyDown(const GuiEvent &event);
   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event);
   virtual void onMouseUp(const GuiEvent &event);
   virtual bool onMouseWheelUp(const GuiEvent &event);
   virtual bool onMouseWheelDown(const GuiEvent &event);
	
	bool onWake();
   virtual void onPreRender();
   virtual void onRender(Point2I offset, const RectI &updateRect);

	void setBitmap(const char *name);

protected:

   Point2F mRange;
   F32 mIncAmount;
   F32 mValue;
   F32 mIncCounter;
   F32 mMulInc;
   StringTableEntry mFormat;
   U32 mMouseDownTime;
   bool mFocusOnMouseWheel;
	S32 mNumberOfBitmaps;
	StringTableEntry mBitmapName;

   CtrlArea mTextAreaHit;
};

#endif //_GUITEXTEDITSLIDERBITMAPCTRL_H_
