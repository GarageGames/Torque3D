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
#include "gui/core/guiArrayCtrl.h"

#include "console/console.h"
#include "console/engineAPI.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiDefaultControlRender.h"


IMPLEMENT_CONOBJECT(GuiArrayCtrl);

ConsoleDocClass( GuiArrayCtrl,
   "@brief Abstract base class for controls that store and display multiple elements in a single view.\n\n"

   "You cannot actually instantiate this class. Instead you can use its childre:\n\n"

   "- GuiConsole\n"
   "- GuiTextListCtrl\n"
   "- GuiTreeViewCtrl\n"
   "- DbgFileView\n"
   "- CreatorTree\n"

   "This base class is primarily used by other internal classes or those dedicated to editors.\n\n"

   "@ingroup GuiCore\n"

   "@internal"
);

IMPLEMENT_CALLBACK( GuiArrayCtrl, onCellSelected, void, ( const Point2I& cell ), ( cell ),
   "Call when a cell in the array is selected (clicked).\n\n"
   "@param @cell Coordinates of the cell"
);
IMPLEMENT_CALLBACK( GuiArrayCtrl, onCellHighlighted, void, ( const Point2I& cell ), ( cell ),
   "Call when a cell in the array is highlighted (moused over).\n\n"
   "@param @cell Coordinates of the cell"
);

//-----------------------------------------------------------------------------

GuiArrayCtrl::GuiArrayCtrl()
{
   mActive = true;

   mCellSize.set(80, 30);
   mSize = Point2I(5, 30);
   mSelectedCell.set(-1, -1);
   mMouseOverCell.set(-1, -1);
   mHeaderDim.set(0, 0);
   mIsContainer = true;
}

//-----------------------------------------------------------------------------

