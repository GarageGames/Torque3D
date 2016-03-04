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

#ifndef _SIMDICTIONARY_H_
#define _SIMDICTIONARY_H_
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif

#ifndef _PLATFORMMUTEX_H_
#include "platform/threads/mutex.h"
#endif

#include "torqueConfig.h"

class SimObject;

#ifdef USE_NEW_SIMDICTIONARY
#include <string>
#include <unordered_map>

#ifndef _SIM_H_
#include "console/sim.h"
#endif

struct StringTableEntryHash
{
   inline size_t operator()(StringTableEntry val) const
   {
      return (size_t)val;
   }
};

struct StringTableEntryEq
{
   inline bool operator()(StringTableEntry s1, StringTableEntry s2) const
   {
      return s1 == s2;
   }
};

typedef std::unordered_map<StringTableEntry, SimObject*, StringTableEntryHash, StringTableEntryEq> StringDictDef;	
typedef std::unordered_map<SimObjectId, SimObject*> SimObjectIdDictDef;
#endif

//----------------------------------------------------------------------------
/// Map of names to SimObjects
///
/// Provides fast lookup for name->object and
/// for fast removal of an object given object*
class SimNameDictionary
{
#ifndef USE_NEW_SIMDICTIONARY
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;
#else
   StringDictDef root;
#endif

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(StringTableEntry name);

   SimNameDictionary();
   ~SimNameDictionary();
};

class SimManagerNameDictionary
{
#ifndef USE_NEW_SIMDICTIONARY
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;
#else
   StringDictDef root;
#endif

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(StringTableEntry name);

   SimManagerNameDictionary();
   ~SimManagerNameDictionary();
};

//----------------------------------------------------------------------------
/// Map of ID's to SimObjects.
///
/// Provides fast lookup for ID->object and
/// for fast removal of an object given object*
class SimIdDictionary
{
#ifndef USE_NEW_SIMDICTIONARY
   enum
   {
      DefaultTableSize = 4096,
      TableBitMask = 4095
   };
   SimObject *table[DefaultTableSize];
#else
   SimObjectIdDictDef root;
#endif

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(S32 id);

   SimIdDictionary();
   ~SimIdDictionary();
};

#endif //_SIMDICTIONARY_H_
