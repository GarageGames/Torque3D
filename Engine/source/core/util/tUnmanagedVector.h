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

#ifndef _TUNMANAGEDVECTOR_H_
#define _TUNMANAGEDVECTOR_H_


/// An array that does not manage the memory it gets passed.  Conversely, the
/// array cannot be enlarged.
///
/// @note As the array does not manage the instances it uses, it will also not
///   destruct them when it is itself destructed.
template< typename T >
class UnmanagedVector
{
   protected:

      U32 mCount;
      T* mArray;

   public:

      typedef T* iterator;
      typedef const T* const_iterator;

      UnmanagedVector()
         : mCount( 0 ), mArray( NULL ) {}
      UnmanagedVector( T* array, U32 count )
         : mCount( count ), mArray( array ) {}

      U32 size() const { return mCount; }
      bool empty() const { return ( mCount == 0 ); }
      const T* address() const { return mArray; }
      T* address() { return mArray; }

      void clear() { mCount = 0; mArray = NULL; }

      iterator begin() { return &mArray[ 0 ]; }
      iterator end() { return &mArray[ mCount ]; }
      const_iterator begin() const { return &mArray[ 0 ]; }
      const_iterator end() const { return &mArray[ mCount ]; }

      const T& first() const
      {
         AssertFatal( !empty(), "UnmanagedVector::first - Vector is empty" );
         return mArray[ 0 ];
      }
      T& first()
      {
         AssertFatal( !empty(), "UnmanagedVector::first - Vector is empty" );
         return mArray[ 0 ];
      }
      const T& last() const
      {
         AssertFatal( !empty(), "UnmanagedVector::last - Vector is empty" );
         return mArray[ mCount - 1 ];
      }
      T& last()
      {
         AssertFatal( !empty(), "UnmanagedVector::last - Vector is empty" );
         return mArray[ mCount - 1 ];
      }

      const T& operator []( U32 index ) const
      {
         AssertFatal( index <= size(), "UnmanagedVector::operator[] - Index out of range" );
         return mArray[ index ];
      }
      T& operator []( U32 index )
      {
         AssertFatal( index <= size(), "UnmanagedVector::operator[] - Index out of range" );
         return mArray[ index ];
      }
};

#endif // !_TUNMANAGEDVECTOR_H_
