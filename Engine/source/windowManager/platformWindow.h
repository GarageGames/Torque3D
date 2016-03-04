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

#ifndef _WINDOWMANAGER_PLATFORMWINDOW_H_
#define _WINDOWMANAGER_PLATFORMWINDOW_H_

#include "math/mRect.h"
#include "core/util/journal/journaledSignal.h"
#include "core/util/safeDelete.h"
#include "windowManager/platformCursorController.h"
#include "windowManager/windowInputGenerator.h"
#ifndef _SIGNAL_H_ //Volumetric Fog
#include "core/util/tSignal.h"
#endif

//forward decl's
class PlatformWindowManager;
class GFXDevice;
struct GFXVideoMode;
class GFXWindowTarget;
class IProcessInput;
typedef Signal<void(PlatformWindow *PlatformWindow, bool resize)> ScreenResChangeSignal;
/// Abstract representation of a native OS window.
///
/// Every windowing system has its own representations and conventions as
/// regards the windows on-screen. In order to provide Torque with means for
/// interfacing with multiple windows, tracking their state, etc. we provide
/// this interface.
///
/// This interface also allows the app to access the render target for the 
/// window it represents, as well as control mode switches, get mode info,
/// and so on.
///
/// @see PlatformWindowManager
class PlatformWindow
{
   friend class PlatformWindowManager;
protected:

   /// Are we enabling IME or other keyboard input translation services,
   /// or concerned about raw input?
   bool mEnableKeyboardTranslation;

   /// When Torque GuiText input controls have focus they need to
   /// disable native OS keyboard accelerator translation.
   bool mEnableAccelerators;

   /// Minimum allowed size for this window. When possible, we will communicate
   /// this to the OS.
   Point2I mMinimumSize;

	/// When the resize is locked, this will be used as both minimum and maximum window size
	Point2I mLockedSize;

	/// When this is true, resizing is locked
	bool mResizeLocked;

   /// Is Idle?
   bool mIsBackground;

   /// Cursor Controller for this Window
   PlatformCursorController *mCursorController;
   
   /// An opaque ID used to resolve references to this Window
   WindowId mWindowId;

   /// Window Mouse/Key input Controller for this Window
   WindowInputGenerator *mWindowInputGenerator;

   /// Suppress device resets
   bool mSuppressReset;

   /// Offscreen Render
   bool mOffscreenRender;

   /// This is set as part of the canvas being shown, and flags that the windows should render as normal from now on.
   // Basically a flag that lets the window manager know that we've handled the splash screen, and to operate as normal.
   bool mDisplayWindow;

   /// Protected constructor so that the win
   PlatformWindow()
   {
      mIsBackground = false; // This could be toggled to true to prefer performance.
      mMinimumSize.set(0,0);
      mLockedSize.set(0,0);
      mResizeLocked = false;
      mEnableKeyboardTranslation = false;
      mEnableAccelerators = true;
      mCursorController = NULL;
      mSuppressReset = false;
      mOffscreenRender = false;
      mDisplayWindow = false;

      // This controller maps window input (Mouse/Keyboard) to a generic input consumer
      mWindowInputGenerator = new WindowInputGenerator( this );
   }
   static ScreenResChangeSignal smScreenResChangeSignal;
public:

   /// To get rid of a window, just delete it. Make sure the GFXDevice is
   /// done with it first!
   virtual ~PlatformWindow() 
   {
      SAFE_DELETE( mCursorController );
      SAFE_DELETE( mWindowInputGenerator );
   }

   /// Get the WindowController associated with this window
   virtual void setInputController( IProcessInput *controller ) { if( mWindowInputGenerator ) mWindowInputGenerator->setInputController( controller ); };

   WindowInputGenerator* getInputGenerator() const { return mWindowInputGenerator; }

   /// Get the ID that uniquely identifies this window in the context of its
   /// window manager.
   virtual WindowId getWindowId() { return 0; };

   enum WindowSystem
   {
      WindowSystem_Unknown = 0,
      WindowSystem_Windows,
      WindowSystem_X11,
   };

   virtual void* getSystemWindow(const WindowSystem system) { return NULL; }

   /// Set the flag that determines whether to suppress a GFXDevice reset
   inline void setSuppressReset(bool suppress) { mSuppressReset = suppress; };

   /// @name GFX State Management
   ///
   /// @{

   /// Return a pointer to the GFX device this window is bound to. A GFX
   /// device may use many windows, but a window can only be used by a
   /// single GFX device.
   virtual GFXDevice *getGFXDevice()=0;

   /// Return a pointer to this window's render target.
   ///
   /// By setting window targets from different windows, we can effect
   /// rendering to multiple windows from a single device.
   virtual GFXWindowTarget *getGFXTarget()=0;

   /// Set the video mode for this window.
   virtual void setVideoMode(const GFXVideoMode &mode);

   /// Get our current video mode - if the window has been resized, it will
   /// reflect this.
   virtual const GFXVideoMode &getVideoMode()=0;

   /// If we're fullscreen, this function returns us to desktop mode.
   ///
   /// This will be either the last mode that we had that was not
   /// fullscreen, or the equivalent mode, windowed.
   virtual bool clearFullscreen()=0;

