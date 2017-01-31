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
#include "gfx/bitmap/gBitmap.h"

#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "core/strings/stringFunctions.h"
#include "core/color.h"
#include "gfx/bitmap/bitmapUtils.h"
#include "math/mRect.h"
#include "console/console.h"
#include "platform/profiler.h"
#include "console/engineAPI.h"

using namespace Torque;

const U32 GBitmap::csFileVersion   = 3;

Vector<GBitmap::Registration>   GBitmap::sRegistrations( __FILE__, __LINE__ );


GBitmap::GBitmap()
 : mInternalFormat(GFXFormatR8G8B8),
   mBits(NULL),
   mByteSize(0),
   mWidth(0),
   mHeight(0),
   mBytesPerPixel(0),
   mNumMipLevels(0),
   mHasTransparency(false)
{
   for (U32 i = 0; i < c_maxMipLevels; i++)
      mMipLevelOffsets[i] = 0xffffffff;
}

GBitmap::GBitmap(const GBitmap& rCopy)
{
   mInternalFormat = rCopy.mInternalFormat;

   mByteSize = rCopy.mByteSize;
   mBits    = new U8[mByteSize];
   dMemcpy(mBits, rCopy.mBits, mByteSize);

   mWidth         = rCopy.mWidth;
   mHeight        = rCopy.mHeight;
   mBytesPerPixel = rCopy.mBytesPerPixel;
   mNumMipLevels  = rCopy.mNumMipLevels;
   dMemcpy(mMipLevelOffsets, rCopy.mMipLevelOffsets, sizeof(mMipLevelOffsets));

   mHasTransparency = rCopy.mHasTransparency;
}


GBitmap::GBitmap(const U32  in_width,
                 const U32  in_height,
                 const bool in_extrudeMipLevels,
                 const GFXFormat in_format)
 : mBits(NULL),
   mByteSize(0)
{
   for (U32 i = 0; i < c_maxMipLevels; i++)
      mMipLevelOffsets[i] = 0xffffffff;

   allocateBitmap(in_width, in_height, in_extrudeMipLevels, in_format);

   mHasTransparency = false;
}

GBitmap::GBitmap(const U32  in_width,
                 const U32  in_height,
                 const U8*  data )
 : mBits(NULL),
   mByteSize(0)
{
   allocateBitmap(in_width, in_height, false, GFXFormatR8G8B8A8);

   mHasTransparency = false;

   for (U32 x = 0; x < in_width; x++)
   {
      for (U32 y = 0; y < in_height; y++)
      {
         U32 offset = (x + y * in_width) * 4;

         ColorI color(data[offset],
                      data[offset + 1],
                      data[offset + 2],
                      data[offset + 3]);

         if (color.alpha < 255)
            mHasTransparency = true;

         setColor(x, y, color);
      }
   }
}


//--------------------------------------------------------------------------

GBitmap::~GBitmap()
{
   deleteImage();
}

//--------------------------------------------------------------------------

void GBitmap::sRegisterFormat( const GBitmap::Registration &reg )
{
   U32 insert = sRegistrations.size();
   for ( U32 i = 0; i < sRegistrations.size(); i++ )
   {
      if ( sRegistrations[i].priority <= reg.priority )
      {
         insert = i;
         break;
      }
   }

   sRegistrations.insert( insert, reg );
}

const GBitmap::Registration   *GBitmap::sFindRegInfo( const String &extension )
{
   for ( U32 i = 0; i < GBitmap::sRegistrations.size(); i++ )
   {
      const GBitmap::Registration   &reg = GBitmap::sRegistrations[i];
      const Vector<String>          &extensions = reg.extensions;

      for ( U32 j = 0; j < extensions.size(); ++j )
      {    
         if ( extensions[j].equal( extension, String::NoCase ) )
            return &reg;
      }
   }

   return NULL;
}

bool GBitmap::sFindFile( const Path &path, Path *outPath )
{
   PROFILE_SCOPE( GBitmap_sFindFile );

   const String origExt( String::ToLower( path.getExtension() ) );

   Path tryPath( path );

   for ( U32 i = 0; i < sRegistrations.size(); i++ )
   {
      const Registration &reg = sRegistrations[i];
      const Vector<String> &extensions = reg.extensions;

      for ( U32 j = 0; j < extensions.size(); ++j )
      {
         // We've already tried this one.
         if ( extensions[j] == origExt )
            continue;

         tryPath.setExtension( extensions[j] );
         if ( !Torque::FS::IsFile( tryPath ) )
            continue;

         if ( outPath )
            *outPath = tryPath;
         return true;
      }
   }

   return false;
}

