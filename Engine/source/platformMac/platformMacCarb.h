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

#ifndef _PLATFORMMACCARB_H_
#define _PLATFORMMACCARB_H_

/// NOTE: Placing system headers before Torque's platform.h will work around the Torque-Redefines-New problems.
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#include "platform/platform.h"
#include "math/mMath.h"

#include "gfx/gl/ggl/ggl.h"
#define __gl_h_
#include <AGL/agl.h>

class MacCarbPlatState
{
public:
   GDHandle          hDisplay;
   CGDirectDisplayID cgDisplay;
   
   bool              captureDisplay;
   bool              fadeWindows;

   WindowPtr         appWindow;   
   char              appWindowTitle[256];
   WindowGroupRef    torqueWindowGroup;

   bool              quit;
   
   AGLContext        ctx;
   bool              ctxNeedsUpdate;

   S32               desktopBitsPixel;
   S32               desktopWidth;
   S32               desktopHeight;
   U32               currentTime;
   bool              isFullScreen;
   
   U32               osVersion;
   
   TSMDocumentID     tsmDoc;
   bool              tsmActive;
   
   U32               firstThreadId;
   
   void*             alertSemaphore;
   S32               alertHit;
   DialogRef         alertDlg;
   EventQueueRef     mainEventQueue;
   
   MRandomLCG        platRandom;
   
   bool              mouseLocked;
   bool              backgrounded;
   
   U32               sleepTicks;

   Point2I           windowSize;
   
   U32               appReturn;
   
   U32               argc;
   char**            argv;
   
   U32               lastTimeTick;
   
   MacCarbPlatState();
};

/// Global singleton that encapsulates a lot of mac platform state & globals.
extern MacCarbPlatState platState;

/// @name Misc Mac Plat Functions
/// Functions that are used by multiple files in the mac plat, but too trivial
/// to require their own header file.
/// @{
/// Fills gGLState with info about this gl renderer's capabilities.
void getGLCapabilities(void);

/// Creates a new mac window, of a particular size, centered on the screen.
/// If a fullScreen window is requested, then the window is created without
/// decoration, in front of all other normal windows AND BEHIND asian text input methods.
/// This path to a fullScreen window allows asian text input methods to work
/// in full screen mode, because it avoids capturing the display.
WindowPtr MacCarbCreateOpenGLWindow( GDHandle hDevice, U32 width, U32 height, bool fullScreen );

/// Asnychronously fade a window into existence, and set menu bar visibility.
/// The fading can be turned off via the preference $pref::mac::fadeWindows.
/// It also sends itself to the main thread if it is called on any other thread.
void MacCarbFadeInWindow( WindowPtr window );

/// Asnychronously fade a window out of existence. The window will be destroyed
/// when the fade is complete.
/// The fading can be turned off via the preference $pref::mac::fadeWindows.
/// It also sends itself to the main thread if it is called on any other thread.
void MacCarbFadeAndReleaseWindow( WindowPtr window );

/// Translates a Mac keycode to a Torque keycode
U8 TranslateOSKeyCode(U8 vcode);
/// @}

/// @name Misc Mac Plat constants
/// @{

/// earlier versions of OSX don't have these convinience macros, so manually stick them here.
#ifndef IntToFixed
#define IntToFixed(a) 	((Fixed)(a) <<16)
#define FixedToInt(a)	((short)(((Fixed)(a) + fixed1/2) >> 16))
#endif

/// window level constants
const U32 kTAlertWindowLevel        = CGShieldingWindowLevel() - 1;
const U32 kTUtilityWindowLevel      = CGShieldingWindowLevel() - 2;
const U32 kTFullscreenWindowLevel   = CGShieldingWindowLevel() - 3;

/// mouse wheel sensitivity factor
const S32 kTMouseWheelMagnificationFactor = 25;

/// Torque Menu Command ID
const U32 kHICommandTorque = 'TORQ';

/// @}

//-----------------------------------------------------------------------------
// Platform Menu Data
//-----------------------------------------------------------------------------
class PlatformPopupMenuData
   {
   public:
      // We assign each new menu item an arbitrary integer tag.
      static S32 getTag()
      {
         static S32 lastTag = 'TORQ';
         return ++lastTag;
      }
      
      MenuRef mMenu;
      S32 tag;
      PlatformPopupMenuData()
      {
         mMenu = NULL;
         tag = getTag();
      }
      
      ~PlatformPopupMenuData()
      {
         if(mMenu)
            CFRelease(mMenu);
         mMenu = NULL;
   }
};

#endif //_PLATFORMMACCARB_H_

