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

#ifndef _CRC_H_
#define _CRC_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif


class Stream;

namespace CRC
{
   /// Initial value for CRCs
   const U32   INITIAL_CRC_VALUE = 0xFFFFFFFF;

   /// Value XORd with the CRC to post condition it (ones complement)
   const U32   CRC_POSTCOND_VALUE = 0xFFFFFFFF;

   /// An invalid CRC
   const U32   INVALID_CRC = 0xFFFFFFFF;

   U32 calculateCRC(const void * buffer, S32 len, U32 crcVal = INITIAL_CRC_VALUE);
   U32 calculateCRCStream(Stream *stream, U32 crcVal = INITIAL_CRC_VALUE);
}

#endif

