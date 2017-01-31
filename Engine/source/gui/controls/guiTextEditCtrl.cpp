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
#include "gui/controls/guiTextEditCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include "gui/controls/guiMLTextCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "core/frameAllocator.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxSystem.h"
#include "core/strings/unicode.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiTextEditCtrl);

ConsoleDocClass( GuiTextEditCtrl,
   "@brief A component that places a text entry box on the screen.\n\n"

   "Fonts and sizes are changed using profiles. The text value can be set or entered by a user.\n\n"

   "@tsexample\n"
   "   new GuiTextEditCtrl(MessageHud_Edit)\n"
   "  {\n"
   "      text = \"Hello World\";\n"
   "      validate = \"validateCommand();\"\n"
   "      escapeCommand = \"escapeCommand();\";\n"
   "      historySize = \"5\";\n"
   "      tabComplete = \"true\";\n"
   "      deniedSound = \"DeniedSoundProfile\";\n"
   "      sinkAllKeyEvents = \"true\";\n"
   "      password = \"true\";\n"
   "      passwordMask = \"*\";\n"
   "       //Properties not specific to this control have been omitted from this example.\n"
   "   };\n"
   "@endtsexample\n\n"

   "@see GuiTextCtrl\n"
   "@see GuiControl\n\n"

   "@ingroup GuiControls\n"
);

IMPLEMENT_CALLBACK( GuiTextEditCtrl, onTabComplete, void, (const char* val),( val ),
   "@brief Called if tabComplete is true, and the 'tab' key is pressed.\n\n"
   "@param val Input to mimick the '1' sent by the actual tab key button press.\n"
   "@tsexample\n"
   "// Tab key has been pressed, causing the callback to occur.\n"
   "GuiTextEditCtrl::onTabComplete(%this,%val)\n"
   "  {\n"
   "     //Code to run when the onTabComplete callback occurs\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiTextCtrl\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiTextEditCtrl, onReturn, void, (),(),
   "@brief Called when the 'Return' or 'Enter' key is pressed.\n\n"
   "@tsexample\n"
   "// Return or Enter key was pressed, causing the callback to occur.\n"
   "GuiTextEditCtrl::onReturn(%this)\n"
   "  {\n"
   "     // Code to run when the onReturn callback occurs\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiTextCtrl\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiTextEditCtrl, onValidate, void, (),(),
   "@brief Called whenever the control is validated.\n\n"
   "@tsexample\n"
   "// The control gets validated, causing the callback to occur\n"
   "GuiTextEditCtrl::onValidated(%this)\n"
   "  {\n"
   "     // Code to run when the control is validated\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiTextCtrl\n"
   "@see GuiControl\n\n"
);

GuiTextEditCtrl::GuiTextEditCtrl()
{
   mInsertOn = true;
   mBlockStart = 0;
   mBlockEnd = 0;
   mCursorPos = 0;
   mCursorOn = false;
   mNumFramesElapsed = 0;

   mDragHit = false;
   mTabComplete = false;
   mScrollDir = 0;

   mUndoBlockStart = 0;
   mUndoBlockEnd = 0;
   mUndoCursorPos = 0;
   mPasswordText = false;

   mSinkAllKeyEvents = false;

   mActive = true;

   mTextValid = true;

   mTextOffsetReset = true;

   mHistoryDirty = false;
   mHistorySize = 0;
   mHistoryLast = -1;
   mHistoryIndex = 0;
   mHistoryBuf = NULL;

#if defined(__MACOSX__)
   UTF8  bullet[4] = { 0xE2, 0x80, 0xA2, 0 };
   
   mPasswordMask = StringTable->insert( bullet );
#else
   mPasswordMask = StringTable->insert( "*" );
#endif
   Sim::findObject( "InputDeniedSound", mDeniedSound );
}

GuiTextEditCtrl::~GuiTextEditCtrl()
{
   //delete the history buffer if it exists
   if (mHistoryBuf)
   {
      for (S32 i = 0; i < mHistorySize; i++)
         delete [] mHistoryBuf[i];

      delete [] mHistoryBuf;
   }
}

void GuiTextEditCtrl::initPersistFields()
{
   addGroup( "Text Input" );
   
      addField("validate",          TypeRealString,Offset(mValidateCommand,   GuiTextEditCtrl), "Script command to be called when the first validater is lost.\n");
      addField("escapeCommand",     TypeRealString,Offset(mEscapeCommand,     GuiTextEditCtrl), "Script command to be called when the Escape key is pressed.\n");
      addField("historySize",       TypeS32,       Offset(mHistorySize,       GuiTextEditCtrl), "How large of a history buffer to maintain.\n");
      addField("tabComplete",       TypeBool,      Offset(mTabComplete,       GuiTextEditCtrl), "If true, when the 'tab' key is pressed, it will act as if the Enter key was pressed on the control.\n");
      addField("deniedSound",       TypeSFXTrackName, Offset(mDeniedSound, GuiTextEditCtrl), "If the attempted text cannot be entered, this sound effect will be played.\n");
      addField("sinkAllKeyEvents",  TypeBool,      Offset(mSinkAllKeyEvents,  GuiTextEditCtrl), "If true, every key event will act as if the Enter key was pressed.\n");
      addField("password",          TypeBool,      Offset(mPasswordText,      GuiTextEditCtrl), "If true, all characters entered will be stored in the control, however will display as the character stored in passwordMask.\n");
      addField("passwordMask",      TypeString,    Offset(mPasswordMask,      GuiTextEditCtrl), "If 'password' is true, this is the character that will be used to mask the characters in the control.\n");
      
   endGroup( "Text Input" );

   Parent::initPersistFields();
}

bool GuiTextEditCtrl::onAdd()
{
   if ( ! Parent::onAdd() )
      return false;

   //create the history buffer
   if ( mHistorySize > 0 )
   {
      mHistoryBuf = new UTF16*[mHistorySize];
      for ( S32 i = 0; i < mHistorySize; i++ )
      {
         mHistoryBuf[i] = new UTF16[GuiTextCtrl::MAX_STRING_LENGTH + 1];
         mHistoryBuf[i][0] = '\0';
      }
   }

   if( mText && mText[0] )
   {
      setText(mText);
   }

   return true;
}

