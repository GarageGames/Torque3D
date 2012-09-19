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

#include "core/color.h"

#define MNG_NO_CMS
#define MNG_SUPPORT_READ
#define MNG_SUPPORT_WRITE
#define MNG_SUPPORT_DISPLAY
#define MNG_STORE_CHUNKS
#define MNG_ACCESS_CHUNKS

#include "lmng/libmng.h"



static bool sReadMNG(Stream &stream, GBitmap *bitmap);
static bool sWriteMNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

static struct _privateRegisterMNG
{
   _privateRegisterMNG()
   {
      GBitmap::Registration reg;

      reg.extensions.push_back( "jng" );
      reg.extensions.push_back( "mng" );

      reg.readFunc = sReadMNG;
      reg.writeFunc = sWriteMNG;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterMNG;


typedef struct 
{
   GBitmap*    image;
	Stream*     stream;
} mngstuff;

static mng_ptr mngMallocFn(mng_size_t size)
{
   mng_ptr data = dMalloc(size);
   return dMemset(data, 0, size);
}

static void mngFreeFn(mng_ptr p, mng_size_t size)
{
   dFree(p);
}

static mng_bool mngOpenDataFn(mng_handle mng)
{
   return MNG_TRUE;
}

static mng_bool mngCloseDataFn(mng_handle mng)
{
   return MNG_TRUE;
}

static mng_bool mngReadDataFn(mng_handle mng, mng_ptr data, mng_uint32 length, mng_uint32 *bytesread)
{
   mngstuff *mymng = (mngstuff *)mng_get_userdata(mng);
   AssertFatal(mymng->stream != NULL, "No stream?");

   bool success = mymng->stream->read(length, data);
   *bytesread = length; // stupid hack
   
   AssertFatal(success, "MNG read catastrophic error!");
   if(success)
      return MNG_TRUE;
   else
      return MNG_FALSE;
}

#if 0
// CodeReview - until we can write these, get rid of warning by disabling method.
static mng_bool mngWriteDataFn(mng_handle mng, mng_ptr data, mng_uint32 length, mng_uint32 *iWritten)
{
   mngstuff *mymng = (mngstuff *)mng_get_userdata(mng);
   AssertFatal(mymng->stream != NULL, "No stream?");

   bool success = mymng->stream->write(length, data);
   *iWritten = length; // stupid hack
   
   AssertFatal(success, "MNG write catastrophic error!");
   if(success)
      return MNG_TRUE;
   else
      return MNG_FALSE;
}
#endif

static mng_bool mngProcessHeaderFn(mng_handle mng, mng_uint32 width, mng_uint32 height)
{
   mngstuff *mymng = (mngstuff *)mng_get_userdata(mng);

   GFXFormat format;
   mng_uint8 colorType = mng_get_colortype(mng);
   mng_uint8 alphaDepth = mng_get_alphadepth(mng);
   switch(colorType)
   {
      case MNG_COLORTYPE_GRAY:
      case MNG_COLORTYPE_JPEGGRAY:
         format = GFXFormatR8G8B8;
         mng_set_canvasstyle(mng, MNG_CANVAS_RGB8);
         break;

      case MNG_COLORTYPE_INDEXED:
         if(alphaDepth >= 1)
         {
            format = GFXFormatR8G8B8A8;
            mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8);
         }
         else
         {
            format = GFXFormatR8G8B8;
            mng_set_canvasstyle(mng, MNG_CANVAS_RGB8);
         }

      case MNG_COLORTYPE_RGB:
      case MNG_COLORTYPE_JPEGCOLOR:
         if(alphaDepth >= 1)
         {
            format = GFXFormatR8G8B8A8;
            mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8);
         }
         else
         {
            format = GFXFormatR8G8B8;
            mng_set_canvasstyle(mng, MNG_CANVAS_RGB8);
         }
         break;

      case MNG_COLORTYPE_RGBA:
      case MNG_COLORTYPE_JPEGCOLORA:
         format = GFXFormatR8G8B8A8;
         mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8);
         break;

      default:
         // This case should never get hit, however it resolves a compiler
         // warning
         format = GFXFormat_FIRST;
         AssertISV( false, "Unknown color format in bitmap MNG Loading" );
   }

   mymng->image->allocateBitmap(width, height, false, format);
   return MNG_TRUE;
}

static mng_ptr mngCanvasLineFn(mng_handle mng, mng_uint32 line)
{
   mngstuff *mymng = (mngstuff *)mng_get_userdata(mng);
   return (mng_ptr) mymng->image->getAddress(0, line);
}

