// File: crn_win32_find_files.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_winhdr.h"

namespace crnlib
{
   class find_files
   {
   public:
      struct file_desc
      {
         inline file_desc() : m_is_dir(false) { }
         
         dynamic_wstring m_fullname;
         dynamic_wstring m_base;
         dynamic_wstring m_rel;
         dynamic_wstring m_name;
         bool            m_is_dir;
         
         inline bool operator== (const file_desc& other) const { return m_fullname == other.m_fullname; }
         inline bool operator< (const file_desc& other) const { return m_fullname < other.m_fullname; }
         
         inline operator size_t() const { return static_cast<size_t>(m_fullname); }
      };

      typedef crnlib::vector<file_desc> file_desc_vec;

      find_files() : m_last_error(S_OK) { }

      enum flags
      {
         cFlagRecursive = 1,
         cFlagAllowDirs = 2,
         cFlagAllowFiles = 4,
         cFlagAllowHidden = 8
      };

      bool find(const wchar_t* pBasepath, const wchar_t* pFilespec, uint flags = cFlagAllowFiles);
      
      bool find(const wchar_t* pSpec, uint flags = cFlagAllowFiles);
      
      inline HRESULT get_last_error() const { return m_last_error; }
      
      const file_desc_vec& get_files() const { return m_files; }
      
   private:
      file_desc_vec m_files;
      HRESULT m_last_error;

      bool find_internal(const wchar_t* pBasepath, const wchar_t* pRelpath, const wchar_t* pFilespec, uint flags);

   }; // class find_files

} // namespace crnlib
