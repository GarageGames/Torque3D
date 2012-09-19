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

#ifndef _FOREST_EDITOR_UNDO_H_
#define _FOREST_EDITOR_UNDO_H_

#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _FORESTITEM_H_
#include "forest/forestItem.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

class ForestData;
class ForestEditorCtrl;

class ForestUndoAction : public UndoAction
{
   typedef UndoAction Parent;

public:

   ForestUndoAction( const Resource<ForestData> &data, ForestEditorCtrl *editor, const char *description );   

   // UndoAction
   virtual void undo() {}
   virtual void redo() {}

protected:

   ForestEditorCtrl *mEditor;
   Vector<ForestItem> mItems;
   Resource<ForestData> mData;
};

class ForestCreateUndoAction : public ForestUndoAction
{
   typedef ForestUndoAction Parent;

public:
  
   ForestCreateUndoAction( const Resource<ForestData> &data,
                           ForestEditorCtrl *editor );

   /// Adds the item to the Forest and stores 
   /// its info for undo later.
   void addItem( ForestItemData *data,
                 const Point3F &position,
                 F32 rotation,
                 F32 scale );

   // UndoAction
   virtual void undo();
   virtual void redo();
};


class ForestDeleteUndoAction : public ForestUndoAction
{
   typedef ForestUndoAction Parent;

public:
  
   ForestDeleteUndoAction( const Resource<ForestData> &data,
                           ForestEditorCtrl *editor );

   ///
   void removeItem( const ForestItem &item );
   void removeItem( const Vector<ForestItem> &itemList );

   // UndoAction
   virtual void undo();
   virtual void redo();
};


class ForestUpdateAction : public ForestUndoAction
{
   typedef ForestUndoAction Parent;

public:
  
   ForestUpdateAction(  const Resource<ForestData> &data,
                        ForestEditorCtrl *editor );

   void saveItem( const ForestItem &item );

   virtual void undo() { _swapState(); }
   virtual void redo() { _swapState(); }

protected:

   void _swapState();
};

#endif // _FOREST_EDITOR_UNDO_H_



