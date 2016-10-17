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

#ifndef _GUITEXTLISTCTRL_H_
#define _GUITEXTLISTCTRL_H_

#ifndef _GUIARRAYCTRL_H_
#include "gui/core/guiArrayCtrl.h"
#endif

class GuiTextListCtrl : public GuiArrayCtrl
{
  private:
   typedef GuiArrayCtrl Parent;

  public:
   struct Entry
   {
      char *text;
      U32 id;
      bool active;
   };

   Vector<Entry> mList;

  protected:
   enum ScrollConst
   {
      UP = 0,
      DOWN = 1
   };
   enum : U32
   {
      InvalidId = 0xFFFFFFFF
   };
   Vector<S32> mColumnOffsets;

   bool  mFitParentWidth;
   bool  mClipColumnText;

   U32 getRowWidth(Entry *row);
   bool cellSelected(Point2I cell);
   void onCellSelected(Point2I cell);

  public:
   GuiTextListCtrl();

   DECLARE_CONOBJECT(GuiTextListCtrl);
   DECLARE_CATEGORY( "Gui Lists" );
   DECLARE_DESCRIPTION( "A control that displays text in tabular form." );
   
   DECLARE_CALLBACK( void, onSelect, (S32 cellid, const char* text));
   DECLARE_CALLBACK( void, onDeleteKey, ( S32 id ));

   static void initPersistFields();

   virtual void setCellSize( const Point2I &size ){ mCellSize = size; }
   virtual void getCellSize(       Point2I &size ){ size = mCellSize; }

   const char *getScriptValue();
   void setScriptValue(const char *value);

   U32 getNumEntries();

   void clear();
   virtual void addEntry(U32 id, const char *text);
   virtual void insertEntry(U32 id, const char *text, S32 index);
   void setEntry(U32 id, const char *text);
   void setEntryActive(U32 id, bool active);
   S32 findEntryById(U32 id);
   S32 findEntryByText(const char *text);
   bool isEntryActive(U32 id);

   U32 getEntryId(U32 index);

   bool onWake();
   void removeEntry(U32 id);
   virtual void removeEntryByIndex(S32 id);
   virtual void sort(U32 column, bool increasing = true);
   virtual void sortNumerical(U32 column, bool increasing = true);

   U32 getSelectedId();
   U32 getSelectedRow();
   const char *getSelectedText();

   bool onKeyDown(const GuiEvent &event);

   virtual void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);

   void setSize(Point2I newSize);
   void onRemove();
   void addColumnOffset(S32 offset) { mColumnOffsets.push_back(offset); }
   void clearColumnOffsets() { mColumnOffsets.clear(); }
};

#endif //_GUI_TEXTLIST_CTRL_H