void GuiTextEditCtrl::onStaticModified(const char* slotName, const char* newValue)
{
   if(!dStricmp(slotName, "text"))
      setText(mText);
}

void GuiTextEditCtrl::execConsoleCallback()
{
   // Execute the console command!
   Parent::execConsoleCallback();

   // Update the console variable:
   if ( mConsoleVariable[0] )
      Con::setVariable(mConsoleVariable, mTextBuffer.getPtr8());
}

void GuiTextEditCtrl::updateHistory( StringBuffer *inTxt, bool moveIndex )
{
   if(!mHistorySize)
      return;

   const UTF16* txt = inTxt->getPtr();
   
   // Reject empty strings.

   if( !txt || !txt[ 0 ] )
      return;

   // see if it's already in
   if(mHistoryLast == -1 || dStrcmp(txt, mHistoryBuf[mHistoryLast]))
   {
      if(mHistoryLast == mHistorySize-1) // we're at the history limit... shuffle the pointers around:
      {
         UTF16 *first = mHistoryBuf[0];
         for(U32 i = 0; i < mHistorySize - 1; i++)
            mHistoryBuf[i] = mHistoryBuf[i+1];
         mHistoryBuf[mHistorySize-1] = first;
         if(mHistoryIndex > 0)
            mHistoryIndex--;
      }
      else
         mHistoryLast++;

      inTxt->getCopy(mHistoryBuf[mHistoryLast], GuiTextCtrl::MAX_STRING_LENGTH);
      mHistoryBuf[mHistoryLast][GuiTextCtrl::MAX_STRING_LENGTH] = '\0';
   }

   if(moveIndex)
      mHistoryIndex = mHistoryLast + 1;
}

void GuiTextEditCtrl::getText( char *dest )
{
   if ( dest )
      mTextBuffer.getCopy8((UTF8*)dest, GuiTextCtrl::MAX_STRING_LENGTH+1);
}

void GuiTextEditCtrl::getRenderText(char *dest)
{
    getText( dest );
}
 
void GuiTextEditCtrl::setText( const UTF8 *txt )
{
   if(txt && txt[0] != 0)
   {
      Parent::setText(txt);
      mTextBuffer.set( txt );
   }
   else
      mTextBuffer.set( "" );

   mCursorPos = mTextBuffer.length();
}

void GuiTextEditCtrl::setText( const UTF16* txt)
{
   if(txt && txt[0] != 0)
   {
      UTF8* txt8 = createUTF8string( txt );
      Parent::setText( txt8 );
      delete[] txt8;
      mTextBuffer.set( txt );
   }
   else
   {
      Parent::setText("");
      mTextBuffer.set("");
   }
   
   mCursorPos = mTextBuffer.length();   
}

bool GuiTextEditCtrl::isAllTextSelected()
{
   if( mBlockStart == 0 && mBlockEnd == mTextBuffer.length() )
      return true;
   else
      return false;
}

void GuiTextEditCtrl::selectAllText()
{
   mBlockStart = 0;
   mBlockEnd = mTextBuffer.length();
   setUpdate();
}

void GuiTextEditCtrl::clearSelectedText()
{
   mBlockStart = 0;
   mBlockEnd = 0;
   setUpdate();
}

void GuiTextEditCtrl::forceValidateText()
{
   if( mValidateCommand.isNotEmpty() )
      evaluate( mValidateCommand );
}

void GuiTextEditCtrl::setCursorPos( const S32 newPos )
{
   S32 charCount = mTextBuffer.length();
   S32 realPos = newPos > charCount ? charCount : newPos < 0 ? 0 : newPos;
   if ( realPos != mCursorPos )
   {
      mCursorPos = realPos;
      setUpdate();
   }
}

S32 GuiTextEditCtrl::calculateCursorPos( const Point2I &globalPos )
{
   Point2I ctrlOffset = localToGlobalCoord( Point2I( 0, 0 ) );
   S32 charLength = 0;
   S32 curX;

   curX = globalPos.x - ctrlOffset.x;
   setUpdate();

   //if the cursor is too far to the left
   if ( curX < 0 )
      return -1;

   //if the cursor is too far to the right
   if ( curX >= ctrlOffset.x + getExtent().x )
      return -2;

   curX = globalPos.x - mTextOffset.x;
   S32 count=0;
   if(mTextBuffer.length() == 0)
      return 0;

   for(count=0; count<mTextBuffer.length(); count++)
   {
      UTF16 c = mTextBuffer.getChar(count);
      if(!mPasswordText && !mProfile->mFont->isValidChar(c))
         continue;
         
      if(mPasswordText)
         charLength += mProfile->mFont->getCharXIncrement( mPasswordMask[0] );
      else
         charLength += mProfile->mFont->getCharXIncrement( c );

      if ( charLength > curX )
         break;      
   }

   return count;
}

void GuiTextEditCtrl::onMouseDown( const GuiEvent &event )
{
   if(!isActive())
      return;
   mDragHit = false;

   // If we have a double click, select all text.  Otherwise
   // act as before by clearing any selection.
   bool doubleClick = (event.mouseClickCount > 1);
   if(doubleClick)
   {
      selectAllText();

   } else
   {
      //undo any block function
      mBlockStart = 0;
      mBlockEnd = 0;
   }

   //find out where the cursor should be
   S32 pos = calculateCursorPos( event.mousePoint );

   // if the position is to the left
   if ( pos == -1 )
      mCursorPos = 0;
   else if ( pos == -2 ) //else if the position is to the right
      mCursorPos = mTextBuffer.length();
   else //else set the mCursorPos
      mCursorPos = pos;

   //save the mouseDragPos
   mMouseDragStart = mCursorPos;

   // lock the mouse
   mouseLock();

   //set the drag var
   mDragHit = true;

   //let the parent get the event
   setFirstResponder();
}

