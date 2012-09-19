/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2007                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: dsp_mmx.c 15397 2008-10-14 02:06:24Z tterribe $

 ********************************************************************/

#include <stdlib.h>

#include "../codec_internal.h"
#include "../dsp.h"

#if defined(USE_ASM)

typedef unsigned long long ogg_uint64_t;

static const __attribute__ ((aligned(8),used)) ogg_int64_t V128 = 0x0080008000800080LL;

#define DSP_OP_AVG(a,b) ((((int)(a)) + ((int)(b)))/2)
#define DSP_OP_DIFF(a,b) (((int)(a)) - ((int)(b)))
#define DSP_OP_ABS_DIFF(a,b) abs((((int)(a)) - ((int)(b))))

static void sub8x8__mmx (unsigned char *FiltPtr, unsigned char *ReconPtr,
                  ogg_int16_t *DctInputPtr, ogg_uint32_t PixelsPerLine,
                  ogg_uint32_t ReconPixelsPerLine)
{
  __asm__ __volatile__ (
    "  .balign 16                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"

    ".rept 8                        \n\t"
    "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */
    "  movq        (%1), %%mm1      \n\t" /* mm1 = ReconPtr */
    "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */
    "  movq        %%mm1, %%mm3     \n\t" /* dup to prepare for up conversion */
    /* convert from UINT8 to INT16 */
    "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */
    "  punpcklbw   %%mm7, %%mm1     \n\t" /* mm1 = INT16(ReconPtr) */
    "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */
    "  punpckhbw   %%mm7, %%mm3     \n\t" /* mm3 = INT16(ReconPtr) */
    /* start calculation */
    "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - ReconPtr */
    "  psubw       %%mm3, %%mm2     \n\t" /* mm2 = FiltPtr - ReconPtr */
    "  movq        %%mm0,  (%2)     \n\t" /* write answer out */
    "  movq        %%mm2, 8(%2)     \n\t" /* write answer out */
    /* Increment pointers */
    "  add         $16, %2          \n\t"
    "  add         %3, %0           \n\t"
    "  add         %4, %1           \n\t"
    ".endr                          \n\t"

     : "+r" (FiltPtr),
       "+r" (ReconPtr),
       "+r" (DctInputPtr)
     : "r" ((ogg_uint64_t)PixelsPerLine),
       "r" ((ogg_uint64_t)ReconPixelsPerLine)
     : "memory"
  );
}

static void sub8x8_128__mmx (unsigned char *FiltPtr, ogg_int16_t *DctInputPtr,
                      ogg_uint32_t PixelsPerLine)
{
  ogg_uint64_t ppl = PixelsPerLine;

  __asm__ __volatile__ (
    "  .balign 16                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"
    "  movq        %[V128], %%mm1   \n\t"

    ".rept 8                        \n\t"
    "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */
    "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */
    /* convert from UINT8 to INT16 */
    "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */
    "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */
    /* start calculation */
    "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - 128 */
    "  psubw       %%mm1, %%mm2     \n\t" /* mm2 = FiltPtr - 128 */
    "  movq        %%mm0,  (%1)     \n\t" /* write answer out */
    "  movq        %%mm2, 8(%1)     \n\t" /* write answer out */
    /* Increment pointers */
    "  add         $16, %1           \n\t"
    "  add         %2, %0           \n\t"
    ".endr                          \n\t"

     : "+r" (FiltPtr),
       "+r" (DctInputPtr)
     : "r" (ppl), /* gcc bug? a cast won't work here, e.g. (ogg_uint64_t)PixelsPerLine */
       [V128] "m" (V128)
     : "memory"
  );
}

static void sub8x8avg2__mmx (unsigned char *FiltPtr, unsigned char *ReconPtr1,
                     unsigned char *ReconPtr2, ogg_int16_t *DctInputPtr,
                     ogg_uint32_t PixelsPerLine,
                     ogg_uint32_t ReconPixelsPerLine)
{
  __asm__ __volatile__ (
    "  .balign 16                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"

    ".rept 8                        \n\t"
    "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */
    "  movq        (%1), %%mm1      \n\t" /* mm1 = ReconPtr1 */
    "  movq        (%2), %%mm4      \n\t" /* mm1 = ReconPtr2 */
    "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */
    "  movq        %%mm1, %%mm3     \n\t" /* dup to prepare for up conversion */
    "  movq        %%mm4, %%mm5     \n\t" /* dup to prepare for up conversion */
    /* convert from UINT8 to INT16 */
    "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */
    "  punpcklbw   %%mm7, %%mm1     \n\t" /* mm1 = INT16(ReconPtr1) */
    "  punpcklbw   %%mm7, %%mm4     \n\t" /* mm1 = INT16(ReconPtr2) */
    "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */
    "  punpckhbw   %%mm7, %%mm3     \n\t" /* mm3 = INT16(ReconPtr1) */
    "  punpckhbw   %%mm7, %%mm5     \n\t" /* mm3 = INT16(ReconPtr2) */
    /* average ReconPtr1 and ReconPtr2 */
    "  paddw       %%mm4, %%mm1     \n\t" /* mm1 = ReconPtr1 + ReconPtr2 */
    "  paddw       %%mm5, %%mm3     \n\t" /* mm3 = ReconPtr1 + ReconPtr2 */
    "  psrlw       $1, %%mm1        \n\t" /* mm1 = (ReconPtr1 + ReconPtr2) / 2 */
    "  psrlw       $1, %%mm3        \n\t" /* mm3 = (ReconPtr1 + ReconPtr2) / 2 */
    "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2) */
    "  psubw       %%mm3, %%mm2     \n\t" /* mm2 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2) */
    "  movq        %%mm0,  (%3)     \n\t" /* write answer out */
    "  movq        %%mm2, 8(%3)     \n\t" /* write answer out */
    /* Increment pointers */
    "  add         $16, %3           \n\t"
    "  add         %4, %0           \n\t"
    "  add         %5, %1           \n\t"
    "  add         %5, %2           \n\t"
    ".endr                          \n\t"

     : "+r" (FiltPtr),
       "+r" (ReconPtr1),
       "+r" (ReconPtr2),
       "+r" (DctInputPtr)
     : "r" ((ogg_uint64_t)PixelsPerLine),
       "r" ((ogg_uint64_t)ReconPixelsPerLine)
     : "memory"
  );
}

