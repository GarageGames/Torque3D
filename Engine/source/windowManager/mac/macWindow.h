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

#ifndef _TORQUE_MACWINDOW_H_
#define _TORQUE_MACWINDOW_H_

#include "windowManager/platformWindow.h"
#include "windowManager/mac/macWindowManager.h"
#include "windowManager/mac/macCursorController.h"

#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif

#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif

class MacWindow : public PlatformWindow
{
public:
   virtual ~MacWindow();

   virtual GFXDevice *getGFXDevice() { return mDevice; }
   virtual GFXWindowTarget *getGFXTarget() { return mTarget; }
   virtual void setVideoMode(const GFXVideoMode &mode);
   virtual const GFXVideoMode &getVideoMode() { return mCurrentMode; }
   
   virtual WindowId getWindowId() { return mWindowId; }
   
   void setDisplay(CGDirectDisplayID display);
   CGDirectDisplayID getDisplay() { return mDisplay; }
   CGRect getMainDisplayBounds() { return mMainDisplayBounds; }
   CGRect getDisplayBounds() { return mDisplayBounds; }

   virtual bool clearFullscreen() 
   { 
      // TODO: properly drop out of full screen
      return true;
   }
   virtual bool isFullscreen() { return mFullscreen; }

   virtual PlatformWindow * getNextWindow() const;
   
   virtual void setMouseLocked( bool enable ) 
   { 
      mShouldMouseLock = enable; 
      if(isFocused()) 
         _doMouseLockNow(); 
   }
   virtual bool isMouseLocked() const { return mMouseLocked; }
   virtual bool shouldLockMouse() const { return mShouldMouseLock; }

   virtual bool setSize(const Point2I &newSize);

   virtual void setClientExtent( const Point2I newExtent );
   virtual const Point2I getClientExtent();
   
   virtual void setBounds( const RectI &newBounds );
   virtual const RectI getBounds() const;

   virtual void setPosition( const Point2I newPosition );
   virtual const Point2I getPosition();
   
   virtual void centerWindow();
   
   virtual Point2I clientToScreen( const Point2I& pos );
   virtual Point2I screenToClient( const Point2I& pos );

   virtual bool setCaption(const char *windowText);
   virtual const char *getCaption() { return mTitle; }

   virtual bool setType( S32 windowType ) { return true; }

   virtual void minimize();
   virtual void maximize();
   virtual void restore();
   virtual bool isMinimized();
   virtual bool isMaximized();
   virtual void show();
   virtual void close();
   virtual void hide();
   virtual bool isOpen();
   virtual bool isVisible();

   virtual bool isFocused();
   virtual void setFocus();
   virtual void clearFocus();
   
   virtual void* getPlatformDrawable() const;
   
   // TODO: These should be private, but GGMacView (an Obj-C class) needs access to these and we can't friend Obj-C classes
   bool _skipNextMouseEvent() { return mSkipMouseEvents != 0; }
   void _skipAnotherMouseEvent() { mSkipMouseEvents++; }
   void _skippedMouseEvent() { mSkipMouseEvents--; }
   
   /// Does the work of actually locking or unlocking the mouse, based on the
   /// value of shouldLockMouse().
   ///
   /// Disassociates the cursor movement from the mouse input and hides the mouse
   /// when locking. Re-associates cursor movement with mouse input and shows the
   /// mouse when unlocking.
   /// 
   /// Returns true if we locked or unlocked the mouse. Returns false if the mouse
   /// was already in the correct state.
   void _doMouseLockNow();
   
   // Helper methods for doMouseLockNow
   void _associateMouse();
   void _dissociateMouse();
   void _centerMouse();

   // For GGMacView
   void _disassociateCocoaWindow();
   
   // Safari support methods
   static void setSafariWindow(NSWindow *window, S32 x = 0, S32 y = 0, S32 width = 0, S32 height = 0);
   static void hideBrowserWindow(bool hide);
   
protected:
   virtual void _setFullscreen(bool fullScreen);
   
private:
   friend class MacWindowManager;
   friend class MacCursorController;
   
   struct SafariWindowInfo
   {
       NSWindow*    safariWindow; /* The Safari Browser Window              */
       S32          x;            /* Position of top left corner relative   */
       S32          y;            /* to a safari page.                      */
       U32          width;        /* Maximum window size                    */
       U32          height;
   };
   
   MacWindow(U32 windowId, const char* windowText, Point2I clientExtent);
   
   void _initCocoaWindow(const char* windowText, Point2I clientExtent);
   void setWindowId(U32 newid) { mWindowId = newid;}
   void signalGainFocus();

   static SafariWindowInfo* sSafariWindowInfo;
   static MacWindow* sInstance;
   
   NSWindow* mCocoaWindow;
   GFXDevice *mDevice;
   GFXWindowTargetRef mTarget;
   GFXVideoMode mCurrentMode;
   
   MacWindow *mNextWindow;

   bool mMouseLocked;
   bool mShouldMouseLock;
      
   const char* mTitle;
   bool mMouseCaptured;
   
   MacWindowManager* mOwningWindowManager;
   U32 mSkipMouseEvents;
   
   bool mFullscreen;
   bool mShouldFullscreen;
   NSDictionary* mDefaultDisplayMode;
   
   void _onAppEvent(WindowId,S32);
   
   CGDirectDisplayID mDisplay;
   CGRect mDisplayBounds;
   CGRect mMainDisplayBounds;
};

#endif