void GuiTextEditCtrl::onMouseDragged( const GuiEvent &event )
{
   S32 pos = calculateCursorPos( event.mousePoint );

   // if the position is to the left
   if ( pos == -1 )
      mScrollDir = -1;
   else if ( pos == -2 ) // the position is to the right
      mScrollDir = 1;
   else // set the new cursor position
   {
      mScrollDir = 0;
      mCursorPos = pos;
   }

   // update the block:
   mBlockStart = getMin( mCursorPos, mMouseDragStart );
   mBlockEnd = getMax( mCursorPos, mMouseDragStart );
   if ( mBlockStart < 0 )
      mBlockStart = 0;

   if ( mBlockStart == mBlockEnd )
      mBlockStart = mBlockEnd = 0;

   //let the parent get the event
   Parent::onMouseDragged(event);
}

void GuiTextEditCtrl::onMouseUp(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
   mDragHit = false;
   mScrollDir = 0;
   mouseUnlock();
}

void GuiTextEditCtrl::saveUndoState()
{
   //save the current state
   mUndoText.set(&mTextBuffer);
   mUndoBlockStart = mBlockStart;
   mUndoBlockEnd   = mBlockEnd;
   mUndoCursorPos  = mCursorPos;
}

void GuiTextEditCtrl::onCopy(bool andCut)
{
   // Don't copy/cut password field!
   if(mPasswordText)
      return;

   if (mBlockEnd > 0)
   {
      //save the current state
      saveUndoState();

      //copy the text to the clipboard
      UTF8* clipBuff = mTextBuffer.createSubstring8(mBlockStart, mBlockEnd - mBlockStart);
      Platform::setClipboard(clipBuff);
      delete[] clipBuff;

      //if we pressed the cut shortcut, we need to cut the selected text from the control...
      if (andCut)
      {
         mTextBuffer.cut(mBlockStart, mBlockEnd - mBlockStart);
         mCursorPos = mBlockStart;
      }

      mBlockStart = 0;
      mBlockEnd = 0;
   }

}

void GuiTextEditCtrl::onPaste()
{           
   //first, make sure there's something in the clipboard to copy...
   const UTF8 *clipboard = Platform::getClipboard();
   if(dStrlen(clipboard) <= 0)
      return;

   //save the current state
   saveUndoState();

   //delete anything hilited
   if (mBlockEnd > 0)
   {
      mTextBuffer.cut(mBlockStart, mBlockEnd - mBlockStart);
      mCursorPos = mBlockStart;
      mBlockStart = 0;
      mBlockEnd = 0;
   }

   // We'll be converting to UTF16, and maybe trimming the string,
   // so let's use a StringBuffer, for convinience.
   StringBuffer pasteText(clipboard);

   // Space left after we remove the highlighted text
   S32 stringLen = mTextBuffer.length();

   // Trim down to fit in a buffer of size mMaxStrLen
   S32 pasteLen = pasteText.length();

   if(stringLen + pasteLen > mMaxStrLen)
   {
      pasteLen = mMaxStrLen - stringLen;

      pasteText.cut(pasteLen, pasteText.length() - pasteLen);
   }

   if (mCursorPos == stringLen)
   {
      mTextBuffer.append(pasteText);
   }
   else
   {
      mTextBuffer.insert(mCursorPos, pasteText);
   }

   mCursorPos += pasteLen;
}

void GuiTextEditCtrl::onUndo()
{
    StringBuffer tempBuffer;
    S32 tempBlockStart;
    S32 tempBlockEnd;
    S32 tempCursorPos;

    //save the current
    tempBuffer.set(&mTextBuffer);
    tempBlockStart = mBlockStart;
    tempBlockEnd   = mBlockEnd;
    tempCursorPos  = mCursorPos;

    //restore the prev
    mTextBuffer.set(&mUndoText);
    mBlockStart = mUndoBlockStart;
    mBlockEnd   = mUndoBlockEnd;
    mCursorPos  = mUndoCursorPos;

    //update the undo
    mUndoText.set(&tempBuffer);
    mUndoBlockStart = tempBlockStart;
    mUndoBlockEnd   = tempBlockEnd;
    mUndoCursorPos  = tempCursorPos;
}

