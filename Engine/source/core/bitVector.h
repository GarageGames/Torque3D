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

#ifndef _BITVECTOR_H_
#define _BITVECTOR_H_


/// Manage a vector of bits of arbitrary size.
class BitVector
{
   protected:

      /// The array of bytes that stores our bits.
      U8* mBits;

      /// The allocated size of the bit array.
      U32 mByteSize;

      /// The size of the vector in bits. 
      U32 mSize;

      /// Returns a size in bytes which is 32bit aligned
      /// and can hold all the requested bits.
      static U32 calcByteSize( const U32 numBits );

      /// Internal function which resizes the bit array.
      void _resize( U32 sizeInBits, bool copyBits );

      bool _test( const BitVector& vector, bool all ) const;

   public:

      /// Default constructor which creates an bit
      /// vector with a bit size of zero.
      BitVector();
      
      /// Constructs a bit vector with the desired size.
      /// @note The resulting vector is not cleared.
      BitVector( U32 sizeInBits );

      /// Copy constructor
      BitVector( const BitVector &r);
      
      /// Destructor.
      ~BitVector();

      /// @name Size Management
      /// @{

      /// Return true if the bit vector is empty.
      bool empty() const { return ( mSize == 0 ); }
      
      /// Resizes the bit vector.
      /// @note The new bits in the vector are not cleared and 
      /// contain random garbage bits.
      void setSize( U32 sizeInBits );

      /// Returns the size in bits.
      U32 getSize() const { return mSize; }

      /// Returns the 32bit aligned size in bytes.
      U32 getByteSize() const { return mByteSize; }

      /// Returns the bits.
      const U8* getBits() const { return mBits; }
      U8* getBits() { return mBits; }

      /// @}

      /// Copy the content of another bit vector.
      void copy( const BitVector &from );

      /// Copy the contents of another bit vector
      BitVector& operator=( const BitVector &r);

      /// @name Mutators
      /// Note that bits are specified by index, unlike BitSet32.
      /// @{

      /// Set the specified bit.
      void set(U32 bit);

      /// Set the specified bit on or off.
      void set(U32 bit, bool on );

      /// Set all the bits.
      void set();

      /// Clear the specified bit.
      void clear(U32 bit);

      /// Clear all the bits.
      void clear();

      /// Does an OR operation between BitVectors.
      void combineOR( const BitVector &other );

      /// Test that the specified bit is set.
      bool test(U32 bit) const;

      /// Test this vector's bits against all the corresponding bits
      /// in @a vector and return true if any of the bits that are
      /// set in @a vector are also set in this vector.
      ///
      /// @param vector Bit vector of the same size.
      bool testAny( const BitVector& vector ) const { return _test( vector, false ); }

      /// Test this vector's bits against all the corresponding bits
      /// in @a vector and return true if all of the bits that are
      /// set in @a vector are also set in this vector.
      ///
      /// @param vector Bit vector of the same size.
      bool testAll( const BitVector& vector ) const { return _test( vector, true ); }

      /// Return true if all bits are set.
      bool testAll() const;
      
      /// Return true if all bits are clear.
      bool testAllClear() const;

      /// @}
};

inline BitVector::BitVector()
{
   mBits     = NULL;
   mByteSize = 0;
   mSize = 0;
}


inline BitVector::BitVector( U32 sizeInBits )
{
   mBits     = NULL;
   mByteSize = 0;
   mSize = 0;
   setSize( sizeInBits );
}

inline BitVector::BitVector( const BitVector &r )
{
   copy(r);
}

inline BitVector::~BitVector()
{
   delete [] mBits;
   mBits = NULL;
   mByteSize = 0;
   mSize = 0;
}

inline U32 BitVector::calcByteSize( U32 numBits )
{
   // Make sure that we are 32 bit aligned
   return (((numBits + 0x7) >> 3) + 0x3) & ~0x3;
}

inline void BitVector::setSize( const U32 sizeInBits )
{
   _resize( sizeInBits, true );
}

inline void BitVector::clear()
{
   if (mSize != 0)
      dMemset( mBits, 0x00, getByteSize() );
}

inline void BitVector::copy( const BitVector &from )
{
   _resize( from.getSize(), false );
   if (mSize != 0)
      dMemcpy( mBits, from.getBits(), getByteSize() );
}

inline BitVector& BitVector::operator=( const BitVector &r)
{
   copy(r);
   return *this;
}

inline void BitVector::set()
{
   if (mSize != 0)
      dMemset(mBits, 0xFF, getByteSize() );
}

inline void BitVector::set(U32 bit)
{
   AssertFatal(bit < mSize, "BitVector::set - Error, out of range bit!");

   mBits[bit >> 3] |= U8(1 << (bit & 0x7));
}

inline void BitVector::set(U32 bit, bool on )
{
   AssertFatal(bit < mSize, "BitVector::set - Error, out of range bit!");

   if ( on )
      mBits[bit >> 3] |= U8(1 << (bit & 0x7));
   else
      mBits[bit >> 3] &= U8(~(1 << (bit & 0x7)));
}

inline void BitVector::clear(U32 bit)
{
   AssertFatal(bit < mSize, "BitVector::clear - Error, out of range bit!");
   mBits[bit >> 3] &= U8(~(1 << (bit & 0x7)));
}

inline bool BitVector::test(U32 bit) const
{
   AssertFatal(bit < mSize, "BitVector::test - Error, out of range bit!");
   return (mBits[bit >> 3] & U8(1 << (bit & 0x7))) != 0;
}

#endif //_BITVECTOR_H_
