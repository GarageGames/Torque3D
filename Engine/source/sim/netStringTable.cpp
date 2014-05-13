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

#include "core/dnet.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"
#include "console/engineAPI.h"
#include "sim/netStringTable.h"


NetStringTable *gNetStringTable = NULL;

NetStringTable::NetStringTable()
{
   mFirstFree = 1;
   mFirstValid = 1;

   mTable = (Entry *) dMalloc(sizeof(Entry) * InitialSize);
   mSize = InitialSize;
   for(U32 i = 0; i < InitialSize; i++)
   {
      mTable[i].next = i + 1;
      mTable[i].refCount = 0;
      mTable[i].scriptRefCount = 0;
   }
   mTable[InitialSize-1].next = InvalidEntry;
   for(U32 j = 0; j < HashTableSize; j++)
      mHashTable[j] = 0;
   mAllocator = new DataChunker(DataChunkerSize);
}

NetStringTable::~NetStringTable()
{
   delete mAllocator;
   dFree( mTable );
}

void NetStringTable::incStringRef(U32 id)
{
   AssertFatal(mTable[id].refCount != 0 || mTable[id].scriptRefCount != 0 , "Cannot inc ref count from zero.");
   mTable[id].refCount++;
}

void NetStringTable::incStringRefScript(U32 id)
{
   AssertFatal(mTable[id].refCount != 0 || mTable[id].scriptRefCount != 0 , "Cannot inc ref count from zero.");
   mTable[id].scriptRefCount++;
}

U32 NetStringTable::addString(const char *string)
{
   U32 hash = _StringTable::hashString(string);
   U32 bucket = hash % HashTableSize;
   for(U32 walk = mHashTable[bucket];walk; walk = mTable[walk].next)
   {
      if(!dStrcmp(mTable[walk].string, string))
      {
         mTable[walk].refCount++;
         return walk;
      }
   }
   U32 e = mFirstFree;
   mFirstFree = mTable[e].next;
   if(mFirstFree == InvalidEntry)
   {
      // in this case, we should expand the table for next time...
      U32 newSize = mSize * 2;
      mTable = (Entry *) dRealloc(mTable, newSize * sizeof(Entry));
      for(U32 i = mSize; i < newSize; i++)
      {
         mTable[i].next = i + 1;
         mTable[i].refCount = 0;
         mTable[i].scriptRefCount = 0;
      }
      mFirstFree = mSize;
      mTable[newSize - 1].next = InvalidEntry;
      mSize = newSize;
   }
   mTable[e].refCount++;
   mTable[e].string = (char *) mAllocator->alloc(dStrlen(string) + 1);
   dStrcpy(mTable[e].string, string);
   mTable[e].next = mHashTable[bucket];
   mHashTable[bucket] = e;
   mTable[e].link = mFirstValid;
   mTable[mFirstValid].prevLink = e;
   mFirstValid = e;
   mTable[e].prevLink = 0;
   return e;
}

U32 GameAddTaggedString(const char *string)
{
   return gNetStringTable->addString(string);
}

const char *NetStringTable::lookupString(U32 id)
{
   if(mTable[id].refCount == 0 && mTable[id].scriptRefCount == 0)
      return NULL;
   return mTable[id].string;
}

void NetStringTable::removeString(U32 id, bool script)
{
   if(!script)
   {
      AssertFatal(mTable[id].refCount != 0, "Error, ref count is already 0!!");
      if(--mTable[id].refCount)
         return;
      if(mTable[id].scriptRefCount)
         return;
   }
   else
   {
      // If both ref counts are already 0, this id is not valid. Ignore
      // the remove
      if (mTable[id].scriptRefCount == 0 && mTable[id].refCount == 0)
         return;

      if(mTable[id].scriptRefCount == 0 && mTable[id].refCount)
      {
         Con::errorf("removeTaggedString failed!  Ref count is already 0 for string: %s", mTable[id].string);
         return;
      }
      if(--mTable[id].scriptRefCount)
         return;
      if(mTable[id].refCount)
         return;
   }
   // unlink first:
   U32 prev = mTable[id].prevLink;
   U32 next = mTable[id].link;
   if(next)
      mTable[next].prevLink = prev;
   if(prev)
      mTable[prev].link = next;
   else
      mFirstValid = next;
   // remove it from the hash table
   U32 hash = _StringTable::hashString(mTable[id].string);
   U32 bucket = hash % HashTableSize;
   for(U32 *walk = &mHashTable[bucket];*walk; walk = &mTable[*walk].next)
   {
      if(*walk == id)
      {
         *walk = mTable[id].next;
         break;
      }
   }
   mTable[id].next = mFirstFree;
   mFirstFree = id;
}