bool GuiTextEditCtrl::onKeyDown(const GuiEvent &event)
{
   if ( !isActive() || !isAwake() )
      return false;

   S32 stringLen = mTextBuffer.length();
   setUpdate();

   // Ugly, but now I'm cool like MarkF.
   if(event.keyCode == KEY_BACKSPACE)
      goto dealWithBackspace;
   
   if ( event.modifier & SI_SHIFT )
   {

      // Added support for word jump selection.

      if ( event.modifier & SI_CTRL )
      {
         switch ( event.keyCode )
         {
            case KEY_LEFT:
            {
               S32 newpos = findPrevWord();               

               if ( mBlockStart == mBlockEnd )
               {
                  // There was not already a selection so start a new one.
                  mBlockStart = newpos;
                  mBlockEnd = mCursorPos;
               }
               else
               {
                  // There was a selection already...
                  // In this case the cursor MUST be at either the
                  // start or end of that selection.

                  if ( mCursorPos == mBlockStart )
                  {
                     // We are at the start block and traveling left so
                     // just extend the start block farther left.                     
                     mBlockStart = newpos;
                  }
                  else
                  {
                     // We are at the end block BUT traveling left
                     // back towards the start block...

                     if ( newpos > mBlockStart )
                     {
                        // We haven't overpassed the existing start block
                        // so just trim back the end block.
                        mBlockEnd = newpos;
                     }
                     else if ( newpos == mBlockStart )
                     {
                        // We are back at the start, so no more selection.
                        mBlockEnd = mBlockStart = 0;
                     }
                     else
                     {
                        // Only other option, we just backtracked PAST
                        // our original start block.
                        // So the new position becomes the start block
                        // and the old start block becomes the end block.
                        mBlockEnd = mBlockStart;
                        mBlockStart = newpos;
                     }
                  }
               }
                              
               mCursorPos = newpos;

               return true;
            }

            case KEY_RIGHT:
            {
               S32 newpos = findNextWord();               

               if ( mBlockStart == mBlockEnd )
               {
                  // There was not already a selection so start a new one.
                  mBlockStart = mCursorPos;
                  mBlockEnd = newpos;
               }
               else
               {
                  // There was a selection already...
                  // In this case the cursor MUST be at either the
                  // start or end of that selection.

                  if ( mCursorPos == mBlockEnd )
                  {
                     // We are at the end block and traveling right so
                     // just extend the end block farther right.                     
                     mBlockEnd = newpos;
                  }
                  else
                  {
                     // We are at the start block BUT traveling right
                     // back towards the end block...

                     if ( newpos < mBlockEnd )
                     {
                        // We haven't overpassed the existing end block
                        // so just trim back the start block.
                        mBlockStart = newpos;
                     }
                     else if ( newpos == mBlockEnd )
                     {
                        // We are back at the end, so no more selection.
                        mBlockEnd = mBlockStart = 0;
                     }
                     else
                     {
                        // Only other option, we just backtracked PAST
                        // our original end block.
                        // So the new position becomes the end block
                        // and the old end block becomes the start block.
                        mBlockStart = mBlockEnd;
                        mBlockEnd = newpos;
                     }
                  }
               }

               mCursorPos = newpos;

               return true;
            }
            
            default:
               break;
         }
      }

      // End support for word jump selection.


        switch ( event.keyCode )
        {
            case KEY_TAB:
               if ( mTabComplete )
               {
              onTabComplete_callback("1");
                  return true;
               }
            break; // We don't want to fall through if we don't handle the TAB here.

            case KEY_HOME:
               mBlockStart = 0;
               mBlockEnd = mCursorPos;
               mCursorPos = 0;
               return true;

            case KEY_END:
                mBlockStart = mCursorPos;
                mBlockEnd = stringLen;
                mCursorPos = stringLen;
                return true;

            case KEY_LEFT:
                if ((mCursorPos > 0) & (stringLen > 0))
                {
                    //if we already have a selected block
                    if (mCursorPos == mBlockEnd)
                    {
                        mCursorPos--;
                        mBlockEnd--;
                        if (mBlockEnd == mBlockStart)
                        {
                            mBlockStart = 0;
                            mBlockEnd = 0;
                        }
                    }
                    else {
                        mCursorPos--;
                        mBlockStart = mCursorPos;

                        if (mBlockEnd == 0)
                        {
                            mBlockEnd = mCursorPos + 1;
                        }
                    }
                }
                return true;

            case KEY_RIGHT:
                if (mCursorPos < stringLen)
                {
                    if ((mCursorPos == mBlockStart) && (mBlockEnd > 0))
                    {
                        mCursorPos++;
                        mBlockStart++;
                        if (mBlockStart == mBlockEnd)
                        {
                            mBlockStart = 0;
                            mBlockEnd = 0;
                        }
                    }
                    else
                    {
                        if (mBlockEnd == 0)
                        {
                            mBlockStart = mCursorPos;
                            mBlockEnd = mCursorPos;
                        }
                        mCursorPos++;
                        mBlockEnd++;
                    }
                }
                return true;

            case KEY_RETURN:
            case KEY_NUMPADENTER:
           
               return dealWithEnter(false);

            default:
               break;
        }
    }
   else if (event.modifier & SI_CTRL)
   {
      switch(event.keyCode)
      {
#if defined(TORQUE_OS_MAC)
         // Added UNIX emacs key bindings - just a little hack here...

         // Ctrl-B - move one character back
         case KEY_B:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_LEFT;
            return(onKeyDown(new_event));
         }

         // Ctrl-F - move one character forward
         case KEY_F:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_RIGHT;
            return(onKeyDown(new_event));
         }

         // Ctrl-A - move to the beginning of the line
         case KEY_A:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_HOME;
            return(onKeyDown(new_event));
         }

         // Ctrl-E - move to the end of the line
         case KEY_E:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_END;
            return(onKeyDown(new_event));
         }

         // Ctrl-P - move backward in history
         case KEY_P:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_UP;
            return(onKeyDown(new_event));
         }

         // Ctrl-N - move forward in history
         case KEY_N:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_DOWN;
            return(onKeyDown(new_event));
         }

         // Ctrl-D - delete under cursor
         case KEY_D:
         { 
            GuiEvent new_event;
            new_event.modifier = 0;
            new_event.keyCode = KEY_DELETE;
            return(onKeyDown(new_event));
         }

         case KEY_U:
         { 
            GuiEvent new_event;
            new_event.modifier = SI_CTRL;
            new_event.keyCode = KEY_DELETE;
            return(onKeyDown(new_event));
         }

         // End added UNIX emacs key bindings
#endif

         // Adding word jump navigation.

         case KEY_LEFT:
         {

            mCursorPos = findPrevWord();
            mBlockStart = 0;
            mBlockEnd = 0;
            return true;
         }

         case KEY_RIGHT:
         {
            mCursorPos = findNextWord();
            mBlockStart = 0;
            mBlockEnd = 0;
            return true;
         }         
         
#if !defined(TORQUE_OS_MAC)
         // Select all
         case KEY_A:
         {
            selectAllText();
            return true;
         }

         // windows style cut / copy / paste / undo keybinds
         case KEY_C:
         case KEY_X:
         {
            // copy, and cut the text if we hit ctrl-x
            onCopy( event.keyCode==KEY_X );
            return true;
         }
         case KEY_V:
         {
            onPaste();

            // Execute the console command!
            execConsoleCallback();
            return true;
         }

         case KEY_Z:
            if (! mDragHit)
            {
               onUndo();
               return true;
            }
#endif

         case KEY_DELETE:
         case KEY_BACKSPACE:
            //save the current state
            saveUndoState();

            //delete everything in the field
            mTextBuffer.set("");
            mCursorPos  = 0;
            mBlockStart = 0;
            mBlockEnd   = 0;

            execConsoleCallback();

            return true;
         default:
            break;
      }
   }
