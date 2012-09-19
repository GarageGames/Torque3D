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
#include "core/util/path.h"

#include "gfx/bitmap/gBitmap.h"

// This must come after our headers due to a conflicting definition of VoidPtr
#include "lungif/gif_lib.h"


using namespace Torque;

static bool sReadGIF(Stream &stream, GBitmap *bitmap);
static bool sWriteGIF(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

static struct _privateRegisterGIF
{
   _privateRegisterGIF()
   {
      GBitmap::Registration reg;

      reg.extensions.push_back( "gif" );

      reg.readFunc = sReadGIF;
      reg.writeFunc = sWriteGIF;

      GBitmap::sRegisterFormat( reg );
   }
} sStaticRegisterGIF;

//-------------------------------------- Replacement I/O for standard LIBjpeg
//                                        functions.  we don't wanna use
//                                        FILE*'s...
static int gifReadDataFn(GifFileType *gifinfo, GifByteType *data, int length)
{
   Stream *stream = (Stream*)gifinfo->UserData;
   AssertFatal(stream != NULL, "gifReadDataFn::No stream.");
   int pos = stream->getPosition();
   if (stream->read(length, data))
      return length;

   if (stream->getStatus() == Stream::EOS)
      return (stream->getPosition()-pos);
   else
      return 0;
}


//--------------------------------------
#if 0 
// CodeReview - until we can write these, get rid of warning by disabling method.
static int gifWriteDataFn(GifFileType *gifinfo, GifByteType *data, int length)
{
   Stream *stream = (Stream*)gifinfo->UserData;
   AssertFatal(stream != NULL, "gifWriteDataFn::No stream.");
   if (stream->write(length, data))
      return length;
   else
      return 0;
}
#endif

//--------------------------------------
static bool sReadGIF( Stream &stream, GBitmap *bitmap )
{
   GifFileType *gifinfo = DGifOpen( (void*)&stream, gifReadDataFn);
   if (!gifinfo)
      return false;

   GifRecordType recordType;
   do
   {
      if (DGifGetRecordType(gifinfo, &recordType) == GIF_ERROR)
         break;

      if (recordType == IMAGE_DESC_RECORD_TYPE)
      {
         if (DGifGetImageDesc(gifinfo) == GIF_ERROR)
            break;

         GFXFormat format = (gifinfo->SBackGroundColor == 0 ) ? GFXFormatR8G8B8 : GFXFormatR8G8B8A8;
         bitmap->allocateBitmap(gifinfo->SWidth, gifinfo->SHeight, false, format);

         // Assume no transparency until proven otherwise
         bitmap->setHasTransparency(false);

         U32 gwidth = gifinfo->Image.Width ? gifinfo->Image.Width : bitmap->getWidth();
         U32 gheight= gifinfo->Image.Height ? gifinfo->Image.Height : bitmap->getHeight();
         U32 gifSize = gwidth * gheight;
         U8  *data   = new U8[gifSize];

         if (DGifGetLine(gifinfo, data, gifSize) != GIF_ERROR)
         {
            // use the global or local color table ?
            GifColorType *color = NULL;
            if (gifinfo->Image.ColorMap)
               color = gifinfo->Image.ColorMap->Colors;
            else if (gifinfo->SColorMap)
               color = gifinfo->SColorMap->Colors;

            if (color)
            {
               U8 *dst = bitmap->getAddress(gifinfo->Image.Left, gifinfo->Image.Top);
               U8 *src = data;
               U32 right  = gifinfo->Image.Left + gwidth;
               U32 bottom = gifinfo->Image.Top + gheight;
               U32 next   = (bitmap->getWidth() - gwidth) * bitmap->getBytesPerPixel();

               if (format == GFXFormatR8G8B8A8)
               {
                  for (U32 y=gifinfo->Image.Top; y<bottom; y++)
                  {
                     for (U32 x=gifinfo->Image.Left; x<right; x++, src++)
                     {
                        if (*src == gifinfo->SBackGroundColor)
                        {
                           // this is a transparent pixel
                           dst[0] = 0;    // red
                           dst[1] = 0;    // green
                           dst[2] = 0;    // blue
                           dst[3] = 0;    // alpha

                           bitmap->setHasTransparency(true);
                        }
                        else
                        {
                           dst[0] = color[*src].Red;
                           dst[1] = color[*src].Green;
                           dst[2] = color[*src].Blue;
                           dst[3] = 0;    // alpha
                        }
                        dst += bitmap->getBytesPerPixel();
                     }
                     dst += next;
                  }
               }
               else
               {
                  for (U32 y=gifinfo->Image.Top; y<bottom; y++)
                  {
                     for (U32 x=gifinfo->Image.Left; x<right; x++, src++)
                     {
                        dst[0] = color[*src].Red;
                        dst[1] = color[*src].Green;
                        dst[2] = color[*src].Blue;
                        dst += bitmap->getBytesPerPixel();
                     }
                     dst += next;
                  }
               }
               delete [] data;
               DGifCloseFile(gifinfo);
               return true;
            }
         }
         // failure
         delete [] data;
         break;
      }
      else if (recordType == EXTENSION_RECORD_TYPE)
      {
         GifByteType *extension;
         S32 extCode;

         // Skip any extension blocks in file
         if (DGifGetExtension(gifinfo, &extCode, &extension) != GIF_ERROR)
         {
            while (extension != NULL)
            {
               if (DGifGetExtensionNext(gifinfo, &extension) == GIF_ERROR)
               {
                  return false;
               }
            }
         }
         else
         {
            return false;
         }
      }

      // There used to be a break right here. This caused the while condition to
      // never get processed, and so it never looped through all the records in
      // the GIF. I took a quick peek back at TGB and TGE histories and I am not
      // sure where this change got made, but I can't figure out why the loading
      // worked at all, ever, with that break in there. The only case I can think
      // of is if the first record in the GIF was the bitmap data.
      // [6/6/2007 Pat]

   }while (recordType != TERMINATE_RECORD_TYPE);


   DGifCloseFile(gifinfo);
   return true;
}


//--------------------------------------------------------------------------
static bool sWriteGIF(GBitmap *bitmap, Stream &stream, U32 compressionLevel)
{
   TORQUE_UNUSED( bitmap );
   TORQUE_UNUSED( stream );
   TORQUE_UNUSED( compressionLevel );

   return false;
}


