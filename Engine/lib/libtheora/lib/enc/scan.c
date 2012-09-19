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
  last mod: $Id: scan.c 13884 2007-09-22 08:38:10Z giles $

 ********************************************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "codec_internal.h"
#include "dsp.h"

#define MAX_SEARCH_LINE_LEN                   7

#define SET8_0(ptr) \
  ((ogg_uint32_t *)ptr)[0] = 0x00000000; \
  ((ogg_uint32_t *)ptr)[1] = 0x00000000;
#define SET8_1(ptr) \
  ((ogg_uint32_t *)ptr)[0] = 0x01010101; \
  ((ogg_uint32_t *)ptr)[1] = 0x01010101;
#define SET8_8(ptr) \
  ((ogg_uint32_t *)ptr)[0] = 0x08080808; \
  ((ogg_uint32_t *)ptr)[1] = 0x08080808;

static ogg_uint32_t LineLengthScores[ MAX_SEARCH_LINE_LEN + 1 ] = {
  0, 0, 0, 0, 2, 4, 12, 24
};

static ogg_uint32_t BodyNeighbourScore = 8;
static double DiffDevisor = 0.0625;
#define HISTORY_BLOCK_FACTOR    2
#define MIN_STEP_THRESH 6
#define SCORE_MULT_LOW    0.5
#define SCORE_MULT_HIGH   4

#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

#define INTERNAL_BLOCK_HEIGHT   8
#define INTERNAL_BLOCK_WIDTH    8

#define BLOCK_NOT_CODED                       0
#define BLOCK_CODED_BAR                       3
#define BLOCK_CODED_SGC                       4
#define BLOCK_CODED_LOW                       4
#define BLOCK_CODED                           5

#define CANDIDATE_BLOCK_LOW                  -2
#define CANDIDATE_BLOCK                      -1

#define FIRST_ROW           0
#define NOT_EDGE_ROW        1
#define LAST_ROW            2

#define YDIFF_CB_ROWS                   (INTERNAL_BLOCK_HEIGHT * 3)
#define CHLOCALS_CB_ROWS                (INTERNAL_BLOCK_HEIGHT * 3)
#define PMAP_CB_ROWS                    (INTERNAL_BLOCK_HEIGHT * 3)

void ConfigurePP( PP_INSTANCE *ppi, int Level ) {
  switch ( Level ){
  case 0:
    ppi->SRFGreyThresh = 1;
    ppi->SRFColThresh = 1;
    ppi->NoiseSupLevel = 2;
    ppi->SgcLevelThresh = 1;
    ppi->SuvcLevelThresh = 1;
    ppi->GrpLowSadThresh = 6;
    ppi->GrpHighSadThresh = 24;
    ppi->PrimaryBlockThreshold = 2;
    ppi->SgcThresh = 10;

    ppi->PAKEnabled = 0;
    break;

  case 1:
    ppi->SRFGreyThresh = 2;
    ppi->SRFColThresh = 2;
    ppi->NoiseSupLevel = 2;
    ppi->SgcLevelThresh = 2;
    ppi->SuvcLevelThresh = 2;
    ppi->GrpLowSadThresh = 8;
    ppi->GrpHighSadThresh = 32;
    ppi->PrimaryBlockThreshold = 5;
    ppi->SgcThresh = 12;

    ppi->PAKEnabled = 1;
    break;

  case 2: /* Default VP3 settings */
    ppi->SRFGreyThresh = 3;
    ppi->SRFColThresh = 3;
    ppi->NoiseSupLevel = 2;
    ppi->SgcLevelThresh = 2;
    ppi->SuvcLevelThresh = 2;
    ppi->GrpLowSadThresh = 8;
    ppi->GrpHighSadThresh = 32;
    ppi->PrimaryBlockThreshold = 5;
    ppi->SgcThresh = 16;

    ppi->PAKEnabled = 1;
    break;

  case 3:
    ppi->SRFGreyThresh = 4;
    ppi->SRFColThresh = 4;
    ppi->NoiseSupLevel = 3;
    ppi->SgcLevelThresh = 3;
    ppi->SuvcLevelThresh = 3;
    ppi->GrpLowSadThresh = 10;
    ppi->GrpHighSadThresh = 48;
    ppi->PrimaryBlockThreshold = 5;
    ppi->SgcThresh = 18;

    ppi->PAKEnabled = 1;
    break;

  case 4:
    ppi->SRFGreyThresh = 5;
    ppi->SRFColThresh = 5;
    ppi->NoiseSupLevel = 3;
    ppi->SgcLevelThresh = 4;
    ppi->SuvcLevelThresh = 4;
    ppi->GrpLowSadThresh = 12;
    ppi->GrpHighSadThresh = 48;
    ppi->PrimaryBlockThreshold = 5;
    ppi->SgcThresh = 20;

    ppi->PAKEnabled = 1;
    break;

  case 5:
    ppi->SRFGreyThresh = 6;
    ppi->SRFColThresh = 6;
    ppi->NoiseSupLevel = 3;
    ppi->SgcLevelThresh = 4;
    ppi->SuvcLevelThresh = 4;
    ppi->GrpLowSadThresh = 12;
    ppi->GrpHighSadThresh = 64;
    ppi->PrimaryBlockThreshold = 10;
    ppi->SgcThresh = 24;

    ppi->PAKEnabled = 1;
    break;

  case 6:
    ppi->SRFGreyThresh = 6;
    ppi->SRFColThresh = 7;
    ppi->NoiseSupLevel = 3;
    ppi->SgcLevelThresh = 4;
    ppi->SuvcLevelThresh = 4;
    ppi->GrpLowSadThresh = 12;
    ppi->GrpHighSadThresh = 64;
    ppi->PrimaryBlockThreshold = 10;
    ppi->SgcThresh = 24;

    ppi->PAKEnabled = 1;
    break;

  default:
    ppi->SRFGreyThresh = 3;
    ppi->SRFColThresh = 3;
    ppi->NoiseSupLevel = 2;
    ppi->SgcLevelThresh = 2;
    ppi->SuvcLevelThresh = 2;
    ppi->GrpLowSadThresh = 10;
    ppi->GrpHighSadThresh = 32;
    ppi->PrimaryBlockThreshold = 5;
    ppi->SgcThresh = 16;
    ppi->PAKEnabled = 1;
    break;
  }
}

static void ScanCalcPixelIndexTable(PP_INSTANCE *ppi){
  ogg_uint32_t i;
  ogg_uint32_t * PixelIndexTablePtr = ppi->ScanPixelIndexTable;

  /* If appropriate add on extra inices for U and V planes. */
  for ( i = 0; i < (ppi->ScanYPlaneFragments); i++ ) {
    PixelIndexTablePtr[ i ] =
      ((i / ppi->ScanHFragments) *
       VFRAGPIXELS * ppi->ScanConfig.VideoFrameWidth);
    PixelIndexTablePtr[ i ] +=
      ((i % ppi->ScanHFragments) * HFRAGPIXELS);
  }

  PixelIndexTablePtr = &ppi->ScanPixelIndexTable[ppi->ScanYPlaneFragments];

  for ( i = 0; i < (ppi->ScanUVPlaneFragments * 2); i++ ){
    PixelIndexTablePtr[ i ] =
      ((i / (ppi->ScanHFragments >> 1) ) *
       (VFRAGPIXELS * (ppi->ScanConfig.VideoFrameWidth >> 1)) );
    PixelIndexTablePtr[ i ] +=
      ((i % (ppi->ScanHFragments >> 1) ) *
       HFRAGPIXELS) + ppi->YFramePixels;
    }
}

static void InitScanMapArrays(PP_INSTANCE *ppi){
  int i;
  unsigned char StepThresh;

  /* Clear down the fragment level map arrays for the current frame. */
  memset( ppi->FragScores, 0,
          ppi->ScanFrameFragments * sizeof(*ppi->FragScores) );
  memset( ppi->SameGreyDirPixels, 0,
          ppi->ScanFrameFragments );
  memset( ppi->FragDiffPixels, 0,
          ppi->ScanFrameFragments );
  memset( ppi->RowChangedPixels, 0,
          3* ppi->ScanConfig.VideoFrameHeight*sizeof(*ppi->RowChangedPixels));

  memset( ppi->ScanDisplayFragments, BLOCK_NOT_CODED, ppi->ScanFrameFragments);

  /* Threshold used in setting up ppi->NoiseScoreBoostTable[] */
  StepThresh = (unsigned int)(ppi->SRFGreyThresh >> 1);
  if ( StepThresh < MIN_STEP_THRESH )
    StepThresh = MIN_STEP_THRESH;
  ppi->SrfThresh = (int)ppi->SRFGreyThresh;

  /* Set up various tables used to tweak pixel score values and
     scoring rules based upon absolute value of a pixel change */
  for ( i = 0; i < 256; i++ ){
    /* Score multiplier table indexed by absolute difference. */
    ppi->AbsDiff_ScoreMultiplierTable[i] = (double)i * DiffDevisor;
    if ( ppi->AbsDiff_ScoreMultiplierTable[i] < SCORE_MULT_LOW )
      ppi->AbsDiff_ScoreMultiplierTable[i] = SCORE_MULT_LOW;
    else if ( ppi->AbsDiff_ScoreMultiplierTable[i] > SCORE_MULT_HIGH)
      ppi->AbsDiff_ScoreMultiplierTable[i] = SCORE_MULT_HIGH;

    /* Table that facilitates a relaxation of the changed locals rules
       in NoiseScoreRow() for pixels that have changed by a large
       amount. */
    if ( i < (ppi->SrfThresh + StepThresh) )
      ppi->NoiseScoreBoostTable[i] = 0;
    else if ( i < (ppi->SrfThresh + (StepThresh * 4)) )
      ppi->NoiseScoreBoostTable[i] = 1;
    else if ( i < (ppi->SrfThresh + (StepThresh * 6)) )
      ppi->NoiseScoreBoostTable[i] = 2;
    else
      ppi->NoiseScoreBoostTable[i] = 3;

  }

  /* Set various other threshold parameters. */

  /* Set variables that control access to the line search algorithms. */
  ppi->LineSearchTripTresh = 16;
  if ( ppi->LineSearchTripTresh > ppi->PrimaryBlockThreshold )
    ppi->LineSearchTripTresh = (unsigned int)(ppi->PrimaryBlockThreshold + 1);

  /* Adjust line search length if block threshold low */
  ppi->MaxLineSearchLen = MAX_SEARCH_LINE_LEN;
  while ( (ppi->MaxLineSearchLen > 0) &&
          (LineLengthScores[ppi->MaxLineSearchLen-1] >
           ppi->PrimaryBlockThreshold) )
    ppi->MaxLineSearchLen -= 1;

}

