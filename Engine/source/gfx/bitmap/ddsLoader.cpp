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
#include "gfx/bitmap/ddsFile.h"

#include "gfx/gfxDevice.h"
#include "core/util/fourcc.h"
#include "console/console.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "gfx/bitmap/gBitmap.h"
#include "console/engineAPI.h"



S32 DDSFile::smActiveCopies = 0;
U32 DDSFile::smDropMipCount = 0;

// These were copied from the DX9 docs. The names are changed
// from the "real" defines since not all platforms have them.
enum DDSSurfaceDescFlags
{
   DDSDCaps         = 0x00000001l,
   DDSDHeight       = 0x00000002l,
   DDSDWidth        = 0x00000004l,
   DDSDPitch        = 0x00000008l,
   DDSDPixelFormat  = 0x00001000l,
   DDSDMipMapCount  = 0x00020000l,
   DDSDLinearSize   = 0x00080000l,
   DDSDDepth        = 0x00800000l,
};

enum DDSPixelFormatFlags
{
   DDPFAlphaPixels   = 0x00000001,
   DDPFFourCC        = 0x00000004,
   DDPFRGB           = 0x00000040,
   DDPFLUMINANCE     = 0x00020000
};


enum DDSCapFlags
{
   DDSCAPSComplex = 0x00000008,
   DDSCAPSTexture = 0x00001000,
   DDSCAPSMipMap  = 0x00400000,

   DDSCAPS2Cubemap = 0x00000200,
   DDSCAPS2Cubemap_POSITIVEX = 0x00000400,
   DDSCAPS2Cubemap_NEGATIVEX = 0x00000800,
   DDSCAPS2Cubemap_POSITIVEY = 0x00001000,
   DDSCAPS2Cubemap_NEGATIVEY = 0x00002000,
   DDSCAPS2Cubemap_POSITIVEZ = 0x00004000,
   DDSCAPS2Cubemap_NEGATIVEZ = 0x00008000,
   DDSCAPS2Volume = 0x00200000,
};

#define FOURCC_DXT1  (MakeFourCC('D','X','T','1'))
#define FOURCC_DXT2  (MakeFourCC('D','X','T','2'))
#define FOURCC_DXT3  (MakeFourCC('D','X','T','3'))
#define FOURCC_DXT4  (MakeFourCC('D','X','T','4'))
#define FOURCC_DXT5  (MakeFourCC('D','X','T','5'))

DDSFile::DDSFile( const DDSFile &dds )
   :  mFlags( dds.mFlags ),
      mHeight( dds.mHeight ),
      mWidth( dds.mWidth ),
      mDepth( dds.mDepth ),
      mPitchOrLinearSize( dds.mPitchOrLinearSize ),
      mMipMapCount( dds.mMipMapCount ),
      mFormat( dds.mFormat ),
      mBytesPerPixel( dds.mBytesPerPixel ),
      mFourCC( dds.mFourCC ),
      mCacheString( dds.mCacheString ),
      mSourcePath( dds.mSourcePath ),
      mHasTransparency( dds.mHasTransparency )
{
   VECTOR_SET_ASSOCIATION( mSurfaces );
   smActiveCopies++;
   
   for ( U32 i=0; i < dds.mSurfaces.size(); i++ )
   {
      SurfaceData *surface = NULL;
      if ( dds.mSurfaces[i] )
         surface = new SurfaceData;

      mSurfaces.push_back( surface );

      if ( !surface )
         continue;

      for ( U32 m=0; m < dds.mSurfaces[i]->mMips.size(); m++ )
      {
         U32 size = dds.getSurfaceSize( m );
         surface->mMips.push_back(new U8[size]);         
         dMemcpy( surface->mMips.last(), dds.mSurfaces[i]->mMips[m], size );
      }
   }
}

void DDSFile::clear()
{
   mFlags = 0;
   mHeight = mWidth = mDepth = mPitchOrLinearSize = mMipMapCount = 0;
   mFormat = GFXFormatR8G8B8;
}

