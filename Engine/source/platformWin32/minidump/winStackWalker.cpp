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

#if defined( TORQUE_MINIDUMP ) && defined( TORQUE_RELEASE )

//-----------------------------------------------------------------------------------------------------------------------------------------
// Sourced from http://www.codeproject.com/threads/StackWalker.asp
//-----------------------------------------------------------------------------------------------------------------------------------------
#include "winStackWalker.h"

#undef UNICODE

#include <tchar.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <psapi.h>

#pragma comment(lib, "version.lib")  // for "VerQueryValue"
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")


// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define strcpy_s strcpy
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif

// Entry for each Callstack-Entry
const S32 STACKWALK_MAX_NAMELEN = 1024; // max name length for symbols
struct CallstackEntry
{
   DWORD64 offset;  // if 0, we have no valid entry
   char name[STACKWALK_MAX_NAMELEN];
   char undName[STACKWALK_MAX_NAMELEN];
   char undFullName[STACKWALK_MAX_NAMELEN];
   DWORD64 offsetFromSmybol;
   DWORD64 offsetFromLine;
   DWORD lineNumber;
   char lineFileName[STACKWALK_MAX_NAMELEN];
   DWORD symType;
   const char * symTypeString;
   char moduleName[STACKWALK_MAX_NAMELEN];
   DWORD64 baseOfImage;
   char loadedImageName[STACKWALK_MAX_NAMELEN];
};


