// File: crn_win32_file_utils.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_win32_file_utils.h"
#include "crn_winhdr.h"

namespace crnlib
{
   bool win32_file_utils::does_file_exist(const wchar_t* pFilename)
   {
      const DWORD fullAttributes = GetFileAttributesW(pFilename);

      if (fullAttributes == INVALID_FILE_ATTRIBUTES) 
         return false;

      if (fullAttributes & FILE_ATTRIBUTE_DIRECTORY)
         return false;

      return true;
   }

   bool win32_file_utils::does_dir_exist(const wchar_t* pDir)
   {
      //-- Get the file attributes.
      DWORD fullAttributes = GetFileAttributesW(pDir);

      if (fullAttributes == INVALID_FILE_ATTRIBUTES)
         return false;

      if (fullAttributes & FILE_ATTRIBUTE_DIRECTORY)
         return true;

      return false;
   }

   bool win32_file_utils::get_file_size(const wchar_t* pFilename, uint64& file_size)
   {
      file_size = 0;

      WIN32_FILE_ATTRIBUTE_DATA attr;

      if (0 == GetFileAttributesExW(pFilename, GetFileExInfoStandard, &attr))
         return false;

      if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         return false;

      file_size = static_cast<uint64>(attr.nFileSizeLow) | (static_cast<uint64>(attr.nFileSizeHigh) << 32U);

      return true;
   }   
   
   bool win32_file_utils::get_file_size(const wchar_t* pFilename, uint32& file_size)
   {
      uint64 file_size64;
      if (!get_file_size(pFilename, file_size64))
      {
         file_size = 0;
         return false;
      }

      if (file_size64 > UINT32_MAX)
         file_size64 = UINT32_MAX;

      file_size = static_cast<uint32>(file_size64);
      return true;
   }
 
} // namespace crnlib