U32 DDSFile::getSurfacePitch( U32 mipLevel ) const
{
   if(mFlags.test(CompressedData))
   {
      U32 sizeMultiple = 0;

      switch(mFormat)
      {
      case GFXFormatDXT1:
         sizeMultiple = 8;
         break;
      case GFXFormatDXT2:
      case GFXFormatDXT3:
      case GFXFormatDXT4:
      case GFXFormatDXT5:
         sizeMultiple = 16;
         break;
      default:
         AssertISV(false, "DDSFile::getPitch - invalid compressed texture format, we only support DXT1-5 right now.");
         break;
      }

      // Maybe need to be DWORD aligned?
      U32 align = getMax(U32(1), getWidth(mipLevel)/4) * sizeMultiple;
      align += 3; align >>=2; align <<=2;
      return align;

   }
   else
      return getWidth(mipLevel) * mBytesPerPixel;
}

U32 DDSFile::getSurfaceSize( U32 height, U32 width, U32 mipLevel ) const 
{
   // Bump by the mip level.
   height = getMax(U32(1), height >> mipLevel);
   width  = getMax(U32(1), width >> mipLevel);

   if(mFlags.test(CompressedData))
   {
      // From the directX docs:
      // max(1, width ÷ 4) x max(1, height ÷ 4) x 8(DXT1) or 16(DXT2-5)

      U32 sizeMultiple = 0;

      switch(mFormat)
      {
      case GFXFormatDXT1:
         sizeMultiple = 8;
         break;
      case GFXFormatDXT2:
      case GFXFormatDXT3:
      case GFXFormatDXT4:
      case GFXFormatDXT5:
         sizeMultiple = 16;
         break;
      default:
         AssertISV(false, "DDSFile::getSurfaceSize - invalid compressed texture format, we only support DXT1-5 right now.");
         break;
      }

      return getMax(U32(1), width/4) * getMax(U32(1), height/4) * sizeMultiple;
   }
   else
   {
      return height * width* mBytesPerPixel;
   }
}

U32 DDSFile::getSizeInBytes() const
{
   // TODO: This doesn't take mDepth into account, so
   // it doesn't work right for volume or cubemap textures!

   U32 bytes = 0;
   for ( U32 i=0; i < mMipMapCount; i++ )
      bytes += getSurfaceSize( mHeight, mWidth, i );

   return bytes;
}

U32 DDSFile::getSizeInBytes( GFXFormat format, U32 height, U32 width, U32 mipLevels )
{
   AssertFatal( format >= GFXFormatDXT1 && format <= GFXFormatDXT5, 
      "DDSFile::getSizeInBytes - Must be a DXT format!" );

   // From the directX docs:
   // max(1, width ÷ 4) x max(1, height ÷ 4) x 8(DXT1) or 16(DXT2-5)

   U32 sizeMultiple = 0;
   if ( format == GFXFormatDXT1 )
      sizeMultiple = 8;
   else
      sizeMultiple = 16;

   U32 mipHeight, mipWidth; 
   U32 bytes = 0;
   for ( U32 m=0; m < mipLevels; m++ )
   {
      mipHeight = getMax( U32(1), height >> m );
      mipWidth  = getMax( U32(1), width >> m );

      bytes += getMax( U32(1), mipWidth / 4 ) * 
               getMax( U32(1), mipHeight / 4 ) * sizeMultiple;
   }

   return bytes;
}

