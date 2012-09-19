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
#include "unit/test.h"
#include "core/threadStatic.h"
#include "math/mRandom.h"
#include "platform/profiler.h"

using namespace UnitTesting;

//-----------------------------------------------------------------------------
// This unit test will blow up without thread static support
#if defined(TORQUE_ENABLE_THREAD_STATICS) && defined(TORQUE_ENABLE_PROFILER)

// Declare a test thread static
DITTS( U32, gInstancedStaticFoo, 42 );
static U32 gTrueStaticFoo = 42;

CreateUnitTest( TestThreadStaticPerformance, "Core/ThreadStaticPerformance" )
{
   void run()
   {
      // Bail if the profiler is turned on right now
      if( !test( !gProfiler->isEnabled(), "Profiler is currently enabled, test cannot continue" ) )
         return;

      // Spawn an instance
      TorqueThreadStaticListHandle testInstance = _TorqueThreadStaticReg::spawnThreadStaticsInstance();

      static const dsize_t TEST_SIZE = 100000;

      // What we are going to do in this test is to test some U32 static
      // performance. The test will be run TEST_SIZE times, and so first create
      // an array of values to standardize the tests on.
      U32 testValue[TEST_SIZE];

      for( int i = 0; i < TEST_SIZE; i++ )
         testValue[i] = gRandGen.randI();

      // Reset the profiler, tell it to dump to console when done
      gProfiler->dumpToConsole();
      gProfiler->enable( true );

      // Value array is initialized, so lets put the foo's through the paces
      PROFILE_START(ThreadStaticPerf_TrueStaticAssign);
      for( int i = 0; i < TEST_SIZE; i++ )
         gTrueStaticFoo = testValue[i];
      PROFILE_END();

      PROFILE_START(ThreadStaticPerf_InstanceStaticAssign);
      for( int i = 0; i < TEST_SIZE; i++ )
         ATTS_( gInstancedStaticFoo, 1 ) = testValue[i];
      PROFILE_END();

      gProfiler->enable( false );

      // Clean up instance
      _TorqueThreadStaticReg::destroyInstance( testInstance );
   }
};

#endif // TORQUE_ENABLE_THREAD_STATICS