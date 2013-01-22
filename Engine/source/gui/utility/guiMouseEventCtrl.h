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

#ifndef _GUIMOUSEEVENTCTRL_H_
#define _GUIMOUSEEVENTCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif


class GuiMouseEventCtrl : public GuiControl
{
   private:
      typedef  GuiControl     Parent;
      void sendMouseEvent(const char * name, const GuiEvent &);

      // field info
      bool        mLockMouse;

   public:

      GuiMouseEventCtrl();

	  DECLARE_CALLBACK( void, onMouseDown, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
  	  DECLARE_CALLBACK( void, onMouseUp, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
  	  DECLARE_CALLBACK( void, onMouseMove, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
  	  DECLARE_CALLBACK( void, onMouseDragged, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
  	  DECLARE_CALLBACK( void, onMouseEnter, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
  	  DECLARE_CALLBACK( void, onMouseLeave, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
	  DECLARE_CALLBACK( void, onRightMouseDown, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
	  DECLARE_CALLBACK( void, onRightMouseUp, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));
	  DECLARE_CALLBACK( void, onRightMouseDragged, ( U8 modifier, Point2I mousePoint,U8 mouseClickCount ));

      // GuiControl
      void onMouseDown(const GuiEvent & event);
      void onMouseUp(const GuiEvent & event);
      void onMouseMove(const GuiEvent & event);
      void onMouseDragged(const GuiEvent & event);
      void onMouseEnter(const GuiEvent & event);
      void onMouseLeave(const GuiEvent & event);
      void onRightMouseDown(const GuiEvent & event);
      void onRightMouseUp(const GuiEvent & event);
      void onRightMouseDragged(const GuiEvent & event);

      static void initPersistFields();

      DECLARE_CONOBJECT( GuiMouseEventCtrl );
      DECLARE_CATEGORY( "Gui Other Script" );
      DECLARE_DESCRIPTION( "A control that relays all mouse events to script." );
};

#endif