static ogg_uint32_t intra8x8_err__mmx (unsigned char *DataPtr, ogg_uint32_t Stride)
{
  ogg_uint64_t  XSum;
  ogg_uint64_t  XXSum;

  __asm__ __volatile__ (
    "  .balign 16                   \n\t"

    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%rdi        \n\t"
    "1:                             \n\t"
    "  movq        (%2), %%mm0      \n\t"       /* take 8 bytes */
    "  movq        %%mm0, %%mm2     \n\t"

    "  punpcklbw   %%mm6, %%mm0     \n\t"
    "  punpckhbw   %%mm6, %%mm2     \n\t"

    "  paddw       %%mm0, %%mm5     \n\t"
    "  paddw       %%mm2, %%mm5     \n\t"

    "  pmaddwd     %%mm0, %%mm0     \n\t"
    "  pmaddwd     %%mm2, %%mm2     \n\t"

    "  paddd       %%mm0, %%mm7     \n\t"
    "  paddd       %%mm2, %%mm7     \n\t"

    "  add         %3, %2           \n\t"       /* Inc pointer into src data */

    "  dec         %%rdi            \n\t"
    "  jnz 1b                       \n\t"

    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $32, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $16, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movd        %%mm5, %%rdi     \n\t"
    "  movsx       %%di, %%rdi      \n\t"
    "  mov         %%rdi, %0        \n\t"

    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $32, %%mm7       \n\t"
    "  paddd       %%mm0, %%mm7     \n\t"
    "  movd        %%mm7, %1        \n\t"

     : "=r" (XSum),
       "=r" (XXSum),
       "+r" (DataPtr)
     : "r" ((ogg_uint64_t)Stride)
     : "rdi", "memory"
  );

  /* Compute population variance as mis-match metric. */
  return (( (XXSum<<6) - XSum*XSum ) );
}

static ogg_uint32_t inter8x8_err__mmx (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                 unsigned char *RefDataPtr, ogg_uint32_t RefStride)
{
  ogg_uint64_t  XSum;
  ogg_uint64_t  XXSum;

  __asm__ __volatile__ (
    "  .balign 16                   \n\t"

    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%rdi        \n\t"
    "1:                             \n\t"
    "  movq        (%2), %%mm0      \n\t"       /* take 8 bytes */
    "  movq        (%3), %%mm1      \n\t"
    "  movq        %%mm0, %%mm2     \n\t"
    "  movq        %%mm1, %%mm3     \n\t"

    "  punpcklbw   %%mm6, %%mm0     \n\t"
    "  punpcklbw   %%mm6, %%mm1     \n\t"
    "  punpckhbw   %%mm6, %%mm2     \n\t"
    "  punpckhbw   %%mm6, %%mm3     \n\t"

    "  psubsw      %%mm1, %%mm0     \n\t"
    "  psubsw      %%mm3, %%mm2     \n\t"

    "  paddw       %%mm0, %%mm5     \n\t"
    "  paddw       %%mm2, %%mm5     \n\t"

    "  pmaddwd     %%mm0, %%mm0     \n\t"
    "  pmaddwd     %%mm2, %%mm2     \n\t"

    "  paddd       %%mm0, %%mm7     \n\t"
    "  paddd       %%mm2, %%mm7     \n\t"

    "  add         %4, %2           \n\t"       /* Inc pointer into src data */
    "  add         %5, %3           \n\t"       /* Inc pointer into ref data */

    "  dec         %%rdi            \n\t"
    "  jnz 1b                       \n\t"

    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $32, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $16, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movd        %%mm5, %%rdi     \n\t"
    "  movsx       %%di, %%rdi      \n\t"
    "  mov         %%rdi, %0        \n\t"

    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $32, %%mm7       \n\t"
    "  paddd       %%mm0, %%mm7     \n\t"
    "  movd        %%mm7, %1        \n\t"

     : "=m" (XSum),
       "=m" (XXSum),
       "+r" (SrcData),
       "+r" (RefDataPtr)
     : "r" ((ogg_uint64_t)SrcStride),
       "r" ((ogg_uint64_t)RefStride)
     : "rdi", "memory"
  );

  /* Compute and return population variance as mis-match metric. */
  return (( (XXSum<<6) - XSum*XSum ));
}

static void restore_fpu (void)
{
  __asm__ __volatile__ (
    "  emms                         \n\t"
  );
}

void dsp_mmx_init(DspFunctions *funcs)
{
  funcs->restore_fpu = restore_fpu;
  funcs->sub8x8 = sub8x8__mmx;
  funcs->sub8x8_128 = sub8x8_128__mmx;
  funcs->sub8x8avg2 = sub8x8avg2__mmx;
  funcs->intra8x8_err = intra8x8_err__mmx;
  funcs->inter8x8_err = inter8x8_err__mmx;
}

#endif /* USE_ASM */
