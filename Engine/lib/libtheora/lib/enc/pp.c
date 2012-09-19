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
  last mod: $Id: pp.c 15057 2008-06-22 21:07:32Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "codec_internal.h"
#include "pp.h"
#include "dsp.h"

#define MAX(a, b) ((a>b)?a:b)
#define MIN(a, b) ((a<b)?a:b)
#define PP_QUALITY_THRESH   49

static const ogg_int32_t SharpenModifier[ Q_TABLE_SIZE ] =
{  -12, -11, -10, -10,  -9,  -9,  -9,  -9,
   -6,  -6,  -6,  -6,  -6,  -6,  -6,  -6,
   -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,
   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0
};

static const ogg_uint32_t DcQuantScaleV1[ Q_TABLE_SIZE ] = {
  22, 20, 19, 18, 17, 17, 16, 16,
  15, 15, 14, 14, 13, 13, 12, 12,
  11, 11, 10, 10, 9,  9,  9,  8,
  8,  8,  7,  7,  7,  6,  6,  6,
  6,  5,  5,  5,  5,  4,  4,  4,
  4,  4,  3,  3,  3,  3,  3,  3,
  3,  2,  2,  2,  2,  2,  2,  2,
  2,  1,  1,  1,  1,  1,  1,  1
};

static const ogg_uint32_t * const DeringModifierV1=DcQuantScaleV1;

static void PClearFrameInfo(PP_INSTANCE * ppi){
  int i;

  if(ppi->ScanPixelIndexTable) _ogg_free(ppi->ScanPixelIndexTable);
  ppi->ScanPixelIndexTable=0;

  if(ppi->ScanDisplayFragments) _ogg_free(ppi->ScanDisplayFragments);
  ppi->ScanDisplayFragments=0;

  for(i = 0 ; i < MAX_PREV_FRAMES ; i ++)
    if(ppi->PrevFragments[i]){
      _ogg_free(ppi->PrevFragments[i]);
      ppi->PrevFragments[i]=0;
    }

  if(ppi->FragScores) _ogg_free(ppi->FragScores);
  ppi->FragScores=0;

  if(ppi->SameGreyDirPixels) _ogg_free(ppi->SameGreyDirPixels);
  ppi->SameGreyDirPixels=0;

  if(ppi->FragDiffPixels) _ogg_free(ppi->FragDiffPixels);
  ppi->FragDiffPixels=0;

  if(ppi->BarBlockMap) _ogg_free(ppi->BarBlockMap);
  ppi->BarBlockMap=0;

  if(ppi->TmpCodedMap) _ogg_free(ppi->TmpCodedMap);
  ppi->TmpCodedMap=0;

  if(ppi->RowChangedPixels) _ogg_free(ppi->RowChangedPixels);
  ppi->RowChangedPixels=0;

  if(ppi->PixelScores) _ogg_free(ppi->PixelScores);
  ppi->PixelScores=0;

  if(ppi->PixelChangedMap) _ogg_free(ppi->PixelChangedMap);
  ppi->PixelChangedMap=0;

  if(ppi->ChLocals) _ogg_free(ppi->ChLocals);
  ppi->ChLocals=0;

  if(ppi->yuv_differences) _ogg_free(ppi->yuv_differences);
  ppi->yuv_differences=0;

}

