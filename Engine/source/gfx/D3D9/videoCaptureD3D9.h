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
#include "gfx/video/videoCapture.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif


class VideoFrameGrabberD3D9 : public VideoFrameGrabber
{
protected:   
   enum CaptureStage {
      eReadyToCapture,         
      eInVideoMemory,       
      eInSystemMemory,
      eNumStages
   };

   // Contains all elements involved in single frame capture and
   // is used to spread the multiple "stages" needed to capture a bitmap
   // over various frames to keep GPU resources from locking the CPU.
   struct CaptureResource {
      GFXTexHandle vidMemTex; //Video memory texture
      GFXTexHandle sysMemTex; //System memory texture
      CaptureStage stage;     //This resource's capture stage

      CaptureResource() : stage(eReadyToCapture) {};
      ~CaptureResource()
      {
         vidMemTex.free();         
         sysMemTex.free();
      }
   };

   // Capture resource array. One item for each capture pipeline stage
   CaptureResource mCapture[eNumStages];

   // Current capture index
   S32 mCurrentCapture;
   
   // Copies a capture's video memory content to system memory
   void copyToSystemMemory(CaptureResource &capture);

   // Copies a capture's syste memory content to a new bitmap
   void copyToBitmap(CaptureResource &capture);

   bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);
      
   //------------------------------------------------
   // Overloaded from VideoFrameGrabber
   //------------------------------------------------
   void captureBackBuffer();
   void makeBitmap();
   void releaseTextures();

public:
   VideoFrameGrabberD3D9();
   ~VideoFrameGrabberD3D9();
};