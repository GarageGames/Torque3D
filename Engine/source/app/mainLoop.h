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

#ifndef _APP_MAINLOOP_H_
#define _APP_MAINLOOP_H_

#include "platform/platform.h"

/// Support class to simplify the process of writing a main loop for Torque apps.
class StandardMainLoop
{
public:
   /// Initialize core libraries and call registered init functions
   static void init();

   /// Pass command line arguments to registered functions and main.cs
   static bool handleCommandLine(S32 argc, const char **argv);

   /// A standard mainloop implementation.
   static bool doMainLoop();

   /// Shut down the core libraries and call registered shutdown fucntions.
   static void shutdown();

   static void setRestart( bool restart );
   static bool requiresRestart();

private:
   /// Handle "pre shutdown" tasks like notifying scripts BEFORE we delete
   /// stuff from under them.
   static void preShutdown();
};

#endif