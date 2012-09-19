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

#ifndef _GUIFADEINBITMAPCTRL_H_
#define _GUIFADEINBITMAPCTRL_H_

#ifndef _GUIBITMAPCTRL_H_
   #include "gui/controls/guiBitmapCtrl.h"
#endif
#ifndef _MEASE_H_
   #include "math/mEase.h"
#endif


/// A control that fades a bitmap in and out.
class GuiFadeinBitmapCtrl : public GuiBitmapCtrl
{
   public:
   
      typedef GuiBitmapCtrl Parent;
      
   protected:
   
      /// Color we fade in from and fade out to.
      ColorF mFadeColor;
      
      /// Reference time on which to base all fade timings.
      U32 mStartTime;
      
      /// Milliseconds for bitmap to fade in.
      U32 mFadeInTime;
      
      /// Milliseconds to wait before fade-out.
      U32 mWaitTime;
      
      /// Milliseconds for bitmap to fade out.
      U32 mFadeOutTime;
      
      /// Easing curve for fade-in.
      EaseF mFadeInEase;
      
      /// Easing curve for fade-out.
      EaseF mFadeOutEase;

      /// Whether the fade cycle has run completely.
      bool mDone;

   public:

      GuiFadeinBitmapCtrl();

      // GuiControl.
      virtual void onPreRender();
      virtual void onMouseDown(const GuiEvent &);
      virtual bool onKeyDown(const GuiEvent &);
      virtual bool onWake();
      virtual void onRender(Point2I offset, const RectI &updateRect);
      
      static void initPersistFields();

      DECLARE_CONOBJECT( GuiFadeinBitmapCtrl );
      DECLARE_DESCRIPTION( "A control that shows a bitmap.  It fades the bitmap in a set amount of time,\n"
                           "then waits a set amount of time, and finally fades the bitmap back out in\n"
                           "another set amount of time." );

	  DECLARE_CALLBACK( void, click, ());
	  DECLARE_CALLBACK( void, onDone, ());
};

#endif // !_GUIFADEINBITMAPCTRL_H_
