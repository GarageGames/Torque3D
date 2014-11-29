// File: crn_strutils.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_strutils.h"
#include <direct.h>

namespace crnlib
{
   char* strcpy_safe(char* pDst, uint dst_len, const char* pSrc)
   {
      CRNLIB_ASSERT(pDst && pSrc && dst_len);
      if (!dst_len)
         return pDst;

      char* q = pDst;
      char c;

      do
      {
         if (dst_len == 1)
         {
            *q++ = '\0';
            break;
         }

         c = *pSrc++;
         *q++ = c;

         dst_len--;

      } while (c);

      CRNLIB_ASSERT((q - pDst) <= (int)dst_len);

      return pDst;
   }

   bool int_to_string(int value, char* pDst, uint len)
   {
      CRNLIB_ASSERT(pDst);

      const uint cBufSize = 16;
      char buf[cBufSize];

      uint j = static_cast<uint>((value < 0) ? -value : value);

      char* p = buf + cBufSize - 1;

      *p-- = '\0';

      do
      {
         *p-- = static_cast<uint8>('0' + (j % 10));
         j /= 10;
      } while (j);

      if (value < 0)
         *p-- = '-';

      const size_t total_bytes = (buf + cBufSize - 1) - p;
      if (total_bytes > len)
         return false;

      for (size_t i = 0; i < total_bytes; i++)
         pDst[i] = p[1 + i];

      return true;
   }

   bool uint_to_string(uint value, char* pDst, uint len)
   {
      CRNLIB_ASSERT(pDst);

      const uint cBufSize = 16;
      char buf[cBufSize];

      char* p = buf + cBufSize - 1;

      *p-- = '\0';

      do
      {
         *p-- = static_cast<uint8>('0' + (value % 10));
         value /= 10;
      } while (value);

      const size_t total_bytes = (buf + cBufSize - 1) - p;
      if (total_bytes > len)
         return false;

      for (size_t i = 0; i < total_bytes; i++)
         pDst[i] = p[1 + i];

      return true;
   }