void PInitFrameInfo(PP_INSTANCE * ppi){
  int i;
  PClearFrameInfo(ppi);

  ppi->ScanPixelIndexTable =
    _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->ScanPixelIndexTable));

  ppi->ScanDisplayFragments =
    _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->ScanDisplayFragments));

  for(i = 0 ; i < MAX_PREV_FRAMES ; i ++)
    ppi->PrevFragments[i] =
      _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->PrevFragments));

  ppi->FragScores =
    _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->FragScores));

  ppi->SameGreyDirPixels =
    _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->SameGreyDirPixels));

  ppi->FragDiffPixels =
    _ogg_malloc(ppi->ScanFrameFragments*sizeof(*ppi->FragScores));

  ppi->BarBlockMap=
    _ogg_malloc(3 * ppi->ScanHFragments*sizeof(*ppi->BarBlockMap));

  ppi->TmpCodedMap =
    _ogg_malloc(ppi->ScanHFragments*sizeof(*ppi->TmpCodedMap));

  ppi->RowChangedPixels =
    _ogg_malloc(3 * ppi->ScanConfig.VideoFrameHeight*
                sizeof(*ppi->RowChangedPixels));

  ppi->PixelScores =
    _ogg_malloc(ppi->ScanConfig.VideoFrameWidth*
                sizeof(*ppi->PixelScores) * PSCORE_CB_ROWS);

  ppi->PixelChangedMap =
    _ogg_malloc(ppi->ScanConfig.VideoFrameWidth*
                sizeof(*ppi->PixelChangedMap) * PMAP_CB_ROWS);

  ppi->ChLocals =
    _ogg_malloc(ppi->ScanConfig.VideoFrameWidth*
                sizeof(*ppi->ChLocals) * CHLOCALS_CB_ROWS);

  ppi->yuv_differences =
    _ogg_malloc(ppi->ScanConfig.VideoFrameWidth*
                sizeof(*ppi->yuv_differences) * YDIFF_CB_ROWS);
}

void ClearPPInstance(PP_INSTANCE *ppi){
  PClearFrameInfo(ppi);
}


void InitPPInstance(PP_INSTANCE *ppi, DspFunctions *funcs){

  memset(ppi,0,sizeof(*ppi));

  memcpy(&ppi->dsp, funcs, sizeof(DspFunctions));

  /* Initializations */
  ppi->PrevFrameLimit = 3; /* Must not exceed MAX_PREV_FRAMES (Note
                              that this number includes the current
                              frame so "1 = no effect") */

  /* Scan control variables. */
  ppi->HFragPixels = 8;
  ppi->VFragPixels = 8;

  ppi->SRFGreyThresh = 4;
  ppi->SRFColThresh = 5;
  ppi->NoiseSupLevel = 3;
  ppi->SgcLevelThresh = 3;
  ppi->SuvcLevelThresh = 4;

  /* Variables controlling S.A.D. breakouts. */
  ppi->GrpLowSadThresh = 10;
  ppi->GrpHighSadThresh = 64;
  ppi->PrimaryBlockThreshold = 5;
  ppi->SgcThresh = 16;  /* (Default values for 8x8 blocks). */

  ppi->UVBlockThreshCorrection = 1.25;
  ppi->UVSgcCorrection = 1.5;

  ppi->MaxLineSearchLen = MAX_SEARCH_LINE_LEN;
}

static void DeringBlockStrong(unsigned char *SrcPtr,
                              unsigned char *DstPtr,
                              ogg_int32_t Pitch,
                              ogg_uint32_t FragQIndex,
                              const ogg_uint32_t *QuantScale){

  ogg_int16_t UDMod[72];
  ogg_int16_t LRMod[72];
  unsigned int j,k,l;
  const unsigned char * Src;
  unsigned int QValue = QuantScale[FragQIndex];

  unsigned char p;
  unsigned char pl;
  unsigned char pr;
  unsigned char pu;
  unsigned char pd;

  int  al;
  int  ar;
  int  au;
  int  ad;

  int  atot;
  int  B;
  int  newVal;

  const unsigned char *curRow = SrcPtr - 1; /* avoid negative array indexes */
  unsigned char *dstRow = DstPtr;
  const unsigned char *lastRow = SrcPtr-Pitch;
  const unsigned char *nextRow = SrcPtr+Pitch;

  unsigned int rowOffset = 0;
  unsigned int round = (1<<6);

  int High;
  int Low;
  int TmpMod;

  int Sharpen = SharpenModifier[FragQIndex];
  High = 3 * QValue;
  if(High>32)High=32;
  Low = 0;


  /* Initialize the Mod Data */
  Src = SrcPtr-Pitch;
  for(k=0;k<9;k++){
    for(j=0;j<8;j++){

      TmpMod = 32 + QValue - (abs(Src[j+Pitch]-Src[j]));

      if(TmpMod< -64)
        TmpMod = Sharpen;

      else if(TmpMod<Low)
        TmpMod = Low;

      else if(TmpMod>High)
        TmpMod = High;

      UDMod[k*8+j] = (ogg_int16_t)TmpMod;
    }
    Src +=Pitch;
  }

  Src = SrcPtr-1;

  for(k=0;k<8;k++){
    for(j=0;j<9;j++){
      TmpMod = 32 + QValue - (abs(Src[j+1]-Src[j]));

      if(TmpMod< -64 )
        TmpMod = Sharpen;

      else if(TmpMod<0)
        TmpMod = Low;

      else if(TmpMod>High)
        TmpMod = High;

      LRMod[k*9+j] = (ogg_int16_t)TmpMod;
    }
    Src+=Pitch;
  }

  for(k=0;k<8;k++){
    /* In the case that this function called with same buffer for
     source and destination, To keep the c and the mmx version to have
     consistant results, intermediate buffer is used to store the
     eight pixel value before writing them to destination
     (i.e. Overwriting souce for the speical case) */
    for(l=0;l<8;l++){

      atot = 128;
      B = round;
      p = curRow[ rowOffset +l +1];

      pl = curRow[ rowOffset +l];
      al = LRMod[k*9+l];
      atot -= al;
      B += al * pl;

      pu = lastRow[ rowOffset +l];
      au = UDMod[k*8+l];
      atot -= au;
      B += au * pu;

      pd = nextRow[ rowOffset +l];
      ad = UDMod[(k+1)*8+l];
      atot -= ad;
      B += ad * pd;

      pr = curRow[ rowOffset +l+2];
      ar = LRMod[k*9+l+1];
      atot -= ar;
      B += ar * pr;

      newVal = ( atot * p + B) >> 7;

      dstRow[ rowOffset +l]= clamp255( newVal );
    }
    rowOffset += Pitch;
  }
}

