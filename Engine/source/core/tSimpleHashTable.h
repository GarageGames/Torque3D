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

// [tom, 9/19/2006] Simple hash table. Not intended to replace map<>, but it is
// generally good enough for simple things that you don't need to iterate.
//
// Note: If you move this to another project, you need the updated tSparseArray.h
// as well as hashFunction.cc/h

#include "platform/platform.h"

#include "core/tSparseArray.h"
#include "core/util/hashFunction.h"
#include "core/strings/stringFunctions.h"

#ifndef _TSIMPLEHASHTABLE_H
#define _TSIMPLEHASHTABLE_H

template <class T> class SimpleHashTable : public SparseArray<T>
{
   typedef SparseArray<T> Parent;

   bool mCaseSensitive;

   char mCaseConvBuf[1024];
   
   // [tom, 9/21/2006] This is incredibly lame and adds a pretty big speed penalty
   inline const char *caseConv(const char *str)
   {
      if(mCaseSensitive)   return str;

      S32 len = dStrlen(str);
      if(len >= sizeof(mCaseConvBuf))  len = sizeof(mCaseConvBuf) - 1;

      char *dptr = mCaseConvBuf;
      const char *sptr = str;
      while(*sptr)
      {
         *dptr = dTolower(*sptr);
         ++sptr;
         ++dptr;
      }
      *dptr = 0;

      return mCaseConvBuf;
   }

public:
   SimpleHashTable(const U32 modulusSize = 64, bool caseSensitive = true) : Parent(modulusSize), mCaseSensitive(caseSensitive)
   {
   }

   void insert(T* pObject, U8 *key, U32 keyLen);
   T*   remove(U8 *key, U32 keyLen);
   T*   retreive(U8 *key, U32 keyLen);

   void insert(T* pObject, const char *key);
   T*   remove(const char *key);
   T*   retreive(const char *key);
};

template <class T> inline void SimpleHashTable<T>::insert(T* pObject, U8 *key, U32 keyLen)
{
   Parent::insert(pObject, Torque::hash(key, keyLen, 0));
}

template <class T> inline T* SimpleHashTable<T>::remove(U8 *key, U32 keyLen)
{
   return Parent::remove(Torque::hash(key, keyLen, 0));
}

template <class T> inline T* SimpleHashTable<T>::retreive(U8 *key, U32 keyLen)
{
   return Parent::retreive(Torque::hash(key, keyLen, 0));
}

template <class T> inline void SimpleHashTable<T>::insert(T* pObject, const char *key)
{
   key = caseConv(key);
   insert(pObject, (U8 *)key, dStrlen(key));
}

template <class T> T* SimpleHashTable<T>::remove(const char *key)
{
   key = caseConv(key);
   return remove((U8 *)key, dStrlen(key));
}

template <class T> T* SimpleHashTable<T>::retreive(const char *key)
{
   key = caseConv(key);
   return retreive((U8 *)key, dStrlen(key));
}


#endif // _TSIMPLEHASHTABLE_H
