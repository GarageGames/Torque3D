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

#ifndef _GUIWINDOWCTRL_H_
#define _GUIWINDOWCTRL_H_

#ifndef _GUICONTAINER_H_
   #include "gui/containers/guiContainer.h"
#endif


/// @addtogroup gui_container_group Containers
///
/// @ingroup gui_group Gui System
/// @{
class GuiWindowCtrl : public GuiContainer
{
	public:
   
		typedef GuiContainer Parent;
      
   protected:
   
      enum
      {
         /// Pixel distance across which snapping takes effect.
         SnapDistance = 12
      };

      /// Base indices for the button bitmap rects.  Each button
      /// bitmap is defined in each of the BitmapStates.
      enum BitmapIndices
      {
         BmpClose,
         BmpMaximize,
         BmpNormal,
         BmpMinimize,

         BmpCount
      };

      /// Button bitmap states.
      enum BitmapStates
      {
         BmpDefault = 0,
         BmpHilite,
         BmpDown,

         BmpStates
      };

      /// Indices for non-button bitmap rects.
      enum
      {
         BorderTopLeftKey = 12,
         BorderTopRightKey,
         BorderTopKey,
         BorderTopLeftNoKey,
         BorderTopRightNoKey,
         BorderTopNoKey,
         BorderLeft,
         BorderRight,
         BorderBottomLeft,
         BorderBottom,
         BorderBottomRight,
         NumBitmaps
      };
      
      /// Window Edge Bit Masks
      ///
      /// Edges can be combined to create a mask of multiple edges.  
      /// This is used for hit detection throughout this class.
      enum Edges
      {
         edgeNone   = 0,      ///< No Edge
         edgeTop    = BIT(1), ///< Top Edge
         edgeLeft   = BIT(2), ///< Left Edge
         edgeRight  = BIT(3), ///< Right Edge
         edgeBottom = BIT(4)  ///< Bottom Edge
      };

      /// @name Flags
      /// @{
		
      /// Allow resizing width of window.
      bool mResizeWidth;
      
      /// Allow resizing height of window.
      bool mResizeHeight;
      
      /// Allow moving window.
      bool mCanMove;
      
      /// Display close button.
      bool mCanClose;
      
      /// Display minimize button.
      bool mCanMinimize;
      
      /// Display maximize button.
      bool mCanMaximize;
      
      ///
      bool mCanCollapse;
      
      bool mCanDock; ///< Show a docking button on the title bar?
      bool mEdgeSnap; ///< Should this window snap to other windows edges?
            
      /// @}
      
      bool mCloseButtonPressed;
      bool mMinimizeButtonPressed;
      bool mMaximizeButtonPressed;

		bool mRepositionWindow;
		bool mResizeWindow;
		bool mSnapSignal;

      StringTableEntry mCloseCommand;
      
      /// Window title string.
      String mText;
      
      S32 mResizeEdge; ///< Resizing Edges Mask (See Edges Enumeration)

      S32 mTitleHeight;

      F32 mResizeMargin;

      bool mMouseMovingWin;
      bool mMouseResizeWidth;
      bool mMouseResizeHeight;
      bool mMinimized;
      bool mMaximized;
		
		Point2I mMousePosition;
      Point2I mMouseDownPosition;
      RectI mOrigBounds;
      RectI mStandardBounds;

      RectI mCloseButton;
      RectI mMinimizeButton;
      RectI mMaximizeButton;
      S32 mMinimizeIndex;
      S32 mTabIndex;

      void positionButtons(void);

      RectI *mBitmapBounds;  //bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
      GFXTexHandle mTextureObject;
      
      /// @name Collapsing
      /// @{

      typedef Vector< GuiWindowCtrl *>	CollapseGroupNumVec;
      
		S32 mCollapseGroup;
		S32 mCollapseGroupNum;
		S32 mPreCollapsedYExtent;
		S32 mPreCollapsedYMinExtent;

		bool mIsCollapsed;
		bool mIsMouseResizing;

		S32 getCollapseGroupNum() { return mCollapseGroupNum; }

		void moveFromCollapseGroup();
		void moveWithCollapseGroup(Point2I windowPosition);
	   
		bool resizeCollapseGroup(bool resizeX, bool resizeY, Point2I resizePos, Point2I resizeWidth);
		void refreshCollapseGroups();

      void handleCollapseGroup();

      /// @}
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onClose, () );
      DECLARE_CALLBACK( void, onMinimize, () );
      DECLARE_CALLBACK( void, onMaximize, () );
      DECLARE_CALLBACK( void, onCollapse, () );
      DECLARE_CALLBACK( void, onRestore, () );
      
      /// @}

   public:
   
      GuiWindowCtrl();

      bool isMinimized(S32 &index);

      virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);

      void setFont(S32 fntTag);

      void setCloseCommand(const char *newCmd);

      GuiControl* findHitControl (const Point2I &pt, S32 initialLayer = -1 );
      S32 findHitEdges( const Point2I &globalPoint );
      void getSnappableWindows( Vector<GuiWindowCtrl*> &windowOutVector, bool canCollapse = false );
      bool resize( const Point2I &newPosition, const Point2I &newExtent );

      //only cycle tabs through the current window, so overwrite the method
      GuiControl* findNextTabable(GuiControl *curResponder, bool firstCall = true);
      GuiControl* findPrevTabable(GuiControl *curResponder, bool firstCall = true);

      S32 getTabIndex(void) { return mTabIndex; }
      void selectWindow(void);

      ////
      const RectI getClientRect();

      /// Mutators for window properties from code.
      /// Using setDataField is a bit overkill.
      void setMobility( bool canMove, bool canClose, bool canMinimize, bool canMaximize, bool canDock, bool edgeSnap )
      {
         mCanMove = canMove;
         mCanClose = canClose;
         mCanMinimize = canMinimize;
         mCanMaximize = canMaximize;
         mCanDock = canDock;
         mEdgeSnap = edgeSnap;
      }

      /// Mutators for window properties from code.
      /// Using setDataField is a bit overkill.
      void setCanResize( bool canResizeWidth, bool canResizeHeight )
      {
         mResizeWidth = canResizeWidth;
         mResizeHeight = canResizeHeight;
      }

      /// Set the text on the window title bar.
      void setText( const String& text )
      {
         mText = text;
      }
      
      /// @name Collapsing
      /// @{
      
      void setCollapseGroup( bool state );
		void toggleCollapseGroup();
		void moveToCollapseGroup( GuiWindowCtrl* hitWindow, bool orientation );
      
      /// @}

      // GuiContainer.
      virtual bool onWake();
      virtual void onSleep();
		virtual void parentResized(const RectI &oldParentRect, const RectI &newParentRect);
      virtual void onMouseDown(const GuiEvent &event);
      virtual void onMouseDragged(const GuiEvent &event);
      virtual void onMouseUp(const GuiEvent &event);
		virtual void onMouseMove(const GuiEvent &event);
      virtual bool onKeyDown(const GuiEvent &event);
      virtual void onRender(Point2I offset, const RectI &updateRect);

      DECLARE_CONOBJECT( GuiWindowCtrl );
      DECLARE_DESCRIPTION( "A control that shows an independent window inside the canvas." );

      static void initPersistFields();
};
/// @}

#endif //_GUI_WINDOW_CTRL_H
