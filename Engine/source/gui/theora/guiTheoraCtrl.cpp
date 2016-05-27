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

#include "gui/core/guiControl.h"
#include "gui/theora/guiTheoraCtrl.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiTheoraCtrl );

ConsoleDocClass( GuiTheoraCtrl,
   "@brief A control to playing Theora videos.\n\n"
   
   "This control can be used to play videos in the Theora video format.  The videos may include audio in Vorbis format.  The "
   "codecs for both formats are integrated with the engine and no codecs must be present on the user's machine.\n\n"
   
   "@tsexample\n"
   "%video = new GuiTheoraCtrl()\n"
   "{\n"
   "   theoraFile = \"videos/intro.ogv\";\n"
   "   playOnWake = false;\n"
   "   stopOnSleep = true;\n"
   "}\n"
   "\n"
   "Canvas.setContent( %video );\n"
   "%video.play();\n"
   "@endtsexample\n\n"
   
   "@see http://www.theora.org\n\n"
   "@ingroup GuiImages"
);

ImplementEnumType( GuiTheoraTranscoder,
   "Routine to use for converting Theora's Y'CbCr pixel format to RGB color space.\n\n"
   "@ingroup GuiImages" )
   { OggTheoraDecoder::TRANSCODER_Auto, "Auto", "Automatically detect most appropriate setting." },
   { OggTheoraDecoder::TRANSCODER_Generic, "Generic", "Slower but beneric transcoder that can convert all Y'CbCr input formats to RGB or RGBA output." },
   { OggTheoraDecoder::TRANSCODER_SSE2420RGBA, "SSE2420RGBA", "Fast SSE2-based transcoder with fixed conversion from 4:2:0 Y'CbCr to RGBA." },
EndImplementEnumType;

//-----------------------------------------------------------------------------

