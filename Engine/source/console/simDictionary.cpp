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

#include "console/simDictionary.h"
#include "console/simBase.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
extern S32 HashPointer(StringTableEntry e);

SimNameDictionary::SimNameDictionary()
{
   hashTable = NULL;
   mutex = Mutex::createMutex();
}

SimNameDictionary::~SimNameDictionary()
{
   delete[] hashTable;
   Mutex::destroyMutex(mutex);
}

void SimNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

   SimObject* checkForDup = find(obj->objectName);

   if (checkForDup)
      Con::warnf("Warning! You have a duplicate datablock name of %s. This can cause problems. You should rename one of them.", obj->objectName);

   Mutex::lockMutex(mutex);
   
   if(!hashTable)
   {
      hashTable = new SimObject *[DefaultTableSize];
      hashTableSize = DefaultTableSize;
      hashEntryCount = 0;
      
      dMemset( hashTable, 0, sizeof( *hashTable ) * DefaultTableSize );
   }
   
   S32 idx = HashPointer(obj->objectName) % hashTableSize;
   obj->nextNameObject = hashTable[idx];
   hashTable[idx] = obj;
   hashEntryCount++;
   
   // Rehash if necessary.

   if( hashEntryCount > hashTableSize )
   {
      // Allocate new table.
      
      U32 newHashTableSize = hashTableSize * 2 + 1;
      SimObject** newHashTable = new SimObject *[ newHashTableSize ];
      dMemset( newHashTable, 0, sizeof( newHashTable[ 0 ] ) * newHashTableSize );
      
      // Move entries over.

      for( U32 i = 0; i < hashTableSize; ++ i )
         for( SimObject* object = hashTable[ i ]; object != NULL; )
         {
            SimObject* next = object->nextNameObject;

            idx = HashPointer( object->objectName ) % newHashTableSize;
            object->nextNameObject = newHashTable[ idx ];
            newHashTable[ idx ] = object;
            
            object = next;
         }
         
      // Switch tables.
      
      delete [] hashTable;
      hashTable = newHashTable;
      hashTableSize = newHashTableSize;
   }
   
   Mutex::unlockMutex(mutex);
}

SimObject* SimNameDictionary::find(StringTableEntry name)
{
   // NULL is a valid lookup - it will always return NULL
   if(!hashTable)
      return NULL;
      
   Mutex::lockMutex(mutex);

   S32 idx = HashPointer(name) % hashTableSize;
   SimObject *walk = hashTable[idx];
   while(walk)
   {
      if(walk->objectName == name)
      {
         Mutex::unlockMutex(mutex);
         return walk;
      }
      walk = walk->nextNameObject;
   }

   Mutex::unlockMutex(mutex);
   return NULL;
}

void SimNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);

   SimObject **walk = &hashTable[HashPointer(obj->objectName) % hashTableSize];
   while(*walk)
   {
      if(*walk == obj)
      {
         *walk = obj->nextNameObject;
			obj->nextNameObject = (SimObject*)-1;
         hashEntryCount--;

         Mutex::unlockMutex(mutex);
         return;
      }
      walk = &((*walk)->nextNameObject);
   }

   Mutex::unlockMutex(mutex);
}	

//----------------------------------------------------------------------------

SimManagerNameDictionary::SimManagerNameDictionary()
{
   hashTable = new SimObject *[DefaultTableSize];
   hashTableSize = DefaultTableSize;
   hashEntryCount = 0;
   
   dMemset( hashTable, 0, sizeof( hashTable[ 0 ] ) * hashTableSize );

   mutex = Mutex::createMutex();
}

SimManagerNameDictionary::~SimManagerNameDictionary()
{
   delete[] hashTable;
   Mutex::destroyMutex(mutex);
}

void SimManagerNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);

   S32 idx = HashPointer(obj->objectName) % hashTableSize;
   obj->nextManagerNameObject = hashTable[idx];
   hashTable[idx] = obj;
   hashEntryCount++;
   
   // Rehash if necessary.

   if( hashEntryCount > hashTableSize )
   {
      // Allocate new table.
      
      U32 newHashTableSize = hashTableSize * 2 + 1;
      SimObject** newHashTable = new SimObject *[ newHashTableSize ];
      dMemset( newHashTable, 0, sizeof( newHashTable[ 0 ] ) * newHashTableSize );
      
      // Move entries over.

      for( U32 i = 0; i < hashTableSize; ++ i )
         for( SimObject* object = hashTable[ i ]; object != NULL; )
         {
            SimObject* next = object->nextManagerNameObject;

            idx = HashPointer( object->objectName ) % newHashTableSize;
            object->nextManagerNameObject = newHashTable[ idx ];
            newHashTable[ idx ] = object;
            
            object = next;
         }
         
      // Switch tables.
      
      delete [] hashTable;
      hashTable = newHashTable;
      hashTableSize = newHashTableSize;
   }
   
   Mutex::unlockMutex(mutex);
}

SimObject* SimManagerNameDictionary::find(StringTableEntry name)
{
   // NULL is a valid lookup - it will always return NULL

   Mutex::lockMutex(mutex);

   S32 idx = HashPointer(name) % hashTableSize;
   SimObject *walk = hashTable[idx];
   while(walk)
   {
      if(walk->objectName == name)
      {
         Mutex::unlockMutex(mutex);
         return walk;
      }
      walk = walk->nextManagerNameObject;
   }

   Mutex::unlockMutex(mutex);
   return NULL;
}

void SimManagerNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);

   SimObject **walk = &hashTable[HashPointer(obj->objectName) % hashTableSize];
   while(*walk)
   {
      if(*walk == obj)
      {
         *walk = obj->nextManagerNameObject;
			obj->nextManagerNameObject = (SimObject*)-1;
         hashEntryCount--;

         Mutex::unlockMutex(mutex);
         return;
      }
      walk = &((*walk)->nextManagerNameObject);
   }

   Mutex::unlockMutex(mutex);
}	

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SimIdDictionary::SimIdDictionary()
{
   dMemset( table, 0, sizeof( table[ 0 ] ) * DefaultTableSize );
   mutex = Mutex::createMutex();
}

SimIdDictionary::~SimIdDictionary()
{
   Mutex::destroyMutex(mutex);
}

void SimIdDictionary::insert(SimObject* obj)
{
   Mutex::lockMutex(mutex);

   S32 idx = obj->getId() & TableBitMask;
   obj->nextIdObject = table[idx];
   AssertFatal( obj->nextIdObject != obj, "SimIdDictionary::insert - Creating Infinite Loop linking to self!" );
   table[idx] = obj;

   Mutex::unlockMutex(mutex);
}

SimObject* SimIdDictionary::find(S32 id)
{
   Mutex::lockMutex(mutex);

   S32 idx = id & TableBitMask;
   SimObject *walk = table[idx];
   while(walk)
   {
      if(walk->getId() == U32(id))
      {
         Mutex::unlockMutex(mutex);
         return walk;
      }
      walk = walk->nextIdObject;
   }

   Mutex::unlockMutex(mutex);

   return NULL;
}

void SimIdDictionary::remove(SimObject* obj)
{
   Mutex::lockMutex(mutex);

   SimObject **walk = &table[obj->getId() & TableBitMask];
   while(*walk && *walk != obj)
      walk = &((*walk)->nextIdObject);
   if(*walk)
      *walk = obj->nextIdObject;

   Mutex::unlockMutex(mutex);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

