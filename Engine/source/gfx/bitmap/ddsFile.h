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

#ifndef _DDSFILE_H_
#define _DDSFILE_H_

#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif
#ifndef _BITSET_H_
#include "core/bitSet.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

class Stream;
class GBitmap;


struct DDSFile
{
   enum DDSFlags
   {
      ComplexFlag = BIT(0), ///< Indicates this includes a mipchain, cubemap, or
                            ///  volume texture, ie, isn't a plain old bitmap.
      MipMapsFlag = BIT(1), ///< Indicates we have a mipmap chain in the file.
      CubeMapFlag = BIT(2), ///< Indicates we are a cubemap. Requires all six faces.
      VolumeFlag  = BIT(3), ///< Indicates we are a volume texture.

      PitchSizeFlag = BIT(4),  ///< Cue as to how to interpret our pitchlinear value.
      LinearSizeFlag = BIT(5), ///< Cue as to how to interpret our pitchlinear value.

      RGBData        = BIT(6), ///< Indicates that this is straight out RGBA data.
      CompressedData = BIT(7), ///< Indicates that this is compressed or otherwise
                               ///  exotic data.

      /// These are the flags for which cubemap 
      /// surfaces are included in the file.
      CubeMap_PosX_Flag = BIT(8),
      CubeMap_NegX_Flag = BIT(9),
      CubeMap_PosY_Flag = BIT(10),
      CubeMap_NegY_Flag = BIT(11),
      CubeMap_PosZ_Flag = BIT(12),
      CubeMap_NegZ_Flag = BIT(13),
   };

   /// The index into mSurfaces for each 
   /// cubemap face.
   enum
   {
      Cubemap_Surface_PosX,
      Cubemap_Surface_NegX,
      Cubemap_Surface_PosY,
      Cubemap_Surface_NegY,
      Cubemap_Surface_PosZ,
      Cubemap_Surface_NegZ,
      Cubemap_Surface_Count,
   };

   BitSet32    mFlags;
   U32         mHeight;
   U32         mWidth;
   U32         mDepth;
   U32         mPitchOrLinearSize;
   U32         mMipMapCount;

   GFXFormat   mFormat;
   U32         mBytesPerPixel; ///< Ignored if we're a compressed texture.
   U32         mFourCC;
   String      mCacheString;
   Torque::Path mSourcePath;

   bool        mHasTransparency;

   // This is ugly... but it allows us to pass the number of
   // mips to drop into the ResourceManager loading process.
   static U32 smDropMipCount;

   struct SurfaceData
   {
      SurfaceData()
      {
         VECTOR_SET_ASSOCIATION( mMips );
      }

      ~SurfaceData()
      {
         // Free our mips!
         for(S32 i=0; i<mMips.size(); i++)
            delete[] mMips[i];
      }

      Vector<U8*> mMips;

      // Helper function to read in a mipchain.
      bool readMipChain();

      void dumpImage(DDSFile *dds, U32 mip, const char *file);
      
      /// Helper for reading a mip level.
      void readNextMip(DDSFile *dds, Stream &s, U32 height, U32 width, U32 mipLevel, bool skip);
      
      /// Helper for writing a mip level.
      void writeNextMip(DDSFile *dds, Stream &s, U32 height, U32 width, U32 mipLevel);
   };
   
   Vector<SurfaceData*> mSurfaces;

   /// Clear all our information; used before reading.
   void clear();

   /// Reads a DDS file from the stream.
   bool read(Stream &s, U32 dropMipCount);

   /// Called from read() to read in the DDS header.
   bool readHeader(Stream &s);

   /// Writes this DDS file to the stream.
   bool write(Stream &s);

   /// Called from write() to write the DDS header.
   bool writeHeader(Stream &s);

   /// For our current format etc., what is the size of a surface with the
   /// given dimensions?
   U32 getSurfaceSize( U32 mipLevel = 0 ) const { return getSurfaceSize( mHeight, mWidth, mipLevel ); }
   U32 getSurfaceSize( U32 height, U32 width, U32 mipLevel = 0 ) const;

   // Helper for getting the size in bytes of a compressed DDS texture.
   static U32 getSizeInBytes( GFXFormat format, U32 height, U32 width, U32 mipLevels );

   /// Returns the total video memory size of the texture
   /// including all mipmaps and compression settings.
   U32 getSizeInBytes() const;

   U32 getWidth( U32 mipLevel = 0 ) const { return getMax( U32(1), mWidth >> mipLevel ); }
   U32 getHeight( U32 mipLevel = 0 ) const { return getMax(U32(1), mHeight >> mipLevel); }
   U32 getDepth( U32 mipLevel = 0 ) const { return getMax(U32(1), mDepth >> mipLevel); }

   U32 getMipLevels() const { return mMipMapCount; }

   bool getHasTransparency() const { return mHasTransparency; }

   bool isCubemap() const { return mFlags.test( CubeMapFlag ); }

   GFXFormat getFormat() const { return mFormat; }

   U32 getSurfacePitch( U32 mipLevel = 0 ) const;

   const Torque::Path &getSourcePath() const { return mSourcePath; }
   const String &getTextureCacheString() const { return mCacheString; }

   static Resource<DDSFile> load( const Torque::Path &path, U32 dropMipCount );

   // For debugging fun!
   static S32 smActiveCopies;

   DDSFile()
   {
      VECTOR_SET_ASSOCIATION( mSurfaces );
      smActiveCopies++;

      mHasTransparency = false;
   }

   DDSFile( const DDSFile &dds );

   ~DDSFile()
   {
      smActiveCopies--;

      // Free our surfaces!
      for(S32 i=0; i<mSurfaces.size(); i++)
         delete mSurfaces[i];

      mSurfaces.clear();
   }

   static DDSFile *createDDSFileFromGBitmap( const GBitmap *gbmp );
};

#endif // _DDSFILE_H_