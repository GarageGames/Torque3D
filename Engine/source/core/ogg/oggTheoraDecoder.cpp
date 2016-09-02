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
#include "core/ogg/oggTheoraDecoder.h"

#include "gfx/gfxFormatUtils.h"
#include "math/mMathFn.h"
#include "console/console.h"


//#define DEBUG_SPEW


//-----------------------------------------------------------------------------

// Lookup tables for the transcoders.
//
// The Y, Cb, and Cr tables are used by both the SSE2 and the generic transcoder.
// For the SSE2 code, the data must be 16 byte aligned.
//
// The clamping table is only used by the generic transcoder.  The SSE2 transcoder
// uses instructions to implicitly clamp out-of-range values.

dALIGN( static S32 sRGBY[ 256 ][ 4 ] );
dALIGN( static S32 sRGBCb[ 256 ][ 4 ] );
dALIGN( static S32 sRGBCr[ 256 ][ 4 ] );

static U8  sClampBuff[ 1024 ];
static U8* sClamp = sClampBuff + 384;

static void initLookupTables()
{
   static bool sGenerated = false;
   if( !sGenerated )
   {
      for( S32 i = 0; i < 256; ++ i )
      {
         // Y.
         
         sRGBY[ i ][ 0 ]   = ( 298 * ( i - 16 ) ) >> 8; // B
         sRGBY[ i ][ 1 ]   = ( 298 * ( i - 16 ) ) >> 8; // G
         sRGBY[ i ][ 2 ]   = ( 298 * ( i - 16 ) ) >> 8; // R
         sRGBY[ i ][ 3 ]   = 0xff;                      // A
         
         // Cb.
         
         sRGBCb[ i ][ 0 ]  = ( 516 * ( i - 128 ) + 128 ) >> 8;       // B
         sRGBCb[ i ][ 1 ]  = - ( ( 100 * ( i - 128 ) + 128 ) >> 8 ); // G 
         
         // Cr.
         
         sRGBCr[ i ][ 1 ]  = - ( ( 208 * ( i - 128 ) + 128 ) >> 8 ); // B
         sRGBCr[ i ][ 2 ]  = ( 409 * ( i - 128 ) + 128 ) >> 8;       // R
      }

      // Setup clamping table for generic transcoder.
      
      for( S32 i = -384; i < 640; ++ i )
         sClamp[ i ] = mClamp( i, 0, 0xFF );
      
      sGenerated = true;
   }
}

static inline S32 sampleG( U8* pCb, U8* pCr )
{
   return sRGBCr[ *pCr ][ 1 ] + sRGBCr[ *pCb ][ 1 ];
}

//=============================================================================
//    OggTheoraDecoder.
//=============================================================================

//-----------------------------------------------------------------------------

OggTheoraDecoder::OggTheoraDecoder( const ThreadSafeRef< OggInputStream >& stream )
   : Parent( stream ),
#ifdef TORQUE_DEBUG
     mLock( 0 ),
#endif
     mTheoraSetup( NULL ),
     mTheoraDecoder( NULL ),
     mTranscoder( TRANSCODER_Auto )
{
   // Initialize.
      
   th_info_init( &mTheoraInfo );
   th_comment_init( &mTheoraComment );
      
   initLookupTables();
}

//-----------------------------------------------------------------------------

OggTheoraDecoder::~OggTheoraDecoder()
{
   // Free packets on the freelist.
   
   OggTheoraFrame* packet;
   while( mFreePackets.tryPopFront( packet ) )
      destructSingle( packet );
      
   // Clean up libtheora structures.
      
   if( mTheoraDecoder )
      th_decode_free( mTheoraDecoder );
   if( mTheoraSetup )
      th_setup_free( mTheoraSetup );
      
   th_comment_clear( &mTheoraComment );
   th_info_clear( &mTheoraInfo );
}

//-----------------------------------------------------------------------------

bool OggTheoraDecoder::_detect( ogg_page* startPage )
{
   _setStartPage( startPage );

   // Read first header packet.

   ogg_packet nextPacket;
   if( !_readNextPacket( &nextPacket )
       || th_decode_headerin( &mTheoraInfo, &mTheoraComment, &mTheoraSetup, &nextPacket ) < 0 )
   {
      th_comment_clear( &mTheoraComment );
      th_info_clear( &mTheoraInfo );
      
      return false;
   }
   
   return true;
}

