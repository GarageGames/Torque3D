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

#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXState.h"
#include "platformX86UNIX/x86UNIXStdConsole.h"
#include "platform/platformInput.h"
#include "console/console.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#ifndef TORQUE_DEDICATED
#include <SDL.h>
#endif

//-----------------------------------------------------------------------------
// This is a mainly a debugging function for intercepting a nonzero exit code
// and generating a core dump for a stack trace.
// Need an S64 here because postQuitMessage uses a U32, and
// forceshutdown uses an S32.  So S64 is needed to
// accomodate them both
static void CheckExitCode(S64 exitCode)
{
   if (exitCode != 0)
   {
      Con::errorf(ConsoleLogEntry::General, 
         "Nonzero exit code: %d, triggering SIGSEGV for core dump",
         exitCode);
      kill(getpid(), SIGSEGV);
   }
}

//-----------------------------------------------------------------------------
static void SignalHandler(int sigtype)
{
   if (sigtype == SIGSEGV || sigtype == SIGTRAP)
   {
      signal(SIGSEGV, SIG_DFL);
      signal(SIGTRAP, SIG_DFL);
      // restore the signal handling to default so that we don't get into 
      // a crash loop with ImmediateShutdown
      ImmediateShutdown(-sigtype, sigtype);
   }
   else
   {
      signal(sigtype, SIG_DFL);
      dPrintf("Unknown signal caught by SignalHandler: %d\n", sigtype);
      // exit to be safe
      ImmediateShutdown(1);
   }
}

//-----------------------------------------------------------------------------
void Cleanup(bool minimal)
{
   if (!minimal)
   {
      Input::destroy();
   }

   StdConsole::destroy();

#ifndef TORQUE_DEDICATED   
   SDL_Quit();
#endif
}

//-----------------------------------------------------------------------------
void ImmediateShutdown(S32 exitCode, S32 signalNum)
{
   bool segfault = signalNum > 0;

   Cleanup(segfault);

   if (!segfault)
   {
      dPrintf("Exiting\n");
      // exit (doesn't call destructors)
      _exit(exitCode);
   }
   else
   {
// there is a problem in kernel 2.4.17 which causes a hang when a segfault 
// occurs.  also subsequent runs of "ps" will hang and the machine has to be
// hard reset to clear up the problem
// JMQ: this bug appears to be fixed in 2.4.18
//#define KERNEL_2_4_WORKAROUND
#ifdef KERNEL_2_4_WORKAROUND
      dPrintf("Segmentation Fault (Exiting without core dump due to #define KERNEL_2_4_WORKAROUND)\n");
      dFflushStdout();
      _exit(exitCode);
#else
      // kill with signal
      kill(getpid(), signalNum);
#endif
   }

}

//-----------------------------------------------------------------------------
void ProcessControlInit()
{
   // JMQ: ignore IO signals background read/write terminal (so that we don't
   // get suspended in daemon mode)
   signal(SIGTTIN, SIG_IGN);
   signal(SIGTTOU, SIG_IGN);

   // we're not interested in the exit status of child processes, so this 
   // prevents zombies from accumulating.
#if defined(__FreeBSD__)
   signal(SIGCHLD, SIG_IGN);
#else
   signal(SIGCLD, SIG_IGN);
#endif

   // install signal handler for SIGSEGV, so that we can attempt
   // clean shutdown
   signal(SIGSEGV, &SignalHandler);
   signal(SIGTRAP, &SignalHandler);
}

//-----------------------------------------------------------------------------
void Platform::postQuitMessage(const S32 in_quitVal)
{
   // if we have a window send a quit event, otherwise just force shutdown
#if 0
   if (x86UNIXState->windowCreated())
   {
      CheckExitCode(in_quitVal);
      SendQuitEvent();
   }
   else
#endif
   {
      forceShutdown(in_quitVal);
   }
}

//-----------------------------------------------------------------------------
void Platform::debugBreak()
{
   // in windows, "Calling DebugBreak causes the program to display 
   // a dialog box as if it had crashed."  So we segfault.
   Con::errorf(ConsoleLogEntry::General, 
      "Platform::debugBreak: triggering SIGSEGV for core dump");
   //kill(getpid(), SIGSEGV);
   kill(getpid(), SIGTRAP);
}

//-----------------------------------------------------------------------------
void Platform::forceShutdown(S32 returnValue)
{
#if 0
   // if a dedicated server is running, turn it off
   if (x86UNIXState->isDedicated() && Game->isRunning())
      Game->setRunning(false);
   else
#endif
      ImmediateShutdown(returnValue);
}

//-----------------------------------------------------------------------------
void Platform::outputDebugString(const char *string, ...)
{
   char buffer[2048];
   
   va_list args;
   va_start( args, string );
   
   dVsprintf( buffer, sizeof(buffer), string, args );
   va_end( args );
   
   U32 length = dStrlen(buffer);
   if( length == (sizeof(buffer) - 1 ) )
      length--;
   
   buffer[length++]  = '\n';
   buffer[length]    = '\0';
   
   fwrite(buffer, sizeof(char), length, stderr);
}

//-----------------------------------------------------------------------------
// testing function
ConsoleFunction(debug_debugbreak, void, 1, 1, "debug_debugbreak()")
{
   Platform::debugBreak();
}

//-----------------------------------------------------------------------------
void Platform::restartInstance()
{
/*
   if (Game->isRunning() )
   {
      //Con::errorf( "Error restarting Instance. Game is Still running!");
      return;
   }

   char cmd[2048];
   sprintf(cmd, "%s &", x86UNIXState->getExePathName());
   system(cmd);
*/
   exit(0);
}
