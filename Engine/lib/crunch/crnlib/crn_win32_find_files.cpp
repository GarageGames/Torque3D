// File: crn_win32_find_files.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_win32_find_files.h"
#include "crn_win32_file_utils.h"
#include "crn_strutils.h"

namespace crnlib
{
   bool find_files::find(const wchar_t* pBasepath, const wchar_t* pFilespec, uint flags)
   {
      m_last_error = S_OK;
      m_files.resize(0);

      return find_internal(pBasepath, L"", pFilespec, flags);
   }
   
   bool find_files::find(const wchar_t* pSpec, uint flags)
   {
      dynamic_wstring find_name(pSpec);

      if (!full_path(find_name))
         return false;  

      dynamic_wstring find_pathname, find_filename;
      if (!split_path(find_name.get_ptr(), find_pathname, find_filename))
         return false;

      return find(find_pathname.get_ptr(), find_filename.get_ptr(), flags);
   }      
   
   bool find_files::find_internal(const wchar_t* pBasepath, const wchar_t* pRelpath, const wchar_t* pFilespec, uint flags)
   {
      WIN32_FIND_DATAW find_data;

      dynamic_wstring filename;
            
      dynamic_wstring_array child_paths;
      if (flags & cFlagRecursive)
      {
         if (wcslen(pRelpath))
            combine_path(filename, pBasepath, pRelpath, L"*");
         else
            combine_path(filename, pBasepath, L"*");
      
         HANDLE handle = FindFirstFileW(filename.get_ptr(), &find_data);
         if (handle == INVALID_HANDLE_VALUE)
         {
            HRESULT hres = GetLastError();
            if ((hres != NO_ERROR) && (hres != ERROR_FILE_NOT_FOUND))
            {
               m_last_error = hres;
               return false;
            }
         }
         else
         {
            do 
            {
               const bool is_dir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

               bool skip = !is_dir;
               if (is_dir)
                  skip = (wcscmp(find_data.cFileName, L".") == 0) || (wcscmp(find_data.cFileName, L"..") == 0);

               if (find_data.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY))
                  skip = true;

               if (find_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
               {
                  if ((flags & cFlagAllowHidden) == 0)
                     skip = true;
               }

               if (!skip)
               {
                  dynamic_wstring child_path(find_data.cFileName);
                  if ((!child_path.count_char(L'?')) && (!child_path.count_char(L'*')))
                     child_paths.push_back(child_path);
               }

            } while (FindNextFileW(handle, &find_data) != 0);
                                    
            HRESULT hres = GetLastError();
            
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
            
            if (hres != ERROR_NO_MORE_FILES) 
            {
               m_last_error = hres;
               return false;
            }
         }            
      }
      
      if (wcslen(pRelpath))
         combine_path(filename, pBasepath, pRelpath, pFilespec);
      else
         combine_path(filename, pBasepath, pFilespec);
               
      HANDLE handle = FindFirstFileW(filename.get_ptr(), &find_data);
      if (handle == INVALID_HANDLE_VALUE)
      {
         HRESULT hres = GetLastError();
         if ((hres != NO_ERROR) && (hres != ERROR_FILE_NOT_FOUND))
         {
            m_last_error = hres;
            return false;
         }
      }
      else
      {
         do 
         {
            const bool is_dir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            bool skip = false;
            if (is_dir)
               skip = (wcscmp(find_data.cFileName, L".") == 0) || (wcscmp(find_data.cFileName, L"..") == 0);

            if (find_data.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY))
               skip = true;

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            {
               if ((flags & cFlagAllowHidden) == 0)
                  skip = true;
            }

            if (!skip)
            {
               if (((is_dir) && (flags & cFlagAllowDirs)) || ((!is_dir) && (flags & cFlagAllowFiles)))
               {
                  m_files.resize(m_files.size() + 1);
                  file_desc& file = m_files.back();
                  file.m_is_dir = is_dir;
                  file.m_base = pBasepath;
                  file.m_name = find_data.cFileName;
                  file.m_rel = pRelpath;
                  if (wcslen(pRelpath))
                     combine_path(file.m_fullname, pBasepath, pRelpath, find_data.cFileName);
                  else
                     combine_path(file.m_fullname, pBasepath, find_data.cFileName);
               }
            }

         } while (FindNextFileW(handle, &find_data) != 0);

         HRESULT hres = GetLastError();
         
         FindClose(handle);
         
         if (hres != ERROR_NO_MORE_FILES)
         {
            m_last_error = hres;
            return false;
         }
      }         
      
      for (uint i = 0; i < child_paths.size(); i++)
      {
         dynamic_wstring child_path;
         if (wcslen(pRelpath))
            combine_path(child_path, pRelpath, child_paths[i].get_ptr());
         else
            child_path = child_paths[i];

         if (!find_internal(pBasepath, child_path.get_ptr(), pFilespec, flags))
            return false;
      }
      
      return true;
   }

} // namespace crnlib
