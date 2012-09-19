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

#include "videoCapture.h"
#include "core/stream/fileStream.h"
#include "console/console.h"
#include "gfx/bitmap/gBitmap.h"

/// This is a very basic "encoder" that records the video as a series of numbered PNGs
/// Good if you're having problems with container-based formats and need extra flexibility.
class VideoEncoderPNG : public VideoEncoder
{
   U32 mCurrentFrame;

public:
   /// Begins accepting frames for encoding
   bool begin()
   {
      mPath += "\\";
      mCurrentFrame = 0;

      return true;
   }

   /// Pushes a new frame into the video stream
   bool pushFrame( GBitmap * bitmap )
   {
      FileStream fs;
      String framePath = mPath + String::ToString("%.6u.png", mCurrentFrame);
      if ( !fs.open( framePath, Torque::FS::File::Write ) )
      {
         Con::errorf( "VideoEncoderPNG::pushFrame() - Failed to open output file '%s'!", framePath.c_str() );
         return false;
      }

      //Increment
      mCurrentFrame++;

      bool result = bitmap->writeBitmap("png", fs, 0);
      pushProcessedBitmap(bitmap);
 
      return result;
   }

   /// Finishes the encoding and closes the video
   bool end()
   {
      return true;
   }

   void setResolution( Point2I* resolution ) 
   {      
      mResolution = *resolution; 
   }
};

REGISTER_VIDEO_ENCODER(VideoEncoderPNG, PNG)