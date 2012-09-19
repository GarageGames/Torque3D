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
  last mod: $Id: reconstruct.c,v 1.6 2003/12/03 08:59:41 arc Exp $

 ********************************************************************/

#include "../codec_internal.h"


static const unsigned __int64 V128 = 0x8080808080808080;

static void copy8x8__mmx (unsigned char *src,
                    unsigned char *dest,
                    unsigned int stride)
{

    //Is this even the fastest way to do this?
    __asm {
        align 16

        mov         eax, src
        mov         ebx, dest
        mov         ecx, stride

        lea         edi, [ecx + ecx * 2]
        movq        mm0, [eax]
        movq        mm1, [eax + ecx]
        movq        mm2, [eax + ecx * 2]
        movq        mm3, [eax + edi]
        lea         eax, [eax + ecx * 4]
        movq        [ebx], mm0
        movq        [ebx + ecx], mm1
        movq        [ebx + ecx * 2], mm2
        movq        [ebx + edi], mm3
        lea         ebx, [ebx + ecx * 4]
        movq        mm0, [eax]
        movq        mm1, [eax + ecx]
        movq        mm2, [eax + ecx * 2]
        movq        mm3, [eax + edi]
        movq        [ebx], mm0
        movq        [ebx + ecx], mm1
        movq        [ebx + ecx * 2], mm2
        movq        [ebx + edi], mm3

    };

}

static void recon_intra8x8__mmx (unsigned char *ReconPtr, ogg_int16_t *ChangePtr,
              ogg_uint32_t LineStep)
{

    __asm {
        align 16

        mov         eax, ReconPtr
        mov         ebx, ChangePtr
        mov         ecx, LineStep

        movq        mm0, V128

        lea         edi, [128 + ebx]
    loop_start:
        movq        mm2, [ebx]

        packsswb    mm2, [8 + ebx]
        por         mm0, mm0
        pxor        mm2, mm0
        lea         ebx, [16 + ebx]
        cmp         ebx, edi

        movq        [eax], mm2



        lea         eax, [eax + ecx]
        jc          loop_start


    };

}





static void recon_inter8x8__mmx (unsigned char *ReconPtr, unsigned char *RefPtr,
              ogg_int16_t *ChangePtr, ogg_uint32_t LineStep)
{

    __asm {

        align 16

        mov         eax, ReconPtr
        mov         ebx, ChangePtr
        mov         ecx, LineStep
        mov         edx, RefPtr

        pxor        mm0, mm0
        lea         edi, [128 + ebx]

    loop_start:
        movq        mm2, [edx]

        movq        mm4, [ebx]
        movq        mm3, mm2
        movq        mm5, [8 + ebx]
        punpcklbw   mm2, mm0
        paddsw      mm2, mm4
        punpckhbw   mm3, mm0
        paddsw      mm3, mm5
        add         edx, ecx
        packuswb    mm2, mm3
        lea         ebx, [16 + ebx]
        cmp         ebx, edi

        movq        [eax], mm2

        lea         eax, [eax + ecx]
        jc          loop_start

    };
}




static void recon_inter8x8_half__mmx (unsigned char *ReconPtr, unsigned char *RefPtr1,
                   unsigned char *RefPtr2, ogg_int16_t *ChangePtr,
               ogg_uint32_t LineStep)
{
    __asm {
        align 16

        mov     eax, ReconPtr
        mov     ebx, ChangePtr
        mov     ecx, RefPtr1
        mov     edx, RefPtr2

        pxor        mm0, mm0
        lea     edi, [128 + ebx]

    loop_start:
        movq        mm2, [ecx]
        movq        mm4, [edx]

        movq        mm3, mm2
        punpcklbw       mm2, mm0
        movq        mm5, mm4
        movq        mm6, [ebx]
        punpckhbw       mm3, mm0
        movq        mm7, [8 + ebx]
        punpcklbw       mm4, mm0
        punpckhbw       mm5, mm0
        paddw       mm2, mm4
        paddw       mm3, mm5
        psrlw       mm2, 1
        psrlw       mm3, 1
        paddw       mm2, mm6
        paddw       mm3, mm7
        lea     ebx, [16 + ebx]
        packuswb        mm2, mm3
        add     ecx, LineStep
        add     edx, LineStep
        movq        [eax], mm2
        add     eax, LineStep
        cmp     ebx, edi
        jc      loop_start

    };

}




void dsp_mmx_recon_init(DspFunctions *funcs)
{
  funcs->copy8x8 = copy8x8__mmx;
  funcs->recon_intra8x8 = recon_intra8x8__mmx;
  funcs->recon_inter8x8 = recon_inter8x8__mmx;
  funcs->recon_inter8x8_half = recon_inter8x8_half__mmx;
}

