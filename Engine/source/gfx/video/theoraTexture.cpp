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

/// If defined, uses a separate file stream to read Vorbis sound data
/// from the Ogg stream.  This removes both the contention caused by
/// concurrently streaming from a single master stream as well as the
/// coupling between Theora reads and Vorbis reads that arises from
/// multiplexing.
#define SPLIT_VORBIS


#include "theoraTexture.h"

#include "sfx/sfxDescription.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxCommon.h"
#include "sfx/sfxMemoryStream.h"
#include "sfx/sfxSound.h"
#ifdef SPLIT_VORBIS
   #include "sfx/media/sfxVorbisStream.h"
#endif

#include "core/stream/fileStream.h"
#include "core/ogg/oggInputStream.h"
#include "core/ogg/oggTheoraDecoder.h"
#include "core/ogg/oggVorbisDecoder.h"
#include "core/util/safeDelete.h"
#include "core/util/rawData.h"
#include "console/console.h"
#include "math/mMath.h"
#include "gfx/bitmap/gBitmap.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxFormatUtils.h"
#include "gfx/gfxTextureManager.h"



/// Profile for the video texture.
GFX_ImplementTextureProfile(  GFXTheoraTextureProfile,
                              GFXTextureProfile::DiffuseMap,
                              GFXTextureProfile::NoMipmap | GFXTextureProfile::Dynamic,
                              GFXTextureProfile::NONE );


//-----------------------------------------------------------------------------

static const char* GetPixelFormatName( OggTheoraDecoder::EPixelFormat format )
{
   switch( format )
   {
      case OggTheoraDecoder::PIXEL_FORMAT_420:
         return "4:2:0";
      case OggTheoraDecoder::PIXEL_FORMAT_422:
         return "4:2:2";
      case OggTheoraDecoder::PIXEL_FORMAT_444:
         return "4:4:4";
         
      case OggTheoraDecoder::PIXEL_FORMAT_Unknown: ;
   }
   return "Unknown";
}

//=============================================================================
//    TheoraTexture::FrameReadItem implementation.
//=============================================================================

//-----------------------------------------------------------------------------

TheoraTexture::FrameReadItem::FrameReadItem( AsyncBufferedInputStream< TheoraTextureFrame*, IInputStream< OggTheoraFrame* >* >* stream, ThreadContext* context )
   : Parent( context ),
     mFrameStream( dynamic_cast< FrameStream* >( stream ) )
{
   AssertFatal( mFrameStream != NULL, "TheoraTexture::FrameReadItem::FrameReadItem() - expecting stream of type 'FrameStream'" );
   
   mAsyncState = mFrameStream->mAsyncState;
   
   // Assign a TheoraTextureFrame record to us.  The nature of
   // AsyncBufferedInputStream ensures that we are always serial
   // here so this is thread-safe.
   
   mFrame = &mFrameStream->mFrames[ mFrameStream->mFrameIndex ];
   mFrameStream->mFrameIndex = ( mFrameStream->mFrameIndex + 1 ) % FrameStream::NUM_FRAME_RECORDS;
}

//-----------------------------------------------------------------------------

