//-----------------------------------------------------------------------------
// Copyright (c) 2016 GarageGames, LLC
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
#include "gfx/bitmap/imageUtils.h"
#include "gfx/bitmap/ddsFile.h"
#include "platform/threads/threadPool.h"
#include "squish/squish.h"

namespace ImageUtil
{
   // get squish quality flag
   S32 _getSquishQuality(const CompressQuality quality)
   {
      switch (quality)
      {
      case LowQuality:
         return squish::kColourRangeFit;
      case MediumQuality:
         return squish::kColourClusterFit;
      case HighQuality:
         return squish::kColourIterativeClusterFit;
      default:
         return squish::kColourRangeFit;//default is low quality
      }
   }

   // get squish compression flag
   S32 _getSquishFormat(const GFXFormat compressFormat)
   {
      switch (compressFormat)
      {
      case GFXFormatBC1:
         return squish::kDxt1;
      case GFXFormatBC2:
         return squish::kDxt3;
      case GFXFormatBC3:
         return squish::kDxt5;
      case GFXFormatBC4:
         return squish::kBc4;
      case GFXFormatBC5:
         return squish::kBc5;
      default:
         return squish::kDxt1;
      }
   }

   //Thread work job for compression
   struct CompressJob : public ThreadPool::WorkItem
   {
      S32 width;
      S32 height;
      S32 flags;
      const U8 *pSrc;
      U8 *pDst;
      GFXFormat format;
      CompressQuality quality;

      CompressJob(const U8 *srcRGBA, U8 *dst, const S32 w, const S32 h, const GFXFormat compressFormat, const CompressQuality compressQuality)
         : pSrc(srcRGBA),pDst(dst), width(w), height(h), format(compressFormat),quality(compressQuality) {}

   protected:
      virtual void execute()
      {
         rawCompress(pSrc,pDst, width, height, format,quality);
      }
   };


   // compress raw pixel data, expects rgba format
   bool rawCompress(const U8 *srcRGBA, U8 *dst, const S32 width, const S32 height, const GFXFormat compressFormat, const CompressQuality compressQuality)
   {
      if (!isCompressedFormat(compressFormat))
         return false;

      S32 squishFlags = _getSquishQuality(compressQuality);
      S32 squishFormat = _getSquishFormat(compressFormat);

      squishFlags |= squishFormat;

      squish::CompressImage(srcRGBA, width,height,dst,squishFlags);

      return true;
   }

   // compress DDSFile
   bool ddsCompress(DDSFile *srcDDS, const GFXFormat compressFormat,const CompressQuality compressQuality)
   {
      if (srcDDS->mBytesPerPixel != 4)
      {
         Con::errorf("ImageCompress::ddsCompress: data must be 32bit");
         return false;
      }

      //can't compress DDSFile if it is already compressed
      if (ImageUtil::isCompressedFormat(srcDDS->mFormat))
      {
         Con::errorf("ImageCompress::ddsCompress: file is already compressed");
         return false;
      }

      const bool cubemap = srcDDS->mFlags.test(DDSFile::CubeMapFlag);
      const U32 mipCount = srcDDS->mMipMapCount;
      // We got this far, so assume we can finish (gosh I hope so)
      srcDDS->mFormat = compressFormat;
      srcDDS->mFlags.set(DDSFile::CompressedData);

      //grab global thread pool
      ThreadPool* pThreadPool = &ThreadPool::GLOBAL();

      if (cubemap)
      {
         static U32 nCubeFaces = 6;
         Vector<U8*> dstDataStore;
         dstDataStore.setSize(nCubeFaces * mipCount);

         for (S32 cubeFace = 0; cubeFace < nCubeFaces; cubeFace++)
         {
            DDSFile::SurfaceData *srcSurface = srcDDS->mSurfaces[cubeFace];
            for (U32 currentMip = 0; currentMip < mipCount; currentMip++)
            {
               const U32 dataIndex = cubeFace * mipCount + currentMip;
               const U8 *srcBits = srcSurface->mMips[currentMip];
               const U32 mipSz = srcDDS->getSurfaceSize(currentMip);
               U8 *dstBits = new U8[mipSz];
               dstDataStore[dataIndex] = dstBits;

               ThreadSafeRef<CompressJob> item(new CompressJob(srcBits, dstBits, srcDDS->getWidth(currentMip), srcDDS->getHeight(currentMip), compressFormat, compressQuality));
               pThreadPool->queueWorkItem(item);

            }
         }

         //wait for work items to finish
         pThreadPool->waitForAllItems();

         for (S32 cubeFace = 0; cubeFace < nCubeFaces; cubeFace++)
         {
            DDSFile::SurfaceData *pSrcSurface = srcDDS->mSurfaces[cubeFace];
            for (U32 currentMip = 0; currentMip < mipCount; currentMip++)
            {
               const U32 dataIndex = cubeFace * mipCount + currentMip;
               delete[] pSrcSurface->mMips[currentMip];
               pSrcSurface->mMips[currentMip] = dstDataStore[dataIndex];
            }
         }
      }
      else
      {
         // The source surface is the original surface of the file
         DDSFile::SurfaceData *pSrcSurface = srcDDS->mSurfaces.last();

         // Create a new surface, this will be the DXT compressed surface. Once we
         // are done, we can discard the old surface, and replace it with this one.
         DDSFile::SurfaceData *pNewSurface = new DDSFile::SurfaceData();
         //no point using threading if only 1 mip
         const bool useThreading = bool(mipCount > 1);
         for (U32 currentMip = 0; currentMip < mipCount; currentMip++)
         {
            const U8 *pSrcBits = pSrcSurface->mMips[currentMip];

            const U32 mipSz = srcDDS->getSurfaceSize(currentMip);
            U8 *pDstBits = new U8[mipSz];
            pNewSurface->mMips.push_back(pDstBits);

            if (useThreading)
            {
               // Create CompressJob item
               ThreadSafeRef<CompressJob> item(new CompressJob(pSrcBits, pDstBits, srcDDS->getWidth(currentMip), srcDDS->getHeight(currentMip), compressFormat, compressQuality));
               pThreadPool->queueWorkItem(item);
            }
            else
               rawCompress(pSrcBits, pDstBits, srcDDS->getWidth(currentMip), srcDDS->getHeight(currentMip), compressFormat, compressQuality);

         }
         //block and wait for CompressJobs to finish
         if(useThreading)
            pThreadPool->waitForAllItems();

         // Now delete the source surface and replace with new compressed surface
         srcDDS->mSurfaces.pop_back();
         delete pSrcSurface;
         srcDDS->mSurfaces.push_back(pNewSurface);
      }

      return true;
   }

