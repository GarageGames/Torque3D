// File: crn_types.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once

namespace crnlib
{
   typedef unsigned char      uint8;
   typedef signed char        int8;
   typedef unsigned short     uint16;
   typedef signed short       int16;
   typedef unsigned int       uint32;
   typedef uint32             uint;
   typedef signed int         int32;

   typedef unsigned __int64   uint64;
   typedef signed __int64     int64;

   const uint8  UINT8_MIN  = 0;
   const uint8  UINT8_MAX  = 0xFFU;
   const uint16 UINT16_MIN = 0;
   const uint16 UINT16_MAX = 0xFFFFU;
   const uint32 UINT32_MIN = 0;
   const uint32 UINT32_MAX = 0xFFFFFFFFU;
   const uint64 UINT64_MIN = 0;
   const uint64 UINT64_MAX = 0xFFFFFFFFFFFFFFFFULL; //0xFFFFFFFFFFFFFFFFui64;

   const int8  INT8_MIN  = -128;
   const int8  INT8_MAX  = 127;
   const int16 INT16_MIN = -32768;
   const int16 INT16_MAX = 32767;
   const int32 INT32_MIN = (-2147483647 - 1);
   const int32 INT32_MAX = 2147483647;
   const int64 INT64_MIN = (int64)0x8000000000000000ULL; //(-9223372036854775807i64 - 1);
   const int64 INT64_MAX = (int64)0x7FFFFFFFFFFFFFFFULL; // 9223372036854775807i64;

#ifdef CRNLIB_PLATFORM_PC_X64
   typedef unsigned __int64 uint_ptr;
   typedef unsigned __int64 uint32_ptr;
   typedef signed __int64 signed_size_t;
   typedef uint64 ptr_bits_t;
#else
   typedef unsigned int uint_ptr;
   typedef unsigned int uint32_ptr;
   typedef signed int signed_size_t;
   typedef uint32 ptr_bits_t;
#endif

   enum eVarArg { cVarArg };
   enum eClear { cClear };
   enum eNoClamp { cNoClamp };
   enum { cInvalidIndex = -1 };

   const uint cIntBits = 32;

   struct empty_type { };

} // namespace crnlib
