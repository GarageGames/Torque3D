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

#ifndef _GUITHEORACTRL_H_
#define _GUITHEORACTRL_H_

#ifdef TORQUE_OGGTHEORA

#ifndef _THEORATEXTURE_H_
   #include "gfx/video/theoraTexture.h"
#endif
#ifndef _OGGTHEORADECODER_H_
   #include "core/ogg/oggTheoraDecoder.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif


/// Control to play back a Theora video file.
class GuiTheoraCtrl : public GuiControl
{
   public:
   
      typedef GuiControl Parent;
      
   protected:
   
      /// The Theora file we should play.
      String mFilename;
      
      /// Theora video player backend.
      TheoraTexture mTheoraTexture;
      
      /// If true, the control's extents will be matched to the video size.
      bool mMatchVideoSize;
      
      /// If true, playback will start automatically when the control receives its
      /// onWake().
      bool mPlayOnWake;

      bool mLoop;
      
      /// Which transcoder to use on the Theora decoder.  This is mostly
      /// meant as a development aid.
      OggTheoraDecoder::ETranscoder mTranscoder;

      /// If true, stop video playback when the control goes to sleep.  Otherwise,
      /// the video will be paused.
      ///
      /// @note We do not currently support to keep video running in the background
      ///   as the Theora decoder does not yet support skipping through bulks of
      ///   outdated data.  This means that when the Theora texture gets its next
      ///   refresh, the decoder will frantically try to wade through a huge amount
      ///   of outdated ogg_packets which even though the actual decoding does not
      ///   take place takes a lot of time.
      bool mStopOnSleep;

      /// Are we done with playback?
      bool mDone;
      
      /// If true, renders some text information into the frame.
      bool mRenderDebugInfo;
      
      /// Our background color.
      ColorI mBackgroundColor;

   public:
   
      GuiTheoraCtrl();

      /// Load the given Theora video file.  Does not start playback.
      void setFile( const String& filename );
      
      /// Start video playback.
      void play();
      
      /// Pause video playback.
      void pause();
      
      /// Stop video playback.
      void stop();
      
      /// Return true if the video has finished playing.
      bool isPlaybackDone() const { return mDone; }

      /// Return the current playback position.
      F32 getCurrentTime()
      {
         return F32( mTheoraTexture.getPosition() ) / 1000.f;
      }

      // GuiControl.
      virtual bool onWake();
      virtual void onSleep();
      virtual void onRender( Point2I offset, const RectI &updateRect );
      virtual void inspectPostApply();

      static void initPersistFields();

      DECLARE_CONOBJECT( GuiTheoraCtrl );
      DECLARE_CATEGORY( "Gui Images" );
      DECLARE_DESCRIPTION( "A control for playing Theora videos." );
};

typedef OggTheoraDecoder::ETranscoder GuiTheoraTranscoder;
DefineEnumType( GuiTheoraTranscoder );

#endif // TORQUE_OGGTHEORA
#endif // !_GUITHEORACTRL_H_
