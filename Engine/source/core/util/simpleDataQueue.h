//-----------------------------------------------------------------------------
// Copyright (c) 2014 James S Urquhart
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

#ifndef _SIMPLEDATAQUEUE_H_
#define _SIMPLEDATAQUEUE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class SimpleDataQueue
{
public:
   typedef struct Block
   {
      U32 size;
      U8 *data;
   } Block;

   SimpleDataQueue(U32 blockSize) :
   mBlockSize(blockSize),
      mBlockPos(blockSize)
   {
   }

   ~SimpleDataQueue()
   {
      clear();
   }

   void clear()
   {
      while (!mBlockQueue.empty())
      {
         dFree(mBlockQueue.front().data);
         mBlockQueue.pop_front();
      }
      mBlockPos = mBlockSize;
   }

   void write(U32 bytes, const void *data)
   {
      U32 bytesToWrite = bytes;
      const char *ptr = (const char*)data;

      while (bytesToWrite != 0)
      {
         U32 bytesLeftInCurrentBlock = mBlockSize - mBlockPos;
         if (bytesLeftInCurrentBlock == 0)
         {
            // Push a new block
            Block block;
            block.size = 0;
            block.data = (U8*)dMalloc(mBlockSize);
            bytesLeftInCurrentBlock = mBlockSize;
            mBlockPos = 0;
            mBlockQueue.push_back(block);
         }

         // Copy onto back block
         Block *block = mBlockQueue.end()-1;
         U32 toWrite = bytesToWrite > bytesLeftInCurrentBlock ? bytesLeftInCurrentBlock : bytesToWrite;
         U8 *blockPtr = block->data + (mBlockSize - bytesLeftInCurrentBlock);
         dMemcpy(blockPtr, ptr, toWrite);
         ptr += toWrite;
         block->size += toWrite;
         bytesToWrite -= toWrite;
         mBlockPos += toWrite;
      }
   }

   // Advances bytes on from read head
   void expendBuffer(U32 maxSize)
   {
      U32 totalBytes = size();
      U32 bytesToWrite = totalBytes > maxSize ? maxSize : totalBytes;

      while (bytesToWrite != 0)
      {
         if (!mBlockQueue.empty())
         {
            // Copy next message
            Block msg = mBlockQueue[0];
            U32 bytesToWriteInBlock = bytesToWrite > msg.size ? msg.size : bytesToWrite;

            if (bytesToWriteInBlock > 0)
            {
               bytesToWrite -= bytesToWriteInBlock;
               msg.size -= bytesToWriteInBlock;
            }

            // Copy remaining message memory to front
            if (msg.size > 0)
            {
               U8 *start = msg.data + bytesToWriteInBlock;
               dMemcpy(msg.data, start, msg.size);
            }

            // Update size pointer
            mBlockQueue.front().size = msg.size;

            // Erase the block if its empty
            if (msg.size == 0)
            {
               mBlockQueue.pop_front();
               if (mBlockQueue.empty())
               {
                  // Make sure we have at least 1 block available
                  mBlockQueue.push_front(msg);
                  mBlockPos = 0;
                  break; // no more data left
               }
               else
               {
                  dFree(msg.data);
               }
            }
            else
            {
               // Check if we are the head block. If so, update the write position
               if (msg.data == mBlockQueue.back().data)
               {
                  mBlockPos = msg.size;
               }
            }
         }
         else
         {
            // Can't write anything else
            break;
         }
      }
   }

   // Copies bytes from read head
   U32 copyToBuffer(void *buffer, U32 maxSize)
   {
      U32 totalBytes = size();
      U32 bytesToWrite = totalBytes > maxSize ? maxSize : totalBytes;
      U8 *ptr = (U8*)buffer;

      // Copy block queue so we can manipulate blocks
      Vector<Block> exploreQueue = mBlockQueue;

      while (bytesToWrite != 0)
      {
         if (!exploreQueue.empty())
         {
            // Copy next message
            Block msg = exploreQueue[0];
            U32 bytesToWriteInBlock = bytesToWrite > msg.size ? msg.size : bytesToWrite;

            if (bytesToWriteInBlock > 0)
            {
               dMemcpy(ptr, msg.data, bytesToWriteInBlock);
               bytesToWrite -= bytesToWriteInBlock;
               msg.size -= bytesToWriteInBlock;
               ptr += bytesToWriteInBlock;
            }

            // Update size pointer
            exploreQueue.front().size = msg.size;

            // Erase the block if its empty
            if (msg.size == 0)
            {
               exploreQueue.pop_front();
               if (exploreQueue.empty())
               {
                  // Make sure we have at least 1 block available
                  exploreQueue.push_front(msg);
                  break; // no more data left
               }
            }
         }
         else
         {
            // Can't write anything else
            break;
         }
      }

      return ptr - (U8*)buffer;
   }

   U32 size()
   {
      U32 count = 0;
      for (Vector<Block>::iterator itr = mBlockQueue.begin(); itr != mBlockQueue.end(); itr++)
      {
         count += (*itr).size;
      }
      return count;
   }

   U32 blocks()
   {
      U32 count = 0;
      for (Vector<Block>::iterator itr = mBlockQueue.begin(); itr != mBlockQueue.end(); itr++)
      {
         count++;
      }
      return count;
   }

protected:
   U32 mBlockSize;
   U32 mBlockPos;

   Vector<Block> mBlockQueue;
};

#endif
