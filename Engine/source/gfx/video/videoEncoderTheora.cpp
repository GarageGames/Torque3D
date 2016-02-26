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

#ifdef TORQUE_OGGTHEORA

#include "videoCapture.h"
#include "core/stream/fileStream.h"
#include "console/console.h"
#include "gfx/bitmap/gBitmap.h"
#include "gfx/bitmap/bitmapUtils.h"

#include "platform/threads/thread.h"
#include "platform/threads/threadSafeDeque.h"
#include "platform/threads/semaphore.h"

#include "theora/theoraenc.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisenc.h"

#include "platform/profiler.h"

//#define THEORA_ENCODER_SINGLE_THREAD

// These are the coefficient lookup tables, so we don't need to multiply
dALIGN( static S32 sYRGB[ 256 ][ 4 ] );
dALIGN( static S32 sURGB[ 256 ][ 4 ] );
dALIGN( static S32 sVRGB[ 256 ][ 4 ] );

/// Initialize the lookup tables used in the RGB-YUV transcoding
static void initLookupTables()
{
   static bool sGenerated = false;
   if( !sGenerated )
   {
      for( S32 i = 0; i < 256; ++ i )
      {
         //Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
         //U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
         //V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128

         // Y coefficients
         sYRGB[ i ][ 0 ] =  66 * i;
         sYRGB[ i ][ 1 ] = 129 * i;
         sYRGB[ i ][ 2 ] =  25 * i + 128 + (16 << 8);

         // U coefficients
         sURGB[ i ][ 0 ] = -38 * i;
         sURGB[ i ][ 1 ] = -74 * i;
         sURGB[ i ][ 2 ] = 112 * i + 128 + (128 << 8);

         // V coefficients
         sVRGB[ i ][ 0 ] = 112 * i;
         sVRGB[ i ][ 1 ] = -94 * i;
         sVRGB[ i ][ 2 ] = -18 * i + 128 + (128 << 8);
      }
      sGenerated = true;
   }
}

/// Theora video capture encoder
class VideoEncoderTheora : public VideoEncoder, public Thread
{
   U32 mCurrentFrame;
   
   ogg_stream_state to; // take physical pages, weld into a logical stream of packets   
   th_enc_ctx      *td; // Theora encoder context
   th_info          ti; // Theora info structure
   th_comment       tc; // Theora comment structure
   
   FileStream mFile;    // Output file

   th_ycbcr_buffer mBuffer; // YCbCr buffer
      
   GBitmap* mLastFrame;

   ThreadSafeDeque< GBitmap* > mFrameBitmapList; // List with unprocessed frame bitmaps   
   Semaphore  mSemaphore;    //Semaphore for preventing the encoder from being lagged behind the game

   bool mErrorStatus; //Status flag, true if OK, false if an error ocurred
   
   /// Sets our error status
   bool setStatus(bool status)
   {
      mErrorStatus = status;
      return mErrorStatus;
   }
   bool getStatus() { return mErrorStatus; }

   /// Encodes one frame
   void encodeFrame( GBitmap* bitmap, bool isLast=false )
   {        
      PROFILE_SCOPE(Theora_encodeFrame);
      
      //Copy bitmap to YUV buffer
      copyBitmapToYUV420(bitmap);      
            
      PROFILE_START(th_encode_ycbcr_in);
      //Submit frame for encoding
      if (th_encode_ycbcr_in(td, mBuffer))
      {
         Platform::outputDebugString("VideoEncoderTheora::encodeFrame() - The buffer size does not match the frame size the encoder was initialized with, or encoding has already completed. .");
         setStatus(false);
         PROFILE_END();
         return;
      }      
      PROFILE_END();
            
      //Fetch the encoded packets
      ogg_packet packet;
      if (!th_encode_packetout(td, isLast, &packet))
      {
         Platform::outputDebugString("VideoEncoderTheora::encodeFrame() - Internal Theora library error.");
         setStatus(false);
         return;
      }

      //Insert packet into vorbis stream page
      ogg_stream_packetin(&to,&packet);

      //Increment
      mCurrentFrame++;

      //Is there a video page flushed? 
      ogg_page videopage;        
      while (ogg_stream_pageout(&to,&videopage))
      {
         //Write the video page to disk
         mFile.write(videopage.header_len, videopage.header);
         mFile.write(videopage.body_len, videopage.body);

         F64 videotime = th_granule_time(td,ogg_page_granulepos(&videopage));
         if (videotime > 0)
         {            
            S32 hundredths=(int)(videotime*100-(long)videotime*100);
            S32 seconds=(long)videotime%60;
            Platform::outputDebugString("Encoding time %g %02i.%02i", videotime, seconds, hundredths);
         }
      }

      mSemaphore.release();
   }
   
