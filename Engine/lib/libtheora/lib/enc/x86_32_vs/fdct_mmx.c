;//==========================================================================
;//
;//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
;//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
;//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
;//  PURPOSE.
;//
;//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
;//
;//--------------------------------------------------------------------------

#include "theora/theora.h"
#include "../codec_internal.h"
#include "../dsp.h"


static const  ogg_int64_t xC1S7 = 0x0fb15fb15fb15fb15;
static const  ogg_int64_t xC2S6 = 0x0ec83ec83ec83ec83;
static const  ogg_int64_t xC3S5 = 0x0d4dbd4dbd4dbd4db;
static const  ogg_int64_t xC4S4 = 0x0b505b505b505b505;
static const  ogg_int64_t xC5S3 = 0x08e3a8e3a8e3a8e3a;
static const  ogg_int64_t xC6S2 = 0x061f861f861f861f8;
static const  ogg_int64_t xC7S1 = 0x031f131f131f131f1;


static __inline void Transpose_mmx( ogg_int16_t *InputData1, ogg_int16_t *OutputData1,
                                 ogg_int16_t *InputData2, ogg_int16_t *OutputData2)
{

    __asm {
        align 16
            mov     eax, InputData1
            mov     ebx, InputData2
            mov     ecx, OutputData1
            mov     edx, OutputData2


        movq    mm0, [eax]    ; /* mm0 = a0 a1 a2 a3 */
        movq    mm4, [ebx]    ; /* mm4 = e4 e5 e6 e7 */
        movq    mm1, [16 + eax]   ; /* mm1 = b0 b1 b2 b3 */
        movq    mm5, [16 + ebx]   ; /* mm5 = f4 f5 f6 f7 */
        movq    mm2, [32 + eax]   ; /* mm2 = c0 c1 c2 c3 */
        movq    mm6, [32 + ebx]   ; /* mm6 = g4 g5 g6 g7 */
        movq    mm3, [48 + eax]   ; /* mm3 = d0 d1 d2 d3 */
        movq    [16 + ecx], mm1   ; /* save  b0 b1 b2 b3 */
        movq    mm7, [48 + ebx]   ; /* mm7 = h0 h1 h2 h3 */
          ; /* Transpose 2x8 block */
        movq    mm1, mm4    ; /* mm1 = e3 e2 e1 e0 */
        punpcklwd   mm4, mm5    ; /* mm4 = f1 e1 f0 e0 */
        movq    [ecx], mm0    ; /* save a3 a2 a1 a0  */
        punpckhwd   mm1, mm5    ; /* mm1 = f3 e3 f2 e2 */
        movq    mm0, mm6    ; /* mm0 = g3 g2 g1 g0 */
        punpcklwd   mm6, mm7    ; /* mm6 = h1 g1 h0 g0 */
        movq    mm5, mm4    ; /* mm5 = f1 e1 f0 e0 */
        punpckldq   mm4, mm6    ; /* mm4 = h0 g0 f0 e0 = MM4 */
        punpckhdq   mm5, mm6    ; /* mm5 = h1 g1 f1 e1 = MM5 */
        movq    mm6, mm1    ; /* mm6 = f3 e3 f2 e2 */
        movq    [edx], mm4    ;
        punpckhwd   mm0, mm7    ; /* mm0 = h3 g3 h2 g2 */
        movq    [16 + edx], mm5   ;
        punpckhdq   mm6, mm0    ; /* mm6 = h3 g3 f3 e3 = MM7 */
        movq    mm4, [ecx]    ; /* mm4 = a3 a2 a1 a0 */
        punpckldq   mm1, mm0    ; /* mm1 = h2 g2 f2 e2 = MM6 */
        movq    mm5, [16 + ecx]   ; /* mm5 = b3 b2 b1 b0 */
        movq    mm0, mm4    ; /* mm0 = a3 a2 a1 a0 */
        movq    [48 + edx], mm6   ;
        punpcklwd   mm0, mm5    ; /* mm0 = b1 a1 b0 a0 */
        movq    [32 + edx], mm1   ;
        punpckhwd   mm4, mm5    ; /* mm4 = b3 a3 b2 a2 */
        movq    mm5, mm2    ; /* mm5 = c3 c2 c1 c0 */
        punpcklwd   mm2, mm3    ; /* mm2 = d1 c1 d0 c0 */
        movq    mm1, mm0    ; /* mm1 = b1 a1 b0 a0 */
        punpckldq   mm0, mm2    ; /* mm0 = d0 c0 b0 a0 = MM0 */
        punpckhdq   mm1, mm2    ; /* mm1 = d1 c1 b1 a1 = MM1 */
        movq    mm2, mm4    ; /* mm2 = b3 a3 b2 a2 */
        movq    [ecx], mm0    ;
        punpckhwd   mm5, mm3    ; /* mm5 = d3 c3 d2 c2 */
        movq    [16 + ecx], mm1   ;
        punpckhdq   mm4, mm5    ; /* mm4 = d3 c3 b3 a3 = MM3 */
        punpckldq   mm2, mm5    ; /* mm2 = d2 c2 b2 a2 = MM2 */
        movq    [48 + ecx], mm4   ;
        movq    [32 + ecx], mm2   ;

    };


}