   /// @return true if this window is fullscreen, false otherwise.
   virtual bool isFullscreen()=0;

   /// Acquire the entire screen
   void setFullscreen(const bool fullscreen);

   /// Set Idle State (Background)
   /// 
   /// This is called to put a window into idle state, which causes it's 
   /// rendering priority to be toned down to prefer performance
   virtual void setBackground( bool val ) { mIsBackground = val; };

   /// Get Idle State (Background)
   ///
   /// This is called to poll the window as to it's idle state.  
   virtual bool getBackground() { return mIsBackground; };

   /// Set whether this window is intended for offscreen rendering
   /// An offscreen window will never be shown or lose focus
   virtual void setOffscreenRender(bool val ) { mOffscreenRender = val; };

   /// Set whether this window is intended for offscreen rendering
   ///
   /// This is called to poll the window as to it's idle state.  
   virtual bool getOffscreenRender() { return mOffscreenRender; };

   /// Set whether this window is should display as normal
   virtual void setDisplayWindow(bool val ) { mDisplayWindow = val; };

   /// Set Focused State (Foreground)
   ///
   /// Claim OS input focus for this window
   virtual void setFocus() { }
   /// @}

   /// @name Caption
   ///
   /// @{

   /// Set the window's caption.
   virtual bool setCaption(const char *cap)=0;

   /// Get the window's caption.
   virtual const char *getCaption()=0;

   /// @}

   /// @name Visibility
   ///
   /// Control how the window is displayed
   /// @{

   /// Minimize the window on screen
   virtual void minimize()=0;

   /// Maximize the window on screen
   virtual void maximize()=0;

   /// Hide the window on screen
   virtual void hide()=0;

   /// Show the window on screen
   virtual void show()=0;

   /// Destroy the window on screen
   virtual void close()=0;

   /// Restore the window from a Maximized or Minimized state
   virtual void restore()=0;

   /// @}

      /// @name Window Bounds
   ///
   /// @{

   /// The Client Rectangle or "Render Area" of a window is the area that 
   /// is occupied by a given client that is rendering to that window.
   /// This does not include the area occupied by a title-bar, menu, 
   /// borders or other non-client elements.
   /// @{

   /// Set the Client Area Extent (Resolution) of this window
   virtual void setClientExtent( const Point2I newExtent ) = 0;

   /// Get the Client Area Extent (Resolution) of this window
   virtual const Point2I getClientExtent() = 0;

   /// @}
   /// The bounds of a Window are defined as the entire area occupied by 
   /// that Window.  This includes the area needed for a title-bar, menu,
   /// borders, and other non-client elements.
   /// 
   /// @{

   /// Resize the window to have some new bounds.
   virtual void setBounds( const RectI &newBounds ) = 0;

   /// Get the position and size (fullscreen windows are always at (0,0)).
   virtual const RectI getBounds() const = 0;

   /// @}
   /// The Position of a window is always in relation to the very upper left 
   /// of the window.  This means that saying setPosition at 0,0 will put the 
   /// position of the window title-bar (if one exists) at 0,0 and the Client
   /// area will be offset from that point by the space needed for the Non-Client
   /// area.
   /// @{

   /// Set the position of this window
   virtual void setPosition( const Point2I newPosition ) = 0;

   /// Get the position of this window
   virtual const Point2I getPosition() = 0;

   virtual void centerWindow() {};

   /// Resize the window to have a new size (but be in the same position).
   virtual bool setSize(const Point2I &newSize)=0;

   /// @}
   
   /// @name Coordinate Space Conversion
   /// @{

   /// Convert the coordinate given in this window space to screen coordinates.
   virtual Point2I clientToScreen( const Point2I& point ) = 0;
   
   /// Convert the given screen coordinates to coordinates in this window space.
   virtual Point2I screenToClient( const Point2I& point ) = 0;
   
   /// @}

   /// @name Windowed state
   ///
   /// This is only really meaningful if the window is not fullscreen.
   ///
   /// @{

   /// Returns true if the window is instantiated in the OS.
   virtual bool isOpen() = 0;

   /// Returns true if the window is visible.
   virtual bool isVisible() = 0;

   /// Returns true if the window has input focus
   virtual bool isFocused() = 0;

   /// Returns true if the window is minimized
   virtual bool isMinimized() = 0;

   /// Returns true if the window is maximized
   virtual bool isMaximized() = 0;
   
   /// @name Keyboard Translation
   ///
   /// When keyboard translation is on, keypress events that correspond to character input
   /// should be send as character input events rather than as raw key events *except* if
   /// shouldNotTranslate() returns true for a specific keypress event.  This enables the
   /// platform layer to perform platform-specific character input mapping.
   ///
   /// @{

   /// Set if relevant keypress events should be translated into character input events.
   virtual void setKeyboardTranslation(const bool enabled)
   {
      mEnableKeyboardTranslation = enabled;
   }

   /// Returns true if keyboard translation is enabled.
   virtual bool getKeyboardTranslation() const
   {
      return mEnableKeyboardTranslation;
   }
   