   bool process(bool ending)
   {
      if (!getStatus())
         return false;

      //Try getting a bitmap for encoding         
      GBitmap* bitmap = NULL;      
      if (mFrameBitmapList.tryPopFront(bitmap))   
      {         
         encodeFrame(bitmap, false);
      }
            
      //Delete previous bitmap
      if (!ending && bitmap)
      {
         if (mLastFrame)
            pushProcessedBitmap(mLastFrame);
         mLastFrame = bitmap;
      }

      //If we're stopping encoding, but didn't have a frame, re-encode the last frame
      if (ending && !bitmap && mLastFrame)
      {
         encodeFrame(mLastFrame, true);         
         pushProcessedBitmap(mLastFrame);
         mLastFrame = NULL;
      }

      // We'll live while we have a last frame
      return (mLastFrame != NULL);
   }
   
public:
   VideoEncoderTheora() :
      mCurrentFrame(0), td(NULL), mLastFrame(NULL)
   {
      setStatus(false);      
   }
      
   virtual void run( void* arg )
   {      
      _setName( "TheoraEncoderThread" );
      while (!checkForStop())
         process(false);

      // Encode all pending frames and close the last one
      while (process(true));
   }   

   /// Begins accepting frames for encoding
   bool begin()
   {
      mPath += ".ogv";
      mCurrentFrame = 0;
            
      //Try opening the file for writing
      if ( !mFile.open( mPath, Torque::FS::File::Write ) )
      {
         Platform::outputDebugString( "VideoEncoderTheora::begin() - Failed to open output file '%s'!", mPath.c_str() );         
         return setStatus(false);
      }

      ogg_stream_init(&to, Platform::getRandom() * S32_MAX);

      ogg_packet op;

      th_info_init(&ti);
      ti.frame_width  = mResolution.x;
      ti.frame_height = mResolution.y;
      ti.pic_width    = mResolution.x;
      ti.pic_height   = mResolution.y;
      ti.pic_x        = 0;
      ti.pic_y        = 0;

      ti.fps_numerator      = (int)(mFramerate * 1000.0f);
      ti.fps_denominator    = 1000;

      ti.aspect_numerator   = 0;
      ti.aspect_denominator = 0;
      ti.colorspace = TH_CS_UNSPECIFIED;
      
      ti.target_bitrate = 0;
      ti.quality        = 63;
      ti.pixel_fmt      = TH_PF_420;
            
      td = th_encode_alloc(&ti);
      if (td == NULL)
      {
         Platform::outputDebugString("VideoEncoderTheora::begin() - Theora initialization error.");   
         return setStatus(false);
      }

      th_info_clear(&ti);              

      // This is needed for youtube compatibility
      S32 vp3_compatible = 1;
      th_encode_ctl(td, TH_ENCCTL_SET_VP3_COMPATIBLE, &vp3_compatible, sizeof(vp3_compatible));      
      
      // Set the encoder to max speed
      S32 speed_max;      
      S32 ret;
      ret = th_encode_ctl(td, TH_ENCCTL_GET_SPLEVEL_MAX, &speed_max, sizeof(speed_max));
      if(ret<0){
         Platform::outputDebugString("VideoEncoderTheora::begin() - could not determine maximum speed level.");
         speed_max = 0;
      }
      ret = th_encode_ctl(td, TH_ENCCTL_SET_SPLEVEL, &speed_max, sizeof(speed_max));

      // write the bitstream header packets with proper page interleave
      th_comment_init(&tc);

      // first packet will get its own page automatically
      if(th_encode_flushheader(td,&tc,&op) <= 0)
      {
         Platform::outputDebugString("VideoEncoderTheora::begin() - Internal Theora library error.");
         return setStatus(false);
      }

      ogg_page og;
      ogg_stream_packetin(&to,&op);
      if(ogg_stream_pageout(&to,&og) != 1)
      {         
         Platform::outputDebugString("VideoEncoderTheora::begin() - Internal Ogg library error.");
         return setStatus(false);
      }
      mFile.write(og.header_len, og.header);
      mFile.write(og.body_len, og.body);

      // create the remaining theora headers    
      while((ret = th_encode_flushheader(td,&tc,&op)) != 0)
      {         
         if(ret < 0)
         {
            Platform::outputDebugString("VideoEncoderTheora::begin() - Internal Theora library error.");
            return setStatus(false);
         }
         ogg_stream_packetin(&to,&op);
      }

      // Flush the rest of our headers. This ensures
      // the actual data in each stream will start
      // on a new page, as per spec.
      while((ret = ogg_stream_flush(&to,&og)) != 0)
      {         
         if(ret < 0)
         {
            Platform::outputDebugString("VideoEncoderTheora::begin() - Internal Ogg library error.");
            return setStatus(false);
         }
         mFile.write(og.header_len, og.header);
         mFile.write(og.body_len, og.body);
      }

      //Initialize the YUV buffer 
      S32 decimation[] = {0,1,1};
      for (U32 i=0; i<3; i++)
      {
         mBuffer[i].width  = mResolution.x >> decimation[i];
         mBuffer[i].height = mResolution.y >> decimation[i];
         mBuffer[i].stride = mBuffer[i].width * sizeof(U8);
         mBuffer[i].data = new U8[mBuffer[i].width*mBuffer[i].height*sizeof(U8)];
      }      

      //Initialize the YUV coefficient lookup tables
      initLookupTables();

      setStatus(true);

#ifndef THEORA_ENCODER_SINGLE_THREAD
      start();
#endif
      
      return getStatus();
   }

