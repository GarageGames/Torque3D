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
#ifndef _GUI_SPLTCONTAINER_H_
#define _GUI_SPLTCONTAINER_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif
#ifndef _GUI_PANEL_H_
#include "gui/containers/guiPanel.h"
#endif
#ifndef _PLATFORMINPUT_H_
#include "platform/platformInput.h"
#endif



/// @addtogroup gui_container_group Containers
///
/// @ingroup gui_group Gui System
/// @{
class  GuiSplitContainer : public GuiContainer
{
   typedef GuiContainer Parent;
public:

   enum Orientation
   {
      Vertical = 0,
      Horizontal = 1
   };

   enum FixedPanel
   {
      None = 0,
      FirstPanel = 1,
      SecondPanel
   };   

   GuiSplitContainer();

   DECLARE_CONOBJECT( GuiSplitContainer );
   DECLARE_DESCRIPTION( "A container that splits its area between two child controls.\n"
                        "The split ratio can be dynamically adjusted with a handle control between the two children.\n"
                        "Splitting can be either horizontal or vertical." );

   // ConsoleObject
   static void initPersistFields();
   virtual bool onAdd();

   // GuiControl   
   virtual bool onWake();
   virtual void parentResized(const RectI &oldParentRect, const RectI &newParentRect);
   virtual bool resize( const Point2I &newPosition, const Point2I &newExtent );   
   virtual void onRender(Point2I offset, const RectI &updateRect);
   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseUp(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event); 

   virtual bool layoutControls( RectI &clientRect );
   virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
   virtual inline Point2I getSplitPoint() { return mSplitPoint; };
   /// The Splitters entire Client Rectangle, this takes into account padding of this control
   virtual inline RectI getSplitRect() { return mSplitRect; };
   virtual void solvePanelConstraints(Point2I newDragPos, GuiContainer * firstPanel, GuiContainer * secondPanel, const RectI& clientRect);
   virtual Point2I getMinExtent() const;   

protected:

   S32         mFixedPanel; 
   S32         mFixedPanelSize;
   S32         mOrientation;
   S32         mSplitterSize;
   Point2I     mSplitPoint;
   RectI       mSplitRect;
   bool        mDragging;

};

typedef GuiSplitContainer::Orientation GuiSplitOrientation;
typedef GuiSplitContainer::FixedPanel GuiSplitFixedPanel;

DefineEnumType( GuiSplitOrientation );
DefineEnumType( GuiSplitFixedPanel );

/// @}

#endif // _GUI_SPLTCONTAINER_H_