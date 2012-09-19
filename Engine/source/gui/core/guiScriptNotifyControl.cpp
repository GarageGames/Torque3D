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

#include "gui/core/guiScriptNotifyControl.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiScriptNotifyCtrl);

ConsoleDocClass( GuiScriptNotifyCtrl,
	"@brief A control which adds several reactions to other GUIs via callbacks.\n\n"

	"GuiScriptNotifyCtrl does not exist to render anything. When parented or made a child of "
	"other controls, you can toggle flags on or off to make use of its specialized callbacks. "
	"Normally these callbacks are used as utility functions by the GUI Editor, or other container "
	"classes. However, for very fancy GUI work where controls interact with each other "
	"constantly, this is a handy utility to make use of.\n\n "

	"@tsexample\n"
	"// Common member fields left out for sake of example\n"
	"new GuiScriptNotifyCtrl()\n"
	"{\n"
	"	onChildAdded = \"0\";\n"
	"	onChildRemoved = \"0\";\n"
	"	onChildResized = \"0\";\n"
	"	onParentResized = \"0\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@ingroup GuiUtil\n");

GuiScriptNotifyCtrl::GuiScriptNotifyCtrl()
{
   mOnChildAdded = false;
   mOnChildRemoved = false;
   mOnResize = false;
   mOnChildResized = false;
   mOnParentResized = false;
}

GuiScriptNotifyCtrl::~GuiScriptNotifyCtrl()
{
}

void GuiScriptNotifyCtrl::initPersistFields()
{
   // Callbacks Group
   addGroup("Callbacks");
   addField("onChildAdded", TypeBool, Offset( mOnChildAdded, GuiScriptNotifyCtrl ), "Enables/disables onChildAdded callback" );
   addField("onChildRemoved", TypeBool, Offset( mOnChildRemoved, GuiScriptNotifyCtrl ), "Enables/disables onChildRemoved callback" );
   addField("onChildResized", TypeBool, Offset( mOnChildResized, GuiScriptNotifyCtrl ), "Enables/disables onChildResized callback" );
   addField("onParentResized", TypeBool, Offset( mOnParentResized, GuiScriptNotifyCtrl ), "Enables/disables onParentResized callback" );
   addField("onResize", TypeBool, Offset( mOnResize, GuiScriptNotifyCtrl ), "Enables/disables onResize callback" );
   addField("onLoseFirstResponder", TypeBool, Offset( mOnLoseFirstResponder, GuiScriptNotifyCtrl ), "Enables/disables onLoseFirstResponder callback" );
   addField("onGainFirstResponder", TypeBool, Offset( mOnGainFirstResponder, GuiScriptNotifyCtrl ), "Enables/disables onGainFirstResponder callback" );
   endGroup("Callbacks");

   Parent::initPersistFields();
}

IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onResize, void, ( SimObjectId ID ), ( ID ),
	"Called when this GUI is resized.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onChildAdded, void, ( SimObjectId ID, SimObjectId childID ), ( ID, childID ),
	"Called when a child is added to this GUI.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
	"@param childID Unique object ID of child being added.\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onChildRemoved, void, ( SimObjectId ID, SimObjectId childID ), ( ID, childID ),
	"Called when a child is removed from this GUI.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
	"@param childID Unique object ID of child being removed.\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onChildResized, void, ( SimObjectId ID, SimObjectId childID ), ( ID, childID ),
	"Called when a child is of this GUI is being resized.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
	"@param childID Unique object ID of child being resized.\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onParentResized, void, ( SimObjectId ID ), ( ID ),
	"Called when this GUI's parent is resized.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onLoseFirstResponder, void, ( SimObjectId ID ), ( ID ),
	"Called when this GUI loses focus.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);
IMPLEMENT_CALLBACK( GuiScriptNotifyCtrl, onGainFirstResponder, void, ( SimObjectId ID ), ( ID ),
	"Called when this GUI gains focus.\n\n"
	"@param ID Unique object ID assigned when created (%this in script).\n"
);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
void GuiScriptNotifyCtrl::onChildAdded( GuiControl *child )
{
   Parent::onChildAdded( child );

   // Call Script.
   if( mOnChildAdded )
      onChildAdded_callback(getId(), child->getId());
}

