// File: crn_win32_console.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_console.h"
#include "crn_event.h"

namespace crnlib
{
   class win32_console
   {
   public:
      static void init();
      static void deinit();
      static void tick();
   
   private:
      static bool console_output_func(eConsoleMessageType type, const wchar_t* pMsg, void* pData);

   };

} // namespace crnlib
