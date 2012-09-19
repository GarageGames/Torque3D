/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2008                *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: dct_decode_mmx.c 15400 2008-10-15 12:10:58Z tterribe $

 ********************************************************************/

#include <stdlib.h>

#include "../codec_internal.h"

#if defined(USE_ASM)

static const __attribute__((aligned(8),used)) ogg_int64_t OC_V3=
 0x0003000300030003LL;
static const __attribute__((aligned(8),used)) ogg_int64_t OC_V4=
 0x0004000400040004LL;

static void loop_filter_v(unsigned char *_pix,int _ystride,
                          const ogg_int16_t *_ll){
  long esi;
  _pix-=_ystride*2;
  __asm__ __volatile__(
    /*mm0=0*/
    "pxor %%mm0,%%mm0\n\t"
    /*esi=_ystride*3*/
    "lea (%[ystride],%[ystride],2),%[s]\n\t"
    /*mm7=_pix[0...8]*/
    "movq (%[pix]),%%mm7\n\t"
    /*mm4=_pix[0...8+_ystride*3]*/
    "movq (%[pix],%[s]),%%mm4\n\t"
    /*mm6=_pix[0...8]*/
    "movq %%mm7,%%mm6\n\t"
    /*Expand unsigned _pix[0...3] to 16 bits.*/
    "punpcklbw %%mm0,%%mm6\n\t"
    "movq %%mm4,%%mm5\n\t"
    /*Expand unsigned _pix[4...8] to 16 bits.*/
    "punpckhbw %%mm0,%%mm7\n\t"
    /*Expand other arrays too.*/
    "punpcklbw %%mm0,%%mm4\n\t"
    "punpckhbw %%mm0,%%mm5\n\t"
    /*mm7:mm6=_p[0...8]-_p[0...8+_ystride*3]:*/
    "psubw %%mm4,%%mm6\n\t"
    "psubw %%mm5,%%mm7\n\t"
    /*mm5=mm4=_pix[0...8+_ystride]*/
    "movq (%[pix],%[ystride]),%%mm4\n\t"
    /*mm1=mm3=mm2=_pix[0..8]+_ystride*2]*/
    "movq (%[pix],%[ystride],2),%%mm2\n\t"
    "movq %%mm4,%%mm5\n\t"
    "movq %%mm2,%%mm3\n\t"
    "movq %%mm2,%%mm1\n\t"
    /*Expand these arrays.*/
    "punpckhbw %%mm0,%%mm5\n\t"
    "punpcklbw %%mm0,%%mm4\n\t"
    "punpckhbw %%mm0,%%mm3\n\t"
    "punpcklbw %%mm0,%%mm2\n\t"
    /*Preload...*/
    "movq %[OC_V3],%%mm0\n\t"
    /*mm3:mm2=_pix[0...8+_ystride*2]-_pix[0...8+_ystride]*/
    "psubw %%mm5,%%mm3\n\t"
    "psubw %%mm4,%%mm2\n\t"
    /*Scale by 3.*/
    "pmullw %%mm0,%%mm3\n\t"
    "pmullw %%mm0,%%mm2\n\t"
    /*Preload...*/
    "movq %[OC_V4],%%mm0\n\t"
    /*f=mm3:mm2==_pix[0...8]-_pix[0...8+_ystride*3]+
       3*(_pix[0...8+_ystride*2]-_pix[0...8+_ystride])*/
    "paddw %%mm7,%%mm3\n\t"
    "paddw %%mm6,%%mm2\n\t"
    /*Add 4.*/
    "paddw %%mm0,%%mm3\n\t"
    "paddw %%mm0,%%mm2\n\t"
    /*"Divide" by 8.*/
    "psraw $3,%%mm3\n\t"
    "psraw $3,%%mm2\n\t"
    /*Now compute lflim of mm3:mm2 cf. Section 7.10 of the sepc.*/
    /*Free up mm5.*/
    "packuswb %%mm5,%%mm4\n\t"
    /*mm0=L L L L*/
    "movq (%[ll]),%%mm0\n\t"
    /*if(R_i<-2L||R_i>2L)R_i=0:*/
    "movq %%mm2,%%mm5\n\t"
    "pxor %%mm6,%%mm6\n\t"
    "movq %%mm0,%%mm7\n\t"
    "psubw %%mm0,%%mm6\n\t"
    "psllw $1,%%mm7\n\t"
    "psllw $1,%%mm6\n\t"
    /*mm2==R_3 R_2 R_1 R_0*/
    /*mm5==R_3 R_2 R_1 R_0*/
    /*mm6==-2L -2L -2L -2L*/
    /*mm7==2L 2L 2L 2L*/
    "pcmpgtw %%mm2,%%mm7\n\t"
    "pcmpgtw %%mm6,%%mm5\n\t"
    "pand %%mm7,%%mm2\n\t"
    "movq %%mm0,%%mm7\n\t"
    "pand %%mm5,%%mm2\n\t"
    "psllw $1,%%mm7\n\t"
    "movq %%mm3,%%mm5\n\t"
    /*mm3==R_7 R_6 R_5 R_4*/
    /*mm5==R_7 R_6 R_5 R_4*/
    /*mm6==-2L -2L -2L -2L*/
    /*mm7==2L 2L 2L 2L*/
    "pcmpgtw %%mm3,%%mm7\n\t"
    "pcmpgtw %%mm6,%%mm5\n\t"
    "pand %%mm7,%%mm3\n\t"
    "movq %%mm0,%%mm7\n\t"
    "pand %%mm5,%%mm3\n\t"
    /*if(R_i<-L)R_i'=R_i+2L;
      if(R_i>L)R_i'=R_i-2L;
      if(R_i<-L||R_i>L)R_i=-R_i':*/
    "psraw $1,%%mm6\n\t"
    "movq %%mm2,%%mm5\n\t"
    "psllw $1,%%mm7\n\t"
    /*mm2==R_3 R_2 R_1 R_0*/
    /*mm5==R_3 R_2 R_1 R_0*/
    /*mm6==-L -L -L -L*/
    /*mm0==L L L L*/
    /*mm5=R_i>L?FF:00*/
    "pcmpgtw %%mm0,%%mm5\n\t"
    /*mm6=-L>R_i?FF:00*/
    "pcmpgtw %%mm2,%%mm6\n\t"
    /*mm7=R_i>L?2L:0*/
    "pand %%mm5,%%mm7\n\t"
    /*mm2=R_i>L?R_i-2L:R_i*/
    "psubw %%mm7,%%mm2\n\t"
    "movq %%mm0,%%mm7\n\t"
    /*mm5=-L>R_i||R_i>L*/
    "por %%mm6,%%mm5\n\t"
    "psllw $1,%%mm7\n\t"
    /*mm7=-L>R_i?2L:0*/
    "pand %%mm6,%%mm7\n\t"
    "pxor %%mm6,%%mm6\n\t"
    /*mm2=-L>R_i?R_i+2L:R_i*/
    "paddw %%mm7,%%mm2\n\t"
    "psubw %%mm0,%%mm6\n\t"
    /*mm5=-L>R_i||R_i>L?-R_i':0*/
    "pand %%mm2,%%mm5\n\t"
    "movq %%mm0,%%mm7\n\t"
    /*mm2=-L>R_i||R_i>L?0:R_i*/
    "psubw %%mm5,%%mm2\n\t"
    "psllw $1,%%mm7\n\t"
    /*mm2=-L>R_i||R_i>L?-R_i':R_i*/
    "psubw %%mm5,%%mm2\n\t"
    "movq %%mm3,%%mm5\n\t"
    /*mm3==R_7 R_6 R_5 R_4*/
    /*mm5==R_7 R_6 R_5 R_4*/
    /*mm6==-L -L -L -L*/
    /*mm0==L L L L*/
    /*mm6=-L>R_i?FF:00*/
    "pcmpgtw %%mm3,%%mm6\n\t"
    /*mm5=R_i>L?FF:00*/
    "pcmpgtw %%mm0,%%mm5\n\t"
    /*mm7=R_i>L?2L:0*/
    "pand %%mm5,%%mm7\n\t"
    /*mm2=R_i>L?R_i-2L:R_i*/
    "psubw %%mm7,%%mm3\n\t"
    "psllw $1,%%mm0\n\t"
    /*mm5=-L>R_i||R_i>L*/
    "por %%mm6,%%mm5\n\t"
    /*mm0=-L>R_i?2L:0*/
    "pand %%mm6,%%mm0\n\t"
    /*mm3=-L>R_i?R_i+2L:R_i*/
    "paddw %%mm0,%%mm3\n\t"
    /*mm5=-L>R_i||R_i>L?-R_i':0*/
    "pand %%mm3,%%mm5\n\t"
    /*mm2=-L>R_i||R_i>L?0:R_i*/
    "psubw %%mm5,%%mm3\n\t"
    /*mm2=-L>R_i||R_i>L?-R_i':R_i*/
    "psubw %%mm5,%%mm3\n\t"
    /*Unfortunately, there's no unsigned byte+signed byte with unsigned
       saturation op code, so we have to promote things back 16 bits.*/
    "pxor %%mm0,%%mm0\n\t"
    "movq %%mm4,%%mm5\n\t"
    "punpcklbw %%mm0,%%mm4\n\t"
    "punpckhbw %%mm0,%%mm5\n\t"
    "movq %%mm1,%%mm6\n\t"
    "punpcklbw %%mm0,%%mm1\n\t"
    "punpckhbw %%mm0,%%mm6\n\t"
    /*_pix[0...8+_ystride]+=R_i*/
    "paddw %%mm2,%%mm4\n\t"
    "paddw %%mm3,%%mm5\n\t"
    /*_pix[0...8+_ystride*2]-=R_i*/
    "psubw %%mm2,%%mm1\n\t"
    "psubw %%mm3,%%mm6\n\t"
    "packuswb %%mm5,%%mm4\n\t"
    "packuswb %%mm6,%%mm1\n\t"
    /*Write it back out.*/
    "movq %%mm4,(%[pix],%[ystride])\n\t"
    "movq %%mm1,(%[pix],%[ystride],2)\n\t"
    :[s]"=&S"(esi)
    :[pix]"r"(_pix),[ystride]"r"((long)_ystride),[ll]"r"(_ll),
     [OC_V3]"m"(OC_V3),[OC_V4]"m"(OC_V4)
    :"memory"
  );
}

