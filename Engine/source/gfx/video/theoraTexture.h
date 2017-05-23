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

#ifndef _THEORATEXTURE_H_
#define _THEORATEXTURE_H_

#ifdef TORQUE_OGGTHEORA

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
   #include "gfx/gfxTextureHandle.h"
#endif
#ifndef _ASYNCPACKETQUEUE_H_
   #include "platform/async/asyncPacketQueue.h"
#endif
#ifndef _ASYNCBUFFEREDSTREAM_H_
   #include "platform/async/asyncBufferedStream.h"
#endif
#ifndef _TIMESOURCE_H_
   #include "core/util/timeSource.h"
#endif
#ifndef _THREADSAFEREFCOUNT_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif
#ifndef _RAWDATA_H_
   #include "core/util/rawData.h"
#endif
#ifndef _SIMOBJECT_H_
   #include "console/simObject.h"
#endif
#ifndef _SFXSTREAM_H_
   #include "sfx/sfxStream.h"
#endif
#ifndef _OGGTHEORADECODER_H_
   #include "core/ogg/oggTheoraDecoder.h"
#endif
#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif



class SFXDescription;
class SFXSound;
class SFXVorbisStream;

class OggInputStream;
class OggVorbisDecoder;

class Stream;


/// A single frame in the video frame stream.
///
/// Frames are uploaded directly into textures by the asynchronous
/// streaming system.  This offloads as much work as possible to the worker
/// threads and guarantees the smoothest possible playback.
///
/// Frame records are re-used and are managed directly by the video frame
/// stream.  The number of textures concurrently used by a Theora stream
/// is determined by its stream read-ahead.
class TheoraTextureFrame
{
   public:
   
      typedef void Parent;
               
      /// The texture containing the video frame.
      GFXTexHandle mTexture;
      
      /// The locked rectangle, if the texture is currently locked.
      /// Frames will remain in locked state except if currently displayed.
      GFXLockedRect* mLockedRect;
      
      ///
      U32 mFrameNumber;
      
      /// The play time in seconds at which to display this frame.
      F32 mFrameTime;
      
      /// The duration in seconds to display this frame.
      F32 mFrameDuration;
      
      TheoraTextureFrame()
         : mLockedRect( NULL ), mFrameNumber(0), mFrameTime(0.0f), mFrameDuration(0.0f)
      {
      }
};


inline void destructSingle( TheoraTextureFrame* frame )
{
   // Do nothing.
}


