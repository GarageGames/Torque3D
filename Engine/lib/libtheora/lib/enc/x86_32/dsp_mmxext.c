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
  last mod: $Id: dsp_mmxext.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#include <stdlib.h>

#include "../codec_internal.h"
#include "../dsp.h"

#if defined(USE_ASM)

#define SAD_MMXEXT_LOOP \
 "  movq (%1), %%mm0             \n\t"  /* take 8 bytes */ \
 "  movq (%2), %%mm1             \n\t" \
 "  psadbw %%mm1, %%mm0          \n\t" \
 "  add %3, %1                   \n\t"  /* Inc pointer into the new data */ \
 "  paddw %%mm0, %%mm7           \n\t"  /* accumulate difference... */ \
 "  add %4, %2                   \n\t"  /* Inc pointer into ref data */


static ogg_uint32_t sad8x8__mmxext (unsigned char *ptr1, ogg_uint32_t stride1,
                            unsigned char *ptr2, ogg_uint32_t stride2)
{
  ogg_uint32_t  DiffVal;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"
    "  pxor %%mm7, %%mm7            \n\t"       /* mm7 contains the result */

    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP
    SAD_MMXEXT_LOOP

    "  movq (%1), %%mm0             \n\t"       /* take 8 bytes */
    "  movq (%2), %%mm1             \n\t"
    "  psadbw %%mm1, %%mm0          \n\t"
    "  paddw %%mm0, %%mm7           \n\t"       /* accumulate difference... */
    "  movd %%mm7, %0               \n\t"

     : "=r" (DiffVal),
       "+r" (ptr1),
       "+r" (ptr2)
     : "r" (stride1),
       "r" (stride2)
     : "memory"
  );

  return DiffVal;
}

#define SAD_TRES_LOOP \
  "  movq (%1), %%mm0             \n\t" /* take 8 bytes */ \
  "  movq (%2), %%mm1             \n\t" \
  "  psadbw %%mm1, %%mm0          \n\t" \
  "  add %3, %1                   \n\t" /* Inc pointer into the new data */ \
  "  paddw %%mm0, %%mm7           \n\t" /* accumulate difference... */ \
  "  add %4, %2                   \n\t" /* Inc pointer into ref data */


static ogg_uint32_t sad8x8_thres__mmxext (unsigned char *ptr1, ogg_uint32_t stride1,
                                  unsigned char *ptr2, ogg_uint32_t stride2,
                                  ogg_uint32_t thres)
{
  ogg_uint32_t  DiffVal;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"
    "  pxor %%mm7, %%mm7            \n\t"       /* mm7 contains the result */

    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP
    SAD_TRES_LOOP

    "  movd %%mm7, %0               \n\t"

     : "=r" (DiffVal),
       "+r" (ptr1),
       "+r" (ptr2)
     : "r" (stride1),
       "r" (stride2)
     : "memory"
  );

  return DiffVal;
}

#define SAD_XY2_TRES \
  "  movq (%1), %%mm0             \n\t" /* take 8 bytes */ \
  "  movq (%2), %%mm1             \n\t" \
  "  movq (%3), %%mm2             \n\t" \
  "  pavgb %%mm2, %%mm1           \n\t" \
  "  psadbw %%mm1, %%mm0          \n\t" \
 \
  "  add %4, %1                   \n\t" /* Inc pointer into the new data */ \
  "  paddw %%mm0, %%mm7           \n\t" /* accumulate difference... */ \
  "  add %5, %2                   \n\t" /* Inc pointer into ref data */ \
  "  add %5, %3                   \n\t" /* Inc pointer into ref data */


static ogg_uint32_t sad8x8_xy2_thres__mmxext (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                      unsigned char *RefDataPtr1,
                                      unsigned char *RefDataPtr2, ogg_uint32_t RefStride,
                                      ogg_uint32_t thres)
{
  ogg_uint32_t  DiffVal;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"
    "  pxor %%mm7, %%mm7            \n\t"       /* mm7 contains the result */
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES
    SAD_XY2_TRES

    "  movd %%mm7, %0               \n\t"
     : "=m" (DiffVal),
       "+r" (SrcData),
       "+r" (RefDataPtr1),
       "+r" (RefDataPtr2)
     : "m" (SrcStride),
       "m" (RefStride)
     : "memory"
  );

  return DiffVal;
}