bool DDSFile::readHeader(Stream &s)
{
   U32 tmp;

   // Read the FOURCC
   s.read(&tmp);

   if(tmp != MakeFourCC('D', 'D', 'S', ' '))
   {
      Con::errorf("DDSFile::readHeader - unexpected magic number, wanted 'DDS '!");
      return false;
   }

   // Read the size of the header.
   s.read(&tmp);

   if(tmp != 124)
   {
      Con::errorf("DDSFile::readHeader - incorrect header size. Expected 124 bytes.");
      return false;
   }

   // Read some flags...
   U32 ddsdFlags;
   s.read(&ddsdFlags);

   // "Always include DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH, DDSD_HEIGHT."
   if(!(ddsdFlags & (DDSDCaps | DDSDPixelFormat | DDSDWidth | DDSDHeight)))
   {
      Con::errorf("DDSFile::readHeader - incorrect surface description flags.");
      return false;
   }

   // Read height and width (always present)
   s.read(&mHeight);
   s.read(&mWidth);

   // Read pitch or linear size.

   // First make sure we have valid flags (either linear size or pitch).
   if((ddsdFlags & (DDSDLinearSize | DDSDPitch)) == (DDSDLinearSize | DDSDPitch))
   {
      // Both are invalid!
      Con::errorf("DDSFile::readHeader - encountered both DDSD_LINEARSIZE and DDSD_PITCH!");
      return false;
   }

   // Ok, some flags are set, so let's do some reading.
   s.read(&mPitchOrLinearSize);

   if(ddsdFlags & DDSDLinearSize)
   {
      mFlags.set(LinearSizeFlag); // ( mHeight / 4 ) * ( mWidth / 4 ) * DDSSIZE
   }
   else if (ddsdFlags & DDSDPitch)
   {
      mFlags.set(PitchSizeFlag); // ( mWidth / 4 ) * DDSSIZE ???
   }
   else
   {
      // Neither set! This appears to be depressingly common.
      // Con::warnf("DDSFile::readHeader - encountered neither DDSD_LINEARSIZE nor DDSD_PITCH!");
   }

   // Do we need to read depth? If so, we are a volume texture!
   s.read(&mDepth);

   if(ddsdFlags & DDSDDepth)
   {
      mFlags.set(VolumeFlag);
   }
   else
   {
      // Wipe it if the flag wasn't set!
      mDepth = 0;
   }

   // Deal with mips!
   s.read(&mMipMapCount);

   if(ddsdFlags & DDSDMipMapCount)
   {
      mFlags.set(MipMapsFlag);
   }
   else
   {
      // Wipe it if the flag wasn't set!
      mMipMapCount = 1;
   }

   // Deal with 11 DWORDS of reserved space (this reserved space brought to
   // you by DirectDraw and the letters F and U).
   for(U32 i=0; i<11; i++)
      s.read(&tmp);

   // Now we're onto the pixel format!
   s.read(&tmp);

   if(tmp != 32)
   {
      Con::errorf("DDSFile::readHeader - pixel format chunk has unexpected size!");
      return false;
   }

   U32 ddpfFlags;

   s.read(&ddpfFlags);

   // Read the next few values so we can deal with them all in one go.
   U32 pfFourCC, pfBitCount, pfRMask, pfGMask, pfBMask, pfAlphaMask;

   s.read(&pfFourCC);
   s.read(&pfBitCount);
   s.read(&pfRMask);
   s.read(&pfGMask);
   s.read(&pfBMask);
   s.read(&pfAlphaMask);

   // Sanity check flags...
   if(!(ddpfFlags & (DDPFRGB | DDPFFourCC | DDPFLUMINANCE)))
   {
      Con::errorf("DDSFile::readHeader - incoherent pixel flags, neither RGB, FourCC, or Luminance!");
      return false;
   }

   // For now let's just dump the header info.
   if(ddpfFlags & DDPFLUMINANCE)
   {
      mFlags.set(RGBData);

      mBytesPerPixel = pfBitCount / 8;      

      bool hasAlpha = ddpfFlags & DDPFAlphaPixels;

      mHasTransparency = hasAlpha;

      // Try to match a format.
      if(hasAlpha)
      {
         // If it has alpha it is one of...
         // GFXFormatA8L8
         // GFXFormatA4L4

         if(pfBitCount == 16)
            mFormat = GFXFormatA8L8;
         else if(pfBitCount == 8)
            mFormat = GFXFormatA4L4;
         else
         {
            Con::errorf("DDSFile::readHeader - unable to match alpha Luminance format!");
            return false;
         }
      }
      else
      {
         // Otherwise it is one of...
         // GFXFormatL16
         // GFXFormatL8

         if(pfBitCount == 16)
            mFormat = GFXFormatL16;
         else if(pfBitCount == 8)
            mFormat = GFXFormatL8;
         else
         {
            Con::errorf("DDSFile::readHeader - unable to match non-alpha Luminance format!");
            return false;
         }
      }
   }
   else if(ddpfFlags & DDPFRGB)
   {
      mFlags.set(RGBData);

      //Con::printf("RGB Pixel format of DDS:");
      //Con::printf("   bitcount = %d (16, 24, 32)", pfBitCount);
      mBytesPerPixel = pfBitCount / 8;
      //Con::printf("   red   mask = %x", pfRMask);
      //Con::printf("   green mask = %x", pfGMask);
      //Con::printf("   blue  mask = %x", pfBMask);

      bool hasAlpha = false;

      if(ddpfFlags & DDPFAlphaPixels)
      {
         hasAlpha = true;
         //Con::printf("   alpha mask = %x", pfAlphaMask);
      }
      else
      {
         //Con::printf("   no alpha.");
      }

      mHasTransparency = hasAlpha;

      // Try to match a format.
      if(hasAlpha)
      {
         // If it has alpha it is one of...
         // GFXFormatR8G8B8A8
         // GFXFormatR5G5B5A1
         // GFXFormatA8

         if(pfBitCount == 32)
            mFormat = GFXFormatR8G8B8A8;
         else if(pfBitCount == 16)
            mFormat = GFXFormatR5G5B5A1;
         else if(pfBitCount == 8)
            mFormat = GFXFormatA8;
         else
         {
            Con::errorf("DDSFile::readHeader - unable to match alpha RGB format!");
            return false;
         }
      }
      else
      {
         // Otherwise it is one of...
         // GFXFormatR8G8B8
         // GFXFormatR8G8B8X8
         // GFXFormatR5G6B5
         // GFXFormatL8

         if(pfBitCount == 24)
            mFormat = GFXFormatR8G8B8;
         else if(pfBitCount == 32)
            mFormat = GFXFormatR8G8B8X8;
         else if(pfBitCount == 16)
            mFormat = GFXFormatR5G6B5;
         else if(pfBitCount == 8)
         {
            // luminance
            mFormat = GFXFormatL8;
         }
         else
         {
            Con::errorf("DDSFile::readHeader - unable to match non-alpha RGB format!");
            return false;
         }
      }


      // Sweet, all done.
   }
   else if (ddpfFlags & DDPFFourCC)
   {
      mHasTransparency = (ddpfFlags & DDPFAlphaPixels);
      mFlags.set(CompressedData);

/*      Con::printf("FourCC Pixel format of DDS:");
      Con::printf("   fourcc = '%c%c%c%c'", ((U8*)&pfFourCC)[0], ((U8*)&pfFourCC)[1], ((U8*)&pfFourCC)[2], ((U8*)&pfFourCC)[3]); */

      // Ok, make a format determination.
      switch(pfFourCC)
      {
      case FOURCC_DXT1:
         mFormat = GFXFormatDXT1;
         break;
      case FOURCC_DXT2:
         mFormat = GFXFormatDXT2;
         break;
      case FOURCC_DXT3:
         mFormat = GFXFormatDXT3;
         break;
      case FOURCC_DXT4:
         mFormat = GFXFormatDXT4;
         break;
      case FOURCC_DXT5:
         mFormat = GFXFormatDXT5;
         break;
      default:
         Con::errorf("DDSFile::readHeader - unknown fourcc = '%c%c%c%c'", ((U8*)&pfFourCC)[0], ((U8*)&pfFourCC)[1], ((U8*)&pfFourCC)[2], ((U8*)&pfFourCC)[3]);
         break;
      }

   }

   // Deal with final caps bits... Is this really necessary?

   U32 caps1, caps2;
   s.read(&caps1);
   s.read(&caps2);
   s.read(&tmp);
   s.read(&tmp); // More icky reserved space.

   // Screw caps1.
   // if(!(caps1 & DDSCAPS_TEXTURE)))
   // {
   // }

   // Caps2 has cubemap/volume info. Care about that.
   if(caps2 & DDSCAPS2Cubemap)
   {
      mFlags.set(CubeMapFlag);

      // Store the face flags too.
      if ( caps2 & DDSCAPS2Cubemap_POSITIVEX ) mFlags.set( CubeMap_PosX_Flag );
      if ( caps2 & DDSCAPS2Cubemap_NEGATIVEX ) mFlags.set( CubeMap_NegX_Flag );
      if ( caps2 & DDSCAPS2Cubemap_POSITIVEY ) mFlags.set( CubeMap_PosY_Flag );
      if ( caps2 & DDSCAPS2Cubemap_NEGATIVEY ) mFlags.set( CubeMap_NegY_Flag );
      if ( caps2 & DDSCAPS2Cubemap_POSITIVEZ ) mFlags.set( CubeMap_PosZ_Flag );
      if ( caps2 & DDSCAPS2Cubemap_NEGATIVEZ ) mFlags.set( CubeMap_NegZ_Flag );
   }

   // MS has ANOTHER reserved word here. This one particularly sucks.
   s.read(&tmp);

   return true;
}

