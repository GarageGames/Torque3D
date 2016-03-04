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

#include <string>
#include <vector>

#include "npWebGamePlugin.h"

// Timer which ticks/renders Torque3D
static CFRunLoopTimerRef gTimer = NULL;

static bool gHidden = false;

static void timer_callback(CFRunLoopRef timer, void *info)
{  
      
   if (gHidden)
      return;
   
   if (!torque_enginetick())
   {
      //TODO:  undefined when get quit from Torque 3D under Safari
   }
   
}

NPWebGamePlugin* NPWebGamePlugin::sInstance = NULL; 

NPWebGamePlugin::NPWebGamePlugin(NPP aInstance)
{
	mOpen = FALSE;
	mInstance = aInstance;
	sInstance = this;
}

NPWebGamePlugin::~NPWebGamePlugin()
{
	Close();
	sInstance = NULL;
}

NPBool NPWebGamePlugin::Open(NPWindow* aWindow)
{
   gHidden = false;
      
   WebCommon::ChangeToGameDirectory();
   
	if (mOpen || WebCommon::gTorque3DModule)
	{
      NP_CGContext *npcontext = reinterpret_cast<NP_CGContext *> (aWindow->window);
      NSWindow* browserWindow = [[NSWindow alloc] initWithWindowRef:npcontext->window];   
      torque_setsafariwindow( browserWindow, aWindow->clipRect.left, aWindow->clipRect.top, aWindow->clipRect.right - aWindow->clipRect.left, aWindow->clipRect.bottom - aWindow->clipRect.top );
      [browserWindow release];
      
      mOpen = TRUE;
      
		return TRUE; 
	}
	if (!aWindow)
		return FALSE;

	mOpen = true;
   
	NP_CGContext *npcontext = reinterpret_cast<NP_CGContext *> (aWindow->window);
	
   // You may want to resize the browser window or set minimum sizes here for your web content  
   NSWindow* browserWindow = [[NSWindow alloc] initWithWindowRef:npcontext->window];  
   
   if (!browserWindow)
   {
      WebCommon::MessageBox( 0, "Web plugin unable to get browser window", "Error");
      return false;
   }

	if (!WebCommon::InitTorque3D(browserWindow, aWindow->clipRect.left, aWindow->clipRect.top, aWindow->clipRect.right - aWindow->clipRect.left, aWindow->clipRect.bottom - aWindow->clipRect.top))
      return false;
      
   //setup high speed timer to tick Torque 3D   
   CFAllocatorRef allocator = kCFAllocatorDefault;
   CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + .001;
   CFTimeInterval interval = .001;
   CFOptionFlags flags = 0;
   CFIndex order = 0;
   CFRunLoopTimerCallBack callback = (CFRunLoopTimerCallBack)timer_callback;
   CFRunLoopTimerContext context = { 0, NULL, NULL, NULL, NULL };
   gTimer = CFRunLoopTimerCreate(allocator, fireDate, interval, flags, order, callback, &context);
   CFRunLoopAddTimer(CFRunLoopGetCurrent(), gTimer, kCFRunLoopDefaultMode);
   
   [browserWindow release];

	return mOpen;
}

void NPWebGamePlugin::Close()
{

	if (!mOpen)
		return;
   
   if (WebCommon::gTorque3DModule)
   {
      gHidden = true;   
      torque_setsafariwindow(NULL, 0, 0, 0, 0);
      torque_reset();
   }

	//WebCommon::ShutdownTorque3D();

	mOpen = false;
}

NPBool NPWebGamePlugin::IsOpen()
{
	return mOpen;
}


