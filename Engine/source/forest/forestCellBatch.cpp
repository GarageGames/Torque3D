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
#include "forest/forestCellBatch.h"

#include "forest/forestItem.h"


ForestCellBatch::ForestCellBatch()
   :  mDirty( false ),
      mBounds( Box3F::Invalid )
{
}

ForestCellBatch::~ForestCellBatch()
{
}

bool ForestCellBatch::add( const ForestItem &item )
{
   // A little hacky, but don't allow more than 65K / 6 items
   // in a cell... this is generally the VB size limit on hardware.
   const U32 maxItems = 10000;
   if ( mItems.size() > maxItems )
      return false;

   // Do the pre batching tests... if it fails
   // then we cannot batch this type!
   if ( !_prepBatch( item ) )
      return false;

   // Add it to our list and we'll populate the VB at render time.
   mItems.push_back( item );
   mDirty = true;

   // Expand out bounds.
   const Box3F &box = item.getWorldBox();
   mBounds.intersect( box );
   return true;
}

void ForestCellBatch::render( SceneRenderState *state )
{
   if ( mDirty )
   {
      _rebuildBatch();
      mDirty = false;
   }

   _render( state );
}
