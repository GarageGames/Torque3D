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

#include "gui/controls/guiMLTextEditCtrl.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/core/guiCanvas.h"
#include "console/consoleTypes.h"
#include "core/frameAllocator.h"
#include "core/stringBuffer.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiMLTextEditCtrl);

ConsoleDocClass( GuiMLTextEditCtrl,
   "@brief A text entry control that accepts the Gui Markup Language ('ML') tags and multiple lines.\n\n"

   "@tsexample\n"
   "new GuiMLTextEditCtrl()\n"
   "	{\n"
   "		lineSpacing = \"2\";\n"
   "		allowColorChars = \"0\";\n"
   "		maxChars = \"-1\";\n"
   "		deniedSound = \"DeniedSoundProfile\";\n"
   "		text = \"\";\n"
   "		escapeCommand = \"onEscapeScriptFunction();\";\n"
   "	  //Properties not specific to this control have been omitted from this example.\n"
   "	};\n"
   "@endtsexample\n\n"

   "@see GuiMLTextCtrl\n"
   "@see GuiControl\n\n"

   "@ingroup GuiControls\n"
);

//--------------------------------------------------------------------------
GuiMLTextEditCtrl::GuiMLTextEditCtrl()
{
   mEscapeCommand = StringTable->insert( "" );

	mIsEditCtrl = true;

   mActive = true;

   mVertMoveAnchorValid = false;
}


//--------------------------------------------------------------------------
GuiMLTextEditCtrl::~GuiMLTextEditCtrl()
{

}


//--------------------------------------------------------------------------
bool GuiMLTextEditCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   // We don't want to get any smaller than our parent:
   Point2I newExt = newExtent;
   GuiControl* parent = getParent();
   if ( parent )
      newExt.y = getMax( parent->getHeight(), newExt.y );

   return Parent::resize( newPosition, newExt );
}


//--------------------------------------------------------------------------
void GuiMLTextEditCtrl::initPersistFields()
{
   addField( "escapeCommand", TypeString, Offset( mEscapeCommand, GuiMLTextEditCtrl ), "Script function to run whenever the 'escape' key is pressed when this control is in focus.\n");
   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
void GuiMLTextEditCtrl::setFirstResponder()
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

void GuiMLTextEditCtrl::onLoseFirstResponder()
{
   GuiCanvas *root = getRoot();
   if (root != NULL)
   {
      root->setNativeAcceleratorsEnabled( true );
      root->disableKeyboardTranslation();
   }

   // Redraw the control:
   setUpdate();
}

//--------------------------------------------------------------------------
bool GuiMLTextEditCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;

   getRoot()->enableKeyboardTranslation();
   return true;
}

//--------------------------------------------------------------------------
// Key events...
bool GuiMLTextEditCtrl::onKeyDown(const GuiEvent& event)
{
   if ( !isActive() )
      return false;

	setUpdate();
	//handle modifiers first...
   if (event.modifier & SI_PRIMARY_CTRL)
   {
      switch(event.keyCode)
      {
			//copy/cut
         case KEY_C:
         case KEY_X:
			{
				//make sure we actually have something selected
				if (mSelectionActive)
				{
		         copyToClipboard(mSelectionStart, mSelectionEnd);

					//if we're cutting, also delete the selection
					if (event.keyCode == KEY_X)
					{
			         mSelectionActive = false;
			         deleteChars(mSelectionStart, mSelectionEnd);
			         mCursorPosition = mSelectionStart;
					}
					else
			         mCursorPosition = mSelectionEnd + 1;
				}
				return true;
			}

			//paste
         case KEY_V:
			{
				const char *clipBuf = Platform::getClipboard();
				if (dStrlen(clipBuf) > 0)
				{
			      // Normal ascii keypress.  Go ahead and add the chars...
			      if (mSelectionActive == true)
			      {
			         mSelectionActive = false;
			         deleteChars(mSelectionStart, mSelectionEnd);
			         mCursorPosition = mSelectionStart;
			      }

			      insertChars(clipBuf, dStrlen(clipBuf), mCursorPosition);
				}
				return true;
			}
         
         default:
            break;
		}
   }
   else if ( event.modifier & SI_SHIFT )
   {
      switch ( event.keyCode )
      {
         case KEY_TAB:
            return( Parent::onKeyDown( event ) );
         default:
            break;
      }
   }
   else if ( event.modifier == 0 )
   {
      switch (event.keyCode)
      {
         // Escape:
         case KEY_ESCAPE:
            if ( mEscapeCommand[0] )
            {
               Con::evaluate( mEscapeCommand );
               return( true );
            }
            return( Parent::onKeyDown( event ) );

         // Deletion
         case KEY_BACKSPACE:
         case KEY_DELETE:
            handleDeleteKeys(event);
            return true;

         // Cursor movement
         case KEY_LEFT:
         case KEY_RIGHT:
         case KEY_UP:
         case KEY_DOWN:
         case KEY_HOME:
         case KEY_END:
            handleMoveKeys(event);
            return true;

         // Special chars...
         case KEY_TAB:
            // insert 3 spaces
            if (mSelectionActive == true)
            {
               mSelectionActive = false;
               deleteChars(mSelectionStart, mSelectionEnd);
               mCursorPosition = mSelectionStart;
            }
            insertChars( "\t", 1, mCursorPosition );
            return true;

         case KEY_RETURN:
            // insert carriage return
            if (mSelectionActive == true)
            {
               mSelectionActive = false;
               deleteChars(mSelectionStart, mSelectionEnd);
               mCursorPosition = mSelectionStart;
            }
            insertChars( "\n", 1, mCursorPosition );
            return true;
            
         default:
            break;
      }
   }

   if ( (mFont && mFont->isValidChar(event.ascii)) || (!mFont && event.ascii != 0) )
   {
      // Normal ascii keypress.  Go ahead and add the chars...
      if (mSelectionActive == true)
      {
         mSelectionActive = false;
         deleteChars(mSelectionStart, mSelectionEnd);
         mCursorPosition = mSelectionStart;
      }

      UTF8 *outString = NULL;
      U32 outStringLen = 0;

#ifdef TORQUE_UNICODE

      UTF16 inData[2] = { event.ascii, 0 };
      StringBuffer inBuff(inData);

      FrameTemp<UTF8> outBuff(4);
      inBuff.getCopy8(outBuff, 4);

      outString = outBuff;
      outStringLen = dStrlen(outBuff);
#else
      char ascii = char(event.ascii);
      outString = &ascii;
      outStringLen = 1;
#endif

      insertChars(outString, outStringLen, mCursorPosition);
      mVertMoveAnchorValid = false;
      return true;
   }

   // Otherwise, let the parent have the event...
   return Parent::onKeyDown(event);
}


