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

#include "platform/platform.h"
#include "platform/platformInput.h"
#include "console/console.h"

#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXStdConsole.h"

//------------------------------------------------------------------------------
void Platform::init()
{
   Con::printf("Initializing platform...");

   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "x86UNIX" );
#if defined(__linux__)
   Con::setVariable( "$platformUnixType", "Linux" );
#elif defined(__OpenBSD__)
   Con::setVariable( "$platformUnixType", "OpenBSD" );
#else
   Con::setVariable( "$platformUnixType", "Unknown" );
#endif

   StdConsole::create();

   Input::init();

   //installRedBookDevices();

#if 0
#ifndef TORQUE_DEDICATED
   // if we're not dedicated do more initialization
   if (!x86UNIXState->isDedicated())
   {
      // init SDL
      if (!InitSDL())
      {
         DisplayErrorAlert("Unable to initialize SDL.");
         ImmediateShutdown(1);
      }

      Con::printf( "Video Init:" );

      // load gl library
      if (!GLLoader::OpenGLInit())
      {
         DisplayErrorAlert("Unable to initialize OpenGL.");
         ImmediateShutdown(1);
      }

      // initialize video
      Video::init();
      if ( Video::installDevice( OpenGLDevice::create() ) )
         Con::printf( "   OpenGL display device detected." );
      else
         Con::printf( "   OpenGL display device not detected." );

      Con::printf(" ");
   }
#endif
   // if we are dedicated, do sleep timing and display results
   if (x86UNIXState->isDedicated())
   {
      const S32 MaxSleepIter = 10;
      U32 totalSleepTime = 0;
      U32 start;
      for (S32 i = 0; i < MaxSleepIter; ++i)
      {
         start = Platform::getRealMilliseconds();
         Sleep(0, 1000000);
         totalSleepTime += Platform::getRealMilliseconds() - start;
      }
      U32 average = static_cast<U32>(totalSleepTime / MaxSleepIter);

      Con::printf("Sleep latency: %ums", average);
      // dPrintf as well, since console output won't be visible yet
      dPrintf("Sleep latency: %ums\n", average);
      if (!x86UNIXState->getDSleep() && average < 10)
      {
         const char* msg = "Sleep latency ok, enabling dsleep for lower cpu " \
            "utilization";
         Con::printf("%s", msg);
         dPrintf("%s\n", msg);
         x86UNIXState->setDSleep(true);
      }
   }
#endif
}

//------------------------------------------------------------------------------
void Platform::shutdown()
{
   Cleanup();
}

//------------------------------------------------------------------------------


extern "C"
{
   bool torque_engineinit(int argc, const char **argv);
   int  torque_enginetick();
   bool torque_engineshutdown();

   int torque_unixmain(int argc, const char **argv)
   {
     if (!torque_engineinit(argc, argv))
         return 1;

     while(torque_enginetick())
     {

     }

     torque_engineshutdown();

     return 0;

   }
}

extern S32 TorqueMain(S32 argc, const char **argv);

int main(int argc, const char **argv)
{
   return TorqueMain(argc, argv);
}
