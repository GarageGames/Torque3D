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
  last mod: $Id: encoder_toplevel.c 15383 2008-10-10 14:33:46Z xiphmont $

 ********************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "toplevel_lookup.h"
#include "../internal.h"
#include "dsp.h"
#include "codec_internal.h"

#define A_TABLE_SIZE        29
#define DF_CANDIDATE_WINDOW 5

/*
 * th_quant_info for VP3
 */

/*The default quantization parameters used by VP3.1.*/
static const int OC_VP31_RANGE_SIZES[1]={63};
static const th_quant_base OC_VP31_BASES_INTRA_Y[2]={
  {
     16, 11, 10, 16, 24,  40, 51, 61,
     12, 12, 14, 19, 26,  58, 60, 55,
     14, 13, 16, 24, 40,  57, 69, 56,
     14, 17, 22, 29, 51,  87, 80, 62,
     18, 22, 37, 58, 68, 109,103, 77,
     24, 35, 55, 64, 81, 104,113, 92,
     49, 64, 78, 87,103, 121,120,101,
     72, 92, 95, 98,112, 100,103, 99
  },
  {
     16, 11, 10, 16, 24,  40, 51, 61,
     12, 12, 14, 19, 26,  58, 60, 55,
     14, 13, 16, 24, 40,  57, 69, 56,
     14, 17, 22, 29, 51,  87, 80, 62,
     18, 22, 37, 58, 68, 109,103, 77,
     24, 35, 55, 64, 81, 104,113, 92,
     49, 64, 78, 87,103, 121,120,101,
     72, 92, 95, 98,112, 100,103, 99
  }
};
static const th_quant_base OC_VP31_BASES_INTRA_C[2]={
  {
     17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99
  },
  {
     17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99
  }
};
static const th_quant_base OC_VP31_BASES_INTER[2]={
  {
     16, 16, 16, 20, 24, 28, 32, 40,
     16, 16, 20, 24, 28, 32, 40, 48,
     16, 20, 24, 28, 32, 40, 48, 64,
     20, 24, 28, 32, 40, 48, 64, 64,
     24, 28, 32, 40, 48, 64, 64, 64,
     28, 32, 40, 48, 64, 64, 64, 96,
     32, 40, 48, 64, 64, 64, 96,128,
     40, 48, 64, 64, 64, 96,128,128
  },
  {
     16, 16, 16, 20, 24, 28, 32, 40,
     16, 16, 20, 24, 28, 32, 40, 48,
     16, 20, 24, 28, 32, 40, 48, 64,
     20, 24, 28, 32, 40, 48, 64, 64,
     24, 28, 32, 40, 48, 64, 64, 64,
     28, 32, 40, 48, 64, 64, 64, 96,
     32, 40, 48, 64, 64, 64, 96,128,
     40, 48, 64, 64, 64, 96,128,128
  }
};

const th_quant_info TH_VP31_QUANT_INFO={
  {
    220,200,190,180,170,170,160,160,
    150,150,140,140,130,130,120,120,
    110,110,100,100, 90, 90, 90, 80,
     80, 80, 70, 70, 70, 60, 60, 60,
     60, 50, 50, 50, 50, 40, 40, 40,
     40, 40, 30, 30, 30, 30, 30, 30,
     30, 20, 20, 20, 20, 20, 20, 20,
     20, 10, 10, 10, 10, 10, 10, 10
  },
  {
    500,450,400,370,340,310,285,265,
    245,225,210,195,185,180,170,160,
    150,145,135,130,125,115,110,107,
    100, 96, 93, 89, 85, 82, 75, 74,
     70, 68, 64, 60, 57, 56, 52, 50,
     49, 45, 44, 43, 40, 38, 37, 35,
     33, 32, 30, 29, 28, 25, 24, 22,
     21, 19, 18, 17, 15, 13, 12, 10
  },
  {
    30,25,20,20,15,15,14,14,
    13,13,12,12,11,11,10,10,
     9, 9, 8, 8, 7, 7, 7, 7,
     6, 6, 6, 6, 5, 5, 5, 5,
     4, 4, 4, 4, 3, 3, 3, 3,
     2, 2, 2, 2, 2, 2, 2, 2,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    {
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_Y},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_C},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_C}
    },
    {
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER}
    }
  }
};


static void EClearFragmentInfo(CP_INSTANCE * cpi){
  if(cpi->extra_fragments)
    _ogg_free(cpi->extra_fragments);
  if(cpi->FragmentLastQ)
    _ogg_free(cpi->FragmentLastQ);
  if(cpi->FragTokens)
    _ogg_free(cpi->FragTokens);
  if(cpi->FragTokenCounts)
    _ogg_free(cpi->FragTokenCounts);
  if(cpi->RunHuffIndices)
    _ogg_free(cpi->RunHuffIndices);
  if(cpi->LastCodedErrorScore)
    _ogg_free(cpi->LastCodedErrorScore);
  if(cpi->ModeList)
    _ogg_free(cpi->ModeList);
  if(cpi->MVList)
    _ogg_free(cpi->MVList);
  if(cpi->DCT_codes )
    _ogg_free( cpi->DCT_codes );
  if(cpi->DCTDataBuffer )
    _ogg_free( cpi->DCTDataBuffer);
  if(cpi->quantized_list)
    _ogg_free( cpi->quantized_list);
  if(cpi->OriginalDC)
    _ogg_free( cpi->OriginalDC);
  if(cpi->PartiallyCodedFlags)
    _ogg_free(cpi->PartiallyCodedFlags);
  if(cpi->PartiallyCodedMbPatterns)
    _ogg_free(cpi->PartiallyCodedMbPatterns);
  if(cpi->UncodedMbFlags)
    _ogg_free(cpi->UncodedMbFlags);

  if(cpi->BlockCodedFlags)
    _ogg_free(cpi->BlockCodedFlags);

  cpi->extra_fragments = 0;
  cpi->FragmentLastQ = 0;
  cpi->FragTokens = 0;
  cpi->FragTokenCounts = 0;
  cpi->RunHuffIndices = 0;
  cpi->LastCodedErrorScore = 0;
  cpi->ModeList = 0;
  cpi->MVList = 0;
  cpi->DCT_codes = 0;
  cpi->DCTDataBuffer = 0;
  cpi->quantized_list = 0;
  cpi->OriginalDC = 0;
  cpi->BlockCodedFlags = 0;
}

