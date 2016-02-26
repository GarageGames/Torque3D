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

#include "platform/platform.h"
#include "platform/typetraits.h"
#include "gui/containers/guiScrollCtrl.h"
#include "console/engineAPI.h"
#include "console/console.h"
#include "gfx/bitmap/gBitmap.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"


IMPLEMENT_CONOBJECT( GuiScrollCtrl );

ConsoleDocClass( GuiScrollCtrl,
   "@brief A container that allows to view one or more possibly larger controls inside its area by "
      "providing horizontal and/or vertical scroll bars.\n\n"
   
   "@ingroup GuiContainers"
);

ImplementEnumType( GuiScrollBarBehavior,
   "Display behavior of a scroll bar.  Determines when a scrollbar will be visible.\n\n"
   "@ingroup GuiContainers" )
   { GuiScrollCtrl::ScrollBarAlwaysOn,     "alwaysOn",   "Always visible." },
   { GuiScrollCtrl::ScrollBarAlwaysOff,    "alwaysOff",  "Never visible." },
   { GuiScrollCtrl::ScrollBarDynamic,      "dynamic",    "Only visible when actually needed, i.e. when the child control(s) exceed the visible space on the given axis." },
EndImplementEnumType;

IMPLEMENT_CALLBACK( GuiScrollCtrl, onScroll, void, (), (),
   "Called each time the child controls are scrolled by some amount." );

//-----------------------------------------------------------------------------

