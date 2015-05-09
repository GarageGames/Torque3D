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

#ifndef _TORQUE_PLATFORM_PLATFORMINTRINSICS_GCC_H_
#define _TORQUE_PLATFORM_PLATFORMINTRINSICS_GCC_H_

/// @file
/// Compiler intrinsics for GCC.

#ifdef TORQUE_OS_MAC
#include <libkern/OSAtomic.h>
#elif defined(TORQUE_OS_PS3)
#include <cell/atomic.h>
#endif

// Fetch-And-Add
//
// NOTE: These do not return the pre-add value because
// not all platforms (damn you OSX) can do that.
//
inline void dFetchAndAdd( volatile U32& ref, U32 val )
{
   #if defined(TORQUE_OS_PS3)
      cellAtomicAdd32( (std::uint32_t *)&ref, val );
   #elif !defined(TORQUE_OS_MAC)
      __sync_fetch_and_add(&ref, val );
   #else
      OSAtomicAdd32( val, (int32_t* ) &ref);
   #endif
}

inline void dFetchAndAdd( volatile S32& ref, S32 val )
{
   #if defined(TORQUE_OS_PS3)
      cellAtomicAdd32( (std::uint32_t *)&ref, val );
   #elif !defined(TORQUE_OS_MAC)
      __sync_fetch_and_add( &ref, val );
   #else
      OSAtomicAdd32( val, (int32_t* ) &ref);
   #endif
}

// Compare-And-Swap

inline bool dCompareAndSwap( volatile U32& ref, U32 oldVal, U32 newVal )
{
   // bool
   //OSAtomicCompareAndSwap32(int32_t oldValue, int32_t newValue, volatile int32_t *theValue);
   #if defined(TORQUE_OS_PS3)
      return ( cellAtomicCompareAndSwap32( (std::uint32_t *)&ref, newVal, oldVal ) == oldVal );
   #elif !defined(TORQUE_OS_MAC)
      return ( __sync_val_compare_and_swap( &ref, oldVal, newVal ) == oldVal );
   #else
      return OSAtomicCompareAndSwap32(oldVal, newVal, (int32_t *) &ref);
   #endif
}

inline bool dCompareAndSwap( volatile U64& ref, U64 oldVal, U64 newVal )
{
   #if defined(TORQUE_OS_PS3)
      return ( cellAtomicCompareAndSwap32( (std::uint32_t *)&ref, newVal, oldVal ) == oldVal );
   #elif !defined(TORQUE_OS_MAC)
      return ( __sync_val_compare_and_swap( &ref, oldVal, newVal ) == oldVal );
   #else
      return OSAtomicCompareAndSwap64(oldVal, newVal, (int64_t *) &ref);
   #endif

}

/// Performs an atomic read operation.
inline U32 dAtomicRead( volatile U32 &ref )
{
   #if defined(TORQUE_OS_PS3)
      return cellAtomicAdd32( (std::uint32_t *)&ref, 0 );
   #elif !defined(TORQUE_OS_MAC)
      return __sync_fetch_and_add( ( volatile long* ) &ref, 0 );
   #else
      return OSAtomicAdd32( 0, (int32_t* ) &ref);
   #endif
}

#endif // _TORQUE_PLATFORM_PLATFORMINTRINSICS_GCC_H_
