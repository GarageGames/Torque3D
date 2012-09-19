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

#ifndef _TVECTORSPEC_H_
#define _TVECTORSPEC_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

// Not exactly a specialization, just a vector to use when speed is a concern
template<class T>
class FastVector : public Vector<T>
{
public:
   // This method was re-written to prevent load-hit-stores during the simple-case.
   void push_back_noresize(const T &x)
   {
#ifdef TORQUE_DEBUG
      AssertFatal(Vector<T>::mElementCount < Vector<T>::mArraySize, "use of push_back_noresize requires that you reserve enough space in the FastVector");
#endif
      Vector<T>::mArray[Vector<T>::mElementCount++] = x;
   }
};

#endif //_TVECTORSPEC_H_

