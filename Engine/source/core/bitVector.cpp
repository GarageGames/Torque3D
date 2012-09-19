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

#include "platform/platform.h"
#include "core/bitVector.h"


void BitVector::_resize( U32 sizeInBits, bool copyBits )
{
   if ( sizeInBits != 0 ) 
   {
      U32 newSize = calcByteSize( sizeInBits );
      if ( mByteSize < newSize ) 
      {
         U8 *newBits = new U8[newSize];
         if( copyBits )
            dMemcpy( newBits, mBits, mByteSize );

         delete [] mBits;
         mBits = newBits;
         mByteSize = newSize;
      }
   } 
   else 
   {
      delete [] mBits;
      mBits     = NULL;
      mByteSize = 0;
   }

   mSize = sizeInBits;
}

void BitVector::combineOR( const BitVector &other )
{
   AssertFatal( mSize == other.mSize, "BitVector::combineOR - Vectors differ in size!" );

   for ( U32 i=0; i < mSize; i++ )
   {
      bool b = test(i) | other.test(i);
      set( i, b );
   }
}

bool BitVector::_test( const BitVector& vector, bool all ) const
{
   AssertFatal( mByteSize == vector.mByteSize, "BitVector::_test - Vectors differ in size!" );
   AssertFatal( mByteSize % 4 == 0, "BitVector::_test - Vector not DWORD aligned!" );

   const U32 numDWORDS = mByteSize / 4;
   const U32* bits1 = reinterpret_cast< const U32* >( mBits );
   const U32* bits2 = reinterpret_cast< const U32* >( vector.mBits );

   for( U32 i = 0; i < numDWORDS; ++ i )
   {
      if( !( bits1[ i ] & bits2[ i ] ) )
         continue;
      else if( bits2[ i ] && all )
         return false;
      else
         return true;
   }

   return false;
}

bool BitVector::testAll() const
{
   const U32 remaider = mSize % 8;
   const U32 testBytes = mSize / 8;

   for ( U32 i=0; i < testBytes; i++ )
      if ( mBits[i] != 0xFF )
         return false;

   if ( remaider == 0 )
      return true;

   const U8 mask = (U8)0xFF >> ( 8 - remaider );
   return ( mBits[testBytes] & mask ) == mask; 
}

bool BitVector::testAllClear() const
{
   const U32 remaider = mSize % 8;
   const U32 testBytes = mSize / 8;

   for ( U32 i=0; i < testBytes; i++ )
      if ( mBits[i] != 0 )
         return false;

   if ( remaider == 0 )
      return true;

   const U8 mask = (U8)0xFF >> ( 8 - remaider );
   return ( mBits[testBytes] & mask ) == 0;
}
