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

#include "core/crc.h"

#include "core/stream/stream.h"

//-----------------------------------------------------------------------------
// simple crc function - generates lookup table on first call

static U32 crcTable[256];
static bool crcTableValid;

static void calculateCRCTable()
{
   U32 val;

   for(S32 i = 0; i < 256; i++)
   {
      val = i;
      for(S32 j = 0; j < 8; j++)
      {
         if(val & 0x01)
            val = 0xedb88320 ^ (val >> 1);
         else
            val = val >> 1;
      }
      crcTable[i] = val;
   }

   crcTableValid = true;
}


//-----------------------------------------------------------------------------

U32 CRC::calculateCRC(const void * buffer, S32 len, U32 crcVal )
{
   // check if need to generate the crc table
   if(!crcTableValid)
      calculateCRCTable();

   // now calculate the crc
   char * buf = (char*)buffer;
   for(S32 i = 0; i < len; i++)
      crcVal = crcTable[(crcVal ^ buf[i]) & 0xff] ^ (crcVal >> 8);
   return(crcVal);
}

U32 CRC::calculateCRCStream(Stream *stream, U32 crcVal )
{
   // check if need to generate the crc table
   if(!crcTableValid)
      calculateCRCTable();

   // now calculate the crc
   stream->setPosition(0);
   S32 len = stream->getStreamSize();
   U8 buf[4096];

   S32 segCount = (len + 4095) / 4096;

   for(S32 j = 0; j < segCount; j++)
   {
      S32 slen = getMin(4096, len - (j * 4096));
      stream->read(slen, buf);
      crcVal = CRC::calculateCRC(buf, slen, crcVal);
   }
   stream->setPosition(0);
   return(crcVal);
}
