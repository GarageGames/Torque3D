//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "util/rectClipper.h"

namespace {

inline void
swap(F32& in_one, F32& in_two)
{
   F32 temp = in_one;
   in_one   = in_two;
   in_two   = temp;
}

}

bool
RectClipper::clipLine(const Point2I& in_rStart,
                      const Point2I& in_rEnd,
                      Point2I&       out_rStart,
                      Point2I&       out_rEnd) const
{
   // Check for trivial rejection
   if ((in_rStart.x < m_clipRect.point.x && in_rEnd.x < m_clipRect.point.x) ||
       (in_rStart.x >= m_clipRect.point.x + m_clipRect.extent.x &&
         in_rEnd.x >= m_clipRect.point.x + m_clipRect.extent.x))
      return false;
   if ((in_rStart.y < m_clipRect.point.y && in_rEnd.y < m_clipRect.point.y) ||
       (in_rStart.y >= m_clipRect.point.y + m_clipRect.extent.y &&
         in_rEnd.y >= m_clipRect.point.y + m_clipRect.extent.y))
      return false;

   F32 x1 = F32(in_rStart.x);
   F32 y1 = F32(in_rStart.y);
   F32 x2 = F32(in_rEnd.x);
   F32 y2 = F32(in_rEnd.y);

   // I'm using essentially what's in the Phoenix libs, Liang-Biarsky based, but
   //  converted to FP math for greater precision on the back end...
   //
   bool flipped = false;
   if (x1 > x2) 
   {
      swap(x1, x2);
      swap(y1, y2);
      flipped = !flipped;
   }

   F32 dx = x2 - x1;
   F32 dy = y2 - y1;

   // Clip x coord
   F32 t;
   if (x1 < F32(m_clipRect.point.x)) 
   {
      t   = (F32(m_clipRect.point.x) - x1) / F32(dx);
      x1  = F32(m_clipRect.point.x);
      y1 += t * dy;
      dx  = x2 - x1;
      dy  = y2 - y1;
   }
   if (x2 >= F32(m_clipRect.point.x + m_clipRect.extent.x))
   {
      t   = (F32(m_clipRect.point.x + m_clipRect.extent.x - 1) - x1) / F32(dx);
      x2  = F32(m_clipRect.point.x + m_clipRect.extent.x - 1);
      y2  = y1 + (t * dy);
      dx  = x2 - x1;
      dy  = y2 - y1;
   }

   // Recheck trivial rejection condition...
   if((y1 > F32(m_clipRect.point.y + m_clipRect.extent.y - 1) &&
         y2 > F32(m_clipRect.point.y + m_clipRect.extent.y - 1)) ||
         (y1 < F32(m_clipRect.point.y) && y2 < F32(m_clipRect.point.y)))
      return false;

   if (y1 > y2) 
   {
      swap(x1, x2);
      swap(y1, y2);
      flipped = !flipped;
   }

   if (y1 < F32(m_clipRect.point.y)) 
   {
      t   = (F32(m_clipRect.point.y) - y1) / F32(dy);
      y1  = F32(m_clipRect.point.y);
      x1 += t * dx;
      dx  = x2 - x1;
      dy  = y2 - y1;
   }
   if (y2 > F32(m_clipRect.point.y + m_clipRect.extent.y - 1))
   {
      t   = (F32(m_clipRect.point.y + m_clipRect.extent.y - 1) - y1) / F32(dy);
      y2  = F32(m_clipRect.point.y + m_clipRect.extent.y - 1);
      x2  = x1 + (t * dx);
   }

   if (flipped == true) 
   {
      out_rEnd.x   = S32(x1 + 0.5f);
      out_rEnd.y   = S32(y1 + 0.5f);
      out_rStart.x = S32(x2 + 0.5f);
      out_rStart.y = S32(y2 + 0.5f);
   } 
   else 
   {
      out_rStart.x = S32(x1 + 0.5f);
      out_rStart.y = S32(y1 + 0.5f);
      out_rEnd.x   = S32(x2 + 0.5f);
      out_rEnd.y   = S32(y2 + 0.5f);
   }

   return true;
}


bool
RectClipper::clipRect(const RectI& in_rRect,
                      RectI&       out_rRect) const
{
   AssertFatal(in_rRect.isValidRect(), "Inappropriate min/max coords for rectangle");

   if (in_rRect.point.x + in_rRect.extent.x - 1 < m_clipRect.point.x ||
       in_rRect.point.x > m_clipRect.point.x + m_clipRect.extent.x - 1)
      return false;
   if (in_rRect.point.y + in_rRect.extent.y - 1 < m_clipRect.point.y ||
       in_rRect.point.y > m_clipRect.point.y + m_clipRect.extent.y - 1)
      return false;

   if (in_rRect.point.x < m_clipRect.point.x) out_rRect.point.x = m_clipRect.point.x;
   else                                       out_rRect.point.x = in_rRect.point.x;

   if (in_rRect.point.y < m_clipRect.point.y) out_rRect.point.y = m_clipRect.point.y;
   else                                       out_rRect.point.y = in_rRect.point.y;

   Point2I bottomR;
   bottomR.x = getMin(in_rRect.point.x + in_rRect.extent.x - 1,
                     m_clipRect.point.x + m_clipRect.extent.x - 1);
   bottomR.y = getMin(in_rRect.point.y + in_rRect.extent.y - 1,
                     m_clipRect.point.y + m_clipRect.extent.y - 1);

   out_rRect.extent.x = bottomR.x - out_rRect.point.x + 1;
   out_rRect.extent.x = bottomR.y - out_rRect.point.y + 1;

   return true;
}
