// File: crn_value.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_value.h"

namespace crnlib
{
   const wchar_t* gValueDataTypeStrings[cDTTotal + 1] = 
   {
      L"invalid",
      L"string",
      L"bool",
      L"int",
      L"uint",
      L"float",
      L"vec3f",
      L"vec3i",
      
      NULL,
   };

} // namespace crnlib
