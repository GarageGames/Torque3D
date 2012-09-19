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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "platformX86UNIX/x86UNIXMessageBox.h"

#define MessageBox_MaxWinWidth 800
#define MessageBox_MaxWinHeight 600
#define MessageBox_MinWinWidth 450

#define MessageBox_ButtonBoxWidth 60
#define MessageBox_ButtonBoxHeight 22
#define MessageBox_ButtonSpacer 20
#define MessageBox_ButtonVMargin 10
#define MessageBox_ButtonHMargin 10

#define MessageBox_LineSpacer 2
#define MessageBox_LineVMargin 10
#define MessageBox_LineHMargin 10

XMessageBoxButton::XMessageBoxButton()
{
   strcpy(mLabel, "");
   mClickVal = -1;
   mLabelWidth = mX = mY = mWidth = mHeight = mMouseX = mMouseX = -1;
   mMouseDown = false;
}

XMessageBoxButton::XMessageBoxButton(const char* label, int clickVal)
{
   strncpy(mLabel, label, LabelSize);
   mClickVal = clickVal;
   mLabelWidth = mX = mY = mWidth = mHeight = mMouseX = mMouseX = -1;
   mMouseDown = false;
}

XMessageBox::XMessageBox(Display* display)
{
   mMessage = "";
   mFS = NULL;
   mDisplay = display;
}

XMessageBox::~XMessageBox()
{
   clearMessageLines();
   if (mDisplay != NULL)
   {
      mDisplay = NULL;
   }
}

int XMessageBox::alertOK(const char *windowTitle, const char *message)
{
   mMessage = message;
   mTitle = windowTitle;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("OK", OK));
   return show();
}

int XMessageBox::alertOKCancel(const char *windowTitle, const char *message)
{
   mMessage = message;
   mTitle = windowTitle;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("OK", OK));
   mButtons.push_back(XMessageBoxButton("Cancel", Cancel));
   return show();
}

int XMessageBox::alertRetryCancel(const char *windowTitle, const char *message)
{
   mMessage = message;
   mTitle = windowTitle;
   mButtons.clear();
   mButtons.push_back(XMessageBoxButton("Retry", Retry));
   mButtons.push_back(XMessageBoxButton("Cancel", Cancel));
   return show();
}

void XMessageBox::repaint()
{
   int white = WhitePixel(mDisplay, DefaultScreen(mDisplay));
   int black = BlackPixel(mDisplay, DefaultScreen(mDisplay));

   int x = 0;
   int y = 0;

   // line V margin
   y = y + MessageBox_LineVMargin * 2;

   // line H margin 
   x = MessageBox_LineHMargin;

   XSetForeground(mDisplay, mGC, black);
   for (unsigned int i = 0; i < mMessageLines.size(); ++i)
   {
      XDrawString(mDisplay, mWin, mGC, x, y, mMessageLines[i], 
         strlen(mMessageLines[i]));
      if (i < (mMessageLines.size() - 1))
         y = y + MessageBox_LineSpacer + mFontHeight;
   }
   XFlush(mDisplay);

   // line V margin
   y = y + MessageBox_LineVMargin;

   int maxButWidth = MessageBox_ButtonBoxWidth;
   int maxButHeight = MessageBox_ButtonBoxHeight;

   // compute size of text labels on buttons
   int fgColor, bgColor;

   int fontDirection, fontAscent, fontDescent;
   Vector<XMessageBoxButton>::iterator iter;
   for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
   {
      XCharStruct strInfo;
      XTextExtents(mFS, iter->getLabel(), strlen(iter->getLabel()), 
         &fontDirection, &fontAscent, &fontDescent,
         &strInfo);
//       if (maxButWidth < strInfo.width)
//          maxButWidth = strInfo.width;
//       if (maxButHeight < (strInfo.ascent + strInfo.descent))
//          maxButHeight = (strInfo.ascent + strInfo.descent);
      iter->setLabelWidth(strInfo.width);
   }
   int buttonBoxWidth = maxButWidth;
   int buttonBoxHeight = maxButHeight;

   // draw buttons
   // button V margin
   y = y + MessageBox_ButtonVMargin;

   // center the buttons 
   x = MessageBox_ButtonHMargin + (mMBWidth - getButtonLineWidth()) / 2;

   for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
   {
      if (iter->drawReverse())
      {
         fgColor = white;
         bgColor = black;
      }
      else
      {
         fgColor = black;
         bgColor = white;
      }

      XSetForeground(mDisplay, mGC, bgColor);
      XFillRectangle(mDisplay, mWin, mGC, x, y, 
         buttonBoxWidth, buttonBoxHeight);
      XSetForeground(mDisplay, mGC, fgColor);
      XDrawRectangle(mDisplay, mWin, mGC, x, y, 
         buttonBoxWidth, buttonBoxHeight);
      XDrawString(mDisplay, mWin, mGC, 
         x + ((buttonBoxWidth - iter->getLabelWidth()) / 2),
         y + mFontAscent + ((buttonBoxHeight - mFontAscent) / 2),
         iter->getLabel(),
         strlen(iter->getLabel()));
      iter->setButtonRect(x, y, buttonBoxWidth, buttonBoxHeight);
      x = x + buttonBoxWidth + MessageBox_ButtonSpacer;
   }   
}