bool DDSFile::read(Stream &s, U32 dropMipCount)
{
   if( !readHeader(s) || mMipMapCount == 0 )
   {
      Con::errorf("DDSFile::read - error reading header!");
      return false;
   }

   // If we're droping mips then make sure we have enough.
   dropMipCount = getMin( dropMipCount, mMipMapCount - 1 );

   // At this point we know what sort of image we contain. So we should
   // allocate some buffers, and read it in.

   // How many surfaces are we talking about?
   if(mFlags.test(CubeMapFlag))
   {
      mSurfaces.setSize( Cubemap_Surface_Count );

      for ( U32 i=0; i < Cubemap_Surface_Count; i++ )
      {
         // Does the cubemap contain this surface?
         if ( mFlags.test( CubeMap_PosX_Flag + ( i << 1 ) ) )
            mSurfaces[i] = new SurfaceData();
         else
         {
            mSurfaces[i] = NULL;
            continue;
         }

         // Load all the mips.
         for(S32 l=0; l<mMipMapCount; l++)
            mSurfaces[i]->readNextMip(this, s, mHeight, mWidth, l, l < dropMipCount );
      }

   }
   else if (mFlags.test(VolumeFlag))
   {
      // Do something with volume
   }
   else
   {
      // It's a plain old texture.

      // First allocate a SurfaceData to stick this in.
      mSurfaces.push_back(new SurfaceData());

      // Load however many mips there are.
      for(S32 i=0; i<mMipMapCount; i++)
         mSurfaces.last()->readNextMip(this, s, mHeight, mWidth, i, i < dropMipCount);

      // Ok, we're done.
   }

   // If we're dropping mips then fix up the stats.
   if ( dropMipCount > 0 )
   {
      // Fix up the pitch and/or linear size.
      if( mFlags.test( LinearSizeFlag ) )
         mPitchOrLinearSize = getSurfaceSize( dropMipCount );
      else if ( mFlags.test( PitchSizeFlag ) )
         mPitchOrLinearSize = getSurfacePitch( dropMipCount );

      // Now fix up the rest of the 
      mMipMapCount = getMax( (U32)1, mMipMapCount - dropMipCount );
      mHeight = getHeight( dropMipCount );
      mWidth = getWidth( dropMipCount );
   }

   return true;
}

