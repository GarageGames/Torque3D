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

#ifndef _GUISCROLLCTRL_H_
#define _GUISCROLLCTRL_H_

#ifndef _GUICONTAINER_H_
   #include "gui/containers/guiContainer.h"
#endif


/// A control providing a window inside a larger client area which can be
/// scrolled using scrollbars.
///
class GuiScrollCtrl : public GuiContainer
{
   public:

      typedef GuiContainer Parent;

      enum Region
      {
         UpArrow,
         DownArrow,
         LeftArrow,
         RightArrow,
         UpPage,
         DownPage,
         LeftPage,
         RightPage,
         VertThumb,
         HorizThumb,
         None
      };
      
      enum ScrollBarBehavior
      {
         ScrollBarAlwaysOn = 0,
         ScrollBarAlwaysOff = 1,
         ScrollBarDynamic = 2
      };

   protected:

      // the scroll control uses a bitmap array to draw all its
      // graphics... these are the bitmaps it needs:
      enum BitmapIndices
      {
         BmpUp,
         BmpDown,
         BmpVThumbTopCap,
         BmpVThumb,
         BmpVThumbBottomCap,
         BmpVPage,
         BmpLeft,
         BmpRight,
         BmpHThumbLeftCap,
         BmpHThumb,
         BmpHThumbRightCap,
         BmpHPage,
         BmpResize,

         BmpCount
      };

      enum BitmapStates
      {
         BmpDefault = 0,
         BmpHilite,
         BmpDisabled,

         BmpStates
      };
      
      RectI *mBitmapBounds;  //bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
      GFXTexHandle mTextureObject;

      S32     mBorderThickness;           // this gets set per class in the constructor
      Point2I mChildMargin;               // the thickness of the margin around the child controls

      // note - it is implicit in the scroll view that the buttons all have the same
      // arrow length and that horizontal and vertical scroll bars have the
      // same thickness

      S32 mScrollBarThickness;        // determined by the width of the vertical page bmp
      S32 mScrollBarArrowBtnLength;   // determined by the height of the up arrow
      S32 mScrollBarDragTolerance;    // maximal distance from scrollbar at which a scrollbar drag is still valid

      bool mHBarEnabled;
      bool mVBarEnabled;
      bool mHasHScrollBar;
      bool mHasVScrollBar;

      Point2I mContentPos;  // the position of the content region in the control's coord system
      Point2I mContentExt;  // the extent of the content region

      Point2I mChildPos;   // the position of the upper left corner of the child control(s)
      Point2I mChildExt;

      Point2I mChildRelPos; // the relative position of the upper left content corner in
                            // the child's coordinate system - 0,0 if scrolled all the way to upper left.

      //--------------------------------------
      // for mouse dragging the thumb
      Point2I mChildRelPosAnchor; // the original childRelPos when scrolling started
      S32 mThumbMouseDelta;

      S32 mLastUpdated;

      S32 mHThumbSize;
      S32 mHThumbPos;

      S32 mVThumbSize;
      S32 mVThumbPos;

      S32 mBaseThumbSize;

      RectI mUpArrowRect;
      RectI mDownArrowRect;
      RectI mLeftArrowRect;
      RectI mRightArrowRect;
      RectI mHTrackRect;
      RectI mVTrackRect;

      //--------------------------------------
      // for determining hit area

      bool mStateDepressed; ///< Is the mouse currently depressed on a scroll region
      Region mHitRegion;    ///< Which region is hit by the mouse

      S32 mForceHScrollBar; ///< Force showing the Horizontal scrollbar
      S32 mForceVScrollBar; ///< Force showing the Vertical scrollbar
      bool mLockHorizScroll; ///< Is horizontal scrolling disabled
      bool mLockVertScroll; ///< Is vertical scrolling disabled

      bool mUseConstantHeightThumb;
      bool mWillFirstRespond;     // for automatically handling arrow keys

      /// Used internally to prevent infinite recursion.
      bool mIgnoreChildResized;   

      /// MouseWheel scroll animation
      /// @{

      /// Is currently performing a scroll animation.
      bool mAnimating; 

      /// Pixels moved per tick when performing a scroll animation.
      S32 mScrollAnimSpeed; 

