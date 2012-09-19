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
  last mod: $Id: dsp_mmx.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#include <stdlib.h>

#include "../codec_internal.h"
#include "../dsp.h"

#if defined(USE_ASM)

static const __attribute__ ((aligned(8),used)) ogg_int64_t V128 = 0x0080008000800080LL;

#define DSP_OP_AVG(a,b) ((((int)(a)) + ((int)(b)))/2)
#define DSP_OP_DIFF(a,b) (((int)(a)) - ((int)(b)))
#define DSP_OP_ABS_DIFF(a,b) abs((((int)(a)) - ((int)(b))))

#define SUB_LOOP                                                              \
  "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */                   \
  "  movq        (%1), %%mm1      \n\t" /* mm1 = ReconPtr */                  \
  "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */\
  "  movq        %%mm1, %%mm3     \n\t" /* dup to prepare for up conversion */\
  /* convert from UINT8 to INT16 */                                           \
  "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */            \
  "  punpcklbw   %%mm7, %%mm1     \n\t" /* mm1 = INT16(ReconPtr) */           \
  "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */            \
  "  punpckhbw   %%mm7, %%mm3     \n\t" /* mm3 = INT16(ReconPtr) */           \
  /* start calculation */                                                     \
  "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - ReconPtr */        \
  "  psubw       %%mm3, %%mm2     \n\t" /* mm2 = FiltPtr - ReconPtr */        \
  "  movq        %%mm0,  (%2)     \n\t" /* write answer out */                \
  "  movq        %%mm2, 8(%2)     \n\t" /* write answer out */                \
  /* Increment pointers */                                                    \
  "  add         $16, %2           \n\t"                                      \
  "  add         %3, %0           \n\t"                                       \
  "  add         %4, %1           \n\t"

static void sub8x8__mmx (unsigned char *FiltPtr, unsigned char *ReconPtr,
                  ogg_int16_t *DctInputPtr, ogg_uint32_t PixelsPerLine,
                  ogg_uint32_t ReconPixelsPerLine)
{
  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
    SUB_LOOP
     : "+r" (FiltPtr),
       "+r" (ReconPtr),
       "+r" (DctInputPtr)
     : "m" (PixelsPerLine),
       "m" (ReconPixelsPerLine)
     : "memory"
  );
}

#define SUB_128_LOOP                                                          \
  "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */                   \
  "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */\
  /* convert from UINT8 to INT16 */                                           \
  "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */            \
  "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */            \
  /* start calculation */                                                     \
  "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - 128 */             \
  "  psubw       %%mm1, %%mm2     \n\t" /* mm2 = FiltPtr - 128 */             \
  "  movq        %%mm0,  (%1)     \n\t" /* write answer out */                \
  "  movq        %%mm2, 8(%1)     \n\t" /* write answer out */                \
  /* Increment pointers */                                                    \
  "  add         $16, %1           \n\t"                                      \
  "  add         %2, %0           \n\t"


static void sub8x8_128__mmx (unsigned char *FiltPtr, ogg_int16_t *DctInputPtr,
                      ogg_uint32_t PixelsPerLine)
{
  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"
    "  movq        %[V128], %%mm1   \n\t"
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
    SUB_128_LOOP
     : "+r" (FiltPtr),
       "+r" (DctInputPtr)
     : "m" (PixelsPerLine),
       [V128] "m" (V128)
     : "memory"
  );
}