static void DeringBlockWeak(unsigned char *SrcPtr,
                            unsigned char *DstPtr,
                            ogg_int32_t Pitch,
                            ogg_uint32_t FragQIndex,
                            const ogg_uint32_t *QuantScale){

  ogg_int16_t UDMod[72];
  ogg_int16_t LRMod[72];
  unsigned int j,k;
  const unsigned char * Src;
  unsigned int QValue = QuantScale[FragQIndex];

  unsigned char p;
  unsigned char pl;
  unsigned char pr;
  unsigned char pu;
  unsigned char pd;

  int  al;
  int  ar;
  int  au;
  int  ad;

  int  atot;
  int  B;
  int  newVal;

  const unsigned char *curRow = SrcPtr-1;
  unsigned char *dstRow = DstPtr;
  const unsigned char *lastRow = SrcPtr-Pitch;
  const unsigned char *nextRow = SrcPtr+Pitch;

  unsigned int rowOffset = 0;
  unsigned int round = (1<<6);

  int High;
  int Low;
  int TmpMod;
  int Sharpen = SharpenModifier[FragQIndex];

  High = 3 * QValue;
  if(High>24)
    High=24;
  Low = 0 ;

  /* Initialize the Mod Data */
  Src=SrcPtr-Pitch;
  for(k=0;k<9;k++) {
    for(j=0;j<8;j++) {

      TmpMod = 32 + QValue - 2*(abs(Src[j+Pitch]-Src[j]));

      if(TmpMod< -64)
        TmpMod = Sharpen;

      else if(TmpMod<Low)
        TmpMod = Low;

            else if(TmpMod>High)
              TmpMod = High;

      UDMod[k*8+j] = (ogg_int16_t)TmpMod;
    }
    Src +=Pitch;
  }

  Src = SrcPtr-1;

  for(k=0;k<8;k++){
    for(j=0;j<9;j++){
      TmpMod = 32 + QValue - 2*(abs(Src[j+1]-Src[j]));

      if(TmpMod< -64 )
        TmpMod = Sharpen;

      else if(TmpMod<Low)
        TmpMod = Low;

      else if(TmpMod>High)
        TmpMod = High;

      LRMod[k*9+j] = (ogg_int16_t)TmpMod;
    }
    Src+=Pitch;
  }

  for(k=0;k<8;k++) {
    for(j=0;j<8;j++){
      atot = 128;
      B = round;
      p = curRow[ rowOffset +j+1];

      pl = curRow[ rowOffset +j];
      al = LRMod[k*9+j];
      atot -= al;
      B += al * pl;

      pu = lastRow[ rowOffset +j];
      au = UDMod[k*8+j];
      atot -= au;
      B += au * pu;

      pd = nextRow[ rowOffset +j];
      ad = UDMod[(k+1)*8+j];
      atot -= ad;
      B += ad * pd;

      pr = curRow[ rowOffset +j+2];
      ar = LRMod[k*9+j+1];
      atot -= ar;
      B += ar * pr;

      newVal = ( atot * p + B) >> 7;

      dstRow[ rowOffset +j] = clamp255( newVal );
    }

    rowOffset += Pitch;
  }
}