void ScanYUVInit( PP_INSTANCE *  ppi, SCAN_CONFIG_DATA * ScanConfigPtr){
  int i;

  /* Set up the various imported data structure pointers. */
  ppi->ScanConfig.Yuv0ptr = ScanConfigPtr->Yuv0ptr;
  ppi->ScanConfig.Yuv1ptr = ScanConfigPtr->Yuv1ptr;
  ppi->ScanConfig.SrfWorkSpcPtr = ScanConfigPtr->SrfWorkSpcPtr;
  ppi->ScanConfig.disp_fragments = ScanConfigPtr->disp_fragments;

  ppi->ScanConfig.RegionIndex = ScanConfigPtr->RegionIndex;

  ppi->ScanConfig.VideoFrameWidth = ScanConfigPtr->VideoFrameWidth;
  ppi->ScanConfig.VideoFrameHeight = ScanConfigPtr->VideoFrameHeight;

  /* UV plane sizes. */
  ppi->VideoUVPlaneWidth = ScanConfigPtr->VideoFrameWidth / 2;
  ppi->VideoUVPlaneHeight = ScanConfigPtr->VideoFrameHeight / 2;

  /* Note the size of each plane in pixels. */
  ppi->YFramePixels = ppi->ScanConfig.VideoFrameWidth *
    ppi->ScanConfig.VideoFrameHeight;
  ppi->UVFramePixels = ppi->VideoUVPlaneWidth * ppi->VideoUVPlaneHeight;

  /* Work out various fragment related values. */
  ppi->ScanYPlaneFragments = ppi->YFramePixels /
    (HFRAGPIXELS * VFRAGPIXELS);
  ppi->ScanUVPlaneFragments = ppi->UVFramePixels /
    (HFRAGPIXELS * VFRAGPIXELS);;
  ppi->ScanHFragments = ppi->ScanConfig.VideoFrameWidth / HFRAGPIXELS;
  ppi->ScanVFragments = ppi->ScanConfig.VideoFrameHeight / VFRAGPIXELS;
  ppi->ScanFrameFragments = ppi->ScanYPlaneFragments +
    (2 * ppi->ScanUVPlaneFragments);

  PInitFrameInfo(ppi);

  /* Set up the scan pixel index table. */
  ScanCalcPixelIndexTable(ppi);

  /* Initialise the previous frame block history lists */
  for ( i = 0; i < MAX_PREV_FRAMES; i++ )
    memset( ppi->PrevFragments[i], BLOCK_NOT_CODED, ppi->ScanFrameFragments);

  /* YUVAnalyseFrame() is not called for the first frame in a sequence
     (a key frame obviously).  This memset insures that for the second
     frame all blocks are marked for coding in line with the behaviour
     for other key frames. */
  memset( ppi->PrevFragments[ppi->PrevFrameLimit-1],
          BLOCK_CODED, ppi->ScanFrameFragments );

  /* Initialise scan arrays */
  InitScanMapArrays(ppi);
}

static void SetFromPrevious(PP_INSTANCE *ppi) {
  unsigned int  i,j;

  /* We buld up the list of previously updated blocks in the zero
     index list of PrevFragments[] so we must start by reseting its
     contents */
  memset( ppi->PrevFragments[0], BLOCK_NOT_CODED, ppi->ScanFrameFragments );

  if ( ppi->PrevFrameLimit > 1 ){
    /* Now build up PrevFragments[0] from PrevFragments[1 to PrevFrameLimit] */
    for ( i = 0; i < ppi->ScanFrameFragments; i++ ){
      for ( j = 1; j < ppi->PrevFrameLimit; j++ ){
        if ( ppi->PrevFragments[j][i] > BLOCK_CODED_BAR ){
          ppi->PrevFragments[0][i] = BLOCK_CODED;
          break;
        }
      }
    }
  }
}

static void UpdatePreviousBlockLists(PP_INSTANCE *ppi) {
  int  i;

  /* Shift previous frame block lists along. */
  for ( i = ppi->PrevFrameLimit; i > 1; i-- ){
    memcpy( ppi->PrevFragments[i], ppi->PrevFragments[i-1],
            ppi->ScanFrameFragments );
  }

  /* Now copy in this frames block list */
  memcpy( ppi->PrevFragments[1], ppi->ScanDisplayFragments,
          ppi->ScanFrameFragments );
}

static void CreateOutputDisplayMap( PP_INSTANCE *ppi,
                                    signed char *InternalFragmentsPtr,
                                    signed char *RecentHistoryPtr,
                                    unsigned char *ExternalFragmentsPtr ) {
  ogg_uint32_t i;
  ogg_uint32_t HistoryBlocksAdded = 0;
  ogg_uint32_t YBand =  (ppi->ScanYPlaneFragments/8);   /* 1/8th of Y image. */

  ppi->OutputBlocksUpdated = 0;
  for ( i = 0; i < ppi->ScanFrameFragments; i++ ) {
    if ( InternalFragmentsPtr[i] > BLOCK_NOT_CODED ) {
      ppi->OutputBlocksUpdated ++;
      ExternalFragmentsPtr[i] = 1;
    }else if ( RecentHistoryPtr[i] == BLOCK_CODED ){
      HistoryBlocksAdded ++;
      ExternalFragmentsPtr[i] = 1;
    }else{
      ExternalFragmentsPtr[i] = 0;
    }
  }

  /* Add in a weighting for the history blocks that have been added */
  ppi->OutputBlocksUpdated += (HistoryBlocksAdded / HISTORY_BLOCK_FACTOR);

  /* Now calculate a key frame candidate indicator.  This is based
     upon Y data only and ignores the top and bottom 1/8 of the
     image.  Also ignore history blocks and BAR blocks. */
  ppi->KFIndicator = 0;
  for ( i = YBand; i < (ppi->ScanYPlaneFragments - YBand); i++ )
    if ( InternalFragmentsPtr[i] > BLOCK_CODED_BAR )
      ppi->KFIndicator ++;

  /* Convert the KF score to a range 0-100 */
  ppi->KFIndicator = ((ppi->KFIndicator*100)/((ppi->ScanYPlaneFragments*3)/4));
}

static int RowSadScan( PP_INSTANCE *ppi,
                       unsigned char * YuvPtr1,
                       unsigned char * YuvPtr2,
                       signed char *  DispFragPtr){
  ogg_int32_t    i, j;
  ogg_uint32_t   GrpSad;
  ogg_uint32_t   LocalGrpLowSadThresh = ppi->ModifiedGrpLowSadThresh;
  ogg_uint32_t   LocalGrpHighSadThresh = ppi->ModifiedGrpHighSadThresh;
  signed char   *LocalDispFragPtr;
  unsigned char *LocalYuvPtr1;
  unsigned char *LocalYuvPtr2;

  int           InterestingBlocksInRow = 0;

  /* For each row of pixels in the row of blocks */
  for ( j = 0; j < VFRAGPIXELS; j++ ){
    /* Set local block map pointer. */
    LocalDispFragPtr = DispFragPtr;

    /* Set the local pixel data pointers for this row.*/
    LocalYuvPtr1 = YuvPtr1;
    LocalYuvPtr2 = YuvPtr2;

    /* Scan along the row of pixels If the block to which a group of
       pixels belongs is already marked for update then do nothing. */
    for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
      if ( *LocalDispFragPtr <= BLOCK_NOT_CODED ){
        /* Calculate the SAD score for the block row */
        GrpSad = dsp_row_sad8(ppi->dsp, LocalYuvPtr1,LocalYuvPtr2);

        /* Now test the group SAD score */
        if ( GrpSad > LocalGrpLowSadThresh ){
          /* If SAD very high we must update else we have candidate block */
          if ( GrpSad > LocalGrpHighSadThresh ){
            /* Force update */
            *LocalDispFragPtr = BLOCK_CODED;
          }else{
            /* Possible Update required */
            *LocalDispFragPtr = CANDIDATE_BLOCK;
          }
          InterestingBlocksInRow = 1;
        }
      }
      LocalDispFragPtr++;

      LocalYuvPtr1 += 8;
      LocalYuvPtr2 += 8;
    }

    /* Increment the base data pointers to the start of the next line. */
    YuvPtr1 += ppi->PlaneStride;
    YuvPtr2 += ppi->PlaneStride;
  }

  return InterestingBlocksInRow;

}

static int ColSadScan( PP_INSTANCE *ppi,
                       unsigned char * YuvPtr1,
                       unsigned char * YuvPtr2,
                       signed char *  DispFragPtr ){
  ogg_int32_t     i;
  ogg_uint32_t    MaxSad;
  ogg_uint32_t    LocalGrpLowSadThresh = ppi->ModifiedGrpLowSadThresh;
  ogg_uint32_t    LocalGrpHighSadThresh = ppi->ModifiedGrpHighSadThresh;
  signed char   * LocalDispFragPtr;

  unsigned char * LocalYuvPtr1;
  unsigned char * LocalYuvPtr2;

  int     InterestingBlocksInRow = 0;

  /* Set the local pixel data pointers for this row. */
  LocalYuvPtr1 = YuvPtr1;
  LocalYuvPtr2 = YuvPtr2;

  /* Set local block map pointer. */
  LocalDispFragPtr = DispFragPtr;

  /* Scan along the row of blocks */
  for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
    /* Skip if block already marked to be coded. */
    if ( *LocalDispFragPtr <= BLOCK_NOT_CODED ){
      /* Calculate the SAD score for the block column */
      MaxSad = dsp_col_sad8x8(ppi->dsp, LocalYuvPtr1, LocalYuvPtr2, ppi->PlaneStride );

      /* Now test the group SAD score */
      if ( MaxSad > LocalGrpLowSadThresh ){
        /* If SAD very high we must update else we have candidate block */
        if ( MaxSad > LocalGrpHighSadThresh ){
          /* Force update */
          *LocalDispFragPtr = BLOCK_CODED;
        }else{
          /* Possible Update required */
          *LocalDispFragPtr = CANDIDATE_BLOCK;
        }
        InterestingBlocksInRow = 1;
      }
    }

    /* Increment the block map pointer. */
    LocalDispFragPtr++;

    /* Step data pointers on ready for next block */
    LocalYuvPtr1 += HFRAGPIXELS;
    LocalYuvPtr2 += HFRAGPIXELS;
  }

  return InterestingBlocksInRow;
}

