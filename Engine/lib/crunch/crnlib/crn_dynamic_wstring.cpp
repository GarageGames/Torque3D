// File: crn_dynamic_wstring.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_dynamic_wstring.h"
#include "crn_winhdr.h"

namespace crnlib
{
   dynamic_wstring g_empty_dynamic_wstring;

   dynamic_wstring::dynamic_wstring(eVarArg dummy, const wchar_t* p, ...) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      dummy;

      CRNLIB_ASSERT(p);

      va_list args;
      va_start(args, p);
      format_args(p, args);
      va_end(args);
   }

   dynamic_wstring::dynamic_wstring(const wchar_t* p) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      CRNLIB_ASSERT(p);
      set(p);
   }

   dynamic_wstring::dynamic_wstring(const wchar_t* p, uint len) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      CRNLIB_ASSERT(p);
      set_from_buf(p, len);
   }

   dynamic_wstring::dynamic_wstring(const dynamic_wstring& other) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      set(other);
   }

   void dynamic_wstring::clear()
   {
      check();

      if (m_pStr)
      {
         crnlib_delete_array(m_pStr);
         m_pStr = NULL;

         m_len = 0;
         m_buf_size = 0;
      }
   }

   void dynamic_wstring::empty()
   {
      truncate(0);
   }

   void dynamic_wstring::optimize()
   {
      if (!m_len)
         clear();
      else
      {
         uint min_buf_size = math::next_pow2((uint)m_len + 1);
         if (m_buf_size > min_buf_size)
         {
            wchar_t* p = crnlib_new_array<wchar_t>(min_buf_size);
            memcpy(p, m_pStr, (m_len + 1) * sizeof(wchar_t));

            crnlib_delete_array(m_pStr);
            m_pStr = p;

            m_buf_size = static_cast<uint16>(min_buf_size);

            check();
         }
      }
   }

   int dynamic_wstring::compare(const wchar_t* p, bool case_sensitive) const
   {
      CRNLIB_ASSERT(p);

      const int result = (case_sensitive ? wcscmp : _wcsicmp)(get_ptr_priv(), p);

      if (result < 0)
         return -1;
      else if (result > 0)
         return 1;

      return 0;
   }

   int dynamic_wstring::compare(const dynamic_wstring& rhs, bool case_sensitive) const
   {
      return compare(rhs.get_ptr_priv(), case_sensitive);
   }

   dynamic_wstring& dynamic_wstring::set(const wchar_t* p, uint max_len)
   {
      CRNLIB_ASSERT(p);

      const uint len = math::minimum<uint>(max_len, static_cast<uint>(wcslen(p)));
      CRNLIB_ASSERT(len < UINT16_MAX);

      if ((!len) || (len >= UINT16_MAX))
         clear();
      else if ((m_pStr) && (p >= m_pStr) && (p < (m_pStr + m_buf_size)))
      {
         if (m_pStr != p)
            memmove(m_pStr, p, len * sizeof(wchar_t));
         m_pStr[len] = L'\0';
         m_len = static_cast<uint16>(len);
      }
      else if (ensure_buf(len, false))
      {
         m_len = static_cast<uint16>(len);
         memcpy(m_pStr, p, (m_len + 1) * sizeof(wchar_t));
      }

      check();

      return *this;
   }

   dynamic_wstring& dynamic_wstring::set(const dynamic_wstring& other, uint max_len)
   {
      if (this == &other)
      {
         if (max_len < m_len)
         {
            m_pStr[max_len] = L'\0';
            m_len = static_cast<uint16>(max_len);
         }
      }
      else
      {
         const uint len = math::minimum<uint>(max_len, other.m_len);

         if (!len)
            clear();
         else if (ensure_buf(len, false))
         {
            m_len = static_cast<uint16>(len);
            memcpy(m_pStr, other.get_ptr_priv(), m_len * sizeof(wchar_t));
            m_pStr[len] = L'\0';
         }
      }

      check();

      return *this;
   }

   bool dynamic_wstring::set_len(uint new_len, wchar_t fill_char)
   {
      if ((new_len >= UINT16_MAX) || (!fill_char))
         return false;

      uint cur_len = m_len;

      if (ensure_buf(new_len, true))
      {
         if (new_len > cur_len)
         {
            for (uint i = 0; i < (new_len - cur_len); i++)
               m_pStr[cur_len + i] = fill_char;
         }

         m_pStr[new_len] = L'\0';

         m_len = static_cast<uint16>(new_len);

         check();
      }

      return true;
   }

   dynamic_wstring& dynamic_wstring::set_from_buf(const void* pBuf, uint buf_size, bool little_endian)
   {
      CRNLIB_ASSERT(pBuf);

      if (buf_size >= UINT16_MAX)
      {
         clear();
         return *this;
      }

      for (uint i = 0; i < buf_size; i++)
      {
         if (static_cast<const wchar_t*>(pBuf)[i] == L'\0')
         {
            CRNLIB_ASSERT(0);
            clear();
            return *this;
         }
      }

      if (ensure_buf(buf_size, false))
      {
         utils::copy_words(reinterpret_cast<uint16*>(m_pStr), reinterpret_cast<const uint16*>(pBuf), buf_size, c_crnlib_little_endian_platform != little_endian);

         m_pStr[buf_size] = L'\0';

         m_len = static_cast<uint16>(buf_size);

         check();
      }

      return *this;
   }

   dynamic_wstring& dynamic_wstring::set_char(uint index, wchar_t c)
   {
      CRNLIB_ASSERT(index <= m_len);

      if (!c)
         truncate(index);
      else if (index < m_len)
      {
         m_pStr[index] = c;

         check();
      }
      else if (index == m_len)
         append_char(c);

      return *this;
   }

   dynamic_wstring& dynamic_wstring::append_char(wchar_t c)
   {
      if (ensure_buf(m_len + 1))
      {
         m_pStr[m_len] = c;
         m_pStr[m_len + 1] = L'\0';
         m_len++;
         check();
      }

      return *this;
   }

   dynamic_wstring& dynamic_wstring::truncate(uint new_len)
   {
      if (new_len < m_len)
      {
         m_pStr[new_len] = L'\0';
         m_len = static_cast<uint16>(new_len);
         check();
      }
      return *this;
   }

   dynamic_wstring& dynamic_wstring::tolower()
   {
      if (m_len)
      {
#ifdef _MSC_VER
         _wcslwr_s(get_ptr_priv(), m_buf_size);
#else
         _wcslwr(get_ptr_priv());
#endif
      }
      return *this;
   }

   dynamic_wstring& dynamic_wstring::toupper()
   {
      if (m_len)
      {
#ifdef _MSC_VER
         _wcsupr_s(get_ptr_priv(), m_buf_size);
#else
         _wcsupr(get_ptr_priv());
#endif
      }
      return *this;
   }

   dynamic_wstring& dynamic_wstring::append(const wchar_t* p)
   {
      CRNLIB_ASSERT(p);

      uint len = static_cast<uint>(wcslen(p));
      uint new_total_len = m_len + len;
      if ((new_total_len) && ensure_buf(new_total_len))
      {
         memcpy(m_pStr + m_len, p, (len + 1) * sizeof(wchar_t));
         m_len = static_cast<uint16>(m_len + len);
         check();
      }

      return *this;
   }

   dynamic_wstring& dynamic_wstring::append(const dynamic_wstring& other)
   {
      uint len = other.m_len;
      uint new_total_len = m_len + len;
      if ((new_total_len) && ensure_buf(new_total_len))
      {
         memcpy(m_pStr + m_len, other.get_ptr_priv(), (len + 1) * sizeof(wchar_t));
         m_len = static_cast<uint16>(m_len + len);
         check();
      }

      return *this;
   }

   dynamic_wstring operator+ (const wchar_t* p, const dynamic_wstring& a)
   {
      return dynamic_wstring(p).append(a);
   }

   dynamic_wstring operator+ (const dynamic_wstring& a, const wchar_t* p)
   {
      return dynamic_wstring(a).append(p);
   }

   dynamic_wstring operator+ (const dynamic_wstring& a, const dynamic_wstring& b)
   {
      return dynamic_wstring(a).append(b);
   }

   dynamic_wstring& dynamic_wstring::format_args(const wchar_t* p, va_list args)
   {
      CRNLIB_ASSERT(p);

      const uint cBufSize = 4096;
      wchar_t buf[cBufSize];

#ifdef _MSC_VER
      int l = _vsnwprintf_s(buf, cBufSize, _TRUNCATE, p, args);
#else
      int l = _vsnwprintf(buf, cBufSize, p, args);
#endif
      if (l <= 0)
         clear();
      else if (ensure_buf(l, false))
      {
         memcpy(m_pStr, buf, (l + 1) * sizeof(wchar_t));

         m_len = static_cast<uint16>(l);

         check();
      }

      return *this;
   }

   dynamic_wstring& dynamic_wstring::format(const wchar_t* p, ...)
   {
      CRNLIB_ASSERT(p);

      va_list args;
      va_start(args, p);
      format_args(p, args);
      va_end(args);
      return *this;
   }

   dynamic_wstring& dynamic_wstring::crop(uint start, uint len)
   {
      if (start >= m_len)
      {
         clear();
         return *this;
      }

      len = math::minimum<uint>(len, m_len - start);

      if (start)
         memmove(get_ptr_priv(), get_ptr_priv() + start, len * sizeof(wchar_t));

      m_pStr[len] = L'\0';

      m_len = static_cast<uint16>(len);

      check();

      return *this;
   }

   dynamic_wstring& dynamic_wstring::substring(uint start, uint end)
   {
      CRNLIB_ASSERT(start <= end);
      if (start > end)
         return *this;
      return crop(start, end - start);
   }

   dynamic_wstring& dynamic_wstring::left(uint len)
   {
      return substring(0, len);
   }

   dynamic_wstring& dynamic_wstring::mid(uint start, uint len)
   {
      return crop(start, len);
   }

   dynamic_wstring& dynamic_wstring::right(uint start)
   {
      return substring(start, get_len());
   }

   dynamic_wstring& dynamic_wstring::tail(uint num)
   {
      return substring(math::maximum<int>(static_cast<int>(get_len()) - static_cast<int>(num), 0), get_len());
   }

   dynamic_wstring& dynamic_wstring::unquote()
   {
      if (m_len >= 2)
      {
         if ( ((*this)[0] == L'\"') && ((*this)[m_len - 1] == L'\"') )
         {
            return mid(1, m_len - 2);
         }
      }

      return *this;
   }

   int dynamic_wstring::find_left(const wchar_t* p, bool case_sensitive) const
   {
      CRNLIB_ASSERT(p);

      const int p_len = (int)wcslen(p);

      for (int i = 0; i <= (m_len - p_len); i++)
         if ((case_sensitive ? wcsncmp : _wcsnicmp)(p, &m_pStr[i], p_len) == 0)
            return i;

      return -1;
   }

   bool dynamic_wstring::contains(const wchar_t* p, bool case_sensitive) const
   {
      return find_left(p, case_sensitive) >= 0;
   }

   uint dynamic_wstring::count_char(wchar_t c) const
   {
      uint count = 0;
      for (uint i = 0; i < m_len; i++)
         if (m_pStr[i] == c)
            count++;
      return count;
   }

   int dynamic_wstring::find_left(wchar_t c) const
   {
      for (uint i = 0; i < m_len; i++)
         if (m_pStr[i] == c)
            return i;
      return -1;
   }

   int dynamic_wstring::find_right(wchar_t c) const
   {
      for (int i = (int)m_len - 1; i >= 0; i--)
         if (m_pStr[i] == c)
            return i;
      return -1;
   }

   int dynamic_wstring::find_right(const wchar_t* p, bool case_sensitive) const
   {
      CRNLIB_ASSERT(p);
      const int p_len = (int)wcslen(p);

      for (int i = m_len - p_len; i >= 0; i--)
         if ((case_sensitive ? wcsncmp : _wcsnicmp)(p, &m_pStr[i], p_len) == 0)
            return i;

      return -1;
   }

   dynamic_wstring& dynamic_wstring::trim()
   {
      int s, e;
      for (s = 0; s < (int)m_len; s++)
         if (!iswspace(m_pStr[s]))
            break;

      for (e = m_len - 1; e > s; e--)
         if (!iswspace(m_pStr[e]))
            break;

      return crop(s, e - s + 1);
   }

   dynamic_wstring& dynamic_wstring::trim_crlf()
   {
      int s = 0, e;

      for (e = m_len - 1; e > s; e--)
         if ((m_pStr[e] != 13) && (m_pStr[e] != 10))
            break;

      return crop(s, e - s + 1);
   }

   dynamic_wstring& dynamic_wstring::remap(int from_char, int to_char)
   {
      for (uint i = 0; i < m_len; i++)
         if (m_pStr[i] == from_char)
            m_pStr[i] = (wchar_t)to_char;
      return *this;
   }