GuiTheoraCtrl::GuiTheoraCtrl()
{
   mFilename         = StringTable->EmptyString();
   mDone             = false;
   mStopOnSleep      = false;
   mMatchVideoSize   = true;
   mPlayOnWake       = true;
   mRenderDebugInfo  = false;
   mTranscoder       = OggTheoraDecoder::TRANSCODER_Auto;
   mLoop             = false;
   
   mBackgroundColor.set( 0, 0, 0, 255);
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::initPersistFields()
{
   addGroup( "Playback");

      addField( "theoraFile",       TypeStringFilename,  Offset( mFilename,         GuiTheoraCtrl ),
         "Theora video file to play." );
      addField( "backgroundColor",  TypeColorI,          Offset( mBackgroundColor,  GuiTheoraCtrl ),
         "Fill color when video is not playing." );
      addField( "playOnWake",       TypeBool,            Offset( mPlayOnWake,       GuiTheoraCtrl ),
         "Whether to start playing video when control is woken up." );
      addField( "loop",             TypeBool,            Offset( mLoop,            GuiTheoraCtrl ),
         "Loop playback." );
      addField( "stopOnSleep",      TypeBool,            Offset( mStopOnSleep,      GuiTheoraCtrl ),
         "Whether to stop video when control is set to sleep.\n\n"
         "If this is not set to true, the video will be paused when the control is put to sleep.  This is because there is no support "
         "for seeking in the video stream in the player backend and letting the time source used to synchronize video (either audio "
         "or a raw timer) get far ahead of frame decoding will cause possibly very long delays when the control is woken up again." );
      addField( "matchVideoSize",   TypeBool,            Offset( mMatchVideoSize,   GuiTheoraCtrl ),
         "Whether to automatically match control extents to the video size." );
      addField( "renderDebugInfo",  TypeBool,            Offset( mRenderDebugInfo,  GuiTheoraCtrl ),
         "If true, displays an overlay on top of the video with useful debugging information." );
      addField( "transcoder",       TYPEID< OggTheoraDecoder::ETranscoder >(), Offset( mTranscoder,       GuiTheoraCtrl ),
         "The routine to use for Y'CbCr to RGB conversion." );

   endGroup( "Playback" );

	Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::setFile( const String& filename )
{
	mDone = false;
   mFilename = filename;
   mTheoraTexture.setFile( filename );
   
   if( mMatchVideoSize && mTheoraTexture.isReady() )
      setExtent( Point2I( mTheoraTexture.getWidth(), mTheoraTexture.getHeight() ) );

   OggTheoraDecoder* decoder = mTheoraTexture._getTheora();
   if( decoder != NULL )
      decoder->setTranscoder( mTranscoder );
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::play()
{
   if( mFilename.isEmpty() )
      return;
   
   if( !mTheoraTexture.isPlaying() )
   {
      mDone = false;
      mTheoraTexture.play();
   }
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::pause()
{
   if( !mTheoraTexture.isPlaying() )
   {
      Con::errorf( "GuiTheoraCtrl::pause - not playing" );
      return;
   }
   
   mTheoraTexture.pause();
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::stop()
{
	mTheoraTexture.stop();
	mDone = true;
}

//-----------------------------------------------------------------------------

bool GuiTheoraCtrl::onWake()
{
	if( !Parent::onWake() )
      return false;

   if( !mTheoraTexture.isReady() )
      setFile( mFilename );
   
   if( mPlayOnWake && !mTheoraTexture.isPlaying() )
      play();

	return true;
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::onSleep()
{
	Parent::onSleep();

   if( mTheoraTexture.isPlaying() )
   {
      if( mStopOnSleep )
         stop();
      else
         pause();
   }
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   const RectI rect(offset, getBounds().extent);

	if( mTheoraTexture.isReady() )
	{
      mTheoraTexture.refresh();
      if( mTheoraTexture.isPlaying()
          || mTheoraTexture.isPaused() )
      {
         // Draw the frame.
         
         GFXDrawUtil* drawUtil = GFX->getDrawUtil();
         drawUtil->clearBitmapModulation();
         drawUtil->drawBitmapStretch( mTheoraTexture.getTexture(), rect );
         
         // Draw frame info, if requested.
         
         if( mRenderDebugInfo )
         {
            String info = String::ToString( "Frame Number: %i | Frame Time: %.2fs | Playback Time: %.2fs | Dropped: %i",
               mTheoraTexture.getFrameNumber(),
               mTheoraTexture.getFrameTime(),
               F32( mTheoraTexture.getPosition() ) / 1000.f,
               mTheoraTexture.getNumDroppedFrames() );
            
            drawUtil->setBitmapModulation( mProfile->mFontColors[ 0 ] );
            drawUtil->drawText( mProfile->mFont, localToGlobalCoord( Point2I( 0, 0 ) ), info, mProfile->mFontColors );
         }
      }
      else
      {
         if(mLoop)
         {
            play();
         } else {
            mDone = true;
         }
      }
	}
	else
 		GFX->getDrawUtil()->drawRectFill(rect, mBackgroundColor); // black rect

	renderChildControls(offset, updateRect);
}

//-----------------------------------------------------------------------------

void GuiTheoraCtrl::inspectPostApply()
{
   if( !mTheoraTexture.getFilename().equal( mFilename, String::NoCase ) )
   {
      stop();
      setFile( mFilename );
      
      if( mPlayOnWake && !mTheoraTexture.isPlaying() )
         play();
   }
   
   if( mMatchVideoSize && mTheoraTexture.isReady() )
      setExtent( Point2I( mTheoraTexture.getWidth(), mTheoraTexture.getHeight() ) );

   OggTheoraDecoder* decoder = mTheoraTexture._getTheora();
   if( decoder != NULL )
      decoder->setTranscoder( mTranscoder );
      
   Parent::inspectPostApply();
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, setFile, void, ( const char* filename ),,
   "Set the video file to play.  If a video is already playing, playback is stopped and "
   "the new video file is loaded.\n\n"
   "@param filename The video file to load." )
{
	object->setFile( filename );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, play, void, (),,
   "Start playing the video.  If the video is already playing, the call is ignored." )
{
   object->play();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, pause, void, (),,
   "Pause playback of the video.  If the video is not currently playing, the call is ignored.\n\n"
   "While stopped, the control displays the last frame." )
{
   object->pause();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, stop, void, (),,
   "Stop playback of the video.  The next call to play() will then start playback from the beginning of the video.\n\n"
   "While stopped, the control renders empty with just the background color." )
{
	object->stop();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, getCurrentTime, F32, (),,
   "Get the current playback time.\n\n"
   "@return The elapsed playback time in seconds." )
{
   return object->getCurrentTime();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiTheoraCtrl, isPlaybackDone, bool, (),,
   "Test whether the video has finished playing.\n\n"
   "@return True if the video has finished playing, false otherwise." )
{
   return object->isPlaybackDone();
}

#endif // TORQUE_OGGTHEORA