bool DDSFile::writeHeader( Stream &s )
{
   // Read the FOURCC
   s.write( 4, "DDS " );

   U32 tmp = 0;

   // Read the size of the header.
   s.write( 124 );

   // Read some flags...
   U32 ddsdFlags = DDSDCaps | DDSDPixelFormat | DDSDWidth | DDSDHeight;
   
   if ( mFlags.test( CompressedData ) )
      ddsdFlags |= DDSDLinearSize;
   else
      ddsdFlags |= DDSDPitch;

   if ( mMipMapCount > 0 )
      ddsdFlags |= DDSDMipMapCount;

   s.write( ddsdFlags );

   // Read height and width (always present)
   s.write( mHeight );
   s.write( mWidth );

   // Ok, some flags are set, so let's do some reading.
   s.write( mPitchOrLinearSize );

   // Do we need to read depth? If so, we are a volume texture!
   s.write( mDepth );

   // Deal with mips!
   s.write( mMipMapCount );

   // Deal with 11 DWORDS of reserved space (this reserved space brought to
   // you by DirectDraw and the letters F and U).
   for(U32 i=0; i<11; i++)
      s.write( tmp ); // is this right?

   // Now we're onto the pixel format!

   // This is the size, in bits,
   // of the pixel format data.
   tmp = 32;
   s.write( tmp );

   U32 ddpfFlags;

   U32 fourCC = 0;

   if ( mFlags.test( CompressedData ) )
   {
      ddpfFlags = DDPFFourCC;
      if (mFormat == GFXFormatDXT1)
         fourCC = FOURCC_DXT1;
      if (mFormat == GFXFormatDXT3)
         fourCC = FOURCC_DXT3;
      if (mFormat == GFXFormatDXT5)
         fourCC = FOURCC_DXT5;
   }
   else
      ddpfFlags = mBytesPerPixel == 4 ? DDPFRGB | DDPFAlphaPixels : DDPFRGB;

   s.write( ddpfFlags );

   // Read the next few values so we can deal with them all in one go.
   //U32 pfFourCC, pfBitCount, pfRMask, pfGMask, pfBMask, pfAlphaMask;

   s.write( fourCC );
   s.write( mBytesPerPixel * 8 );
   s.write( 0x000000FF );
   s.write( 0x00FF0000 );
   s.write( 0x0000FF00 );
   s.write( 0xFF000000 );

   // Deal with final caps bits... Is this really necessary?
 
   U32 caps1 = DDSCAPSTexture;
   if ( mMipMapCount > 0 )
      caps1 |= DDSCAPSComplex | DDSCAPSMipMap;

   tmp = 0;

   s.write( caps1 );
   s.write( tmp );
   s.write( tmp );
   s.write( tmp );// More icky reserved space.

   // MS has ANOTHER reserved word here. This one particularly sucks.
   s.write( tmp );

   return true;
}

