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

#ifndef _GUIINPUTCTRL_H_
#define _GUIINPUTCTRL_H_

#ifndef _GUIMOUSEEVENTCTRL_H_
   #include "gui/utility/guiMouseEventCtrl.h"
#endif


/// A control that locks the mouse and reports all keyboard input events
/// to script.  This is useful for implementing custom keyboard handling code.
class GuiInputCtrl : public GuiMouseEventCtrl
{
   public:

      typedef GuiMouseEventCtrl Parent;
   
      // GuiControl.
      virtual bool onWake();
      virtual void onSleep();

      virtual bool onInputEvent( const InputEventInfo &event );
      
      static void initPersistFields();

      DECLARE_CONOBJECT(GuiInputCtrl);
      DECLARE_CATEGORY( "Gui Other Script" );
      DECLARE_DESCRIPTION( "A control that locks the mouse and reports all keyboard input events to script." );

	  DECLARE_CALLBACK( void, onInputEvent, ( const char* device, const char* action, bool state ));
};

#endif // _GUI_INPUTCTRL_H
