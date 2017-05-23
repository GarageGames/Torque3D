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
#include "core/dataChunker.h"


//----------------------------------------------------------------------------

DataChunker::DataChunker(S32 size)
{
   mChunkSize          = size;
   mCurBlock           = NULL;
}

DataChunker::~DataChunker()
{
   freeBlocks();
}

void *DataChunker::alloc(S32 size)
{
   if (size > mChunkSize)
   {
      DataBlock * temp = (DataBlock*)dMalloc(DataChunker::PaddDBSize + size);
      AssertFatal(temp, "Malloc failed");
      constructInPlace(temp);
      if (mCurBlock)
      {
         temp->next = mCurBlock->next;
         mCurBlock->next = temp;
      }
      else
      {
         mCurBlock = temp;
         temp->curIndex = mChunkSize;
      }
      return temp->getData();
   }

   if(!mCurBlock || size + mCurBlock->curIndex > mChunkSize)
   {
      const U32 paddDBSize = (sizeof(DataBlock) + 3) & ~3;
      DataBlock *temp = (DataBlock*)dMalloc(paddDBSize+ mChunkSize);
      AssertFatal(temp, "Malloc failed");
      constructInPlace(temp);
      temp->next = mCurBlock;
      mCurBlock = temp;
   }
   
   void *ret = mCurBlock->getData() + mCurBlock->curIndex;
   mCurBlock->curIndex += (size + 3) & ~3; // dword align
   return ret;
}

DataChunker::DataBlock::DataBlock()
{
   curIndex = 0;
   next = NULL;
}

DataChunker::DataBlock::~DataBlock()
{
}

void DataChunker::freeBlocks(bool keepOne)
{
   while(mCurBlock && mCurBlock->next)
   {
      DataBlock *temp = mCurBlock->next;
      dFree(mCurBlock);
      mCurBlock = temp;
   }
   if (!keepOne)
   {
      if (mCurBlock) dFree(mCurBlock);
      mCurBlock = NULL;
   }
   else if (mCurBlock)
   {
      mCurBlock->curIndex = 0;
      mCurBlock->next = NULL;
   }
}