static void EInitFragmentInfo(CP_INSTANCE * cpi){

  /* clear any existing info */
  EClearFragmentInfo(cpi);

  /* Perform Fragment Allocations */
  cpi->extra_fragments =
    _ogg_malloc(cpi->pb.UnitFragments*sizeof(unsigned char));

  /* A note to people reading and wondering why malloc returns aren't
     checked:

     lines like the following that implement a general strategy of
     'check the return of malloc; a zero pointer means we're out of
     memory!'...:

  if(!cpi->extra_fragments) { EDeleteFragmentInfo(cpi); return FALSE; }

     ...are not useful.  It's true that many platforms follow this
     malloc behavior, but many do not.  The more modern malloc
     strategy is only to allocate virtual pages, which are not mapped
     until the memory on that page is touched.  At *that* point, if
     the machine is out of heap, the page fails to be mapped and a
     SEGV is generated.

     That means that if we want to deal with out of memory conditions,
     we *must* be prepared to process a SEGV.  If we implement the
     SEGV handler, there's no reason to to check malloc return; it is
     a waste of code. */

  cpi->FragmentLastQ =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->FragmentLastQ));
  cpi->FragTokens =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->FragTokens));
  cpi->OriginalDC =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->OriginalDC));
  cpi->FragTokenCounts =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->FragTokenCounts));
  cpi->RunHuffIndices =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->RunHuffIndices));
  cpi->LastCodedErrorScore =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->LastCodedErrorScore));
  cpi->BlockCodedFlags =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->BlockCodedFlags));
  cpi->ModeList =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->ModeList));
  cpi->MVList =
    _ogg_malloc(cpi->pb.UnitFragments*
                sizeof(*cpi->MVList));
  cpi->DCT_codes =
    _ogg_malloc(64*
                sizeof(*cpi->DCT_codes));
  cpi->DCTDataBuffer =
    _ogg_malloc(64*
                sizeof(*cpi->DCTDataBuffer));
  cpi->quantized_list =
    _ogg_malloc(64*
                sizeof(*cpi->quantized_list));
  cpi->PartiallyCodedFlags =
    _ogg_malloc(cpi->pb.MacroBlocks*
                sizeof(*cpi->PartiallyCodedFlags));
  cpi->PartiallyCodedMbPatterns =
    _ogg_malloc(cpi->pb.MacroBlocks*
                sizeof(*cpi->PartiallyCodedMbPatterns));
  cpi->UncodedMbFlags =
    _ogg_malloc(cpi->pb.MacroBlocks*
                sizeof(*cpi->UncodedMbFlags));

}

static void EClearFrameInfo(CP_INSTANCE * cpi) {
  if(cpi->ConvDestBuffer )
    _ogg_free(cpi->ConvDestBuffer );
  cpi->ConvDestBuffer = 0;

  if(cpi->yuv0ptr)
    _ogg_free(cpi->yuv0ptr);
  cpi->yuv0ptr = 0;

  if(cpi->yuv1ptr)
    _ogg_free(cpi->yuv1ptr);
  cpi->yuv1ptr = 0;

  if(cpi->OptimisedTokenListEb )
    _ogg_free(cpi->OptimisedTokenListEb);
  cpi->OptimisedTokenListEb = 0;

  if(cpi->OptimisedTokenList )
    _ogg_free(cpi->OptimisedTokenList);
  cpi->OptimisedTokenList = 0;

  if(cpi->OptimisedTokenListHi )
    _ogg_free(cpi->OptimisedTokenListHi);
  cpi->OptimisedTokenListHi = 0;

  if(cpi->OptimisedTokenListPl )
    _ogg_free(cpi->OptimisedTokenListPl);
  cpi->OptimisedTokenListPl = 0;

}

static void EInitFrameInfo(CP_INSTANCE * cpi){
  int FrameSize = cpi->pb.ReconYPlaneSize + 2 * cpi->pb.ReconUVPlaneSize;

  /* clear any existing info */
  EClearFrameInfo(cpi);

  /* allocate frames */
  cpi->ConvDestBuffer =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->ConvDestBuffer));
  cpi->yuv0ptr =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->yuv0ptr));
  cpi->yuv1ptr =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->yuv1ptr));
  cpi->OptimisedTokenListEb =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->OptimisedTokenListEb));
  cpi->OptimisedTokenList =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->OptimisedTokenList));
  cpi->OptimisedTokenListHi =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->OptimisedTokenListHi));
  cpi->OptimisedTokenListPl =
    _ogg_malloc(FrameSize*
                sizeof(*cpi->OptimisedTokenListPl));
}

static void SetupKeyFrame(CP_INSTANCE *cpi) {
  /* Make sure the "last frame" buffer contains the first frame data
     as well. */
  memcpy ( cpi->yuv0ptr, cpi->yuv1ptr,
           cpi->pb.ReconYPlaneSize + 2 * cpi->pb.ReconUVPlaneSize );

  /* Initialise the cpi->pb.display_fragments and other fragment
     structures for the first frame. */
  memset( cpi->pb.display_fragments, 1, cpi->pb.UnitFragments );
  memset( cpi->extra_fragments, 1, cpi->pb.UnitFragments );

  /* Set up for a KEY FRAME */
  cpi->pb.FrameType = KEY_FRAME;
}

static void AdjustKeyFrameContext(CP_INSTANCE *cpi) {
  ogg_uint32_t i;
  ogg_uint32_t  AvKeyFrameFrequency =
    (ogg_uint32_t) (cpi->CurrentFrame / cpi->KeyFrameCount);
  ogg_uint32_t  AvKeyFrameBytes =
    (ogg_uint32_t) (cpi->TotKeyFrameBytes / cpi->KeyFrameCount);
  ogg_uint32_t TotalWeight=0;
  ogg_int32_t AvKeyFramesPerSecond;
  ogg_int32_t MinFrameTargetRate;

  /* Update the frame carry over. */
  cpi->TotKeyFrameBytes += oggpackB_bytes(cpi->oggbuffer);

  /* reset keyframe context and calculate weighted average of last
     KEY_FRAME_CONTEXT keyframes */
  for( i = 0 ; i < KEY_FRAME_CONTEXT ; i ++ ) {
    if ( i < KEY_FRAME_CONTEXT -1) {
      cpi->PriorKeyFrameSize[i] = cpi->PriorKeyFrameSize[i+1];
      cpi->PriorKeyFrameDistance[i] = cpi->PriorKeyFrameDistance[i+1];
    } else {
      cpi->PriorKeyFrameSize[KEY_FRAME_CONTEXT - 1] =
        oggpackB_bytes(cpi->oggbuffer);
      cpi->PriorKeyFrameDistance[KEY_FRAME_CONTEXT - 1] =
        cpi->LastKeyFrame;
    }

    AvKeyFrameBytes += PriorKeyFrameWeight[i] *
      cpi->PriorKeyFrameSize[i];
    AvKeyFrameFrequency += PriorKeyFrameWeight[i] *
      cpi->PriorKeyFrameDistance[i];
    TotalWeight += PriorKeyFrameWeight[i];
  }
  AvKeyFrameBytes /= TotalWeight;
  AvKeyFrameFrequency /= TotalWeight;
  AvKeyFramesPerSecond =  100 * cpi->Configuration.OutputFrameRate /
    AvKeyFrameFrequency ;

  /* Calculate a new target rate per frame allowing for average key
     frame frequency over newest frames . */
  if ( 100 * cpi->Configuration.TargetBandwidth >
       AvKeyFrameBytes * AvKeyFramesPerSecond &&
       (100 * cpi->Configuration.OutputFrameRate - AvKeyFramesPerSecond )){
    cpi->frame_target_rate =
      (ogg_int32_t)(100* cpi->Configuration.TargetBandwidth -
                    AvKeyFrameBytes * AvKeyFramesPerSecond ) /
      ( (100 * cpi->Configuration.OutputFrameRate - AvKeyFramesPerSecond ) );
  } else {
    /* don't let this number get too small!!! */
    cpi->frame_target_rate = 1;
  }

  /* minimum allowable frame_target_rate */
  MinFrameTargetRate = (cpi->Configuration.TargetBandwidth /
                        cpi->Configuration.OutputFrameRate) / 3;

  if(cpi->frame_target_rate < MinFrameTargetRate ) {
    cpi->frame_target_rate = MinFrameTargetRate;
  }

  cpi->LastKeyFrame = 1;
  cpi->LastKeyFrameSize=oggpackB_bytes(cpi->oggbuffer);

}