   bool string_to_int(const char*& pBuf, int& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const char* p = pBuf;

      while (*p && isspace(*p))
         p++;

      uint result = 0;
      bool negative = false;

      if (!isdigit(*p))
      {
         if (p[0] == '-')
         {
            negative = true;
            p++;
         }
         else
            return false;
      }

      while (*p && isdigit(*p))
      {
         if (result & 0xE0000000U)
            return false;

         const uint result8 = result << 3U;
         const uint result2 = result << 1U;

         if (result2 > (0xFFFFFFFFU - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - '0';
         if (c > (0xFFFFFFFFU - result))
            return false;

         result += c;

         p++;
      }

      if (negative)
      {
         if (result > 0x80000000U)
         {
            value = 0;
            return false;
         }
         value = -static_cast<int>(result);
      }
      else
      {
         if (result > 0x7FFFFFFFU)
         {
            value = 0;
            return false;
         }
         value = static_cast<int>(result);
      }

      pBuf = p;

      return true;
   }

   bool string_to_int(const wchar_t*& pBuf, int& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const wchar_t* p = pBuf;

      while (*p && isspace(*p))
         p++;

      uint result = 0;
      bool negative = false;

      if (!iswdigit(*p))
      {
         if (p[0] == '-')
         {
            negative = true;
            p++;
         }
         else
            return false;
      }

      while (*p && iswdigit(*p))
      {
         if (result & 0xE0000000U)
            return false;

         const uint result8 = result << 3U;
         const uint result2 = result << 1U;

         if (result2 > (0xFFFFFFFFU - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - L'0';
         if (c > (0xFFFFFFFFU - result))
            return false;

         result += c;

         p++;
      }

      if (negative)
      {
         if (result > 0x80000000U)
         {
            value = 0;
            return false;
         }
         value = -static_cast<int>(result);
      }
      else
      {
         if (result > 0x7FFFFFFFU)
         {
            value = 0;
            return false;
         }
         value = static_cast<int>(result);
      }

      pBuf = p;

      return true;
   }

   bool string_to_int64(const char*& pBuf, int64& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const char* p = pBuf;

      while (*p && isspace(*p))
         p++;

      uint64 result = 0;
      bool negative = false;

      if (!isdigit(*p))
      {
         if (p[0] == '-')
         {
            negative = true;
            p++;
         }
         else
            return false;
      }

      while (*p && isdigit(*p))
      {
         if (result & 0xE000000000000000ULL)
            return false;

         const uint64 result8 = result << 3U;
         const uint64 result2 = result << 1U;

         if (result2 > (0xFFFFFFFFFFFFFFFFULL - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - '0';
         if (c > (0xFFFFFFFFFFFFFFFFULL - result))
            return false;

         result += c;

         p++;
      }

      if (negative)
      {
         if (result > 0x8000000000000000ULL)
         {
            value = 0;
            return false;
         }
         value = -static_cast<int64>(result);
      }
      else
      {
         if (result > 0x7FFFFFFFFFFFFFFFULL)
         {
            value = 0;
            return false;
         }
         value = static_cast<int64>(result);
      }

      pBuf = p;

      return true;
   }

   bool string_to_uint(const char*& pBuf, uint& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const char* p = pBuf;

      while (*p && isspace(*p))
         p++;

      uint result = 0;

      if (!isdigit(*p))
         return false;

      while (*p && isdigit(*p))
      {
         if (result & 0xE0000000U)
            return false;

         const uint result8 = result << 3U;
         const uint result2 = result << 1U;

         if (result2 > (0xFFFFFFFFU - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - '0';
         if (c > (0xFFFFFFFFU - result))
            return false;

         result += c;

         p++;
      }

      value = result;

      pBuf = p;

      return true;
   }

   bool string_to_uint(const wchar_t*& pBuf, uint& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const wchar_t* p = pBuf;

      while (*p && iswspace(*p))
         p++;

      uint result = 0;

      if (!iswdigit(*p))
         return false;

      while (*p && iswdigit(*p))
      {
         if (result & 0xE0000000U)
            return false;

         const uint result8 = result << 3U;
         const uint result2 = result << 1U;

         if (result2 > (0xFFFFFFFFU - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - L'0';
         if (c > (0xFFFFFFFFU - result))
            return false;

         result += c;

         p++;
      }

      value = result;

      pBuf = p;

      return true;
   }

   bool string_to_uint64(const char*& pBuf, uint64& value)
   {
      value = 0;

      CRNLIB_ASSERT(pBuf);
      const char* p = pBuf;

      while (*p && isspace(*p))
         p++;

      uint64 result = 0;

      if (!isdigit(*p))
         return false;

      while (*p && isdigit(*p))
      {
         if (result & 0xE000000000000000ULL)
            return false;

         const uint64 result8 = result << 3U;
         const uint64 result2 = result << 1U;

         if (result2 > (0xFFFFFFFFFFFFFFFFULL - result8))
            return false;

         result = result8 + result2;

         uint c = p[0] - '0';
         if (c > (0xFFFFFFFFFFFFFFFFULL - result))
            return false;

         result += c;

         p++;
      }

      value = result;

      pBuf = p;

      return true;
   }

   bool string_to_bool(const char* p, bool& value)
   {
      CRNLIB_ASSERT(p);

      value = false;

      if (_stricmp(p, "false") == 0)
         return true;

      if (_stricmp(p, "true") == 0)
      {
         value = true;
         return true;
      }

      const char* q = p;
      uint v;
      if (string_to_uint(q, v))
      {
         if (!v)
            return true;
         else if (v == 1)
         {
            value = true;
            return true;
         }
      }

      return false;
   }

   bool string_to_bool(const wchar_t* p, bool& value)
   {
      CRNLIB_ASSERT(p);

      value = false;

      if (_wcsicmp(p, L"false") == 0)
         return true;

      if (_wcsicmp(p, L"true") == 0)
      {
         value = true;
         return true;
      }

      const wchar_t* q = p;
      uint v;
      if (string_to_uint(q, v))
      {
         if (!v)
            return true;
         else if (v == 1)
         {
            value = true;
            return true;
         }
      }

      return false;
   }

   bool string_to_float(const char*& p, float& value, uint round_digit)
   {
      CRNLIB_ASSERT(p);
      value = 0;

      enum { AF_BLANK = 1, AF_SIGN = 2, AF_DPOINT = 3, AF_BADCHAR = 4, AF_OVRFLOW = 5, AF_EXPONENT = 6, AF_NODIGITS = 7 };

      const char* buf = p;

      int status = 0;

      if (round_digit > 10)
         round_digit = 10;

      int got_sign_flag = 0;
      int got_dp_flag   = 0;
      int got_num_flag  = 0;

      int got_e_flag    = 0;
      int got_e_sign_flag = 0;
      int e_sign = 0;

      uint whole_count   = 0;
      uint frac_count    = 0;

      float whole         = 0;
      float frac          = 0;
      float scale         = 1;
      float exponent      = 1;

      while (*buf)
      {
         if (!isspace(*buf))
            break;

         buf++;
      }

      while (*buf)
      {
         int i = *buf++;

         switch (i)
         {
            case 'e':
            case 'E':
            {
              got_e_flag = 1;
              goto exit_while;
            }
            case '+':
            {
              if ((got_num_flag) || (got_sign_flag))
              {
                status = AF_SIGN;
                goto af_exit;
              }

              got_sign_flag = 1;

              break;
            }
            case '-':
            {
              if ((got_num_flag) || (got_sign_flag))
              {
                status = AF_SIGN;
                goto af_exit;
              }

              got_sign_flag = -1;

              break;
            }
            case '.':
            {
              if (got_dp_flag)
              {
                status = AF_DPOINT;
                goto af_exit;
              }

              got_dp_flag = 1;

              break;
            }
            default:
            {
              if ((i < '0') || (i > '9'))
                goto exit_while;
              else
              {
                i -= '0';

                got_num_flag = 1;

                if (got_dp_flag)
                {
                  if (frac_count < round_digit)
                  {
                    frac = frac * 10.0f + i;

                    scale = scale * 10.0f;
                  }
                  else if (frac_count == round_digit)
                  {
                    if (i >= 5)               /* check for round */
                      frac = frac + 1.0f;
                  }

                  frac_count++;
                }
                else
                {
                  whole = whole * 10.0f + i;

                  whole_count++;

                  if (whole > 1e+30f)
                  {
                    status = AF_OVRFLOW;
                    goto af_exit;
                  }
                }
              }

              break;
            }
         }
      }

    exit_while:

      if (got_e_flag)
      {
         if ((got_num_flag == 0) && (got_dp_flag))
         {
            status = AF_EXPONENT;
            goto af_exit;
         }

         int e = 0;
         e_sign = 1;
         got_num_flag = 0;
         got_e_sign_flag = 0;

         while (*buf)
         {
            int i = *buf++;

            if (i == '+')
            {
              if ((got_num_flag) || (got_e_sign_flag))
              {
                status = AF_EXPONENT;
                goto af_exit;
              }

              e_sign = 1;
              got_e_sign_flag = 1;
            }
            else if (i == '-')
            {
              if ((got_num_flag) || (got_e_sign_flag))
              {
                status = AF_EXPONENT;
                goto af_exit;
              }

              e_sign = -1;
              got_e_sign_flag = 1;
            }
            else if ((i >= '0') && (i <= '9'))
            {
              got_num_flag = 1;

              if ((e = (e * 10) + (i - 48)) > 16)
              {
                status = AF_EXPONENT;
                goto af_exit;
              }
            }
            else
              break;
         }

         for (int i = 1; i <= e; i++)   /* compute 10^e */
            exponent = exponent * 10.0f;
      }

      if (((whole_count + frac_count) == 0) && (got_e_flag == 0))
      {
         status = AF_NODIGITS;
         goto af_exit;
      }

      if (frac)
         whole = whole + (frac / scale);

      if (got_e_flag)
      {
         if (e_sign > 0)
            whole = whole * exponent;
         else
            whole = whole / exponent;
      }

      if (got_sign_flag < 0)
         whole = -whole;

      value = whole;
      p = buf;

   af_exit:
      return (status == 0);
   }

   bool string_to_float(const wchar_t*& p, float& value, uint round_digit)
   {
      CRNLIB_ASSERT(p);
      value = 0;

      enum { AF_BLANK = 1, AF_SIGN = 2, AF_DPOINT = 3, AF_BADCHAR = 4, AF_OVRFLOW = 5, AF_EXPONENT = 6, AF_NODIGITS = 7 };

      const wchar_t* buf = p;

      int status = 0;

      if (round_digit > 10)
         round_digit = 10;

      int got_sign_flag = 0;
      int got_dp_flag   = 0;
      int got_num_flag  = 0;

      int got_e_flag    = 0;
      int got_e_sign_flag = 0;
      int e_sign = 0;

      uint whole_count   = 0;
      uint frac_count    = 0;

      float whole         = 0;
      float frac          = 0;
      float scale         = 1;
      float exponent      = 1;

      while (*buf)
      {
         if (!iswspace(*buf))
            break;

         buf++;
      }

      while (*buf)
      {
         int i = *buf++;

         switch (i)
         {
            case L'e':
            case L'E':
            {
              got_e_flag = 1;
              goto exit_while;
            }
            case L'+':
            {
              if ((got_num_flag) || (got_sign_flag))
              {
                status = AF_SIGN;
                goto af_exit;
              }

              got_sign_flag = 1;

              break;
            }
            case L'-':
            {
              if ((got_num_flag) || (got_sign_flag))
              {
                status = AF_SIGN;
                goto af_exit;
              }

              got_sign_flag = -1;

              break;
            }
            case L'.':
            {
              if (got_dp_flag)
              {
                status = AF_DPOINT;
                goto af_exit;
              }

              got_dp_flag = 1;

              break;
            }
            default:
            {
              if ((i < L'0') || (i > L'9'))
                goto exit_while;
              else
              {
                i -= L'0';

                got_num_flag = 1;

                if (got_dp_flag)
                {
                  if (frac_count < round_digit)
                  {
                    frac = frac * 10.0f + i;

                    scale = scale * 10.0f;
                  }
                  else if (frac_count == round_digit)
                  {
                    if (i >= 5)               /* check for round */
                      frac = frac + 1.0f;
                  }

                  frac_count++;
                }
                else
                {
                  whole = whole * 10.0f + i;

                  whole_count++;

                  if (whole > 1e+30f)
                  {
                    status = AF_OVRFLOW;
                    goto af_exit;
                  }
                }
              }

              break;
            }
         }
      }

    exit_while:

      if (got_e_flag)
      {
         if ((got_num_flag == 0) && (got_dp_flag))
         {
            status = AF_EXPONENT;
            goto af_exit;
         }

         int e = 0;
         e_sign = 1;
         got_num_flag = 0;
         got_e_sign_flag = 0;

         while (*buf)
         {
            int i = *buf++;

            if (i == L'+')
            {
              if ((got_num_flag) || (got_e_sign_flag))
              {
                status = AF_EXPONENT;
                goto af_exit;
              }

              e_sign = 1;
              got_e_sign_flag = 1;
            }
            else if (i == L'-')
            {
              if ((got_num_flag) || (got_e_sign_flag))
              {
                status = AF_EXPONENT;
                goto af_exit;
              }

              e_sign = -1;
              got_e_sign_flag = 1;
            }
            else if ((i >= L'0') && (i <= L'9'))
            {
              got_num_flag = 1;

              if ((e = (e * 10) + (i - 48)) > 16)
              {
                status = AF_EXPONENT;
                goto af_exit;
              }
            }
            else
              break;
         }

         for (int i = 1; i <= e; i++)   /* compute 10^e */
            exponent = exponent * 10.0f;
      }

      if (((whole_count + frac_count) == 0) && (got_e_flag == 0))
      {
         status = AF_NODIGITS;
         goto af_exit;
      }

      if (frac)
         whole = whole + (frac / scale);

      if (got_e_flag)
      {
         if (e_sign > 0)
            whole = whole * exponent;
         else
            whole = whole / exponent;
      }

      if (got_sign_flag < 0)
         whole = -whole;

      value = whole;
      p = buf;

   af_exit:
      return (status == 0);
   }

   bool split_path(const char* p, dynamic_string* pDrive, dynamic_string* pDir, dynamic_string* pFilename, dynamic_string* pExt)
   {
      CRNLIB_ASSERT(p);

      char drive_buf[_MAX_DRIVE];
      char dir_buf[_MAX_DIR];
      char fname_buf[_MAX_FNAME];
      char ext_buf[_MAX_EXT];

#ifdef _MSC_VER
      errno_t error = _splitpath_s(p,
         pDrive      ? drive_buf : NULL, pDrive    ? _MAX_DRIVE  : 0,
         pDir        ? dir_buf   : NULL, pDir      ? _MAX_DIR    : 0,
         pFilename   ? fname_buf : NULL, pFilename ? _MAX_FNAME  : 0,
         pExt        ? ext_buf   : NULL, pExt      ? _MAX_EXT    : 0);
      if (error != 0)
         return false;
#else
      _splitpath(p,
         pDrive      ? drive_buf : NULL,
         pDir        ? dir_buf   : NULL,
         pFilename   ? fname_buf : NULL,
         pExt        ? ext_buf   : NULL);
#endif

      if (pDrive)    *pDrive = drive_buf;
      if (pDir)      *pDir = dir_buf;
      if (pFilename) *pFilename = fname_buf;
      if (pExt)      *pExt = ext_buf;

      return true;
   }

   bool split_path(const wchar_t* p, dynamic_wstring* pDrive, dynamic_wstring* pDir, dynamic_wstring* pFilename, dynamic_wstring* pExt)
   {
      CRNLIB_ASSERT(p);

      wchar_t drive_buf[_MAX_DRIVE];
      wchar_t dir_buf[_MAX_DIR];
      wchar_t fname_buf[_MAX_FNAME];
      wchar_t ext_buf[_MAX_EXT];

#ifdef _MSC_VER
      errno_t error = _wsplitpath_s(p,
         pDrive      ? drive_buf : NULL, pDrive    ? _MAX_DRIVE  : 0,
         pDir        ? dir_buf   : NULL, pDir      ? _MAX_DIR    : 0,
         pFilename   ? fname_buf : NULL, pFilename ? _MAX_FNAME  : 0,
         pExt        ? ext_buf   : NULL, pExt      ? _MAX_EXT    : 0);
      if (error != 0)
         return false;
#else
      _wsplitpath(p,
         pDrive      ? drive_buf : NULL,
         pDir        ? dir_buf   : NULL,
         pFilename   ? fname_buf : NULL,
         pExt        ? ext_buf   : NULL);
#endif

      if (pDrive)    *pDrive = drive_buf;
      if (pDir)      *pDir = dir_buf;
      if (pFilename) *pFilename = fname_buf;
      if (pExt)      *pExt = ext_buf;

      return true;
   }

   bool split_path(const char* p, dynamic_string& path, dynamic_string& filename)
   {
      dynamic_string temp_drive, temp_path, temp_ext;
      if (!split_path(p, &temp_drive, &temp_path, &filename, &temp_ext))
         return false;

      filename += temp_ext;

      combine_path(path, temp_drive.get_ptr(), temp_path.get_ptr());
      return true;
   }

   bool split_path(const wchar_t* p, dynamic_wstring& path, dynamic_wstring& filename)
   {
      dynamic_wstring temp_drive, temp_path, temp_ext;
      if (!split_path(p, &temp_drive, &temp_path, &filename, &temp_ext))
         return false;

      filename += temp_ext;

      combine_path(path, temp_drive.get_ptr(), temp_path.get_ptr());
      return true;
   }

   bool get_pathname(const char* p, dynamic_string& path)
   {
      dynamic_string temp_drive, temp_path;
      if (!split_path(p, &temp_drive, &temp_path, NULL, NULL))
         return false;

      combine_path(path, temp_drive.get_ptr(), temp_path.get_ptr());
      return true;
   }

   bool get_pathname(const wchar_t* p, dynamic_wstring& path)
   {
      dynamic_wstring temp_drive, temp_path;
      if (!split_path(p, &temp_drive, &temp_path, NULL, NULL))
         return false;

      combine_path(path, temp_drive.get_ptr(), temp_path.get_ptr());
      return true;
   }

   bool get_filename(const char* p, dynamic_string& filename)
   {
      dynamic_string temp_ext;
      if (!split_path(p, NULL, NULL, &filename, &temp_ext))
         return false;

      filename += temp_ext;
      return true;
   }

   bool get_filename(const wchar_t* p, dynamic_wstring& filename)
   {
      dynamic_wstring temp_ext;
      if (!split_path(p, NULL, NULL, &filename, &temp_ext))
         return false;

      filename += temp_ext;
      return true;
   }

   void combine_path(dynamic_string& dst, const char* pA, const char* pB)
   {
      dynamic_string temp;
      temp = pA;
      if ((!temp.is_empty()) && (pB[0] != '\\') && (pB[0] != '/'))
      {
         char c = temp[temp.get_len() - 1];
         if ((c != '\\') && (c != '/'))
         {
            temp.append_char('\\');
         }
      }
      temp += pB;
      dst.swap(temp);
   }

   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB)
   {
      dynamic_wstring temp;
      temp = pA;
      if ((!temp.is_empty()) && (pB[0] != L'\\') && (pB[0] != L'/'))
      {
         wchar_t c = temp[temp.get_len() - 1];
         if ((c != L'\\') && (c != L'/'))
         {
            temp.append_char(L'\\');
         }
      }
      temp += pB;
      dst.swap(temp);
   }

   void combine_path(dynamic_string& dst, const char* pA, const char* pB, const char* pC)
   {
      combine_path(dst, pA, pB);
      combine_path(dst, dst.get_ptr(), pC);
   }

   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB, const wchar_t* pC)
   {
      combine_path(dst, pA, pB);
      combine_path(dst, dst.get_ptr(), pC);
   }

   void combine_path(dynamic_wstring& dst, const wchar_t* pA, const wchar_t* pB, const wchar_t* pC, const wchar_t *pD)
   {
      combine_path(dst, pA, pB);
      combine_path(dst, dst.get_ptr(), pC);
      combine_path(dst, dst.get_ptr(), pD);
   }

   bool full_path(dynamic_string& path)
   {
#ifndef _XBOX
      char buf[CRNLIB_MAX_PATH];

      char* p = _fullpath(buf, path.get_ptr(), CRNLIB_MAX_PATH);
      if (!p)
         return false;

      path.set(buf);
#endif
      return true;
   }

   bool full_path(dynamic_wstring& path)
   {
#ifndef _XBOX
      wchar_t buf[CRNLIB_MAX_PATH];

      wchar_t* p = _wfullpath(buf, path.get_ptr(), CRNLIB_MAX_PATH);
      if (!p)
         return false;

      path.set(buf);
#endif
      return true;
   }

   bool get_extension(dynamic_string& filename)
   {
      int sep = filename.find_right('\\');
      if (sep < 0)
         sep = filename.find_right('/');

      int dot = filename.find_right('.');
      if (dot < sep)
      {
         filename.clear();
         return false;
      }

      filename.right(dot + 1);

      return true;
   }

   bool get_extension(dynamic_wstring& filename)
   {
      int sep = filename.find_right(L'\\');
      if (sep < 0)
         sep = filename.find_right(L'/');

      int dot = filename.find_right(L'.');
      if (dot < sep)
      {
         filename.clear();
         return false;
      }

      filename.right(dot + 1);

      return true;
   }

   bool remove_extension(dynamic_string& filename)
   {
      int sep = filename.find_right('\\');
      if (sep < 0)
         sep = filename.find_right('/');

      int dot = filename.find_right('.');
      if (dot < sep)
         return false;

      filename.left(dot);

      return true;
   }

   bool remove_extension(dynamic_wstring& filename)
   {
      int sep = filename.find_right(L'\\');
      if (sep < 0)
         sep = filename.find_right(L'/');

      int dot = filename.find_right(L'.');
      if (dot < sep)
         return false;

      filename.left(dot);

      return true;
   }

   bool create_path(const dynamic_wstring& path)
   {
      bool unc = false;
      dynamic_wstring cur_path;

      const int l = path.get_len();

      int n = 0;
      while (n < l)
      {
         const wchar_t c = path.get_ptr()[n];

         const bool sep = (c == L'/') || (c == L'\\');

         if ((sep) || (n == (l - 1)))
         {
            if ((n == (l - 1)) && (!sep))
               cur_path.append_char(c);

            bool valid = false;
            if ((cur_path.get_len() > 3) && (cur_path.get_ptr()[1] == L':'))
               valid = true;
            else if (cur_path.get_len() > 2)
            {
               if (unc)
                  valid = true;
               unc = true;
            }

            if (valid)
               _wmkdir(cur_path.get_ptr());
         }

         cur_path.append_char(c);

         n++;
      }

      return true;
   }

   void trim_trailing_seperator(dynamic_wstring& path)
   {
      if ( (path.get_len()) && ( (path[path.get_len() - 1] == L'\\') || (path[path.get_len() - 1] == L'/') ) )
         path.truncate(path.get_len() - 1);
   }

} // namespace crnlib



