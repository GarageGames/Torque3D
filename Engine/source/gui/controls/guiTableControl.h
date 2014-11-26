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
#ifndef _GUITABLECONTROL_H_
#define _GUITABLECONTROL_H_

#include "gui/controls/guiTextListCtrl.h"
#include "console/arrayObject.h"
#include "gui/containers/guiScrollCtrl.h"


class GuiTableControl :
   public GuiScrollCtrl
{
   typedef GuiScrollCtrl Parent;
   friend class GuiTextListCtrl;
protected:
   GuiTextListCtrl *mTextListCtrl;

   bool mChildAdded;
   S32 mHeadingSize;

   struct Heading
   {
      S32 column;
      char* text;
   };

   Heading mHeadingList;

   
   ArrayObject *mColumnOrder;

   S32 mCurVertHit;

   S32 mDividerLockPos;
public:

   enum Region { 
      DIVIDER,
      HEADER,
      NONE
   };

   Region mCurRegion;

   Vector<S32> mColumnOffsets;
   bool mFitParentWidth;

   GuiTableControl(void);
   ~GuiTableControl(void);

   DECLARE_CONOBJECT(GuiTableControl);
   DECLARE_CATEGORY( "Gui Table Control" );
   DECLARE_DESCRIPTION( "A control that displays text in tabular form, with headings." );

   DECLARE_CALLBACK(void, onSortedColumn, ( S32 columnId, const char* columnName, bool increasing ) );
   DECLARE_CALLBACK(void, onSelect, (const char* cellid, const char* text) );

   void onRender(Point2I offset, const RectI &updateRect);
   Region findHitRegion( const Point2I &mousePos );
   void addChildControls( );
   void addHeading( const char* heading, int column );
   void addChildRow(U32 id, const char *text);
   void clearChildren();
   void setColumnSort(S32 column, bool ascending );
   S32 findEntryByColumnText( S32 columnId, const char *text);
   U32 getSelectedRow();
   void setSelectedRow(Point2I cell);
   bool isLastIndex( S32 index );
   void renderSortOrder(S32 index, Point2I offset );

   //GuiScrollCtrl
   void scrollTo(S32 x, S32 y);
   void computeSizes();

   //Console
   void onStaticModified( const char* slotName, const char* newValue );

   //GuiControl
   virtual void getCursor(  GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent );
   virtual void onMouseDown( const GuiEvent &event );
   virtual void onMouseDragged( const GuiEvent &event );
   virtual void onMouseUp( const GuiEvent &event );
   GuiControl* findHitControl(const Point2I &pt, S32 initialLayer = -1);

   //Console Object
   static void initPersistFields( );
};
#endif