static void UpdateFrame(CP_INSTANCE *cpi){

  double CorrectionFactor;

  /* Reset the DC predictors. */
  cpi->pb.LastIntraDC = 0;
  cpi->pb.InvLastIntraDC = 0;
  cpi->pb.LastInterDC = 0;
  cpi->pb.InvLastInterDC = 0;

  /* Initialise bit packing mechanism. */
  oggpackB_reset(cpi->oggbuffer);

  /* mark as video frame */
  oggpackB_write(cpi->oggbuffer,0,1);

  /* Write out the frame header information including size. */
  WriteFrameHeader(cpi);

  /* Copy back any extra frags that are to be updated by the codec
     as part of the background cleanup task */
  CopyBackExtraFrags(cpi);

  /* Encode the data.  */
  EncodeData(cpi);

  /* Adjust drop frame trigger. */
  if ( cpi->pb.FrameType != KEY_FRAME ) {
    /* Apply decay factor then add in the last frame size. */
    cpi->DropFrameTriggerBytes =
      ((cpi->DropFrameTriggerBytes * (DF_CANDIDATE_WINDOW-1)) /
       DF_CANDIDATE_WINDOW) + oggpackB_bytes(cpi->oggbuffer);
  }else{
    /* Increase cpi->DropFrameTriggerBytes a little. Just after a key
       frame may actually be a good time to drop a frame. */
    cpi->DropFrameTriggerBytes =
      (cpi->DropFrameTriggerBytes * DF_CANDIDATE_WINDOW) /
      (DF_CANDIDATE_WINDOW-1);
  }

  /* Test for overshoot which may require a dropped frame next time
     around.  If we are already in a drop frame condition but the
     previous frame was not dropped then the threshold for continuing
     to allow dropped frames is reduced. */
  if ( cpi->DropFrameCandidate ) {
    if ( cpi->DropFrameTriggerBytes >
         (cpi->frame_target_rate * (DF_CANDIDATE_WINDOW+1)) )
      cpi->DropFrameCandidate = 1;
    else
      cpi->DropFrameCandidate = 0;
  } else {
    if ( cpi->DropFrameTriggerBytes >
         (cpi->frame_target_rate * ((DF_CANDIDATE_WINDOW*2)-2)) )
      cpi->DropFrameCandidate = 1;
    else
      cpi->DropFrameCandidate = 0;
  }

  /* Update the BpbCorrectionFactor variable according to whether or
     not we were close enough with our selection of DCT quantiser.  */
  if ( cpi->pb.FrameType != KEY_FRAME ) {
    /* Work out a size correction factor. */
    CorrectionFactor = (double)oggpackB_bytes(cpi->oggbuffer) /
      (double)cpi->ThisFrameTargetBytes;

    if ( (CorrectionFactor > 1.05) &&
         (cpi->pb.ThisFrameQualityValue <
          cpi->pb.QThreshTable[cpi->Configuration.ActiveMaxQ]) ) {
      CorrectionFactor = 1.0 + ((CorrectionFactor - 1.0)/2);
      if ( CorrectionFactor > 1.5 )
        cpi->BpbCorrectionFactor *= 1.5;
      else
        cpi->BpbCorrectionFactor *= CorrectionFactor;

      /* Keep BpbCorrectionFactor within limits */
      if ( cpi->BpbCorrectionFactor > MAX_BPB_FACTOR )
        cpi->BpbCorrectionFactor = MAX_BPB_FACTOR;
    } else if ( (CorrectionFactor < 0.95) &&
                (cpi->pb.ThisFrameQualityValue > VERY_BEST_Q) ){
      CorrectionFactor = 1.0 - ((1.0 - CorrectionFactor)/2);
      if ( CorrectionFactor < 0.75 )
        cpi->BpbCorrectionFactor *= 0.75;
      else
        cpi->BpbCorrectionFactor *= CorrectionFactor;

      /* Keep BpbCorrectionFactor within limits */
      if ( cpi->BpbCorrectionFactor < MIN_BPB_FACTOR )
        cpi->BpbCorrectionFactor = MIN_BPB_FACTOR;
    }
  }

  /* Adjust carry over and or key frame context. */
  if ( cpi->pb.FrameType == KEY_FRAME ) {
    /* Adjust the key frame context unless the key frame was very small */
    AdjustKeyFrameContext(cpi);
  } else {
    /* Update the frame carry over */
    cpi->CarryOver += ((ogg_int32_t)cpi->frame_target_rate -
                       (ogg_int32_t)oggpackB_bytes(cpi->oggbuffer));
  }
  cpi->TotalByteCount += oggpackB_bytes(cpi->oggbuffer);
}

