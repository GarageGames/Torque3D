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

#ifndef _GFXFORMATUTILS_H_
#define _GFXFORMATUTILS_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _GFXENUMS_H_
   #include "gfx/gfxEnums.h"
#endif
#ifndef _COLOR_H_
   #include "core/color.h"
#endif


//WIP: still in early stages



/// Some information about a GFXFormat.
struct GFXFormatInfo
{
   protected:

      struct Data
      {
         /// Bytes per single pixel.
         U32 mBytesPerPixel;
         
         /// If true, format has alpha channel.
         bool mHasAlpha;
         
         /// If true, format uses compression.
         bool mIsCompressed;
         
         /// If true, channels are in floating-point.
         bool mIsFloatingPoint;

         Data() {}
         Data( U32 bpp, bool hasAlpha = false, bool isCompressed = false, bool isFP = false )
            : mBytesPerPixel( bpp ),
              mHasAlpha( hasAlpha ),
              mIsCompressed( isCompressed ),
              mIsFloatingPoint( isFP ) {}
      };
      
      GFXFormat mFormat;
      
      static Data smFormatInfos[ GFXFormat_COUNT ];

   public:

      GFXFormatInfo( GFXFormat format )
         : mFormat( format ) {}

      /// @return the number of bytes per pixel in this format.
      /// @note For compressed formats that can't give a fixed value per pixel,
      ///   this will be zero.
      U32 getBytesPerPixel() const { return smFormatInfos[ mFormat ].mBytesPerPixel; }

      /// @return true if the format has an alpha channel.
      bool hasAlpha() const { return smFormatInfos[ mFormat ].mHasAlpha; }
      
      /// @return true if format uses compression.
      bool isCompressed() const { return smFormatInfos[ mFormat ].mIsCompressed; }

      /// @return true if channels are stored in floating-point format.
      bool isFloatingPoint() const { return smFormatInfos[ mFormat ].mIsFloatingPoint; }
};


#if 0
///
extern void GFXCopyPixels( GFXFormat fromFormat, U32 fromWidth, U32 fromHeight, U8* fromData,
                           GFXFormat toFormat, U32 toWidth, U32 toHeight, U8* toData );
#endif


inline void GFXPackPixel( GFXFormat format, U8*& ptr, U8 red, U8 green, U8 blue, U8 alpha, bool leastSignficantFirst = true )
{
   switch( format )
   {
      case GFXFormatR8G8B8A8:
         if( leastSignficantFirst )
         {
            ptr[ 0 ] = blue;
            ptr[ 1 ] = green;
            ptr[ 2 ] = red;
            ptr[ 3 ] = alpha;
         }
         else
         {
            ptr[ 0 ] = red;
            ptr[ 1 ] = green;
            ptr[ 2 ] = blue;
            ptr[ 3 ] = alpha;
         }
         ptr += 4;
         break;
         
      case GFXFormatR8G8B8:
         if( leastSignficantFirst )
         {
            ptr[ 0 ] = blue;
            ptr[ 1 ] = green;
            ptr[ 2 ] = red;
         }
         else
         {
            ptr[ 0 ] = red;
            ptr[ 1 ] = green;
            ptr[ 2 ] = blue;
         }
         ptr += 3;
         break;
         
      default:
         AssertISV( false, "GFXPackPixel() - pixel format not implemented." );
   }
}

#endif // _GFXFORMATUTILS_H_