//--------------------------------------
void GuiMLTextEditCtrl::handleDeleteKeys(const GuiEvent& event)
{
   if ( isSelectionActive() )
   {
      mSelectionActive = false;
      deleteChars(mSelectionStart, mSelectionEnd+1);
      mCursorPosition = mSelectionStart;
   }
   else
   {
      switch ( event.keyCode )
      {
         case KEY_BACKSPACE:
            if (mCursorPosition != 0)
            {
               // delete one character left
               deleteChars(mCursorPosition-1, mCursorPosition);
               setUpdate();
            }
            break;

         case KEY_DELETE:
            if (mCursorPosition != mTextBuffer.length())
            {
               // delete one character right
               deleteChars(mCursorPosition, mCursorPosition+1);
               setUpdate();
            }
            break;

        default:
            AssertFatal(false, "Unknown key code received!");
      }
   }
}


//--------------------------------------
void GuiMLTextEditCtrl::handleMoveKeys(const GuiEvent& event)
{
   if ( event.modifier & SI_SHIFT )
      return;

   mSelectionActive = false;

   switch ( event.keyCode )
   {
      case KEY_LEFT:
         mVertMoveAnchorValid = false;
         // move one left
         if ( mCursorPosition != 0 )
         {
            mCursorPosition--;
            setUpdate();
         }
         break;

      case KEY_RIGHT:
         mVertMoveAnchorValid = false;
         // move one right
         if ( mCursorPosition != mTextBuffer.length() )
         {
            mCursorPosition++;
            setUpdate();
         }
         break;

      case KEY_UP:
      case KEY_DOWN:
      {
         Line* walk;
         for ( walk = mLineList; walk->next; walk = walk->next )
         {
            if ( mCursorPosition <= ( walk->textStart + walk->len ) )
               break;
         }

         if ( !walk )
            return;

         if ( event.keyCode == KEY_UP )
         {
            if ( walk == mLineList )
               return;
         }
         else if ( walk->next == NULL )
            return;

         Point2I newPos;
         newPos.set( 0, walk->y );

         // Find the x-position:
         if ( !mVertMoveAnchorValid )
         {
            Point2I cursorTopP, cursorBottomP;
            ColorI color;
            getCursorPositionAndColor(cursorTopP, cursorBottomP, color);
            mVertMoveAnchor = cursorTopP.x;
            mVertMoveAnchorValid = true;
         }

         newPos.x = mVertMoveAnchor;

         // Set the new y-position:
         if (event.keyCode == KEY_UP)
            newPos.y--;
         else
            newPos.y += (walk->height + 1);

         if (setCursorPosition(getTextPosition(newPos)))
            mVertMoveAnchorValid = false;
         break;
      }

      case KEY_HOME:
      case KEY_END:
      {
         mVertMoveAnchorValid = false;
         Line* walk;
         for (walk = mLineList; walk->next; walk = walk->next)
         {
            if (mCursorPosition <= (walk->textStart + walk->len))
               break;
         }

         if (walk)
         {
            if (event.keyCode == KEY_HOME)
            {
               //place the cursor at the beginning of the first atom if there is one
               if (walk->atomList)
                  mCursorPosition = walk->atomList->textStart;
               else
                  mCursorPosition = walk->textStart;
            }
            else
            {
               mCursorPosition = walk->textStart;
               mCursorPosition += walk->len;
            }
            setUpdate();
         }
         break;
      }

      default:
         AssertFatal(false, "Unknown move key code was received!");
   }

   ensureCursorOnScreen();
}

//--------------------------------------------------------------------------
void GuiMLTextEditCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   Parent::onRender(offset, updateRect);

   // We are the first responder, draw our cursor in the appropriate position...
   if (isFirstResponder()) 
   {
      Point2I top, bottom;
      ColorI color;
      getCursorPositionAndColor(top, bottom, color);
      GFX->getDrawUtil()->drawLine(top + offset, bottom + offset, mProfile->mCursorColor);
   }
}