static void SadPass2( PP_INSTANCE *ppi,
                      ogg_int32_t RowNumber,
                      signed char *  DispFragPtr ){
  ogg_int32_t  i;

  /* First row */
  if ( RowNumber == 0 ) {
    /* First block in row. */
    if ( DispFragPtr[0] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[1] == BLOCK_CODED) ||
           (DispFragPtr[ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[ppi->PlaneHFragments+1] == BLOCK_CODED) ){
        ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[0] = DispFragPtr[0];
      }
    }else{
      ppi->TmpCodedMap[0] = DispFragPtr[0];
    }

    /* All but first and last in row */
    for ( i = 1; (i < ppi->PlaneHFragments-1); i++ ){
      if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
        if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
             (DispFragPtr[i+1] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments+1] == BLOCK_CODED) ){
          ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
        }else{
          ppi->TmpCodedMap[i] = DispFragPtr[i];
        }
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }

    /* Last block in row. */
    i = ppi->PlaneHFragments-1;
    if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
           (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ){
        ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }else{
      ppi->TmpCodedMap[i] = DispFragPtr[i];
    }
  }else if ( RowNumber < (ppi->PlaneVFragments - 1) ){
    /* General case */
    /* First block in row. */
    if ( DispFragPtr[0] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[1] == BLOCK_CODED) ||
           (DispFragPtr[(-ppi->PlaneHFragments)] == BLOCK_CODED) ||
           (DispFragPtr[(-ppi->PlaneHFragments)+1] == BLOCK_CODED) ||
           (DispFragPtr[ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[ppi->PlaneHFragments+1] == BLOCK_CODED) ){
        ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[0] = DispFragPtr[0];
      }
    }else{
      ppi->TmpCodedMap[0] = DispFragPtr[0];
    }

    /* All but first and last in row */
    for ( i = 1; (i < ppi->PlaneHFragments-1); i++ ){
      if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
        if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
             (DispFragPtr[i+1] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments+1] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ||
             (DispFragPtr[i+ppi->PlaneHFragments+1] == BLOCK_CODED) ){
          ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
        }else{
          ppi->TmpCodedMap[i] = DispFragPtr[i];
        }
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }

    /* Last block in row. */
    i = ppi->PlaneHFragments-1;
    if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
           (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
           (DispFragPtr[i+ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[i+ppi->PlaneHFragments-1] == BLOCK_CODED) ){
        ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }else{
      ppi->TmpCodedMap[i] = DispFragPtr[i];
    }
  }else{
    /* Last row */
    /* First block in row. */
    if ( DispFragPtr[0] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[1] == BLOCK_CODED) ||
           (DispFragPtr[(-ppi->PlaneHFragments)] == BLOCK_CODED) ||
           (DispFragPtr[(-ppi->PlaneHFragments)+1] == BLOCK_CODED)){
        ppi->TmpCodedMap[0] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[0] = DispFragPtr[0];
      }
    }else{
      ppi->TmpCodedMap[0] = DispFragPtr[0];
    }

    /* All but first and last in row */
    for ( i = 1; (i < ppi->PlaneHFragments-1); i++ ){
      if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
        if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
             (DispFragPtr[i+1] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ||
             (DispFragPtr[i-ppi->PlaneHFragments+1] == BLOCK_CODED) ){
          ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
        }else{
          ppi->TmpCodedMap[i] = DispFragPtr[i];
        }
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }

    /* Last block in row. */
    i = ppi->PlaneHFragments-1;
    if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
      if ( (DispFragPtr[i-1] == BLOCK_CODED) ||
           (DispFragPtr[i-ppi->PlaneHFragments] == BLOCK_CODED) ||
           (DispFragPtr[i-ppi->PlaneHFragments-1] == BLOCK_CODED) ){
        ppi->TmpCodedMap[i] =  BLOCK_CODED_LOW;
      }else{
        ppi->TmpCodedMap[i] = DispFragPtr[i];
      }
    }else{
      ppi->TmpCodedMap[i] = DispFragPtr[i];
    }
  }

  /* Now copy back the modified Fragment data */
  memcpy( &DispFragPtr[0], &ppi->TmpCodedMap[0], (ppi->PlaneHFragments) );
}

static unsigned char ApplyPakLowPass( PP_INSTANCE *ppi,
                                      unsigned char * SrcPtr ){
  unsigned char * SrcPtr1 = SrcPtr - 1;
  unsigned char * SrcPtr0 = SrcPtr1 - ppi->PlaneStride; /* Note the
                                                           use of
                                                           stride not
                                                           width. */
  unsigned char * SrcPtr2 = SrcPtr1 + ppi->PlaneStride;

  return  (unsigned char)( ( (ogg_uint32_t)SrcPtr0[0] +
              (ogg_uint32_t)SrcPtr0[1] +
              (ogg_uint32_t)SrcPtr0[2] +
              (ogg_uint32_t)SrcPtr1[0] +
              (ogg_uint32_t)SrcPtr1[2] +
              (ogg_uint32_t)SrcPtr2[0] +
              (ogg_uint32_t)SrcPtr2[1] +
              (ogg_uint32_t)SrcPtr2[2]   ) >> 3 );

}

static void RowDiffScan( PP_INSTANCE *ppi,
                         unsigned char * YuvPtr1,
                         unsigned char * YuvPtr2,
                         ogg_int16_t   * YUVDiffsPtr,
                         unsigned char * bits_map_ptr,
                         signed char   * SgcPtr,
                         signed char   * DispFragPtr,
                         unsigned char * FDiffPixels,
                         ogg_int32_t   * RowDiffsPtr,
                         unsigned char * ChLocalsPtr, int EdgeRow ){

  ogg_int32_t    i,j;
  ogg_int32_t    FragChangedPixels;

  ogg_int16_t Diff;     /* Temp local workspace. */

  /* Cannot use kernel if at edge or if PAK disabled */
  if ( (!ppi->PAKEnabled) || EdgeRow ){
    for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
      /* Reset count of pixels changed for the current fragment. */
      FragChangedPixels = 0;

      /* Test for break out conditions to save time. */
      if (*DispFragPtr == CANDIDATE_BLOCK){

        /* Clear down entries in changed locals array */
        SET8_0(ChLocalsPtr);

        for ( j = 0; j < HFRAGPIXELS; j++ ){
          /* Take a local copy of the measured difference. */
          Diff = (int)YuvPtr1[j] - (int)YuvPtr2[j];

          /* Store the actual difference value */
          YUVDiffsPtr[j] = Diff;

          /* Test against the Level thresholds and record the results */
          SgcPtr[0] += ppi->SgcThreshTable[Diff+255];

          /* Test against the SRF thresholds */
          bits_map_ptr[j] = ppi->SrfThreshTable[Diff+255];
          FragChangedPixels += ppi->SrfThreshTable[Diff+255];
        }
      }else{
        /* If we are breaking out here mark all pixels as changed. */
        if ( *DispFragPtr > BLOCK_NOT_CODED ){
          SET8_1(bits_map_ptr);
          SET8_8(ChLocalsPtr);
        }else{
          SET8_0(ChLocalsPtr);
        }
      }

      *RowDiffsPtr += FragChangedPixels;
      *FDiffPixels += (unsigned char)FragChangedPixels;

      YuvPtr1 += HFRAGPIXELS;
      YuvPtr2 += HFRAGPIXELS;
      bits_map_ptr += HFRAGPIXELS;
      ChLocalsPtr += HFRAGPIXELS;
      YUVDiffsPtr += HFRAGPIXELS;
      SgcPtr ++;
      FDiffPixels ++;

      /* If we have a lot of changed pixels for this fragment on this
         row then the fragment is almost sure to be picked (e.g. through
         the line search) so we can mark it as selected and then ignore
         it. */
      if (FragChangedPixels >= 7){
        *DispFragPtr = BLOCK_CODED_LOW;
      }
      DispFragPtr++;
    }
  }else{

    /*************************************************************/
    /* First fragment of row !! */

    i = 0;
    /* Reset count of pixels changed for the current fragment. */
    FragChangedPixels = 0;

    /* Test for break out conditions to save time. */
    if (*DispFragPtr == CANDIDATE_BLOCK){
      /* Clear down entries in changed locals array */
      SET8_0(ChLocalsPtr);

      for ( j = 0; j < HFRAGPIXELS; j++ ){
        /* Take a local copy of the measured difference. */
        Diff = (int)YuvPtr1[j] - (int)YuvPtr2[j];

        /* Store the actual difference value */
        YUVDiffsPtr[j] = Diff;

        /* Test against the Level thresholds and record the results */
        SgcPtr[0] += ppi->SgcThreshTable[Diff+255];

        if (j>0 && ppi->SrfPakThreshTable[Diff+255] )
          Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) -
            (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );

        /* Test against the SRF thresholds */
        bits_map_ptr[j] = ppi->SrfThreshTable[Diff+255];
        FragChangedPixels += ppi->SrfThreshTable[Diff+255];
      }
    }else{
      /* If we are breaking out here mark all pixels as changed. */
      if ( *DispFragPtr > BLOCK_NOT_CODED ){
        SET8_1(bits_map_ptr);
        SET8_8(ChLocalsPtr);
      }else{
        SET8_0(ChLocalsPtr);
      }
    }

    *RowDiffsPtr += FragChangedPixels;
    *FDiffPixels += (unsigned char)FragChangedPixels;

    YuvPtr1 += HFRAGPIXELS;
    YuvPtr2 += HFRAGPIXELS;
    bits_map_ptr += HFRAGPIXELS;
    ChLocalsPtr += HFRAGPIXELS;
    YUVDiffsPtr += HFRAGPIXELS;
    SgcPtr ++;
    FDiffPixels ++;

    /* If we have a lot of changed pixels for this fragment on this
       row then the fragment is almost sure to be picked
       (e.g. through the line search) so we can mark it as selected
       and then ignore it. */
    if (FragChangedPixels >= 7){
      *DispFragPtr = BLOCK_CODED_LOW;
    }
    DispFragPtr++;
    /*************************************************************/
    /* Fragment in between!! */

    for ( i = HFRAGPIXELS ; i < ppi->PlaneWidth-HFRAGPIXELS;
          i += HFRAGPIXELS ){
      /* Reset count of pixels changed for the current fragment. */
      FragChangedPixels = 0;

      /* Test for break out conditions to save time. */
      if (*DispFragPtr == CANDIDATE_BLOCK){
        /* Clear down entries in changed locals array */
        SET8_0(ChLocalsPtr);
        for ( j = 0; j < HFRAGPIXELS; j++ ){
          /* Take a local copy of the measured difference. */
          Diff = (int)YuvPtr1[j] - (int)YuvPtr2[j];

          /* Store the actual difference value */
          YUVDiffsPtr[j] = Diff;

          /* Test against the Level thresholds and record the results */
          SgcPtr[0] += ppi->SgcThreshTable[Diff+255];

          if (ppi->SrfPakThreshTable[Diff+255] )
            Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) -
              (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );


          /* Test against the SRF thresholds */
          bits_map_ptr[j] = ppi->SrfThreshTable[Diff+255];
          FragChangedPixels += ppi->SrfThreshTable[Diff+255];
        }
      }else{
        /* If we are breaking out here mark all pixels as changed. */
        if ( *DispFragPtr > BLOCK_NOT_CODED ){
          SET8_1(bits_map_ptr);
          SET8_8(ChLocalsPtr);
        }else{
          SET8_0(ChLocalsPtr);
        }
      }

      *RowDiffsPtr += FragChangedPixels;
      *FDiffPixels += (unsigned char)FragChangedPixels;

      YuvPtr1 += HFRAGPIXELS;
      YuvPtr2 += HFRAGPIXELS;
      bits_map_ptr += HFRAGPIXELS;
      ChLocalsPtr += HFRAGPIXELS;
      YUVDiffsPtr += HFRAGPIXELS;
      SgcPtr ++;
      FDiffPixels ++;

      /* If we have a lot of changed pixels for this fragment on this
         row then the fragment is almost sure to be picked
         (e.g. through the line search) so we can mark it as selected
         and then ignore it. */
      if (FragChangedPixels >= 7){
        *DispFragPtr = BLOCK_CODED_LOW;
      }
      DispFragPtr++;
    }
    /*************************************************************/
    /* Last fragment of row !! */

    /* Reset count of pixels changed for the current fragment. */
    FragChangedPixels = 0;

    /* Test for break out conditions to save time. */
    if (*DispFragPtr == CANDIDATE_BLOCK){
      /* Clear down entries in changed locals array */
      SET8_0(ChLocalsPtr);

      for ( j = 0; j < HFRAGPIXELS; j++ ){
        /* Take a local copy of the measured difference. */
        Diff = (int)YuvPtr1[j] - (int)YuvPtr2[j];

        /* Store the actual difference value */
        YUVDiffsPtr[j] = Diff;

        /* Test against the Level thresholds and record the results */
        SgcPtr[0] += ppi->SgcThreshTable[Diff+255];

        if (j<7 && ppi->SrfPakThreshTable[Diff+255] )
          Diff = (int)ApplyPakLowPass( ppi, &YuvPtr1[j] ) -
            (int)ApplyPakLowPass( ppi, &YuvPtr2[j] );


        /* Test against the SRF thresholds */
        bits_map_ptr[j] = ppi->SrfThreshTable[Diff+255];
        FragChangedPixels += ppi->SrfThreshTable[Diff+255];
      }
    }else{
      /* If we are breaking out here mark all pixels as changed.*/
      if ( *DispFragPtr > BLOCK_NOT_CODED ) {
          SET8_1(bits_map_ptr);
          SET8_8(ChLocalsPtr);
        }else{
          SET8_0(ChLocalsPtr);
        }
    }
    /* If we have a lot of changed pixels for this fragment on this
       row then the fragment is almost sure to be picked (e.g. through
       the line search) so we can mark it as selected and then ignore
       it. */
    *RowDiffsPtr += FragChangedPixels;
    *FDiffPixels += (unsigned char)FragChangedPixels;

    /* If we have a lot of changed pixels for this fragment on this
       row then the fragment is almost sure to be picked (e.g. through
       the line search) so we can mark it as selected and then ignore
       it. */
    if (FragChangedPixels >= 7){
      *DispFragPtr = BLOCK_CODED_LOW;
    }
    DispFragPtr++;

  }
}