static mng_bool mngRefreshFn(mng_handle mng, mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 h)
{
	return MNG_TRUE;
}

static mng_uint32 mngGetTicksFn(mng_handle mng)
{
	return 0;
}

static mng_bool mngSetTimerFn(mng_handle mng, mng_uint32 msecs)
{
	return MNG_TRUE;
}

static mng_bool mngFatalErrorFn(mng_handle mng, mng_int32 code, mng_int8 severity, mng_chunkid chunktype, mng_uint32 chunkseq, mng_int32 extra1, mng_int32 extra2, mng_pchar text)
{
   mng_cleanup(&mng);
   
   AssertISV(false, avar("Error reading MNG file:\n %s", (const char*)text));
   return MNG_FALSE;
}

static bool sReadMNG(Stream &stream, GBitmap *bitmap)
{
   mngstuff mnginfo;
   dMemset(&mnginfo, 0, sizeof(mngstuff));

   mng_handle mng = mng_initialize(&mnginfo, mngMallocFn, mngFreeFn, MNG_NULL);
   if(mng == NULL)
      return false;
   
   // setup the callbacks
   mng_setcb_errorproc(mng, mngFatalErrorFn);
   mng_setcb_openstream(mng, mngOpenDataFn);
   mng_setcb_closestream(mng, mngCloseDataFn);
   mng_setcb_readdata(mng, mngReadDataFn);
   mng_setcb_processheader(mng, mngProcessHeaderFn);
   mng_setcb_getcanvasline(mng, mngCanvasLineFn);
   mng_setcb_refresh(mng, mngRefreshFn);
   mng_setcb_gettickcount(mng, mngGetTicksFn);
   mng_setcb_settimer(mng, mngSetTimerFn);
   
   mnginfo.image = bitmap;
   mnginfo.stream = &stream;
   
   mng_read(mng);
   mng_display(mng);

   // hacks :(
   // libmng doesn't support returning data in gray/gray alpha format, 
   // so we grab as RGB/RGBA and just cut off the g and b
   mng_uint8 colorType = mng_get_colortype(mng);
   switch(colorType)
   {
      case MNG_COLORTYPE_GRAY:
      case MNG_COLORTYPE_JPEGGRAY:
         {
            GBitmap temp(*bitmap);
            bitmap->deleteImage();
            bitmap->allocateBitmap(temp.getWidth(), temp.getHeight(), false, GFXFormatA8);
            
            // force getColor to read in in the same color value for each channel
            // since the gray colortype has the real alpha in the first channel
            temp.setFormat( GFXFormatA8 );

            ColorI color;
            for(U32 row = 0; row < bitmap->getHeight(); row++)
            {
               for(U32 col = 0; col < bitmap->getWidth(); col++)
               {
                  temp.getColor(col, row, color);
                  bitmap->setColor(col, row, color);
               }
            }
         }

         break;
   }

   mng_cleanup(&mng);

   // Check this bitmap for transparency
   bitmap->checkForTransparency();

   return true;
}

