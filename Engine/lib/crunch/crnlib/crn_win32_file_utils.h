// File: crn_win32_file_utils.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   struct win32_file_utils
   {
      static bool does_file_exist(const wchar_t* pFilename);
      static bool does_dir_exist(const wchar_t* pDir);
      static bool get_file_size(const wchar_t* pFilename, uint64& file_size);
      static bool get_file_size(const wchar_t* pFilename, uint32& file_size);
   };

} // namespace crnlib
