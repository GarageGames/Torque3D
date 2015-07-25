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

#include "webConfig.h"
#include "webCommon.h"

#include <string>
#include <vector>

// Platform specific shared library handling

#ifdef WIN32

#pragma warning( disable : 4996)

#define TORQUE_OPEN LoadLibraryA
#define TORQUE_FUNCTION GetProcAddress
#define TORQUE_CLOSE FreeLibrary

#define strncasecmp strnicmp
#define strcasecmp stricmp

#else // Mac

#define TORQUE_OPEN(path) dlopen(path, RTLD_LAZY | RTLD_LOCAL)
#define TORQUE_FUNCTION dlsym
#define TORQUE_CLOSE dlclose

#endif

// C Interface exported from the Torque 3D DLL (or Bundle on Mac)

extern "C"
{
   // initialize Torque 3D including argument handling
   bool (*torque_engineinit)(int argc, const char **argv) = NULL;

   // tick Torque 3D's main loop
   int  (*torque_enginetick)() = NULL;

   // set Torque 3D into web deployment mode (disable fullscreen exlusive mode, etc)
   int  (*torque_setwebdeployment)() = NULL;

   // shutdown the engine
   bool (*torque_engineshutdown)() = NULL;

   // signal an engine shutdown (as with the quit(); console command)
   void (*torque_enginesignalshutdown)() = NULL;

   // reset the engine, unloading any current level and returning to the main menu
   void (*torque_reset)() = NULL;

   // Evaluate arbitrary TorqueScript (ONLY CALL torque_evaluate FROM TRUSTED CODE!!!)
   const char* (*torque_evaluate)(const char* code) = NULL;

   // Get a console variable
   const char* (*torque_getvariable)(const char* name) = NULL;
   // Set a console variable
   void (*torque_setvariable)(const char* name, const char* value) = NULL;

   // Export a function to the Torque 3D console system which matches the StringCallback function prototype
   // specify the nameSpace, functionName, usage, min and max arguments 
   void (*torque_exportstringcallback)(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  int minArgs, int maxArgs) = NULL;

   // Set a TorqueScript console function as secure and available for JavaScript via the callScript plugin method
   void (*torque_addsecurefunction)(const char* nameSpace, const char* fname) = NULL;
   // Call a TorqueScript console function that has been marked as secure
   const char* (*torque_callsecurefunction)(const char* nameSpace, const char* name, int argc, const char ** argv) = NULL;

   // resize the Torque 3D child window to the specified width and height
   void (*torque_resizewindow)(int width, int height) = NULL;

#ifndef WIN32   
   // On Mac, handle the parent safari window 
   void (*torque_setsafariwindow)(NSWindow* window, int32 x, int32 y, int32 width, int32 height) = NULL;
   // On Mac, sets the executable path 
   void (*torque_setexecutablepath)(const char *path) = NULL;
#else
   // retrieve the render windows hwnd
   void* (*torque_gethwnd)() = NULL;

   // directly add a message to the Torque 3D event queue, bypassing the Windows event queue
   // this is useful in the case of the IE plugin, where we are hooking into an application 
   // level message, and posting to the windows queue would cause a hang
   void (*torque_directmessage)(unsigned int message, unsigned int wparam, unsigned int lparam) = NULL;
#endif

};

namespace WebCommon
{

   std::string gPluginMIMEType;

#ifdef WIN32

   HMODULE gTorque3DModule = NULL;
   HMODULE gPluginModule = 0;

   // bring up a platform specific message box (used for error reporting)
   void MessageBox(void* parentWindow, const char* msg, const char* caption )
   {
      ::MessageBoxA( (HWND) parentWindow, msg, caption, MB_OK|MB_ICONWARNING);
   }

   // retrieve the game directory using the filename (which includes the full path) of the plugin DLL
   const char* GetGameDirectory()
   {
      static char dir[4096];

      GetModuleFileNameA(gPluginModule, dir, 4096);

      int i = strlen(dir) - 1;
      while (i>=0)
      {
         if (dir[i] == '\\' || dir[i] == '/')
         {
            dir[i] = 0;
            break;
         }

         i--;
      }

      return dir;

   }

