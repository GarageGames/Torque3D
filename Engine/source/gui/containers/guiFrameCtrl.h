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

#ifndef _GUIFRAMECTRL_H_
#define _GUIFRAMECTRL_H_

#ifndef _GUICONTAINER_H_
   #include "gui/containers/guiContainer.h"
#endif


// for debugging porpoises...
#define GUI_FRAME_DEBUG
// ...save the porpoises


/// A gui control allowing a window to be subdivided into panes,
/// each of which displays a gui control child of the
/// GuiFrameSetCtrl. Each gui control child will have an associated
/// FrameDetail through which frame-specific details can be
/// assigned. Frame-specific values override the values specified
/// for the entire frameset.
///
/// Note that it is possible to have more children than frames,
/// or more frames than children. In the former case, the extra
/// children will not be visible (they are moved beyond the
/// visible extent of the frameset). In the latter case, frames
/// will be empty.
///
/// If a frameset had two columns and two rows but only three
/// gui control children they would be assigned to frames as
/// follows:
///                 1 | 2
///                 -----
///                 3 |
///
/// The last frame would be blank.
///
class GuiFrameSetCtrl : public GuiContainer
{
private:
   typedef GuiContainer Parent;
public:
   enum FrameState
   {
      FRAME_STATE_ON,                                    // ON overrides OFF
      FRAME_STATE_OFF,                                   // OFF overrides AUTO
      FRAME_STATE_AUTO,                                  // AUTO == ON, unless overridden

      NO_HIT = -1,

      DEFAULT_BORDER_WIDTH = 4,
      DEFAULT_COLUMNS = 1,
      DEFAULT_ROWS = 1,
      DEFAULT_MIN_FRAME_EXTENT = 64
   };
   enum Region
   {
      VERTICAL_DIVIDER,
      HORIZONTAL_DIVIDER,
      DIVIDER_INTERSECTION,
      NONE
   };
   struct FrameDetail
   {
      U32 mBorderWidth;
      ColorI mBorderColor;
      S32 mBorderEnable;
      S32 mBorderMovable;
      Point2I mMinExtent;
      RectSpacingI mPadding;
      FrameDetail()                                      { mBorderWidth = DEFAULT_BORDER_WIDTH; mBorderEnable = FRAME_STATE_AUTO; mBorderMovable = FRAME_STATE_AUTO; mMinExtent.set(DEFAULT_MIN_FRAME_EXTENT, DEFAULT_MIN_FRAME_EXTENT); mPadding.setAll( 0 ); }
   };
   DECLARE_CONOBJECT(GuiFrameSetCtrl);
   DECLARE_DESCRIPTION( "A container that allows to subdivide its space into rows and columns.\n"
      "Child controls are assigned to the cells row by row." );
   static void initPersistFields();

   GuiFrameSetCtrl();
   GuiFrameSetCtrl(U32 columns, U32 rows, const U32 columnOffsets[] = NULL, const U32 rowOffsets[] = NULL);
   virtual ~GuiFrameSetCtrl();

   void addObject(SimObject *obj);
   void removeObject(SimObject *obj);

   virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);

   virtual void onMouseDown(const GuiEvent &event);
   virtual void onMouseUp(const GuiEvent &event);
   virtual void onMouseDragged(const GuiEvent &event);

   bool onAdd();
   void onRender(Point2I offset, const RectI &updateRect );
protected:
   /* member variables */
   Vector<S32> mColumnOffsets;
   Vector<S32> mRowOffsets;
   GuiCursor *mMoveCursor;
   GuiCursor *mUpDownCursor;
   GuiCursor *mLeftRightCursor;
   GuiCursor *mDefaultCursor;
   FrameDetail mFramesetDetails;
   VectorPtr<FrameDetail *> mFrameDetails;
   bool mAutoBalance;
   S32   mFudgeFactor;

   /* divider activation member variables */
   Region mCurHitRegion;
   Point2I mLocOnDivider;
   S32 mCurVerticalHit;
   S32 mCurHorizontalHit;

   bool init(U32 columns, U32 rows, const U32 columnOffsets[], const U32 rowOffsets[]);

   Region findHitRegion(const Point2I &point);
   Region pointInAnyRegion(const Point2I &point);
   S32 findResizableFrames(S32 indexes[]);
   bool hitVerticalDivider(S32 x, const Point2I &point);
   bool hitHorizontalDivider(S32 y, const Point2I &point);

   virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
   void rebalance(const Point2I &newExtent);

   void computeSizes(bool balanceFrames = false);
   void computeMovableRange(Region hitRegion, S32 vertHit, S32 horzHit, S32 numIndexes, const S32 indexes[], S32 ranges[]);

   void drawDividers(const Point2I &offset);
public:
   U32 columns() const                                   { return(mColumnOffsets.size()); }
   U32 rows() const                                      { return(mRowOffsets.size()); }
   U32 borderWidth() const                               { return(mFramesetDetails.mBorderWidth); }
   Vector<S32>* columnOffsets()                          { return(&mColumnOffsets); }
   Vector<S32>* rowOffsets()                             { return(&mRowOffsets); }
   FrameDetail* framesetDetails()                        { return(&mFramesetDetails); }

   bool findFrameContents(S32 index, GuiControl **gc, FrameDetail **fd);

   void frameBorderEnable(S32 index, const char *state = NULL);
   void frameBorderMovable(S32 index, const char *state = NULL);
   void frameMinExtent(S32 index, const Point2I &extent);
   void framePadding(S32 index, const RectSpacingI &padding);
   RectSpacingI getFramePadding(S32 index);

   void balanceFrames()    { computeSizes(true); }
   void updateSizes()      { computeSizes();     }

   bool onWake();

private:
   GuiFrameSetCtrl(const GuiFrameSetCtrl &);
   GuiFrameSetCtrl& operator=(const GuiFrameSetCtrl &);
};

typedef GuiFrameSetCtrl::FrameState GuiFrameState;
DefineEnumType( GuiFrameState );

//-----------------------------------------------------------------------------
// x is the first value inside the next column, so the divider x-coords
// precede x.
inline bool GuiFrameSetCtrl::hitVerticalDivider(S32 x, const Point2I &point)
{
   return((point.x >= S32(x - mFramesetDetails.mBorderWidth)) && (point.x < x) && (point.y >= 0) && (point.y < S32(getHeight())));
}

//-----------------------------------------------------------------------------
// y is the first value inside the next row, so the divider y-coords precede y.
inline bool GuiFrameSetCtrl::hitHorizontalDivider(S32 y, const Point2I &point)
{
   return((point.x >= 0) && (point.x < S32(getWidth())) && (point.y >= S32(y - mFramesetDetails.mBorderWidth)) && (point.y < y));
}

#endif // _GUI_FRAME_CTRL_H