static void DeringFrame(PB_INSTANCE *pbi,
                        unsigned char *Src, unsigned char *Dst){
  ogg_uint32_t  col,row;
  unsigned char  *SrcPtr;
  unsigned char  *DestPtr;
  ogg_uint32_t BlocksAcross,BlocksDown;
  const ogg_uint32_t *QuantScale;
  ogg_uint32_t Block;
  ogg_uint32_t LineLength;

  ogg_int32_t Thresh1,Thresh2,Thresh3,Thresh4;

  Thresh1 = 384;
  Thresh2 = 4 * Thresh1;
  Thresh3 = 5 * Thresh2/4;
  Thresh4 = 5 * Thresh2/2;

  QuantScale = DeringModifierV1;

  BlocksAcross = pbi->HFragments;
  BlocksDown = pbi->VFragments;

  SrcPtr = Src + pbi->ReconYDataOffset;
  DestPtr = Dst + pbi->ReconYDataOffset;
  LineLength = pbi->YStride;

  Block = 0;

  for ( row = 0 ; row < BlocksDown; row ++){
    for (col = 0; col < BlocksAcross; col ++){
      ogg_uint32_t Quality = pbi->FragQIndex[Block];
      ogg_int32_t Variance = pbi->FragmentVariances[Block];

      if( pbi->PostProcessingLevel >5 && Variance > Thresh3 ){
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);

        if( (col > 0 &&
             pbi->FragmentVariances[Block-1] > Thresh4 ) ||
            (col + 1 < BlocksAcross &&
             pbi->FragmentVariances[Block+1] > Thresh4 ) ||
            (row + 1 < BlocksDown &&
             pbi->FragmentVariances[Block+BlocksAcross] > Thresh4) ||
            (row > 0 &&
             pbi->FragmentVariances[Block-BlocksAcross] > Thresh4) ){

          DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                            LineLength,Quality,QuantScale);
          DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                            LineLength,Quality,QuantScale);
        }
      } else if(Variance > Thresh2 ) {

        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
      } else if(Variance > Thresh1 ) {

        DeringBlockWeak(SrcPtr + 8 * col, DestPtr + 8 * col,
                        LineLength,Quality,QuantScale);

      } else {

        dsp_copy8x8(pbi->dsp, SrcPtr + 8 * col, DestPtr + 8 * col, LineLength);

      }

      ++Block;

    }
    SrcPtr += 8 * LineLength;
    DestPtr += 8 * LineLength;
  }

  /* Then U */

  BlocksAcross /= 2;
  BlocksDown /= 2;
  LineLength /= 2;

  SrcPtr = Src + pbi->ReconUDataOffset;
  DestPtr = Dst + pbi->ReconUDataOffset;
  for ( row = 0 ; row < BlocksDown; row ++) {
    for (col = 0; col < BlocksAcross; col ++) {
      ogg_uint32_t Quality = pbi->FragQIndex[Block];
      ogg_int32_t Variance = pbi->FragmentVariances[Block];

      if( pbi->PostProcessingLevel >5 && Variance > Thresh4 ) {
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);

      }else if(Variance > Thresh2 ){
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
      }else if(Variance > Thresh1 ){
        DeringBlockWeak(SrcPtr + 8 * col, DestPtr + 8 * col,
                        LineLength,Quality,QuantScale);
      }else{
        dsp_copy8x8(pbi->dsp, SrcPtr + 8 * col, DestPtr + 8 * col, LineLength);
      }

      ++Block;

    }
    SrcPtr += 8 * LineLength;
    DestPtr += 8 * LineLength;
  }

  /* Then V */
  SrcPtr = Src + pbi->ReconVDataOffset;
  DestPtr = Dst + pbi->ReconVDataOffset;

  for ( row = 0 ; row < BlocksDown; row ++){
    for (col = 0; col < BlocksAcross; col ++){

      ogg_uint32_t Quality = pbi->FragQIndex[Block];
      ogg_int32_t Variance = pbi->FragmentVariances[Block];


      if( pbi->PostProcessingLevel >5 && Variance > Thresh4 ) {
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);

      }else if(Variance > Thresh2 ){
        DeringBlockStrong(SrcPtr + 8 * col, DestPtr + 8 * col,
                          LineLength,Quality,QuantScale);
      }else if(Variance > Thresh1 ){
        DeringBlockWeak(SrcPtr + 8 * col, DestPtr + 8 * col,
                        LineLength,Quality,QuantScale);
      }else{
        dsp_copy8x8(pbi->dsp, SrcPtr + 8 * col, DestPtr + 8 * col, LineLength);
      }

      ++Block;

    }
    SrcPtr += 8 * LineLength;
    DestPtr += 8 * LineLength;

  }

}

