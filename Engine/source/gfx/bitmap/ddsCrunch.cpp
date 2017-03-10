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

#pragma warning( push )
#pragma warning( disable : 4201 ) // warning C4201: nonstandard extension used : nameless struct/union
#include "crn_image.h"
#include "crn_texture_comp.h"
#pragma warning( pop ) // C4201

#include "gfx/bitmap/ddsFile.h"
#include "gfx/bitmap/ddsUtils.h"
#include "core/stream/memStream.h"
#include "console/consoleInternal.h"

//-----------------------------------------------------------------------------

using namespace crnlib;

namespace DDSUtil
{

// If false is returned, from this method, the source DDS is not modified
bool crunchDDS( DDSFile *srcDDS, const GFXFormat dxtFormat, bool isNormalMap )
{
   PROFILE_SCOPE(CRUNCH_DXT_COMPRESS);
   // Sanity check
   if( srcDDS->mBytesPerPixel != 4 )
   {
      AssertFatal( false, "Crunch wants 32-bit source data" );
      return false;
   }
   if(srcDDS->getWidth() > cCRNMaxLevelResolution || srcDDS->getHeight() > cCRNMaxLevelResolution)
   {
      Con::errorf("Crunch supports resolutions up to %d (given: %d)", cCRNMaxLevelResolution, getMax(srcDDS->getWidth(), srcDDS->getHeight()));
      return false;
   }

   crn_format fmt = cCRNFmtInvalid;
   // Flag which format we are using
   switch( dxtFormat )
   {
   case GFXFormatDXT1:
      fmt = cCRNFmtDXT1;
      break;
   case GFXFormatDXT2:
   case GFXFormatDXT3:
      fmt = cCRNFmtDXT3;
      break;
   case GFXFormatDXT4:
   case GFXFormatDXT5:
      fmt = cCRNFmtDXT5;
      break;
   default:
      AssertFatal( false, "Error in crunchDDS: dxtFormat assumption failed" );
      return false;
      break;
   }

   // Force DXT5 if the source format is with alpha
   if( srcDDS->getHasTransparency() )
      fmt = cCRNFmtDXT5;

   // The source surface is the original surface of the file
   DDSFile::SurfaceData *srcSurface = srcDDS->mSurfaces.last();

   crn_comp_params comp_params;
   comp_params.m_faces = 1;
   comp_params.m_file_type = cCRNFileTypeDDS;
   comp_params.m_levels = srcSurface->mMips.size();
   comp_params.m_width = srcDDS->getWidth();
   comp_params.m_height = srcDDS->getHeight();
   comp_params.m_format = fmt;
   comp_params.m_dxt_quality = cCRNDXTQualitySuperFast;
   comp_params.m_num_helper_threads = 1; // TODO: When updated ThreadPool is ready, use ThreadPool::instance()->getNumThreads();
   
   comp_params.set_flag(cCRNCompFlagDXT1AForTransparency, !isNormalMap);

   U32 pActual_quality_level = 0;

   image_u8 temp_images[cCRNMaxFaces][cCRNMaxLevels];
   for(S32 i=0; i < srcSurface->mMips.size(); i++)
   {
      // Grab source image bits and make it crunch-compatible type
      crn_uint32 *pSrc_image = (crn_uint32*)srcSurface->mMips[i];
      // Assign the source at the top-most level
      comp_params.m_pImages[0][i] = pSrc_image;
   }
   crnlib::vector<uint8> comp_data;

   // Do the job!
   if (!create_compressed_texture(comp_params, comp_data, &pActual_quality_level, 0))
      return false;

   // Okay, the comp_data now contains a full DDS file with all mip-maps, in compressed format
   // and we need to copy it to the destination (srcDDS).
   srcDDS->mSurfaces.clear();
   srcDDS->clear();
   DDSFile *dstDDS = srcDDS; // new DDSFile;
   MemStream stream(comp_data.size(), comp_data.get_ptr(), true, false);

   if( !dstDDS->read( stream, 0 ) )
   {
      AssertFatal(false, "We shouldn't get in here, something is wrong!");
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------

class crunchDDSWorker : public ThreadPool::WorkItem
{
public:
   typedef ThreadPool::WorkItem Parent;

protected:
   ThreadSafeRef<crunchDDSWorkItem> mReply;
   virtual void execute()
   {
      mReply->succeed = DDSUtil::crunchDDS(mReply->mSrcDDS, mReply->mDxtFormat, mReply->mIsNormalMap);
      ThreadPool::GLOBAL().queueWorkItem(mReply); // TODO: When updated ThreadPool is ready, use ThreadPool::instance()->queueWorkItem( mReply );
      return;
   }
public:
   crunchDDSWorker(crunchDDSWorkItem *reply) : mReply(reply)
   {
   }
};

//-----------------------------------------------------------------------------

class crunchDDSCallbackWorker : public ThreadPool::WorkItem
{
public:
   typedef ThreadPool::WorkItem Parent;

protected:
   crunchCallback mCB;
   DDSFile *mSrcDDS;
   GFXFormat mDxtFormat;
   bool mIsNormalMap;
   virtual void execute()
   {
      if(mCB)
      {
         bool result = DDSUtil::crunchDDS(mSrcDDS, mDxtFormat, mIsNormalMap);
         mCB(mSrcDDS, result);
         return;
      }
   }
public:
   crunchDDSCallbackWorker(crunchCallback cb, DDSFile *srcDDS, const GFXFormat dxtFormat = GFXFormatDXT1, bool isNormalMap = false )
      : mSrcDDS(srcDDS), mCB(cb), mDxtFormat(dxtFormat), mIsNormalMap(isNormalMap)
   {
   }
};

//-----------------------------------------------------------------------------

void crunchDDS( crunchDDSWorkItem *item )
{
   ThreadSafeRef<crunchDDSWorker> workItem (new crunchDDSWorker(item));
   ThreadPool::GLOBAL().queueWorkItem(workItem); // TODO: When updated ThreadPool is ready, use ThreadPool::instance()->queueWorkItem( workItem );
}

//-----------------------------------------------------------------------------

void crunchDDS( crunchCallback cb, DDSFile *srcDDS, const GFXFormat dxtFormat, bool isNormalMap )
{
   ThreadSafeRef<crunchDDSCallbackWorker> workItem (new crunchDDSCallbackWorker(cb, srcDDS, dxtFormat, isNormalMap));
   ThreadPool::GLOBAL().queueWorkItem(workItem); // TODO: When updated ThreadPool is ready, use ThreadPool::instance()->queueWorkItem( workItem );
}

} // namespace DDSUtil


/*

//-----------------------------------------------------------------------------
// Crunch DDS Usage Example 1

ImplementCrunchDDSWorkItem(ddsPackSave);

void ddsPackSave::execute()
{
   FileStream fs;
   if(fs.open( "test.dds", Torque::FS::File::Write ))
   {
      mSrcDDS->write(fs);
      delete mSrcDDS;
      fs.close();
   }
}

void testDDSCrunchExample1()
{
   GBitmap bitmap; // Assuming we have loaded it or generated
   bitmap.extrudeMipLevels();
   DDSFile* outDDS = DDSFile::createDDSFileFromGBitmap(&bitmap);
   DDSUtil::crunchDDS(new ddsPackSave(outDDS, GFXFormatDXT5, true, true));
}

//-----------------------------------------------------------------------------
// Crunch DDS Usage Example 2

void crunchedExample2(DDSFile * result, bool succeed)
{
   if(!succeed)
   {
      // If we got in here, it means the compression has failed!
      return;
   }
   FileStream fs;
   if(fs.open( "test.dds", Torque::FS::File::Write ))
   {
      result->write(fs);
      delete result;
      fs.close();
   }
}

void testDDSCrunchExample2()
{
   GBitmap bitmap; // Assuming we have loaded it or generated
   bitmap.extrudeMipLevels();
   DDSFile* outDDS = DDSFile::createDDSFileFromGBitmap(&bitmap);
   DDSUtil::crunchDDS(crunchedExample2, outDDS, GFXFormatDXT5, true);
}

//-----------------------------------------------------------------------------

*/

//-----------------------------------------------------------------------------