void TheoraTexture::FrameReadItem::execute()
{
   // Read Theora frame data.
   
   OggTheoraFrame* frame;
   if( mFrameStream->getSourceStream()->read( &frame, 1 ) != 1 )
      return;
      
   // Copy the data into the texture.
   
   OggTheoraDecoder* decoder = mAsyncState->getTheora();
   const U32 height = decoder->getFrameHeight();
   const U32 framePitch = decoder->getFrameWidth() * 4;
      
   GFXLockedRect* rect = mFrame->mLockedRect;
   if( rect )
   {
      const U32 usePitch = getMin(framePitch, mFrame->mTexture->getWidth() * 4);  
      const U32 maxHeight = getMin(height, mFrame->mTexture->getHeight());  
      if( (framePitch == rect->pitch) && (height == maxHeight) )  
         dMemcpy( rect->bits, frame->data, rect->pitch * height );
      else
      {
         // Scanline length does not match.  Copy line by line.
         
         U8* dst = rect->bits;
         U8* src = ( U8* ) frame->data;

         // Center the video if it is too big for the texture mode  
         if ( height > maxHeight )  
            src += framePitch * ((height - maxHeight) / 2);  
         if ( framePitch > usePitch )  
            src += (framePitch - usePitch) / 2;  
         for( U32 i = 0; i < maxHeight; ++ i )  
         {
            dMemcpy( dst, src, usePitch );  
            dst += rect->pitch;
            src += framePitch;
         }
      }
   }
   #ifdef TORQUE_DEBUG
   else
      Platform::outputDebugString( "[TheoraTexture] texture not locked on frame %i", frame->mFrameNumber );
   #endif
   
   // Copy frame metrics.
   
   mFrame->mFrameNumber = frame->mFrameNumber;
   mFrame->mFrameTime = frame->mFrameTime;
   mFrame->mFrameDuration = frame->mFrameDuration;

   // Yield the frame packet back to the Theora decoder.
   
   decoder->reusePacket( frame );
   
   // Buffer the frame.
   
   mFrameStream->_onArrival( mFrame );
}

//=============================================================================
//    TheoraTexture::FrameStream implementation.
//=============================================================================

//-----------------------------------------------------------------------------

TheoraTexture::FrameStream::FrameStream( AsyncState* asyncState, bool looping )
   : Parent( asyncState->getTheora(), 0, FRAME_READ_AHEAD, looping ),
     mAsyncState( asyncState ),
     mFrameIndex( 0 )
{
   // Create the textures.
   
   OggTheoraDecoder* theora = asyncState->getTheora();
   const U32 width = theora->getFrameWidth();
   const U32 height = theora->getFrameHeight();
   
   for( U32 i = 0; i < NUM_FRAME_RECORDS; ++ i )
   {      
      mFrames[ i ].mTexture.set(
         width,
         height,
         GFXFormatR8G8B8A8,
         &GFXTheoraTextureProfile,
         String::ToString( "Theora texture frame buffer %i (%s:%i)", i, __FILE__, __LINE__ )
      );
   }
   
   acquireTextureLocks();
}

//-----------------------------------------------------------------------------

void TheoraTexture::FrameStream::acquireTextureLocks()
{
   for( U32 i = 0; i < NUM_FRAME_RECORDS; ++ i )
      if( !mFrames[ i ].mLockedRect )
         mFrames[ i ].mLockedRect = mFrames[ i ].mTexture.lock();
}

//-----------------------------------------------------------------------------

void TheoraTexture::FrameStream::releaseTextureLocks()
{
   for( U32 i = 0; i < NUM_FRAME_RECORDS; ++ i )
      if( mFrames[ i ].mLockedRect )
      {
         mFrames[ i ].mTexture.unlock();
         mFrames[ i ].mLockedRect = NULL;
      }
}

//=============================================================================
//    TheoraTexture::AsyncState implementation.
//=============================================================================

//-----------------------------------------------------------------------------

TheoraTexture::AsyncState::AsyncState( const ThreadSafeRef< OggInputStream >& oggStream, bool looping )
   : mOggStream( oggStream ),
     mTheoraDecoder( dynamic_cast< OggTheoraDecoder* >( oggStream->getDecoder( "Theora" ) ) ),
     mVorbisDecoder( dynamic_cast< OggVorbisDecoder* >( oggStream->getDecoder( "Vorbis" ) ) ),
     mCurrentTime( 0 )
{
   if( mTheoraDecoder )
   {
      mTheoraDecoder->setTimeSource( this );
      mFrameStream = new FrameStream( this, looping );
   }
}

//-----------------------------------------------------------------------------

TheoraTextureFrame* TheoraTexture::AsyncState::readNextFrame()
{
   TheoraTextureFrame* frame;
   if( mFrameStream->read( &frame, 1 ) )
      return frame;
   else
      return NULL;
}

//-----------------------------------------------------------------------------

