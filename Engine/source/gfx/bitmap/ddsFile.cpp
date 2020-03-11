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
#include "gfx/bitmap/ddsData.h"
#include "gfx/bitmap/bitmapUtils.h"
#include "gfx/bitmap/imageUtils.h"
#include "gfx/gfxDevice.h"
#include "core/util/fourcc.h"
#include "console/console.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "gfx/bitmap/gBitmap.h"
#include "console/engineAPI.h"

#include <squish.h>

S32 DDSFile::smActiveCopies = 0;
U32 DDSFile::smDropMipCount = 0;

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
      case GFXFormatBC1:
      case GFXFormatBC4:
         sizeMultiple = 8;
         break;
      case GFXFormatBC2:
      case GFXFormatBC3:      
      case GFXFormatBC5:
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
      case GFXFormatBC1:
      case GFXFormatBC4:
         sizeMultiple = 8;
         break;
      case GFXFormatBC2:
      case GFXFormatBC3:      
      case GFXFormatBC5:
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
   // it doesn't work right for volume textures!

   U32 bytes = 0;
   if (mFlags.test(CubeMapFlag))
   {
      for(U32 cubeFace=0;cubeFace < Cubemap_Surface_Count;cubeFace++)
         for (U32 i = 0; i < mMipMapCount; i++)
            bytes += getSurfaceSize(mHeight, mWidth, i);
   }
   else
   {
      for (U32 i = 0; i < mMipMapCount; i++)
         bytes += getSurfaceSize(mHeight, mWidth, i);
   }

   return bytes;
}

U32 DDSFile::getSizeInBytes( GFXFormat format, U32 height, U32 width, U32 mipLevels )
{
   AssertFatal( ImageUtil::isCompressedFormat(format), 
      "DDSFile::getSizeInBytes - Must be a Block Compression format!" );

   // From the directX docs:
   // max(1, width ÷ 4) x max(1, height ÷ 4) x 8(DXT1) or 16(DXT2-5)

   U32 sizeMultiple = 0;
   if ( format == GFXFormatBC1 || format == GFXFormatBC1_SRGB || format == GFXFormatBC4)
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
   U32 fourcc;
   // Read the FOURCC
   s.read(&fourcc);

   if(fourcc != DDS_MAGIC)
   {
      Con::errorf("DDSFile::readHeader - unexpected magic number, wanted 'DDS '!");
      return false;
   }

   //dds headers
   dds::DDS_HEADER header = {};
   dds::DDS_HEADER_DXT10 dx10header = {};
   //todo DX10 formats
   bool hasDx10Header = false;

   //read in header
   s.read(DDS_HEADER_SIZE, &header);
   //check for dx10 header support
   if ((header.ddspf.flags & DDS_FOURCC) && (header.ddspf.fourCC == dds::D3DFMT_DX10))
   {
      //read in dx10 header
      s.read(DDS_HEADER_DX10_SIZE, &dx10header);
      if (!dds::validateHeaderDx10(dx10header))
         return false;

      hasDx10Header = true;
   }

   //make sure our dds header is valid
   if (!dds::validateHeader(header))
      return false;

   // store details
   mPitchOrLinearSize = header.pitchOrLinearSize;
   mMipMapCount = header.mipMapCount ? header.mipMapCount : 1;
   mHeight = header.height;
   mWidth = header.width;
   mDepth = header.depth;
   mFourCC = header.ddspf.fourCC;

   //process dx10 header
   if (hasDx10Header)
   {
      if (dx10header.arraySize > 1)
      {
         Con::errorf("DDSFile::readHeader - DX10 arrays not supported");
         return false;
      }

      mFormat = dds::getGFXFormat(dx10header.dxgiFormat);
      //make sure getGFXFormat gave us a valid format
      if (mFormat == GFXFormat_FIRST)
         return false;
      //cubemap
      if (dx10header.miscFlag & dds::D3D10_RESOURCE_MISC_TEXTURECUBE)
      {
         mFlags.set(CubeMap_All_Flags | ComplexFlag);
      }

      mHasTransparency = ImageUtil::isAlphaFormat(mFormat);

      //mip map flag
      if (mMipMapCount > 1)
         mFlags.set(MipMapsFlag | ComplexFlag);

      if (ImageUtil::isCompressedFormat(mFormat))
         mFlags.set(CompressedData);
      else
      {
         mBytesPerPixel = header.ddspf.bpp / 8;
         mFlags.set(RGBData);
      }

      // we finished now
      return true;
   }

   //process regular header

   //D3DFMT_DX10 is caught above, no need to check now
   if (header.ddspf.flags & DDS_FOURCC)
   {
      mFormat = dds::getGFXFormat(mFourCC);
      //make sure getGFXFormat gave us a valid format
      if (mFormat == GFXFormat_FIRST)
         return false;

      if (ImageUtil::isCompressedFormat(mFormat))
         mFlags.set(CompressedData);
      else
      {
         mBytesPerPixel = header.ddspf.bpp / 8;
         mFlags.set(RGBData);
      }
   }
   else
   {
      mFormat = dds::getGFXFormat(header.ddspf);
      //make sure getGFXFormat gave us a valid format
      if (mFormat == GFXFormat_FIRST)
         return false;

      mBytesPerPixel = header.ddspf.bpp / 8;
      mFlags.set(RGBData);
   }

   //mip map flag
   if (mMipMapCount > 1)
      mFlags.set(MipMapsFlag | ComplexFlag);

   //set transparency flag
   mHasTransparency = (header.ddspf.flags & DDS_ALPHAPIXELS);

   if (header.flags & DDS_HEADER_FLAGS_LINEARSIZE)
      mFlags.set(LinearSizeFlag);
   else if (header.flags & DDS_HEADER_FLAGS_PITCH)
      mFlags.set(PitchSizeFlag);

   //set cubemap flags
   if (header.cubemapFlags & DDS_CUBEMAP)
   {
      mFlags.set(CubeMapFlag | ComplexFlag);
      // Store the face flags too.
      if (header.cubemapFlags & DDS_CUBEMAP_POSITIVEX) mFlags.set(CubeMap_PosX_Flag);
      if (header.cubemapFlags & DDS_CUBEMAP_NEGATIVEX) mFlags.set(CubeMap_NegX_Flag);
      if (header.cubemapFlags & DDS_CUBEMAP_POSITIVEY) mFlags.set(CubeMap_PosY_Flag);
      if (header.cubemapFlags & DDS_CUBEMAP_NEGATIVEY) mFlags.set(CubeMap_NegY_Flag);
      if (header.cubemapFlags & DDS_CUBEMAP_POSITIVEZ) mFlags.set(CubeMap_PosZ_Flag);
      if (header.cubemapFlags & DDS_CUBEMAP_NEGATIVEZ) mFlags.set(CubeMap_NegZ_Flag);
   }



   return true;
}

