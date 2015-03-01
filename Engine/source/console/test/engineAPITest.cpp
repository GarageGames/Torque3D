#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"
#include "console/simBase.h"
#include "console/engineAPI.h"
#include "math/mMath.h"


TEST(EngineAPI, EngineMarshallData)
{
	// Reserve some values
	ConsoleValue values[16];
	ConsoleValueRef refValues[16];

	for (U32 i=0; i<16; i++)
	{
		values[i].init();
		refValues[i].value = &values[i];
	}

	// Basic string casting...
	SimObject *foo = new SimObject();
	foo->registerObject();

	const char *value = EngineMarshallData(foo);
	EXPECT_TRUE(dStricmp(value, foo->getIdString()) == 0) 
		<< "SimObject should be casted to its ID";

	U32 unsignedNumber = 123;
	S32 signedNumber = -123;
	value = EngineMarshallData(unsignedNumber);
	EXPECT_TRUE(dStricmp(value, "123") == 0) 
		<< "Integer should be converted to 123";
	value = EngineMarshallData(signedNumber);
	EXPECT_TRUE(dStricmp(value, "-123") == 0) 
		<< "Integer should be converted to -123";

	bool boolValue = true;
	value = EngineMarshallData(boolValue);
	EXPECT_TRUE(dStricmp(value, "1") == 0) 
		<< "Bool should be converted to 1";

	Point3F point(1,2,3);
	value = EngineMarshallData(point);
	EXPECT_TRUE(dStricmp(value, "1 2 3") == 0) 
		<< "Point3F should be converted to 1 2 3";

	F32 floatValue = 1.23f;
	value = EngineMarshallData(floatValue);
	EXPECT_TRUE(dStricmp(value, "1.23") == 0) 
		<< "F32 should be converted to 1.23";

	// Argv based casting
	S32 argc = 0;
	EngineMarshallData(foo, argc, refValues);
	EngineMarshallData((const SimObject*)foo, argc, refValues);
	EngineMarshallData(point, argc, refValues);
	EngineMarshallData(unsignedNumber, argc, refValues);
	EngineMarshallData(signedNumber, argc, refValues);
	EngineMarshallData(boolValue, argc, refValues);
	EngineMarshallData(floatValue, argc, refValues);

	EXPECT_TRUE(argc == 7) 
		<< "7 args should have been set";

	EXPECT_TRUE(values[0].type == ConsoleValue::TypeInternalInt && values[0].getSignedIntValue() == foo->getId())
		<< "1st arg should be foo's id";

	EXPECT_TRUE(values[1].type == ConsoleValue::TypeInternalInt && values[1].getSignedIntValue() == foo->getId())
		<< "2nd arg should be foo's id";

	EXPECT_TRUE(values[2].type == ConsoleValue::TypeInternalString && dStricmp(values[2].getStringValue(), "1 2 3") == 0)
		<< "3rd arg should be 1 2 3";

	EXPECT_TRUE(values[3].type == ConsoleValue::TypeInternalFloat && values[3].getSignedIntValue() == 123)
		<< "4th arg should be 123";

	EXPECT_TRUE(values[4].type == ConsoleValue::TypeInternalFloat && values[4].getSignedIntValue() == -123)
		<< "5th arg should be -123";

	EXPECT_TRUE(values[5].type == ConsoleValue::TypeInternalFloat && values[5].getBoolValue() == true)
		<< "6th arg should be -123";

	EXPECT_TRUE(values[6].type == ConsoleValue::TypeInternalFloat && mRound(values[6].getFloatValue() * 100) == 123)
		<< "7th arg should be 1.23";

	foo->deleteObject();
}

TEST(EngineAPI, EngineUnMarshallData)
{
	SimObject *foo = new SimObject();
	foo->registerObject();

	SimObject *testFoo = EngineUnmarshallData<SimObject*>()(foo->getIdString());

	EXPECT_TRUE(foo == testFoo)
		<< "Unmarshalling foo's id should return foo";

	testFoo = EngineUnmarshallData<SimObject*>()("ShouldNotExist_Really123");
	EXPECT_TRUE(testFoo == NULL)
		<< "Unmarshalling a bad object should return NULL";

	foo->deleteObject();
}

TEST(EngineAPI, _EngineConsoleCallbackHelper)
{
	Con::evaluate("if (isObject(TestConExec)) {\r\nTestConExec.delete();\r\n}\r\nfunction testExecutef(%a,%b,%c,%d,%e,%f,%g,%h,%i,%j,%k){return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j SPC %k;}\r\nfunction TestConExec::testThisFunction(%this,%a,%b,%c,%d,%e,%f,%g,%h,%i,%j){ return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j;}\r\nnew ScriptObject(TestConExec);\r\n", false, "test");
	
	SimObject *testObject = NULL;
	Sim::findObject("TestConExec", testObject);

   _EngineConsoleCallbackHelper helper("testExecutef", NULL);
   const char *returnValue = helper.call<const char*>("a", "b", "c");

	EXPECT_TRUE(dStricmp(returnValue, "a b c        ") == 0) <<
		"All values should be printed in the correct order";
   
   _EngineConsoleCallbackHelper objectHelper("testThisFunction", testObject);
   returnValue = helper.call<const char*>("a", "b", "c");

	EXPECT_TRUE(dStricmp(returnValue, "a b c        ") == 0) <<
		"All values should be printed in the correct order";
}

// NOTE: this is also indirectly tested by the executef tests
TEST(EngineAPI, _EngineConsoleExecCallbackHelper)
{
	Con::evaluate("if (isObject(TestConExec)) {\r\nTestConExec.delete();\r\n}\r\nfunction testExecutef(%a,%b,%c,%d,%e,%f,%g,%h,%i,%j,%k){return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j SPC %k;}\r\nfunction TestConExec::testThisFunction(%this,%a,%b,%c,%d,%e,%f,%g,%h,%i,%j){ return %a SPC %b SPC %c SPC %d SPC %e SPC %f SPC %g SPC %h SPC %i SPC %j;}\r\nnew ScriptObject(TestConExec);\r\n", false, "test");
	
	SimObject *testObject = NULL;
	Sim::findObject("TestConExec", testObject);

   _EngineConsoleExecCallbackHelper<const char*> helper("testExecutef");
   const char *returnValue = helper.call<const char*>("a", "b", "c");

	EXPECT_TRUE(dStricmp(returnValue, "a b c        ") == 0) <<
		"All values should be printed in the correct order";
   
   _EngineConsoleExecCallbackHelper<SimObject*> objectHelper(testObject);
   returnValue = objectHelper.call<const char*>("testThisFunction", "a", "b", "c");

	EXPECT_TRUE(dStricmp(returnValue, "a b c       ") == 0) <<
		"All values should be printed in the correct order";
}

#endif