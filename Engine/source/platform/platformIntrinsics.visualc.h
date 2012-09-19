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

#ifndef _TORQUE_PLATFORM_PLATFORMINTRINSICS_VISUALC_H_
#define _TORQUE_PLATFORM_PLATFORMINTRINSICS_VISUALC_H_

/// @file
/// Compiler intrinsics for Visual C++.

#if defined(TORQUE_OS_XENON)
#  include <Xtl.h>
#  define _InterlockedExchangeAdd InterlockedExchangeAdd
#  define _InterlockedExchangeAdd64 InterlockedExchangeAdd64
#else
#	include <intrin.h>
#endif

// Fetch-And-Add
//
// NOTE: These do not return the pre-add value because
// not all platforms (damn you OSX) can do that.
//
inline void dFetchAndAdd( volatile U32& ref, U32 val )
{  
   _InterlockedExchangeAdd( ( volatile long* ) &ref, val );
}
inline void dFetchAndAdd( volatile S32& ref, S32 val )
{
   _InterlockedExchangeAdd( ( volatile long* ) &ref, val );
}

#if defined(TORQUE_OS_XENON)
// Not available on x86
inline void dFetchAndAdd( volatile U64& ref, U64 val )
{
   _InterlockedExchangeAdd64( ( volatile __int64* ) &ref, val );
}
#endif

// Compare-And-Swap

inline bool dCompareAndSwap( volatile U32& ref, U32 oldVal, U32 newVal )
{
   return ( _InterlockedCompareExchange( ( volatile long* ) &ref, newVal, oldVal ) == oldVal );
}
inline bool dCompareAndSwap( volatile U64& ref, U64 oldVal, U64 newVal )
{
   return ( _InterlockedCompareExchange64( ( volatile __int64* ) &ref, newVal, oldVal ) == oldVal );
}

/// Performs an atomic read operation.
inline U32 dAtomicRead( volatile U32 &ref )
{
   return _InterlockedExchangeAdd( ( volatile long* )&ref, 0 );
}

#endif // _TORQUE_PLATFORM_PLATFORMINTRINSICS_VISUALC_H_