void UpdateFragQIndex(PB_INSTANCE *pbi){

  ogg_uint32_t  ThisFrameQIndex;
  ogg_uint32_t  i;

  /* Check this frame quality  index */
  ThisFrameQIndex = pbi->FrameQIndex;


  /* It is not a key frame, so only reset those are coded */
  for( i = 0; i < pbi->UnitFragments; i++  )
    if( pbi->display_fragments[i])
      pbi->FragQIndex[i] = ThisFrameQIndex;

}

static void DeblockLoopFilteredBand(PB_INSTANCE *pbi,
                             unsigned char *SrcPtr,
                             unsigned char *DesPtr,
                             ogg_uint32_t PlaneLineStep,
                             ogg_uint32_t FragsAcross,
                             ogg_uint32_t StartFrag,
                             const ogg_uint32_t *QuantScale){
  ogg_uint32_t j,k;
  ogg_uint32_t CurrentFrag=StartFrag;
  ogg_int32_t QStep;
  ogg_int32_t FLimit;
  unsigned char *Src, *Des;
  ogg_int32_t  x[10];
  ogg_int32_t  Sum1, Sum2;

  while(CurrentFrag < StartFrag + FragsAcross){

    Src=SrcPtr+8*(CurrentFrag-StartFrag)-PlaneLineStep*5;
    Des=DesPtr+8*(CurrentFrag-StartFrag)-PlaneLineStep*4;

    QStep = QuantScale[pbi->FragQIndex[CurrentFrag+FragsAcross]];
    FLimit = ( QStep * 3 ) >> 2;

    for( j=0; j<8 ; j++){
      x[0] = Src[0];
      x[1] = Src[PlaneLineStep];
      x[2] = Src[PlaneLineStep*2];
      x[3] = Src[PlaneLineStep*3];
      x[4] = Src[PlaneLineStep*4];
      x[5] = Src[PlaneLineStep*5];
      x[6] = Src[PlaneLineStep*6];
      x[7] = Src[PlaneLineStep*7];
      x[8] = Src[PlaneLineStep*8];
      x[9] = Src[PlaneLineStep*9];

      Sum1=Sum2=0;

      for(k=1;k<=4;k++){
        Sum1 += abs(x[k]-x[k-1]);
        Sum2 += abs(x[k+4]-x[k+5]);
      }

      pbi->FragmentVariances[CurrentFrag] +=((Sum1>255)?255:Sum1);
      pbi->FragmentVariances[CurrentFrag + FragsAcross] += ((Sum2>255)?255:Sum2);

      if( Sum1 < FLimit &&
          Sum2 < FLimit &&
          (x[5] - x[4]) < QStep &&
          (x[4] - x[5]) < QStep ){

        /* low pass filtering (LPF7: 1 1 1 2 1 1 1) */
        Des[0              ] = (x[0] + x[0] +x[0] + x[1] * 2 +
                                x[2] + x[3] +x[4] + 4) >> 3;
        Des[PlaneLineStep  ] = (x[0] + x[0] +x[1] + x[2] * 2 +
                                x[3] + x[4] +x[5] + 4) >> 3;
        Des[PlaneLineStep*2] = (x[0] + x[1] +x[2] + x[3] * 2 +
                                x[4] + x[5] +x[6] + 4) >> 3;
        Des[PlaneLineStep*3] = (x[1] + x[2] +x[3] + x[4] * 2 +
                                x[5] + x[6] +x[7] + 4) >> 3;
        Des[PlaneLineStep*4] = (x[2] + x[3] +x[4] + x[5] * 2 +
                                x[6] + x[7] +x[8] + 4) >> 3;
        Des[PlaneLineStep*5] = (x[3] + x[4] +x[5] + x[6] * 2 +
                                x[7] + x[8] +x[9] + 4) >> 3;
        Des[PlaneLineStep*6] = (x[4] + x[5] +x[6] + x[7] * 2 +
                                x[8] + x[9] +x[9] + 4) >> 3;
        Des[PlaneLineStep*7] = (x[5] + x[6] +x[7] + x[8] * 2 +
                                x[9] + x[9] +x[9] + 4) >> 3;

      }else {
        /* copy the pixels to destination */
        Des[0              ]= (unsigned char)x[1];
        Des[PlaneLineStep  ]= (unsigned char)x[2];
        Des[PlaneLineStep*2]= (unsigned char)x[3];
        Des[PlaneLineStep*3]= (unsigned char)x[4];
        Des[PlaneLineStep*4]= (unsigned char)x[5];
        Des[PlaneLineStep*5]= (unsigned char)x[6];
        Des[PlaneLineStep*6]= (unsigned char)x[7];
        Des[PlaneLineStep*7]= (unsigned char)x[8];
      }
      Src ++;
      Des ++;
    }


    /* done with filtering the horizontal edge, now let's do the
       vertical one */
    /* skip the first one */
    if(CurrentFrag==StartFrag)
      CurrentFrag++;
    else{
      Des=DesPtr-8*PlaneLineStep+8*(CurrentFrag-StartFrag);
      Src=Des-5;
      Des-=4;

      QStep = QuantScale[pbi->FragQIndex[CurrentFrag]];
      FLimit = ( QStep * 3 ) >> 2;

      for( j=0; j<8 ; j++){
        x[0] = Src[0];
        x[1] = Src[1];
        x[2] = Src[2];
        x[3] = Src[3];
        x[4] = Src[4];
        x[5] = Src[5];
        x[6] = Src[6];
        x[7] = Src[7];
        x[8] = Src[8];
        x[9] = Src[9];

        Sum1=Sum2=0;

        for(k=1;k<=4;k++){
          Sum1 += abs(x[k]-x[k-1]);
          Sum2 += abs(x[k+4]-x[k+5]);
        }

        pbi->FragmentVariances[CurrentFrag-1] += ((Sum1>255)?255:Sum1);
        pbi->FragmentVariances[CurrentFrag] += ((Sum2>255)?255:Sum2);

        if( Sum1 < FLimit &&
            Sum2 < FLimit &&
            (x[5] - x[4]) < QStep &&
            (x[4] - x[5]) < QStep ){

          /* low pass filtering (LPF7: 1 1 1 2 1 1 1) */
          Des[0] = (x[0] + x[0] +x[0] + x[1] * 2 + x[2] + x[3] +x[4] + 4) >> 3;
          Des[1] = (x[0] + x[0] +x[1] + x[2] * 2 + x[3] + x[4] +x[5] + 4) >> 3;
          Des[2] = (x[0] + x[1] +x[2] + x[3] * 2 + x[4] + x[5] +x[6] + 4) >> 3;
          Des[3] = (x[1] + x[2] +x[3] + x[4] * 2 + x[5] + x[6] +x[7] + 4) >> 3;
          Des[4] = (x[2] + x[3] +x[4] + x[5] * 2 + x[6] + x[7] +x[8] + 4) >> 3;
          Des[5] = (x[3] + x[4] +x[5] + x[6] * 2 + x[7] + x[8] +x[9] + 4) >> 3;
          Des[6] = (x[4] + x[5] +x[6] + x[7] * 2 + x[8] + x[9] +x[9] + 4) >> 3;
          Des[7] = (x[5] + x[6] +x[7] + x[8] * 2 + x[9] + x[9] +x[9] + 4) >> 3;
        }

        Src += PlaneLineStep;
        Des += PlaneLineStep;
      }
      CurrentFrag ++;
    }
  }
}

