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
#include "gui/containers/guiStackCtrl.h"

IMPLEMENT_CONOBJECT(GuiStackControl);

ConsoleDocClass( GuiStackControl,
   "@brief A container that stacks its children horizontally or vertically.\n\n"

   "This container maintains a horizontal or vertical stack of GUI controls. If "
   "one is added, deleted, or resized, then the stack is resized to fit. The "
   "order of the stack is determined by the internal order of the children (ie. "
   "the order of addition).<br>"

   "@tsexample\n"
   "new GuiStackControl()\n"
   "{\n"
   "   stackingType = \"Dynamic\";\n"
   "   horizStacking = \"Left to Right\";\n"
   "   vertStacking = \"Top to Bottom\";\n"
   "   padding = \"4\";\n"
   "   dynamicSize = \"1\";\n"
   "   dynamicNonStackExtent = \"0\";\n"
   "   dynamicPos = \"0\";\n"
   "   changeChildSizeToFit = \"1\";\n"
   "   changeChildPosition = \"1\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

ImplementEnumType( GuiStackingType,
   "Stacking method used to position child controls.\n\n"
   "@ingroup GuiContainers" )
   { GuiStackControl::stackingTypeVert, "Vertical", "Stack children vertically by setting their Y position" },
   { GuiStackControl::stackingTypeHoriz,"Horizontal", "Stack children horizontall by setting their X position" },
   { GuiStackControl::stackingTypeDyn,"Dynamic", "Automatically switch between "
      "Vertical and Horizontal stacking. Vertical stacking is chosen when the "
      "stack control is taller than it is wide, horizontal stacking is chosen "
      "when the stack control is wider than it is tall." }
EndImplementEnumType;

ImplementEnumType( GuiHorizontalStackingType,
   "Determines how child controls are stacked horizontally.\n\n"
   "@ingroup GuiContainers" )
   { GuiStackControl::horizStackLeft, "Left to Right", "Child controls are positioned in order from left to right (left-most control is first)" },
   { GuiStackControl::horizStackRight,"Right to Left", "Child controls are positioned in order from right to left (right-most control is first)" }
EndImplementEnumType;

ImplementEnumType( GuiVerticalStackingType,
   "Determines how child controls are stacked vertically.\n\n"
   "@ingroup GuiContainers" )
   { GuiStackControl::vertStackTop, "Top to Bottom", "Child controls are positioned in order from top to bottom (top-most control is first)" },
   { GuiStackControl::vertStackBottom,"Bottom to Top", "Child controls are positioned in order from bottom to top (bottom-most control is first)" }
EndImplementEnumType;


GuiStackControl::GuiStackControl()
{
   setMinExtent(Point2I(16,16));
   mResizing = false;
   mStackingType = stackingTypeVert;
   mStackVertSizing = vertStackTop;
   mStackHorizSizing = horizStackLeft;
   mPadding = 0;
   mIsContainer = true;
   mDynamicSize = true;
   mDynamicNonStackExtent = false;
   mDynamicPos = false;
   mChangeChildSizeToFit = true;
   mChangeChildPosition = true;
}

void GuiStackControl::initPersistFields()
{

   addGroup( "Stacking" );
   addField( "stackingType", TYPEID< StackingType >(), Offset(mStackingType, GuiStackControl),
      "Determines the method used to position the child controls.\n\n" );

   addField( "horizStacking", TYPEID< HorizontalType >(), Offset(mStackHorizSizing, GuiStackControl),
      "Controls the type of horizontal stacking to use (<i>Left to Right</i> or "
      "<i>Right to Left</i>)" );

   addField( "vertStacking", TYPEID< VerticalType >(), Offset(mStackVertSizing, GuiStackControl),
      "Controls the type of vertical stacking to use (<i>Top to Bottom</i> or "
      "<i>Bottom to Top</i>)" );

   addField( "padding", TypeS32, Offset(mPadding, GuiStackControl),
      "Distance (in pixels) between stacked child controls." );

   addField( "dynamicSize", TypeBool, Offset(mDynamicSize, GuiStackControl),
      "Determines whether to resize the stack control along the stack axis (change "
      "width for horizontal stacking, change height for vertical stacking).\n\n"
      "If true, the stack width/height will be resized to the sum of the child control widths/heights. "
      "If false, the stack will not be resized." );

   addField( "dynamicNonStackExtent", TypeBool, Offset(mDynamicNonStackExtent, GuiStackControl),
      "Determines whether to resize the stack control along the non-stack axis (change "
      "height for horizontal stacking, change width for vertical stacking). No effect "
      "if dynamicSize is false.\n\n"
      "If true, the stack will be resized to the maximum of the child control widths/heights. "
      "If false, the stack will not be resized." );

   addField( "dynamicPos", TypeBool, Offset(mDynamicPos, GuiStackControl),
      "Determines whether to reposition the stack along the stack axis when it is "
      "auto-resized. No effect if dynamicSize is false.\n\n"
      "If true, the stack will grow left for horizontal stacking, and grow up for vertical stacking.\n"
      "If false, the stack will grow right for horizontal stacking, and grow down for vertical stacking.\n" );

   addField( "changeChildSizeToFit", TypeBool, Offset(mChangeChildSizeToFit, GuiStackControl),
      "Determines whether to resize child controls.\n\n"
      "If true, horizontally stacked children keep their width, but have their "
      "height set to the stack control height. Vertically stacked children keep "
      "their height, but have their width set to the stack control width. If "
      "false, child controls are not resized." );

   addField( "changeChildPosition", TypeBool, Offset(mChangeChildPosition, GuiStackControl),
      "Determines whether to reposition child controls.\n\n"
      "If true, horizontally stacked children are aligned along the top edge of "
      "the stack control. Vertically stacked children are aligned along the left "
      "edge of the stack control. If false, horizontally stacked children retain "
      "their Y position, and vertically stacked children retain their X position." );
   endGroup( "Stacking" );

   Parent::initPersistFields();
}

DefineEngineMethod( GuiStackControl, isFrozen, bool, (),,
   "Return whether or not this control is frozen" )
{
   return object->isFrozen();
}

DefineEngineMethod( GuiStackControl, freeze, void, ( bool freeze ),,
   "Prevents control from restacking - useful when adding or removing child controls\n"
   "@param freeze True to freeze the control, false to unfreeze it\n\n"
   "@tsexample\n"
   "%stackCtrl.freeze(true);\n"
   "// add controls to stack\n"
   "%stackCtrl.freeze(false);\n"
   "@endtsexample\n" )
{
   object->freeze( freeze );
}

DefineEngineMethod( GuiStackControl, updateStack, void, (),,
   "Restack the child controls.\n" )
{
   object->updatePanes();
}

bool GuiStackControl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   updatePanes();

   return true;
}

