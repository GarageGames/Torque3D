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

#ifndef _BITVECTORW_H_
#define _BITVECTORW_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

/// @see BitVector
class BitVectorW
{
      U32   mNumEntries;
      U32   mBitWidth;
      U32   mBitMask;
      U8 *  mDataPtr;

   public:
      BitVectorW()   {mDataPtr=NULL; setDims(0,0);}
      ~BitVectorW()  {if(mDataPtr) delete [] mDataPtr;}

      U32   bitWidth() const     {return mBitWidth;}
      U32   numEntries() const   {return mNumEntries;}
      U8 *  dataPtr() const      {return mDataPtr;}

      U32   getU17(U32 idx) const;        // get and set for bit widths
      void  setU17(U32 idx, U32 val);     // of 17 or less
      U32   numBytes() const;
      void  setDims(U32 sz, U32 w);
};

//-------------------------------------------------------------------------------------

inline U32 BitVectorW::numBytes() const
{
   if (mNumEntries > 0)
      return (mBitWidth * mNumEntries) + 32 >> 3;
   else
      return 0;
}

// Alloc the data - note it does work for a bit width of zero (lookups return zero)
inline void BitVectorW::setDims(U32 size, U32 width)
{
   if (mDataPtr)
      delete [] mDataPtr;

   if (size > 0 && width <= 17)
   {
      mBitWidth = width;
      mNumEntries = size;
      mBitMask = (1 << width) - 1;
      U32 dataSize = numBytes();
      mDataPtr = new U8 [dataSize];
      dMemset(mDataPtr, 0, dataSize);
   }
   else
   {
      mDataPtr = NULL;
      mBitWidth = mBitMask = mNumEntries = 0;
   }
}

//-------------------------------------------------------------------------------------
// For coding ease, the get and set methods might read or write an extra byte or two.
// If more or less max bit width is ever needed, add or remove the x[] expressions.

inline U32 BitVectorW::getU17(U32 i) const
{
   if (mDataPtr) {
      register U8 *  x = &mDataPtr[(i *= mBitWidth) >> 3];
      return (U32(*x) + (U32(x[1])<<8) + (U32(x[2])<<16) >> (i&7)) & mBitMask;
   }
   return 0;
}

inline void BitVectorW::setU17(U32 i, U32 value)
{
   if (mDataPtr) {
      register U8 *  x = &mDataPtr[(i *= mBitWidth) >> 3];
      register U32   mask = mBitMask << (i &= 7);
      x[0] = (x[0] & (~mask >>  0))  |  ((value <<= i) & (mask >>  0));
      x[1] = (x[1] & (~mask >>  8))  |  ((value >>  8) & (mask >>  8));
      x[2] = (x[2] & (~mask >> 16))  |  ((value >> 16) & (mask >> 16));
   }
}

#endif //_BITVECTORW_H_