//-----------------------------------------------------------------------------------------------------------------------------------------
// Implementation of platform functions
//-----------------------------------------------------------------------------------------------------------------------------------------
void dGetStackTrace(char * traceBuffer, CONTEXT const & ContextRecord)
{
   StackWalker sw;
   sw.setOutputBuffer(traceBuffer);
   sw.ShowCallstack(GetCurrentThread(), ContextRecord);
   sw.setOutputBuffer(NULL);
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//Constructor
//-----------------------------------------------------------------------------------------------------------------------------------------
StackWalker::StackWalker(DWORD options, LPCSTR szSymPath)
: m_dwProcessId(GetCurrentProcessId()),
m_hProcess(GetCurrentProcess()),
m_options(options),
m_modulesLoaded(false),
m_pOutputBuffer(NULL)
{
   if (szSymPath != NULL)
   {
      m_szSymPath = _strdup(szSymPath);
      m_options |= SymBuildPath;
   }
   else
      m_szSymPath = NULL;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//Destructor
//-----------------------------------------------------------------------------------------------------------------------------------------
StackWalker::~StackWalker()
{
   SymCleanup(m_hProcess);
   if (m_szSymPath != NULL) free(m_szSymPath);
   m_szSymPath = NULL;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//setOutputBuffer - Points the StackWalker at a buffer where it will write out any output.  If this is set to NULL then the output
//                  will only be sent to DebugString
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::setOutputBuffer(char * buffer)
{
   m_pOutputBuffer = buffer;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//OnOutput
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::OnOutput(LPCSTR buffer)
{
   OutputDebugStringA(buffer);
   if(m_pOutputBuffer) strcat(m_pOutputBuffer, buffer);
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//Init
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::Init(LPCSTR szSymPath)
{
   // SymInitialize
   if (szSymPath != NULL) m_szSymPath = _strdup(szSymPath);
   if (SymInitialize(m_hProcess, m_szSymPath, FALSE) == FALSE)
   {
      this->OnDbgHelpErr("SymInitialize", GetLastError(), 0);
   }

   DWORD symOptions = SymGetOptions();
   symOptions |= SYMOPT_LOAD_LINES;
   symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
   symOptions = SymSetOptions(symOptions);

   char buf[STACKWALK_MAX_NAMELEN] = {0};
   if(SymGetSearchPath(m_hProcess, buf, STACKWALK_MAX_NAMELEN) == FALSE)
   {
      this->OnDbgHelpErr("SymGetSearchPath", GetLastError(), 0);
   }

   char szUserName[1024] = {0};
   DWORD dwSize = 1024;
   GetUserNameA(szUserName, &dwSize);
   this->OnSymInit(buf, symOptions, szUserName);

   return TRUE;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//LoadModules
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::LoadModules()
{
   if (m_modulesLoaded) return true;

   // Build the sym-path:
   char *szSymPath = NULL;
   if ( (m_options & SymBuildPath) != 0)
   {
      const size_t nSymPathLen = 4096;
      szSymPath = (char*) malloc(nSymPathLen);
      if (szSymPath == NULL)
      {
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
         return false;
      }
      szSymPath[0] = 0;
      // Now first add the (optional) provided sympath:
      if (m_szSymPath != NULL)
      {
         strcat_s(szSymPath, nSymPathLen, m_szSymPath);
         strcat_s(szSymPath, nSymPathLen, ";");
      }

      strcat_s(szSymPath, nSymPathLen, ".;");

      const size_t nTempLen = 1024;
      CHAR szTemp[nTempLen];
      // Now add the current directory:
      if (GetCurrentDirectoryA(nTempLen, szTemp) > 0)
      {
         szTemp[nTempLen-1] = 0;
         strcat_s(szSymPath, nSymPathLen, szTemp);
         strcat_s(szSymPath, nSymPathLen, ";");
      }

      // Now add the path for the main-module:
      if (GetModuleFileNameA(NULL, szTemp, nTempLen) > 0)
      {
         szTemp[nTempLen-1] = 0;
         for (char *p = (szTemp+strlen(szTemp)-1); p >= szTemp; --p)
         {
            // locate the rightmost path separator
            if ( (*p == '\\') || (*p == '/') || (*p == ':') )
            {
               *p = 0;
               break;
            }
         }  // for (search for path separator...)
         if (strlen(szTemp) > 0)
         {
            strcat_s(szSymPath, nSymPathLen, szTemp);
            strcat_s(szSymPath, nSymPathLen, ";");
         }
      }
      if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", szTemp, nTempLen) > 0)
      {
         szTemp[nTempLen-1] = 0;
         strcat_s(szSymPath, nSymPathLen, szTemp);
         strcat_s(szSymPath, nSymPathLen, ";");
      }
      if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", szTemp, nTempLen) > 0)
      {
         szTemp[nTempLen-1] = 0;
         strcat_s(szSymPath, nSymPathLen, szTemp);
         strcat_s(szSymPath, nSymPathLen, ";");
      }
      if (GetEnvironmentVariableA("SYSTEMROOT", szTemp, nTempLen) > 0)
      {
         szTemp[nTempLen-1] = 0;
         strcat_s(szSymPath, nSymPathLen, szTemp);
         strcat_s(szSymPath, nSymPathLen, ";");
         // also add the "system32"-directory:
         strcat_s(szTemp, nTempLen, "\\system32");
         strcat_s(szSymPath, nSymPathLen, szTemp);
         strcat_s(szSymPath, nSymPathLen, ";");
      }

      if ( (m_options & SymBuildPath) != 0 )
      {
         if (GetEnvironmentVariableA("SYSTEMDRIVE", szTemp, nTempLen) > 0)
         {
            szTemp[nTempLen-1] = 0;
            strcat_s(szSymPath, nSymPathLen, "SRV*");
            strcat_s(szSymPath, nSymPathLen, szTemp);
            strcat_s(szSymPath, nSymPathLen, "\\websymbols");
            strcat_s(szSymPath, nSymPathLen, "*http://msdl.microsoft.com/download/symbols;");
         }
         else
            strcat_s(szSymPath, nSymPathLen, "SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;");
      }
   }

   // First Init the whole stuff...
   bool bRet = Init(szSymPath);
   if (szSymPath != NULL) 
   {
      free(szSymPath); 
      szSymPath = NULL;
   }
   if (bRet == false)
   {
      this->OnDbgHelpErr("Error while initializing dbghelp.dll", 0, 0);
      SetLastError(ERROR_DLL_INIT_FAILED);
      return false;
   }

   if(GetModuleListTH32(m_hProcess, m_dwProcessId)) 
   {
      m_modulesLoaded = true;
      return true;
   }

   // then try psapi
   if(GetModuleListPSAPI(m_hProcess))
   {
      m_modulesLoaded = true;
      return true;
   }

   return false;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//LoadModule
//-----------------------------------------------------------------------------------------------------------------------------------------
DWORD StackWalker::LoadModule(HANDLE hProcess, LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size)
{
   CHAR *szImg = _strdup(img);
   CHAR *szMod = _strdup(mod);
   DWORD result = ERROR_SUCCESS;
   if ( (szImg == NULL) || (szMod == NULL) )
      result = ERROR_NOT_ENOUGH_MEMORY;
   else
   {
      if (SymLoadModule64(hProcess, 0, szImg, szMod, baseAddr, size) == 0)
         result = GetLastError();
   }
   ULONGLONG fileVersion = 0;
   if(szImg != NULL)
   {
      // try to retrieve the file-version:
      if ( (m_options & StackWalker::RetrieveFileVersion) != 0)
      {
         VS_FIXEDFILEINFO *fInfo = NULL;
         DWORD dwHandle;
         DWORD dwSize = GetFileVersionInfoSizeA(szImg, &dwHandle);
         if (dwSize > 0)
         {
            LPVOID vData = malloc(dwSize);
            if (vData != NULL)
            {
               if (GetFileVersionInfoA(szImg, dwHandle, dwSize, vData) != 0)
               {
                  UINT len;
                  char szSubBlock[] = _T("\\");
                  if (VerQueryValueA(vData, szSubBlock, (LPVOID*) &fInfo, &len) == 0)
                     fInfo = NULL;
                  else
                  {
                     fileVersion = ((ULONGLONG)fInfo->dwFileVersionLS) + ((ULONGLONG)fInfo->dwFileVersionMS << 32);
                  }
               }
               free(vData);
            }
         }
      }

      // Retrive some additional-infos about the module
      IMAGEHLP_MODULE64 Module;
      const char *szSymType = "-unknown-";
      if (this->GetModuleInfo(hProcess, baseAddr, &Module) != FALSE)
      {
         switch(Module.SymType)
         {
         case SymNone:
            szSymType = "-nosymbols-";
            break;
         case SymCoff:
            szSymType = "COFF";
            break;
         case SymCv:
            szSymType = "CV";
            break;
         case SymPdb:
            szSymType = "PDB";
            break;
         case SymExport:
            szSymType = "-exported-";
            break;
         case SymDeferred:
            szSymType = "-deferred-";
            break;
         case SymSym:
            szSymType = "SYM";
            break;
         case 8: //SymVirtual:
            szSymType = "Virtual";
            break;
         case 9: // SymDia:
            szSymType = "DIA";
            break;
         }
      }
      this->OnLoadModule(img, mod, baseAddr, size, result, szSymType, Module.LoadedImageName, fileVersion);
   }
   if (szImg != NULL) free(szImg);
   if (szMod != NULL) free(szMod);
   return result;
}


// The following is used to pass the "userData"-Pointer to the user-provided readMemoryFunction
// This has to be done due to a problem with the "hProcess"-parameter in x64...
// Because this class is in no case multi-threading-enabled (because of the limitations of dbghelp.dll) it is "safe" to use a static-variable
static StackWalker::PReadProcessMemoryRoutine s_readMemoryFunction = NULL;
static LPVOID s_readMemoryFunction_UserData = NULL;


//-----------------------------------------------------------------------------------------------------------------------------------------
//ShowCallstack
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::ShowCallstack(HANDLE hThread, CONTEXT const & context, PReadProcessMemoryRoutine readMemoryFunction, LPVOID pUserData)
{
   CONTEXT c = context;
   IMAGEHLP_SYMBOL64 *pSym = NULL;
   IMAGEHLP_MODULE64 Module;
   IMAGEHLP_LINE64 Line;
   S32 frameNum;

   if (!m_modulesLoaded) LoadModules();


   s_readMemoryFunction = readMemoryFunction;
   s_readMemoryFunction_UserData = pUserData;



   // init STACKFRAME for first call
   STACKFRAME64 s; // in/out stackframe
   memset(&s, 0, sizeof(s));
   DWORD imageType;
#ifdef _M_IX86
   // normally, call ImageNtHeader() and use machine info from PE header
   imageType = IMAGE_FILE_MACHINE_I386;
   s.AddrPC.Offset = c.Eip;
   s.AddrPC.Mode = AddrModeFlat;
   s.AddrFrame.Offset = c.Ebp;
   s.AddrFrame.Mode = AddrModeFlat;
   s.AddrStack.Offset = c.Esp;
   s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
   imageType = IMAGE_FILE_MACHINE_AMD64;
   s.AddrPC.Offset = c.Rip;
   s.AddrPC.Mode = AddrModeFlat;
   s.AddrFrame.Offset = c.Rsp;
   s.AddrFrame.Mode = AddrModeFlat;
   s.AddrStack.Offset = c.Rsp;
   s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
   imageType = IMAGE_FILE_MACHINE_IA64;
   s.AddrPC.Offset = c.StIIP;
   s.AddrPC.Mode = AddrModeFlat;
   s.AddrFrame.Offset = c.IntSp;
   s.AddrFrame.Mode = AddrModeFlat;
   s.AddrBStore.Offset = c.RsBSP;
   s.AddrBStore.Mode = AddrModeFlat;
   s.AddrStack.Offset = c.IntSp;
   s.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

   pSym = (IMAGEHLP_SYMBOL64 *) malloc(sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
   if (!pSym) goto cleanup;  // not enough memory...
   memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
   pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
   pSym->MaxNameLength = STACKWALK_MAX_NAMELEN;

   memset(&Line, 0, sizeof(Line));
   Line.SizeOfStruct = sizeof(Line);

   memset(&Module, 0, sizeof(Module));
   Module.SizeOfStruct = sizeof(Module);

   for (frameNum = 0; ; ++frameNum )
   {
      // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
      // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
      // assume that either you are done, or that the stack is so hosed that the next
      // deeper frame could not be found.
      // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
      if ( ! StackWalk64(imageType, this->m_hProcess, hThread, &s, &c, myReadProcMem, SymFunctionTableAccess64, SymGetModuleBase64, NULL) )
      {
         this->OnDbgHelpErr("StackWalk64", GetLastError(), s.AddrPC.Offset);
         break;
      }

      CallstackEntry csEntry;
      csEntry.offset = s.AddrPC.Offset;
      csEntry.name[0] = 0;
      csEntry.undName[0] = 0;
      csEntry.undFullName[0] = 0;
      csEntry.offsetFromSmybol = 0;
      csEntry.offsetFromLine = 0;
      csEntry.lineFileName[0] = 0;
      csEntry.lineNumber = 0;
      csEntry.loadedImageName[0] = 0;
      csEntry.moduleName[0] = 0;
      if (s.AddrPC.Offset == s.AddrReturn.Offset)
      {
         OnDbgHelpErr("StackWalk64-Endless-Callstack!", 0, s.AddrPC.Offset);
         break;
      }
      if (s.AddrPC.Offset != 0)
      {
         // we seem to have a valid PC, show procedure info
         if (SymGetSymFromAddr64(m_hProcess, s.AddrPC.Offset, &(csEntry.offsetFromSmybol), pSym) != FALSE)
         {
            // TODO: Mache dies sicher...!
            strcpy_s(csEntry.name, pSym->Name);
            UnDecorateSymbolName( pSym->Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY );
            UnDecorateSymbolName( pSym->Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE );
         }
         else
         {
            this->OnDbgHelpErr("SymGetSymFromAddr64", GetLastError(), s.AddrPC.Offset);
         }

         // show line number info, NT5.0-method
         if (SymGetLineFromAddr64(this->m_hProcess, s.AddrPC.Offset, (PDWORD)&(csEntry.offsetFromLine), &Line) != FALSE)
         {
            csEntry.lineNumber = Line.LineNumber;
            // TODO: Mache dies sicher...!
            strcpy_s(csEntry.lineFileName, Line.FileName);
         }
         else
         {
            this->OnDbgHelpErr("SymGetLineFromAddr64", GetLastError(), s.AddrPC.Offset);
         }

         // show module info
         if( GetModuleInfo(this->m_hProcess, s.AddrPC.Offset, &Module) )
         { 
            switch ( Module.SymType )
            {
            case SymNone:
               csEntry.symTypeString = "-nosymbols-";
               break;
            case SymCoff:
               csEntry.symTypeString = "COFF";
               break;
            case SymCv:
               csEntry.symTypeString = "CV";
               break;
            case SymPdb:
               csEntry.symTypeString = "PDB";
               break;
            case SymExport:
               csEntry.symTypeString = "-exported-";
               break;
            case SymDeferred:
               csEntry.symTypeString = "-deferred-";
               break;
            case SymSym:
               csEntry.symTypeString = "SYM";
               break;
            case SymDia:
               csEntry.symTypeString = "DIA";
               break;
            case SymVirtual:
               csEntry.symTypeString = "Virtual";
               break;
            default:
               //_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
               csEntry.symTypeString = NULL;
               break;
            }

            // TODO: Mache dies sicher...!
            strcpy_s(csEntry.moduleName, Module.ModuleName);
            csEntry.baseOfImage = Module.BaseOfImage;
            strcpy_s(csEntry.loadedImageName, Module.LoadedImageName);
         }
         else
         {
            OnDbgHelpErr("SymGetModuleInfo64", GetLastError(), s.AddrPC.Offset);
         }
      }

      CallstackEntryType et = nextEntry;
      if (frameNum == 0) et = firstEntry;
      OnCallstackEntry(et, csEntry);

      if (s.AddrReturn.Offset == 0)
      {
         OnCallstackEntry(lastEntry, csEntry);
         SetLastError(ERROR_SUCCESS);
         break;
      }
   }

cleanup:
   if (pSym) free( pSym );

   return true;
}



BOOL __stdcall StackWalker::myReadProcMem(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
   if (s_readMemoryFunction == NULL)
   {
      SIZE_T st;
      BOOL bRet = ReadProcessMemory(hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, &st);
      *lpNumberOfBytesRead = (DWORD) st;
      //printf("ReadMemory: hProcess: %p, baseAddr: %p, buffer: %p, size: %d, read: %d, result: %d\n", hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, (DWORD) st, (DWORD) bRet);
      return bRet;
   }
   else
   {
      return s_readMemoryFunction(hProcess, qwBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead, s_readMemoryFunction_UserData);
   }
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//OnLoadModule
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
{
   CHAR buffer[STACKWALK_MAX_NAMELEN];
   if (fileVersion == 0)
      _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s'\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName);
   else
   {
      DWORD v4 = (DWORD) fileVersion & 0xFFFF;
      DWORD v3 = (DWORD) (fileVersion>>16) & 0xFFFF;
      DWORD v2 = (DWORD) (fileVersion>>32) & 0xFFFF;
      DWORD v1 = (DWORD) (fileVersion>>48) & 0xFFFF;
      _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s', fileVersion: %d.%d.%d.%d\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName, v1, v2, v3, v4);
   }
   if(OutputModules & m_options) OnOutput(buffer);
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//GetModuleInfo
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64 *pModuleInfo)
{

   pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
   void *pData = malloc(4096); // reserve enough memory, so the bug in v6.3.5.1 does not lead to memory-overwrites...
   if (pData == NULL)
   {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return false;
   }
   memcpy(pData, pModuleInfo, sizeof(IMAGEHLP_MODULE64));
   if (SymGetModuleInfo64(hProcess, baseAddr, (IMAGEHLP_MODULE64*) pData) != FALSE)
   {
      // only copy as much memory as is reserved...
      memcpy(pModuleInfo, pData, sizeof(IMAGEHLP_MODULE64));
      pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
      free(pData);
      return true;
   }
   free(pData);
   SetLastError(ERROR_DLL_INIT_FAILED);
   return false;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//GetModuleListTH32
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::GetModuleListTH32(HANDLE hProcess, DWORD pid)
{
   // CreateToolhelp32Snapshot()
   typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
   // Module32First()
   typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
   // Module32Next()
   typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

   // try both dlls...
   const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
   HINSTANCE hToolhelp = NULL;
   tCT32S pCT32S = NULL;
   tM32F pM32F = NULL;
   tM32N pM32N = NULL;

   HANDLE hSnap;
   MODULEENTRY32 me;
   me.dwSize = sizeof(me);
   BOOL keepGoing;
   size_t i;

   for (i = 0; i<(sizeof(dllname) / sizeof(dllname[0])); i++ )
   {
      hToolhelp = LoadLibraryA( dllname[i] );
      if (hToolhelp == NULL)
         continue;
      pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
      pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
      pM32N = (tM32N) GetProcAddress(hToolhelp, "Module32Next");
      if ( (pCT32S != NULL) && (pM32F != NULL) && (pM32N != NULL) )
         break; // found the functions!
      FreeLibrary(hToolhelp);
      hToolhelp = NULL;
   }

   if (hToolhelp == NULL)
      return false;

   hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
   if (hSnap == (HANDLE) -1)
      return false;

   keepGoing = !!pM32F( hSnap, &me );
   S32 cnt = 0;
   while (keepGoing)
   {
      this->LoadModule(hProcess, me.szExePath, me.szModule, (DWORD64) me.modBaseAddr, me.modBaseSize);
      cnt++;
      keepGoing = !!pM32N( hSnap, &me );
   }
   CloseHandle(hSnap);
   FreeLibrary(hToolhelp);
   if (cnt <= 0) return false;

   return true;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//GetModuleListPSAPI
//-----------------------------------------------------------------------------------------------------------------------------------------
bool StackWalker::GetModuleListPSAPI(HANDLE hProcess)
{
   DWORD i;
   //ModuleEntry e;
   DWORD cbNeeded;
   MODULEINFO mi;
   HMODULE *hMods = 0;
   char *tt = NULL;
   char *tt2 = NULL;
   const SIZE_T TTBUFLEN = 8096;
   S32 cnt = 0;

   hMods = (HMODULE*) malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
   tt = (char*) malloc(sizeof(char) * TTBUFLEN);
   tt2 = (char*) malloc(sizeof(char) * TTBUFLEN);
   if ( (hMods == NULL) || (tt == NULL) || (tt2 == NULL) )
      goto cleanup;

   if ( !EnumProcessModules(hProcess, hMods, TTBUFLEN, &cbNeeded) )
   {
      //_ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"), g_dwShowCount, gle );
      goto cleanup;
   }

   if ( cbNeeded > TTBUFLEN )
   {
      //_ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"), g_dwShowCount, lenof( hMods ) );
      goto cleanup;
   }

   for ( i = 0; i < cbNeeded / sizeof hMods[0]; i++ )
   {
      // base address, size
      GetModuleInformation(hProcess, hMods[i], &mi, sizeof mi );
      // image file name
      tt[0] = 0;
      GetModuleFileNameExA(hProcess, hMods[i], tt, TTBUFLEN );
      // module name
      tt2[0] = 0;
      GetModuleBaseNameA(hProcess, hMods[i], tt2, TTBUFLEN );

      DWORD dwRes = this->LoadModule(hProcess, tt, tt2, (DWORD64) mi.lpBaseOfDll, mi.SizeOfImage);
      if (dwRes != ERROR_SUCCESS)
         this->OnDbgHelpErr("LoadModule", dwRes, 0);
      cnt++;
   }

cleanup:
   if (tt2 != NULL) free(tt2);
   if (tt != NULL) free(tt);
   if (hMods != NULL) free(hMods);

   return cnt != 0;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//OnCallstackEntry
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
{
   CHAR buffer[STACKWALK_MAX_NAMELEN];
   if ( (eType != lastEntry) && (entry.offset != 0) )
   {
      if (entry.name[0] == 0) strcpy_s(entry.name, "(function-name not available)");
      if (entry.undName[0] != 0) strcpy_s(entry.name, entry.undName);
      if (entry.undFullName[0] != 0) strcpy_s(entry.name, entry.undFullName);
      if (entry.lineFileName[0] == 0)
      {
         strcpy_s(entry.lineFileName, "(filename not available)");
         if (entry.moduleName[0] == 0) strcpy_s(entry.moduleName, "(module-name not available)");
         _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s): %s: %s\n", (LPVOID) entry.offset, entry.moduleName, entry.lineFileName, entry.name);
      }
      else
      {
         _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s (%d): %s\n", entry.lineFileName, entry.lineNumber, entry.name);	
      }
      OnOutput(buffer);
   }
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//OnDbgHelpErr
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
{
   CHAR buffer[STACKWALK_MAX_NAMELEN];
   _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "ERROR: %s, GetLastError: %d (Address: %p)\n", szFuncName, gle, (LPVOID) addr);
   OnOutput(buffer);
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//OnSymInit
//-----------------------------------------------------------------------------------------------------------------------------------------
void StackWalker::OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName)
{
   //Symbol Search path
   CHAR buffer[STACKWALK_MAX_NAMELEN];
   _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "SymInit: Symbol-SearchPath: '%s', symOptions: %d, UserName: '%s'\n", szSearchPath, symOptions, szUserName);
   if(OutputSymPath & m_options) OnOutput(buffer);

   //OS-version
   OSVERSIONINFOEX ver;
   ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));
   ver.dwOSVersionInfoSize = sizeof(ver);
   if (GetVersionEx( (OSVERSIONINFO*) &ver) != FALSE)
   {
      _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "OS-Version: %d.%d.%d (%s) 0x%x-0x%x\n", 
         ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber,
         ver.szCSDVersion, ver.wSuiteMask, ver.wProductType);
      if(OutputOS & m_options) OnOutput(buffer);
   }
}

#endif