void GuiStackControl::onSleep()
{
   Parent::onSleep();
}

void GuiStackControl::updatePanes()
{
   // Prevent recursion
   if(mResizing) 
      return;

   // Set Resizing.
   mResizing = true;

   Point2I extent = getExtent();

   // Do we need to stack horizontally?
   if( ( extent.x > extent.y && mStackingType == stackingTypeDyn ) || mStackingType == stackingTypeHoriz )
   {
      stackHorizontal( mStackHorizSizing == horizStackLeft );
   }
   // Or, vertically?
   else if( ( extent.y > extent.x && mStackingType == stackingTypeDyn ) || mStackingType == stackingTypeVert)
   {
      stackVertical( mStackVertSizing == vertStackTop );
   }

   // Clear Sizing Flag.
   mResizing = false;
}

void GuiStackControl::freeze(bool _shouldfreeze)
{
   mResizing = _shouldfreeze;
}

void GuiStackControl::stackVertical(bool fromTop)
{
   if( empty() )
      return;

   S32 begin, end, step;
   if ( fromTop )
   {
      // Stack from Child0 at top to ChildN at bottom
      begin = 0;
      end = size();
      step = 1;
   }
   else
   {
      // Stack from ChildN at top to Child0 at bottom
      begin = size()-1;
      end = -1;
      step = -1;
   }

   // Place each visible child control
   S32 maxWidth = 0;
   Point2I curPos(0, 0);
   for ( S32 i = begin; i != end; i += step )
   {
      GuiControl * gc = dynamic_cast<GuiControl*>( at(i) );
      if ( gc && gc->isVisible() )
      {
         // Add padding between controls
         if ( curPos.y > 0 )
            curPos.y += mPadding;

         Point2I childPos = curPos;
         if ( !mChangeChildPosition )
            childPos.x = gc->getLeft();

         Point2I childSize( gc->getExtent() );
         if ( mChangeChildSizeToFit )
            childSize.x = getWidth();

         gc->resize( childPos, childSize );

         curPos.y += gc->getHeight();
         maxWidth = getMax( maxWidth, childPos.x + childSize.x );
      }
   }

   if ( mDynamicSize )
   {
      // Conform our size to the sum of the child sizes.
      Point2I newPos( getPosition() );
      Point2I newSize( mDynamicNonStackExtent ? maxWidth : getWidth(), curPos.y );

      newSize.setMax( getMinExtent() );

      // Grow the stack up instead of down?
      if ( mDynamicPos )
         newPos.y -= ( newSize.y - getHeight() );

      resize( newPos, newSize );
   }
}