void TheoraTexture::AsyncState::start()
{
   mFrameStream->start();
}

//-----------------------------------------------------------------------------

void TheoraTexture::AsyncState::stop()
{
   mFrameStream->stop();
}

//-----------------------------------------------------------------------------

bool TheoraTexture::AsyncState::isAtEnd()
{
   return mOggStream->isAtEnd();
}

//=============================================================================
//    TheoraTexture implementation.
//=============================================================================

//-----------------------------------------------------------------------------

TheoraTexture::TheoraTexture()
   : mPlaybackQueue( NULL ),
     mCurrentFrame( NULL ),
     mIsPaused( true )
{
   GFXTextureManager::addEventDelegate( this, &TheoraTexture::_onTextureEvent );
}

//-----------------------------------------------------------------------------

TheoraTexture::~TheoraTexture()
{
   GFXTextureManager::removeEventDelegate( this, &TheoraTexture::_onTextureEvent );
   _reset();
}

//-----------------------------------------------------------------------------

void TheoraTexture::_reset()
{
   // Stop the async streams.
      
   if( mAsyncState != NULL )
      mAsyncState->stop();
      
   // Delete the playback queue.

   if( mPlaybackQueue )
      SAFE_DELETE( mPlaybackQueue );
            
   // Kill the sound source.

   if( mSFXSource != NULL )
   {
      mSFXSource->stop();
      SFX_DELETE( mSFXSource );
      mSFXSource = NULL;
   }
         
   mLastFrameNumber = 0;
   mNumDroppedFrames = 0;
   mCurrentFrame = NULL;
   mAsyncState = NULL; 
   mIsPaused = false;  
   mPlaybackTimer.reset();
}

//-----------------------------------------------------------------------------

void TheoraTexture::_onTextureEvent( GFXTexCallbackCode code )
{
   switch( code )
   {
      case GFXZombify:
         mCurrentFrame = NULL;
         if( mAsyncState )
         {
            // Blast out work items and then release all texture locks.
            
            ThreadPool::GLOBAL().flushWorkItems();
            mAsyncState->getFrameStream()->releaseTextureLocks();
            
            // The Theora decoder does not implement seeking at the moment,
            // so we absolutely want to make sure we don't fall behind too far or
            // we may end up having the decoder go crazy trying to skip through
            // Ogg packets (even just reading these undecoded packets takes a
            // lot of time).  So, for the time being, just pause playback when
            // we go zombie.
            
            if( mSFXSource )
               mSFXSource->pause();
            else
               mPlaybackTimer.pause();
         }
         break;
         
      case GFXResurrect:
         if( mAsyncState )
         {
            // Reacquire texture locks.
            
            mAsyncState->getFrameStream()->acquireTextureLocks();
            
            // Resume playback if we have paused it.
            
            if( !mIsPaused )
            {
               if( mSFXSource )
                  mSFXSource->play();
               else
                  mPlaybackTimer.start();
            }
         }
         break;
   }
}

//-----------------------------------------------------------------------------

void TheoraTexture::_initVideo()
{
   OggTheoraDecoder* theora = _getTheora();
         
   // Set the decoder's pixel output format to match
   // the texture format.
   
   OggTheoraDecoder::PacketFormat format;
   format.mFormat = GFXFormatR8G8B8A8;
   format.mPitch = GFXFormatInfo( format.mFormat ).getBytesPerPixel() * theora->getFrameWidth();
   
   theora->setPacketFormat( format );
}

//-----------------------------------------------------------------------------

