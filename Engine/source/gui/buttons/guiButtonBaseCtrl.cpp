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

#include "gui/buttons/guiButtonBaseCtrl.h"

#include "console/console.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gui/core/guiCanvas.h"
#include "i18n/lang.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"


IMPLEMENT_CONOBJECT( GuiButtonBaseCtrl );

ConsoleDocClass( GuiButtonBaseCtrl,
   "@brief The base class for the various button controls.\n\n"
   
   "This is the base class for the various types of button controls.  If no more specific functionality is required than "
   "offered by this class, then it can be instantiated and used directly.  Otherwise, its subclasses should be used:\n"
   
   "- GuiRadioCtrl (radio buttons)\n"
   "- GuiCheckBoxCtrl (checkboxes)\n"
   "- GuiButtonCtrl (push buttons with text labels)\n"
   "- GuiBitmapButtonCtrl (bitmapped buttons)\n"
   "- GuiBitmapButtonTextCtrl (bitmapped buttons with a text label)\n"
   "- GuiToggleButtonCtrl (toggle buttons, i.e. push buttons with \"sticky\" behavior)\n"
   "- GuiSwatchButtonCtrl (color swatch buttons)\n"
   "- GuiBorderButtonCtrl (push buttons for surrounding child controls)\n\n"
   
   "@ingroup GuiButtons"
);

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onMouseDown, void, (), (),
   "If #useMouseEvents is true, this is called when the left mouse button is pressed on an (active) "
   "button." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onMouseUp, void, (), (),
   "If #useMouseEvents is true, this is called when the left mouse button is release over an (active) "
   "button.\n\n"
   "@note To trigger actions, better use onClick() since onMouseUp() will also be called when the mouse was "
      "not originally pressed on the button." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onClick, void, (), (),
   "Called when the primary action of the button is triggered (e.g. by a left mouse click)." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onDoubleClick, void, (), (),
   "Called when the left mouse button is double-clicked on the button." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onRightClick, void, (), (),
   "Called when the right mouse button is clicked on the button." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onMouseEnter, void, (), (),
   "If #useMouseEvents is true, this is called when the mouse cursor moves over the button (only if the button "
   "is the front-most visible control, though)." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onMouseLeave, void, (), (),
   "If #useMouseEvents is true, this is called when the mouse cursor moves off the button (only if the button "
   "had previously received an onMouseEvent() event)." );

IMPLEMENT_CALLBACK( GuiButtonBaseCtrl, onMouseDragged, void, (), (),
   "If #useMouseEvents is true, this is called when a left mouse button drag is detected, i.e. when the user "
   "pressed the left mouse button on the control and then moves the mouse over a certain distance threshold with "
   "the mouse button still pressed." );


ImplementEnumType( GuiButtonType,
   "Type of button control.\n\n"
   "@ingroup GuiButtons" )
   { GuiButtonBaseCtrl::ButtonTypePush, "PushButton", "A button that triggers an action when clicked." },
   { GuiButtonBaseCtrl::ButtonTypeCheck, "ToggleButton", "A button that is toggled between on and off state." },
   { GuiButtonBaseCtrl::ButtonTypeRadio, "RadioButton", "A button placed in groups for presenting choices." },
EndImplementEnumType;


//-----------------------------------------------------------------------------

