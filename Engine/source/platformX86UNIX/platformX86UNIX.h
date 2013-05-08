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

#ifndef _PLATFORMX86UNIX_H_
#define _PLATFORMX86UNIX_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _EVENT_H_
#include "platform/input/event.h"
#endif

#include <stdio.h>
#include <string.h>

// these will be used to construct the user preference directory where
// created files will be stored (~/PREF_DIR_ROOT/PREF_DIR_GAME_NAME)
#define PREF_DIR_ROOT ".garagegames"
#define PREF_DIR_GAME_NAME "torqueDemo"

// event codes for custom SDL events
const S32 TORQUE_SETVIDEOMODE = 1;

extern bool GL_EXT_Init( void );

extern void PlatformBlitInit( void );

// Process Control functions
void Cleanup(bool minimal=false);
void ImmediateShutdown(S32 exitCode, S32 signalNum = 0);
void ProcessControlInit();
bool AcquireProcessMutex(const char *mutexName);

// Utility functions
// Convert a string to lowercase in place
char *strtolwr(char* str);

void DisplayErrorAlert(const char* errMsg, bool showSDLError = true);

// Just like strstr, except case insensitive
// (Found this function at http://www.codeguru.com/string/stristr.html)
extern char *stristr(char *szStringToBeSearched, const char *szSubstringToSearchFor);

extern "C"
{
   // x86UNIX doesn't have a way to automatically get the executable file name
   void setExePathName(const char* exePathName);
}
#endif
