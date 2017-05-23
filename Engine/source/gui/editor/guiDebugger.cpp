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

#include "platform/platform.h"
#include "gui/editor/guiDebugger.h"

#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"
#include "gfx/gfxDrawUtil.h"
#include "core/volume.h"


IMPLEMENT_CONOBJECT(DbgFileView);

ConsoleDocClass( DbgFileView,
   "@brief Remnant from ancient script debugger (TGE days?)\n\n"
   "Possibly useful for an editor tooltip.\n\n"
   "@internal"
);

static const char* itoa(S32 i)
{
   static char buf[32];
   dSprintf(buf, sizeof(buf), "%d", i);
   return buf;
}

DbgFileView::~DbgFileView()
{
   clear();
}

DbgFileView::DbgFileView()
{
   VECTOR_SET_ASSOCIATION(mFileView);

   mFileName = NULL;
   mPCFileName = NULL;
   mPCCurrentLine = -1;

   mBlockStart = -1;
   mBlockEnd = -1;

   mFindString[0] = '\0';
	mFindLineNumber = -1;

   mSize.set(1, 0);
}

DefineConsoleMethod(DbgFileView, setCurrentLine, void, (S32 line, bool selected), , "(int line, bool selected)"
              "Set the current highlighted line.")
{
   object->setCurrentLine(line, selected);
}

DefineConsoleMethod(DbgFileView, getCurrentLine, const char *, (), , "()"
              "Get the currently executing file and line, if any.\n\n"
              "@returns A string containing the file, a tab, and then the line number."
              " Use getField() with this.")
{
	S32 lineNum;
   const char *file = object->getCurrentLine(lineNum);
   char* ret = Con::getReturnBuffer(256);
	dSprintf(ret, sizeof(ret), "%s\t%d", file, lineNum);
	return ret;
}

DefineConsoleMethod(DbgFileView, open, bool, (const char * filename), , "(string filename)"
              "Open a file for viewing.\n\n"
              "@note This loads the file from the local system.")
{
   return object->openFile(filename);
}

DefineConsoleMethod(DbgFileView, clearBreakPositions, void, (), , "()"
              "Clear all break points in the current file.")
{
   object->clearBreakPositions();
}

DefineConsoleMethod(DbgFileView, setBreakPosition, void, (U32 line), , "(int line)"
              "Set a breakpoint at the specified line.")
{
   object->setBreakPosition(line);
}

DefineConsoleMethod(DbgFileView, setBreak, void, (U32 line), , "(int line)"
              "Set a breakpoint at the specified line.")
{
   object->setBreakPointStatus(line, true);
}

DefineConsoleMethod(DbgFileView, removeBreak, void, (U32 line), , "(int line)"
              "Remove a breakpoint from the specified line.")
{
   object->setBreakPointStatus(line, false);
}

DefineConsoleMethod(DbgFileView, findString, bool, (const char * findThis), , "(string findThis)"
              "Find the specified string in the currently viewed file and "
              "scroll it into view.")
{
   return object->findString(findThis);
}

//this value is the offset used in the ::onRender() method...
static S32 gFileXOffset = 44;

void DbgFileView::AdjustCellSize()
{
   if (! bool(mFont))
      return;
   S32 maxWidth = 1;
   for (U32 i = 0; i < mFileView.size(); i++)
   {
      S32 cellWidth = gFileXOffset + mFont->getStrWidth((const UTF8 *)mFileView[i].text);
      maxWidth = getMax(maxWidth, cellWidth);
   }

   mCellSize.set(maxWidth, mFont->getHeight() + 2);
   setSize(mSize);
}

bool DbgFileView::onWake()
{
   if (! Parent::onWake())
      return false;

   //clear the mouse over var
   mMouseOverVariable[0] = '\0';
   mbMouseDragging = false;

   //adjust the cellwidth to the maximum line length
   AdjustCellSize();
   mSize.set(1, mFileView.size());

   return true;
}

void DbgFileView::addLine(const char *string, U32 strLen)
{
   // first compute the size
   U32 size = 1; // for null
   for(U32 i = 0; i < strLen; i++)
   {
      if(string[i] == '\t')
         size += 3;
      else if(string[i] != '\r')
         size++;
   }
   FileLine fl;
   fl.breakPosition = false;
   fl.breakOnLine = false;
   if(size)
   {
      fl.text = (char *) dMalloc(size);

      U32 dstIndex = 0;
      for(U32 i = 0; i < strLen; i++)
      {
         if(string[i] == '\t')
         {
            fl.text[dstIndex] = ' ';
            fl.text[dstIndex + 1] = ' ';
            fl.text[dstIndex + 2] = ' ';
            dstIndex += 3;
         }
         else if(string[i] != '\r')
            fl.text[dstIndex++] = string[i];
      }
      fl.text[dstIndex] = 0;
   }
   else
      fl.text = NULL;
   mFileView.push_back(fl);
}

