// File: crn_mem.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_spinlock.h"
#include "crn_console.h"
#include "../inc/crnlib.h"
#include <malloc.h>
#include "crn_winhdr.h"

#define CRNLIB_MEM_STATS 0

#ifndef CRNLIB_USE_WIN32_API
#define _msize malloc_usable_size
#endif

namespace crnlib
{
#if CRNLIB_MEM_STATS
   #if CRNLIB_64BIT_POINTERS
      typedef LONGLONG mem_stat_t;
      #define CRNLIB_MEM_COMPARE_EXCHANGE InterlockedCompareExchange64
   #else
      typedef LONG mem_stat_t;
      #define CRNLIB_MEM_COMPARE_EXCHANGE InterlockedCompareExchange
   #endif

   static volatile mem_stat_t g_total_blocks;
   static volatile mem_stat_t g_total_allocated;
   static volatile mem_stat_t g_max_allocated;

   static mem_stat_t update_total_allocated(int block_delta, mem_stat_t byte_delta)
   {
      mem_stat_t cur_total_blocks;
      for ( ; ; )
      {
         cur_total_blocks = (mem_stat_t)g_total_blocks;
         mem_stat_t new_total_blocks = static_cast<mem_stat_t>(cur_total_blocks + block_delta);
         CRNLIB_ASSERT(new_total_blocks >= 0);
         if (CRNLIB_MEM_COMPARE_EXCHANGE(&g_total_blocks, new_total_blocks, cur_total_blocks) == cur_total_blocks)
            break;
      }

      mem_stat_t cur_total_allocated, new_total_allocated;
      for ( ; ; )
      {
         cur_total_allocated = g_total_allocated;
         new_total_allocated = static_cast<mem_stat_t>(cur_total_allocated + byte_delta);
         CRNLIB_ASSERT(new_total_allocated >= 0);
         if (CRNLIB_MEM_COMPARE_EXCHANGE(&g_total_allocated, new_total_allocated, cur_total_allocated) == cur_total_allocated)
            break;
      }
      for ( ; ; )
      {
         mem_stat_t cur_max_allocated = g_max_allocated;
         mem_stat_t new_max_allocated = CRNLIB_MAX(new_total_allocated, cur_max_allocated);
         if (CRNLIB_MEM_COMPARE_EXCHANGE(&g_max_allocated, new_max_allocated, cur_max_allocated) == cur_max_allocated)
            break;
      }
      return new_total_allocated;
   }
#endif // CRNLIB_MEM_STATS
   
   static void* crnlib_default_realloc(void* p, size_t size, size_t* pActual_size, bool movable, void* pUser_data)
   {
      pUser_data;

      void* p_new;

      if (!p)
      {
         p_new = ::malloc(size);
         CRNLIB_ASSERT( (reinterpret_cast<ptr_bits_t>(p_new) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1)) == 0 );

         if (pActual_size)
            *pActual_size = p_new ? ::_msize(p_new) : 0;
      }
      else if (!size)
      {
         ::free(p);
         p_new = NULL;

         if (pActual_size)
            *pActual_size = 0;
      }
      else
      {
         void* p_final_block = p;
#ifdef WIN32
         p_new = ::_expand(p, size);
#else

         p_new = NULL;
#endif

         if (p_new)
         {
            CRNLIB_ASSERT( (reinterpret_cast<ptr_bits_t>(p_new) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1)) == 0 );
            p_final_block = p_new;
         }
         else if (movable)
         {
            p_new = ::realloc(p, size);

            if (p_new)
            {
               CRNLIB_ASSERT( (reinterpret_cast<ptr_bits_t>(p_new) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1)) == 0 );
               p_final_block = p_new;
            }
         }

         if (pActual_size)
            *pActual_size = ::_msize(p_final_block);
      }

      return p_new;
   }

   static size_t crnlib_default_msize(void* p, void* pUser_data)
   {
      pUser_data;
      return p ? _msize(p) : 0;
   }

   static crn_realloc_func    g_pRealloc = crnlib_default_realloc;
   static crn_msize_func      g_pMSize   = crnlib_default_msize;
   static void*               g_pUser_data;
   
   void crnlib_mem_error(const char* p_msg)
   {
      crnlib_assert(p_msg, __FILE__, __LINE__);
   }
   
   void* crnlib_malloc(size_t size, size_t* pActual_size)
   {
      size = (size + sizeof(uint32) - 1U) & ~(sizeof(uint32) - 1U);
      if (!size)
         size = sizeof(uint32);

      if (size > CRNLIB_MAX_POSSIBLE_BLOCK_SIZE)
      {
         crnlib_mem_error("crnlib_malloc: size too big");
         return NULL;
      }

      size_t actual_size = size;
      uint8* p_new = static_cast<uint8*>((*g_pRealloc)(NULL, size, &actual_size, true, g_pUser_data));

      if (pActual_size)
         *pActual_size = actual_size;

      if ((!p_new) || (actual_size < size))
      {
         crnlib_mem_error("crnlib_malloc: out of memory");
         return NULL;
      }

      CRNLIB_ASSERT((reinterpret_cast<ptr_bits_t>(p_new) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1)) == 0);

