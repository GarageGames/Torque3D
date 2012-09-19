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
  last mod: $Id: dct.c 13884 2007-09-22 08:38:10Z giles $

 ********************************************************************/

#include "codec_internal.h"
#include "dsp.h"
#include "../cpu.h"

static ogg_int32_t xC1S7 = 64277;
static ogg_int32_t xC2S6 = 60547;
static ogg_int32_t xC3S5 = 54491;
static ogg_int32_t xC4S4 = 46341;
static ogg_int32_t xC5S3 = 36410;
static ogg_int32_t xC6S2 = 25080;
static ogg_int32_t xC7S1 = 12785;

#define SIGNBITDUPPED(X) ((signed )(((X) & 0x80000000)) >> 31)
#define DOROUND(X) ( (SIGNBITDUPPED(X) & (0xffff)) + (X) )

static void fdct_short__c ( ogg_int16_t * InputData, ogg_int16_t * OutputData ){
  int loop;

  ogg_int32_t  is07, is12, is34, is56;
  ogg_int32_t  is0734, is1256;
  ogg_int32_t  id07, id12, id34, id56;

  ogg_int32_t  irot_input_x, irot_input_y;
  ogg_int32_t  icommon_product1;   /* Re-used product  (c4s4 * (s12 - s56)). */
  ogg_int32_t  icommon_product2;   /* Re-used product  (c4s4 * (d12 + d56)). */

  ogg_int32_t  temp1, temp2;         /* intermediate variable for computation */

  ogg_int32_t  InterData[64];
  ogg_int32_t *ip = InterData;
  ogg_int16_t * op = OutputData;
  for (loop = 0; loop < 8; loop++){
    /* Pre calculate some common sums and differences. */
    is07 = InputData[0] + InputData[7];
    is12 = InputData[1] + InputData[2];
    is34 = InputData[3] + InputData[4];
    is56 = InputData[5] + InputData[6];

    id07 = InputData[0] - InputData[7];
    id12 = InputData[1] - InputData[2];
    id34 = InputData[3] - InputData[4];
    id56 = InputData[5] - InputData[6];

    is0734 = is07 + is34;
    is1256 = is12 + is56;

    /* Pre-Calculate some common product terms. */
    icommon_product1 = xC4S4*(is12 - is56);
    icommon_product1 = DOROUND(icommon_product1);
    icommon_product1>>=16;

    icommon_product2 = xC4S4*(id12 + id56);
    icommon_product2 = DOROUND(icommon_product2);
    icommon_product2>>=16;


    ip[0] = (xC4S4*(is0734 + is1256));
    ip[0] = DOROUND(ip[0]);
    ip[0] >>= 16;

    ip[4] = (xC4S4*(is0734 - is1256));
    ip[4] = DOROUND(ip[4]);
    ip[4] >>= 16;

    /* Define inputs to rotation for outputs 2 and 6 */
    irot_input_x = id12 - id56;
    irot_input_y = is07 - is34;

    /* Apply rotation for outputs 2 and 6.  */
    temp1=xC6S2*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC2S6*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[2] = temp1 + temp2;

    temp1=xC6S2*irot_input_y;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC2S6*irot_input_x ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[6] = temp1 -temp2 ;

    /* Define inputs to rotation for outputs 1 and 7  */
    irot_input_x = icommon_product1 + id07;
    irot_input_y = -( id34 + icommon_product2 );

    /* Apply rotation for outputs 1 and 7.  */

    temp1=xC1S7*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC7S1*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[1] = temp1 - temp2;

    temp1=xC7S1*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC1S7*irot_input_y ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[7] = temp1 + temp2 ;

    /* Define inputs to rotation for outputs 3 and 5 */
    irot_input_x = id07 - icommon_product1;
    irot_input_y = id34 - icommon_product2;

    /* Apply rotation for outputs 3 and 5. */
    temp1=xC3S5*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC5S3*irot_input_y ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[3] = temp1 - temp2 ;

    temp1=xC5S3*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC3S5*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    ip[5] = temp1 + temp2;

    /* Increment data pointer for next row. */
    InputData += 8 ;
    ip += 8; /* advance pointer to next row */

  }


  /* Performed DCT on rows, now transform the columns */
  ip = InterData;
  for (loop = 0; loop < 8; loop++){
    /* Pre calculate some common sums and differences.  */
    is07 = ip[0 * 8] + ip[7 * 8];
    is12 = ip[1 * 8] + ip[2 * 8];
    is34 = ip[3 * 8] + ip[4 * 8];
    is56 = ip[5 * 8] + ip[6 * 8];

    id07 = ip[0 * 8] - ip[7 * 8];
    id12 = ip[1 * 8] - ip[2 * 8];
    id34 = ip[3 * 8] - ip[4 * 8];
    id56 = ip[5 * 8] - ip[6 * 8];

    is0734 = is07 + is34;
    is1256 = is12 + is56;

    /* Pre-Calculate some common product terms. */
    icommon_product1 = xC4S4*(is12 - is56) ;
    icommon_product2 = xC4S4*(id12 + id56) ;
    icommon_product1 = DOROUND(icommon_product1);
    icommon_product2 = DOROUND(icommon_product2);
    icommon_product1>>=16;
    icommon_product2>>=16;


    temp1 = xC4S4*(is0734 + is1256) ;
    temp2 = xC4S4*(is0734 - is1256) ;
    temp1 = DOROUND(temp1);
    temp2 = DOROUND(temp2);
    temp1>>=16;
    temp2>>=16;
    op[0*8] = (ogg_int16_t) temp1;
    op[4*8] = (ogg_int16_t) temp2;

    /* Define inputs to rotation for outputs 2 and 6 */
    irot_input_x = id12 - id56;
    irot_input_y = is07 - is34;

    /* Apply rotation for outputs 2 and 6.  */
    temp1=xC6S2*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC2S6*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[2*8] = (ogg_int16_t) (temp1 + temp2);

    temp1=xC6S2*irot_input_y;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC2S6*irot_input_x ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[6*8] = (ogg_int16_t) (temp1 -temp2) ;

    /* Define inputs to rotation for outputs 1 and 7 */
    irot_input_x = icommon_product1 + id07;
    irot_input_y = -( id34 + icommon_product2 );

    /* Apply rotation for outputs 1 and 7. */
    temp1=xC1S7*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC7S1*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[1*8] = (ogg_int16_t) (temp1 - temp2);

    temp1=xC7S1*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC1S7*irot_input_y ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[7*8] = (ogg_int16_t) (temp1 + temp2);

    /* Define inputs to rotation for outputs 3 and 5 */
    irot_input_x = id07 - icommon_product1;
    irot_input_y = id34 - icommon_product2;

    /* Apply rotation for outputs 3 and 5. */
    temp1=xC3S5*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC5S3*irot_input_y ;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[3*8] = (ogg_int16_t) (temp1 - temp2) ;

    temp1=xC5S3*irot_input_x;
    temp1=DOROUND(temp1);
    temp1>>=16;
    temp2=xC3S5*irot_input_y;
    temp2=DOROUND(temp2);
    temp2>>=16;
    op[5*8] = (ogg_int16_t) (temp1 + temp2);

    /* Increment data pointer for next column.  */
    ip ++;
    op ++;
  }
}

void dsp_dct_init (DspFunctions *funcs, ogg_uint32_t cpu_flags)
{
  funcs->fdct_short = fdct_short__c;
  dsp_dct_decode_init(funcs, cpu_flags);
  dsp_idct_init(funcs, cpu_flags);
#if defined(USE_ASM)
  if (cpu_flags & OC_CPU_X86_MMX) {
    dsp_mmx_fdct_init(funcs);
  }
#endif
}

