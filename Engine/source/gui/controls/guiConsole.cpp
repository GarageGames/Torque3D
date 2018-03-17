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

#include "console/console.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiTypes.h"
#include "gui/core/guiControl.h"
#include "gui/controls/guiConsole.h"
#include "gui/containers/guiScrollCtrl.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiConsole);

ConsoleDocClass( GuiConsole,
   "@brief The on-screen, in-game console. Calls getLog() to get the on-screen console entries, then renders them as needed.\n\n"

   "@tsexample\n"
   "	new GuiConsole()\n"
   "		{\n"
   "			//Properties not specific to this control have been omitted from this example.\n"
   "		};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n\n"

   "@ingroup GuiCore"
);

IMPLEMENT_CALLBACK( GuiConsole, onMessageSelected, void, ( ConsoleLogEntry::Level level, const char* message ), ( level, message ),
   "Called when a message in the log is clicked.\n\n"
   "@param level Diagnostic level of the message.\n"
   "@param message Message text.\n" );

IMPLEMENT_CALLBACK(GuiConsole, onNewMessage, void, (U32 errorCount, U32 warnCount, U32 normalCount), (errorCount, warnCount, normalCount),
   "Called when a new message is logged.\n\n"
   "@param errorCount The number of error messages logged.\n"
   "@param warnCount The number of warning messages logged.\n"
   "@param normalCount The number of normal messages logged.\n");


//-----------------------------------------------------------------------------

GuiConsole::GuiConsole()
{
   setExtent(64, 64);
   mCellSize.set(1, 1);
   mSize.set(1, 0);

   mDisplayErrors = true;
   mDisplayWarnings = true;
   mDisplayNormalMessages = true;
   mFiltersDirty = true;
}

//-----------------------------------------------------------------------------

bool GuiConsole::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the font
   mFont = mProfile->mFont;

   return true;
}

//-----------------------------------------------------------------------------

S32 GuiConsole::getMaxWidth(S32 startIndex, S32 endIndex)
{
   //sanity check
   U32 size;
   ConsoleLogEntry *log;

   if (startIndex < 0 || (U32)endIndex >= mFilteredLog.size() || startIndex > endIndex)
      return 0;

   S32 result = 0;
   for(S32 i = startIndex; i <= endIndex; i++)
      result = getMax(result, (S32)(mFont->getStrWidth((const UTF8 *)mFilteredLog[i].mString)));
   
   return(result + 6);
}

void GuiConsole::refreshLogText()
{
   U32 size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);

   if (mFilteredLog.size() != size || mFiltersDirty)
   {
      mFilteredLog.clear();

      U32 errorCount = 0;
      U32 warnCount = 0;
      U32 normalCount = 0;

      //Filter the log if needed
      for (U32 i = 0; i < size; ++i)
      {
         ConsoleLogEntry &entry = log[i];

         if (entry.mLevel == ConsoleLogEntry::Error)
         {
            errorCount++;
            if (mDisplayErrors)
            {
               mFilteredLog.push_back(entry);
            }
         }
         else if (entry.mLevel == ConsoleLogEntry::Warning)
         {
            warnCount++;
            if (mDisplayWarnings)
            {
               mFilteredLog.push_back(entry);
            }
         }
         else if (entry.mLevel == ConsoleLogEntry::Normal)
         {
            normalCount++;
            if (mDisplayNormalMessages)
            {
               mFilteredLog.push_back(entry);
            }
         }
      }

      onNewMessage_callback(errorCount, warnCount, normalCount);
   }

   Con::unlockLog();
}

//-----------------------------------------------------------------------------

void GuiConsole::onPreRender()
{
   //see if the size has changed
   U32 prevSize = getHeight() / mCellSize.y;

   refreshLogText();
   
   //first, find out if the console was scrolled up
   bool scrolled = false;
   GuiScrollCtrl *parent = dynamic_cast<GuiScrollCtrl*>(getParent());

   if(parent)
      scrolled = parent->isScrolledToBottom();

   //find the max cell width for the new entries
   S32 newMax = getMaxWidth(prevSize, mFilteredLog.size() - 1);
   if(newMax > mCellSize.x)
      mCellSize.set(newMax, mFont->getHeight());

   //set the array size
   mSize.set(1, mFilteredLog.size());

   //resize the control
   setExtent(Point2I(mCellSize.x, mCellSize.y * mFilteredLog.size()));

   //if the console was not scrolled, make the last entry visible
   if (scrolled)
      scrollCellVisible(Point2I(0,mSize.y - 1));
}

