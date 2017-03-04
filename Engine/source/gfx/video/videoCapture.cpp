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
#include "gfx/video/videoCapture.h"

#include "console/console.h"
#include "core/strings/stringFunctions.h"
#include "core/util/journal/journal.h"
#include "core/module.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxTextureManager.h"
#include "console/engineAPI.h"


Vector<VideoCapture::EncoderFactory> VideoCapture::mEncoderFactoryFnList;

MODULE_BEGIN( VideoCapture )

   MODULE_INIT_BEFORE( GFX )
   MODULE_SHUTDOWN_BEFORE( GFX )

   MODULE_INIT
   {
      ManagedSingleton< VideoCapture >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      VIDCAP->end();
      ManagedSingleton< VideoCapture >::deleteSingleton();
   }

MODULE_END;

VideoCapture::VideoCapture() : 
   mEncoder(NULL),
   mIsRecording(false),
   mCanvas(NULL),
   mFrameGrabber(NULL),
   mWaitingForCanvas(false),
   mResolution(0,0),
   mFrameRate(30.0f),
   mEncoderName("THEORA"),
   mFileName(""),
   mMsPerFrameError(0)
{     
}

S32 VideoCapture::getMsPerFrame()
{
   //Add accumulated error to ms per frame before rounding
   F32 roundTime = mFloor(mMsPerFrame + mMsPerFrameError + 0.5f);

   //Accumulate the rounding errors
   mMsPerFrameError += mMsPerFrame - roundTime;
      
   return (S32)roundTime;
}

void VideoCapture::begin( GuiCanvas* canvas )
{
   // No longer waiting for a canvas
   mWaitingForCanvas = false;

   // No specified file
   if (mFileName.isEmpty())
   {
      Con::errorf("VideoCapture: no file specified!");
      return;
   }

   // No framegrabber, cannot capture
   if (mFrameGrabber == NULL)
   {
      Con::errorf("VideoCapture: cannot capture without a VideoFrameGrabber! One should be created in the GFXDevice initialization!");
      return;
   }

   // Set the active encoder
   if (!initEncoder(mEncoderName))
      return;

   // Store the canvas, so we know which one to capture from
   mCanvas = canvas;

   // If the resolution is zero, get the current video mode
   if (mResolution.isZero())
      mResolution = mCanvas->getPlatformWindow()->getVideoMode().resolution;

   // Set the encoder file, framerate and resolution
   mEncoder->setFile(mFileName);
   mEncoder->setFramerate( &mFrameRate );   
   mEncoder->setResolution( &mResolution );   

   // The frame grabber must know about the resolution as well, since it'll do the resizing for us
   mFrameGrabber->setOutResolution( mResolution );
   
   // Calculate the ms per frame
   mMsPerFrame = 1000.0f / mFrameRate;

   // Start the encoder
   if (!mEncoder->begin())
      return;

   // We're now recording
   mIsRecording = true;
   mNextFramePosition = 0.0f;
}

void VideoCapture::end()
{
   if (!mIsRecording)
      return;

   if (mEncoder && !mEncoder->end())
      Con::errorf("VideoCapture: an error has ocurred while closing the video stream");

   // Garbage collect the processed bitmaps
   deleteProcessedBitmaps();
   
   delete mEncoder;
   mEncoder = NULL;

   mIsRecording = false;
}

void VideoCapture::capture()
{   
   // If this is the first frame, capture and encode it right away   
   if (mNextFramePosition == 0.0f)
   {
      mVideoCaptureStartTime = Platform::getVirtualMilliseconds();
      mCapturedFramePos = -1.0f;
   }

   // Calculate the frame position for this captured frame
   U32 frameTimeMs = Platform::getVirtualMilliseconds() - mVideoCaptureStartTime;
   F32 framePosition = (F32)frameTimeMs / mMsPerFrame;

   // Repeat until the current frame is captured
   while (framePosition > mCapturedFramePos)
   {
      // If the frame position is closer to the next frame position 
      // than the previous one capture it
      if ( mFabs(framePosition - mNextFramePosition) < mFabs(mCapturedFramePos - mNextFramePosition) )
      {
         mFrameGrabber->captureBackBuffer();      
         mCapturedFramePos  = framePosition;
      }
      
      // If the new frame position is greater or equal than the next frame time
      // tell the framegrabber to make bitmaps out from the last captured backbuffer until the video catches up
      while ( framePosition >= mNextFramePosition )
      {
         mFrameGrabber->makeBitmap();        
         mNextFramePosition++;
      }
   }

   // Fetch bitmaps from the framegrabber and encode them
   GBitmap *bitmap = NULL;
   while ( (bitmap = mFrameGrabber->fetchBitmap()) != NULL )
   {     
      //mEncoder->pushProcessedBitmap(bitmap);                 
      if (!mEncoder->pushFrame(bitmap))
      {
          Con::errorf("VideoCapture: an error occurred while encoding a frame. Recording aborted.");
          end();
          break;
      }
   }

   // Garbage collect the processed bitmaps
   deleteProcessedBitmaps();
}


