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

#ifndef _WINCONSOLE_H_
#define _WINCONSOLE_H_

#define MAX_CMDS 10
#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

class WinConsole
{
   bool winConsoleEnabled;

   HANDLE stdOut;
   HANDLE stdIn;
   HANDLE stdErr;
   char inbuf[512];
   S32  inpos;
   bool lineOutput;
   char curTabComplete[512];
   S32  tabCompleteStart;
   char rgCmds[MAX_CMDS][512];
   S32  iCmdIndex;

   void printf(const char *s, ...);

public:
   WinConsole();
   ~WinConsole();

   void process();
   void enable(bool);
   void processConsoleLine(const char *consoleLine);
   static void create();
   static void destroy();
   static bool isEnabled();
};

extern WinConsole *WindowsConsole;

#endif
