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

#ifndef	__WIN_PLATFORM_STACKWALKER__
#define	__WIN_PLATFORM_STACKWALKER__

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )

#include <windows.h>
#include <dbghelp.h>

#include "platform/types.h"

class StackWalker
{
public:

   typedef enum StackWalkOptions
   {
      //		RetrieveNone = 0,				// No additional info will be retrieved (only the address is available)
      //		RetrieveSymbol = 1,			// Try to get the symbol-name
      //		RetrieveLine = 2,				// Try to get the line for this symbol
      //		RetrieveModuleInfo = 4,		// Try to retrieve the module-infos
      RetrieveFileVersion = 8,	// Also retrieve the version for the DLL/EXE
      RetrieveVerbose = 0xF,		// Contains all of the above retrieve options

      SymBuildPath = 0x10,			// Generate a "good" symbol-search-path
      SymUseSymSrv = 0x20,			// Also use a public Symbol Server
      SymAll = 0x30,					// Contains all of the above Symbol options

      OutputSymPath = 0x80,		//print out the symbol path
      OutputOS = 0x100,				//print out the OS path
      OutputModules = 0x200,		//print out the Modules

      OptionsDefault = 0x3F,		// Less verbose output (default)
      OptionsAll = 0x2FF			// Contains all options
   };

   StackWalker(DWORD optionFlags = OptionsDefault, LPCSTR szSymPath = NULL);
   virtual ~StackWalker();

   typedef BOOL (__stdcall *PReadProcessMemoryRoutine)(HANDLE      hProcess,
      DWORD64     qwBaseAddress,
      PVOID       lpBuffer,
      DWORD       nSize,
      LPDWORD     lpNumberOfBytesRead,
      LPVOID      pUserData  // optional data, which was passed in "ShowCallstack"
      );

   // pUserData is optional to identify some data in the 'readMemoryFunction'-callback
   bool ShowCallstack(HANDLE hThread,  CONTEXT const & Context, PReadProcessMemoryRoutine readMemoryFunction = NULL, LPVOID pUserData = NULL);

   void setOutputBuffer(char * buffer);

private:
   typedef enum CallstackEntryType {firstEntry, nextEntry, lastEntry};

   HANDLE m_hProcess;
   DWORD m_dwProcessId;
   bool m_modulesLoaded;
   LPSTR m_szSymPath;
   S32 m_options;
   char * m_pOutputBuffer;

   static BOOL __stdcall myReadProcMem(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);

   bool Init(LPCSTR szSymPath);

   virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);
   virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion);
   virtual void OnCallstackEntry(CallstackEntryType eType, struct CallstackEntry &entry);
   virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
   virtual void OnOutput(LPCSTR szText);

   bool LoadModules();
   DWORD LoadModule(HANDLE hProcess, LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size);
   bool GetModuleListTH32(HANDLE hProcess, DWORD pid);
   bool GetModuleListPSAPI(HANDLE hProcess);
   bool GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64 *pModuleInfo);
};

void dGetStackTrace(char * traceBuffer, CONTEXT const & ContextRecord);

#endif
#endif