static bool sWriteMNG(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   TORQUE_UNUSED( bitmap );
   TORQUE_UNUSED( stream );
   TORQUE_UNUSED( compressionLevel );

   return false;
#if 0
   // ONLY RGB bitmap writing supported at this time!
   AssertFatal(getFormat() == GFXFormatR8G8B8 || getFormat() == GFXFormatR8G8B8A8 || getFormat() == GFXFormatA8, "GBitmap::writeMNG: ONLY RGB bitmap writing supported at this time.");
   if(getFormat() != GFXFormatR8G8B8 && getFormat() != GFXFormatR8G8B8A8 && getFormat() != GFXFormatA8)
      return (false);
      
   // maximum image size allowed
   #define MAX_HEIGHT 4096
   if(getHeight() >= MAX_HEIGHT)
      return false;
      
   mngstuff mnginfo;
   dMemset(&mnginfo, 0, sizeof(mngstuff));
   mng_handle mng = mng_initialize(&mnginfo, mngMallocFn, mngFreeFn, MNG_NULL);
   if(mng == NULL) {
      return false;
   }
   
   // setup the callbacks
   mng_setcb_openstream(mng, mngOpenDataFn);
   mng_setcb_closestream(mng, mngCloseDataFn);
   mng_setcb_writedata(mng, mngWriteDataFn);
   
   // create the file in memory
   mng_create(mng);
   
   mng_putchunk_defi(mng, 0, 0, 0, MNG_FALSE, 0, 0, MNG_FALSE, 0, getWidth(), 0, getHeight());
   
   mnginfo.image = (GBitmap*)this;
   mnginfo.stream = &stream;
   
   switch(getFormat()) {
      case GFXFormatA8:
         mng_putchunk_ihdr(mng, getWidth(), getHeight(), 
            MNG_BITDEPTH_8, 
            MNG_COLORTYPE_GRAY, 
            MNG_COMPRESSION_DEFLATE, 
            MNG_FILTER_ADAPTIVE, 
            MNG_INTERLACE_NONE);
         
         // not implemented in lib yet
         //mng_putimgdata_ihdr(mng, getWidth(), getHeight(), 
         //   MNG_COLORTYPE_GRAY, 
         //   MNG_BITDEPTH_8, 
         //   MNG_COMPRESSION_DEFLATE, 
         //   MNG_FILTER_ADAPTIVE, 
         //   MNG_INTERLACE_NONE, 
         //   MNG_CANVAS_GRAY8, mngCanvasLineFn);
         break;
     case GFXFormatR8G8B8:
         mng_putchunk_ihdr(mng, getWidth(), getHeight(), 
            MNG_BITDEPTH_8, 
            MNG_COLORTYPE_RGB, 
            MNG_COMPRESSION_DEFLATE, 
            MNG_FILTER_ADAPTIVE, 
            MNG_INTERLACE_NONE);
            
         // not implemented in lib yet
         //mng_putimgdata_ihdr(mng, getWidth(), getHeight(), 
         //   MNG_COLORTYPE_RGB, 
         //   MNG_BITDEPTH_8, 
         //   MNG_COMPRESSION_DEFLATE, 
         //   MNG_FILTER_ADAPTIVE, 
         //   MNG_INTERLACE_NONE, 
         //   MNG_CANVAS_RGB8, mngCanvasLineFn);
         break;
     case GFXFormatR8G8B8A8:
         mng_putchunk_ihdr(mng, getWidth(), getHeight(), 
            MNG_BITDEPTH_8, 
            MNG_COLORTYPE_RGBA, 
            MNG_COMPRESSION_DEFLATE, 
            MNG_FILTER_ADAPTIVE, 
            MNG_INTERLACE_NONE);
            
         // not implemented in lib yet
         //mng_putimgdata_ihdr(mng, getWidth(), getHeight(), 
         //   MNG_COLORTYPE_RGBA, 
         //   MNG_BITDEPTH_8, 
         //   MNG_COMPRESSION_DEFLATE, 
         //   MNG_FILTER_ADAPTIVE, 
         //   MNG_INTERLACE_NONE, 
         //   MNG_CANVAS_RGBA8, mngCanvasLineFn);
         break;
   }
   
   
   // below is a hack until libmng is mature enough to handle this itself
   //-----------------------------------------------------------------------------
   
  
   U8 *tmpbuffer = new U8[this->byteSize + getHeight()];
	if(tmpbuffer == 0)
	{
	   mng_cleanup(&mng);
	   return false;
	}
	
	// transfer data, add filterbyte
   U32 effwdt = getWidth() * this->bytesPerPixel;
   for(U32 Row = 0; Row < getHeight(); Row++)
   {
      // first Byte in each scanline is filterbyte: currently 0 -> no filter
      tmpbuffer[Row * (effwdt + 1)] = 0; 
      
      // copy the scanline
      dMemcpy(tmpbuffer + Row * (effwdt + 1) + 1, getAddress(0, Row), effwdt);
   } 
   
   // compress data with zlib
   U8 *dstbuffer = new U8[this->byteSize + getHeight()];
   if(dstbuffer == 0)
   {
      delete [] tmpbuffer;
      mng_cleanup(&mng);
      return false;
   }

   U32 dstbufferSize = this->byteSize + getHeight();
   if(Z_OK != compress2((Bytef*)dstbuffer,(uLongf*)&dstbufferSize, (const Bytef*)tmpbuffer, dstbufferSize, 9))
   {
      delete [] tmpbuffer;
      delete [] dstbuffer;
      mng_cleanup(&mng);
      return false;
   }
   
   mng_putchunk_idat(mng, dstbufferSize, (mng_ptr*)dstbuffer);
   
   
   //-----------------------------------------------------------------------------
   
   
   mng_putchunk_iend(mng);
   
   delete [] tmpbuffer;
   delete [] dstbuffer;
   
   mng_write(mng);
   mng_cleanup(&mng);
   
   return true;
#endif
}