//-----------------------------------------------------------------------------

bool OggTheoraDecoder::_init()
{
   ogg_packet nextPacket;
   
   // Read header packets.
   
   bool haveTheoraHeader = true;
   while( 1 )
   {
      if( !_readNextPacket( &nextPacket ) )
      {
         haveTheoraHeader = false;
         break;
      }
      
      S32 result = th_decode_headerin( &mTheoraInfo, &mTheoraComment, &mTheoraSetup, &nextPacket );
      if( result < 0 )
      {
         haveTheoraHeader = false;
         break;
      }
      else if( result == 0 )
         break;
   }
   
   // Fail if we have no valid and complete Theora header.
      
   if( !haveTheoraHeader )
   {
      th_comment_clear( &mTheoraComment );
      th_info_clear( &mTheoraInfo );
      
      Con::errorf( "OggTheoraDecoder::_init() - incorrect or corrupt Theora headers" );
      
      return false;
   }

   // Init the decoder.
   
   mTheoraDecoder = th_decode_alloc( &mTheoraInfo, mTheoraSetup );
   
   // Feed the first video packet to the decoder.
   
   ogg_int64_t granulePos;
   th_decode_packetin( mTheoraDecoder, &nextPacket, &granulePos );
   
   mCurrentFrameTime = th_granule_time( mTheoraDecoder, granulePos );
   mCurrentFrameNumber = 0;
   mFrameDuration = 1.f / getFramesPerSecond();
   
   // Make sure we have a valid pitch.
   
   if( !mPacketFormat.mPitch )
      mPacketFormat.mPitch = getFrameWidth() * GFXFormatInfo( mPacketFormat.mFormat ).getBytesPerPixel();
      
   return true;
}

//-----------------------------------------------------------------------------

bool OggTheoraDecoder::_packetin( ogg_packet* packet )
{
   ogg_int64_t granulePos;

   if( th_decode_packetin( mTheoraDecoder, packet, &granulePos ) != 0 )
      return false;

   // See if we should drop this frame.
   //RDTODO: if we have fallen too far behind, start skipping pages

   F32 granuleTime = th_granule_time( mTheoraDecoder, granulePos );
   mCurrentFrameTime = granuleTime;
   mCurrentFrameNumber ++;

   bool dropThisFrame = false;
   TimeSourceRef timeSource = mTimeSource;
   if( timeSource )
   {
      F32 currentTick = F32( timeSource->getPosition() ) / 1000.f;

      if( currentTick >= ( mCurrentFrameTime + mFrameDuration ) )
         dropThisFrame = true;
   }

#ifdef DEBUG_SPEW
   Platform::outputDebugString( "[OggTheoraDecoder] new frame %i at %f sec%s",
      U32( th_granule_frame( mTheoraDecoder, granulePos ) ),
      granuleTime,
      dropThisFrame ? " !! DROPPED !!" : "" );
#endif

   return !dropThisFrame;
}

//-----------------------------------------------------------------------------

