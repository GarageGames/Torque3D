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

#ifndef _FORESTCELLBATCH_H_
#define _FORESTCELLBATCH_H_

#include "scene/sceneRenderState.h"

class ForestItem;


class ForestCellBatch 
{
protected:

   /// Used to detect when the batch rendering 
   /// objects need to be repacked.
   bool mDirty; 

   /// The items in the batch.
   Vector<ForestItem> mItems;

   /// The world space bounding box of this batch.
   Box3F mBounds;

   virtual bool _prepBatch( const ForestItem &item ) = 0;
   virtual void _rebuildBatch() = 0;
   virtual void _render( const SceneRenderState *state ) = 0;

public:
   
   ForestCellBatch();
   virtual ~ForestCellBatch();

   bool add( const ForestItem &item );
   S32 getItemCount() const { return mItems.size(); }

   void render( SceneRenderState *state );
   const Box3F& getWorldBox() const { return mBounds; }
};


#endif // _FORESTCELLBATCH_H_
