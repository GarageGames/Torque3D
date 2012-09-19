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

#ifndef _TORQUE_TYPES_H_
#define _TORQUE_TYPES_H_

//------------------------------------------------------------------------------
//-------------------------------------- Basic Types...

typedef signed char        S8;      ///< Compiler independent Signed Char
typedef unsigned char      U8;      ///< Compiler independent Unsigned Char

typedef signed short       S16;     ///< Compiler independent Signed 16-bit short
typedef unsigned short     U16;     ///< Compiler independent Unsigned 16-bit short

typedef signed int         S32;     ///< Compiler independent Signed 32-bit integer
typedef unsigned int       U32;     ///< Compiler independent Unsigned 32-bit integer

typedef float              F32;     ///< Compiler independent 32-bit float
typedef double             F64;     ///< Compiler independent 64-bit float

struct EmptyType {};             ///< "Null" type used by templates

#define TORQUE_UNUSED(var) (void)var

//------------------------------------------------------------------------------
//------------------------------------- String Types

typedef char           UTF8;        ///< Compiler independent 8  bit Unicode encoded character
typedef unsigned short UTF16;       ///< Compiler independent 16 bit Unicode encoded character
typedef unsigned int   UTF32;       ///< Compiler independent 32 bit Unicode encoded character

typedef const char* StringTableEntry;

//------------------------------------------------------------------------------
//-------------------------------------- Type constants...
#define __EQUAL_CONST_F F32(0.000001)                             ///< Constant float epsilon used for F32 comparisons

extern const F32 Float_Inf;
static const F32 Float_One  = F32(1.0);                           ///< Constant float 1.0
static const F32 Float_Half = F32(0.5);                           ///< Constant float 0.5
static const F32 Float_Zero = F32(0.0);                           ///< Constant float 0.0
static const F32 Float_Pi   = F32(3.14159265358979323846);        ///< Constant float PI
static const F32 Float_2Pi  = F32(2.0 * 3.14159265358979323846);  ///< Constant float 2*PI
static const F32 Float_InversePi = F32(1.0 / 3.14159265358979323846); ///< Constant float 1 / PI
static const F32 Float_HalfPi = F32(0.5 * 3.14159265358979323846);    ///< Constant float 1/2 * PI
static const F32 Float_2InversePi = F32(2.0 / 3.14159265358979323846);///< Constant float 2 / PI
static const F32 Float_Inverse2Pi = F32(0.5 / 3.14159265358979323846);///< Constant float 0.5 / PI

static const F32 Float_Sqrt2 = F32(1.41421356237309504880f);          ///< Constant float sqrt(2)
static const F32 Float_SqrtHalf = F32(0.7071067811865475244008443f);  ///< Constant float sqrt(0.5)

static const S8  S8_MIN  = S8(-128);                              ///< Constant Min Limit S8
static const S8  S8_MAX  = S8(127);                               ///< Constant Max Limit S8
static const U8  U8_MAX  = U8(255);                               ///< Constant Max Limit U8

static const S16 S16_MIN = S16(-32768);                           ///< Constant Min Limit S16
static const S16 S16_MAX = S16(32767);                            ///< Constant Max Limit S16
static const U16 U16_MAX = U16(65535);                            ///< Constant Max Limit U16

static const S32 S32_MIN = S32(-2147483647 - 1);                  ///< Constant Min Limit S32
static const S32 S32_MAX = S32(2147483647);                       ///< Constant Max Limit S32
static const U32 U32_MAX = U32(0xffffffff);                       ///< Constant Max Limit U32

static const F32 F32_MIN = F32(1.175494351e-38F);                 ///< Constant Min Limit F32
static const F32 F32_MAX = F32(3.402823466e+38F);                 ///< Constant Max Limit F32

// define all the variants of Offset that we might use
#define _Offset_Normal(x, cls) ((dsize_t)((const char *)&(((cls *)1)->x)-(const char *)1))
#define _Offset_Variant_1(x, cls) ((int)(&((cls *)1)->x) - 1)
#define _Offset_Variant_2(x, cls) offsetof(cls, x) // also requires #include <stddef.h>

//--------------------------------------
// Identify the compiler being used

