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

#if defined(TORQUE_SUPPORTS_VC_INLINE_X86_ASM)
static S32 m_mulDivS32_ASM(S32 a, S32 b, S32 c)
{  // a * b / c
   S32 r;
   _asm
   {
      mov   eax, a
      imul  b
      idiv  c
      mov   r, eax
   }
   return r;
}


static U32 m_mulDivU32_ASM(S32 a, S32 b, U32 c)
{  // a * b / c
   S32 r;
   _asm
   {
      mov   eax, a
      mov   edx, 0
      mul   b
      div   c
      mov   r, eax
   }
   return r;
}

static void m_sincos_ASM( F32 angle, F32 *s, F32 *c )
{
   _asm
   {
      fld     angle
      fsincos
      mov     eax, c
      fstp    dword ptr [eax]
      mov     eax, s
      fstp    dword ptr [eax]
   }
}

U32 Platform::getMathControlState()
{
   U16 cw;
   _asm
   {
      fstcw cw
   }
   return cw;
}

void Platform::setMathControlState(U32 state)
{
   U16 cw = state;
   _asm
   {
      fldcw cw
   }
}

void Platform::setMathControlStateKnown()
{
   U16 cw = 0x27F;
   _asm
   {
      fldcw cw
   }
}
#else
// @source http://msdn.microsoft.com/en-us/library/c9676k6h.aspx
U32 Platform::getMathControlState( )
{
   U32 control_word = 0;
   S32 error = _controlfp_s( &control_word, _DN_FLUSH, _MCW_DN );

   return error ? 0 : control_word;
}

void Platform::setMathControlState( U32 state )
{
   U32 control_word = 0;
   _controlfp_s( &control_word, state, _MCW_DN );
}

void Platform::setMathControlStateKnown( )
{
   U32 control_word = 0;
   _controlfp_s (&control_word, _PC_64, _MCW_DN);
}
#endif

//------------------------------------------------------------------------------
void mInstallLibrary_ASM()
{
#if defined(TORQUE_SUPPORTS_VC_INLINE_X86_ASM)
   m_mulDivS32              = m_mulDivS32_ASM;
   m_mulDivU32              = m_mulDivU32_ASM;
   
   m_sincos = m_sincos_ASM;
#endif
}