void NetStringTable::repack()
{
   DataChunker *newAllocator = new DataChunker(DataChunkerSize);
   for(U32 walk = mFirstValid; walk; walk = mTable[walk].link)
   {
      const char *prevStr = mTable[walk].string;


      mTable[walk].string = (char *) newAllocator->alloc(dStrlen(prevStr) + 1);
      dStrcpy(mTable[walk].string, prevStr);
   }
   delete mAllocator;
   mAllocator = newAllocator;
}

void NetStringTable::create()
{
   AssertFatal(gNetStringTable == NULL, "Error, calling NetStringTable::create twice.");
   gNetStringTable = new NetStringTable();
}

void NetStringTable::destroy()
{
   AssertFatal(gNetStringTable != NULL, "Error, not calling NetStringTable::create.");
   delete gNetStringTable;
   gNetStringTable = NULL;
}

void NetStringTable::expandString(NetStringHandle &inString, char *buf, U32 bufSize, U32 argc, const char **argv)
{
   buf[0] = StringTagPrefixByte;
   dSprintf(buf + 1, bufSize - 1, "%d ", inString.getIndex());

   const char *string = inString.getString();
   if (string != NULL) {
      U32 index = dStrlen(buf);
      while(index < bufSize)
      {
         char c = *string++;
         if(c == '%')
         {
            c = *string++;
            if(c >= '1' && c <= '9')
            {
               U32 strIndex = c - '1';
               if(strIndex >= argc)
                  continue;
               // start copying out of arg index
               const char *copy = argv[strIndex];
               // skip past any tags:
               if(*copy == StringTagPrefixByte)
               {
                  while(*copy && *copy != ' ')
                     copy++;
                  if(*copy)
                     copy++;
               }

               while(*copy && index < bufSize)
                  buf[index++] = *copy++;
               continue;
            }
         }
         buf[index++] = c;
         if(!c)
            break;
      }
      buf[bufSize - 1] = 0;
   } else {
      dStrcat(buf, "<NULL>");
   }
}

#ifdef TORQUE_DEBUG

void NetStringTable::dumpToConsole()
{
   U32 count = 0;
   S32 maxIndex = -1;
   for ( U32 i = 0; i < mSize; i++ )
   {
      if ( mTable[i].refCount > 0 || mTable[i].scriptRefCount > 0)
      {
         Con::printf( "%d: \"%c%s%c\" REF: %d", i, 0x10, mTable[i].string, 0x11, mTable[i].refCount );
         if ( maxIndex == -1 || mTable[i].refCount > mTable[maxIndex].refCount )
            maxIndex = i;
         count++;
      }
   }
   Con::printf( ">> STRINGS: %d MAX REF COUNT: %d \"%c%s%c\" <<",
         count,
         ( maxIndex == -1 ) ? 0 : mTable[maxIndex].refCount,
         0x10,
         ( maxIndex == -1 ) ? "" : mTable[maxIndex].string,
         0x11 );
}

DefineEngineFunction( dumpNetStringTable, void, (),,
   "@brief Dump the current contents of the networked string table to the console.\n\n"
   "The results are returned in three columns.  The first column is the network string ID.  "
   "The second column is the string itself.  The third column is the reference count to the "
   "network string.\n\n"
   "@note This function is available only in debug builds.\n\n"
   "@ingroup Networking" )
{
   gNetStringTable->dumpToConsole();
}

#endif // DEBUG
