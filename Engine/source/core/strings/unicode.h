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

#ifndef _UNICODE_H_
#define _UNICODE_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif


/// Unicode conversion utility functions
///
/// Some definitions first: 
/// - <b>Code Point</b>: a single character of Unicode text. Used to disabmiguate from C char type.
/// - <b>UTF-32</b>: a Unicode encoding format where one code point is always 32 bits wide.
///   This format can in theory contain any Unicode code point that will ever be needed, now or in the future. 4billion+ code points should be enough, right?
/// - <b>UTF-16</b>: a variable length Unicode encoding format where one code point can be
///   either one or two 16-bit code units long.
/// - <b>UTF-8</b>: a variable length Unicode endocing format where one code point can be
///   up to four 8-bit code units long. The first bit of a single byte UTF-8 code point is 0.
///   The first few bits of a multi-byte code point determine the length of the code point.
///   @see http://en.wikipedia.org/wiki/UTF-8
/// - <b>Surrogate Pair</b>: a pair of special UTF-16 code units, that encode a code point
///   that is too large to fit into 16 bits. The surrogate values sit in a special reserved range of Unicode.
/// - <b>Code Unit</b>: a single unit of a variable length Unicode encoded code point.
///   UTF-8 has 8 bit wide code units. UTF-16 has 16 bit wide code units.
/// - <b>BMP</b>: "Basic Multilingual Plane". Unicode values U+0000 - U+FFFF. This range
///   of Unicode contains all the characters for all the languages of the world, that one would
///   usually be interested in. All code points in the BMP are 16 bits wide or less.

/// The current implementation of these conversion functions deals only with the BMP.
/// Any code points above 0xFFFF, the top of the BMP, are replaced with the
///  standard unicode replacement character: 0xFFFD.
/// Any UTF16 surrogates are read correctly, but replaced.
/// UTF-8 code points up to 6 code units wide will be read, but 5+ is illegal, 
///  and 4+ is above the BMP, and will be replaced.
///  This means that UTF-8 output is clamped to 3 code units ( bytes ) per code point.

//-----------------------------------------------------------------------------
/// Functions that convert buffers of unicode code points, allocating a buffer.
/// - These functions allocate their own return buffers. You are responsible for
///   calling delete[] on these buffers.
/// - Because they allocate memory, do not use these functions in a tight loop.
/// - These are useful when you need a new long term copy of a string.
UTF16* createUTF16string( const UTF8 *unistring);

UTF8*  createUTF8string( const UTF16 *unistring);

//-----------------------------------------------------------------------------
/// Functions that convert buffers of unicode code points, into a provided buffer.
/// - These functions are useful for working on existing buffers.
/// - These cannot convert a buffer in place. If unistring is the same memory as
///   outbuffer, the behavior is undefined.
/// - The converter clamps output to the BMP (Basic Multilingual Plane) .
/// - Conversion to UTF-8 requires a buffer of 3 bytes (U8's) per character, + 1.
/// - Conversion to UTF-16 requires a buffer of 1 U16 (2 bytes) per character, + 1.
/// - Conversion to UTF-32 requires a buffer of 1 U32 (4 bytes) per character, + 1.
/// - UTF-8 only requires 3 bytes per character in the worst case.
/// - Output is null terminated. Be sure to provide 1 extra byte, U16 or U32 for
///   the null terminator, or you will see truncated output.
/// - If the provided buffer is too small, the output will be truncated.
U32 convertUTF8toUTF16N(const UTF8 *unistring, UTF16 *outbuffer, U32 len);

U32 convertUTF16toUTF8N( const UTF16 *unistring, UTF8  *outbuffer, U32 len);

/// Safe conversion function for statically sized buffers.
template <size_t N>
inline U32 convertUTF8toUTF16(const UTF8 *unistring, UTF16 (&outbuffer)[N])
{
   return convertUTF8toUTF16N(unistring, outbuffer, (U32) N);
}

/// Safe conversion function for statically sized buffers.
template <size_t N>
inline U32 convertUTF16toUTF8(const UTF16 *unistring, UTF8 (&outbuffer)[N])
{
   return convertUTF16toUTF8N(unistring, outbuffer, (U32) N);
}

//-----------------------------------------------------------------------------
/// Functions that converts one unicode codepoint at a time
/// - Since these functions are designed to be used in tight loops, they do not
///   allocate buffers.
/// - oneUTF8toUTF32() and oneUTF16toUTF32() return the converted Unicode code point
///   in *codepoint, and set *unitsWalked to the \# of code units *codepoint took up.
///   The next Unicode code point should start at *(codepoint + *unitsWalked).
/// - oneUTF32toUTF8()  requires a 3 byte buffer, and returns the \# of bytes used.
UTF32  oneUTF8toUTF32( const UTF8 *codepoint,  U32 *unitsWalked = NULL);
UTF32  oneUTF16toUTF32(const UTF16 *codepoint, U32 *unitsWalked = NULL);
UTF16  oneUTF32toUTF16(const UTF32 codepoint);
U32    oneUTF32toUTF8( const UTF32 codepoint, UTF8 *threeByteCodeunitBuf);

//-----------------------------------------------------------------------------
/// Functions that calculate the length of unicode strings.
/// - Since calculating the length of a UTF8 string is nearly as expensive as
///   converting it to another format, a dStrlen for UTF8 is not provided here.
/// - If *unistring does not point to a null terminated string of the correct type,
///   the behavior is undefined.
U32 dStrlen(const UTF16 *unistring);
U32 dStrlen(const UTF32 *unistring);

//-----------------------------------------------------------------------------
/// Scanning for characters in unicode strings
UTF16* dStrrchr(UTF16* unistring, U32 c);
const UTF16* dStrrchr(const UTF16* unistring, U32 c);

UTF16* dStrchr(UTF16* unistring, U32 c);
const UTF16* dStrchr(const UTF16* unistring, U32 c);
//-----------------------------------------------------------------------------
/// Functions that scan for characters in a utf8 string.
/// - this is useful for getting a character-wise offset into a UTF8 string, 
///   as opposed to a byte-wise offset into a UTF8 string: foo[i]
const UTF8* getNthCodepoint(const UTF8 *unistring, const U32 n);

//------------------------------------------------------------------------------
/// Functions to read and validate UTF BOMs (Byte Order Marker)
/// For reference: http://en.wikipedia.org/wiki/Byte_Order_Mark
bool chompUTF8BOM( const char *inString, char **outStringPtr );
bool isValidUTF8BOM( U8 bom[4] );

#endif // _UNICODE_H_