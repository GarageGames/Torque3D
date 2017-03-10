// File: crn_dynamic_string.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   class dynamic_wstring;

   class dynamic_string
   {
      friend class dynamic_wstring;

   public:
      inline dynamic_string() : m_buf_size(0), m_len(0), m_pStr(NULL) { }
      dynamic_string(eVarArg dummy, const char* p, ...);
      dynamic_string(const char* p);
      dynamic_string(const char* p, uint len);
      dynamic_string(const dynamic_string& other);

      inline ~dynamic_string() { if (m_pStr) crnlib_delete_array(m_pStr); }

      explicit dynamic_string(const wchar_t* pStr);
      dynamic_string& set(const wchar_t *pStr);
      dynamic_wstring& as_utf16(dynamic_wstring &buf);

      // Truncates the string to 0 chars and frees the buffer.
      void clear();
      void optimize();

      // Truncates the string to 0 chars, but does not free the buffer.
      void empty();

      inline uint get_len() const { return m_len; }
      inline bool is_empty() const { return !m_len; }

      inline const char* get_ptr() const { return m_pStr ? m_pStr : ""; }

      inline const char* get_ptr_raw() const { return m_pStr; }
      inline       char* get_ptr_raw()       { return m_pStr; }

      inline char operator[] (uint i) const { CRNLIB_ASSERT(i <= m_len); return get_ptr()[i]; }

      inline operator size_t() const { return fast_hash(get_ptr(), m_len) ^ fast_hash(&m_len, sizeof(m_len)); }

      int compare(const char* p, bool case_sensitive = false) const;
      int compare(const dynamic_string& rhs, bool case_sensitive = false) const;

      inline bool operator== (const dynamic_string& rhs) const { return compare(rhs) == 0; }
      inline bool operator== (const char* p) const { return compare(p) == 0; }

      inline bool operator!= (const dynamic_string& rhs) const { return compare(rhs) != 0; }
      inline bool operator!= (const char* p) const { return compare(p) != 0; }

      inline bool operator< (const dynamic_string& rhs) const { return compare(rhs) < 0; }
      inline bool operator< (const char* p) const { return compare(p) < 0; }

      inline bool operator> (const dynamic_string& rhs) const { return compare(rhs) > 0; }
      inline bool operator> (const char* p) const { return compare(p) > 0; }

      inline bool operator<= (const dynamic_string& rhs) const { return compare(rhs) <= 0; }
      inline bool operator<= (const char* p) const { return compare(p) <= 0; }

      inline bool operator>= (const dynamic_string& rhs) const { return compare(rhs) >= 0; }
      inline bool operator>= (const char* p) const { return compare(p) >= 0; }

      friend inline bool operator== (const char* p, const dynamic_string& rhs) { return rhs.compare(p) == 0; }

      dynamic_string& set(const char* p, uint max_len = UINT_MAX);
      dynamic_string& set(const dynamic_string& other, uint max_len = UINT_MAX);

      bool set_len(uint new_len, char fill_char = ' ');

      // Set from non-zero terminated buffer.
      dynamic_string& set_from_buf(const void* pBuf, uint buf_size);

      dynamic_string& operator= (const dynamic_string& rhs) { return set(rhs); }
      dynamic_string& operator= (const dynamic_wstring& rhs);
      dynamic_string& operator= (const char* p) { return set(p); }

      dynamic_string& set_char(uint index, char c);
      dynamic_string& append_char(char c);
      dynamic_string& append_char(int c) { CRNLIB_ASSERT((c >= 0) && (c <= 255)); return append_char(static_cast<char>(c)); }
      dynamic_string& truncate(uint new_len);
      dynamic_string& tolower();
      dynamic_string& toupper();

      dynamic_string& append(const char* p);
      dynamic_string& append(const dynamic_string& other);
      dynamic_string& operator += (const char* p) { return append(p); }
      dynamic_string& operator += (const dynamic_string& other) { return append(other); }

      friend dynamic_string operator+ (const char* p, const dynamic_string& a);
      friend dynamic_string operator+ (const dynamic_string& a, const char* p);
      friend dynamic_string operator+ (const dynamic_string& a, const dynamic_string& b);

      dynamic_string& format_args(const char* p, va_list args);
      dynamic_string& format(const char* p, ...);

      dynamic_string& crop(uint start, uint len);
      dynamic_string& substring(uint start, uint end);
      dynamic_string& left(uint len);
      dynamic_string& mid(uint start, uint len);
      dynamic_string& right(uint start);
      dynamic_string& tail(uint num);

      dynamic_string& unquote();

      uint count_char(char c) const;

      int find_left(const char* p, bool case_sensitive = false) const;
      int find_left(char c) const;

      int find_right(char c) const;
      int find_right(const char* p, bool case_sensitive = false) const;

      bool contains(const char* p, bool case_sensitive = false) const;

      dynamic_string& trim();
      dynamic_string& trim_crlf();

      dynamic_string& remap(int from_char, int to_char);

      void swap(dynamic_string& other);

      // Returns -1 on failure, or the number of bytes written.
      int serialize(void* pBuf, uint buf_size, bool little_endian) const;

      // Returns -1 on failure, or the number of bytes read.
      int deserialize(const void* pBuf, uint buf_size, bool little_endian);

      void translate_lf_to_crlf();

   private:
      uint16      m_buf_size;
      uint16      m_len;
      char*       m_pStr;

#ifdef CRNLIB_BUILD_DEBUG
      void check() const;
#else
      inline void check() const { }
#endif

      bool expand_buf(uint new_buf_size, bool preserve_contents);

      const char* get_ptr_priv() const { return m_pStr ? m_pStr : ""; }
      char* get_ptr_priv() { return (char*)(m_pStr ? m_pStr : ""); }

      bool ensure_buf(uint len, bool preserve_contents = true);
   };

   typedef crnlib::vector<dynamic_string> dynamic_string_array;

   extern dynamic_string g_empty_dynamic_string;

   CRNLIB_DEFINE_BITWISE_MOVABLE(dynamic_string);

   inline void swap (dynamic_string& a, dynamic_string& b)
   {
      a.swap(b);
   }

} // namespace crnlib
