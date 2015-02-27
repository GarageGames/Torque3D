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

#ifndef _GUIICONLISTCTRL_H_
#define _GUIICONLISTCTRL_H_

#ifndef _GUIARRAYCTRL_H_
#include "gui/core/guiArrayCtrl.h"
#endif

class GuiIconListCtrl : public GuiArrayCtrl
{
  private:
   typedef GuiArrayCtrl Parent;

  public:
   struct Entry
   {
	  String iconPath;
	  GFXTexHandle mTextureObject;
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
   enum {
      InvalidId = 0xFFFFFFFF
   };
   Vector<S32> mColumnOffsets;

   bool  mFitParentWidth;
   bool  mClipColumnText;
   RectI mIconBounds;

   U32 getRowWidth(Entry *row);
   bool cellSelected(Point2I cell);
   void onCellSelected(Point2I cell);

  public:
   GuiIconListCtrl();

   DECLARE_CONOBJECT(GuiIconListCtrl);
   DECLARE_CATEGORY( "Gui Lists" );
   DECLARE_DESCRIPTION( "A control that displays text in tabular form." );
   
   Delegate<bool(GuiIconListCtrl* sender, S32 index, const String& text)> selectEvent;
   Delegate<bool(GuiIconListCtrl* sender, S32 index)> deleteEvent;

   DECLARE_CALLBACK( void, onSelect, (const char* cellid, const char* text));
   DECLARE_CALLBACK( void, onDeleteKey, ( const char* id ));

   static void initPersistFields();

   virtual void setCellSize( const Point2I &size )
   {
	   mCellSize = size;
	   if (mIconBounds.extent.isZero())
	   {
		mIconBounds.extent = mCellSize;
	   }
   }
   virtual void getCellSize( Point2I &size ){ size = mCellSize; }

   const RectI& getIconBounds() const { return mIconBounds; }
   void setIconBounds(const RectI& value) { mIconBounds = value; }

   const char* getScriptValue();
   void setScriptValue(const char *value);

   U32 getNumEntries();

   void clear();
   virtual void addEntry(U32 id, const String& iconPath);
   virtual void insertEntry(U32 id, const String& iconPath, S32 index);
   void setEntry(U32 id, const String& iconPath);
   void setEntryActive(U32 id, bool active);
   S32 findEntryById(U32 id);
   S32 findEntryByIconPath(const String& iconPath);
   bool isEntryActive(U32 id);

   U32 getEntryId(U32 index);

   bool onWake();
   void removeEntry(U32 id);
   virtual void removeEntryByIndex(S32 id);
   virtual void sort(U32 column, bool increasing = true);
   virtual void sortNumerical(U32 column, bool increasing = true);

   U32 getSelectedId();
   U32 getSelectedRow();
   const String& getSelectedPath();

   bool onKeyDown(const GuiEvent &event);

   // Not yet implemented - refer to guiIconPopUpCtrl
   //virtual void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);

   void setSize(Point2I newSize);
   void onRemove();
   void addColumnOffset(S32 offset) { mColumnOffsets.push_back(offset); }
   void clearColumnOffsets() { mColumnOffsets.clear(); }
};

#endif //_GUIICONLISTCTRL_H_
