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
    last mod: $Id: fdct_mmx.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

/* mmx fdct implementation */

#include "theora/theora.h"
#include "../codec_internal.h"
#include "../dsp.h"

#if defined(USE_ASM)

static const __attribute__ ((aligned(8),used)) ogg_int64_t xC1S7 = 0x0fb15fb15fb15fb15LL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC2S6 = 0x0ec83ec83ec83ec83LL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC3S5 = 0x0d4dbd4dbd4dbd4dbLL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC4S4 = 0x0b505b505b505b505LL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC5S3 = 0x08e3a8e3a8e3a8e3aLL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC6S2 = 0x061f861f861f861f8LL;
static const __attribute__ ((aligned(8),used)) ogg_int64_t xC7S1 = 0x031f131f131f131f1LL;

/* execute stage 1 of forward DCT */
#define Fdct_mmx(ip0,ip1,ip2,ip3,ip4,ip5,ip6,ip7,temp)                        \
  "  movq      " #ip0 ", %%mm0      \n\t"                                     \
  "  movq      " #ip1 ", %%mm1      \n\t"                                     \
  "  movq      " #ip3 ", %%mm2      \n\t"                                     \
  "  movq      " #ip5 ", %%mm3      \n\t"                                     \
  "  movq        %%mm0, %%mm4       \n\t"                                     \
  "  movq        %%mm1, %%mm5       \n\t"                                     \
  "  movq        %%mm2, %%mm6       \n\t"                                     \
  "  movq        %%mm3, %%mm7       \n\t"                                     \
                                                                              \
  "  paddsw    " #ip7 ", %%mm0      \n\t" /* mm0 = ip0 + ip7 = is07 */        \
  "  paddsw    " #ip2 ", %%mm1      \n\t" /* mm1 = ip1 + ip2 = is12 */        \
  "  paddsw    " #ip4 ", %%mm2      \n\t" /* mm2 = ip3 + ip4 = is34 */        \
  "  paddsw    " #ip6 ", %%mm3      \n\t" /* mm3 = ip5 + ip6 = is56 */        \
  "  psubsw    " #ip7 ", %%mm4      \n\t" /* mm4 = ip0 - ip7 = id07 */        \
  "  psubsw    " #ip2 ", %%mm5      \n\t" /* mm5 = ip1 - ip2 = id12 */        \
                                                                              \
  "  psubsw      %%mm2, %%mm0       \n\t" /* mm0 = is07 - is34 */             \
                                                                              \
  "  paddsw      %%mm2, %%mm2       \n\t"                                     \
                                                                              \
  "  psubsw    " #ip4 ", %%mm6      \n\t" /* mm6 = ip3 - ip4 = id34 */        \
                                                                              \
  "  paddsw      %%mm0, %%mm2       \n\t" /* mm2 = is07 + is34 = is0734 */    \
  "  psubsw      %%mm3, %%mm1       \n\t" /* mm1 = is12 - is56 */             \
  "  movq        %%mm0," #temp "    \n\t" /* Save is07 - is34 to free mm0; */ \
  "  paddsw      %%mm3, %%mm3       \n\t"                                     \
  "  paddsw      %%mm1, %%mm3       \n\t" /* mm3 = is12 + 1s56  = is1256 */   \
                                                                              \
  "  psubsw    " #ip6 ", %%mm7      \n\t" /* mm7 = ip5 - ip6 = id56 */        \
  /* ------------------------------------------------------------------- */   \
  "  psubsw      %%mm7, %%mm5       \n\t" /* mm5 = id12 - id56 */             \
  "  paddsw      %%mm7, %%mm7       \n\t"                                     \
  "  paddsw      %%mm5, %%mm7       \n\t" /* mm7 = id12 + id56 */             \
  /* ------------------------------------------------------------------- */   \
  "  psubsw      %%mm3, %%mm2       \n\t" /* mm2 = is0734 - is1256 */         \
  "  paddsw      %%mm3, %%mm3       \n\t"                                     \
                                                                              \
  "  movq        %%mm2, %%mm0       \n\t" /* make a copy */                   \
  "  paddsw      %%mm2, %%mm3       \n\t" /* mm3 = is0734 + is1256 */         \
                                                                              \
  "  pmulhw      %[xC4S4], %%mm0    \n\t" /* mm0 = xC4S4 * ( is0734 - is1256 ) - ( is0734 - is1256 ) */ \
  "  paddw       %%mm2, %%mm0       \n\t" /* mm0 = xC4S4 * ( is0734 - is1256 ) */ \
  "  psrlw       $15, %%mm2         \n\t"                                     \
  "  paddw       %%mm2, %%mm0       \n\t" /* Truncate mm0, now it is op[4] */ \
                                                                              \
  "  movq        %%mm3, %%mm2       \n\t"                                     \
  "  movq        %%mm0," #ip4 "     \n\t" /* save ip4, now mm0,mm2 are free */ \
                                                                              \
  "  movq        %%mm3, %%mm0       \n\t"                                     \
  "  pmulhw      %[xC4S4], %%mm3    \n\t" /* mm3 = xC4S4 * ( is0734 +is1256 ) - ( is0734 +is1256 ) */ \
                                                                              \
  "  psrlw       $15, %%mm2         \n\t"                                     \
  "  paddw       %%mm0, %%mm3       \n\t" /* mm3 = xC4S4 * ( is0734 +is1256 )    */ \
  "  paddw       %%mm2, %%mm3       \n\t" /* Truncate mm3, now it is op[0] */ \
                                                                              \
  "  movq        %%mm3," #ip0 "     \n\t"                                     \
  /* ------------------------------------------------------------------- */   \
  "  movq      " #temp ", %%mm3     \n\t" /* mm3 = irot_input_y */            \
  "  pmulhw      %[xC2S6], %%mm3    \n\t" /* mm3 = xC2S6 * irot_input_y - irot_input_y */ \
                                                                              \
  "  movq      " #temp ", %%mm2     \n\t"                                     \
  "  movq        %%mm2, %%mm0       \n\t"                                     \
                                                                              \
  "  psrlw       $15, %%mm2         \n\t" /* mm3 = xC2S6 * irot_input_y */    \
  "  paddw       %%mm0, %%mm3       \n\t"                                     \
                                                                              \
  "  paddw       %%mm2, %%mm3       \n\t" /* Truncated */                     \
  "  movq        %%mm5, %%mm0       \n\t"                                     \
                                                                              \
  "  movq        %%mm5, %%mm2       \n\t"                                     \
  "  pmulhw      %[xC6S2], %%mm0    \n\t" /* mm0 = xC6S2 * irot_input_x */    \
                                                                              \
  "  psrlw       $15, %%mm2         \n\t"                                     \
  "  paddw       %%mm2, %%mm0       \n\t" /* Truncated */                     \
                                                                              \
  "  paddsw      %%mm0, %%mm3       \n\t" /* ip[2] */                         \
  "  movq        %%mm3," #ip2 "     \n\t" /* Save ip2 */                      \
                                                                              \
  "  movq        %%mm5, %%mm0       \n\t"                                     \
  "  movq        %%mm5, %%mm2       \n\t"                                     \
                                                                              \
  "  pmulhw     %[xC2S6], %%mm5     \n\t" /* mm5 = xC2S6 * irot_input_x - irot_input_x */ \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  movq      " #temp ", %%mm3     \n\t"                                     \
  "  paddw       %%mm0, %%mm5       \n\t" /* mm5 = xC2S6 * irot_input_x */    \
                                                                              \
  "  paddw       %%mm2, %%mm5       \n\t" /* Truncated */                     \
  "  movq        %%mm3, %%mm2       \n\t"                                     \
                                                                              \
  "  pmulhw      %[xC6S2], %%mm3    \n\t" /* mm3 = xC6S2 * irot_input_y */    \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  paddw       %%mm2, %%mm3       \n\t" /* Truncated */                     \
  "  psubsw      %%mm5, %%mm3       \n\t"                                     \
                                                                              \
  "  movq        %%mm3," #ip6 "     \n\t"                                     \
  /* ------------------------------------------------------------------- */   \
  "  movq        %[xC4S4], %%mm0    \n\t"                                     \
  "  movq        %%mm1, %%mm2       \n\t"                                     \
  "  movq        %%mm1, %%mm3       \n\t"                                     \
                                                                              \
  "  pmulhw      %%mm0, %%mm1       \n\t" /* mm0 = xC4S4 * ( is12 - is56 ) - ( is12 - is56 ) */ \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  paddw       %%mm3, %%mm1       \n\t" /* mm0 = xC4S4 * ( is12 - is56 ) */ \
  "  paddw       %%mm2, %%mm1       \n\t" /* Truncate mm1, now it is icommon_product1 */ \
                                                                              \
  "  movq        %%mm7, %%mm2       \n\t"                                     \
  "  movq        %%mm7, %%mm3       \n\t"                                     \
                                                                              \
  "  pmulhw      %%mm0, %%mm7       \n\t" /* mm7 = xC4S4 * ( id12 + id56 ) - ( id12 + id56 ) */ \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  paddw       %%mm3, %%mm7       \n\t" /* mm7 = xC4S4 * ( id12 + id56 ) */ \
  "  paddw       %%mm2, %%mm7       \n\t" /* Truncate mm7, now it is icommon_product2 */ \
  /* ------------------------------------------------------------------- */   \
  "  pxor        %%mm0, %%mm0       \n\t" /* Clear mm0 */                     \
  "  psubsw      %%mm6, %%mm0       \n\t" /* mm0 = - id34 */                  \
                                                                              \
  "  psubsw      %%mm7, %%mm0       \n\t" /* mm0 = - ( id34 + idcommon_product2 ) */ \
  "  paddsw      %%mm6, %%mm6       \n\t"                                     \
  "  paddsw      %%mm0, %%mm6       \n\t" /* mm6 = id34 - icommon_product2 */ \
                                                                              \
  "  psubsw      %%mm1, %%mm4       \n\t" /* mm4 = id07 - icommon_product1 */ \
  "  paddsw      %%mm1, %%mm1       \n\t"                                     \
  "  paddsw      %%mm4, %%mm1       \n\t" /* mm1 = id07 + icommon_product1 */ \
  /* ------------------------------------------------------------------- */   \
  "  movq        %[xC1S7], %%mm7    \n\t"                                     \
  "  movq        %%mm1, %%mm2       \n\t"                                     \
                                                                              \
  "  movq        %%mm1, %%mm3       \n\t"                                     \
  "  pmulhw      %%mm7, %%mm1       \n\t" /* mm1 = xC1S7 * irot_input_x - irot_input_x */ \
                                                                              \
  "  movq        %[xC7S1], %%mm7    \n\t"                                     \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  paddw       %%mm3, %%mm1       \n\t" /* mm1 = xC1S7 * irot_input_x */    \
  "  paddw       %%mm2, %%mm1       \n\t" /* Trucated */                      \
                                                                              \
  "  pmulhw      %%mm7, %%mm3       \n\t" /* mm3 = xC7S1 * irot_input_x */    \
  "  paddw       %%mm2, %%mm3       \n\t" /* Truncated */                     \
                                                                              \
  "  movq        %%mm0, %%mm5       \n\t"                                     \
  "  movq        %%mm0, %%mm2       \n\t"                                     \
                                                                              \
  "  movq        %[xC1S7], %%mm7    \n\t"                                     \
  "  pmulhw      %%mm7, %%mm0       \n\t" /* mm0 = xC1S7 * irot_input_y - irot_input_y */ \
                                                                              \
  "  movq        %[xC7S1], %%mm7    \n\t"                                     \
  "  psrlw       $15, %%mm2         \n\t"                                     \
                                                                              \
  "  paddw       %%mm5, %%mm0       \n\t" /* mm0 = xC1S7 * irot_input_y */    \
  "  paddw       %%mm2, %%mm0       \n\t" /* Truncated */                     \
                                                                              \
  "  pmulhw      %%mm7, %%mm5       \n\t" /* mm5 = xC7S1 * irot_input_y */    \
  "  paddw       %%mm2, %%mm5       \n\t" /* Truncated */                     \
                                                                              \
  "  psubsw      %%mm5, %%mm1       \n\t" /* mm1 = xC1S7 * irot_input_x - xC7S1 * irot_input_y = ip1 */ \
  "  paddsw      %%mm0, %%mm3       \n\t" /* mm3 = xC7S1 * irot_input_x - xC1S7 * irot_input_y = ip7 */ \
                                                                              \
  "  movq        %%mm1," #ip1 "     \n\t"                                     \
  "  movq        %%mm3," #ip7 "     \n\t"                                     \
  /* ------------------------------------------------------------------- */   \
  "  movq        %[xC3S5], %%mm0    \n\t"                                     \
  "  movq        %[xC5S3], %%mm1    \n\t"                                     \
                                                                              \
  "  movq        %%mm6, %%mm5       \n\t"                                     \
  "  movq        %%mm6, %%mm7       \n\t"                                     \
                                                                              \
  "  movq        %%mm4, %%mm2       \n\t"                                     \
  "  movq        %%mm4, %%mm3       \n\t"                                     \
                                                                              \
  "  pmulhw      %%mm0, %%mm4       \n\t" /* mm4 = xC3S5 * irot_input_x - irot_input_x */ \
  "  pmulhw      %%mm1, %%mm6       \n\t" /* mm6 = xC5S3 * irot_input_y - irot_input_y */ \
                                                                              \
  "  psrlw       $15, %%mm2         \n\t"                                     \
  "  psrlw       $15, %%mm5         \n\t"                                     \
                                                                              \
  "  paddw       %%mm3, %%mm4       \n\t" /* mm4 = xC3S5 * irot_input_x */    \
  "  paddw       %%mm7, %%mm6       \n\t" /* mm6 = xC5S3 * irot_input_y */    \
                                                                              \
  "  paddw       %%mm2, %%mm4       \n\t" /* Truncated */                     \
  "  paddw       %%mm5, %%mm6       \n\t" /* Truncated */                     \
                                                                              \
  "  psubsw      %%mm6, %%mm4       \n\t" /* ip3 */                           \
  "  movq        %%mm4," #ip3 "     \n\t"                                     \
                                                                              \
  "  movq        %%mm3, %%mm4       \n\t"                                     \
  "  movq        %%mm7, %%mm6       \n\t"                                     \
                                                                              \
  "  pmulhw      %%mm1, %%mm3       \n\t" /* mm3 = xC5S3 * irot_input_x - irot_input_x */ \
  "  pmulhw      %%mm0, %%mm7       \n\t" /* mm7 = xC3S5 * irot_input_y - irot_input_y */ \
                                                                              \
  "  paddw       %%mm2, %%mm4       \n\t"                                     \
  "  paddw       %%mm5, %%mm6       \n\t"                                     \
                                                                              \
  "  paddw       %%mm4, %%mm3       \n\t" /* mm3 = xC5S3 * irot_input_x */    \
  "  paddw       %%mm6, %%mm7       \n\t" /* mm7 = xC3S5 * irot_input_y */    \
                                                                              \
  "  paddw       %%mm7, %%mm3       \n\t" /* ip5 */                           \
  "  movq        %%mm3," #ip5 "     \n\t"