#if defined(TORQUE_OS_MAC)
   // mac style cut / copy / paste / undo keybinds
   else if (event.modifier & SI_ALT)
   {
      // Mac command key maps to alt in torque.

      // Added Mac cut/copy/paste/undo keys
      switch(event.keyCode)
      {
         // Select all
         case KEY_A:
         {
            selectAllText();
            return true;
         }
         case KEY_C:
         case KEY_X:
         {
            // copy, and cut the text if we hit cmd-x
            onCopy( event.keyCode==KEY_X );
            return true;
         }
         case KEY_V:
         {
            onPaste();

            // Execute the console command!
            execConsoleCallback();

            return true;
         }
            
         case KEY_Z:
            if (! mDragHit)
            {
               onUndo();
               return true;
            }
            
         default:
            break;
      }
   }
#endif
   else
   {
      switch(event.keyCode)
      {
         case KEY_ESCAPE:
            if( mEscapeCommand.isNotEmpty() )
            {
               evaluate( mEscapeCommand );
               return( true );
            }
            return( Parent::onKeyDown( event ) );

         case KEY_RETURN:
         case KEY_NUMPADENTER:
           
            return dealWithEnter(true);

         case KEY_UP:
         {
            if( mHistorySize > 0 )
            {
               if(mHistoryDirty)
               {
                  updateHistory(&mTextBuffer, false);
                  mHistoryDirty = false;
               }

               mHistoryIndex--;
               
               if(mHistoryIndex >= 0 && mHistoryIndex <= mHistoryLast)
                  setText(mHistoryBuf[mHistoryIndex]);
               else if(mHistoryIndex < 0)
                  mHistoryIndex = 0;
            }
               
            return true;
         }

         case KEY_DOWN:
         {
            if( mHistorySize > 0 )
            {
               if(mHistoryDirty)
               {
                  updateHistory(&mTextBuffer, false);
                  mHistoryDirty = false;
               }
               mHistoryIndex++;
               if(mHistoryIndex > mHistoryLast)
               {
                  mHistoryIndex = mHistoryLast + 1;
                  setText("");
               }
               else
                  setText(mHistoryBuf[mHistoryIndex]);
            }
            return true;
         }

         case KEY_LEFT:
            
            // If we have a selection put the cursor to the left side of it.
            if ( mBlockStart != mBlockEnd )
            {
               mCursorPos = mBlockStart;
               mBlockStart = mBlockEnd = 0;
            }
            else
            {
               mBlockStart = mBlockEnd = 0;
               mCursorPos = getMax( mCursorPos - 1, 0 );               
            }

            return true;

         case KEY_RIGHT:

            // If we have a selection put the cursor to the right side of it.            
            if ( mBlockStart != mBlockEnd )
            {
               mCursorPos = mBlockEnd;
               mBlockStart = mBlockEnd = 0;
            }
            else
            {
               mBlockStart = mBlockEnd = 0;
               mCursorPos = getMin( mCursorPos + 1, stringLen );               
            }

            return true;

         case KEY_BACKSPACE:
dealWithBackspace:
            //save the current state
            saveUndoState();

            if (mBlockEnd > 0)
            {
               mTextBuffer.cut(mBlockStart, mBlockEnd-mBlockStart);
               mCursorPos  = mBlockStart;
               mBlockStart = 0;
               mBlockEnd   = 0;
               mHistoryDirty = true;

               // Execute the console command!
               execConsoleCallback();

            }
            else if (mCursorPos > 0)
            {
               mTextBuffer.cut(mCursorPos-1, 1);
               mCursorPos--;
               mHistoryDirty = true;

               // Execute the console command!
               execConsoleCallback();
            }
            return true;

         case KEY_DELETE:
            //save the current state
            saveUndoState();

            if (mBlockEnd > 0)
            {
               mHistoryDirty = true;
               mTextBuffer.cut(mBlockStart, mBlockEnd-mBlockStart);

               mCursorPos = mBlockStart;
               mBlockStart = 0;
               mBlockEnd = 0;

               // Execute the console command!
               execConsoleCallback();
            }
            else if (mCursorPos < stringLen)
            {
               mHistoryDirty = true;
               mTextBuffer.cut(mCursorPos, 1);

               // Execute the console command!
               execConsoleCallback();
            }
            return true;

         case KEY_INSERT:
            mInsertOn = !mInsertOn;
            return true;

         case KEY_HOME:
            mBlockStart = 0;
            mBlockEnd   = 0;
            mCursorPos  = 0;
            return true;

         case KEY_END:
            mBlockStart = 0;
            mBlockEnd   = 0;
            mCursorPos  = stringLen;
            return true;
      
         default:
            break;

         }
   }

   switch ( event.keyCode )
   {
      case KEY_TAB:
         if ( mTabComplete )
         {
         onTabComplete_callback("0");
            return( true );
         }
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ESCAPE:
         return Parent::onKeyDown( event );
      default:
         break;
   }
         
   // Handle character input events.

   if( mProfile->mFont->isValidChar( event.ascii ) )
   {
      handleCharInput( event.ascii );
      return true;
   }

   // Or eat it if that's appropriate.
   if( mSinkAllKeyEvents )
      return true;

   // Not handled - pass the event to it's parent.
   return Parent::onKeyDown( event );
}

bool GuiTextEditCtrl::dealWithEnter( bool clearResponder )
{
   //first validate
   if (mProfile->mReturnTab)
   {
      onLoseFirstResponder();
   }

   updateHistory(&mTextBuffer, true);
   mHistoryDirty = false;

   //next exec the alt console command
   execAltConsoleCallback();

   // Notify of Return
   onReturn_callback();

   if (mProfile->mReturnTab)
   {
      GuiCanvas *root = getRoot();
      if (root)
      {
         root->tabNext();
         return true;
      }
   }
   
   if( clearResponder )
      clearFirstResponder();

   return true;
}

void GuiTextEditCtrl::setFirstResponder()
{
   Parent::setFirstResponder();

   GuiCanvas *root = getRoot();
   if (root != NULL)
   {
      root->enableKeyboardTranslation();
  

      // If the native OS accelerator keys are not disabled
      // then some key events like Delete, ctrl+V, etc may
      // not make it down to us.
      root->setNativeAcceleratorsEnabled( false );
   }
}

