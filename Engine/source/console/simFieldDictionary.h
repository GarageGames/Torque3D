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

#ifndef _SIMFIELDDICTIONARY_H_
#define _SIMFIELDDICTIONARY_H_

// Forward Refs
class ConsoleBaseType;
class SimObject;

#include "core/stringTable.h"
#include "core/stream/stream.h"

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

/// Dictionary to keep track of dynamic fields on SimObject.
class SimFieldDictionary
{
   friend class SimFieldDictionaryIterator;

public:
   struct Entry
   {
      Entry() : type( NULL ) {};

      StringTableEntry slotName;
      char *value;
      Entry *next;
      ConsoleBaseType *type;
   };
   enum
   {
      HashTableSize = 19
   };
   Entry *mHashTable[HashTableSize];

private:
   static Entry   *smFreeList;

   void           freeEntry(Entry *entry);
   Entry*         addEntry( U32 bucket, StringTableEntry slotName, ConsoleBaseType* type, char* value = 0 );

   static U32     getHashValue( StringTableEntry slotName );
   static U32     getHashValue( const String& fieldName );

   U32   mNumFields;

   /// In order to efficiently detect when a dynamic field has been
   /// added or deleted, we increment this every time we add or
   /// remove a field.
   U32 mVersion;

public:
   const U32 getVersion() const { return mVersion; }

   SimFieldDictionary();
   ~SimFieldDictionary();
   void setFieldType(StringTableEntry slotName, const char *typeString);
   void setFieldType(StringTableEntry slotName, const U32 typeId);
   void setFieldType(StringTableEntry slotName, ConsoleBaseType *type);
   void setFieldValue(StringTableEntry slotName, const char *value);
   const char *getFieldValue(StringTableEntry slotName);
   U32 getFieldType(StringTableEntry slotName) const;
   Entry  *findDynamicField(const String &fieldName) const;
   Entry  *findDynamicField( StringTableEntry fieldName) const;
   void writeFields(SimObject *obj, Stream &strem, U32 tabStop);
   void printFields(SimObject *obj);
   void assignFrom(SimFieldDictionary *dict);
   U32   getNumFields() const { return mNumFields; }

   Entry  *operator[](U32 index);
};

class SimFieldDictionaryIterator
{
   SimFieldDictionary *          mDictionary;
   S32                           mHashIndex;
   SimFieldDictionary::Entry *   mEntry;

public:
   SimFieldDictionaryIterator(SimFieldDictionary*);
   SimFieldDictionary::Entry* operator++();
   SimFieldDictionary::Entry* operator*();
};


#endif // _SIMFIELDDICTIONARY_H_
