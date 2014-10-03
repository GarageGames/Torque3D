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

#ifndef _VIDEOCAPTURE_H_
#define _VIDEOCAPTURE_H_

#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif

#ifndef _THREADSAFEDEQUE_H_
#include "platform/threads/threadSafeDeque.h"
#endif


class GuiCanvas;
class VideoFrameGrabber;
class VideoEncoder;
class GBitmap;

typedef VideoEncoder* (*VideoEncoderFactoryFn)();


/// Abstract frame grabber class
/// implementation and initalization depends on video device
class VideoFrameGrabber
{
   friend class VideoCapture;
protected:
   Point2I mResolution; // The resolution used to capture the back buffer (scaling will be used)
   Vector<GBitmap*> mBitmapList; //List of bitmaps created from backbuffer captures

   /// Sets the output frame resolution
   void setOutResolution( const Point2I& res ) { mResolution = res; }   

   /// Pushes a fresh bitmap into our list
   void pushNewBitmap( GBitmap* bitmap ) { mBitmapList.push_back(bitmap); }

   /// Returns one captured bitmap. Returns NULL if there are no more bitmaps.
   GBitmap* fetchBitmap();

   /// Texture event callback
   void _onTextureEvent(GFXTexCallbackCode code);
   
   /// Captures the current backbuffer. If the last capture wasn't made into a bitmap, it will be overriden.   
   virtual void captureBackBuffer() = 0;

   /// Starts converting the last backbuffer capture to a bitmap
   /// Depending on the VideoFrameGrabber implementation, this may not produce a bitmap right away.
   virtual void makeBitmap() = 0;

   /// Releases internal textures
   virtual void releaseTextures() {};

public:
   VideoFrameGrabber();
   virtual ~VideoFrameGrabber();
};


/// Video capture interface class
class VideoCapture
{   
private:
   struct EncoderFactory {
      const char* name;
      VideoEncoderFactoryFn factory;      
   };

   /// List of encoder factory functions
   static Vector<EncoderFactory> mEncoderFactoryFnList;

   // The frame position of the latest backbuffer capture
   F32 mCapturedFramePos;

   /// Our current video encoder
   VideoEncoder* mEncoder;

   /// Our video frame grabber
   VideoFrameGrabber* mFrameGrabber;

   /// The canvas we're recording from
   GuiCanvas* mCanvas;

   /// True if we're recording
   bool mIsRecording;

   /// Time when we captured the previous frame
   U32 mVideoCaptureStartTime;

   /// Frame to be captured next
   F32 mNextFramePosition;

   /// The per-frame time (in milliseconds)
   F32 mMsPerFrame;
   
   /// The framerate we'll use to record
   F32 mFrameRate;

   /// Accumulated error when converting the per-frame-time to integer
   /// this is used to dither the value and keep overall time advancing
   /// correct
   F32 mMsPerFrameError;

   /// Name of the encoder we'll be using
   String mEncoderName;

   /// The video output resolution
   Point2I mResolution;

   /// Output filename
   String mFileName;

   /// Tur if we're waiting for a canvas to bre created before capturing
   bool mWaitingForCanvas;

   /// Vector with bitmaps to delete
   Vector< GBitmap* > mBitmapDeleteList;

   /// Initializes our encoder
   bool initEncoder( const char* name );   

   /// Deletes processed bitmaps
   void deleteProcessedBitmaps();
      
public:
   VideoCapture();
      
   /// Start a video capture session
   void begin( GuiCanvas* canvas );

   /// Captures a new frame
   void capture();
   
   /// Ends a video capture
   void end();

   /// Sets the output filename
   void setFilename( const char* filename ) { mFileName = filename; }

   /// Sets the encoder we'll use
   void setEncoderName( const char* encoder ) { mEncoderName = encoder; }

   /// Sets the framerate
   void setFramerate( F32 fps ) { mFrameRate = fps; }

   /// Sets the video output resolution
   void setResolution(const Point2I& res) { mResolution = res; }

   /// Returns true if we're capturing
   bool isRecording() { return mIsRecording; }

   /// Returns the number of milliseconds per frame
   S32 getMsPerFrame();

   /// Sets the video farme grabber (cannot record without one).
   void setFrameGrabber( VideoFrameGrabber* grabber ) { mFrameGrabber = grabber; }

   /// This will make the video capture begin capturing 
   /// as soon as a GuiCanvas is created
   void waitForCanvas() { mWaitingForCanvas = true; }
   bool isWaitingForCanvas() { return mWaitingForCanvas; }

   /// Registers an encoder
   static void registerEncoder( const char* name, VideoEncoderFactoryFn factoryFn );    

   // For ManagedSingleton.
   static const char* getSingletonName() { return "VideoCapture"; }   
};



/// Abstract video encoder class
class VideoEncoder
{
protected:
   // Video output file path
   String mPath;

   // Video framerate
   F32 mFramerate;

   // Video resolution
   Point2I mResolution;

   // List with bitmaps which are done encoding
   ThreadSafeDeque< GBitmap* > mProcessedBitmaps;
public:
   virtual ~VideoEncoder() { }

   // Stores an encoded bitmap to be dealt with later
   void pushProcessedBitmap( GBitmap* bitmap );
      
public:
   /// Sets the file the encoder will write to
   void setFile( const char* path );

   /// Sets the framerate (and fixes it if its invalid)
   virtual void setFramerate( F32* framerate ) { mFramerate = *framerate; }

   /// Sets the output resolution (and fixes it if its invalid)
   virtual void setResolution( Point2I* resolution ) { mResolution = *resolution; }

   /// Begins accepting frames for encoding
   virtual bool begin() = 0;

   /// Pushes a new frame into the video stream
   virtual bool pushFrame( GBitmap * bitmap ) = 0;

   /// Finishes the encoding and closes the video
   virtual bool end() = 0;

   /// Returns an already encoded bitmap. Video capture will get these and manage their deletion
   GBitmap* getProcessedBitmap();
};

/// Returns the VideoCapture singleton.
#define VIDCAP ManagedSingleton<VideoCapture>::instance()

//-----------------------------------------
/// VIDEO ENCODER REGISTRATION MACRO
#define REGISTER_VIDEO_ENCODER(ClassName, EncoderName)   \
   VideoEncoder* EncoderFactory##EncoderName() { return new ClassName(); } \
   struct __VidEncReg##EncoderName { __VidEncReg##EncoderName() { VideoCapture::registerEncoder( #EncoderName, &EncoderFactory##EncoderName ); } }; \
   static __VidEncReg##EncoderName _gEncoderRegistration;

#endif // !_VIDEOCAPTURE_H_