   // retrieve the name of our game DLL (includes Torque 3D engine) based on naming convention
   const char* GetGameLibrary()
   {
      char dir[4096];
      static char lib[4096];

      lib[0] = 0;

      GetModuleFileNameA(gPluginModule, dir, 4096);

      int i = strlen(dir) - 1;
      while (i>=0)
      {
         if (dir[i] == '\\' || dir[i] == '/')
         {
            // copy, minus the "NP " or "IE " of plugin name
#ifdef _DEBUG				
            sprintf(lib, "%s_DEBUG.dll", &dir[i+4]);
#else
            sprintf(lib, "%s.dll", &dir[i+4]);
#endif
            return lib;
         }

         // strip off end
         if (!strncmp(&dir[i], " Plugin", 7))
            dir[i] = 0;

         i--;
      }

      return lib;
   }

#else

   void* gTorque3DModule = NULL;
   NSBundle* gPluginBundle = NULL;

   // bring up a platform specific message box (used for error reporting)
   void MessageBox(void* parentWindow, const char* msg, const char* caption )
   {

      // convert title and message to NSStrings
      NSString *nsTitle = [NSString stringWithUTF8String:caption];
      NSString *nsMessage = [NSString stringWithUTF8String:msg];


      NSAlert *alert = [NSAlert alertWithMessageText:nsTitle
defaultButton:@"OK"
alternateButton:nil
otherButton:nil
informativeTextWithFormat:nsMessage];
      [alert runModal];		
   }

   NSBundle* GetPluginBundle()
   {
      if (gPluginBundle)
         return gPluginBundle;

      NSDictionary *mimeTypes;
      NSString *mime;
      NSEnumerator *f;

      NSArray *bundles = [NSBundle allBundles]; 
      for (int i = 0; i < [bundles count]; i++) { 
         NSBundle *b = [bundles objectAtIndex:i]; 

         mimeTypes=[b objectForInfoDictionaryKey:@"WebPluginMIMETypes"];

         if (!mimeTypes)
            continue;

         f=[mimeTypes keyEnumerator];

         while((mime=[f nextObject]))
         {
            if (gPluginMIMEType == std::string([mime UTF8String]))
            {
               gPluginBundle = b;
               break;
            }
         }
      } 

      return gPluginBundle;

   }

   // retrieve the game's install folder based on entries from our plugin's Info.plist
   const char* GetGameDirectory()
   {
      static char gamePath[2048] = {'\0'};      

      if (gamePath[0])
         return gamePath;

      NSBundle* pluginBundle = GetPluginBundle();

      if (!pluginBundle)
         return NULL;

      NSString *gameInstallPathKey = [NSString stringWithUTF8String:"GameInstallPath"];
      NSString *gameInstallPath = [pluginBundle objectForInfoDictionaryKey: gameInstallPathKey];

      if (!gameInstallPath)
         return NULL;

      strcpy(gamePath, [gameInstallPath UTF8String]);

      if (!gamePath[0])
         return NULL;

      return gamePath;
   }

   // retrieve the game bundle (including Torque 3D engine) from the plugin Info.plist
   const char* GetGameLibrary()
   {
      static char libPath[2048] = {'\0'};

      if (libPath[0])
         return libPath;

      const char* gamePath = GetGameDirectory();

      if (!gamePath)
         return NULL;

      NSBundle* pluginBundle = GetPluginBundle();

      if (!pluginBundle)
         return NULL;

      // NSString* bundleIdentifier = [pluginBundle bundleIdentifier];

      NSString *gameNameKey = [NSString stringWithUTF8String:"GameName"];
      NSString *gameName = [pluginBundle objectForInfoDictionaryKey: gameNameKey];

      if (!gameName)
         return NULL;

      const char* cgameName = [gameName UTF8String]; 

      if (!cgameName[0])
         return NULL;

#ifdef DEBUG
      sprintf(libPath, "%s%s_DEBUG.app/Contents/Frameworks/%s Bundle.bundle/Contents/MacOS/%s Bundle", gamePath, cgameName, cgameName, cgameName);
#else
      sprintf(libPath, "%s%s.app/Contents/Frameworks/%s Bundle.bundle/Contents/MacOS/%s Bundle", gamePath, cgameName, cgameName, cgameName);
#endif

      return libPath;
   }

#endif 

