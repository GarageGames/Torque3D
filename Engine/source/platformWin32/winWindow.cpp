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
#include "platformWin32/platformWin32.h"
#include "platformWin32/winConsole.h"
#include "platformWin32/winDirectInput.h"
#include "windowManager/win32/win32Window.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "math/mRandom.h"
#include "core/stream/fileStream.h"
#include "T3D/resource.h"
#include <d3d9.h>
#include "gfx/gfxInit.h"
#include "gfx/gfxDevice.h"
#include "core/strings/unicode.h"
#include "gui/core/guiCanvas.h"


extern void createFontInit();
extern void createFontShutdown();
extern void installRedBookDevices();
extern void handleRedBookCallback(U32, U32);

static MRandomLCG sgPlatRandom;
static bool sgQueueEvents;

// is keyboard input a standard (non-changing) VK keycode
#define dIsStandardVK(c) (((0x08 <= (c)) && ((c) <= 0x12)) || \
                          ((c) == 0x1b) ||                    \
                          ((0x20 <= (c)) && ((c) <= 0x2e)) || \
                          ((0x30 <= (c)) && ((c) <= 0x39)) || \
                          ((0x41 <= (c)) && ((c) <= 0x5a)) || \
                          ((0x70 <= (c)) && ((c) <= 0x7B)))

extern InputObjectInstances DIK_to_Key( U8 dikCode );

// static helper variables
static HANDLE gMutexHandle = NULL;
static bool sgDoubleByteEnabled = false;

// track window states
Win32PlatState winState;


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Microsoft Layer for Unicode
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/mslu/winprog/compiling_your_application_with_the_microsoft_layer_for_unicode.asp
//
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef UNICODE

HMODULE LoadUnicowsProc(void)
{
    return(LoadLibraryA("unicows.dll"));
}

#ifdef _cplusplus
extern "C" {
#endif
extern FARPROC _PfnLoadUnicows = (FARPROC) &LoadUnicowsProc;
#ifdef _cplusplus
}
#endif

#endif

//--------------------------------------
Win32PlatState::Win32PlatState()
{
   log_fp      = NULL;
   hinstOpenGL = NULL;
   hinstGLU    = NULL;
   hinstOpenAL = NULL;
   appDC       = NULL;
   appInstance = NULL;
   currentTime = 0;
   processId   = 0;
}

//--------------------------------------
bool Platform::excludeOtherInstances(const char *mutexName)
{
#ifdef UNICODE
   UTF16 b[512];
   convertUTF8toUTF16((UTF8 *)mutexName, b);
   gMutexHandle = CreateMutex(NULL, true, b);
#else
   gMutexHandle = CreateMutex(NULL, true, mutexName);
#endif
   if(!gMutexHandle)
      return false;

   if(GetLastError() == ERROR_ALREADY_EXISTS)
   {
      CloseHandle(gMutexHandle);
      gMutexHandle = NULL;
      return false;
   }

   return true;
}

void Platform::restartInstance()
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   TCHAR cen_buf[2048];
   GetModuleFileName( NULL, cen_buf, 2047);

   // Start the child process. 
   if( CreateProcess( cen_buf,
      NULL,            // Command line
      NULL,           // Process handle not inheritable
      NULL,           // Thread handle not inheritable
      FALSE,          // Set handle inheritance to FALSE
      0,              // No creation flags
      NULL,           // Use parent's environment block
      NULL,           // Use parent's starting directory 
      &si,            // Pointer to STARTUPINFO structure
      &pi )           // Pointer to PROCESS_INFORMATION structure
      != false )
   {
      WaitForInputIdle( pi.hProcess, 5000 );
      CloseHandle( pi.hProcess );
      CloseHandle( pi.hThread );
   }
}

///just check if the app's global mutex exists, and if so, 
///return true - otherwise, false. Should be called before ExcludeOther 
/// at very start of app execution.
bool Platform::checkOtherInstances(const char *mutexName)
{
#ifdef TORQUE_MULTITHREAD

	HANDLE pMutex	=	NULL;
   
#ifdef UNICODE
   UTF16 b[512];
   convertUTF8toUTF16((UTF8 *)mutexName, b);
   pMutex  = CreateMutex(NULL, true, b);
#else
   pMutex = CreateMutex(NULL, true, mutexName);
#endif

   if(!pMutex)
      return false;

   if(GetLastError() == ERROR_ALREADY_EXISTS)
   {
	   //another mutex of the same name exists
	   //close ours
      CloseHandle(pMutex);
      pMutex = NULL;
      return true;
   }

   CloseHandle(pMutex);
   pMutex = NULL;
#endif

   //we don;t care, always false
   return false;
}

