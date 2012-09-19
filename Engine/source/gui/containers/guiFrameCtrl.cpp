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
#include "gui/containers/guiFrameCtrl.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT(GuiFrameSetCtrl);

ConsoleDocClass( GuiFrameSetCtrl,
   "@brief A gui control allowing a window to be subdivided into panes, each "
   "of which displays a gui control child of the GuiFrameSetCtrl.\n\n"

   "Each gui control child will have an associated FrameDetail through which "
   "frame-specific details can be assigned. Frame-specific values override the "
   "values specified for the entire frameset.\n\n"

   "Note that it is possible to have more children than frames, or more frames "
   "than children. In the former case, the extra children will not be visible "
   "(they are moved beyond the visible extent of the frameset). In the latter "
   "case, frames will be empty. For example, if a frameset had two columns and "
   "two rows but only three gui control children they would be assigned to "
   "frames as follows:\n"

   "<pre>\n"
   "                 1 | 2\n"
   "                 -----\n"
   "                 3 |\n"
   "</pre>\n"

   "The last frame would be blank.\n\n"

   "@tsexample\n"
   "new GuiFrameSetCtrl()\n"
   "{\n"
   "   columns = \"3\";\n"
   "   rows = \"2\";\n"
   "   borderWidth = \"1\";\n"
   "   borderColor = \"128 128 128\";\n"
   "   borderEnable = \"dynamic\";\n"
   "   borderMovable = \"dynamic\";\n"
   "   autoBalance = \"1\";\n"
   "   fudgeFactor = \"0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

//-----------------------------------------------------------------------------

ImplementEnumType( GuiFrameState,
   "\n\n"
   "@ingroup GuiContainers" )
   { GuiFrameSetCtrl::FRAME_STATE_ON,    "alwaysOn"  },
   { GuiFrameSetCtrl::FRAME_STATE_OFF,   "alwaysOff" },
   { GuiFrameSetCtrl::FRAME_STATE_AUTO,  "dynamic"   }
EndImplementEnumType;

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::initPersistFields()
{
   addField( "columns", TypeS32Vector, Offset(mColumnOffsets, GuiFrameSetCtrl),
      "A vector of column offsets (determines the width of each column)." );

   addField( "rows", TypeS32Vector, Offset(mRowOffsets, GuiFrameSetCtrl),
      "A vector of row offsets (determines the height of each row)." );

   addField( "borderWidth", TypeS32, Offset(mFramesetDetails.mBorderWidth, GuiFrameSetCtrl),
      "Width of interior borders between cells in pixels." );

   addField( "borderColor", TypeColorI, Offset(mFramesetDetails.mBorderColor, GuiFrameSetCtrl),
      "Color of interior borders between cells." );

   addField( "borderEnable", TYPEID< FrameState >(), Offset(mFramesetDetails.mBorderEnable, GuiFrameSetCtrl),
      "Controls whether frame borders are enabled.\n\nFrames use this value "
      "unless overridden for that frame using <i>%ctrl.frameBorder(index)</i>" );

   addField( "borderMovable", TYPEID< FrameState >(), Offset(mFramesetDetails.mBorderMovable, GuiFrameSetCtrl),
      "Controls whether borders can be dynamically repositioned with the mouse "
      "by the user.\n\nFrames use this value unless overridden for that frame "
      "using <i>%ctrl.frameMovable(index)</i>" );

   addField( "autoBalance", TypeBool, Offset(mAutoBalance, GuiFrameSetCtrl),
      "If true, row and column offsets are automatically scaled to match the "
      "new extents when the control is resized." );

   addField( "fudgeFactor", TypeS32, Offset(mFudgeFactor, GuiFrameSetCtrl),
      "Offset for row and column dividers in pixels" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
DefineEngineMethod( GuiFrameSetCtrl, frameBorder, void, ( S32 index, const char* state ), ( "dynamic" ),
   "Override the <i>borderEnable</i> setting for this frame.\n\n"
   "@param index  Index of the frame to modify\n"
   "@param state  New borderEnable state: \"on\", \"off\" or \"dynamic\"\n" )
{
   object->frameBorderEnable( index, state );
}

DefineEngineMethod( GuiFrameSetCtrl, frameMovable, void, ( S32 index, const char* state ), ( "dynamic" ),
   "Override the <i>borderMovable</i> setting for this frame.\n\n"
   "@param index  Index of the frame to modify\n"
   "@param state  New borderEnable state: \"on\", \"off\" or \"dynamic\"\n" )
{
   object->frameBorderMovable( index, state );
}

DefineEngineMethod( GuiFrameSetCtrl, frameMinExtent, void, ( S32 index, S32 width, S32 height ),,
   "Set the minimum width and height for the frame. It will not be possible "
   "for the user to resize the frame smaller than this.\n\n"
   "@param index  Index of the frame to modify\n"
   "@param width  Minimum width in pixels\n"
   "@param height Minimum height in pixels\n" )
{
   Point2I extent( getMax( 0, width ), getMax( 0, height ) );
   object->frameMinExtent( index, extent);
}

DefineEngineMethod( GuiFrameSetCtrl, framePadding, void, ( S32 index, RectSpacingI padding ),,
   "Set the padding for this frame. Padding introduces blank space on the inside "
   "edge of the frame.\n\n"
   "@param index     Index of the frame to modify\n"
   "@param padding   Frame top, bottom, left, and right padding\n" )
{
   object->framePadding( index, padding);
}

DefineEngineMethod( GuiFrameSetCtrl, getFramePadding, RectSpacingI, ( S32 index ),,
   "Get the padding for this frame.\n\n"
   "@param index     Index of the frame to query\n" )
{
   return object->getFramePadding( index );
}

DefineEngineMethod( GuiFrameSetCtrl, addColumn, void, (),,
   "Add a new column.\n\n" )
{
   Vector<S32> * columns = object->columnOffsets();
   columns->push_back(0);
   object->balanceFrames();
}

DefineEngineMethod( GuiFrameSetCtrl, addRow, void, (),,
   "Add a new row.\n\n" )
{
   Vector<S32> * rows = object->rowOffsets();
   rows->push_back(0);
   object->balanceFrames();
}

DefineEngineMethod( GuiFrameSetCtrl, removeColumn, void, (),,
   "Remove the last (rightmost) column.\n\n" )
{
   Vector<S32> * columns = object->columnOffsets();

   if(columns->size() > 0)
   {
      columns->setSize(columns->size() - 1);
      object->balanceFrames();
   }
   else
      Con::errorf(ConsoleLogEntry::General, "No columns exist to remove");
}

DefineEngineMethod( GuiFrameSetCtrl, removeRow, void, (),,
   "Remove the last (bottom) row.\n\n" )
{
   Vector<S32> * rows = object->rowOffsets();

   if(rows->size() > 0)
   {
      rows->setSize(rows->size() - 1);
      object->balanceFrames();
   }
   else
      Con::errorf(ConsoleLogEntry::General, "No rows exist to remove");
}

DefineEngineMethod( GuiFrameSetCtrl, getColumnCount, S32, (),,
   "Get the number of columns.\n\n"
   "@return The number of columns\n" )
{
   return(object->columnOffsets()->size());
}

DefineEngineMethod( GuiFrameSetCtrl, getRowCount, S32, (),,
   "Get the number of rows.\n\n"
   "@return The number of rows\n" )
{
   return(object->rowOffsets()->size());
}

DefineEngineMethod( GuiFrameSetCtrl, getColumnOffset, S32, ( S32 index ),,
   "Get the horizontal offset of a column.\n\n"
   "@param index Index of the column to query\n"
   "@return Column offset in pixels\n" )
{
   if(index < 0 || index > object->columnOffsets()->size())
   {
      Con::errorf(ConsoleLogEntry::General, "Column index out of range");
      return(0);
   }
   return((*object->columnOffsets())[index]);
}

DefineEngineMethod( GuiFrameSetCtrl, getRowOffset, S32, ( S32 index ),,
   "Get the vertical offset of a row.\n\n"
   "@param index Index of the row to query\n"
   "@return Row offset in pixels\n" )
{
   if(index < 0 || index > object->rowOffsets()->size())
   {
      Con::errorf(ConsoleLogEntry::General, "Row index out of range");
      return(0);
   }
   return((*object->rowOffsets())[index]);
}

DefineEngineMethod( GuiFrameSetCtrl, setColumnOffset, void, ( S32 index, S32 offset ),,
   "Set the horizontal offset of a column.\n\n"
   "Note that column offsets must always be in increasing order, and therefore "
   "this offset must be between the offsets of the colunns either side.\n"
   "@param index  Index of the column to modify\n"
   "@param offset New column offset\n" )
{
   Vector<S32> & columns = *(object->columnOffsets());

   if(index < 0 || index > columns.size())
   {
      Con::errorf(ConsoleLogEntry::General, "Column index out of range");
      return;
   }

   // check the offset
   if(((index > 0) && (offset < columns[index-1])) ||
      ((index < (columns.size() - 1)) && (offset > columns[index+1])))
   {
      Con::errorf(ConsoleLogEntry::General, "Invalid column offset");
      return;
   }

   columns[index] = offset;
   object->updateSizes();
}

DefineEngineMethod( GuiFrameSetCtrl, setRowOffset, void, ( S32 index, S32 offset ),,
   "Set the vertical offset of a row.\n\n"
   "Note that row offsets must always be in increasing order, and therefore "
   "this offset must be between the offsets of the rows either side.\n"
   "@param index  Index of the row to modify\n"
   "@param offset New row offset\n" )
{
   Vector<S32> & rows = *(object->rowOffsets());

   if(index < 0 || index > rows.size())
   {
      Con::errorf(ConsoleLogEntry::General, "Row index out of range");
      return;
   }

   // check the offset
   if(((index > 0) && (offset < rows[index-1])) ||
      ((index < (rows.size() - 1)) && (offset > rows[index+1])))
   {
      Con::errorf(ConsoleLogEntry::General, "Invalid row offset");
      return;
   }

   rows[index] = offset;
   object->updateSizes();
}

DefineEngineMethod( GuiFrameSetCtrl, updateSizes, void, (),,
   "Recalculates child control sizes." )
{
   object->updateSizes();
}

//-----------------------------------------------------------------------------
GuiFrameSetCtrl::GuiFrameSetCtrl()
{
   VECTOR_SET_ASSOCIATION(mColumnOffsets);
   VECTOR_SET_ASSOCIATION(mRowOffsets);
   VECTOR_SET_ASSOCIATION(mFrameDetails);

   mAutoBalance = true;
   mIsContainer = true;

   init(1, 1, NULL, NULL);
}

//-----------------------------------------------------------------------------
GuiFrameSetCtrl::GuiFrameSetCtrl(U32 columns, U32 rows, const U32 columnOffsets[], const U32 rowOffsets[])
{
   init(columns, rows, columnOffsets, rowOffsets);
}

//-----------------------------------------------------------------------------
GuiFrameSetCtrl::~GuiFrameSetCtrl()
{
   while (mFrameDetails.size() > 0)
   {
      delete mFrameDetails.last();
      mFrameDetails.pop_back();
   }
}

//-----------------------------------------------------------------------------

void GuiFrameSetCtrl::addObject(SimObject *object)
{
   AssertFatal(object != NULL, "GuiFrameSetCtrl::addObject: NULL object");

   // assign the object to a frame - give it default frame details
   Parent::addObject(object);
   GuiControl *gc = dynamic_cast<GuiControl *>(object);
   if (gc != NULL)
   {
      FrameDetail *detail = new FrameDetail;
      detail->mMinExtent = gc->getMinExtent();
      mFrameDetails.push_back(detail);
   }
   else
      mFrameDetails.push_back(NULL);
   // resize it to fit into the frame to which it is assigned (if no frame for it, don't bother resizing)
   if(isAwake())
      computeSizes();
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::removeObject(SimObject *object)
{
   if (object != NULL)
   {
      VectorPtr<SimObject *>::iterator soitr;
      VectorPtr<FrameDetail *>::iterator fditr = mFrameDetails.begin();
      for (soitr = begin(); soitr != end(); soitr++, fditr++)
      {
         if (*soitr == object)
         {
            delete *fditr;
            mFrameDetails.erase(fditr);
            break;
         }
      }
   }
   Parent::removeObject(object);

   if(isAwake())
      computeSizes();
}

//-----------------------------------------------------------------------------
bool GuiFrameSetCtrl::resize(const Point2I &newPos, const Point2I &newExtent)
{
   // rebalance before losing the old extent (if required)
   if (mAutoBalance == true)
      rebalance(newExtent);

   if( !Parent::resize(newPos, newExtent) )
      return false;

   // compute new sizing info for the frames - takes care of resizing the children
   computeSizes( !mAutoBalance );

   return true;
}

//-----------------------------------------------------------------------------

void GuiFrameSetCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return;

   Region curRegion = NONE;
   // Determine the region by mouse position.
   Point2I curMousePos = globalToLocalCoord(lastGuiEvent.mousePoint);
   curRegion = pointInAnyRegion(curMousePos);

   PlatformWindow *pWindow = pRoot->getPlatformWindow();
   AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
   PlatformCursorController *pController = pWindow->getCursorController();
   AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

   switch (curRegion)
   {
   case VERTICAL_DIVIDER:
      // change to left-right cursor
      if(pRoot->mCursorChanged != PlatformCursorController::curResizeVert)
      {
         //*** We've already changed the cursor, so set it back before we change it again.
         if(pRoot->mCursorChanged != -1)
            pController->popCursor();

         //*** Now change the cursor shape
         pController->pushCursor(PlatformCursorController::curResizeVert);
         pRoot->mCursorChanged = PlatformCursorController::curResizeVert;

      }
      break;

   case HORIZONTAL_DIVIDER:
      // change to up-down cursor
      if(pRoot->mCursorChanged != PlatformCursorController::curResizeHorz)
      {
         //*** We've already changed the cursor, so set it back before we change it again.
         if(pRoot->mCursorChanged != -1)
            pController->popCursor();

         //*** Now change the cursor shape
         pController->pushCursor(PlatformCursorController::curResizeHorz);
         pRoot->mCursorChanged = PlatformCursorController::curResizeHorz;
      }
      break;

   case DIVIDER_INTERSECTION:
      // change to move cursor
      if(pRoot->mCursorChanged != PlatformCursorController::curResizeAll)
      {
         //*** We've already changed the cursor, so set it back before we change it again.
         if(pRoot->mCursorChanged != -1)
            pController->popCursor();

         //*** Now change the cursor shape
         pController->pushCursor(PlatformCursorController::curResizeAll);
         pRoot->mCursorChanged = PlatformCursorController::curResizeAll;

      }
      break;

   case NONE:
   default:
      if(pRoot->mCursorChanged != -1)
      {
         //*** We've already changed the cursor, so set it back before we change it again.
         pController->popCursor();

         pRoot->mCursorChanged = -1;
      }
      break;
   }

}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::onMouseDown(const GuiEvent &event)
{
   if (mFramesetDetails.mBorderEnable != FRAME_STATE_OFF && mFramesetDetails.mBorderMovable != FRAME_STATE_OFF)
   {
      // determine if a divider was hit
      Point2I curMousePos = globalToLocalCoord(event.mousePoint);
      findHitRegion(curMousePos);                        // sets mCurVerticalHit, mCurHorizontalHit, & mCurHitRegion

      if (mCurHitRegion != NONE)
      {
         mouseLock();
         setFirstResponder();
         setUpdate();
      }
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::onMouseUp(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
   if (mCurHitRegion != NONE)
   {
      mCurHitRegion = NONE;
      mCurVerticalHit = NO_HIT;
      mCurHorizontalHit = NO_HIT;
      mouseUnlock();
      setUpdate();
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::onMouseDragged(const GuiEvent &event)
{
   if (mCurHitRegion != NONE)
   {
      // identify the frames involved in the resizing, checking if they are resizable
      S32 indexes[4];
      S32 activeFrames = findResizableFrames(indexes);
      if (activeFrames > 0)
      {
         S32 range[4];
         // determine the range of movement, limiting as specified by individual frames
         computeMovableRange(mCurHitRegion, mCurVerticalHit, mCurHorizontalHit, activeFrames, indexes, range);
         Point2I curMousePos = globalToLocalCoord(event.mousePoint);
         switch (mCurHitRegion)
         {
            case VERTICAL_DIVIDER:
               mColumnOffsets[mCurVerticalHit] = getMin(getMax(range[0], curMousePos.x - mLocOnDivider.x), range[1]);
               break;
            case HORIZONTAL_DIVIDER:
               mRowOffsets[mCurHorizontalHit] = getMin(getMax(range[0], curMousePos.y - mLocOnDivider.y), range[1]);
               break;
            case DIVIDER_INTERSECTION:
               mColumnOffsets[mCurVerticalHit] = getMin(getMax(range[0], curMousePos.x - mLocOnDivider.x), range[1]);
               mRowOffsets[mCurHorizontalHit] = getMin(getMax(range[2], curMousePos.y - mLocOnDivider.y), range[3]);
               break;
            default:
               return;
         }
         computeSizes();
      }
   }
}

//-----------------------------------------------------------------------------
bool GuiFrameSetCtrl::onAdd()
{
   if (Parent::onAdd() == false)
      return(false);

   return(true);
}

bool GuiFrameSetCtrl::onWake()
{
   if (Parent::onWake() == false)
      return(false);

   computeSizes();
   return(true);
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::onRender(Point2I offset, const RectI &updateRect )
{

   Parent::onRender( offset, updateRect );
 
   drawDividers(offset);

}

//-----------------------------------------------------------------------------
bool GuiFrameSetCtrl::init(U32 columns, U32 rows, const U32 columnOffsets[], const U32 rowOffsets[])
{
   if (columns != 0 && rows != 0)
   {
      mColumnOffsets.clear();
      mRowOffsets.clear();
      U32 i;
      for (i = 0; i < columns; i++)
      {
         if (columnOffsets == NULL)
            mColumnOffsets.push_back(0);
         else
         {
            AssertFatal(columnOffsets != NULL, "GuiFrameSetCtrl::init: NULL column offsets");
            mColumnOffsets.push_back((U32)columnOffsets[i]);
            if (i > 0)
            {
               AssertFatal(mColumnOffsets[i - 1] < mColumnOffsets[i], "GuiFrameSetCtrl::init: column offsets must be monotonically increasing");
               mColumnOffsets.clear();
               return(false);
            }
         }
      }
      for (i = 0; i < rows; i++)
      {
         if (rowOffsets == NULL)
            mRowOffsets.push_back(0);
         else
         {
            AssertFatal(rowOffsets != NULL, "GuiFrameSetCtrl::init: NULL row offsets");
            mRowOffsets.push_back((U32)rowOffsets[i]);
            if (i > 0)
            {
               AssertFatal(mRowOffsets[i - 1] < mRowOffsets[i], "GuiFrameSetCtrl::init: row offsets must be monotonically increasing");
               mRowOffsets.clear();
               return(false);
            }
         }
      }
   }
   mFramesetDetails.mBorderWidth = DEFAULT_BORDER_WIDTH;
   mFramesetDetails.mBorderEnable = FRAME_STATE_AUTO;
   mFramesetDetails.mBorderMovable = FRAME_STATE_AUTO;
   mAutoBalance = false;
   mFudgeFactor = 0;
   mCurHitRegion = NONE;
   mCurVerticalHit = NO_HIT;
   mCurHorizontalHit = NO_HIT;
   return(true);
}

//-----------------------------------------------------------------------------
// point is assumed to already be in local coordinates.
GuiFrameSetCtrl::Region GuiFrameSetCtrl::findHitRegion(const Point2I &point)
{
   Vector<S32>::iterator itr;
   S32 i = 1;
   // step through vertical dividers
   for (itr = mColumnOffsets.begin() + 1; itr < mColumnOffsets.end(); itr++, i++)
   {
      if (hitVerticalDivider(*itr, point) == true)
      {
         mCurVerticalHit = i;
         mLocOnDivider.x = point.x - (*itr);
         break;
      }
   }
   i = 1;
   // step through horizontal dividers
   for (itr = mRowOffsets.begin() + 1; itr < mRowOffsets.end(); itr++, i++)
   {
      if (hitHorizontalDivider(*itr, point) == true)
      {
         mCurHorizontalHit = i;
         mLocOnDivider.y = point.y - (*itr);
         break;
      }
   }
   // now set type of hit...
   if (mCurVerticalHit != NO_HIT)
   {
      if (mCurHorizontalHit != NO_HIT)
         return(mCurHitRegion = DIVIDER_INTERSECTION);
      else
         return(mCurHitRegion = VERTICAL_DIVIDER);
   }
   else if (mCurHorizontalHit != NO_HIT)
      return(mCurHitRegion = HORIZONTAL_DIVIDER);
   else
      return(mCurHitRegion = NONE);
}

GuiFrameSetCtrl::Region GuiFrameSetCtrl::pointInAnyRegion(const Point2I &point)
{
   Vector<S32>::iterator itr;
   S32 i = 1;
   S32 curVertHit = NO_HIT, curHorzHit = NO_HIT;
   Region result = NONE;
   // step through vertical dividers
   for (itr = mColumnOffsets.begin() + 1; itr < mColumnOffsets.end(); itr++, i++)
   {
      if (hitVerticalDivider(*itr, point) == true)
      {
         curVertHit = i;
         break;
      }
   }
   i = 1;
   // step through horizontal dividers
   for (itr = mRowOffsets.begin() + 1; itr < mRowOffsets.end(); itr++, i++)
   {
      if (hitHorizontalDivider(*itr, point) == true)
      {
         curHorzHit = i;
         break;
      }
   }
   // now select the type of region in which the point lies
   if (curVertHit != NO_HIT)
   {
      if (curHorzHit != NO_HIT)
         result = DIVIDER_INTERSECTION;
      else
         result = VERTICAL_DIVIDER;
   }
   else if (curHorzHit != NO_HIT)
      result = HORIZONTAL_DIVIDER;
   return(result);
}

//-----------------------------------------------------------------------------
// indexes must have at least 4 entries.
// This *may* modify mCurVerticalHit, mCurHorizontalHit, and mCurHitRegion if it
// determines that movement is disabled by frame content.
// If it does make such a change, it also needs to do the reset performed by
// onMouseUp if it sets mCurHitRegion to NONE.
S32 GuiFrameSetCtrl::findResizableFrames(S32 indexes[])
{
   AssertFatal(indexes != NULL, "GuiFrameSetCtrl::findResizableFrames: NULL indexes");

   // first, find the column and row indexes of the affected columns/rows
   S32 validIndexes = 0;
   switch (mCurHitRegion)
   {
      case VERTICAL_DIVIDER:                             // columns
         indexes[0] = mCurVerticalHit - 1;
         indexes[1] = mCurVerticalHit;
         validIndexes = 2;
         break;
      case HORIZONTAL_DIVIDER:                           // rows
         indexes[0] = mCurHorizontalHit - 1;
         indexes[1] = mCurHorizontalHit;
         validIndexes = 2;
         break;
      case DIVIDER_INTERSECTION:                         // columns & rows
         indexes[0] = mCurVerticalHit - 1;               // columns
         indexes[1] = mCurVerticalHit;
         indexes[2] = mCurHorizontalHit - 1;             // rows
         indexes[3] = mCurHorizontalHit;
         validIndexes = 4;
         break;
      default:
         break;
   }
   // now, make sure these indexes are for movable frames
   VectorPtr<SimObject *>::iterator soitr;
   VectorPtr<FrameDetail *>::iterator fditr = mFrameDetails.begin();
   GuiControl *gc;
   S32 column = 0;
   S32 row = 0;
   S32 columns = mColumnOffsets.size();
   S32 rows = mRowOffsets.size();
   for (soitr = begin(); soitr != end() && validIndexes > 0; soitr++, fditr++)
   {
      // don't continue if some of the frames are empty
      if (fditr == mFrameDetails.end())
         break;
      // otherwise, check the gui elements for move-restrictions
      gc = dynamic_cast<GuiControl *>(*soitr);
      if (gc != NULL)
      {
         if (column == columns)
         {
            column = 0;
            row++;
         }
         if (row == rows)
            break;
         switch (mCurHitRegion)
         {
            case VERTICAL_DIVIDER:
               if ((column == indexes[0] || column == indexes[1]) && (*fditr) && (*fditr)->mBorderMovable == FRAME_STATE_OFF)
                  validIndexes = 0;
               break;
            case HORIZONTAL_DIVIDER:
               if ((row == indexes[0] || row == indexes[1]) && (*fditr) && (*fditr)->mBorderMovable == FRAME_STATE_OFF)
                  validIndexes = 0;
               break;
            case DIVIDER_INTERSECTION:
               if ((column == indexes[0] || column == indexes[1]) && (*fditr) && (*fditr)->mBorderMovable == FRAME_STATE_OFF)
               {
                  if ((row == indexes[2] || row == indexes[3]) && (*fditr) && (*fditr)->mBorderMovable == FRAME_STATE_OFF)
                     validIndexes = 0;
                  else
                  {
                     mCurHitRegion = HORIZONTAL_DIVIDER;
                     mCurVerticalHit = NO_HIT;
                     indexes[0] = indexes[2];
                     indexes[1] = indexes[3];
                     validIndexes = 2;
                  }
               }
               else if ((row == indexes[2] || row == indexes[3]) && (*fditr) && (*fditr)->mBorderMovable == FRAME_STATE_OFF)
               {
                  mCurHitRegion = VERTICAL_DIVIDER;
                  mCurHorizontalHit = NO_HIT;
                  validIndexes = 2;
               }
               break;
            default:
               return(0);
         }
         column++;
      }
   }
   if (validIndexes == 0)
   {
      mCurHitRegion = NONE;
      mCurVerticalHit = NO_HIT;
      mCurHorizontalHit = NO_HIT;
      mouseUnlock();
      setUpdate();
   }
   return(validIndexes);
}

//-----------------------------------------------------------------------------
// This method locates the gui control and frame detail associated with a
// particular frame index.
bool GuiFrameSetCtrl::findFrameContents(S32 index, GuiControl **gc, FrameDetail **fd)
{
   AssertFatal(gc != NULL, "GuiFrameSetCtrl::findFrameContents: NULL gui control pointer");
   AssertFatal(fd != NULL, "GuiFrameSetCtrl::findFrameContents: NULL frame detail pointer");
   AssertFatal(*gc == NULL, "GuiFrameSetCtrl::findFrameContents: contents of gui control must be NULL");
   AssertFatal(*fd == NULL, "GuiFrameSetCtrl::findFrameContents: contents of frame detail must be NULL");

   if (index >= 0 && index < size())
   {
      VectorPtr<SimObject *>::iterator soitr;
      VectorPtr<FrameDetail *>::iterator fditr = mFrameDetails.begin();
      for (soitr = begin(); soitr != end(); soitr++, fditr++, index--)
      {
         if (index == 0)
         {
            GuiControl *guiCtrl = dynamic_cast<GuiControl *>(*soitr);
            if (guiCtrl != NULL)
            {
               *gc = guiCtrl;
               *fd = *fditr;
               return(true);
            }
            else
               break;
         }
      }
   }
   return(false);
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::computeSizes(bool balanceFrames)
{
   S32 columns = mColumnOffsets.size();
   S32 rows = mRowOffsets.size();
   S32 vDividers = columns - 1;
   S32 hDividers = rows - 1;

   if ( !balanceFrames && mFrameDetails.size() == ( columns * rows ) )
   {
      // This will do some goofy things if you allow this control to resize smaller than
      // the additive minimum extents of its frames--so don't.
      S32 index, delta;

      if ( columns > 1 )
      {
         index = columns - 1;
         delta = mFrameDetails[index]->mMinExtent.x - ( getWidth() - mColumnOffsets[index] );
         while ( delta > 0 )
         {
            mColumnOffsets[index--] -= delta;
            if ( index >= 0 )
               delta = mFrameDetails[index]->mMinExtent.x - ( mColumnOffsets[index + 1] - mColumnOffsets[index] );
            else
               break;
         }
      }

      if ( rows > 1 )
      {
         index = rows - 1;
         delta = mFrameDetails[columns * index]->mMinExtent.y - ( getHeight() - mRowOffsets[index] );
         while ( delta > 0 )
         {
            mRowOffsets[index--] -= delta;
            if ( index >= 0 )
               delta = mFrameDetails[columns * index]->mMinExtent.y - ( mRowOffsets[index + 1] - mRowOffsets[index] );
            else
               break;
         }
      }
   }

   // first, update the divider placement if necessary
   if (balanceFrames == true && mColumnOffsets.size() > 0 && mRowOffsets.size() > 0)
   {
      Vector<S32>::iterator itr;
      F32 totWidth = F32(getWidth() - vDividers * mFramesetDetails.mBorderWidth);
      F32 totHeight = F32(getHeight() - hDividers * mFramesetDetails.mBorderWidth);
      F32 frameWidth = totWidth/(F32)columns;
      F32 frameHeight = totHeight/(F32)rows;
      F32 i = 0.;
      for (itr = mColumnOffsets.begin(); itr != mColumnOffsets.end(); itr++, i++)
         *itr = (S32)(i * (frameWidth + (F32)mFramesetDetails.mBorderWidth));
      i = 0.;
      for (itr = mRowOffsets.begin(); itr != mRowOffsets.end(); itr++, i++)
         *itr = (S32)(i * (frameHeight + (F32)mFramesetDetails.mBorderWidth));
   }

   // now, resize the contents of each frame (and move content w/o a frame beyond visible range)
   VectorPtr<SimObject *>::iterator soitr;
   VectorPtr<FrameDetail *>::iterator fditr = mFrameDetails.begin();
   GuiControl *gc;
   S32 column = 0;
   S32 row = 0;
   Point2I newPos;
   Point2I newExtent;
   // step through all the children
   for (soitr = begin(); soitr != end(); soitr++, fditr++)
   {
      // column and row track the current frame being resized
      if (column == columns)
      {
         column = 0;
         row++;
      }
      // resize the contents if its a gui control...
      gc = dynamic_cast<GuiControl *>(*soitr);
      if (gc != NULL)
      {
         if (row >= rows)
         {
            // no more visible frames
            newPos = getExtent();
            newExtent.set(DEFAULT_MIN_FRAME_EXTENT, DEFAULT_MIN_FRAME_EXTENT);
            gc->resize(newPos, newExtent);
            continue;
         }
         else
         {
            // determine x components of new position & extent
            newPos.x = mColumnOffsets[column];
            if (column == vDividers)
               newExtent.x = getWidth() - mColumnOffsets[column];             // last column
            else
               newExtent.x = mColumnOffsets[column + 1] - mColumnOffsets[column] - mFramesetDetails.mBorderWidth;   // any other column
            // determine y components of new position & extent
            newPos.y = mRowOffsets[row];
            if (row == hDividers)
               newExtent.y = getHeight() - mRowOffsets[row];                   // last row
            else
               newExtent.y = mRowOffsets[row + 1] - mRowOffsets[row] - mFramesetDetails.mBorderWidth;            // any other row

            if ( *fditr )
            {
               FrameDetail *fd = ( *fditr );

               // Account for Padding.
               newPos.x    += fd->mPadding.left;
               newPos.y    += fd->mPadding.top;
               newExtent.x -= ( fd->mPadding.left + fd->mPadding.right );
               newExtent.y -= ( fd->mPadding.top + fd->mPadding.bottom );
            }

            // apply the new position & extent
            gc->resize(newPos, newExtent);
            column++;
         }
      }
   }
}

//-----------------------------------------------------------------------------
// this method looks at the previous offsets, and uses them to redistribute
// the available height & width proportionally.
void GuiFrameSetCtrl::rebalance(const Point2I &newExtent)
{
   TORQUE_UNUSED(newExtent);

   // look at old_width and old_height - current extent
   F32 widthScale = (F32)newExtent.x/(F32)getWidth();
   F32 heightScale = (F32)newExtent.y/(F32)getHeight();
   Vector<S32>::iterator itr;
   // look at old width offsets
   for (itr = mColumnOffsets.begin() + 1; itr < mColumnOffsets.end(); itr++)
      // multiply each by new_width/old_width
      *itr = S32(F32(*itr) * widthScale);
   // look at old height offsets
   for (itr = mRowOffsets.begin() + 1; itr < mRowOffsets.end(); itr++)
      // multiply each by new_height/new_width
      *itr = S32(F32(*itr) * heightScale);

}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::computeMovableRange(Region hitRegion, S32 vertHit, S32 horzHit, S32 numIndexes, const S32 indexes[], S32 ranges[])
{
   S32 hardRanges[4] = { 0 };
   switch (numIndexes)
   {
      case 2:
         switch (hitRegion)
         {
            case VERTICAL_DIVIDER:
               ranges[0] = hardRanges[0] = (vertHit <= 1) ? mFramesetDetails.mBorderWidth : mColumnOffsets[vertHit - 1] + mFramesetDetails.mBorderWidth;
               ranges[1] = hardRanges[1] = (vertHit >= (mColumnOffsets.size() - 1)) ? getWidth() : mColumnOffsets[vertHit + 1] - mFramesetDetails.mBorderWidth;
               break;
            case HORIZONTAL_DIVIDER:
               ranges[0] = hardRanges[0] = (horzHit <= 1) ? mFramesetDetails.mBorderWidth : mRowOffsets[horzHit - 1] + mFramesetDetails.mBorderWidth;
               ranges[1] = hardRanges[1] = (horzHit >= (mRowOffsets.size() - 1)) ? getHeight() : mRowOffsets[horzHit + 1] - mFramesetDetails.mBorderWidth;
               break;
            default:
               return;
         }
         break;
      case 4:
         if (hitRegion == DIVIDER_INTERSECTION)
         {
            ranges[0] = hardRanges[0] = (vertHit <= 1) ? mFramesetDetails.mBorderWidth : mColumnOffsets[vertHit - 1] + mFramesetDetails.mBorderWidth;
            ranges[1] = hardRanges[1] = (vertHit >= (mColumnOffsets.size() - 1)) ? getWidth() : mColumnOffsets[vertHit + 1] - mFramesetDetails.mBorderWidth;
            ranges[2] = hardRanges[2] = (horzHit <= 1) ? mFramesetDetails.mBorderWidth : mRowOffsets[horzHit - 1] + mFramesetDetails.mBorderWidth;
            ranges[3] = hardRanges[3] = (horzHit >= (mRowOffsets.size() - 1)) ? getHeight() : mRowOffsets[horzHit + 1] - mFramesetDetails.mBorderWidth;
         }
         else
            return;
         break;
      default:
         return;
   }
   // now that we have the hard ranges, reduce ranges based on minimum frame extents
   VectorPtr<SimObject *>::iterator soitr;
   VectorPtr<FrameDetail *>::iterator fditr = mFrameDetails.begin();
   GuiControl *gc;
   S32 column = 0;
   S32 row = 0;
   S32 columns = mColumnOffsets.size();
   S32 rows = mRowOffsets.size();
   for (soitr = begin(); soitr != end(); soitr++, fditr++)
   {
      // only worry about visible frames
      if (column == columns)
      {
         column = 0;
         row++;
      }
      if (row == rows)
         return;
      gc = dynamic_cast<GuiControl *>(*soitr);
      if (gc != NULL)
      {
         // the gui control is in a visible frame, so look at its frame details
         if ((*fditr) != NULL)
         {
            switch (hitRegion)
            {
               case VERTICAL_DIVIDER:
                  if (column == indexes[0])
                     ranges[0] = getMax(ranges[0], hardRanges[0] + (*fditr)->mMinExtent.x);
                  if (column == indexes[1])
                     ranges[1] = getMin(ranges[1], hardRanges[1] - (*fditr)->mMinExtent.x);
                  break;
               case HORIZONTAL_DIVIDER:
                  if (row == indexes[0])
                     ranges[0] = getMax(ranges[0], hardRanges[0] + (*fditr)->mMinExtent.y);
                  if (row == indexes[1])
                     ranges[1] = getMin(ranges[1], hardRanges[1] - (*fditr)->mMinExtent.y);
                  break;
               case DIVIDER_INTERSECTION:
                  if (column == indexes[0])
                     ranges[0] = getMax(ranges[0], hardRanges[0] + (*fditr)->mMinExtent.x);
                  if (column == indexes[1])
                     ranges[1] = getMin(ranges[1], hardRanges[1] - (*fditr)->mMinExtent.x);
                  if (row == indexes[2])
                     ranges[2] = getMax(ranges[2], hardRanges[2] + (*fditr)->mMinExtent.y);
                  if (row == indexes[3])
                     ranges[3] = getMin(ranges[3], hardRanges[3] - (*fditr)->mMinExtent.y);
                  break;
               default:
                  return;
            }
         }
         column++;
      }
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::drawDividers(const Point2I &offset)
{
   // draw the frame dividers, if they are enabled
   if (mFramesetDetails.mBorderEnable != FRAME_STATE_OFF)
   {
      RectI r;
      Vector<S32>::iterator itr;
      for (itr = mColumnOffsets.begin() + 1; itr < mColumnOffsets.end(); itr++)
      {
         r.point = Point2I(*itr - mFramesetDetails.mBorderWidth, mFudgeFactor) + offset;
         r.extent.set(mFramesetDetails.mBorderWidth, getHeight() - ( 2 * mFudgeFactor ) );
         GFX->getDrawUtil()->drawRectFill(r, mFramesetDetails.mBorderColor);
      }
      for (itr = mRowOffsets.begin() + 1; itr < mRowOffsets.end(); itr++)
      {
         r.point = Point2I(mFudgeFactor, *itr - mFramesetDetails.mBorderWidth) + offset;
         r.extent.set(getWidth() - ( 2 * mFudgeFactor ), mFramesetDetails.mBorderWidth);
         GFX->getDrawUtil()->drawRectFill(r, mFramesetDetails.mBorderColor);
      }
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::frameBorderEnable(S32 index, const char *state)
{
   GuiControl *gc = NULL;
   FrameDetail *fd = NULL;
   if (findFrameContents(index, &gc, &fd) == true && fd != NULL)
   {
      if (state != NULL)
         fd->mBorderEnable = EngineUnmarshallData< FrameState >()( state );
      else
         // defaults to AUTO if NULL passed in state
         fd->mBorderEnable = FRAME_STATE_AUTO;
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::frameBorderMovable(S32 index, const char *state)
{
   GuiControl *gc = NULL;
   FrameDetail *fd = NULL;
   if (findFrameContents(index, &gc, &fd) == true && fd != NULL)
   {
      if (state != NULL)
         fd->mBorderMovable = EngineUnmarshallData< FrameState >()( state );
      else
         // defaults to AUTO if NULL passed in state
         fd->mBorderMovable = FRAME_STATE_AUTO;
   }
}

//-----------------------------------------------------------------------------
void GuiFrameSetCtrl::frameMinExtent(S32 index, const Point2I &extent)
{
   GuiControl *gc = NULL;
   FrameDetail *fd = NULL;
   if (findFrameContents(index, &gc, &fd) == true && fd != NULL)
      fd->mMinExtent = extent;
}

void GuiFrameSetCtrl::framePadding(S32 index, const RectSpacingI &padding)
{
   GuiControl *gc = NULL;
   FrameDetail *fd = NULL;
   if (findFrameContents(index, &gc, &fd) == true && fd != NULL)
      fd->mPadding = padding;
}

RectSpacingI GuiFrameSetCtrl::getFramePadding(S32 index)
{
   GuiControl *gc = NULL;
   FrameDetail *fd = NULL;
   if (findFrameContents(index, &gc, &fd) == true && fd != NULL)
      return fd->mPadding;

   return RectSpacingI( 0, 0, 0, 0 );
}