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

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include "gui/controls/guiConsoleEditCtrl.h"
#include "core/frameAllocator.h"

IMPLEMENT_CONOBJECT(GuiConsoleEditCtrl);

ConsoleDocClass( GuiConsoleEditCtrl,
   "@brief Text entry element of a GuiConsole.\n\n"
   "@tsexample\n"
   "new GuiConsoleEditCtrl(ConsoleEntry)\n"
   "{\n"
   "   profile = \"ConsoleTextEditProfile\";\n"
   "   horizSizing = \"width\";\n"
   "   vertSizing = \"top\";\n"
   "   position = \"0 462\";\n"
   "   extent = \"640 18\";\n"
   "   minExtent = \"8 8\";\n"
   "   visible = \"1\";\n"
   "   altCommand = \"ConsoleEntry::eval();\";\n"
   "   helpTag = \"0\";\n"
   "   maxLength = \"255\";\n"
   "   historySize = \"40\";\n"
   "   password = \"0\";\n"
   "   tabComplete = \"0\";\n"
   "   sinkAllKeyEvents = \"1\";\n"
   "   useSiblingScroller = \"1\";\n"
   "};\n"
   "@endtsexample\n\n"
   "@ingroup GuiCore"
);

GuiConsoleEditCtrl::GuiConsoleEditCtrl()
{
   mSinkAllKeyEvents = true;
   mSiblingScroller = NULL;
   mUseSiblingScroller = true;
}

void GuiConsoleEditCtrl::initPersistFields()
{
   addGroup("GuiConsoleEditCtrl");
   addField("useSiblingScroller", TypeBool, Offset(mUseSiblingScroller, GuiConsoleEditCtrl));
   endGroup("GuiConsoleEditCtrl");

   Parent::initPersistFields();
}

bool GuiConsoleEditCtrl::onKeyDown(const GuiEvent &event)
{
   setUpdate();

   if (event.keyCode == KEY_TAB) 
   {
      // Get a buffer that can hold the completed text...
      FrameTemp<UTF8> tmpBuff(GuiTextCtrl::MAX_STRING_LENGTH);
      // And copy the text to be completed into it.
      mTextBuffer.getCopy8(tmpBuff, GuiTextCtrl::MAX_STRING_LENGTH);

      // perform the completion
      bool forward = (event.modifier & SI_SHIFT) == 0;
      mCursorPos = Con::tabComplete(tmpBuff, mCursorPos, GuiTextCtrl::MAX_STRING_LENGTH, forward);

      // place results in our buffer.
      mTextBuffer.set(tmpBuff);
      return true;
   }
   else if ((event.keyCode == KEY_PAGE_UP) || (event.keyCode == KEY_PAGE_DOWN)) 
   {
      // See if there's some other widget that can scroll the console history.
      if (mUseSiblingScroller) 
      {
         if (mSiblingScroller) 
         {
            return mSiblingScroller->onKeyDown(event);
         }
         else 
         {
            // Let's see if we can find it...
            SimGroup* pGroup = getGroup();
            if (pGroup) 
            {
               // Find the first scroll control in the same group as us.
               for (SimSetIterator itr(pGroup); *itr; ++itr) 
               {
                  mSiblingScroller = dynamic_cast<GuiScrollCtrl*>(*itr);
                  if (mSiblingScroller != NULL)
                  {
                     return mSiblingScroller->onKeyDown(event);
                  }
               }
            }

            // No luck... so don't try, next time.
            mUseSiblingScroller = false;
         }
      }
   }
	else if( event.keyCode == KEY_RETURN || event.keyCode == KEY_NUMPADENTER )
	{
      if ( event.modifier & SI_SHIFT &&
           mTextBuffer.length() + dStrlen("echo();") <= GuiTextCtrl::MAX_STRING_LENGTH )
      {
         // Wrap the text with echo( %s );

         char buf[GuiTextCtrl::MAX_STRING_LENGTH];
         getText( buf );

         String text( buf );         
         text.replace( ";", "" );

         text = String::ToString( "echo(%s);", text.c_str() );

         setText( text );
      }

		return Parent::dealWithEnter(false);
	}

   return Parent::onKeyDown(event);
}