#ifndef TORQUE_SDL
//--------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
   ShowCursor(true);
#ifdef UNICODE
   UTF16 m[1024], t[512];
   convertUTF8toUTF16((UTF8 *)windowTitle, t);
   convertUTF8toUTF16((UTF8 *)message, m);
   MessageBox(NULL, m, t, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
#else
   MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
#endif
}

//--------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
   ShowCursor(true);
#ifdef UNICODE
   UTF16 m[1024], t[512];
   convertUTF8toUTF16((UTF8 *)windowTitle, t);
   convertUTF8toUTF16((UTF8 *)message, m);
   return MessageBox(NULL, m, t, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
#else
   return MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
#endif
}

//--------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
   ShowCursor(true);
#ifdef UNICODE
   UTF16 m[1024], t[512];
   convertUTF8toUTF16((UTF8 *)windowTitle, t);
   convertUTF8toUTF16((UTF8 *)message, m);
   return (MessageBox(NULL, m, t, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
#else
   return (MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
#endif
}

Platform::ALERT_ASSERT_RESULT Platform::AlertAssert(const char *windowTitle, const char *message)
{
#ifndef TORQUE_TOOLS
   ShowCursor(true);
#endif // TORQUE_TOOLS

#ifdef UNICODE
   UTF16 messageUTF[1024], title[512];
   convertUTF8toUTF16((UTF8 *)windowTitle, title);
   convertUTF8toUTF16((UTF8 *)message, messageUTF);
#else
   const char* messageUTF = message;
   const char* title = windowTitle;
#endif

   // TODO: Change this to a custom dialog that has Exit, Ignore, Ignore All, and Debug buttons
   ALERT_ASSERT_RESULT alertResult = ALERT_ASSERT_DEBUG;
   int result = MessageBox(winState.appWindow, messageUTF, title, MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL | MB_SETFOREGROUND);
   switch( result )
   {
		case IDABORT:
			alertResult = ALERT_ASSERT_EXIT;
			break;
		case IDIGNORE:
			alertResult = ALERT_ASSERT_IGNORE;
			break;
		default:
		case IDRETRY:
			alertResult = ALERT_ASSERT_DEBUG;
			break;
   }

   return alertResult;
}

#endif

//--------------------------------------
HIMC gIMEContext;

static void InitInput()
{
#ifndef TORQUE_LIB
#ifdef UNICODE
   //gIMEContext = ImmGetContext(getWin32WindowHandle());
   //ImmReleaseContext( getWin32WindowHandle(), gIMEContext );
#endif
#endif
}

//--------------------------------------
void Platform::init()
{
   Con::printf("Initializing platform...");

   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "windows" );

   WinConsole::create();

   if ( !WinConsole::isEnabled() )
      Input::init();

   InitInput();   // in case DirectInput falls through

   installRedBookDevices();

   sgDoubleByteEnabled = GetSystemMetrics( SM_DBCSENABLED );
   sgQueueEvents = true;
   Con::printf("Done");
}

//--------------------------------------
void Platform::shutdown()
{
	sgQueueEvents = false;

   if(gMutexHandle)
      CloseHandle(gMutexHandle);

   Input::destroy();
   
   GFXDevice::destroy();

   WinConsole::destroy();
}

extern bool LinkConsoleFunctions;

#ifndef TORQUE_SHARED

extern S32 TorqueMain(S32 argc, const char **argv);

//--------------------------------------
static S32 run(S32 argc, const char **argv)
{
   // Console hack to ensure consolefunctions get linked in
   LinkConsoleFunctions=true;

   createFontInit();

   S32 ret = TorqueMain(argc, argv);

   createFontShutdown();

   return ret;
}

//--------------------------------------
S32 main(S32 argc, const char **argv)
{
   winState.appInstance = GetModuleHandle(NULL);
   return run(argc, argv);
}

//--------------------------------------

#include "app/mainLoop.h"

S32 WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, S32)
{
   Vector<char *> argv( __FILE__, __LINE__ );

   enum { moduleNameSize = 256 };
   char moduleName[moduleNameSize];
#ifdef TORQUE_UNICODE
   {
      TCHAR buf[ moduleNameSize ];
      GetModuleFileNameW( NULL, buf, moduleNameSize );
      convertUTF16toUTF8( buf, moduleName );
   }
#else
   GetModuleFileNameA(NULL, moduleName, moduleNameSize);
#endif
   argv.push_back(moduleName);

   for (const char* word,*ptr = lpszCmdLine; *ptr; )
   {
      // Eat white space
      for (; dIsspace(*ptr) && *ptr; ptr++)
         ;
      
      // Pick out the next word
      for (word = ptr; !dIsspace(*ptr) && *ptr; ptr++)
         ;
      
      // Add the word to the argument list.
      if (*word) 
      {
         S32 len = ptr - word;
         char *arg = (char *) dMalloc(len + 1);
         dStrncpy(arg, word, len);
         arg[len] = 0;
         argv.push_back(arg);
      }
   }

   winState.appInstance = hInstance;

   S32 retVal = run(argv.size(), (const char **) argv.address());

   for(U32 j = 1; j < argv.size(); j++)
      dFree(argv[j]);

   return retVal;
}