void GuiScriptNotifyCtrl::onChildRemoved( GuiControl *child )
{
   Parent::onChildRemoved( child );

   // Call Script.
   if( mOnChildRemoved )
      onChildRemoved_callback(getId(), child->getId());
}
//----------------------------------------------------------------

bool GuiScriptNotifyCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if( !Parent::resize( newPosition, newExtent ) )
      return false;

   // Call Script.
   if( mOnResize )
      onResize_callback(getId());

   return true;
}

void GuiScriptNotifyCtrl::childResized(GuiScriptNotifyCtrl *child)
{
   Parent::childResized( child );

   // Call Script.
   if( mOnChildResized )
      onChildResized_callback(getId(), child->getId());
}

void GuiScriptNotifyCtrl::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   Parent::parentResized( oldParentRect, newParentRect );

   // Call Script.
   if( mOnParentResized )
      onParentResized_callback(getId());
}
 
void GuiScriptNotifyCtrl::onLoseFirstResponder()
{
   Parent::onLoseFirstResponder();

   // Call Script.
   if( mOnLoseFirstResponder )
      onLoseFirstResponder_callback(getId());
}

void GuiScriptNotifyCtrl::setFirstResponder( GuiControl* firstResponder )
{
   Parent::setFirstResponder( firstResponder );

   // Call Script.
   if( mOnGainFirstResponder && isFirstResponder() )
      onGainFirstResponder_callback(getId());
}

void GuiScriptNotifyCtrl::setFirstResponder()
{
   Parent::setFirstResponder();

   // Call Script.
   if( mOnGainFirstResponder && isFirstResponder() )
      onGainFirstResponder_callback(getId());
}

void GuiScriptNotifyCtrl::onMessage(GuiScriptNotifyCtrl *sender, S32 msg)
{
   Parent::onMessage( sender, msg );
}

void GuiScriptNotifyCtrl::onDialogPush()
{
   Parent::onDialogPush();
}

void GuiScriptNotifyCtrl::onDialogPop()
{
   Parent::onDialogPop();
}


//void GuiScriptNotifyCtrl::onMouseUp(const GuiEvent &event)
//{
//}
//
//void GuiScriptNotifyCtrl::onMouseDown(const GuiEvent &event)
//{
//}
//
//void GuiScriptNotifyCtrl::onMouseMove(const GuiEvent &event)
//{
//}
//
//void GuiScriptNotifyCtrl::onMouseDragged(const GuiEvent &event)
//{
//}
//
//void GuiScriptNotifyCtrl::onMouseEnter(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onMouseLeave(const GuiEvent &)
//{
//}
//
//bool GuiScriptNotifyCtrl::onMouseWheelUp( const GuiEvent &event )
//{
//}
//
//bool GuiScriptNotifyCtrl::onMouseWheelDown( const GuiEvent &event )
//{
//}
//
//void GuiScriptNotifyCtrl::onRightMouseDown(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onRightMouseUp(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onRightMouseDragged(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onMiddleMouseDown(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onMiddleMouseUp(const GuiEvent &)
//{
//}
//
//void GuiScriptNotifyCtrl::onMiddleMouseDragged(const GuiEvent &)
//{
//}
//void GuiScriptNotifyCtrl::onMouseDownEditor(const GuiEvent &event, Point2I offset)
//{
//}
//void GuiScriptNotifyCtrl::onRightMouseDownEditor(const GuiEvent &event, Point2I offset)
//{
//}

//bool GuiScriptNotifyCtrl::onKeyDown(const GuiEvent &event)
//{
//  if ( Parent::onKeyDown( event ) )
//     return true;
//}
//
//bool GuiScriptNotifyCtrl::onKeyRepeat(const GuiEvent &event)
//{
//   // default to just another key down.
//   return onKeyDown(event);
//}
//
//bool GuiScriptNotifyCtrl::onKeyUp(const GuiEvent &event)
//{
//  if ( Parent::onKeyUp( event ) )
//     return true;
//}
