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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "platform/platform.h"
#include "console/simFieldDictionary.h"

#include "console/console.h"
#include "console/consoleInternal.h"
#include "core/frameAllocator.h"

SimFieldDictionary::Entry *SimFieldDictionary::smFreeList = NULL;

static Chunker<SimFieldDictionary::Entry> fieldChunker;

U32 SimFieldDictionary::getHashValue(StringTableEntry slotName)
{
   return HashPointer(slotName) % HashTableSize;
}

U32 SimFieldDictionary::getHashValue(const String& fieldName)
{
   return getHashValue(StringTable->insert(fieldName));
}

SimFieldDictionary::Entry *SimFieldDictionary::addEntry(U32 bucket, StringTableEntry slotName, ConsoleBaseType* type, char* value)
{
   Entry* ret;
   if (smFreeList)
   {
      ret = smFreeList;
      smFreeList = ret->next;
   }
   else
      ret = fieldChunker.alloc();

   ret->next = mHashTable[bucket];
   ret->slotName = slotName;
   ret->type = type;
   ret->value = value;

   mHashTable[bucket] = ret;
   mNumFields++;
   mVersion++;

   return ret;
}

void SimFieldDictionary::freeEntry(SimFieldDictionary::Entry *ent)
{
   ent->next = smFreeList;
   smFreeList = ent;

   mNumFields--;
}

SimFieldDictionary::SimFieldDictionary()
   : mNumFields(0),
   mVersion(0)
{
   dMemset(mHashTable, 0, sizeof(mHashTable));
}

SimFieldDictionary::~SimFieldDictionary()
{
   for (U32 i = 0; i < HashTableSize; i++)
   {
      for (Entry *walk = mHashTable[i]; walk;)
      {
         Entry *temp = walk;
         walk = temp->next;

         if (temp->value)
            dFree(temp->value);
         freeEntry(temp);
      }
   }

   AssertFatal(mNumFields == 0, "Incorrect count on field dictionary");
}

void SimFieldDictionary::setFieldType(StringTableEntry slotName, const char *typeString)
{
   ConsoleBaseType *cbt = ConsoleBaseType::getTypeByName(typeString);
   setFieldType(slotName, cbt);
}

void SimFieldDictionary::setFieldType(StringTableEntry slotName, const U32 typeId)
{
   ConsoleBaseType *cbt = ConsoleBaseType::getType(typeId);
   setFieldType(slotName, cbt);
}

void SimFieldDictionary::setFieldType(StringTableEntry slotName, ConsoleBaseType *type)
{
   // If the field exists on the object, set the type
   U32 bucket = getHashValue(slotName);

   for (Entry *walk = mHashTable[bucket]; walk; walk = walk->next)
   {
      if (walk->slotName == slotName)
      {
         // Found and type assigned, let's bail
         walk->type = type;
         return;
      }
   }

   // Otherwise create the field, and set the type. Assign a null value.
   addEntry(bucket, slotName, type);
}

U32 SimFieldDictionary::getFieldType(StringTableEntry slotName) const
{
   U32 bucket = getHashValue(slotName);

   for (Entry *walk = mHashTable[bucket]; walk; walk = walk->next)
      if (walk->slotName == slotName)
         return walk->type ? walk->type->getTypeID() : TypeString;

   return TypeString;
}

SimFieldDictionary::Entry  *SimFieldDictionary::findDynamicField(const String &fieldName) const
{
   U32 bucket = getHashValue(fieldName);

   for (Entry *walk = mHashTable[bucket]; walk; walk = walk->next)
   {
      if (fieldName.equal(walk->slotName, String::NoCase))
         return walk;
   }

   return NULL;
}

SimFieldDictionary::Entry *SimFieldDictionary::findDynamicField(StringTableEntry fieldName) const
{
   U32 bucket = getHashValue(fieldName);

   for (Entry *walk = mHashTable[bucket]; walk; walk = walk->next)
   {
      if (walk->slotName == fieldName)
      {
         return walk;
      }
   }

   return NULL;
}