#define SUB_AVG2_LOOP                                                         \
  "  movq        (%0), %%mm0      \n\t" /* mm0 = FiltPtr */                   \
  "  movq        (%1), %%mm1      \n\t" /* mm1 = ReconPtr1 */                 \
  "  movq        (%2), %%mm4      \n\t" /* mm1 = ReconPtr2 */                 \
  "  movq        %%mm0, %%mm2     \n\t" /* dup to prepare for up conversion */\
  "  movq        %%mm1, %%mm3     \n\t" /* dup to prepare for up conversion */\
  "  movq        %%mm4, %%mm5     \n\t" /* dup to prepare for up conversion */\
  /* convert from UINT8 to INT16 */                                           \
  "  punpcklbw   %%mm7, %%mm0     \n\t" /* mm0 = INT16(FiltPtr) */            \
  "  punpcklbw   %%mm7, %%mm1     \n\t" /* mm1 = INT16(ReconPtr1) */          \
  "  punpcklbw   %%mm7, %%mm4     \n\t" /* mm1 = INT16(ReconPtr2) */          \
  "  punpckhbw   %%mm7, %%mm2     \n\t" /* mm2 = INT16(FiltPtr) */            \
  "  punpckhbw   %%mm7, %%mm3     \n\t" /* mm3 = INT16(ReconPtr1) */          \
  "  punpckhbw   %%mm7, %%mm5     \n\t" /* mm3 = INT16(ReconPtr2) */          \
  /* average ReconPtr1 and ReconPtr2 */                                       \
  "  paddw       %%mm4, %%mm1     \n\t" /* mm1 = ReconPtr1 + ReconPtr2 */     \
  "  paddw       %%mm5, %%mm3     \n\t" /* mm3 = ReconPtr1 + ReconPtr2 */     \
  "  psrlw       $1, %%mm1        \n\t" /* mm1 = (ReconPtr1 + ReconPtr2) / 2 */ \
  "  psrlw       $1, %%mm3        \n\t" /* mm3 = (ReconPtr1 + ReconPtr2) / 2 */ \
  "  psubw       %%mm1, %%mm0     \n\t" /* mm0 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2) */ \
  "  psubw       %%mm3, %%mm2     \n\t" /* mm2 = FiltPtr - ((ReconPtr1 + ReconPtr2) / 2) */ \
  "  movq        %%mm0,  (%3)     \n\t" /* write answer out */                \
  "  movq        %%mm2, 8(%3)     \n\t" /* write answer out */                \
  /* Increment pointers */                                                    \
  "  add         $16, %3           \n\t"                                      \
  "  add         %4, %0           \n\t"                                       \
  "  add         %5, %1           \n\t"                                       \
  "  add         %5, %2           \n\t"


static void sub8x8avg2__mmx (unsigned char *FiltPtr, unsigned char *ReconPtr1,
                     unsigned char *ReconPtr2, ogg_int16_t *DctInputPtr,
                     ogg_uint32_t PixelsPerLine,
                     ogg_uint32_t ReconPixelsPerLine)
{
  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm7, %%mm7     \n\t"
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
    SUB_AVG2_LOOP
     : "+r" (FiltPtr),
       "+r" (ReconPtr1),
       "+r" (ReconPtr2),
       "+r" (DctInputPtr)
     : "m" (PixelsPerLine),
       "m" (ReconPixelsPerLine)
     : "memory"
  );
}

static ogg_uint32_t row_sad8__mmx (unsigned char *Src1, unsigned char *Src2)
{
  ogg_uint32_t MaxSad;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm6, %%mm6     \n\t"       /* zero out mm6 for unpack */
    "  pxor        %%mm7, %%mm7     \n\t"       /* zero out mm7 for unpack */
    "  movq        (%1), %%mm0      \n\t"       /* take 8 bytes */
    "  movq        (%2), %%mm1      \n\t"

    "  movq        %%mm0, %%mm2     \n\t"
    "  psubusb     %%mm1, %%mm0     \n\t"       /* A - B */
    "  psubusb     %%mm2, %%mm1     \n\t"       /* B - A */
    "  por         %%mm1, %%mm0     \n\t"       /* and or gives abs difference */

    "  movq        %%mm0, %%mm1     \n\t"

    "  punpcklbw   %%mm6, %%mm0     \n\t"       /* ; unpack low four bytes to higher precision */
    "  punpckhbw   %%mm7, %%mm1     \n\t"       /* ; unpack high four bytes to higher precision */

    "  movq        %%mm0, %%mm2     \n\t"
    "  movq        %%mm1, %%mm3     \n\t"
    "  psrlq       $32, %%mm2       \n\t"       /* fold and add */
    "  psrlq       $32, %%mm3       \n\t"
    "  paddw       %%mm2, %%mm0     \n\t"
    "  paddw       %%mm3, %%mm1     \n\t"
    "  movq        %%mm0, %%mm2     \n\t"
    "  movq        %%mm1, %%mm3     \n\t"
    "  psrlq       $16, %%mm2       \n\t"
    "  psrlq       $16, %%mm3       \n\t"
    "  paddw       %%mm2, %%mm0     \n\t"
    "  paddw       %%mm3, %%mm1     \n\t"

    "  psubusw     %%mm0, %%mm1     \n\t"
    "  paddw       %%mm0, %%mm1     \n\t"       /* mm1 = max(mm1, mm0) */
    "  movd        %%mm1, %0        \n\t"
    "  andl        $0xffff, %0      \n\t"

     : "=m" (MaxSad),
       "+r" (Src1),
       "+r" (Src2)
     :
     : "memory"
  );
  return MaxSad;
}

