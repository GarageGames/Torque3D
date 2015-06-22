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

;
; NASM version of optimized funcs in mMath_C
;

; The following funcs are included:
;  m_ceil_ASM, m_ceilD_ASM, m_floor_ASM, m_floorD_ASM
;  m_fmod_ASM, m_fmodD_ASM, m_mulDivS32_ASM, m_mulDivU32_ASM
;  m_sincos_ASM, m_sincosD_ASM

; The other funcs from mMath_C were determined to compile into fast
;  code using MSVC --Paul Bowman


segment .data


temp_int64			dq		0.0
const_0pt5_D		dq		0.4999999999995
temp_int32			dd		0
const_0pt5			dd		0.49999995
const_neg1			dd		-1.0


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

%define rnd_adjD	qword [const_0pt5_D]
%define rnd_adj		dword [const_0pt5]


%define val		dword [esp+4]
%define val64	qword [esp+4]
;
; static F32 m_ceil_ASM(F32 val)
;
export_fn m_ceil_ASM
    fld		val
    fadd	rnd_adj
    fistp	qword [temp_int64]
    fild	qword [temp_int64]
	ret

;
; static F64 m_ceilD_ASM(F64 val64)
;
export_fn m_ceilD_ASM
    fld		val64
    fadd	rnd_adjD
    fistp	qword [temp_int64]
    fild	qword [temp_int64]
	ret

; 
; static F32 m_floor_ASM(F32 val)
; 
export_fn m_floor_ASM
    fld		val
    fsub	rnd_adj
    fistp	qword [temp_int64]
    fild	qword [temp_int64] 
	ret


;
; static F32 m_floorD_ASM( F64 val64 )
;
export_fn m_floorD_ASM
    fld		val64
    fsub	rnd_adjD
    fistp	qword [temp_int64]
    fild	qword [temp_int64] 
	ret



%define arg_a		dword [esp+4]
%define arg_b		dword [esp+8]
%define arg_c		dword [esp+12]

;
; static S32 m_mulDivS32_ASM( S32 a, S32 b, S32 c )
;
;    // Note: this returns different (but correct) values than the C
;    //  version.  C code must be overflowing...returns -727
;    //  if a b and c are 1 million, for instance.  This version returns
;    //  1 million.
; return (S32) ((S64)a*(S64)b) / (S64)c;
;
export_fn m_mulDivS32_ASM
    mov     eax, arg_a
    imul    arg_b
    idiv    arg_c
	ret

;
; static U32 m_mulDivU32_ASM( U32 a, U32 b, U32 c )
;
;    // Note: again, C version overflows
;
export_fn m_mulDivU32_ASM
    mov     eax, arg_a
    mul     arg_b
    div     arg_c
	ret



; val is already defined above to be esp+4
%define		modulo	dword [esp+8]


;
; static F32 m_fmod_ASM(F32 val, F32 modulo)
;
export_fn m_fmod_ASM
    mov     eax, val
    fld     modulo
    fabs
    fld     val
    fabs
    fdiv    st0, st1
    fld     st0
    fsub	rnd_adj
    fistp   qword [temp_int64]
    fild    qword [temp_int64]
    fsubp   st1, st0
    fmulp   st1, st0

;    // sign bit can be read as integer high bit, 
;    //  as long as # isn't 0x80000000
    cmp     eax, 0x80000000
    jbe     notneg

    fmul    dword [const_neg1]

notneg:
	ret


%define val64hi		dword [esp+8]
%define val64		qword [esp+4]
%define modulo64	qword [esp+12]

;
; static F32 m_fmodD_ASM(F64 val, F64 modulo)
;
export_fn m_fmodD_ASM
    mov     eax, val64hi
    fld     modulo64
    fabs
    fld     val64
    fabs
    fdiv    st0, st1
    fld     st0
    fsub	rnd_adjD
    fistp   qword [temp_int64]
    fild    qword [temp_int64]
    fsubp   st1, st0
    fmulp   st1, st0

;    // sign bit can be read as integer high bit, 
;    //  as long as # isn't 0x80000000
    cmp     eax, 0x80000000
    jbe     notnegD

    fmul    dword [const_neg1]

notnegD:
	ret

	 

%define angle		dword [esp+4]
%define res_sin		dword [esp+8]
%define res_cos		dword [esp+12]

;
;static void m_sincos_ASM( F32 angle, F32 *s, F32 *c )
;
export_fn m_sincos_ASM
    mov     eax, res_cos
    fld     angle
    fsincos
    fstp    dword [eax]
    mov     eax, res_sin
    fstp    dword [eax]
	ret



%define angle64		qword [esp+4]
%define res_sin64	dword [esp+12]
%define res_cos64	dword [esp+16]
;
;static void m_sincosD_ASM( F64 angle, F64 *s, F64 *c )
;
export_fn m_sincosD_ASM
    mov     eax, res_cos64
    fld     angle64
    fsincos
    fstp    qword [eax]
    mov     eax, res_sin64
    fstp    qword [eax]
	ret