static void DeblockVerticalEdgesInLoopFilteredBand(PB_INSTANCE *pbi,
                                            unsigned char *SrcPtr,
                                            unsigned char *DesPtr,
                                            ogg_uint32_t PlaneLineStep,
                                            ogg_uint32_t FragsAcross,
                                            ogg_uint32_t StartFrag,
                                            const ogg_uint32_t *QuantScale){
  ogg_uint32_t j,k;
  ogg_uint32_t CurrentFrag=StartFrag;
  ogg_int32_t QStep;
  ogg_int32_t FLimit;
  unsigned char *Src, *Des;
  ogg_int32_t  x[10];
  ogg_int32_t  Sum1, Sum2;

  while(CurrentFrag < StartFrag + FragsAcross-1) {

    Src=SrcPtr+8*(CurrentFrag-StartFrag+1)-5;
    Des=DesPtr+8*(CurrentFrag-StartFrag+1)-4;

    QStep = QuantScale[pbi->FragQIndex[CurrentFrag+1]];
    FLimit = ( QStep * 3)>>2 ;

    for( j=0; j<8 ; j++){
      x[0] = Src[0];
      x[1] = Src[1];
      x[2] = Src[2];
      x[3] = Src[3];
      x[4] = Src[4];
      x[5] = Src[5];
      x[6] = Src[6];
      x[7] = Src[7];
      x[8] = Src[8];
      x[9] = Src[9];

      Sum1=Sum2=0;

      for(k=1;k<=4;k++){
        Sum1 += abs(x[k]-x[k-1]);
        Sum2 += abs(x[k+4]-x[k+5]);
      }

      pbi->FragmentVariances[CurrentFrag] += ((Sum1>255)?255:Sum1);
      pbi->FragmentVariances[CurrentFrag+1] += ((Sum2>255)?255:Sum2);


      if( Sum1 < FLimit &&
          Sum2 < FLimit &&
          (x[5] - x[4]) < QStep &&
          (x[4] - x[5]) < QStep ){

        /* low pass filtering (LPF7: 1 1 1 2 1 1 1) */
        Des[0] = (x[0] + x[0] +x[0] + x[1] * 2 + x[2] + x[3] +x[4] + 4) >> 3;
        Des[1] = (x[0] + x[0] +x[1] + x[2] * 2 + x[3] + x[4] +x[5] + 4) >> 3;
        Des[2] = (x[0] + x[1] +x[2] + x[3] * 2 + x[4] + x[5] +x[6] + 4) >> 3;
        Des[3] = (x[1] + x[2] +x[3] + x[4] * 2 + x[5] + x[6] +x[7] + 4) >> 3;
        Des[4] = (x[2] + x[3] +x[4] + x[5] * 2 + x[6] + x[7] +x[8] + 4) >> 3;
        Des[5] = (x[3] + x[4] +x[5] + x[6] * 2 + x[7] + x[8] +x[9] + 4) >> 3;
        Des[6] = (x[4] + x[5] +x[6] + x[7] * 2 + x[8] + x[9] +x[9] + 4) >> 3;
        Des[7] = (x[5] + x[6] +x[7] + x[8] * 2 + x[9] + x[9] +x[9] + 4) >> 3;
      }
      Src +=PlaneLineStep;
                Des +=PlaneLineStep;

    }
    CurrentFrag ++;
  }
}