void SimFieldDictionary::setFieldValue(StringTableEntry slotName, const char *value)
{
   U32 bucket = getHashValue(slotName);
   Entry **walk = &mHashTable[bucket];
   while (*walk && (*walk)->slotName != slotName)
      walk = &((*walk)->next);

   Entry *field = *walk;
   if (!value || !*value)
   {
      if (field)
      {
         mVersion++;

         if (field->value)
            dFree(field->value);

         *walk = field->next;
         freeEntry(field);
      }
   }
   else
   {
      if (field)
      {
         if (field->value)
            dFree(field->value);

         field->value = dStrdup(value);
      }
      else
         addEntry(bucket, slotName, 0, dStrdup(value));
   }
}

const char *SimFieldDictionary::getFieldValue(StringTableEntry slotName)
{
   U32 bucket = getHashValue(slotName);

   for (Entry *walk = mHashTable[bucket]; walk; walk = walk->next)
      if (walk->slotName == slotName)
         return walk->value;

   return NULL;
}

void SimFieldDictionary::assignFrom(SimFieldDictionary *dict)
{
   mVersion++;

   for (U32 i = 0; i < HashTableSize; i++)
   {
      for (Entry *walk = dict->mHashTable[i]; walk; walk = walk->next)
      {
         setFieldValue(walk->slotName, walk->value);
         setFieldType(walk->slotName, walk->type);
      }
   }
}

static S32 QSORT_CALLBACK compareEntries(const void* a, const void* b)
{
   SimFieldDictionary::Entry *fa = *((SimFieldDictionary::Entry **)a);
   SimFieldDictionary::Entry *fb = *((SimFieldDictionary::Entry **)b);
   return dStricmp(fa->slotName, fb->slotName);
}

void SimFieldDictionary::writeFields(SimObject *obj, Stream &stream, U32 tabStop)
{
   const AbstractClassRep::FieldList &list = obj->getFieldList();
   Vector<Entry *> flist(__FILE__, __LINE__);

   for (U32 curEntry = 0; curEntry < HashTableSize; curEntry++)
   {
      for (Entry *walk = mHashTable[curEntry]; walk; walk = walk->next)
      {
         // make sure we haven't written this out yet:
         U32 curField;
         for (curField = 0; curField < list.size(); curField++)
            if (list[curField].pFieldname == walk->slotName)
               break;

         if (curField != list.size())
            continue;


         if (!obj->writeField(walk->slotName, walk->value))
            continue;

         flist.push_back(walk);
      }
   }

   // Sort Entries to prevent version control conflicts
   dQsort(flist.address(), flist.size(), sizeof(Entry *), compareEntries);

   // Save them out
   for (Vector<Entry *>::iterator itr = flist.begin(); itr != flist.end(); itr++)
   {
      U32 nBufferSize = (dStrlen((*itr)->value) * 2) + dStrlen((*itr)->slotName) + 16;
      FrameTemp<char> expandedBuffer(nBufferSize);

      stream.writeTabs(tabStop + 1);

      const char *typeName = (*itr)->type && (*itr)->type->getTypeID() != TypeString ? (*itr)->type->getTypeName() : "";
      dSprintf(expandedBuffer, nBufferSize, "%s%s%s = \"", typeName, *typeName ? " " : "", (*itr)->slotName);
      if ((*itr)->value)
         expandEscape((char*)expandedBuffer + dStrlen(expandedBuffer), (*itr)->value);
      dStrcat(expandedBuffer, "\";\r\n", nBufferSize);

      stream.write(dStrlen(expandedBuffer), expandedBuffer);
   }

}
void SimFieldDictionary::printFields(SimObject *obj)
{
   const AbstractClassRep::FieldList &list = obj->getFieldList();
   char expandedBuffer[4096];
   Vector<Entry *> flist(__FILE__, __LINE__);

   for (U32 curEntry = 0; curEntry < HashTableSize; curEntry++)
   {
      for (Entry *walk = mHashTable[curEntry]; walk; walk = walk->next)
      {
         // make sure we haven't written this out yet:
         U32 curField;
         for (curField = 0; curField < list.size(); curField++)
            if (list[curField].pFieldname == walk->slotName)
               break;

         if (curField != list.size())
            continue;

         flist.push_back(walk);
      }
   }
   dQsort(flist.address(), flist.size(), sizeof(Entry *), compareEntries);

   for (Vector<Entry *>::iterator itr = flist.begin(); itr != flist.end(); itr++)
   {
      const char* type = "string";
      if ((*itr)->type)
         type = (*itr)->type->getTypeClassName();

      dSprintf(expandedBuffer, sizeof(expandedBuffer), "  %s %s = \"", type, (*itr)->slotName);
      if ((*itr)->value)
         expandEscape(expandedBuffer + dStrlen(expandedBuffer), (*itr)->value);
      Con::printf("%s\"", expandedBuffer);
   }
}