   bool decompress(const U8 *src, U8 *dstRGBA,const S32 width,const S32 height, const GFXFormat srcFormat)
   {
      if (!isCompressedFormat(srcFormat))
         return false;

      S32 squishFlag = _getSquishFormat(srcFormat);
      squish::DecompressImage(dstRGBA, width, height, src, squishFlag);  

      return true;
   }

   void swizzleDDS(DDSFile *srcDDS, const Swizzle<U8, 4> &swizzle)
   {
      if (srcDDS->mFlags.test(DDSFile::CubeMapFlag))
      {
         for (S32 cubeFace = 0; cubeFace < DDSFile::Cubemap_Surface_Count; cubeFace++)
         {
            for (S32 i = 0; i < srcDDS->mMipMapCount; i++)
            {
               swizzle.InPlace(srcDDS->mSurfaces[cubeFace]->mMips[i], srcDDS->getSurfaceSize(i));
            }
         }
      }
      else
      {
         for (S32 i = 0; i < srcDDS->mMipMapCount; i++)
         {
            swizzle.InPlace(srcDDS->mSurfaces.last()->mMips[i], srcDDS->getSurfaceSize(i));
         }
      }
   }

   bool isCompressedFormat(const GFXFormat format)
   {
      if (format >= GFXFormatBC1 && format <= GFXFormatBC3_SRGB)
         return true;
      else
         return false;
   }

   bool isAlphaFormat(const GFXFormat format)
   {
      switch (format)
      {
         case GFXFormatA8:
         case GFXFormatA4L4:
         case GFXFormatA8L8:
         case GFXFormatR5G5B5A1:
         case GFXFormatR8G8B8A8:
         case GFXFormatB8G8R8A8:
         case GFXFormatR16G16B16A16F:
         case GFXFormatR32G32B32A32F:
         case GFXFormatR10G10B10A2:
         //case GFXFormatBC1://todo BC1 can store alpha
         case GFXFormatBC2:
         case GFXFormatBC3:
            return true;
         default:
            return false;
      }
   }

   bool isSRGBFormat(const GFXFormat format)
   {
      switch (format)
      {
      case GFXFormatR8G8B8_SRGB:
      case GFXFormatR8G8B8A8_SRGB:
      case GFXFormatBC1_SRGB:
      case GFXFormatBC2_SRGB:
      case GFXFormatBC3_SRGB:
         return true;
      default:
         return false;
      };
   }

   GFXFormat toSRGBFormat(const GFXFormat format)
   {
      switch (format)
      {
      case GFXFormatR8G8B8:
         return GFXFormatR8G8B8_SRGB;
      case GFXFormatR8G8B8X8:
      case GFXFormatR8G8B8A8:
         return GFXFormatR8G8B8A8_SRGB;
      case GFXFormatBC1:
         return GFXFormatBC1_SRGB;
      case GFXFormatBC2:
         return GFXFormatBC2_SRGB;
      case GFXFormatBC3:
         return GFXFormatBC3_SRGB;
      default:
         return format;
      };
   }
}