U32 OggTheoraDecoder::read( OggTheoraFrame** buffer, U32 num )
{
   #ifdef TORQUE_DEBUG
   AssertFatal( dCompareAndSwap( mLock, 0, 1 ), "OggTheoraDecoder::read() - simultaneous reads not thread-safe" );
   #endif
   
   U32 numRead = 0;
   
   for( U32 i = 0; i < num; ++ i )
   {
      // Read and decode a packet.
      
      if( !_nextPacket() )
         return numRead; // End of stream.
      
      // Decode the frame to Y'CbCr.
      
      th_ycbcr_buffer ycbcr;
      th_decode_ycbcr_out( mTheoraDecoder, ycbcr );
      
      // Allocate a packet.
      
      const U32 width = getFrameWidth();
      const U32 height = getFrameHeight();
      
      OggTheoraFrame* packet;
      if( !mFreePackets.tryPopFront( packet ) )
         packet = constructSingle< OggTheoraFrame* >( mPacketFormat.mPitch * height );
         
      packet->mFrameNumber = mCurrentFrameNumber;
      packet->mFrameTime = mCurrentFrameTime;
      packet->mFrameDuration = mFrameDuration;
      
      // Transcode the packet.
      
      #if ( defined( TORQUE_COMPILER_GCC ) || defined( TORQUE_COMPILER_VISUALC ) ) && defined( TORQUE_CPU_X86 )
      
      if(      ( mTranscoder == TRANSCODER_Auto || mTranscoder == TRANSCODER_SSE2420RGBA ) &&
               getDecoderPixelFormat() == PIXEL_FORMAT_420 &&
               Platform::SystemInfo.processor.properties & CPU_PROP_SSE2 &&
               mPacketFormat.mFormat == GFXFormatR8G8B8A8 &&
               mTheoraInfo.pic_x == 0 &&
               mTheoraInfo.pic_y == 0 )
      {
         _transcode420toRGBA_SSE2( ycbcr, ( U8* ) packet->data, width, height, mPacketFormat.mPitch );
      }
      else
      
      #endif
      
      {
         // Use generic transcoder.
         
         _transcode( ycbcr, ( U8* ) packet->data, width, height );
      }
                  
      buffer[ i ] = packet;
      ++ numRead;
   }
   
   #ifdef TORQUE_DEBUG
   AssertFatal( dCompareAndSwap( mLock, 1, 0 ), "" );
   #endif
   
   return numRead;
}

//-----------------------------------------------------------------------------

void OggTheoraDecoder::_transcode( th_ycbcr_buffer ycbcr, U8* buffer, const U32 width, const U32 height )
{
   #define ycbcrToRGB( rgb, pY, pCb, pCr, G )                        \
   {                                                                 \
      GFXPackPixel(                                                  \
         mPacketFormat.mFormat,                                      \
         rgb,                                                        \
         sClamp[ sRGBY[ *pY ][ 2 ] + sRGBCr[ *pCr ][ 2 ] ],          \
         sClamp[ sRGBY[ *pY ][ 1 ] + G ],                            \
         sClamp[ sRGBY[ *pY ][ 0 ] + sRGBCb[ *pCb ][ 0 ] ],          \
         255                                                         \
      );                                                             \
   }

   // Determine number of chroma samples per 4-pixel luma block.
   
   U32 numChromaSamples = 4;
   EPixelFormat pixelFormat = getDecoderPixelFormat();
   if( pixelFormat == PIXEL_FORMAT_422 )
      numChromaSamples = 2;
   else if( pixelFormat == OggTheoraDecoder::PIXEL_FORMAT_420 )
      numChromaSamples = 1;

   // Convert and copy the pixels.  Deal with all three
   // possible plane configurations.
               
   const U32 pictOffsetY = _getPictureOffset( ycbcr, 0 );
   const U32 pictOffsetU = _getPictureOffset( ycbcr, 1 );
   const U32 pictOffsetV = _getPictureOffset( ycbcr, 2 );

   for( U32 y = 0; y < height; y += 2 )
   {
      U8* dst0 = buffer + y * mPacketFormat.mPitch;
      U8* dst1 = dst0 + mPacketFormat.mPitch;
   
      U8* pY0 = _getPixelPtr( ycbcr, 0, pictOffsetY, 0, y );
      U8* pY1 = _getPixelPtr( ycbcr, 0, pictOffsetY, 0, y + 1 );
      U8* pU0 = _getPixelPtr( ycbcr, 1, pictOffsetU, 0, y );
      U8* pU1 = _getPixelPtr( ycbcr, 1, pictOffsetU, 0, y + 1 );
      U8* pV0 = _getPixelPtr( ycbcr, 2, pictOffsetV, 0, y );
      U8* pV1 = _getPixelPtr( ycbcr, 2, pictOffsetV, 0, y + 1 );
      
      for( U32 x = 0; x < width; x += 2 )
      {
         // Pixel 0x0.
         
         S32 G = sampleG( pU0, pV0 );
         
         ycbcrToRGB( dst0, pY0, pU0, pV0, G );
         
         ++ pY0;
         
         if( numChromaSamples == 4 )
         {
            ++ pU0;
            ++ pV0;
         }
         
         // Pixel 0x1.
         
         if( numChromaSamples == 4 )
            G = sampleG( pU0, pV0 );
            
         ycbcrToRGB( dst0, pY0, pU0, pV0, G );
         
         ++ pY0;
         ++ pU0;
         ++ pV0;
         
         // Pixel 1x0.
         
         if( numChromaSamples != 1 )
            G = sampleG( pU1, pV1 );
         
         ycbcrToRGB( dst1, pY1, pU1, pV1, G );
         
         ++ pY1;
         
         if( numChromaSamples == 4 )
         {
            ++ pU1;
            ++ pV1;
         }
         
         // Pixel 1x1.
         
         if( numChromaSamples == 4 )
            G = sampleG( pU1, pV1 );
            
         ycbcrToRGB( dst1, pY1, pU1, pV1, G );
         
         ++ pY1;
         ++ pU1;
         ++ pV1;
      }
   }
   
   #undef ycbcrToRGB
}

