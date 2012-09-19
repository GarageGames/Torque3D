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

#include "console/engineAPI.h"
#include "platform/platform.h"
#include "gui/containers/guiDynamicCtrlArrayCtrl.h"


GuiDynamicCtrlArrayControl::GuiDynamicCtrlArrayControl()
{
   mCols = 0;
   mColSize = 64;
   mRows = 0;
   mRowSize = 64;
   mRowSpacing = 0;
   mColSpacing = 0;
   mIsContainer = true;

   mResizing = false;

   mSizeToChildren = false;
   mAutoCellSize = false;

   mFrozen = false;
   mDynamicSize = false;
   mFillRowFirst = true;

   mPadding.set( 0, 0, 0, 0 );
}

GuiDynamicCtrlArrayControl::~GuiDynamicCtrlArrayControl()
{
}

IMPLEMENT_CONOBJECT(GuiDynamicCtrlArrayControl);

ConsoleDocClass( GuiDynamicCtrlArrayControl,
   "@brief A container that arranges children into a grid.\n\n"

   "This container maintains a 2D grid of GUI controls. If one is added, deleted, "
   "or resized, then the grid is updated. The insertion order into the grid is "
   "determined by the internal order of the children (ie. the order of addition).<br>"

   "Children are added to the grid by row or column until they fill the assocated "
   "GuiDynamicCtrlArrayControl extent (width or height). For example, a "
   "GuiDynamicCtrlArrayControl with 15 children, and <i>fillRowFirst</i> set to "
   "true may be arranged as follows:\n\n"

   "<pre>\n"
   "1  2  3  4  5  6\n"
   "7  8  9  10 11 12\n"
   "13 14 15\n"
   "</pre>\n"

   "If <i>dynamicSize</i> were set to true in this case, the GuiDynamicCtrlArrayControl "
   "height would be calculated to fit the 3 rows of child controls.\n\n"

   "@tsexample\n"
   "new GuiDynamicCtrlArrayControl()\n"
   "{\n"
   "   colSize = \"128\";\n"
   "   rowSize = \"18\";\n"
   "   colSpacing = \"2\";\n"
   "   rowSpacing = \"2\";\n"
   "   frozen = \"0\";\n"
   "   autoCellSize = \"1\";\n"
   "   fillRowFirst = \"1\";\n"
   "   dynamicSize = \"1\";\n"
   "   padding = \"0 0 0 0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

// ConsoleObject...

void GuiDynamicCtrlArrayControl::initPersistFields()
{
   addField( "colCount", TypeS32, Offset( mCols, GuiDynamicCtrlArrayControl ),
      "Number of columns the child controls have been arranged into. This "
      "value is calculated automatically when children are added, removed or "
      "resized; writing it directly has no effect." );

   addField( "colSize", TypeS32, Offset( mColSize, GuiDynamicCtrlArrayControl ),
      "Width of each column. If <i>autoCellSize</i> is set, this will be "
      "calculated automatically from the widest child control" );

   addField( "rowCount", TypeS32, Offset( mRows, GuiDynamicCtrlArrayControl ),
      "Number of rows the child controls have been arranged into. This value "
      "is calculated automatically when children are added, removed or resized; "
      "writing it directly has no effect." );

   addField( "rowSize", TypeS32, Offset( mRowSize, GuiDynamicCtrlArrayControl ),
      "Height of each row. If <i>autoCellSize</i> is set, this will be "
      "calculated automatically from the tallest child control" );

   addField( "rowSpacing", TypeS32, Offset( mRowSpacing, GuiDynamicCtrlArrayControl ),
      "Spacing between rows" );

   addField( "colSpacing", TypeS32, Offset( mColSpacing, GuiDynamicCtrlArrayControl ),
      "Spacing between columns" );

   addField( "frozen", TypeBool, Offset( mFrozen, GuiDynamicCtrlArrayControl ),
      "When true, the array will not update when new children are added or in "
      "response to child resize events. This is useful to prevent unnecessary "
      "resizing when adding, removing or resizing a number of child controls." );

   addField( "autoCellSize", TypeBool, Offset( mAutoCellSize, GuiDynamicCtrlArrayControl ),
      "When true, the cell size is set to the widest/tallest child control." );

   addField( "fillRowFirst", TypeBool, Offset( mFillRowFirst, GuiDynamicCtrlArrayControl ),
      "Controls whether rows or columns are filled first.\n\nIf true, controls are "
      "added to the grid left-to-right (to fill a row); then rows are added "
      "top-to-bottom as shown below:\n"
      "<pre>1 2 3 4\n"
      "5 6 7 8</pre>\n"
      "If false, controls are added to the grid top-to-bottom (to fill a column); "
      "then columns are added left-to-right as shown below:\n"
      "<pre>1 3 5 7\n"
      "2 4 6 8</pre>" );

   addField( "dynamicSize", TypeBool, Offset( mDynamicSize, GuiDynamicCtrlArrayControl ),
      "If true, the width or height of this control will be automatically "
      "calculated based on the number of child controls (width if "
      "<i>fillRowFirst</i> is false, height if <i>fillRowFirst</i> is true)." );

   addField( "padding", TypeRectSpacingI, Offset( mPadding, GuiDynamicCtrlArrayControl ),
      "Padding around the top, bottom, left, and right of this control. This "
      "reduces the area available for child controls." );

   Parent::initPersistFields();
}


// SimObject...

void GuiDynamicCtrlArrayControl::inspectPostApply()
{
   resize(getPosition(), getExtent());
   Parent::inspectPostApply();
}


// SimSet...

void GuiDynamicCtrlArrayControl::addObject(SimObject *obj)
{
   Parent::addObject(obj);

   if ( !mFrozen )
      refresh();
}


// GuiControl...

bool GuiDynamicCtrlArrayControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if ( size() == 0 )
      return Parent::resize( newPosition, newExtent );

   if ( mResizing ) 
      return false;

   mResizing = true;
   
   // Calculate the cellSize based on our widest/tallest child control
   // if the flag to do so is set.
   if ( mAutoCellSize )
   {
      mColSize = 1;
      mRowSize = 1;

      for ( U32 i = 0; i < size(); i++ )
      {
         GuiControl *child = dynamic_cast<GuiControl*>(operator [](i));
         if ( child && child->isVisible() )
         {
            if ( mColSize < child->getWidth() )
               mColSize = child->getWidth();
            if ( mRowSize < child->getHeight() )
               mRowSize = child->getHeight();
         }
      }
   }

   // Count number of visible, children guiControls.
   S32 numChildren = 0;
   for ( U32 i = 0; i < size(); i++ )
   {
      GuiControl *child = dynamic_cast<GuiControl*>(operator [](i));
      if ( child && child->isVisible() )
         numChildren++;         
   }

   // Calculate number of rows and columns.
   if ( !mFillRowFirst )
   {
      mRows = 1;
      while ( ( ( mRows + 1 ) * mRowSize + mRows * mRowSpacing ) <= ( newExtent.y - ( mPadding.top + mPadding.bottom ) ) )
         mRows++;

      mCols = numChildren / mRows;
      if ( numChildren % mRows > 0 )
         mCols++;
   }
   else
   {
      mCols = 1;
      while ( ( ( mCols + 1 ) * mColSize + mCols * mColSpacing ) <= ( newExtent.x - ( mPadding.left + mPadding.right ) ) )
         mCols++;

      mRows = numChildren / mCols;
      if ( numChildren % mCols > 0 )
         mRows++;
   }       

   // Place each child...
   S32 childcount = 0;
   for ( S32 i = 0; i < size(); i++ )
   {
      // Place control
      GuiControl *gc = dynamic_cast<GuiControl*>(operator [](i));

      // Added check if child is visible.  Invisible children don't take part
      if ( gc && gc->isVisible() ) 
      {
         S32 curCol, curRow;

         // Get the current column and row...
         if ( mFillRowFirst )
         {
            curCol = childcount % mCols;
            curRow = childcount / mCols;
         }
         else
         {
            curCol = childcount / mRows;
            curRow = childcount % mRows;
         }

         // Reposition and resize
         Point2I newPos( mPadding.left + curCol * ( mColSize + mColSpacing ), mPadding.top + curRow * ( mRowSize + mRowSpacing ) );
         gc->resize( newPos, Point2I( mColSize, mRowSize ) );

         childcount++;
      }
   }

   Point2I realExtent( newExtent );

   if ( mDynamicSize )
   {
      if ( mFillRowFirst )      
         realExtent.y = mRows * mRowSize + ( mRows - 1 ) * mRowSpacing + ( mPadding.top + mPadding.bottom );
      else
         realExtent.x = mCols * mColSize + ( mCols - 1 ) * mColSpacing + ( mPadding.left + mPadding.right );
   }
   
   mResizing = false;

   return Parent::resize( newPosition, realExtent );
}

void GuiDynamicCtrlArrayControl::childResized(GuiControl *child)
{
   Parent::childResized(child);

   if ( !mFrozen )
      refresh();
}

void GuiDynamicCtrlArrayControl::refresh()
{
   resize( getPosition(), getExtent() );
}

DefineEngineMethod( GuiDynamicCtrlArrayControl, refresh, void, (),,
   "Recalculates the position and size of this control and all its children." )
{
   object->refresh();
}