#if CRNLIB_MEM_STATS
      CRNLIB_ASSERT((*g_pMSize)(p_new, g_pUser_data) == actual_size);
      update_total_allocated(1, static_cast<mem_stat_t>(actual_size));
#endif

      return p_new;
   }

   void* crnlib_realloc(void* p, size_t size, size_t* pActual_size, bool movable)
   {
      if ((ptr_bits_t)p & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1))
      {
         crnlib_mem_error("crnlib_realloc: bad ptr");
         return NULL;
      }

      if (size > CRNLIB_MAX_POSSIBLE_BLOCK_SIZE)
      {
         crnlib_mem_error("crnlib_malloc: size too big");
         return NULL;
      }

#if CRNLIB_MEM_STATS
      size_t cur_size = p ? (*g_pMSize)(p, g_pUser_data) : 0;
      CRNLIB_ASSERT(!p || (cur_size >= sizeof(uint32)));
#endif
      if ((size) && (size < sizeof(uint32))) 
         size = sizeof(uint32);

      size_t actual_size = size;
      void* p_new = (*g_pRealloc)(p, size, &actual_size, movable, g_pUser_data);

      if (pActual_size)
         *pActual_size = actual_size;

      CRNLIB_ASSERT((reinterpret_cast<ptr_bits_t>(p_new) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1)) == 0);

#if CRNLIB_MEM_STATS
      CRNLIB_ASSERT(!p_new || ((*g_pMSize)(p_new, g_pUser_data) == actual_size));

      int num_new_blocks = 0;
      if (p)
      {
         if (!p_new)
            num_new_blocks = -1;
      }
      else if (p_new)
      {
         num_new_blocks = 1;
      }
      update_total_allocated(num_new_blocks, static_cast<mem_stat_t>(actual_size) - static_cast<mem_stat_t>(cur_size));
#endif

      return p_new;
   }

   void* crnlib_calloc(size_t count, size_t size, size_t* pActual_size)
   {
      size_t total = count * size;
      void *p = crnlib_malloc(total, pActual_size);
      if (p) memset(p, 0, total);
      return p;
   }

   void crnlib_free(void* p)
   {
      if (!p)
         return;

      if (reinterpret_cast<ptr_bits_t>(p) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1))
      {
         crnlib_mem_error("crnlib_free: bad ptr");
         return;
      }

#if CRNLIB_MEM_STATS
      size_t cur_size = (*g_pMSize)(p, g_pUser_data);
      CRNLIB_ASSERT(cur_size >= sizeof(uint32));
      update_total_allocated(-1, -static_cast<mem_stat_t>(cur_size));
#endif

      (*g_pRealloc)(p, 0, NULL, true, g_pUser_data);
   }

   size_t crnlib_msize(void* p)
   {
      if (!p)
         return 0;

      if (reinterpret_cast<ptr_bits_t>(p) & (CRNLIB_MIN_ALLOC_ALIGNMENT - 1))
      {
         crnlib_mem_error("crnlib_msize: bad ptr");
         return 0;
      }

      return (*g_pMSize)(p, g_pUser_data);
   }
  
   void crnlib_print_mem_stats()
   {
#if CRNLIB_MEM_STATS
      if (console::is_initialized())
      {
         console::debug(L"crnlib_print_mem_stats:");
         console::debug(L"Current blocks: %u, allocated: %I64u, max ever allocated: %I64i", g_total_blocks, (int64)g_total_allocated, (int64)g_max_allocated);
      }
      else
      {
         printf("crnlib_print_mem_stats:\n");
         printf("Current blocks: %u, allocated: %I64u, max ever allocated: %I64i\n", g_total_blocks, (int64)g_total_allocated, (int64)g_max_allocated);
      }
#endif
   }

} // namespace crnlib

void crn_set_memory_callbacks(crn_realloc_func pRealloc, crn_msize_func pMSize, void* pUser_data)
{
   if ((!pRealloc) || (!pMSize))
   {
      crnlib::g_pRealloc = crnlib::crnlib_default_realloc;
      crnlib::g_pMSize = crnlib::crnlib_default_msize;
      crnlib::g_pUser_data = NULL;
   }
   else
   {
      crnlib::g_pRealloc = pRealloc;
      crnlib::g_pMSize = pMSize;
      crnlib::g_pUser_data = pUser_data;
   }
}
