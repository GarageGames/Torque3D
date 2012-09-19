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
#include "gui/controls/guiDecoyCtrl.h"
#include "gui/buttons/guiButtonBaseCtrl.h"

#include "console/consoleTypes.h"
#include "gfx/primBuilder.h"

//-----------------------------------------------------------------------------
// GuiDecoyCtrl
//-----------------------------------------------------------------------------

/*
So far this control has been designed in mind solely for button controls. I'm pretty sure
it can be used for other things, but to do anything more in depth; it has to be extended.
Make sure you know a little about how guiCanvas hands out signals to gui controls before you tinker
in this class.

Been thinking about this class a little more. I tried pretty hard to protect this class into being 
guiControl like agnostic. But I ended up adding a check specifically for buttons in the 
onMouseUp function. Its been protected with a dynamic_cast and a NULL check; but in the end, the only way
too solve the main problem, that GuiCanvas cannot process more than one mouse action for more than one
gui control at a time, is for it to get a rewrite.
*/

IMPLEMENT_CONOBJECT(GuiDecoyCtrl);

ConsoleDocClass( GuiDecoyCtrl,
				"@brief Designed soley for buttons, primarily used in editor.\n\n"
				"Currently editor use only, no real application without extension.\n\n "
				"@internal");

GuiDecoyCtrl::GuiDecoyCtrl() : mIsDecoy(true),
							   mMouseOver(false),
							   mDecoyReference(NULL)
{
}

GuiDecoyCtrl::~GuiDecoyCtrl()
{
}

void GuiDecoyCtrl::initPersistFields()
{
   addField("isDecoy",       TypeBool,         Offset(mIsDecoy, GuiDecoyCtrl), "Sets this control to decoy mode");

   Parent::initPersistFields();
}

void GuiDecoyCtrl::onMouseUp(const GuiEvent &event)
{
	mouseUnlock();
	setUpdate();  

	//this code is pretty hacky. right now there is no way that guiCanvas will allow sending more than
    //one signal to one gui control at a time. 
	if(mIsDecoy == true)
	{
		mVisible = false;

		GuiControl *parent = getParent();
		Point2I localPoint = parent->globalToLocalCoord(event.mousePoint);
		GuiControl *tempControl = parent->findHitControl(localPoint);

		//the decoy control has the responsibility of keeping track of the decoyed controls status
		if( mDecoyReference != NULL && tempControl == mDecoyReference)
			tempControl->onMouseUp(event);
		else if(mDecoyReference != NULL && tempControl != mDecoyReference)
		{
			//as explained earlier, this control was written in the mindset for buttons.
			//nothing bad should ever happen if not a button due to the checks in this function though.
			GuiButtonBaseCtrl *unCastCtrl = NULL;
			unCastCtrl = dynamic_cast<GuiButtonBaseCtrl *>( mDecoyReference );
			if(unCastCtrl != NULL)
				unCastCtrl->resetState();
		}
		mVisible = true;
	}
}

void GuiDecoyCtrl::onMouseDown(const GuiEvent &event)
{
	if ( !mVisible || !mAwake )
      return;
	
	mouseLock();

	if(mIsDecoy == true)
	{
		mVisible = false;

		GuiControl *parent = getParent();
		Point2I localPoint = parent->globalToLocalCoord(event.mousePoint);

		GuiControl *tempControl = parent->findHitControl(localPoint);
		tempControl->onMouseDown(event);

		mVisible = true;
	}
	
   execConsoleCallback();
   setUpdate();
}

void GuiDecoyCtrl::onMouseMove(const GuiEvent &event)
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      parent->onMouseMove( event );

   Point2I localPoint = parent->globalToLocalCoord(event.mousePoint);

   //also pretty hacky. since guiCanvas, *NOT* GuiControl, distributes the calls for onMouseEnter
   //and onMouseLeave, we simulate those calls here through a series of checks.
   if(mIsDecoy == true)
   {
	    mVisible = false;
		GuiControl *parent = getParent();
		GuiControl *tempControl = parent->findHitControl(localPoint);

		//the decoy control has the responsibility of keeping track of the decoyed controls status
		if(mMouseOverDecoy == false && mDecoyReference != NULL)
		{
			tempControl->onMouseEnter(event);
			mMouseOverDecoy = true;
		}
		else if(tempControl != mDecoyReference && mDecoyReference != NULL)
		{
			mDecoyReference->onMouseLeave(event);
			mMouseOverDecoy = false;
		}
		
		mDecoyReference = tempControl;
		mVisible = true;
   }
}

void GuiDecoyCtrl::onMouseDragged(const GuiEvent &event)
{
}

void GuiDecoyCtrl::onMouseEnter(const GuiEvent &event)
{
   if ( !mVisible || !mAwake )
      return;

   setUpdate();
   Con::executef( this , "onMouseEnter" );
   mMouseOver = true;
}

void GuiDecoyCtrl::onMouseLeave(const GuiEvent &event)
{
   if ( !mVisible || !mAwake )
      return;

   setUpdate();
   Con::executef( this , "onMouseLeave" );
   mMouseOver = false;
}

bool GuiDecoyCtrl::onMouseWheelUp( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelUp( event );
   else
      return false;
}

bool GuiDecoyCtrl::onMouseWheelDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelDown( event );
   else
      return false;
}

void GuiDecoyCtrl::onRightMouseDown(const GuiEvent &)
{
}

void GuiDecoyCtrl::onRightMouseUp(const GuiEvent &)
{
}

void GuiDecoyCtrl::onRightMouseDragged(const GuiEvent &)
{
}

void GuiDecoyCtrl::onMiddleMouseDown(const GuiEvent &)
{
}

void GuiDecoyCtrl::onMiddleMouseUp(const GuiEvent &)
{
}

void GuiDecoyCtrl::onMiddleMouseDragged(const GuiEvent &)
{
}