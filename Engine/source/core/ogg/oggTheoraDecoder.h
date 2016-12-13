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

#ifndef _OGGTHEORADECODER_H_
#define _OGGTHEORADECODER_H_

#ifndef _OGGINPUTSTREAM_H_
   #include "core/ogg/oggInputStream.h"
#endif
#ifndef _TSTREAM_H_
   #include "core/stream/tStream.h"
#endif
#ifndef _RAWDATA_H_
   #include "core/util/rawData.h"
#endif
#ifndef _GFXENUMS_H_
   #include "gfx/gfxEnums.h"
#endif
#ifndef _THREADSAFEDEQUE_H_
   #include "platform/threads/threadSafeDeque.h"
#endif
#include "theora/theoradec.h"


/// A single decoded Theora video frame.
class OggTheoraFrame : public RawData
{
   public:
   
      typedef RawData Parent;
      
      OggTheoraFrame() {}
      OggTheoraFrame( S8* data, U32 size, bool ownMemory = false )
         : Parent( data, size, ownMemory ) {}
         
      /// Serial number of this frame in the stream.
      U32 mFrameNumber;
      
      /// Playtime in seconds at which to display this frame.
      F32 mFrameTime;
      
      /// Seconds to display this frame.
      F32 mFrameDuration;
};


