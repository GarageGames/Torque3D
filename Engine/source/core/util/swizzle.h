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

#ifndef _SWIZZLE_H_
#define _SWIZZLE_H_

#include "platform/platform.h"
#include "core/frameAllocator.h"

/// This class will swizzle 'sizeof( T )' length chunks of memory into different
/// patterns which are user described. The pattern is described by an instance
/// of Swizzle and this swizzle can then be executed on buffers. The following 
/// must be true of the buffer size:
///    size % ( sizeof( T ) * mapLength ) == 0
template<class T, dsize_t mapLength>
class Swizzle
{
private:
   /// This is an array from 0..n. Each entry in the array is a dsize_t with values
   /// in the range 0..n. Each value in the range 0..n can occur any number of times.
   /// 
   /// For example:
   /// This is our data set, an array of characters with 4 elements
   /// { 'a', 'b', 'c', 'd' } 
   ///
   /// If the map { 3, 2, 1, 0 } was applied to this set, the result would be:
   /// { 'd', 'c', 'b', 'a' }
   ///
   /// If the map { 3, 0, 2, 2 } was applied to the set, the result would be:
   /// { 'd', 'a', 'c', 'c' }
   dsize_t mMap[mapLength];

public:
   /// Construct a swizzle
   /// @see Swizzle::mMap
   Swizzle( const dsize_t *map );

   virtual ~Swizzle(){}

   /// This method will, in the general case, use the ToBuffer method to swizzle
   /// the memory specified into a temporary buffer, allocated by FrameTemp, and
   /// then copy the temporary memory into the source memory.
   ///
   /// @param  memory   Pointer to the start of the buffer to swizzle
   /// @param  size     Size of the buffer
   virtual void InPlace( void *memory, const dsize_t size ) const;

   /// This method copies the data from source to destination while applying the
   /// re-ordering. This method is, in the non-specalized case, O(N^2) where N
   /// is sizeof( T ) / size; the number of instances of type 'T' in the buffer
   ///
   /// @param  destination The destination of the swizzled data
   /// @param  source      The source data to be swizzled
   /// @param  size        Size of the source and destination buffers.
   virtual void ToBuffer( void *destination, const void *source, const dsize_t size ) const;
};

// Null swizzle
template<class T, dsize_t mapLength>
class NullSwizzle : public Swizzle<T, mapLength>
{
public:
   NullSwizzle( const dsize_t *map = NULL ) : Swizzle<T, mapLength>( map ) {};

   virtual void InPlace( void *memory, const dsize_t size ) const {}

   virtual void ToBuffer( void *destination, const void *source, const dsize_t size ) const
   {
      dMemcpy( destination, source, size );
   }
};

//------------------------------------------------------------------------------
// Common Swizzles 
namespace Swizzles
{
   extern Swizzle<U8, 4> bgra;
   extern Swizzle<U8, 4> argb;
   extern Swizzle<U8, 4> rgba;
   extern Swizzle<U8, 4> abgr;
   
   extern Swizzle<U8, 3> bgr;
   extern Swizzle<U8, 3> rgb;

   extern NullSwizzle<U8, 4> null;
};

//------------------------------------------------------------------------------

template<class T, dsize_t mapLength>
Swizzle<T, mapLength>::Swizzle( const dsize_t *map )
{
   if( map != NULL )
      dMemcpy( mMap, map, sizeof( dsize_t ) * mapLength );
}

//------------------------------------------------------------------------------

template<class T, dsize_t mapLength>
inline void Swizzle<T, mapLength>::ToBuffer( void *destination, const void *source, const dsize_t size ) const
{
   // TODO: OpenMP?
   AssertFatal( size % ( sizeof( T ) * mapLength ) == 0, "Bad buffer size for swizzle, see docs." );
   AssertFatal( destination != NULL, "Swizzle::ToBuffer - got a NULL destination pointer!" );
   AssertFatal( source != NULL, "Swizzle::ToBuffer - got a NULL source pointer!" );

   T *dest = reinterpret_cast<T *>( destination );
   const T *src = reinterpret_cast<const T *>( source );

   for( int i = 0; i < size / ( mapLength * sizeof( T ) ); i++ )
   {
      dMemcpy( dest, src, mapLength * sizeof( T ) );

      for( int j = 0; j < mapLength; j++ )
         *dest++ = src[mMap[j]];
      
      src += mapLength;
   }
}

//------------------------------------------------------------------------------

template<class T, dsize_t mapLength>
inline void Swizzle<T, mapLength>::InPlace( void *memory, const dsize_t size ) const
{
   // Just in case the inliner messes up the FrameTemp scoping (not sure if it would) -patw
   {
      // FrameTemp should work because the PNG loading code uses the FrameAllocator, so
      // it should only get used on an image w/ that size as max -patw
      FrameTemp<U8> buffer( size );
      dMemcpy( ~buffer, memory, size );
      ToBuffer( memory, ~buffer, size );
   }
}

//------------------------------------------------------------------------------

// Template specializations for certain swizzles
//#include "core/util/swizzleSpec.h"

#ifdef TORQUE_OS_XENON
#  include "platformXbox/altivecSwizzle.h"
#endif

#endif