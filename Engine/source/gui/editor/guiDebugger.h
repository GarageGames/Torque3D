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

#ifndef _GUIDEBUGGER_H_
#define _GUIDEBUGGER_H_

#ifndef _GUIARRAYCTRL_H_
#include "gui/core/guiArrayCtrl.h"
#endif

class DbgFileView : public GuiArrayCtrl
{
  private:

   typedef GuiArrayCtrl Parent;

   struct FileLine
   {
      bool breakPosition;
      bool breakOnLine;
      char *text;
   };

   Vector<FileLine> mFileView;

   StringTableEntry mFileName;

   void AdjustCellSize();

   //used to display the program counter
   StringTableEntry mPCFileName;
   S32 mPCCurrentLine;

   //vars used to highlight the selected line segment for copying
   bool mbMouseDragging;
   S32 mMouseDownChar;
   S32 mBlockStart;
   S32 mBlockEnd;

   char mMouseOverVariable[256];
   char mMouseOverValue[256];
   S32 findMouseOverChar(const char *text, S32 stringPosition);
   bool findMouseOverVariable();
   S32 mMouseVarStart;
   S32 mMouseVarEnd;

   //find vars
   char mFindString[256];
   S32 mFindLineNumber;

  public:

   DbgFileView();
   ~DbgFileView();
   
   DECLARE_CONOBJECT(DbgFileView);
   DECLARE_CATEGORY( "Gui Editor" );

   bool onWake();

   void clear();
   void clearBreakPositions();

   void setCurrentLine(S32 lineNumber, bool setCurrentLine);
   const char *getCurrentLine(S32 &lineNumber);
   bool openFile(const char *fileName);
   void scrollToLine(S32 lineNumber);
   void setBreakPointStatus(U32 lineNumber, bool value);
   void setBreakPosition(U32 line);
   void addLine(const char *text, U32 textLen);

   bool findString(const char *text);

   void onMouseDown(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);

   void onPreRender();
   void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
};

#endif //_GUI_DEBUGGER_H