template <class Type>
static inline Type max(Type v1, Type v2)
{
   if (v1 <= v2)
      return v2;
   else
      return v1;
}

template <class Type>
static inline Type min(Type v1, Type v2)
{
   if (v1 > v2)
      return v2;
   else
      return v1;
}

void XMessageBox::clearMessageLines()
{
   Vector<char*>::iterator iter;
   for (iter = mMessageLines.begin(); iter != mMessageLines.end(); ++iter)
      delete [] *iter;
   mMessageLines.clear();
}

void XMessageBox::splitMessage()
{
   clearMessageLines();
   if (mMessage == NULL || strlen(mMessage)==0)
      // JMQTODO: what to do with empty strings?
      return;

   // need to break message up in to lines, and store lines in 
   // mMessageLines

   int numChars = strlen(mMessage);
   const int ScratchBufSize = 2048;
   char scratchBuf[ScratchBufSize];
   memset(scratchBuf, 0, ScratchBufSize);

   int fontDirection, fontAscent, fontDescent;
   XCharStruct strInfo;

   char *curChar = const_cast<char*>(mMessage);
   char *endChar;
   char *curWrapped = scratchBuf;
   int curWidth = 0;
   int maxWidth = mMaxWindowWidth - (MessageBox_LineHMargin);

   while ( // while pointers are in range...
      (curChar - mMessage) < numChars &&
      (curWrapped - scratchBuf) < ScratchBufSize)
   {
      // look for next space in remaining string
      endChar = index(curChar, ' ');
      if (endChar == NULL)
         endChar = index(curChar, '\0');

      if (endChar != NULL)
         // increment one char past the space to include it
         endChar++;
      else
         // otherwise, set the endchar to one char ahead
         endChar = curChar + 1;

      // compute length of substr
      int len = endChar - curChar;
      XTextExtents(mFS, curChar, len, 
         &fontDirection, &fontAscent, &fontDescent,
         &strInfo);
      // if its too big, time to add a new line...
      if ((curWidth + strInfo.width) > maxWidth)
      {
         // create a new block for the line and add it
         *curWrapped = '\0';
         int len = strlen(scratchBuf);
         char* line = new char[len+1];
         strncpy(line, scratchBuf, len+1); // +1 gets the null char
         mMessageLines.push_back(line);
         
         // reset curWrapped to the beginning of the scratch buffer
         curWrapped = scratchBuf;
         curWidth = 0;
      }
      // copy the current string into curWrapped if we have enough room
      int bytesRemaining = 
         ScratchBufSize - (curWrapped - scratchBuf);
      if (bytesRemaining >= len)
         strncpy(curWrapped, curChar, len);

      curWrapped += len;
      curWidth += strInfo.width;
      curChar = endChar;
   }

   // make a final line out of any leftover stuff in the scratch buffer
   if (curWrapped != scratchBuf)
   {
      *curWrapped = '\0';
      int len = strlen(scratchBuf);
      char* line = new char[len+1];
      strncpy(line, scratchBuf, len+1); // +1 gets the null char
      mMessageLines.push_back(line);
   }
}

int XMessageBox::loadFont()
{
  // load the font
   mFS = XLoadQueryFont(mDisplay, 
      "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*");
   
   if (mFS == NULL)
      mFS = XLoadQueryFont(mDisplay, "fixed");

   if (mFS == NULL)
      return -1;

   // dummy call to XTextExtents to get the font specs
   XCharStruct strInfo;
   
   XTextExtents(mFS, "foo", 1, 
      &mFontDirection, &mFontAscent, &mFontDescent,
      &strInfo);

   mFontHeight = mFontAscent + mFontDescent;
   return 0;
}

int XMessageBox::getButtonLineWidth()
{
   return mButtons.size() * MessageBox_ButtonBoxWidth +
      (mButtons.size() - 1) * MessageBox_ButtonSpacer +
      MessageBox_ButtonHMargin * 2;
}