#ifdef CRNLIB_BUILD_DEBUG
   void dynamic_wstring::check() const
   {
      if (!m_pStr)
      {
         CRNLIB_ASSERT(!m_buf_size && !m_len);
      }
      else
      {
         CRNLIB_ASSERT(m_buf_size);
         CRNLIB_ASSERT((m_buf_size == UINT16_MAX) || math::is_power_of_2((uint32)m_buf_size));
         CRNLIB_ASSERT(m_len < m_buf_size);
         CRNLIB_ASSERT(wcslen(m_pStr) == m_len);
      }
   }
#endif

   bool dynamic_wstring::ensure_buf(uint len, bool preserve_contents)
   {
      uint buf_size_needed = len + 1;

      CRNLIB_ASSERT(buf_size_needed <= UINT16_MAX);

      if (buf_size_needed <= UINT16_MAX)
      {
         if (buf_size_needed > m_buf_size)
            expand_buf(buf_size_needed, preserve_contents);
      }

      return m_buf_size >= buf_size_needed;
   }

   bool dynamic_wstring::expand_buf(uint new_buf_size, bool preserve_contents)
   {
      new_buf_size = math::minimum<uint>(UINT16_MAX, math::next_pow2(math::maximum<uint>(m_buf_size, new_buf_size)));

      if (new_buf_size != m_buf_size)
      {
         wchar_t* p = crnlib_new_array<wchar_t>(new_buf_size);

         if (preserve_contents)
            memcpy(p, get_ptr_priv(), (m_len + 1) * sizeof(wchar_t));

         crnlib_delete_array(m_pStr);
         m_pStr = p;

         m_buf_size = static_cast<uint16>(new_buf_size);

         if (preserve_contents)
            check();
      }

      return m_buf_size >= new_buf_size;
   }

   void dynamic_wstring::swap(dynamic_wstring& other)
   {
      utils::swap(other.m_buf_size, m_buf_size);
      utils::swap(other.m_len, m_len);
      utils::swap(other.m_pStr, m_pStr);
   }

   int dynamic_wstring::serialize(void* pBuf, uint buf_size, bool little_endian) const
   {
      CRNLIB_ASSERT(pBuf);

      uint buf_left = buf_size;

      if (m_len > UINT16_MAX)
         return -1;

      if (!utils::write_val((uint16)m_len, pBuf, buf_left, little_endian))
         return -1;

      if (buf_left < (m_len * sizeof(wchar_t)))
         return -1;

      utils::copy_words(reinterpret_cast<uint16*>(pBuf), reinterpret_cast<const uint16*>(get_ptr_priv()), m_len, little_endian != c_crnlib_little_endian_platform);

      buf_left -= m_len * sizeof(wchar_t);

      return buf_size - buf_left;
   }

   int dynamic_wstring::deserialize(const void* pBuf, uint buf_size, bool little_endian)
   {
      CRNLIB_ASSERT(pBuf);

      uint buf_left = buf_size;

      if (buf_left < sizeof(uint16)) return -1;

      uint16 l;
      if (!utils::read_obj(l, pBuf, buf_left, little_endian))
         return -1;

      if (buf_left < (l * sizeof(wchar_t)))
         return -1;

      set_from_buf(pBuf, l, little_endian);

      buf_left -= l * sizeof(wchar_t);

      return buf_size - buf_left;
   }

   dynamic_wstring::dynamic_wstring(const char* p) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      set(p);
   }

   dynamic_wstring::dynamic_wstring(const dynamic_string& s) :
      m_buf_size(0), m_len(0), m_pStr(NULL)
   {
      set(s.get_ptr());
   }

   dynamic_wstring& dynamic_wstring::set(const char* p)
   {
      CRNLIB_ASSERT(p);
      if (!p)
      {
         clear();
         return *this;
      }

      uint l = static_cast<uint>(strlen(p));
      if (!l)
      {
         clear();
         return *this;
      }

      const uint num_needed = static_cast<uint>(MultiByteToWideChar(CP_ACP, 0, p, l, NULL, 0));
      if (!num_needed)
      {
         clear();
         return *this;
      }

      if (!ensure_buf(num_needed, false))
      {
         clear();
         return *this;
      }

      const uint num_written = static_cast<uint>(MultiByteToWideChar(CP_ACP, 0, p, l, m_pStr, num_needed));
      CRNLIB_ASSERT(num_needed == num_written);

      m_pStr[num_written] = L'\0';
      m_len = static_cast<uint16>(num_written);

      check();

      return *this;
   }

   dynamic_string& dynamic_wstring::as_ansi(dynamic_string& buf)
   {
      if (!m_len)
      {
         buf.clear();
         return buf;
      }

      const uint num_needed = WideCharToMultiByte(CP_ACP, 0, m_pStr, m_len, NULL, 0, NULL, NULL);
      if (num_needed <= 0)
      {
         buf.clear();
         return buf;
      }

      if (!buf.ensure_buf(num_needed, false))
      {
         buf.clear();
         return buf;
      }

      const uint num_written = WideCharToMultiByte(CP_ACP, 0, m_pStr, m_len, buf.get_ptr_raw(), num_needed, NULL, NULL);
      CRNLIB_ASSERT(num_written == num_needed);

      buf.get_ptr_raw()[num_written] = 0;
      buf.m_len = static_cast<uint16>(num_written);

      buf.check();

      return buf;
   }

   dynamic_wstring& dynamic_wstring::operator= (const dynamic_string& rhs)
   {
      return set(rhs.get_ptr());
   }

} // namespace crnlib
