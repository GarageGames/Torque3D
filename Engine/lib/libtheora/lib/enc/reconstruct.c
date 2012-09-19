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
  last mod: $Id: reconstruct.c 13884 2007-09-22 08:38:10Z giles $

 ********************************************************************/

#include "codec_internal.h"

static void copy8x8__c (unsigned char *src,
                  unsigned char *dest,
                  unsigned int stride)
{
  int j;
  for ( j = 0; j < 8; j++ ){
    ((ogg_uint32_t*)dest)[0] = ((ogg_uint32_t*)src)[0];
    ((ogg_uint32_t*)dest)[1] = ((ogg_uint32_t*)src)[1];
    src+=stride;
    dest+=stride;
  }
}

static void recon_intra8x8__c (unsigned char *ReconPtr, ogg_int16_t *ChangePtr,
                               ogg_uint32_t LineStep)
{
  ogg_uint32_t i;

  for (i = 8; i; i--){
    /* Convert the data back to 8 bit unsigned */
    /* Saturate the output to unsigend 8 bit values */
    ReconPtr[0] = clamp255( ChangePtr[0] + 128 );
    ReconPtr[1] = clamp255( ChangePtr[1] + 128 );
    ReconPtr[2] = clamp255( ChangePtr[2] + 128 );
    ReconPtr[3] = clamp255( ChangePtr[3] + 128 );
    ReconPtr[4] = clamp255( ChangePtr[4] + 128 );
    ReconPtr[5] = clamp255( ChangePtr[5] + 128 );
    ReconPtr[6] = clamp255( ChangePtr[6] + 128 );
    ReconPtr[7] = clamp255( ChangePtr[7] + 128 );

    ReconPtr += LineStep;
    ChangePtr += 8;
  }
}

static void recon_inter8x8__c (unsigned char *ReconPtr, unsigned char *RefPtr,
          ogg_int16_t *ChangePtr, ogg_uint32_t LineStep)
{
  ogg_uint32_t i;

  for (i = 8; i; i--){
    ReconPtr[0] = clamp255(RefPtr[0] + ChangePtr[0]);
    ReconPtr[1] = clamp255(RefPtr[1] + ChangePtr[1]);
    ReconPtr[2] = clamp255(RefPtr[2] + ChangePtr[2]);
    ReconPtr[3] = clamp255(RefPtr[3] + ChangePtr[3]);
    ReconPtr[4] = clamp255(RefPtr[4] + ChangePtr[4]);
    ReconPtr[5] = clamp255(RefPtr[5] + ChangePtr[5]);
    ReconPtr[6] = clamp255(RefPtr[6] + ChangePtr[6]);
    ReconPtr[7] = clamp255(RefPtr[7] + ChangePtr[7]);

    ChangePtr += 8;
    ReconPtr += LineStep;
    RefPtr += LineStep;
  }
}

static void recon_inter8x8_half__c (unsigned char *ReconPtr, unsigned char *RefPtr1,
               unsigned char *RefPtr2, ogg_int16_t *ChangePtr,
         ogg_uint32_t LineStep)
{
  ogg_uint32_t  i;

  for (i = 8; i; i--){
    ReconPtr[0] = clamp255((((int)RefPtr1[0] + (int)RefPtr2[0]) >> 1) + ChangePtr[0] );
    ReconPtr[1] = clamp255((((int)RefPtr1[1] + (int)RefPtr2[1]) >> 1) + ChangePtr[1] );
    ReconPtr[2] = clamp255((((int)RefPtr1[2] + (int)RefPtr2[2]) >> 1) + ChangePtr[2] );
    ReconPtr[3] = clamp255((((int)RefPtr1[3] + (int)RefPtr2[3]) >> 1) + ChangePtr[3] );
    ReconPtr[4] = clamp255((((int)RefPtr1[4] + (int)RefPtr2[4]) >> 1) + ChangePtr[4] );
    ReconPtr[5] = clamp255((((int)RefPtr1[5] + (int)RefPtr2[5]) >> 1) + ChangePtr[5] );
    ReconPtr[6] = clamp255((((int)RefPtr1[6] + (int)RefPtr2[6]) >> 1) + ChangePtr[6] );
    ReconPtr[7] = clamp255((((int)RefPtr1[7] + (int)RefPtr2[7]) >> 1) + ChangePtr[7] );

    ChangePtr += 8;
    ReconPtr += LineStep;
    RefPtr1 += LineStep;
    RefPtr2 += LineStep;
  }
}

void dsp_recon_init (DspFunctions *funcs, ogg_uint32_t cpu_flags)
{
  funcs->copy8x8 = copy8x8__c;
  funcs->recon_intra8x8 = recon_intra8x8__c;
  funcs->recon_inter8x8 = recon_inter8x8__c;
  funcs->recon_inter8x8_half = recon_inter8x8_half__c;
#if defined(USE_ASM)
  if (cpu_flags & OC_CPU_X86_MMX) {
    dsp_mmx_recon_init(funcs);
  }
#endif
}
