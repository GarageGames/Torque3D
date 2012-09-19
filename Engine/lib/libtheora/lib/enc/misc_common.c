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
  last mod: $Id: misc_common.c 15323 2008-09-19 19:43:59Z giles $

 ********************************************************************/

#include <string.h>
#include "codec_internal.h"
#include "block_inline.h"

#define FIXED_Q                 150
#define MAX_UP_REG_LOOPS        2

/* Gives the initial bytes per block estimate for each Q value */
static const double BpbTable[Q_TABLE_SIZE] = {
  0.42,  0.45,  0.46,  0.49,  0.51,  0.53,  0.56,  0.58,
  0.61,  0.64,  0.68,  0.71,  0.74,  0.77,  0.80,  0.84,
  0.89,  0.92,  0.98,  1.01,  1.04,  1.13,  1.17,  1.23,
  1.28,  1.34,  1.41,  1.45,  1.51,  1.59,  1.69,  1.80,
  1.84,  1.94,  2.02,  2.15,  2.23,  2.34,  2.44,  2.50,
  2.69,  2.80,  2.87,  3.04,  3.16,  3.29,  3.59,  3.66,
  3.86,  3.94,  4.22,  4.50,  4.64,  4.70,  5.24,  5.34,
  5.61,  5.87,  6.11,  6.41,  6.71,  6.99,  7.36,  7.69
};

static const double KfBpbTable[Q_TABLE_SIZE] = {
  0.74,  0.81,  0.88,  0.94,  1.00,  1.06,  1.14,  1.19,
  1.27,  1.34,  1.42,  1.49,  1.54,  1.59,  1.66,  1.73,
  1.80,  1.87,  1.97,  2.01,  2.08,  2.21,  2.25,  2.36,
  2.39,  2.50,  2.55,  2.65,  2.71,  2.82,  2.95,  3.01,
  3.11,  3.19,  3.31,  3.42,  3.58,  3.66,  3.78,  3.89,
  4.11,  4.26,  4.36,  4.39,  4.63,  4.76,  4.85,  5.04,
  5.26,  5.29,  5.47,  5.64,  5.76,  6.05,  6.35,  6.67,
  6.91,  7.17,  7.40,  7.56,  8.02,  8.45,  8.86,  9.38
};

double GetEstimatedBpb( CP_INSTANCE *cpi, ogg_uint32_t TargetQ ){
  ogg_uint32_t i;
  ogg_int32_t ThreshTableIndex = Q_TABLE_SIZE - 1;
  double BytesPerBlock;

  /* Search for the Q table index that matches the given Q. */
  for ( i = 0; i < Q_TABLE_SIZE; i++ ) {
    if ( TargetQ >= cpi->pb.QThreshTable[i] ) {
      ThreshTableIndex = i;
      break;
    }
  }

  /* Adjust according to Q shift and type of frame */
  if ( cpi->pb.FrameType == KEY_FRAME ) {
    /* Get primary prediction */
    BytesPerBlock = KfBpbTable[ThreshTableIndex];
  } else {
    /* Get primary prediction */
    BytesPerBlock = BpbTable[ThreshTableIndex];
    BytesPerBlock = BytesPerBlock * cpi->BpbCorrectionFactor;
  }

  return BytesPerBlock;
}

static void UpRegulateMB( CP_INSTANCE *cpi, ogg_uint32_t RegulationQ,
                   ogg_uint32_t SB, ogg_uint32_t MB, int NoCheck ) {
  ogg_int32_t  FragIndex;
  ogg_uint32_t B;

  /* Variables used in calculating corresponding row,col and index in
     UV planes */
  ogg_uint32_t UVRow;
  ogg_uint32_t UVColumn;
  ogg_uint32_t UVFragOffset;

  /* There may be MB's lying out of frame which must be ignored. For
   these MB's Top left block will have a negative Fragment Index. */
  if ( QuadMapToMBTopLeft(cpi->pb.BlockMap, SB, MB ) >= 0 ) {
    /* Up regulate the component blocks Y then UV. */
    for ( B=0; B<4; B++ ){
      FragIndex = QuadMapToIndex1( cpi->pb.BlockMap, SB, MB, B );

      if ( ( !cpi->pb.display_fragments[FragIndex] ) &&
           ( (NoCheck) || (cpi->FragmentLastQ[FragIndex] > RegulationQ) ) ){
        cpi->pb.display_fragments[FragIndex] = 1;
        cpi->extra_fragments[FragIndex] = 1;
        cpi->FragmentLastQ[FragIndex] = RegulationQ;
        cpi->MotionScore++;
      }
    }

    /* Check the two UV blocks */
    FragIndex = QuadMapToMBTopLeft(cpi->pb.BlockMap, SB, MB );

    UVRow = (FragIndex / (cpi->pb.HFragments * 2));
    UVColumn = (FragIndex % cpi->pb.HFragments) / 2;
    UVFragOffset = (UVRow * (cpi->pb.HFragments / 2)) + UVColumn;

    FragIndex = cpi->pb.YPlaneFragments + UVFragOffset;
    if ( ( !cpi->pb.display_fragments[FragIndex] ) &&
         ( (NoCheck) || (cpi->FragmentLastQ[FragIndex] > RegulationQ) ) ) {
      cpi->pb.display_fragments[FragIndex] = 1;
      cpi->extra_fragments[FragIndex] = 1;
      cpi->FragmentLastQ[FragIndex] = RegulationQ;
      cpi->MotionScore++;
    }

    FragIndex += cpi->pb.UVPlaneFragments;
    if ( ( !cpi->pb.display_fragments[FragIndex] ) &&
         ( (NoCheck) || (cpi->FragmentLastQ[FragIndex] > RegulationQ) ) ) {
      cpi->pb.display_fragments[FragIndex] = 1;
      cpi->extra_fragments[FragIndex] = 1;
      cpi->FragmentLastQ[FragIndex] = RegulationQ;
      cpi->MotionScore++;
    }
  }
}

