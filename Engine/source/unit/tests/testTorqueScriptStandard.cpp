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
#include "unit/memoryTester.h"
#include "console/engineAPI.h"
#include "console/SimObject.h"

using namespace UnitTesting;

//////////////////////////////////////////////////////////////////////////
CreateUnitTest(TestTorqueScriptStandard, "TorqueScript/StandardOperations")
{
	SimObject *TSTestObject;

	void StringEquals(const char* strA, const char* strB)
	{
		char buffer[2048];
		dSprintf(buffer, sizeof(buffer), "TSTestObject.testValue = (%s $= %s);", strA, strB);
		Con::evaluate(buffer);

		char outBuffer[256];
		dSprintf(outBuffer, sizeof(outBuffer), "Expected %s to equal %s", strA, strB);

		test( Con::getBoolVariable("TSTestObject.testValue"), outBuffer );
	}

	void StringDoesNotEqual(const char* strA, const char* strB)
	{
		char buffer[2048];
		dSprintf(buffer, sizeof(buffer), "TSTestObject.testValue = (%s $= %s);", strA, strB);
		Con::evaluate(buffer);

		char outBuffer[256];
		dSprintf(outBuffer, sizeof(outBuffer), "Expected %s to equal %s", strA, strB);

		test( !Con::getBoolVariable("TSTestObject.testValue"), outBuffer );
	}

	void Equals(const char* strA, const char* strB)
	{
		char buffer[2048];
		dSprintf(buffer, sizeof(buffer), "TSTestObject.testValue = (%s == %s);", strA, strB);
		Con::evaluate(buffer);

		char outBuffer[256];
		dSprintf(outBuffer, sizeof(outBuffer), "Expected %s to equal %s", strA, strB);

		test( Con::getBoolVariable("TSTestObject.testValue"), outBuffer );
	}

	void DoesNotEqual(const char* strA, const char* strB)
	{
		char buffer[2048];
		dSprintf(buffer, sizeof(buffer), "TSTestObject.testValue = (%s == %s);", strA, strB);
		Con::evaluate(buffer);

		char outBuffer[256];
		dSprintf(outBuffer, sizeof(outBuffer), "Expected %s to equal %s", strA, strB);

		test( !Con::getBoolVariable("TSTestObject.testValue"), outBuffer );
	}

   void run()
   {
		TSTestObject = new SimObject();
		TSTestObject->assignName("TSTestObject");
		TSTestObject->registerObject();

		//Strings
		//comparable with strequals
		StringEquals("\"\"", "\"\"");
		StringEquals("c", "c");
		StringDoesNotEqual("d", "e");
		StringDoesNotEqual("\"\"", "a");
		StringDoesNotEqual("b", "\"\"");

		//case insensitivity tests
		StringEquals("hi", "HI");
		StringEquals("ho", "HO");
		StringEquals("dfjdkfjdf", "DFJDKFJDF");

		//should always compare true with equals
		Equals("trip", "trip");
		Equals("trap", "TRAP");
		Equals("ping", "pong");
		Equals("pow", "\"\"");
		Equals("crash", "sdkhskhsdf");

		//Construct without quotes
		StringEquals("yellow", "\"yellow\"");
		StringEquals("red", "\"red\"");
		StringEquals("", "\"\"");

		//cast to zero when non-numeric
		Equals("\"luis\" + 0", "0");
		Equals("\"jeff\" + 1", "1");
		Equals("\"bank\" - 2", "-2");
		Equals("\"thomas\" * 2", "0");
		Equals("\"andrew\" / 2", "0");
		StringEquals("\"daniel\" / -1", "\"-0\"");

		//cast to numbers when numeric
		Equals("\"1\" + 1", "2");
		Equals("\"9\" - 6", "3");
		Equals("\"6\" * 2", "12");
		Equals("\"8\" / 4", "2");
		Equals("\"8.9\" + 0.1", "9");

		//constructed as ints or floats
		Equals("0", "0.0");
		Equals("1", "1.0");
		Equals("1000", "1000.0");
		Equals("-2", "-2.0");
		Equals("0", "-0");
		DoesNotEqual("5", "5.1");

		//cast to strings
		StringDoesNotEqual("1000", "\"1000.0\"");
		Equals("1001", "\"1001.0\"");
		StringDoesNotEqual("0", "\"-0\"");
		Equals("0", "\"-0\"");
		StringEquals("1000.0", "\"1000\"");
		StringEquals("0 @ 1", "\"01\"");
		StringEquals("goodbye @ 5", "\"goodbye5\"");

		//Order of operations
		Equals("2 * 2 + 1", "5");
		Equals("2 * (2 + 1)", "6");
		Equals("2 / 2 + 1", "2");
		Equals("2 / (2 + 1)", "2/3");
		Equals("2 * 2 - 1", "3");
		Equals("2 * (2 - 1)", "2");
		Equals("2 / 2 - 1", "0");
		Equals("2 / (2 - 1)", "2");

		// << and >> have low precedence
		Equals("1 << 1 + 1", "4");
		Equals("(1 << 1) + 1", "3");
		Equals("4 >> 1 + 1", "1");
		Equals("(4 >> 1) + 1", "3");

		//numbers shift
		Equals("0 >> 1", "0");
		Equals("0 << 1", "0");

		TSTestObject->deleteObject();
   }
};