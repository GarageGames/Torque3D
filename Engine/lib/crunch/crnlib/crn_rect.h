// File: crn_rect.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_vec.h"
#include "crn_hash.h"

namespace crnlib
{
   class rect
   {
   public:
      inline rect()
      {
      }
      
      inline rect(eClear)
      {
         clear();
      }
            
      inline rect(int left, int top, int right, int bottom)
      {
         set(left, top, right, bottom);
      }
      
      inline rect(const vec2I& lo, const vec2I& hi)
      {
         m_corner[0] = lo;
         m_corner[1] = hi;
      }
      
      inline rect(const vec2I& point)
      {
         m_corner[0] = point;
         m_corner[1].set(point[0] + 1, point[1] + 1);
      }
      
      inline void clear()
      {
         m_corner[0].clear();
         m_corner[1].clear();  
      }
      
      inline void set(int left, int top, int right, int bottom)
      {
         m_corner[0].set(left, top);
         m_corner[1].set(right, bottom);
      }

      inline void set(const vec2I& lo, const vec2I& hi)
      {
         m_corner[0] = lo;
         m_corner[1] = hi;
      }
      
      inline void set(const vec2I& point)
      {
         m_corner[0] = point;
         m_corner[1].set(point[0] + 1, point[1] + 1);
      }
      
      inline uint get_width() const { return m_corner[1][0] - m_corner[0][0]; }
      inline uint get_height() const { return m_corner[1][1] - m_corner[0][1]; }
            
      inline int get_left() const { return m_corner[0][0]; }
      inline int get_top() const { return m_corner[0][1]; }
      inline int get_right() const { return m_corner[1][0]; }
      inline int get_bottom() const { return m_corner[1][1]; }
      
      inline bool is_empty() const { return (m_corner[1][0] <= m_corner[0][0]) || (m_corner[1][1] <= m_corner[0][1]); }
      
      inline uint get_dimension(uint axis) const { return m_corner[1][axis] - m_corner[0][axis]; }
      
      inline const vec2I& operator[] (uint i) const { CRNLIB_ASSERT(i < 2); return m_corner[i]; }
      inline       vec2I& operator[] (uint i)       { CRNLIB_ASSERT(i < 2); return m_corner[i]; }

   private:
      vec2I m_corner[2];
   };
   
} // namespace crnlib
