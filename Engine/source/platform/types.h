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

#if (defined _MSC_VER) && (_MSC_VER <= 1500)
#include "platformWin32/stdint.h"
#else
#include <stdint.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------Basic Types--------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef signed char        S8;      ///< Compiler independent Signed Char
typedef unsigned char      U8;      ///< Compiler independent Unsigned Char

typedef signed short       S16;     ///< Compiler independent Signed 16-bit short
typedef unsigned short     U16;     ///< Compiler independent Unsigned 16-bit short

typedef signed int         S32;     ///< Compiler independent Signed 32-bit integer
typedef unsigned int       U32;     ///< Compiler independent Unsigned 32-bit integer

typedef float              F32;     ///< Compiler independent 32-bit float
typedef double             F64;     ///< Compiler independent 64-bit float

struct EmptyType {};                ///< "Null" type used by templates

#define TORQUE_UNUSED(var) (void)sizeof(var)

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------String Types--------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef char           UTF8;        ///< Compiler independent 8  bit Unicode encoded character

#if defined(_MSC_VER) && defined(__clang__)
// Clang's MSVC compatibility mode doesn't currently support /Zc:wchar_t-,
// which we rely on to avoid type conversion errors when calling system
// APIs when UTF16 is defined as unsigned short.  So, just define UTF16
// as wchar_t instead since it's always a 2 byte unsigned on windows anyway.
typedef wchar_t        UTF16;
#else
typedef unsigned short UTF16;       ///< Compiler independent 16 bit Unicode encoded character
#endif

typedef unsigned int   UTF32;       ///< Compiler independent 32 bit Unicode encoded character

typedef const char* StringTableEntry;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------- Type constants-------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define __EQUAL_CONST_F F32(0.000001)                                  ///< Constant float epsilon used for F32 comparisons

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
#ifdef TORQUE_CPU_X64
   typedef U64 MEM_ADDRESS;
#else
   typedef U32 MEM_ADDRESS;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------- GeneralMath Helpers ---------------------------------------- //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Determines if number is a power of two.
/// Zero is not a power of two. So we want take into account that edge case
inline bool isPow2(const U32 num)
{
   return (num != 0) && ((num & (num - 1)) == 0);
}

/// Determines the binary logarithm of the input value rounded down to the nearest power of 2.
inline U32 getBinLog2(U32 value)
{
   F32 floatValue = F32(value);
   return (*((U32 *) &floatValue) >> 23) - 127;
}

/// Determines the binary logarithm of the next greater power of two of the input number.
inline U32 getNextBinLog2(U32 number)
{
   return getBinLog2(number) + (isPow2(number) ? 0 : 1);
}

/// Determines the next greater power of two from the value. If the value is a power of two, it is returned.
inline U32 getNextPow2(U32 value)
{
   return isPow2(value) ? value : (1 << (getBinLog2(value) + 1));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------Many versions of min and max--------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DeclareTemplatizedMinMax(type) \
  inline type getMin(type a, type b) { return a > b ? b : a; } \
  inline type getMax(type a, type b) { return a > b ? a : b; }

DeclareTemplatizedMinMax( U32 )
DeclareTemplatizedMinMax( S32 )
DeclareTemplatizedMinMax( U16 )
DeclareTemplatizedMinMax( S16 )
DeclareTemplatizedMinMax( U8 )
DeclareTemplatizedMinMax( S8 )
DeclareTemplatizedMinMax( F32 )
DeclareTemplatizedMinMax( F64 )

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------FOURCC------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(TORQUE_BIG_ENDIAN)
#define makeFourCCTag(c0,c1,c2,c3) ((U32) ((((U32)((U8)(c0)))<<24) + (((U32)((U8)(c1)))<<16) + (((U32)((U8)(c2)))<<8) + ((((U32)((U8)(c3))))))
#else
#ifdef TORQUE_LITTLE_ENDIAN
#define makeFourCCTag(c3,c2,c1,c0) ((U32) ((((U32)((U8)(c0)))<<24) + (((U32)((U8)(c1)))<<16) + (((U32)((U8)(c2)))<<8) + (((U32)((U8)(c3))))))
#else
#error BYTE_ORDER not defined
#endif
#endif

#define BIT(x) (1 << (x))                       ///< Returns value with bit x set (2^x)

#if defined(TORQUE_OS_WIN)
#define STDCALL __stdcall
#else
#define STDCALL
#endif

#endif //_TORQUE_TYPES_H_
