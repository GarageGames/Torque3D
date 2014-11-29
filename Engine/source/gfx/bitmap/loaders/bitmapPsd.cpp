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

#include "core/stream/stream.h"
#include "gfx/bitmap/gBitmap.h"
#include "console/consoleInternal.h"

static bool sReadPSD(Stream &stream, GBitmap *bitmap);
static bool sWritePSD(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

static struct _privateRegisterPSD
{
   _privateRegisterPSD()
   {
      GBitmap::Registration reg;

      reg.extensions.push_back( "psd" );

      reg.readFunc = sReadPSD;
      reg.writeFunc = sWritePSD;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterPSD;

//------------------------------------------------------------------------------

static void sPSDReadRawRGBA(Stream *stream, U8 *data, U32 numPixels, U16 channels)
{
   for(S32 ch = 0; ch < 4; ch++)
   {
      U8 *currentPixel = data + ch;
      if(ch < channels)
      {
         for(S32 i = 0; i < numPixels; i++)
         {
            stream->read(currentPixel);
            currentPixel += 4;
         }
      }
      else
      {
         for(S32 i = 0; i < numPixels; i++)
         {
            *currentPixel = (ch == 3) ? 255 : 0;
            currentPixel += 4;
         }
      }
   }
}

//------------------------------------------------------------------------------

#define skipStream(skip) \
   stream->setPosition(stream->getPosition() + skip);

static U8 _readU8(Stream * stream)
{
   U8 val;
   stream->read(&val);
   return val;
}

#define readU8() _readU8(stream);

static void sPSDReadRLE(Stream *stream, U8 *data, U32 numPixels, U16 channels, U32 height)
{
   // TIFF-based RLE
   // When RLE compression is used, we need to skip 2 bytes (for each row) per channel, not sure what it contains though
   skipStream(channels * height * 2);
   // Read it!
   for(S32 ch = 0; ch < 4; ch++)
   {
      U8 *currentPixel = data + ch;
      if(ch < channels)
      {
         S32 pixelsRead = 0;
         while(pixelsRead < numPixels)
         {
            S32 rowLen = readU8();
            if(rowLen == 128)
               continue;
            if(rowLen < 128)
            {
               rowLen++;
               pixelsRead += rowLen;
               while(rowLen)
               {
                  *currentPixel = readU8();
                  currentPixel += 4;
                  rowLen--;
               }
            }
            else
            {
               U32 byte = readU8();
               rowLen ^= 0x0FF;
               rowLen += 2;
               pixelsRead += rowLen;
               while(rowLen)
               {
                  *currentPixel = byte;
                  currentPixel += 4;
                  rowLen--;
               }
            }
         }
      }
      else
      {
         for(S32 i = 0; i < numPixels; i++)
         {
            *currentPixel = (ch == 3 ? 255 : 0);
            currentPixel += 4;
         }
      }
   }
}

struct PSDHeader
{
   U32 fileId;          // PSD identifier (expecting "8BPS") 4
   U16 version;         // PSD version (expecting 1) 2
   U16 channels;        // Number of channels stored in PSD 2
   U16 depth;           // Depth (expecting 8 bits) 2
   U16 colorMode;       // Color mode (expecting pure RGB) 2
   U32 width;           // the width of the image. 4
   U32 height;          // the height of the image. 4
   U16 compressionType; // Expect 1 for RLE or 0 for raw uncompressed data. Anything else is unsupported! 2
};

static U16 _readU16(Stream *stream)
{
   U8 val1;
   U8 val2;
   stream->read(&val1);
   stream->read(&val2);
   return (val1 << 8) + val2;
}

#define readU16() _readU16(stream);

static U32 _readU32(Stream *stream)
{
   U16 val1 = readU16();
   U16 val2 = readU16();
   return (val1 << 16) + val2;
}

#define readU32() _readU32(stream);

static bool sPSDReadAndBuildHeader(Stream *stream, PSDHeader *header)
{
   // Identify if it is a real PSD
   header->fileId = readU32();
   if(header->fileId != 0x38425053) // Valid PSD file have "8BPS" header
   {
      Con::errorf("Can't read -- non-PSD file specified.");
      return false;
   }
   // File version
   header->version = readU16();
   if(header->version != 1)
   {
      Con::errorf("Unsupported PSD version! Got %d, expected 1.", header->version);
      return false;
   }

   // Next 6 bytes are reserved, skip it!
   skipStream(6);
   // Read header
   header->channels = readU16();
   header->height = readU32();
   header->width = readU32();
   header->depth = readU16();
   header->colorMode = readU16();

   // The PSD stores part of additional data in a header, so we should skip it
   U32 tmp = readU32();
   skipStream(tmp); // Some color modes stores indexed color of its palette
   tmp = readU32();
   skipStream(tmp); // xmpmeta
   tmp = readU32();
   skipStream(tmp); // Reserved data?

   header->compressionType = readU16();

   // Now, check everything, to be sure the format is valid for reading
   if(header->channels < 0 || header->channels > 16)
   {
      Con::errorf("Unsupported number of channels found: %d. We can read PSD with 0>16 only!", header->channels);
      return false;
   }
   if(header->depth != 8)
   {
      Con::errorf("Unsupported bit depth: %d, we can load only 8 bits!", header->depth);
      return false;
   }
   if(header->colorMode != 3)
   {
      //TODO: Implement mode 1 (grayscale)
      Con::errorf("Unsupported color mode: %d, we can load only RGB (mode 3)!", header->colorMode);
      return false;
   }

   return true;
}

#undef readU32
#undef readU16
#undef readU8
#undef skipStream

//------------------------------------------------------------------------------

static bool sReadPSD(Stream &stream, GBitmap *bitmap)
{
   PSDHeader header;
   if(!sPSDReadAndBuildHeader(&stream, &header))
      return false;

   // Sanity check
   U32 numPixels = header.width * header.height;
   if( numPixels == 0 )
   {
      Con::errorf( "Texture has width and/or height set to 0" );
      return false;
   }

   bitmap->allocateBitmap( header.width, header.height, false, GFXFormatR8G8B8A8 );
   U8 *buffer = bitmap->getAddress(0, 0);
   switch(header.compressionType)
   {
   case 0: // Uncompressed RGBA data (8-bit raw image)
      sPSDReadRawRGBA(&stream, buffer, numPixels, header.channels);
      break;
   case 1: // RLE
      sPSDReadRLE(&stream, buffer, numPixels, header.channels, header.height);
      break;
   default:
      Con::printf("Unsupported compression method in PSD: %d", header.compressionType);
      return false;
      break;
   }

   // If we read 4 channels, means we have an alpha channel
   bitmap->setHasTransparency( header.channels == 4 );

   return true;
}

static bool sWritePSD(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   AssertISV(false, "GBitmap::writePSD - doesn't support writing PSD files!")
   return false;
}
