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

#include "gui/utility/guiInputCtrl.h"
#include "sim/actionMap.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiInputCtrl);

ConsoleDocClass( GuiInputCtrl,
	"@brief A control that locks the mouse and reports all keyboard input events to script.\n\n"

	"This is useful for implementing custom keyboard handling code, and most commonly "
	"used in Torque for a menu that allows a user to remap their in-game controls\n\n "

	"@tsexample\n"
	"new GuiInputCtrl(OptRemapInputCtrl)\n"
	"{\n"
	"	lockMouse = \"0\";\n"
	"	position = \"0 0\";\n"
	"	extent = \"64 64\";\n"
	"	minExtent = \"8 8\";\n"
	"	horizSizing = \"center\";\n"
	"	vertSizing = \"bottom\";\n"
	"	profile = \"GuiInputCtrlProfile\";\n"
	"	visible = \"1\";\n"
	"	active = \"1\";\n"
	"	tooltipProfile = \"GuiToolTipProfile\";\n"
	"	hovertime = \"1000\";\n"
	"	isContainer = \"0\";\n"
	"	canSave = \"1\";\n"
	"	canSaveDynamicFields = \"0\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@see GuiMouseEventCtrl\n"

	"@ingroup GuiUtil\n");

//------------------------------------------------------------------------------

void GuiInputCtrl::initPersistFields()
{
   

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------

bool GuiInputCtrl::onWake()
{
   // Set the default profile on start-up:
   if( !mProfile )
   {
      GuiControlProfile* profile;
      Sim::findObject( "GuiInputCtrlProfile", profile);
      if( profile )
         setControlProfile( profile );
   }

   if ( !Parent::onWake() )
      return( false );

   if( !smDesignTime )
      mouseLock();
      
   setFirstResponder();

   return( true );
}


//------------------------------------------------------------------------------
void GuiInputCtrl::onSleep()
{
   Parent::onSleep();
   mouseUnlock();
   clearFirstResponder();
}


//------------------------------------------------------------------------------
static bool isModifierKey( U16 keyCode )
{
   switch ( keyCode )
   {
      case KEY_LCONTROL:
      case KEY_RCONTROL:
      case KEY_LALT:
      case KEY_RALT:
      case KEY_LSHIFT:
      case KEY_RSHIFT:
         return( true );
   }

   return( false );
}

IMPLEMENT_CALLBACK( GuiInputCtrl, onInputEvent, void, (const char* device, const char* action, bool state ),
														  ( device, action, state),
	"@brief Callback that occurs when an input is triggered on this control\n\n"
	"@param device The device type triggering the input, such as keyboard, mouse, etc\n"
	"@param action The actual event occuring, such as a key or button\n"
	"@param state True if the action is being pressed, false if it is being release\n\n"
);

//------------------------------------------------------------------------------
bool GuiInputCtrl::onInputEvent( const InputEventInfo &event )
{
   // TODO - add POV support...
   if ( event.action == SI_MAKE )
   {
      if ( event.objType == SI_BUTTON
        || event.objType == SI_POV
        || ( ( event.objType == SI_KEY ) && !isModifierKey( event.objInst ) ) )
      {
         char deviceString[32];
         if ( !ActionMap::getDeviceName( event.deviceType, event.deviceInst, deviceString ) )
            return( false );

         const char* actionString = ActionMap::buildActionString( &event );

		 //Con::executef( this, "onInputEvent", deviceString, actionString, "1" );
		 onInputEvent_callback(deviceString, actionString, 1);

         return( true );
      }
   }
   else if ( event.action == SI_BREAK )
   {
      if ( ( event.objType == SI_KEY ) && isModifierKey( event.objInst ) )
      {
         char keyString[32];
         if ( !ActionMap::getKeyString( event.objInst, keyString ) )
            return( false );

         //Con::executef( this, "onInputEvent", "keyboard", keyString, "0" );
		 onInputEvent_callback("keyboard", keyString, 0);

         return( true );
      }
   }

   return( false );
}
