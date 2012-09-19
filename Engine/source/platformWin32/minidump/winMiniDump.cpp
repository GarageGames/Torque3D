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

#include "platformWin32/platformWin32.h"
#include "platformWin32/minidump/winStackWalker.h"
#include "core/fileio.h"
#include "core/strings/stringFunctions.h"
#include "console/console.h"
#include "app/net/serverQuery.h"

#pragma pack(push,8)
#include <DbgHelp.h>
#include <time.h>
#pragma pack(pop)

#pragma comment(lib, "dbghelp.lib")

extern Win32PlatState winState;

//Forward declarations for the dialog functions
BOOL CALLBACK MiniDumpDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT DisplayMiniDumpDialog(HINSTANCE hinst, HWND hwndOwner);
LPWORD lpwAlign(LPWORD lpIn);
char gUserInput[4096];

//Console variables
extern StringTableEntry gMiniDumpDir;
extern StringTableEntry gMiniDumpUser;
extern StringTableEntry gMiniDumpExec;
extern StringTableEntry gMiniDumpParams;
extern StringTableEntry gMiniDumpExecDir;


char* dStrrstr(char* dst, const char* src, const char* findStr, char* replaceStr)
{
   //see if str contains findStr, if not then return
   const char* findpos = strstr(src, findStr);
   if(!findpos) 
   {
      strcpy(dst, src);
   }
   else
   {
      //copy the new string to the buffer
      dst[0]='\0';
      strncat(dst, src, findpos-src);
      strcat(dst, replaceStr);
      const char* cur = findpos + strlen(findStr);
      strcat(dst, cur);
   }

   return dst;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//  CreateMiniDump()
//-----------------------------------------------------------------------------------------------------------------------------------------
INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo)
{
   //Get any information we can from the user and store it in gUserInput
   try
   {
      while(ShowCursor(TRUE) < 0);
      DisplayMiniDumpDialog(winState.appInstance, winState.appWindow);
   }
   catch(...)
   {
      //dSprintf(gUserInput, 4096, "The user could not enter a description of what was occurring.\n\n\n");
   }

   //Build a Game, Date and Time stamped MiniDump folder
   time_t theTime;
   time(&theTime);
   tm* pLocalTime = localtime(&theTime);
   char crashFolder[2048];
   dSprintf(crashFolder, 2048, "%s_%02d.%02d_%02d.%02d.%02d", 
      Platform::getExecutableName(),
      pLocalTime->tm_mon+1, pLocalTime->tm_mday,
      pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

   //Builed the fully qualified MiniDump path
   char crashPath[2048];
   char fileName[2048];
   if(gMiniDumpDir==NULL)
   {
      dSprintf(crashPath, 2048, "%s/MiniDump/%s", Platform::getCurrentDirectory(), crashFolder);
   }
   else
   {
      dSprintf(crashPath, 2048, "%s/%s", gMiniDumpDir, crashFolder);
   }

   dSprintf(fileName, 2048, "%s/Minidump.dmp",crashPath);
   if (!Platform::createPath (fileName))return false;  //create the directory

   //Save the minidump
   File fileObject;
   if(fileObject.open(fileName, File::Write) == File::Ok)
   {
      MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;
      DumpExceptionInfo.ThreadId = GetCurrentThreadId();
      DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
      DumpExceptionInfo.ClientPointers	= true;
      MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), (HANDLE)fileObject.getHandle(), MiniDumpNormal, &DumpExceptionInfo, NULL, NULL );
      fileObject.close();
   }

   //copy over the log file
   char fromFile[2048];
   dSprintf(fromFile, 2048, "%s/%s", Platform::getCurrentDirectory(), "console.log" );
   dSprintf(fileName, 2048, "%s/console.log", crashPath);
   Con::setLogMode(3); //ensure that the log file is closed (so it can be copied)
   dPathCopy(fromFile, fileName, true);

   //copy over the exe file
   char exeName[1024];
   dSprintf(exeName, 1024, Platform::getExecutableName());
   exeName[dStrlen(exeName)-4]=0;
   dSprintf(fromFile, 2048, "%s/%s.dll", Platform::getCurrentDirectory(), exeName );
   dSprintf(fileName, 2048, "%s/%s.dll", crashPath, exeName );
   dPathCopy(fromFile, fileName, true);

   //copy over the pdb file
   char pdbName[1024];
   dStrcpy(pdbName, exeName);	
   dStrncat(pdbName, ".pdb", 4);
   dSprintf(fromFile, 2048, "%s/%s", Platform::getCurrentDirectory(), pdbName );
   dSprintf(fileName, 2048, "%s/%s", crashPath, pdbName );
   dPathCopy(fromFile, fileName, true);

   //save the call stack
   char traceBuffer[65536];
   traceBuffer[0] = 0;
   dGetStackTrace( traceBuffer, *static_cast<const CONTEXT *>(ExceptionInfo->ContextRecord) );

   //save the user input and the call stack to a file
   char crashlogFile[2048];
   dSprintf(crashlogFile, 2048, "%s/crash.log", crashPath);
   if(fileObject.open(crashlogFile, File::Write) == File::Ok)
   {
      fileObject.write(strlen(gUserInput), gUserInput);
      fileObject.write(strlen(traceBuffer), traceBuffer);
      fileObject.close();
   }

   //call the external program indicated in script
   if(gMiniDumpExec!= NULL)
   {
      //replace special variables in gMiniDumpParams
      if(gMiniDumpParams)
      {
         char updateParams[4096];
         char finalParams[4096];
         dStrrstr(finalParams, gMiniDumpParams, "%crashpath%", crashPath);
         dStrrstr(updateParams, finalParams, "%crashfolder%", crashFolder);
         dStrrstr(finalParams, updateParams, "%crashlog%", crashlogFile);
         ShellExecuteA(NULL, "", gMiniDumpExec, finalParams, gMiniDumpExecDir ? gMiniDumpExecDir : "", SW_SHOWNORMAL);
      }
      else
      {
         ShellExecuteA(NULL, "", gMiniDumpExec, "", gMiniDumpExecDir ? gMiniDumpExecDir : "", SW_SHOWNORMAL);
      }
   }

   return EXCEPTION_EXECUTE_HANDLER;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//  MiniDumpDialogProc - Used By DisplayMiniDumpDialog