static ogg_uint32_t row_sad8__mmxext (unsigned char *Src1, unsigned char *Src2)
{
  ogg_uint32_t MaxSad;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  movd        (%1), %%mm0      \n\t"
    "  movd        (%2), %%mm1      \n\t"
    "  psadbw      %%mm0, %%mm1     \n\t"
    "  movd        4(%1), %%mm2     \n\t"
    "  movd        4(%2), %%mm3     \n\t"
    "  psadbw      %%mm2, %%mm3     \n\t"

    "  pmaxsw      %%mm1, %%mm3     \n\t"
    "  movd        %%mm3, %0        \n\t"
    "  andl        $0xffff, %0      \n\t"

     : "=m" (MaxSad),
       "+r" (Src1),
       "+r" (Src2)
     :
     : "memory"
  );

  return MaxSad;
}

static ogg_uint32_t col_sad8x8__mmxext (unsigned char *Src1, unsigned char *Src2,
                                    ogg_uint32_t stride)
{
  ogg_uint32_t MaxSad;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm3, %%mm3     \n\t"       /* zero out mm3 for unpack */
    "  pxor        %%mm4, %%mm4     \n\t"       /* mm4 low sum */
    "  pxor        %%mm5, %%mm5     \n\t"       /* mm5 high sum */
    "  pxor        %%mm6, %%mm6     \n\t"       /* mm6 low sum */
    "  pxor        %%mm7, %%mm7     \n\t"       /* mm7 high sum */
    "  mov         $4, %%edi        \n\t"       /* 4 rows */
    "1:                             \n\t"
    "  movq        (%1), %%mm0      \n\t"       /* take 8 bytes */
    "  movq        (%2), %%mm1      \n\t"       /* take 8 bytes */

    "  movq        %%mm0, %%mm2     \n\t"
    "  psubusb     %%mm1, %%mm0     \n\t"       /* A - B */
    "  psubusb     %%mm2, %%mm1     \n\t"       /* B - A */
    "  por         %%mm1, %%mm0     \n\t"       /* and or gives abs difference */
    "  movq        %%mm0, %%mm1     \n\t"

    "  punpcklbw   %%mm3, %%mm0     \n\t"       /* unpack to higher precision for accumulation */
    "  paddw       %%mm0, %%mm4     \n\t"       /* accumulate difference... */
    "  punpckhbw   %%mm3, %%mm1     \n\t"       /* unpack high four bytes to higher precision */
    "  paddw       %%mm1, %%mm5     \n\t"       /* accumulate difference... */
    "  add         %3, %1           \n\t"       /* Inc pointer into the new data */
    "  add         %3, %2           \n\t"       /* Inc pointer into the new data */

    "  dec         %%edi            \n\t"
    "  jnz 1b                       \n\t"

    "  mov         $4, %%edi        \n\t"       /* 4 rows */
    "2:                             \n\t"
    "  movq        (%1), %%mm0      \n\t"       /* take 8 bytes */
    "  movq        (%2), %%mm1      \n\t"       /* take 8 bytes */

    "  movq        %%mm0, %%mm2     \n\t"
    "  psubusb     %%mm1, %%mm0     \n\t"       /* A - B */
    "  psubusb     %%mm2, %%mm1     \n\t"       /* B - A */
    "  por         %%mm1, %%mm0     \n\t"       /* and or gives abs difference */
    "  movq        %%mm0, %%mm1     \n\t"

    "  punpcklbw   %%mm3, %%mm0     \n\t"       /* unpack to higher precision for accumulation */
    "  paddw       %%mm0, %%mm6     \n\t"       /* accumulate difference... */
    "  punpckhbw   %%mm3, %%mm1     \n\t"       /* unpack high four bytes to higher precision */
    "  paddw       %%mm1, %%mm7     \n\t"       /* accumulate difference... */
    "  add         %3, %1           \n\t"       /* Inc pointer into the new data */
    "  add         %3, %2           \n\t"       /* Inc pointer into the new data */

    "  dec         %%edi            \n\t"
    "  jnz 2b                       \n\t"

    "  pmaxsw      %%mm6, %%mm7     \n\t"
    "  pmaxsw      %%mm4, %%mm5     \n\t"
    "  pmaxsw      %%mm5, %%mm7     \n\t"
    "  movq        %%mm7, %%mm6     \n\t"
    "  psrlq       $32, %%mm6       \n\t"
    "  pmaxsw      %%mm6, %%mm7     \n\t"
    "  movq        %%mm7, %%mm6     \n\t"
    "  psrlq       $16, %%mm6       \n\t"
    "  pmaxsw      %%mm6, %%mm7     \n\t"
    "  movd        %%mm7, %0        \n\t"
    "  andl        $0xffff, %0      \n\t"

     : "=r" (MaxSad),
       "+r" (Src1),
       "+r" (Src2)
     : "r" (stride)
     : "memory", "edi"
  );

  return MaxSad;
}

