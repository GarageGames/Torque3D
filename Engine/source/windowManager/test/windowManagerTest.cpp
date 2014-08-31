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
#include "windowManager/platformWindowMgr.h"

// Mysteriously, TEST(WindowManager, BasicAPI) gives an error. Huh.
TEST(WinMgr, BasicAPI)
{
   PlatformWindowManager *pwm = CreatePlatformWindowManager();

   // Check out the primary desktop area...
   RectI primary = pwm->getPrimaryDesktopArea();

   EXPECT_TRUE(primary.isValidRect())
      << "Got some sort of invalid rect from the window manager!";

   // Now try to get info about all the monitors.
   Vector<RectI> monitorRects;
   pwm->getMonitorRegions(monitorRects);

   EXPECT_GT(monitorRects.size(), 0)
      << "Should get at least one monitor rect back from getMonitorRegions!";

   // This test is here just to detect overflow/runaway situations. -- BJG
   EXPECT_LT(monitorRects.size(), 64)
      << "Either something's wrong, or you have a lot of monitors...";

   for(S32 i=0; i<monitorRects.size(); i++)
   {
      EXPECT_TRUE(monitorRects[i].isValidRect())
         << "Got an invalid rect for this monitor - no good.";
   }

   // No way to destroy the window manager.
};

#endif