bool DDSFile::read(Stream &s, U32 dropMipCount)
{
   if( !readHeader(s) )
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
   // write DDS magic
   U32 magic = DDS_MAGIC;
   s.write(magic);

   dds::DDS_HEADER header = {};
   dds::DDS_HEADER_DXT10 dx10header = {};

   bool hasDx10Header = false;
   //flags
   U32 surfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
   U32 cubemapFlags = 0;
   U32 headerFlags = DDS_HEADER_FLAGS_TEXTURE;

   //pixel format
   const dds::DDS_PIXELFORMAT &format = dds::getDDSFormat(mFormat);

   // todo better dx10 support
   if (format.fourCC == dds::D3DFMT_DX10)
   {
      dx10header.dxgiFormat = dds::getDXGIFormat(mFormat);
      dx10header.arraySize = 1;
      dx10header.resourceDimension = dds::D3D10_RESOURCE_DIMENSION_TEXTURE2D;
      dx10header.miscFlag = 0;
      dx10header.miscFlags2 = 0;
      hasDx10Header = true;
   }

   if (mFlags.test(CompressedData))
      headerFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
   else
      headerFlags |= DDS_HEADER_FLAGS_PITCH;

   if (mMipMapCount > 1)
   {
      surfaceFlags |= DDS_SURFACE_FLAGS_MIPMAP;
      headerFlags |= DDS_HEADER_FLAGS_MIPMAP;
   }   

   //cubemap flags
   if (mFlags.test(CubeMapFlag))
   {
      surfaceFlags |= DDS_SURFACE_FLAGS_CUBEMAP;
      cubemapFlags |= DDS_CUBEMAP_ALLFACES;
   }

   //volume texture
   if (mDepth > 0)
   {
      headerFlags |= DDS_HEADER_FLAGS_VOLUME;
      dx10header.resourceDimension = dds::D3D10_RESOURCE_DIMENSION_TEXTURE3D;
   }

   //main dds header
   header.size = sizeof(dds::DDS_HEADER);
   header.flags = headerFlags;
   header.height = mHeight;
   header.width = mWidth;
   header.pitchOrLinearSize = mPitchOrLinearSize;
   header.depth = mDepth;
   header.ddspf = format;
   header.mipMapCount = mMipMapCount;
   header.surfaceFlags = surfaceFlags;   
   header.cubemapFlags = cubemapFlags;   
   memset(header.reserved1, 0, sizeof(header.reserved1));
   memset(header.reserved2, 0, sizeof(header.reserved2));

   //check our header is ok
   if (!dds::validateHeader(header))
      return false;

   //Write out the header
   s.write(DDS_HEADER_SIZE, &header);
   
   //Write out dx10 header
   if (hasDx10Header)
      s.write(DDS_HEADER_DX10_SIZE, &dx10header);

   return true;
}