bool GuiArrayCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the font
   mFont = mProfile->mFont;

   return true;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onSleep()
{
   Parent::onSleep();
   mFont = NULL;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::setSize(Point2I newSize)
{
   mSize = newSize;
   Point2I newExtent(newSize.x * mCellSize.x + mHeaderDim.x, newSize.y * mCellSize.y + mHeaderDim.y);

   setExtent(newExtent);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::getScrollDimensions(S32 &cell_size, S32 &num_cells)
{
   cell_size = mCellSize.y;
   num_cells = mSize.y;
}

//-----------------------------------------------------------------------------

bool GuiArrayCtrl::cellSelected(Point2I cell)
{
   if (cell.x < 0 || cell.x >= mSize.x || cell.y < 0 || cell.y >= mSize.y)
   {
      mSelectedCell = Point2I(-1,-1);
      return false;
   }

   mSelectedCell = cell;
   scrollSelectionVisible();
   onCellSelected(cell);
	setUpdate();
   return true;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onCellSelected(Point2I cell)
{
   // [rene, 21-Jan-11 ] clashes with callbacks defined in derived classes
   Con::executef(this, "onSelect", Con::getFloatArg(cell.x), Con::getFloatArg(cell.y));

   onCellSelected_callback( cell );

   //call the console function
   execConsoleCallback();
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onCellHighlighted(Point2I cell)
{
   onCellHighlighted_callback( cell );
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::setSelectedCell(Point2I cell)
{
   cellSelected(cell);
}

//-----------------------------------------------------------------------------

Point2I GuiArrayCtrl::getSelectedCell()
{
   return mSelectedCell;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::scrollSelectionVisible()
{
   scrollCellVisible(mSelectedCell);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::scrollCellVisible(Point2I cell)
{
   //make sure we have a parent
   //make sure we have a valid cell selected
   GuiScrollCtrl *parent = dynamic_cast<GuiScrollCtrl*>(getParent());
   if(!parent || cell.x < 0 || cell.y < 0)
      return;

   RectI cellBounds(cell.x * mCellSize.x, cell.y * mCellSize.y, mCellSize.x, mCellSize.y);
   parent->scrollRectVisible(cellBounds);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onRenderColumnHeaders(Point2I offset, Point2I parentOffset, Point2I headerDim)
{
   if (mProfile->mBorder)
   {
      RectI cellR(offset.x + headerDim.x, parentOffset.y, getWidth() - headerDim.x, headerDim.y);
      GFX->getDrawUtil()->drawRectFill(cellR, mProfile->mBorderColor);
   }
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onRenderRowHeader(Point2I offset, Point2I parentOffset, Point2I headerDim, Point2I cell)
{
   ColorI color;
   RectI cellR;
   if (cell.x % 2)
      color.set(255, 0, 0, 255);
   else
      color.set(0, 255, 0, 255);

   cellR.point.set(parentOffset.x, offset.y);
   cellR.extent.set(headerDim.x, mCellSize.y);
   GFX->getDrawUtil()->drawRectFill(cellR, color);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   ColorI color(255 * (cell.x % 2), 255 * (cell.y % 2), 255 * ((cell.x + cell.y) % 2), 255);
   if (selected)
   {
      color.set(255, 0, 0, 255);
   }
   else if (mouseOver)
   {
      color.set(0, 0, 255, 255);
   }

   //draw the cell
   RectI cellR(offset.x, offset.y, mCellSize.x, mCellSize.y);
   GFX->getDrawUtil()->drawRectFill(cellR, color);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // The unmodified offset which was passed into this method.
   const Point2I inOffset( offset );

   //Parent::onRender( offset, updateRect );

   // We render our fill, borders, and child controls ourself.
   // This allows us to render child controls after we render cells,
   // so child controls appear on-top, as they should.   

   // Render our fill and borders.
   // This code from GuiControl::onRender().
   {      
      RectI ctrlRect(offset, getExtent());

      //if opaque, fill the update rect with the fill color
      if ( mProfile->mOpaque )
         GFX->getDrawUtil()->drawRectFill(ctrlRect, mProfile->mFillColor);

      //if there's a border, draw the border
      if ( mProfile->mBorder )
         renderBorder(ctrlRect, mProfile);
   }

   //make sure we have a parent
   GuiControl *parent = getParent();
   if (! parent)
      return;

   S32 i, j;
   RectI headerClip;
   RectI clipRect(updateRect.point, updateRect.extent);

   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));

   //if we have column headings
   if (mHeaderDim.y > 0)
   {
      headerClip.point.x =   parentOffset.x + mHeaderDim.x;
      headerClip.point.y =   parentOffset.y;
      headerClip.extent.x =  clipRect.extent.x;// - headerClip.point.x; // This seems to fix some strange problems with some Gui's, bug? -pw
      headerClip.extent.y =  mHeaderDim.y;

      if (headerClip.intersect(clipRect))
      {
         GFX->setClipRect(headerClip);

         //now render the header
         onRenderColumnHeaders(offset, parentOffset, mHeaderDim);

         clipRect.point.y += headerClip.extent.y;
         clipRect.extent.y -= headerClip.extent.y;
      }
      offset.y += mHeaderDim.y;
   }

   //if we have row headings
   if (mHeaderDim.x > 0)
   {
      clipRect.point.x = getMax(clipRect.point.x, parentOffset.x + mHeaderDim.x);
      offset.x += mHeaderDim.x;
   }

   //save the original for clipping the row headers
   RectI origClipRect = clipRect;

   for (j = 0; j < mSize.y; j++)
   {
      //skip until we get to a visible row
      if ((j + 1) * mCellSize.y + offset.y < updateRect.point.y)
         continue;

      //break once we've reached the last visible row
      if(j * mCellSize.y + offset.y >= updateRect.point.y + updateRect.extent.y)
         break;

      //render the header
      if (mHeaderDim.x > 0)
      {
         headerClip.point.x = parentOffset.x;
         headerClip.extent.x = mHeaderDim.x;
         headerClip.point.y = offset.y + j * mCellSize.y;
         headerClip.extent.y = mCellSize.y;
         if (headerClip.intersect(origClipRect))
         {
            GFX->setClipRect(headerClip);

            //render the row header
            onRenderRowHeader(Point2I(0, offset.y + j * mCellSize.y),
                              Point2I(parentOffset.x, offset.y + j * mCellSize.y),
                              mHeaderDim, Point2I(0, j));
         }
      }

      //render the cells for the row
      for (i = 0; i < mSize.x; i++)
      {
         //skip past columns off the left edge
         if ((i + 1) * mCellSize.x + offset.x < updateRect.point.x)
            continue;

         //break once past the last visible column
         if (i * mCellSize.x + offset.x >= updateRect.point.x + updateRect.extent.x)
            break;

         S32 cellx = offset.x + i * mCellSize.x;
         S32 celly = offset.y + j * mCellSize.y;

         RectI cellClip(cellx, celly, mCellSize.x, mCellSize.y);

         //make sure the cell is within the update region
         if (cellClip.intersect(clipRect))
         {
            //set the clip rect
            GFX->setClipRect(cellClip);

            //render the cell
            onRenderCell(Point2I(cellx, celly), Point2I(i, j),
               i == mSelectedCell.x && j == mSelectedCell.y,
               i == mMouseOverCell.x && j == mMouseOverCell.y);
         }
      }
   }

   // Done rendering cells.
   // Render child controls, if any, on top.
   renderChildControls( inOffset, updateRect );
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseDown( const GuiEvent &event )
{
   if ( !mActive || !mAwake || !mVisible )
      return;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseUp( const GuiEvent &event )
{
   if ( !mActive || !mAwake || !mVisible )
      return;

   //let the guiControl method take care of the rest
   Parent::onMouseUp(event);

   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell(
         (pt.x < 0 ? -1 : pt.x / mCellSize.x), 
         (pt.y < 0 ? -1 : pt.y / mCellSize.y)
      );

   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      //store the previously selected cell
      Point2I prevSelected = mSelectedCell;

      //select the new cell
      cellSelected(Point2I(cell.x, cell.y));

      //if we double clicked on the *same* cell, evaluate the altConsole Command
      if ( ( event.mouseClickCount > 1 ) && ( prevSelected == mSelectedCell ) )
         execAltConsoleCallback();
   }
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseEnter(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;

   //get the cell
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      mMouseOverCell = cell;
      setUpdateRegion(Point2I(cell.x * mCellSize.x + mHeaderDim.x,
                              cell.y * mCellSize.y + mHeaderDim.y), mCellSize );
	  onCellHighlighted(mMouseOverCell);
   }
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseLeave(const GuiEvent & /*event*/)
{
   setUpdateRegion(Point2I(mMouseOverCell.x * mCellSize.x + mHeaderDim.x,
                           mMouseOverCell.y * mCellSize.y + mHeaderDim.y), mCellSize);
   mMouseOverCell.set(-1,-1);
   onCellHighlighted(mMouseOverCell);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseDragged(const GuiEvent &event)
{
   // for the array control, the behavior of onMouseDragged is the same
   // as on mouse moved - basically just recalc the current mouse over cell
   // and set the update regions if necessary
   GuiArrayCtrl::onMouseMove(event);
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onMouseMove(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x != mMouseOverCell.x || cell.y != mMouseOverCell.y)
   {
      if (mMouseOverCell.x != -1)
      {
         setUpdateRegion(Point2I(mMouseOverCell.x * mCellSize.x + mHeaderDim.x,
                           mMouseOverCell.y * mCellSize.y + mHeaderDim.y), mCellSize);
      }

      if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
      {
         setUpdateRegion(Point2I(cell.x * mCellSize.x + mHeaderDim.x,
                           cell.y * mCellSize.y + mHeaderDim.y), mCellSize);
         mMouseOverCell = cell;
      }
      else
         mMouseOverCell.set(-1,-1);
   }
   onCellHighlighted(mMouseOverCell);
}

//-----------------------------------------------------------------------------

bool GuiArrayCtrl::onKeyDown(const GuiEvent &event)
{
   //if this control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake)) return true;

   //get the parent
   S32 pageSize = 1;
   GuiControl *parent = getParent();
   if (parent && mCellSize.y > 0)
   {
      pageSize = getMax(1, (parent->getHeight() / mCellSize.y) - 1);
   }

   Point2I delta(0,0);
   switch (event.keyCode)
   {
      case KEY_LEFT:
         delta.set(-1, 0);
         break;
      case KEY_RIGHT:
         delta.set(1, 0);
         break;
      case KEY_UP:
         delta.set(0, -1);
         break;
      case KEY_DOWN:
         delta.set(0, 1);
         break;
      case KEY_PAGE_UP:
         delta.set(0, -pageSize);
         break;
      case KEY_PAGE_DOWN:
         delta.set(0, pageSize);
         break;
      case KEY_HOME:
         cellSelected( Point2I( 0, 0 ) );
         return( true );
      case KEY_END:
         cellSelected( Point2I( 0, mSize.y - 1 ) );
         return( true );
      default:
         return Parent::onKeyDown(event);
   }
   if (mSize.x < 1 || mSize.y < 1)
      return true;

   //select the first cell if no previous cell was selected
   if (mSelectedCell.x == -1 || mSelectedCell.y == -1)
   {
      cellSelected(Point2I(0,0));
      return true;
   }

   //select the cell
   Point2I cell = mSelectedCell;
   cell.x = getMax(0, getMin(mSize.x - 1, cell.x + delta.x));
   cell.y = getMax(0, getMin(mSize.y - 1, cell.y + delta.y));
   cellSelected(cell);

   return true;
}

//-----------------------------------------------------------------------------

void GuiArrayCtrl::onRightMouseDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return;

   Parent::onRightMouseDown( event );

   Point2I cell;
   if( _findHitCell( event.mousePoint, cell ) )
   {
      char buf[32];
      dSprintf( buf, sizeof( buf ), "%d %d", event.mousePoint.x, event.mousePoint.y );

      // [rene, 21-Jan-11 ] clashes with callbacks defined in derived classes

      // Pass it to the console:
  	   Con::executef(this, "onRightMouseDown", Con::getIntArg(cell.x), Con::getIntArg(cell.y), buf);
   }
}

//-----------------------------------------------------------------------------

bool GuiArrayCtrl::_findHitCell( const Point2I& pos, Point2I& cellOut )
{
   Point2I pt = globalToLocalCoord( pos );
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell( ( pt.x < 0 ? -1 : pt.x / mCellSize.x ), ( pt.y < 0 ? -1 : pt.y / mCellSize.y ) );
   if( cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y )
   {
      cellOut = cell;
      return true;
   }

   return false;
}