void TheoraTexture::_initAudio( const ThreadSafeRef< SFXStream >& stream )
{
   // Create an SFXDescription if we don't have one.
   
   if( !mSFXDescription )
   {
      SFXDescription* description = new SFXDescription;
      description->mIsStreaming = true;
      description->registerObject();
      description->setAutoDelete( true );
      
      mSFXDescription = description;
   }
   
   // Create an SFX memory stream that consumes the output
   // of the Vorbis decoder.
   
   ThreadSafeRef< SFXStream > sfxStream = stream;
   if( !sfxStream )
   {
      OggVorbisDecoder* vorbis = _getVorbis();
      SFXFormat sfxFormat( vorbis->getNumChannels(),
                           vorbis->getNumChannels() * 16,
                           vorbis->getSamplesPerSecond() );
                           
      sfxStream = new SFXMemoryStream( sfxFormat, vorbis );
   }
   
   // Create the SFXSource.
   
   mSFXSource = SFX->createSourceFromStream( sfxStream, mSFXDescription );
}

//-----------------------------------------------------------------------------

TheoraTexture::TimeSourceType* TheoraTexture::_getTimeSource() const
{
   if( mSFXSource != NULL )
      return mSFXSource;
   else
      return ( TimeSourceType* ) &mPlaybackTimer;
}

//-----------------------------------------------------------------------------

U32 TheoraTexture::getWidth() const
{
   return _getTheora()->getFrameWidth();
}

//-----------------------------------------------------------------------------

U32 TheoraTexture::getHeight() const
{
   return _getTheora()->getFrameHeight();
}

//-----------------------------------------------------------------------------

bool TheoraTexture::setFile( const String& filename, SFXDescription* desc )
{
   _reset();
   
   if( filename.isEmpty() )
      return true;
   
   // Check SFX profile.
   
   if( desc && !desc->mIsStreaming )
   {
      Con::errorf( "TheoraTexture::setFile - Not a streaming SFXDescription" );
      return false;
   }
   mSFXDescription = desc;

   // Open the Theora file.
   
   Stream* stream = FileStream::createAndOpen( filename, Torque::FS::File::Read );
   if( !stream )
   {
      Con::errorf( "TheoraTexture::setFile - Theora file '%s' not found.", filename.c_str() );
      return false;
   }
   
   // Create the OGG stream.
   
   Con::printf( "TheoraTexture - Loading file '%s'", filename.c_str() );
   
   ThreadSafeRef< OggInputStream > oggStream = new OggInputStream( stream );
   oggStream->addDecoder< OggTheoraDecoder >();
   #ifndef SPLIT_VORBIS
   oggStream->addDecoder< OggVorbisDecoder >();
   #endif
   
   if( !oggStream->init() )
   {
      Con::errorf( "TheoraTexture - Failed to initialize OGG stream" );
      return false;
   }
   
   mFilename = filename;
   mAsyncState = new AsyncState( oggStream, desc ? desc->mIsLooping : false );   
      
   // Set up video.
   
   OggTheoraDecoder* theoraDecoder = _getTheora();
   if( !theoraDecoder )
   {
      Con::errorf( "TheoraTexture - '%s' is not a Theora file", filename.c_str() );
      mAsyncState = NULL;
      return false;
   }
   
   Con::printf( "   - Theora: %ix%i pixels, %.02f fps, %s format",
      theoraDecoder->getFrameWidth(),
      theoraDecoder->getFrameHeight(),
      theoraDecoder->getFramesPerSecond(),
      GetPixelFormatName( theoraDecoder->getDecoderPixelFormat() ) );
      
   _initVideo();
   
   // Set up sound if we have it.  For performance reasons, create
   // a separate physical stream for the Vorbis sound data rather than
   // using the bitstream from the multiplexed OGG master stream.  The
   // contention caused by the OGG page/packet feeding will otherwise
   // slow us down significantly.
   
   #ifdef SPLIT_VORBIS
      
   stream = FileStream::createAndOpen( filename, Torque::FS::File::Read );
   if( stream )
   {
      ThreadSafeRef< SFXStream > vorbisStream = SFXVorbisStream::create( stream );
      if( !vorbisStream )
      {
         Con::errorf( "TheoraTexture - could not create Vorbis stream for '%s'", filename.c_str() );
         
         // Stream is deleted by SFXVorbisStream.
      }
      else
      {
         Con::printf( "   - Vorbis: %i channels, %i kHz",
            vorbisStream->getFormat().getChannels(),
            vorbisStream->getFormat().getSamplesPerSecond() / 1000 );
            
         _initAudio( vorbisStream );
      }
   }
   
   #else
   
   OggVorbisDecoder* vorbisDecoder = _getVorbis();
   if( vorbisDecoder )
   {
      Con::printf( "   - Vorbis: %i bits, %i channels, %i kHz",
         vorbisDecoder->getNumChannels(),
         vorbisDecoder->getSamplesPerSecond() / 1000 );

      _initAudio();
   }
   
   #endif
      
   // Initiate the background request chain.
   
   mAsyncState->start();
   
   return true;
}

