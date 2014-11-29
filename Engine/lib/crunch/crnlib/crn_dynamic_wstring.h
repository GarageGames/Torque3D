// File: crn_dynamic_wstring.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   // UCS-2 string class (plane 0 characters only)
   class dynamic_wstring
   {
   public:
      inline dynamic_wstring() : m_buf_size(0), m_len(0), m_pStr(NULL) { }
      dynamic_wstring(eVarArg dummy, const wchar_t* p, ...);
      dynamic_wstring(const wchar_t* p);
      dynamic_wstring(const wchar_t* p, uint len);
      dynamic_wstring(const dynamic_wstring& other);

      // Conversion from UCS-2 to ANSI and vice versa
      explicit dynamic_wstring(const char* p);
      explicit dynamic_wstring(const dynamic_string& s);
      dynamic_wstring& set(const char* p);
      dynamic_string& as_ansi(dynamic_string& buf);

      inline ~dynamic_wstring() { CRNLIB_ASSUME(sizeof(wchar_t) == sizeof(uint16)); if (m_pStr) crnlib_delete_array(m_pStr); }

      // Truncates the string to 0 chars and frees the buffer.
      void clear();
      void optimize();

      // Truncates the string to 0 chars, but does not free the buffer.
      void empty();

      inline uint get_len() const { return m_len; }
      inline bool is_empty() const { return !m_len; }

      inline const wchar_t* get_ptr() const { return m_pStr ? m_pStr : L""; }

      inline const wchar_t* get_ptr_raw() const { return m_pStr; }
      inline       wchar_t* get_ptr_raw()       { return m_pStr; }

      inline wchar_t operator[] (uint i) const { CRNLIB_ASSERT(i <= m_len); return get_ptr()[i]; }

      inline operator size_t() const { return fast_hash(get_ptr(), m_len * sizeof(wchar_t)) ^ fast_hash(&m_len, sizeof(m_len)); }

      int compare(const wchar_t* p, bool case_sensitive = false) const;
      int compare(const dynamic_wstring& rhs, bool case_sensitive = false) const;

      inline bool operator== (const dynamic_wstring& rhs) const { return compare(rhs) == 0; }
      inline bool operator== (const wchar_t* p) const { return compare(p) == 0; }

      inline bool operator!= (const dynamic_wstring& rhs) const { return compare(rhs) != 0; }
      inline bool operator!= (const wchar_t* p) const { return compare(p) != 0; }

      inline bool operator< (const dynamic_wstring& rhs) const { return compare(rhs) < 0; }
      inline bool operator< (const wchar_t* p) const { return compare(p) < 0; }

      inline bool operator> (const dynamic_wstring& rhs) const { return compare(rhs) > 0; }
      inline bool operator> (const wchar_t* p) const { return compare(p) > 0; }

      inline bool operator<= (const dynamic_wstring& rhs) const { return compare(rhs) <= 0; }
      inline bool operator<= (const wchar_t* p) const { return compare(p) <= 0; }

      inline bool operator>= (const dynamic_wstring& rhs) const { return compare(rhs) >= 0; }
      inline bool operator>= (const wchar_t* p) const { return compare(p) >= 0; }

      friend inline bool operator== (const wchar_t* p, const dynamic_wstring& rhs) { return rhs.compare(p) == 0; }

      dynamic_wstring& set(const wchar_t* p, uint max_len = UINT_MAX);
      dynamic_wstring& set(const dynamic_wstring& other, uint max_len = UINT_MAX);

      bool set_len(uint new_len, wchar_t fill_char = ' ');

      // Set from non-zero terminated buffer.
      // little_endian is the endianness of the buffer's data
      dynamic_wstring& set_from_buf(const void* pBuf, uint buf_size, bool little_endian = c_crnlib_little_endian_platform);

      dynamic_wstring& operator= (const dynamic_wstring& rhs) { return set(rhs); }
      dynamic_wstring& operator= (const dynamic_string& rhs);
      dynamic_wstring& operator= (const wchar_t* p) { return set(p); }
      dynamic_wstring& operator= (const char* p) { return set(p); }

      dynamic_wstring& set_char(uint index, wchar_t c);
      dynamic_wstring& append_char(wchar_t c);
      dynamic_wstring& append_char(int c) { CRNLIB_ASSERT((c >= 0) && (c <= 0xFFFF)); return append_char(static_cast<wchar_t>(c)); }
      dynamic_wstring& truncate(uint new_len);
      dynamic_wstring& tolower();
      dynamic_wstring& toupper();

      dynamic_wstring& append(const wchar_t* p);
      dynamic_wstring& append(const dynamic_wstring& other);
      dynamic_wstring& operator += (const wchar_t* p) { return append(p); }
      dynamic_wstring& operator += (const dynamic_wstring& other) { return append(other); }

      friend dynamic_wstring operator+ (const wchar_t* p, const dynamic_wstring& a);
      friend dynamic_wstring operator+ (const dynamic_wstring& a, const wchar_t* p);
      friend dynamic_wstring operator+ (const dynamic_wstring& a, const dynamic_wstring& b);

      dynamic_wstring& format_args(const wchar_t* p, va_list args);
      dynamic_wstring& format(const wchar_t* p, ...);

      dynamic_wstring& crop(uint start, uint len);
      dynamic_wstring& substring(uint start, uint end);
      dynamic_wstring& left(uint len);
      dynamic_wstring& mid(uint start, uint len);
      dynamic_wstring& right(uint start);
      dynamic_wstring& tail(uint num);

      dynamic_wstring& unquote();

      uint count_char(wchar_t c) const;

      int find_left(const wchar_t* p, bool case_sensitive = false) const;
      int find_left(wchar_t c) const;

      int find_right(wchar_t c) const;
      int find_right(const wchar_t* p, bool case_sensitive = false) const;

      bool contains(const wchar_t* p, bool case_sensitive = false) const;

      dynamic_wstring& trim();
      dynamic_wstring& trim_crlf();

      dynamic_wstring& remap(int from_char, int to_char);

      void swap(dynamic_wstring& other);

      int serialize(void* pBuf, uint buf_size, bool little_endian) const;
      int deserialize(const void* pBuf, uint buf_size, bool little_endian);

   private:
      // These values are in characters, not bytes!
      uint16      m_buf_size;
      uint16      m_len;
      wchar_t*    m_pStr;

#ifdef CRNLIB_BUILD_DEBUG
      void check() const;
#else
      void check() const { }
#endif

      bool ensure_buf(uint len, bool preserve_contents = true);
      bool expand_buf(uint new_buf_size, bool preserve_contents);

      const wchar_t* get_ptr_priv() const { return m_pStr ? m_pStr : L""; }
      wchar_t* get_ptr_priv() { return (wchar_t*)(m_pStr ? m_pStr : L""); }
   };

   typedef crnlib::vector<dynamic_wstring> dynamic_wstring_array;

   extern dynamic_wstring g_empty_dynamic_wstring;

   CRNLIB_DEFINE_BITWISE_MOVABLE(dynamic_wstring);

   inline void swap (dynamic_wstring& a, dynamic_wstring& b)
   {
      a.swap(b);
   }

} // namespace crnlib