void DbgFileView::clear()
{
   for(Vector<FileLine>::iterator i = mFileView.begin(); i != mFileView.end(); i++)
      dFree(i->text);
   mFileView.clear();
}

bool DbgFileView::findString(const char *text)
{
	//make sure we have a valid string to find
	if (!text || !text[0])
		return false;

	//see which line we start searching from
	S32 curLine = 0;
	bool searchAgain = false;
	if (mFindLineNumber >= 0 && !dStricmp(mFindString, text))
	{
		searchAgain = true;
		curLine = mFindLineNumber;
	}
	else
		mFindLineNumber = -1;

	//copy the search text
	dStrncpy(mFindString, text, 255);
	S32 length = dStrlen(mFindString);

	//loop through looking for the next instance
	while (curLine < mFileView.size())
	{
		char *curText;
		if (curLine == mFindLineNumber && mBlockStart >= 0)
			curText = &mFileView[curLine].text[mBlockStart + 1];
		else
			curText = &mFileView[curLine].text[0];

		//search for the string (the hard way... - apparently dStrupr is broken...
		char *found = NULL;
		char *curTextPtr = curText;
		while (*curTextPtr != '\0')
		{
			if (!dStrnicmp(mFindString, curTextPtr, length))
			{
				found = curTextPtr;
				break;
			}
			else
				curTextPtr++;
		}

		//did we find it?
		if (found)
		{
			//scroll first
			mFindLineNumber = curLine;
	      scrollToLine(mFindLineNumber + 1);

			//then hilite
			mBlockStart = (S32)(found - &mFileView[curLine].text[0]);
			mBlockEnd = mBlockStart + length;

			return true;
		}
		else
			curLine++;
	}

	//didn't find anything - reset the vars for the next search
	mBlockStart = -1;
	mBlockEnd = -1;
	mFindLineNumber = -1;

	setSelectedCell(Point2I(-1, -1));
	return false;
}

void DbgFileView::setCurrentLine(S32 lineNumber, bool setCurrentLine)
{
   //update the line number
   if (setCurrentLine)
   {
      mPCFileName = mFileName;
      mPCCurrentLine = lineNumber;
      mBlockStart = -1;
      mBlockEnd = -1;
		if (lineNumber >= 0)
	      scrollToLine(mPCCurrentLine);
   }
   else
   {
      scrollToLine(lineNumber);
   }
}

const char* DbgFileView::getCurrentLine(S32 &lineNumber)
{
	lineNumber = mPCCurrentLine;
	return mPCFileName;
}

bool DbgFileView::openFile(const char *fileName)
{
   if ((! fileName) || (! fileName[0]))
      return false;

   StringTableEntry newFile = StringTable->insert(fileName);
   if (mFileName == newFile)
      return true;

   void *data = NULL;
   U32 dataSize = 0;
   Torque::FS::ReadFile(fileName, data, dataSize, true);
   if(data == NULL)
   {
      Con::printf("DbgFileView: unable to open file %s.", fileName);
      return false;
   }

   //copy the file name
   mFileName = newFile;

   //clear the old mFileView
   clear();
   setSize(Point2I(1, 0));

   //begin reading and parsing at each '\n'
   char *parsePtr = (char *)data;
   for(;;) {
      char *tempPtr = dStrchr(parsePtr, '\n');
      if(tempPtr)
         addLine(parsePtr, tempPtr - parsePtr);
      else if(parsePtr[0])
         addLine(parsePtr, dStrlen(parsePtr));
      if(!tempPtr)
         break;
      parsePtr = tempPtr + 1;
   }
   //delete the buffer
   delete [] static_cast<char*>(data);

   //set the file size
   AdjustCellSize();
   setSize(Point2I(1, mFileView.size()));

   return true;
}

void DbgFileView::scrollToLine(S32 lineNumber)
{
   GuiControl *parent = getParent();
   if (! parent)
      return;

   S32 yOffset = (lineNumber - 1) * mCellSize.y;

   //see if the line is already visible
   if (! (yOffset + getPosition().y >= 0 && yOffset + getPosition().y < parent->getHeight() - mCellSize.y))
   {
      //reposition the control
      S32 newYOffset = getMin(0, getMax(parent->getHeight() - getHeight(), (mCellSize.y * 4) - yOffset));
      setPosition(Point2I(getPosition().x, newYOffset));
   }

   //hilite the line
   cellSelected(Point2I(0, lineNumber - 1));
}