//-----------------------------------------------------------------------------

bool TheoraTexture::isPlaying() const
{
   if( !mAsyncState || !mCurrentFrame )
      return false;
      
   if( mSFXSource )
      return mSFXSource->isPlaying();
   else
      return mPlaybackTimer.isStarted();
}

//-----------------------------------------------------------------------------

void TheoraTexture::play()
{
   if( isPlaying() )
      return;
      
   if( !mAsyncState )
      setFile( mFilename, mSFXDescription );
      
   // Construct playback queue that sync's to our time source,
   // writes to us, and drops outdated packets.
   
   if( !mPlaybackQueue )
      mPlaybackQueue = new PlaybackQueueType( 1, _getTimeSource(), this, 0, true );
         
   // Start playback.
   
   if( mSFXSource )
      mSFXSource->play();
   else
      mPlaybackTimer.start();
      
   mIsPaused = false;
}

//-----------------------------------------------------------------------------

void TheoraTexture::pause()
{
   if( mSFXSource )
      mSFXSource->pause();
   else
      mPlaybackTimer.pause();
      
   mIsPaused = true;
}

//-----------------------------------------------------------------------------

void TheoraTexture::stop()
{
   _reset();
}

//-----------------------------------------------------------------------------

void TheoraTexture::refresh()
{
   PROFILE_SCOPE( TheoraTexture_refresh );
   
   if( !mAsyncState || !mPlaybackQueue )
      return;
      
   // Synchronize the async state to our current time.
   // Unfortunately, we cannot set the Theora decoder to
   // synchronize directly with us as our lifetime and the
   // lifetime of our time sources isn't bound to the
   // threaded state.
   
   mAsyncState->syncTime( _getTimeSource()->getPosition() );
            
   // Update the texture, if necessary.
   
   bool haveFrame = false;
   while( mPlaybackQueue->needPacket() )
   {
      // Lock the current frame.
      
      if( mCurrentFrame && !mCurrentFrame->mLockedRect )
         mCurrentFrame->mLockedRect = mCurrentFrame->mTexture.lock();
         
      // Try to read a new frame.
         
      TheoraTextureFrame* frame = mAsyncState->readNextFrame();
      if( !frame )
         break;
         
      // Submit frame to queue.
         
      mPlaybackQueue->submitPacket(
         frame,
         frame->mFrameDuration * 1000.f,
         false,
         frame->mFrameTime * 1000.f
      );
      
      // See if we have dropped frames.
      
      if( frame->mFrameNumber != mLastFrameNumber + 1 )
         mNumDroppedFrames += frame->mFrameNumber - mLastFrameNumber - 1;
      mLastFrameNumber = frame->mFrameNumber;
      
      haveFrame = true;
   }
   
   // Unlock current frame.
   
   if( mCurrentFrame && mCurrentFrame->mLockedRect )
   {
      mCurrentFrame->mTexture.unlock();
      mCurrentFrame->mLockedRect = NULL;
   }
   
   // Release async state if we have reached the
   // end of the Ogg stream.
   
   if( mAsyncState->isAtEnd() && !haveFrame )
      _reset();
}

//-----------------------------------------------------------------------------

void TheoraTexture::write( TheoraTextureFrame* const* frames, U32 num )
{
   if( !num )
      return;
      
   mCurrentFrame = frames[ num - 1 ]; // Only used last.
}

#endif // TORQUE_OGGTHEORA
