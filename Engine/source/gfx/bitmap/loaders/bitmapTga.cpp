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


static bool sReadTGA(Stream &stream, GBitmap *bitmap);
static bool sWriteTGA(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

static struct _privateRegisterTGA
{
   _privateRegisterTGA()
   {
      GBitmap::Registration reg;

      reg.extensions.push_back( "tga" );

      reg.readFunc = sReadTGA;
      reg.writeFunc = sWriteTGA;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterTGA;


//------------------------------------------------------------------------------
//-------------------------------------- Supplementary I/O
//

enum eImageType
{
   TypeNoData        = 0,
   TypeUncPaletted   = 1,
   TypeUncTruecolor  = 2,
   TypeUncGrayscale  = 3,
   TypeRlePaletted   = 9,
   TypeRleTruecolor  = 10,
   TypeRleGrayscale  = 11
};

enum ePixelMap
{
   MapLowerLeft      = 0,
   MapLowerRight     = 1,
   MapUpperLeft      = 2,
   MapUpperRight     = 3,
};

static void tga_write_pixel_to_mem( U8 * dat, U8 img_spec, U32 number, 
                                   U32 w, U32 h, U32 pixel, U32 bppOut )
{
   // write the pixel to the data regarding how the
   // header says the data is ordered.

   U32 x, y;

   switch( (img_spec & 0x30) >> 4 )
   {
   case MapLowerRight:
      x = w - 1 - (number % w);
      y = h - 1 - (number / w);
      break;

   case MapUpperLeft:
      x = number % w;
      y = number / w;
      break;

   case MapUpperRight:
      x = w - 1 - (number % w);
      y = number / w;
      break;

   case MapLowerLeft:
   default:
      x = number % w;
      y = h - 1 - (number / w);
      break;

   }

   U32 addy = (y * w + x) * bppOut;
   for ( U32 j = 0; j < bppOut; j++ )
      dat[addy + j] = (U8)((pixel >> (j * 8)) & 0xFF);
}

static U32 tga_get_pixel( Stream& stream, U8 bppIn, 
                            U8 * colormap, U8 cmapBytesEntry )
{
   /* get the image data value out */

   U32 tmp_int32 = 0;

   for ( U32 j = 0; j < bppIn; j++ )
   {
      U8 tmp_byte;
      if ( !stream.read( &tmp_byte ) )
         tmp_int32 = 0;
      else
         tmp_int32 += tmp_byte << (j * 8);
   }

   /* byte-order correct the thing */
   switch( bppIn )
   {
   case 2:
      tmp_int32 = convertLEndianToHost( (U16)tmp_int32 );
      break;

   case 3: /* intentional fall-thru */
   case 4:
      tmp_int32 = convertLEndianToHost( tmp_int32 );
      break;
   }

   U32 tmp_col;

   if ( colormap )
   {
      /* need to look up value to get real color */
      tmp_col = 0;
      for ( U32 j = 0; j < cmapBytesEntry; j++ )
         tmp_col += colormap[cmapBytesEntry * tmp_int32 + j] << (8 * j);
   }
   else
   {
      tmp_col = tmp_int32;
   }

   return tmp_col;
}

static U32 tga_convert_color( U32 pixel, U32 bppIn, U8 alphabits, U32 bppOut )
{
   // this is not only responsible for converting from different depths
   // to other depths, it also switches BGR to RGB.

   // this thing will also premultiply alpha, on a pixel by pixel basis.

   U8 r, g, b, a;

   switch( bppIn )
   {
   case 32:
      if ( alphabits == 0 )
         goto is_24_bit_in_disguise;
      // 32-bit to 32-bit -- nop.
      break;

   case 24:
is_24_bit_in_disguise:
      // 24-bit to 32-bit; (only force alpha to full)
      pixel |= 0xFF000000;
      break;

   case 15:
is_15_bit_in_disguise:
      r = (U8)(((F32)((pixel & 0x7C00) >> 10)) * 8.2258f);
      g = (U8)(((F32)((pixel & 0x03E0) >> 5 )) * 8.2258f);
      b = (U8)(((F32)(pixel & 0x001F)) * 8.2258f);
      // 15-bit to 32-bit; (force alpha to full)
      pixel = 0xFF000000 + (r << 16) + (g << 8) + b;
      break;

   case 16:
      if ( alphabits == 1 )
         goto is_15_bit_in_disguise;

      // 16-bit to 32-bit; (force alpha to full)
      r = (U8)(((F32)((pixel & 0xF800) >> 11)) * 8.2258f);
      g = (U8)(((F32)((pixel & 0x07E0) >> 5 )) * 4.0476f);
      b = (U8)(((F32)(pixel & 0x001F)) * 8.2258f);
      pixel = 0xFF000000 + (r << 16) + (g << 8) + b;
      break;
   }

   // convert the 32-bit pixel from BGR to RGB.
   pixel = (pixel & 0xFF00FF00) + ((pixel & 0xFF) << 16) + ((pixel & 0xFF0000) >> 16);

   r = pixel & 0x000000FF;
   g = (pixel & 0x0000FF00) >> 8;
   b = (pixel & 0x00FF0000) >> 16;
   a = (pixel & 0xFF000000) >> 24;

   // not premultiplied alpha -- multiply.
   r = (U8)(((F32)r / 255.0f) * ((F32)a / 255.0f) * 255.0f);
   g = (U8)(((F32)g / 255.0f) * ((F32)a / 255.0f) * 255.0f);
   b = (U8)(((F32)b / 255.0f) * ((F32)a / 255.0f) * 255.0f);

   pixel = r + (g << 8) + (b << 16) + (a << 24);

   /* now convert from 32-bit to whatever they want. */
   switch( bppOut )
   {
   case 4:
      // 32 to 32 -- nop.
      break;

   case 3:
      // 32 to 24 -- discard alpha.
      pixel &= 0x00FFFFFF;
      break;
   }

   return pixel;
}

static bool sReadTGA(Stream &stream, GBitmap *bitmap)
{
   struct Header
   {
      U8    idLength;         // length of the image_id string below.
      U8    cmapType;         // paletted image <=> cmapType
      U8    imageType;        // can be any of the IMG_TYPE constants above.
      U16   cmapFirst;        // 
      U16   cmapLength;       // how long the colormap is
      U8    cmapEntrySize;    // how big a palette entry is.
      U16   xOrigin;          // the x origin of the image in the image data.
      U16   yOrigin;          // the y origin of the image in the image data.
      U16   width;            // the width of the image.
      U16   height;           // the height of the image.
      U8    pixelDepth;       // the depth of a pixel in the image.
      U8    imageDesc;        // the image descriptor.
   };

   // Read header
   Header header;
   stream.read( &header.idLength );
   stream.read( &header.cmapType );
   stream.read( &header.imageType );
   stream.read( &header.cmapFirst );
   stream.read( &header.cmapLength );
   stream.read( &header.cmapEntrySize );
   stream.read( &header.xOrigin );
   stream.read( &header.yOrigin );
   stream.read( &header.width );
   stream.read( &header.height );
   stream.read( &header.pixelDepth );
   stream.read( &header.imageDesc );

   U32 numPixels = header.width * header.height;
   if ( numPixels == 0 )
   {
      //Con::errorf( "Texture has width and/or height set to 0" );
      return false;
   }

   U8 alphabits = header.imageDesc & 0x0F;

   /* seek past the image id, if there is one */
   if ( header.idLength )
   {
      if ( !stream.setPosition( stream.getPosition() + header.idLength ) )
      {
         //Con::errorf( "Unexpected end of stream encountered" );
         return false;
      }
   }

   /* if this is a 'nodata' image, just jump out. */
   if ( header.imageType == TypeNoData )
   {
      //Con::errorf( "Texture contains no data" );
      return false;
   }

   /* deal with the colormap, if there is one. */
   U8* colormap = NULL;
   U32 cmapBytes = 0;
   U8 cmapBytesEntry = 0;

   if ( header.cmapType )
   {
      switch( header.imageType )
      {
      case TypeUncPaletted:
      case TypeRlePaletted:
         break;

      case TypeUncTruecolor:
      case TypeRleTruecolor:
         // this should really be an error, but some really old
         // crusty targas might actually be like this (created by TrueVision, no less!)
         // so, we'll hack our way through it.
         break;

      case TypeUncGrayscale:
      case TypeRleGrayscale:
         //Con::errorf( "Found colormap for a grayscale image" );
         return false;
      }

      /* ensure colormap entry size is something we support */
      if ( !(header.cmapEntrySize == 15 || 
          header.cmapEntrySize == 16 ||
          header.cmapEntrySize == 24 ||
          header.cmapEntrySize == 32) )
      {
         //Con::errorf( "Unsupported colormap entry size" );
         return false;
      }

      /* allocate memory for a colormap */
      if ( header.cmapEntrySize & 0x07 )
         cmapBytesEntry = (((8 - (header.cmapEntrySize & 0x07)) + header.cmapEntrySize) >> 3);
      else
         cmapBytesEntry = (header.cmapEntrySize >> 3);

      cmapBytes = cmapBytesEntry * header.cmapLength;
      colormap = new U8[ cmapBytes ];

      for ( U32 i = 0; i < header.cmapLength; i++ )
      {
         /* seek ahead to first entry used */
         if ( header.cmapFirst != 0 )
            stream.setPosition( stream.getPosition() + header.cmapFirst * cmapBytesEntry );

         U32 tmp_int32 = 0;
         for ( U32 j = 0; j < cmapBytesEntry; j++ )
         {
            U8 tmp_byte;
            if ( !stream.read( &tmp_byte ) )
            {
               delete [] colormap;
               //Con::errorf( "Bad colormap" );
               return false;
            }
            tmp_int32 += tmp_byte << (j * 8);
         }

         // byte order correct.
         tmp_int32 = convertLEndianToHost( tmp_int32 );

         for ( U32 j = 0; j < cmapBytesEntry; j++ )
            colormap[i * cmapBytesEntry + j] = (tmp_int32 >> (8 * j)) & 0xFF;
      }
   }

   // compute number of bytes in an image data unit (either index or BGR triple)
   U8 inBytesPerPixel = 0;
   if ( header.pixelDepth & 0x07 )
      inBytesPerPixel = (((8 - (header.pixelDepth & 0x07)) + header.pixelDepth) >> 3);
   else
      inBytesPerPixel = (header.pixelDepth >> 3);

   /* assume that there's one byte per pixel */
   if ( inBytesPerPixel == 0 )
      inBytesPerPixel = 1;

   GFXFormat gfxFmt;
   U32 outBytesPerPixel;
   switch ( header.pixelDepth )
   {
   case 32:
      gfxFmt = GFXFormatR8G8B8A8;
      outBytesPerPixel = 4;
      break;

   case 24:
   default:
      gfxFmt = GFXFormatR8G8B8;
      outBytesPerPixel = 3;
      break;
   }

   bitmap->allocateBitmap( header.width, header.height, false, gfxFmt );

   // compute the true number of bits per pixel
   U8 trueBitsPerPixel = header.cmapType ? header.cmapEntrySize : header.pixelDepth;

   // Override the number of alpha bits if necessary
   // Some apps generate transparent TGAs with alphabits set to 0 in the image descriptor
   if ( ( trueBitsPerPixel == 32 ) && ( alphabits == 0 ) )
      alphabits = 8;

   switch( header.imageType )
   {
   case TypeUncTruecolor:
   case TypeUncGrayscale:
   case TypeUncPaletted:

      /* FIXME: support grayscale */

      for ( U32 i = 0; i < numPixels; i++ )
      {
         // get the color value.
         U32 tmp_col = tga_get_pixel( stream, inBytesPerPixel, colormap, cmapBytesEntry );
         tmp_col = tga_convert_color( tmp_col, trueBitsPerPixel, alphabits, outBytesPerPixel );

         // now write the data out.
         tga_write_pixel_to_mem( bitmap->getAddress( 0, 0 ), header.imageDesc, 
            i, header.width, header.height, tmp_col, outBytesPerPixel );
      }
      break;

   case TypeRleTruecolor:
   case TypeRleGrayscale:
   case TypeRlePaletted:

      // FIXME: handle grayscale..

      for ( U32 i = 0; i < numPixels; )
      {
         /* a bit of work to do to read the data.. */
         U8 packet_header;
         if ( !stream.read( 1, &packet_header ) )
         {
            // well, just let them fill the rest with null pixels then...
            packet_header = 1;
         }

         if ( packet_header & 0x80 )
         {
            /* run length packet */
            U32 tmp_col = tga_get_pixel( stream, inBytesPerPixel, colormap, cmapBytesEntry );
            tmp_col = tga_convert_color( tmp_col, trueBitsPerPixel, alphabits, outBytesPerPixel );

            U8 repcount = (packet_header & 0x7F) + 1;

            /* write all the data out */
            for ( U32 j = 0; j < repcount; j++ )
            {
               tga_write_pixel_to_mem( bitmap->getAddress( 0, 0 ), header.imageDesc, 
                   i + j, header.width, header.height, tmp_col, outBytesPerPixel );
            }

            i += repcount;

         }
         else
         {
            /* raw packet */
            /* get pixel from file */
            U8 repcount = (packet_header & 0x7F) + 1;

            for ( U32 j = 0; j < repcount; j++ )
            {
               U32 tmp_col = tga_get_pixel( stream, inBytesPerPixel, colormap, cmapBytesEntry );
               tmp_col = tga_convert_color( tmp_col, trueBitsPerPixel, alphabits, outBytesPerPixel );

               tga_write_pixel_to_mem( bitmap->getAddress( 0, 0 ), header.imageDesc, 
                   i + j, header.width, header.height, tmp_col, outBytesPerPixel );
            }

            i += repcount;
         }
      }
      break;

   default:
       //Con::errorf( "Unknown image type" );
      return false;
   }

   delete [] colormap;

   // 32-bit tgas have an alpha channel
   bitmap->setHasTransparency( header.pixelDepth == 32 );

   return true;
}

static bool sWriteTGA(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   AssertISV(false, "GBitmap::writeTGA - doesn't support writing tga files!");

   return false;
}
