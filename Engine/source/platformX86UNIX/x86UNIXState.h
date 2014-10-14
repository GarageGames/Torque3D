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

#include "math/mPoint2.h"
#include "platformX86UNIX/platformX86UNIX.h"
//#include "platformX86UNIX/platformGL.h"
#include "core/strings/stringFunctions.h"

#ifndef TORQUE_DEDICATED
#include <X11/Xlib.h> // for Display, Window and other X mojo
#else
#define Display int
#define Window int
#define Screen int
#endif

#include <libgen.h> // for basename

typedef void (*LockFunc_t)(void);

class DisplayPtrManager;

class x86UNIXPlatformState
{
      friend class DisplayPtrManager;

   private:
      Point2I              mDesktopSize;
      Point2I              mWindowSize;
      S32                  mDesktopBpp;
      Display              *mDisplay;
      Window               mCurrentWindow;
      Screen               *mScreenPointer;
      int                  mScreenNumber;
      char                 mWindowName[40];
      char                 mExePathName[4096];
      char                 mExeName[40];
      bool                 mWindowCreated;
      bool                 mWindowActive;
      bool                 mWindowLocked;
      bool                 mXWindowsRunning;
      bool                 mDedicated;
      bool                 mCDAudioEnabled;
      bool                 mDSleep;
      bool                 mUseRedirect;

      // Access to the display* needs to be controlled because the SDL event
      // loop runs in a separate thread.  If you need the display pointer,
      // use the DisplayPtrManager class.  See the clipboard functions in 
      // x86unixinput.cc for an example.
      Display *getDisplayPointer() { return mDisplay; }

   public:
      U32      currentTime;

      void setDisplayPointer( Display *displayPointer ) 
          { mDisplay = displayPointer; }

      void setScreenNumber( int newNumber ) { mScreenNumber = newNumber; }
      int getScreenNumber() { return mScreenNumber; }
      
      void setScreenPointer( Screen *newScreenPointer ) 
          { mScreenPointer = newScreenPointer; }
      Screen * getScreenPointer() { return mScreenPointer; }

      // for compatibility, convert 24 bpp to 32
      void setDesktopBpp( S32 bpp ) 
      { 
         if (bpp == 24) 
            mDesktopBpp = 32;
         else 
            mDesktopBpp = bpp; 
      }     
      S32 getDesktopBpp() { return mDesktopBpp; }

      void setDesktopSize( S32 horizontal, S32 vertical )
          { mDesktopSize.set( horizontal, vertical ); }
      Point2I getDesktopSize() { return mDesktopSize; }

      void setWindow( Window newWindow ) { mCurrentWindow = newWindow; }
      Window getWindow() { return mCurrentWindow; }

      void setWindowSize (S32 horizontal, S32 vertical ) 
          { mWindowSize.set ( horizontal, vertical ); }
      void setWindowSize( Point2I size ) { mWindowSize = size; }       
      Point2I& getWindowSize() { return ( mWindowSize ); }
 
      void setWindowName (const char * windowName) 
      {
         if (windowName == NULL)
            dStrncpy( mWindowName, "", sizeof( mWindowName ));
         else
            dStrncpy( mWindowName, windowName, sizeof( mWindowName ) );
      }
      const char * getWindowName() { return mWindowName; }

      void setExePathName(const char* exePathName)
      {
         if (exePathName == NULL)
            dStrncpy(mExePathName, "", sizeof(mExePathName));
         else
            dStrncpy(mExePathName, exePathName, sizeof(mExePathName));

         // set the base exe name field
         char tempBuf[2048];
         dStrncpy(tempBuf, mExePathName, 2048);
         dStrncpy(mExeName, basename(tempBuf), sizeof(mExeName));
      }
      const char * getExePathName() { return mExePathName; }
      const char * getExeName() { return mExeName; }