// PC-lint
#if defined(_lint)
#  include "platform/types.lint.h"
// Metrowerks CodeWarrior
#elif defined(__MWERKS__)
#  include "platform/types.codewarrior.h"
// Microsoft Visual C++/Visual.NET
#elif defined(_MSC_VER)
#  include "platform/types.visualc.h"
// GNU GCC
#elif defined(__GNUC__)
#  include "platform/types.gcc.h"
#else
#  error "Unknown Compiler"
#endif

/// Integral type matching the host's memory address width.
#ifdef TORQUE_64BITS
   typedef U64 MEM_ADDRESS;
#else
   typedef U32 MEM_ADDRESS;
#endif

//-------------------------------------- Some all-around useful inlines and globals
//

/// Returns power of 2 number which is as small as possible but
/// still greater than or equal to input number.  Note: returns 0
/// for an input of 0 even though that is not a power of 2.
/// @param num Any U32
inline U32 getNextPow2(U32 num)
{
   // Taken from: http://graphics.stanford.edu/~seander/bithacks.html

   num--;
   num |= num >> 1;
   num |= num >> 2;
   num |= num >> 4;
   num |= num >> 8;
   num |= num >> 16;
   num++;

   return num;
}

/// Return integer log2 of input number (rounding down).  So, e.g.,
/// getBinLog2(7) == 2 whereas getBinLog2(8) == 3.  If known
/// @param num Any U32
/// @param knownPow2 Is num a known power of 2?
inline U32 getBinLog2(U32 num, bool knownPow2 = false)
{
   // Taken from: http://graphics.stanford.edu/~seander/bithacks.html
   
   static const U32 MultiplyDeBruijnBitPosition[32] = 
   {
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
   };
   
   if (!knownPow2)
   {
      num |= num >> 1; // first round down to power of 2 
      num |= num >> 2;
      num |= num >> 4;
      num |= num >> 8;
      num |= num >> 16;
      num = (num >> 1) + 1;
   }
   
   return MultiplyDeBruijnBitPosition[(num * 0x077CB531UL) >> 27];
}

///   Determines if the given U32 is some 2^n
/// @param num Any U32
///   @returns true if in_num is a power of two, otherwise false
inline bool isPow2(const U32 num)
{
   return (num & (num - 1)) == 0;
}

/// Determines the binary logarithm of the next greater power of two of the input number.
inline U32 getNextBinLog2(U32 number)
{
   return getBinLog2(number) + (isPow2(number) ? 0 : 1);
}

//----------------Many versions of min and max-------------
//---not using template functions because MS VC++ chokes---

/// Returns the lesser of the two parameters: a & b.
inline U32 getMin(U32 a, U32 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline U16 getMin(U16 a, U16 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline U8 getMin(U8 a, U8 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S32 getMin(S32 a, S32 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S16 getMin(S16 a, S16 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S8 getMin(S8 a, S8 b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline float getMin(float a, float b)
{
   return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline double getMin(double a, double b)
{
   return a>b ? b : a;
}

/// Returns the greater of the two parameters: a & b.
inline U32 getMax(U32 a, U32 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline U16 getMax(U16 a, U16 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline U8 getMax(U8 a, U8 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S32 getMax(S32 a, S32 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S16 getMax(S16 a, S16 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S8 getMax(S8 a, S8 b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline float getMax(float a, float b)
{
   return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline double getMax(double a, double b)
{
   return a>b ? a : b;
}

//-------------------------------------- Use this instead of Win32 FOURCC()
//                                        macro...
//
#define makeFourCCTag(ch0, ch1, ch2, ch3)    \
   (((U32(ch0) & 0xFF) << 0)  |             \
    ((U32(ch1) & 0xFF) << 8)  |             \
    ((U32(ch2) & 0xFF) << 16) |             \
    ((U32(ch3) & 0xFF) << 24) )

#define makeFourCCString(ch0, ch1, ch2, ch3) { ch0, ch1, ch2, ch3 }

#define BIT(x) (1 << (x))                       ///< Returns value with bit x set (2^x)

#if defined(TORQUE_OS_WIN32)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

#endif //_TORQUE_TYPES_H_
