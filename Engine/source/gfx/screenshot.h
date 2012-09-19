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
#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_

#include "gfx/gfxDevice.h"

class GuiCanvas;
class Point2I;
class Frustum;

//**************************************************************************
/*!
   This class will eventually support various capabilities such as panoramics,
   high rez captures, and cubemap captures.
   
   Right now it just captures standard screenshots, but it does support
   captures from multisample back buffers, so antialiased captures will work.
*/
//**************************************************************************

class ScreenShot
{
   /// This is overloaded to copy the current GFX 
   /// backbuffer to a new bitmap.
	virtual GBitmap* _captureBackBuffer() { return NULL; }

   /// This is set to toggle the capture.
   bool mPending;

   /// If true write the file as a JPG else its a PNG.
   bool mWriteJPG;

   /// The full path to the screenshot file to write.
   char mFilename[256];

   /// The number of times to tile the backbuffer to 
   /// generate screenshots larger than normally possible.
   U32 mTiles;

   /// How much pixel percentage overlap between tiles
   Point2F mPixelOverlap;

   /// How much the frustum must be adjusted for overlap
   Point2F mFrustumOverlap;

   /// The current tile we're rendering.
   Point2I mCurrTile;

   /// Helper for taking simple single tile screenshots and
   /// outputing it as a single file to disk.
   void _singleCapture( GuiCanvas *canvas );
   
public:
  
   /// Constructor.
   ScreenShot();

   /// Used to start the screenshot capture.
   void setPending( const char *filename, bool writeJPG, S32 tiles, F32 overlap );

   /// Returns true if we're in the middle of screenshot capture.
   bool isPending() const { return mPending; }

   /// Prepares the view frustum for tiled screenshot rendering.
   /// @see GuiTSCtrl::setupFrustum
   void tileFrustum( Frustum& frustum );

   ///
   void tileGui( const Point2I &screenSize );

   /// Called from the canvas to capture a pending screenshot.
   /// @see GuiCanvas::onRenderEvent
   void capture( GuiCanvas *canvas );

};

extern ScreenShot *gScreenShot;

#endif  // _SCREENSHOT_H_
