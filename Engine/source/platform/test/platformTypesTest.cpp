//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platform.h"
#include "core/util/endian.h"

TEST(PlatformTypes, Sizes)
{
   // Run through all the types and ensure they're the right size.
#define CheckType(typeName, expectedSize) \
   EXPECT_EQ( sizeof(typeName), expectedSize) \
      << "Wrong size for a " #typeName ", expected " #expectedSize;

   // One byte types.
   CheckType(bool, 1);
   CheckType(U8,   1);
   CheckType(S8,   1);
   CheckType(UTF8, 1);

   // Two byte types.
   CheckType(U16,   2);
   CheckType(S16,   2);
   CheckType(UTF16, 2);

   // Four byte types.
   CheckType(U32,   4);
   CheckType(S32,   4);
   CheckType(F32,   4);
   CheckType(UTF32, 4);

   // Eight byte types.
   CheckType(U64,   8);
   CheckType(S64,   8);
   CheckType(F64,   8);

   // 16 byte (128bit) types will go here, when we get some.
#undef CheckType
};

TEST(PlatformTypes, EndianConversion)
{
   // Convenient and non-palindrome byte patterns to test with.
   const U16 U16Test = 0xA1B2;
   const S16 S16Test = 0x52A1;

   const U32 U32Test = 0xA1B2C3D4;
   const S32 S32Test = 0xD4C3B2A1;
   const F32 F32Test = 1234.5678f;

   //const U64 U64Test = 0xA1B2C3D4E3F2E10A;
   //const S64 S64Test = 0x1A2B3C4D3E2F1EA0;
   const F64 F64Test = 12345678.9101112131415;

   // Run through all the conversions - bump stuff from host to little or big
   // endian and back again.
#define CheckEndianRoundTrip(type, b_or_l) \
   EXPECT_EQ( type##Test, convert##b_or_l##EndianToHost(convertHostTo##b_or_l##Endian(type##Test))) \
      << "Failed to convert the " #type " test value to " #b_or_l " endian and back to host endian order.";

#define CheckTypeBothWays(type)      \
   CheckEndianRoundTrip(type, B); \
   CheckEndianRoundTrip(type, L); 

#define CheckIntsForBitSize(bits)   \
   CheckTypeBothWays( U##bits ); \
   CheckTypeBothWays( S##bits );

   // Don't check 8-bit types - they aren't affected by endian issues.

   // Check the >1 byte int types, though.
   CheckIntsForBitSize(16);
   CheckIntsForBitSize(32);
   // CheckIntsForBitSize(64); // don't have convertHostToLEndian(const U64/S64) so this doesn't work

   // And check the float types.
   CheckTypeBothWays(F32);
   CheckTypeBothWays(F64);

   // We'd check 128bit types here, if we had any.

#undef CheckIntsForBitSize
#undef CheckTypeBothWays
#undef CheckEndianRoundTrip
};

TEST(PlatformTypes, EndianSwap)
{
   U32 swap32 = 0xABCDEF12;
   U16 swap16 = 0xABCD;
      
   EXPECT_EQ(endianSwap(swap32), 0x12EFCDAB)
      << "32 bit endianSwap should reverse byte order, but didn't.";
   EXPECT_EQ(endianSwap(swap16), 0xCDAB)
      << "16 bit endianSwap should reverse byte order, but didn't.";
};

#endif