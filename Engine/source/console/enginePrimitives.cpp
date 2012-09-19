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

#include "console/enginePrimitives.h"
#include "console/engineTypeInfo.h"


IMPLEMENT_PRIMITIVE( bool,          bool,,      "Boolean true/false." );
IMPLEMENT_PRIMITIVE( S8,            byte,,      "8bit signed integer." );
IMPLEMENT_PRIMITIVE( U8,            ubyte,,     "8bit unsigned integer." );
IMPLEMENT_PRIMITIVE( S32,           int,,       "32bit signed integer." );
IMPLEMENT_PRIMITIVE( U32,           uint,,      "32bit unsigned integer." );
IMPLEMENT_PRIMITIVE( F32,           float,,     "32bit single-precision floating-point." );
IMPLEMENT_PRIMITIVE( F64,           double,,    "64bit double-precision floating-point." );
IMPLEMENT_PRIMITIVE( String,        string,,    "Null-terminated UTF-16 Unicode string." );
IMPLEMENT_PRIMITIVE( void*,         ptr,,       "Opaque pointer." );