#else //TORQUE_SHARED

extern "C"
{
	bool torque_engineinit(S32 argc, const char **argv);
	S32  torque_enginetick();
	S32  torque_getreturnstatus();
	bool torque_engineshutdown();
};

S32 TorqueMain(int argc, const char **argv)
{
	if (!torque_engineinit(argc, argv))
		return 1;

	while(torque_enginetick())
	{

	}

	torque_engineshutdown();

	return torque_getreturnstatus();

}



extern "C" {

S32 torque_winmain( HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, S32)
{
	Vector<char *> argv( __FILE__, __LINE__ );

   enum { moduleNameSize = 256 };
   char moduleName[moduleNameSize];
#ifdef TORQUE_UNICODE
   {
      TCHAR buf[ moduleNameSize ];
      GetModuleFileNameW( NULL, buf, moduleNameSize );
      convertUTF16toUTF8( buf, moduleName );
   }
#else
   GetModuleFileNameA(NULL, moduleName, moduleNameSize);
#endif
	argv.push_back(moduleName);

	for (const char* word,*ptr = lpszCmdLine; *ptr; )
	{
		// Eat white space
		for (; dIsspace(*ptr) && *ptr; ptr++)
			;

      // Test for quotes
      bool withinQuotes = dIsquote(*ptr);

      if (!withinQuotes)
      {
		   // Pick out the next word
		   for (word = ptr; !dIsspace(*ptr) && *ptr; ptr++)
			   ;
      }
      else
      {
         // Advance past the first quote.  We don't want to include it.
         ptr++;

		   // Pick out the next quote
		   for (word = ptr; !dIsquote(*ptr) && *ptr; ptr++)
			   ;
      }

		// Add the word to the argument list.
		if (*word) 
		{
			S32 len = ptr - word;
			char *arg = (char *) dMalloc(len + 1);
			dStrncpy(arg, word, len);
			arg[len] = 0;
			argv.push_back(arg);
		}

      // If we had a quote, skip past it for the next arg
      if (withinQuotes && *ptr)
      {
         ptr++;
      }
	}

	winState.appInstance = hInstance;

	S32 retVal = TorqueMain(argv.size(), (const char **) argv.address());

	for(U32 j = 1; j < argv.size(); j++)
		dFree(argv[j]);

	return retVal;
}

} // extern "C"

#endif



//--------------------------------------

F32 Platform::getRandom()
{
   return sgPlatRandom.randF();
}

