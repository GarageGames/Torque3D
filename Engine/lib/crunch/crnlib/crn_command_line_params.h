// File: crn_command_line_params.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_value.h"
#include <map>

namespace crnlib
{
   class command_line_params
   {
   public:
      struct param_value
      {
         param_value() : m_index(0), m_modifier(0) { }
                           
         dynamic_wstring_array   m_values;
         uint                    m_index;
         int8                    m_modifier;
      };
      
      typedef std::multimap<dynamic_wstring, param_value>   param_map;
      typedef param_map::const_iterator                     param_map_const_iterator;
      typedef param_map::iterator                           param_map_iterator;
      
      command_line_params();
      
      void clear();
      
      static bool split_params(const wchar_t* p, dynamic_wstring_array& params);
                        
      struct param_desc
      {
         const wchar_t* m_pName;
         uint           m_num_values;
         bool           m_support_listing_file;
      };
      
      bool parse(const dynamic_wstring_array& params, uint n, const param_desc* pParam_desc);
      bool parse(const wchar_t* pCmd_line, uint n, const param_desc* pParam_desc, bool skip_first_param = true);
      
      const dynamic_wstring_array& get_array() const { return m_params; }
      
      bool is_param(uint index) const;
      
      const param_map& get_map() const { return m_param_map; }
      
      uint get_num_params() const { return static_cast<uint>(m_param_map.size()); }
                  
      param_map_const_iterator begin() const { return m_param_map.begin(); }
      param_map_const_iterator end() const { return m_param_map.end(); }
            
      uint find(uint num_keys, const wchar_t** ppKeys, crnlib::vector<param_map_const_iterator>* pIterators, crnlib::vector<uint>* pUnmatched_indices) const;
      
      void find(const wchar_t* pKey, param_map_const_iterator& begin, param_map_const_iterator& end) const;
                        
      uint get_count(const wchar_t* pKey) const;
      
      // Returns end() if param cannot be found, or index is out of range.
      param_map_const_iterator get_param(const wchar_t* pKey, uint index) const;
      
      bool has_key(const wchar_t* pKey) const { return get_param(pKey, 0) != end(); }
                              
      bool has_value(const wchar_t* pKey, uint index) const;
      uint get_num_values(const wchar_t* pKey, uint index) const;
      
      bool get_value_as_bool(const wchar_t* pKey, uint index = 0, bool def = false) const;	
      
      int get_value_as_int(const wchar_t* pKey, uint index, int def, int l = INT_MIN, int h = INT_MAX, uint value_index = 0) const;	
      float get_value_as_float(const wchar_t* pKey, uint index, float def = 0.0f, float l = -math::cNearlyInfinite, float h = math::cNearlyInfinite, uint value_index = 0) const;
      
      bool get_value_as_string(const wchar_t* pKey, uint index, dynamic_wstring& value, uint value_index = 0) const;
      const dynamic_wstring& get_value_as_string_or_empty(const wchar_t* pKey, uint index = 0, uint value_index = 0) const;
                                    
   private:
      dynamic_wstring_array   m_params;
      
      param_map               m_param_map;
      
      static bool load_string_file(const wchar_t* pFilename, dynamic_wstring_array& strings);
   };

} // namespace crnlib