static void DeblockPlane(PB_INSTANCE *pbi,
                  unsigned char *SourceBuffer,
                  unsigned char *DestinationBuffer,
                  ogg_uint32_t Channel ){

  ogg_uint32_t i,k;
  ogg_uint32_t PlaneLineStep=0;
  ogg_uint32_t StartFrag =0;
  ogg_uint32_t PixelIndex=0;
  unsigned char * SrcPtr=0, * DesPtr=0;
  ogg_uint32_t FragsAcross=0;
  ogg_uint32_t FragsDown=0;
  const ogg_uint32_t *QuantScale=0;

  switch( Channel ){
  case 0:
    /* Get the parameters */
    PlaneLineStep = pbi->YStride;
    FragsAcross = pbi->HFragments;
    FragsDown = pbi->VFragments;
    StartFrag = 0;
    PixelIndex = pbi->ReconYDataOffset;
    SrcPtr = & SourceBuffer[PixelIndex];
    DesPtr = & DestinationBuffer[PixelIndex];
    break;

  case 1:
    /* Get the parameters */
    PlaneLineStep = pbi->UVStride;
    FragsAcross = pbi->HFragments / 2;
    FragsDown = pbi->VFragments / 2;
    StartFrag = pbi->YPlaneFragments;

    PixelIndex = pbi->ReconUDataOffset;
    SrcPtr = & SourceBuffer[PixelIndex];
    DesPtr = & DestinationBuffer[PixelIndex];
    break;

  default:
    /* Get the parameters */
    PlaneLineStep = pbi->UVStride;
    FragsAcross = pbi->HFragments / 2;
    FragsDown = pbi->VFragments / 2;
    StartFrag =   pbi->YPlaneFragments + pbi->UVPlaneFragments;

    PixelIndex = pbi->ReconVDataOffset;
    SrcPtr = & SourceBuffer[PixelIndex];
    DesPtr = & DestinationBuffer[PixelIndex];
    break;
  }

  QuantScale = DcQuantScaleV1;

  for(i=0;i<4;i++)
    memcpy(DesPtr+i*PlaneLineStep, SrcPtr+i*PlaneLineStep, PlaneLineStep);

  k = 1;

  while( k < FragsDown ){

    SrcPtr += 8*PlaneLineStep;
    DesPtr += 8*PlaneLineStep;

    /* Filter both the horizontal and vertical block edges inside the band */
    DeblockLoopFilteredBand(pbi, SrcPtr, DesPtr, PlaneLineStep,
                            FragsAcross, StartFrag, QuantScale);

    /* Move Pointers */
    StartFrag += FragsAcross;

    k ++;
  }

  /* The Last band */
  for(i=0;i<4;i++)
    memcpy(DesPtr+(i+4)*PlaneLineStep,
           SrcPtr+(i+4)*PlaneLineStep,
           PlaneLineStep);

  DeblockVerticalEdgesInLoopFilteredBand(pbi,SrcPtr,DesPtr,PlaneLineStep,
                                         FragsAcross,StartFrag,QuantScale);

}