static void CompressFirstFrame(CP_INSTANCE *cpi) {
  ogg_uint32_t i;

  /* set up context of key frame sizes and distances for more local
     datarate control */
  for( i = 0 ; i < KEY_FRAME_CONTEXT ; i ++ ) {
    cpi->PriorKeyFrameSize[i] = cpi->Configuration.KeyFrameDataTarget;
    cpi->PriorKeyFrameDistance[i] = cpi->pb.info.keyframe_frequency_force;
  }

  /* Keep track of the total number of Key Frames Coded. */
  cpi->KeyFrameCount = 1;
  cpi->LastKeyFrame = 1;
  cpi->TotKeyFrameBytes = 0;

  /* A key frame is not a dropped frame there for reset the count of
     consequative dropped frames. */
  cpi->DropCount = 0;

  SetupKeyFrame(cpi);

  /* Calculate a new target rate per frame allowing for average key
     frame frequency and size thus far. */
  if ( cpi->Configuration.TargetBandwidth >
       ((cpi->Configuration.KeyFrameDataTarget *
         cpi->Configuration.OutputFrameRate)/
        cpi->pb.info.keyframe_frequency) ) {

    cpi->frame_target_rate =
      (ogg_int32_t)((cpi->Configuration.TargetBandwidth -
                     ((cpi->Configuration.KeyFrameDataTarget *
                       cpi->Configuration.OutputFrameRate)/
                      cpi->pb.info.keyframe_frequency)) /
                    cpi->Configuration.OutputFrameRate);
  }else
    cpi->frame_target_rate = 1;

  /* Set baseline frame target rate. */
  cpi->BaseLineFrameTargetRate = cpi->frame_target_rate;

  /* A key frame is not a dropped frame there for reset the count of
     consequative dropped frames. */
  cpi->DropCount = 0;

  /* Initialise drop frame trigger to 5 frames worth of data. */
  cpi->DropFrameTriggerBytes = cpi->frame_target_rate * DF_CANDIDATE_WINDOW;

  /* Set a target size for this key frame based upon the baseline
     target and frequency */
  cpi->ThisFrameTargetBytes = cpi->Configuration.KeyFrameDataTarget;

  /* Get a DCT quantizer level for the key frame. */
  cpi->MotionScore = cpi->pb.UnitFragments;

  RegulateQ(cpi, cpi->pb.UnitFragments);

  cpi->pb.LastFrameQualityValue = cpi->pb.ThisFrameQualityValue;

  /* Initialise quantizer. */
  UpdateQC(cpi, cpi->pb.ThisFrameQualityValue );

  /* Initialise the cpi->pb.display_fragments and other fragment
     structures for the first frame. */
  for ( i = 0; i < cpi->pb.UnitFragments; i ++ )
    cpi->FragmentLastQ[i] = cpi->pb.ThisFrameQualityValue;

  /* Compress and output the frist frame. */
  PickIntra( cpi,
             cpi->pb.YSBRows, cpi->pb.YSBCols);
  UpdateFrame(cpi);

  /* Initialise the carry over rate targeting variables. */
  cpi->CarryOver = 0;

}

static void CompressKeyFrame(CP_INSTANCE *cpi){
  ogg_uint32_t  i;

  /* Before we compress reset the carry over to the actual frame carry over */
  cpi->CarryOver = cpi->Configuration.TargetBandwidth * cpi->CurrentFrame  /
    cpi->Configuration.OutputFrameRate - cpi->TotalByteCount;

  /* Keep track of the total number of Key Frames Coded */
  cpi->KeyFrameCount += 1;

  /* A key frame is not a dropped frame there for reset the count of
     consequative dropped frames. */
  cpi->DropCount = 0;

  SetupKeyFrame(cpi);

  /* set a target size for this frame */
  cpi->ThisFrameTargetBytes = (ogg_int32_t) cpi->frame_target_rate +
    ( (cpi->Configuration.KeyFrameDataTarget - cpi->frame_target_rate) *
      cpi->LastKeyFrame / cpi->pb.info.keyframe_frequency_force );

  if ( cpi->ThisFrameTargetBytes > cpi->Configuration.KeyFrameDataTarget )
    cpi->ThisFrameTargetBytes = cpi->Configuration.KeyFrameDataTarget;

  /* Get a DCT quantizer level for the key frame. */
  cpi->MotionScore = cpi->pb.UnitFragments;

  RegulateQ(cpi, cpi->pb.UnitFragments);

  cpi->pb.LastFrameQualityValue = cpi->pb.ThisFrameQualityValue;

  /* Initialise DCT tables. */
  UpdateQC(cpi, cpi->pb.ThisFrameQualityValue );

  /* Initialise the cpi->pb.display_fragments and other fragment
     structures for the first frame. */
  for ( i = 0; i < cpi->pb.UnitFragments; i ++ )
    cpi->FragmentLastQ[i] = cpi->pb.ThisFrameQualityValue;


  /* Compress and output the frist frame. */
  PickIntra( cpi,
             cpi->pb.YSBRows, cpi->pb.YSBCols);
  UpdateFrame(cpi);

}