bool GBitmap::sFindFiles( const Path &path, Vector<Path> *outFoundPaths )
{
   PROFILE_SCOPE( GBitmap_sFindFiles );
   
   Path  tryPath( path );

   for ( U32 i = 0; i < GBitmap::sRegistrations.size(); i++ )
   {
      const GBitmap::Registration   &reg = GBitmap::sRegistrations[i];
      const Vector<String>          &extensions = reg.extensions;

      for ( U32 j = 0; j < extensions.size(); ++j )
      {
         tryPath.setExtension( extensions[j] );

         if ( Torque::FS::IsFile( tryPath ) )
         {
            if ( outFoundPaths )
               outFoundPaths->push_back( tryPath );
            else
               return true;
         }
      }
   }

   return outFoundPaths ? outFoundPaths->size() > 0 : false;
}

String GBitmap::sGetExtensionList()
{
   String list;

   for ( U32 i = 0; i < sRegistrations.size(); i++ )
   {
      const Registration &reg = sRegistrations[i];
      for ( U32 j = 0; j < reg.extensions.size(); j++ )
      {
         list += reg.extensions[j];
         list += " ";         
      }
   }

   return list;
}

//--------------------------------------------------------------------------
void GBitmap::deleteImage()
{
   delete [] mBits;
   mBits    = NULL;
   mByteSize = 0;

   mWidth        = 0;
   mHeight       = 0;
   mNumMipLevels = 0;
}


//--------------------------------------------------------------------------

void GBitmap::copyRect(const GBitmap *src, const RectI &srcRect, const Point2I &dstPt, const U32 srcMipLevel, const U32 dstMipLevel)
{
   if(src->getFormat() != getFormat())
      return;
   if(srcRect.extent.x + srcRect.point.x > src->getWidth(srcMipLevel) || srcRect.extent.y + srcRect.point.y > src->getHeight(srcMipLevel))
      return;
   if(srcRect.extent.x + dstPt.x > getWidth(dstMipLevel) || srcRect.extent.y + dstPt.y > getHeight(dstMipLevel))
      return;

   for(U32 i = 0; i < srcRect.extent.y; i++)
   {
      dMemcpy(getAddress(dstPt.x, dstPt.y + i, dstMipLevel),
              src->getAddress(srcRect.point.x, srcRect.point.y + i, srcMipLevel),
              mBytesPerPixel * srcRect.extent.x);
   }
}

//--------------------------------------------------------------------------
void GBitmap::allocateBitmap(const U32 in_width, const U32 in_height, const bool in_extrudeMipLevels, const GFXFormat in_format )
{
   //-------------------------------------- Some debug checks...
   U32 svByteSize = mByteSize;
   U8 *svBits = mBits;

   AssertFatal(in_width != 0 && in_height != 0, "GBitmap::allocateBitmap: width or height is 0");

   if (in_extrudeMipLevels == true) 
   {
      AssertFatal(isPow2(in_width) == true && isPow2(in_height) == true, "GBitmap::GBitmap: in order to extrude mip levels, bitmap w/h must be pow2");
   }

   mInternalFormat = in_format;
   mWidth          = in_width;
   mHeight         = in_height;

   mBytesPerPixel = 1;
   switch (mInternalFormat) 
   {
     case GFXFormatA8:
     case GFXFormatL8:           mBytesPerPixel = 1;
      break;
     case GFXFormatR8G8B8:       mBytesPerPixel = 3;
      break;
     case GFXFormatR8G8B8A8_LINEAR_FORCE:
     case GFXFormatR8G8B8X8:
     case GFXFormatR8G8B8A8:     mBytesPerPixel = 4;
      break;
     case GFXFormatR5G6B5:
     case GFXFormatR5G5B5A1:     mBytesPerPixel = 2;
      break;
     default:
      AssertFatal(false, "GBitmap::GBitmap: misunderstood format specifier");
      break;
   }

   // Set up the mip levels, if necessary...
   mNumMipLevels       = 1;
   U32 allocPixels = in_width * in_height * mBytesPerPixel;
   mMipLevelOffsets[0] = 0;


   if (in_extrudeMipLevels == true) 
   {
      U32 currWidth  = in_width;
      U32 currHeight = in_height;

      do 
      {
         mMipLevelOffsets[mNumMipLevels] = mMipLevelOffsets[mNumMipLevels - 1] +
                                         (currWidth * currHeight * mBytesPerPixel);
         currWidth  >>= 1;
         currHeight >>= 1;
         if (currWidth  == 0) currWidth  = 1;
         if (currHeight == 0) currHeight = 1;

         mNumMipLevels++;
         allocPixels += currWidth * currHeight * mBytesPerPixel;
      } while (currWidth != 1 || currHeight != 1);

      U32 expectedMips = mFloor(mLog2(mMax(in_width, in_height))) + 1;
      AssertFatal(mNumMipLevels == expectedMips, "GBitmap::allocateBitmap: mipmap count wrong");
   }
   AssertFatal(mNumMipLevels <= c_maxMipLevels, "GBitmap::allocateBitmap: too many miplevels");

   // Set up the memory...
   mByteSize = allocPixels;
   mBits    = new U8[mByteSize];

   dMemset(mBits, 0xFF, mByteSize);

   if(svBits != NULL)
   {
      dMemcpy(mBits, svBits, getMin(mByteSize, svByteSize));
      delete[] svBits;
   }
}