//-----------------------------------------------------------------------------

void GuiConsole::onRenderCell(Point2I offset, Point2I cell, bool /*selected*/, bool /*mouseOver*/)
{
   U32 size;
   ConsoleLogEntry *log;
   
   ConsoleLogEntry &entry = mFilteredLog[cell.y];
   switch (entry.mLevel)
   {
      case ConsoleLogEntry::Normal:   GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor); break;
      case ConsoleLogEntry::Warning:  GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorHL); break;
      case ConsoleLogEntry::Error:    GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorNA); break;
      default: AssertFatal(false, "GuiConsole::onRenderCell - Unrecognized ConsoleLogEntry type, update this.");
   }
   GFX->getDrawUtil()->drawText(mFont, Point2I(offset.x + 3, offset.y), entry.mString, mProfile->mFontColors);
}

//-----------------------------------------------------------------------------

void GuiConsole::onCellSelected( Point2I cell )
{
   Parent::onCellSelected( cell );

   U32 size;
   ConsoleLogEntry* log;

   ConsoleLogEntry& entry = mFilteredLog[cell.y];
   onMessageSelected_callback( entry.mLevel, entry.mString );
}

void GuiConsole::setDisplayFilters(bool errors, bool warns, bool normal)
{
   mDisplayErrors = errors;
   mDisplayWarnings = warns;
   mDisplayNormalMessages = normal;
   mFiltersDirty = true;

   refreshLogText();

   //find the max cell width for the new entries
   S32 newMax = getMaxWidth(0, mFilteredLog.size() - 1);
   mCellSize.set(newMax, mFont->getHeight());

   //set the array size
   mSize.set(1, mFilteredLog.size());

   //resize the control
   setExtent(Point2I(mCellSize.x, mCellSize.y * mFilteredLog.size()));

   scrollCellVisible(Point2I(0, mSize.y - 1));

   mFiltersDirty = false;
}

DefineEngineMethod(GuiConsole, setDisplayFilters, void, (bool errors, bool warns, bool normal), (true, true, true),
   "Sets the current display filters for the console gui. Allows you to indicate if it should display errors, warns and/or normal messages.\n\n"
   "@param errors If true, the console gui will display any error messages that were emitted.\n\n"
   "@param warns If true, the console gui will display any warning messages that were emitted.\n\n"
   "@param normal If true, the console gui will display any regular messages that were emitted.\n\n")
{
   object->setDisplayFilters(errors, warns, normal);
}

DefineEngineMethod(GuiConsole, getErrorFilter, bool, (), ,
   "Returns if the error filter is on or not.")
{
   return object->getErrorFilter();
}

DefineEngineMethod(GuiConsole, getWarnFilter, bool, (), ,
   "Returns if the warning filter is on or not.")
{
   return object->getWarnFilter();
}

DefineEngineMethod(GuiConsole, getNormalFilter, bool, (), ,
   "Returns if the normal message filter is on or not.")
{
   return object->getNormalFilter();
}

DefineEngineMethod(GuiConsole, toggleErrorFilter, void, (), ,
   "Toggles the error filter.")
{
   object->toggleErrorFilter();
}

DefineEngineMethod(GuiConsole, toggleWarnFilter, void, (), ,
   "Toggles the warning filter.")
{
   object->toggleWarnFilter();
}

DefineEngineMethod(GuiConsole, toggleNormalFilter, void, (), ,
   "Toggles the normal messages filter.")
{
   object->toggleNormalFilter();
}

DefineEngineMethod(GuiConsole, refresh, void, (), ,
   "Refreshes the displayed messages.")
{
   object->refresh();
}