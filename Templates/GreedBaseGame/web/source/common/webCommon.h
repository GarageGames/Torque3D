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

#ifndef _webcommon_h
#define _webcommon_h

// Common web functionality between IE/Safari/Firefox/Chrome

// Platform specific includes
#ifdef WIN32

   #include <windows.h>

#else  // Mac

   #include <Cocoa/Cocoa.h>
   #include <WebKit/npapi.h>
   #include <WebKit/npfunctions.h>
   #include <WebKit/npruntime.h>
   #include <dlfcn.h>
   
   #define OSCALL

#endif

// common includes
#include <string>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

// Function prototype matching Torque 3D console callback convention
typedef const char * (*StringCallback)(void *obj, int argc, const char *argv[]);

namespace WebCommon
{
   // loads the Torque 3D shared library, sets web deployment mode, retrieves engine "C" interface
	bool InitTorque3D(void *platformWindow, int clipLeft = 0, int clipTop= 0, int clipRight = 0, int clipBottom = 0);
   
   // sets the current directory to the game install path
   bool ChangeToGameDirectory();

   // unloads the Torque 3D shared library (first signaling a shutdown for clean exit)
	void ShutdownTorque3D();

   // checks a given domain against the allowed domains in webConfig.h
	bool CheckDomain(const char* url);

   // exposes TorqueScript functions marked as secure in webConfig.h, these functions can then be called on the page via Javascript
	void AddSecureFunctions();

   // bring up a platform specific message box (used for error reporting)
	void MessageBox(void* parentWindow, const char* msg, const char* caption);

	// a handy string function that eats white space
	void StringCopy(char* dst, const char* src, int count);

   void SetVariable(const char* variable, const char* value);
   const char* GetVariable(const char* variable);

   extern std::string gPluginMIMEType; 

#ifdef WIN32
   // the handle of our plugin's DLL
   extern HMODULE gPluginModule;
   // the handle of the Torque 3D DLL
   extern HMODULE gTorque3DModule;

   //string conversion to/from wstring and string
   std::string WStringToString( const std::wstring& wstr);
   std::wstring StringToWString( const std::string& str);

   void ConvertToWindowsPathSep(std::string& path);
#else //Mac
   // ptr to the Torque 3D Bundle
   extern void* gTorque3DModule;
#endif

};

// C Interface exported from the Torque 3D DLL (or Bundle on Mac)

extern "C"
{
   // initialize Torque 3D including argument handling
	extern bool (*torque_engineinit)(int argc, const char **argv);

   // tick Torque 3D's main loop
	extern int  (*torque_enginetick)();
	
   // set Torque 3D into web deployment mode (disable fullscreen exlusive mode, etc)
   extern int  (*torque_setwebdeployment)();

   // shutdown the engine
	extern bool (*torque_engineshutdown)();
   
   // signal an engine shutdown (as with the quit(); console command)
	extern void (*torque_enginesignalshutdown)();

   // reset the engine, unloading any current level and returning to the main menu
	extern void (*torque_reset)();

	// Evaluate arbitrary TorqueScript (ONLY CALL torque_evaluate FROM TRUSTED CODE!!!)
	extern const char* (*torque_evaluate)(const char* code);

   // Get a console variable
   // For security, restricted to "Javascript::" namespace 
	extern const char* (*torque_getvariable)(const char* name);
   // Set a console variable
   // For security, restricted to "Javascript::" namespace 
	extern void (*torque_setvariable)(const char* name, const char* value);

   // Export a function to the Torque 3D console system which matches the StringCallback function prototype
   // specify the nameSpace, functionName, usage, min and max arguments 
	extern void (*torque_exportstringcallback)(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  int minArgs, int maxArgs);

   // Set a TorqueScript console function as secure and available for JavaScript via the callScript plugin method
	extern void (*torque_addsecurefunction)(const char* nameSpace, const char* fname);
   // Call a TorqueScript console function that has been marked as secure
	extern const char* (*torque_callsecurefunction)(const char* nameSpace, const char* name, int argc, const char ** argv);

   // resize the Torque 3D child window to the specified width and height
	extern void (*torque_resizewindow)(int width, int height);

#ifndef WIN32   
   // On Mac, handle the parent safari window 
   extern void (*torque_setsafariwindow)(NSWindow* window, int32 x, int32 y, int32 width, int32 height);

   // On Mac, sets the executable path 
   extern void (*torque_setexecutablepath)(const char *path);
#else
   // retrieve the render windows hwnd
   extern void* (*torque_gethwnd)();

   // directly add a message to the Torque 3D event queue, bypassing the Windows event queue
   // this is useful in the case of the IE plugin, where we are hooking into an application 
   // level message, and posting to the windows queue would cause a hang
   extern void (*torque_directmessage)(unsigned int message, unsigned int wparam, unsigned int lparam);
#endif

};




#endif