//-----------------------------------------------------------------------------
#if defined( TORQUE_CPU_X86 )
void OggTheoraDecoder::_transcode420toRGBA_SSE2( th_ycbcr_buffer ycbcr, U8* buffer, U32 width, U32 height, U32 pitch )
{
   AssertFatal( width % 2 == 0, "OggTheoraDecoder::_transcode420toRGBA_SSE2() - width must be multiple of 2" );
   AssertFatal( height % 2 == 0, "OggTheoraDecoder::_transcode420toRGBA_SSE2() - height must be multiple of 2" );
      
   unsigned char* ydata = ycbcr[ 0 ].data;
   unsigned char* udata = ycbcr[ 1 ].data;
   unsigned char* vdata = ycbcr[ 2 ].data;
   
   S32* ycoeff = ( S32* ) sRGBY;
   S32* ucoeff = ( S32* ) sRGBCb;
   S32* vcoeff = ( S32* ) sRGBCr;
      
   // At the end of a line loop, we need to jump over the padding resulting from the difference
   // between pitch and width plus jump a whole scanline as we always operate two scanlines
   // at a time.
   const U32 stride = pitch - width * 4 + pitch;
   
   // Same thing for the Y channel.
   const U32 ystrideDelta = ycbcr[ 0 ].stride - width + ycbcr[ 0 ].stride;
   const U32 ypitch = ycbcr[ 0 ].stride;
   
   // U and V only jump a single scanline so we only need to advance by the padding on the
   // right.  Both planes are half-size.
   const U32 ustrideDelta = ycbcr[ 1 ].stride - width / 2;
   const U32 vstrideDelta = ycbcr[ 2 ].stride - width / 2;
         
   #if defined( TORQUE_COMPILER_VISUALC ) && defined( TORQUE_CPU_X86 )

   __asm
   {
         mov ecx,height

      hloop:

         push ecx
         mov ecx,width

      wloop:

         push ecx
         xor eax,eax
         
         // Load and accumulate coefficients for U and V in XMM0.
         
         mov esi,udata
         mov ebx,ucoeff
         mov edx,ydata
         mov al,[esi]
         xor ecx,ecx
         mov edi,vdata
         shl eax,4
         movdqa xmm0,[ebx+eax]

         mov ebx,vcoeff
         mov cl,[edi]
         mov esi,ycoeff
         shl ecx,4
         paddd xmm0,[ebx+ecx]
         xor eax,eax
         xor ebx,ebx
         
         // Load coefficients for Y of the four pixels into XMM1-XMM4.
         
         mov ecx,ypitch
         mov al,[edx]
         mov bl,[edx+1]
         shl eax,4
         shl ebx,4
         movdqa xmm1,[esi+eax]
         movdqa xmm2,[esi+ebx]
         xor eax,eax
         xor ebx,ebx
         
         mov al,[edx+ecx]
         mov bl,[edx+ecx+1]
         shl eax,4
         shl ebx,4
         movdqa xmm3,[esi+eax]
         movdqa xmm4,[esi+ebx]

         mov edi,buffer
         mov ecx,pitch
         
         // Add Cb and Cr on top of Y.
         
         paddd xmm1,xmm0
         paddd xmm2,xmm0
         paddd xmm3,xmm0
         paddd xmm4,xmm0
                  
         // Pack pixels together.  We need to pack twice per pixel
         // to go from 32bits via 16bits to 8bits.
         //
         // Right now we're simply packing two garbage pixels for the
         // second packing operation.  An alternative would be to pack the
         // four pixels into one XMM register and then do a packed shuffle
         // to split out the lower two pixels before the move.
         
         packssdw xmm1,xmm2
         packssdw xmm3,xmm4
         packuswb xmm1,xmm6
         packuswb xmm3,xmm7
         
         // Store pixels.
   
         movq qword ptr [edi],xmm1
         movq qword ptr [edi+ecx],xmm3
         
         // Loop width.
         
         pop ecx

         add ydata,2
         inc udata
         inc vdata
         add buffer,8

         sub ecx,2
         jnz wloop
         
         // Loop height.
     
         pop ecx

         mov ebx,stride
         mov eax,ystrideDelta
         mov edi,ustrideDelta
         mov esi,vstrideDelta

         add buffer,ebx
         add ydata,eax
         add udata,edi
         add vdata,esi
  
         sub ecx,2
         jnz hloop
   };
   
   #elif defined( TORQUE_COMPILER_GCC ) && defined( TORQUE_CPU_X86 )

   asm(  "pushal\n"                                // Save all general-purpose registers.
         
         "movl %0,%%ecx\n"                         // Load height into ECX.
         
      ".hloop_sse:\n"
      
         "pushl %%ecx\n"                           // Save counter.
         "movl %1,%%ecx\n"                         // Load width into ECX.
         
      ".wloop_sse:\n"
      
         "pushl %%ecx\n"                           // Save counter.
         "xorl %%eax,%%eax\n"                      // Zero out eax for later use.
         
         // Load and accumulate coefficients for U and V in XMM0.
         
         "movl %3,%%esi\n"                         // Load U pointer into ESI.
         "movl %8,%%ebx\n"                         // Load U coefficient table into EBX.
         "movl %2,%%edx\n"                         // Load Y pointer into EDX.
         "movb (%%esi),%%al\n"                     // Load U into AL.
         "xorl %%ecx,%%ecx\n"                      // Clear ECX.
         "movl %4,%%edi\n"                         // Load V pointer into EDI.
         "shll $4,%%eax\n"                         // Multiply EAX by 16 to index into table.
         "movdqa (%%ebx,%%eax),%%xmm0\n"           // Load Cb coefficient into XMM0.
         
         "movl %9,%%ebx\n"                         // Load V coefficients table into EBX.
         "movb (%%edi),%%cl\n"                     // Load V into CL.
         "movl %7,%%esi\n"                         // Load Y coefficients table into ESI.
         "shll $4,%%ecx\n"                         // Multiply ECX by 16 to index into table.
         "paddd (%%ebx,%%ecx),%%xmm0\n"            // Add Cr coefficient to Cb coefficient.
         "xorl %%eax,%%eax\n"                      // Clear EAX.
         "xorl %%ebx,%%ebx\n"                      // Clear EBX.
         
         // Load coefficients for Y of the four pixels into XMM1-XMM4.
         
         "movl %14,%%ecx\n"                        // Load Y pitch into ECX (needed later for lower two pixels).
         "movb (%%edx),%%al\n"                     // Load upper-left pixel Y into AL.
         "movb 1(%%edx),%%bl\n"                    // Load upper-right pixel Y into BL.
         "shll $4,%%eax\n"                         // Multiply EAX by 16 to index into table.
         "shll $4,%%ebx\n"                         // Multiply EBX by 16 to index into table.
         "movdqa (%%esi,%%eax),%%xmm1\n"           // Load coefficient for upper-left pixel Y into XMM1.
         "movdqa (%%esi,%%ebx),%%xmm2\n"           // Load coefficient for upper-right pixel Y into XMM2.
         "xorl %%eax,%%eax\n"                      // Clear EAX.
         "xorl %%ebx,%%ebx\n"                      // Clear EBX.
         
         "movb (%%edx,%%ecx),%%al\n"               // Load lower-left pixel Y into AL.
         "movb 1(%%edx,%%ecx),%%bl\n"              // Load lower-right pixel Y into AL.
         "shll $4,%%eax\n"                         // Multiply EAX by 16 to index into table.
         "shll $4,%%ebx\n"                         // Multiply EBX by 16 to index into table.
         "movdqa (%%esi,%%eax),%%xmm3\n"           // Load coefficient for lower-left pixel Y into XMM3.
         "movdqa (%%esi,%%ebx),%%xmm4\n"           // Load coefficient for lower-right pixel Y into XMM4.
         
         "movl %5,%%edi\n"                         // Load buffer pointer into EDI (for later use).
         "movl %6,%%ecx\n"                         // Load pitch into ECX (for later use).
         
         // Add Cb and Cr on top of Y.
         
         "paddd %%xmm0,%%xmm1\n"                   // Add chroma channels to upper-left pixel.
         "paddd %%xmm0,%%xmm2\n"                   // Add chroma channels to upper-right pixel.
         "paddd %%xmm0,%%xmm3\n"                   // Add chroma channels to lower-left pixel.
         "paddd %%xmm0,%%xmm4\n"                   // Add chroma channels to lower-right pixel.
                  
         // Pack pixels together.  We need to pack twice per pixel
         // to go from 32bits via 16bits to 8bits.
         //
         // Right now we're simply packing two garbage pixels for the
         // second packing operation.  An alternative would be to pack the
         // four pixels into one XMM register and then do a packed shuffle
         // to split out the lower two pixels before the move.
         
         "packssdw %%xmm2,%%xmm1\n"                // Pack 32bit channels together into 16bit channels on upper two pixels.
         "packssdw %%xmm4,%%xmm3\n"                // Pack 32bit channels together into 16bit channels on lower two pixels.
         "packuswb %%xmm6,%%xmm1\n"                // Pack 16bit channels together into 8bit channels on upper two pixels (plus two garbage pixels).
         "packuswb %%xmm7,%%xmm3\n"                // Pack 16bit channels together into 8bit channels on lower two pixels (plus two garbage pixels).
         
         // Store pixels.
         
         "movq %%xmm1,(%%edi)\n"                    // Store upper two pixels.
         "movq %%xmm3,(%%edi,%%ecx)\n"              // Store lower two pixels.
         
         // Loop width.
         
         "popl %%ecx\n"                            // Restore width counter.
         
         "addl $2,%2\n"                            // Bump Y pointer by two pixels (1 bpp).
         "incl %3\n"                               // Bump U pointer by one pixel (1 bpp).
         "incl %4\n"                               // Bump V pointer by one pixel (1 bpp).
         "addl $8,%5\n"                            // Bump buffer pointer by two pixels (4 bpp).

         "subl $2,%%ecx\n"
         "jnz .wloop_sse\n"
         
         // Loop height.
         
         "popl %%ecx\n"                            // Restore height counter.

         "movl %10,%%ebx\n"                        // Load buffer stride into EBX.
         "movl %11,%%eax\n"                        // Load Y stride delta into EAX.
         "movl %12,%%edi\n"                        // Load U stride delta into EDI.
         "movl %13,%%esi\n"                        // Load V stride delta into ESI.
         
         "addl %%ebx,%5\n"                         // Bump buffer pointer by stride delta.
         "addl %%eax,%2\n"                         // Bump Y pointer by stride delta.
         "addl %%edi,%3\n"                         // Bump U pointer by stride delta.
         "addl %%esi,%4\n"                         // Bump V pointer by stride delta.
         
         "subl $2,%%ecx\n"
         "jnz .hloop_sse\n"
         
         "popal\n"
      :
      : "m" ( height ),                                        // 0
        "m" ( width ),                                         // 1
        "m" ( ydata ),                                         // 2
        "m" ( udata ),                                         // 3
        "m" ( vdata ),                                         // 4
        "m" ( buffer ),                                        // 5
        "m" ( pitch ),                                         // 6
        "m" ( ycoeff ),                                        // 7
        "m" ( ucoeff ),                                        // 8
        "m" ( vcoeff ),                                        // 9
        "m" ( stride ),                                        // 10
        "m" ( ystrideDelta ),                                  // 11
        "m" ( ustrideDelta ),                                  // 12
        "m" ( vstrideDelta ),                                  // 13
        "m" ( ypitch )                                         // 14
   );
   
   #endif
}
#endif