   /// Pushes a new frame into the video stream
   bool pushFrame( GBitmap * bitmap )
   {      
                  
      // Push the bitmap into the frame list
      mFrameBitmapList.pushBack( bitmap );

      mSemaphore.acquire();

#ifdef THEORA_ENCODER_SINGLE_THREAD
      process(false);
#endif
      
      return getStatus();
   }

   /// Finishes the encoding and closes the video
   bool end()
   {  
      //Let's wait the thread stop doing whatever it needs to do
      stop();
      join();

#ifdef THEORA_ENCODER_SINGLE_THREAD
      while (process(true));
#endif
            
      th_encode_free(td);
      ogg_stream_clear(&to);
      th_comment_clear(&tc);

      mFile.close();
      return true;
   }
   

   void setResolution( Point2I* resolution ) 
   { 
      /* Theora has a divisible-by-sixteen restriction for the encoded frame size */
      /* scale the picture size up to the nearest /16 and calculate offsets */
      resolution->x = (resolution->x) + 15 & ~0xF;
      resolution->y = (resolution->y) + 15 & ~0xF;

      mResolution = *resolution; 
   }
   
   /// Converts the bitmap to YUV420 and copies it into our internal buffer
   void copyBitmapToYUV420( GBitmap* bitmap )
   {
      PROFILE_SCOPE(copyBitmapToYUV420);

      // Convert luma
      const U8* rgb = bitmap->getBits();           
      
      // Chroma planes are half width and half height
      U32 w = mResolution.x / 2;
      U32 h = mResolution.y / 2;      

      // We'll update two luminance rows at once
      U8* yuv_y0 = mBuffer[0].data;
      U8* yuv_y1 = mBuffer[0].data + mBuffer[0].stride;

      // Get pointers to chroma planes
      U8* yuv_u = mBuffer[1].data;
      U8* yuv_v = mBuffer[2].data;

      // We'll also need to read two RGB rows at once
      U32 rgbStride = mResolution.x * bitmap->getBytesPerPixel();
      const U8* row0 = rgb;
      const U8* row1 = row0 + rgbStride;

      for(U32 y = 0; y < h; y++) 
      {         
         for(U32 x = 0; x < w; x++) 
         {
            // Fetch two RGB samples from each RGB row (for downsampling the chroma)
            U8 r0 = *row0++;
            U8 g0 = *row0++;
            U8 b0 = *row0++;
            U8 r1 = *row0++;
            U8 g1 = *row0++;
            U8 b1 = *row0++;
            U8 r2 = *row1++;
            U8 g2 = *row1++;
            U8 b2 = *row1++;
            U8 r3 = *row1++;
            U8 g3 = *row1++;
            U8 b3 = *row1++;

            // Convert the four RGB samples into four luminance samples            
            *yuv_y0 = ( (sYRGB[r0][0] + sYRGB[g0][1] + sYRGB[b0][2]) >> 8);
            yuv_y0++;
            *yuv_y0 = ( (sYRGB[r1][0] + sYRGB[g1][1] + sYRGB[b1][2]) >> 8);
            yuv_y0++;
            *yuv_y1 = ( (sYRGB[r2][0] + sYRGB[g2][1] + sYRGB[b2][2]) >> 8);
            yuv_y1++;
            *yuv_y1 = ( (sYRGB[r3][0] + sYRGB[g3][1] + sYRGB[b3][2]) >> 8);
            yuv_y1++;

            // Downsample the four RGB samples
            U8 r = (r0 + r1 + r2 + r3) >> 2;
            U8 g = (g0 + g1 + g2 + g3) >> 2;
            U8 b = (b0 + b1 + b2 + b3) >> 2;

            // Convert downsampled RGB into chroma
            *yuv_u = ( (sURGB[r][0] + sURGB[g][1] + sURGB[b][2]) >> 8);
            *yuv_v = ( (sVRGB[r][0] + sVRGB[g][1] + sVRGB[b][2]) >> 8);
            yuv_u++;
            yuv_v++;
         }

         //Next RGB rows
         row0 += rgbStride;
         row1 += rgbStride;

         //Next luminance rows
         yuv_y0 += mBuffer[0].stride;
         yuv_y1 += mBuffer[0].stride;
      }      
   }
};

REGISTER_VIDEO_ENCODER(VideoEncoderTheora, THEORA)

#endif