      /// The target position when performing a scroll animation.
      Point2I mScrollTargetPos;

      /// Platform time of the last call to onPreRender
      S32 mLastPreRender;

      /// @}
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onScroll, () );
      
      /// @}

      Region findHitRegion(const Point2I &);

      virtual bool calcChildExtents();
      virtual void calcScrollRects(void);
      void calcThumbs();
      void scrollByRegion(Region reg);
      void scrollByMouseWheel( const GuiEvent &event );
      
      /// Tell the kids that the mouse moved (relatively)
      void updateChildMousePos();
      
      ///
      void _onMouseDown( const GuiEvent& event, bool lockMouse );

   public:
   
      GuiScrollCtrl();
                                 
      void autoScroll(Region reg);

      void scrollDeltaAnimate(S32 x, S32 y);
      void scrollTo(S32 x, S32 y);
      void scrollToObject(GuiControl *targetControl);
      void scrollDelta(S32 x, S32 y);
      void scrollRectVisible(RectI rect);

      /// Is the given client space rect completely visible within the actual
      /// visible area, or is some of it clipped.  Returns true if it is
      /// completely visible.
      bool isRectCompletelyVisible(const RectI& rect);
      bool isPointVisible( const Point2I& point );

      void computeSizes();

      // you can change the bitmap array dynamically.
      void loadBitmapArray();

      void addObject(SimObject *obj);
      bool resize(const Point2I &newPosition, const Point2I &newExtent);
      void childResized(GuiControl *child);
      Point2I getChildPos() { return mChildPos; }
      Point2I getChildRelPos() { return mChildRelPos; };
      Point2I getChildExtent() { return mChildExt; }
      Point2I getContentExtent() { return mContentExt; }
      Point2I getChildMargin() { return mChildMargin; } //  Added to aid in sizing calculations
      S32 getBorderThickness(void) { return mBorderThickness; }
      S32 scrollBarThickness() const                        { return(mScrollBarThickness); }
      S32 scrollBarArrowBtnLength() const                   { return(mScrollBarArrowBtnLength); }
      bool hasHScrollBar() const                            { return(mHasHScrollBar); }
      bool hasVScrollBar() const                            { return(mHasVScrollBar); }
      bool enabledHScrollBar() const                        { return(mHBarEnabled); }
      bool enabledVScrollBar() const                        { return(mVBarEnabled); }

      bool isScrolledToBottom() { return mChildPos.y + mChildExt.y <= mContentPos.y + mContentExt.y; }

      bool wantsTabListMembership();
      bool becomeFirstResponder();
      bool loseFirstResponder();

      Region getCurHitRegion(void) { return mHitRegion; }

      // GuiControl
      virtual bool onKeyDown(const GuiEvent &event);
      virtual void onMouseDown(const GuiEvent &event);
      virtual bool onMouseDownEditor( const GuiEvent& event, Point2I offset );
      virtual void onMouseUp(const GuiEvent &event);
      virtual void onMouseDragged(const GuiEvent &event);
      virtual bool onMouseWheelUp(const GuiEvent &event);
      virtual bool onMouseWheelDown(const GuiEvent &event);

      virtual bool onWake();
      virtual void onSleep();

      virtual void onPreRender();
      virtual void onRender(Point2I offset, const RectI &updateRect);
      virtual void drawBorder(const Point2I &offset, bool isFirstResponder);
      virtual void drawVScrollBar(const Point2I &offset);
      virtual void drawHScrollBar(const Point2I &offset);
      virtual void drawScrollCorner(const Point2I &offset);
      virtual GuiControl* findHitControl(const Point2I &pt, S32 initialLayer = -1);

      static void initPersistFields();

      DECLARE_CONOBJECT(GuiScrollCtrl);
      DECLARE_DESCRIPTION( "A container that allows to view a larger GUI control inside its smaller area "
                           "by providing horizontal and/or vertical scroll bars." );
};

typedef GuiScrollCtrl::ScrollBarBehavior GuiScrollBarBehavior;
DefineEnumType( GuiScrollBarBehavior );

#endif //_GUI_SCROLL_CTRL_H