static void CompressFrame( CP_INSTANCE *cpi) {
  ogg_int32_t min_blocks_per_frame;
  ogg_uint32_t  i;
  int DropFrame = 0;
  ogg_uint32_t  ResidueBlocksAdded=0;
  ogg_uint32_t  KFIndicator = 0;

  double QModStep;
  double QModifier = 1.0;

  /* Clear down the macro block level mode and MV arrays. */
  for ( i = 0; i < cpi->pb.UnitFragments; i++ ) {
    cpi->pb.FragCodingMethod[i] = CODE_INTER_NO_MV;  /* Default coding mode */
    cpi->pb.FragMVect[i].x = 0;
    cpi->pb.FragMVect[i].y = 0;
  }

  /* Default to delta frames. */
  cpi->pb.FrameType = DELTA_FRAME;

  /* Clear down the difference arrays for the current frame. */
  memset( cpi->pb.display_fragments, 0, cpi->pb.UnitFragments );
  memset( cpi->extra_fragments, 0, cpi->pb.UnitFragments );

  /* Calculate the target bytes for this frame. */
  cpi->ThisFrameTargetBytes = cpi->frame_target_rate;

  /* Correct target to try and compensate for any overall rate error
     that is developing */

  /* Set the max allowed Q for this frame based upon carry over
     history.  First set baseline worst Q for this frame */
  cpi->Configuration.ActiveMaxQ = cpi->Configuration.MaxQ + 10;
  if ( cpi->Configuration.ActiveMaxQ >= Q_TABLE_SIZE )
    cpi->Configuration.ActiveMaxQ = Q_TABLE_SIZE - 1;

  /* Make a further adjustment based upon the carry over and recent
   history..  cpi->Configuration.ActiveMaxQ reduced by 1 for each 1/2
   seconds worth of -ve carry over up to a limit of 6.  Also
   cpi->Configuration.ActiveMaxQ reduced if frame is a
   "DropFrameCandidate".  Remember that if we are behind the bit
   target carry over is -ve. */
  if ( cpi->CarryOver < 0 ) {
    if ( cpi->DropFrameCandidate ) {
      cpi->Configuration.ActiveMaxQ -= 4;
    }

    if ( cpi->CarryOver <
         -((ogg_int32_t)cpi->Configuration.TargetBandwidth*3) )
      cpi->Configuration.ActiveMaxQ -= 6;
    else
      cpi->Configuration.ActiveMaxQ +=
        (ogg_int32_t) ((cpi->CarryOver*2) /
                       (ogg_int32_t)cpi->Configuration.TargetBandwidth);

    /* Check that we have not dropped quality too far */
    if ( cpi->Configuration.ActiveMaxQ < cpi->Configuration.MaxQ )
      cpi->Configuration.ActiveMaxQ = cpi->Configuration.MaxQ;
  }

  /* Calculate the Q Modifier step size required to cause a step down
     from full target bandwidth to 40% of target between max Q and
     best Q */
  QModStep = 0.5 / (double)((Q_TABLE_SIZE - 1) -
                            cpi->Configuration.ActiveMaxQ);

  /* Set up the cpi->QTargetModifier[] table. */
  for ( i = 0; i < cpi->Configuration.ActiveMaxQ; i++ ) {
    cpi->QTargetModifier[i] = QModifier;
  }
  for ( i = cpi->Configuration.ActiveMaxQ; i < Q_TABLE_SIZE; i++ ) {
    cpi->QTargetModifier[i] = QModifier;
    QModifier -= QModStep;
  }

  /* if we are allowed to drop frames and are falling behind (eg more
     than x frames worth of bandwidth) */
  if ( cpi->pb.info.dropframes_p &&
       ( cpi->DropCount < cpi->MaxConsDroppedFrames) &&
       ( cpi->CarryOver <
         -((ogg_int32_t)cpi->Configuration.TargetBandwidth)) &&
       ( cpi->DropFrameCandidate) ) {
    /* (we didn't do this frame so we should have some left over for
       the next frame) */
    cpi->CarryOver += cpi->frame_target_rate;
    DropFrame = 1;
    cpi->DropCount ++;

    /* Adjust DropFrameTriggerBytes to account for the saving achieved. */
    cpi->DropFrameTriggerBytes =
      (cpi->DropFrameTriggerBytes *
       (DF_CANDIDATE_WINDOW-1))/DF_CANDIDATE_WINDOW;

    /* Even if we drop a frame we should account for it when
        considering key frame seperation. */
    cpi->LastKeyFrame++;
  } else if ( cpi->CarryOver <
              -((ogg_int32_t)cpi->Configuration.TargetBandwidth * 2) ) {
    /* Reduce frame bit target by 1.75% for each 1/10th of a seconds
       worth of -ve carry over down to a minimum of 65% of its
       un-modified value. */

    cpi->ThisFrameTargetBytes =
      (ogg_uint32_t)(cpi->ThisFrameTargetBytes * 0.65);
  } else if ( cpi->CarryOver < 0 ) {
    /* Note that cpi->CarryOver is a -ve here hence 1.0 "+" ... */
    cpi->ThisFrameTargetBytes =
      (ogg_uint32_t)(cpi->ThisFrameTargetBytes *
                     (1.0 + ( ((cpi->CarryOver * 10)/
                               ((ogg_int32_t)cpi->
                                Configuration.TargetBandwidth)) * 0.0175) ));
  }

  if ( !DropFrame ) {
    /*  pick all the macroblock modes and motion vectors */
    ogg_uint32_t InterError;
    ogg_uint32_t IntraError;


    /* Set Baseline filter level. */
    ConfigurePP( &cpi->pp, cpi->pb.info.noise_sensitivity);

    /* Score / analyses the fragments. */
    cpi->MotionScore = YUVAnalyseFrame(&cpi->pp, &KFIndicator );

    /* Get the baseline Q value */
    RegulateQ( cpi, cpi->MotionScore );

    /* Recode blocks if the error score in last frame was high. */
    ResidueBlocksAdded  = 0;
    for ( i = 0; i < cpi->pb.UnitFragments; i++ ){
      if ( !cpi->pb.display_fragments[i] ){
        if ( cpi->LastCodedErrorScore[i] >=
             ResidueErrorThresh[cpi->pb.FrameQIndex] ) {
          cpi->pb.display_fragments[i] = 1; /* Force block update */
          cpi->extra_fragments[i] = 1;      /* Insures up to date
                                               pixel data is used. */
          ResidueBlocksAdded ++;
        }
      }
    }

    /* Adjust the motion score to allow for residue blocks
       added. These are assumed to have below average impact on
       bitrate (Hence ResidueBlockFactor). */
    cpi->MotionScore = cpi->MotionScore +
      (ResidueBlocksAdded / ResidueBlockFactor[cpi->pb.FrameQIndex]);

    /* Estimate the min number of blocks at best Q */
    min_blocks_per_frame =
      (ogg_int32_t)(cpi->ThisFrameTargetBytes /
                    GetEstimatedBpb( cpi, VERY_BEST_Q ));
    if ( min_blocks_per_frame == 0 )
      min_blocks_per_frame = 1;

    /* If we have less than this number then consider adding in some
       extra blocks */
    if ( cpi->MotionScore < min_blocks_per_frame ) {
      min_blocks_per_frame =
        cpi->MotionScore +
        (ogg_int32_t)(((min_blocks_per_frame - cpi->MotionScore) * 4) / 3 );
      UpRegulateDataStream( cpi, VERY_BEST_Q, min_blocks_per_frame );
    }else{
      /* Reset control variable for best quality final pass. */
      cpi->FinalPassLastPos = 0;
    }

    /* Get the modified Q prediction taking into account extra blocks added. */
    RegulateQ( cpi, cpi->MotionScore );

    /* Unless we are already well ahead (4 seconds of data) of the
       projected bitrate */
    if ( cpi->CarryOver <
         (ogg_int32_t)(cpi->Configuration.TargetBandwidth * 4) ){
      /* Look at the predicted Q (pbi->FrameQIndex).  Adjust the
         target bits for this frame based upon projected Q and
         re-calculate.  The idea is that if the Q is better than a
         given (good enough) level then we will try and save some bits
         for use in more difficult segments. */
      cpi->ThisFrameTargetBytes =
        (ogg_int32_t) (cpi->ThisFrameTargetBytes *
                       cpi->QTargetModifier[cpi->pb.FrameQIndex]);

      /* Recalculate Q again */
      RegulateQ( cpi, cpi->MotionScore );
    }


    /* Select modes and motion vectors for each of the blocks : return
       an error score for inter and intra */
    PickModes( cpi, cpi->pb.YSBRows, cpi->pb.YSBCols,
               cpi->pb.info.width,
               &InterError, &IntraError );

    /* decide whether we really should have made this frame a key frame */
    /* forcing out a keyframe if the max interval is up is done at a higher level */
    if( cpi->pb.info.keyframe_auto_p){
      if( ( 2* IntraError < 5 * InterError )
          && ( KFIndicator >= (ogg_uint32_t)
               cpi->pb.info.keyframe_auto_threshold)
          && ( cpi->LastKeyFrame > cpi->pb.info.keyframe_mindistance)
          ){
        CompressKeyFrame(cpi);  /* Code a key frame */
        return;
      }

    }

    /* Increment the frames since last key frame count */
    cpi->LastKeyFrame++;

    /* Proceed with the frame update. */
    UpdateFrame(cpi);
    cpi->DropCount = 0;

    if ( cpi->MotionScore > 0 ){
      /* Note the Quantizer used for each block coded. */
      for ( i = 0; i < cpi->pb.UnitFragments; i++ ){
        if ( cpi->pb.display_fragments[i] ){
          cpi->FragmentLastQ[i] = cpi->pb.ThisFrameQualityValue;
        }
      }

    }
  }else{
    /* even if we 'drop' a frame, a placeholder must be written as we
       currently assume fixed frame rate timebase as Ogg mapping
       invariant */
    UpdateFrame(cpi);
  }
}

/********************** The toplevel: encode ***********************/

static int _ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static void theora_encode_dispatch_init(CP_INSTANCE *cpi);