static void ConsolidateDiffScanResults( PP_INSTANCE *ppi,
                                        unsigned char * FDiffPixels,
                                        signed char * SgcScoresPtr,
                                        signed char * DispFragPtr ){
  ogg_int32_t i;

  for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
    /* Consider only those blocks that were candidates in the
       difference scan. Ignore definite YES and NO cases. */
    if ( DispFragPtr[i] == CANDIDATE_BLOCK ){
      if ( ((ogg_uint32_t)abs(SgcScoresPtr[i]) > ppi->BlockSgcThresh) ){
        /* Block marked for update due to Sgc change */
        DispFragPtr[i] = BLOCK_CODED_SGC;
      }else if ( FDiffPixels[i] == 0 ){
        /* Block is no longer a candidate for the main tests but will
           still be considered a candidate in RowBarEnhBlockMap() */
        DispFragPtr[i] = CANDIDATE_BLOCK_LOW;
      }
    }
  }
}

static void RowChangedLocalsScan( PP_INSTANCE *ppi,
                                  unsigned char * PixelMapPtr,
                                  unsigned char * ChLocalsPtr,
                                  signed char  * DispFragPtr,
                                  unsigned char   RowType ){

  unsigned char changed_locals = 0;
  unsigned char * PixelsChangedPtr0;
  unsigned char * PixelsChangedPtr1;
  unsigned char * PixelsChangedPtr2;
  ogg_int32_t i, j;
  ogg_int32_t LastRowIndex = ppi->PlaneWidth - 1;

  /* Set up the line based pointers into the bits changed map. */
  PixelsChangedPtr0 = PixelMapPtr - ppi->PlaneWidth;
  if ( PixelsChangedPtr0 < ppi->PixelChangedMap )
    PixelsChangedPtr0 += ppi->PixelMapCircularBufferSize;
  PixelsChangedPtr0 -= 1;

  PixelsChangedPtr1 = PixelMapPtr - 1;

  PixelsChangedPtr2 = PixelMapPtr + ppi->PlaneWidth;
  if ( PixelsChangedPtr2 >=
       (ppi->PixelChangedMap + ppi->PixelMapCircularBufferSize) )
    PixelsChangedPtr2 -= ppi->PixelMapCircularBufferSize;
  PixelsChangedPtr2 -= 1;

  if ( RowType == NOT_EDGE_ROW ){
    /* Scan through the row of pixels and calculate changed locals. */
    for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
      /* Skip a group of 8 pixels if the assosciated fragment has no
         pixels of interest. */
      if ( *DispFragPtr == CANDIDATE_BLOCK ){
        for ( j = 0; j < HFRAGPIXELS; j++ ){
          changed_locals = 0;

          /* If the pixel itself has changed */
          if ( PixelsChangedPtr1[1] ){
            if ( (i > 0) || (j > 0) ){
              changed_locals += PixelsChangedPtr0[0];
              changed_locals += PixelsChangedPtr1[0];
              changed_locals += PixelsChangedPtr2[0];
            }

            changed_locals += PixelsChangedPtr0[1];
            changed_locals += PixelsChangedPtr2[1];

            if ( (i + j) < LastRowIndex ){
              changed_locals += PixelsChangedPtr0[2];
              changed_locals += PixelsChangedPtr1[2];
              changed_locals += PixelsChangedPtr2[2];
            }

            /* Store the number of changed locals */
            *ChLocalsPtr |= changed_locals;
          }

          /* Increment to next pixel in the row */
          ChLocalsPtr++;
          PixelsChangedPtr0++;
          PixelsChangedPtr1++;
          PixelsChangedPtr2++;
        }
      }else{
        if ( *DispFragPtr > BLOCK_NOT_CODED )
          SET8_0(ChLocalsPtr);

        /* Step pointers */
        ChLocalsPtr += HFRAGPIXELS;
        PixelsChangedPtr0 += HFRAGPIXELS;
        PixelsChangedPtr1 += HFRAGPIXELS;
        PixelsChangedPtr2 += HFRAGPIXELS;
      }

      /* Move on to next fragment. */
      DispFragPtr++;

    }
  }else{
    /* Scan through the row of pixels and calculate changed locals. */
    for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
      /* Skip a group of 8 pixels if the assosciated fragment has no
         pixels of interest */
      if ( *DispFragPtr == CANDIDATE_BLOCK ){
        for ( j = 0; j < HFRAGPIXELS; j++ ){
          changed_locals = 0;

          /* If the pixel itself has changed */
          if ( PixelsChangedPtr1[1] ){
            if ( RowType == FIRST_ROW ){
              if ( (i > 0) || (j > 0) ){
                changed_locals += PixelsChangedPtr1[0];
                changed_locals += PixelsChangedPtr2[0];
              }

              changed_locals += PixelsChangedPtr2[1];

              if ( (i + j) < LastRowIndex ){
                changed_locals += PixelsChangedPtr1[2];
                changed_locals += PixelsChangedPtr2[2];
              }
            }else{
              if ( (i > 0) || (j > 0 ) ){
                changed_locals += PixelsChangedPtr0[0];
                changed_locals += PixelsChangedPtr1[0];
              }

              changed_locals += PixelsChangedPtr0[1];

              if ( (i + j) < LastRowIndex ){
                changed_locals += PixelsChangedPtr0[2];
                changed_locals += PixelsChangedPtr1[2];
              }
            }

            /* Store the number of changed locals */
            *ChLocalsPtr |= changed_locals;
          }

          /* Increment to next pixel in the row */
          ChLocalsPtr++;
          PixelsChangedPtr0++;
          PixelsChangedPtr1++;
          PixelsChangedPtr2++;
        }
      }else{
        if ( *DispFragPtr > BLOCK_NOT_CODED )
          SET8_0(ChLocalsPtr);

        /* Step pointers */
        ChLocalsPtr += HFRAGPIXELS;
        PixelsChangedPtr0 += HFRAGPIXELS;
        PixelsChangedPtr1 += HFRAGPIXELS;
        PixelsChangedPtr2 += HFRAGPIXELS;
      }

      /* Move on to next fragment. */
      DispFragPtr++;
    }
  }
}

static void NoiseScoreRow( PP_INSTANCE *ppi,
                           unsigned char * PixelMapPtr,
                           unsigned char * ChLocalsPtr,
                           ogg_int16_t   * YUVDiffsPtr,
                           unsigned char * PixelNoiseScorePtr,
                           ogg_uint32_t  * FragScorePtr,
                           signed char   * DispFragPtr,
                           ogg_int32_t   * RowDiffsPtr ){
  ogg_int32_t i,j;
  unsigned char  changed_locals = 0;
  ogg_int32_t  Score;
  ogg_uint32_t FragScore;
  ogg_int32_t  AbsDiff;

  /* For each pixel in the row */
  for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
    /* Skip a group of 8 pixels if the assosciated fragment has no
       pixels of interest. */
    if ( *DispFragPtr == CANDIDATE_BLOCK ){
      /* Reset the cumulative fragment score. */
      FragScore = 0;

      /* Pixels grouped along the row into fragments */
      for ( j = 0; j < HFRAGPIXELS; j++ ){
        if ( PixelMapPtr[j] ){
          AbsDiff = (ogg_int32_t)( abs(YUVDiffsPtr[j]) );
          changed_locals = ChLocalsPtr[j];

          /* Give this pixel a score based on changed locals and level
             of its own change. */
          Score = (1 + ((ogg_int32_t)(changed_locals +
                                      ppi->NoiseScoreBoostTable[AbsDiff]) -
                        ppi->NoiseSupLevel));

          /* For no zero scores adjust by a level based score multiplier. */
          if ( Score > 0 ){
            Score = ((double)Score *
                     ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
            if ( Score < 1 )
              Score = 1;
          }else{
            /* Set -ve values to 0 */
            Score = 0;

            /* If there are no changed locals then clear the pixel
               changed flag and decrement the pixels changed in
               fragment count to speed later stages. */
            if ( changed_locals == 0 ){
              PixelMapPtr[j] = 0;
              *RowDiffsPtr -= 1;
            }
          }

          /* Update the pixel scores etc. */
          PixelNoiseScorePtr[j] = (unsigned char)Score;
          FragScore += (ogg_uint32_t)Score;
        }
      }

      /* Add fragment score (with plane correction factor) into main
         data structure */
      *FragScorePtr += (ogg_int32_t)(FragScore *
                                     ppi->YUVPlaneCorrectionFactor);

      /* If score is greater than trip threshold then mark blcok for update. */
      if ( *FragScorePtr > ppi->BlockThreshold ){
        *DispFragPtr = BLOCK_CODED_LOW;
      }
    }

    /* Increment the various pointers */
    FragScorePtr++;
    DispFragPtr++;
    PixelNoiseScorePtr += HFRAGPIXELS;
    PixelMapPtr += HFRAGPIXELS;
    ChLocalsPtr += HFRAGPIXELS;
    YUVDiffsPtr += HFRAGPIXELS;
  }
}

