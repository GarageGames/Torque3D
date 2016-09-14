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

#include "platform/platform.h"

#include "core/stream/fileStream.h"
#include "core/stream/memStream.h"
#include "core/strings/stringFunctions.h"

#include "gfx/bitmap/gBitmap.h"
#include "gfx/bitmap/pngUtils.h"

#define PNG_INTERNAL 1
#include <time.h>
#include "lpng/png.h"
#include "zlib/zlib.h"

#ifdef NULL
#undef NULL
#define NULL 0
#endif


static bool sReadPNG(Stream &stream, GBitmap *bitmap);

/// Compression levels for PNGs range from 0-9.
/// A value outside that range will cause the write routine to look for the best compression for a given PNG.  This can be slow.
static bool sWritePNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel);
static bool _writePNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel, U32 strategy, U32 filter);

static struct _privateRegisterPNG
{
   _privateRegisterPNG()
   {
      GBitmap::Registration reg;

      reg.priority = 100;
      reg.extensions.push_back( "png" );

      reg.readFunc = sReadPNG;
      reg.writeFunc = sWritePNG;
      reg.defaultCompression = 6;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterPNG;


//-------------------------------------- Replacement I/O for standard LIBPng
//                                        functions.  we don't wanna use
//                                        FILE*'s...
static void pngReadDataFn(png_structp png_ptr,
                    png_bytep   data,
                    png_size_t  length)
{
   AssertFatal(png_get_io_ptr(png_ptr) != NULL, "No stream?");

   Stream *strm = (Stream*)png_get_io_ptr(png_ptr);
   bool success = strm->read(length, data);
   AssertFatal(success, "pngReadDataFn - failed to read from stream!");
}


//--------------------------------------
static void pngWriteDataFn(png_structp png_ptr,
                     png_bytep   data,
                     png_size_t  length)
{
   AssertFatal(png_get_io_ptr(png_ptr) != NULL, "No stream?");

   Stream *strm = (Stream*)png_get_io_ptr(png_ptr);
   bool success = strm->write(length, data);
   AssertFatal(success, "pngWriteDataFn - failed to write to stream!");
}


//--------------------------------------
static void pngFlushDataFn(png_structp /*png_ptr*/)
{
   //
}

static png_voidp pngMallocFn(png_structp /*png_ptr*/, png_size_t size)
{
   return FrameAllocator::alloc(size);
}

static void pngFreeFn(png_structp /*png_ptr*/, png_voidp /*mem*/)
{
}

static png_voidp pngRealMallocFn(png_structp /*png_ptr*/, png_size_t size)
{
   return (png_voidp)dMalloc(size);
}

static void pngRealFreeFn(png_structp /*png_ptr*/, png_voidp mem)
{
   dFree(mem);
}

//--------------------------------------
static void pngFatalErrorFn(png_structp     /*png_ptr*/,
                     png_const_charp pMessage)
{
   AssertISV(false, avar("Error reading PNG file:\n %s", pMessage));
}


//--------------------------------------
static void pngWarningFn(png_structp, png_const_charp /*pMessage*/)
{
   //   AssertWarn(false, avar("Warning reading PNG file:\n %s", pMessage));
}


//--------------------------------------
static bool sReadPNG(Stream &stream, GBitmap *bitmap)
{
   static const U32 cs_headerBytesChecked = 8;

   U8 header[cs_headerBytesChecked];
   stream.read(cs_headerBytesChecked, header);

   bool isPng = png_check_sig(header, cs_headerBytesChecked) != 0;
   if (isPng == false) 
   {
      AssertWarn(false, "GBitmap::readPNG: stream doesn't contain a PNG");
      return false;
   }

   U32 prevWaterMark = FrameAllocator::getWaterMark();
   png_structp png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
      NULL,
      pngFatalErrorFn,
      pngWarningFn,
      NULL,
      pngRealMallocFn,
      pngRealFreeFn);

   if (png_ptr == NULL) 
   {
      FrameAllocator::setWaterMark(prevWaterMark);
      return false;
   }

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) 
   {
      png_destroy_read_struct(&png_ptr,
         (png_infopp)NULL,
         (png_infopp)NULL);

      FrameAllocator::setWaterMark(prevWaterMark);
      return false;
   }

   png_infop end_info = png_create_info_struct(png_ptr);
   if (end_info == NULL) 
   {
      png_destroy_read_struct(&png_ptr,
         &info_ptr,
         (png_infopp)NULL);

      FrameAllocator::setWaterMark(prevWaterMark);
      return false;
   }

   png_set_read_fn(png_ptr, &stream, pngReadDataFn);

   // Read off the info on the image.
   png_set_sig_bytes(png_ptr, cs_headerBytesChecked);
   png_read_info(png_ptr, info_ptr);

