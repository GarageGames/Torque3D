//-----------------------------------------------------------------------------
// Copyright (c) 2012 Daniel Buckmaster
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
#include "gui/containers/guiFlexibleArrayCtrl.h"
#include "platform/types.h"

GuiFlexibleArrayControl::GuiFlexibleArrayControl()
{
   mRows = 0;
   mRowSpacing = 0;
   mColSpacing = 0;
   mIsContainer = true;

   mResizing = false;

   mFrozen = false;

   mPadding.set(0, 0, 0, 0);
}

GuiFlexibleArrayControl::~GuiFlexibleArrayControl()
{
}

IMPLEMENT_CONOBJECT(GuiFlexibleArrayControl);

ConsoleDocClass( GuiFlexibleArrayControl,
   "@brief A container that arranges children into a grid.\n\n"

   "This container maintains a 2D grid of GUI controls. If one is added, deleted, "
   "or resized, then the grid is updated. The insertion order into the grid is "
   "determined by the internal order of the children (ie. the order of addition).<br>"

   "Children are added to the grid by row or column until they fill the assocated "
   "GuiFlexibleArrayControl extent (width or height). For example, a "
   "GuiFlexibleArrayControl with 10 children, and <i>fillRowFirst</i> set to "
   "true may be arranged as follows:\n\n"

   "<pre>\n"
   "1  2  ...3...  4\n"
   "..5..  6  7  .8.\n"
   "9 ....10....\n"
   "</pre>\n"

   "@tsexample\n"
   "new GuiFlexibleArrayControl()\n"
   "{\n"
   "   colSpacing = \"2\";\n"
   "   rowSpacing = \"2\";\n"
   "   frozen = \"0\";\n"
   "   padding = \"0 0 0 0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiDynamicCtrlArrayControl\n"
   "@ingroup GuiContainers"
);

// ConsoleObject...

void GuiFlexibleArrayControl::initPersistFields()
{
   addField("rowCount", TypeS32, Offset( mRows, GuiFlexibleArrayControl),
      "Number of rows the child controls have been arranged into. This value "
      "is calculated automatically when children are added, removed or resized; "
      "writing it directly has no effect.");

   addField("rowSpacing", TypeS32, Offset(mRowSpacing, GuiFlexibleArrayControl),
      "Spacing between rows");

   addField("colSpacing", TypeS32, Offset(mColSpacing, GuiFlexibleArrayControl),
      "Spacing between columns");

   addField("frozen", TypeBool, Offset(mFrozen, GuiFlexibleArrayControl),
      "When true, the array will not update when new children are added or in "
      "response to child resize events. This is useful to prevent unnecessary "
      "resizing when adding, removing or resizing a number of child controls.");

   addField("padding", TypeRectSpacingI, Offset(mPadding, GuiFlexibleArrayControl),
      "Padding around the top, bottom, left, and right of this control. This "
      "reduces the area available for child controls.");

   Parent::initPersistFields();
}

void GuiFlexibleArrayControl::inspectPostApply()
{
   resize(getPosition(), getExtent());
   Parent::inspectPostApply();
}

void GuiFlexibleArrayControl::addObject(SimObject *obj)
{
   Parent::addObject(obj);

   if(!mFrozen)
      refresh();
}

bool GuiFlexibleArrayControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if(size() == 0)
      return Parent::resize(newPosition, newExtent);

   if(mResizing) 
      return false;

   mResizing = true;

   // Place each child.
   S32 childcount = 0;
   Point2I pos(mPadding.left, mPadding.top);
   mRows = 0;
   S32 rowHeight = 0;
   for(S32 i = 0; i < size(); i++)
   {
      GuiControl *gc = dynamic_cast<GuiControl*>(operator [](i));
      if(gc && gc->isVisible()) 
      {
         if(pos.x + gc->getWidth() > getExtent().x - mPadding.right)
         {
            pos.y += rowHeight + mRowSpacing;
            pos.x = mPadding.left;
            rowHeight = 0;
            mRows++;
         }
         gc->setPosition(pos);

         rowHeight = getMax(rowHeight, gc->getHeight());

         pos.x += mColSpacing + gc->getWidth();
         childcount++;
      }
   }

   Point2I realExtent(newExtent);
   realExtent.y = pos.y + rowHeight;
   realExtent.y += mPadding.bottom;

   mResizing = false;

   return Parent::resize(newPosition, realExtent);
}

void GuiFlexibleArrayControl::childResized(GuiControl *child)
{
   Parent::childResized(child);

   if ( !mFrozen )
      refresh();
}

void GuiFlexibleArrayControl::refresh()
{
   resize(getPosition(), getExtent());
}

DefineEngineMethod( GuiFlexibleArrayControl, refresh, void, (),,
   "Recalculates the position and size of this control and all its children." )
{
   object->refresh();
}