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

#ifndef _RECTCLIPPER_H_
#define _RECTCLIPPER_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif


class RectClipper
{
   RectI m_clipRect;

  public:
   RectClipper(const RectI& in_rRect);

   bool  clipPoint(const Point2I& in_rPoint) const;
   bool  clipLine(const Point2I& in_rStart,
                  const Point2I& in_rEnd,
                  Point2I&       out_rStart,
                  Point2I&       out_rEnd) const;
   bool  clipRect(const RectI& in_rRect,
                  RectI&       out_rRect) const;
};

//------------------------------------------------------------------------------
//-------------------------------------- INLINES
//
inline
RectClipper::RectClipper(const RectI& in_rRect)
 : m_clipRect(in_rRect)
{
   //
}

inline bool
RectClipper::clipPoint(const Point2I& in_rPoint) const
{
   if ((in_rPoint.x < m_clipRect.point.x) ||
       (in_rPoint.y < m_clipRect.point.y) ||
       (in_rPoint.x >= m_clipRect.point.x + m_clipRect.extent.x) ||
       (in_rPoint.y >= m_clipRect.point.y + m_clipRect.extent.y))
      return false;
   return true;
}

#endif //_RECTCLIPPER_H_
