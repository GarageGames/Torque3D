// File: crn_strutils.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   char* strcpy_safe(char* pDst, uint dst_len, const char* pSrc);

   bool int_to_string(int value, char* pDst, uint len);
   bool uint_to_string(uint value, char* pDst, uint len);
   
   bool string_to_int(const char*& pBuf, int& value);
   bool string_to_int(const wchar_t*& pBuf, int& value);
   
   bool string_to_uint(const char*& pBuf, uint& value);
   bool string_to_uint(const wchar_t*& pBuf, uint& value);
   
   bool string_to_int64(const char*& pBuf, int64& value);
   bool string_to_uint64(const char*& pBuf, uint64& value);
      
   bool string_to_bool(const char* p, bool& value);
   bool string_to_bool(const wchar_t* p, bool& value);
   
   bool string_to_float(const char*& p, float& value, uint round_digit = 10U);
   bool string_to_float(const wchar_t*& p, float& value, uint round_digit = 10U);
      
   bool split_path(const char* p, dynamic_string* pDrive, dynamic_string* pDir, dynamic_string* pFilename, dynamic_string* pExt);
   bool split_path(const wchar_t* p, dynamic_wstring* pDrive, dynamic_wstring* pDir, dynamic_wstring* pFilename, dynamic_wstring* pExt);
   
   bool split_path(const char* p, dynamic_string& path, dynamic_string& filename);
   bool split_path(const wchar_t* p, dynamic_wstring& path, dynamic_wstring& filename);
      
   bool get_pathname(const char* p, dynamic_string& path);
   bool get_pathname(const wchar_t* p, dynamic_wstring& path);
   
   bool get_filename(const char* p, dynamic_string& filename);
   bool get_filename(const wchar_t* p, dynamic_wstring& filename);
   
   void combine_path(dynamic_string& dst, const char* pA, const char* pB);
   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB);
   
   void combine_path(dynamic_string& dst, const char* pA, const char* pB, const char* pC);
   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB, const wchar_t* pC);
   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB, const wchar_t* pC, const wchar_t *pD);
   
   bool full_path(dynamic_string& path);
   bool full_path(dynamic_wstring& path);
   
   bool get_extension(dynamic_string& filename);
   bool get_extension(dynamic_wstring& filename);
   
   bool remove_extension(dynamic_string& filename);
   bool remove_extension(dynamic_wstring& filename);
   
   bool create_path(const dynamic_wstring& path);
   
   void trim_trailing_seperator(dynamic_wstring& path);
                  
} // namespace crnlib