////--------------------------------------
/// Spawn the default Operating System web browser with a URL
/// @param webAddress URL to pass to browser
/// @return true if browser successfully spawned
bool Platform::openWebBrowser( const char* webAddress )
{
   static bool sHaveKey = false;
   static wchar_t sWebKey[512];
   char utf8WebKey[512];

   {
      HKEY regKey;
      DWORD size = sizeof( sWebKey );

      if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, dT("\\http\\shell\\open\\command"), 0, KEY_QUERY_VALUE, &regKey ) != ERROR_SUCCESS )
      {
         Con::errorf( ConsoleLogEntry::General, "Platform::openWebBrowser - Failed to open the HKCR\\http registry key!!!");
         return( false );
      }

      if ( RegQueryValueEx( regKey, dT(""), NULL, NULL, (U8 *)sWebKey, &size ) != ERROR_SUCCESS ) 
      {
         Con::errorf( ConsoleLogEntry::General, "Platform::openWebBrowser - Failed to query the open command registry key!!!" );
         return( false );
      }

      RegCloseKey( regKey );
      sHaveKey = true;

      convertUTF16toUTF8(sWebKey,utf8WebKey);

#ifdef UNICODE
      char *p = dStrstr((const char *)utf8WebKey, "%1"); 
#else
      char *p = strstr( (const char *) sWebKey  , "%1"); 
#endif
      if (p) *p = 0; 

   }

   STARTUPINFO si;
   dMemset( &si, 0, sizeof( si ) );
   si.cb = sizeof( si );

   char buf[1024];
#ifdef UNICODE
   dSprintf( buf, sizeof( buf ), "%s %s", utf8WebKey, webAddress );   
   UTF16 b[1024];
   convertUTF8toUTF16((UTF8 *)buf, b);
#else
   dSprintf( buf, sizeof( buf ), "%s %s", sWebKey, webAddress );   
#endif

   //Con::errorf( ConsoleLogEntry::General, "** Web browser command = %s **", buf );

   PROCESS_INFORMATION pi;
   dMemset( &pi, 0, sizeof( pi ) );
   CreateProcess( NULL,
#ifdef UNICODE
      b,
#else
      buf, 
#endif
      NULL,
      NULL,
      false,
      CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
      NULL,
      NULL,
      &si,
      &pi );

   return( true );
}

//--------------------------------------
// Login password routines:
//--------------------------------------
#ifdef UNICODE
static const UTF16* TorqueRegKey = dT("SOFTWARE\\GarageGames\\Torque");
#else
static const char* TorqueRegKey = "SOFTWARE\\GarageGames\\Torque";
#endif

const char* Platform::getLoginPassword()
{
   HKEY regKey;
   char* returnString = NULL;
   if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TorqueRegKey, 0, KEY_QUERY_VALUE, &regKey ) == ERROR_SUCCESS )
   {
      U8 buf[32];
      DWORD size = sizeof( buf );
      if ( RegQueryValueEx( regKey, dT("LoginPassword"), NULL, NULL, buf, &size ) == ERROR_SUCCESS )
      {
         returnString = Con::getReturnBuffer( size + 1 );
         dStrcpy( returnString, (const char*) buf );
      }

      RegCloseKey( regKey );
   }

   if ( returnString )
      return( returnString );
   else
      return( "" );
}

//--------------------------------------
bool Platform::setLoginPassword( const char* password )
{
   HKEY regKey;
   if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TorqueRegKey, 0, KEY_WRITE, &regKey ) == ERROR_SUCCESS )
   {
      if ( RegSetValueEx( regKey, dT("LoginPassword"), 0, REG_SZ, (const U8*) password, dStrlen( password ) + 1 ) != ERROR_SUCCESS )
         Con::errorf( ConsoleLogEntry::General, "setLoginPassword - Failed to set the subkey value!" );

      RegCloseKey( regKey );
      return( true );
   }
   else
      Con::errorf( ConsoleLogEntry::General, "setLoginPassword - Failed to open the Torque registry key!" );

   return( false );
}

//--------------------------------------
// Silly Korean registry key checker:
//
// NOTE: "Silly" refers to the nature of this hack, and is not intended
//       as commentary on Koreans as a nationality. Thank you for your
//       attention.
//--------------------------------------
DefineConsoleFunction( isKoreanBuild, bool, ( ), , "isKoreanBuild()")
{
   HKEY regKey;
   bool result = false;
   if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TorqueRegKey, 0, KEY_QUERY_VALUE, &regKey ) == ERROR_SUCCESS )
   {
      DWORD val;
      DWORD size = sizeof( val );
      if ( RegQueryValueEx( regKey, dT("Korean"), NULL, NULL, (U8*) &val, &size ) == ERROR_SUCCESS )
         result = ( val > 0 );

      RegCloseKey( regKey );
   }

   return( result );
}
