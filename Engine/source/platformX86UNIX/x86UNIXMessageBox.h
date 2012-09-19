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

#ifndef _X86UNIXMESSAGEBOX_H_
#define _X86UNIXMESSAGEBOX_H_

#include <X11/Xlib.h>
#include "core/util/tVector.h"

class XMessageBoxButton
{
   public:
      XMessageBoxButton();
      XMessageBoxButton(const char* label, int clickVal);

      const char *getLabel() { return static_cast<const char*>(mLabel); }
      int getClickVal() { return mClickVal; }

      int getLabelWidth() { return mLabelWidth; }
      void setLabelWidth(int width) { mLabelWidth = width; }

      void setButtonRect(int x, int y, int width, int height)
      {
         mX = x;
         mY = y;
         mWidth = width;
         mHeight = height;
      }
      void setMouseCoordinates(int x, int y)
      {
         mMouseX = x;
         mMouseY = y;
      }

      bool drawReverse()
      {
         return mMouseDown && pointInRect(mMouseX, mMouseY);
      }

      bool pointInRect(int x, int y)
      {
         if (x >= mX && x <= (mX+mWidth) &&
            y >= mY && y <= (mY+mHeight))
            return true;
         return false;
      }

      void setMouseDown(bool mouseDown) { mMouseDown = mouseDown; }
      bool isMouseDown() { return mMouseDown; }

   private:
      static const int LabelSize = 100;
      char mLabel[LabelSize];
      int mClickVal;
      int mLabelWidth;
      int mX, mY, mWidth, mHeight;
      int mMouseX, mMouseY;
      bool mMouseDown;
};

class XMessageBox
{
   public:
      static const int OK = 1;
      static const int Cancel = 2;
      static const int Retry = 3;

      XMessageBox(Display* display);
      ~XMessageBox();

      int alertOK(const char *windowTitle, const char *message);
      int alertOKCancel(const char *windowTitle, const char *message);
      int alertRetryCancel(const char *windowTitle, const char *message);
   private:
      int show();
      void repaint();
      void splitMessage();
      void clearMessageLines();
      int loadFont();
      void setDimensions();
      int getButtonLineWidth();

      const char* mMessage;
      const char* mTitle;
      Vector<XMessageBoxButton> mButtons;
      Vector<char*> mMessageLines;

      Display* mDisplay;
      GC mGC;
      Window mWin;
      XFontStruct* mFS;
      int mFontHeight;
      int mFontAscent;
      int mFontDescent;
      int mFontDirection;

      int mScreenWidth, mScreenHeight, mMaxWindowWidth, mMaxWindowHeight;
      int mMBWidth, mMBHeight;
};

#endif // #define _X86UNIXMESSAGEBOX_H_