static void DeblockFrame(PB_INSTANCE *pbi, unsigned char *SourceBuffer,
                  unsigned char *DestinationBuffer){

  memset(pbi->FragmentVariances, 0 , sizeof(ogg_int32_t) * pbi->UnitFragments);


  UpdateFragQIndex(pbi);

  /* Y */
  DeblockPlane( pbi, SourceBuffer, DestinationBuffer, 0);

  /* U */
  DeblockPlane( pbi, SourceBuffer, DestinationBuffer, 1);

  /* V */
  DeblockPlane( pbi, SourceBuffer, DestinationBuffer, 2);

}

void PostProcess(PB_INSTANCE *pbi){

  switch (pbi->PostProcessingLevel){
  case 8:
    /* on a slow machine, use a simpler and faster deblocking filter */
    DeblockFrame(pbi, pbi->LastFrameRecon,pbi->PostProcessBuffer);
    break;

  case 6:
    DeblockFrame(pbi, pbi->LastFrameRecon,pbi->PostProcessBuffer);
    UpdateUMVBorder(pbi, pbi->PostProcessBuffer );
    DeringFrame(pbi, pbi->PostProcessBuffer, pbi->PostProcessBuffer);
    break;

  case 5:
    DeblockFrame(pbi, pbi->LastFrameRecon,pbi->PostProcessBuffer);
    UpdateUMVBorder(pbi, pbi->PostProcessBuffer );
    DeringFrame(pbi, pbi->PostProcessBuffer, pbi->PostProcessBuffer);
    break;
  case 4:
    DeblockFrame(pbi, pbi->LastFrameRecon, pbi->PostProcessBuffer);
    break;
  case 1:
    UpdateFragQIndex(pbi);
    break;

  case 0:
    break;

  default:
    DeblockFrame(pbi, pbi->LastFrameRecon, pbi->PostProcessBuffer);
    UpdateUMVBorder(pbi, pbi->PostProcessBuffer );
    DeringFrame(pbi, pbi->PostProcessBuffer, pbi->PostProcessBuffer);
    break;
  }
}