GuiScrollCtrl::GuiScrollCtrl()
 : mChildMargin( 0, 0 ),
   mBorderThickness( 1 ),
   mScrollBarThickness( 16 ),
   mScrollBarArrowBtnLength( 16 ),
   mScrollBarDragTolerance( 130 ),
   mStateDepressed( false ),
   mHitRegion( None ),
   mWillFirstRespond( true ),
   mUseConstantHeightThumb( false ),
   mForceVScrollBar( ScrollBarAlwaysOn ),
   mForceHScrollBar( ScrollBarAlwaysOn ),
   mLockHorizScroll( false ),
   mLockVertScroll( false ),
   mIgnoreChildResized( false ),
   mAnimating( false ),
   mScrollAnimSpeed( -1 ),
   mScrollTargetPos( -1, -1 ),
   mChildExt(0, 0),
   mChildPos(0, 0),
   mBaseThumbSize(0)
{
   mIsContainer = true;
   setExtent(200,200);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::initPersistFields()
{
   addGroup( "Scolling" );
   
      addField( "willFirstRespond",     TypeBool,    Offset(mWillFirstRespond, GuiScrollCtrl));
      addField( "hScrollBar",           TYPEID< ScrollBarBehavior >(),    Offset(mForceHScrollBar, GuiScrollCtrl),
         "When to display the horizontal scrollbar.");
      addField( "vScrollBar",           TYPEID< ScrollBarBehavior >(),    Offset(mForceVScrollBar, GuiScrollCtrl),
         "When to display the vertical scrollbar.");
      addField( "lockHorizScroll",      TypeBool,    Offset(mLockHorizScroll, GuiScrollCtrl),
         "Horizontal scrolling not allowed if set.");
      addField( "lockVertScroll",       TypeBool,    Offset(mLockVertScroll, GuiScrollCtrl),
         "Vertical scrolling not allowed if set.");
      addField( "constantThumbHeight",  TypeBool,    Offset(mUseConstantHeightThumb, GuiScrollCtrl));
      addField( "childMargin",          TypePoint2I, Offset(mChildMargin, GuiScrollCtrl),
         "Padding region to put around child contents." );
      addField( "mouseWheelScrollSpeed", TypeS32,    Offset(mScrollAnimSpeed, GuiScrollCtrl),
         "Pixels/Tick - if not positive then mousewheel scrolling occurs instantly (like other scrolling).");
      
   endGroup( "Scrolling" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::resize(const Point2I &newPos, const Point2I &newExt)
{
   if( !Parent::resize(newPos, newExt) )
      return false;

   computeSizes();
   return true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::childResized(GuiControl *child)
{
   if ( mIgnoreChildResized )
      return;

   Parent::childResized(child);
   computeSizes();
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   mTextureObject = mProfile->mTextureObject;
   if (mTextureObject && (mProfile->constructBitmapArray() >= BmpStates * BmpCount))
   {
      mBitmapBounds = mProfile->mBitmapArrayRects.address();

      //init
      mBaseThumbSize = mBitmapBounds[BmpStates * BmpVThumbTopCap].extent.y +
         mBitmapBounds[BmpStates * BmpVThumbBottomCap].extent.y;
      mScrollBarThickness      = mBitmapBounds[BmpStates * BmpVPage].extent.x;
      mScrollBarArrowBtnLength = mBitmapBounds[BmpStates * BmpUp].extent.y;
      computeSizes();
   } 
   else
   {
      Con::warnf("No texture loaded for scroll control named %s with profile %s", getName(), mProfile->getName());
   }
   
   return true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onSleep()
{
   // Reset the mouse tracking state of this control
   //  when it is put to sleep
   mStateDepressed = false;
   mHitRegion = None;

   Parent::onSleep();
   mTextureObject = NULL;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::calcChildExtents()
{
   // scroll control should deal well with multiple gui controls
   if( !size() )
      return false;

   // Find size and relative position of the client rectangle.
   
	Point2I maxPos( TypeTraits< S32 >::MIN, TypeTraits< S32 >::MIN );
	Point2I minPos( TypeTraits< S32 >::MAX, TypeTraits< S32 >::MAX );
   
   bool haveVisibleChild = false;
   for( U32 i = 0; i < size(); i++ )
   {
      GuiControl *ctrl = (GuiControl*)at(i);
      if( ctrl->isVisible() )
      {
         haveVisibleChild = true;
         
         minPos.x = getMin( ctrl->getPosition().x, minPos.x );
         minPos.y = getMin( ctrl->getPosition().y, minPos.y );

         // This is +1 but the remaining code here all works with extents +1.
         Point2I ctrlMax = ctrl->getPosition() + ctrl->getExtent();
                  
         maxPos.x = getMax( ctrlMax.x, maxPos.x );
         maxPos.y = getMax( ctrlMax.y, maxPos.y );
      }
   }
   
   if( !haveVisibleChild )
      return false;
   
   mChildPos = minPos;
   mChildExt = maxPos - minPos;

   return true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollRectVisible(RectI rect)
{
   // rect is passed in virtual client space
   if(rect.extent.x > mContentExt.x)
      rect.extent.x = mContentExt.x;
   if(rect.extent.y > mContentExt.y)
      rect.extent.y = mContentExt.y;

   // Determine the points bounding the requested rectangle
	Point2I rectUpperLeft  = rect.point;
	Point2I rectLowerRight = rect.point + rect.extent;

   // Determine the points bounding the actual visible area...
	Point2I visUpperLeft = mChildRelPos;
	Point2I visLowerRight = mChildRelPos + mContentExt;
	Point2I delta(0,0);

   // We basically try to make sure that first the top left of the given
   // rect is visible, and if it is, then that the bottom right is visible.

   // Make sure the rectangle is visible along the X axis...
	if(rectUpperLeft.x < visUpperLeft.x)
		delta.x = rectUpperLeft.x - visUpperLeft.x;
	else if(rectLowerRight.x > visLowerRight.x)
		delta.x = rectLowerRight.x - visLowerRight.x;

   // Make sure the rectangle is visible along the Y axis...
	if(rectUpperLeft.y < visUpperLeft.y)
		delta.y = rectUpperLeft.y - visUpperLeft.y;
	else if(rectLowerRight.y > visLowerRight.y)
		delta.y = rectLowerRight.y - visLowerRight.y;

   // If we had any changes, scroll, otherwise don't.
   if(delta.x || delta.y)
		scrollDelta(delta.x, delta.y);
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::isPointVisible( const Point2I& point )
{
   return    ( point.x >= mChildRelPos.x && point.x <= ( mChildRelPos.x + mContentExt.x ) )
          && ( point.y >= mChildRelPos.y && point.y <= ( mChildRelPos.y + mContentExt.y ) );
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::isRectCompletelyVisible(const RectI& rect)
{
   // rect is passed in virtual client space
   // Determine the points bounding the requested rectangle
	Point2I rectUpperLeft  = rect.point;
	Point2I rectLowerRight = rect.point + rect.extent;

   // Determine the points bounding the actual visible area...
	Point2I visUpperLeft = mChildRelPos;
	Point2I visLowerRight = mChildRelPos + mContentExt;

   // Make sure the rectangle is visible along the X axis...
	if(rectUpperLeft.x < visUpperLeft.x)
		return false;
	else if(rectLowerRight.x > visLowerRight.x)
		return false;

   // Make sure the rectangle is visible along the Y axis...
	if(rectUpperLeft.y < visUpperLeft.y)
		return false;
	else if(rectLowerRight.y > visLowerRight.y)
		return false;

   return true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::addObject(SimObject *object)
{
   Parent::addObject(object);
   computeSizes();
}

//-----------------------------------------------------------------------------

GuiControl* GuiScrollCtrl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   if(pt.x < mProfile->mBorderThickness || pt.y < mProfile->mBorderThickness)
      return this;
   if(pt.x >= getWidth() - mProfile->mBorderThickness - (mHasVScrollBar ? mScrollBarThickness : 0) ||
      pt.y >= getHeight() - mProfile->mBorderThickness - (mHasHScrollBar ? mScrollBarThickness : 0))
      return this;
   return Parent::findHitControl(pt, initialLayer);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::computeSizes()
{
   S32 thickness = (mProfile ? mProfile->mBorderThickness : 1);
   Point2I borderExtent(thickness, thickness);
   mContentPos = borderExtent + mChildMargin;
   mContentExt = getExtent() - (mChildMargin * 2)
                                - (borderExtent * 2);

   Point2I childLowerRight;

   mHBarEnabled = false;
   mVBarEnabled = false;
   mHasVScrollBar = (mForceVScrollBar == ScrollBarAlwaysOn);
   mHasHScrollBar = (mForceHScrollBar == ScrollBarAlwaysOn);

   setUpdate();

   if (calcChildExtents())
   {
      childLowerRight = mChildPos + mChildExt;

      if (mHasVScrollBar)
         mContentExt.x -= mScrollBarThickness;
      if (mHasHScrollBar)
         mContentExt.y -= mScrollBarThickness;
      if (mChildExt.x > mContentExt.x && (mForceHScrollBar == ScrollBarDynamic))
      {
         mHasHScrollBar = true;
         mContentExt.y -= mScrollBarThickness;
      }
      if (mChildExt.y > mContentExt.y && (mForceVScrollBar == ScrollBarDynamic))
      {
         mHasVScrollBar = true;
         mContentExt.x -= mScrollBarThickness;

         // If Extent X Changed, check Horiz Scrollbar.
         if (mChildExt.x > mContentExt.x && !mHasHScrollBar && (mForceHScrollBar == ScrollBarDynamic))
         {
            mHasHScrollBar = true;
            mContentExt.y -= mScrollBarThickness;
         }
      }
      Point2I contentLowerRight = mContentPos + mContentExt;

      // see if the child controls need to be repositioned (null space in control)
      Point2I delta(0,0);

      if (mChildPos.x > mContentPos.x)
         delta.x = mContentPos.x - mChildPos.x;
      else if (contentLowerRight.x > childLowerRight.x)
      {
         S32 diff = contentLowerRight.x - childLowerRight.x;
         delta.x = getMin(mContentPos.x - mChildPos.x, diff);
      }

      //reposition the children if the child extent > the scroll content extent
      if (mChildPos.y > mContentPos.y)
         delta.y = mContentPos.y - mChildPos.y;
      else if (contentLowerRight.y > childLowerRight.y)
      {
         S32 diff = contentLowerRight.y - childLowerRight.y;
         delta.y = getMin(mContentPos.y - mChildPos.y, diff);
      }

      // apply the deltas to the children...
      if (delta.x || delta.y)
      {
         SimGroup::iterator i;
         for(i = begin(); i != end();i++)
         {
            GuiControl *ctrl = (GuiControl *) (*i);
            ctrl->setPosition( ctrl->getPosition() + delta );
         }
         mChildPos += delta;
         childLowerRight += delta;
      }
      // enable needed scroll bars
      if (mChildExt.x > mContentExt.x)
         mHBarEnabled = true;
      if (mChildExt.y > mContentExt.y)
         mVBarEnabled = true;
      mChildRelPos = mContentPos - mChildPos;
   }

   // Prevent resizing our children from recalling this function!
   mIgnoreChildResized = true;

   if ( mLockVertScroll )
   {
      // If vertical scroll is locked we size our child's height to our own
      SimGroup::iterator i;
      for(i = begin(); i != end();i++)
      {
         GuiControl *ctrl = (GuiControl *) (*i);
         ctrl->setHeight( mContentExt.y  );
      }
   }

   if ( mLockHorizScroll )
   {
      // If horizontal scroll is locked we size our child's width to our own
      SimGroup::iterator i;
      for(i = begin(); i != end();i++)
      {
         GuiControl *ctrl = (GuiControl *) (*i);
         ctrl->setWidth( mContentExt.x  );
      }
   }

   mIgnoreChildResized = false;

   // build all the rectangles and such...
   calcScrollRects();
   calcThumbs();
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::calcScrollRects(void)
{
   S32 thickness = ( mProfile ? mProfile->mBorderThickness : 1 );
   if (mHasHScrollBar)
   {
      mLeftArrowRect.set(thickness,
                        getHeight() - thickness - mScrollBarThickness,
                        mScrollBarArrowBtnLength,
                        mScrollBarThickness);

      mRightArrowRect.set(getWidth() - thickness - (mHasVScrollBar ? mScrollBarThickness - 1 : 0) - mScrollBarArrowBtnLength,
                        getHeight() - thickness - mScrollBarThickness,
                        mScrollBarArrowBtnLength,
                        mScrollBarThickness);
      mHTrackRect.set(mLeftArrowRect.point.x + mLeftArrowRect.extent.x,
                        mLeftArrowRect.point.y,
                        mRightArrowRect.point.x - (mLeftArrowRect.point.x + mLeftArrowRect.extent.x),
                        mScrollBarThickness);
   }
   if (mHasVScrollBar)
   {
      mUpArrowRect.set(getWidth() - thickness - mScrollBarThickness,
                        thickness,
                        mScrollBarThickness,
                        mScrollBarArrowBtnLength);
      mDownArrowRect.set(getWidth() - thickness - mScrollBarThickness,
                        getHeight() - thickness - (mHasHScrollBar ? mScrollBarThickness - 1 : 0) - mScrollBarArrowBtnLength,
                        mScrollBarThickness,
                        mScrollBarArrowBtnLength);
      mVTrackRect.set(mUpArrowRect.point.x,
                        mUpArrowRect.point.y + mUpArrowRect.extent.y,
                        mScrollBarThickness,
                        mDownArrowRect.point.y - (mUpArrowRect.point.y + mUpArrowRect.extent.y) );
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::calcThumbs()
{
   if (mHBarEnabled && mChildExt.x > 0)
   {
      U32 trackSize = mHTrackRect.len_x();

      if (mUseConstantHeightThumb)
         mHThumbSize = mBaseThumbSize;
      else
         mHThumbSize = getMax(mBaseThumbSize, ( S32 )mCeil( ( F32 )( mContentExt.x * trackSize) / ( F32 )mChildExt.x ) );

      mHThumbPos = mHTrackRect.point.x + (mChildRelPos.x * (trackSize - mHThumbSize)) / (mChildExt.x - mContentExt.x);
   }
   if (mVBarEnabled && mChildExt.y > 0)
   {
      U32 trackSize = mVTrackRect.len_y();

      if (mUseConstantHeightThumb)
         mVThumbSize = mBaseThumbSize;
      else
         mVThumbSize = getMax(mBaseThumbSize, ( S32 )mCeil( ( F32 )( mContentExt.y * trackSize ) / ( F32 )mChildExt.y ) );

      mVThumbPos = mVTrackRect.point.y + (mChildRelPos.y * (trackSize - mVThumbSize)) / (mChildExt.y - mContentExt.y);
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollDelta(S32 deltaX, S32 deltaY)
{
   scrollTo(mChildRelPos.x + deltaX, mChildRelPos.y + deltaY);

   onScroll_callback();
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollDeltaAnimate(S32 x, S32 y)
{
   if ( !size() )
      return;

   if ( mAnimating )
      mScrollTargetPos += Point2I( x, y );
   else
      mScrollTargetPos = mChildRelPos + Point2I( x, y );

   setUpdate();

   mScrollTargetPos.setMin( mChildExt - mContentExt );
   mScrollTargetPos.setMax( Point2I::Zero );
   
   mAnimating = true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollTo(S32 x, S32 y)
{
   if( !size() )
      return;

   if ( x == mChildRelPos.x && y == mChildRelPos.y )
      return;

   setUpdate();
   if (x > mChildExt.x - mContentExt.x)
      x = mChildExt.x - mContentExt.x;
   if (x < 0)
      x = 0;

   if (y > mChildExt.y - mContentExt.y)
      y = mChildExt.y - mContentExt.y;
   if (y < 0)
      y = 0;

   Point2I delta(x - mChildRelPos.x, y - mChildRelPos.y);
   mChildRelPos += delta;
   mChildPos -= delta;

   for(SimSet::iterator i = begin(); i != end();i++)
   {
      GuiControl *ctrl = (GuiControl *) (*i);
      ctrl->setPosition( ctrl->getPosition() - delta );
   }
   calcThumbs();

   onScroll_callback();
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollToObject(GuiControl *targetControl)
{
	bool        isValidChild     = false;
	Point2I     relativePosition = targetControl->getPosition();
	GuiControl* parentControl    = targetControl->getParent();
	while (parentControl)
	{
		GuiScrollCtrl* scrollControl = dynamic_cast<GuiScrollCtrl*>(parentControl);
		if (scrollControl == this)
		{
			relativePosition += scrollControl->getChildRelPos();
			isValidChild      = true;
			break;
		}

		relativePosition += parentControl->getPosition();
		parentControl     = parentControl->getParent();
	}

	if (isValidChild)
	{
		scrollRectVisible(RectI(relativePosition, targetControl->getExtent()));
	}
	else
	{
		Con::errorf("GuiScrollCtrl::scrollToObject() - Specified object is not a child of this scroll control (%d)!", targetControl->getId());
	}
}

//-----------------------------------------------------------------------------

GuiScrollCtrl::Region GuiScrollCtrl::findHitRegion(const Point2I &pt)
{
   if (mVBarEnabled && mHasVScrollBar)
   {
      if (mUpArrowRect.pointInRect(pt))
         return UpArrow;
      else if (mDownArrowRect.pointInRect(pt))
         return DownArrow;
      else if (mVTrackRect.pointInRect(pt))
      {
         if (pt.y < mVThumbPos)
            return UpPage;
         else if (pt.y < mVThumbPos + mVThumbSize)
            return VertThumb;
         else
            return DownPage;
      }
   }
   if (mHBarEnabled && mHasHScrollBar)
   {
      if (mLeftArrowRect.pointInRect(pt))
         return LeftArrow;
      else if (mRightArrowRect.pointInRect(pt))
         return RightArrow;
      else if (mHTrackRect.pointInRect(pt))
      {
         if (pt.x < mHThumbPos)
            return LeftPage;
         else if (pt.x < mHThumbPos + mHThumbSize)
            return HorizThumb;
         else
            return RightPage;
      }
   }
   return None;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::wantsTabListMembership()
{
   return true;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::loseFirstResponder()
{
   setUpdate();
   return true;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::becomeFirstResponder()
{
   setUpdate();
   return mWillFirstRespond;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::onKeyDown(const GuiEvent &event)
{
   if (mWillFirstRespond)
   {
      switch (event.keyCode)
      {
         case KEY_RIGHT:
            scrollByRegion(RightArrow);
            return true;

         case KEY_LEFT:
            scrollByRegion(LeftArrow);
            return true;

         case KEY_DOWN:
            scrollByRegion(DownArrow);
            return true;

         case KEY_UP:
            scrollByRegion(UpArrow);
            return true;

         case KEY_PAGE_UP:
            scrollByRegion(UpPage);
            return true;

         case KEY_PAGE_DOWN:
            scrollByRegion(DownPage);
            return true;
            
         default:
            break;
      }
   }
   return Parent::onKeyDown(event);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::_onMouseDown( const GuiEvent &event, bool lockMouse )
{
   if( lockMouse )
   {
      mouseLock();
      mStateDepressed = true;
   }

   setUpdate();

   Point2I curMousePos = globalToLocalCoord(event.mousePoint);
   mHitRegion = findHitRegion(curMousePos);

   // Set a 0.5 second delay before we start scrolling
   mLastUpdated = Platform::getVirtualMilliseconds() + 500;

   scrollByRegion(mHitRegion);

   if (mHitRegion == VertThumb)
   {
      mChildRelPosAnchor = mChildRelPos;
      mThumbMouseDelta = curMousePos.y - mVThumbPos;
   }
   else if (mHitRegion == HorizThumb)
   {
      mChildRelPosAnchor = mChildRelPos;
      mThumbMouseDelta = curMousePos.x - mHThumbPos;
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onMouseDown(const GuiEvent &event)
{
   _onMouseDown( event, true );
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::onMouseDownEditor( const GuiEvent& event, Point2I offset )
{
   // If ALT is pressed while clicking on a horizontal or vertical scrollbar,
   // do a scroll.
   
   if( event.modifier & SI_PRIMARY_ALT )
   {
      Region hitRegion = findHitRegion( globalToLocalCoord( event.mousePoint ) );
      if( hitRegion != None )
      {
         _onMouseDown( event, false );
         return true;
      }
   }
   
   return false;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onMouseUp(const GuiEvent &)
{
   mouseUnlock();

   setUpdate();

   mHitRegion = None;
   mStateDepressed = false;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onMouseDragged(const GuiEvent &event)
{
   Point2I curMousePos = globalToLocalCoord(event.mousePoint);
   setUpdate();

   if ( (mHitRegion != VertThumb) && (mHitRegion != HorizThumb) )
   {
      Region hit = findHitRegion(curMousePos);
      if (hit != mHitRegion)
         mStateDepressed = false;
      else
         mStateDepressed = true;
      return;
   }

   // ok... if the mouse is 'near' the scroll bar, scroll with it
   // otherwise, snap back to the previous position.

   if (mHitRegion == VertThumb)
   {
      if (curMousePos.x >= mVTrackRect.point.x - mScrollBarDragTolerance &&
         curMousePos.x <= mVTrackRect.point.x + mVTrackRect.extent.x - 1 + mScrollBarDragTolerance &&
         curMousePos.y >= mVTrackRect.point.y - mScrollBarDragTolerance &&
         curMousePos.y <= mVTrackRect.point.y + mVTrackRect.extent.y - 1 + mScrollBarDragTolerance)
      {
         S32 newVThumbPos = curMousePos.y - mThumbMouseDelta;
         if(newVThumbPos != mVThumbPos)
         {
            S32 newVPos = (newVThumbPos - mVTrackRect.point.y) *
                          (mChildExt.y - mContentExt.y) /
                          (mVTrackRect.extent.y - mVThumbSize);

            scrollTo(mChildRelPosAnchor.x, newVPos);
         }
      }
      else
         scrollTo(mChildRelPosAnchor.x, mChildRelPosAnchor.y);
   }
   else if (mHitRegion == HorizThumb)
   {
      if (curMousePos.x >= mHTrackRect.point.x - mScrollBarDragTolerance &&
         curMousePos.x <= mHTrackRect.point.x + mHTrackRect.extent.x - 1 + mScrollBarDragTolerance &&
         curMousePos.y >= mHTrackRect.point.y - mScrollBarDragTolerance &&
         curMousePos.y <= mHTrackRect.point.y + mHTrackRect.extent.y - 1 + mScrollBarDragTolerance)
      {
         S32 newHThumbPos = curMousePos.x - mThumbMouseDelta;
         if(newHThumbPos != mHThumbPos)
         {
            S32 newHPos = (newHThumbPos - mHTrackRect.point.x) *
                          (mChildExt.x - mContentExt.x) /
                          (mHTrackRect.extent.x - mHThumbSize);

            scrollTo(newHPos, mChildRelPosAnchor.y);
         }
      }
      else
         scrollTo(mChildRelPosAnchor.x, mChildRelPosAnchor.y);
   }
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::onMouseWheelUp(const GuiEvent &event)
{
   if ( !mAwake || !mVisible )
      return false;

   scrollByMouseWheel( event );

   return true;
}

//-----------------------------------------------------------------------------

bool GuiScrollCtrl::onMouseWheelDown(const GuiEvent &event)
{
   if ( !mAwake || !mVisible )
      return false;

   scrollByMouseWheel( event );   

   return true;
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::updateChildMousePos()
{      
   // We pass a fake GuiEvent to child controls onMouseMove
   // since although the mouse has not moved 'they' have.
   //
   // Its possible this could cause problems if a GuiControl
   // responds to more than just the mouse position in the onMouseMove
   // event, like for example doing something different depending on
   // a modifier key, which we aren't filling in to the structure!

   GuiEvent event;
   event.mousePoint = getRoot()->getCursorPos();

   iterator itr;
   for ( itr = begin(); itr != end(); itr++ )
   {
      GuiControl *child = static_cast<GuiControl*>( *itr );
      child->onMouseMove( event );
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onPreRender()
{
   Parent::onPreRender();

   S32 currentTime = Platform::getVirtualMilliseconds();
   S32 deltaMs = currentTime - mLastPreRender;
   mLastPreRender = currentTime;

   // Update mouse-wheel scroll animation if we are currently doing one...

   if ( mAnimating )
   {            
      //U32 frames = Con::getIntVariable( "$frames", 0 );
      //frames++;
      //Con::setIntVariable( "$frames", frames );

      F32 deltaTicks = deltaMs / 32.0f;

      if ( mScrollAnimSpeed <= 0 )
      {
         scrollTo( mScrollTargetPos.x, mScrollTargetPos.y );
      }      
      else
      {
         S32 maxPixels = deltaTicks * mScrollAnimSpeed;

         Point2I toTarget = mScrollTargetPos - mChildRelPos;
         S32 signx = toTarget.x > 0 ? 1 : -1;
         S32 signy = toTarget.y > 0 ? 1 : -1;

         S32 deltaX = getMin( mAbs(toTarget.x), maxPixels ) * signx;
         S32 deltaY = getMin( mAbs(toTarget.y), maxPixels ) * signy;

         scrollDelta( deltaX, deltaY );
      }

      if ( mChildRelPos == mScrollTargetPos )   
      {
         //Con::printf( "Animated Frames : %d", frames );
         //Con::setIntVariable( "$frames", 0 );
         mAnimating = false;
      }

      updateChildMousePos();
   }

   // Now scroll in response to a 'depressed state' if appropriate...

   // Short circuit if not depressed to save cycles
   if( mStateDepressed != true )
      return;
   
   //default to one second, though it shouldn't be necessary
   U32 timeThreshold = 1000;

   // We don't want to scroll by pages at an interval the same as when we're scrolling
   // using the arrow buttons, so adjust accordingly.
   switch( mHitRegion )
   {
   case UpPage:
   case DownPage:
   case LeftPage:
   case RightPage:
      timeThreshold = 200;
      break;
   case UpArrow:
   case DownArrow:
   case LeftArrow:
   case RightArrow:
      timeThreshold = 20;
      break;
   default:
      // Neither a button or a page, don't scroll (shouldn't get here)
      return;
      break;
   };

   S32 timeElapsed = Platform::getVirtualMilliseconds() - mLastUpdated;

   if ( ( timeElapsed > 0 ) && ( timeElapsed > timeThreshold ) )
   {
      mLastUpdated = Platform::getVirtualMilliseconds();
      scrollByRegion(mHitRegion);
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollByRegion(Region reg)
{
   setUpdate();
   if(!size())
      return;
   GuiControl *content = (GuiControl *) front();
   U32 rowHeight, columnWidth;
   U32 pageHeight, pageWidth;

   content->getScrollLineSizes(&rowHeight, &columnWidth);

   if(rowHeight >= mContentExt.y)
      pageHeight = 1;
   else
      pageHeight = mContentExt.y - rowHeight;

   if(columnWidth >= mContentExt.x)
      pageWidth = 1;
   else
      pageWidth = mContentExt.x - columnWidth;

   if (mVBarEnabled)
   {
      switch(reg)
      {
         case UpPage:
            scrollDelta(0, -(S32)pageHeight);
            break;
         case DownPage:
            scrollDelta(0, pageHeight);
            break;
         case UpArrow:
            scrollDelta(0, -(S32)rowHeight);
            break;
         case DownArrow:
            scrollDelta(0, rowHeight);
            break;
         default:
            break;
      }
   }

   if (mHBarEnabled)
   {
      switch(reg)
      {
         case LeftPage:
            scrollDelta(-(S32)pageWidth, 0);
            break;
         case RightPage:
            scrollDelta(pageWidth, 0);
            break;
         case LeftArrow:
            scrollDelta(-(S32)columnWidth, 0);
            break;
         case RightArrow:
            scrollDelta(columnWidth, 0);
            break;
         default:
            break;
      }
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::scrollByMouseWheel( const GuiEvent &event )
{
   setUpdate();
   if ( !size() )
      return;

   if( event.mouseAxis == 1 )
      scrollDeltaAnimate( 0, -event.fval );
   else
      scrollDeltaAnimate( -event.fval, 0 );
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // draw content controls
   // create a rect to intersect with the updateRect
   RectI contentRect(mContentPos.x + offset.x, mContentPos.y + offset.y, mContentExt.x, mContentExt.y);
   contentRect.intersect(updateRect);
   
   // Always call parent
   Parent::onRender(offset, contentRect);
   
   if( mTextureObject )
   {
      // Reset the ClipRect as the parent call can modify it when rendering
      // the child controls
      GFX->setClipRect( updateRect );

      //draw the scroll corner
      if (mHasVScrollBar && mHasHScrollBar)
         drawScrollCorner(offset);

      // draw scroll bars
      if (mHasVScrollBar)
         drawVScrollBar(offset);

      if (mHasHScrollBar)
         drawHScrollBar(offset);
   }
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::drawBorder( const Point2I &offset, bool /*isFirstResponder*/ )
{
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::drawVScrollBar(const Point2I &offset)
{
    if ( mTextureObject.isNull() )
    {
        return;
    }

    // Start Point.
    Point2I pos = ( offset + mUpArrowRect.point );

    // Up Arrow.
    S32 upArrowBitmap = ( BmpStates * BmpUp );
    if ( !mVBarEnabled )
    {
        upArrowBitmap += BmpDisabled;
    }
    else if ( mHitRegion == UpArrow && mStateDepressed )
    {
        upArrowBitmap += BmpHilite;
    }

    // Render Up Arrow.
    GFXDrawUtil* drawUtil = GFX->getDrawUtil();
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[upArrowBitmap]);

    // Update Pos.
    pos.y += mBitmapBounds[upArrowBitmap].extent.y;

    // Track.
    S32 trackBitmap = ( BmpStates * BmpVPage );
    if ( !mVBarEnabled )
    {
        trackBitmap += BmpDisabled;
    }
    else if ( mHitRegion == DownPage && mStateDepressed )
    {
        trackBitmap += BmpHilite;
    }

    // Determine the Track Rect.
    RectI trackRect;
    trackRect.point    = pos;
    trackRect.extent.x = mBitmapBounds[trackBitmap].extent.x;
    trackRect.extent.y = ( offset.y + mDownArrowRect.point.y ) - pos.y;

    // Render Track?
    if ( trackRect.extent.y > 0 )
    {
        // Render Track.
       drawUtil->clearBitmapModulation();
       drawUtil->drawBitmapStretchSR(mTextureObject, trackRect, mBitmapBounds[trackBitmap]);
    }

    // Update Pos.
    pos.y += trackRect.extent.y;

    // Down Arrow.
    S32 downArrowBitmap = ( BmpStates * BmpDown );
    if ( !mVBarEnabled )
    {
        downArrowBitmap += BmpDisabled;
    }
    else if ( mHitRegion == DownArrow && mStateDepressed )
    {
        downArrowBitmap += BmpHilite;
    }

    // Render Down Arrow.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[downArrowBitmap]);

    // Render the Thumb?
    if ( !mVBarEnabled )
    {
        // Nope.
        return;
    }

    // Reset the Pos.
    pos.y = ( offset.y + mVThumbPos );

    // Determine the Bitmaps.
    S32 thumbBitmapTop    = ( BmpStates * BmpVThumbTopCap );
    S32 thumbBitmapMiddle = ( BmpStates * BmpVThumb );
    S32 thumbBitmapBottom = ( BmpStates * BmpVThumbBottomCap );

    if ( mHitRegion == VertThumb && mStateDepressed )
    {
        thumbBitmapTop    += BmpHilite;
        thumbBitmapMiddle += BmpHilite;
        thumbBitmapBottom += BmpHilite;
    }

    // Render Thumb Top.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[thumbBitmapTop]);

    // Update Pos.
    pos.y += mBitmapBounds[thumbBitmapTop].extent.y;

    // Determine the Thumb Rect.
    RectI thumbRect;
    thumbRect.point    = pos;
    thumbRect.extent.x = mBitmapBounds[thumbBitmapMiddle].extent.x;
    thumbRect.extent.y = mVThumbSize - ( mBitmapBounds[thumbBitmapTop].extent.y + mBitmapBounds[thumbBitmapBottom].extent.y );

    // Render Thumb?
    if ( thumbRect.extent.y > 0 )
    {
        // Render Track.
       drawUtil->clearBitmapModulation();
       drawUtil->drawBitmapStretchSR(mTextureObject, thumbRect, mBitmapBounds[thumbBitmapMiddle]);
    }

    // Update Pos.
    pos.y += thumbRect.extent.y;

    // Render the Thumb Bottom.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[thumbBitmapBottom]);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::drawHScrollBar(const Point2I &offset)
{
    if ( mTextureObject.isNull() )
    {
        return;
    }

    // Start Point.
    Point2I pos = ( offset + mLeftArrowRect.point );

    // Left Arrow.
    S32 leftArrowBitmap = ( BmpStates * BmpLeft );
    if ( !mHBarEnabled )
    {
        leftArrowBitmap += BmpDisabled;
    }
    else if ( mHitRegion == LeftArrow && mStateDepressed )
    {
        leftArrowBitmap += BmpHilite;
    }

    // Render Up Arrow.
    GFXDrawUtil* drawUtil = GFX->getDrawUtil();
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[leftArrowBitmap]);

    // Update Pos.
    pos.x += mBitmapBounds[leftArrowBitmap].extent.x;

    // Track.
    S32 trackBitmap = ( BmpStates * BmpHPage );
    if ( !mHBarEnabled )
    {
        trackBitmap += BmpDisabled;
    }
    else if ( mHitRegion == LeftPage && mStateDepressed )
    {
        trackBitmap += BmpHilite;
    }

    // Determine the Track Rect.
    RectI trackRect;
    trackRect.point    = pos;
    trackRect.extent.x = ( offset.x + mRightArrowRect.point.x ) - pos.x;
    trackRect.extent.y = mBitmapBounds[trackBitmap].extent.y;

    // Render Track?
    if ( trackRect.extent.x > 0 )
    {
        // Render Track.
       drawUtil->clearBitmapModulation();
       drawUtil->drawBitmapStretchSR(mTextureObject, trackRect, mBitmapBounds[trackBitmap]);
    }

    // Update Pos.
    pos.x += trackRect.extent.x;

    // Right Arrow.
    S32 rightArrowBitmap = ( BmpStates * BmpRight );
    if ( !mHBarEnabled )
    {
        rightArrowBitmap += BmpDisabled;
    }
    else if ( mHitRegion == RightArrow && mStateDepressed )
    {
        rightArrowBitmap += BmpHilite;
    }

    // Render Right Arrow.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[rightArrowBitmap]);

    // Render the Thumb?
    if ( !mHBarEnabled )
    {
        // Nope.
        return;
    }

    // Reset the Pos.
    pos.x = ( offset.x + mHThumbPos );

    // Determine the Bitmaps.
    S32 thumbBitmapLeft   = ( BmpStates * BmpHThumbLeftCap );
    S32 thumbBitmapMiddle = ( BmpStates * BmpHThumb );
    S32 thumbBitmapRight  = ( BmpStates * BmpHThumbRightCap );

    if ( mHitRegion == HorizThumb && mStateDepressed )
    {
        thumbBitmapLeft   += BmpHilite;
        thumbBitmapMiddle += BmpHilite;
        thumbBitmapRight  += BmpHilite;
    }

    // Render Thumb Left.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[thumbBitmapLeft]);

    // Update Pos.
    pos.x += mBitmapBounds[thumbBitmapLeft].extent.x;

    // Determine the Thumb Rect.
    RectI thumbRect;
    thumbRect.point    = pos;
    thumbRect.extent.x = mHThumbSize - ( mBitmapBounds[thumbBitmapLeft].extent.x + mBitmapBounds[thumbBitmapRight].extent.x );
    thumbRect.extent.y = mBitmapBounds[thumbBitmapMiddle].extent.y;

    // Render Thumb?
    if ( thumbRect.extent.x > 0 )
    {
        // Render Track.
       drawUtil->clearBitmapModulation();
       drawUtil->drawBitmapStretchSR(mTextureObject, thumbRect, mBitmapBounds[thumbBitmapMiddle]);
    }

    // Update Pos.
    pos.x += thumbRect.extent.x;

    // Render the Thumb Bottom.
    drawUtil->clearBitmapModulation();
    drawUtil->drawBitmapSR(mTextureObject, pos, mBitmapBounds[thumbBitmapRight]);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::drawScrollCorner(const Point2I &offset)
{
   Point2I pos = offset;
   pos.x += mRightArrowRect.point.x + mRightArrowRect.extent.x - 1;
   pos.y += mRightArrowRect.point.y;
   GFX->getDrawUtil()->clearBitmapModulation();
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, pos, mBitmapBounds[BmpStates * BmpResize]);
}

//-----------------------------------------------------------------------------

void GuiScrollCtrl::autoScroll(Region reg)
{
   scrollByRegion(reg);
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, scrollToTop, void, (),,
   "Scroll all the way to the top of the vertical and left of the horizontal scrollbar." )
{
   object->scrollTo( 0, 0 );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, scrollToBottom, void, (),,
   "Scroll all the way to the bottom of the vertical scrollbar and the left of the horizontal bar." )
{
   object->scrollTo( 0, 0x7FFFFFFF );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, setScrollPosition, void, ( S32 x, S32 y ),,
   "Set the position of the scrolled content.\n\n"
   "@param x Position on X axis.\n"
   "@param y Position on y axis.\n" )
{
   object->scrollTo( x, y );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, scrollToObject, void, ( GuiControl* control ),,
   "Scroll the control so that the given child @a control is visible.\n\n"
   "@param control A child control." )
{
   if( control )
      object->scrollToObject( control );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, getScrollPosition, Point2I, (),,
   "Get the current coordinates of the scrolled content.\n\n"
   "@return The current position of the scrolled content." )
{
   return object->getChildRelPos();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, getScrollPositionX, S32, (),,
   "Get the current X coordinate of the scrolled content.\n\n"
   "@return The current X coordinate of the scrolled content." )
{
   return object->getChildRelPos().x;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, getScrollPositionY, S32, (),,
   "Get the current Y coordinate of the scrolled content."
   "@return The current Y coordinate of the scrolled content." )
{
   return object->getChildRelPos().y;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiScrollCtrl, computeSizes, void, (),,
   "Refresh sizing and positioning of child controls." )
{
   object->computeSizes();
}