static void UpRegulateBlocks (CP_INSTANCE *cpi, ogg_uint32_t RegulationQ,
                       ogg_int32_t RecoveryBlocks,
                       ogg_uint32_t * LastSB, ogg_uint32_t * LastMB ) {

  ogg_uint32_t LoopTimesRound = 0;
  ogg_uint32_t MaxSB = cpi->pb.YSBRows *
    cpi->pb.YSBCols;   /* Tot super blocks in image */
  ogg_uint32_t SB, MB; /* Super-Block and macro block indices. */

  /* First scan for blocks for which a residue update is outstanding. */
  while ( (cpi->MotionScore < RecoveryBlocks) &&
          (LoopTimesRound < MAX_UP_REG_LOOPS) ) {
    LoopTimesRound++;

    for ( SB = (*LastSB); SB < MaxSB; SB++ ) {
      /* Check its four Macro-Blocks */
      for ( MB=(*LastMB); MB<4; MB++ ) {
        /* Mark relevant blocks for update */
        UpRegulateMB( cpi, RegulationQ, SB, MB, 0 );

        /* Keep track of the last refresh MB. */
        (*LastMB) += 1;
        if ( (*LastMB) == 4 )
          (*LastMB) = 0;

        /* Termination clause */
        if (cpi->MotionScore >= RecoveryBlocks) {
          /* Make sure we don't stall at SB level */
          if ( *LastMB == 0 )
            SB++;
          break;
        }
      }

      /* Termination clause */
      if (cpi->MotionScore >= RecoveryBlocks)
        break;
    }

    /* Update super block start index  */
    if ( SB >= MaxSB){
      (*LastSB) = 0;
    }else{
      (*LastSB) = SB;
    }
  }
}

void UpRegulateDataStream (CP_INSTANCE *cpi, ogg_uint32_t RegulationQ,
                           ogg_int32_t RecoveryBlocks ) {
  ogg_uint32_t LastPassMBPos = 0;
  ogg_uint32_t StdLastMBPos = 0;

  ogg_uint32_t MaxSB = cpi->pb.YSBRows *
    cpi->pb.YSBCols;    /* Tot super blocks in image */

  ogg_uint32_t SB=0;    /* Super-Block index */
  ogg_uint32_t MB;      /* Macro-Block index */

  /* Decduct the number of blocks in an MB / 2 from the recover block count.
     This will compensate for the fact that once we start checking an MB
     we test every block in that macro block */
  if ( RecoveryBlocks > 3 )
    RecoveryBlocks -= 3;

  /* Up regulate blocks last coded at higher Q */
  UpRegulateBlocks( cpi, RegulationQ, RecoveryBlocks,
                    &cpi->LastEndSB, &StdLastMBPos );

  /* If we have still not used up the minimum number of blocks and are
     at the minimum Q then run through a final pass of the data to
     insure that each block gets a final refresh. */
  if ( (RegulationQ == VERY_BEST_Q) &&
       (cpi->MotionScore < RecoveryBlocks) ) {
    if ( cpi->FinalPassLastPos < MaxSB ) {
      for ( SB = cpi->FinalPassLastPos; SB < MaxSB; SB++ ) {
        /* Check its four Macro-Blocks */
        for ( MB=LastPassMBPos; MB<4; MB++ ) {
          /* Mark relevant blocks for update */
          UpRegulateMB( cpi, RegulationQ, SB, MB, 1 );

          /* Keep track of the last refresh MB. */
          LastPassMBPos += 1;
          if ( LastPassMBPos == 4 ) {
            LastPassMBPos = 0;

            /* Increment SB index */
            cpi->FinalPassLastPos += 1;
          }

          /* Termination clause */
          if (cpi->MotionScore >= RecoveryBlocks)
            break;
        }

        /* Termination clause */
        if (cpi->MotionScore >= RecoveryBlocks)
          break;

      }
    }
  }
}

