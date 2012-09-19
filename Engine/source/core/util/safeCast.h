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

#ifndef _TORQUE_CORE_UTIL_SAFECAST_H_
#define _TORQUE_CORE_UTIL_SAFECAST_H_

#include "platform/platform.h"

template< class T, typename I >
inline T* safeCast( I* inPtr )
{
   if( !inPtr )
      return 0;
   else
   {
      T* outPtr = dynamic_cast< T* >( inPtr );
      AssertFatal( outPtr != 0, "safeCast failed" );
      return outPtr;
   }
}

template<>
inline void* safeCast< void >( void* inPtr )
{
   return inPtr;
}

template< class T, typename I >
inline T* safeCastISV( I* inPtr )
{
   if( !inPtr )
      return 0;
   else
   {
      T* outPtr = dynamic_cast< T* >( inPtr );
      AssertISV( outPtr != 0, "safeCast failed" );
      return outPtr;
   }
}

#endif // _TORQUE_CORE_UTIL_SAFECAST_H_
