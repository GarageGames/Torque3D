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

#ifndef USE_CLASSIC_SIMDICTIONARY
#include <string>
#include <unordered_map>
#endif

class SimObject;


#ifndef USE_CLASSIC_SIMDICTIONARY

#include "core/strings/stringFunctions.h"

struct DictionaryHash 
{
    inline size_t operator()(const char* val) const
	{
	    return (long)val;
	}
};
	 
struct eqstr 
{
    inline bool operator()(const char *s1, const char *s2) const 
	{
	    return dStrcmp(s1, s2) == 0;
	}
};
#endif

#ifndef USE_CLASSIC_SIMDICTIONARY
typedef std::unordered_map<const char * , SimObject*, DictionaryHash, eqstr>  StringDictDef;	
typedef std::unordered_map<U32 ,SimObject*> U32DictDef;	
#endif



//----------------------------------------------------------------------------
/// Map of names to SimObjects
///
/// Provides fast lookup for name->object and
/// for fast removal of an object given object*
class SimNameDictionary
{
#ifdef USE_CLASSIC_SIMDICTIONARY
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
#ifdef USE_CLASSIC_SIMDICTIONARY
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
#ifdef USE_CLASSIC_SIMDICTIONARY
   enum
   {
      DefaultTableSize = 4096,
      TableBitMask = 4095
   };
   SimObject *table[DefaultTableSize];
#else
   U32DictDef root;
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