bool DDSFile::write( Stream &s )
{
   if(!writeHeader(s))
   {
      Con::errorf("DDSFile::write - error writing header!");
      return false;
   }

   // At this point we know what sort of image we contain. So we should
   // allocate some buffers, and read it in.

   // How many surfaces are we talking about?
   if(mFlags.test(CubeMapFlag))
   {
      // Do something with cubemaps.
   }
   else if (mFlags.test(VolumeFlag))
   {
      // Do something with volume
   }
   else
   {
      // It's a plain old texture.

      // Load however many mips there are.
      for ( S32 i = 0; i < mMipMapCount; i++ )
         mSurfaces.last()->writeNextMip(this, s, mHeight, mWidth, i);

      // Ok, we're done.
   }

   return true;
}

void DDSFile::SurfaceData::dumpImage(DDSFile *dds, U32 mip, const char *file)
{
   GBitmap *foo = new GBitmap(dds->mWidth >> mip, dds->mHeight >> mip, false, dds->mFormat);

   // Copy our data in.
   dMemcpy(foo->getWritableBits(), mMips[mip], dds->getSurfaceSize(dds->mHeight, dds->mWidth, mip) );
   
   FileStream  stream;

   stream.open( file, Torque::FS::File::Write );

   if ( stream.getStatus() == Stream::Ok )
   {
      // Write it out.
      foo->writeBitmap("png", stream);
   }

   // Clean up.
   delete foo;
}

void DDSFile::SurfaceData::readNextMip(DDSFile *dds, Stream &s, U32 height, U32 width, U32 mipLevel, bool skip)
{
   U32 size = dds->getSurfaceSize(height, width, mipLevel);

   // If we're skipping this mip then seek forward.
   if ( skip )
      s.setPosition( s.getPosition() + size );
   else
   {
      mMips.push_back(new U8[size]);
      if(!s.read(size, mMips.last()))
         Con::errorf("DDSFile::SurfaceData::addNextMip - failed to read mip!");
   }
}

void DDSFile::SurfaceData::writeNextMip(DDSFile *dds, Stream &s, U32 height, U32 width, U32 mipLevel)
{
   U32 size = dds->getSurfaceSize(height, width, mipLevel);
   if(!s.write(size, mMips[mipLevel]))
      Con::errorf("DDSFile::SurfaceData::writeNextMip - failed to write mip!");
}

//------------------------------------------------------------------------------

template<> void *Resource<DDSFile>::create( const Torque::Path &path )
{
#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "Resource<DDSFile>::create - [%s]", path.getFullPath().c_str() );
#endif

   FileStream stream;

   stream.open( path.getFullPath(), Torque::FS::File::Read );

   if ( stream.getStatus() != Stream::Ok )
      return NULL;

   DDSFile *retDDS = new DDSFile;

   if( !retDDS->read( stream, DDSFile::smDropMipCount ) )
   {
      delete retDDS;
      return NULL;
   }
   else
   {
      // Set source file name
      retDDS->mSourcePath = path;
      retDDS->mCacheString = Torque::Path::Join( path.getRoot(), ':', path.getPath() );
      retDDS->mCacheString = Torque::Path::Join( retDDS->mCacheString, '/', path.getFileName() );
   }

   return retDDS;
}

