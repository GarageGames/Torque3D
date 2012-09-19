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

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

//------------------------------------------------------------------------------
// Endian conversions

inline U8 endianSwap(const U8 in_swap)
{
   return in_swap;
}

inline S8 endianSwap(const S8 in_swap)
{
   return in_swap;
}

/**
Convert the byte ordering on the U16 to and from big/little endian format.
@param in_swap Any U16
@returns swapped U16.
*/

inline U16 endianSwap(const U16 in_swap)
{
   return U16(((in_swap >> 8) & 0x00ff) |
      ((in_swap << 8) & 0xff00));
}

inline S16 endianSwap(const S16 in_swap)
{
   return S16(endianSwap(U16(in_swap)));
}

/**
Convert the byte ordering on the U32 to and from big/little endian format.
@param in_swap Any U32
@returns swapped U32.
*/
inline U32 endianSwap(const U32 in_swap)
{
   return U32(((in_swap >> 24) & 0x000000ff) |
      ((in_swap >>  8) & 0x0000ff00) |
      ((in_swap <<  8) & 0x00ff0000) |
      ((in_swap << 24) & 0xff000000));
}

inline S32 endianSwap(const S32 in_swap)
{
   return S32(endianSwap(U32(in_swap)));
}

inline U64 endianSwap(const U64 in_swap)
{
   U32 *inp = (U32 *) &in_swap;
   U64 ret;
   U32 *outp = (U32 *) &ret;
   outp[0] = endianSwap(inp[1]);
   outp[1] = endianSwap(inp[0]);
   return ret;
}

inline S64 endianSwap(const S64 in_swap)
{
   return S64(endianSwap(U64(in_swap)));
}

inline F32 endianSwap(const F32 in_swap)
{
   U32 result = endianSwap(* ((U32 *) &in_swap) );
   return * ((F32 *) &result);
}

inline F64 endianSwap(const F64 in_swap)
{
   U64 result = endianSwap(* ((U64 *) &in_swap) );
   return * ((F64 *) &result);
}

//------------------------------------------------------------------------------
// Endian conversions

#ifdef TORQUE_LITTLE_ENDIAN

#define TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(type) \
   inline type convertHostToLEndian(type i) { return i; } \
   inline type convertLEndianToHost(type i) { return i; } \
   inline type convertHostToBEndian(type i) { return endianSwap(i); } \
   inline type convertBEndianToHost(type i) { return endianSwap(i); }

#elif defined(TORQUE_BIG_ENDIAN)

#define TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(type) \
   inline type convertHostToLEndian(type i) { return endianSwap(i); } \
   inline type convertLEndianToHost(type i) { return endianSwap(i); } \
   inline type convertHostToBEndian(type i) { return i; } \
   inline type convertBEndianToHost(type i) { return i; }

#else
#error "Endian define not set!"
#endif


TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(U8)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(S8)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(U16)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(S16)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(U32)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(S32)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(U64)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(S64)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(F32)
TORQUE_DECLARE_TEMPLATIZED_ENDIAN_CONV(F64)

#endif

