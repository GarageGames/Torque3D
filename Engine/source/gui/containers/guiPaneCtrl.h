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

#ifndef _GUIPANECTRL_H_
#define _GUIPANECTRL_H_

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


/// Collapsable pane control.
///
/// This class wraps a single child control and displays a header with caption
/// above it. If you click the header it will collapse or expand. The control
/// resizes itself based on its collapsed/expanded size.
///
/// In the GUI editor, if you just want the header you can make collapsable
/// false. The caption field lets you set the caption. It expects a bitmap
/// (from the GuiControlProfile) that contains two images - the first is
/// displayed when the control is expanded and the second is displayed when
/// it is collapsed. The header is sized based off of the first image.
class GuiPaneControl : public GuiControl
{
   public:
   
      typedef GuiControl Parent;

   protected:

      /// If true, the pane can be collapsed and extended by clicking its header.
      bool mCollapsable;
      
      /// Whether the pane is currently collapsed.
      bool mCollapsed;
      
      /// Whether to display the bitmapped pane bar behind the header text, too.
      bool mBarBehindText;
      
      ///
      Point2I mOriginalExtents;
      
      /// Text to display as the pane caption.
      String mCaption;
      
      /// String table text ID to use as caption string.
      StringTableEntry mCaptionID;
      
      ///
      Point2I mThumbSize;

      ///
      bool mMouseOver;
      
      ///
      bool mDepressed;

   public:
   
      GuiPaneControl();

      /// Return whether the pane is currently collapsed.
      bool getCollapsed() { return mCollapsed; };
      
      /// Collapse or expand the pane.
      void setCollapsed(bool isCollapsed);

      virtual void setCaptionID(S32 id);
      virtual void setCaptionID(const char *id);
      
      // GuiControl.
      virtual bool onWake();

      virtual void onMouseDown(const GuiEvent &event);
      virtual void onMouseUp(const GuiEvent &event);
      virtual void onMouseMove(const GuiEvent &event);
      virtual void onMouseLeave(const GuiEvent &event);
      virtual void onMouseEnter(const GuiEvent &event);

      virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);
      virtual void onRender(Point2I offset, const RectI &updateRect);

      static void initPersistFields();
      
      DECLARE_CONOBJECT(GuiPaneControl);
      DECLARE_CATEGORY( "Gui Containers" );
      DECLARE_DESCRIPTION( "A container that wraps a single child control displaying a header\n"
         "with a caption above it.  If enabled, the pane can be collapsed and expanded\n"
         "by clicking the header." );
};

#endif