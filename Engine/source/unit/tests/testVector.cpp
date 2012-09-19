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
#include "unit/memoryTester.h"
#include "core/util/tVector.h"

using namespace UnitTesting;

CreateUnitTest(TestVectorAllocate, "Types/Vector")
{
   void run()
   {
      MemoryTester m;
      m.mark();

      Vector<S32> *vector = new Vector<S32>;
      for(S32 i=0; i<1000; i++)
         vector->push_back(10000 + i);

      // Erase the first element, 500 times.
      for(S32 i=0; i<500; i++)
         vector->erase(U32(0));

      vector->compact();

      test(vector->size() == 500, "Vector was unexpectedly short!");

      delete vector;

      test(m.check(), "Vector allocation test leaked memory!");
   }
};