template<> ResourceBase::Signature  Resource<DDSFile>::signature()
{
   return MakeFourCC('D','D','S',' '); // Direct Draw Surface
}

Resource<DDSFile> DDSFile::load( const Torque::Path &path, U32 dropMipCount )
{
   PROFILE_SCOPE( DDSFile_load );
   
   // HACK:  It sucks that we cannot pass parameters into 
   // the resource manager loading system.
   DDSFile::smDropMipCount = dropMipCount;
   Resource<DDSFile> ret = ResourceManager::get().load( path );
   DDSFile::smDropMipCount = 0;

   // Any kind of error checking or path stepping can happen here

   return ret;
}

//------------------------------------------------------------------------------

DDSFile *DDSFile::createDDSFileFromGBitmap( const GBitmap *gbmp )
{
   if( gbmp == NULL )
      return NULL;

   DDSFile *ret = new DDSFile;

   // Set up the DDSFile properties that matter. Since this is a GBitmap, there
   // are assumptions that can be made
   ret->mHeight = gbmp->getHeight();
   ret->mWidth = gbmp->getWidth();
   ret->mDepth = 0;
   ret->mFormat = gbmp->getFormat();
   ret->mFlags.set(RGBData);
   ret->mBytesPerPixel = gbmp->getBytesPerPixel();
   ret->mMipMapCount = gbmp->getNumMipLevels();
   ret->mHasTransparency = gbmp->getHasTransparency();

   // ASSUMPTION!!!
   // This _most likely_ does not belong here, but it is safe to assume that if
   // a GBitmap is 24-bit, and it's being converted to a DDS, it is most likely
   // going to be either:
   // a) Uploaded as a 32-bit texture, and just needs to be padded to RGBX
   // b) Uploaded as a compressed format, and needs to be padded to 32-bits anyway
   if( ret->mFormat == GFXFormatR8G8B8 )
   {
      ret->mFormat = GFXFormatR8G8B8X8;
      ret->mBytesPerPixel = 4;
   }

   if( ret->mMipMapCount > 1 )
      ret->mFlags.set(MipMapsFlag);

   // One surface per GBitmap
   ret->mSurfaces.push_back( new SurfaceData() );

   // Load the mips
   for( S32 i = 0; i < ret->mMipMapCount; i++ )
   {
      const U32 mipSz = ret->getSurfaceSize(i);
      ret->mSurfaces.last()->mMips.push_back( new U8[mipSz] );

      U8 *mipMem = ret->mSurfaces.last()->mMips.last();
      
      // If this is a straight copy, just do it, otherwise (ugh)
      if( ret->mFormat == gbmp->getFormat() )
         dMemcpy( mipMem, gbmp->getBits(i), mipSz );
      else
      {
         // Assumption:
         AssertFatal( gbmp->getBytesPerPixel() + 1 == ret->mBytesPerPixel, "Assumption failed, not 24->32 bit straight convert." );

         for( S32 pxl = 0; pxl < gbmp->getWidth(i) * gbmp->getHeight(i); pxl++ )
         {
            U8 *dst = &mipMem[pxl * ret->mBytesPerPixel];
            const U8 *src = &gbmp->getBits(i)[pxl * gbmp->getBytesPerPixel()];
            dMemcpy( dst, src, gbmp->getBytesPerPixel() * sizeof(U8) );
            dst[ret->mBytesPerPixel - 1] = 255;
         } 
      }

      // Uncomment to debug-dump each mip level
      //ret->mSurfaces.last()->dumpImage( ret, i, avar( "%d_Gbmp_xmip%d", ret, i ) );
   }

   return ret;
}

DefineEngineFunction( getActiveDDSFiles, S32, (),,
   "Returns the count of active DDSs files in memory.\n"
   "@ingroup Rendering\n" )
{
   return DDSFile::smActiveCopies;
}