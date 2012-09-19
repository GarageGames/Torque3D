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

#include "platformMac/platformMacCarb.h"
#include "platform/event.h"
#include "core/util/journal/process.h"
#include "console/console.h"

void Platform::postQuitMessage(const U32 in_quitVal)
{
   Process::requestShutdown();
}

void Platform::debugBreak()
{
   DebugStr("\pDEBUG_BREAK!");
}

void Platform::forceShutdown(S32 returnValue)
{
   exit(returnValue);
}   

void Platform::restartInstance()
{
   // execl() leaves open file descriptors open, that's the main reason it's not
   // used here. We want to start fresh.
   // get the path to the torque executable
   CFBundleRef mainBundle =  CFBundleGetMainBundle();
   CFURLRef execURL = CFBundleCopyExecutableURL(mainBundle);
   CFStringRef execString = CFURLCopyFileSystemPath(execURL, kCFURLPOSIXPathStyle);

   // append ampersand so that we can launch without blocking.
   // encase in quotes so that spaces in the path are accepted.
   CFMutableStringRef mut = CFStringCreateMutableCopy(NULL, 0, execString);
   CFStringInsert(mut, 0, CFSTR("\""));
   CFStringAppend(mut, CFSTR("\" & "));
   
   U32 len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(mut), kCFStringEncodingUTF8);
   char *execCString = new char[len+1];
   CFStringGetCString(mut, execCString, len, kCFStringEncodingUTF8);
   execCString[len] = '\0';
   
   Con::printf("---- %s -----",execCString);
   system(execCString);
}