void VideoCapture::registerEncoder( const char* name, VideoEncoderFactoryFn factoryFn )
{
   mEncoderFactoryFnList.increment();
   mEncoderFactoryFnList.last().name = name;
   mEncoderFactoryFnList.last().factory = factoryFn;
}


bool VideoCapture::initEncoder( const char* name )
{
   if ( mEncoder )
   {
      Con::errorf("VideoCapture:: cannot change video encoder while capturing! Stop the capture first!");
      return false;
   }

   // Try creating an encoder based on the name
   for (U32 i=0; i<mEncoderFactoryFnList.size(); i++)
   {
      if (dStricmp(name, mEncoderFactoryFnList[i].name) == 0)
      {
         mEncoder = mEncoderFactoryFnList[i].factory();
         return true;
      }
   }

   //If we got here there's no encoder matching the speficied name
   Con::errorf("\"%s\" isn't a valid encoder!", name);
   return false;
}

void VideoCapture::deleteProcessedBitmaps()
{
   if (mEncoder == NULL)
      return;

   //Grab bitmaps processed by our encoder
   GBitmap* bitmap = NULL;
   while ( (bitmap = mEncoder->getProcessedBitmap()) != NULL )
      mBitmapDeleteList.push_back(bitmap);

   //Now delete them (or not... se below)
   while ( mBitmapDeleteList.size() )
   {
      bitmap = mBitmapDeleteList[0];
      mBitmapDeleteList.pop_front();

      // Delete the bitmap only if it's the different than the next one (or it's the last one).
      // This is done because repeated frames re-use the same GBitmap object
      // and thus their pointers will appearl multiple times in the list      
      if (mBitmapDeleteList.size() == 0 || bitmap != mBitmapDeleteList[0])
         delete bitmap;
   }
}

///----------------------------------------------------------------------

///----------------------------------------------------------------------

void VideoEncoder::setFile( const char* path )
{
   mPath = path;
}

GBitmap* VideoEncoder::getProcessedBitmap()
{
   GBitmap* bitmap = NULL;
   if (mProcessedBitmaps.tryPopFront(bitmap))
      return bitmap;   
   return NULL;
}

void VideoEncoder::pushProcessedBitmap( GBitmap* bitmap )
{
   mProcessedBitmaps.pushBack(bitmap);
}

///----------------------------------------------------------------------

///----------------------------------------------------------------------

GBitmap* VideoFrameGrabber::fetchBitmap()
{
   if (mBitmapList.size() == 0)
      return NULL;
   
   GBitmap *bitmap = mBitmapList.first();
   mBitmapList.pop_front();
   return bitmap;
}

VideoFrameGrabber::VideoFrameGrabber()
{
   GFXTextureManager::addEventDelegate( this, &VideoFrameGrabber::_onTextureEvent );
}

VideoFrameGrabber::~VideoFrameGrabber()
{
   GFXTextureManager::removeEventDelegate( this, &VideoFrameGrabber::_onTextureEvent );
}

void VideoFrameGrabber::_onTextureEvent(GFXTexCallbackCode code)
{
   if ( code == GFXZombify )
      releaseTextures();
}

///----------------------------------------------------------------------

///----------------------------------------------------------------------
//WLE - Vince
//Changing the resolution to Point2I::Zero instead of the Point2I(0,0) better to use constants.
DefineEngineFunction( startVideoCapture, void, 
   ( GuiCanvas *canvas, const char *filename, const char *encoder, F32 framerate, Point2I resolution ),
   ( "THEORA", 30.0f, Point2I::Zero ),
   "Begins a video capture session.\n"
   "@see stopVideoCapture\n"
   "@ingroup Rendering\n" )
{
#ifdef TORQUE_DEBUG
   Con::errorf("Recording video is disabled in debug!");
#else
   if ( !canvas )
   {
      Con::errorf("startVideoCapture -Please specify a GuiCanvas object to record from!");
      return;
   }

   VIDCAP->setFilename( filename );   
   VIDCAP->setEncoderName( encoder );   
   VIDCAP->setFramerate( framerate );
   
   if ( !resolution.isZero() )
      VIDCAP->setResolution(resolution);

   VIDCAP->begin(canvas);
#endif
}

DefineEngineFunction( stopVideoCapture, void, (),,
   "Stops the video capture session.\n"
   "@see startVideoCapture\n"   
   "@ingroup Rendering\n" )
{
   VIDCAP->end();
}

DefineEngineFunction( playJournalToVideo, void, 
   ( const char *journalFile, const char *videoFile, const char *encoder, F32 framerate, Point2I resolution ),
   ( nullAsType<const char*>(), "THEORA", 30.0f, Point2I::Zero ),
   "Load a journal file and capture it video.\n"
   "@ingroup Rendering\n" )
{
   if ( !videoFile )
      videoFile = journalFile;

   VIDCAP->setFilename( Torque::Path( videoFile ).getFileName() );   
   VIDCAP->setEncoderName( encoder );   
   VIDCAP->setFramerate( framerate );

   if ( !resolution.isZero() )
      VIDCAP->setResolution(resolution);

   VIDCAP->waitForCanvas();

   Journal::Play( journalFile );
}