static void PrimaryEdgeScoreRow( PP_INSTANCE *ppi,
                                 unsigned char * ChangedLocalsPtr,
                                 ogg_int16_t   * YUVDiffsPtr,
                                 unsigned char * PixelNoiseScorePtr,
                                 ogg_uint32_t  * FragScorePtr,
                                 signed char   * DispFragPtr,
                                 unsigned char   RowType ){
  ogg_uint32_t     BodyNeighbours;
  ogg_uint32_t     AbsDiff;
  unsigned char    changed_locals = 0;
  ogg_int32_t      Score;
  ogg_uint32_t     FragScore;
  unsigned char  * CHLocalsPtr0;
  unsigned char  * CHLocalsPtr1;
  unsigned char  * CHLocalsPtr2;
  ogg_int32_t      i,j;
  ogg_int32_t      LastRowIndex = ppi->PlaneWidth - 1;

  /* Set up pointers into the current previous and next row of the
     changed locals data structure. */
  CHLocalsPtr0 = ChangedLocalsPtr - ppi->PlaneWidth;
  if ( CHLocalsPtr0 < ppi->ChLocals )
    CHLocalsPtr0 += ppi->ChLocalsCircularBufferSize;
  CHLocalsPtr0 -= 1;

  CHLocalsPtr1 = ChangedLocalsPtr - 1;

  CHLocalsPtr2 = ChangedLocalsPtr + ppi->PlaneWidth;
  if ( CHLocalsPtr2 >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
    CHLocalsPtr2 -= ppi->ChLocalsCircularBufferSize;
  CHLocalsPtr2 -= 1;


  /* The defining rule used here is as follows. */
  /* An edge pixels has 3-5 changed locals. */
  /* And one or more of these changed locals has itself got 7-8
     changed locals. */

  if ( RowType == NOT_EDGE_ROW ){
    /* Loop for all pixels in the row. */
    for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
      /* Does the fragment contain anything interesting to work with. */
      if ( *DispFragPtr == CANDIDATE_BLOCK ){
        /* Reset the cumulative fragment score. */
        FragScore = 0;

        /* Pixels grouped along the row into fragments */
        for ( j = 0; j < HFRAGPIXELS; j++ ){
          /* How many changed locals has the current pixel got. */
          changed_locals = ChangedLocalsPtr[j];

          /* Is the pixel a suitable candidate */
          if ( (changed_locals > 2) && (changed_locals < 6) ){
            /* The pixel may qualify... have a closer look.  */
            BodyNeighbours = 0;

            /* Count the number of "BodyNeighbours" .. Pixels that
               have 7 or more changed neighbours.  */
            if ( (i > 0) || (j > 0 ) ){
              if ( CHLocalsPtr0[0] >= 7 )
                BodyNeighbours++;
              if ( CHLocalsPtr1[0] >= 7 )
                BodyNeighbours++;
              if ( CHLocalsPtr2[0] >= 7 )
                BodyNeighbours++;
            }

            if ( CHLocalsPtr0[1] >= 7 )
              BodyNeighbours++;
            if ( CHLocalsPtr2[1] >= 7 )
              BodyNeighbours++;

            if ( (i + j) < LastRowIndex ){
              if ( CHLocalsPtr0[2] >= 7 )
                BodyNeighbours++;
              if ( CHLocalsPtr1[2] >= 7 )
                BodyNeighbours++;
              if ( CHLocalsPtr2[2] >= 7 )
                BodyNeighbours++;
            }

            if ( BodyNeighbours > 0 ){
              AbsDiff = abs( YUVDiffsPtr[j] );
              Score = (ogg_int32_t)
                ( (double)(BodyNeighbours *
                           BodyNeighbourScore) *
                  ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
              if ( Score < 1 )
                Score = 1;

              /* Increment the score by a value determined by the
                 number of body neighbours. */
              PixelNoiseScorePtr[j] += (unsigned char)Score;
              FragScore += (ogg_uint32_t)Score;
            }
          }

          /* Increment pointers into changed locals buffer */
          CHLocalsPtr0 ++;
          CHLocalsPtr1 ++;
          CHLocalsPtr2 ++;
        }

        /* Add fragment score (with plane correction factor) into main
           data structure */
        *FragScorePtr += (ogg_int32_t)(FragScore *
                                       ppi->YUVPlaneCorrectionFactor);

        /* If score is greater than trip threshold then mark blcok for
           update. */
        if ( *FragScorePtr > ppi->BlockThreshold ){
          *DispFragPtr = BLOCK_CODED_LOW;
        }

      }else{
        /* Nothing to do for this fragment group */
        /* Advance pointers into changed locals buffer */
        CHLocalsPtr0 += HFRAGPIXELS;
        CHLocalsPtr1 += HFRAGPIXELS;
        CHLocalsPtr2 += HFRAGPIXELS;
      }

      /* Increment the various pointers */
      FragScorePtr++;
      DispFragPtr++;
      PixelNoiseScorePtr += HFRAGPIXELS;
      ChangedLocalsPtr += HFRAGPIXELS;
      YUVDiffsPtr += HFRAGPIXELS;
    }
  }else{
    /* This is either the top or bottom row of pixels in a plane. */
    /* Loop for all pixels in the row. */
    for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
      /* Does the fragment contain anything interesting to work with. */
      if ( *DispFragPtr == CANDIDATE_BLOCK ){
        /* Reset the cumulative fragment score. */
        FragScore = 0;

        /* Pixels grouped along the row into fragments */
        for ( j = 0; j < HFRAGPIXELS; j++ ){
          /* How many changed locals has the current pixel got. */
          changed_locals = ChangedLocalsPtr[j];

          /* Is the pixel a suitable candidate */
          if ( (changed_locals > 2) && (changed_locals < 6) ){
            /* The pixel may qualify... have a closer look. */
            BodyNeighbours = 0;

            /* Count the number of "BodyNeighbours" .. Pixels
               that have 7 or more changed neighbours. */
            if ( RowType == LAST_ROW ){
              /* Test for cases where it could be the first pixel on
                 the line */
              if ( (i > 0) || (j > 0) ){
                if ( CHLocalsPtr0[0] >= 7 )
                  BodyNeighbours++;
                if ( CHLocalsPtr1[0] >= 7 )
                  BodyNeighbours++;
              }

              if ( CHLocalsPtr0[1] >= 7 )
                BodyNeighbours++;

              /* Test for the end of line case */
              if ( (i + j) < LastRowIndex ){
                if ( CHLocalsPtr0[2] >= 7 )
                  BodyNeighbours++;

                if ( CHLocalsPtr1[2] >= 7 )
                  BodyNeighbours++;
              }
            }else{
              /* First Row */
              /* Test for cases where it could be the first pixel on
                 the line */
              if ( (i > 0) || (j > 0) ){
                if ( CHLocalsPtr1[0] >= 7 )
                  BodyNeighbours++;
                if ( CHLocalsPtr2[0] >= 7 )
                  BodyNeighbours++;
              }

              /* Test for the end of line case */
              if ( CHLocalsPtr2[1] >= 7 )
                BodyNeighbours++;

              if ( (i + j) < LastRowIndex ){
                if ( CHLocalsPtr1[2] >= 7 )
                  BodyNeighbours++;
                if ( CHLocalsPtr2[2] >= 7 )
                  BodyNeighbours++;
              }
            }

            /* Allocate a score according to the number of Body neighbours. */
            if ( BodyNeighbours > 0 ){
              AbsDiff = abs( YUVDiffsPtr[j] );
              Score = (ogg_int32_t)
                ( (double)(BodyNeighbours * BodyNeighbourScore) *
                  ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
              if ( Score < 1 )
                Score = 1;

              PixelNoiseScorePtr[j] += (unsigned char)Score;
              FragScore += (ogg_uint32_t)Score;
            }
          }

          /* Increment pointers into changed locals buffer */
          CHLocalsPtr0 ++;
          CHLocalsPtr1 ++;
          CHLocalsPtr2 ++;
        }

        /* Add fragment score (with plane correction factor) into main
           data structure */
        *FragScorePtr +=
          (ogg_int32_t)(FragScore * ppi->YUVPlaneCorrectionFactor);

        /* If score is greater than trip threshold then mark blcok for
           update. */
        if ( *FragScorePtr > ppi->BlockThreshold ){
          *DispFragPtr = BLOCK_CODED_LOW;
        }

      }else{
        /* Nothing to do for this fragment group */
        /* Advance pointers into changed locals buffer */
        CHLocalsPtr0 += HFRAGPIXELS;
        CHLocalsPtr1 += HFRAGPIXELS;
        CHLocalsPtr2 += HFRAGPIXELS;
      }

      /* Increment the various pointers */
      FragScorePtr++;
      DispFragPtr++;
      PixelNoiseScorePtr += HFRAGPIXELS;
      ChangedLocalsPtr += HFRAGPIXELS;
      YUVDiffsPtr += HFRAGPIXELS;
    }
  }
}

static void PixelLineSearch( PP_INSTANCE *ppi,
                             unsigned char * ChangedLocalsPtr,
                             ogg_int32_t RowNumber,
                             ogg_int32_t ColNumber,
                             unsigned char direction,
                             ogg_uint32_t * line_length ){
  /* Exit if the pixel does not qualify or we have fallen off the edge
     of either the image plane or the row. */
  if ( (RowNumber < 0) ||
       (RowNumber >= ppi->PlaneHeight) ||
       (ColNumber < 0) ||
       (ColNumber >= ppi->PlaneWidth) ||
       ((*ChangedLocalsPtr) <= 1) ||
       ((*ChangedLocalsPtr) >= 6) ){
    /* If not then it isn't part of any line. */
    return;
  }

  if (*line_length < ppi->MaxLineSearchLen){
    ogg_uint32_t TmpLineLength;
    ogg_uint32_t BestLineLength;
    unsigned char * search_ptr;

    /* Increment the line length to include this pixel. */
    *line_length += 1;
    BestLineLength = *line_length;

    /* Continue search  */
    /* up */
    if ( direction == UP ){
      TmpLineLength = *line_length;

      search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
      if ( search_ptr < ppi->ChLocals )
        search_ptr += ppi->ChLocalsCircularBufferSize;

      PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber,
                       direction, &TmpLineLength );

      if ( TmpLineLength > BestLineLength )
        BestLineLength = TmpLineLength;
    }

    /* up and left */
    if ( (BestLineLength < ppi->MaxLineSearchLen) &&
         ((direction == UP) || (direction == LEFT)) ){
      TmpLineLength = *line_length;

      search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
      if ( search_ptr < ppi->ChLocals )
        search_ptr += ppi->ChLocalsCircularBufferSize;
      search_ptr -= 1;

      PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber - 1,
                       direction,  &TmpLineLength );

      if ( TmpLineLength > BestLineLength )
        BestLineLength = TmpLineLength;
    }

    /* up and right */
    if ( (BestLineLength < ppi->MaxLineSearchLen) &&
         ((direction == UP) || (direction == RIGHT)) ){
      TmpLineLength = *line_length;

      search_ptr = ChangedLocalsPtr - ppi->PlaneWidth;
      if ( search_ptr < ppi->ChLocals )
        search_ptr += ppi->ChLocalsCircularBufferSize;
      search_ptr += 1;

      PixelLineSearch( ppi, search_ptr, RowNumber - 1, ColNumber + 1,
                       direction, &TmpLineLength );

      if ( TmpLineLength > BestLineLength )
        BestLineLength = TmpLineLength;
    }

    /* left */
    if ( (BestLineLength < ppi->MaxLineSearchLen) && ( direction == LEFT ) ){
      TmpLineLength = *line_length;
      PixelLineSearch( ppi, ChangedLocalsPtr - 1, RowNumber, ColNumber - 1,
                       direction, &TmpLineLength );

      if ( TmpLineLength > BestLineLength )
        BestLineLength = TmpLineLength;
    }

    /* right */
    if ( (BestLineLength < ppi->MaxLineSearchLen) && ( direction == RIGHT ) ){
      TmpLineLength = *line_length;
      PixelLineSearch( ppi, ChangedLocalsPtr + 1, RowNumber, ColNumber + 1,
                       direction, &TmpLineLength );

      if ( TmpLineLength > BestLineLength )
        BestLineLength = TmpLineLength;
    }

    /* Down */
    if ( BestLineLength < ppi->MaxLineSearchLen ){
      TmpLineLength = *line_length;
      if ( direction == DOWN ){
        search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
        if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
          search_ptr -= ppi->ChLocalsCircularBufferSize;

        PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber, direction,
                         &TmpLineLength );

        if ( TmpLineLength > BestLineLength )
          BestLineLength = TmpLineLength;
      }


      /* down and left */
      if ( (BestLineLength < ppi->MaxLineSearchLen) &&
           ((direction == DOWN) || (direction == LEFT)) ){
        TmpLineLength = *line_length;

        search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
        if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
          search_ptr -= ppi->ChLocalsCircularBufferSize;
        search_ptr -= 1;

        PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber - 1,
                         direction, &TmpLineLength );

        if ( TmpLineLength > BestLineLength )
          BestLineLength = TmpLineLength;
      }

      /* down and right */
      if ( (BestLineLength < ppi->MaxLineSearchLen) &&
           ((direction == DOWN) || (direction == RIGHT)) ){
        TmpLineLength = *line_length;

        search_ptr = ChangedLocalsPtr + ppi->PlaneWidth;
        if ( search_ptr >= (ppi->ChLocals + ppi->ChLocalsCircularBufferSize) )
          search_ptr -= ppi->ChLocalsCircularBufferSize;
        search_ptr += 1;

        PixelLineSearch( ppi, search_ptr, RowNumber + 1, ColNumber + 1,
                         direction, &TmpLineLength );

        if ( TmpLineLength > BestLineLength )
          BestLineLength = TmpLineLength;
      }
    }

    /* Note the search value for this pixel. */
    *line_length = BestLineLength;
  }
}