S32 DbgFileView::findMouseOverChar(const char *text, S32 stringPosition)
{
   static char tempBuf[512];
   char *bufPtr = &tempBuf[1];

   // Handle the empty string correctly.
   if (text[0] == '\0') {
      return -1;
   }

   // Copy the line's text into the scratch buffer.
   dStrncpy(tempBuf, text, 512);

   // Make sure we have a null terminator.
   tempBuf[511] = '\0';

   // Loop over the characters...
   bool found = false;
   bool finished = false;
   do {
      // Note the current character, then replace it with EOL.
      char c = *bufPtr;
      *bufPtr = '\0';
      // Is the resulting string long enough to include the current
      // mouse position?
      if ((S32)mFont->getStrWidth((const UTF8 *)tempBuf) > stringPosition) {
         // Yep.
         // Restore the character.
         *bufPtr = c;
         // Set bufPtr to point to the char under the mouse.
         bufPtr--;
         // Bail.
         found = true;
         finished = true;
      }
      else {
         // Nope.
         // Restore the character.
         *bufPtr = c;
         // Move on to the next character.
         bufPtr++;
         // Bail if EOL.
         if (*bufPtr == '\0') finished = true;
      }
   } while (!finished);

   // Did we find a character under the mouse?
   if (found) {
      // If so, return its position.
      return bufPtr - tempBuf;
   }
   // If not, return -1.
   else return -1;
}

bool DbgFileView::findMouseOverVariable()
{
   GuiCanvas *root = getRoot();
   AssertFatal(root, "Unable to get the root Canvas.");

   Point2I curMouse = root->getCursorPos();
   Point2I pt = globalToLocalCoord(curMouse);

   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      S32 stringPosition = pt.x - gFileXOffset;
      char tempBuf[256], *varNamePtr = &tempBuf[1];
      dStrcpy(tempBuf, mFileView[cell.y].text);

      //find the current mouse over char
      S32 charNum = findMouseOverChar(mFileView[cell.y].text, stringPosition);
      if (charNum >= 0)
      {
         varNamePtr = &tempBuf[charNum];
      }
      else
      {
         mMouseOverVariable[0] = '\0';
         mMouseOverValue[0] = '\0';
         return false;
      }

      //now make sure we can go from the current cursor mPosition to the beginning of a var name
      bool found = false;
      while (varNamePtr >= &tempBuf[0])
      {
         if (*varNamePtr == '%' || *varNamePtr == '$')
         {
            found = true;
            break;
         }
         else if ((dToupper(*varNamePtr) >= 'A' && dToupper(*varNamePtr) <= 'Z') ||
                  (*varNamePtr >= '0' && *varNamePtr <= '9') || *varNamePtr == '_' || *varNamePtr == ':')
         {
            varNamePtr--;
         }
         else
         {
            break;
         }
      }

      //mouse wasn't over a possible variable name
      if (! found)
      {
         mMouseOverVariable[0] = '\0';
         mMouseOverValue[0] = '\0';
         return false;
      }

      //set the var char start positions
      mMouseVarStart = varNamePtr - tempBuf;

      //now copy the (possible) var name into the buf
      char *tempPtr = &mMouseOverVariable[0];

      //copy the leading '%' or '$'
      *tempPtr++ = *varNamePtr++;

      //now copy letters and numbers until the end of the name
      while ((dToupper(*varNamePtr) >= 'A' && dToupper(*varNamePtr) <= 'Z') ||
                  (*varNamePtr >= '0' && *varNamePtr <= '9') || *varNamePtr == '_' || *varNamePtr == ':')
      {
         *tempPtr++ = *varNamePtr++;
      }
      *tempPtr = '\0';

      //set the var char end positions
      mMouseVarEnd = varNamePtr - tempBuf;

      return true;
   }
   return false;
}

void DbgFileView::clearBreakPositions()
{
   for(Vector<FileLine>::iterator i = mFileView.begin(); i != mFileView.end(); i++)
   {
      i->breakPosition = false;
      i->breakOnLine = false;
   }
}

void DbgFileView::setBreakPosition(U32 line)
{
   if(line > mFileView.size())
      return;
   mFileView[line-1].breakPosition = true;
}

void DbgFileView::setBreakPointStatus(U32 line, bool set)
{
   if(line > mFileView.size())
      return;
   mFileView[line-1].breakOnLine = set;
}

void DbgFileView::onPreRender()
{
	setUpdate();
   char oldVar[256];
   dStrcpy(oldVar, mMouseOverVariable);
   bool found = findMouseOverVariable();
   if (found && mPCCurrentLine >= 0)
   {
      //send the query only when the var changes
      if (dStricmp(oldVar, mMouseOverVariable))
			Con::executef("DbgSetCursorWatch", mMouseOverVariable);
   }
	else
		Con::executef("DbgSetCursorWatch", "");
}

