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

#ifndef _UTIL_DELEGATE_H_
#define _UTIL_DELEGATE_H_

#include "core/util/FastDelegate.h"

/// @def Delegate
/// The macro which abstracts the details of the delegate implementation.
#define Delegate fastdelegate::FastDelegate

/// @typedef DelegateMemento
/// An opaque structure which can hold an arbitary delegate.
/// @see Delegate
typedef fastdelegate::DelegateMemento DelegateMemento;


template<class T>
class DelegateRemapper : public DelegateMemento
{
public:
   DelegateRemapper() : mOffset(0) {}

   void set(T * t, const DelegateMemento & memento)
   {
      SetMementoFrom(memento);
      if (m_pthis)
         mOffset = ((int)m_pthis) - ((int)t);
   }

   void rethis(T * t)
   {
      if (m_pthis)
         m_pthis = (fastdelegate::detail::GenericClass *)(mOffset + (int)t);
   }

protected:
   S32 mOffset;
};

#endif // _UTIL_DELEGATE_H_