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


static bool sReadBMP(Stream &stream, GBitmap *bitmap);
static bool sWriteBMP(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

static struct _privateRegisterBMP
{
   _privateRegisterBMP()
   {
      GBitmap::Registration reg;

      reg.extensions.push_back( "bmp" );

      reg.readFunc = sReadBMP;
      reg.writeFunc = sWriteBMP;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterBMP;


// structures mirror those defined by the win32 API

struct RGBQUAD
{
   U8 rgbBlue;
   U8 rgbGreen;
   U8 rgbRed;
   U8 rgbReserved;
};

struct BITMAPFILEHEADER
{
   U16 bfType;
   U32 bfSize;
   U16 bfReserved1;
   U16 bfReserved2;
   U32 bfOffBits;
};

struct BITMAPINFOHEADER
{
   U32 biSize;
   S32 biWidth;
   S32 biHeight;
   U16 biPlanes;
   U16 biBitCount;
   U32 biCompression;
   U32 biSizeImage;
   S32 biXPelsPerMeter;
   S32 biYPelsPerMeter;
   U32 biClrUsed;
   U32 biClrImportant;
};

// constants for the biCompression field
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L


//------------------------------------------------------------------------------
//-------------------------------------- Supplementary I/O (Partially located in
//                                                          bitmapPng.cc)
//

static bool sReadBMP(Stream &stream, GBitmap *bitmap)
{
   BITMAPINFOHEADER  bi;
   BITMAPFILEHEADER  bf;
   RGBQUAD           rgb[256];

   stream.read(&bf.bfType);
   stream.read(&bf.bfSize);
   stream.read(&bf.bfReserved1);
   stream.read(&bf.bfReserved2);
   stream.read(&bf.bfOffBits);

   stream.read(&bi.biSize);
   stream.read(&bi.biWidth);
   stream.read(&bi.biHeight);
   stream.read(&bi.biPlanes);
   stream.read(&bi.biBitCount);
   stream.read(&bi.biCompression);
   stream.read(&bi.biSizeImage);
   stream.read(&bi.biXPelsPerMeter);
   stream.read(&bi.biYPelsPerMeter);
   stream.read(&bi.biClrUsed);
   stream.read(&bi.biClrImportant);

   GFXFormat fmt = GFXFormatR8G8B8;
   if(bi.biBitCount == 8)
   {
      // read in texture palette
      if(!bi.biClrUsed)
         bi.biClrUsed = 256;
      stream.read(sizeof(RGBQUAD) * bi.biClrUsed, rgb);
   }
   bitmap->allocateBitmap(bi.biWidth, bi.biHeight, false, fmt);
   U32   width  = bitmap->getWidth();
   U32   height = bitmap->getHeight();
   U32   bytesPerPixel = bitmap->getBytesPerPixel();

   for(U32 i = 0; i < bi.biHeight; i++)
   {
      U8 *rowDest = bitmap->getAddress(0, height - i - 1);
      if (bi.biBitCount == 8)
      {
         // use palette...don't worry about being slow
         for (S32 j=0; j<width; j++)
         {
            U8 palIdx;
            stream.read(&palIdx);
            U8 * pixelLocation = &rowDest[j*bytesPerPixel];
            pixelLocation[0] = rgb[palIdx].rgbRed;
            pixelLocation[1] = rgb[palIdx].rgbGreen;
            pixelLocation[2] = rgb[palIdx].rgbBlue;
            if (bytesPerPixel==3)
               pixelLocation[3] = 255;
         }
      }
      else
         stream.read(bytesPerPixel * width, rowDest);
   }

   if(bytesPerPixel == 3 && bi.biBitCount != 8) // do BGR swap
   {
      U8 *ptr = bitmap->getAddress(0,0);
      for(int i = 0; i < width * height; i++)
      {
         U8 tmp = ptr[0];
         ptr[0] = ptr[2];
         ptr[2] = tmp;
         ptr += 3;
      }
   }

   // We know BMP's don't have any transparency
   bitmap->setHasTransparency(false);

   return true;
}

static bool sWriteBMP(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   TORQUE_UNUSED( compressionLevel );  // BMP does not use compression

   BITMAPINFOHEADER  bi;
   BITMAPFILEHEADER  bf;

   bi.biSize            = sizeof(BITMAPINFOHEADER);
   bi.biWidth           = bitmap->getWidth();
   bi.biHeight          = bitmap->getHeight();         //our data is top-down
   bi.biPlanes = 1;

   if(bitmap->getFormat() == GFXFormatR8G8B8)
   {
      bi.biBitCount = 24;
      bi.biCompression = BI_RGB;
      bi.biClrUsed = 0;
   }
   else
   {
      bi.biBitCount = 0;
      bi.biCompression = BI_RGB; // Removes warning C4701 on line
      AssertISV(false, "GBitmap::writeMSBmp - only support R8G8B8 formats!");
   }

   U32   width  = bitmap->getWidth();
   U32   height = bitmap->getHeight();

   U32 bytesPP = bi.biBitCount >> 3;
   bi.biSizeImage       = width * height * bytesPP;
   bi.biXPelsPerMeter   = 0;
   bi.biYPelsPerMeter   = 0;
   bi.biClrUsed         = 0;
   bi.biClrImportant    = 0;

   bf.bfType   = makeFourCCTag('B','M',0,0);     //Type of file 'BM'
   bf.bfOffBits= sizeof(BITMAPINFOHEADER)
               + sizeof(BITMAPFILEHEADER)
               + (sizeof(RGBQUAD)*bi.biClrUsed);
   bf.bfSize            = bf.bfOffBits + bi.biSizeImage;
   bf.bfReserved1       = 0;
   bf.bfReserved2       = 0;

   stream.write(bf.bfType);
   stream.write(bf.bfSize);
   stream.write(bf.bfReserved1);
   stream.write(bf.bfReserved2);
   stream.write(bf.bfOffBits);

   stream.write(bi.biSize);
   stream.write(bi.biWidth);
   stream.write(bi.biHeight);
   stream.write(bi.biPlanes);
   stream.write(bi.biBitCount);
   stream.write(bi.biCompression);
   stream.write(bi.biSizeImage);
   stream.write(bi.biXPelsPerMeter);
   stream.write(bi.biYPelsPerMeter);
   stream.write(bi.biClrUsed);
   stream.write(bi.biClrImportant);

   //write the bitmap bits
   U8* pMSUpsideDownBits = new U8[bi.biSizeImage];
   for (U32 i = 0; i < height; i++)
   {
      const U8* pSrc = bitmap->getAddress(0, i);
      U8* pDst = pMSUpsideDownBits + (height - i - 1) * width * bytesPP;

      dMemcpy(pDst, pSrc, width * bytesPP);
   }

   stream.write(bi.biSizeImage, pMSUpsideDownBits);
   delete [] pMSUpsideDownBits;

   return stream.getStatus() == Stream::Ok;
}