GuiButtonBaseCtrl::GuiButtonBaseCtrl()
{
   mDepressed = false;
   mMouseOver = false;
   mActive = true;
   static StringTableEntry sButton = StringTable->insert( "Button" );
   mButtonText = sButton;
   mButtonTextID = StringTable->EmptyString();
   mStateOn = false;
   mRadioGroup = -1;
   mButtonType = ButtonTypePush;
   mUseMouseEvents = false;
   mMouseDragged = false;
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::initPersistFields()
{
   addGroup( "Button" );
   	
      addField( "text", TypeCaseString, Offset(mButtonText, GuiButtonBaseCtrl),
         "Text label to display on button (if button class supports text labels)." );
      addField( "textID", TypeString, Offset(mButtonTextID, GuiButtonBaseCtrl),
         "ID of string in string table to use for text label on button.\n\n"
         "@see setTextID\n"
         "@see GuiControl::langTableMod\n"
         "@see LangTable\n\n" );
      addField( "groupNum", TypeS32, Offset(mRadioGroup, GuiButtonBaseCtrl),
         "Radio button toggle group number.  All radio buttons that are assigned the same #groupNum and that "
         "are parented to the same control will synchronize their toggle state, i.e. if one radio button is toggled on "
         "all other radio buttons in its group will be toggled off.\n\n" 
         "The default group is -1." );
      addField( "buttonType", TYPEID< ButtonType >(), Offset(mButtonType, GuiButtonBaseCtrl),
         "Button behavior type.\n" );
      addField( "useMouseEvents", TypeBool, Offset(mUseMouseEvents, GuiButtonBaseCtrl),
         "If true, mouse events will be passed on to script.  Default is false.\n" );
      
   endGroup( "Button" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiButtonBaseCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   // is we have a script variable, make sure we're in sync
   if ( mConsoleVariable[0] )
   	mStateOn = Con::getBoolVariable( mConsoleVariable );
   if(mButtonTextID && *mButtonTextID != 0)
	   setTextID(mButtonTextID);

   return true;
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::setText( const char* text )
{
   mButtonText = StringTable->insert(text, true);
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::setTextID(const char *id)
{
	S32 n = Con::getIntVariable(id, -1);
	if(n != -1)
	{
		mButtonTextID = StringTable->insert(id);
		setTextID(n);
	}
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::setTextID(S32 id)
{
	const UTF8 *str = getGUIString(id);
	if(str)
		setText((const char*)str);
	//mButtonTextID = id;
}

//-----------------------------------------------------------------------------

const char *GuiButtonBaseCtrl::getText()
{
   return mButtonText;
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::setStateOn( bool bStateOn )
{
   if(!mActive)
      return;

   if(mButtonType == ButtonTypeCheck)
   {
      mStateOn = bStateOn;
   }
   else if(mButtonType == ButtonTypeRadio)
   {
      messageSiblings(mRadioGroup);
      mStateOn = bStateOn;
   }		
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::acceleratorKeyPress(U32)
{
   if( !mActive )
      return;

   //set the bool
   mDepressed = true;

   if (mProfile->mTabable)
      setFirstResponder();
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::acceleratorKeyRelease(U32)
{
   if (! mActive)
      return;

   if (mDepressed)
   {
      //set the bool
      mDepressed = false;
      //perform the action
      onAction();
   }

   //update
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
      return;

   if (mProfile->mCanKeyFocus)
      setFirstResponder();

   if (mProfile->mSoundButtonDown)
      SFX->playOnce(mProfile->mSoundButtonDown);
      
   mMouseDownPoint = event.mousePoint;
   mMouseDragged = false;

   if( mUseMouseEvents )
	  onMouseDown_callback();

   //lock the mouse
   mouseLock();
   mDepressed = true;

   // If we have a double click then execute the alt command.
   if ( event.mouseClickCount == 2 )
   {
      onDoubleClick_callback();
      execAltConsoleCallback();
   }

   //update
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMouseEnter(const GuiEvent &event)
{
   setUpdate();

   if( mUseMouseEvents )
      onMouseEnter_callback();

   if(isMouseLocked())
   {
      mDepressed = true;
      mMouseOver = true;
   }
   else
   {
      if ( mActive && mProfile->mSoundButtonOver )
         SFX->playOnce(mProfile->mSoundButtonOver);

      mMouseOver = true;
   }
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMouseLeave(const GuiEvent &)
{
   setUpdate();

   if( mUseMouseEvents )
      onMouseLeave_callback();
   if( isMouseLocked() )
      mDepressed = false;
   mMouseOver = false;
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMouseUp(const GuiEvent &event)
{
   mouseUnlock();

   if( !mActive )
      return;
   
   setUpdate();

   if( mUseMouseEvents )
      onMouseUp_callback();

   //if we released the mouse within this control, perform the action
   if( mDepressed )
      onAction();

   mDepressed = false;
   mMouseDragged = false;
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onRightMouseUp(const GuiEvent &event)
{
   onRightClick_callback();
   Parent::onRightMouseUp( event );
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMouseDragged( const GuiEvent& event )
{
   if( mUseMouseEvents )
   {
      // If we haven't started a drag yet, find whether we have moved past
      // the tolerance value.
      
      if( !mMouseDragged )
      {
         Point2I delta = mMouseDownPoint - event.mousePoint;
         if( mAbs( delta.x ) > 2 || mAbs( delta.y ) > 2 )
            mMouseDragged = true;
      }
      
      if( mMouseDragged )
         onMouseDragged_callback();
   }
      
   Parent::onMouseDragged( event );
}

//-----------------------------------------------------------------------------

bool GuiButtonBaseCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, kill the event
   if (!mActive)
      return true;

   //see if the key down is a return or space or not
   if ((event.keyCode == KEY_RETURN || event.keyCode == KEY_SPACE)
       && event.modifier == 0)
   {
	   if ( mProfile->mSoundButtonDown )
         SFX->playOnce( mProfile->mSoundButtonDown);

      return true;
   }
   //otherwise, pass the event to it's parent
   return Parent::onKeyDown(event);
}

//-----------------------------------------------------------------------------

bool GuiButtonBaseCtrl::onKeyUp(const GuiEvent &event)
{
   //if the control is a dead end, kill the event
   if (!mActive)
      return true;

   //see if the key down is a return or space or not
   if (mDepressed &&
      (event.keyCode == KEY_RETURN || event.keyCode == KEY_SPACE) &&
      event.modifier == 0)
   {
      onAction();
      return true;
   }

   //otherwise, pass the event to it's parent
   return Parent::onKeyUp(event);
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::setScriptValue(const char *value)
{
	mStateOn = dAtob(value);

	// Update the console variable:
	if ( mConsoleVariable[0] )
		Con::setBoolVariable( mConsoleVariable, mStateOn );

   setUpdate();
}

//-----------------------------------------------------------------------------

const char *GuiButtonBaseCtrl::getScriptValue()
{
	return mStateOn ? "1" : "0";
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onAction()
{
    if(!mActive)
        return;

    if(mButtonType == ButtonTypeCheck)
    {
        mStateOn = mStateOn ? false : true;
   }
   else if(mButtonType == ButtonTypeRadio)
    {
        mStateOn = true;
        messageSiblings(mRadioGroup);
    }
    setUpdate();

   // Update the console variable:
   if ( mConsoleVariable[0] )
      Con::setBoolVariable( mConsoleVariable, mStateOn );

    onClick_callback();
    Parent::onAction();
}

//-----------------------------------------------------------------------------

void GuiButtonBaseCtrl::onMessage( GuiControl *sender, S32 msg )
{
	Parent::onMessage(sender, msg);
	if( mRadioGroup == msg && mButtonType == ButtonTypeRadio )
	{
		setUpdate();
		mStateOn = ( sender == this );

		// Update the console variable:
		if ( mConsoleVariable[0] )
			Con::setBoolVariable( mConsoleVariable, mStateOn );
	}
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, performClick, void, (),,
   "Simulate a click on the button.\n"
   "This method will trigger the button's action just as if the button had been pressed by the "
   "user.\n\n" )
{
   object->onAction();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, setText, void, ( const char* text ),,
   "Set the text displayed on the button's label.\n"
   "@param text The text to display as the button's text label.\n"
   "@note Not all buttons render text labels.\n\n"
   "@see getText\n"
   "@see setTextID\n" )
{
   object->setText( text );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, setTextID, void, ( const char* id ),,
   "Set the text displayed on the button's label using a string from the string table "
   "assigned to the control.\n\n"
   "@param id Name of the variable that contains the integer string ID.  Used to look up "
      "string in table.\n\n"
   "@note Not all buttons render text labels.\n\n"
   "@see setText\n"
   "@see getText\n"
   "@see GuiControl::langTableMod\n"
   "@see LangTable\n\n"
   "@ref Gui_i18n" )
{
	object->setTextID( id );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, getText, const char*, (),,
   "Get the text display on the button's label (if any).\n\n"
   "@return The button's label." )
{
   return object->getText( );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, setStateOn, void, ( bool isOn ), ( true ),
   "For toggle or radio buttons, set whether the button is currently activated or not.  For radio buttons, "
   "toggling a button on will toggle all other radio buttons in its group to off.\n\n"
   "@param isOn If true, the button will be toggled on (if not already); if false, it will be toggled off.\n\n"
   "@note Toggling the state of a button with this method will <em>not</em> not trigger the action associated with the "
      "button.  To do that, use performClick()." )
{
   object->setStateOn( isOn );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiButtonBaseCtrl, resetState, void, (),,
   "Reset the mousing state of the button.\n\n"
   "This method should not generally be called." )
{
   object->resetState();
}