void GuiStackControl::stackHorizontal(bool fromLeft)
{
   if( empty() )
      return;

   S32 begin, end, step;
   if ( fromLeft )
   {
      // Stack from Child0 at left to ChildN at right
      begin = 0;
      end = size();
      step = 1;
   }
   else
   {
      // Stack from ChildN at left to Child0 at right
      begin = size()-1;
      end = -1;
      step = -1;
   }

   // Place each visible child control
   S32 maxHeight = 0;
   Point2I curPos(0, 0);
   for ( S32 i = begin; i != end; i += step )
   {
      GuiControl * gc = dynamic_cast<GuiControl*>( at(i) );
      if ( gc && gc->isVisible() )
      {
         // Add padding between controls
         if ( curPos.x > 0 )
            curPos.x += mPadding;

         Point2I childPos = curPos;
         if ( !mChangeChildPosition )
            childPos.y = gc->getTop();

         Point2I childSize( gc->getExtent() );
         if ( mChangeChildSizeToFit )
            childSize.y = getHeight();

         gc->resize( childPos, childSize );

         curPos.x += gc->getWidth();
         maxHeight = getMax( maxHeight, childPos.y + childSize.y );
      }
   }

   if ( mDynamicSize )
   {
      // Conform our size to the sum of the child sizes.
      Point2I newPos( getPosition() );
      Point2I newSize( curPos.x, mDynamicNonStackExtent ? maxHeight : getHeight() );

      newSize.setMax( getMinExtent() );

      // Grow the stack left instead of right?
      if ( mDynamicPos )
         newPos.x -= ( newSize.x - getWidth() );

      resize( newPos, newSize );
   }
}

bool GuiStackControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if( !Parent::resize( newPosition, newExtent ) )
      return false;

   updatePanes();

   // CodeReview This logic should be updated to correctly return true/false
   //  based on whether it sized it's children. [7/1/2007 justind]
   return true;
}

void GuiStackControl::addObject(SimObject *obj)
{
   Parent::addObject(obj);

   updatePanes();
}

void GuiStackControl::removeObject(SimObject *obj)
{
   Parent::removeObject(obj);

   updatePanes();
}

bool GuiStackControl::reOrder(SimObject* obj, SimObject* target)
{
   bool ret = Parent::reOrder(obj, target);
   if (ret)
      updatePanes();

   return ret;
}

void GuiStackControl::childResized(GuiControl *child)
{
   updatePanes();
}