void XMessageBox::setDimensions()
{
   mMBWidth = MessageBox_MaxWinWidth;
   mMBHeight = MessageBox_MaxWinHeight;

   // determine width of button line
   int buttonWidth = getButtonLineWidth();

   // if there is only one line, the desired width is the greater of the 
   // line width and the buttonWidth, otherwise the lineWidth is the 
   // max possible width which we already set.
   if (mMessageLines.size() == 1)
   {
      XCharStruct strInfo;
      int fontDirection, fontAscent, fontDescent;

      XTextExtents(mFS, mMessageLines[0], strlen(mMessageLines[0]), 
         &fontDirection, &fontAscent, &fontDescent,
         &strInfo);

      mMBWidth = max(MessageBox_LineHMargin * 2 + strInfo.width,
         buttonWidth);
      mMBWidth = max(mMBWidth, MessageBox_MinWinWidth);
   }

   // determine the height of the button line
   int buttonHeight = MessageBox_ButtonBoxHeight + 
      MessageBox_ButtonVMargin * 2;

   int lineHeight = mFontHeight * mMessageLines.size() +
      (mMessageLines.size() - 1) * MessageBox_LineSpacer +
      MessageBox_LineVMargin * 2;

   mMBHeight = buttonHeight + lineHeight;
}

int XMessageBox::show()
{
   if (mDisplay == NULL)
      return -1;

   int retVal = 0;
   retVal = loadFont();
   if (retVal < 0)
      return retVal;

   // set the maximum window dimensions
   mScreenWidth = DisplayWidth(mDisplay, DefaultScreen(mDisplay));
   mScreenHeight = DisplayHeight(mDisplay, DefaultScreen(mDisplay));
   mMaxWindowWidth = min(mScreenWidth, MessageBox_MaxWinWidth);
   mMaxWindowHeight = min(mScreenHeight, MessageBox_MaxWinHeight);

   // split the message into a vector of lines
   splitMessage();

   // set the dialog dimensions
   setDimensions();

   mWin = XCreateSimpleWindow(
      mDisplay,
      DefaultRootWindow(mDisplay),
      (mScreenWidth - mMBWidth) / 2,  (mScreenHeight - mMBHeight) / 2,
      mMBWidth, mMBHeight,
      1, 
      BlackPixel(mDisplay, DefaultScreen(mDisplay)),
      WhitePixel(mDisplay, DefaultScreen(mDisplay)));

   mGC = XCreateGC(mDisplay, mWin, 0, 0);

   XSetFont(mDisplay, mGC, mFS->fid);

   // set input mask
   XSelectInput(mDisplay, mWin, 
      ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);

   // set wm protocols in case they hit X
   Atom wm_delete_window = 
      XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
   Atom wm_protocols = 
      XInternAtom(mDisplay, "WM_PROTOCOLS", False);
   XSetWMProtocols (mDisplay, mWin, &wm_delete_window, 1);
   // set pop up dialog hint
   XSetTransientForHint(mDisplay, mWin, mWin);
   
   // set title
   XTextProperty wtitle;
   wtitle.value = (unsigned char *)mTitle;
   wtitle.encoding = XA_STRING;
   wtitle.format = 8;
   wtitle.nitems = strlen(mTitle);
   XSetWMName(mDisplay, mWin, &wtitle);

   // show window
   XMapWindow(mDisplay, mWin);
   // move it in case some bozo window manager repositioned it
   XMoveWindow(mDisplay, mWin, 
      (mScreenWidth - mMBWidth) / 2,  (mScreenHeight - mMBHeight) / 2);
   // raise it to top
   XRaiseWindow(mDisplay, mWin);

   XMessageBoxButton* clickedButton = NULL;
   XEvent event;
   Vector<XMessageBoxButton>::iterator iter;
   bool done = false;
   while (!done)
   {
      XNextEvent(mDisplay, &event);
      switch (event.type)
      {
         case Expose:
            repaint();
            break;
         case MotionNotify:
            for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
               iter->setMouseCoordinates(event.xmotion.x, event.xmotion.y);
            break;
         case ButtonPress:
            for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
            {
               if (iter->pointInRect(event.xbutton.x, event.xbutton.y))
               {
                  iter->setMouseDown(true);
                  iter->setMouseCoordinates(event.xbutton.x, event.xbutton.y);
                  break;
               }
            }
            break;
         case ButtonRelease:
            for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
            {
               if (iter->pointInRect(event.xbutton.x, event.xbutton.y) &&
                  iter->isMouseDown())
               {
                  // we got a winner!
                  clickedButton = iter;
                  done = true;
                  break;
               }
            }
            if (clickedButton == NULL)
            {
               // user released outside a button.  clear the button states
               for (iter = mButtons.begin(); iter != mButtons.end(); ++iter)
                  iter->setMouseDown(false);
            }
            break;
         case ClientMessage:
            if (event.xclient.message_type == wm_protocols &&
               event.xclient.data.l[0] == static_cast<long>(wm_delete_window))
               done = true;
            break;
      }
      repaint();
   }

   XUnmapWindow(mDisplay, mWin);
   XDestroyWindow(mDisplay, mWin);
   XFreeGC(mDisplay, mGC);
   XFreeFont(mDisplay, mFS);

   if (clickedButton != NULL)
      return clickedButton->getClickVal();
   else
      return -1;
}
