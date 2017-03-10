// File: crn_win32_console.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_win32_console.h"
#include "crn_winhdr.h"

namespace crnlib
{
   void win32_console::init()
   {
      console::init();
      console::add_console_output_func(console_output_func, NULL);
   }

   void win32_console::deinit()
   {
      console::remove_console_output_func(console_output_func);
      console::deinit();
   }

   void win32_console::tick()
   {
   }

#ifdef CRNLIB_PLATFORM_PC
   bool win32_console::console_output_func(eConsoleMessageType type, const wchar_t* pMsg, void* pData)
   {
      pData;

      if (console::get_output_disabled())
         return true;

      HANDLE cons = GetStdHandle(STD_OUTPUT_HANDLE);

      DWORD attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      switch (type)
      {
         case cDebugConsoleMessage:    attr = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
         case cMessageConsoleMessage:  attr = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
         case cWarningConsoleMessage:  attr = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY; break;
         case cErrorConsoleMessage:    attr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
         default: break;
      }

      if (INVALID_HANDLE_VALUE != cons)
         SetConsoleTextAttribute(cons, (WORD)attr);

      if (console::get_prefixes())
      {
         switch (type)
         {
            case cDebugConsoleMessage:
               wprintf(L"Debug: %s", pMsg);
               break;
            case cWarningConsoleMessage:
               wprintf(L"Warning: %s", pMsg);
               break;
            case cErrorConsoleMessage:
               wprintf(L"Error: %s", pMsg);
               break;
            default:
               wprintf(L"%s", pMsg);
               break;
         }
      }
      else
      {
         wprintf(L"%s", pMsg);
      }

      if (console::get_crlf())
         wprintf(L"\n");

      if (INVALID_HANDLE_VALUE != cons)
         SetConsoleTextAttribute(cons, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

      return true;
   }
#else
   bool win32_console::console_output_func(eConsoleMessageType type, const wchar_t* pMsg, void* pData)
   {
      if (console::get_output_disabled())
         return true;

      if (console::get_prefixes())
      {
         switch (type)
         {
         case cDebugConsoleMessage:
            wprintf(L"Debug: %s", pMsg);
            break;
         case cWarningConsoleMessage:
            wprintf(L"Warning: %s", pMsg);
            break;
         case cErrorConsoleMessage:
            wprintf(L"Error: %s", pMsg);
            break;
         default:
            wprintf(L"%s", pMsg);
            break;
         }
      }
      else
      {
         wprintf(L"%s", pMsg);
      }

      if (console::get_crlf())
         wprintf(L"\n");

      return true;
   }
#endif

} // namespace crnlib

