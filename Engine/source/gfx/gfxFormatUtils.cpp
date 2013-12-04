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

#include "gfx/gfxFormatUtils.h"
#include "gfx/gfxDevice.h"


//RDTODO: complete format infos


//-----------------------------------------------------------------------------

GFXFormatInfo::Data GFXFormatInfo::smFormatInfos[ GFXFormat_COUNT ] =
{
   // 8 bit texture formats...
   GFXFormatInfo::Data( 1,    false, false, false ),   // GFXFormatA8
   GFXFormatInfo::Data( 1,    false, false, false ),   // GFXFormatL8
   GFXFormatInfo::Data( 1,    true,  false, false ),   // GFXFormatA4L4

   // 16 bit texture formats...
   GFXFormatInfo::Data( 2,    false, false, false ),   // GFXFormatR5G6B5
   GFXFormatInfo::Data( 2,    true,  false, false ),   // GFXFormatR5G5B5A1
   GFXFormatInfo::Data( 2,    true,  false, false ),   // GFXFormatA8L8
   GFXFormatInfo::Data( 2,    false, false, false ),   // GFXFormatL16
   GFXFormatInfo::Data( 2,    false, false, false ),   // GFXFormatR16F
   GFXFormatInfo::Data( 2,    false, false, false ),   // GFXFormatD16

   // 24 bit texture formats...
   GFXFormatInfo::Data( 3,    false, false, false ),   // GFXFormatR8G8B8

   // 32 bit texture formats...
   GFXFormatInfo::Data( 4,    true,  false, false ),   // GFXFormatR8G8B8A8
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatR8G8B8X8
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatR32F
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatR16G16
   GFXFormatInfo::Data( 4,    false, false, true  ),   // GFXFormatR16G16F
   GFXFormatInfo::Data( 4,    true,  false, false ),   // GFXFormatR10G10B10A2
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatD32
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatD24X8
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatD24S8
   GFXFormatInfo::Data( 4,    false, false, false ),   // GFXFormatD24FS8

   // 64 bit texture formats...
   GFXFormatInfo::Data( 8,    true,  false, false ),   // GFXFormatR16G16B16A16
   GFXFormatInfo::Data( 8,    true,  false, true  ),   // GFXFormatR16G16B16A16F

   // 128 bit texture formats...
   GFXFormatInfo::Data( 16,   true,  false, true  ),  // GFXFormatR32G32B32A32F

   // Compressed formats...
   GFXFormatInfo::Data( 0,    false, true,  false ),   // GFXFormatDXT1
   GFXFormatInfo::Data( 0,    true,  true,  false ),   // GFXFormatDXT2
   GFXFormatInfo::Data( 0,    true,  true,  false ),   // GFXFormatDXT3
   GFXFormatInfo::Data( 0,    true,  true,  false ),   // GFXFormatDXT4
   GFXFormatInfo::Data( 0,    true,  true,  false ),   // GFXFormatDXT5
};

//-----------------------------------------------------------------------------

void GFXCopyPixels(  GFXFormat fromFormat, U32 fromWidth, U32 fromHeight, U8* fromData,
                     GFXFormat toFormat, U32 toWidth, U32 toHeight, U8* toData )
{
   if( fromFormat == toFormat
       && fromWidth == toWidth
       && fromHeight == toHeight )
      dMemcpy( toData, fromData, fromWidth * fromHeight * GFXFormatInfo( fromFormat ).getBytesPerPixel() );
   else
   {
      AssertFatal( false, "Not implemented" );
   }
}
