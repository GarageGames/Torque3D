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
#include "forest/editor/forestUndo.h"

#include "forest/forestDataFile.h"
#include "forest/editor/forestEditorCtrl.h"


ForestUndoAction::ForestUndoAction( const Resource<ForestData> &data, 
                                    ForestEditorCtrl *editor,
                                    const char *description )
 : UndoAction( description ),
   mData( data ),
   mEditor( editor )
{
}



ForestCreateUndoAction::ForestCreateUndoAction( const Resource<ForestData> &data,
                                                ForestEditorCtrl *editor )
 : ForestUndoAction( data, editor, "Create Forest Items" )
{
}

void ForestCreateUndoAction::addItem( ForestItemData *data,
                                      const Point3F &position,
                                      F32 rotation,
                                      F32 scale )
{
   const ForestItem &item = mData->addItem( data, position, rotation, scale );
   mItems.push_back( item );

   // We store the datablock ID rather than the actual pointer
   // since the pointer could go bad.
   SimObjectId dataId = item.getData()->getId();
   mItems.last().setData( (ForestItemData*)dataId );
}

void ForestCreateUndoAction::redo()
{
   for ( U32 i = 0; i < mItems.size(); i++ )
   {
      const ForestItem &item = mItems[i];

      // Not 64bit safe!
      // We store the datablock ID rather than the actual pointer
      // since the pointer could go bad.
      ForestItemData *data;
      if ( !Sim::findObject( (SimObjectId)(item.getData()), data ) )
      {
         Con::errorf( "ForestCreateUndoAction::redo() - ForestItemData for item to restore does not seem to exist. Undo stack may be hosed." );
         continue;
      }

      mData->addItem( item.getKey(),
                      data,
                      item.getTransform(),
                      item.getScale() );
   }

   mEditor->onUndoAction();
}

void ForestCreateUndoAction::undo()
{
   for ( S32 i = mItems.size()-1; i >= 0; i-- )
   {
      const ForestItem &item = mItems[i];
      mData->removeItem( item.getKey(), item.getPosition() );
   }

   mEditor->onUndoAction();
}



ForestDeleteUndoAction::ForestDeleteUndoAction( const Resource<ForestData> &data,
                                                ForestEditorCtrl *editor )
 : ForestUndoAction( data, editor, "Delete Forest Items" )
{
}

void ForestDeleteUndoAction::removeItem( const ForestItem &item )
{
   // Not 64bit safe!
   // We store the datablock ID rather than the actual pointer
   // since the pointer could go bad.
   SimObjectId dataId = item.getData()->getId();

   mItems.push_back( item );
   mItems.last().setData( (ForestItemData*)dataId );
   mData->removeItem( item.getKey(), item.getPosition() );
}

void ForestDeleteUndoAction::removeItem( const Vector<ForestItem> &itemList )
{
   for ( S32 i = 0; i < itemList.size(); i++ )
      removeItem( itemList[i] );   
}

void ForestDeleteUndoAction::redo()
{
   for ( U32 i = 0; i < mItems.size(); i++ )
   {
      const ForestItem &item = mItems[i];
      mData->removeItem( item.getKey(), item.getPosition() );
   }

   mEditor->onUndoAction();
}

void ForestDeleteUndoAction::undo()
{
   for ( S32 i = mItems.size()-1; i >= 0; i-- )
   {
      const ForestItem &item = mItems[i];

      // We store the datablock ID rather than the actual pointer
      // since the pointer could go bad.
      ForestItemData *data;
      if ( !Sim::findObject( (SimObjectId)(item.getData()), data ) )
      {
         Con::errorf( "ForestDeleteUndoAction::undo() - ForestItemData for item to restore does not seem to exist. Undo stack may be hosed." );
         continue;
      }

      mData->addItem( item.getKey(),
                      data,
                      item.getTransform(),
                      item.getScale() );
   }

   mEditor->onUndoAction();
}



ForestUpdateAction::ForestUpdateAction( const Resource<ForestData> &data, 
                                        ForestEditorCtrl *editor )
 : ForestUndoAction( data, editor, "Update Forest Items" )
{
}

void ForestUpdateAction::saveItem( const ForestItem &item )
{
   // We just store the current state... we undo it later.
   mItems.push_back( item );

   // We store the datablock ID rather than the actual pointer
   // since the pointer could go bad.
   SimObjectId dataId = item.getData()->getId();   
   mItems.last().setData( (ForestItemData*)dataId );   
}

void ForestUpdateAction::_swapState()
{
   Vector<ForestItem> prevItems = mItems;
   mItems.clear();
   
   for ( U32 i=0; i < prevItems.size(); i++ )
   {
      const ForestItem &item = prevItems[i];

      // Save the current state so we can reverse this swap.
      //
      // Note that we do 'not' want the const ref returned by findItem
      // because when we call updateItem below we would lose our copy
      // of the items state before the call.
      //
      ForestItem newItem = mData->findItem( item.getKey() );

      if ( !newItem.isValid() )
      {
         Con::errorf( "ForestUpdateAction::_swapState() - saved item no longer exists. Undo stack may be hosed." );
         continue;
      }

      // Not 64bit safe!
      // We store the datablock ID rather than the actual pointer
      // since the pointer could go bad.
      ForestItemData *data;
      if ( !Sim::findObject( (SimObjectId)(item.getData()), data ) )
      {
         Con::errorf( "ForestUpdateAction::_swapState() - ForestItemData for item to restore does not seem to exist. Undo stack may be hosed." );
         continue;
      }

      // Now revert to the old state.
      mData->updateItem( item.getKey(),
                         item.getPosition(), 
                         data,
                         item.getTransform(),
                         item.getScale() );

      // Save the state before this swap for the next swap.      
      newItem.setData( (ForestItemData*)data->getId() );
      mItems.push_back( newItem );
   }

   mEditor->onUndoAction();
}