static unsigned char LineSearchScorePixel( PP_INSTANCE *ppi,
                                           unsigned char * ChangedLocalsPtr,
                                           ogg_int32_t RowNumber,
                                           ogg_int32_t ColNumber ){
    ogg_uint32_t line_length = 0;
    ogg_uint32_t line_length2 = 0;
    ogg_uint32_t line_length_score = 0;
    ogg_uint32_t tmp_line_length = 0;
    ogg_uint32_t tmp_line_length2 = 0;

    /* Look UP and Down */
    PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber,
                     ColNumber, UP, &tmp_line_length );

    if (tmp_line_length < ppi->MaxLineSearchLen) {
      /* Look DOWN */
      PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber,
                       ColNumber, DOWN, &tmp_line_length2 );
      line_length = tmp_line_length + tmp_line_length2 - 1;

      if ( line_length > ppi->MaxLineSearchLen )
        line_length = ppi->MaxLineSearchLen;
    }else
      line_length = tmp_line_length;

    /* If no max length line found then look left and right */
    if ( line_length < ppi->MaxLineSearchLen ){
      tmp_line_length = 0;
      tmp_line_length2 = 0;

      PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber,
                       ColNumber, LEFT,  &tmp_line_length );
      if (tmp_line_length < ppi->MaxLineSearchLen){
        PixelLineSearch( ppi, ChangedLocalsPtr, RowNumber,
                         ColNumber, RIGHT,  &tmp_line_length2 );
        line_length2 = tmp_line_length + tmp_line_length2 - 1;

        if ( line_length2 > ppi->MaxLineSearchLen )
          line_length2 = ppi->MaxLineSearchLen;
      }else
        line_length2 = tmp_line_length;

    }

    /* Take the largest line length */
    if ( line_length2 > line_length )
      line_length = line_length2;

    /* Create line length score */
    line_length_score = LineLengthScores[line_length];

    return (unsigned char)line_length_score;
}

static void LineSearchScoreRow( PP_INSTANCE *ppi,
                                unsigned char * ChangedLocalsPtr,
                                ogg_int16_t   * YUVDiffsPtr,
                                unsigned char * PixelNoiseScorePtr,
                                ogg_uint32_t  * FragScorePtr,
                                signed char   * DispFragPtr,
                                ogg_int32_t     RowNumber ){
  ogg_uint32_t AbsDiff;
  unsigned char  changed_locals = 0;
  ogg_int32_t  Score;
  ogg_uint32_t FragScore;
  ogg_int32_t  i,j;

  /* The defining rule used here is as follows. */
  /* An edge pixels has 2-5 changed locals. */
  /* And one or more of these changed locals has itself got 7-8
     changed locals. */

  /* Loop for all pixels in the row. */
  for ( i = 0; i < ppi->PlaneWidth; i += HFRAGPIXELS ){
    /* Does the fragment contain anything interesting to work with. */
    if ( *DispFragPtr == CANDIDATE_BLOCK ){
      /* Reset the cumulative fragment score. */
      FragScore = 0;

      /* Pixels grouped along the row into fragments */
      for ( j = 0; j < HFRAGPIXELS; j++ ){
        /* How many changed locals has the current pixel got. */
        changed_locals = ChangedLocalsPtr[j];

        /* Is the pixel a suitable candidate for edge enhancement */
        if ( (changed_locals > 1) && (changed_locals < 6) &&
             (PixelNoiseScorePtr[j] < ppi->LineSearchTripTresh) ) {
          Score = (ogg_int32_t)
            LineSearchScorePixel( ppi, &ChangedLocalsPtr[j], RowNumber, i+j );

          if ( Score ){
            AbsDiff = abs( YUVDiffsPtr[j] );
            Score = (ogg_int32_t)
              ( (double)Score * ppi->AbsDiff_ScoreMultiplierTable[AbsDiff] );
            if ( Score < 1 )
              Score = 1;

            PixelNoiseScorePtr[j] += (unsigned char)Score;
            FragScore += (ogg_uint32_t)Score;
          }
        }
      }

      /* Add fragment score (with plane correction factor) into main
         data structure */
      *FragScorePtr +=
        (ogg_int32_t)(FragScore * ppi->YUVPlaneCorrectionFactor);

      /* If score is greater than trip threshold then mark blcok for update. */
      if ( *FragScorePtr > ppi->BlockThreshold ){
        *DispFragPtr = BLOCK_CODED_LOW;
      }
    }

    /* Increment the various pointers */
    FragScorePtr++;
    DispFragPtr++;
    PixelNoiseScorePtr += HFRAGPIXELS;
    ChangedLocalsPtr += HFRAGPIXELS;
    YUVDiffsPtr += HFRAGPIXELS;

  }
}

static void RowCopy( PP_INSTANCE *ppi, ogg_uint32_t BlockMapIndex ){

  ogg_uint32_t   i,j;

  ogg_uint32_t   PixelIndex = ppi->ScanPixelIndexTable[BlockMapIndex];
  signed char   * BlockMapPtr = &ppi->ScanDisplayFragments[BlockMapIndex];
  signed char   * PrevFragmentsPtr = &ppi->PrevFragments[0][BlockMapIndex];

  unsigned char  * SourcePtr;
  unsigned char  * DestPtr;

  /* Copy pixels from changed blocks back to reference frame. */
  for ( i = 0; i < (ogg_uint32_t)ppi->PlaneHFragments; i ++ ){
    /* If the fragement is marked for update or was recently marked
       for update (PrevFragmentsPtr[i]) */
    if ( (BlockMapPtr[i] > BLOCK_NOT_CODED) ||
         (PrevFragmentsPtr[i] == BLOCK_CODED) ){
      /* Set up the various pointers required. */
      SourcePtr = &ppi->ScanConfig.Yuv1ptr[PixelIndex];
      DestPtr = &ppi->ScanConfig.SrfWorkSpcPtr[PixelIndex];

      /* For each row of the block */
      for ( j = 0; j < VFRAGPIXELS; j++ ){
        /* Copy the data unaltered from source to destination */
        memcpy(DestPtr,SourcePtr,8);

        /* Increment pointers for next line in the block */
        SourcePtr += ppi->PlaneWidth;
        DestPtr += ppi->PlaneWidth;
      }
    }

    /* Increment pixel index for next block. */
    PixelIndex += HFRAGPIXELS;
  }
}