static ogg_uint32_t inter8x8_err_xy2__mmxext (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                     unsigned char *RefDataPtr1,
                                     unsigned char *RefDataPtr2, ogg_uint32_t RefStride)
{
  ogg_uint32_t XSum;
  ogg_uint32_t XXSum;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm4, %%mm4     \n\t"
    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%edi        \n\t"
    "1:                             \n\t"
    "  movq        (%2), %%mm0      \n\t"       /* take 8 bytes */

    "  movq        (%3), %%mm2      \n\t"
    "  movq        (%4), %%mm1      \n\t"       /* take average of mm2 and mm1 */
    "  pavgb       %%mm2, %%mm1     \n\t"

    "  movq        %%mm0, %%mm2     \n\t"
    "  movq        %%mm1, %%mm3     \n\t"

    "  punpcklbw   %%mm6, %%mm0     \n\t"
    "  punpcklbw   %%mm4, %%mm1     \n\t"
    "  punpckhbw   %%mm6, %%mm2     \n\t"
    "  punpckhbw   %%mm4, %%mm3     \n\t"

    "  psubsw      %%mm1, %%mm0     \n\t"
    "  psubsw      %%mm3, %%mm2     \n\t"

    "  paddw       %%mm0, %%mm5     \n\t"
    "  paddw       %%mm2, %%mm5     \n\t"

    "  pmaddwd     %%mm0, %%mm0     \n\t"
    "  pmaddwd     %%mm2, %%mm2     \n\t"

    "  paddd       %%mm0, %%mm7     \n\t"
    "  paddd       %%mm2, %%mm7     \n\t"

    "  add         %5, %2           \n\t"       /* Inc pointer into src data */
    "  add         %6, %3           \n\t"       /* Inc pointer into ref data */
    "  add         %6, %4           \n\t"       /* Inc pointer into ref data */

    "  dec         %%edi            \n\t"
    "  jnz 1b                       \n\t"

    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $32, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movq        %%mm5, %%mm0     \n\t"
    "  psrlq       $16, %%mm5       \n\t"
    "  paddw       %%mm0, %%mm5     \n\t"
    "  movd        %%mm5, %%edi     \n\t"
    "  movsx       %%di, %%edi      \n\t"
    "  movl        %%edi, %0        \n\t"

    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $32, %%mm7       \n\t"
    "  paddd       %%mm0, %%mm7     \n\t"
    "  movd        %%mm7, %1        \n\t"

     : "=m" (XSum),
       "=m" (XXSum),
       "+r" (SrcData),
       "+r" (RefDataPtr1),
       "+r" (RefDataPtr2)
     : "m" (SrcStride),
       "m" (RefStride)
     : "edi", "memory"
  );

  /* Compute and return population variance as mis-match metric. */
  return (( (XXSum<<6) - XSum*XSum ));
}

void dsp_mmxext_init(DspFunctions *funcs)
{
  funcs->row_sad8 = row_sad8__mmxext;
  funcs->col_sad8x8 = col_sad8x8__mmxext;
  funcs->sad8x8 = sad8x8__mmxext;
  funcs->sad8x8_thres = sad8x8_thres__mmxext;
  funcs->sad8x8_xy2_thres = sad8x8_xy2_thres__mmxext;
  funcs->inter8x8_err_xy2 = inter8x8_err_xy2__mmxext;
}

#endif /* USE_ASM */