int theora_encode_init(theora_state *th, theora_info *c){
  int i;

  CP_INSTANCE *cpi;

  memset(th, 0, sizeof(*th));
  /*Currently only the 4:2:0 format is supported.*/
  if(c->pixelformat!=OC_PF_420)return OC_IMPL;
  th->internal_encode=cpi=_ogg_calloc(1,sizeof(*cpi));
  theora_encode_dispatch_init(cpi);

  dsp_static_init (&cpi->dsp);
  memcpy (&cpi->pb.dsp, &cpi->dsp, sizeof(DspFunctions));

  c->version_major=TH_VERSION_MAJOR;
  c->version_minor=TH_VERSION_MINOR;
  c->version_subminor=TH_VERSION_SUB;

  InitTmpBuffers(&cpi->pb);
  InitPPInstance(&cpi->pp, &cpi->dsp);

  /* Initialise Configuration structure to legal values */
  if(c->quality>63)c->quality=63;
  if(c->quality<0)c->quality=32;
  if(c->target_bitrate<0)c->target_bitrate=0;
  /* we clamp target_bitrate to 24 bits after setting up the encoder */

  cpi->Configuration.BaseQ = c->quality;
  cpi->Configuration.FirstFrameQ = c->quality;
  cpi->Configuration.MaxQ = c->quality;
  cpi->Configuration.ActiveMaxQ = c->quality;

  cpi->MVChangeFactor    =    14;
  cpi->FourMvChangeFactor =   8;
  cpi->MinImprovementForNewMV = 25;
  cpi->ExhaustiveSearchThresh = 2500;
  cpi->MinImprovementForFourMV = 100;
  cpi->FourMVThreshold = 10000;
  cpi->BitRateCapFactor = 1.5;
  cpi->InterTripOutThresh = 5000;
  cpi->MVEnabled = 1;
  cpi->InterCodeCount = 127;
  cpi->BpbCorrectionFactor = 1.0;
  cpi->GoldenFrameEnabled = 1;
  cpi->InterPrediction = 1;
  cpi->MotionCompensation = 1;
  cpi->ThreshMapThreshold = 5;
  cpi->MaxConsDroppedFrames = 1;

  /* Set encoder flags. */
  /* if not AutoKeyframing cpi->ForceKeyFrameEvery = is frequency */
  if(!c->keyframe_auto_p)
    c->keyframe_frequency_force = c->keyframe_frequency;

  /* Set the frame rate variables. */
  if ( c->fps_numerator < 1 )
    c->fps_numerator = 1;
  if ( c->fps_denominator < 1 )
    c->fps_denominator = 1;

  /* don't go too nuts on keyframe spacing; impose a high limit to
     make certain the granulepos encoding strategy works */
  if(c->keyframe_frequency_force>32768)c->keyframe_frequency_force=32768;
  if(c->keyframe_mindistance>32768)c->keyframe_mindistance=32768;
  if(c->keyframe_mindistance>c->keyframe_frequency_force)
    c->keyframe_mindistance=c->keyframe_frequency_force;
  cpi->pb.keyframe_granule_shift=_ilog(c->keyframe_frequency_force-1);

  /* clamp the target_bitrate to a maximum of 24 bits so we get a
     more meaningful value when we write this out in the header. */
  if(c->target_bitrate>(1<<24)-1)c->target_bitrate=(1<<24)-1;

  /* copy in config */
  memcpy(&cpi->pb.info,c,sizeof(*c));
  th->i=&cpi->pb.info;
  th->granulepos=-1;

  /* Set up default values for QTargetModifier[Q_TABLE_SIZE] table */
  for ( i = 0; i < Q_TABLE_SIZE; i++ )
    cpi->QTargetModifier[i] = 1.0;

  /* Set up an encode buffer */
  cpi->oggbuffer = _ogg_malloc(sizeof(oggpack_buffer));
  oggpackB_writeinit(cpi->oggbuffer);

  /* Set data rate related variables. */
  cpi->Configuration.TargetBandwidth = (c->target_bitrate) / 8;

  cpi->Configuration.OutputFrameRate =
    (double)( c->fps_numerator /
              c->fps_denominator );

  cpi->frame_target_rate = cpi->Configuration.TargetBandwidth /
    cpi->Configuration.OutputFrameRate;

  /* Set key frame data rate target; this is nominal keyframe size */
  cpi->Configuration.KeyFrameDataTarget = (c->keyframe_data_target_bitrate *
                                           c->fps_denominator /
                                           c->fps_numerator ) / 8;

  /* Note the height and width in the pre-processor control structure. */
  cpi->ScanConfig.VideoFrameHeight = cpi->pb.info.height;
  cpi->ScanConfig.VideoFrameWidth = cpi->pb.info.width;

  InitFrameDetails(&cpi->pb);
  EInitFragmentInfo(cpi);
  EInitFrameInfo(cpi);

  /* Set up pre-processor config pointers. */
  cpi->ScanConfig.Yuv0ptr = cpi->yuv0ptr;
  cpi->ScanConfig.Yuv1ptr = cpi->yuv1ptr;
  cpi->ScanConfig.SrfWorkSpcPtr = cpi->ConvDestBuffer;
  cpi->ScanConfig.disp_fragments = cpi->pb.display_fragments;
  cpi->ScanConfig.RegionIndex = cpi->pb.pixel_index_table;

  /* Initialise the pre-processor module. */
  ScanYUVInit(&cpi->pp, &(cpi->ScanConfig));

  /* Initialise Motion compensation */
  InitMotionCompensation(cpi);

  /* Initialise the compression process. */
  /* We always start at frame 1 */
  cpi->CurrentFrame = 1;

  /* Reset the rate targeting correction factor. */
  cpi->BpbCorrectionFactor = 1.0;

  cpi->TotalByteCount = 0;
  cpi->TotalMotionScore = 0;

  /* Up regulation variables. */
  cpi->FinalPassLastPos = 0;  /* Used to regulate a final unrestricted pass. */
  cpi->LastEndSB = 0;         /* Where we were in the loop last time.  */
  cpi->ResidueLastEndSB = 0;  /* Where we were in the residue update
                                 loop last time. */

  InitHuffmanSet(&cpi->pb);

  /* This makes sure encoder version specific tables are initialised */
  memcpy(&cpi->pb.quant_info, &TH_VP31_QUANT_INFO, sizeof(th_quant_info));
  InitQTables(&cpi->pb);

  /* Indicate that the next frame to be compressed is the first in the
     current clip. */
  cpi->ThisIsFirstFrame = 1;
  cpi->readyflag = 1;

  cpi->pb.HeadersWritten = 0;
  /*We overload this flag to track header output.*/
  cpi->doneflag=-3;

  return 0;
}

