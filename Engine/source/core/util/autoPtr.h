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

#ifndef _AUTOPTR_H_
#define _AUTOPTR_H_

#ifndef _TYPETRAITS_H_
#  include "platform/typetraits.h"
#endif


template<class T>
struct AutoPtrRef
{
   T* _ptr;
   AutoPtrRef(T *ptr)
      : _ptr(ptr)
   {}
};

/// A simple smart pointer.
/// An extended version of std::auto_ptr which supports a deletion policy.
/// The delete policy indicates how the ptr is to be deleted. DeleteSingle,
/// the default, is used to delete individual objects. DeleteArray can
/// can be used to delete arrays.
/// <code>
///    AutoPtr<Object> ptr(new Object);
///    AutoPtr<Object,DeleteSingle> ptr(new Object);
///    AutoPtr<Object,DeleteArray> ptr(new Object[10]);
/// </code>
/// AutoPtrs do not perform reference counting and assume total ownership
/// of any object assigned to them.  Assigning an AutoPtr to another transfers
/// that ownership and resets the source AutoPtr to 0.
template<class T, class P = DeleteSingle>
class AutoPtr
{
public:
   typedef T ValueType;

   explicit AutoPtr(T *ptr = 0): _ptr(ptr) {}
   ~AutoPtr()
   {
       P::destroy(_ptr);
   }

   // Copy constructors
   AutoPtr(AutoPtr &rhs): _ptr(rhs.release()) {}

   template<class U>
   AutoPtr(AutoPtr<U,P> &rhs): _ptr(rhs.release()) { }

   /// Transfer ownership, any object currently be referenced is deleted and
   /// rhs is set to 0.
   AutoPtr& operator= (AutoPtr &rhs)
   {
       reset(rhs.release());
       return *this;
   }

   template<class U>
   AutoPtr& operator= (AutoPtr<U,P> &rhs)
   {
       reset(rhs.release());
       return *this;
   }

   // Access
   T* ptr() const { return _ptr; }
   T& operator*() const { return *_ptr; }
   T* operator->() const { return _ptr; }
   T& operator[](size_t index) { return (_ptr)[index]; }

   /// Release ownership of the object without deleting it.
   T* release()
   {
       T* tmp(_ptr);
       _ptr = 0;
       return tmp;
   }

   /// Equivalent to *this = (T*)ptr, except that operator=(T*) isn't provided for.
   void reset(T* ptr = 0)
   {
       if (_ptr != ptr)
       {
           P::destroy(_ptr);
           _ptr = ptr;
       }
   }

   // Conversion to/from ref type
   AutoPtr(AutoPtrRef<T> ref): _ptr(ref._ptr) {}
   AutoPtr& operator= (AutoPtrRef<T> ref)
   {
        reset(ref._ptr);
        return *this;
   }
   
   bool isNull() const { return _ptr == NULL; }
   bool isValid() const { return !isNull(); }

   template<class U>
   operator AutoPtrRef<U>() { return AutoPtrRef<U>(release()); }

   template<class U>
   operator AutoPtr<U,P>() { return AutoPtr<U,P>(release()); }

private:
   T  *_ptr;
};

#endif
