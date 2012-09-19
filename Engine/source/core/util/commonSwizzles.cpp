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

#include "core/util/swizzle.h"

namespace Swizzles
{
   dsize_t _bgra[] = { 2, 1, 0, 3 };
   dsize_t _bgr[] = { 2, 1, 0 };
   dsize_t _rgb[] = { 0, 1, 2 };
   dsize_t _argb[] = { 3, 0, 1, 2 };
   dsize_t _rgba[] = { 0, 1, 2, 3 };
   dsize_t _abgr[] = { 3, 2, 1, 0 };

   Swizzle<U8, 4> bgra( _bgra );
   Swizzle<U8, 3> bgr( _bgr );
   Swizzle<U8, 3> rgb( _rgb );
   Swizzle<U8, 4> argb( _argb );
   Swizzle<U8, 4> rgba( _rgba );
   Swizzle<U8, 4> abgr( _abgr );

   NullSwizzle<U8, 4> null;
}