void GuiTextEditCtrl::onLoseFirstResponder()
{
   GuiCanvas *root = getRoot();
   if( root )
   {
    root->setNativeAcceleratorsEnabled( true );
    root->disableKeyboardTranslation();
   }

   //execute the validate command
   if( mValidateCommand.isNotEmpty() )
      evaluate( mValidateCommand );

   onValidate_callback();

   // Redraw the control:
   setUpdate();

   // Lost Responder
   Parent::onLoseFirstResponder();
}

void GuiTextEditCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   RectI ctrlRect( offset, getExtent() );

   //if opaque, fill the update rect with the fill color
   if ( mProfile->mOpaque )
   {
      if ( !mTextValid )
         GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColorERR );
      else if ( isFirstResponder() )
         GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColorHL );
      else
         GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColor );
   }

   //if there's a border, draw the border
   if ( mProfile->mBorder )
   {
      renderBorder( ctrlRect, mProfile );
      if ( !mTextValid )
         GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColorERR );
   }

   drawText( ctrlRect, isFirstResponder() );
}

void GuiTextEditCtrl::onPreRender()
{
   if ( isFirstResponder() )
   {
      U32 timeElapsed = Platform::getVirtualMilliseconds() - mTimeLastCursorFlipped;
      mNumFramesElapsed++;
      if ( ( timeElapsed > 500 ) && ( mNumFramesElapsed > 3 ) )
      {
         mCursorOn = !mCursorOn;
         mTimeLastCursorFlipped = Platform::getVirtualMilliseconds();
         mNumFramesElapsed = 0;
         setUpdate();
      }

      //update the cursor if the text is scrolling
      if ( mDragHit )
      {
         if ( ( mScrollDir < 0 ) && ( mCursorPos > 0 ) )
            mCursorPos--;
         else if ( ( mScrollDir > 0 ) && ( mCursorPos < (S32) mTextBuffer.length() ) )
            mCursorPos++;
      }
   }
}

void GuiTextEditCtrl::drawText( const RectI &drawRect, bool isFocused )
{
   StringBuffer textBuffer;
   Point2I drawPoint = drawRect.point;
   Point2I paddingLeftTop, paddingRightBottom;

   // Or else just copy it over.
   char *renderText = Con::getReturnBuffer( GuiTextEditCtrl::MAX_STRING_LENGTH );
   getRenderText( renderText );

   // Apply password masking (make the masking char optional perhaps?)
   if(mPasswordText)
   {
      const U32 renderLen = dStrlen( renderText );
      for( U32 i = 0; i < renderLen; i++ )
         textBuffer.append(mPasswordMask);
   }
   else
   {
       textBuffer.set( renderText );
   }

   // Just a little sanity.
   if(mCursorPos > textBuffer.length()) 
      mCursorPos = textBuffer.length();

   paddingLeftTop.set(( mProfile->mTextOffset.x != 0 ? mProfile->mTextOffset.x : 3 ), mProfile->mTextOffset.y);
   paddingRightBottom = paddingLeftTop;

   // Center vertically:
   drawPoint.y += ( ( drawRect.extent.y - paddingLeftTop.y - paddingRightBottom.y - S32( mProfile->mFont->getHeight() ) ) / 2 ) + paddingLeftTop.y;

   // Align horizontally:
   
   S32 textWidth = mProfile->mFont->getStrNWidth(textBuffer.getPtr(), textBuffer.length());

   switch( mProfile->mAlignment )
   {
   case GuiControlProfile::RightJustify:
      drawPoint.x += ( drawRect.extent.x - textWidth - paddingRightBottom.x );
      break;
   case GuiControlProfile::CenterJustify:
      drawPoint.x += ( ( drawRect.extent.x - textWidth ) / 2 );
      break;
   default:
   case GuiControlProfile::LeftJustify :
      drawPoint.x += paddingLeftTop.x;
      break;
   }

   ColorI fontColor = mActive ? mProfile->mFontColor : mProfile->mFontColorNA;

   // now draw the text
   Point2I cursorStart, cursorEnd;
   mTextOffset.y = drawPoint.y;
   mTextOffset.x = drawPoint.x;

   if ( drawRect.extent.x - paddingLeftTop.x > textWidth )
      mTextOffset.x = drawPoint.x;
   else
   {
      // Alignment affects large text
      if ( mProfile->mAlignment == GuiControlProfile::RightJustify
         || mProfile->mAlignment == GuiControlProfile::CenterJustify )
      {
         if ( mTextOffset.x + textWidth < (drawRect.point.x + drawRect.extent.x) - paddingRightBottom.x)
            mTextOffset.x = (drawRect.point.x + drawRect.extent.x) - paddingRightBottom.x - textWidth;
      }
   }

   // calculate the cursor
   if( isFocused && mActive )
   {
      // Where in the string are we?
      S32 cursorOffset=0, charWidth=0;
      UTF16 tempChar = textBuffer.getChar(mCursorPos);

      // Alright, we want to terminate things momentarily.
      if(mCursorPos > 0)
      {
         cursorOffset = mProfile->mFont->getStrNWidth(textBuffer.getPtr(), mCursorPos);
      }
      else
         cursorOffset = 0;

      if( tempChar && mProfile->mFont->isValidChar( tempChar ) )
         charWidth = mProfile->mFont->getCharWidth( tempChar );
      else
         charWidth = paddingRightBottom.x;

      if( mTextOffset.x + cursorOffset + 1 >= (drawRect.point.x + drawRect.extent.x) ) // +1 is for the cursor width
      {
         // Cursor somewhere beyond the textcontrol,
         // skip forward roughly 25% of the total width (if possible)
         S32 skipForward = drawRect.extent.x / 4 * 3;

         if ( cursorOffset + skipForward > textWidth )
            mTextOffset.x = (drawRect.point.x + drawRect.extent.x) - paddingRightBottom.x - textWidth;
         else
         {
            //mTextOffset.x -= skipForward;
            S32 mul = (S32)( mFloor( (cursorOffset-drawRect.extent.x) / skipForward ) );
            mTextOffset.x -= skipForward * mul + drawRect.extent.x - 1; // -1 is for the cursor width
         }
      }
      else if( mTextOffset.x + cursorOffset < drawRect.point.x + paddingLeftTop.x )
      {
         // Cursor somewhere before the textcontrol
         // skip backward roughly 25% of the total width (if possible)
         S32 skipBackward = drawRect.extent.x / 4 * 3;

         if ( cursorOffset - skipBackward < 0 )
            mTextOffset.x = drawRect.point.x + paddingLeftTop.x;
         else
         {
            S32 mul = (S32)( mFloor( cursorOffset / skipBackward ) );
            mTextOffset.x += drawRect.point.x - mTextOffset.x - skipBackward * mul;
         }
      }
      cursorStart.x = mTextOffset.x + cursorOffset;

#ifdef TORQUE_OS_MAC
      cursorStart.x += charWidth/2;
#endif
      
      cursorEnd.x = cursorStart.x;

      S32 cursorHeight = mProfile->mFont->getHeight();
      if ( cursorHeight < drawRect.extent.y )
      {
         cursorStart.y = drawPoint.y;
         cursorEnd.y = cursorStart.y + cursorHeight;
      }
      else
      {
         cursorStart.y = drawRect.point.y;
         cursorEnd.y = cursorStart.y + drawRect.extent.y;
      }
   }

   //draw the text
   if ( !isFocused )
      mBlockStart = mBlockEnd = 0;

   //also verify the block start/end
   if ((mBlockStart > textBuffer.length() || (mBlockEnd > textBuffer.length()) || (mBlockStart > mBlockEnd)))
      mBlockStart = mBlockEnd = 0;

   Point2I tempOffset = mTextOffset;

   //draw the portion before the highlight
   if ( mBlockStart > 0 )
   {
      GFX->getDrawUtil()->setBitmapModulation( fontColor );
      const UTF16* preString2 = textBuffer.getPtr();
      GFX->getDrawUtil()->drawText( mProfile->mFont, tempOffset, preString2, mProfile->mFontColors );
      tempOffset.x += mProfile->mFont->getStrNWidth(preString2, mBlockStart);
   }

   //draw the highlighted portion
   if ( mBlockEnd > 0 )
   {
      const UTF16* highlightBuff = textBuffer.getPtr() + mBlockStart;
      U32 highlightBuffLen = mBlockEnd-mBlockStart;

      S32 highlightWidth = mProfile->mFont->getStrNWidth(highlightBuff, highlightBuffLen);

      GFX->getDrawUtil()->drawRectFill( Point2I( tempOffset.x, drawRect.point.y ),
         Point2I( tempOffset.x + highlightWidth, drawRect.point.y + drawRect.extent.y - 1),
         mProfile->mFontColorSEL );

      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColorHL );
      GFX->getDrawUtil()->drawTextN( mProfile->mFont, tempOffset, highlightBuff, highlightBuffLen, mProfile->mFontColors );
      tempOffset.x += highlightWidth;
   }

   //draw the portion after the highlight
   if(mBlockEnd < textBuffer.length())
   {
      const UTF16* finalBuff = textBuffer.getPtr() + mBlockEnd;
      U32 finalBuffLen = textBuffer.length() - mBlockEnd;

      GFX->getDrawUtil()->setBitmapModulation( fontColor );
      GFX->getDrawUtil()->drawTextN( mProfile->mFont, tempOffset, finalBuff, finalBuffLen, mProfile->mFontColors );
   }

   //draw the cursor
   if ( isFocused && mCursorOn )
      GFX->getDrawUtil()->drawLine( cursorStart, cursorEnd, mProfile->mCursorColor );
}

