//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "core/util/journal/process.h"

FIXTURE(Process)
{
public:
   U32 remainingTicks;
   void notification()
   {
      if(remainingTicks == 0)
         Process::requestShutdown();
      remainingTicks--;
   }
};

TEST_FIX(Process, BasicAPI)
{
   // We'll run 30 ticks, then quit.
   remainingTicks = 30;

   // Register with the process list.
   Process::notify(this, &ProcessFixture::notification);

   // And do 30 notifies, making sure we end on the 30th.
   for(S32 i = 0; i < 30; i++)
   {
      EXPECT_TRUE(Process::processEvents())
         << "Should quit after 30 ProcessEvents() calls - not before!";
   }

   EXPECT_FALSE(Process::processEvents())
      << "Should quit after the 30th ProcessEvent() call!";

   Process::remove(this, &ProcessFixture::notification);
};

#endif