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

#ifndef _TICKCACHE_H_
#define _TICKCACHE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

struct Move;

struct TickCacheEntry
{
   enum { MaxPacketSize=140 };

   U8 packetData[MaxPacketSize];
   TickCacheEntry * next;
   Move * move;

   // If you want to assign moves to tick cache for later playback, allocate them here
   Move * allocateMove();
};

struct TickCacheHead;

class TickCache
{
public:
   TickCache();
   ~TickCache();

   TickCacheEntry * addCacheEntry();
   void dropOldest();
   void dropNextOldest();
   void ageCache(S32 numToAge, S32 len);
   void setCacheSize(S32 len);
   void beginCacheList();
   TickCacheEntry * incCacheList(bool addIfNeeded=true);

private:
   TickCacheHead * mTickCacheHead;
};

inline TickCache::TickCache()
{
   mTickCacheHead = NULL;
}

#endif // _TICKCACHE_H_