#define Transpose_mmx(ip0,ip1,ip2,ip3,ip4,ip5,ip6,ip7,                  \
                      op0,op1,op2,op3,op4,op5,op6,op7)                  \
  "  movq      " #ip0 ", %%mm0      \n\t" /* mm0 = a0 a1 a2 a3 */       \
  "  movq      " #ip4 ", %%mm4      \n\t" /* mm4 = e4 e5 e6 e7 */       \
  "  movq      " #ip1 ", %%mm1      \n\t" /* mm1 = b0 b1 b2 b3 */       \
  "  movq      " #ip5 ", %%mm5      \n\t" /* mm5 = f4 f5 f6 f7 */       \
  "  movq      " #ip2 ", %%mm2      \n\t" /* mm2 = c0 c1 c2 c3 */       \
  "  movq      " #ip6 ", %%mm6      \n\t" /* mm6 = g4 g5 g6 g7 */       \
  "  movq      " #ip3 ", %%mm3      \n\t" /* mm3 = d0 d1 d2 d3 */       \
  "  movq        %%mm1," #op1 "     \n\t" /* save  b0 b1 b2 b3 */       \
  "  movq      " #ip7 ", %%mm7      \n\t" /* mm7 = h0 h1 h2 h3 */       \
   /* Transpose 2x8 block */                                            \
  "  movq        %%mm4, %%mm1       \n\t" /* mm1 = e3 e2 e1 e0 */       \
  "  punpcklwd   %%mm5, %%mm4       \n\t" /* mm4 = f1 e1 f0 e0 */       \
  "  movq        %%mm0," #op0 "     \n\t" /* save a3 a2 a1 a0  */       \
  "  punpckhwd   %%mm5, %%mm1       \n\t" /* mm1 = f3 e3 f2 e2 */       \
  "  movq        %%mm6, %%mm0       \n\t" /* mm0 = g3 g2 g1 g0 */       \
  "  punpcklwd   %%mm7, %%mm6       \n\t" /* mm6 = h1 g1 h0 g0 */       \
  "  movq        %%mm4, %%mm5       \n\t" /* mm5 = f1 e1 f0 e0 */       \
  "  punpckldq   %%mm6, %%mm4       \n\t" /* mm4 = h0 g0 f0 e0 = MM4 */ \
  "  punpckhdq   %%mm6, %%mm5       \n\t" /* mm5 = h1 g1 f1 e1 = MM5 */ \
  "  movq        %%mm1, %%mm6       \n\t" /* mm6 = f3 e3 f2 e2 */       \
  "  movq        %%mm4," #op4 "     \n\t"                               \
  "  punpckhwd   %%mm7, %%mm0       \n\t" /* mm0 = h3 g3 h2 g2 */       \
  "  movq        %%mm5," #op5 "     \n\t"                               \
  "  punpckhdq   %%mm0, %%mm6       \n\t" /* mm6 = h3 g3 f3 e3 = MM7 */ \
  "  movq      " #op0 ", %%mm4      \n\t" /* mm4 = a3 a2 a1 a0 */       \
  "  punpckldq   %%mm0, %%mm1       \n\t" /* mm1 = h2 g2 f2 e2 = MM6 */ \
  "  movq      " #op1 ", %%mm5      \n\t" /* mm5 = b3 b2 b1 b0 */       \
  "  movq        %%mm4, %%mm0       \n\t" /* mm0 = a3 a2 a1 a0 */       \
  "  movq        %%mm6," #op7 "     \n\t"                               \
  "  punpcklwd   %%mm5, %%mm0       \n\t" /* mm0 = b1 a1 b0 a0 */       \
  "  movq        %%mm1," #op6 "     \n\t"                               \
  "  punpckhwd   %%mm5, %%mm4       \n\t" /* mm4 = b3 a3 b2 a2 */       \
  "  movq        %%mm2, %%mm5       \n\t" /* mm5 = c3 c2 c1 c0 */       \
  "  punpcklwd   %%mm3, %%mm2       \n\t" /* mm2 = d1 c1 d0 c0 */       \
  "  movq        %%mm0, %%mm1       \n\t" /* mm1 = b1 a1 b0 a0 */       \
  "  punpckldq   %%mm2, %%mm0       \n\t" /* mm0 = d0 c0 b0 a0 = MM0 */ \
  "  punpckhdq   %%mm2, %%mm1       \n\t" /* mm1 = d1 c1 b1 a1 = MM1 */ \
  "  movq        %%mm4, %%mm2       \n\t" /* mm2 = b3 a3 b2 a2 */       \
  "  movq        %%mm0," #op0 "     \n\t"                               \
  "  punpckhwd   %%mm3, %%mm5       \n\t" /* mm5 = d3 c3 d2 c2 */       \
  "  movq        %%mm1," #op1 "     \n\t"                               \
  "  punpckhdq   %%mm5, %%mm4       \n\t" /* mm4 = d3 c3 b3 a3 = MM3 */ \
  "  punpckldq   %%mm5, %%mm2       \n\t" /* mm2 = d2 c2 b2 a2 = MM2 */ \
  "  movq        %%mm4," #op3 "     \n\t"                               \
  "  movq        %%mm2," #op2 "     \n\t"