static void RowBarEnhBlockMap( PP_INSTANCE *ppi,
                               signed char   * UpdatedBlockMapPtr,
                               signed char   * BarBlockMapPtr,
                               ogg_uint32_t RowNumber ){
  int i;

  /* Start by blanking the row in the bar block map structure. */
  memset( BarBlockMapPtr, BLOCK_NOT_CODED, ppi->PlaneHFragments );

  /* First row */
  if ( RowNumber == 0 ){

    /* For each fragment in the row. */
    for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
      /* Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW. Uncoded or
         coded blocks will be ignored. */
      if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK ){
        /* Is one of the immediate neighbours updated in the main map. */
        /* Note special cases for blocks at the start and end of rows. */
        if ( i == 0 ){

          if ((UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
              (UpdatedBlockMapPtr[i+ppi->PlaneHFragments]>BLOCK_NOT_CODED ) ||
              (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1]>BLOCK_NOT_CODED ) )
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;


        }else if ( i == (ppi->PlaneHFragments - 1) ){

          if ((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
              (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1]>BLOCK_NOT_CODED) ||
               (UpdatedBlockMapPtr[i+ppi->PlaneHFragments]>BLOCK_NOT_CODED) )
              BarBlockMapPtr[i] = BLOCK_CODED_BAR;

        }else{
          if((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED) )
              BarBlockMapPtr[i] = BLOCK_CODED_BAR;
        }
      }
    }

  } else if ( RowNumber == (ogg_uint32_t)(ppi->PlaneVFragments-1)) {

    /* Last row */
    /* Used to read PlaneHFragments */

    /* For each fragment in the row. */
    for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
      /* Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW
         Uncoded or coded blocks will be ignored. */
      if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK ){
        /* Is one of the immediate neighbours updated in the main map. */
        /* Note special cases for blocks at the start and end of rows. */
        if ( i == 0 ){
          if((UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ))
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;

        }else if ( i == (ppi->PlaneHFragments - 1) ){
          if((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) )
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;
        }else{
          if((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED) )
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;
        }
      }
    }

  }else{
    /* All other rows */
    /* For each fragment in the row. */
    for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
      /* Test for CANDIDATE_BLOCK or CANDIDATE_BLOCK_LOW */
      /* Uncoded or coded blocks will be ignored. */
      if ( UpdatedBlockMapPtr[i] <= CANDIDATE_BLOCK ){
        /* Is one of the immediate neighbours updated in the main map. */
        /* Note special cases for blocks at the start and end of rows. */
        if ( i == 0 ){

          if((UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED) )
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;

        }else if ( i == (ppi->PlaneHFragments - 1) ){

          if((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) )
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;

        }else{
          if((UpdatedBlockMapPtr[i-1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+1] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i-ppi->PlaneHFragments+1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments-1] > BLOCK_NOT_CODED)||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments] > BLOCK_NOT_CODED ) ||
             (UpdatedBlockMapPtr[i+ppi->PlaneHFragments+1] > BLOCK_NOT_CODED ))
            BarBlockMapPtr[i] = BLOCK_CODED_BAR;
        }
      }
    }
  }
}

static void BarCopyBack( PP_INSTANCE *ppi,
                         signed char  * UpdatedBlockMapPtr,
                         signed char  * BarBlockMapPtr ){
  ogg_int32_t i;

  /* For each fragment in the row. */
  for ( i = 0; i < ppi->PlaneHFragments; i ++ ){
    if ( BarBlockMapPtr[i] > BLOCK_NOT_CODED ){
      UpdatedBlockMapPtr[i] = BarBlockMapPtr[i];
    }
  }
}

static void AnalysePlane( PP_INSTANCE *ppi,
                          unsigned char * PlanePtr0,
                          unsigned char * PlanePtr1,
                          ogg_uint32_t FragArrayOffset,
                          ogg_uint32_t PWidth,
                          ogg_uint32_t PHeight,
                          ogg_uint32_t PStride ) {
  unsigned char  * RawPlanePtr0;
  unsigned char  * RawPlanePtr1;

  ogg_int16_t  * YUVDiffsPtr;
  ogg_int16_t  * YUVDiffsPtr1;
  ogg_int16_t  * YUVDiffsPtr2;

  ogg_uint32_t FragIndex;
  ogg_uint32_t ScoreFragIndex1;
  ogg_uint32_t ScoreFragIndex2;
  ogg_uint32_t ScoreFragIndex3;
  ogg_uint32_t ScoreFragIndex4;

  int   UpdatedOrCandidateBlocks = 0;

  unsigned char  * ChLocalsPtr0;
  unsigned char  * ChLocalsPtr1;
  unsigned char  * ChLocalsPtr2;

  unsigned char  * PixelsChangedPtr0;
  unsigned char  * PixelsChangedPtr1;

  unsigned char  * PixelScoresPtr1;
  unsigned char  * PixelScoresPtr2;

  signed char   * DispFragPtr0;
  signed char   * DispFragPtr1;
  signed char   * DispFragPtr2;

  ogg_uint32_t * FragScoresPtr1;
  ogg_uint32_t * FragScoresPtr2;

  ogg_int32_t  * RowDiffsPtr;
  ogg_int32_t  * RowDiffsPtr1;
  ogg_int32_t  * RowDiffsPtr2;

  ogg_int32_t  i,j;

  ogg_int32_t  RowNumber1;
  ogg_int32_t  RowNumber2;
  ogg_int32_t  RowNumber3;
  ogg_int32_t  RowNumber4;

  int   EdgeRow;
  ogg_int32_t  LineSearchRowNumber = 0;

  /* Variables used as temporary stores for frequently used values. */
  ogg_int32_t  Row0Mod3;
  ogg_int32_t  Row1Mod3;
  ogg_int32_t  Row2Mod3;
  ogg_int32_t  BlockRowPixels;

  /* Set pixel difference threshold */
  if ( FragArrayOffset == 0 ){
    /* Luminance */
    ppi->LevelThresh = (int)ppi->SgcLevelThresh;
    ppi->NegLevelThresh = -ppi->LevelThresh;

    ppi->SrfThresh = (int)ppi->SRFGreyThresh;
    ppi->NegSrfThresh = -ppi->SrfThresh;

    /* Scores correction for Y pixels. */
    ppi->YUVPlaneCorrectionFactor = 1.0;

    ppi->BlockThreshold = ppi->PrimaryBlockThreshold;
    ppi->BlockSgcThresh = ppi->SgcThresh;
  }else{
    /* Chrominance */
    ppi->LevelThresh = (int)ppi->SuvcLevelThresh;
    ppi->NegLevelThresh = -ppi->LevelThresh;

    ppi->SrfThresh = (int)ppi->SRFColThresh;
    ppi->NegSrfThresh = -ppi->SrfThresh;

    /* Scores correction for UV pixels. */
    ppi->YUVPlaneCorrectionFactor = 1.5;

    /* Block threholds different for subsampled U and V blocks */
    ppi->BlockThreshold =
      (ppi->PrimaryBlockThreshold / ppi->UVBlockThreshCorrection);
    ppi->BlockSgcThresh =
      (ppi->SgcThresh / ppi->UVSgcCorrection);
  }

  /* Initialise the SRF thresh table and pointer. */
  memset( ppi->SrfThreshTable, 1, 512 );
  for ( i = ppi->NegSrfThresh; i <= ppi->SrfThresh; i++ )
    ppi->SrfThreshTable[i+255] = 0;

  /* Initialise the PAK thresh table. */
  for ( i = -255; i <= 255; i++ )
    if ( ppi->SrfThreshTable[i+255] &&
         (i <= ppi->HighChange) &&
         (i >= ppi->NegHighChange) )
      ppi->SrfPakThreshTable[i+255] = 1;
    else
      ppi->SrfPakThreshTable[i+255] = 0;

  /* Initialise the SGc lookup table */
  for ( i = -255; i <= 255; i++ ){
    if ( i <= ppi->NegLevelThresh )
      ppi->SgcThreshTable[i+255] = (unsigned char) -1;
    else if ( i >= ppi->LevelThresh )
      ppi->SgcThreshTable[i+255] = 1;
    else
      ppi->SgcThreshTable[i+255] = 0;
  }

  /* Set up plane dimension variables */
  ppi->PlaneHFragments = PWidth / HFRAGPIXELS;
  ppi->PlaneVFragments = PHeight / VFRAGPIXELS;
  ppi->PlaneWidth = PWidth;
  ppi->PlaneHeight = PHeight;
  ppi->PlaneStride = PStride;

  /* Set up local pointers into the raw image data. */
  RawPlanePtr0 = PlanePtr0;
  RawPlanePtr1 = PlanePtr1;

  /* Note size and endo points for circular buffers. */
  ppi->YuvDiffsCircularBufferSize = YDIFF_CB_ROWS * ppi->PlaneWidth;
  ppi->ChLocalsCircularBufferSize = CHLOCALS_CB_ROWS * ppi->PlaneWidth;
  ppi->PixelMapCircularBufferSize = PMAP_CB_ROWS * ppi->PlaneWidth;

  /* Set high change thresh where PAK not needed */
  ppi->HighChange = ppi->SrfThresh * 4;
  ppi->NegHighChange = -ppi->HighChange;

  /* Set up row difference pointers. */
  RowDiffsPtr = ppi->RowChangedPixels;
  RowDiffsPtr1 = ppi->RowChangedPixels;
  RowDiffsPtr2 = ppi->RowChangedPixels;

  BlockRowPixels = ppi->PlaneWidth * VFRAGPIXELS;

  for ( i = 0; i < (ppi->PlaneVFragments + 4); i++ ){
    RowNumber1 = (i - 1);
    RowNumber2 = (i - 2);
    RowNumber3 = (i - 3);
    RowNumber4 = (i - 4);

    /* Pre calculate some frequently used values */
    Row0Mod3 = i % 3;
    Row1Mod3 = RowNumber1 % 3;
    Row2Mod3 = RowNumber2 % 3;

    /*  For row diff scan last two iterations are invalid */
    if ( i < ppi->PlaneVFragments ){
      FragIndex = (i * ppi->PlaneHFragments) + FragArrayOffset;
      YUVDiffsPtr = &ppi->yuv_differences[Row0Mod3 * BlockRowPixels];

      PixelsChangedPtr0 = (&ppi->PixelChangedMap[Row0Mod3 * BlockRowPixels]);
      DispFragPtr0 =  &ppi->ScanDisplayFragments[FragIndex];

      ChLocalsPtr0 = (&ppi->ChLocals[Row0Mod3 * BlockRowPixels]);

    }

    /* Set up the changed locals pointer to trail behind by one row of
       fragments. */
    if ( i > 0 ){
      /* For last iteration the ch locals and noise scans are invalid */
      if ( RowNumber1 < ppi->PlaneVFragments ){
        ScoreFragIndex1 = (RowNumber1 * ppi->PlaneHFragments) +
          FragArrayOffset;

        ChLocalsPtr1 = &ppi->ChLocals[Row1Mod3 * BlockRowPixels];
        PixelsChangedPtr1 =
          &ppi->PixelChangedMap[(Row1Mod3) * BlockRowPixels];

        PixelScoresPtr1 = &ppi->PixelScores[(RowNumber1 % 4) * BlockRowPixels];

        YUVDiffsPtr1 = &ppi->yuv_differences[Row1Mod3 * BlockRowPixels];
        FragScoresPtr1 = &ppi->FragScores[ScoreFragIndex1];
        DispFragPtr1 = &ppi->ScanDisplayFragments[ScoreFragIndex1];

      }

      if ( RowNumber2 >= 0 ){
        ScoreFragIndex2 = (RowNumber2 * ppi->PlaneHFragments) +
          FragArrayOffset;
        ChLocalsPtr2 = (&ppi->ChLocals[Row2Mod3 * BlockRowPixels]);
        YUVDiffsPtr2 = &ppi->yuv_differences[Row2Mod3 * BlockRowPixels];

        PixelScoresPtr2 = &ppi->PixelScores[(RowNumber2 % 4) * BlockRowPixels];

        FragScoresPtr2 =  &ppi->FragScores[ScoreFragIndex2];
        DispFragPtr2 = &ppi->ScanDisplayFragments[ScoreFragIndex2];
      }else{
        ChLocalsPtr2 = NULL;
      }
    }else{
      ChLocalsPtr1 = NULL;
      ChLocalsPtr2 = NULL;
    }

    /* Fast break out test for obvious yes and no cases in this row of
       blocks */
    if ( i < ppi->PlaneVFragments ){
      dsp_save_fpu (ppi->dsp);
      UpdatedOrCandidateBlocks =
        RowSadScan( ppi, RawPlanePtr0, RawPlanePtr1, DispFragPtr0 );
      UpdatedOrCandidateBlocks |=
        ColSadScan( ppi, RawPlanePtr0, RawPlanePtr1, DispFragPtr0 );
      dsp_restore_fpu (ppi->dsp);
    }else{
      /* Make sure we still call other functions if RowSadScan() disabled */
      UpdatedOrCandidateBlocks = 1;
    }

    /* Consolidation and fast break ot tests at Row 1 level */
    if ( (i > 0) && (RowNumber1 < ppi->PlaneVFragments) ){
      /* Mark as coded any candidate block that lies adjacent to a
         coded block. */
      SadPass2( ppi, RowNumber1, DispFragPtr1 );

      /* Check results of diff scan in last set of blocks. */
      /* Eliminate NO cases and add in +SGC cases */
      ConsolidateDiffScanResults( ppi, &ppi->FragDiffPixels[ScoreFragIndex1],
                                  &ppi->SameGreyDirPixels[ScoreFragIndex1],
                                  DispFragPtr1
                                  );
    }

    for ( j = 0; j < VFRAGPIXELS; j++ ){
      /* Last two iterations do not apply */
      if ( i < ppi->PlaneVFragments ){
        /* Is the current fragment at an edge. */
        EdgeRow = ( ( (i == 0) && (j == 0) ) ||
                    ( (i == (ppi->PlaneVFragments - 1)) &&
                      (j == (VFRAGPIXELS - 1)) ) );

        /* Clear the arrays that will be used for the changed pixels maps */
        memset( PixelsChangedPtr0, 0, ppi->PlaneWidth );

        /* Difference scan and map each row */
        if ( UpdatedOrCandidateBlocks ){
          /* Scan the row for interesting differences */
          /* Also clear the array that will be used for changed locals map */
          RowDiffScan( ppi, RawPlanePtr0, RawPlanePtr1,
                       YUVDiffsPtr, PixelsChangedPtr0,
                       &ppi->SameGreyDirPixels[FragIndex],
                       DispFragPtr0, &ppi->FragDiffPixels[FragIndex],
                       RowDiffsPtr, ChLocalsPtr0, EdgeRow);
        }else{
          /* Clear the array that will be used for changed locals map */
          memset( ChLocalsPtr0, 0, ppi->PlaneWidth );
        }

        /* The actual image plane pointers must be incremented by
           stride as this may be different (more) than the plane
           width. Our own internal buffers use ppi->PlaneWidth. */
        RawPlanePtr0 += ppi->PlaneStride;
        RawPlanePtr1 += ppi->PlaneStride;
        PixelsChangedPtr0 += ppi->PlaneWidth;
        ChLocalsPtr0 += ppi->PlaneWidth;
        YUVDiffsPtr += ppi->PlaneWidth;
        RowDiffsPtr++;
      }

      /* Run behind calculating the changed locals data and noise scores. */
      if ( ChLocalsPtr1 != NULL ){
        /* Last few iterations do not apply */
        if ( RowNumber1 < ppi->PlaneVFragments ){
          /* Blank the next row in the pixel scores data structure. */
          memset( PixelScoresPtr1, 0, ppi->PlaneWidth );

          /* Don't bother doing anything if there are no changed
             pixels in this row */
          if ( *RowDiffsPtr1 ){
            /* Last valid row is a special case */
            if ( i < ppi->PlaneVFragments )
              RowChangedLocalsScan( ppi, PixelsChangedPtr1, ChLocalsPtr1,
                                    DispFragPtr1,
                                    ( (((i-1)==0) && (j==0)) ?
                                      FIRST_ROW : NOT_EDGE_ROW) );
            else
              RowChangedLocalsScan( ppi, PixelsChangedPtr1, ChLocalsPtr1,
                                    DispFragPtr1,
                                    ((j==(VFRAGPIXELS-1)) ?
                                     LAST_ROW : NOT_EDGE_ROW) );

            NoiseScoreRow( ppi, PixelsChangedPtr1, ChLocalsPtr1, YUVDiffsPtr1,
                           PixelScoresPtr1, FragScoresPtr1, DispFragPtr1,
                           RowDiffsPtr1 );
          }

          ChLocalsPtr1 += ppi->PlaneWidth;
          PixelsChangedPtr1 += ppi->PlaneWidth;
          YUVDiffsPtr1 += ppi->PlaneWidth;
          PixelScoresPtr1 += ppi->PlaneWidth;
          RowDiffsPtr1 ++;
        }

        /* Run edge enhancement algorithms */
        if ( RowNumber2 < ppi->PlaneVFragments ){
          if ( ChLocalsPtr2 != NULL ){
            /* Don't bother doing anything if there are no changed
               pixels in this row */
            if ( *RowDiffsPtr2 ){
              if ( RowNumber1 < ppi->PlaneVFragments ){
                PrimaryEdgeScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
                                     PixelScoresPtr2, FragScoresPtr2,
                                     DispFragPtr2,
                                     ( (((i-2)==0) && (j==0)) ?
                                       FIRST_ROW : NOT_EDGE_ROW)  );
              }else{
                /* Edge enhancement */
                PrimaryEdgeScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
                                     PixelScoresPtr2, FragScoresPtr2,
                                     DispFragPtr2,
                                     ((j==(VFRAGPIXELS-1)) ?
                                      LAST_ROW : NOT_EDGE_ROW) );
              }

              /* Recursive line search */
              LineSearchScoreRow( ppi, ChLocalsPtr2, YUVDiffsPtr2,
                                  PixelScoresPtr2, FragScoresPtr2,
                                  DispFragPtr2,
                                  LineSearchRowNumber );
            }

            ChLocalsPtr2 += ppi->PlaneWidth;
            YUVDiffsPtr2 += ppi->PlaneWidth;
            PixelScoresPtr2 += ppi->PlaneWidth;
            LineSearchRowNumber += 1;
            RowDiffsPtr2 ++;
          }
        }
      }
    }

    /* BAR algorithm */
    if ( (RowNumber3 >= 0) && (RowNumber3 < ppi->PlaneVFragments) ){
      ScoreFragIndex3 = (RowNumber3 * ppi->PlaneHFragments) + FragArrayOffset;
      RowBarEnhBlockMap(ppi,
                        &ppi->ScanDisplayFragments[ScoreFragIndex3],
                        &ppi->BarBlockMap[(RowNumber3 % 3) *
                                         ppi->PlaneHFragments],
                        RowNumber3 );
    }

    /* BAR copy back and "ppi->SRF filtering" or "pixel copy back" */
    if ( (RowNumber4 >= 0) && (RowNumber4 < ppi->PlaneVFragments) ){
      /* BAR copy back stage must lag by one more row to avoid BAR blocks
         being used in BAR descisions. */
      ScoreFragIndex4 = (RowNumber4 * ppi->PlaneHFragments) + FragArrayOffset;

      BarCopyBack(ppi, &ppi->ScanDisplayFragments[ScoreFragIndex4],
                  &ppi->BarBlockMap[(RowNumber4 % 3) * ppi->PlaneHFragments]);

      /* Copy over the data from any blocks marked for update into the
         output buffer. */
      RowCopy(ppi, ScoreFragIndex4);
    }
  }
}

