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

#ifndef _TSPARSEARRAY_H_
#define _TSPARSEARRAY_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif

template <class T>
class SparseArray
{
  protected:
   struct Node {
      T*    pObject;
      U32   key;

      Node* next;
   };

  protected:
   U32   mModulus;
   Node* mSentryTables;

public:
   SparseArray(const U32 modulusSize = 64);
   ~SparseArray();

   void insert(T* pObject, U32 key);
   T*   remove(U32 key);
   T*   retreive(U32 key);

   void clearTables();           // Note: _deletes_ the objects!
};

template <class T>
inline SparseArray<T>::SparseArray(const U32 modulusSize)
{
   AssertFatal(modulusSize > 0, "Error, modulus must be > 0");

   mModulus      = modulusSize;
   mSentryTables = new Node[mModulus];
   for (U32 i = 0; i < mModulus; i++)
      mSentryTables[i].next = NULL;
}

template <class T>
inline SparseArray<T>::~SparseArray()
{
   clearTables();
}

template <class T>
inline void SparseArray<T>::clearTables()
{
   for (U32 i = 0; i < mModulus; i++) {
      Node* pProbe = mSentryTables[i].next;
      while (pProbe != NULL) {
         Node* pNext = pProbe->next;
         delete pProbe->pObject;
         delete pProbe;
         pProbe = pNext;
      }
   }
   delete [] mSentryTables;
   mSentryTables = NULL;
   mModulus = 0;
}

template <class T>
inline void SparseArray<T>::insert(T* pObject, U32 key)
{
   U32 insert = key % mModulus;
   Node* pNew = new Node;
   pNew->pObject = pObject;
   pNew->key     = key;
   pNew->next    = mSentryTables[insert].next;
   mSentryTables[insert].next = pNew;

#ifdef TORQUE_DEBUG
   Node* probe = pNew->next;
   while (probe != NULL) {
      AssertFatal(probe->key != key, "error, duplicate keys in sparse array!");
      probe = probe->next;
   }
#endif
}

template <class T>
inline T* SparseArray<T>::remove(U32 key)
{
   U32 remove  = key % mModulus;
   Node* probe = &mSentryTables[remove];
   while (probe->next != NULL) {
      if (probe->next->key == key) {
         Node* remove = probe->next;
         T* pReturn   = remove->pObject;
         probe->next  = remove->next;
         delete remove;
         return pReturn;
      }
      probe = probe->next;
   }

   // [tom, 8/19/2006] This assert is also utterly, utterly useless
   // AssertFatal(false, "Key didn't exist in the array!");
   return NULL;
}

template <class T>
inline T* SparseArray<T>::retreive(U32 key)
{
   U32 retrieve = key % mModulus;
   Node* probe  = &mSentryTables[retrieve];
   while (probe->next != NULL) {
      if (probe->next->key == key) {
         return probe->next->pObject;
      }
      probe = probe->next;
   }

   // [tom, 11/16/2005] This assert is utterly, utterly useless
   // AssertFatal(false, "Key didn't exist in the array!");
   return NULL;
}

#endif //_TSPARSEARRAY_H_

