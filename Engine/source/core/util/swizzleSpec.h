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

#ifndef _SWIZZLESPEC_H_
#define _SWIZZLESPEC_H_

//------------------------------------------------------------------------------
// <U8, 4> (most common) Specialization
//------------------------------------------------------------------------------
#include "core/util/byteswap.h"

template<>
inline void Swizzle<U8, 4>::InPlace( void *memory, const dsize_t size ) const
{
   AssertFatal( size % 4 == 0, "Bad buffer size for swizzle, see docs." );

   U8 *dest = reinterpret_cast<U8 *>( memory );
   U8 *src = reinterpret_cast<U8 *>( memory );

   // Fast divide by 4 since we are assured a proper size
   for( S32 i = 0; i < size >> 2; i++ )
   {
      BYTESWAP( *dest++, src[mMap[0]] );
      BYTESWAP( *dest++, src[mMap[1]] );
      BYTESWAP( *dest++, src[mMap[2]] );
      BYTESWAP( *dest++, src[mMap[3]] );
      
      src += 4;
   }
}

template<>
inline void Swizzle<U8, 4>::ToBuffer( void *destination, const void *source, const dsize_t size ) const
{
   AssertFatal( size % 4 == 0, "Bad buffer size for swizzle, see docs." );

   U8 *dest = reinterpret_cast<U8 *>( destination );
   const U8 *src = reinterpret_cast<const U8 *>( source );

   // Fast divide by 4 since we are assured a proper size
   for( S32 i = 0; i < size >> 2; i++ )
   {
      *dest++ = src[mMap[0]];
      *dest++ = src[mMap[1]];
      *dest++ = src[mMap[2]];
      *dest++ = src[mMap[3]];
      
      src += 4;
   }
}

//------------------------------------------------------------------------------
// <U8, 3> Specialization
//------------------------------------------------------------------------------

template<>
inline void Swizzle<U8, 3>::InPlace( void *memory, const dsize_t size ) const
{
   AssertFatal( size % 3 == 0, "Bad buffer size for swizzle, see docs." );

   U8 *dest = reinterpret_cast<U8 *>( memory );
   U8 *src = reinterpret_cast<U8 *>( memory );

   for( S32 i = 0; i < size /3; i++ )
   {
      BYTESWAP( *dest++, src[mMap[0]] );
      BYTESWAP( *dest++, src[mMap[1]] );
      BYTESWAP( *dest++, src[mMap[2]] );
      
      src += 3;
   }
}

template<>
inline void Swizzle<U8, 3>::ToBuffer( void *destination, const void *source, const dsize_t size ) const
{
   AssertFatal( size % 3 == 0, "Bad buffer size for swizzle, see docs." );

   U8 *dest = reinterpret_cast<U8 *>( destination );
   const U8 *src = reinterpret_cast<const U8 *>( source );

   for( S32 i = 0; i < size / 3; i++ )
   {
      *dest++ = src[mMap[0]];
      *dest++ = src[mMap[1]];
      *dest++ = src[mMap[2]];
      
      src += 3;
   }
}


#endif