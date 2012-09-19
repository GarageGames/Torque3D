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

#include "unit/test.h"
#include "core/util/journal/process.h"
#include "core/util/safeDelete.h"

using namespace UnitTesting;

CreateUnitTest(TestingProcess, "Journal/Process")
{
   // How many ticks remaining?
   U32 _remainingTicks;

   // Callback for process list.
   void process()
   {
      if(_remainingTicks==0)
         Process::requestShutdown();

      _remainingTicks--;
   }

   void run()
   {
      // We'll run 30 ticks, then quit.
      _remainingTicks = 30;

      // Register with the process list.
      Process::notify(this, &TestingProcess::process);

      // And do 30 notifies, making sure we end on the 30th.
      for(S32 i=0; i<30; i++)
         test(Process::processEvents(), "Should quit after 30 ProcessEvents() calls - not before!");
      test(!Process::processEvents(), "Should quit after the 30th ProcessEvent() call!");
   }
};