      bool windowCreated() { return mWindowCreated; }
      bool windowActive() { return mWindowActive; }
      bool windowLocked() { return mWindowLocked; }
      void setWindowCreated(bool windowCreated) 
          { mWindowCreated = windowCreated; }
      void setWindowActive(bool windowActive) 
          { mWindowActive = windowActive; }
      void setWindowLocked(bool windowLocked) 
          { mWindowLocked = windowLocked; }

      bool isXWindowsRunning() { return mXWindowsRunning; }
      void setXWindowsRunning(bool running) { mXWindowsRunning = running; }

      bool isDedicated() { return mDedicated; }
      void setDedicated(bool dedicated) { mDedicated = dedicated; }

      bool getCDAudioEnabled() { return mCDAudioEnabled; }
      void setCDAudioEnabled(bool enabled) { mCDAudioEnabled = enabled; }

      bool getDSleep() { return mDSleep; }
      void setDSleep(bool enabled) { mDSleep = enabled; }

      bool getUseRedirect() { return mUseRedirect; }
      void setUseRedirect(bool enabled) { mUseRedirect = enabled; }
      
      x86UNIXPlatformState()
      {
         currentTime = 0;
         mDesktopBpp = 16;
         mDesktopSize.set( 0, 0 );
         mWindowSize.set( 800, 600 );
         setWindowName("Torque");
         setExePathName(NULL);
         mWindowCreated = mWindowActive = mWindowLocked = false;
         mXWindowsRunning = false;
         mDedicated = false;
         mCDAudioEnabled = false;
         mDSleep = false;
#ifdef USE_FILE_REDIRECT
         mUseRedirect = true;
#else
         mUseRedirect = false;
#endif
      }
};

extern x86UNIXPlatformState  * x86UNIXState;

class DisplayPtrManager
{
      // static interface
   private:
      static bool sgDisplayLocked;
      static LockFunc_t sgLockFunc;
      static LockFunc_t sgUnlockFunc;

      static bool lockDisplay() 
      { 
         if (!sgDisplayLocked && sgLockFunc) 
         {
            sgLockFunc(); 
            sgDisplayLocked = true;
            return true;
         }
         else 
            return false;
      }
      static void unlockDisplay() 
      {
         if (sgDisplayLocked && sgUnlockFunc) 
         {
            sgUnlockFunc();
            sgDisplayLocked = false;
         }
      }

      //friend Display* x86UNIXPlatformState::getDisplayPointer();
     
   public:
      static void setDisplayLockFunction(LockFunc_t lockFunc) 
          { sgLockFunc = lockFunc; }
      static void setDisplayUnlockFunction(LockFunc_t unlockFunc) 
          { sgUnlockFunc = unlockFunc; }

      // nonstatic interface
   private:
      bool mAcquiredLock; // true if this instance acquired the display lock
      // (multiple instances of DisplayPtrManager can coexist, but only 
      // the first to access the display pointer will be responsible for 
      // acquiring and releasing the lock)
      bool mOpenedDisplay; // true if this instance created a display pointer
      // because the one in platform state was null.
      Display* mDisplay;

   private:
      Display* openDisplay()
      {
#ifndef TORQUE_DEDICATED
         mDisplay = XOpenDisplay(NULL);
         if (mDisplay != NULL)
            mOpenedDisplay = true;
#endif
         return mDisplay;
      }

      void closeDisplay()
      {
         if (mOpenedDisplay)
         {
#ifndef TORQUE_DEDICATED
            XCloseDisplay(mDisplay);
            mDisplay = NULL;
            mOpenedDisplay = false;
#endif
         }
      }
   public:
      DisplayPtrManager() 
      {
         mAcquiredLock = false;
         mOpenedDisplay = false;
         mDisplay = NULL;
      }

      ~DisplayPtrManager()
      {
         if (mAcquiredLock)
         {
            DisplayPtrManager::unlockDisplay();
            mAcquiredLock = false;
         }
         closeDisplay();
      }

      Display* getDisplayPointer()
      {
         Display* display = x86UNIXState->getDisplayPointer();
         if (display == NULL)
            return openDisplay();

         mAcquiredLock = DisplayPtrManager::lockDisplay();
         return display;
      }
};