void DbgFileView::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseDown(event);
      return;
   }

   Point2I pt = globalToLocalCoord(event.mousePoint);
   bool doubleClick = (event.mouseClickCount > 1);

   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      //if we clicked on the breakpoint mark
      if (pt.x >= 0 && pt.x <= 12)
      {
         //toggle the break point
         if (mFileView[cell.y].breakPosition)
         {
            if (mFileView[cell.y].breakOnLine)
               Con::executef(this, "onRemoveBreakPoint", itoa(cell.y + 1));
            else
               Con::executef(this, "onSetBreakPoint", itoa(cell.y + 1));
         }
      }
      else
      {
         Point2I prevSelected = mSelectedCell;
         Parent::onMouseDown(event);
         mBlockStart= -1;
         mBlockEnd = -1;

         //open the file view
         if (mSelectedCell.y == prevSelected.y && doubleClick && mMouseOverVariable[0])
         {
            Con::executef(this, "onSetWatch", mMouseOverVariable);
            mBlockStart = mMouseVarStart;
            mBlockEnd = mMouseVarEnd;
         }
         else
         {
            S32 stringPosition = pt.x - gFileXOffset;

            //find which character we're over
            S32 charNum = findMouseOverChar(mFileView[mSelectedCell.y].text, stringPosition);
            if (charNum >= 0)
            {
               //lock the mouse
               mouseLock();
               setFirstResponder();

               //set the block hilite start and end
               mbMouseDragging = true;
               mMouseDownChar = charNum;
            }
         }
      }
   }
   else
   {
      Parent::onMouseDown(event);
   }
}

void DbgFileView::onMouseDragged(const GuiEvent &event)
{
   if (mbMouseDragging)
   {
      Point2I pt = globalToLocalCoord(event.mousePoint);
      S32 stringPosition = pt.x - gFileXOffset;

      //find which character we're over
      S32 charNum = findMouseOverChar(mFileView[mSelectedCell.y].text, stringPosition);
      if (charNum >= 0)
      {
         if (charNum < mMouseDownChar)
         {

            mBlockEnd = mMouseDownChar + 1;
            mBlockStart = charNum;
         }
         else
         {
            mBlockEnd = charNum + 1;
            mBlockStart = mMouseDownChar;
         }
      }

      //otherwise, the cursor is past the end of the string
      else
      {
         mBlockStart = mMouseDownChar;
         mBlockEnd = dStrlen(mFileView[mSelectedCell.y].text) + 1;
      }
   }
}

void DbgFileView::onMouseUp(const GuiEvent &)
{
   //unlock the mouse
   mouseUnlock();

   mbMouseDragging = false;
}

void DbgFileView::onRenderCell(Point2I offset, Point2I cell, bool selected, bool)
{
   Point2I cellOffset = offset;
   cellOffset.x += 4;

   //draw the break point marks
   if (mFileView[cell.y].breakOnLine)
   {
      GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorHL);
      GFX->getDrawUtil()->drawText(mFont, cellOffset, "#");
   }
   else if (mFileView[cell.y].breakPosition)
   {
      GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor);
      GFX->getDrawUtil()->drawText(mFont, cellOffset, "-");
   }
   cellOffset.x += 8;

   //draw in the "current line" indicator
   if (mFileName == mPCFileName && (cell.y + 1 == mPCCurrentLine))
   {
      GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorHL);
      GFX->getDrawUtil()->drawText(mFont, cellOffset, "=>");
   }

	//by this time, the cellOffset has been incremented by 44 - the value of gFileXOffset
   cellOffset.x += 32;

   //hilite the line if selected
   if (selected)
   {
      if (mBlockStart == -1)
      {
         GFX->getDrawUtil()->drawRectFill(RectI(cellOffset.x - 2, cellOffset.y - 3,
                                 mCellSize.x + 4, mCellSize.y + 6), mProfile->mFillColorHL);
      }
      else if (mBlockStart >= 0 && mBlockEnd > mBlockStart && mBlockEnd <= S32(dStrlen(mFileView[cell.y].text) + 1))
      {
         S32 startPos, endPos;
         char tempBuf[256];
         dStrcpy(tempBuf, mFileView[cell.y].text);

         //get the end coord
         tempBuf[mBlockEnd] = '\0';
         endPos = mFont->getStrWidth((const UTF8 *)tempBuf);

         //get the start coord
         tempBuf[mBlockStart] = '\0';
         startPos = mFont->getStrWidth((const UTF8 *)tempBuf);

         //draw the hilite
         GFX->getDrawUtil()->drawRectFill(RectI(cellOffset.x + startPos, cellOffset.y - 3, endPos - startPos + 2, mCellSize.y + 6), mProfile->mFillColorHL);
      }
   }

   //draw the line of text
   GFX->getDrawUtil()->setBitmapModulation(mFileView[cell.y].breakOnLine ? mProfile->mFontColorHL : mProfile->mFontColor);
   GFX->getDrawUtil()->drawText(mFont, cellOffset, mFileView[cell.y].text);
}