static __inline void Fdct_mmx( ogg_int16_t *InputData1, ogg_int16_t *InputData2, ogg_int16_t *temp)
{

    __asm {
        align 16


                mov     eax, InputData1
                mov     ebx, InputData2
                mov     ecx, temp
        movq    mm0, [eax]    ;
        movq    mm1, [16 + eax]   ;
        movq    mm2, [48 + eax]   ;
        movq    mm3, [16 + ebx]   ;
        movq    mm4, mm0    ;
        movq    mm5, mm1    ;
        movq    mm6, mm2    ;
        movq    mm7, mm3    ;
                ;
        paddsw    mm0, [48 + ebx]   ; /* mm0 = ip0 + ip7 = is07 */
        paddsw    mm1, [32 + eax]   ; /* mm1 = ip1 + ip2 = is12 */
        paddsw    mm2, [ebx]    ; /* mm2 = ip3 + ip4 = is34 */
        paddsw    mm3, [32 + ebx]   ; /* mm3 = ip5 + ip6 = is56 */
        psubsw    mm4, [48 + ebx]   ; /* mm4 = ip0 - ip7 = id07 */
        psubsw    mm5, [32 + eax]   ; /* mm5 = ip1 - ip2 = id12 */
                ;
        psubsw    mm0, mm2    ; /* mm0 = is07 - is34 */
                ;
        paddsw    mm2, mm2    ;
                ;
        psubsw    mm6, [ebx]    ; /* mm6 = ip3 - ip4 = id34 */
                ;
        paddsw    mm2, mm0    ; /* mm2 = is07 + is34 = is0734 */
        psubsw    mm1, mm3    ; /* mm1 = is12 - is56 */
        movq    [ecx], mm0    ; /* Save is07 - is34 to free mm0; */
        paddsw    mm3, mm3    ;
        paddsw    mm3, mm1    ; /* mm3 = is12 + 1s56  = is1256 */
                ;
        psubsw    mm7, [32 + ebx]   ; /* mm7 = ip5 - ip6 = id56 */
          ; /* ------------------------------------------------------------------- */
        psubsw    mm5, mm7    ; /* mm5 = id12 - id56 */
        paddsw    mm7, mm7    ;
        paddsw    mm7, mm5    ; /* mm7 = id12 + id56 */
          ; /* ------------------------------------------------------------------- */
        psubsw    mm2, mm3    ; /* mm2 = is0734 - is1256 */
        paddsw    mm3, mm3    ;
                ;
        movq    mm0, mm2    ; /* make a copy */
        paddsw    mm3, mm2    ; /* mm3 = is0734 + is1256 */
                ;
        pmulhw    mm0, xC4S4    ; /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */
        paddw   mm0, mm2    ; /* mm0 = xC4S4 * ( is0734 - is1256 ) */
        psrlw   mm2, 15   ;
        paddw   mm0, mm2    ; /* Truncate mm0, now it is op[4] */
                ;
        movq    mm2, mm3    ;
        movq    [ebx], mm0    ; /* save ip4, now mm0,mm2 are free */
                ;
        movq    mm0, mm3    ;
        pmulhw    mm3, xC4S4    ; /* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */
                ;
        psrlw   mm2, 15   ;
        paddw   mm3, mm0    ; /* mm3 = xC4S4 * ( is0734 +is1256 )  */
        paddw   mm3, mm2    ; /* Truncate mm3, now it is op[0] */
                ;
        movq    [eax], mm3    ;
          ; /* ------------------------------------------------------------------- */
        movq    mm3, [ecx]    ; /* mm3 = irot_input_y */
        pmulhw    mm3, xC2S6  ; /* mm3 = xC2S6 * irot_input_y - irot_input_y */
                ;
        movq    mm2, [ecx]    ;
        movq    mm0, mm2    ;
                ;
        psrlw   mm2, 15   ; /* mm3 = xC2S6 * irot_input_y */
        paddw   mm3, mm0    ;
                ;
        paddw   mm3, mm2    ; /* Truncated */
        movq    mm0, mm5    ;
                ;
        movq    mm2, mm5    ;
        pmulhw    mm0, xC6S2    ; /* mm0 = xC6S2 * irot_input_x */
                ;
        psrlw   mm2, 15   ;
        paddw   mm0, mm2    ; /* Truncated */
                ;
        paddsw    mm3, mm0    ; /* ip[2] */
        movq    [32 + eax], mm3   ; /* Save ip2 */
                ;
        movq    mm0, mm5    ;
        movq    mm2, mm5    ;
                ;
        pmulhw    mm5, xC2S6    ; /* mm5 = xC2S6 * irot_input_x - irot_input_x */
        psrlw   mm2, 15   ;
                ;
        movq    mm3, [ecx]    ;
        paddw   mm5, mm0    ; /* mm5 = xC2S6 * irot_input_x */
                ;
        paddw   mm5, mm2    ; /* Truncated */
        movq    mm2, mm3    ;
                ;
        pmulhw    mm3, xC6S2    ; /* mm3 = xC6S2 * irot_input_y */
        psrlw   mm2, 15   ;
                ;
        paddw   mm3, mm2    ; /* Truncated */
        psubsw    mm3, mm5    ;
                ;
        movq    [32 + ebx], mm3   ;
          ; /* ------------------------------------------------------------------- */
        movq    mm0, xC4S4    ;
        movq    mm2, mm1    ;
        movq    mm3, mm1    ;
                ;
        pmulhw    mm1, mm0    ; /* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */
        psrlw   mm2, 15   ;
                ;
        paddw   mm1, mm3    ; /* mm0 = xC4S4 * ( is12 - is56 ) */
        paddw   mm1, mm2    ; /* Truncate mm1, now it is icommon_product1 */
                ;
        movq    mm2, mm7    ;
        movq    mm3, mm7    ;
                ;
        pmulhw    mm7, mm0    ; /* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */
        psrlw   mm2, 15   ;
                ;
        paddw   mm7, mm3    ; /* mm7 = xC4S4 * ( id12 + id56 ) */
        paddw   mm7, mm2    ; /* Truncate mm7, now it is icommon_product2 */
          ; /* ------------------------------------------------------------------- */
        pxor    mm0, mm0    ; /* Clear mm0 */
        psubsw    mm0, mm6    ; /* mm0 = - id34 */
                ;
        psubsw    mm0, mm7    ; /* mm0 = - ( id34 + idcommon_product2 ) */
        paddsw    mm6, mm6    ;
        paddsw    mm6, mm0    ; /* mm6 = id34 - icommon_product2 */
                ;
        psubsw    mm4, mm1    ; /* mm4 = id07 - icommon_product1 */
        paddsw    mm1, mm1    ;
        paddsw    mm1, mm4    ; /* mm1 = id07 + icommon_product1 */
          ; /* ------------------------------------------------------------------- */
        movq    mm7, xC1S7    ;
        movq    mm2, mm1    ;
                ;
        movq    mm3, mm1    ;
        pmulhw    mm1, mm7    ; /* mm1 = xC1S7 * irot_input_x - irot_input_x */
                ;
        movq    mm7, xC7S1    ;
        psrlw   mm2, 15   ;
                ;
        paddw   mm1, mm3    ; /* mm1 = xC1S7 * irot_input_x */
        paddw   mm1, mm2    ; /* Trucated */
                ;
        pmulhw    mm3, mm7    ; /* mm3 = xC7S1 * irot_input_x */
        paddw   mm3, mm2    ; /* Truncated */
                ;
        movq    mm5, mm0    ;
        movq    mm2, mm0    ;
                ;
        movq    mm7, xC1S7    ;
        pmulhw    mm0, mm7    ; /* mm0 = xC1S7 * irot_input_y - irot_input_y */
                ;
        movq    mm7, xC7S1    ;
        psrlw   mm2, 15   ;
                ;
        paddw   mm0, mm5    ; /* mm0 = xC1S7 * irot_input_y */
        paddw   mm0, mm2    ; /* Truncated */
                ;
        pmulhw    mm5, mm7    ; /* mm5 = xC7S1 * irot_input_y */
        paddw   mm5, mm2    ; /* Truncated */
                ;
        psubsw    mm1, mm5    ; /* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = ip1 */
        paddsw    mm3, mm0    ; /* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = ip7 */
                ;
        movq    [16 + eax], mm1   ;
        movq    [48 + ebx], mm3   ;
          ; /* ------------------------------------------------------------------- */
        movq    mm0, xC3S5    ;
        movq    mm1, xC5S3    ;
                ;
        movq    mm5, mm6    ;
        movq    mm7, mm6    ;
                ;
        movq    mm2, mm4    ;
        movq    mm3, mm4    ;
                ;
        pmulhw    mm4, mm0    ; /* mm4 = xC3S5 * irot_input_x - irot_input_x */
        pmulhw    mm6, mm1    ; /* mm6 = xC5S3 * irot_input_y - irot_input_y */
                ;
        psrlw   mm2, 15   ;
        psrlw   mm5, 15   ;
                ;
        paddw   mm4, mm3    ; /* mm4 = xC3S5 * irot_input_x */
        paddw   mm6, mm7    ; /* mm6 = xC5S3 * irot_input_y */
                ;
        paddw   mm4, mm2    ; /* Truncated */
        paddw   mm6, mm5    ; /* Truncated */
                ;
        psubsw    mm4, mm6    ; /* ip3 */
        movq    [48 + eax], mm4   ;
                ;
        movq    mm4, mm3    ;
        movq    mm6, mm7    ;
                ;
        pmulhw    mm3, mm1    ; /* mm3 = xC5S3 * irot_input_x - irot_input_x */
        pmulhw    mm7, mm0    ; /* mm7 = xC3S5 * irot_input_y - irot_input_y */
                ;
        paddw   mm4, mm2    ;
        paddw   mm6, mm5    ;
                ;
        paddw   mm3, mm4    ; /* mm3 = xC5S3 * irot_input_x */
        paddw   mm7, mm6    ; /* mm7 = xC3S5 * irot_input_y */
                ;
        paddw   mm3, mm7    ; /* ip5 */
        movq    [16 + ebx], mm3   ;

};

}


static void fdct_short__mmx ( ogg_int16_t *InputData, ogg_int16_t *OutputData)
{

  static ogg_int16_t tmp[32];
  ogg_int16_t* align_tmp = (ogg_int16_t*)((unsigned char*)tmp + (16 - ((int)tmp)&15));


  Transpose_mmx(InputData, OutputData, InputData + 4, OutputData + 4);
  Fdct_mmx(OutputData, OutputData + 4, align_tmp);

  Transpose_mmx(InputData + 32, OutputData + 32, InputData + 36, OutputData + 36);
  Fdct_mmx(OutputData+32, OutputData + 36, align_tmp);

  Transpose_mmx(OutputData, OutputData, OutputData + 32, OutputData + 32);
  Fdct_mmx(OutputData, OutputData + 32, align_tmp);

  Transpose_mmx(OutputData + 4, OutputData + 4, OutputData + 36, OutputData + 36);
  Fdct_mmx(OutputData + 4, OutputData + 36, align_tmp);

  __asm     emms

}

void dsp_mmx_fdct_init(DspFunctions *funcs)
{
  funcs->fdct_short = fdct_short__mmx;
}
