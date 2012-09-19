;-----------------------------------------------------------------------------
; Copyright (c) 2012 GarageGames, LLC
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to
; deal in the Software without restriction, including without limitation the
; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
; sell copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;-----------------------------------------------------------------------------


segment .data

matA     dd 0
result   dd 0
matB     dd 0

segment .text

%macro export_fn 1 
   %ifidn __OUTPUT_FORMAT__, elf
   ; No underscore needed for ELF object files
   global %1
   %1:
   %else
   global _%1
   _%1:
   %endif
%endmacro


%define arg(x) [esp+(x*4)] 



;void SSE_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result)

export_fn SSE_MatrixF_x_MatrixF

      mov         edx, arg(1) 
      mov         ecx, arg(2)
      mov         eax, arg(3)

      movss       xmm0, [edx]
      movups      xmm1, [ecx]
      shufps      xmm0, xmm0, 0
      movss       xmm2, [edx+4]
      mulps       xmm0, xmm1
      shufps      xmm2, xmm2, 0
      movups      xmm3, [ecx+10h]
      movss       xmm7, [edx+8]
      mulps       xmm2, xmm3
      shufps      xmm7, xmm7, 0
      addps       xmm0, xmm2
      movups      xmm4, [ecx+20h]
      movss       xmm2, [edx+0Ch]
      mulps       xmm7, xmm4
      shufps      xmm2, xmm2, 0
      addps       xmm0, xmm7
      movups      xmm5, [ecx+30h]
      movss       xmm6, [edx+10h]
      mulps       xmm2, xmm5
      movss       xmm7, [edx+14h]
      shufps      xmm6, xmm6, 0
      addps       xmm0, xmm2
      shufps      xmm7, xmm7, 0
      movlps      [eax], xmm0
      movhps      [eax+8], xmm0
      mulps       xmm7, xmm3
      movss       xmm0, [edx+18h]
      mulps       xmm6, xmm1
      shufps      xmm0, xmm0, 0
      addps       xmm6, xmm7
      mulps       xmm0, xmm4
      movss       xmm2, [edx+24h]
      addps       xmm6, xmm0
      movss       xmm0, [edx+1Ch]
      movss       xmm7, [edx+20h]
      shufps      xmm0, xmm0, 0
      shufps      xmm7, xmm7, 0
      mulps       xmm0, xmm5
      mulps       xmm7, xmm1
      addps       xmm6, xmm0
      shufps      xmm2, xmm2, 0
      movlps      [eax+10h], xmm6
      movhps      [eax+18h], xmm6
      mulps       xmm2, xmm3
      movss       xmm6, [edx+28h]
      addps       xmm7, xmm2
      shufps      xmm6, xmm6, 0
      movss       xmm2, [edx+2Ch]
      mulps       xmm6, xmm4
      shufps      xmm2, xmm2, 0
      addps       xmm7, xmm6
      mulps       xmm2, xmm5
      movss       xmm0, [edx+34h]
      addps       xmm7, xmm2
      shufps      xmm0, xmm0, 0
      movlps      [eax+20h], xmm7
      movss       xmm2, [edx+30h]
      movhps      [eax+28h], xmm7
      mulps       xmm0, xmm3
      shufps      xmm2, xmm2, 0
      movss       xmm6, [edx+38h]
      mulps       xmm2, xmm1
      shufps      xmm6, xmm6, 0
      addps       xmm2, xmm0
      mulps       xmm6, xmm4
      movss       xmm7, [edx+3Ch]
      shufps      xmm7, xmm7, 0
      addps       xmm2, xmm6
      mulps       xmm7, xmm5
      addps       xmm2, xmm7
      movups      [eax+30h], xmm2

      ret