void RegulateQ( CP_INSTANCE *cpi, ogg_int32_t UpdateScore ) {
  double PredUnitScoreBytes;
  ogg_uint32_t QIndex = Q_TABLE_SIZE - 1;
  ogg_uint32_t i;

  if ( UpdateScore > 0 ) {
    double TargetUnitScoreBytes = (double)cpi->ThisFrameTargetBytes /
      (double)UpdateScore;
    double LastBitError = 10000.0;       /* Silly high number */
    /* Search for the best Q for the target bitrate. */
    for ( i = 0; i < Q_TABLE_SIZE; i++ ) {
      PredUnitScoreBytes = GetEstimatedBpb( cpi, cpi->pb.QThreshTable[i] );
      if ( PredUnitScoreBytes > TargetUnitScoreBytes ) {
        if ( (PredUnitScoreBytes - TargetUnitScoreBytes) <= LastBitError ) {
          QIndex = i;
        } else {
          QIndex = i - 1;
        }
        break;
      } else {
        LastBitError = TargetUnitScoreBytes - PredUnitScoreBytes;
      }
    }
  }

  /* QIndex should now indicate the optimal Q. */
  cpi->pb.ThisFrameQualityValue = cpi->pb.QThreshTable[QIndex];

  /* Apply range restrictions for key frames. */
  if ( cpi->pb.FrameType == KEY_FRAME ) {
    if ( cpi->pb.ThisFrameQualityValue > cpi->pb.QThreshTable[20] )
      cpi->pb.ThisFrameQualityValue = cpi->pb.QThreshTable[20];
    else if ( cpi->pb.ThisFrameQualityValue < cpi->pb.QThreshTable[50] )
      cpi->pb.ThisFrameQualityValue = cpi->pb.QThreshTable[50];
  }

  /* Limit the Q value to the maximum available value */
  if (cpi->pb.ThisFrameQualityValue >
      cpi->pb.QThreshTable[cpi->Configuration.ActiveMaxQ]) {
    cpi->pb.ThisFrameQualityValue =
      (ogg_uint32_t)cpi->pb.QThreshTable[cpi->Configuration.ActiveMaxQ];
  }

  if(cpi->FixedQ) {
    if ( cpi->pb.FrameType == KEY_FRAME ) {
      cpi->pb.ThisFrameQualityValue = cpi->pb.QThreshTable[43];
      cpi->pb.ThisFrameQualityValue = cpi->FixedQ;
    } else {
      cpi->pb.ThisFrameQualityValue = cpi->FixedQ;
    }
  }

  /* If the quantizer value has changed then re-initialise it */
  if ( cpi->pb.ThisFrameQualityValue != cpi->pb.LastFrameQualityValue ) {
    /* Initialise quality tables. */
    UpdateQC( cpi, cpi->pb.ThisFrameQualityValue );
    cpi->pb.LastFrameQualityValue = cpi->pb.ThisFrameQualityValue;
  }
}

void CopyBackExtraFrags(CP_INSTANCE *cpi){
  ogg_uint32_t  i,j;
  unsigned char * SrcPtr;
  unsigned char * DestPtr;
  ogg_uint32_t  PlaneLineStep;
  ogg_uint32_t  PixelIndex;

  /*  Copy back for Y plane. */
  PlaneLineStep = cpi->pb.info.width;
  for ( i = 0; i < cpi->pb.YPlaneFragments; i++ ) {
    /* We are only interested in updated fragments. */
    if ( cpi->extra_fragments[i] ) {
      /* Get the start index for the fragment. */
      PixelIndex = cpi->pb.pixel_index_table[i];
      SrcPtr = &cpi->yuv1ptr[PixelIndex];
      DestPtr = &cpi->ConvDestBuffer[PixelIndex];

      for ( j = 0; j < VFRAGPIXELS; j++ ) {
        memcpy( DestPtr, SrcPtr, HFRAGPIXELS);

        SrcPtr += PlaneLineStep;
        DestPtr += PlaneLineStep;
      }
    }
  }

  /* Now the U and V planes */
  PlaneLineStep = cpi->pb.info.width / 2;
  for ( i = cpi->pb.YPlaneFragments;
        i < (cpi->pb.YPlaneFragments + (2 * cpi->pb.UVPlaneFragments)) ;
        i++ ) {

    /* We are only interested in updated fragments. */
    if ( cpi->extra_fragments[i] ) {
      /* Get the start index for the fragment. */
      PixelIndex = cpi->pb.pixel_index_table[i];
      SrcPtr = &cpi->yuv1ptr[PixelIndex];
      DestPtr = &cpi->ConvDestBuffer[PixelIndex];

      for ( j = 0; j < VFRAGPIXELS; j++ ) {
        memcpy( DestPtr, SrcPtr, HFRAGPIXELS);
        SrcPtr += PlaneLineStep;
        DestPtr += PlaneLineStep;
      }
    }
  }
}

