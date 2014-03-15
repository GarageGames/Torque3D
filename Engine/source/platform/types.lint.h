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

#ifndef TORQUE_TYPES_LINT_H_
#define TORQUE_TYPES_LINT_H_

typedef signed long long S64;
typedef unsigned long long U64;

typedef unsigned int dsize_t;

struct FileTime
{
   U32 v1;
   U32 v2;
};

#define TORQUE_OS_STRING "Lint"
#define TORQUE_CPU_STRING "x86"
#define TORQUE_LITTLE_ENDIAN
#define TORQUE_SUPPORTS_NASM
#define TORQUE_SUPPORTS_VC_INLINE_X86_ASM
#define TORQUE_OS_WIN
#define TORQUE_COMPILER_VISUALC 1500

#ifndef FN_CDECL
#define FN_CDECL
#endif

#ifndef Offset
#define Offset(x, cls) _Offset_Normal(x, cls)
#define OffsetNonConst(x, cls) _Offset_Normal(x, cls)
#endif

#ifndef NULL
#define NULL 0
#endif

#endif
