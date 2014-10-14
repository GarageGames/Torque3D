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

#ifndef _PLATFORM_PLATFORMWINDOWMGR_H_
#define _PLATFORM_PLATFORMWINDOWMGR_H_

#include "math/mRect.h"
#include "core/util/journal/journaledSignal.h"
#include "windowManager/platformWindow.h"


// Global macro
#define WindowManager PlatformWindowManager::get()

/// Abstract representation of a manager for native OS windows.
///
/// The PlatformWindowManager interface provides a variety of methods for querying 
/// the current desktop configuration, as well as allocating and retrieving
/// existing windows. It may also manage application-level event handling.
class PlatformWindowManager
{
   // Generator for window IDs.
   S32 mIdSource;
   
protected:
   /// Get the next available window Id
   inline S32 getNextId() { return mIdSource++; }
public:

   /// Get Global Singleton
   static PlatformWindowManager *get();
   
   PlatformWindowManager() : mIdSource(0) {};

   virtual ~PlatformWindowManager() 
   {
   }

   static void processCmdLineArgs(const S32 argc, const char **argv);

   /// Return the extents in window coordinates of the primary desktop
   /// area. On a single monitor system this is just the display extents.
   /// On a multimon system this is the primary monitor (which Torque should
   /// launch on).
   virtual RectI getPrimaryDesktopArea() = 0;

   /// Retrieve the currently set desktop bit depth
   /// @return The current desktop bit depth, or -1 if an error occurred
   virtual S32 getDesktopBitDepth() = 0;

   /// Retrieve the currently set desktop resolution
   /// @return The current desktop bit depth, or Point2I(-1,-1) if an error occurred
   virtual Point2I getDesktopResolution() = 0;

   // Build out the monitor list.
   virtual void buildMonitorsList() {}

   // Find the first monitor index that matches the given name.  The actual match
   // algorithm depends on the implementation.  Provides a default value of -1 to
   // indicate no match.
   virtual S32 findFirstMatchingMonitor(const char* name) { return -1; }

   // Retrieve the number of monitors.  Provides a default count of 0 for systems that
   // don't provide information on connected monitors.
   virtual U32 getMonitorCount() { return 0; }

   // Get the name of the requested monitor.  Provides a default of "" for platorms
   // that do not provide information on connected monitors.
   virtual const char* getMonitorName(U32 index) { return ""; }

   // Get the requested monitor's rectangular region.
   virtual RectI getMonitorRect(U32 index) { return RectI(0, 0, 0, 0); }

   /// Populate a vector with all monitors and their extents in window space.
   virtual void getMonitorRegions(Vector<RectI> &regions) = 0;

   /// Create a new window, appropriate for the specified device and mode.
   ///
   /// @return Pointer to the new window.
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode) = 0;

   /// Populate a list with references to all the windows created from this manager.
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows) = 0;

   /// Get the window that currently has the input focus or NULL.
   virtual PlatformWindow* getFocusedWindow() = 0;

   /// Get a window from a device ID.
   ///
   /// @return The window associated with the specified ID, or NULL if no
   ///         match was found.
   virtual PlatformWindow *getWindowById(WindowId id)=0;

   /// Get the first window in the window list
   ///
   /// @return The first window in the list, or NULL if no windows found
   virtual PlatformWindow *getFirstWindow()=0;


   /// Set the parent window
   ///
   /// This can be used to render in a child window.
   virtual void setParentWindow(void* newParent) = 0;

   /// Get the parent window
   virtual void* getParentWindow() = 0;


   /// This method cues the appearance of that window ("lowering the curtain").
   virtual void lowerCurtain()=0;

   /// @see lowerCurtain
   ///
   /// This method removes the curtain window.
   virtual void raiseCurtain()=0;

   /// This method indicates to created windows to show as normal.
   virtual void setDisplayWindow(bool set){}

private:
   /// Process command line arguments from StandardMainLoop. This is done to
   /// allow web plugin functionality, where we are passed platform-specific
   /// information allowing us to set ourselves up in the web browser,
   /// to occur in a platform-neutral way.
   virtual void _processCmdLineArgs(const S32 argc, const char **argv)=0;
};

/// Global function to allocate a new platform window manager.
///
/// This returns an instance of the appropriate window manager for the current OS.
///
/// Depending on situation (for instance, if we are a web plugin) we may
/// need to get the window manager from somewhere else.
PlatformWindowManager *CreatePlatformWindowManager();

#endif