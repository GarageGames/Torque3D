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
#include "console/compiler.h"
#include "console/consoleInternal.h"
#include "core/util/tDictionary.h"
#include "core/strings/stringFunctions.h"
#include "app/mainLoop.h"
#include "windowManager/platformWindow.h"
#include "windowManager/platformWindowMgr.h"

#ifdef TORQUE_OS_WIN32
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/winDispatch.h"
extern void createFontInit(void);
extern void createFontShutdown(void);   
#endif

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
   extern INT CreateMiniDump(LPEXCEPTION_POINTERS ExceptionInfo);
#endif

static HashTable<StringTableEntry,StringTableEntry> gSecureScript;

#ifdef TORQUE_OS_MAC

// ObjC hooks for shared library support
// See:  macMain.mm

void torque_mac_engineinit(int argc, const char **argv);
void  torque_mac_enginetick();
void torque_mac_engineshutdown();

#endif 

extern bool LinkConsoleFunctions;

extern "C" {

   // reset the engine, unloading any current level and returning to the main menu
	void torque_reset()
	{
		Con::evaluate("disconnect();");
	}

   // initialize Torque 3D including argument handling
	int torque_engineinit(S32 argc, const char **argv)
	{

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif

		LinkConsoleFunctions = true;

#if !defined(TORQUE_OS_XENON) && !defined(TORQUE_OS_PS3) && defined(_MSC_VER)
		createFontInit();
#endif


#ifdef TORQUE_OS_MAC
		torque_mac_engineinit(argc, argv);
#endif
		// Initialize the subsystems.
		StandardMainLoop::init();

		// Handle any command line args.
		if(!StandardMainLoop::handleCommandLine(argc, argv))
		{
			Platform::AlertOK("Error", "Failed to initialize game, shutting down.");
			return false;
		}

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }

		__except( CreateMiniDump(GetExceptionInformation()) )
		{
			_exit(0);
		}
#endif

      return true;

	}

   // tick Torque 3D's main loop
	int torque_enginetick()
	{

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif


#ifdef TORQUE_OS_MAC
		torque_mac_enginetick();
#endif

		bool ret = StandardMainLoop::doMainLoop(); 
      return ret;

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }
		__except( CreateMiniDump(GetExceptionInformation()) )
		{
			_exit(0);
		}
#endif

      

	}

   // signal an engine shutdown (as with the quit(); console command)
	void torque_enginesignalshutdown()
	{
		Con::evaluate("quit();");
	}

   // shutdown the engine
	int torque_engineshutdown()
	{

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      __try {
#endif

		// Clean everything up.
		StandardMainLoop::shutdown();

#if !defined(TORQUE_OS_XENON) && !defined(TORQUE_OS_PS3) && defined(_MSC_VER)
		createFontShutdown();
#endif

#ifdef TORQUE_OS_MAC
		torque_mac_engineshutdown();
#endif

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )
      }

		__except( CreateMiniDump(GetExceptionInformation()) )
		{
			_exit(0);
		}
