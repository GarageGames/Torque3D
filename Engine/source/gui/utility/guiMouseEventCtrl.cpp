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

#include "gui/utility/guiMouseEventCtrl.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiMouseEventCtrl);

ConsoleDocClass( GuiMouseEventCtrl,
   "@brief Used to overlaps a 'hot region' where you want to catch inputs with and have specific events occur based on individual callbacks.\n\n"

   "Mouse event callbacks supported by this control are: onMouseUp, onMouseDown, onMouseMove, onMouseDragged, onMouseEnter, onMouseLeave,\n"
   "onRightMouseDown, onRightMouseUp and onRightMouseDragged.\n\n"

   "@tsexample\n"
   "new GuiMouseEventCtrl()\n"
   "{\n"
   "	lockMouse = \"0\";\n"
   "	//Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n\n"

   "@ingroup GuiCore\n"
);


IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseDown, void, ( S32 modifier, Point2I mousePoint, S32 mouseClickCount ),
														  ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse is pressed down while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse was pressed down in this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseDown(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseUp, void, ( S32 modifier, Point2I mousePoint, S32 mouseClickCount ),
													    ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse is released while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse was released in this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseUp(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseMove, void, ( S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse is moved (without dragging) while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse was moved in this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseMove(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseDragged, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse is dragged while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse was dragged in this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseDragged(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseEnter, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse enters this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse entered this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseEnter(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onMouseLeave, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse leaves this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Mouse left this control, causing the callback\n"
   "GuiMouseEventCtrl::onMouseLeave(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onRightMouseDown, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the right mouse button is pressed while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Right mouse button was pressed in this control, causing the callback\n"
   "GuiMouseEventCtrl::onRightMouseDown(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onRightMouseUp, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the right mouse button is released while in this control.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Right mouse button was released in this control, causing the callback\n"
   "GuiMouseEventCtrl::onRightMouseUp(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMouseEventCtrl, onRightMouseDragged, void, (  S32 modifier, Point2I mousePoint, S32 mouseClickCount  ),
												   ( modifier, mousePoint, mouseClickCount ),
   "@brief Callback that occurs whenever the mouse is dragged in this control while the right mouse button is pressed.\n\n"
   "@param modifier Key that was pressed during this callback. Values are:\n\n" 
   "$EventModifier::RSHIFT\n\n"
   "$EventModifier::SHIFT\n\n"
   "$EventModifier::LCTRL\n\n"
   "$EventModifier::RCTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::CTRL\n\n"
   "$EventModifier::RALT\n\n"
   "$EventModifier::ALT\n\n"
   "@param mousePoint X/Y location of the mouse point\n"
   "@param mouseClickCount How many mouse clicks have occured for this event\n\n"
   "@tsexample\n"
   "// Right mouse button was dragged in this control, causing the callback\n"
   "GuiMouseEventCtrl::onRightMouseDragged(%this,%modifier,%mousePoint,%mouseClickCount)\n"
   "{\n"
   "	// Code to call when a mouse event occurs.\n"
   "}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

GuiMouseEventCtrl::GuiMouseEventCtrl()
{
   mLockMouse = false;
}

//------------------------------------------------------------------------------
void GuiMouseEventCtrl::sendMouseEvent(const char * name, const GuiEvent & event)
{
   if(dStricmp(name,"onMouseDown") == 0)
	   onMouseDown_callback(event.modifier, event.mousePoint, event.mouseClickCount);
   else if(dStricmp(name,"onMouseUp") == 0)
	   onMouseUp_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onMouseMove") == 0)
		onMouseMove_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onMouseDragged") == 0)
		onMouseDragged_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onMouseEnter") == 0)
		onMouseEnter_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onMouseLeave") == 0)
		onMouseLeave_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onRightMouseDown") == 0)
		onRightMouseDown_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onRightMouseUp") == 0)
		onRightMouseUp_callback(event.modifier, event.mousePoint, event.mouseClickCount);
	else if(dStricmp(name,"onRightMouseDragged") == 0)
		onRightMouseDragged_callback(event.modifier, event.mousePoint, event.mouseClickCount);
}

//------------------------------------------------------------------------------
void GuiMouseEventCtrl::initPersistFields()
{
   addGroup( "Input" );
   
      addField("lockMouse", TypeBool, Offset(mLockMouse, GuiMouseEventCtrl),
         "Whether the control should lock the mouse between up and down button events." );
      
   endGroup( "Input" );

   Parent::initPersistFields();

   Con::setIntVariable("$EventModifier::LSHIFT",      SI_LSHIFT);
   Con::setIntVariable("$EventModifier::RSHIFT",      SI_RSHIFT);
   Con::setIntVariable("$EventModifier::SHIFT",       SI_SHIFT);
   Con::setIntVariable("$EventModifier::LCTRL",       SI_LCTRL);
   Con::setIntVariable("$EventModifier::RCTRL",       SI_RCTRL);
   Con::setIntVariable("$EventModifier::CTRL",        SI_CTRL);
   Con::setIntVariable("$EventModifier::LALT",        SI_LALT);
   Con::setIntVariable("$EventModifier::RALT",        SI_RALT);
   Con::setIntVariable("$EventModifier::ALT",         SI_ALT);
}

//------------------------------------------------------------------------------
void GuiMouseEventCtrl::onMouseDown(const GuiEvent & event)
{
   if(mLockMouse)
      mouseLock();
   sendMouseEvent("onMouseDown", event);
}

void GuiMouseEventCtrl::onMouseUp(const GuiEvent & event)
{
   if(mLockMouse)
      mouseUnlock();
   sendMouseEvent("onMouseUp", event);
}

void GuiMouseEventCtrl::onMouseMove(const GuiEvent & event)
{
   sendMouseEvent("onMouseMove", event);
}

void GuiMouseEventCtrl::onMouseDragged(const GuiEvent & event)
{
   sendMouseEvent("onMouseDragged", event);
}

void GuiMouseEventCtrl::onMouseEnter(const GuiEvent & event)
{
   sendMouseEvent("onMouseEnter", event);
}

void GuiMouseEventCtrl::onMouseLeave(const GuiEvent & event)
{
   sendMouseEvent("onMouseLeave", event);
}

void GuiMouseEventCtrl::onRightMouseDown(const GuiEvent & event)
{
   if(mLockMouse)
      mouseLock();
   sendMouseEvent("onRightMouseDown", event);
}

void GuiMouseEventCtrl::onRightMouseUp(const GuiEvent & event)
{
   if(mLockMouse)
      mouseUnlock();
   sendMouseEvent("onRightMouseUp", event);
}

void GuiMouseEventCtrl::onRightMouseDragged(const GuiEvent & event)
{
   sendMouseEvent("onRightMouseDragged", event);
}