/// Decodes a Theora substream into frame packets.
///
/// Frame packets contain raw pixel data in the set pixel format (default is R8G8B8).
/// Reading on a thread is safe, but remember to keep a reference to the OggInputStream
/// master from the worker thread.
class OggTheoraDecoder : public OggDecoder,
                         public IInputStream< OggTheoraFrame* >
{
   public:
   
      typedef OggDecoder Parent;
      
      /// Y'CbCr pixel format of the source video stream.
      /// For informational purposes only.  Packet out is determined
      /// by PacketFormat.
      enum EPixelFormat
      {
         PIXEL_FORMAT_444,    // Full Y, full Cb, full Cr.
         PIXEL_FORMAT_422,    // Full Y, half-width Cb, half-width Cr.
         PIXEL_FORMAT_420,    // Full Y, half-widht+height Cb, half-width+height Cr.
         PIXEL_FORMAT_Unknown
      };
            
      /// Descriptor for surface format that this stream should
      /// decode into.  This saves an otherwise potentitally necessary
      /// swizzling step.
      ///
      /// @note The output channel ordering will be in device format, i.e.
      ///   least-significant first.
      struct PacketFormat
      {
         /// Pixel format.
         GFXFormat mFormat;
         
         /// Bytes per scanline.
         U32 mPitch;
         
         /// Default descriptor sets up for RGB.
         PacketFormat()
            : mFormat( GFXFormatR8G8B8 ),
              mPitch( 0 ) {}
         
         ///
         PacketFormat( GFXFormat format, U32 pitch )
            : mFormat( format ),
              mPitch( pitch ) {}
      };
      
      ///
      enum ETranscoder
      {
         TRANSCODER_Auto,           ///< Auto-detect from current formats and processor capabilities.
         TRANSCODER_Generic,        ///< Generic transcoder that handles all source and target formats; 32bit integer + lookup tables.
         TRANSCODER_SSE2420RGBA,    ///< SSE2 transcoder with fixed 4:2:0 to RGBA conversion; 32bit integer + lookup tables.
      };
            
   protected:
   
      typedef IPositionable< U32 >* TimeSourceRef;
      
      /// @name libtheora Data
      /// @{
            
      ///
      th_comment mTheoraComment;
      
      ///
      th_info mTheoraInfo;
      
      ///
      th_setup_info* mTheoraSetup;
      
      ///
      th_dec_ctx* mTheoraDecoder;
      
      /// @}
      
      ///
      PacketFormat mPacketFormat;
            
      ///
      F32 mFrameDuration;

      ///
      F32 mCurrentFrameTime;
      
      ///
      U32 mCurrentFrameNumber;
            
      /// If this is set, the decoder will drop frames that are
      /// already outdated with respect to the time source.
      ///
      /// @note Times are in milliseconds and in video time.
      TimeSourceRef mTimeSource;
      
      /// Transcoder to use for color space conversion.  If the current
      /// setting is invalid, will fall back to generic.
      ETranscoder mTranscoder;
      
      ///
      ThreadSafeDeque< OggTheoraFrame* > mFreePackets;
      
      #ifdef TORQUE_DEBUG
      U32 mLock;
      #endif
      
      /// Generic transcoder going from any of the Y'CbCr pixel formats to
      /// any RGB format (that is supported by GFXFormatUtils).
      void _transcode( th_ycbcr_buffer ycbcr, U8* buffer, U32 width, U32 height );
#if defined( TORQUE_CPU_X86 )
      /// Transcoder with fixed 4:2:0 to RGBA conversion using SSE2 assembly. Unused on 64 bit archetecture.
      void _transcode420toRGBA_SSE2( th_ycbcr_buffer ycbcr, U8* buffer, U32 width, U32 height, U32 pitch );
#endif
      // OggDecoder.
      virtual bool _detect( ogg_page* startPage );
      virtual bool _init();
      virtual bool _packetin( ogg_packet* packet );

      ///
      U32 _getPixelOffset( th_ycbcr_buffer buffer, U32 plane, U32 x, U32 y ) const
      {
         switch( getDecoderPixelFormat() )
         {
            case PIXEL_FORMAT_444:  break;
            case PIXEL_FORMAT_422:  if( plane != 0 ) x >>= 1; break;
            case PIXEL_FORMAT_420:  if( plane != 0 ) { x >>= 1; y >>= 1; } break;
            
            default:
               AssertFatal( false, "OggTheoraDecoder::_getPixelOffset() - invalid pixel format" );
         }
         
         return ( y * buffer[ plane ].stride + x );
      }

      ///
      U8* _getPixelPtr( th_ycbcr_buffer buffer, U32 plane, U32 offset, U32 x, U32 y ) const
      {
         return ( buffer[ plane ].data + offset + _getPixelOffset( buffer, plane, x, y ) );
      }
      
      ///
      U32 _getPictureOffset( th_ycbcr_buffer buffer, U32 plane )
      {
         return _getPixelOffset( buffer, plane, mTheoraInfo.pic_x, mTheoraInfo.pic_y );
      }
            
   public:
   
      ///
      OggTheoraDecoder( const ThreadSafeRef< OggInputStream >& stream );
      
      ~OggTheoraDecoder();
      
      /// Return the width of video image frames in pixels.
      /// @note This returns the actual picture width rather than Theora's internal encoded frame width.
      U32 getFrameWidth() const { return mTheoraInfo.pic_width; }
      
      /// Return the height of video image frames in pixels.
      /// @note This returns the actual picture height rather than Theora's internal encoded frame height.
      U32 getFrameHeight() const { return mTheoraInfo.pic_height; }
      
      ///
      F32 getFramesPerSecond() const { return ( F32( mTheoraInfo.fps_numerator ) / F32( mTheoraInfo.fps_denominator ) ); }
      
      ///
      EPixelFormat getDecoderPixelFormat() const
      {
         switch( mTheoraInfo.pixel_fmt )
         {
            case TH_PF_444:   return PIXEL_FORMAT_444;
            case TH_PF_422:   return PIXEL_FORMAT_422;
            case TH_PF_420:   return PIXEL_FORMAT_420;
            default:          return PIXEL_FORMAT_Unknown;
         }
      }
      
      ///
      const PacketFormat& getPacketFormat() const { return mPacketFormat; }
      
      ///
      void setPacketFormat( const PacketFormat& format ) { mPacketFormat = format; }
      
      /// Set the reference time source.  Frames will be dropped if the decoder
      /// falls behind the time of this source.
      ///
      /// @note The time source must have at least the same lifetime as the decoder.
      void setTimeSource( const TimeSourceRef& timeSource ) { mTimeSource = timeSource; }
            
      /// Set the Y'CbCr->RGB transcoder to use.
      void setTranscoder( ETranscoder transcoder ) { mTranscoder = transcoder; }
      
      ///
      void reusePacket( OggTheoraFrame* packet ) { mFreePackets.pushBack( packet ); }
         
      // OggDecoder.
      virtual const char* getName() const { return "Theora"; }
      
      // IInputStream.
      virtual U32 read( OggTheoraFrame** buffer, U32 num );
};

#endif // !_OGGTHEORADECODER_H_