   // OK, at this point, if we have reached it ok, then we can reset the
   //  image to accept the new data...
   //
   bitmap->deleteImage();

   png_uint_32 width;
   png_uint_32 height;
   S32 bit_depth;
   S32 color_type;

   png_get_IHDR(png_ptr, info_ptr,
      &width, &height,             // obv.
      &bit_depth, &color_type,     // obv.
      NULL,                        // interlace
      NULL,                        // compression_type
      NULL);                       // filter_type

   // First, handle the color transformations.  We need this to read in the
   //  data as RGB or RGBA, _always_, with a maximal channel width of 8 bits.
   //
   bool transAlpha     = false;
   GFXFormat format = GFXFormatR8G8B8;

   // Strip off any 16 bit info
   //
   if (bit_depth == 16 && color_type != PNG_COLOR_TYPE_GRAY) 
   {
      png_set_strip_16(png_ptr);
   }

   // Expand a transparency channel into a full alpha channel...
   //
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) 
   {
      png_set_expand(png_ptr);
      transAlpha = true;
   }

   if (color_type == PNG_COLOR_TYPE_PALETTE) 
   {
      png_set_expand(png_ptr);
      format = transAlpha ? GFXFormatR8G8B8A8 : GFXFormatR8G8B8;
   }
   else if (color_type == PNG_COLOR_TYPE_GRAY) 
   {
      png_set_expand(png_ptr);

      if (bit_depth == 16)
         format = GFXFormatR5G6B5;
      else
         format = GFXFormatA8;
   }
   else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
   {
      png_set_expand(png_ptr);
      png_set_gray_to_rgb(png_ptr);
      format = GFXFormatR8G8B8A8;
   }
   else if (color_type == PNG_COLOR_TYPE_RGB) 
   {
      format = transAlpha ? GFXFormatR8G8B8A8 : GFXFormatR8G8B8;
      png_set_expand(png_ptr);
   }
   else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) 
   {
      png_set_expand(png_ptr);
      format = GFXFormatR8G8B8A8;
   }

   // Update the info pointer with the result of the transformations
   //  above...
   png_read_update_info(png_ptr, info_ptr);

   png_uint_32 rowBytes = png_get_rowbytes(png_ptr, info_ptr);
   if (format == GFXFormatR8G8B8) 
   {
      AssertFatal(rowBytes == width * 3,
         "Error, our rowbytes are incorrect for this transform... (3)");
   }
   else if (format == GFXFormatR8G8B8A8) 
   {
      AssertFatal(rowBytes == width * 4,
         "Error, our rowbytes are incorrect for this transform... (4)");
   }
   else if (format == GFXFormatR5G6B5) 
   {
      AssertFatal(rowBytes == width * 2,
         "Error, our rowbytes are incorrect for this transform... (2)");
   }

   // actually allocate the bitmap space...
   bitmap->allocateBitmap(width, height,
      false,            // don't extrude miplevels...
      format);          // use determined format...

   // Set up the row pointers...
   png_bytep* rowPointers = new png_bytep[ height ];
   U8* pBase = (U8*)bitmap->getBits();
   
   for (U32 i = 0; i < height; i++)
      rowPointers[i] = pBase + (i * rowBytes);

   // And actually read the image!
   png_read_image(png_ptr, rowPointers);

   // We're outta here, destroy the png structs, and release the lock
   //  as quickly as possible...
   //png_read_end(png_ptr, end_info);
   delete [] rowPointers;
   png_read_end(png_ptr, NULL);
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

   // Ok, the image is read in, now we need to finish up the initialization,
   //  which means: setting up the detailing members, init'ing the palette
   //  key, etc...
   //
   // actually, all of that was handled by allocateBitmap, so we're outta here
   //

   // Check this bitmap for transparency
   bitmap->checkForTransparency();

   FrameAllocator::setWaterMark(prevWaterMark);

   return true;
}

