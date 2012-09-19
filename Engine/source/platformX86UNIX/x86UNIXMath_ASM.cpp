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

#include "math/mMath.h"

static S32 m_mulDivS32_ASM(S32 a, S32 b, S32 c)
{  // a * b / c
   S32 r;
   
   __asm__ __volatile__(
      "imul  %2\n"
      "idiv  %3\n"
      : "=a" (r) : "a" (a) , "b" (b) , "c" (c) 
      );
   return r;
}   


static U32 m_mulDivU32_ASM(S32 a, S32 b, U32 c)
{  // a * b / c
   S32 r;
   __asm__ __volatile__(
      "mov   $0, %%edx\n"
      "mul   %2\n"
      "div   %3\n"
      : "=a" (r) : "a" (a) , "b" (b) , "c" (c) 
      );
   return r;
}   

//------------------------------------------------------------------------------
void mInstallLibrary_ASM()
{
   m_mulDivS32              = m_mulDivS32_ASM;
   m_mulDivU32              = m_mulDivU32_ASM;
}