ogg_uint32_t YUVAnalyseFrame( PP_INSTANCE *ppi, ogg_uint32_t * KFIndicator ){

  /* Initialise the map arrays. */
  InitScanMapArrays(ppi);

  /* If the motion level in the previous frame was high then adjust
     the high and low SAD thresholds to speed things up. */
  ppi->ModifiedGrpLowSadThresh = ppi->GrpLowSadThresh;
  ppi->ModifiedGrpHighSadThresh = ppi->GrpHighSadThresh;


  /* Set up the internal plane height and width variables. */
  ppi->VideoYPlaneWidth = ppi->ScanConfig.VideoFrameWidth;
  ppi->VideoYPlaneHeight = ppi->ScanConfig.VideoFrameHeight;
  ppi->VideoUVPlaneWidth = ppi->ScanConfig.VideoFrameWidth / 2;
  ppi->VideoUVPlaneHeight = ppi->ScanConfig.VideoFrameHeight / 2;

  /* To start with the strides will be set from the widths */
  ppi->VideoYPlaneStride = ppi->VideoYPlaneWidth;
  ppi->VideoUPlaneStride = ppi->VideoUVPlaneWidth;
  ppi->VideoVPlaneStride = ppi->VideoUVPlaneWidth;

  /* Set up the plane pointers */
  ppi->YPlanePtr0 = ppi->ScanConfig.Yuv0ptr;
  ppi->YPlanePtr1 = ppi->ScanConfig.Yuv1ptr;
  ppi->UPlanePtr0 = (ppi->ScanConfig.Yuv0ptr + ppi->YFramePixels);
  ppi->UPlanePtr1 = (ppi->ScanConfig.Yuv1ptr + ppi->YFramePixels);
  ppi->VPlanePtr0 = (ppi->ScanConfig.Yuv0ptr + ppi->YFramePixels +
                     ppi->UVFramePixels);
  ppi->VPlanePtr1 = (ppi->ScanConfig.Yuv1ptr + ppi->YFramePixels +
                     ppi->UVFramePixels);

  /* Check previous frame lists and if necessary mark extra blocks for
     update. */
  SetFromPrevious(ppi);

  /* Ananlyse the U and V palnes. */
  AnalysePlane( ppi, ppi->UPlanePtr0, ppi->UPlanePtr1,
                ppi->ScanYPlaneFragments, ppi->VideoUVPlaneWidth,
                ppi->VideoUVPlaneHeight, ppi->VideoUPlaneStride );
  AnalysePlane( ppi, ppi->VPlanePtr0, ppi->VPlanePtr1,
                (ppi->ScanYPlaneFragments + ppi->ScanUVPlaneFragments),
                ppi->VideoUVPlaneWidth, ppi->VideoUVPlaneHeight,
                ppi->VideoVPlaneStride );

  /* Now analyse the Y plane. */
  AnalysePlane( ppi, ppi->YPlanePtr0, ppi->YPlanePtr1, 0,
                ppi->VideoYPlaneWidth, ppi->VideoYPlaneHeight,
                ppi->VideoYPlaneStride );

  /* Update the list of previous frame block updates. */
  UpdatePreviousBlockLists(ppi);

  /* Create an output block map for the calling process. */
  CreateOutputDisplayMap( ppi, ppi->ScanDisplayFragments,
                          ppi->PrevFragments[0],
                          ppi->ScanConfig.disp_fragments );

  /* Set the candidate key frame indicator (0-100) */
  *KFIndicator = ppi->KFIndicator;

  /* Return the normalised block count (this is actually a motion
     level weighting not a true block count). */
  return ppi->OutputBlocksUpdated;
}

