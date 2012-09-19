/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2007                *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: dct_decode.c 15400 2008-10-15 12:10:58Z tterribe $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "codec_internal.h"
#include "quant_lookup.h"


#define GOLDEN_FRAME_THRESH_Q   50
#define PUR 8
#define PU 4
#define PUL 2
#define PL 1
#define HIGHBITDUPPED(X) (((signed short) X)  >> 15)


static const int ModeUsesMC[MAX_MODES] = { 0, 0, 1, 1, 1, 0, 1, 1 };

static void SetupBoundingValueArray_Generic(ogg_int16_t *BoundingValuePtr,
                                            ogg_int32_t FLimit){

  ogg_int32_t i;

  /* Set up the bounding value array. */
  memset ( BoundingValuePtr, 0, (256*sizeof(*BoundingValuePtr)) );
  for ( i = 0; i < FLimit; i++ ){
    BoundingValuePtr[127-i-FLimit] = (-FLimit+i);
    BoundingValuePtr[127-i] = -i;
    BoundingValuePtr[127+i] = i;
    BoundingValuePtr[127+i+FLimit] = FLimit-i;
  }
}

static void ExpandKFBlock ( PB_INSTANCE *pbi, ogg_int32_t FragmentNumber ){
  ogg_uint32_t ReconPixelsPerLine;
  ogg_int32_t     ReconPixelIndex;

  /* Select the appropriate inverse Q matrix and line stride */
  if ( FragmentNumber<(ogg_int32_t)pbi->YPlaneFragments ){
    ReconPixelsPerLine = pbi->YStride;
    pbi->dequant_coeffs = pbi->dequant_Y_coeffs;
  }else if ( FragmentNumber<(ogg_int32_t)(pbi->YPlaneFragments + pbi->UVPlaneFragments) ){
    ReconPixelsPerLine = pbi->UVStride;
    pbi->dequant_coeffs = pbi->dequant_U_coeffs;
  }else{
    ReconPixelsPerLine = pbi->UVStride;
    pbi->dequant_coeffs = pbi->dequant_V_coeffs;
  }

  /* Set up pointer into the quantisation buffer. */
  pbi->quantized_list = &pbi->QFragData[FragmentNumber][0];

  /* Invert quantisation and DCT to get pixel data. */
  switch(pbi->FragCoefEOB[FragmentNumber]){
  case 0:case 1:
    IDct1( pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  case 2: case 3:
    dsp_IDct3(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  case 4:case 5:case 6:case 7:case 8: case 9:case 10:
    dsp_IDct10(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  default:
    dsp_IDctSlow(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
  }

  /* Convert fragment number to a pixel offset in a reconstruction buffer. */
  ReconPixelIndex = pbi->recon_pixel_index_table[FragmentNumber];

  /* Get the pixel index for the first pixel in the fragment. */
  dsp_recon_intra8x8 (pbi->dsp, (unsigned char *)(&pbi->ThisFrameRecon[ReconPixelIndex]),
              (ogg_int16_t *)pbi->ReconDataBuffer, ReconPixelsPerLine);
}

static void ExpandBlock ( PB_INSTANCE *pbi, ogg_int32_t FragmentNumber){
  unsigned char *LastFrameRecPtr;   /* Pointer into previous frame
                                       reconstruction. */
  unsigned char *LastFrameRecPtr2;  /* Pointer into previous frame
                                       reconstruction for 1/2 pixel MC. */

  ogg_uint32_t   ReconPixelsPerLine; /* Pixels per line */
  ogg_int32_t    ReconPixelIndex;    /* Offset for block into a
                                        reconstruction buffer */
  ogg_int32_t    ReconPtr2Offset;    /* Offset for second
                                        reconstruction in half pixel
                                        MC */
  ogg_int32_t    MVOffset;           /* Baseline motion vector offset */
  ogg_int32_t    MvShift  ;          /* Shift to correct to 1/2 or 1/4 pixel */
  ogg_int32_t    MvModMask;          /* Mask to determine whether 1/2
                                        pixel is used */

  /* Get coding mode for this block */
  if ( pbi->FrameType == KEY_FRAME ){
    pbi->CodingMode = CODE_INTRA;
  }else{
    /* Get Motion vector and mode for this block. */
    pbi->CodingMode = pbi->FragCodingMethod[FragmentNumber];
  }

  /* Select the appropriate inverse Q matrix and line stride */
  if ( FragmentNumber<(ogg_int32_t)pbi->YPlaneFragments ) {
    ReconPixelsPerLine = pbi->YStride;
    MvShift = 1;
    MvModMask = 0x00000001;

    /* Select appropriate dequantiser matrix. */
    if ( pbi->CodingMode == CODE_INTRA )
      pbi->dequant_coeffs = pbi->dequant_Y_coeffs;
    else
      pbi->dequant_coeffs = pbi->dequant_InterY_coeffs;
  }else{
    ReconPixelsPerLine = pbi->UVStride;
    MvShift = 2;
    MvModMask = 0x00000003;

    /* Select appropriate dequantiser matrix. */
    if ( pbi->CodingMode == CODE_INTRA )
      if ( FragmentNumber <
                (ogg_int32_t)(pbi->YPlaneFragments + pbi->UVPlaneFragments) )
        pbi->dequant_coeffs = pbi->dequant_U_coeffs;
      else
        pbi->dequant_coeffs = pbi->dequant_V_coeffs;
    else
      if ( FragmentNumber <
                (ogg_int32_t)(pbi->YPlaneFragments + pbi->UVPlaneFragments) )
        pbi->dequant_coeffs = pbi->dequant_InterU_coeffs;
      else
        pbi->dequant_coeffs = pbi->dequant_InterV_coeffs;
  }

  /* Set up pointer into the quantisation buffer. */
  pbi->quantized_list = &pbi->QFragData[FragmentNumber][0];

  /* Invert quantisation and DCT to get pixel data. */
  switch(pbi->FragCoefEOB[FragmentNumber]){
  case 0:case 1:
    IDct1( pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  case 2: case 3:
    dsp_IDct3(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  case 4:case 5:case 6:case 7:case 8: case 9:case 10:
    dsp_IDct10(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
    break;
  default:
    dsp_IDctSlow(pbi->dsp, pbi->quantized_list, pbi->dequant_coeffs, pbi->ReconDataBuffer );
  }

  /* Convert fragment number to a pixel offset in a reconstruction buffer. */
  ReconPixelIndex = pbi->recon_pixel_index_table[FragmentNumber];

  /* Action depends on decode mode. */
  if ( pbi->CodingMode == CODE_INTER_NO_MV ){
    /* Inter with no motion vector */
    /* Reconstruct the pixel data using the last frame reconstruction
       and change data when the motion vector is (0,0), the recon is
       based on the lastframe without loop filtering---- for testing */
    dsp_recon_inter8x8 (pbi->dsp, &pbi->ThisFrameRecon[ReconPixelIndex],
                &pbi->LastFrameRecon[ReconPixelIndex],
                  pbi->ReconDataBuffer, ReconPixelsPerLine);
  }else if ( ModeUsesMC[pbi->CodingMode] ) {
    /* The mode uses a motion vector. */
    /* Get vector from list */
    pbi->MVector.x = pbi->FragMVect[FragmentNumber].x;
    pbi->MVector.y = pbi->FragMVect[FragmentNumber].y;

    /* Work out the base motion vector offset and the 1/2 pixel offset
       if any.  For the U and V planes the MV specifies 1/4 pixel
       accuracy. This is adjusted to 1/2 pixel as follows ( 0->0,
       1/4->1/2, 1/2->1/2, 3/4->1/2 ). */
    MVOffset = 0;
    ReconPtr2Offset = 0;
    if ( pbi->MVector.x > 0 ){
      MVOffset = pbi->MVector.x >> MvShift;
      if ( pbi->MVector.x & MvModMask )
        ReconPtr2Offset += 1;
    } else if ( pbi->MVector.x < 0 ) {
      MVOffset -= (-pbi->MVector.x) >> MvShift;
      if ( (-pbi->MVector.x) & MvModMask )
        ReconPtr2Offset -= 1;
    }

    if ( pbi->MVector.y > 0 ){
      MVOffset += (pbi->MVector.y >>  MvShift) * ReconPixelsPerLine;
      if ( pbi->MVector.y & MvModMask )
        ReconPtr2Offset += ReconPixelsPerLine;
    } else if ( pbi->MVector.y < 0 ){
      MVOffset -= ((-pbi->MVector.y) >> MvShift) * ReconPixelsPerLine;
      if ( (-pbi->MVector.y) & MvModMask )
        ReconPtr2Offset -= ReconPixelsPerLine;
    }

    /* Set up the first of the two reconstruction buffer pointers. */
    if ( pbi->CodingMode==CODE_GOLDEN_MV ) {
      LastFrameRecPtr = &pbi->GoldenFrame[ReconPixelIndex] + MVOffset;
    }else{
      LastFrameRecPtr = &pbi->LastFrameRecon[ReconPixelIndex] + MVOffset;
    }

    /* Set up the second of the two reconstruction pointers. */
    LastFrameRecPtr2 = LastFrameRecPtr + ReconPtr2Offset;

    /* Select the appropriate reconstruction function */
    if ( (int)(LastFrameRecPtr - LastFrameRecPtr2) == 0 ) {
      /* Reconstruct the pixel dats from the reference frame and change data
         (no half pixel in this case as the two references were the same. */
      dsp_recon_inter8x8 (pbi->dsp,
          &pbi->ThisFrameRecon[ReconPixelIndex],
                  LastFrameRecPtr, pbi->ReconDataBuffer,
                  ReconPixelsPerLine);
    }else{
      /* Fractional pixel reconstruction. */
      /* Note that we only use two pixels per reconstruction even for
         the diagonal. */
      dsp_recon_inter8x8_half(pbi->dsp, &pbi->ThisFrameRecon[ReconPixelIndex],
                            LastFrameRecPtr, LastFrameRecPtr2,
                            pbi->ReconDataBuffer, ReconPixelsPerLine);
    }
  } else if ( pbi->CodingMode == CODE_USING_GOLDEN ){
    /* Golden frame with motion vector */
    /* Reconstruct the pixel data using the golden frame
       reconstruction and change data */
    dsp_recon_inter8x8 (pbi->dsp, &pbi->ThisFrameRecon[ReconPixelIndex],
                &pbi->GoldenFrame[ ReconPixelIndex ],
                  pbi->ReconDataBuffer, ReconPixelsPerLine);
  } else {
    /* Simple Intra coding */
    /* Get the pixel index for the first pixel in the fragment. */
    dsp_recon_intra8x8 (pbi->dsp, &pbi->ThisFrameRecon[ReconPixelIndex],
              pbi->ReconDataBuffer, ReconPixelsPerLine);
  }
}

static void UpdateUMV_HBorders( PB_INSTANCE *pbi,
                                unsigned char * DestReconPtr,
                                ogg_uint32_t  PlaneFragOffset ) {
  ogg_uint32_t  i;
  ogg_uint32_t  PixelIndex;

  ogg_uint32_t  PlaneStride;
  ogg_uint32_t  BlockVStep;
  ogg_uint32_t  PlaneFragments;
  ogg_uint32_t  LineFragments;
  ogg_uint32_t  PlaneBorderWidth;

  unsigned char   *SrcPtr1;
  unsigned char   *SrcPtr2;
  unsigned char   *DestPtr1;
  unsigned char   *DestPtr2;

  /* Work out various plane specific values */
  if ( PlaneFragOffset == 0 ) {
    /* Y Plane */
    BlockVStep = (pbi->YStride *
                  (VFRAGPIXELS - 1));
    PlaneStride = pbi->YStride;
    PlaneBorderWidth = UMV_BORDER;
    PlaneFragments = pbi->YPlaneFragments;
    LineFragments = pbi->HFragments;
  }else{
    /* U or V plane. */
    BlockVStep = (pbi->UVStride *
                  (VFRAGPIXELS - 1));
    PlaneStride = pbi->UVStride;
    PlaneBorderWidth = UMV_BORDER / 2;
    PlaneFragments = pbi->UVPlaneFragments;
    LineFragments = pbi->HFragments / 2;
  }

  /* Setup the source and destination pointers for the top and bottom
     borders */
  PixelIndex = pbi->recon_pixel_index_table[PlaneFragOffset];
  SrcPtr1 = &DestReconPtr[ PixelIndex - PlaneBorderWidth ];
  DestPtr1 = SrcPtr1 - (PlaneBorderWidth * PlaneStride);

  PixelIndex = pbi->recon_pixel_index_table[PlaneFragOffset +
                                           PlaneFragments - LineFragments] +
    BlockVStep;
  SrcPtr2 = &DestReconPtr[ PixelIndex - PlaneBorderWidth];
  DestPtr2 = SrcPtr2 + PlaneStride;

  /* Now copy the top and bottom source lines into each line of the
     respective borders */
  for ( i = 0; i < PlaneBorderWidth; i++ ) {
    memcpy( DestPtr1, SrcPtr1, PlaneStride );
    memcpy( DestPtr2, SrcPtr2, PlaneStride );
    DestPtr1 += PlaneStride;
    DestPtr2 += PlaneStride;
  }
}

static void UpdateUMV_VBorders( PB_INSTANCE *pbi,
                                unsigned char * DestReconPtr,
                                ogg_uint32_t  PlaneFragOffset ){
  ogg_uint32_t  i;
  ogg_uint32_t  PixelIndex;

  ogg_uint32_t  PlaneStride;
  ogg_uint32_t  LineFragments;
  ogg_uint32_t  PlaneBorderWidth;
  ogg_uint32_t   PlaneHeight;

  unsigned char   *SrcPtr1;
  unsigned char   *SrcPtr2;
  unsigned char   *DestPtr1;
  unsigned char   *DestPtr2;

  /* Work out various plane specific values */
  if ( PlaneFragOffset == 0 ) {
    /* Y Plane */
    PlaneStride = pbi->YStride;
    PlaneBorderWidth = UMV_BORDER;
    LineFragments = pbi->HFragments;
    PlaneHeight = pbi->info.height;
  }else{
    /* U or V plane. */
    PlaneStride = pbi->UVStride;
    PlaneBorderWidth = UMV_BORDER / 2;
    LineFragments = pbi->HFragments / 2;
    PlaneHeight = pbi->info.height / 2;
  }

  /* Setup the source data values and destination pointers for the
     left and right edge borders */
  PixelIndex = pbi->recon_pixel_index_table[PlaneFragOffset];
  SrcPtr1 = &DestReconPtr[ PixelIndex ];
  DestPtr1 = &DestReconPtr[ PixelIndex - PlaneBorderWidth ];

  PixelIndex = pbi->recon_pixel_index_table[PlaneFragOffset +
                                           LineFragments - 1] +
    (HFRAGPIXELS - 1);
  SrcPtr2 = &DestReconPtr[ PixelIndex ];
  DestPtr2 = &DestReconPtr[ PixelIndex + 1 ];

  /* Now copy the top and bottom source lines into each line of the
     respective borders */
  for ( i = 0; i < PlaneHeight; i++ ) {
    memset( DestPtr1, SrcPtr1[0], PlaneBorderWidth );
    memset( DestPtr2, SrcPtr2[0], PlaneBorderWidth );
    SrcPtr1 += PlaneStride;
    SrcPtr2 += PlaneStride;
    DestPtr1 += PlaneStride;
    DestPtr2 += PlaneStride;
  }
}

void UpdateUMVBorder( PB_INSTANCE *pbi,
                      unsigned char * DestReconPtr ) {
  ogg_uint32_t  PlaneFragOffset;

  /* Y plane */
  PlaneFragOffset = 0;
  UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
  UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );

  /* Then the U and V Planes */
  PlaneFragOffset = pbi->YPlaneFragments;
  UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
  UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );

  PlaneFragOffset = pbi->YPlaneFragments + pbi->UVPlaneFragments;
  UpdateUMV_VBorders( pbi, DestReconPtr, PlaneFragOffset );
  UpdateUMV_HBorders( pbi, DestReconPtr, PlaneFragOffset );
}

static void CopyRecon( PB_INSTANCE *pbi, unsigned char * DestReconPtr,
                unsigned char * SrcReconPtr ) {
  ogg_uint32_t  i;
  ogg_uint32_t  PlaneLineStep; /* Pixels per line */
  ogg_uint32_t  PixelIndex;

  unsigned char  *SrcPtr;      /* Pointer to line of source image data */
  unsigned char  *DestPtr;     /* Pointer to line of destination image data */

  /* Copy over only updated blocks.*/

  /* First Y plane */
  PlaneLineStep = pbi->YStride;
  for ( i = 0; i < pbi->YPlaneFragments; i++ ) {
    if ( pbi->display_fragments[i] ) {
      PixelIndex = pbi->recon_pixel_index_table[i];
      SrcPtr = &SrcReconPtr[ PixelIndex ];
      DestPtr = &DestReconPtr[ PixelIndex ];

      dsp_copy8x8 (pbi->dsp, SrcPtr, DestPtr, PlaneLineStep);
    }
  }

  /* Then U and V */
  PlaneLineStep = pbi->UVStride;
  for ( i = pbi->YPlaneFragments; i < pbi->UnitFragments; i++ ) {
    if ( pbi->display_fragments[i] ) {
      PixelIndex = pbi->recon_pixel_index_table[i];
      SrcPtr = &SrcReconPtr[ PixelIndex ];
      DestPtr = &DestReconPtr[ PixelIndex ];

      dsp_copy8x8 (pbi->dsp, SrcPtr, DestPtr, PlaneLineStep);

    }
  }
}

static void CopyNotRecon( PB_INSTANCE *pbi, unsigned char * DestReconPtr,
                   unsigned char * SrcReconPtr ) {
  ogg_uint32_t  i;
  ogg_uint32_t  PlaneLineStep; /* Pixels per line */
  ogg_uint32_t  PixelIndex;

  unsigned char  *SrcPtr;      /* Pointer to line of source image data */
  unsigned char  *DestPtr;     /* Pointer to line of destination image data*/

  /* Copy over only updated blocks. */

  /* First Y plane */
  PlaneLineStep = pbi->YStride;
  for ( i = 0; i < pbi->YPlaneFragments; i++ ) {
    if ( !pbi->display_fragments[i] ) {
      PixelIndex = pbi->recon_pixel_index_table[i];
      SrcPtr = &SrcReconPtr[ PixelIndex ];
      DestPtr = &DestReconPtr[ PixelIndex ];

      dsp_copy8x8 (pbi->dsp, SrcPtr, DestPtr, PlaneLineStep);
    }
  }

  /* Then U and V */
  PlaneLineStep = pbi->UVStride;
  for ( i = pbi->YPlaneFragments; i < pbi->UnitFragments; i++ ) {
    if ( !pbi->display_fragments[i] ) {
      PixelIndex = pbi->recon_pixel_index_table[i];
      SrcPtr = &SrcReconPtr[ PixelIndex ];
      DestPtr = &DestReconPtr[ PixelIndex ];

      dsp_copy8x8 (pbi->dsp, SrcPtr, DestPtr, PlaneLineStep);

    }
  }
}

void ExpandToken( Q_LIST_ENTRY * ExpandedBlock,
                  unsigned char * CoeffIndex, ogg_uint32_t Token,
                  ogg_int32_t ExtraBits ){
  /* Is the token is a combination run and value token. */
  if ( Token >= DCT_RUN_CATEGORY1 ){
    /* Expand the token and additional bits to a zero run length and
       data value.  */
    if ( Token < DCT_RUN_CATEGORY2 ) {
      /* Decoding method depends on token */
      if ( Token < DCT_RUN_CATEGORY1B ) {
        /* Step on by the zero run length */
        *CoeffIndex += (unsigned char)((Token - DCT_RUN_CATEGORY1) + 1);

        /* The extra bit determines the sign. */
        if ( ExtraBits & 0x01 )
          ExpandedBlock[*CoeffIndex] = -1;
        else
          ExpandedBlock[*CoeffIndex] = 1;
      } else if ( Token == DCT_RUN_CATEGORY1B ) {
        /* Bits 0-1 determines the zero run length */
        *CoeffIndex += (6 + (ExtraBits & 0x03));

        /* Bit 2 determines the sign */
        if ( ExtraBits & 0x04 )
          ExpandedBlock[*CoeffIndex] = -1;
        else
          ExpandedBlock[*CoeffIndex] = 1;
      }else{
        /* Bits 0-2 determines the zero run length */
        *CoeffIndex += (10 + (ExtraBits & 0x07));

        /* Bit 3 determines the sign */
        if ( ExtraBits & 0x08 )
          ExpandedBlock[*CoeffIndex] = -1;
        else
          ExpandedBlock[*CoeffIndex] = 1;
      }
    }else{
      /* If token == DCT_RUN_CATEGORY2 we have a single 0 followed by
         a value */
      if ( Token == DCT_RUN_CATEGORY2 ){
        /* Step on by the zero run length */
        *CoeffIndex += 1;

        /* Bit 1 determines sign, bit 0 the value */
        if ( ExtraBits & 0x02 )
          ExpandedBlock[*CoeffIndex] = -(2 + (ExtraBits & 0x01));
        else
          ExpandedBlock[*CoeffIndex] = 2 + (ExtraBits & 0x01);
      }else{
        /* else we have 2->3 zeros followed by a value */
        /* Bit 0 determines the zero run length */
        *CoeffIndex += 2 + (ExtraBits & 0x01);

        /* Bit 2 determines the sign, bit 1 the value */
        if ( ExtraBits & 0x04 )
          ExpandedBlock[*CoeffIndex] = -(2 + ((ExtraBits & 0x02) >> 1));
        else
          ExpandedBlock[*CoeffIndex] = 2 + ((ExtraBits & 0x02) >> 1);
      }
    }

    /* Step on over value */
    *CoeffIndex += 1;

  } else if ( Token == DCT_SHORT_ZRL_TOKEN ) {
    /* Token is a ZRL token so step on by the appropriate number of zeros */
    *CoeffIndex += ExtraBits + 1;
  } else if ( Token == DCT_ZRL_TOKEN ) {
    /* Token is a ZRL token so step on by the appropriate number of zeros */
    *CoeffIndex += ExtraBits + 1;
  } else if ( Token < LOW_VAL_TOKENS ) {
    /* Token is a small single value token. */
    switch ( Token ) {
    case ONE_TOKEN:
      ExpandedBlock[*CoeffIndex] = 1;
      break;
    case MINUS_ONE_TOKEN:
      ExpandedBlock[*CoeffIndex] = -1;
      break;
    case TWO_TOKEN:
      ExpandedBlock[*CoeffIndex] = 2;
      break;
    case MINUS_TWO_TOKEN:
      ExpandedBlock[*CoeffIndex] = -2;
      break;
    }

    /* Step on the coefficient index. */
    *CoeffIndex += 1;
  }else{
    /* Token is a larger single value token */
    /* Expand the token and additional bits to a data value. */
    if ( Token < DCT_VAL_CATEGORY3 ) {
      /* Offset from LOW_VAL_TOKENS determines value */
      Token = Token - LOW_VAL_TOKENS;

      /* Extra bit determines sign */
      if ( ExtraBits )
        ExpandedBlock[*CoeffIndex] =
          -((Q_LIST_ENTRY)(Token + DCT_VAL_CAT2_MIN));
      else
        ExpandedBlock[*CoeffIndex] =
          (Q_LIST_ENTRY)(Token + DCT_VAL_CAT2_MIN);
    } else if ( Token == DCT_VAL_CATEGORY3 ) {
      /* Bit 1 determines sign, Bit 0 the value */
      if ( ExtraBits & 0x02 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT3_MIN + (ExtraBits & 0x01));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT3_MIN + (ExtraBits & 0x01);
    } else if ( Token == DCT_VAL_CATEGORY4 ) {
      /* Bit 2 determines sign, Bit 0-1 the value */
      if ( ExtraBits & 0x04 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT4_MIN + (ExtraBits & 0x03));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT4_MIN + (ExtraBits & 0x03);
    } else if ( Token == DCT_VAL_CATEGORY5 ) {
      /* Bit 3 determines sign, Bit 0-2 the value */
      if ( ExtraBits & 0x08 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT5_MIN + (ExtraBits & 0x07));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT5_MIN + (ExtraBits & 0x07);
    } else if ( Token == DCT_VAL_CATEGORY6 ) {
      /* Bit 4 determines sign, Bit 0-3 the value */
      if ( ExtraBits & 0x10 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT6_MIN + (ExtraBits & 0x0F));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT6_MIN + (ExtraBits & 0x0F);
    } else if ( Token == DCT_VAL_CATEGORY7 ) {
      /* Bit 5 determines sign, Bit 0-4 the value */
      if ( ExtraBits & 0x20 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT7_MIN + (ExtraBits & 0x1F));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT7_MIN + (ExtraBits & 0x1F);
    } else if ( Token == DCT_VAL_CATEGORY8 ) {
      /* Bit 9 determines sign, Bit 0-8 the value */
      if ( ExtraBits & 0x200 )
        ExpandedBlock[*CoeffIndex] = -(DCT_VAL_CAT8_MIN + (ExtraBits & 0x1FF));
      else
        ExpandedBlock[*CoeffIndex] = DCT_VAL_CAT8_MIN + (ExtraBits & 0x1FF);
    }

    /* Step on the coefficient index. */
    *CoeffIndex += 1;
  }
}

void ClearDownQFragData(PB_INSTANCE *pbi){
  ogg_int32_t       i;
  Q_LIST_ENTRY *    QFragPtr;

  for ( i = 0; i < pbi->CodedBlockIndex; i++ ) {
    /* Get the linear index for the current fragment. */
    QFragPtr = pbi->QFragData[pbi->CodedBlockList[i]];
    memset(QFragPtr, 0, 64*sizeof(Q_LIST_ENTRY));
  }
}

static void loop_filter_h(unsigned char * PixelPtr,
                          ogg_int32_t LineLength,
                          ogg_int16_t *BoundingValuePtr){
  ogg_int32_t j;
  ogg_int32_t FiltVal;
  PixelPtr-=2;

  for ( j = 0; j < 8; j++ ){
    FiltVal =
      ( PixelPtr[0] ) -
      ( PixelPtr[1] * 3 ) +
      ( PixelPtr[2] * 3 ) -
      ( PixelPtr[3] );

    FiltVal = *(BoundingValuePtr+((FiltVal + 4) >> 3));

    PixelPtr[1] = clamp255(PixelPtr[1] + FiltVal);
    PixelPtr[2] = clamp255(PixelPtr[2] - FiltVal);

    PixelPtr += LineLength;
  }
}

static void loop_filter_v(unsigned char * PixelPtr,
                          ogg_int32_t LineLength,
                          ogg_int16_t *BoundingValuePtr){
  ogg_int32_t j;
  ogg_int32_t FiltVal;
  PixelPtr -= 2*LineLength;

  for ( j = 0; j < 8; j++ ) {
    FiltVal = ( (ogg_int32_t)PixelPtr[0] ) -
      ( (ogg_int32_t)PixelPtr[LineLength] * 3 ) +
      ( (ogg_int32_t)PixelPtr[2 * LineLength] * 3 ) -
      ( (ogg_int32_t)PixelPtr[3 * LineLength] );

    FiltVal = *(BoundingValuePtr+((FiltVal + 4) >> 3));

    PixelPtr[LineLength] = clamp255(PixelPtr[LineLength] + FiltVal);
    PixelPtr[2 * LineLength] = clamp255(PixelPtr[2*LineLength] - FiltVal);

    PixelPtr ++;
  }
}

static void LoopFilter__c(PB_INSTANCE *pbi, int FLimit){

  int j;
  ogg_int16_t BoundingValues[256];
  ogg_int16_t *bvp = BoundingValues+127;
  unsigned char *cp = pbi->display_fragments;
  ogg_uint32_t *bp = pbi->recon_pixel_index_table;

  if ( FLimit == 0 ) return;
  SetupBoundingValueArray_Generic(BoundingValues, FLimit);

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
            loop_filter_h(&pbi->LastFrameRecon[bp[0]],stride,bvp);
          if(bp_left>bp_begin)
            loop_filter_v(&pbi->LastFrameRecon[bp[0]],stride,bvp);
          if(bp+1<bp_right && !cp[1])
            loop_filter_h(&pbi->LastFrameRecon[bp[0]]+8,stride,bvp);
          if(bp+h<bp_end && !cp[h])
            loop_filter_v(&pbi->LastFrameRecon[bp[h]],stride,bvp);
        }
        bp++;
        cp++;
      }
    }
  }
}

void ReconRefFrames (PB_INSTANCE *pbi){
  ogg_int32_t i;
  unsigned char *SwapReconBuffersTemp;

  /* predictor multiplier up-left, up, up-right,left, shift
     Entries are packed in the order L, UL, U, UR, with missing entries
      moved to the end (before the shift parameters). */
  static const ogg_int16_t pc[16][6]={
    {0,0,0,0,0,0},
    {1,0,0,0,0,0},      /* PL */
    {1,0,0,0,0,0},      /* PUL */
    {1,0,0,0,0,0},      /* PUL|PL */
    {1,0,0,0,0,0},      /* PU */
    {1,1,0,0,1,1},      /* PU|PL */
    {0,1,0,0,0,0},      /* PU|PUL */
    {29,-26,29,0,5,31}, /* PU|PUL|PL */
    {1,0,0,0,0,0},      /* PUR */
    {75,53,0,0,7,127},  /* PUR|PL */
    {1,1,0,0,1,1},      /* PUR|PUL */
    {75,0,53,0,7,127},  /* PUR|PUL|PL */
    {1,0,0,0,0,0},      /* PUR|PU */
    {75,0,53,0,7,127},  /* PUR|PU|PL */
    {3,10,3,0,4,15},    /* PUR|PU|PUL */
    {29,-26,29,0,5,31}  /* PUR|PU|PUL|PL */
  };

  /* boundary case bit masks. */
  static const int bc_mask[8]={
    /* normal case no boundary condition */
    PUR|PU|PUL|PL,
    /* left column */
    PUR|PU,
    /* top row */
    PL,
    /* top row, left column */
    0,
    /* right column */
    PU|PUL|PL,
    /* right and left column */
    PU,
    /* top row, right column */
    PL,
    /* top row, right and left column */
    0
  };

  /* value left value up-left, value up, value up-right, missing
      values skipped. */
  int v[4];

  /* fragment number left, up-left, up, up-right */
  int fn[4];

  /* predictor count. */
  int pcount;

  short wpc;
  static const short Mode2Frame[] = {
    1,  /* CODE_INTER_NO_MV     0 => Encoded diff from same MB last frame  */
    0,  /* CODE_INTRA           1 => DCT Encoded Block */
    1,  /* CODE_INTER_PLUS_MV   2 => Encoded diff from included MV MB last frame */
    1,  /* CODE_INTER_LAST_MV   3 => Encoded diff from MRU MV MB last frame */
    1,  /* CODE_INTER_PRIOR_MV  4 => Encoded diff from included 4 separate MV blocks */
    2,  /* CODE_USING_GOLDEN    5 => Encoded diff from same MB golden frame */
    2,  /* CODE_GOLDEN_MV       6 => Encoded diff from included MV MB golden frame */
    1   /* CODE_INTER_FOUR_MV   7 => Encoded diff from included 4 separate MV blocks */
  };
  short Last[3];
  short PredictedDC;
  int FragsAcross=pbi->HFragments;
  int FromFragment,ToFragment;
  int FragsDown = pbi->VFragments;

  int WhichFrame;
  int WhichCase;
  int j,k,m,n;

  void (*ExpandBlockA) ( PB_INSTANCE *pbi, ogg_int32_t FragmentNumber );

  if ( pbi->FrameType == KEY_FRAME )
    ExpandBlockA=ExpandKFBlock;
  else
    ExpandBlockA=ExpandBlock;

  /* for y,u,v */
  for ( j = 0; j < 3 ; j++) {
    /* pick which fragments based on Y, U, V */
    switch(j){
    case 0: /* y */
      FromFragment = 0;
      ToFragment = pbi->YPlaneFragments;
      FragsAcross = pbi->HFragments;
      FragsDown = pbi->VFragments;
      break;
    case 1: /* u */
      FromFragment = pbi->YPlaneFragments;
      ToFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments ;
      FragsAcross = pbi->HFragments >> 1;
      FragsDown = pbi->VFragments >> 1;
      break;
    /*case 2:  v */
    default:
      FromFragment = pbi->YPlaneFragments + pbi->UVPlaneFragments;
      ToFragment = pbi->YPlaneFragments + (2 * pbi->UVPlaneFragments) ;
      FragsAcross = pbi->HFragments >> 1;
      FragsDown = pbi->VFragments >> 1;
      break;
    }

    /* initialize our array of last used DC Components */
    for(k=0;k<3;k++)
      Last[k]=0;

    i=FromFragment;

    /* do prediction on all of Y, U or V */
    for ( m = 0 ; m < FragsDown ; m++) {
      for ( n = 0 ; n < FragsAcross ; n++, i++){

        /* only do 2 prediction if fragment coded and on non intra or
           if all fragments are intra */
        if( pbi->display_fragments[i] || (pbi->FrameType == KEY_FRAME) ){
          /* Type of Fragment */
          WhichFrame = Mode2Frame[pbi->FragCodingMethod[i]];

          /* Check Borderline Cases */
          WhichCase = (n==0) + ((m==0) << 1) + ((n+1 == FragsAcross) << 2);

          fn[0]=i-1;
          fn[1]=i-FragsAcross-1;
          fn[2]=i-FragsAcross;
          fn[3]=i-FragsAcross+1;

          /* fragment valid for prediction use if coded and it comes
             from same frame as the one we are predicting */
          for(k=pcount=wpc=0; k<4; k++) {
            int pflag;
            pflag=1<<k;
            if((bc_mask[WhichCase]&pflag) &&
               pbi->display_fragments[fn[k]] &&
               (Mode2Frame[pbi->FragCodingMethod[fn[k]]] == WhichFrame)){
              v[pcount]=pbi->QFragData[fn[k]][0];
              wpc|=pflag;
              pcount++;
            }
          }

          if(wpc==0){
            /* fall back to the last coded fragment */
            pbi->QFragData[i][0] += Last[WhichFrame];

          }else{

            /* don't do divide if divisor is 1 or 0 */
            PredictedDC = pc[wpc][0]*v[0];
            for(k=1; k<pcount; k++){
              PredictedDC += pc[wpc][k]*v[k];
            }

            /* if we need to do a shift */
            if(pc[wpc][4] != 0 ){

              /* If negative add in the negative correction factor */
              PredictedDC += (HIGHBITDUPPED(PredictedDC) & pc[wpc][5]);

              /* Shift in lieu of a divide */
              PredictedDC >>= pc[wpc][4];
            }

            /* check for outranging on the two predictors that can outrange */
            if((wpc&(PU|PUL|PL)) == (PU|PUL|PL)){
              if( abs(PredictedDC - v[2]) > 128) {
                PredictedDC = v[2];
              } else if( abs(PredictedDC - v[0]) > 128) {
                PredictedDC = v[0];
              } else if( abs(PredictedDC - v[1]) > 128) {
                PredictedDC = v[1];
              }
            }

            pbi->QFragData[i][0] += PredictedDC;

          }

          /* Save the last fragment coded for whatever frame we are
             predicting from */
          Last[WhichFrame] = pbi->QFragData[i][0];

          /* Inverse DCT and reconstitute buffer in thisframe */
          ExpandBlockA( pbi, i );
        }
      }
    }
  }

  /* Copy the current reconstruction back to the last frame recon buffer. */
  if(pbi->CodedBlockIndex > (ogg_int32_t) (pbi->UnitFragments >> 1)){
    SwapReconBuffersTemp = pbi->ThisFrameRecon;
    pbi->ThisFrameRecon = pbi->LastFrameRecon;
    pbi->LastFrameRecon = SwapReconBuffersTemp;
    CopyNotRecon( pbi, pbi->LastFrameRecon, pbi->ThisFrameRecon );
  }else{
    CopyRecon( pbi, pbi->LastFrameRecon, pbi->ThisFrameRecon );
  }

  /* Apply a loop filter to edge pixels of updated blocks */
  dsp_LoopFilter(pbi->dsp, pbi, pbi->quant_info.loop_filter_limits[pbi->FrameQIndex]);

  /* We may need to update the UMV border */
  UpdateUMVBorder(pbi, pbi->LastFrameRecon);

  /* Reconstruct the golden frame if necessary.
     For VFW codec only on key frames */
  if ( pbi->FrameType == KEY_FRAME ){
    CopyRecon( pbi, pbi->GoldenFrame, pbi->LastFrameRecon );
    /* We may need to update the UMV border */
    UpdateUMVBorder(pbi, pbi->GoldenFrame);
  }
}

void dsp_dct_decode_init (DspFunctions *funcs, ogg_uint32_t cpu_flags)
{
  funcs->LoopFilter = LoopFilter__c;
#if defined(USE_ASM)
  // Todo: Port the dct for MSC one day.
#if !defined (_MSC_VER)
  if (cpu_flags & OC_CPU_X86_MMX) {
    dsp_mmx_dct_decode_init(funcs);
  }
#endif
#endif
}