int theora_encode_YUVin(theora_state *t,
                         yuv_buffer *yuv){
  ogg_int32_t i;
  unsigned char *LocalDataPtr;
  unsigned char *InputDataPtr;
  CP_INSTANCE *cpi=(CP_INSTANCE *)(t->internal_encode);

  if(!cpi->readyflag)return OC_EINVAL;
  if(cpi->doneflag>0)return OC_EINVAL;

  /* If frame size has changed, abort out for now */
  if (yuv->y_height != (int)cpi->pb.info.height ||
      yuv->y_width != (int)cpi->pb.info.width )
    return(-1);


  /* Copy over input YUV to internal YUV buffers. */
  /* we invert the image for backward compatibility with VP3 */
  /* First copy over the Y data */
  LocalDataPtr = cpi->yuv1ptr + yuv->y_width*(yuv->y_height - 1);
  InputDataPtr = yuv->y;
  for ( i = 0; i < yuv->y_height; i++ ){
    memcpy( LocalDataPtr, InputDataPtr, yuv->y_width );
    LocalDataPtr -= yuv->y_width;
    InputDataPtr += yuv->y_stride;
  }

  /* Now copy over the U data */
  LocalDataPtr = &cpi->yuv1ptr[(yuv->y_height * yuv->y_width)];
  LocalDataPtr += yuv->uv_width*(yuv->uv_height - 1);
  InputDataPtr = yuv->u;
  for ( i = 0; i < yuv->uv_height; i++ ){
    memcpy( LocalDataPtr, InputDataPtr, yuv->uv_width );
    LocalDataPtr -= yuv->uv_width;
    InputDataPtr += yuv->uv_stride;
  }

  /* Now copy over the V data */
  LocalDataPtr =
    &cpi->yuv1ptr[((yuv->y_height*yuv->y_width)*5)/4];
  LocalDataPtr += yuv->uv_width*(yuv->uv_height - 1);
  InputDataPtr = yuv->v;
  for ( i = 0; i < yuv->uv_height; i++ ){
    memcpy( LocalDataPtr, InputDataPtr, yuv->uv_width );
    LocalDataPtr -= yuv->uv_width;
    InputDataPtr += yuv->uv_stride;
  }

  /* Special case for first frame */
  if ( cpi->ThisIsFirstFrame ){
    CompressFirstFrame(cpi);
    cpi->ThisIsFirstFrame = 0;
    cpi->ThisIsKeyFrame = 0;
  } else {

    /* don't allow generating invalid files that overflow the p-frame
       shift, even if keyframe_auto_p is turned off */
    if(cpi->LastKeyFrame >= (ogg_uint32_t)
       cpi->pb.info.keyframe_frequency_force)
      cpi->ThisIsKeyFrame = 1;

    if ( cpi->ThisIsKeyFrame ) {
      CompressKeyFrame(cpi);
      cpi->ThisIsKeyFrame = 0;
    } else  {
      /* Compress the frame. */
      CompressFrame( cpi );
    }

  }

  /* Update stats variables. */
  cpi->LastFrameSize = oggpackB_bytes(cpi->oggbuffer);
  cpi->CurrentFrame++;
  cpi->packetflag=1;

  t->granulepos=
    ((cpi->CurrentFrame - cpi->LastKeyFrame)<<cpi->pb.keyframe_granule_shift)+
    cpi->LastKeyFrame - 1;

  return 0;
}

int theora_encode_packetout( theora_state *t, int last_p, ogg_packet *op){
  CP_INSTANCE *cpi=(CP_INSTANCE *)(t->internal_encode);
  long bytes=oggpackB_bytes(cpi->oggbuffer);

  if(!bytes)return(0);
  if(!cpi->packetflag)return(0);
  if(cpi->doneflag>0)return(-1);

  op->packet=oggpackB_get_buffer(cpi->oggbuffer);
  op->bytes=bytes;
  op->b_o_s=0;
  op->e_o_s=last_p;

  op->packetno=cpi->CurrentFrame;
  op->granulepos=t->granulepos;

  cpi->packetflag=0;
  if(last_p)cpi->doneflag=1;

  return 1;
}

static void _tp_writebuffer(oggpack_buffer *opb, const char *buf, const long len)
{
  long i;

  for (i = 0; i < len; i++)
    oggpackB_write(opb, *buf++, 8);
}

static void _tp_writelsbint(oggpack_buffer *opb, long value)
{
  oggpackB_write(opb, value&0xFF, 8);
  oggpackB_write(opb, value>>8&0xFF, 8);
  oggpackB_write(opb, value>>16&0xFF, 8);
  oggpackB_write(opb, value>>24&0xFF, 8);
}

/* build the initial short header for stream recognition and format */
int theora_encode_header(theora_state *t, ogg_packet *op){
  CP_INSTANCE *cpi=(CP_INSTANCE *)(t->internal_encode);
  int offset_y;

  oggpackB_reset(cpi->oggbuffer);
  oggpackB_write(cpi->oggbuffer,0x80,8);
  _tp_writebuffer(cpi->oggbuffer, "theora", 6);

  oggpackB_write(cpi->oggbuffer,TH_VERSION_MAJOR,8);
  oggpackB_write(cpi->oggbuffer,TH_VERSION_MINOR,8);
  oggpackB_write(cpi->oggbuffer,TH_VERSION_SUB,8);

  oggpackB_write(cpi->oggbuffer,cpi->pb.info.width>>4,16);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.height>>4,16);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.frame_width,24);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.frame_height,24);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.offset_x,8);
  /* Applications use offset_y to mean offset from the top of the image; the
   * meaning in the bitstream is the opposite (from the bottom). Transform.
   */
  offset_y = cpi->pb.info.height - cpi->pb.info.frame_height -
    cpi->pb.info.offset_y;
  oggpackB_write(cpi->oggbuffer,offset_y,8);

  oggpackB_write(cpi->oggbuffer,cpi->pb.info.fps_numerator,32);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.fps_denominator,32);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.aspect_numerator,24);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.aspect_denominator,24);

  oggpackB_write(cpi->oggbuffer,cpi->pb.info.colorspace,8);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.target_bitrate,24);
  oggpackB_write(cpi->oggbuffer,cpi->pb.info.quality,6);

  oggpackB_write(cpi->oggbuffer,cpi->pb.keyframe_granule_shift,5);

  oggpackB_write(cpi->oggbuffer,cpi->pb.info.pixelformat,2);

  oggpackB_write(cpi->oggbuffer,0,3); /* spare config bits */

  op->packet=oggpackB_get_buffer(cpi->oggbuffer);
  op->bytes=oggpackB_bytes(cpi->oggbuffer);

  op->b_o_s=1;
  op->e_o_s=0;

  op->packetno=0;

  op->granulepos=0;
  cpi->packetflag=0;

  return(0);
}