SimFieldDictionary::Entry  *SimFieldDictionary::operator[](U32 index)
{
   AssertFatal(index < mNumFields, "out of range");

   if (index > mNumFields)
      return NULL;

   SimFieldDictionaryIterator itr(this);

   for (S32 i = 0; i < index && *itr; i++)
      ++itr;

   return (*itr);
}

//------------------------------------------------------------------------------
SimFieldDictionaryIterator::SimFieldDictionaryIterator(SimFieldDictionary * dictionary)
{
   mDictionary = dictionary;
   mHashIndex = -1;
   mEntry = 0;
   operator++();
}

SimFieldDictionary::Entry* SimFieldDictionaryIterator::operator++()
{
   if (!mDictionary)
      return(mEntry);

   if (mEntry)
      mEntry = mEntry->next;

   while (!mEntry && (mHashIndex < (SimFieldDictionary::HashTableSize - 1)))
      mEntry = mDictionary->mHashTable[++mHashIndex];

   return(mEntry);
}

SimFieldDictionary::Entry* SimFieldDictionaryIterator::operator*()
{
   return(mEntry);
}
// A variation of the stock SimFieldDictionary::setFieldValue(), this method adds the
// <no_replace> argument which, when true, prohibits the replacement of fields that
// already have a value. 
//
// AFX uses this when an effects-choreographer (afxMagicSpell, afxEffectron) is created 
// using the new operator. It prevents any in-line effect parameters from being overwritten
// by default parameters that are copied over later.
void SimFieldDictionary::setFieldValue(StringTableEntry slotName, const char *value, ConsoleBaseType *type, bool no_replace)
{
   if (!no_replace)
   {
      setFieldValue(slotName, value);
      return;
   }

   if (!value || !*value)
      return;

   U32 bucket = getHashValue(slotName);
   Entry **walk = &mHashTable[bucket];
   while (*walk && (*walk)->slotName != slotName)
      walk = &((*walk)->next);

   Entry *field = *walk;
   if (field)
      return;

   addEntry(bucket, slotName, type, dStrdup(value));
}
// A variation of the stock SimFieldDictionary::assignFrom(), this method adds <no_replace>
// and <filter> arguments. When true, <no_replace> prohibits the replacement of fields that already
// have a value. When <filter> is specified, only fields with leading characters that exactly match
// the characters in <filter> are copied.
void SimFieldDictionary::assignFrom(SimFieldDictionary *dict, const char* filter, bool no_replace)
{
   dsize_t filter_len = (filter) ? dStrlen(filter) : 0;
   if (filter_len == 0 && !no_replace)
   {
      assignFrom(dict);
      return;
   }

   mVersion++;

   if (filter_len == 0)
   {
      for (U32 i = 0; i < HashTableSize; i++)
         for (Entry *walk = dict->mHashTable[i]; walk; walk = walk->next)
            setFieldValue(walk->slotName, walk->value, walk->type, no_replace);
   }
   else
   {
      for (U32 i = 0; i < HashTableSize; i++)
         for (Entry *walk = dict->mHashTable[i]; walk; walk = walk->next)
            if (dStrncmp(walk->slotName, filter, filter_len) == 0)
               setFieldValue(walk->slotName, walk->value, walk->type, no_replace);
   }
}