static ogg_uint32_t col_sad8x8__mmx (unsigned char *Src1, unsigned char *Src2,
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

    "  psubusw     %%mm6, %%mm7     \n\t"
    "  paddw       %%mm6, %%mm7     \n\t"       /* mm7 = max(mm7, mm6) */
    "  psubusw     %%mm4, %%mm5     \n\t"
    "  paddw       %%mm4, %%mm5     \n\t"       /* mm5 = max(mm5, mm4) */
    "  psubusw     %%mm5, %%mm7     \n\t"
    "  paddw       %%mm5, %%mm7     \n\t"       /* mm7 = max(mm5, mm7) */
    "  movq        %%mm7, %%mm6     \n\t"
    "  psrlq       $32, %%mm6       \n\t"
    "  psubusw     %%mm6, %%mm7     \n\t"
    "  paddw       %%mm6, %%mm7     \n\t"       /* mm7 = max(mm5, mm7) */
    "  movq        %%mm7, %%mm6     \n\t"
    "  psrlq       $16, %%mm6       \n\t"
    "  psubusw     %%mm6, %%mm7     \n\t"
    "  paddw       %%mm6, %%mm7     \n\t"       /* mm7 = max(mm5, mm7) */
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

#define SAD_LOOP                                                              \
  "  movq        (%1), %%mm0      \n\t" /* take 8 bytes */                    \
  "  movq        (%2), %%mm1      \n\t"                                       \
  "  movq        %%mm0, %%mm2     \n\t"                                       \
  "  psubusb     %%mm1, %%mm0     \n\t"         /* A - B */                         \
  "  psubusb     %%mm2, %%mm1     \n\t" /* B - A */                           \
  "  por         %%mm1, %%mm0     \n\t" /* and or gives abs difference */     \
  "  movq        %%mm0, %%mm1     \n\t"                                       \
  "  punpcklbw   %%mm6, %%mm0     \n\t" /* unpack to higher precision for accumulation */ \
  "  paddw       %%mm0, %%mm7     \n\t" /* accumulate difference... */        \
  "  punpckhbw   %%mm6, %%mm1     \n\t" /* unpack high four bytes to higher precision */ \
  "  add         %3, %1           \n\t" /* Inc pointer into the new data */   \
  "  paddw       %%mm1, %%mm7     \n\t" /* accumulate difference... */        \
  "  add         %4, %2           \n\t" /* Inc pointer into ref data */

static ogg_uint32_t sad8x8__mmx (unsigned char *ptr1, ogg_uint32_t stride1,
                            unsigned char *ptr2, ogg_uint32_t stride2)
{
  ogg_uint32_t  DiffVal;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"       /* zero out mm6 for unpack */
    "  pxor        %%mm7, %%mm7     \n\t"       /* mm7 contains the result */
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    SAD_LOOP
    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $32, %%mm7       \n\t"
    "  paddw       %%mm0, %%mm7     \n\t"
    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $16, %%mm7       \n\t"
    "  paddw       %%mm0, %%mm7     \n\t"
    "  movd        %%mm7, %0        \n\t"
    "  andl        $0xffff, %0      \n\t"

     : "=m" (DiffVal),
       "+r" (ptr1),
       "+r" (ptr2)
     : "r" (stride1),
       "r" (stride2)
     : "memory"
  );

  return DiffVal;
}

static ogg_uint32_t sad8x8_thres__mmx (unsigned char *ptr1, ogg_uint32_t stride1,
                                  unsigned char *ptr2, ogg_uint32_t stride2,
                                  ogg_uint32_t thres)
{
  return sad8x8__mmx (ptr1, stride1, ptr2, stride2);
}

