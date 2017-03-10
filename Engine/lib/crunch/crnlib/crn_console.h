// File: crn_console.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_dynamic_string.h"

namespace crnlib
{
   class dynamic_string;
   class data_stream;
   class mutex;
   
   enum eConsoleMessageType
   {
      cDebugConsoleMessage,      // debugging messages
      cProgressConsoleMessage,   // progress messages
      cInfoConsoleMessage,       // ordinary messages 
      cConsoleConsoleMessage,    // user console output
      cMessageConsoleMessage,    // high importance messages
      cWarningConsoleMessage,    // warnings
      cErrorConsoleMessage,      // errors
      
      cCMTTotal
   };
   
   typedef bool (*console_output_func)(eConsoleMessageType type, const wchar_t* pMsg, void* pData);
   
   class console 
   {
   public:
      static void init();
      static void deinit();

      static bool is_initialized() { return m_pMutex != NULL; }
      
      static void set_default_category(eConsoleMessageType category);
      static eConsoleMessageType get_default_category();

      static void add_console_output_func(console_output_func pFunc, void* pData);
      static void remove_console_output_func(console_output_func pFunc);
      
      static void printf(const wchar_t* p, ...);   
      
      static void vprintf(eConsoleMessageType type, const wchar_t* p, va_list args);
      static void printf(eConsoleMessageType type, const wchar_t* p, ...);
                  
      static void cons(const wchar_t* p, ...);                        
      static void debug(const wchar_t* p, ...);      
      static void progress(const wchar_t* p, ...);
      static void info(const wchar_t* p, ...);
      static void message(const wchar_t* p, ...);
      static void warning(const wchar_t* p, ...);
      static void error(const wchar_t* p, ...);

      // FIXME: All console state is currently global!
      static void disable_prefixes();
      static void enable_prefixes();
      static bool get_prefixes() { return m_prefixes; }
      
      static void disable_crlf();
      static void enable_crlf();
      static bool get_crlf() { return m_crlf; }
      
      static void disable_output() { m_output_disabled = true; }
      static void enable_output() { m_output_disabled = false; }
      static bool get_output_disabled() { return m_output_disabled; }
      
      static void set_log_stream(data_stream* pStream) { m_pLog_stream = pStream; }
      static data_stream* get_log_stream() { return m_pLog_stream; }
      
      static uint get_num_messages(eConsoleMessageType type) { return m_num_messages[type]; }
   
   private:      
      static eConsoleMessageType m_default_category;
      
      struct console_func 
      {
         console_func(console_output_func func = NULL, void* pData = NULL) : m_func(func), m_pData(pData) { }
         
         console_output_func  m_func;
         void*                m_pData;
      };
      static crnlib::vector<console_func> m_output_funcs;
       
      static bool m_crlf, m_prefixes, m_output_disabled;
      
      static data_stream* m_pLog_stream;
      
      static mutex* m_pMutex;
      
      static uint m_num_messages[cCMTTotal];
   };

} // namespace crnlib

