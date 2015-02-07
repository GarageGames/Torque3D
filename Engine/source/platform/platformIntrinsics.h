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

#ifndef _PLATFORMINTRINSICS_H_
#define _PLATFORMINTRINSICS_H_

#ifndef _TORQUE_TYPES_H_
#  include "platform/types.h"
#endif

#if defined( TORQUE_COMPILER_VISUALC )
#  include "platform/platformIntrinsics.visualc.h"
#elif defined ( TORQUE_COMPILER_GCC )
#  include "platform/platformIntrinsics.gcc.h"
#else
#  error No intrinsics implemented for compiler.
#endif

//TODO: 64bit safe

template< typename T >
inline bool dCompareAndSwap( T* volatile& refPtr, T* oldPtr, T* newPtr )
{
   return dCompareAndSwap( *reinterpret_cast< volatile uintptr_t* >( &refPtr ), ( uintptr_t ) oldPtr, ( uintptr_t ) newPtr );
}

// Test-And-Set

inline bool dTestAndSet( volatile U32& ref )
{
   return dCompareAndSwap( ref, 0, 1 );
}
inline bool dTestAndSet( volatile U64& ref )
{
   return dCompareAndSwap( ref, 0, 1 );
}

#endif // _PLATFORMINTRINSICS_H_
