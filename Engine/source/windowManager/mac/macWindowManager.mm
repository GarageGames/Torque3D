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

#include <Cocoa/Cocoa.h>
#include "windowManager/mac/macWindowManager.h"
#include "windowManager/mac/macWindow.h"
#include "core/util/journal/process.h"
#include "console/console.h"
#include "gfx/gfxDevice.h"

PlatformWindowManager* CreatePlatformWindowManager()
{
   return new MacWindowManager();
}

static inline RectI convertCGRectToRectI(NSRect r)
{
   return RectI(r.origin.x, r.origin.y, r.size.width, r.size.height);
}

MacWindowManager::MacWindowManager() : mNotifyShutdownDelegate(this, &MacWindowManager::onShutdown), mIsShuttingDown(false)
{
   mWindowList.clear();
   Process::notifyShutdown(mNotifyShutdownDelegate);
}

MacWindowManager::~MacWindowManager()
{  
   for(U32 i = 0; i < mWindowList.size(); i++)
      delete mWindowList[i];
   mWindowList.clear();
   
   CGReleaseDisplayFadeReservation(mFadeToken);
}

RectI MacWindowManager::getPrimaryDesktopArea()
{
   // Get the area of the main desktop that isn't taken by the dock or menu bar.
   return convertCGRectToRectI([[NSScreen mainScreen] visibleFrame]);
}

void MacWindowManager::getMonitorRegions(Vector<RectI> &regions)
{
   // Populate a vector with all monitors and their extents in window space.
   NSArray *screenList = [NSScreen screens];
   for(U32 i = 0; i < [screenList count]; i++)
   {
      NSRect screenBounds = [[screenList objectAtIndex: i] frame];
      regions.push_back(convertCGRectToRectI(screenBounds));
   }
}

S32 MacWindowManager::getDesktopBitDepth()
{
   // get the current desktop bit depth
   // TODO: return -1 if an error occurred
   return NSBitsPerPixelFromDepth([[NSScreen mainScreen] depth]);
}

Point2I MacWindowManager::getDesktopResolution()
{
   // get the current desktop width/height
   // TODO: return Point2I(-1,-1) if an error occurred
   NSRect desktopBounds = [[NSScreen mainScreen] frame];
   return Point2I((U32)desktopBounds.size.width, (U32)desktopBounds.size.height);
}

S32 MacWindowManager::getWindowCount()
{
   // Get the number of PlatformWindow's in this manager
   return mWindowList.size();
}

void MacWindowManager::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   // Populate a list with references to all the windows created from this manager.
   windows.merge(mWindowList);
}

PlatformWindow * MacWindowManager::getFirstWindow()
{
   if (mWindowList.size() > 0)
      return mWindowList[0];
      
   return NULL;
}


PlatformWindow* MacWindowManager::getFocusedWindow()
{
   for (U32 i = 0; i < mWindowList.size(); i++)
   {
      if( mWindowList[i]->isFocused() )
         return mWindowList[i];
   }

   return NULL;
}

PlatformWindow* MacWindowManager::getWindowById(WindowId zid)
{
   // Find the window by its arbirary WindowId.
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      PlatformWindow* w = mWindowList[i];
      if( w->getWindowId() == zid)
         return w;
   }
   return NULL;
}

void MacWindowManager::lowerCurtain()
{
   // fade all displays.
   CGError err;
   err = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval, &mFadeToken);
   AssertWarn(!err, "MacWindowManager::lowerCurtain() could not get a token");
   if(err) return;
   
   err = CGDisplayFade(mFadeToken, 0.3, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, true);
   AssertWarn(!err, "MacWindowManager::lowerCurtain() failed the fade");
   if(err) return;
   
   // we do not release the token, because that will un-fade the screen!
   // the token will last for 15 sec, and then the screen will un-fade regardless.
   //CGReleaseDisplayFadeReservation(mFadeToken);
}

void MacWindowManager::raiseCurtain()
{
   // release the fade on all displays
   CGError err;
   err = CGDisplayFade(mFadeToken, 0.3, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, false);
   AssertWarn(!err, "MacWindowManager::raiseCurtain() failed the fade");
   
   err = CGReleaseDisplayFadeReservation(mFadeToken);
   AssertWarn(!err, "MacWindowManager::raiseCurtain() failed releasing the token");
}


void MacWindowManager::_processCmdLineArgs(const S32 argc, const char **argv)
{
   // TODO: accept command line args if necessary.
}

PlatformWindow *MacWindowManager::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   MacWindow* window = new MacWindow(getNextId(), getEngineProductString(), mode.resolution);
   _addWindow(window);
   
   // Set the video mode on the window
   window->setVideoMode(mode);

   // Make sure our window is shown and drawn to.
   window->show();

   // Bind the window to the specified device.
   if(device)
   {
      window->mDevice = device;
      window->mTarget = device->allocWindowTarget(window);
      AssertISV(window->mTarget, 
         "MacWindowManager::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("MacWindowManager::createWindow - created a window with no device!");
   }

   return window;
}

void MacWindowManager::_addWindow(MacWindow* window)
{
#ifdef TORQUE_DEBUG
   // Make sure we aren't adding the window twice
   for(U32 i = 0; i < mWindowList.size(); i++)
      AssertFatal(window != mWindowList[i], "MacWindowManager::_addWindow - Should not add a window more than once");
#endif
   if (mWindowList.size() > 0)
      window->mNextWindow = mWindowList.last();
   else
      window->mNextWindow = NULL;

   mWindowList.push_back(window);
   window->mOwningWindowManager = this;
   window->appEvent.notify(this, &MacWindowManager::_onAppSignal);
}

void MacWindowManager::_removeWindow(MacWindow* window)
{
   for(WindowList::iterator i = mWindowList.begin(); i != mWindowList.end(); i++)
   {
      if(*i == window)
      {
         mWindowList.erase(i);
         return;
      }
   }
   AssertFatal(false, avar("MacWindowManager::_removeWindow - Failed to remove window %x, perhaps it was already removed?", window));
}

void MacWindowManager::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
   
   for(U32 i = 0; i < mWindowList.size(); i++)
   {
      if(mWindowList[i]->getWindowId() == wnd)
         continue;
      
      mWindowList[i]->signalGainFocus();
   }
}

bool MacWindowManager::onShutdown()
{
   mIsShuttingDown = true;
   return true;
}

bool MacWindowManager::canWindowGainFocus(MacWindow* window)
{
   return !mIsShuttingDown;
}
