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

#ifndef _GUIDRAGANDDROPCTRL_H_
#define _GUIDRAGANDDROPCTRL_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif
#ifndef _GFXDEVICE_H_
   #include "gfx/gfxDevice.h"
#endif
#ifndef _CONSOLE_H_
   #include "console/console.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif


/// A special control that implements drag-and-drop behavior.
///
class GuiDragAndDropControl : public GuiControl
{
   public:

      typedef GuiControl Parent;
   
   private:
   
      /// The mouse down offset from the upper left of the control.
      Point2I mOffset;
      
      /// If true, the control deletes itself when the left mouse button is released.
      bool mDeleteOnMouseUp;

      /// Controls may want to react when they are dragged over, entered or exited.
      SimObjectPtr<GuiControl> mLastTarget;
      
      GuiControl* findDragTarget(Point2I mousePoint, const char* method);
      
      Point2I getDropPoint() const
      {
         return getPosition() + (getExtent() / 2);
      }

   public:
   
      GuiDragAndDropControl() {}

      void startDragging(Point2I offset = Point2I(0, 0));

      // GuiControl.
      virtual void onMouseDown(const GuiEvent& event);
      virtual void onMouseDragged(const GuiEvent& event);
      virtual void onMouseUp(const GuiEvent& event);

      static void initPersistFields();

      DECLARE_CONOBJECT( GuiDragAndDropControl );
      DECLARE_CATEGORY( "Gui Other" );
      DECLARE_DESCRIPTION( "A special control that implements drag&drop behavior.\n"
                           "The control will notify other controls as it moves across the canvas.\n"
                           "Content can be attached through dynamic fields or child objects." );
};

#endif