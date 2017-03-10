// File: crn_console.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_console.h"
#include "crn_data_stream.h"

namespace crnlib
{
   eConsoleMessageType                       console::m_default_category = cInfoConsoleMessage;
   crnlib::vector<console::console_func>      console::m_output_funcs;
   bool                                      console::m_crlf = true;
   bool                                      console::m_prefixes = true;
   bool                                      console::m_output_disabled;
   data_stream*                              console::m_pLog_stream;
   mutex*                                    console::m_pMutex;
   uint                                      console::m_num_messages[cCMTTotal];

   const uint cConsoleBufSize = 4096;

   void console::init()
   {
      if (!m_pMutex)
      {
         m_pMutex = crnlib_new<mutex>();
      }
   }

   void console::deinit()
   {
      if (m_pMutex)
      {
         crnlib_delete(m_pMutex);
         m_pMutex = NULL;
      }
   }

   void console::disable_crlf()
   {
      init();

      m_crlf = false;
   }

   void console::enable_crlf()
   {
      init();

      m_crlf = true;
   }

   void console::vprintf(eConsoleMessageType type, const wchar_t* p, va_list args)
   {
      init();

      scoped_mutex lock(*m_pMutex);

      m_num_messages[type]++;

      wchar_t buf[cConsoleBufSize];
#ifdef _MSC_VER
      vswprintf_s(buf, cConsoleBufSize, p, args);
#else
      vswprintf(buf, p, args);
#endif

      bool handled = false;

      if (m_output_funcs.size())
      {
         for (uint i = 0; i < m_output_funcs.size(); i++)
            if (m_output_funcs[i].m_func(type, buf, m_output_funcs[i].m_pData))
               handled = true;
      }

      const wchar_t* pPrefix = NULL;
      if (m_prefixes)
      {
         switch (type)
         {
            case cDebugConsoleMessage:    pPrefix = L"Debug: ";   break;
            case cWarningConsoleMessage:  pPrefix = L"Warning: "; break;
            case cErrorConsoleMessage:    pPrefix = L"Error: ";   break;
            default: break;
         }
      }

      if ((!m_output_disabled) && (!handled))
      {
#ifdef _XBOX
         if (pPrefix)
            OutputDebugStringW(pPrefix);
         OutputDebugStringW(buf);
         if (m_crlf)
            OutputDebugStringW(L"\n");
#else
         if (pPrefix)
            ::wprintf(pPrefix);
         ::wprintf(m_crlf ? L"%s\n" : L"%s", buf);
#endif
      }

      if ((type != cProgressConsoleMessage) && (m_pLog_stream))
      {
         // Yes this is bad.
         dynamic_wstring utf16_buf(buf);

         dynamic_string ansi_buf;
         utf16_buf.as_ansi(ansi_buf);
         ansi_buf.translate_lf_to_crlf();

         m_pLog_stream->printf(m_crlf ? "%s\r\n" : "%s", ansi_buf.get_ptr());
         m_pLog_stream->flush();
      }
   }

   void console::printf(eConsoleMessageType type, const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(type, p, args);
      va_end(args);
   }

   void console::printf(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(m_default_category, p, args);
      va_end(args);
   }

   void console::set_default_category(eConsoleMessageType category)
   {
      init();

      m_default_category = category;
   }

   eConsoleMessageType console::get_default_category()
   {
      init();

      return m_default_category;
   }

   void console::add_console_output_func(console_output_func pFunc, void* pData)
   {
      init();

      scoped_mutex lock(*m_pMutex);

      m_output_funcs.push_back(console_func(pFunc, pData));
   }

   void console::remove_console_output_func(console_output_func pFunc)
   {
      init();

      scoped_mutex lock(*m_pMutex);

      for (int i = m_output_funcs.size() - 1; i >= 0; i--)
      {
         if (m_output_funcs[i].m_func == pFunc)
         {
            m_output_funcs.erase(m_output_funcs.begin() + i);
         }
      }

      if (!m_output_funcs.size())
      {
         m_output_funcs.clear();
      }
   }

   void console::progress(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cProgressConsoleMessage, p, args);
      va_end(args);
   }

   void console::info(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cInfoConsoleMessage, p, args);
      va_end(args);
   }

   void console::message(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cMessageConsoleMessage, p, args);
      va_end(args);
   }

   void console::cons(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cConsoleConsoleMessage, p, args);
      va_end(args);
   }

   void console::debug(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cDebugConsoleMessage, p, args);
      va_end(args);
   }

   void console::warning(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cWarningConsoleMessage, p, args);
      va_end(args);
   }

   void console::error(const wchar_t* p, ...)
   {
      va_list args;
      va_start(args, p);
      vprintf(cErrorConsoleMessage, p, args);
      va_end(args);
   }

} // namespace crnlib