bool DDSFile::write( Stream &s )
{
   if(!writeHeader(s))
   {
      Con::errorf("DDSFile::write - error writing header!");
      return false;
   }

   // How many surfaces are we talking about?
   if(mFlags.test(CubeMapFlag))
   {
      // Do something with cubemaps.
      for (U32 cubeFace = 0; cubeFace < Cubemap_Surface_Count; cubeFace++)
      {
         // write the mips
         for (S32 i = 0; i < mMipMapCount; i++)
            mSurfaces[cubeFace]->writeNextMip(this, s, mHeight, mWidth, i);
      }
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

DDSFile *DDSFile::createDDSCubemapFileFromGBitmaps(GBitmap **gbmps)
{
   if (gbmps == NULL)
      return NULL;

   AssertFatal(gbmps[0], "createDDSCubemapFileFromGBitmaps bitmap 0 is null");
   AssertFatal(gbmps[1], "createDDSCubemapFileFromGBitmaps bitmap 1 is null");
   AssertFatal(gbmps[2], "createDDSCubemapFileFromGBitmaps bitmap 2 is null");
   AssertFatal(gbmps[3], "createDDSCubemapFileFromGBitmaps bitmap 3 is null");
   AssertFatal(gbmps[4], "createDDSCubemapFileFromGBitmaps bitmap 4 is null");
   AssertFatal(gbmps[5], "createDDSCubemapFileFromGBitmaps bitmap 5 is null");

   DDSFile *ret = new DDSFile;
   //all cubemaps have the same dimensions and formats
   GBitmap *pBitmap = gbmps[0];

   if (pBitmap->getFormat() != GFXFormatR8G8B8A8)
   {
      Con::errorf("createDDSCubemapFileFromGBitmaps: Only GFXFormatR8G8B8A8 supported for now");
      return NULL;
   }

   // Set up the DDSFile properties that matter. Since this is a GBitmap, there
   // are assumptions that can be made
   ret->mHeight = pBitmap->getHeight();
   ret->mWidth = pBitmap->getWidth();
   ret->mDepth = 0;
   ret->mFormat = pBitmap->getFormat();
   ret->mFlags.set( RGBData | CubeMapFlag | CubeMap_PosX_Flag | CubeMap_NegX_Flag | CubeMap_PosY_Flag |
      CubeMap_NegY_Flag | CubeMap_PosZ_Flag | CubeMap_NegZ_Flag);
   ret->mBytesPerPixel = pBitmap->getBytesPerPixel();
   //todo implement mip mapping
   ret->mMipMapCount = pBitmap->getNumMipLevels();
   ret->mHasTransparency = pBitmap->getHasTransparency();

   for (U32 cubeFace = 0; cubeFace < Cubemap_Surface_Count; cubeFace++)
   {
      ret->mSurfaces.push_back(new SurfaceData());
      // Load the mips
      for (S32 i = 0; i < ret->mMipMapCount; i++)
      {
         const U32 mipSz = ret->getSurfaceSize(i);
         ret->mSurfaces.last()->mMips.push_back(new U8[mipSz]);

         U8 *mipMem = ret->mSurfaces.last()->mMips.last();
         //straight copy
         dMemcpy(mipMem, gbmps[cubeFace]->getBits(i), mipSz);
      }
   }

   return ret;
}

bool DDSFile::decompressToGBitmap(GBitmap *dest)
{
   // TBD: do we support other formats?
   if (mFormat != GFXFormatBC1 && mFormat != GFXFormatBC2 && mFormat != GFXFormatBC3)
      return false;

   dest->allocateBitmapWithMips(getWidth(), getHeight(), getMipLevels(), GFXFormatR8G8B8A8);

   // Decompress and copy mips...

   U32 numMips = getMipLevels();

   for (U32 i = 0; i < numMips; i++)
   {
      U8 *addr = dest->getAddress(0, 0, i);
      const U8 *mipBuffer = mSurfaces[0]->mMips[i];
      ImageUtil::decompress(mipBuffer, addr, getWidth(i), getHeight(i), mFormat);

   }

   return true;
}

DefineEngineFunction( getActiveDDSFiles, S32, (),,
   "Returns the count of active DDSs files in memory.\n"
   "@ingroup Rendering\n" )
{
   return DDSFile::smActiveCopies;
}