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

#ifndef _GBITMAP_H_
#define _GBITMAP_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

#ifndef _SWIZZLE_H_
#include "core/util/swizzle.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h" // For the format
#endif

//-------------------------------------- Forward decls.
class Stream;
class RectI;
class Point2I;
class ColorI;
class ColorF;

//------------------------------------------------------------------------------
//-------------------------------------- GBitmap

class GBitmap
{
public:
   enum Constants
   {
      /// The maximum mipmap levels we support.  The current
      /// value lets us support up to 4096 x 4096 images.
      c_maxMipLevels = 13 
   };

   struct Registration
   {
      /// The read function prototype.
      typedef bool(*ReadFunc)(Stream &stream, GBitmap *bitmap);

      /// The write function prototype.  Compression levels are image-specific - see their registration declaration for details.
      typedef bool(*WriteFunc)(GBitmap *bitmap, Stream &stream, U32 compressionLevel);

      /// Used to sort the registrations so that 
      /// lookups occur in a fixed order.
      U32 priority;

      Vector<String>   extensions;     ///< the list of file extensions for this bitmap type [these should be lower case]
      
      ReadFunc    readFunc;            ///< the read function to call for this bitmap type
      WriteFunc   writeFunc;           ///< the write function to call for this bitmap type
      U32         defaultCompression;  ///< the default compression level [levels are image-specific - see their registration declaration for details]

      Registration()
      {
         priority = 0;
         VECTOR_SET_ASSOCIATION( extensions );
      }
   };

   /// Load the given bitmap file.  It will try known file
   /// extensions if one is not specified.  If all else fails
   /// it will look up the folder hierarchy for a match.
   ///
   /// Important: Don't do something like this...
   ///
   /// @code
   ///    GBitmap* bitmap; // WRONG TYPE!
   ///    bitmap = GBitmap::load( filename );
   /// @endcode
   ///
   /// Resources are reference-counted and the smart pointer conversion will
   /// release the bitmap and thus render the resulting bitmap pointer invalid!
   /// The right way is like this:
   ///
   /// @code
   ///    Resource<GBitmap> bitmap; // Correct!
   ///    bitmap = GBitmap::load( filename );
   /// @endcode
   ///
   static Resource<GBitmap> load(const Torque::Path &path);

protected:

   static Resource<GBitmap> _load(const Torque::Path &path);
   static Resource<GBitmap> _search(const Torque::Path &path);

public:
   GBitmap();
   GBitmap(const GBitmap&);

   GBitmap(const U32  in_width,
           const U32  in_height,
           const bool in_extrudeMipLevels = false,
           const GFXFormat in_format = GFXFormatR8G8B8 );

   // This builds a GBitmap with the R8G8B8A8 format using the passed in
   // data (assumes that there is width * height * 4 U8's in data)
   GBitmap(const U32  in_width,
           const U32  in_height,
           const U8*  data );

   virtual ~GBitmap();


   static void sRegisterFormat( const Registration &reg );
   static const Registration* sFindRegInfo( const String &extension );   
   
   /// Find the first file matching the registered extensions 
   /// skipping the original.
   static bool sFindFile( const Torque::Path &path, Torque::Path *outPath );

   /// Given a path to a file, try all known extensions.  If the file exists on disk, fill in path
   /// with the correct extension and return true.  Otherwise, return false.
   static bool sFindFiles( const Torque::Path &path, Vector<Torque::Path> *outFoundPaths );

   /// Returns a space separated string of all registered extensions.
   static String sGetExtensionList();

   void allocateBitmap(const U32  in_width,
                       const U32  in_height,
                       const bool in_extrudeMipLevels = false,
                       const GFXFormat in_format = GFXFormatR8G8B8 );

   void extrudeMipLevels(bool clearBorders = false);
   void extrudeMipLevelsDetail();

   U32   getNumMipLevels() const { return mNumMipLevels; }

   GBitmap *createPaddedBitmap() const;
   GBitmap *createPow2Bitmap() const;

   /// Copies a color channel by index into the first channel 
   /// of the output bitmap.  The output bitmap must be the same
   /// dimensions as the source.
   void copyChannel( U32 index, GBitmap *outBitmap ) const;
   
   void copyRect(const GBitmap *in, const RectI &srcRect, const Point2I &dstPoint, const U32 srcMipLevel = 0, const U32 dstMipLevel = 0);

   GFXFormat   getFormat() const { return mInternalFormat; }
   bool        setFormat(GFXFormat fmt);

   U32         getWidth(const U32 in_mipLevel  = 0) const;
   U32         getHeight(const U32 in_mipLevel = 0) const;
   U32         getDepth(const U32 in_mipLevel = 0) const;