//--------------------------------------------------------------------------
static bool _writePNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel, U32 strategy, U32 filter)
{
   GFXFormat   format = bitmap->getFormat();

   // ONLY RGB bitmap writing supported at this time!
   AssertFatal(   format == GFXFormatR8G8B8 || 
                  format == GFXFormatR8G8B8A8 || 
                  format == GFXFormatR8G8B8X8 || 
                  format == GFXFormatA8 ||
                  format == GFXFormatR5G6B5 ||
                  format == GFXFormatR8G8B8A8_LINEAR_FORCE, "_writePNG: ONLY RGB bitmap writing supported at this time.");

   if (  format != GFXFormatR8G8B8 && 
         format != GFXFormatR8G8B8A8 && 
         format != GFXFormatR8G8B8X8 && 
         format != GFXFormatA8 &&
         format != GFXFormatR5G6B5 && format != GFXFormatR8G8B8A8_LINEAR_FORCE)
      return false;

   png_structp png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
      NULL,
      pngFatalErrorFn,
      pngWarningFn,
      NULL,
      pngMallocFn,
      pngFreeFn);
   if (png_ptr == NULL)
      return (false);

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      return false;
   }

   png_set_write_fn(png_ptr, &stream, pngWriteDataFn, pngFlushDataFn);

   // Set the compression level and image filters
   png_set_compression_window_bits(png_ptr, 15);
   png_set_compression_level(png_ptr, compressionLevel);
   png_set_filter(png_ptr, 0, filter);

   // Set the image information here.  Width and height are up to 2^31,
   // bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
   // the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
   // PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
   // or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
   // PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
   // currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED

   U32   width = bitmap->getWidth();
   U32   height = bitmap->getHeight();

   if (format == GFXFormatR8G8B8)
   {
      png_set_IHDR(png_ptr, info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_RGB,       // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatR8G8B8A8 || format == GFXFormatR8G8B8X8 || format == GFXFormatR8G8B8A8_LINEAR_FORCE)
   {
      png_set_IHDR(png_ptr, info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_RGB_ALPHA, // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatA8)
   {
      png_set_IHDR(png_ptr, info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_GRAY,      // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatR5G6B5) 
   {
      png_set_IHDR(png_ptr, info_ptr,
         width, height,               // the width & height
         16, PNG_COLOR_TYPE_GRAY,     // bit_depth, color_type,
         PNG_INTERLACE_NONE,          // no interlace
         PNG_COMPRESSION_TYPE_DEFAULT,   // compression type
         PNG_FILTER_TYPE_DEFAULT);       // filter type
      
      png_color_8_struct sigBit = { 0 };
      sigBit.gray = 16;
      png_set_sBIT(png_ptr, info_ptr, &sigBit );

      png_set_swap( png_ptr );
   }

   png_write_info(png_ptr, info_ptr);
   FrameAllocatorMarker marker;
   png_bytep* row_pointers = (png_bytep*)marker.alloc( height * sizeof( png_bytep ) );
   for (U32 i=0; i<height; i++)
      row_pointers[i] = const_cast<png_bytep>(bitmap->getAddress(0, i));

   png_write_image(png_ptr, row_pointers);

   // Write S3TC data if present...
   // Write FXT1 data if present...

   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

   return true;
}


//--------------------------------------------------------------------------
static bool sWritePNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   U32 waterMark = FrameAllocator::getWaterMark();

   if ( compressionLevel < 10 )
   {
      bool retVal = _writePNG(bitmap, stream, compressionLevel, 0, PNG_ALL_FILTERS);
      FrameAllocator::setWaterMark(waterMark);
      return retVal;
   }

   // check all our methods of compression to find the best one and use it
   U8* buffer = new U8[1 << 22]; // 4 Megs.  Should be enough...
   MemStream* pMemStream = new MemStream(1 << 22, buffer, false, true);

   const U32 zStrategies[] = { Z_DEFAULT_STRATEGY,
      Z_FILTERED };
   const U32 pngFilters[]  = { PNG_FILTER_NONE,
      PNG_FILTER_SUB,
      PNG_FILTER_UP,
      PNG_FILTER_AVG,
      PNG_FILTER_PAETH,
      PNG_ALL_FILTERS };

   U32 minSize      = 0xFFFFFFFF;
   U32 bestStrategy = 0xFFFFFFFF;
   U32 bestFilter   = 0xFFFFFFFF;
   U32 bestCLevel   = 0xFFFFFFFF;

   for (U32 cl = 0; cl <=9; cl++) 
   {
      for (U32 zs = 0; zs < 2; zs++) 
      {
         for (U32 pf = 0; pf < 6; pf++) 
         {
            pMemStream->setPosition(0);

            U32 waterMarkInner = FrameAllocator::getWaterMark();

            if (_writePNG(bitmap, *pMemStream, cl, zStrategies[zs], pngFilters[pf]) == false)
               AssertFatal(false, "Handle this error!");

            FrameAllocator::setWaterMark(waterMarkInner);

            if (pMemStream->getPosition() < minSize) 
            {
               minSize = pMemStream->getPosition();
               bestStrategy = zs;
               bestFilter   = pf;
               bestCLevel   = cl;
            }
         }
      }
   }
   AssertFatal(minSize != 0xFFFFFFFF, "Error, no best found?");

   delete pMemStream;
   delete [] buffer;


   bool retVal = _writePNG(bitmap, stream,
      bestCLevel,
      zStrategies[bestStrategy],
      pngFilters[bestFilter]);
   FrameAllocator::setWaterMark(waterMark);

   return retVal;
}

//--------------------------------------------------------------------------
// Stores PNG stream data
struct DeferredPNGWriterData {
   png_structp png_ptr;
   png_infop info_ptr;
   U32 width;
   U32 height;   
};
DeferredPNGWriter::DeferredPNGWriter() : 
   mData( NULL ),
   mActive(false)
{
   mData = new DeferredPNGWriterData();
}
DeferredPNGWriter::~DeferredPNGWriter()
{
   delete mData;
}

bool DeferredPNGWriter::begin( GFXFormat format, S32 width, S32 height, Stream &stream, U32 compressionLevel )
{   
   // ONLY RGB bitmap writing supported at this time!
   AssertFatal(   format == GFXFormatR8G8B8 || 
                  format == GFXFormatR8G8B8A8 || 
                  format == GFXFormatR8G8B8X8 || 
                  format == GFXFormatA8 ||
                  format == GFXFormatR5G6B5, "_writePNG: ONLY RGB bitmap writing supported at this time.");

   if (  format != GFXFormatR8G8B8 && 
         format != GFXFormatR8G8B8A8 && 
         format != GFXFormatR8G8B8X8 && 
         format != GFXFormatA8 &&
         format != GFXFormatR5G6B5 )
      return false;

   mData->png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
      NULL,
      pngFatalErrorFn,
      pngWarningFn,
      NULL,
      pngRealMallocFn,
      pngRealFreeFn);
   if (mData->png_ptr == NULL)
      return (false);

   mData->info_ptr = png_create_info_struct(mData->png_ptr);
   if (mData->info_ptr == NULL)
   {
      png_destroy_write_struct(&mData->png_ptr, (png_infopp)NULL);
      return false;
   }

   png_set_write_fn(mData->png_ptr, &stream, pngWriteDataFn, pngFlushDataFn);

   // Set the compression level and image filters
   png_set_compression_window_bits(mData->png_ptr, 15);
   png_set_compression_level(mData->png_ptr, compressionLevel);
   png_set_filter(mData->png_ptr, 0, PNG_ALL_FILTERS);

   // Set the image information here.  Width and height are up to 2^31,
   // bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
   // the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
   // PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
   // or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
   // PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
   // currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
   
   if (format == GFXFormatR8G8B8)
   {
      png_set_IHDR(mData->png_ptr, mData->info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_RGB,       // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatR8G8B8A8 || format == GFXFormatR8G8B8X8)
   {
      png_set_IHDR(mData->png_ptr, mData->info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_RGB_ALPHA, // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatA8)
   {
      png_set_IHDR(mData->png_ptr, mData->info_ptr,
         width, height,               // the width & height
         8, PNG_COLOR_TYPE_GRAY,      // bit_depth, color_type,
         NULL,                        // no interlace
         NULL,                        // compression type
         NULL);                       // filter type
   }
   else if (format == GFXFormatR5G6B5) 
   {
      png_set_IHDR(mData->png_ptr, mData->info_ptr,
         width, height,               // the width & height
         16, PNG_COLOR_TYPE_GRAY,     // bit_depth, color_type,
         PNG_INTERLACE_NONE,          // no interlace
         PNG_COMPRESSION_TYPE_DEFAULT,   // compression type
         PNG_FILTER_TYPE_DEFAULT);       // filter type
      
      png_color_8_struct sigBit = { 0 };
      sigBit.gray = 16;
      png_set_sBIT(mData->png_ptr, mData->info_ptr, &sigBit );

      png_set_swap( mData->png_ptr );
   }

   png_write_info(mData->png_ptr, mData->info_ptr);
   
   mActive = true;

   return true;
}
void DeferredPNGWriter::append( GBitmap* bitmap, U32 rows)
{
   AssertFatal(mActive, "Cannot append to an inactive DeferredPNGWriter!");

   U32 height = getMin( bitmap->getHeight(), rows);

   FrameAllocatorMarker marker;
   png_bytep* row_pointers = (png_bytep*)marker.alloc( height * sizeof( png_bytep ) );
   for (U32 i=0; i<height; i++)
      row_pointers[i] = const_cast<png_bytep>(bitmap->getAddress(0, i));

   png_write_rows(mData->png_ptr, row_pointers, height);
}
void DeferredPNGWriter::end()
{
   AssertFatal(mActive, "Cannot end an inactive DeferredPNGWriter!");

   png_write_end(mData->png_ptr, mData->info_ptr);
   png_destroy_write_struct(&mData->png_ptr, (png_infopp)NULL);

   mActive = false;
}