/*This code implements the bulk of loop_filter_h().
  Data are striped p0 p1 p2 p3 ... p0 p1 p2 p3 ..., so in order to load all
   four p0's to one register we must transpose the values in four mmx regs.
  When half is done we repeat this for the rest.*/
static void loop_filter_h4(unsigned char *_pix,long _ystride,
                           const ogg_int16_t *_ll){
  long esi;
  long edi;
  __asm__ __volatile__(
    /*x x x x 3 2 1 0*/
    "movd (%[pix]),%%mm0\n\t"
    /*esi=_ystride*3*/
    "lea (%[ystride],%[ystride],2),%[s]\n\t"
    /*x x x x 7 6 5 4*/
    "movd (%[pix],%[ystride]),%%mm1\n\t"
    /*x x x x B A 9 8*/
    "movd (%[pix],%[ystride],2),%%mm2\n\t"
    /*x x x x F E D C*/
    "movd (%[pix],%[s]),%%mm3\n\t"
    /*mm0=7 3 6 2 5 1 4 0*/
    "punpcklbw %%mm1,%%mm0\n\t"
    /*mm2=F B E A D 9 C 8*/
    "punpcklbw %%mm3,%%mm2\n\t"
    /*mm1=7 3 6 2 5 1 4 0*/
    "movq %%mm0,%%mm1\n\t"
    /*mm0=F B 7 3 E A 6 2*/
    "punpckhwd %%mm2,%%mm0\n\t"
    /*mm1=D 9 5 1 C 8 4 0*/
    "punpcklwd %%mm2,%%mm1\n\t"
    "pxor %%mm7,%%mm7\n\t"
    /*mm5=D 9 5 1 C 8 4 0*/
    "movq %%mm1,%%mm5\n\t"
    /*mm1=x C x 8 x 4 x 0==pix[0]*/
    "punpcklbw %%mm7,%%mm1\n\t"
    /*mm5=x D x 9 x 5 x 1==pix[1]*/
    "punpckhbw %%mm7,%%mm5\n\t"
    /*mm3=F B 7 3 E A 6 2*/
    "movq %%mm0,%%mm3\n\t"
    /*mm0=x E x A x 6 x 2==pix[2]*/
    "punpcklbw %%mm7,%%mm0\n\t"
    /*mm3=x F x B x 7 x 3==pix[3]*/
    "punpckhbw %%mm7,%%mm3\n\t"
    /*mm1=mm1-mm3==pix[0]-pix[3]*/
    "psubw %%mm3,%%mm1\n\t"
    /*Save a copy of pix[2] for later.*/
    "movq %%mm0,%%mm4\n\t"
    /*mm0=mm0-mm5==pix[2]-pix[1]*/
    "psubw %%mm5,%%mm0\n\t"
    /*Scale by 3.*/
    "pmullw %[OC_V3],%%mm0\n\t"
    /*f=mm1==_pix[0]-_pix[3]+ 3*(_pix[2]-_pix[1])*/
    "paddw %%mm1,%%mm0\n\t"
    /*Add 4.*/
    "paddw %[OC_V4],%%mm0\n\t"
    /*"Divide" by 8, producing the residuals R_i.*/
    "psraw $3,%%mm0\n\t"
    /*Now compute lflim of mm0 cf. Section 7.10 of the sepc.*/
    /*mm6=L L L L*/
    "movq (%[ll]),%%mm6\n\t"
    /*if(R_i<-2L||R_i>2L)R_i=0:*/
    "movq %%mm0,%%mm1\n\t"
    "pxor %%mm2,%%mm2\n\t"
    "movq %%mm6,%%mm3\n\t"
    "psubw %%mm6,%%mm2\n\t"
    "psllw $1,%%mm3\n\t"
    "psllw $1,%%mm2\n\t"
    /*mm0==R_3 R_2 R_1 R_0*/
    /*mm1==R_3 R_2 R_1 R_0*/
    /*mm2==-2L -2L -2L -2L*/
    /*mm3==2L 2L 2L 2L*/
    "pcmpgtw %%mm0,%%mm3\n\t"
    "pcmpgtw %%mm2,%%mm1\n\t"
    "pand %%mm3,%%mm0\n\t"
    "pand %%mm1,%%mm0\n\t"
    /*if(R_i<-L)R_i'=R_i+2L;
      if(R_i>L)R_i'=R_i-2L;
      if(R_i<-L||R_i>L)R_i=-R_i':*/
    "psraw $1,%%mm2\n\t"
    "movq %%mm0,%%mm1\n\t"
    "movq %%mm6,%%mm3\n\t"
    /*mm0==R_3 R_2 R_1 R_0*/
    /*mm1==R_3 R_2 R_1 R_0*/
    /*mm2==-L -L -L -L*/
    /*mm6==L L L L*/
    /*mm2=-L>R_i?FF:00*/
    "pcmpgtw %%mm0,%%mm2\n\t"
    /*mm1=R_i>L?FF:00*/
    "pcmpgtw %%mm6,%%mm1\n\t"
    /*mm3=2L 2L 2L 2L*/
    "psllw $1,%%mm3\n\t"
    /*mm6=2L 2L 2L 2L*/
    "psllw $1,%%mm6\n\t"
    /*mm3=R_i>L?2L:0*/
    "pand %%mm1,%%mm3\n\t"
    /*mm6=-L>R_i?2L:0*/
    "pand %%mm2,%%mm6\n\t"
    /*mm0=R_i>L?R_i-2L:R_i*/
    "psubw %%mm3,%%mm0\n\t"
    /*mm1=-L>R_i||R_i>L*/
    "por %%mm2,%%mm1\n\t"
    /*mm0=-L>R_i?R_i+2L:R_i*/
    "paddw %%mm6,%%mm0\n\t"
    /*mm1=-L>R_i||R_i>L?R_i':0*/
    "pand %%mm0,%%mm1\n\t"
    /*mm0=-L>R_i||R_i>L?0:R_i*/
    "psubw %%mm1,%%mm0\n\t"
    /*mm0=-L>R_i||R_i>L?-R_i':R_i*/
    "psubw %%mm1,%%mm0\n\t"
    /*_pix[1]+=R_i;*/
    "paddw %%mm0,%%mm5\n\t"
    /*_pix[2]-=R_i;*/
    "psubw %%mm0,%%mm4\n\t"
    /*mm5=x x x x D 9 5 1*/
    "packuswb %%mm7,%%mm5\n\t"
    /*mm4=x x x x E A 6 2*/
    "packuswb %%mm7,%%mm4\n\t"
    /*mm5=E D A 9 6 5 2 1*/
    "punpcklbw %%mm4,%%mm5\n\t"
    /*edi=6 5 2 1*/
    "movd %%mm5,%%edi\n\t"
    "movw %%di,1(%[pix])\n\t"
    /*Why is there such a big stall here?*/
    "psrlq $32,%%mm5\n\t"
    "shrl $16,%%edi\n\t"
    "movw %%di,1(%[pix],%[ystride])\n\t"
    /*edi=E D A 9*/
    "movd %%mm5,%%edi\n\t"
    "movw %%di,1(%[pix],%[ystride],2)\n\t"
    "shrl $16,%%edi\n\t"
    "movw %%di,1(%[pix],%[s])\n\t"
    :[s]"=&S"(esi),[d]"=&D"(edi),
     [pix]"+r"(_pix),[ystride]"+r"(_ystride),[ll]"+r"(_ll)
    :[OC_V3]"m"(OC_V3),[OC_V4]"m"(OC_V4)
    :"memory"
  );
}