//--------------------------------------------------------------------------
void GBitmap::extrudeMipLevels(bool clearBorders)
{
   if(mNumMipLevels == 1)
      allocateBitmap(getWidth(), getHeight(), true, getFormat());

   switch (getFormat())
   {
      case GFXFormatR5G5B5A1:
      {
         for(U32 i = 1; i < mNumMipLevels; i++)
            bitmapExtrude5551(getBits(i - 1), getWritableBits(i), getHeight(i), getWidth(i));
         break;
      }

      case GFXFormatR8G8B8:
      {
         for(U32 i = 1; i < mNumMipLevels; i++)
            bitmapExtrudeRGB(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
         break;
      }

      case GFXFormatR8G8B8A8:
      case GFXFormatR8G8B8X8:
      {
         for(U32 i = 1; i < mNumMipLevels; i++)
            bitmapExtrudeRGBA(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
         break;
      }
      
      default:
         break;
   }
   if (clearBorders)
   {
      for (U32 i = 1; i<mNumMipLevels; i++)
      {
         U32 width = getWidth(i);
         U32 height = getHeight(i);
         if (height<3 || width<3)
            // bmp is all borders at this mip level
            dMemset(getWritableBits(i),0,width*height*mBytesPerPixel);
         else
         {
            width *= mBytesPerPixel;
            U8 * bytes = getWritableBits(i);
            U8 * end = bytes + (height-1)*width - mBytesPerPixel; // end = last row, 2nd column
            // clear first row sans the last pixel
            dMemset(bytes,0,width-mBytesPerPixel);
            bytes -= mBytesPerPixel;
            while (bytes<end)
            {
               // clear last pixel of row N-1 and first pixel of row N
               bytes += width;
               dMemset(bytes,0,mBytesPerPixel*2);
            }
            // clear last row sans the first pixel
            dMemset(bytes+2*mBytesPerPixel,0,width-mBytesPerPixel);
         }
      }
   }
}

//--------------------------------------------------------------------------
void GBitmap::extrudeMipLevelsDetail()
{
   AssertFatal(getFormat() == GFXFormatR8G8B8, "Error, only handles RGB for now...");
   U32 i,j;

   if(mNumMipLevels == 1)
      allocateBitmap(getWidth(), getHeight(), true, getFormat());

   for (i = 1; i < mNumMipLevels; i++) {
      bitmapExtrudeRGB(getBits(i - 1), getWritableBits(i), getHeight(i-1), getWidth(i-1));
   }

   // Ok, now that we have the levels extruded, we need to move the lower miplevels
   //  closer to 0.5.
   for (i = 1; i < mNumMipLevels - 1; i++) {
      U8* pMipBits = (U8*)getWritableBits(i);
      U32 numBytes = getWidth(i) * getHeight(i) * 3;

      U32 shift    = i;
      U32 start    = ((1 << i) - 1) * 0x80;

      for (j = 0; j < numBytes; j++) {
         U32 newVal = (start + pMipBits[j]) >> shift;
         AssertFatal(newVal <= 255, "Error, oob");
         pMipBits[j] = U8(newVal);
      }
   }
   AssertFatal(getWidth(mNumMipLevels - 1) == 1 && getHeight(mNumMipLevels - 1) == 1,
               "Error, last miplevel should be 1x1!");
   ((U8*)getWritableBits(mNumMipLevels - 1))[0] = 0x80;
   ((U8*)getWritableBits(mNumMipLevels - 1))[1] = 0x80;
   ((U8*)getWritableBits(mNumMipLevels - 1))[2] = 0x80;
}

//--------------------------------------------------------------------------
bool GBitmap::setFormat(GFXFormat fmt)
{
   if (getFormat() == fmt)
      return true;

   PROFILE_SCOPE(GBitmap_setFormat);

   // this is a nasty pointer math hack
   // is there a quick way to calc pixels of a fully mipped bitmap?
   U32 pixels = 0;
   for (U32 i=0; i < mNumMipLevels; i++)
      pixels += getHeight(i) * getWidth(i);

   switch( getFormat() )
   {
      case GFXFormatR8G8B8:
         switch ( fmt )
         {
            case GFXFormatR5G5B5A1:
#ifdef _XBOX
               bitmapConvertRGB_to_1555(mBits, pixels);
#else
               bitmapConvertRGB_to_5551(mBits, pixels);
#endif
               mInternalFormat = GFXFormatR5G5B5A1;
               mBytesPerPixel  = 2;
               break;

            case GFXFormatR8G8B8A8:
            case GFXFormatR8G8B8X8:
               // Took this out, it may crash -patw
               //AssertFatal( mNumMipLevels == 1, "Do the mip-mapping in hardware." );

               bitmapConvertRGB_to_RGBX( &mBits, pixels );
               mInternalFormat = fmt;
               mBytesPerPixel = 4;
               mByteSize = pixels * 4;
               break;

            default:
               AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
               return false;
         }
         break;

      case GFXFormatR8G8B8X8:
         switch( fmt )
         {
            // No change needed for this
            case GFXFormatR8G8B8A8:
               mInternalFormat = GFXFormatR8G8B8A8;
               break;

            case GFXFormatR8G8B8:
               bitmapConvertRGBX_to_RGB( &mBits, pixels );
               mInternalFormat = GFXFormatR8G8B8;
               mBytesPerPixel = 3;
               mByteSize = pixels * 3;
               break;

            default:
               AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
               return false;
         }
         break;

      case GFXFormatR8G8B8A8:
         switch( fmt )
         {
            // No change needed for this
            case GFXFormatR8G8B8X8:
               mInternalFormat = GFXFormatR8G8B8X8;
               break;

            case GFXFormatR8G8B8:
               bitmapConvertRGBX_to_RGB( &mBits, pixels );
               mInternalFormat = GFXFormatR8G8B8;
               mBytesPerPixel = 3;
               mByteSize = pixels * 3;
               break;

            default:
               AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
               return false;
         }
         break;

      case GFXFormatA8:
         switch( fmt )
         {
            case GFXFormatR8G8B8A8:
               mInternalFormat = GFXFormatR8G8B8A8;
               bitmapConvertA8_to_RGBA( &mBits, pixels );
               mBytesPerPixel = 4;
               mByteSize = pixels * 4;
               break;

            default:
               AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
               return false;
         }
         break;

      default:
         AssertWarn(0, "GBitmap::setFormat: unable to convert bitmap to requested format.");
         return false;
   }

   U32 offset = 0;
   for (U32 j=0; j < mNumMipLevels; j++)
   {
      mMipLevelOffsets[j] = offset;
      offset += getHeight(j) * getWidth(j) * mBytesPerPixel;
   }

   return true;
}

//------------------------------------------------------------------------------

bool GBitmap::checkForTransparency()
{
   mHasTransparency = false;

   ColorI pixel(255, 255, 255, 255);

   switch (mInternalFormat)
   {
      // Non-transparent formats
      case GFXFormatL8:
      case GFXFormatR8G8B8:
      case GFXFormatR5G6B5:
         break;
      // Transparent formats
      case GFXFormatA8:
      case GFXFormatR8G8B8A8:
      case GFXFormatR5G5B5A1:
         // Let getColor() do the heavy lifting
         for (U32 x = 0; x < mWidth; x++)
         {
            for (U32 y = 0; y < mHeight; y++)
            {
               if (getColor(x, y, pixel))
               {
                  if (pixel.alpha < 255)
                  {
                     mHasTransparency = true;
                     break;
                  }
               }
            }
         }

         break;
      default:
         AssertFatal(false, "GBitmap::checkForTransparency: misunderstood format specifier");
      break;
   }

   return mHasTransparency;
}

//------------------------------------------------------------------------------
ColorF GBitmap::sampleTexel(F32 u, F32 v) const
{
	ColorF col(0.5f, 0.5f, 0.5f);
	// normally sampling wraps all the way around at 1.0,
	// but locking doesn't support this, and we seem to calc
	// the uv based on a clamped 0 - 1...
	Point2F max((F32)(getWidth()-1), (F32)(getHeight()-1));
	Point2F posf;
	posf.x = mClampF(((u) * max.x), 0.0f, max.x);
	posf.y = mClampF(((v) * max.y), 0.0f, max.y);
	Point2I posi((S32)posf.x, (S32)posf.y);

	const U8 *buffer = getBits();
	U32 lexelindex = ((posi.y * getWidth()) + posi.x) * mBytesPerPixel;

	if(mBytesPerPixel == 2)
	{
		//U16 *buffer = (U16 *)lockrect->pBits;
	}
	else if(mBytesPerPixel > 2)
	{		
		col.red = F32(buffer[lexelindex + 0]) / 255.0f;
      col.green = F32(buffer[lexelindex + 1]) / 255.0f;
		col.blue = F32(buffer[lexelindex + 2]) / 255.0f;
	}

	return col;
}

//--------------------------------------------------------------------------
bool GBitmap::getColor(const U32 x, const U32 y, ColorI& rColor) const
{
   if (x >= mWidth || y >= mHeight)
      return false;

   const U8* pLoc = getAddress(x, y);

   switch (mInternalFormat) {
     case GFXFormatA8:
     case GFXFormatL8:
      rColor.set( *pLoc, *pLoc, *pLoc, *pLoc );
      break;

     case GFXFormatR8G8B8:
     case GFXFormatR8G8B8X8:
        rColor.set( pLoc[0], pLoc[1], pLoc[2], 255 );
      break;

     case GFXFormatR8G8B8A8:
      rColor.set( pLoc[0], pLoc[1], pLoc[2], pLoc[3] );
      break;

     case GFXFormatR5G5B5A1:
#if defined(TORQUE_OS_MAC)
      rColor.set( (*((U16*)pLoc) >> 0) & 0x1F,
                  (*((U16*)pLoc) >> 5) & 0x1F,
                  (*((U16*)pLoc) >> 10) & 0x1F,
                  ((*((U16*)pLoc) >> 15) & 0x01) ? 255 : 0 );
#else
      rColor.set( *((U16*)pLoc) >> 11,
                  (*((U16*)pLoc) >> 6) & 0x1f,
                  (*((U16*)pLoc) >> 1) & 0x1f,
                  (*((U16*)pLoc) & 1) ? 255 : 0 );
#endif
      break;

     default:
      AssertFatal(false, "Bad internal format");
      return false;
   }

   return true;
}


//--------------------------------------------------------------------------


bool GBitmap::setColor(const U32 x, const U32 y, const ColorI& rColor)
{
   if (x >= mWidth || y >= mHeight)
      return false;

   U8* pLoc = getAddress(x, y);

   switch (mInternalFormat) {
     case GFXFormatA8:
     case GFXFormatL8:
      *pLoc = rColor.alpha;
      break;

     case GFXFormatR8G8B8:
      dMemcpy( pLoc, &rColor, 3 * sizeof( U8 ) );
      break;

     case GFXFormatR8G8B8A8:
     case GFXFormatR8G8B8X8:
      dMemcpy( pLoc, &rColor, 4 * sizeof( U8 ) );
      break;
      
     case GFXFormatR5G6B5:
      #ifdef TORQUE_OS_MAC
         *((U16*)pLoc) = (rColor.red << 11) | (rColor.green << 5) | (rColor.blue << 0) ;
      #else
         *((U16*)pLoc) = (rColor.blue << 0) | (rColor.green << 5) | (rColor.red << 11);
      #endif
      break;

     case GFXFormatR5G5B5A1:
      #ifdef TORQUE_OS_MAC
         *((U16*)pLoc) = (((rColor.alpha>0) ? 1 : 0)<<15) | (rColor.blue << 10) | (rColor.green << 5) | (rColor.red << 0);
      #else
         *((U16*)pLoc) = (rColor.blue << 1) | (rColor.green << 6) | (rColor.red << 11) | ((rColor.alpha>0) ? 1 : 0);
      #endif
      break;

     default:
      AssertFatal(false, "Bad internal format");
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool GBitmap::combine( const GBitmap *bitmapA, const GBitmap *bitmapB, const GFXTextureOp combineOp )
{
   // Check valid texture ops
   switch( combineOp )
   {
      case GFXTOPAdd:
      case GFXTOPSubtract:
         break;

      default:
         Con::errorf( "GBitmap::combine - Invalid op type" );
         return false;
   }

   // Check bitmapA format
   switch( bitmapA->getFormat() )
   {
   case GFXFormatR8G8B8:
   case GFXFormatR8G8B8X8:
   case GFXFormatR8G8B8A8:
      break;

   default:
      Con::errorf( "GBitmap::combine - invalid format for bitmapA" );
      return false;
   }

   // Check bitmapB format
   switch( bitmapB->getFormat() )
   {
   case GFXFormatR8G8B8:
   case GFXFormatR8G8B8X8:
   case GFXFormatR8G8B8A8:
      break;

   default:
      Con::errorf( "GBitmap::combine - invalid format for bitmapB" );
      return false;
   }

   // Determine format of result texture
   // CodeReview: This is dependent on the order of the GFXFormat enum. [5/11/2007 Pat]
   GFXFormat resFmt = static_cast<GFXFormat>( getMax( bitmapA->getFormat(), bitmapB->getFormat() ) );
   U32 resWidth = getMax( bitmapA->getWidth(), bitmapB->getWidth() );
   U32 resHeight = getMax( bitmapA->getHeight(), bitmapB->getHeight() );

   // Adjust size OF bitmap based on the biggest one
   if( bitmapA->getWidth() != bitmapB->getWidth() ||
       bitmapA->getHeight() != bitmapB->getHeight() )
   {
      // Delete old bitmap
      deleteImage();

      // Allocate new one
      allocateBitmap( resWidth, resHeight, false, resFmt );
   }
   
   // Adjust format of result bitmap (if resFmt == getFormat() it will not perform the format convert)
   setFormat( resFmt );

   // Perform combine
   U8 *destBits = getWritableBits();
   const U8 *aBits = bitmapA->getBits();
   const U8 *bBits = bitmapB->getBits();

   for( S32 y = 0; y < getHeight(); y++ )
   {
      for( S32 x = 0; x < getWidth(); x++ )
      {
         for( S32 _byte = 0; _byte < mBytesPerPixel; _byte++ )
         {
            U8 pxA = 0;
            U8 pxB = 0;

            // Get contributions from A and B
            if( y < bitmapA->getHeight() && 
                x < bitmapA->getWidth() &&
                _byte < bitmapA->mBytesPerPixel )
               pxA = *aBits++;

            if( y < bitmapB->getHeight() && 
               x < bitmapB->getWidth() &&
               _byte < bitmapB->mBytesPerPixel )
               pxB = *bBits++;

            // Combine them (clamp values 0-U8_MAX)
            switch( combineOp )
            {
               case GFXTOPAdd:
                  *destBits++ = getMin( U8( pxA + pxB ), U8_MAX );
                  break;

               case GFXTOPSubtract:
                  *destBits++ = getMax( U8( pxA - pxB ), U8( 0 ) );
                  break;
               default:
                  AssertFatal(false, "GBitmap::combine - Invalid combineOp");
                  break;
            }
         }
      }
   }

   return true;
}

void GBitmap::fill( const ColorI &rColor )
{
   // Set the first pixel using the slow 
   // but proper method.
   setColor( 0, 0, rColor );
   mHasTransparency = rColor.alpha < 255;
      
   // Now fill the first row of the bitmap by 
   // copying the first pixel across the row.
   const U32 stride = getWidth() * mBytesPerPixel;
   const U8 *src = getBits();
   U8 *dest = getWritableBits() + mBytesPerPixel;
   const U8 *end = src + stride;
   for ( ; dest != end; dest += mBytesPerPixel )
      dMemcpy( dest, src, mBytesPerPixel );

   // Now copy the first row to all the others.
   // 
   // TODO: This could adaptively size the copy 
   // amount to copy more rows from the source
   // and reduce the total number of memcpy calls.
   //
   dest = getWritableBits() + stride;
   end = src + ( stride * getHeight() );
   for ( ; dest != end; dest += stride )
      dMemcpy( dest, src, stride );
}

void GBitmap::fillWhite()
{
   dMemset( getWritableBits(), 255, mByteSize );
   mHasTransparency = false;
}

GBitmap* GBitmap::createPaddedBitmap() const
{
   if (isPow2(getWidth()) && isPow2(getHeight()))
      return NULL;

   AssertFatal(getNumMipLevels() == 1,
      "Cannot have non-pow2 bitmap with miplevels");

   U32 width = getWidth();
   U32 height = getHeight();

   U32 newWidth  = getNextPow2(getWidth());
   U32 newHeight = getNextPow2(getHeight());

   GBitmap* pReturn = new GBitmap(newWidth, newHeight, false, getFormat());

   for (U32 i = 0; i < height; i++) 
   {
      U8*       pDest = (U8*)pReturn->getAddress(0, i);
      const U8* pSrc  = (const U8*)getAddress(0, i);

      dMemcpy(pDest, pSrc, width * mBytesPerPixel);

      pDest += width * mBytesPerPixel;
      // set the src pixel to the last pixel in the row
      const U8 *pSrcPixel = pDest - mBytesPerPixel; 

      for(U32 j = width; j < newWidth; j++)
         for(U32 k = 0; k < mBytesPerPixel; k++)
            *pDest++ = pSrcPixel[k];
   }

   for(U32 i = height; i < newHeight; i++)
   {
      U8* pDest = (U8*)pReturn->getAddress(0, i);
      U8* pSrc = (U8*)pReturn->getAddress(0, height-1);
      dMemcpy(pDest, pSrc, newWidth * mBytesPerPixel);
   }

   return pReturn;
}

GBitmap* GBitmap::createPow2Bitmap() const
{
   if (isPow2(getWidth()) && isPow2(getHeight()))
      return NULL;

   AssertFatal(getNumMipLevels() == 1,
               "Cannot have non-pow2 bitmap with miplevels");

   U32 width = getWidth();
   U32 height = getHeight();

   U32 newWidth  = getNextPow2(getWidth());
   U32 newHeight = getNextPow2(getHeight());

   GBitmap* pReturn = new GBitmap(newWidth, newHeight, false, getFormat());

   U8*       pDest = (U8*)pReturn->getAddress(0, 0);
   const U8* pSrc  = (const U8*)getAddress(0, 0);

   F32 yCoeff = (F32) height / (F32) newHeight;
   F32 xCoeff = (F32) width / (F32) newWidth;

   F32 currY = 0.0f;
   for (U32 y = 0; y < newHeight; y++)
   {
      F32 currX = 0.0f;
      //U32 yDestOffset = (pReturn->mWidth * pReturn->mBytesPerPixel) * y;
      //U32 xDestOffset = 0;
      //U32 ySourceOffset = (U32)((mWidth * mBytesPerPixel) * currY);
      //F32 xSourceOffset = 0.0f;
      for (U32 x = 0; x < newWidth; x++)
      {
         pDest = (U8*) pReturn->getAddress(x, y);
         pSrc = (U8*) getAddress((S32)currX, (S32)currY);
         for (U32 p = 0; p < pReturn->mBytesPerPixel; p++) 
         {
            pDest[p] = pSrc[p];
         }
         currX += xCoeff;
      }
      currY += yCoeff;
   }

   return pReturn;
}

void GBitmap::copyChannel( U32 index, GBitmap *outBitmap ) const
{
   AssertFatal( index < mBytesPerPixel, "GBitmap::copyChannel() - Bad channel offset!" );
   AssertFatal( outBitmap, "GBitmap::copyChannel() - Null output bitmap!" );   
   AssertFatal( outBitmap->getWidth() == getWidth(), "GBitmap::copyChannel() - Width mismatch!" );
   AssertFatal( outBitmap->getHeight() == getHeight(), "GBitmap::copyChannel() - Height mismatch!" );

   U8 *outBits = outBitmap->getWritableBits();
   const U32 outBytesPerPixel = outBitmap->getBytesPerPixel();
   const U8 *srcBits = getBits() + index;
   const U8 *endBits = getBits() + mByteSize;

   for (  ; srcBits < endBits;  )
   {
      *outBits = *srcBits;
      outBits += outBytesPerPixel;
      srcBits += mBytesPerPixel;
   }
}

//------------------------------------------------------------------------------

bool GBitmap::read(Stream& io_rStream)
{
   // Handle versioning
   U32 version;
   io_rStream.read(&version);
   AssertFatal(version == csFileVersion, "Bitmap::read: incorrect file version");

   //-------------------------------------- Read the object
   U32 fmt;
   io_rStream.read(&fmt);
   mInternalFormat = GFXFormat(fmt);
   mBytesPerPixel = 1;
   switch (mInternalFormat) {
     case GFXFormatA8:
     case GFXFormatL8:  mBytesPerPixel = 1;
      break;
     case GFXFormatR8G8B8:        mBytesPerPixel = 3;
      break;
     case GFXFormatR8G8B8A8:       mBytesPerPixel = 4;
      break;
     case GFXFormatR5G6B5:
     case GFXFormatR5G5B5A1:    mBytesPerPixel = 2;
      break;
     default:
      AssertFatal(false, "GBitmap::read: misunderstood format specifier");
      break;
   }

   io_rStream.read(&mByteSize);

   mBits = new U8[mByteSize];
   io_rStream.read(mByteSize, mBits);

   io_rStream.read(&mWidth);
   io_rStream.read(&mHeight);

   io_rStream.read(&mNumMipLevels);
   for (U32 i = 0; i < c_maxMipLevels; i++)
      io_rStream.read(&mMipLevelOffsets[i]);

   checkForTransparency();

   return (io_rStream.getStatus() == Stream::Ok);
}

bool GBitmap::write(Stream& io_rStream) const
{
   // Handle versioning
   io_rStream.write(csFileVersion);

   //-------------------------------------- Write the object
   io_rStream.write(U32(mInternalFormat));

   io_rStream.write(mByteSize);
   io_rStream.write(mByteSize, mBits);

   io_rStream.write(mWidth);
   io_rStream.write(mHeight);

   io_rStream.write(mNumMipLevels);
   for (U32 i = 0; i < c_maxMipLevels; i++)
      io_rStream.write(mMipLevelOffsets[i]);

   return (io_rStream.getStatus() == Stream::Ok);
}

//------------------------------------------------------------------------------
//-------------------------------------- Persistent I/O
//

bool  GBitmap::readBitmap( const String &bmType, Stream &ioStream )
{
   const GBitmap::Registration   *regInfo = GBitmap::sFindRegInfo( bmType );

   if ( regInfo == NULL )
   {
      Con::errorf( "[GBitmap::readBitmap] unable to find registration for extension [%s]", bmType.c_str() );
      return NULL;
   }

   return regInfo->readFunc( ioStream, this );
}

bool  GBitmap::writeBitmap( const String &bmType, Stream &ioStream, U32 compressionLevel )
{
   const GBitmap::Registration   *regInfo = GBitmap::sFindRegInfo( bmType );

   if ( regInfo == NULL )
   {
      Con::errorf( "[GBitmap::writeBitmap] unable to find registration for extension [%s]", bmType.c_str() );
      return NULL;
   }

   return regInfo->writeFunc( this, ioStream, (compressionLevel == U32_MAX) ? regInfo->defaultCompression : compressionLevel );
}

template<> void *Resource<GBitmap>::create(const Torque::Path &path)
{
   PROFILE_SCOPE( ResourceGBitmap_create );

#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "Resource<GBitmap>::create - [%s]", path.getFullPath().c_str() );
#endif

   FileStream  stream;

   stream.open( path.getFullPath(), Torque::FS::File::Read );

   if ( stream.getStatus() != Stream::Ok )
   {
      Con::errorf( "Resource<GBitmap>::create - failed to open '%s'", path.getFullPath().c_str() );
      return NULL;
   }

   GBitmap *bmp = new GBitmap;
   const String extension = path.getExtension();
   if( !bmp->readBitmap( extension, stream ) )
   {
      Con::errorf( "Resource<GBitmap>::create - error reading '%s'", path.getFullPath().c_str() );
      delete bmp;
      bmp = NULL;
   }

   return bmp;
}

template<> ResourceBase::Signature  Resource<GBitmap>::signature()
{
   return MakeFourCC('b','i','t','m');
}

Resource<GBitmap> GBitmap::load(const Torque::Path &path)
{
   Resource<GBitmap> ret = _load( path );
   if ( ret != NULL )
      return ret;

   // Do a recursive search.
   return _search( path );
}

Resource<GBitmap> GBitmap::_load(const Torque::Path &path)
{
   PROFILE_SCOPE( GBitmap_load );

   if ( Torque::FS::IsFile( path ) )
      return ResourceManager::get().load( path );
  
   Path foundPath;
   if ( GBitmap::sFindFile( path, &foundPath ) )
   {
      Resource<GBitmap> ret = ResourceManager::get().load( foundPath );
      if ( ret != NULL )
         return ret;
   }

   return Resource< GBitmap >( NULL );
}

Resource<GBitmap> GBitmap::_search(const Torque::Path &path)
{
   PROFILE_SCOPE( GBitmap_search );

   // If unable to load texture in current directory
   // look in the parent directory.  But never look in the root.
   Path newPath( path );
   while ( true )
   {
      String filePath = newPath.getPath();
      String::SizeType slash = filePath.find( '/', filePath.length(), String::Right );

      if ( slash == String::NPos )
         break;

      slash = filePath.find( '/', filePath.length(), String::Right );
      if ( slash == String::NPos )
         break;

      String truncPath = filePath.substr( 0, slash );
      newPath.setPath( truncPath );

      Resource<GBitmap> ret = _load( newPath );
      if ( ret != NULL )
         return ret;
   }

   return Resource< GBitmap >( NULL );
}

DefineEngineFunction( getBitmapInfo, String, ( const char *filename ),,
   "Returns image info in the following format: width TAB height TAB bytesPerPixel. "
   "It will return an empty string if the file is not found.\n"
   "@ingroup Rendering\n" )
{
   Resource<GBitmap> image = GBitmap::load( filename );
   if ( !image )
      return String::EmptyString;

   return String::ToString( "%d\t%d\t%d", image->getWidth(), 
                                          image->getHeight(),
                                          image->getBytesPerPixel() );
}
