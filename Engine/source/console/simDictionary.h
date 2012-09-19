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

class SimObject;

//----------------------------------------------------------------------------
/// Map of names to SimObjects
///
/// Provides fast lookup for name->object and
/// for fast removal of an object given object*
class SimNameDictionary
{
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;

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
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;

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
   enum
   {
      DefaultTableSize = 4096,
      TableBitMask = 4095
   };
   SimObject *table[DefaultTableSize];

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(S32 id);

   SimIdDictionary();
   ~SimIdDictionary();
};

#endif //_SIMDICTIONARY_H_