static void loop_filter_h(unsigned char *_pix,int _ystride,
                          const ogg_int16_t *_ll){
  _pix-=2;
  loop_filter_h4(_pix,_ystride,_ll);
  loop_filter_h4(_pix+(_ystride<<2),_ystride,_ll);
}

static void loop_filter_mmx(PB_INSTANCE *pbi, int FLimit){
  int j;
  ogg_int16_t __attribute__((aligned(8)))  ll[4];
  unsigned char *cp = pbi->display_fragments;
  ogg_uint32_t *bp = pbi->recon_pixel_index_table;

  if ( FLimit == 0 ) return;
  ll[0]=ll[1]=ll[2]=ll[3]=FLimit;

  for ( j = 0; j < 3 ; j++){
    ogg_uint32_t *bp_begin = bp;
    ogg_uint32_t *bp_end;
    int stride;
    int h;

    switch(j) {
    case 0: /* y */
      bp_end = bp + pbi->YPlaneFragments;
      h = pbi->HFragments;
      stride = pbi->YStride;
      break;
    default: /* u,v, 4:20 specific */
      bp_end = bp + pbi->UVPlaneFragments;
      h = pbi->HFragments >> 1;
      stride = pbi->UVStride;
      break;
    }

    while(bp<bp_end){
      ogg_uint32_t *bp_left = bp;
      ogg_uint32_t *bp_right = bp + h;
      while(bp<bp_right){
        if(cp[0]){
          if(bp>bp_left)
            loop_filter_h(&pbi->LastFrameRecon[bp[0]],stride,ll);
          if(bp_left>bp_begin)
            loop_filter_v(&pbi->LastFrameRecon[bp[0]],stride,ll);
          if(bp+1<bp_right && !cp[1])
            loop_filter_h(&pbi->LastFrameRecon[bp[0]]+8,stride,ll);
          if(bp+h<bp_end && !cp[h])
            loop_filter_v(&pbi->LastFrameRecon[bp[h]],stride,ll);
        }
        bp++;
        cp++;
      }
    }
  }

  __asm__ __volatile__("emms\n\t");
}

/* install our implementation in the function table */
void dsp_mmx_dct_decode_init(DspFunctions *funcs)
{
  funcs->LoopFilter = loop_filter_mmx;
}

#endif /* USE_ASM */