   bool ChangeToGameDirectory()
   {
      const char* gameDir = GetGameDirectory();

      if (!gameDir)
         return false;

#ifdef WIN32
      return SetCurrentDirectoryA(gameDir);
#else
      return (chdir(gameDir) == 0);
#endif

   }

   // loads the Torque 3D shared library, sets web deployment mode, retrieves engine "C" interface
   bool InitTorque3D(void* platformWindow, int clipLeft, int clipTop, int clipRight, int clipBottom)
   {
      const char* gameDir = GetGameDirectory();
      const char* gameLib = GetGameLibrary();

      if (gTorque3DModule)
      {	
         WebCommon::MessageBox( 0, "This plugin allows only one instance", "Error");
         return false;
      }

      if (!gameDir || !gameLib)
      {
         WebCommon::MessageBox( 0, "Unable to get game plugin information", "Error");
         return false;
      }

      if (!ChangeToGameDirectory())
         return false;

      std::string gameLibStr = gameLib;
#ifdef WIN32
      // We want to use an absolute path to the game library
      gameLibStr = gameDir;
      WebCommon::ConvertToWindowsPathSep(gameLibStr);
      gameLibStr += "\\";
      gameLibStr += gameLib;
#endif

      gTorque3DModule = TORQUE_OPEN(gameLibStr.c_str());

      if (!gTorque3DModule)
      {
         char error[4096];
#ifdef WIN32
         sprintf(error, "Could not load game library: %s/%s.  Please make sure you have the latest DirectX installed.", gameDir, gameLib);
#else
         sprintf(error, "Could not load game library: %s/%s. ", gameDir, gameLib);
#endif
         WebCommon::MessageBox( 0, error, "Error");
         return false;
      }

      // snag all the exported functions of the "C" interface

      torque_engineinit = (bool (*)(int argc, const char **argv))TORQUE_FUNCTION(gTorque3DModule, "torque_engineinit");
      torque_enginetick = (int (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_enginetick");
      torque_setwebdeployment = (int (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_setwebdeployment");
      torque_engineshutdown = (bool (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_engineshutdown");
      torque_enginesignalshutdown = (void (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_enginesignalshutdown");
      torque_reset = (void (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_reset");
      torque_evaluate = (const char* (*)(const char* code))TORQUE_FUNCTION(gTorque3DModule, "torque_evaluate");

      torque_getvariable = (const char* (*)(const char* name))TORQUE_FUNCTION(gTorque3DModule, "torque_getvariable");
      torque_setvariable = (void (*)(const char* name, const char* value))TORQUE_FUNCTION(gTorque3DModule, "torque_setvariable");
      torque_exportstringcallback = (void (*)(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  int minArgs, int maxArgs))TORQUE_FUNCTION(gTorque3DModule, "torque_exportstringcallback");

      torque_addsecurefunction = (void (*)(const char* nameSpace, const char* name))TORQUE_FUNCTION(gTorque3DModule, "torque_addsecurefunction");
      torque_callsecurefunction = (const char* (*)(const char* nameSpace, const char* name, int argc, const char ** argv))TORQUE_FUNCTION(gTorque3DModule, "torque_callsecurefunction");

      torque_resizewindow = (void (*)(int width, int height))TORQUE_FUNCTION(gTorque3DModule, "torque_resizewindow");

      // check that we got them all

      if (!torque_engineinit ||
         !torque_enginetick ||
         !torque_setwebdeployment ||
         !torque_engineshutdown ||
         !torque_enginesignalshutdown ||
         !torque_reset ||
         !torque_evaluate ||
         !torque_getvariable ||
         !torque_setvariable ||
         !torque_exportstringcallback ||
         !torque_addsecurefunction ||
         !torque_callsecurefunction ||
         !torque_resizewindow )
      {
         WebCommon::MessageBox( platformWindow, "The plugin could not be initialized (missing function exports)", "Error");
         TORQUE_CLOSE(gTorque3DModule);
         gTorque3DModule = NULL;
         return false;
      }

#ifndef WIN32
      torque_setexecutablepath = (void (*)(const char *path)) dlsym(gTorque3DModule, "torque_setexecutablepath");
      torque_setsafariwindow = (void (*)(NSWindow* nswnd, int32, int32, int32, int32)) dlsym(gTorque3DModule, "torque_setsafariwindow");

      if (!torque_setexecutablepath || !torque_setsafariwindow)
      {
         WebCommon::MessageBox( platformWindow, "The plugin could not be initialized (missing function exports)", "Error");
         TORQUE_CLOSE(gTorque3DModule);
         gTorque3DModule = NULL;
         return false;
      }
#else
      torque_gethwnd = (void* (*)())TORQUE_FUNCTION(gTorque3DModule, "torque_gethwnd");
      torque_directmessage = (void (*)(unsigned int message, unsigned int wparam, unsigned int lparam))TORQUE_FUNCTION(gTorque3DModule, "torque_directmessage");
      if (!torque_gethwnd || !torque_directmessage)
      {
         WebCommon::MessageBox( platformWindow, "The plugin could not be initialized (missing function exports)", "Error");
         TORQUE_CLOSE(gTorque3DModule);
         gTorque3DModule = NULL;
         return false;
      }
#endif

      //tell Torque3D that we're a browser plugin
      torque_setwebdeployment();

      const char* args[3];
      int argc;

#ifdef WIN32
      // windows uses a command line arg for parent window
      char parentWindow[256];
      argc = 3;
      sprintf(parentWindow, "%I64u", (unsigned __int64)platformWindow);
      args[0] = "game.exe"; //just to satisfy command line parsing
      args[1] = "-window";
      args[2] = parentWindow;
#else

      NSWindow* browserWindow = (NSWindow*) platformWindow;   

      // tell Torque 3D about our parent browser window
      // we initialize with zero size as the page hasn't completely loaded yet
      // so, the plugin hasn't been resized by the page and it is better to not show 
      // anything than wrong extents
      torque_setsafariwindow( browserWindow, 0, 0, 0, 0);

      argc = 1;
      args[0] = gameDir; // just to satisfy command line parsing

#endif

      // initialize Torque 3D!
      if (!torque_engineinit(argc, args))
      {
         WebCommon::MessageBox( platformWindow, "The plugin could not be initialized (internal initialization error)", "Error");
         return false;
      }

      return true;
   }

   // unloads the Torque 3D shared library (first signaling a shutdown for clean exit)
   void ShutdownTorque3D()
   {
      if (gTorque3DModule)
      {
         ChangeToGameDirectory();
         torque_enginesignalshutdown();
         torque_enginetick();
         torque_engineshutdown();
         TORQUE_CLOSE(gTorque3DModule);
      }

      gTorque3DModule = NULL;

   }

   // checks a given domain against the allowed domains in webConfig.h
   bool CheckDomain(const char* url)
   {
      bool domainCheck = true;

#ifndef WEBDEPLOY_DOMAIN_CHECK
      domainCheck = false;
#endif

#ifdef DEBUG 
#  ifdef WEBDEPLOY_DOMAIN_ALLOW_DEBUG
      domainCheck = false;
#  endif
#endif

      if (!domainCheck)
         return true; // gets rid of "unreachable code" warning

      if (strlen(url) > 512)
         return false;

      if (strlen(url) < 5)
         return false;

      //do not allow file when using domain checking
      if (!strncasecmp(url,"file",4))
         return false;

      char curl[512] = {0};

      unsigned int begin = 0;
      while(url[begin])
      {
         if (url[begin] == ':')
         {
            if (begin + 3 > strlen(url))
               return false;
            begin+=3; //skip ://
            break;
         }
         begin++;
      }

      unsigned int end = begin;

      while(end < strlen(url))
      {
         if (url[end] == '/')
         {
            break;
         }

         end++;
      }

      strcpy(curl, &url[begin]);
      curl[end-begin] = 0;

      // iterate checking against our allowed domains
      for (int i = 0; gAllowedDomains[i]; i++)
         if (!strcasecmp(curl, gAllowedDomains[i]))
            return true;

      WebCommon::MessageBox( 0 , "This plugin cannot be executed from the domain specified", "Error");

      return false;
   }

   // exposes TorqueScript functions marked as secure in webConfig.h, these functions can then be called on the page via Javascript
   void AddSecureFunctions()
   {
      char snamespace[256];
      char fname[256];

      //define secure functions here
      for (unsigned int i = 0; gSecureScript[i]; i++)
      {
         snamespace[0] = 0;
         strcpy(fname, gSecureScript[i]);

         //scan through looking for namespace
         for (unsigned int j = 1; j < strlen(fname)-2; j++)
         {
            if (fname[j] == ':' && fname[j+1] == ':')
            {
               strcpy(snamespace, gSecureScript[i]);
               snamespace[j] = 0;
               strcpy(fname,&gSecureScript[i][j+2]);
               break;

            }
         }

         torque_addsecurefunction(snamespace, fname);

      }

   }

   //simple string copy that eats white space
   void StringCopy(char* dst, const char* src, int count)
   {
      int i, j;
      bool eat = true;
      for (i = 0, j = 0; i < count ; i++)
      {
         if (src[i] == 0)
         {
            dst[j] = 0;
            return;
         }
         if (src[i] != '"' && src[i] != '\t' && src[i] != '\n' && src[i] != ')' && src[i] != '(')
         {
            if (eat && src[i] == ' ')
               continue;

            if (src[i] == '\'') 
            {
               eat = !eat;
               continue;
            }

            dst[j++] = src[i];
         }
      }
   }

   void SetVariable(const char* variable, const char* value)
   {
      char mvar[1024];
      char mvar2[1024];

      if (strlen(variable) > 1023)
      {
         WebCommon::MessageBox( 0, "WebCommon::SetVariable - buffer overrun", "Error");
         return;
      }

      // make local copies stripping off $ decorator is needed
      if (variable[0] == '$')
         strcpy(mvar, &variable[1]);
      else
         strcpy(mvar, variable);

      const char* js = "javascript::";

      if (strncasecmp(js, mvar, 12))
         sprintf(mvar2, "Javascript::%s", mvar);
      else
         strcpy(mvar2, mvar);

      torque_setvariable(mvar2, value);
   }

   const char* GetVariable(const char* variable)
   {
      char mvar[1024];
      char mvar2[1024];

      if (strlen(variable) > 1023)
      {
         WebCommon::MessageBox( 0, "WebCommon::GetVariable - buffer overrun", "Error");
         return "0";
      }

      // make local copies stripping off $ decorator is needed
      if (variable[0] == '$')
         strcpy(mvar, &variable[1]);
      else
         strcpy(mvar, variable);

      const char* js = "javascript::";

      if (strncasecmp(js, mvar, 12))
         sprintf(mvar2, "Javascript::%s", mvar);
      else
         strcpy(mvar2, mvar);

      return torque_getvariable(mvar2);

   }


#ifdef WIN32

   // string conversion to/from wstring and string
   std::wstring StringToWString( const std::string& str )
   {
      size_t size = str.length();
      wchar_t* w = new wchar_t[size+1];

      memset( w, 0, sizeof(wchar_t) * (size+1) );

      MultiByteToWideChar( CP_ACP,
         0,
         str.c_str(),
         size,
         w,
         size );

      std::wstring ws(w);
      delete[] w;
      return ws;
   }

   std::string WStringToString( const std::wstring& wstr )
   {
      size_t size = wstr.length();
      char* s = new char[size+1];

      memset( s, 0, sizeof(char) * (size+1) );

      WideCharToMultiByte( CP_ACP,
         0,
         wstr.c_str(),
         size,
         s,
         size,
         NULL,
         NULL );

      std::string str(s);
      delete[] s;
      return str;
   }

   void ConvertToWindowsPathSep(std::string& path)
   {
      size_t pos = 0;
      while(pos < path.size() || pos != std::string::npos)
      {
         pos = path.find("/", pos);
         if(pos != std::string::npos)
         {
            path.replace(pos, 1, "\\");
            ++pos;
         }
      }
   }

#endif

} //namespace WebCommon