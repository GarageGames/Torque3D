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
#include "core/util/tVector.h"

// Define some test data used below.
FIXTURE(Vector)
{
public:
   struct Dtor
   {
      bool* ptr;
      Dtor() {} // Needed for vector increment.
      Dtor(bool* ptr): ptr(ptr) {}
      ~Dtor()
      {
         *ptr = true;
      }
   };

   static const S32 ints[];
   static const U32 length;
   static S32 QSORT_CALLBACK sortInts(const S32* a, const S32* b)
   {
      S32 av = *a;
      S32 bv = *b;

      if (av < bv)
         return -1;
      else if (av > bv)
         return 1;
      else
         return 0;
   }
};

const S32 VectorFixture::ints[] = {0, 10, 2, 3, 14, 4, 12, 6, 16, 7, 8, 1, 11, 5, 13, 9, 15};
const U32 VectorFixture::length = sizeof(VectorFixture::ints) / sizeof(S32);

TEST_FIX(Vector, Allocation)
{
   Vector<S32> *vector = new Vector<S32>;
   for (S32 i = 0; i < 1000; i++)
      vector->push_back(10000 + i);

   // Erase the first element, 500 times.
   for (S32 i = 0; i < 500; i++)
      vector->erase(U32(0));

   vector->compact();

   EXPECT_EQ(vector->size(), 500) << "Vector was unexpectedly short!";

   delete vector;
}

TEST_FIX(Vector, Deallocation)
{
   bool dtorVals[10];
   Vector<Dtor> v;

   // Only add the first 9 entries; the last is populated below.
   for (U32 i = 0; i < 9; i++)
      v.push_back(Dtor(&dtorVals[i]));

   // Fill the values array with false so we can test for destruction.
   for (U32 i = 0; i < 10; i++)
      dtorVals[i] = false;

   v.decrement();
   EXPECT_TRUE(dtorVals[8]) << "Vector::decrement failed to call destructor";

   v.decrement(2);
   EXPECT_TRUE(dtorVals[7]) << "Vector::decrement failed to call destructor";
   EXPECT_TRUE(dtorVals[6]) << "Vector::decrement failed to call destructor";

   v.pop_back();
   EXPECT_TRUE(dtorVals[5]) << "Vector::pop_back failed to call destructor";

   v.increment();
   v.last() = Dtor(&dtorVals[9]);
   v.clear();

   // All elements should have been destructed.
   for (U32 i = 0; i < 10; i++)
      EXPECT_TRUE(dtorVals[i])
         << "Element " << i << "'s destructor was not called";
}

TEST_FIX(Vector, Sorting)
{
   Vector<S32> v;

   for(U32 i = 0; i < length; i++)
      v.push_back(ints[i]);

   v.sort(sortInts);

   for(U32 i = 0; i < length - 1; i++)
      EXPECT_TRUE(v[i] <= v[i + 1])
         << "Element " << i << " was not in sorted order";
}

#endif