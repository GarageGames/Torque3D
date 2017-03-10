// File: crn_command_line_params.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_command_line_params.h"
#include "crn_console.h"
#include "crn_cfile_stream.h"

namespace crnlib
{
   command_line_params::command_line_params()
   {
   }
   
   void command_line_params::clear()
   {
      m_params.clear();
      
      m_param_map.clear();
   }
   
   bool command_line_params::split_params(const wchar_t* p, dynamic_wstring_array& params)
   {
      bool within_param = false;
      bool within_quote = false;
      
      uint ofs = 0;
      dynamic_wstring str;
      
      while (p[ofs])
      {
         const wchar_t c = p[ofs];
         
         if (within_param)
         {
            if (within_quote)
            {
               if (c == L'"')
                  within_quote = false;
               
               str.append_char(c);
            }
            else if ((c == L' ') || (c == L'\t'))
            {
               if (!str.is_empty())
               {
                  params.push_back(str);
                  str.clear();
               }
               within_param = false;
            }
            else
            {
               if (c == L'"')
                  within_quote = true;
                  
               str.append_char(c);
            }
         }
         else if ((c != L' ') && (c != L'\t'))
         {
            within_param = true;
            
            if (c == L'"')
               within_quote = true;
            
            str.append_char(c);
         }
         
         ofs++;
      }
      
      if (within_quote)
      {
         console::error(L"Unmatched quote in command line \"%s\"", p);
         return false;
      }
      
      if (!str.is_empty())
         params.push_back(str);
      
      return true;
   }
   
   bool command_line_params::load_string_file(const wchar_t* pFilename, dynamic_wstring_array& strings)
   {
      cfile_stream in_stream;
      if (!in_stream.open(pFilename, cDataStreamReadable | cDataStreamSeekable))
      {
         console::error(L"Unable to open file \"%s\" for reading!", pFilename);
         return false;
      }
      
      dynamic_string ansi_str;
      
      for ( ; ; )
      {
         if (!in_stream.read_line(ansi_str))
            break;
         
         ansi_str.trim();
         if (ansi_str.is_empty())
            continue;
       
         strings.push_back(dynamic_wstring(ansi_str.get_ptr()));
      }
      
      return true;
   }
   
   bool command_line_params::parse(const dynamic_wstring_array& params, uint n, const param_desc* pParam_desc)
   {
      CRNLIB_ASSERT(n && pParam_desc);
      
      m_params = params;
      
      uint arg_index = 0;
      while (arg_index < params.size())
      {
         const uint cur_arg_index = arg_index;
         const dynamic_wstring& src_param = params[arg_index++];
               
         if (src_param.is_empty())
            continue;
                           
         if ((src_param[0] == L'/') || (src_param[0] == L'-'))
         {
            if (src_param.get_len() < 2)
            {
               console::error(L"Invalid command line parameter: \"%s\"", src_param.get_ptr());
               return false;
            }
            
            dynamic_wstring key_str(src_param);
            
            key_str.right(1);
            
            int modifier = 0;
            wchar_t c = key_str[key_str.get_len() - 1];
            if (c == L'+')
               modifier = 1;
            else if (c == L'-') 
               modifier = -1;
               
            if (modifier)
               key_str.left(key_str.get_len() - 1);
                        
            uint param_index;
            for (param_index = 0; param_index < n; param_index++)
               if (key_str == pParam_desc[param_index].m_pName)
                  break;
                  
            if (param_index == n)
            {
               console::error(L"Unrecognized command line parameter: \"%s\"", src_param.get_ptr());
               return false;  
            }
            
            const param_desc& desc = pParam_desc[param_index];
            
            const uint cMaxValues = 16;
            dynamic_wstring val_str[cMaxValues];
            uint num_val_strs = 0;
            if (desc.m_num_values) 
            {  
               CRNLIB_ASSERT(desc.m_num_values <= cMaxValues);
               
               if ((arg_index + desc.m_num_values) > params.size())
               {
                  console::error(L"Expected %u value(s) after command line parameter: \"%s\"", desc.m_num_values, src_param.get_ptr());
                  return false;  
               }
               
               for (uint v = 0; v < desc.m_num_values; v++)
                  val_str[num_val_strs++] = params[arg_index++];
            }             
            
            dynamic_wstring_array strings;
            
            if ((desc.m_support_listing_file) && (val_str[0].get_len() >= 2) && (val_str[0][0] == L'@'))
            {
               dynamic_wstring filename(val_str[0]);
               filename.right(1);
               filename.unquote();
            
               if (!load_string_file(filename.get_ptr(), strings))
               {
                  console::error(L"Failed loading listing file \"%s\"!", filename.get_ptr());
                  return false;
               }
            }
            else
            {
               for (uint v = 0; v < num_val_strs; v++)
               {
                  val_str[v].unquote();
                  strings.push_back(val_str[v]);
               }
            }
            
            param_value pv;
            pv.m_values.swap(strings);
            pv.m_index = cur_arg_index;
            pv.m_modifier = (int8)modifier;
            m_param_map.insert(std::make_pair(key_str, pv));
         }
         else
         {
            param_value pv;
            pv.m_values.push_back(src_param);
            pv.m_values.back().unquote();
            pv.m_index = cur_arg_index;
            m_param_map.insert(std::make_pair(g_empty_dynamic_wstring, pv));
         }
      }

      return true;
   }
   
   bool command_line_params::parse(const wchar_t* pCmd_line, uint n, const param_desc* pParam_desc, bool skip_first_param)
   {
      CRNLIB_ASSERT(n && pParam_desc);
   
      dynamic_wstring_array p;
      if (!split_params(pCmd_line, p))
         return 0;

      if (p.empty())
         return 0;

      if (skip_first_param)
         p.erase(0U);

      return parse(p, n, pParam_desc);
   }
   