static ogg_uint32_t sad8x8_xy2_thres__mmx (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                      unsigned char *RefDataPtr1,
                                      unsigned char *RefDataPtr2, ogg_uint32_t RefStride,
                                      ogg_uint32_t thres)
{
  ogg_uint32_t  DiffVal;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pcmpeqd     %%mm5, %%mm5     \n\t"       /* fefefefefefefefe in mm5 */
    "  paddb       %%mm5, %%mm5     \n\t"

    "  pxor        %%mm6, %%mm6     \n\t"       /* zero out mm6 for unpack */
    "  pxor        %%mm7, %%mm7     \n\t"       /* mm7 contains the result */
    "  mov         $8, %%edi        \n\t"       /* 8 rows */
    "1:                             \n\t"
    "  movq        (%1), %%mm0      \n\t"       /* take 8 bytes */

    "  movq        (%2), %%mm2      \n\t"
    "  movq        (%3), %%mm3      \n\t"       /* take average of mm2 and mm3 */
    "  movq        %%mm2, %%mm1     \n\t"
    "  pand        %%mm3, %%mm1     \n\t"
    "  pxor        %%mm2, %%mm3     \n\t"
    "  pand        %%mm5, %%mm3     \n\t"
    "  psrlq       $1, %%mm3        \n\t"
    "  paddb       %%mm3, %%mm1     \n\t"

    "  movq        %%mm0, %%mm2     \n\t"

    "  psubusb     %%mm1, %%mm0     \n\t"       /* A - B */
    "  psubusb     %%mm2, %%mm1     \n\t"       /* B - A */
    "  por         %%mm1, %%mm0     \n\t"       /* and or gives abs difference */
    "  movq        %%mm0, %%mm1     \n\t"

    "  punpcklbw   %%mm6, %%mm0     \n\t"       /* unpack to higher precision for accumulation */
    "  paddw       %%mm0, %%mm7     \n\t"       /* accumulate difference... */
    "  punpckhbw   %%mm6, %%mm1     \n\t"       /* unpack high four bytes to higher precision */
    "  add         %4, %1           \n\t"       /* Inc pointer into the new data */
    "  paddw       %%mm1, %%mm7     \n\t"       /* accumulate difference... */
    "  add         %5, %2           \n\t"       /* Inc pointer into ref data */
    "  add         %5, %3           \n\t"       /* Inc pointer into ref data */

    "  dec         %%edi            \n\t"
    "  jnz 1b                       \n\t"

    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $32, %%mm7       \n\t"
    "  paddw       %%mm0, %%mm7     \n\t"
    "  movq        %%mm7, %%mm0     \n\t"
    "  psrlq       $16, %%mm7       \n\t"
    "  paddw       %%mm0, %%mm7     \n\t"
    "  movd        %%mm7, %0        \n\t"
    "  andl        $0xffff, %0      \n\t"

     : "=m" (DiffVal),
       "+r" (SrcData),
       "+r" (RefDataPtr1),
       "+r" (RefDataPtr2)
     : "m" (SrcStride),
       "m" (RefStride)
     : "edi", "memory"
  );

  return DiffVal;
}

static ogg_uint32_t intra8x8_err__mmx (unsigned char *DataPtr, ogg_uint32_t Stride)
{
  ogg_uint32_t  XSum;
  ogg_uint32_t  XXSum;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%edi        \n\t"
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

     : "=r" (XSum),
       "=r" (XXSum),
       "+r" (DataPtr)
     : "r" (Stride)
     : "edi", "memory"
  );

  /* Compute population variance as mis-match metric. */
  return (( (XXSum<<6) - XSum*XSum ) );
}

static ogg_uint32_t inter8x8_err__mmx (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                 unsigned char *RefDataPtr, ogg_uint32_t RefStride)
{
  ogg_uint32_t  XSum;
  ogg_uint32_t  XXSum;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%edi        \n\t"
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
       "+r" (RefDataPtr)
     : "m" (SrcStride),
       "m" (RefStride)
     : "edi", "memory"
  );

  /* Compute and return population variance as mis-match metric. */
  return (( (XXSum<<6) - XSum*XSum ));
}

static ogg_uint32_t inter8x8_err_xy2__mmx (unsigned char *SrcData, ogg_uint32_t SrcStride,
                                     unsigned char *RefDataPtr1,
                                     unsigned char *RefDataPtr2, ogg_uint32_t RefStride)
{
  ogg_uint32_t XSum;
  ogg_uint32_t XXSum;

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"

    "  pcmpeqd     %%mm4, %%mm4     \n\t"       /* fefefefefefefefe in mm4 */
    "  paddb       %%mm4, %%mm4     \n\t"
    "  pxor        %%mm5, %%mm5     \n\t"
    "  pxor        %%mm6, %%mm6     \n\t"
    "  pxor        %%mm7, %%mm7     \n\t"
    "  mov         $8, %%edi        \n\t"
    "1:                             \n\t"
    "  movq        (%2), %%mm0      \n\t"       /* take 8 bytes */

    "  movq        (%3), %%mm2      \n\t"
    "  movq        (%4), %%mm3      \n\t"       /* take average of mm2 and mm3 */
    "  movq        %%mm2, %%mm1     \n\t"
    "  pand        %%mm3, %%mm1     \n\t"
    "  pxor        %%mm2, %%mm3     \n\t"
    "  pand        %%mm4, %%mm3     \n\t"
    "  psrlq       $1, %%mm3        \n\t"
    "  paddb       %%mm3, %%mm1     \n\t"

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
  funcs->row_sad8 = row_sad8__mmx;
  funcs->col_sad8x8 = col_sad8x8__mmx;
  funcs->sad8x8 = sad8x8__mmx;
  funcs->sad8x8_thres = sad8x8_thres__mmx;
  funcs->sad8x8_xy2_thres = sad8x8_xy2_thres__mmx;
  funcs->intra8x8_err = intra8x8_err__mmx;
  funcs->inter8x8_err = inter8x8_err__mmx;
  funcs->inter8x8_err_xy2 = inter8x8_err_xy2__mmx;
}

#endif /* USE_ASM */
