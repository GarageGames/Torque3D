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

#ifndef _GUISTACKCTRL_H_
#define _GUISTACKCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#include "gfx/gfxDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"

/// A stack of GUI controls.
///
/// This maintains a horizontal or vertical stack of GUI controls. If one is deleted, or
/// resized, then the stack is resized to fit. The order of the stack is
/// determined by the internal order of the children (ie, order of addition).
class GuiStackControl : public GuiControl
{
protected:
   typedef GuiControl Parent;
   bool  mResizing;
   S32   mPadding;
   S32   mStackHorizSizing;      ///< Set from horizSizingOptions.
   S32   mStackVertSizing;       ///< Set from vertSizingOptions.
   S32   mStackingType;
   bool  mDynamicSize;           ///< Resize this control along the stack axis to fit the summed extent of the children (width or height depends on the stack type)
   bool  mDynamicNonStackExtent; ///< Resize this control along the non-stack axis to fit the max extent of the children (width or height depends on the stack type)
   bool  mDynamicPos;            ///< Reposition this control along the stack axis when it is resized (by mDynamicSize) (left or up depends on the stack type)
   bool  mChangeChildSizeToFit;  ///< Does the child resize to fit i.e. should a horizontal stack resize its children's height to fit?
   bool  mChangeChildPosition;   ///< Do we reset the child's position in the opposite direction we are stacking?

public:
   GuiStackControl();

   enum StackingType
   {
      stackingTypeVert,  ///< Always stack vertically
      stackingTypeHoriz, ///< Always stack horizontally
      stackingTypeDyn    ///< Dynamically switch based on width/height
   };

   enum HorizontalType
   {
      horizStackLeft = 0,///< Stack from left to right when horizontal
      horizStackRight,   ///< Stack from right to left when horizontal
   };

   enum VerticalType
   {
      vertStackTop,      ///< Stack from top to bottom when vertical
      vertStackBottom,   ///< Stack from bottom to top when vertical
   };

   bool resize(const Point2I &newPosition, const Point2I &newExtent);
   void childResized(GuiControl *child);
   bool isFrozen() { return mResizing; };
   /// prevent resizing. useful when adding many items.
   void freeze(bool);

   bool onWake();
   void onSleep();

   void updatePanes();

   virtual void stackVertical(bool fromTop);
   virtual void stackHorizontal(bool fromLeft);

   S32 getCount() { return size(); }; /// Returns the number of children in the stack

   void addObject(SimObject *obj);
   void removeObject(SimObject *obj);

   bool reOrder(SimObject* obj, SimObject* target = 0);

   static void initPersistFields();
   
   DECLARE_CONOBJECT(GuiStackControl);
   DECLARE_CATEGORY( "Gui Containers" );
   DECLARE_DESCRIPTION( "A container that stacks its children horizontally or vertically." );
};

typedef GuiStackControl::StackingType GuiStackingType;
typedef GuiStackControl::HorizontalType GuiHorizontalStackingType;
typedef GuiStackControl::VerticalType GuiVerticalStackingType;

DefineEnumType( GuiStackingType );
DefineEnumType( GuiHorizontalStackingType );
DefineEnumType( GuiVerticalStackingType );

#endif