bool GuiTextEditCtrl::hasText()
{
   return ( mTextBuffer.length() );
}

void GuiTextEditCtrl::invalidText(bool playSound)
{
   mTextValid = false;

   if ( playSound )
      playDeniedSound();
}

void GuiTextEditCtrl::validText()
{
   mTextValid = true;
}

bool GuiTextEditCtrl::isValidText()
{
   return mTextValid;
}

void GuiTextEditCtrl::playDeniedSound()
{
   if ( mDeniedSound )
      SFX->playOnce( mDeniedSound );
}

const char *GuiTextEditCtrl::getScriptValue()
{
   return StringTable->insert(mTextBuffer.getPtr8());
}

void GuiTextEditCtrl::setScriptValue(const char *value)
{
   mTextBuffer.set(value);
   mCursorPos = mTextBuffer.length();
}

void GuiTextEditCtrl::handleCharInput( U16 ascii )
{
   S32 stringLen = mTextBuffer.length();

   // Get the character ready to add to a UTF8 string.
   UTF16 convertedChar[2] = { ascii, 0 };

   //see if it's a number field
   if ( mProfile->mNumbersOnly )
   {
      if (ascii == '-')
      {
         //a minus sign only exists at the beginning, and only a single minus sign
         if (mCursorPos != 0 && !isAllTextSelected())
         {
            invalidText();
            return;
         }

         if (mInsertOn && (mTextBuffer.getChar(0) == '-'))
         {
            invalidText();
            return;
         }
      }
      // BJTODO: This is probably not unicode safe.
      else if (ascii != '.' && (ascii < '0' || ascii > '9'))
      {
         invalidText();
         return;
      }
      else
         validText();
   }

   //save the current state
   saveUndoState();

   bool alreadyCut = false;

   //delete anything highlighted
   if ( mBlockEnd > 0 )
   {
      mTextBuffer.cut(mBlockStart, mBlockEnd-mBlockStart);
      mCursorPos  = mBlockStart;
      mBlockStart = 0;
      mBlockEnd   = 0;

      // We just changed the string length!
      // Get its new value.
      stringLen = mTextBuffer.length();

      // If we already had text highlighted, we just want to cut that text.
      // Don't cut the next character even if insert is not on.
      alreadyCut = true;
   }

   if ( ( mInsertOn && ( stringLen < mMaxStrLen ) ) ||
      ( !mInsertOn && ( mCursorPos < mMaxStrLen ) ) )
   {
      if ( mCursorPos == stringLen )
      {
         mTextBuffer.append(convertedChar);
         mCursorPos++;
      }
      else
      {
         if ( mInsertOn || alreadyCut )
         {
            mTextBuffer.insert(mCursorPos, convertedChar);
            mCursorPos++;
         }
         else
         {
            mTextBuffer.cut(mCursorPos, 1);
            mTextBuffer.insert(mCursorPos, convertedChar);
            mCursorPos++;
         }
      }
   }
   else
      playDeniedSound();

   //reset the history index
   mHistoryDirty = true;

   //execute the console command if it exists
   execConsoleCallback();
}

