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

#include <stdio.h>
#include <string.h>

#include "core/strings/stringFunctions.h"

#include "console/console.h"

#include "unit/test.h"


namespace UnitTesting
{

//-----------------------------------------------------------------------------

TestRegistry *TestRegistry::_list = 0;


//-----------------------------------------------------------------------------

static const int MaxMarginCount = 32;
static const int MaxMarginValue = 128;
static int _Margin[MaxMarginCount] = { 3 };
static int* _MarginPtr = _Margin;
static char _MarginString[MaxMarginValue];

static void _printMargin()
{
   if (*_MarginPtr)
      ::fwrite(_MarginString,1,*_MarginPtr,stdout);
}

void UnitMargin::Push(int margin)
{
   if (_MarginPtr < _Margin + MaxMarginCount) {
      *++_MarginPtr = (margin < MaxMarginValue)? margin: MaxMarginValue;
      memset(_MarginString,' ',*_MarginPtr);
   }
}

void UnitMargin::Pop()
{
   if (_MarginPtr > _Margin) {
      _MarginPtr--;
      memset(_MarginString,' ',*_MarginPtr);
   }
}

int UnitMargin::Current()
{
   return *_MarginPtr;
}

void UnitPrint(const char* str)
{
   static bool lineStart = true;
   Platform::outputDebugString(str);

   // Need to scan for '\n' in order to support margins
   const char* ptr = str, *itr = ptr;
   for (; *itr != 0; itr++)
      if (*itr == '\n') 
      {
         if (lineStart)
            _printMargin();
         ::fwrite(ptr,1,itr - ptr + 1,stdout);
         ptr = itr + 1;
         lineStart = true;
      }

   // End the line with a carriage return unless the
   // line ends with a line continuation char.
   if (ptr != itr) {
      if (lineStart)
         _printMargin();
      if (itr[-1] == '\\') {
         ::fwrite(ptr,1,itr - ptr - 1,stdout);
         lineStart = false;
      }
      else {
         ::fwrite(ptr,1,itr - ptr,stdout);
         ::fwrite("\n",1,1,stdout);
         lineStart = true;
      }
   }
   else {
      ::fwrite("\n",1,1,stdout);
      lineStart = true;
   }
   ::fflush(stdout);
}


//-----------------------------------------------------------------------------

UnitTest::UnitTest() {
   _testCount = 0;
   _failureCount = 0;
   _warningCount = 0;
   _lastTestResult = true;
}

void UnitTest::fail(const char* msg)
{
   Con::warnf("** Failed: %s",msg);
   dFetchAndAdd( _failureCount, 1 );
}

void UnitTest::warn(const char* msg)
{
   Con::warnf("** Warning: %s",msg);
   dFetchAndAdd( _warningCount, 1 );
}


//-----------------------------------------------------------------------------

TestRegistry::TestRegistry(const char* name, bool interactive, const char *className)
{
   // Check that no existing test uses the same class-name; this is guaranteed
   // to lead to funkiness.
   TestRegistry *walk = _list;
   while(walk)
   {
      if(walk->_className)
      {
         AssertFatal(dStricmp(className, walk->_className), "TestRegistry::TestRegistry - got two unit tests with identical class names; they must have unique class names!");
      }

      walk = walk->_next;
   }

   // Add us to the list.
   _next = _list;
   _list = this;
   
   // And fill in our fields.
   _name = name;
   _className = className;
   _isInteractive = interactive;
}

DynamicTestRegistration::DynamicTestRegistration( const char *name, UnitTest *test ) : TestRegistry( name, false, NULL ), mUnitTest( test )
{

}

DynamicTestRegistration::~DynamicTestRegistration()
{
   // Un-link ourselves from the test registry
   TestRegistry *walk = _list;

   // Easy case!
   if( walk == this )
      _list = _next;
   else
   {
      // Search for us and remove
      while( ( walk != 0 ) && ( walk->_next != 0 ) && ( walk->_next != this ) )
         walk = walk->_next;

      // When this loop is broken, walk will be the unit test in the list previous to this one
      if( walk != 0 && walk->_next != 0 )
         walk->_next = walk->_next->_next;
   }
}


//-----------------------------------------------------------------------------

TestRun::TestRun()
{
   _subCount = 0;
   _testCount = 0;
   _failureCount = 0;
   _warningCount = 0;
}

void TestRun::printStats()
{
   Con::printf("-- %d test%s run (with %d sub-test%s)",
      _testCount,(_testCount != 1)? "s": "",
      _subCount,(_subCount != 1)? "s": "");

   if (_testCount)
   {
      if (_failureCount)
         Con::printf("** %d reported failure%s",
            _failureCount,(_failureCount != 1)? "s": "");
      else if (_warningCount)
         Con::printf("** %d reported warning%s",
            _warningCount,(_warningCount != 1)? "s": "");
      else
         Con::printf("-- No reported failures");
   }
}

void TestRun::test(TestRegistry* reg)
{
   Con::printf("-- Testing: %s %s",reg->getName(), reg->isInteractive() ? "(interactive)" : "" );

   UnitMargin::Push(_Margin[0]);

   // Run the test.
   UnitTest* test = reg->newTest();
   test->run();

   UnitMargin::Pop();

   // Update stats.
   _failureCount += test->getFailureCount();
   _subCount += test->getTestCount();
   _warningCount += test->getWarningCount();
   _testCount++;

   // Don't forget to delete the test!
   delete test;
}

// [tom, 2/5/2007] To provide a predictable environment for the tests, this
// now changes the current directory to the executable's directory before
// running the tests. The previous current directory is restored on exit.

bool TestRun::test(const char* module, bool skipInteractive)
{
   StringTableEntry cwdSave = Platform::getCurrentDirectory();

   int len = strlen(module);

   const char *skipMsg = skipInteractive ? "(skipping interactive tests)" : "";

   // Indicate to the user what we're up to.
   if (!len)
      Con::printf("-- Running all unit tests %s", skipMsg);
   else
      Con::printf("-- Running %s tests %s",module, skipMsg);
      
   for (TestRegistry* itr = TestRegistry::getFirst(); itr; itr = itr->getNext())
   {
      if (!len || !dStrnicmp(module,itr->getName(),len))
      {
         // Skip the test if it's interactive and we're in skipinteractive mode.
         if(skipInteractive && itr->isInteractive())
            continue;
         
         // Otherwise, run the test!
         Platform::setCurrentDirectory(Platform::getMainDotCsDir());
         test(itr);
      }
   }
   // Print out a nice report on how we did.
   printStats();

   Platform::setCurrentDirectory(cwdSave);

   // And indicate our failure situation in the return value.
   return !_failureCount;
}

} // Namespace