//-----------------------------------------------------------------------------------------------------------------------------------------
const int ID_TEXT=200;
const int ID_USERTEXT=300;
const int ID_DONE=400;
BOOL CALLBACK MiniDumpDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
   char text[128]= "";

   switch (message) 
   { 
   case WM_INITDIALOG :
      SetDlgItemTextA ( hwndDlg, ID_USERTEXT, text );
      return TRUE ;

   case WM_COMMAND: 
      switch (LOWORD(wParam)) 
      { 
      case ID_DONE:
         if( !GetDlgItemTextA(hwndDlg, ID_USERTEXT, gUserInput, 4096) )  gUserInput[0]='\0'; 
         strcat(gUserInput, "\n\n\n");
         EndDialog(hwndDlg, wParam); 
         return TRUE; 
      default:
         return TRUE;
      } 
   } 
   return FALSE; 
} 


//-----------------------------------------------------------------------------------------------------------------------------------------
//  Helper function to DWORD align the Dialog Box components (Used in DisplayMiniDumpDialog()
//-----------------------------------------------------------------------------------------------------------------------------------------
LPWORD lpwAlign(LPWORD lpIn)
{
   ULONG ul;

   ul = (ULONG)lpIn;
   ul ++;
   ul >>=1;
   ul <<=1;
   return (LPWORD)ul;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
//  Create the Dialog Box to get input from the user
//-----------------------------------------------------------------------------------------------------------------------------------------
LRESULT DisplayMiniDumpDialog(HINSTANCE hinst, HWND hwndOwner)
{
   HGLOBAL hgbl = GlobalAlloc(GMEM_ZEROINIT, 1024);
   if (!hgbl) return -1;

   //-----------------------------------------------------------------
   // Define the dialog box
   //-----------------------------------------------------------------
   LPDLGTEMPLATE lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
   lpdt->style = WS_POPUP | WS_BORDER | DS_MODALFRAME | WS_CAPTION;
   lpdt->cdit = 3;         // Number of controls
   lpdt->x  = 100;  
   lpdt->y  = 100;
   lpdt->cx = 300; 
   lpdt->cy = 90;

   LPWORD lpw = (LPWORD)(lpdt + 1);
   *lpw++ = 0;             // No menu
   *lpw++ = 0;             // Predefined dialog box class (by default)

   LPWSTR lpwsz = (LPWSTR)lpw;
   int nchar = 1 + MultiByteToWideChar(CP_ACP, 0, "MiniDump Crash Report", -1, lpwsz, 50);
   lpw += nchar;

   //-----------------------------------------------------------------
   // Define a static text message
   //-----------------------------------------------------------------
   lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
   LPDLGITEMTEMPLATE lpdit = (LPDLGITEMTEMPLATE)lpw;
   lpdit->x  = 10; 
   lpdit->y  = 10;
   lpdit->cx = 290; 
   lpdit->cy = 10;
   lpdit->id = ID_TEXT;    // Text identifier
   lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;

   lpw = (LPWORD)(lpdit + 1);
   *lpw++ = 0xFFFF;
   *lpw++ = 0x0082;        // Static class

   LPSTR msg = "The program has crashed.  Please describe what was happening:";
   for (lpwsz = (LPWSTR)lpw; *lpwsz++ = (WCHAR)*msg++;);
   lpw = (LPWORD)lpwsz;

   *lpw++ = 0;             // No creation data        

   //-----------------------------------------------------------------
   // Define a DONE button
   //-----------------------------------------------------------------
   lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
   lpdit = (LPDLGITEMTEMPLATE)lpw;
   lpdit->x  = 265; 
   lpdit->y  = 75;
   lpdit->cx = 25; 
   lpdit->cy = 12;
   lpdit->id = ID_DONE;       // OK button identifier
   lpdit->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;// | BS_DEFPUSHBUTTON;

   lpw = (LPWORD)(lpdit + 1);
   *lpw++ = 0xFFFF;
   *lpw++ = 0x0080;        // Button class

   lpwsz = (LPWSTR)lpw;
   nchar = 1 + MultiByteToWideChar(CP_ACP, 0, "Done", -1, lpwsz, 50);
   lpw += nchar;
   *lpw++ = 0;					// No creation data

   //-----------------------------------------------------------------
   // Define a text entry message
   //-----------------------------------------------------------------
   lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
   lpdit = (LPDLGITEMTEMPLATE)lpw;
   lpdit->x  = 10; 
   lpdit->y  = 22;
   lpdit->cx = 280; 
   lpdit->cy = 50;
   lpdit->id = ID_USERTEXT;    // Text identifier
   lpdit->style = ES_LEFT | WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE;

   lpw = (LPWORD)(lpdit + 1);
   *lpw++ = 0xFFFF;
   *lpw++ = 0x0081;        // Text edit class

   *lpw++ = 0;             // No creation data



   GlobalUnlock(hgbl); 
   LRESULT ret = DialogBoxIndirect(	hinst, 
      (LPDLGTEMPLATE)hgbl, 
      hwndOwner, 
      (DLGPROC)MiniDumpDialogProc); 
   GlobalFree(hgbl); 
   return ret; 
}

#endif