   /// Returns true if the given keypress event should not be translated.
   virtual bool shouldNotTranslate( U32 modifiers, U32 keyCode ) const;
   
   /// @}

   /// Used to disable native OS keyboard accelerators.
   virtual void setAcceleratorsEnabled(const bool enabled)
   {
      mEnableAccelerators = enabled;
   }

   /// Returns true if native OS keyboard accelerators are enabled.
   virtual bool getAcceleratorsEnabled() const
   {
      return mEnableAccelerators;
   }

   /// Sets a minimum window size. We'll work with the OS to prevent user
   /// from sizing the window to less than this. Setting to (0,0) means
   /// user has complete freedom of resize.
   virtual void setMinimumWindowSize(Point2I minSize)
   {
      mMinimumSize = minSize;
   }

   /// Returns the current minimum window size for this window.
   virtual Point2I getMinimumWindowSize()
   {
      return mMinimumSize;
   }

	/// Locks/unlocks window resizing
	virtual void lockSize(bool locked)
	{
		mResizeLocked = locked;
		if (mResizeLocked)
			mLockedSize = getBounds().extent;
	}

	/// Returns true if the window size is locked
	virtual bool isSizeLocked()
	{
		return mResizeLocked;
	}

	/// Returns the locked window size
	virtual Point2I getLockedSize()
	{
		return mLockedSize;
	}
   /// @}


   /// @name Window Cursor
   ///
   /// Accessors to control a windows cursor shape and visibility
   ///
   /// @{
   /// Get the CursorController that this window owns.
   virtual PlatformCursorController *getCursorController() { return mCursorController; };

   /// Set the cursor position based on logical coordinates from the upper-right corner
   ///
   /// @param x The X position of the cursor
   /// @param y The Y position of the cursor
   virtual void setCursorPosition(S32 x, S32 y)
   {
      if( mCursorController != NULL )
         mCursorController->setCursorPosition(x,y);
   }

   /// Get the cursor position based on logical coordinates from the upper-right corner
   ///
   /// @param point A reference to a Point2I to store the coordinates
   virtual void getCursorPosition( Point2I &point )
   {
      if( mCursorController != NULL )
         mCursorController->getCursorPosition(point);
   }
   
   /// Set the cursor visibility on this window
   /// 
   /// @param visible Whether the cursor should be visible or not
   virtual void setCursorVisible(bool visible)
   {
      if( mCursorController != NULL )
         mCursorController->setCursorVisible(visible);
   }

   /// Get the cursor visibility on this window
   /// 
   /// @return true if the cursor is visible or false if it's hidden
   virtual bool isCursorVisible()
   {
      if( mCursorController != NULL )
         return mCursorController->isCursorVisible();
      return false;
   }

   /// Lock the mouse to this window.
   ///
   /// When this is set, the mouse will always be returned to the center
   /// of the client area after every mouse event. The mouse will also be
   /// hidden while it is locked.
   ///
   /// The mouse cannot be moved out of the bounds of the window, but the
   /// window may lose focus (for instance by an alt-tab or other event).
   /// While the window lacks focus, no mouse events will be reported.
   virtual void setMouseLocked( bool enable )=0;

   /// Is the mouse locked ?
   virtual bool isMouseLocked() const = 0;

   /// Should the mouse be locked at the next opportunity ?
   ///
   /// This flag is set to the current state of the mouse lock
   /// on a window, to specify the preferred lock status of the
   /// mouse in a platform window.
   /// 
   /// This is important for situations where a call is made 
   /// to setMouseLocked, and the window is not in a state that
   /// it can be cleanly locked. Take for example if it was called
   /// while the window is in the background, then it is not appropriate
   /// to lock the window, but rather the window should query this
   /// state at it's next opportunity and lock the mouse if requested.
   virtual bool shouldLockMouse() const = 0;

   /// @}

   virtual PlatformWindow * getNextWindow() const = 0;

   /// @name Event Handlers
   ///
   /// Various events that this window receives. These are all subclasses of
   /// JournaledSignal, so you can subscribe to them and receive notifications
   /// per the documentation for that class.
   ///
   /// @{

   ///
   AppEvent          appEvent;
   MouseEvent        mouseEvent;
   MouseWheelEvent   wheelEvent;
   ButtonEvent       buttonEvent;
   LinearEvent       linearEvent;
   KeyEvent          keyEvent;
   CharEvent         charEvent;
   DisplayEvent      displayEvent;
   ResizeEvent       resizeEvent;
   IdleEvent         idleEvent;

   /// @}
   static ScreenResChangeSignal& getScreenResChangeSignal() { return smScreenResChangeSignal; }
   
   /// Get the platform specific object needed to create or attach an accelerated
   /// graohics drawing context on or to the window
   /// Win32 D3D and OpenGL typically needs an HWND
   /// Mac Cocoa OpenGL typically needs an NSOpenGLView
   /// Mac Carbon OpenGL typically needs a WindowRef
   ///
   virtual void* getPlatformDrawable() const = 0;
protected:
   virtual void _setFullscreen(const bool fullScreen) {};
   virtual void _setVideoMode(const GFXVideoMode &mode) {};
};

#endif