/// TheoraTexture decodes Ogg Theora files, and their audio.
///
/// TheoraTexture objects can be used similarly to TextureObjects. Just
/// set the video, call play(), and then refresh every frame to get the
/// latest video. Audio happens automagically.
///
/// @note Uses Theora and ogg libraries which are Copyright (C) Xiph.org Foundation
class TheoraTexture : private IOutputStream< TheoraTextureFrame* >,
                      public IPositionable< U32 >
{
   public:
   
      typedef void Parent;
      
   protected:
         
      typedef IPositionable< U32 > TimeSourceType;
      typedef GenericTimeSource<> TimerType;      
      typedef AsyncPacketQueue< TheoraTextureFrame*, TimeSourceType*, IOutputStream< TheoraTextureFrame* >*, F32 > PlaybackQueueType;
      
      class FrameStream;
      class AsyncState;
      friend class GuiTheoraCtrl; // accesses OggTheoraDecoder to set transcoder
      
      /// Parameters for tuning streaming behavior.
      enum
      {
         /// Number of textures to load in background.
         FRAME_READ_AHEAD = 6,
      };
            
      /// WorkItem that reads a frame from a Theora decoder and uploads it into a TheoraTextureFrame.
      ///
      /// Loading directly into textures moves the costly uploads out of the main thread into worker
      /// threads.  The downside to this is that since we cannot do GFX work on the worker threads,
      /// we need to expect textures to get to us in locked state.
      class FrameReadItem : public ThreadWorkItem
      {
         public:
         
            typedef ThreadWorkItem Parent;
            
         protected:
         
            /// The asynchronous state we belong to.  This reference
            /// ensures that all our streaming state stays live for as long as our
            /// work item is in the pipeline.
            ThreadSafeRef< AsyncState > mAsyncState;
            
            ///
            FrameStream* mFrameStream;
            
            /// The frame texture we are loading.
            TheoraTextureFrame* mFrame;
            
            // WorkItem.
            virtual void execute();
            
         public:
         
            ///
            FrameReadItem( AsyncBufferedInputStream< TheoraTextureFrame*, IInputStream< OggTheoraFrame* >* >* stream,
                           ThreadPool::Context* context );
      };
      
      /// Stream filter that turns a stream of OggTheoraFrames into a buffered background stream of TheoraTextureFrame
      /// records.  
      ///
      /// This streams allocates a fixed amount 'M' of TheoraTextureFrames.  Reading the n-th frame from the stream, will
      /// automatically invalidate the (n-M)-th frame.
      class FrameStream : public AsyncSingleBufferedInputStream< TheoraTextureFrame*, IInputStream< OggTheoraFrame* >*, FrameReadItem >
      {
         public:
         
            typedef AsyncSingleBufferedInputStream< TheoraTextureFrame*, IInputStream< OggTheoraFrame* >*, FrameReadItem > Parent;
            
         protected:
         
            friend class FrameReadItem;
            
            enum
            {
               /// Number of TheoraTextureFrame records to allocate.
               ///
               /// We need to pre-allocate TheoraTextureFrames as we cannot do GFX operations
               /// on the fly on worker threads.  This number corresponds to the length of the
               /// buffering queue plus one record that will be returned to the user as the
               /// current frame record.
               NUM_FRAME_RECORDS = FRAME_READ_AHEAD + 1
            };
            
            /// Asynchronous state of the texture object.
            /// This is *NOT* a ThreadSafeRef to not create a cycle.
            AsyncState* mAsyncState;
            
            /// Wrap-around index into "mFrames."
            U32 mFrameIndex;
            
            /// The pre-allocated TheoraTextureFrames.
            TheoraTextureFrame mFrames[ NUM_FRAME_RECORDS ];
                     
         public:
         
            ///
            FrameStream( AsyncState* asyncState, bool looping = false );
            
            ///
            void acquireTextureLocks();
            
            ///
            void releaseTextureLocks();
      };
      
      /// Encapsulation of compound asynchronous state.  Allows releasing
      /// the entire state in one go.
      class AsyncState :   public ThreadSafeRefCount< AsyncState >,
                           private IPositionable< U32 >
      {
         public:
         
            typedef void Parent;
            friend class FrameStream; // mDecoderBufferStream
         
         protected:
         
            typedef AsyncSingleBufferedInputStream< OggTheoraFrame* > DecoderBufferStream;
         
            /// Last synchronization position in the video stream.  This is what the
            /// Theora decoder gets passed to see if frames are outdated.
            U32 mCurrentTime;
         
            /// The Ogg master stream.
            ThreadSafeRef< OggInputStream > mOggStream;
         
            /// The raw video decoding stream.
            OggTheoraDecoder* mTheoraDecoder;
            
            /// The raw sound decoding stream; NULL if no Vorbis in video or if
            /// Vorbis is streamed separately.
            OggVorbisDecoder* mVorbisDecoder;
            
            /// The background-buffered frame stream.
            ThreadSafeRef< FrameStream > mFrameStream;
            
         public:
            
            ///
            AsyncState( const ThreadSafeRef< OggInputStream >& oggStream, bool looping = false );
            
            /// Return the Theora decoder substream.
            OggTheoraDecoder* getTheora() const { return mTheoraDecoder; }
            
            /// Return the Vorbis decoder substream.
            /// @note If Vorbis streaming is split out into a separate physical substream,
            ///   this method will always return NULL even if Vorbis sound is being used.
            OggVorbisDecoder* getVorbis() const { return mVorbisDecoder; }
            
            ///
            const ThreadSafeRef< FrameStream >& getFrameStream() const { return mFrameStream; }
                        
            ///
            TheoraTextureFrame* readNextFrame();
                        
            ///
            void start();
            
            ///
            void stop();
            
            ///
            bool isAtEnd();
            
            ///
            void syncTime( U32 ms ) { mCurrentTime = ms; }
            
         private:
         
            // IPositionable.
            virtual U32 getPosition() const { return mCurrentTime; }
            virtual void setPosition( U32 pos ) {}
      };
      
      /// The Theora video file.
      String mFilename;
      
      /// The SFXDescription used for sound playback.  Synthesized if not provided.
      SimObjectPtr< SFXDescription > mSFXDescription;
                  
      /// If there's a Vorbis stream, this is the sound source used for playback.
      /// Playback will synchronize to this source then.
      SimObjectPtr< SFXSound > mSFXSource;
      
      /// The current frame.
      TheoraTextureFrame* mCurrentFrame;
                        
      /// The queue that synchronizes the writing of frames to the TheoraTexture.
      PlaybackQueueType* mPlaybackQueue;
      
      /// The timer for synchronizing playback when there is no audio stream
      /// to synchronize to.
      TimerType mPlaybackTimer;
                  
      /// Our threaded state.
      ThreadSafeRef< AsyncState > mAsyncState;
      
      ///
      bool mIsPaused;
      
      ///
      U32 mLastFrameNumber;
      
      /// Number of frames we have dropped so far.
      U32 mNumDroppedFrames;

      /// Release all dynamic playback state.
      void _reset();
      
      /// Initialize video playback.
      void _initVideo();
      
      /// Initialize audio playback.
      void _initAudio( const ThreadSafeRef< SFXStream >& stream = NULL );
      
      /// Return the Theora decoder stream or NULL.
      OggTheoraDecoder* _getTheora() const { return ( mAsyncState != NULL ? mAsyncState->getTheora() : NULL ); }
      
      /// Return the Vorbis decoder stream or NULL.
      OggVorbisDecoder* _getVorbis() const { return ( mAsyncState != NULL ? mAsyncState->getVorbis() : NULL ); }
      
      /// Return the object that is acting as our time source.
      TimeSourceType* _getTimeSource() const;
      
      ///
      void _onTextureEvent( GFXTexCallbackCode code );
                  
      // IOutputStream.
      virtual void write( TheoraTextureFrame* const* frames, U32 num );

   public:
   
      ///
      TheoraTexture();
            
      ~TheoraTexture();

      /// Return the width of a single video frame in pixels.
      U32 getWidth() const;
      
      /// Return the height of a single video frame in pixels.
      U32 getHeight() const;
      
      /// Return the filename of the Theora video file loaded by the player.
      const String& getFilename() const { return mFilename; }

      /// Load the given Theora video file.  Set up audio using the given SFXDescription (must have
      /// streaming enabled).  If no SFXDescription is provided, a default description is used.
      bool setFile( const String& filename, SFXDescription* desc = NULL );
      
      /// Start video playback.
      void play();
      
      /// Pause video playback.
      void pause();
      
      /// Stop video playback.
      void stop();
      
      /// Refresh the current frame.  This should be called before getTexture() to ensure that
      /// the texture returned by getTexture() contains up-to-date contents.
      void refresh();
      
      /// Return true if a video file has been loaded and is ready for playback.
      bool isReady() const { return ( mAsyncState != NULL ); }
      
      /// Return true if the video is currently playing.
      bool isPlaying() const;
      
      /// Return true if the video is currently paused.
      bool isPaused() const { return mIsPaused; }
      
      /// Return the sequence number of the current frame.  Starts at 0.
      U32 getFrameNumber() const { return mCurrentFrame->mFrameNumber; }
      
      /// Return the playback position of the current frame.
      F32 getFrameTime() const { return mCurrentFrame->mFrameTime; }
      
      /// Return the number of frames that have been dropped so far.
      U32 getNumDroppedFrames() const { return mNumDroppedFrames; }
      
      /// Return the texture containing the current frame.  Call refresh() first
      /// to ensure the texture contents are up-to-date.
      const GFXTexHandle& getTexture() const { return mCurrentFrame->mTexture; }
      GFXTexHandle& getTexture() { return mCurrentFrame->mTexture; }
      
      // IPositionable.
      virtual U32 getPosition() const { return _getTimeSource()->getPosition(); }
      virtual void setPosition( U32 pos ) {} // Not (yet?) implemented.
};

#endif // TORQUE_OGGTHEORA
#endif // !_THEORATEXTURE_H_