   U8*         getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel = 0);
   const U8*   getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel = 0) const;

   const U8*   getBits(const U32 in_mipLevel = 0) const;
   U8*         getWritableBits(const U32 in_mipLevel = 0);

   U32         getByteSize() const { return mByteSize; }
   U32         getBytesPerPixel() const { return mBytesPerPixel; }

   /// Use these functions to set and get the mHasTransparency value
   /// This is used to indicate that this bitmap has pixels that have
   /// an alpha value less than 255 (used by the auto-Material mapper)
   bool        getHasTransparency() const { return mHasTransparency; }
   void        setHasTransparency(bool hasTransparency) { mHasTransparency = hasTransparency; }
   
   /// In general you will want to use this function if there is not a
   /// good spot in the bitmap loader(s) to check the alpha value of
   /// the pixels. This function uses the texture format to loop over
   /// the bitmap bits and to check for alpha values less than 255
   bool        checkForTransparency();

   ColorF      sampleTexel(F32 u, F32 v) const;
   bool        getColor(const U32 x, const U32 y, ColorI& rColor) const;
   bool        setColor(const U32 x, const U32 y, const ColorI& rColor);

   /// This method will combine bitmapA and bitmapB using the operation specified
   /// by combineOp. The result will be stored in the bitmap that this method is
   /// called on. The size of the resulting bitmap will be the larger of A and B.
   /// The format of the resulting bitmap will be the format of A or B, whichever
   /// has a larger byte size.
   ///
   /// @note There are some restrictions on ops and formats that will probably change
   /// based on how we use this function.
   bool combine( const GBitmap *bitmapA, const GBitmap *bitmapB, const GFXTextureOp combineOp );

   /// Fills the first mip level of the bitmap with the specified color.
   void fill( const ColorI &rColor );

   /// An optimized version of fill().
   void fillWhite();

   //-------------------------------------- Internal data/operators

   void deleteImage();

   //-------------------------------------- Input/Output interface

   /// Read a bitmap from a stream
   /// @param bmType This is a file extension to describe the type of the data [i.e. "png" for PNG file, etc]
   /// @param ioStream The stream to read from
   bool  readBitmap( const String &bmType, Stream &ioStream );

   /// Write a bitmap to a stream
   /// @param bmType This is a file extension to describe the type of the data [i.e. "png" for PNG file, etc]
   /// @param ioStream The stream to read from
   /// @param compressionLevel Image format-specific compression level.  If set to U32_MAX, we use the default compression defined when the format was registered.
   bool  writeBitmap( const String &bmType, Stream &ioStream, U32 compressionLevel = U32_MAX );

   bool readMNG(Stream& io_rStream);               // located in bitmapMng.cc
   bool writeMNG(Stream& io_rStream) const;

   bool read(Stream& io_rStream);
   bool write(Stream& io_rStream) const;

   template<class T, dsize_t mapLength>
   void swizzle(const Swizzle<T,mapLength> *s);

   static Vector<Registration>   sRegistrations;

private:
   GFXFormat mInternalFormat;

   U8* mBits; // Master bytes
   U32 mByteSize;
   U32 mWidth;
   U32 mHeight;
   U32 mBytesPerPixel;

   U32 mNumMipLevels;
   U32 mMipLevelOffsets[c_maxMipLevels];

   bool mHasTransparency;

   static const U32 csFileVersion;
};

//------------------------------------------------------------------------------
//-------------------------------------- Inlines
//

inline U32 GBitmap::getWidth(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getWidth: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   U32 retVal = mWidth >> in_mipLevel;

   return (retVal != 0) ? retVal : 1;
}

inline U32 GBitmap::getHeight(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("Bitmap::getHeight: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   U32 retVal = mHeight >> in_mipLevel;

   return (retVal != 0) ? retVal : 1;
}

inline const U8* GBitmap::getBits(const U32 in_mipLevel) const
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getBits: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   return &mBits[mMipLevelOffsets[in_mipLevel]];
}

inline U8* GBitmap::getWritableBits(const U32 in_mipLevel)
{
   AssertFatal(in_mipLevel < mNumMipLevels,
               avar("GBitmap::getWritableBits: mip level out of range: (%d, %d)",
                    in_mipLevel, mNumMipLevels));

   return &mBits[mMipLevelOffsets[in_mipLevel]];
}

inline U8* GBitmap::getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel)
{
   return (getWritableBits(mipLevel) + ((in_y * getWidth(mipLevel)) + in_x) * mBytesPerPixel);
}

inline const U8* GBitmap::getAddress(const S32 in_x, const S32 in_y, const U32 mipLevel) const
{
   return (getBits(mipLevel) + ((in_y * getWidth(mipLevel)) + in_x) * mBytesPerPixel);
}

template<class T, dsize_t mapLength>
void GBitmap::swizzle(const Swizzle<T,mapLength> *s )
{
   const U32 memSize = getWidth() * getHeight() * mBytesPerPixel;

   void *b = dMalloc(memSize);

   s->ToBuffer(b, getWritableBits(), memSize);

   dMemcpy(getWritableBits(), b, memSize);

   dFree(b);
}

#endif //_GBITMAP_H_