#endif

		// Return.  
		return true;

	}

	bool torque_isdebugbuild()
	{
#ifdef _DEBUG
		return true;
#else
		return false;
#endif

	}

	int torque_getconsolebool(const char* name)
	{
		return Con::getBoolVariable(name);
	}

	void torque_setconsolebool(const char* name, bool value)
	{
		Con::setBoolVariable(name, value);
	}

	static char* gExecutablePath = NULL;

	const char* torque_getexecutablepath()
	{
		return gExecutablePath;
	} 

	void torque_setexecutablepath(const char* directory)
	{
		gExecutablePath = new char[strlen(directory)+1];
		strcpy(gExecutablePath, directory);
	} 

   // set Torque 3D into web deployment mode (disable fullscreen exlusive mode, etc)
	void torque_setwebdeployment()
	{
		Platform::setWebDeployment(true);
	}

   // Get a console variable
	const char* torque_getvariable(const char* name)
	{
		return Con::getVariable(StringTable->insert(name));
	}

   // Set a console variable
	void torque_setvariable(const char* name, const char* value)
	{
		Con::setVariable(StringTable->insert(name), StringTable->insert(value));
	}

	static Namespace::Entry* GetEntry(const char* nameSpace, const char* name)                                          
	{
		Namespace* ns = NULL;

		if (!nameSpace || !dStrlen(nameSpace))
			ns = Namespace::mGlobalNamespace;
		else
		{
			nameSpace = StringTable->insert(nameSpace);
			ns = Namespace::find(nameSpace); //can specify a package here, maybe need, maybe not
		}

		if (!ns)
			return NULL;

		name = StringTable->insert(name);

		Namespace::Entry* entry = ns->lookupRecursive(name);

		return entry;
	}

   // Export a function to the Torque 3D console system which matches the StringCallback function prototype
   // specify the nameSpace, functionName, usage, min and max arguments 
	void torque_exportstringcallback(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
	{
		if (!nameSpace || !dStrlen(nameSpace))
			Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
		else
			Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
	}

	void torque_callvoidfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{

		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return;

		entry->cb.mVoidCallbackFunc(NULL, argc, argv);      
	}

	F32 torque_callfloatfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{

		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return 0.0f;

		return entry->cb.mFloatCallbackFunc(NULL, argc, argv);      
	}

	S32 torque_callintfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{

		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return 0;

		return entry->cb.mIntCallbackFunc(NULL, argc, argv);      
	}


	const char * torque_callstringfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{
		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return "";

		return entry->cb.mStringCallbackFunc(NULL, argc, argv);      
	}

	bool torque_callboolfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{
		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return "";

		return entry->cb.mBoolCallbackFunc(NULL, argc, argv);      
	}


	const char * torque_callscriptfunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{
		Namespace::Entry* entry = GetEntry(nameSpace, name);

		if (!entry)
			return "";

		if(!entry->mFunctionOffset)
			return "";

		const char* ret = entry->mCode->exec(entry->mFunctionOffset, StringTable->insert(name), entry->mNamespace, argc, argv, false, entry->mPackage);

		if (!ret || !dStrlen(ret))
			return "";

		return ret;

	}


   // Call a TorqueScript console function that has been marked as secure
	const char* torque_callsecurefunction(const char* nameSpace, const char* name, S32 argc, const char ** argv)
	{
		static const char* invalidChars = "()=:{}";
		String s = nameSpace;
		s += "::";
		s += name;
		s = String::ToUpper(s);

		if (!gSecureScript.count(StringTable->insert(s.c_str())))
		{
			Con::warnf("\nAttempt to call insecure script: %s\n", s.c_str());
			return "";
		}

		// scan through for invalid characters
		for (S32 i = 0; i < argc ; i++)
			for (S32 j = 0; j < dStrlen(invalidChars) ; j++)
				for (S32 k = 0; k < dStrlen(argv[i]); k++)
					if (invalidChars[j] == argv[i][k])
					{
						Con::warnf("\nInvalid parameter passed to secure script: %s, %s\n", s.c_str(), argv[i]);
						return "";
					}

					Namespace::Entry* entry = GetEntry(nameSpace, name);

					if (!entry)
						return "";

					static char returnBuffer[32];

					switch(entry->mType)
					{
					case Namespace::Entry::ConsoleFunctionType:
						return torque_callscriptfunction(nameSpace, name, argc, argv);

					case Namespace::Entry::StringCallbackType:
						return torque_callstringfunction(nameSpace, name, argc, argv);

					case Namespace::Entry::IntCallbackType:
						dSprintf(returnBuffer, sizeof(returnBuffer), "%d", torque_callintfunction(nameSpace, name, argc, argv));
						return returnBuffer;

					case Namespace::Entry::FloatCallbackType:
						dSprintf(returnBuffer, sizeof(returnBuffer), "%g", torque_callfloatfunction(nameSpace, name, argc, argv));
						return returnBuffer;

					case Namespace::Entry::VoidCallbackType:
						torque_callvoidfunction(nameSpace, name, argc, argv);
						return "";

					case Namespace::Entry::BoolCallbackType:
						dSprintf(returnBuffer, sizeof(returnBuffer), "%d", (U32) torque_callboolfunction(nameSpace, name, argc, argv));
						return returnBuffer;
					};

					return "";

	}

   // Set a TorqueScript console function as secure and available for JavaScript via the callScript plugin method
	void torque_addsecurefunction(const char* nameSpace, const char* fname)
	{
		String s = nameSpace;
		s += "::";
		s += fname;
		s = String::ToUpper(s);

		gSecureScript.insertEqual(StringTable->insert(s.c_str()), StringTable->insert(s.c_str()));
	}


   // Evaluate arbitrary TorqueScript (ONLY CALL torque_evaluate FROM TRUSTED CODE!!!)
	const char* torque_evaluate(const char* code)
	{
		return Con::evaluate(code);
	}

   // resize the Torque 3D child window to the specified width and height
	void torque_resizewindow(S32 width, S32 height)
	{
		if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
			PlatformWindowManager::get()->getFirstWindow()->setSize(Point2I(width,height));
	}

#ifdef TORQUE_OS_WIN32
   // retrieve the hwnd of our render window
   void* torque_gethwnd()
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*) PlatformWindowManager::get()->getFirstWindow();
         return (void *) w->getHWND();
      }

      return NULL;
   }

   // directly add a message to the Torque 3D event queue, bypassing the Windows event queue
   // this is useful in the case of the IE plugin, where we are hooking into an application 
   // level message, and posting to the windows queue would cause a hang
   void torque_directmessage(U32 message, U32 wparam, U32 lparam)
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*) PlatformWindowManager::get()->getFirstWindow();
         Dispatch(DelayedDispatch,w->getHWND(),message,wparam,lparam);
      }      
   }
   
#endif
}

// This function is solely to test the TorqueScript <-> Javascript binding
// By default, it is marked as secure by the web plugins and then can be called from
// Javascript on the web page to ensure that function calls across the language
// boundry are working with arguments and return values
ConsoleFunction(testJavaScriptBridge, const char *, 4, 4, "testBridge(arg1, arg2, arg3)")
{
	S32 failed = 0;
	if(argc != 4)
		failed = 1;
	else
	{
		if (dStrcmp(argv[1],"one"))
			failed = 2;
		if (dStrcmp(argv[2],"two"))
			failed = 2;
		if (dStrcmp(argv[3],"three"))
			failed = 2;
	}

	//attempt to call from TorqueScript -> JavaScript
	const char* jret = Con::evaluate("JS::bridgeCallback(\"one\",\"two\",\"three\");");

	if (dStrcmp(jret,"42"))
		failed = 3;

	char *ret = Con::getReturnBuffer(256);

	dSprintf(ret, 256, "%i", failed);

	return ret;
}





