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

#include "core/util/journal/process.h"
#include "core/util/journal/journal.h"
#include "core/module.h"


MODULE_BEGIN( Process )

   MODULE_INIT
   {
      Process::init();
   }
   
   MODULE_SHUTDOWN
   {
      Process::shutdown();
   }

MODULE_END;

static Process* _theOneProcess = NULL; ///< the one instance of the Process class

//-----------------------------------------------------------------------------

void Process::requestShutdown()
{
   Process::get()._RequestShutdown = true;
}

//-----------------------------------------------------------------------------

Process::Process()
:  _RequestShutdown( false )
{
}

Process &Process::get()
{
   struct Cleanup
   {
      ~Cleanup()
      {
         if( _theOneProcess )
            delete _theOneProcess;
      }
   };
   static Cleanup cleanup;

   // NOTE that this function is not thread-safe
   //    To make it thread safe, use the double-checked locking mechanism for singleton objects

   if ( !_theOneProcess )
      _theOneProcess = new Process;

   return *_theOneProcess;
}

bool Process::init()
{
   return Process::get()._signalInit.trigger();
}

void  Process::handleCommandLine(S32 argc, const char **argv)
{
   Process::get()._signalCommandLine.trigger(argc, argv);
}

bool  Process::processEvents()
{
   // Process all the devices. We need to call these even during journal
   // playback to ensure that the OS event queues are serviced.
   Process::get()._signalProcess.trigger();

   if (!Process::get()._RequestShutdown) 
   {
      if (Journal::IsPlaying())
         return Journal::PlayNext();
      return true;
   }

   // Reset the Quit flag so the function can be called again.
   Process::get()._RequestShutdown = false;
   return false;
}

bool  Process::shutdown()
{
   return Process::get()._signalShutdown.trigger();
}
