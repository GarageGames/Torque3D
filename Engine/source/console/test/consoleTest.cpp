#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"
#include "console/simBase.h"
#include "console/engineAPI.h"
#include "math/mMath.h"
#include "console/stringStack.h"

TEST(Con, executef)
{
	char buffer[128];
	Con::evaluate("if (isObject(TestConExec)) {\r\nTestConExec.delete();\r\n}\r\nfunction testExecutef(%a,%b,%c,%d,%e,%f,%g,%h,%i,%j,%k){return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j SPC %k;}\r\nfunction TestConExec::testThisFunction(%this,%a,%b,%c,%d,%e,%f,%g,%h,%i,%j){ return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j;}\r\nnew ScriptObject(TestConExec);\r\n", false, "test");
	
	SimObject *testObject = NULL;
	Sim::findObject("TestConExec", testObject);

	EXPECT_TRUE(testObject != NULL)
		<< "TestConExec object should exist";

	// Check basic calls with SimObject. We'll do this for every single possible call just to make sure.
	const char *returnValue = NULL;
	
	returnValue = Con::executef(testObject, "testThisFunction");
	EXPECT_TRUE(dStricmp(returnValue, "         ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a");
	EXPECT_TRUE(dStricmp(returnValue, "a         ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b");
	EXPECT_TRUE(dStricmp(returnValue, "a b        ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c");
	EXPECT_TRUE(dStricmp(returnValue, "a b c       ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d      ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d", "e");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e     ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d", "e", "f");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f    ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d", "e", "f", "g");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g   ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d", "e", "f", "g", "h");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g h  ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef(testObject, "testThisFunction", "a", "b", "c", "d", "e", "f", "g", "h", "i");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g h i ") == 0) <<
		"All values should be printed in the correct order";

	// Now test without the object
	
	returnValue = Con::executef("testExecutef");
	EXPECT_TRUE(dStricmp(returnValue, "          ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a");
	EXPECT_TRUE(dStricmp(returnValue, "a          ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b");
	EXPECT_TRUE(dStricmp(returnValue, "a b         ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c");
	EXPECT_TRUE(dStricmp(returnValue, "a b c        ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d       ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e      ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e", "f");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f     ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e", "f", "g");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g    ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e", "f", "g", "h");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g h   ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e", "f", "g", "h", "i");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g h i  ") == 0) <<
		"All values should be printed in the correct order";
	
	returnValue = Con::executef("testExecutef", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j");
	EXPECT_TRUE(dStricmp(returnValue, "a b c d e f g h i j ") == 0) <<
		"All values should be printed in the correct order";

	// Test type conversions with and without SimObject...

	// Integer
	returnValue = Con::executef(testObject, "testThisFunction", 123);
	EXPECT_TRUE(dStricmp(returnValue, "123         ") == 0) <<
		"Integer should be converted";
	returnValue = Con::executef("testExecutef", 123);
	EXPECT_TRUE(dStricmp(returnValue, "123          ") == 0) <<
		"Integer should be converted";

	// Float
	returnValue = Con::executef(testObject, "testThisFunction", (F32)123.4);
	EXPECT_TRUE(dStricmp(returnValue, "123.4         ") == 0) <<
		"Float should be converted";
	returnValue = Con::executef("testExecutef", (F32)123.4);
	EXPECT_TRUE(dStricmp(returnValue, "123.4          ") == 0) <<
		"Float should be converted";

	// SimObject
	dSprintf(buffer, sizeof(buffer), "%i         ", testObject->getId());
	returnValue = Con::executef(testObject, "testThisFunction", testObject);
	EXPECT_TRUE(dStricmp(returnValue, buffer) == 0) <<
		"SimObject should be converted";
	dSprintf(buffer, sizeof(buffer), "%i          ", testObject->getId());
	returnValue = Con::executef("testExecutef", testObject);
	EXPECT_TRUE(dStricmp(returnValue, buffer) == 0) <<
		"SimObject should be converted";

	// Point3F
	Point3F point(1,2,3);
	returnValue = Con::executef(testObject, "testThisFunction", point);
	EXPECT_TRUE(dStricmp(returnValue, "1 2 3         ") == 0) <<
		"Point3F should be converted";
	returnValue = Con::executef("testExecutef", point);
	EXPECT_TRUE(dStricmp(returnValue, "1 2 3          ") == 0) <<
		"Point3F should be converted";

   // Finally test the function arg offset. This should be consistently 0 after each call
   
	EXPECT_TRUE(STR.mFunctionOffset == 0) <<
		"Function offset should be 0";

   const char *floatArg = Con::getFloatArg(1.23);
	EXPECT_TRUE(STR.mFunctionOffset > 0) <<
		"Function offset should not be 0";

   Con::executef("testExecutef", floatArg);
   
	EXPECT_TRUE(STR.mFunctionOffset == 0) <<
		"Function offset should be 0";

   floatArg = Con::getFloatArg(1.23);
	EXPECT_TRUE(STR.mFunctionOffset > 0) <<
		"Function offset should not be 0";

   Con::executef("testImaginaryFunction_", floatArg);
   
	EXPECT_TRUE(STR.mFunctionOffset == 0) <<
		"Function offset should be 0";
}

TEST(Con, execute)
{
	Con::evaluate("if (isObject(TestConExec)) {\r\nTestConExec.delete();\r\n}\r\nfunction testScriptExecuteFunction(%a,%b) {return %a SPC %b;}\nfunction TestConExec::testScriptExecuteFunction(%this, %a,%b) {return %a SPC %b;}new ScriptObject(TestConExec);\r\n", false, "testExecute");
	
	U32 startStackPos = CSTK.mStackPos;
	U32 startStringStackPos = STR.mStart;
	U32 startStackFrame = CSTK.mFrame;
	
	SimObject *testObject = NULL;
	Sim::findObject("TestConExec", testObject);

	EXPECT_TRUE(testObject != NULL)
		<< "TestConExec object should exist";

	// const char* versions of execute should maintain stack
	const char *argv[] = {"testScriptExecuteFunction", "1", "2"};
	const char *argvObject[] = {"testScriptExecuteFunction", "", "1", "2"};
	const char *returnValue = Con::execute(3, argv);

	EXPECT_TRUE(dStricmp(returnValue, "1 2") == 0) <<
		"execute should return 1 2";
	
	EXPECT_TRUE(CSTK.mStackPos == startStackPos) <<
		"execute should restore stack";

	returnValue = Con::execute(testObject, 4, argvObject);

	EXPECT_TRUE(dStricmp(returnValue, "1 2") == 0) <<
		"execute should return 1 2";
	
	EXPECT_TRUE(CSTK.mStackPos == startStackPos) <<
		"execute should restore stack";

	// ConsoleValueRef versions of execute should not restore stack
	CSTK.pushFrame();
	STR.pushFrame();

	ConsoleValue valueArg[4];
	ConsoleValueRef refArg[4];
	ConsoleValue valueArgObject[4];
	ConsoleValueRef refArgObject[4];
	for (U32 i=0; i<4; i++)
	{
		refArg[i].value = &valueArg[i];
		refArgObject[i].value = &valueArgObject[i];
		valueArgObject[i].init();
		valueArg[i].init();
	}

	refArg[0] = "testScriptExecuteFunction";
	refArg[1] = "1";
	refArg[2] = "2";

	refArgObject[0] = "testScriptExecuteFunction";
	refArgObject[2] = "1";
	refArgObject[3] = "2";

	returnValue = Con::execute(3, refArg);

	EXPECT_TRUE(dStricmp(returnValue, "1 2") == 0) <<
		"execute should return 1 2";
	
	EXPECT_TRUE(CSTK.mStackPos == startStackPos) <<
		"execute should restore stack";

	CSTK.popFrame();
	STR.popFrame();

	CSTK.pushFrame();
	STR.pushFrame();

	returnValue = Con::execute(testObject, 4, refArgObject);

	EXPECT_TRUE(dStricmp(returnValue, "1 2") == 0) <<
		"execute should return 1 2";
	
	EXPECT_TRUE(CSTK.mStackPos == startStackPos) <<
		"execute should restore stack";

	CSTK.popFrame();
	STR.popFrame();
}

static U32 gConsoleStackFrame = 0;

ConsoleFunction(testConsoleStackFrame, S32, 1, 1, "")
{
	gConsoleStackFrame = CSTK.mFrame;
	return (U32)Con::executef("testScriptEvalFunction"); // execute a sub function which manipulates the stack
}

TEST(Con, evaluate)
{
	U32 startStackPos = CSTK.mStackPos;
	U32 startStringStackPos = STR.mStart;
	S32 returnValue = Con::evaluate("function testScriptEvalFunction() {return \"1\"@\"2\"@\"3\";}\nreturn testConsoleStackFrame();", false, "testEvaluate");
	U32 frame = CSTK.mFrame;

	EXPECT_TRUE(returnValue == 123) <<
		"Evaluate should return 123";

	EXPECT_TRUE(gConsoleStackFrame == (frame+2)) <<
		"Console stack frame inside function should be +2";

	EXPECT_TRUE(CSTK.mFrame == frame) <<
		"Console stack frame outside function should be the same as before";

	EXPECT_TRUE(STR.mStart == startStringStackPos) <<
		"Console string stack should not be changed";

	EXPECT_TRUE(CSTK.mStackPos == startStackPos) <<
		"Console stack should not be changed";

}

#endif