/* build the comment header packet from the passed metadata */
int theora_encode_comment(theora_comment *tc, ogg_packet *op)
{
  const char *vendor = theora_version_string();
  const int vendor_length = strlen(vendor);
  oggpack_buffer *opb;

  opb = _ogg_malloc(sizeof(oggpack_buffer));
  oggpackB_writeinit(opb);
  oggpackB_write(opb, 0x81, 8);
  _tp_writebuffer(opb, "theora", 6);

  _tp_writelsbint(opb, vendor_length);
  _tp_writebuffer(opb, vendor, vendor_length);

  _tp_writelsbint(opb, tc->comments);
  if(tc->comments){
    int i;
    for(i=0;i<tc->comments;i++){
      if(tc->user_comments[i]){
        _tp_writelsbint(opb,tc->comment_lengths[i]);
        _tp_writebuffer(opb,tc->user_comments[i],tc->comment_lengths[i]);
      }else{
        oggpackB_write(opb,0,32);
      }
    }
  }
  op->bytes=oggpack_bytes(opb);

  /* So we're expecting the application will free this? */
  op->packet=_ogg_malloc(oggpack_bytes(opb));
  memcpy(op->packet, oggpack_get_buffer(opb), oggpack_bytes(opb));
  oggpack_writeclear(opb);

  _ogg_free(opb);

  op->b_o_s=0;
  op->e_o_s=0;

  op->packetno=0;
  op->granulepos=0;

  return (0);
}

/* build the final header packet with the tables required
   for decode */
int theora_encode_tables(theora_state *t, ogg_packet *op){
  CP_INSTANCE *cpi=(CP_INSTANCE *)(t->internal_encode);

  oggpackB_reset(cpi->oggbuffer);
  oggpackB_write(cpi->oggbuffer,0x82,8);
  _tp_writebuffer(cpi->oggbuffer,"theora",6);

  WriteQTables(&cpi->pb,cpi->oggbuffer);
  WriteHuffmanTrees(cpi->pb.HuffRoot_VP3x,cpi->oggbuffer);

  op->packet=oggpackB_get_buffer(cpi->oggbuffer);
  op->bytes=oggpackB_bytes(cpi->oggbuffer);

  op->b_o_s=0;
  op->e_o_s=0;

  op->packetno=0;

  op->granulepos=0;
  cpi->packetflag=0;

  cpi->pb.HeadersWritten = 1;

  return(0);
}

static void theora_encode_clear (theora_state  *th){
  CP_INSTANCE *cpi;
  cpi=(CP_INSTANCE *)th->internal_encode;
  if(cpi){

    ClearHuffmanSet(&cpi->pb);
    ClearFragmentInfo(&cpi->pb);
    ClearFrameInfo(&cpi->pb);
    EClearFragmentInfo(cpi);
    EClearFrameInfo(cpi);
    ClearTmpBuffers(&cpi->pb);
    ClearPPInstance(&cpi->pp);

    oggpackB_writeclear(cpi->oggbuffer);
    _ogg_free(cpi->oggbuffer);
    _ogg_free(cpi);
  }

  memset(th,0,sizeof(*th));
}


/* returns, in seconds, absolute time of current packet in given
   logical stream */
static double theora_encode_granule_time(theora_state *th,
 ogg_int64_t granulepos){
#ifndef THEORA_DISABLE_FLOAT
  CP_INSTANCE *cpi=(CP_INSTANCE *)(th->internal_encode);
  PB_INSTANCE *pbi=(PB_INSTANCE *)(th->internal_decode);

  if(cpi)pbi=&cpi->pb;

  if(granulepos>=0){
    ogg_int64_t iframe=granulepos>>pbi->keyframe_granule_shift;
    ogg_int64_t pframe=granulepos-(iframe<<pbi->keyframe_granule_shift);

    return (iframe+pframe)*
      ((double)pbi->info.fps_denominator/pbi->info.fps_numerator);

  }
#endif

  return(-1); /* negative granulepos or float calculations disabled */
}

/* returns frame number of current packet in given logical stream */
static ogg_int64_t theora_encode_granule_frame(theora_state *th,
 ogg_int64_t granulepos){
  CP_INSTANCE *cpi=(CP_INSTANCE *)(th->internal_encode);
  PB_INSTANCE *pbi=(PB_INSTANCE *)(th->internal_decode);

  if(cpi)pbi=&cpi->pb;

  if(granulepos>=0){
    ogg_int64_t iframe=granulepos>>pbi->keyframe_granule_shift;
    ogg_int64_t pframe=granulepos-(iframe<<pbi->keyframe_granule_shift);

    return (iframe+pframe-1);
  }

  return(-1);
}


static int theora_encode_control(theora_state *th,int req,
 void *buf,size_t buf_sz) {
  CP_INSTANCE *cpi;
  PB_INSTANCE *pbi;
  int value;

  if(th == NULL)
    return TH_EFAULT;

  cpi = th->internal_encode;
  pbi = &cpi->pb;

  switch(req) {
    case TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE:
      {
	ogg_uint32_t keyframe_frequency_force;
	if( (buf==NULL) || (buf_sz!=sizeof(ogg_uint32_t))) return TH_EINVAL;
	keyframe_frequency_force=*(ogg_uint32_t *)buf;
	
	keyframe_frequency_force=
	  OC_MINI(keyframe_frequency_force,
		  1U<<cpi->pb.keyframe_granule_shift);
	cpi->pb.info.keyframe_frequency_force=
	  OC_MAXI(1,keyframe_frequency_force);
	*(ogg_uint32_t *)buf=cpi->pb.info.keyframe_frequency_force;
	return 0;
      }
    case TH_ENCCTL_SET_QUANT_PARAMS:
      if( ( buf==NULL&&buf_sz!=0 )
           || ( buf!=NULL&&buf_sz!=sizeof(th_quant_info) )
           || cpi->pb.HeadersWritten ){
        return TH_EINVAL;
      }

      if(buf==NULL)
	memcpy(&pbi->quant_info, &TH_VP31_QUANT_INFO, sizeof(th_quant_info));
      else
	memcpy(&pbi->quant_info, buf, sizeof(th_quant_info));
      InitQTables(pbi);

      return 0;
    case TH_ENCCTL_SET_VP3_COMPATIBLE:
      if(cpi->pb.HeadersWritten)
        return TH_EINVAL;

      memcpy(&pbi->quant_info, &TH_VP31_QUANT_INFO, sizeof(th_quant_info));
      InitQTables(pbi);

      return 0;
    case TH_ENCCTL_SET_SPLEVEL:
      if(buf == NULL || buf_sz != sizeof(int))
        return TH_EINVAL;

      memcpy(&value, buf, sizeof(int));

      switch(value) {
        case 0:
          cpi->MotionCompensation = 1;
          pbi->info.quick_p = 0;
        break;

        case 1:
          cpi->MotionCompensation = 1;
          pbi->info.quick_p = 1;
        break;

        case 2:
          cpi->MotionCompensation = 0;
          pbi->info.quick_p = 1;
        break;

        default:
          return TH_EINVAL;
      }

      return 0;
    case TH_ENCCTL_GET_SPLEVEL_MAX:
      value = 2;
      memcpy(buf, &value, sizeof(int));
      return 0;
    default:
      return TH_EIMPL;
  }
}

static void theora_encode_dispatch_init(CP_INSTANCE *cpi){
  cpi->dispatch_vtbl.clear=theora_encode_clear;
  cpi->dispatch_vtbl.control=theora_encode_control;
  cpi->dispatch_vtbl.granule_frame=theora_encode_granule_frame;
  cpi->dispatch_vtbl.granule_time=theora_encode_granule_time;
}