   bool command_line_params::is_param(uint index) const
   {
      CRNLIB_ASSERT(index < m_params.size());
      if (index >= m_params.size())
         return false;

      const dynamic_wstring& w = m_params[index];
      if (w.is_empty())
         return false;

      return (w.get_len() >= 2) && ((w[0] == L'-') || (w[0] == L'/'));
   }
   
   uint command_line_params::find(uint num_keys, const wchar_t** ppKeys, crnlib::vector<param_map_const_iterator>* pIterators, crnlib::vector<uint>* pUnmatched_indices) const
   {
      CRNLIB_ASSERT(ppKeys);
      
      if (pUnmatched_indices)
      {
         pUnmatched_indices->resize(m_params.size());
         for (uint i = 0; i < m_params.size(); i++)
            (*pUnmatched_indices)[i] = i;
      }
      
      uint n = 0;
      for (uint i = 0; i < num_keys; i++)
      {
         const wchar_t* pKey = ppKeys[i];

         param_map_const_iterator begin, end;
         find(pKey, begin, end);
         
         while (begin != end)
         {
            if (pIterators)   
               pIterators->push_back(begin);
            
            if (pUnmatched_indices)
            {
               int k = pUnmatched_indices->find(begin->second.m_index);
               if (k >= 0)
                  pUnmatched_indices->erase_unordered(k);
            }
               
            n++;
            begin++;
         }
      }
   
      return n;
   }

   void command_line_params::find(const wchar_t* pKey, param_map_const_iterator& begin, param_map_const_iterator& end) const
   {
      dynamic_wstring key(pKey);
      begin = m_param_map.lower_bound(key);
      end = m_param_map.upper_bound(key);
   }

   uint command_line_params::get_count(const wchar_t* pKey) const
   {
      param_map_const_iterator begin, end;
      find(pKey, begin, end);

      uint n = 0;
      
      while (begin != end)
      {
         n++;
         begin++;
      }
      
      return n;
   }
   
   command_line_params::param_map_const_iterator command_line_params::get_param(const wchar_t* pKey, uint index) const
   {
      param_map_const_iterator begin, end;
      find(pKey, begin, end);
      
      if (begin == end)
         return m_param_map.end();

      uint n = 0;
      
      while ((begin != end) && (n != index))
      {
         n++;
         begin++;
      }
      
      if (begin == end)
         return m_param_map.end();

      return begin;
   }

   bool command_line_params::has_value(const wchar_t* pKey, uint index) const
   {
      return get_num_values(pKey, index) != 0;
   }
   
   uint command_line_params::get_num_values(const wchar_t* pKey, uint index) const
   {
      param_map_const_iterator it = get_param(pKey, index);

      if (it == end())
         return 0;

      return it->second.m_values.size();
   }
   
   bool command_line_params::get_value_as_bool(const wchar_t* pKey, uint index, bool def) const
   {
      param_map_const_iterator it = get_param(pKey, index);
      if (it == end())
         return def;
      
      if (it->second.m_modifier)
         return it->second.m_modifier > 0;
      else
         return true;
   }
   
   int command_line_params::get_value_as_int(const wchar_t* pKey, uint index, int def, int l, int h, uint value_index) const
   {
      param_map_const_iterator it = get_param(pKey, index);
      if ((it == end()) || (value_index >= it->second.m_values.size()))
         return def;

      int val;
      const wchar_t* p = it->second.m_values[value_index].get_ptr();
      if (!string_to_int(p, val))
      {
         crnlib::console::warning(L"Invalid value specified for parameter \"%s\", using default value of %i", pKey, def);
         return def;
      }

      if (val < l)
      {
         crnlib::console::warning(L"Value %i for parameter \"%s\" is out of range, clamping to %i", val, pKey, l);
         val = l;
      }
      else if (val > h)
      {
         crnlib::console::warning(L"Value %i for parameter \"%s\" is out of range, clamping to %i", val, pKey, h);
         val = h;
      }
      
      return val;
   }
   
   float command_line_params::get_value_as_float(const wchar_t* pKey, uint index, float def, float l, float h, uint value_index) const
   {
      param_map_const_iterator it = get_param(pKey, index);
      if ((it == end()) || (value_index >= it->second.m_values.size()))
         return def;

      float val;
      const wchar_t* p = it->second.m_values[value_index].get_ptr();
      if (!string_to_float(p, val))
      {
         crnlib::console::warning(L"Invalid value specified for float parameter \"%s\", using default value of %f", pKey, def);
         return def;
      }

      if (val < l)
      {
         crnlib::console::warning(L"Value %f for parameter \"%s\" is out of range, clamping to %f", val, pKey, l);
         val = l;
      }
      else if (val > h)
      {
         crnlib::console::warning(L"Value %f for parameter \"%s\" is out of range, clamping to %f", val, pKey, h);
         val = h;
      }

      return val;
   }
   
   bool command_line_params::get_value_as_string(const wchar_t* pKey, uint index, dynamic_wstring& value, uint value_index) const
   {
      param_map_const_iterator it = get_param(pKey, index);
      if ((it == end()) || (value_index >= it->second.m_values.size()))
      {
         value.empty();
         return false;
      }

      value = it->second.m_values[value_index];
      return true;
   }
   
   const dynamic_wstring& command_line_params::get_value_as_string_or_empty(const wchar_t* pKey, uint index, uint value_index) const
   {
      param_map_const_iterator it = get_param(pKey, index);
      if ((it == end()) || (value_index >= it->second.m_values.size()))
      return g_empty_dynamic_wstring;
      
      return it->second.m_values[value_index];
   }

} // namespace crnlib