/* This performs a 2D Forward DCT on an 8x8 block with short
   coefficients. We try to do the truncation to match the C
   version. */
static void fdct_short__mmx ( ogg_int16_t *InputData, ogg_int16_t *OutputData)
{
  ogg_int16_t __attribute__((aligned(8))) temp[8*8];

  __asm__ __volatile__ (
    "  .p2align 4                   \n\t"
    /*
     * Input data is an 8x8 block.  To make processing of the data more efficent
     * we will transpose the block of data to two 4x8 blocks???
     */
    Transpose_mmx (  (%0), 16(%0), 32(%0), 48(%0),  8(%0), 24(%0), 40(%0), 56(%0),
                     (%1), 16(%1), 32(%1), 48(%1),  8(%1), 24(%1), 40(%1), 56(%1))
    Fdct_mmx      (  (%1), 16(%1), 32(%1), 48(%1),  8(%1), 24(%1), 40(%1), 56(%1), (%2))

    Transpose_mmx (64(%0), 80(%0), 96(%0),112(%0), 72(%0), 88(%0),104(%0),120(%0),
                   64(%1), 80(%1), 96(%1),112(%1), 72(%1), 88(%1),104(%1),120(%1))
    Fdct_mmx      (64(%1), 80(%1), 96(%1),112(%1), 72(%1), 88(%1),104(%1),120(%1), (%2))

    Transpose_mmx ( 0(%1), 16(%1), 32(%1), 48(%1), 64(%1), 80(%1), 96(%1),112(%1),
                    0(%1), 16(%1), 32(%1), 48(%1), 64(%1), 80(%1), 96(%1),112(%1))
    Fdct_mmx      ( 0(%1), 16(%1), 32(%1), 48(%1), 64(%1), 80(%1), 96(%1),112(%1), (%2))

    Transpose_mmx ( 8(%1), 24(%1), 40(%1), 56(%1), 72(%1), 88(%1),104(%1),120(%1),
                    8(%1), 24(%1), 40(%1), 56(%1), 72(%1), 88(%1),104(%1),120(%1))
    Fdct_mmx      ( 8(%1), 24(%1), 40(%1), 56(%1), 72(%1), 88(%1),104(%1),120(%1), (%2))

    "  emms                         \n\t"

    : "+r" (InputData),
      "+r" (OutputData)
    : "r" (temp),
      [xC1S7] "m" (xC1S7),      /* gcc 3.1+ allows named asm parameters */
      [xC2S6] "m" (xC2S6),
      [xC3S5] "m" (xC3S5),
      [xC4S4] "m" (xC4S4),
      [xC5S3] "m" (xC5S3),
      [xC6S2] "m" (xC6S2),
      [xC7S1] "m" (xC7S1)
    : "memory"
  );
}

/* install our implementation in the function table */
void dsp_mmx_fdct_init(DspFunctions *funcs)
{
  funcs->fdct_short = fdct_short__mmx;
}

#endif /* USE_ASM */
