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

#ifndef _DDS_UTILS_H_
#define _DDS_UTILS_H_

#include "platform/threads/threadPool.h"

struct DDSFile;

#define ImplementCrunchDDSWorkItem(name) \
class name : public DDSUtil::crunchDDSWorkItem \
{ \
public: \
   typedef DDSUtil::crunchDDSWorkItem Parent; \
   void execute(); \
   name(DDSFile *srcDDS, GFXFormat dxtFormat = GFXFormatDXT1, bool isNormalMap = false, bool forceMainThread = false) \
      : Parent(srcDDS, dxtFormat, isNormalMap, forceMainThread) \
   { \
   } \
};

namespace DDSUtil
{
   class crunchDDSWorkItem : public ThreadPool::WorkItem
   {
   public:
      typedef ThreadPool::WorkItem Parent;
      DDSFile *mSrcDDS;
      GFXFormat mDxtFormat;
      bool mIsNormalMap;
      bool succeed;
      crunchDDSWorkItem(DDSFile *srcDDS, GFXFormat dxtFormat, bool isNormalMap, bool forceMainThread)
         : mSrcDDS(srcDDS), mDxtFormat(dxtFormat), mIsNormalMap(isNormalMap),
         succeed(false)
      {
         // TODO: When updated ThreadPool is ready, use:
         //if(forceMainThread)
         //   mFlags.set(ThreadPool::WorkItem::FlagMainThreadOnly, true);
      }
   };

   /// This uses crunchDDS() method inside, keeping for backward comp.
   bool squishDDS( DDSFile *srcDDS, const GFXFormat dxtFormat );
   void swizzleDDS( DDSFile *srcDDS, const Swizzle<U8, 4> &swizzle );

   /// Compress DDS, wait for result
   bool crunchDDS( DDSFile *srcDDS, const GFXFormat dxtFormat, bool isNormalMap = false );

   /// Compress DDS in a separate thread. After it finished, the specified WorkItem will be processed
   void crunchDDS( crunchDDSWorkItem *item );

   typedef void (*crunchCallback)(DDSFile *, bool);
   void crunchDDS( crunchCallback, DDSFile *srcDDS, const GFXFormat dxtFormat = GFXFormatDXT1, bool isNormalMap = false );

};

#endif
