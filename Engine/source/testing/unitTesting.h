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

#ifndef _UNIT_TESTING_H_
#define _UNIT_TESTING_H_

#ifdef TORQUE_TESTS_ENABLED

#include <gtest/gtest.h>

/// Convenience to define a test fixture with a Fixture suffix for use with
/// TEST_FIX.
#define FIXTURE(test_fixture)\
   class test_fixture##Fixture : public ::testing::Test

/// Allow test fixtures named with a Fixture suffix, so that we can name tests
/// after a class name rather than having to call them XXTest.
#define TEST_FIX(test_fixture, test_name)\
   GTEST_TEST_(test_fixture, test_name, test_fixture##Fixture, \
   ::testing::internal::GetTypeId<test_fixture##Fixture>())

#endif // TORQUE_TESTS_ENABLED

#endif // _UNIT_TESTING_H_