S32 GuiTextEditCtrl::findPrevWord()
{   
   // First the first word to the left of the current cursor position 
   // and return the positional index of its starting character.

   // We define the first character of a word as any non-whitespace
   // character which has a non-alpha-numeric character to its immediate left.

   const UTF8* text = mTextBuffer.getPtr8();

   for ( S32 i = mCursorPos - 1; i > 0; i-- )
   {  
      if ( !dIsspace( text[i] ) )
      {
         if ( !dIsalnum( text[i-1] ) )
         {
            return i;
         }
      }
   }

   return 0;
}

S32 GuiTextEditCtrl::findNextWord()
{
   // First the first word to the right of the current cursor position 
   // and return the positional index of its starting character.

   // We define the first character of a word as any non-whitespace
   // character which has a non-alpha-numeric character to its immediate left.
   
   const UTF8* text = mTextBuffer.getPtr8();

   for ( S32 i = mCursorPos + 1; i < mTextBuffer.length(); i++ )
   {  
      if ( !dIsspace( text[i] ) )
      {
         if ( !dIsalnum( text[i-1] ) )
         {
            return i;
         }
      }
   }

   return mTextBuffer.length();
}

DefineEngineMethod( GuiTextEditCtrl, getText, const char*, (),,
   "@brief Acquires the current text displayed in this control.\n\n"
   "@tsexample\n"
   "// Acquire the value of the text control.\n"
   "%text = %thisGuiTextEditCtrl.getText();\n"
   "@endtsexample\n\n"
   "@return The current text within the control.\n\n"
   "@see GuiControl")
{
   if( !object->hasText() )
      return StringTable->EmptyString();

   char *retBuffer = Con::getReturnBuffer( GuiTextEditCtrl::MAX_STRING_LENGTH );
   object->getText( retBuffer );

   return retBuffer;
}

DefineEngineMethod( GuiTextEditCtrl, setText, void, (const char* text),,
   "@brief Sets the text in the control.\n\n"
   "@param text Text to place in the control.\n"
   "@tsexample\n"
   "// Define the text to display\n"
   "%text = \"Text!\"\n\n"
   "// Inform the GuiTextEditCtrl to display the defined text\n"
   "%thisGuiTextEditCtrl.setText(%text);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setText( text );
}

DefineEngineMethod( GuiTextEditCtrl, getCursorPos, S32, (),,
   "@brief Returns the current position of the text cursor in the control.\n\n"
   "@tsexample\n"
   "// Acquire the cursor position in the control\n"
   "%position = %thisGuiTextEditCtrl.getCursorPost();\n"
   "@endtsexample\n\n"
   "@return Text cursor position within the control.\n\n"
   "@see GuiControl")
{
   return( object->getCursorPos() );
}

DefineEngineMethod( GuiTextEditCtrl, setCursorPos, void, (S32 position),,
   "@brief Sets the text cursor at the defined position within the control.\n\n"
   "@param position Text position to set the text cursor.\n"
   "@tsexample\n"
   "// Define the cursor position\n"
   "%position = \"12\";\n\n"
   "// Inform the GuiTextEditCtrl control to place the text cursor at the defined position\n"
   "%thisGuiTextEditCtrl.setCursorPos(%position);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setCursorPos( position );
}

DefineEngineMethod( GuiTextEditCtrl, isAllTextSelected, bool, (),,
   "@brief Checks to see if all text in the control has been selected.\n\n"
   "@tsexample\n"
   "// Check to see if all text has been selected or not.\n"
   "%allSelected = %thisGuiTextEditCtrl.isAllTextSelected();\n"
   "@endtsexample\n\n"
   "@return True if all text in the control is selected, otherwise false.\n\n"
   "@see GuiControl")
{
   return object->isAllTextSelected();
}

DefineEngineMethod( GuiTextEditCtrl, selectAllText, void, (),,
   "@brief Selects all text within the control.\n\n"
   "@tsexample\n"
   "// Inform the control to select all of its text.\n"
   "%thisGuiTextEditCtrl.selectAllText();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->selectAllText();
}

DefineEngineMethod( GuiTextEditCtrl, clearSelectedText, void, (),,
   "@brief Unselects all selected text in the control.\n\n"
   "@tsexample\n"
   "// Inform the control to unselect all of its selected text\n"
   "%thisGuiTextEditCtrl.clearSelectedText();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clearSelectedText();
}

DefineEngineMethod( GuiTextEditCtrl, forceValidateText, void, (),,
   "@brief Force a validation to occur.\n\n"
   "@tsexample\n"
   "// Inform the control to force a validation of its text.\n"
   "%thisGuiTextEditCtrl.forceValidateText();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->forceValidateText();
}

DefineEngineMethod(GuiTextEditCtrl, invalidText, void, (bool playSound), (true),
   "@brief Trigger the invalid sound and make the box red.nn"
   "@param playSound Play the invalid text sound or not.n")
{
   object->invalidText(playSound);
}


DefineEngineMethod(GuiTextEditCtrl, validText, void, (), ,
   "@brief Restores the box to normal color.nn")
{
   object->validText();
}

DefineEngineMethod(GuiTextEditCtrl, isValidText, bool, (), ,
   "@brief Returns if the text is set to valid or not.n"
   "@Return true if text is set to valid, false if not.nn")
{
   return object->isValidText();
}
